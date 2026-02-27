/*
 * Database.h – Poem index, history, and favorites for Poetry Box 2
 *
 * SD card layout expected:
 *   /<Author Name>/poem_title.txt   (one folder per author)
 *   /Favorites/favorites.txt        (list of favorite poem paths, one per line)
 *
 * LittleFS (internal flash) layout:
 *   /index.txt   – cached author index for fast startup
 *   /history.txt – last N printed poem paths (avoids immediate repeats)
 */

#pragma once
#include <Arduino.h>
#include <SD.h>
#include "LittleFS.h"

// ── Tuneable limits ────────────────────────────────────────────────────────────
#define MAX_AUTHORS    200
#define HISTORY_SIZE    64
#define MAX_PATH_LEN   192
#define MAX_NAME_LEN    80
#define HIST_LINE_LEN  192

// ── File paths ─────────────────────────────────────────────────────────────────
#define INDEX_FILE      "/index.txt"
#define HISTORY_FILE    "/history.txt"
#define FAVORITES_DIR   "Favorites"
#define FAVORITES_FILE  "/Favorites/favorites.txt"

// ── Author index (loaded into RAM for fast access) ─────────────────────────────
struct Author {
  char     name[MAX_NAME_LEN]; // directory name on SD (no leading slash)
  uint16_t num_poems;
};

static Author   auth_array[MAX_AUTHORS];
static uint16_t num_auth    = 0;
static uint16_t total_poems = 0;

// Path of the most recently printed poem (used for add-to-favorites)
static char last_poem_path[MAX_PATH_LEN] = "";

// ══════════════════════════════════════════════════════════════════════════════
// Internal helpers
// ══════════════════════════════════════════════════════════════════════════════

// Return just the basename of a path (everything after the last '/')
static const char* _basename(const char* path) {
  const char* slash = strrchr(path, '/');
  return slash ? slash + 1 : path;
}

// Count non-hidden, non-directory files inside an SD directory
static uint16_t _count_poems_in_dir(const char* dir_path) {
  File dir = SD.open(dir_path);
  if (!dir) return 0;
  uint16_t count = 0;
  while (true) {
    File f = dir.openNextFile();
    if (!f) break;
    const char* n = _basename(f.name());
    if (!f.isDirectory() && n[0] != '.') count++;
    f.close();
  }
  dir.close();
  return count;
}

// ══════════════════════════════════════════════════════════════════════════════
// Index management
// ══════════════════════════════════════════════════════════════════════════════

// Scan SD card and build the author index; persist it to LittleFS for future
// fast loads.  Also ensures the Favorites directory and file exist.
static void build_index() {
  num_auth    = 0;
  total_poems = 0;

  // Ensure Favorites folder and file exist on SD
  if (!SD.exists("/" FAVORITES_DIR)) SD.mkdir("/" FAVORITES_DIR);
  if (!SD.exists(FAVORITES_FILE)) {
    File f = SD.open(FAVORITES_FILE, FILE_WRITE);
    if (f) f.close();
  }

  File root = SD.open("/");
  if (!root) return;

  while (num_auth < MAX_AUTHORS) {
    File entry = root.openNextFile();
    if (!entry) break;

    if (!entry.isDirectory()) { entry.close(); continue; }

    const char* raw  = entry.name();
    const char* name = _basename(raw);   // strip leading '/' if present

    // Skip hidden entries and the Favorites folder
    if (name[0] == '.' || strcmp(name, FAVORITES_DIR) == 0) {
      entry.close();
      continue;
    }

    // Build full path for counting
    char dir_path[MAX_NAME_LEN + 2];
    snprintf(dir_path, sizeof(dir_path), "/%s", name);
    entry.close();

    uint16_t cnt = _count_poems_in_dir(dir_path);
    if (cnt == 0) continue;

    strncpy(auth_array[num_auth].name, name, MAX_NAME_LEN - 1);
    auth_array[num_auth].name[MAX_NAME_LEN - 1] = '\0';
    auth_array[num_auth].num_poems = cnt;
    num_auth++;
    total_poems += cnt;
  }
  root.close();

  // ── Persist to LittleFS ──────────────────────────────────────────────────
  File idx = LittleFS.open(INDEX_FILE, FILE_WRITE);
  if (!idx) return;
  idx.printf("%u %u\n", num_auth, total_poems);
  for (int i = 0; i < num_auth; i++) {
    idx.printf("%s|%u\n", auth_array[i].name, auth_array[i].num_poems);
  }
  idx.close();
}

// Load the persisted index from LittleFS.  Returns true on success.
static bool _load_index_from_cache() {
  File idx = LittleFS.open(INDEX_FILE, FILE_READ);
  if (!idx) return false;

  char line[MAX_NAME_LEN + 16];
  int  len = idx.readBytesUntil('\n', line, (int)sizeof(line) - 1);
  line[len] = '\0';

  unsigned na, tp;
  if (sscanf(line, "%u %u", &na, &tp) != 2 || na == 0) {
    idx.close(); return false;
  }
  num_auth    = (uint16_t)na;
  total_poems = (uint16_t)tp;

  for (int i = 0; i < num_auth; i++) {
    len = idx.readBytesUntil('\n', line, (int)sizeof(line) - 1);
    line[len] = '\0';
    char* sep = strchr(line, '|');
    if (!sep) { idx.close(); num_auth = 0; total_poems = 0; return false; }
    *sep = '\0';
    strncpy(auth_array[i].name, line, MAX_NAME_LEN - 1);
    auth_array[i].name[MAX_NAME_LEN - 1] = '\0';
    auth_array[i].num_poems = (uint16_t)atoi(sep + 1);
  }
  idx.close();
  return true;
}

// Public: load index from cache (fast) or build from SD (slow, first boot)
void load_index() {
  if (!_load_index_from_cache()) build_index();
}

// Public: force a full rebuild (e.g. after adding new poems to the SD card)
void rebuild_index() {
  LittleFS.remove(INDEX_FILE);
  build_index();
}

// ══════════════════════════════════════════════════════════════════════════════
// History (last N poems, stored in LittleFS)
// ══════════════════════════════════════════════════════════════════════════════

static char history_buf[HISTORY_SIZE][HIST_LINE_LEN];
static int  history_count = 0;

static void _save_history() {
  File hist = LittleFS.open(HISTORY_FILE, FILE_WRITE);
  if (!hist) return;
  for (int i = 0; i < history_count; i++) hist.println(history_buf[i]);
  hist.close();
}

void init_history() {
  history_count = 0;
  if (!LittleFS.exists(HISTORY_FILE)) {
    File h = LittleFS.open(HISTORY_FILE, FILE_WRITE);
    if (h) h.close();
    return;
  }
  File hist = LittleFS.open(HISTORY_FILE, FILE_READ);
  if (!hist) return;
  while (hist.available() && history_count < HISTORY_SIZE) {
    int len = hist.readBytesUntil('\n', history_buf[history_count],
                                  HIST_LINE_LEN - 1);
    history_buf[history_count][len] = '\0';
    if (len > 0) history_count++;
  }
  hist.close();
}

static bool _is_in_history(const char* path) {
  for (int i = 0; i < history_count; i++)
    if (strcmp(history_buf[i], path) == 0) return true;
  return false;
}

static void _append_to_history(const char* path) {
  if (_is_in_history(path)) return;
  if (history_count >= HISTORY_SIZE) {
    // Shift out the oldest entry
    for (int i = 0; i < HISTORY_SIZE - 1; i++)
      strncpy(history_buf[i], history_buf[i + 1], HIST_LINE_LEN - 1);
    history_count = HISTORY_SIZE - 1;
  }
  strncpy(history_buf[history_count], path, HIST_LINE_LEN - 1);
  history_buf[history_count][HIST_LINE_LEN - 1] = '\0';
  history_count++;
  _save_history();
}

// ══════════════════════════════════════════════════════════════════════════════
// Poem access helpers
// ══════════════════════════════════════════════════════════════════════════════

// Fill buf with the full SD path of the poem_i-th poem (0-indexed) inside
// the author directory.  Returns true on success.
static bool _get_poem_path(int auth_i, uint16_t poem_i,
                            char* buf, int buf_size) {
  if (auth_i < 0 || auth_i >= num_auth) return false;
  char dir_path[MAX_NAME_LEN + 2];
  snprintf(dir_path, sizeof(dir_path), "/%s", auth_array[auth_i].name);
  File dir = SD.open(dir_path);
  if (!dir) return false;

  uint16_t count = 0;
  bool     found = false;
  while (true) {
    File f = dir.openNextFile();
    if (!f) break;
    const char* fn = _basename(f.name());
    if (!f.isDirectory() && fn[0] != '.') {
      if (count == poem_i) {
        snprintf(buf, buf_size, "/%s/%s", auth_array[auth_i].name, fn);
        found = true;
        f.close();
        break;
      }
      count++;
    }
    f.close();
  }
  dir.close();
  return found;
}

// Public wrappers ─────────────────────────────────────────────────────────────

int      get_num_authors()               { return num_auth; }
const char* get_author_name(int i)       { return (i >= 0 && i < num_auth) ? auth_array[i].name : ""; }
uint16_t get_author_poem_count(int i)    { return (i >= 0 && i < num_auth) ? auth_array[i].num_poems : 0; }

bool get_poem_path(int auth_i, uint16_t poem_i, char* buf, int buf_size) {
  return _get_poem_path(auth_i, poem_i, buf, buf_size);
}

// Return the human-readable title from a full path (strips directory and extension)
const char* get_poem_title(const char* path) {
  static char title_buf[MAX_PATH_LEN];
  strncpy(title_buf, _basename(path), sizeof(title_buf) - 1);
  title_buf[sizeof(title_buf) - 1] = '\0';
  // Remove .txt extension
  char* dot = strrchr(title_buf, '.');
  if (dot) *dot = '\0';
  return title_buf;
}

// Print a poem from SD to the thermal printer
void print_poem_to_printer(const char* path) {
  File poem = SD.open(path);
  if (!poem) {
    Serial.printf("Cannot open: %s\n", path);
    return;
  }
  printer.setFont('A');
  printer.boldOff();
  while (poem.available()) printer.write(poem.read());
  printer.println();
  printer.feed(3);
  poem.close();
  strncpy(last_poem_path, path, MAX_PATH_LEN - 1);
}

// ══════════════════════════════════════════════════════════════════════════════
// Random poem selection (history-aware)
// ══════════════════════════════════════════════════════════════════════════════

// Fill buf with a randomly selected poem path that has not been recently
// printed.  Returns true on success.
bool get_random_poem_path(char* buf, int buf_size) {
  if (total_poems == 0) return false;

  // Try up to 20 times to find a poem not in history
  for (int attempt = 0; attempt < 20; attempt++) {
    uint16_t idx = (uint16_t)random(total_poems);

    // Map global index → author + poem-within-author
    uint16_t cumulative = 0;
    int      auth_i     = -1;
    uint16_t poem_i     = 0;
    for (int i = 0; i < num_auth; i++) {
      if (idx < cumulative + auth_array[i].num_poems) {
        auth_i = i;
        poem_i = idx - cumulative;
        break;
      }
      cumulative += auth_array[i].num_poems;
    }
    if (auth_i < 0) continue;

    char path[MAX_PATH_LEN];
    if (!_get_poem_path(auth_i, poem_i, path, sizeof(path))) continue;

    if (!_is_in_history(path)) {
      strncpy(buf, path, buf_size - 1);
      buf[buf_size - 1] = '\0';
      _append_to_history(path);
      strncpy(last_poem_path, path, MAX_PATH_LEN - 1);
      return true;
    }
  }

  // Fallback: just pick anything (e.g. history has grown very large)
  uint16_t idx        = (uint16_t)random(total_poems);
  uint16_t cumulative = 0;
  int      auth_i     = 0;
  uint16_t poem_i     = 0;
  for (int i = 0; i < num_auth; i++) {
    if (idx < cumulative + auth_array[i].num_poems) {
      auth_i = i;
      poem_i = idx - cumulative;
      break;
    }
    cumulative += auth_array[i].num_poems;
  }
  char path[MAX_PATH_LEN];
  if (_get_poem_path(auth_i, poem_i, path, sizeof(path))) {
    strncpy(buf, path, buf_size - 1);
    buf[buf_size - 1] = '\0';
    _append_to_history(path);
    strncpy(last_poem_path, path, MAX_PATH_LEN - 1);
    return true;
  }
  return false;
}

// ══════════════════════════════════════════════════════════════════════════════
// Favorites (stored in /Favorites/favorites.txt on the SD card)
// ══════════════════════════════════════════════════════════════════════════════

uint16_t count_favorites() {
  File fav = SD.open(FAVORITES_FILE);
  if (!fav) return 0;
  uint16_t count = 0;
  while (fav.available()) if (fav.read() == '\n') count++;
  fav.close();
  return count;
}

bool is_favorite(const char* path) {
  File fav = SD.open(FAVORITES_FILE);
  if (!fav) return false;
  char line[MAX_PATH_LEN];
  while (fav.available()) {
    int len = fav.readBytesUntil('\n', line, (int)sizeof(line) - 1);
    line[len] = '\0';
    if (strcmp(line, path) == 0) { fav.close(); return true; }
  }
  fav.close();
  return false;
}

// Add path to favorites.  Returns false if already favorited.
bool add_to_favorites(const char* path) {
  if (is_favorite(path)) return false;
  File fav = SD.open(FAVORITES_FILE, FILE_APPEND);
  if (!fav) return false;
  fav.println(path);
  fav.close();
  return true;
}

// Fill buf with the n-th favorite path (0-indexed).  Returns true on success.
bool get_favorite_path(uint16_t n, char* buf, int buf_size) {
  File fav = SD.open(FAVORITES_FILE);
  if (!fav) return false;
  uint16_t count = 0;
  while (fav.available()) {
    int len = fav.readBytesUntil('\n', buf, buf_size - 1);
    buf[len] = '\0';
    if (len > 0) {
      if (count == n) { fav.close(); return true; }
      count++;
    }
  }
  fav.close();
  return false;
}

// Pick a random favorite path
bool get_random_favorite_path(char* buf, int buf_size) {
  uint16_t cnt = count_favorites();
  if (cnt == 0) return false;
  return get_favorite_path((uint16_t)random(cnt), buf, buf_size);
}
