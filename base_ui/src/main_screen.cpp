#include "main_screen.h"
#include "spdlog/spdlog.h"
#include <cstdio>

MainScreen::MainScreen()
    : screen_obj(NULL),
      header_cont(NULL),
      conn_icon_label(NULL),
      state_title_label(NULL),
      left_zone(NULL),
      camera_cont(NULL),
      cam_offline_cont(NULL),
      cam_offline_label(NULL),
      center_zone(NULL),
      progress_bar(NULL),
      percent_label(NULL),
      time_label(NULL),
      nozzle_label(NULL),
      bed_label(NULL),
      right_zone(NULL),
      pause_btn(NULL),
      pause_btn_label(NULL),
      stop_btn(NULL),
      stop_btn_label(NULL),
      start_btn(NULL),
      start_btn_label(NULL),
      confirm_modal(NULL) {}

MainScreen::~MainScreen() {}

void MainScreen::create(lv_obj_t *parent) {
  if (parent == NULL) {
    parent = lv_scr_act();
  }
  screen_obj = parent;

  // Set strict Flat Design for main background
  lv_obj_set_size(screen_obj, 480, 320);
  lv_obj_set_style_bg_color(screen_obj, lv_color_hex(0x121212), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(screen_obj, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_style_pad_all(screen_obj, 0, LV_PART_MAIN);

  build_header(screen_obj);

  // Main content area below header (480x290)
  lv_obj_t *content_area = lv_obj_create(screen_obj);
  lv_obj_set_size(content_area, 480, 290);
  lv_obj_set_pos(content_area, 0, 30);
  lv_obj_set_style_bg_opa(content_area, LV_OPA_TRANSP, LV_PART_MAIN);
  lv_obj_set_style_border_width(content_area, 0, LV_PART_MAIN);
  lv_obj_set_style_pad_all(content_area, 4, LV_PART_MAIN);

  build_left_zone(content_area);
  build_center_zone(content_area);
  build_right_zone(content_area);
  create_stop_confirm_modal();

  set_print_state("idle");
  set_connection_status(true);
}

void MainScreen::build_header(lv_obj_t *parent) {
  header_cont = lv_obj_create(parent);
  lv_obj_set_size(header_cont, 480, 30);
  lv_obj_set_pos(header_cont, 0, 0);
  lv_obj_set_style_bg_color(header_cont, lv_color_hex(0x1F1F1F), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(header_cont, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_style_border_width(header_cont, 0, LV_PART_MAIN);
  lv_obj_set_style_pad_hor(header_cont, 8, LV_PART_MAIN);
  lv_obj_set_style_pad_ver(header_cont, 4, LV_PART_MAIN);

  state_title_label = lv_label_create(header_cont);
  lv_label_set_text(state_title_label, "Ender 3 V3 KE - Простой");
  lv_obj_set_style_text_color(state_title_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
  lv_obj_align(state_title_label, LV_ALIGN_LEFT_MID, 0, 0);

  conn_icon_label = lv_label_create(header_cont);
  lv_label_set_text(conn_icon_label, "");
  lv_obj_set_style_text_color(conn_icon_label, lv_color_hex(0xFF3333), LV_PART_MAIN);
  lv_obj_align(conn_icon_label, LV_ALIGN_RIGHT_MID, 0, 0);
}

void MainScreen::build_left_zone(lv_obj_t *parent) {
  // Left Zone: Camera Stream (Width ~188px, Height 282px)
  left_zone = lv_obj_create(parent);
  lv_obj_set_size(left_zone, 188, 282);
  lv_obj_set_pos(left_zone, 0, 0);
  lv_obj_set_style_bg_color(left_zone, lv_color_hex(0x1E1E1E), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(left_zone, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_style_border_color(left_zone, lv_color_hex(0x333333), LV_PART_MAIN);
  lv_obj_set_style_border_width(left_zone, 1, LV_PART_MAIN);
  lv_obj_set_style_pad_all(left_zone, 0, LV_PART_MAIN);

  camera_cont = lv_obj_create(left_zone);
  lv_obj_set_size(camera_cont, 188, 282);
  lv_obj_set_pos(camera_cont, 0, 0);
  lv_obj_set_style_bg_opa(camera_cont, LV_OPA_TRANSP, LV_PART_MAIN);
  lv_obj_set_style_border_width(camera_cont, 0, LV_PART_MAIN);
  lv_obj_set_style_pad_all(camera_cont, 0, LV_PART_MAIN);

  // Camera offline placeholder
  cam_offline_cont = lv_obj_create(left_zone);
  lv_obj_set_size(cam_offline_cont, 188, 282);
  lv_obj_set_pos(cam_offline_cont, 0, 0);
  lv_obj_set_style_bg_color(cam_offline_cont, lv_color_hex(0x222222), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(cam_offline_cont, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_style_border_width(cam_offline_cont, 0, LV_PART_MAIN);

  cam_offline_label = lv_label_create(cam_offline_cont);
  lv_label_set_text(cam_offline_label, "📷\nКамера offline");
  lv_obj_set_style_text_align(cam_offline_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
  lv_obj_set_style_text_color(cam_offline_label, lv_color_hex(0xAAAAAA), LV_PART_MAIN);
  lv_obj_center(cam_offline_label);
}

void MainScreen::build_center_zone(lv_obj_t *parent) {
  // Center Zone: Telemetry & Progress (Width ~188px, Height 282px)
  center_zone = lv_obj_create(parent);
  lv_obj_set_size(center_zone, 188, 282);
  lv_obj_set_pos(center_zone, 192, 0);
  lv_obj_set_style_bg_color(center_zone, lv_color_hex(0x181818), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(center_zone, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_style_border_color(center_zone, lv_color_hex(0x333333), LV_PART_MAIN);
  lv_obj_set_style_border_width(center_zone, 1, LV_PART_MAIN);
  lv_obj_set_style_pad_all(center_zone, 6, LV_PART_MAIN);

  // Large Progress Bar (Height 40px)
  progress_bar = lv_bar_create(center_zone);
  lv_obj_set_size(progress_bar, 172, 40);
  lv_obj_set_pos(progress_bar, 2, 8);
  lv_bar_set_range(progress_bar, 0, 100);
  lv_bar_set_value(progress_bar, 0, LV_ANIM_OFF);
  lv_obj_set_style_bg_color(progress_bar, lv_color_hex(0x333333), LV_PART_MAIN);
  lv_obj_set_style_bg_color(progress_bar, lv_color_hex(0x00A8FF), LV_PART_INDICATOR);

  // Progress percentage label (Montserrat 28)
  percent_label = lv_label_create(center_zone);
  lv_label_set_text(percent_label, "0%");
  lv_obj_set_style_text_color(percent_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
  lv_obj_set_style_font(percent_label, &lv_font_montserrat_28, LV_PART_MAIN);
  lv_obj_set_pos(percent_label, 6, 54);

  // Time remaining label (Montserrat 16)
  time_label = lv_label_create(center_zone);
  lv_label_set_text(time_label, "Время: --:--:--");
  lv_obj_set_style_text_color(time_label, lv_color_hex(0xCCCCCC), LV_PART_MAIN);
  lv_obj_set_style_font(time_label, &lv_font_montserrat_16, LV_PART_MAIN);
  lv_obj_set_pos(time_label, 6, 96);

  // Temperature status
  nozzle_label = lv_label_create(center_zone);
  lv_label_set_text(nozzle_label, "Сопло: 0° / 0°");
  lv_obj_set_style_text_color(nozzle_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
  lv_obj_set_pos(nozzle_label, 6, 140);

  bed_label = lv_label_create(center_zone);
  lv_label_set_text(bed_label, "Стол: 0° / 0°");
  lv_obj_set_style_text_color(bed_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
  lv_obj_set_pos(bed_label, 6, 180);
}

void MainScreen::build_right_zone(lv_obj_t *parent) {
  // Right Zone: Controls (Width ~90px, Height 282px)
  right_zone = lv_obj_create(parent);
  lv_obj_set_size(right_zone, 90, 282);
  lv_obj_set_pos(right_zone, 384, 0);
  lv_obj_set_style_bg_color(right_zone, lv_color_hex(0x121212), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(right_zone, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_style_border_width(right_zone, 0, LV_PART_MAIN);
  lv_obj_set_style_pad_all(right_zone, 0, LV_PART_MAIN);

  // Pause Button (Giant)
  pause_btn = lv_btn_create(right_zone);
  lv_obj_set_size(pause_btn, 86, 134);
  lv_obj_set_pos(pause_btn, 2, 0);
  lv_obj_set_style_bg_color(pause_btn, lv_color_hex(0xFFA500), LV_PART_MAIN); // Yellow
  lv_obj_set_style_shadow_width(pause_btn, 0, LV_PART_MAIN);
  lv_obj_add_event_cb(pause_btn, _handle_pause_click, LV_EVENT_CLICKED, this);

  pause_btn_label = lv_label_create(pause_btn);
  lv_label_set_text(pause_btn_label, "⏸\nПАУЗА");
  lv_obj_set_style_text_align(pause_btn_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
  lv_obj_set_style_text_color(pause_btn_label, lv_color_hex(0x000000), LV_PART_MAIN);
  lv_obj_center(pause_btn_label);

  // Stop Button (Giant)
  stop_btn = lv_btn_create(right_zone);
  lv_obj_set_size(stop_btn, 86, 134);
  lv_obj_set_pos(stop_btn, 2, 142);
  lv_obj_set_style_bg_color(stop_btn, lv_color_hex(0xFF0000), LV_PART_MAIN); // Red
  lv_obj_set_style_shadow_width(stop_btn, 0, LV_PART_MAIN);
  lv_obj_add_event_cb(stop_btn, _handle_stop_click, LV_EVENT_CLICKED, this);

  stop_btn_label = lv_label_create(stop_btn);
  lv_label_set_text(stop_btn_label, "⏹\nСТОП");
  lv_obj_set_style_text_align(stop_btn_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
  lv_obj_set_style_text_color(stop_btn_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
  lv_obj_center(stop_btn_label);

  // Start Button (Idle mode)
  start_btn = lv_btn_create(right_zone);
  lv_obj_set_size(start_btn, 86, 276);
  lv_obj_set_pos(start_btn, 2, 0);
  lv_obj_set_style_bg_color(start_btn, lv_color_hex(0x00CC66), LV_PART_MAIN); // Green
  lv_obj_set_style_shadow_width(start_btn, 0, LV_PART_MAIN);
  lv_obj_add_event_cb(start_btn, _handle_start_click, LV_EVENT_CLICKED, this);

  start_btn_label = lv_label_create(start_btn);
  lv_label_set_text(start_btn_label, "▶\nСТАРТ");
  lv_obj_set_style_text_align(start_btn_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
  lv_obj_set_style_text_color(start_btn_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
  lv_obj_center(start_btn_label);
}

void MainScreen::create_stop_confirm_modal() {
  confirm_modal = lv_obj_create(screen_obj);
  lv_obj_set_size(confirm_modal, 360, 200);
  lv_obj_center(confirm_modal);
  lv_obj_set_style_bg_color(confirm_modal, lv_color_hex(0x222222), LV_PART_MAIN);
  lv_obj_set_style_border_color(confirm_modal, lv_color_hex(0xFF0000), LV_PART_MAIN);
  lv_obj_set_style_border_width(confirm_modal, 2, LV_PART_MAIN);
  lv_obj_add_flag(confirm_modal, LV_OBJ_FLAG_HIDDEN);

  lv_obj_t *title = lv_label_create(confirm_modal);
  lv_label_set_text(title, "Подтверждение остановки");
  lv_obj_set_style_text_color(title, lv_color_hex(0xFF5555), LV_PART_MAIN);
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);

  lv_obj_t *msg = lv_label_create(confirm_modal);
  lv_label_set_text(msg, "Вы уверены, что хотите\nостановить печать?");
  lv_obj_set_style_text_align(msg, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
  lv_obj_set_style_text_color(msg, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
  lv_obj_align(msg, LV_ALIGN_CENTER, 0, -10);

  // Confirm Stop button
  lv_obj_t *yes_btn = lv_btn_create(confirm_modal);
  lv_obj_set_size(yes_btn, 130, 40);
  lv_obj_align(yes_btn, LV_ALIGN_BOTTOM_LEFT, 10, -10);
  lv_obj_set_style_bg_color(yes_btn, lv_color_hex(0xFF0000), LV_PART_MAIN);
  lv_obj_add_event_cb(yes_btn, _handle_confirm_stop_click, LV_EVENT_CLICKED, this);

  lv_obj_t *yes_lbl = lv_label_create(yes_btn);
  lv_label_set_text(yes_lbl, "Да, стоп");
  lv_obj_center(yes_lbl);

  // Cancel button
  lv_obj_t *no_btn = lv_btn_create(confirm_modal);
  lv_obj_set_size(no_btn, 130, 40);
  lv_obj_align(no_btn, LV_ALIGN_BOTTOM_RIGHT, -10, -10);
  lv_obj_set_style_bg_color(no_btn, lv_color_hex(0x555555), LV_PART_MAIN);
  lv_obj_add_event_cb(no_btn, _handle_cancel_stop_click, LV_EVENT_CLICKED, this);

  lv_obj_t *no_lbl = lv_label_create(no_btn);
  lv_label_set_text(no_lbl, "Отмена");
  lv_obj_center(no_lbl);
}

void MainScreen::set_print_progress(int percent) {
  if (percent < 0) percent = 0;
  if (percent > 100) percent = 100;
  if (current_progress != percent) {
    current_progress = percent;
    if (progress_bar) lv_bar_set_value(progress_bar, percent, LV_ANIM_OFF);
    if (percent_label) {
      char buf[32];
      snprintf(buf, sizeof(buf), "%d%%", percent);
      lv_label_set_text(percent_label, buf);
      lv_obj_invalidate(percent_label);
    }
  }
}

void MainScreen::set_time_remaining(const std::string &time_str) {
  if (time_label) {
    std::string text = "Время: " + time_str;
    lv_label_set_text(time_label, text.c_str());
    lv_obj_invalidate(time_label);
  }
}

void MainScreen::set_nozzle_temp(int current, int target) {
  nozzle_cur = current;
  nozzle_tgt = target;
  if (nozzle_label) {
    char buf[64];
    snprintf(buf, sizeof(buf), "Сопло: %d° / %d°", current, target);
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

void MainScreen::set_bed_temp(int current, int target) {
  bed_cur = current;
  bed_tgt = target;
  if (bed_label) {
    char buf[64];
    snprintf(buf, sizeof(buf), "Стол: %d° / %d°", current, target);
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

void MainScreen::set_print_state(const std::string &state) {
  print_state = state;
  if (state == "printing") {
    lv_label_set_text(state_title_label, "Ender 3 V3 KE - Печать...");
    lv_obj_clear_flag(pause_btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(stop_btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(start_btn, LV_OBJ_FLAG_HIDDEN);
    lv_label_set_text(pause_btn_label, "⏸\nПАУЗА");
  } else if (state == "paused") {
    lv_label_set_text(state_title_label, "Ender 3 V3 KE - Пауза");
    lv_obj_clear_flag(pause_btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(stop_btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(start_btn, LV_OBJ_FLAG_HIDDEN);
    lv_label_set_text(pause_btn_label, "▶\nПРОДОЛЖ");
  } else {
    lv_label_set_text(state_title_label, "Ender 3 V3 KE - Простой");
    lv_obj_add_flag(pause_btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(stop_btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(start_btn, LV_OBJ_FLAG_HIDDEN);
  }
}

void MainScreen::set_connection_status(bool connected) {
  is_connected = connected;
  if (conn_icon_label) {
    if (connected) {
      lv_label_set_text(conn_icon_label, "");
    } else {
      lv_label_set_text(conn_icon_label, "🔌 Ошибка связи");
    }
    lv_obj_invalidate(conn_icon_label);
  }
}

void MainScreen::set_camera_offline(bool offline) {
  if (offline) {
    lv_obj_clear_flag(cam_offline_cont, LV_OBJ_FLAG_HIDDEN);
  } else {
    lv_obj_add_flag(cam_offline_cont, LV_OBJ_FLAG_HIDDEN);
  }
}

void MainScreen::_handle_pause_click(lv_event_t *e) {
  MainScreen *screen = static_cast<MainScreen*>(lv_event_get_user_data(e));
  if (screen->print_state == "paused") {
    if (screen->resume_cb) screen->resume_cb();
  } else {
    if (screen->pause_cb) screen->pause_cb();
  }
}

void MainScreen::_handle_stop_click(lv_event_t *e) {
  MainScreen *screen = static_cast<MainScreen*>(lv_event_get_user_data(e));
  if (screen->confirm_modal) {
    lv_obj_clear_flag(screen->confirm_modal, LV_OBJ_FLAG_HIDDEN);
  }
}

void MainScreen::_handle_start_click(lv_event_t *e) {
  MainScreen *screen = static_cast<MainScreen*>(lv_event_get_user_data(e));
  if (screen->start_cb) screen->start_cb();
}

void MainScreen::_handle_confirm_stop_click(lv_event_t *e) {
  MainScreen *screen = static_cast<MainScreen*>(lv_event_get_user_data(e));
  if (screen->confirm_modal) {
    lv_obj_add_flag(screen->confirm_modal, LV_OBJ_FLAG_HIDDEN);
  }
  if (screen->cancel_cb) screen->cancel_cb();
}

void MainScreen::_handle_cancel_stop_click(lv_event_t *e) {
  MainScreen *screen = static_cast<MainScreen*>(lv_event_get_user_data(e));
  if (screen->confirm_modal) {
    lv_obj_add_flag(screen->confirm_modal, LV_OBJ_FLAG_HIDDEN);
  }
}
