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
      finetune_panel(ws, lv_lock),
      zoffset_panel(ws, lv_lock),
      print_status_panel(ws, lv_lock, NULL),
      print_panel(ws, lv_lock, print_status_panel, sm),
      extruder_panel(ws, lv_lock, numpad, sm),
      prompt_panel(ws, lv_lock, lv_layer_top()),
      spoolman_panel(sm),
      tabview(NULL),
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
      finetune_btn(NULL),
      right_zone(NULL),
      pause_btn(NULL),
      stop_btn(NULL),
      start_btn(NULL),
      confirm_modal(NULL) {
  ws.register_notify_update(this);
  create_panel();
}

MainPanel::~MainPanel() {
  ws.unregister_notify_update(this);
}

void MainPanel::init(json &data) {
  if (printertune_panel) {
    printertune_panel->init(data);
  }
}

void MainPanel::subscribe() {
  ws.register_notify_update(this);
}

PrinterTunePanel& MainPanel::get_tune_panel() {
  return *printertune_panel;
}

void MainPanel::enable_spoolman() {
  if (setting_panel) setting_panel->enable_spoolman();
}

void MainPanel::show_console() {
  if (tabview) lv_tabview_set_act(tabview, 2, LV_ANIM_ON);
}

void MainPanel::create_panel() {
  if (tabview != NULL) return; // Guard against double creation

  tabview = lv_tabview_create(lv_scr_act(), LV_DIR_TOP, 30);
  lv_obj_set_size(tabview, 480, 272);

  main_tab = lv_tabview_add_tab(tabview, "Главная");
  macros_tab = lv_tabview_add_tab(tabview, "Макросы");
  console_tab = lv_tabview_add_tab(tabview, "Консоль");
  printertune_tab = lv_tabview_add_tab(tabview, "Калибровка");
  setting_tab = lv_tabview_add_tab(tabview, "Настройки");

  create_main(main_tab);

  macros_panel = std::make_unique<MacrosPanel>(ws, lv_lock, macros_tab, [this]() { show_console(); });
  console_panel = std::make_unique<ConsolePanel>(ws, lv_lock, console_tab);
  printertune_panel = std::make_unique<PrinterTunePanel>(ws, lv_lock, printertune_tab, finetune_panel, zoffset_panel);
  setting_panel = std::make_unique<SettingPanel>(ws, lv_lock, setting_tab, spoolman_panel);
}

void MainPanel::create_main(lv_obj_t *parent) {
  main_cont = lv_obj_create(parent);
  lv_obj_set_size(main_cont, 480, 242);
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
  // Left Zone (35% = ~165px, Height 238px)
  left_zone = lv_obj_create(parent);
  lv_obj_set_size(left_zone, 165, 238);
  lv_obj_set_pos(left_zone, 0, 0);
  lv_obj_set_style_bg_color(left_zone, lv_color_hex(0x1A1A1A), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(left_zone, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_style_border_color(left_zone, lv_color_hex(0x333333), LV_PART_MAIN);
  lv_obj_set_style_border_width(left_zone, 1, LV_PART_MAIN);
  lv_obj_set_style_pad_all(left_zone, 4, LV_PART_MAIN);

  // Nozzle Temperature
  lv_obj_t *nozzle_title = lv_label_create(left_zone);
  lv_label_set_text(nozzle_title, "🔥 Сопло");
  lv_obj_set_style_text_color(nozzle_title, lv_color_hex(0xAAAAAA), LV_PART_MAIN);
  lv_obj_set_pos(nozzle_title, 4, 4);

  nozzle_label = lv_label_create(left_zone);
  lv_label_set_text(nozzle_label, "0° / 0°");
  lv_obj_set_style_text_color(nozzle_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
  lv_obj_set_style_font(nozzle_label, &lv_font_montserrat_18, LV_PART_MAIN);
  lv_obj_set_pos(nozzle_label, 4, 24);

  // Bed Temperature
  lv_obj_t *bed_title = lv_label_create(left_zone);
  lv_label_set_text(bed_title, "🛏 Стол");
  lv_obj_set_style_text_color(bed_title, lv_color_hex(0xAAAAAA), LV_PART_MAIN);
  lv_obj_set_pos(bed_title, 4, 76);

  bed_label = lv_label_create(left_zone);
  lv_label_set_text(bed_label, "0° / 0°");
  lv_obj_set_style_text_color(bed_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
  lv_obj_set_style_font(bed_label, &lv_font_montserrat_18, LV_PART_MAIN);
  lv_obj_set_pos(bed_label, 4, 96);

  // Part Fan Speed
  lv_obj_t *fan_title = lv_label_create(left_zone);
  lv_label_set_text(fan_title, "❄ Обдув");
  lv_obj_set_style_text_color(fan_title, lv_color_hex(0xAAAAAA), LV_PART_MAIN);
  lv_obj_set_pos(fan_title, 4, 148);

  fan_label = lv_label_create(left_zone);
  lv_label_set_text(fan_label, "0%");
  lv_obj_set_style_text_color(fan_label, lv_color_hex(0x00A8FF), LV_PART_MAIN);
  lv_obj_set_style_font(fan_label, &lv_font_montserrat_18, LV_PART_MAIN);
  lv_obj_set_pos(fan_label, 4, 168);
}

void MainPanel::build_center_zone(lv_obj_t *parent) {
  // Center Zone (45% = ~212px, Height 238px)
  center_zone = lv_obj_create(parent);
  lv_obj_set_size(center_zone, 212, 238);
  lv_obj_set_pos(center_zone, 170, 0);
  lv_obj_set_style_bg_color(center_zone, lv_color_hex(0x181818), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(center_zone, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_style_border_color(center_zone, lv_color_hex(0x333333), LV_PART_MAIN);
  lv_obj_set_style_border_width(center_zone, 1, LV_PART_MAIN);
  lv_obj_set_style_pad_all(center_zone, 4, LV_PART_MAIN);

  // Remaining Time
  lv_obj_t *time_title = lv_label_create(center_zone);
  lv_label_set_text(time_title, "Осталось времени");
  lv_obj_set_style_text_color(time_title, lv_color_hex(0x888888), LV_PART_MAIN);
  lv_obj_set_pos(time_title, 4, 4);

  time_rem_label = lv_label_create(center_zone);
  lv_label_set_text(time_rem_label, "00:00:00");
  lv_obj_set_style_text_color(time_rem_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
  lv_obj_set_style_font(time_rem_label, &lv_font_montserrat_20, LV_PART_MAIN);
  lv_obj_set_pos(time_rem_label, 4, 22);

  // Layer status
  layer_label = lv_label_create(center_zone);
  lv_label_set_text(layer_label, "Слой: 0 из 0");
  lv_obj_set_style_text_color(layer_label, lv_color_hex(0xCCCCCC), LV_PART_MAIN);
  lv_obj_set_pos(layer_label, 4, 56);

  // Thick Progress Bar
  progress_bar = lv_bar_create(center_zone);
  lv_obj_set_size(progress_bar, 196, 28);
  lv_obj_set_pos(progress_bar, 4, 80);
  lv_bar_set_range(progress_bar, 0, 100);
  lv_bar_set_value(progress_bar, 0, LV_ANIM_OFF);
  lv_obj_set_style_bg_color(progress_bar, lv_color_hex(0x333333), LV_PART_MAIN);
  lv_obj_set_style_bg_color(progress_bar, lv_color_hex(0x00A8FF), LV_PART_INDICATOR);

  progress_percent_label = lv_label_create(progress_bar);
  lv_label_set_text(progress_percent_label, "0%");
  lv_obj_set_style_text_color(progress_percent_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
  lv_obj_center(progress_percent_label);

  // Bed Mesh 3D heatmap button
  bedmesh_btn = lv_btn_create(center_zone);
  lv_obj_set_size(bedmesh_btn, 196, 36);
  lv_obj_set_pos(bedmesh_btn, 4, 122);
  lv_obj_set_style_bg_color(bedmesh_btn, lv_color_hex(0x2B2B2B), LV_PART_MAIN);
  lv_obj_set_style_shadow_width(bedmesh_btn, 0, LV_PART_MAIN);
  lv_obj_add_event_cb(bedmesh_btn, _handle_bedmesh_click, LV_EVENT_CLICKED, this);

  lv_obj_t *bm_lbl = lv_label_create(bedmesh_btn);
  lv_label_set_text(bm_lbl, "📊 Сетка стола");
  lv_obj_set_style_text_color(bm_lbl, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
  lv_obj_center(bm_lbl);

  // FineTune mid-print button
  finetune_btn = lv_btn_create(center_zone);
  lv_obj_set_size(finetune_btn, 196, 36);
  lv_obj_set_pos(finetune_btn, 4, 168);
  lv_obj_set_style_bg_color(finetune_btn, lv_color_hex(0x2B2B2B), LV_PART_MAIN);
  lv_obj_set_style_shadow_width(finetune_btn, 0, LV_PART_MAIN);
  lv_obj_add_event_cb(finetune_btn, _handle_finetune_click, LV_EVENT_CLICKED, this);

  lv_obj_t *ft_lbl = lv_label_create(finetune_btn);
  lv_label_set_text(ft_lbl, "⚙️ Настройка");
  lv_obj_set_style_text_color(ft_lbl, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
  lv_obj_center(ft_lbl);
}

void MainPanel::build_right_zone(lv_obj_t *parent) {
  // Right Zone (20% = ~92px, Height 238px)
  right_zone = lv_obj_create(parent);
  lv_obj_set_size(right_zone, 92, 238);
  lv_obj_set_pos(right_zone, 386, 0);
  lv_obj_set_style_bg_color(right_zone, lv_color_hex(0x121212), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(right_zone, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_style_border_width(right_zone, 0, LV_PART_MAIN);
  lv_obj_set_style_pad_all(right_zone, 0, LV_PART_MAIN);

  // Giant Print Button (Idle mode)
  start_btn = lv_btn_create(right_zone);
  lv_obj_set_size(start_btn, 88, 234);
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
  lv_obj_set_size(pause_btn, 88, 114);
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
  lv_obj_set_size(stop_btn, 88, 114);
  lv_obj_set_pos(stop_btn, 2, 122);
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
  lv_obj_set_size(confirm_modal, 360, 200);
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
  lv_obj_set_size(yes_btn, 130, 40);
  lv_obj_align(yes_btn, LV_ALIGN_BOTTOM_LEFT, 10, -10);
  lv_obj_set_style_bg_color(yes_btn, lv_color_hex(0xFF0000), LV_PART_MAIN);
  lv_obj_add_event_cb(yes_btn, _handle_confirm_cancel, LV_EVENT_CLICKED, this);

  lv_obj_t *yes_lbl = lv_label_create(yes_btn);
  lv_label_set_text(yes_lbl, "Да, отменить");
  lv_obj_center(yes_lbl);

  lv_obj_t *no_btn = lv_btn_create(confirm_modal);
  lv_obj_set_size(no_btn, 130, 40);
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
      lv_obj_set_style_text_color(nozzle_label, lv_color_hex(0x3399FF), LV_PART_MAIN);
    } else if (target > 0 && current >= target - 2) {
      lv_obj_set_style_text_color(nozzle_label, lv_color_hex(0x00CC66), LV_PART_MAIN);
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
      lv_obj_set_style_text_color(bed_label, lv_color_hex(0x3399FF), LV_PART_MAIN);
    } else if (target > 0 && current >= target - 2) {
      lv_obj_set_style_text_color(bed_label, lv_color_hex(0x00CC66), LV_PART_MAIN);
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

void MainPanel::consume(json &data) {
  State *s = State::get_instance();
  auto ps = s->get_data("/printer_state"_json_pointer);
  json st = ps;

  if (ps.contains("params") && ps["params"].is_array() && !ps["params"].empty()) {
    st = ps["params"][0];
  } else if (data.contains("params") && data["params"].is_array() && !data["params"].empty()) {
    st = data["params"][0];
  } else if (data.contains("result") && data["result"].contains("status")) {
    st = data["result"]["status"];
  }

  if (st.contains("extruder")) {
    auto &ext = st["extruder"];
    int cur = ext.contains("temperature") && ext["temperature"].is_number() ? ext["temperature"].get<int>() : cur_nozzle;
    int tgt = ext.contains("target") && ext["target"].is_number() ? ext["target"].get<int>() : tgt_nozzle;
    set_nozzle_temp(cur, tgt);
  }
  if (st.contains("heater_bed")) {
    auto &bed = st["heater_bed"];
    int cur = bed.contains("temperature") && bed["temperature"].is_number() ? bed["temperature"].get<int>() : cur_bed;
    int tgt = bed.contains("target") && bed["target"].is_number() ? bed["target"].get<int>() : tgt_bed;
    set_bed_temp(cur, tgt);
  }
  if (st.contains("fan0")) {
    auto &fan = st["fan0"];
    int speed = fan.contains("speed") && fan["speed"].is_number() ? static_cast<int>(fan["speed"].get<double>() * 100.0) : cur_fan;
    set_fan_speed(speed);
  }
  if (st.contains("display_status") && st["display_status"].contains("progress")) {
    int prog = static_cast<int>(st["display_status"]["progress"].get<double>() * 100.0);
    set_print_progress(prog);
  }
  if (st.contains("print_stats")) {
    auto &ps_obj = st["print_stats"];
    if (ps_obj.contains("state")) {
      set_print_state(ps_obj["state"].get<std::string>());
    }
    if (ps_obj.contains("print_duration")) {
      double print_dur = ps_obj["print_duration"].get<double>();
      double total_dur = ps_obj.contains("total_duration") ? ps_obj["total_duration"].get<double>() : 0.0;
      if (cur_progress > 0 && cur_progress < 100 && print_dur > 0) {
        double total_est = (total_dur > print_dur) ? total_dur : (print_dur / (cur_progress / 100.0));
        double rem = total_est - print_dur;
        if (rem < 0) rem = 0;
        int rem_sec = static_cast<int>(rem);
        int h = rem_sec / 3600;
        int m = (rem_sec % 3600) / 60;
        int s = rem_sec % 60;
        char buf[32];
        snprintf(buf, sizeof(buf), "%02d:%02d:%02d", h, m, s);
        set_time_remaining(buf);
      }
    }
    if (ps_obj.contains("info")) {
      auto &info = ps_obj["info"];
      int cur_l = info.contains("current_layer") && info["current_layer"].is_number() ? info["current_layer"].get<int>() : 0;
      int tot_l = info.contains("total_layer") && info["total_layer"].is_number() ? info["total_layer"].get<int>() : 0;
      set_layer_info(cur_l, tot_l);
    }
  }
}

void MainPanel::_handle_pause_click(lv_event_t *e) {
  MainPanel *panel = static_cast<MainPanel*>(lv_event_get_user_data(e));
  panel->ws.gcode_script("M300 S1000 P50");
  if (panel->current_state == "paused") {
    panel->ws.gcode_script("_SMART_RESUME");
  } else {
    panel->ws.gcode_script("_SMART_PAUSE");
  }
}

void MainPanel::_handle_stop_click(lv_event_t *e) {
  MainPanel *panel = static_cast<MainPanel*>(lv_event_get_user_data(e));
  panel->ws.gcode_script("M300 S1000 P50");
  if (panel->confirm_modal) {
    lv_obj_clear_flag(panel->confirm_modal, LV_OBJ_FLAG_HIDDEN);
  }
}

void MainPanel::_handle_start_click(lv_event_t *e) {
  MainPanel *panel = static_cast<MainPanel*>(lv_event_get_user_data(e));
  panel->print_panel.foreground();
}

void MainPanel::_handle_bedmesh_click(lv_event_t *e) {
  MainPanel *panel = static_cast<MainPanel*>(lv_event_get_user_data(e));
  lv_tabview_set_act(panel->tabview, 3, LV_ANIM_ON);
}

void MainPanel::_handle_finetune_click(lv_event_t *e) {
  MainPanel *panel = static_cast<MainPanel*>(lv_event_get_user_data(e));
  panel->finetune_panel.foreground();
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
