#include "main_panel.h"
#include "spdlog/spdlog.h"
#include "state.h"
#include "utils.h"
#include <cstdio>

MainPanel::MainPanel(KWebSocketClient &ws, std::mutex &lv_lock, SpoolmanPanel &sm)
    : NotifyConsumer(lv_lock),
      ws(ws),
      homing_panel(ws, lv_lock),
      fan_panel(ws, lv_lock),
      led_panel(ws, lv_lock),
      macros_panel(ws, lv_lock),
      console_panel(ws, lv_lock),
      setting_panel(ws, lv_lock),
      print_status_panel(ws, lv_lock, NULL),
      print_panel(ws, lv_lock),
      printertune_panel(ws, lv_lock),
      extruder_panel(ws, lv_lock),
      prompt_panel(ws, lv_lock),
      spoolman_panel(sm),
      left_zone(NULL),
      nozzle_label(NULL),
      bed_label(NULL),
      fan_label(NULL),
      center_zone(NULL),
      time_rem_label(NULL),
      layer_label(NULL),
      progress_bar(NULL),
      progress_percent_label(NULL),
      bedmesh_btn(NULL),
      right_zone(NULL),
      pause_btn(NULL),
      stop_btn(NULL),
      start_btn(NULL),
      confirm_modal(NULL) {}

MainPanel::~MainPanel() {}

void MainPanel::init(json &data) {}

void MainPanel::subscribe() {}

PrinterTunePanel& MainPanel::get_tune_panel() {
  return printertune_panel;
}

void MainPanel::enable_spoolman() {}

void MainPanel::create_panel() {
  tabview = lv_tabview_create(lv_scr_act(), LV_DIR_TOP, 35);
  lv_obj_set_size(tabview, 480, 320);

  main_tab = lv_tabview_add_tab(tabview, "Главная");
  macros_tab = lv_tabview_add_tab(tabview, "Макросы");
  console_tab = lv_tabview_add_tab(tabview, "Консоль");
  printertune_tab = lv_tabview_add_tab(tabview, "Калибровка");
  setting_tab = lv_tabview_add_tab(tabview, "Настройки");

  create_main(main_tab);
  macros_panel.create_panel(macros_tab);
  console_panel.create_panel(console_tab);
  printertune_panel.create_panel(printertune_tab);
  setting_panel.create_panel(setting_tab);
}

void MainPanel::create_main(lv_obj_t *parent) {
  main_cont = lv_obj_create(parent);
  lv_obj_set_size(main_cont, 480, 280);
  lv_obj_set_pos(main_cont, 0, 0);
  lv_obj_set_style_bg_color(main_cont, lv_color_hex(0x121212), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(main_cont, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_style_border_width(main_cont, 0, LV_PART_MAIN);
  lv_obj_set_style_pad_all(main_cont, 2, LV_PART_MAIN);

  build_left_zone(main_cont);
  build_center_zone(main_cont);
  build_right_zone(main_cont);
  create_cancel_confirm_modal();

  set_print_state("idle");
}

void MainPanel::build_left_zone(lv_obj_t *parent) {
  // Left Zone (35% = ~165px)
  left_zone = lv_obj_create(parent);
  lv_obj_set_size(left_zone, 165, 276);
  lv_obj_set_pos(left_zone, 0, 0);
  lv_obj_set_style_bg_color(left_zone, lv_color_hex(0x1A1A1A), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(left_zone, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_style_border_color(left_zone, lv_color_hex(0x333333), LV_PART_MAIN);
  lv_obj_set_style_border_width(left_zone, 1, LV_PART_MAIN);
  lv_obj_set_style_pad_all(left_zone, 6, LV_PART_MAIN);

  // Nozzle Temperature
  lv_obj_t *nozzle_title = lv_label_create(left_zone);
  lv_label_set_text(nozzle_title, "🔥 Сопло");
  lv_obj_set_style_text_color(nozzle_title, lv_color_hex(0xAAAAAA), LV_PART_MAIN);
  lv_obj_set_pos(nozzle_title, 4, 10);

  nozzle_label = lv_label_create(left_zone);
  lv_label_set_text(nozzle_label, "0° / 0°");
  lv_obj_set_style_text_color(nozzle_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
  lv_obj_set_style_font(nozzle_label, &lv_font_montserrat_20, LV_PART_MAIN);
  lv_obj_set_pos(nozzle_label, 4, 34);

  // Bed Temperature
  lv_obj_t *bed_title = lv_label_create(left_zone);
  lv_label_set_text(bed_title, "🛏 Стол");
  lv_obj_set_style_text_color(bed_title, lv_color_hex(0xAAAAAA), LV_PART_MAIN);
  lv_obj_set_pos(bed_title, 4, 90);

  bed_label = lv_label_create(left_zone);
  lv_label_set_text(bed_label, "0° / 0°");
  lv_obj_set_style_text_color(bed_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
  lv_obj_set_style_font(bed_label, &lv_font_montserrat_20, LV_PART_MAIN);
  lv_obj_set_pos(bed_label, 4, 114);

  // Part Fan Speed
  lv_obj_t *fan_title = lv_label_create(left_zone);
  lv_label_set_text(fan_title, "❄ Обдув");
  lv_obj_set_style_text_color(fan_title, lv_color_hex(0xAAAAAA), LV_PART_MAIN);
  lv_obj_set_pos(fan_title, 4, 170);

  fan_label = lv_label_create(left_zone);
  lv_label_set_text(fan_label, "0%");
  lv_obj_set_style_text_color(fan_label, lv_color_hex(0x00A8FF), LV_PART_MAIN);
  lv_obj_set_style_font(fan_label, &lv_font_montserrat_20, LV_PART_MAIN);
  lv_obj_set_pos(fan_label, 4, 194);
}

void MainPanel::build_center_zone(lv_obj_t *parent) {
  // Center Zone (45% = ~212px)
  center_zone = lv_obj_create(parent);
  lv_obj_set_size(center_zone, 212, 276);
  lv_obj_set_pos(center_zone, 170, 0);
  lv_obj_set_style_bg_color(center_zone, lv_color_hex(0x181818), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(center_zone, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_style_border_color(center_zone, lv_color_hex(0x333333), LV_PART_MAIN);
  lv_obj_set_style_border_width(center_zone, 1, LV_PART_MAIN);
  lv_obj_set_style_pad_all(center_zone, 6, LV_PART_MAIN);

  // Remaining Time (Huge)
  lv_obj_t *time_title = lv_label_create(center_zone);
  lv_label_set_text(time_title, "Осталось времени");
  lv_obj_set_style_text_color(time_title, lv_color_hex(0x888888), LV_PART_MAIN);
  lv_obj_set_pos(time_title, 6, 6);

  time_rem_label = lv_label_create(center_zone);
  lv_label_set_text(time_rem_label, "00:00:00");
  lv_obj_set_style_text_color(time_rem_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
  lv_obj_set_style_font(time_rem_label, &lv_font_montserrat_28, LV_PART_MAIN);
  lv_obj_set_pos(time_rem_label, 6, 26);

  // Layer status
  layer_label = lv_label_create(center_zone);
  lv_label_set_text(layer_label, "Слой: 0 из 0");
  lv_obj_set_style_text_color(layer_label, lv_color_hex(0xCCCCCC), LV_PART_MAIN);
  lv_obj_set_pos(layer_label, 6, 68);

  // Thick Progress Bar
  progress_bar = lv_bar_create(center_zone);
  lv_obj_set_size(progress_bar, 196, 36);
  lv_obj_set_pos(progress_bar, 6, 96);
  lv_bar_set_range(progress_bar, 0, 100);
  lv_bar_set_value(progress_bar, 0, LV_ANIM_OFF);
  lv_obj_set_style_bg_color(progress_bar, lv_color_hex(0x333333), LV_PART_MAIN);
  lv_obj_set_style_bg_color(progress_bar, lv_color_hex(0x00A8FF), LV_PART_INDICATOR);

  progress_percent_label = lv_label_create(progress_bar);
  lv_label_set_text(progress_percent_label, "0%");
  lv_obj_set_style_text_color(progress_percent_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
  lv_obj_center(progress_percent_label);

  // Bed Mesh heatmap button
  bedmesh_btn = lv_btn_create(center_zone);
  lv_obj_set_size(bedmesh_btn, 196, 44);
  lv_obj_set_pos(bedmesh_btn, 6, 214);
  lv_obj_set_style_bg_color(bedmesh_btn, lv_color_hex(0x2B2B2B), LV_PART_MAIN);
  lv_obj_set_style_shadow_width(bedmesh_btn, 0, LV_PART_MAIN);
  lv_obj_add_event_cb(bedmesh_btn, _handle_bedmesh_click, LV_EVENT_CLICKED, this);

  lv_obj_t *bm_lbl = lv_label_create(bedmesh_btn);
  lv_label_set_text(bm_lbl, "📊 Сетка стола");
  lv_obj_set_style_text_color(bm_lbl, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
  lv_obj_center(bm_lbl);
}

void MainPanel::build_right_zone(lv_obj_t *parent) {
  // Right Zone (20% = ~92px)
  right_zone = lv_obj_create(parent);
  lv_obj_set_size(right_zone, 92, 276);
  lv_obj_set_pos(right_zone, 386, 0);
  lv_obj_set_style_bg_color(right_zone, lv_color_hex(0x121212), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(right_zone, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_style_border_width(right_zone, 0, LV_PART_MAIN);
  lv_obj_set_style_pad_all(right_zone, 0, LV_PART_MAIN);

  // Giant Print Button (Idle mode)
  start_btn = lv_btn_create(right_zone);
  lv_obj_set_size(start_btn, 88, 272);
  lv_obj_set_pos(start_btn, 2, 2);
  lv_obj_set_style_bg_color(start_btn, lv_color_hex(0x00CC66), LV_PART_MAIN); // Green
  lv_obj_set_style_shadow_width(start_btn, 0, LV_PART_MAIN);
  lv_obj_add_event_cb(start_btn, _handle_start_click, LV_EVENT_CLICKED, this);

  lv_obj_t *start_lbl = lv_label_create(start_btn);
  lv_label_set_text(start_lbl, "▶\nПЕЧАТЬ");
  lv_obj_set_style_text_align(start_lbl, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
  lv_obj_set_style_text_color(start_lbl, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
  lv_obj_center(start_lbl);

  // Giant Pause Button (Printing mode)
  pause_btn = lv_btn_create(right_zone);
  lv_obj_set_size(pause_btn, 88, 132);
  lv_obj_set_pos(pause_btn, 2, 2);
  lv_obj_set_style_bg_color(pause_btn, lv_color_hex(0xFFA500), LV_PART_MAIN); // Yellow
  lv_obj_set_style_shadow_width(pause_btn, 0, LV_PART_MAIN);
  lv_obj_add_event_cb(pause_btn, _handle_pause_click, LV_EVENT_CLICKED, this);

  lv_obj_t *pause_lbl = lv_label_create(pause_btn);
  lv_label_set_text(pause_lbl, "⏸\nПАУЗА");
  lv_obj_set_style_text_align(pause_lbl, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
  lv_obj_set_style_text_color(pause_lbl, lv_color_hex(0x000000), LV_PART_MAIN);
  lv_obj_center(pause_lbl);

  // Giant Stop Button (Printing mode)
  stop_btn = lv_btn_create(right_zone);
  lv_obj_set_size(stop_btn, 88, 132);
  lv_obj_set_pos(stop_btn, 2, 140);
  lv_obj_set_style_bg_color(stop_btn, lv_color_hex(0xFF0000), LV_PART_MAIN); // Red
  lv_obj_set_style_shadow_width(stop_btn, 0, LV_PART_MAIN);
  lv_obj_add_event_cb(stop_btn, _handle_stop_click, LV_EVENT_CLICKED, this);

  lv_obj_t *stop_lbl = lv_label_create(stop_btn);
  lv_label_set_text(stop_lbl, "⏹\nСТОП");
  lv_obj_set_style_text_align(stop_lbl, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
  lv_obj_set_style_text_color(stop_lbl, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
  lv_obj_center(stop_lbl);
}

void MainPanel::create_cancel_confirm_modal() {
  confirm_modal = lv_obj_create(lv_scr_act());
  lv_obj_set_size(confirm_modal, 380, 220);
  lv_obj_center(confirm_modal);
  lv_obj_set_style_bg_color(confirm_modal, lv_color_hex(0x222222), LV_PART_MAIN);
  lv_obj_set_style_border_color(confirm_modal, lv_color_hex(0xFF0000), LV_PART_MAIN);
  lv_obj_set_style_border_width(confirm_modal, 2, LV_PART_MAIN);
  lv_obj_add_flag(confirm_modal, LV_OBJ_FLAG_HIDDEN);

  lv_obj_t *title = lv_label_create(confirm_modal);
  lv_label_set_text(title, "Подтверждение отмены");
  lv_obj_set_style_text_color(title, lv_color_hex(0xFF5555), LV_PART_MAIN);
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);

  lv_obj_t *msg = lv_label_create(confirm_modal);
  lv_label_set_text(msg, "Вы уверены, что хотите\nотменить текущую печать?");
  lv_obj_set_style_text_align(msg, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
  lv_obj_set_style_text_color(msg, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
  lv_obj_align(msg, LV_ALIGN_CENTER, 0, -10);

  lv_obj_t *yes_btn = lv_btn_create(confirm_modal);
  lv_obj_set_size(yes_btn, 140, 42);
  lv_obj_align(yes_btn, LV_ALIGN_BOTTOM_LEFT, 10, -10);
  lv_obj_set_style_bg_color(yes_btn, lv_color_hex(0xFF0000), LV_PART_MAIN);
  lv_obj_add_event_cb(yes_btn, _handle_confirm_cancel, LV_EVENT_CLICKED, this);

  lv_obj_t *yes_lbl = lv_label_create(yes_btn);
  lv_label_set_text(yes_lbl, "Да, отменить");
  lv_obj_center(yes_lbl);

  lv_obj_t *no_btn = lv_btn_create(confirm_modal);
  lv_obj_set_size(no_btn, 140, 42);
  lv_obj_align(no_btn, LV_ALIGN_BOTTOM_RIGHT, -10, -10);
  lv_obj_set_style_bg_color(no_btn, lv_color_hex(0x555555), LV_PART_MAIN);
  lv_obj_add_event_cb(no_btn, _handle_dismiss_cancel, LV_EVENT_CLICKED, this);

  lv_obj_t *no_lbl = lv_label_create(no_btn);
  lv_label_set_text(no_lbl, "Отмена");
  lv_obj_center(no_lbl);
}

void MainPanel::set_nozzle_temp(int current, int target) {
  cur_nozzle = current;
  tgt_nozzle = target;
  if (nozzle_label) {
    char buf[32];
    snprintf(buf, sizeof(buf), "%d° / %d°", current, target);
    lv_label_set_text(nozzle_label, buf);

    if (target > 0 && current < target - 2) {
      lv_obj_set_style_text_color(nozzle_label, lv_color_hex(0x3399FF), LV_PART_MAIN); // Blue = heating
    } else if (target > 0 && current >= target - 2) {
      lv_obj_set_style_text_color(nozzle_label, lv_color_hex(0x00CC66), LV_PART_MAIN); // Green = target reached
    } else {
      lv_obj_set_style_text_color(nozzle_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    }
    lv_obj_invalidate(nozzle_label);
  }
}

void MainPanel::set_bed_temp(int current, int target) {
  cur_bed = current;
  tgt_bed = target;
  if (bed_label) {
    char buf[32];
    snprintf(buf, sizeof(buf), "%d° / %d°", current, target);
    lv_label_set_text(bed_label, buf);

    if (target > 0 && current < target - 2) {
      lv_obj_set_style_text_color(bed_label, lv_color_hex(0x3399FF), LV_PART_MAIN); // Blue = heating
    } else if (target > 0 && current >= target - 2) {
      lv_obj_set_style_text_color(bed_label, lv_color_hex(0x00CC66), LV_PART_MAIN); // Green = target reached
    } else {
      lv_obj_set_style_text_color(bed_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    }
    lv_obj_invalidate(bed_label);
  }
}

void MainPanel::set_fan_speed(int speed_percent) {
  cur_fan = speed_percent;
  if (fan_label) {
    char buf[32];
    snprintf(buf, sizeof(buf), "%d%%", speed_percent);
    lv_label_set_text(fan_label, buf);
    lv_obj_invalidate(fan_label);
  }
}

void MainPanel::set_time_remaining(const std::string &time_str) {
  if (time_rem_label) {
    lv_label_set_text(time_rem_label, time_str.c_str());
    lv_obj_invalidate(time_rem_label);
  }
}

void MainPanel::set_layer_info(int current_layer, int total_layers) {
  if (layer_label) {
    char buf[64];
    snprintf(buf, sizeof(buf), "Слой: %d из %d", current_layer, total_layers);
    lv_label_set_text(layer_label, buf);
    lv_obj_invalidate(layer_label);
  }
}

void MainPanel::set_print_progress(int percent) {
  if (percent < 0) percent = 0;
  if (percent > 100) percent = 100;
  if (cur_progress != percent) {
    cur_progress = percent;
    if (progress_bar) lv_bar_set_value(progress_bar, percent, LV_ANIM_OFF);
    if (progress_percent_label) {
      char buf[32];
      snprintf(buf, sizeof(buf), "%d%%", percent);
      lv_label_set_text(progress_percent_label, buf);
      lv_obj_invalidate(progress_percent_label);
    }
  }
}

void MainPanel::set_print_state(const std::string &state) {
  current_state = state;
  if (state == "printing" || state == "paused") {
    lv_obj_add_flag(start_btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(pause_btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(stop_btn, LV_OBJ_FLAG_HIDDEN);
  } else {
    lv_obj_clear_flag(start_btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(pause_btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(stop_btn, LV_OBJ_FLAG_HIDDEN);
  }
}

void MainPanel::consume(json &data) {}

void MainPanel::_handle_pause_click(lv_event_t *e) {
  MainPanel *panel = static_cast<MainPanel*>(lv_event_get_user_data(e));
  if (panel->current_state == "paused") {
    panel->ws.gcode_script("_SMART_RESUME");
  } else {
    panel->ws.gcode_script("_SMART_PAUSE");
  }
}

void MainPanel::_handle_stop_click(lv_event_t *e) {
  MainPanel *panel = static_cast<MainPanel*>(lv_event_get_user_data(e));
  if (panel->confirm_modal) {
    lv_obj_clear_flag(panel->confirm_modal, LV_OBJ_FLAG_HIDDEN);
  }
}

void MainPanel::_handle_start_click(lv_event_t *e) {
  MainPanel *panel = static_cast<MainPanel*>(lv_event_get_user_data(e));
  // Switch to file selection or resume print
  lv_tabview_set_act(panel->tabview, 0, LV_ANIM_ON);
}

void MainPanel::_handle_bedmesh_click(lv_event_t *e) {
  MainPanel *panel = static_cast<MainPanel*>(lv_event_get_user_data(e));
  lv_tabview_set_act(panel->tabview, 3, LV_ANIM_ON); // Switch to Tune/BedMesh tab
}

void MainPanel::_handle_confirm_cancel(lv_event_t *e) {
  MainPanel *panel = static_cast<MainPanel*>(lv_event_get_user_data(e));
  if (panel->confirm_modal) {
    lv_obj_add_flag(panel->confirm_modal, LV_OBJ_FLAG_HIDDEN);
  }
  panel->ws.gcode_script("CANCEL_PRINT");
}

void MainPanel::_handle_dismiss_cancel(lv_event_t *e) {
  MainPanel *panel = static_cast<MainPanel*>(lv_event_get_user_data(e));
  if (panel->confirm_modal) {
    lv_obj_add_flag(panel->confirm_modal, LV_OBJ_FLAG_HIDDEN);
  }
}
