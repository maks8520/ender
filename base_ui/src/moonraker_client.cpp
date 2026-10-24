#include "moonraker_client.h"
#include "spdlog/spdlog.h"
#include <cstdio>

MoonrakerClient::MoonrakerClient(KWebSocketClient &ws, MainScreen &screen, std::mutex &lv_lock)
    : NotifyConsumer(lv_lock), ws(ws), screen(screen) {
  screen.set_pause_callback([this]() { send_pause(); });
  screen.set_resume_callback([this]() { send_resume(); });
  screen.set_cancel_callback([this]() { send_cancel(); });
  screen.set_start_callback([this]() { send_start(); });
}

MoonrakerClient::~MoonrakerClient() {}

void MoonrakerClient::init() {
  subscribe();
}

void MoonrakerClient::subscribe() {
  spdlog::info("MoonrakerClient subscribing to printer objects");
  nlohmann::json sub = {
      {"jsonrpc", "2.0"},
      {"method", "printer.objects.subscribe"},
      {"params", {
          {"objects", {
              {"print_stats", nullptr},
              {"extruder", nullptr},
              {"heater_bed", nullptr},
              {"display_status", nullptr}
          }}
      }},
      {"id", 1001}
  };
  ws.send_jsonrpc("printer.objects.subscribe", sub["params"]);
}

void MoonrakerClient::send_pause() {
  spdlog::info("MoonrakerClient: sending PAUSE");
  ws.gcode_script("PAUSE");
}

void MoonrakerClient::send_resume() {
  spdlog::info("MoonrakerClient: sending RESUME");
  ws.gcode_script("RESUME");
}

void MoonrakerClient::send_cancel() {
  spdlog::info("MoonrakerClient: sending CANCEL_PRINT");
  ws.gcode_script("CANCEL_PRINT");
}

void MoonrakerClient::send_start() {
  spdlog::info("MoonrakerClient: sending START / RESUME");
  ws.gcode_script("RESUME");
}

void MoonrakerClient::on_connected() {
  std::lock_guard<std::mutex> lock(lv_lock);
  screen.set_connection_status(true);
  subscribe();
}

void MoonrakerClient::on_disconnected() {
  std::lock_guard<std::mutex> lock(lv_lock);
  screen.set_connection_status(false);
}

void MoonrakerClient::consume(nlohmann::json &j) {
  std::lock_guard<std::mutex> lock(lv_lock);

  if (j.contains("method") && j["method"] == "notify_status_update") {
    if (j.contains("params") && j["params"].is_array() && !j["params"].empty()) {
      parse_printer_objects(j["params"][0]);
    }
  } else if (j.contains("result") && j["result"].contains("status")) {
    parse_printer_objects(j["result"]["status"]);
  }
}

void MoonrakerClient::parse_printer_objects(const nlohmann::json &objects) {
  if (objects.contains("print_stats")) {
    parse_print_stats(objects["print_stats"]);
  }
  if (objects.contains("extruder")) {
    parse_extruder(objects["extruder"]);
  }
  if (objects.contains("heater_bed")) {
    parse_heater_bed(objects["heater_bed"]);
  }
  if (objects.contains("display_status")) {
    parse_display_status(objects["display_status"]);
  }
}

void MoonrakerClient::parse_print_stats(const nlohmann::json &stats) {
  if (stats.contains("state")) {
    std::string state_str = stats["state"].get<std::string>();
    if (state_str == "printing") {
      print_state = "printing";
    } else if (state_str == "paused") {
      print_state = "paused";
    } else {
      print_state = "idle";
    }
    screen.set_print_state(print_state);
  }

  if (stats.contains("total_duration")) {
    total_duration = stats["total_duration"].get<double>();
  }

  if (stats.contains("print_duration")) {
    double print_duration = stats["print_duration"].get<double>();
    if (current_progress > 0 && current_progress < 100 && print_duration > 0) {
      double total_est = (total_duration > print_duration) ? total_duration : (print_duration / (current_progress / 100.0));
      double rem = total_est - print_duration;
      if (rem < 0) rem = 0;
      int rem_sec = static_cast<int>(rem);
      int h = rem_sec / 3600;
      int m = (rem_sec % 3600) / 60;
      int s = rem_sec % 60;
      char buf[32];
      snprintf(buf, sizeof(buf), "%02d:%02d:%02d", h, m, s);
      screen.set_time_remaining(buf);
    } else {
      screen.set_time_remaining("--:--:--");
    }
  }
}

void MoonrakerClient::parse_extruder(const nlohmann::json &ext) {
  if (ext.contains("temperature")) {
    nozzle_temp = static_cast<int>(ext["temperature"].get<double>());
  }
  if (ext.contains("target")) {
    nozzle_target = static_cast<int>(ext["target"].get<double>());
  }
  screen.set_nozzle_temp(nozzle_temp, nozzle_target);
}

void MoonrakerClient::parse_heater_bed(const nlohmann::json &bed) {
  if (bed.contains("temperature")) {
    bed_temp = static_cast<int>(bed["temperature"].get<double>());
  }
  if (bed.contains("target")) {
    bed_target = static_cast<int>(bed["target"].get<double>());
  }
  screen.set_bed_temp(bed_temp, bed_target);
}

void MoonrakerClient::parse_display_status(const nlohmann::json &ds) {
  if (ds.contains("progress")) {
    double prog = ds["progress"].get<double>();
    current_progress = static_cast<int>(prog * 100.0);
    screen.set_print_progress(current_progress);
  }
}
