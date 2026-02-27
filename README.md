# Poetry Box 2

An ESP32-based embedded device that prints random poems from a large public-domain library onto thermal paper.  
Load the SD card, power it on, press **GREEN**, and a poem prints.

---

## Hardware

| Component | Notes |
|-----------|-------|
| **ESP32** (38-pin DevKit) | Main microcontroller |
| **20×4 I2C LCD** | Adafruit MCP23008 backpack, address 0x20 |
| **Adafruit Thermal Printer** | CSN-A2 / TTL serial, 9600 baud |
| **MicroSD card module** | SPI interface |
| **4 momentary push-buttons** | Green, Yellow, Blue, Red |

### Wiring

#### I²C LCD
| LCD pin | ESP32 pin |
|---------|-----------|
| SDA | GPIO 21 |
| SCL | GPIO 22 |
| VCC | 5 V |
| GND | GND |

#### SD Card (SPI)
| SD pin | ESP32 pin |
|--------|-----------|
| SCK | GPIO 8 |
| MISO | GPIO 9 |
| MOSI | GPIO 10 |
| CS | GPIO 18 |
| VCC | 3.3 V |
| GND | GND |

#### Thermal Printer (UART)
| Printer pin | ESP32 pin |
|-------------|-----------|
| TX (printer) | GPIO 16 (Serial1 RX) |
| RX (printer) | GPIO 17 (Serial1 TX) |
| VH (power) | 5–9 V (see printer spec) |
| GND | GND |

#### Buttons (active-LOW, INPUT_PULLUP)
| Button | ESP32 pin |
|--------|-----------|
| GREEN | GPIO 32 |
| YELLOW | GPIO 33 |
| BLUE | GPIO 25 |
| RED | GPIO 26 |

Connect one side of each button to the GPIO pin and the other side to GND.

---

## SD Card Setup

Format the SD card as **FAT32**.  
Organise poem files like this:

```
/
├── Emily Dickinson/
│   ├── Hope is the thing with feathers.txt
│   └── Because I could not stop for Death.txt
├── Walt Whitman/
│   └── O Captain My Captain.txt
├── Edgar Allan Poe/
│   └── The Raven.txt
├── Favorites/           ← created automatically on first boot
│   └── favorites.txt    ← created automatically
└── ...
```

**Poem file format** (plain text `.txt`):
```
Poem Title
Author Name

Line 1
Line 2
...
```

> The `public_poems/` directory inside `Poetry_Box_2/` contains a ready-to-use library of  
> ~120 public-domain poets.  Copy the author folders to the root of the SD card.

### Special folder: `Favorites/`

The firmware automatically creates `Favorites/` and `Favorites/favorites.txt` on the SD card  
on first boot.  Do **not** delete this folder while the device is running.

---

## Arduino Libraries Required

Install these via **Arduino IDE → Tools → Manage Libraries** (or `arduino-cli lib install`):

| Library | Version tested |
|---------|---------------|
| `Adafruit LiquidCrystal` | ≥ 1.1.0 |
| `Adafruit Thermal Printer Library` | ≥ 1.4.0 |
| `SD` (ESP32 built-in) | ESP32 core ≥ 2.0 |
| `LittleFS` (ESP32 built-in) | ESP32 core ≥ 2.0 |
| `SPI` (ESP32 built-in) | — |

---

## Building & Uploading

1. Open `Poetry_Box_2/Poetry_Box_2.ino` in Arduino IDE.
2. Set **Board** → `ESP32 Dev Module` (or your specific variant).
3. Set **Upload Speed** → `115200`.
4. Select the correct **Port**.
5. Click **Upload**.

---

## Features

### Print Random Poem
Press **GREEN** on the home screen.  
The firmware selects a poem at random, avoiding any poem printed in the last 64 sessions  
(history stored in internal flash via LittleFS).

### Favorites
- **YELLOW → Favorites menu** from the home screen.
- **BLUE (within the Favorites menu)** – mark the last-printed poem as a favourite.
- **GREEN (within the Favorites menu)** – print a random favourite.
- **YELLOW (within the Favorites menu)** – browse the full favourites list.
- Favourites are saved to `/Favorites/favorites.txt` on the SD card and persist across reboots.

### Browse by Author
- **BLUE** from the home screen opens the author browser.
- **YELLOW / BLUE** scroll up / down through the list.
- **GREEN** selects an author and opens their poem list.
- **GREEN** on a poem prints it.
- **RED** returns to the previous screen.

### History De-duplication
The last 64 printed poem paths are stored in `/history.txt` on the internal flash.  
When printing a random poem, the firmware tries up to 20 times to find a poem not in history  
before falling back to any random poem.

### Fast Startup (Index Cache)
On the **first boot** after a new SD card is inserted, the firmware scans all author  
directories and counts their poems.  This scan takes a few seconds.  
The result is saved to `/index.txt` in the internal LittleFS partition.  
On all subsequent boots, the index loads from flash in under 100 ms.

**To force a rebuild** (e.g. after adding or removing poems from the SD card):  
**Hold RED for 3 seconds** from any screen.

---

## Button Quick-Reference

### Home Screen
| Button | Action |
|--------|--------|
| GREEN | Print a random poem |
| YELLOW | Open Favourites menu |
| BLUE | Browse poems by author |
| RED | Show help / controls |

### Help Screen
| Button | Action |
|--------|--------|
| RED | Return to home |

### Favourites Menu
| Button | Action |
|--------|--------|
| GREEN | Print a random favourite |
| YELLOW | Browse all favourites |
| BLUE | Add the last-printed poem to favourites |
| RED | Home |

### Browse Authors / Browse Poems / Browse Favourites
| Button | Action |
|--------|--------|
| YELLOW | Scroll up |
| BLUE | Scroll down |
| GREEN | Select / print highlighted item |
| RED | Go back one level |

### Any Screen
| Input | Action |
|-------|--------|
| Hold RED (3 s) | Rebuild poem index from SD card |

---

## Project Structure

```
Poetry_Box_2/
├── Poetry_Box_2.ino  – Hardware init, setup(), loop()
├── Database.h        – Author index, poem access, history, favourites
├── Menu.h            – LCD UI state machine, button handling
└── public_poems/     – Ready-to-use public-domain poem library
    ├── Emily Dickinson/
    ├── Walt Whitman/
    └── ...

junk/                 – Old / prototype sketches (kept for reference)
    ├── EmilyESP/
    ├── ReadWrite_mod_for_all/
    ├── improved_untested_printer/
    └── ...
```

---

## Internal Flash (LittleFS) Layout

| File | Contents |
|------|----------|
| `/index.txt` | Cached author names and poem counts |
| `/history.txt` | Paths of the last 64 printed poems |

Both files are created automatically.  They can be safely deleted; they will be recreated on next boot.

---

## Troubleshooting

| Symptom | Likely cause | Fix |
|---------|--------------|-----|
| **SD CARD ERROR!** on LCD | Bad wiring or wrong CS pin | Check SPI wiring; verify `SD_CS_PIN` in `.ino` |
| **FLASH FS ERROR!** on LCD | LittleFS partition missing | Erase flash and re-upload; or change partition scheme in IDE |
| Printer prints garbage | Wrong baud rate | Check `PRINTER_BAUD` constant (default 9600) |
| "NO POEMS FOUND!" | SD card empty or wrong folder structure | Ensure author folders are at root `/` of the SD card |
| Long startup delay | Index is being built for first time | Wait; subsequent boots are fast |
| New poems not appearing | Index cache is stale | Hold RED 3 s to rebuild index |

---

## License

Poem texts are from the public domain.  
Firmware source code is released under the MIT License.
