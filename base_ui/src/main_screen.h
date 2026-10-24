#ifndef __MAIN_SCREEN_H__
#define __MAIN_SCREEN_H__

#include "lvgl/lvgl.h"
#include <string>
#include <functional>

class MainScreen {
 public:
  MainScreen();
  ~MainScreen();

  // Create LVGL 480x320 landscape layout
  void create(lv_obj_t *parent = NULL);

  // UI state update methods (uses partial invalidation)
  void set_print_progress(int percent);
  void set_time_remaining(const std::string &time_str);
  void set_nozzle_temp(int current, int target);
  void set_bed_temp(int current, int target);
  void set_print_state(const std::string &state); // "printing", "paused", "complete", "idle"
  void set_connection_status(bool connected);
  void set_camera_offline(bool offline);

  // Access camera parent container (left zone)
  lv_obj_t* get_camera_container() { return camera_cont; }

  // Callbacks for UI events
  void set_pause_callback(std::function<void()> cb) { pause_cb = cb; }
  void set_resume_callback(std::function<void()> cb) { resume_cb = cb; }
  void set_cancel_callback(std::function<void()> cb) { cancel_cb = cb; }
  void set_start_callback(std::function<void()> cb) { start_cb = cb; }

 private:
  void build_left_zone(lv_obj_t *parent);
  void build_center_zone(lv_obj_t *parent);
  void build_right_zone(lv_obj_t *parent);
  void build_header(lv_obj_t *parent);
  void create_stop_confirm_modal();

  lv_obj_t *screen_obj;
  lv_obj_t *header_cont;
  lv_obj_t *conn_icon_label;
  lv_obj_t *state_title_label;

  // Left Zone
  lv_obj_t *left_zone;
  lv_obj_t *camera_cont;
  lv_obj_t *cam_offline_cont;
  lv_obj_t *cam_offline_label;

  // Center Zone
  lv_obj_t *center_zone;
  lv_obj_t *progress_bar;
  lv_obj_t *percent_label;
  lv_obj_t *time_label;
  lv_obj_t *nozzle_label;
  lv_obj_t *bed_label;

  // Right Zone
  lv_obj_t *right_zone;
  lv_obj_t *pause_btn;
  lv_obj_t *pause_btn_label;
  lv_obj_t *stop_btn;
  lv_obj_t *stop_btn_label;
  lv_obj_t *start_btn;
  lv_obj_t *start_btn_label;

  // Confirmation modal
  lv_obj_t *confirm_modal;

  // Internal state tracking
  int current_progress = 0;
  int nozzle_cur = 0, nozzle_tgt = 0;
  int bed_cur = 0, bed_tgt = 0;
  std::string print_state = "idle";
  bool is_connected = true;

  std::function<void()> pause_cb;
  std::function<void()> resume_cb;
  std::function<void()> cancel_cb;
  std::function<void()> start_cb;

  static void _handle_pause_click(lv_event_t *e);
  static void _handle_stop_click(lv_event_t *e);
  static void _handle_start_click(lv_event_t *e);
  static void _handle_confirm_stop_click(lv_event_t *e);
  static void _handle_cancel_stop_click(lv_event_t *e);
};

#endif // __MAIN_SCREEN_H__
