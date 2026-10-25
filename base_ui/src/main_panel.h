#ifndef __MAIN_PANEL_H__
#define __MAIN_PANEL_H__

#include "websocket_client.h"
#include "notify_consumer.h"
#include "sensor_container.h"
#include "button_container.h"
#include "prompt_panel.h"
#include "numpad.h"
#include "homing_panel.h"
#include "extruder_panel.h"
#include "fan_panel.h"
#include "led_panel.h"
#include "print_panel.h"
#include "macros_panel.h"
#include "console_panel.h"
#include "printertune_panel.h"
#include "setting_panel.h"
#include "print_status_panel.h"
#include "spoolman_panel.h"
#include "finetune_panel.h"
#include "zoffset_panel.h"
#include "lvgl/lvgl.h"

#include <mutex>
#include <map>
#include <memory>

class MainPanel : public NotifyConsumer {
 public:
  MainPanel(KWebSocketClient &ws, std::mutex &lv_lock, SpoolmanPanel &sm);
  ~MainPanel();

  virtual void consume(json &data) override;
  void init(json &data);
  void subscribe();
  PrinterTunePanel& get_tune_panel();
  void enable_spoolman();
  bool is_prompt_visible() const { return prompt_panel.is_visible(); }

  void create_panel();
  void create_main(lv_obj_t *parent);

  void set_nozzle_temp(int current, int target);
  void set_bed_temp(int current, int target);
  void set_fan_speed(int speed_percent);
  void set_time_remaining(const std::string &time_str);
  void set_layer_info(int current_layer, int total_layers);
  void set_print_progress(int percent);
  void set_print_state(const std::string &state);

  void show_console();
  lv_obj_t* get_main_container() { return main_cont; }

 private:
  void build_left_zone(lv_obj_t *parent);
  void build_center_zone(lv_obj_t *parent);
  void build_right_zone(lv_obj_t *parent);
  void create_cancel_confirm_modal();

  KWebSocketClient &ws;
  HomingPanel homing_panel;
  FanPanel fan_panel;
  LedPanel led_panel;
  FineTunePanel finetune_panel;
  ZOffsetPanel zoffset_panel;
  PrintStatusPanel print_status_panel;
  PrintPanel print_panel;
  Numpad numpad;
  ExtruderPanel extruder_panel;
  PromptPanel prompt_panel;
  SpoolmanPanel &spoolman_panel;

  lv_obj_t *tabview;
  lv_obj_t *main_tab;
  lv_obj_t *macros_tab;
  std::unique_ptr<MacrosPanel> macros_panel;
  lv_obj_t *console_tab;
  std::unique_ptr<ConsolePanel> console_panel;
  lv_obj_t *printertune_tab;
  std::unique_ptr<PrinterTunePanel> printertune_panel;
  lv_obj_t *setting_tab;
  std::unique_ptr<SettingPanel> setting_panel;
  lv_obj_t *main_cont;

  // 3-Zone Refined Layout Objects (480x272)
  lv_obj_t *left_zone;
  lv_obj_t *nozzle_label;
  lv_obj_t *bed_label;
  lv_obj_t *fan_label;

  lv_obj_t *center_zone;
  lv_obj_t *time_rem_label;
  lv_obj_t *layer_label;
  lv_obj_t *progress_bar;
  lv_obj_t *progress_percent_label;
  lv_obj_t *bedmesh_btn;
  lv_obj_t *finetune_btn;

  lv_obj_t *right_zone;
  lv_obj_t *pause_btn;
  lv_obj_t *stop_btn;
  lv_obj_t *start_btn;

  lv_obj_t *confirm_modal;

  int cur_nozzle = 0, tgt_nozzle = 0;
  int cur_bed = 0, tgt_bed = 0;
  int cur_fan = 0;
  int cur_progress = 0;
  std::string current_state = "idle";

  static void _handle_pause_click(lv_event_t *e);
  static void _handle_stop_click(lv_event_t *e);
  static void _handle_start_click(lv_event_t *e);
  static void _handle_bedmesh_click(lv_event_t *e);
  static void _handle_finetune_click(lv_event_t *e);
  static void _handle_confirm_cancel(lv_event_t *e);
  static void _handle_dismiss_cancel(lv_event_t *e);
};

#endif // __MAIN_PANEL_H__
