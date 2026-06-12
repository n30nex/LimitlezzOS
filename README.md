# LimitlezzOS

A mesh-native handheld OS for the **LilyGO T-Deck** (ESP32-S3, SX1262 LoRa,
320×240 TFT, BlackBerry QWERTY, trackball) that unifies **Meshtastic** and
**MeshCore** into a single network-tagged inbox, driven entirely by the
trackball.

This repo implements the **UI layer** of the design handoff
(`docs/design/`) in C on **LVGL 8.3** — the real target stack for the
T-Deck — exactly as the master spec prescribes: flat solid fills, 1px
hairlines, a 2px near-white focus ring, baked font tables, no images, no
gradients, no alpha layering.

![screens](docs/screens.png)

## Layout

```
platformio.ini        two envs: tdeck (ESP32-S3 firmware) + native (SDL2 sim)
partitions.csv        OTA_0 / OTA_1 / otadata / config / appfs  (locked early, spec §11)
include/lv_conf.h     LVGL 8.3.11, aggressively stripped (spec §4.4)
src/ui/theme.h        design tokens (colors computed from the design's oklch values)
src/ui/theme_colors.h generated oklch -> hex table
src/ui/data.{h,c}     mock data model = the seam where the radio backend plugs in
src/ui/ui.{h,c}       state machine, nav stack, trackball focus engine, shared widgets
src/ui/screens/       the 13 screens
src/ui/fonts/         Material Symbols Rounded subsets baked to LVGL C arrays
src/main_tdeck.cpp    hardware bring-up: ST7789, I2C keyboard @0x55, trackball GPIOs
sim/main_sim.c        SDL2 simulator + headless screenshot mode
docs/design/          the original design handoff bundle (source of truth)
```

## Build & run

**Simulator** (needs SDL2: `brew install sdl2`):

```sh
pio run -e native
.pio/build/native/program                      # interactive, 2x scale window
.pio/build/native/program --shots out/         # dump every screen as BMP
```

Keys: arrows = trackball roll · Enter = ball click · Esc/Backspace = back ·
1/2/3 = Messages network filter · typing goes into the conversation composer.

**T-Deck firmware**:

```sh
pio run -e tdeck -t upload                     # flash over USB-C
```

Current footprint: 582 KB flash (11% of the 5 MB OTA slot), 147 KB static RAM.

## What's implemented (UI portion of spec Stage 1/2)

- **Trackball-first focus engine** — exactly one focused element; row-major
  grid nav (`up=i-cols, down=i+cols`, clamped, no row wrap); focusless
  screens scroll; focused row auto-scrolls into view. Left/right also
  switches tabs on tabbed screens so everything is reachable by trackball.
- **Lock** — clock, network presence, unlock via ball click.
- **Home** — single iOS-style 4×2 grid, solid color tiles, near-white ring.
- **Messages** — unified inbox; Direct/Channels tabs; All/Meshtastic/MeshCore
  filter chips (keys 1/2/3); per-thread network dot + unread badge; threads
  of a disabled network are dimmed, not removed, with a "history kept" note.
- **Conversation** — network-bound thread: tag in the nav bar, encrypted
  caption, bubbles, and a send button that names the outgoing network so
  reply routing is never ambiguous. QWERTY types into the composer; Enter
  appends the bubble and pins the thread to the bottom.
- **Meshtastic / MeshCore managers** — per-network identity cards,
  Nodes/Channels and Contacts/Rooms tabs, SNR color coding, role badges,
  online dots.
- **App Store** — featured card + install flow (GET → "…" → OPEN).
- **Contacts / detail** — unified directory with network dots; detail page
  with Message (jumps into the bound conversation) and spec table.
- **Settings** — airtime scheduler bar that rebalances live when the
  first-class network toggles flip (and drives the Messages dimming);
  value-cycling rows; brightness slider (left/right while focused);
  System & battery page (arc gauge + stat bars).
- **Terminal / Files** — mono console with blinking cursor; /sdcard listing.

## What's *not* here (backend phases of the master spec)

Radio HAL + TDM scheduler, the actual Meshtastic/MeshCore stacks, Lua app
VM/sandbox, App Store networking, OTA logic, Feedback Manager (LED/buzzer/
backlight), onboarding flow. `src/ui/data.c` is the seam: replace its
constants with the Messaging service and the UI carries over unchanged
(spec phases 1.3, 1.6, 2.2–2.3).

## Hardware notes

`src/main_tdeck.cpp` powers the peripheral rail (GPIO 10) before init,
drives the ST7789 via TFT_eSPI (pins in `platformio.ini`), polls the I2C
keyboard (0x55) and counts trackball pulses with interrupts. Pin map is per
LilyGO's published T-Deck reference; verify against your board revision on
first flash. `partitions.csv` is locked per spec §11 — change it only
before real deployments exist.
