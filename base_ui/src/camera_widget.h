#ifndef __CAMERA_WIDGET_H__
#define __CAMERA_WIDGET_H__

#include "lvgl/lvgl.h"
#include "main_screen.h"
#include <string>
#include <atomic>
#include <thread>
#include <mutex>
#include <vector>

class CameraWidget {
 public:
  CameraWidget(MainScreen &screen, std::mutex &lv_lock);
  ~CameraWidget();

  void init(const std::string &stream_url, int target_fps = 12, bool enabled = true);
  void start();
  void stop();

  bool is_running() const { return running.load(); }

 private:
  void stream_thread_proc();
  void update_frame_canvas(const std::vector<uint8_t> &jpeg_data);

  MainScreen &screen;
  std::mutex &lv_lock;

  std::string stream_url;
  int target_fps;
  bool enabled;

  std::atomic<bool> running{false};
  std::thread worker_thread;

  lv_obj_t *img_obj;
  lv_img_dsc_t img_dsc;
  std::vector<uint8_t> frame_buffer;

  bool is_offline = true;
};

#endif // __CAMERA_WIDGET_H__
