#ifndef __MOONRAKER_CLIENT_H__
#define __MOONRAKER_CLIENT_H__

#include "websocket_client.h"
#include "notify_consumer.h"
#include "main_screen.h"
#include "hv/json.hpp"
#include <string>
#include <mutex>
#include <memory>

class MoonrakerClient : public NotifyConsumer {
 public:
  MoonrakerClient(KWebSocketClient &ws, MainScreen &screen, std::mutex &lv_lock);
  virtual ~MoonrakerClient();

  void init();
  void subscribe();
  virtual void consume(nlohmann::json &j) override;

  // Command handlers
  void send_pause();
  void send_resume();
  void send_cancel();
  void send_start();

  void on_connected();
  void on_disconnected();

 private:
  void parse_printer_objects(const nlohmann::json &objects);
  void parse_print_stats(const nlohmann::json &stats);
  void parse_extruder(const nlohmann::json &ext);
  void parse_heater_bed(const nlohmann::json &bed);
  void parse_display_status(const nlohmann::json &ds);

  KWebSocketClient &ws;
  MainScreen &screen;

  int current_progress = 0;
  int nozzle_temp = 0, nozzle_target = 0;
  int bed_temp = 0, bed_target = 0;
  double total_duration = 0.0;
  std::string print_state = "idle";
};

#endif // __MOONRAKER_CLIENT_H__
