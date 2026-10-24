#include "camera_widget.h"
#include "spdlog/spdlog.h"
#include <chrono>
#include <sstream>
#include <cstring>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

CameraWidget::CameraWidget(MainScreen &screen, std::mutex &lv_lock)
    : screen(screen),
      lv_lock(lv_lock),
      target_fps(12),
      enabled(true),
      img_obj(NULL) {
  std::memset(&img_dsc, 0, sizeof(img_dsc));
}

CameraWidget::~CameraWidget() {
  stop();
}

void CameraWidget::init(const std::string &url, int fps, bool is_enabled) {
  stream_url = url;
  target_fps = (fps > 0 && fps <= 30) ? fps : 12;
  enabled = is_enabled;

  if (!enabled) {
    screen.set_camera_offline(true);
    return;
  }

  std::lock_guard<std::mutex> lock(lv_lock);
  lv_obj_t *parent = screen.get_camera_container();
  if (parent) {
    img_obj = lv_img_create(parent);
    lv_obj_set_size(img_obj, 188, 282);
    lv_obj_center(img_obj);
  }
}

void CameraWidget::start() {
  if (!enabled || running.load()) return;
  running.store(true);
  worker_thread = std::thread(&CameraWidget::stream_thread_proc, this);
}

void CameraWidget::stop() {
  if (!running.load()) return;
  running.store(false);
  if (worker_thread.joinable()) {
    worker_thread.join();
  }
}

void CameraWidget::stream_thread_proc() {
  spdlog::info("CameraWidget stream thread started for URL: {}", stream_url);

  // Parse URL: http://host:port/path
  std::string host = "127.0.0.1";
  int port = 8080;
  std::string path = "/?action=stream";

  size_t proto_end = stream_url.find("://");
  std::string url_noproto = (proto_end != std::string::npos) ? stream_url.substr(proto_end + 3) : stream_url;
  size_t path_start = url_noproto.find('/');
  std::string host_port = (path_start != std::string::npos) ? url_noproto.substr(0, path_start) : url_noproto;
  if (path_start != std::string::npos) {
    path = url_noproto.substr(path_start);
  }

  size_t colon = host_port.find(':');
  if (colon != std::string::npos) {
    host = host_port.substr(0, colon);
    port = std::stoi(host_port.substr(colon + 1));
  } else {
    host = host_port;
  }

  auto last_frame_time = std::chrono::steady_clock::now();
  int frame_interval_ms = 1000 / target_fps;

  while (running.load()) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
      is_offline = true;
      {
        std::lock_guard<std::mutex> lock(lv_lock);
        screen.set_camera_offline(true);
      }
      std::this_thread::sleep_for(std::chrono::seconds(2));
      continue;
    }

    struct sockaddr_in serv_addr;
    std::memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    inet_pton(AF_INET, host.c_str(), &serv_addr.sin_addr);

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
      close(sockfd);
      is_offline = true;
      {
        std::lock_guard<std::mutex> lock(lv_lock);
        screen.set_camera_offline(true);
      }
      std::this_thread::sleep_for(std::chrono::seconds(2));
      continue;
    }

    std::ostringstream req;
    req << "GET " << path << " HTTP/1.1\r\n"
        << "Host: " << host << ":" << port << "\r\n"
        << "User-Agent: GuppyScreen\r\n"
        << "Connection: keep-alive\r\n\r\n";
    std::string req_str = req.str();
    send(sockfd, req_str.c_str(), req_str.size(), 0);

    std::vector<uint8_t> buffer;
    buffer.reserve(262144);
    uint8_t chunk[8192];

    while (running.load()) {
      ssize_t bytes_read = recv(sockfd, chunk, sizeof(chunk), 0);
      if (bytes_read <= 0) break;

      buffer.insert(buffer.end(), chunk, chunk + bytes_read);

      // Extract the LATEST complete JPEG frame to prevent socket buffering lag
      size_t latest_soi = std::string::npos;
      size_t latest_eoi = std::string::npos;

      for (size_t i = 0; i + 1 < buffer.size(); ++i) {
        if (buffer[i] == 0xFF && buffer[i + 1] == 0xD8) {
          latest_soi = i;
        } else if (buffer[i] == 0xFF && buffer[i + 1] == 0xD9 && latest_soi != std::string::npos && i > latest_soi) {
          latest_eoi = i + 2;
        }
      }

      if (latest_soi != std::string::npos && latest_eoi != std::string::npos && latest_eoi > latest_soi) {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_frame_time).count();
        if (elapsed >= frame_interval_ms) {
          std::vector<uint8_t> jpeg_frame(buffer.begin() + latest_soi, buffer.begin() + latest_eoi);
          update_frame_canvas(jpeg_frame);
          last_frame_time = now;
        }
        buffer.erase(buffer.begin(), buffer.begin() + latest_eoi);
      }

      if (buffer.size() > 524288) {
        buffer.clear();
      }
    }

    close(sockfd);
  }
  spdlog::info("CameraWidget stream thread stopped");
}

void CameraWidget::update_frame_canvas(const std::vector<uint8_t> &jpeg_data) {
  if (jpeg_data.empty()) return;

  std::lock_guard<std::mutex> lock(lv_lock);
  if (!img_obj) return;

  frame_buffer = jpeg_data;
  img_dsc.header.always_zero = 0;
  img_dsc.header.w = 0;
  img_dsc.header.h = 0;
  img_dsc.header.cf = LV_IMG_CF_RAW;
  img_dsc.data_size = frame_buffer.size();
  img_dsc.data = frame_buffer.data();

  lv_img_cache_invalidate_src(&img_dsc);
  lv_img_set_src(img_obj, NULL);
  lv_img_set_src(img_obj, &img_dsc);
  lv_obj_invalidate(img_obj);

  if (is_offline) {
    is_offline = false;
    screen.set_camera_offline(false);
  }
}
