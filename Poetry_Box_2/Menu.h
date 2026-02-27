/*
 * Menu.h – Button handling and LCD UI for Poetry Box 2
 *
 * Controls (all buttons are INPUT_PULLUP, active-LOW):
 *   GREEN  (GPIO 32) – Primary action: print random / select item / confirm
 *   YELLOW (GPIO 33) – Secondary action / scroll up / open favorites menu
 *   BLUE   (GPIO 25) – Tertiary action  / scroll down / browse by author
 *   RED    (GPIO 26) – Back / home screen
 *                      Hold RED (3 s) → rebuild poem index from SD
 *
 * Screens:
 *   Home          GREEN=print random  YELLOW=favorites  BLUE=by author  RED=help
 *   Help          RED=home
 *   Favorites     GREEN=random fav  YELLOW=browse favs  BLUE=fav last poem  RED=home
 *   Browse Authors  YELLOW/BLUE=scroll  GREEN=select author  RED=home
 *   Browse Poems    YELLOW/BLUE=scroll  GREEN=print poem  RED=back to authors
 *   Browse Favs     YELLOW/BLUE=scroll  GREEN=print fav    RED=back to fav menu
 *   Message         (auto-returns to home after a timeout)
 */

#pragma once
#include <Arduino.h>

// ── Button pin assignments ────────────────────────────────────────────────────
#define BTN_GREEN  32
#define BTN_YELLOW 33
#define BTN_BLUE   25
#define BTN_RED    26

// ── LCD geometry ─────────────────────────────────────────────────────────────
#define LCD_COLS   20
#define LCD_ROWS    4
#define LIST_ROWS   3   // rows below the title row available for list items

// ── Menu state machine ───────────────────────────────────────────────────────
enum MenuState {
  STATE_HOME = 0,
  STATE_HELP,
  STATE_FAVORITES,
  STATE_BROWSE_AUTHORS,
  STATE_BROWSE_POEMS,
  STATE_BROWSE_FAVS,
  STATE_MSG
};

static MenuState      current_state   = STATE_HOME;
static int            selected_author = 0;   // author index while browsing poems
static int            scroll_pos      = 0;   // index of the topmost visible item
static int            cursor_pos      = 0;   // index of the highlighted item
static unsigned long  msg_until       = 0;   // millis() expiry for message screen
static bool           last_btn[4]     = {false, false, false, false};
static unsigned long  red_press_start = 0;

static const uint8_t btn_pins[4] = {BTN_GREEN, BTN_YELLOW, BTN_BLUE, BTN_RED};

#define RED_HOLD_MS 3000   // hold RED for 3 s to rebuild index

// ── LCD helpers ──────────────────────────────────────────────────────────────

static void _lcd_clear_row(int row) {
  lcd.setCursor(0, row);
  lcd.print("                    ");  // 20 spaces
}

// Print text to a specific row, padding/truncating to LCD_COLS
static void _lcd_row(int row, const char* text) {
  _lcd_clear_row(row);
  lcd.setCursor(0, row);
  char buf[LCD_COLS + 1];
  strncpy(buf, text, LCD_COLS);
  buf[LCD_COLS] = '\0';
  lcd.print(buf);
}

// ── Button initialisation ────────────────────────────────────────────────────

void init_buttons() {
  for (int i = 0; i < 4; i++) {
    pinMode(btn_pins[i], INPUT_PULLUP);
    last_btn[i] = (digitalRead(btn_pins[i]) == LOW);
  }
}

// ══════════════════════════════════════════════════════════════════════════════
// Screen-rendering functions
// ══════════════════════════════════════════════════════════════════════════════

void show_home() {
  current_state = STATE_HOME;
  lcd.clear();
  _lcd_row(0, "GREEN:PRINT RANDOM");
  _lcd_row(1, "YELLOW:FAVORITES");
  _lcd_row(2, "BLUE:BY AUTHOR");
  _lcd_row(3, "RED:HELP");
}

static void show_help() {
  current_state = STATE_HELP;
  lcd.clear();
  _lcd_row(0, "G:HOME  Y/B:SCROLL");
  _lcd_row(1, "G:SELECT  R:BACK");
  _lcd_row(2, "HOLD RED:REBUILD");
  _lcd_row(3, "INDEX  RED:HOME");
}

static void show_favorites_menu() {
  current_state = STATE_FAVORITES;
  lcd.clear();
  _lcd_row(0, "GREEN:RANDOM FAV");
  _lcd_row(1, "YELLOW:BROWSE FAVS");
  _lcd_row(2, "BLUE:FAV LAST POEM");
  _lcd_row(3, "RED:HOME");
}

// Render a scrollable list.  title is row 0; items occupy rows 1-3.
// get_item(index, buf, buf_size) fills buf with the display text for that item.
static void _show_list(const char* title, int total_items,
                        void (*get_item)(int, char*, int)) {
  lcd.clear();
  _lcd_row(0, title);
  for (int row = 0; row < LIST_ROWS; row++) {
    int idx = scroll_pos + row;
    if (idx >= total_items) { _lcd_clear_row(row + 1); continue; }
    char item[LCD_COLS + 1];
    get_item(idx, item, (int)sizeof(item));
    char line[LCD_COLS + 1];
    // Use '>' to mark the highlighted item
    snprintf(line, sizeof(line), "%c%.19s", (idx == cursor_pos) ? '>' : ' ', item);
    _lcd_row(row + 1, line);
  }
}

// ── List item providers ───────────────────────────────────────────────────────

static void _author_item(int idx, char* buf, int sz) {
  strncpy(buf, get_author_name(idx), sz - 1);
  buf[sz - 1] = '\0';
}

static int _poem_auth_idx = 0;  // which author we are currently browsing

static void _poem_item(int idx, char* buf, int sz) {
  char path[MAX_PATH_LEN];
  if (get_poem_path(_poem_auth_idx, (uint16_t)idx, path, sizeof(path))) {
    strncpy(buf, get_poem_title(path), sz - 1);
    buf[sz - 1] = '\0';
  } else {
    strncpy(buf, "---", sz - 1);
    buf[sz - 1] = '\0';
  }
}

static uint16_t _fav_total = 0;

static void _fav_item(int idx, char* buf, int sz) {
  char path[MAX_PATH_LEN];
  if (get_favorite_path((uint16_t)idx, path, sizeof(path))) {
    strncpy(buf, get_poem_title(path), sz - 1);
    buf[sz - 1] = '\0';
  } else {
    strncpy(buf, "---", sz - 1);
    buf[sz - 1] = '\0';
  }
}

// ── Public list-entry points ──────────────────────────────────────────────────

static void show_author_list() {
  current_state = STATE_BROWSE_AUTHORS;
  scroll_pos    = 0;
  cursor_pos    = 0;
  _show_list("BROWSE AUTHORS:", get_num_authors(), _author_item);
}

static void show_poem_list(int auth_i) {
  current_state   = STATE_BROWSE_POEMS;
  _poem_auth_idx  = auth_i;
  selected_author = auth_i;
  scroll_pos      = 0;
  cursor_pos      = 0;
  char title[LCD_COLS + 1];
  snprintf(title, sizeof(title), "%.20s", get_author_name(auth_i));
  _show_list(title, (int)get_author_poem_count(auth_i), _poem_item);
}

static void show_fav_list() {
  current_state = STATE_BROWSE_FAVS;
  scroll_pos    = 0;
  cursor_pos    = 0;
  _fav_total    = count_favorites();
  _show_list("FAVORITES:", (int)_fav_total, _fav_item);
}

// Timed message; returns to home after duration_ms (0 = stays until button)
static void show_message(const char* line1, const char* line2,
                          unsigned long duration_ms) {
  current_state = STATE_MSG;
  lcd.clear();
  _lcd_row(0, line1);
  _lcd_row(1, line2);
  _lcd_row(3, "GREEN:HOME");
  msg_until = duration_ms ? (millis() + duration_ms) : 0;
}

// ══════════════════════════════════════════════════════════════════════════════
// Printing helper
// ══════════════════════════════════════════════════════════════════════════════

static void _do_print(const char* path) {
  char short_title[LCD_COLS + 1];
  strncpy(short_title, get_poem_title(path), LCD_COLS);
  short_title[LCD_COLS] = '\0';
  show_message("PRINTING...", short_title, 0);
  print_poem_to_printer(path);
  show_message("PRINTED!", short_title, 4000);
}

// ══════════════════════════════════════════════════════════════════════════════
// Scroll helpers
// ══════════════════════════════════════════════════════════════════════════════

static void _scroll(int delta, int total, void (*get_item)(int, char*, int),
                    const char* title) {
  cursor_pos += delta;
  if (cursor_pos < 0)            cursor_pos = 0;
  if (cursor_pos >= total)       cursor_pos = total - 1;
  if (cursor_pos < scroll_pos)   scroll_pos = cursor_pos;
  if (cursor_pos >= scroll_pos + LIST_ROWS) scroll_pos = cursor_pos - LIST_ROWS + 1;
  _show_list(title, total, get_item);
}

// ══════════════════════════════════════════════════════════════════════════════
// Button event handlers
// ══════════════════════════════════════════════════════════════════════════════

static void _on_green() {
  char path[MAX_PATH_LEN];
  switch (current_state) {
    case STATE_HOME:
      if (get_random_poem_path(path, sizeof(path))) _do_print(path);
      else show_message("NO POEMS FOUND!", "Check SD card.", 3000);
      break;

    case STATE_HELP:
      show_home();
      break;

    case STATE_FAVORITES:
      if (get_random_favorite_path(path, sizeof(path))) _do_print(path);
      else show_message("NO FAVORITES YET!", "Print & fav a poem.", 3000);
      break;

    case STATE_BROWSE_AUTHORS:
      show_poem_list(cursor_pos);
      break;

    case STATE_BROWSE_POEMS:
      if (get_poem_path(selected_author, (uint16_t)cursor_pos, path, sizeof(path))) {
        _append_to_history(path);
        strncpy(last_poem_path, path, MAX_PATH_LEN - 1);
        _do_print(path);
      }
      break;

    case STATE_BROWSE_FAVS:
      if (get_favorite_path((uint16_t)cursor_pos, path, sizeof(path))) {
        _append_to_history(path);
        strncpy(last_poem_path, path, MAX_PATH_LEN - 1);
        _do_print(path);
      }
      break;

    case STATE_MSG:
      show_home();
      break;

    default:
      show_home();
      break;
  }
}

static void _on_yellow() {
  char title[LCD_COLS + 1];
  switch (current_state) {
    case STATE_HOME:
      show_favorites_menu();
      break;
    case STATE_FAVORITES:
      show_fav_list();
      break;
    case STATE_BROWSE_AUTHORS:
      _scroll(-1, get_num_authors(), _author_item, "BROWSE AUTHORS:");
      break;
    case STATE_BROWSE_POEMS:
      snprintf(title, sizeof(title), "%.20s", get_author_name(selected_author));
      _scroll(-1, (int)get_author_poem_count(selected_author), _poem_item, title);
      break;
    case STATE_BROWSE_FAVS:
      _scroll(-1, (int)_fav_total, _fav_item, "FAVORITES:");
      break;
    default:
      break;
  }
}

static void _on_blue() {
  switch (current_state) {
    case STATE_HOME:
      show_author_list();
      break;

    case STATE_FAVORITES:
      // Favorite the last-printed poem
      if (last_poem_path[0] == '\0') {
        show_message("PRINT A POEM FIRST", "", 2000);
      } else if (add_to_favorites(last_poem_path)) {
        show_message("ADDED TO FAVS!", get_poem_title(last_poem_path), 2000);
      } else {
        show_message("ALREADY A FAVORITE", "", 2000);
      }
      break;

    case STATE_BROWSE_AUTHORS:
      _scroll(1, get_num_authors(), _author_item, "BROWSE AUTHORS:");
      break;
    case STATE_BROWSE_POEMS: {
      char title[LCD_COLS + 1];
      snprintf(title, sizeof(title), "%.20s", get_author_name(selected_author));
      _scroll(1, (int)get_author_poem_count(selected_author), _poem_item, title);
      break;
    }
    case STATE_BROWSE_FAVS:
      _scroll(1, (int)_fav_total, _fav_item, "FAVORITES:");
      break;

    default:
      break;
  }
}

// RED press – start hold timer
static void _on_red_press() {
  red_press_start = millis();
}

// RED release – short press = back/home; long hold = rebuild index
static void _on_red_release() {
  unsigned long held = millis() - red_press_start;
  if (held >= RED_HOLD_MS) {
    lcd.clear();
    _lcd_row(0, "REBUILDING INDEX");
    _lcd_row(1, "Please wait...");
    rebuild_index();
    show_message("INDEX REBUILT!", "", 2000);
    return;
  }
  // Short press: navigate back
  switch (current_state) {
    case STATE_BROWSE_POEMS:
      // Return to author list, restoring position
      current_state = STATE_BROWSE_AUTHORS;
      scroll_pos    = selected_author >= LIST_ROWS ? selected_author - LIST_ROWS + 1 : 0;
      cursor_pos    = selected_author;
      _show_list("BROWSE AUTHORS:", get_num_authors(), _author_item);
      break;
    case STATE_BROWSE_FAVS:
      show_favorites_menu();
      break;
    case STATE_FAVORITES:
    case STATE_BROWSE_AUTHORS:
    case STATE_HELP:
    case STATE_MSG:
    default:
      show_home();
      break;
  }
}

// ══════════════════════════════════════════════════════════════════════════════
// Main input handler – call every loop() iteration
// ══════════════════════════════════════════════════════════════════════════════

void handle_input() {
  // Auto-dismiss timed messages
  if (current_state == STATE_MSG && msg_until > 0 && millis() > msg_until) {
    show_home();
    return;
  }

  bool btn_now[4];
  for (int i = 0; i < 4; i++)
    btn_now[i] = (digitalRead(btn_pins[i]) == LOW); // active-LOW

  for (int i = 0; i < 4; i++) {
    if (btn_now[i] && !last_btn[i]) {
      // Falling edge (button pressed)
      switch (i) {
        case 0: _on_green();     break;
        case 1: _on_yellow();    break;
        case 2: _on_blue();      break;
        case 3: _on_red_press(); break;
      }
    }
    if (!btn_now[i] && last_btn[i] && i == 3) {
      // Rising edge on RED only (to detect hold time)
      _on_red_release();
    }
    last_btn[i] = btn_now[i];
  }

  delay(15); // ~15 ms debounce / polling interval
}
