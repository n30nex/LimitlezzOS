# Handoff: LimitlezzOS — mesh-native handheld OS for the LilyGo T-Deck

## Overview
LimitlezzOS is a concept operating-system UI for the **LilyGo T-Deck** — a handheld
LoRa device with a **2.8″ 320×240 landscape TFT LCD**, a BlackBerry-style physical
QWERTY keyboard, and a **trackball**. The OS unifies two mesh networks —
**Meshtastic** and **MeshCore** — into a single messaging surface while keeping each
radio's stack cleanly separated. Navigation is **trackball-first**: a single focus
highlight roams the UI; clicking the ball selects.

The product thesis: *users think in conversations, not networks.* So there is one
merged inbox, but **every conversation and message is tagged with its network**
(cyan = Meshtastic, amber = MeshCore) and **contacts are network-bound** (one
contact = one network address), which makes reply routing unambiguous.

## About the Design Files
The files in this bundle are **design references created in HTML** — prototypes that
demonstrate the intended look, layout, and behavior. **They are not production code to
copy verbatim.** They are authored as "Design Components" (a streaming HTML component
format) and rely on a bundled runtime (`support.js`); that runtime is an authoring
convenience, **not** something to ship.

Your task is to **recreate these designs in the target environment**. For a real
T-Deck this is typically **C/C++ with LVGL** (the common UI stack for ESP32-S3
devices) or an equivalent embedded GUI library. If this is being prototyped for web
or another platform, use that platform's idioms (React, SwiftUI, etc.). Treat the HTML
as the **single source of truth for visual + interaction design**, and re-implement it
with the destination's real input handling, fonts, and rendering.

> The device bezel (body, speaker, keyboard, trackball, "LILYGO" silkscreen) and the
> outer presentation page are **mockup chrome only**. On real hardware the OS renders
> **full-screen at 320×240**. Only the content *inside the 320×240 screen* is the
> actual product UI.

## Fidelity
**High-fidelity (hifi).** Final colors, typography, spacing, iconography, and
interaction model. Recreate the in-screen UI pixel-accurately at 320×240.

---

## Rendering constraints (important)
The target hardware is **weak** (ESP32-S3 driving LVGL on a 320×240 SPI TFT). The design
is intentionally **flat and cheap to draw**: solid color fills only, 1px borders for
separation, simple 2px focus rings, no gradients, no shadows, no blur, no transparency/
alpha blending, no glows. When recreating, prefer solid `lv_obj` fills and borders;
avoid `lv_grad`, shadows, opacity layers, and per-frame redraws. Keep animation minimal
(focus moves can snap or use a very short transition).

## Hardware & input model
- **Display:** 320×240 px, landscape, color TFT. Design everything to this exact canvas.
- **Status bar:** top 22 px. App nav bars: top 29 px.
- **Trackball:** emulated in the mock with **Arrow keys = move focus**, **Enter =
  ball click (select)**, **Esc/Backspace = back**. On hardware, map the trackball's
  4-way roll to focus movement and the ball press to "select".
- **Focus model:** exactly one focusable element is highlighted at all times. The grid
  is navigated as a uniform row-major matrix of `cols` columns:
  `up = i-cols`, `down = i+cols`, `left = i-1`, `right = i+1` (clamped; left/right do
  not wrap across rows). On screens with no focusable items, up/down scroll the list.
- **Keyboard:** the physical QWERTY types directly into the active text field (e.g. the
  message composer). In Messages, number keys **1 / 2 / 3** switch the network filter
  (All / Meshtastic / MeshCore).

---

## Design tokens

### Color — surfaces (dark theme, cool neutral)
| Token | Value | Use |
|---|---|---|
| Screen base | `#0b0d11` / `#0c0e12` | App background |
| Lock gradient | `radial-gradient(125% 90% at 50% -10%, #16242c, #0c0f14 55%, #08090c)` | Lock screen |
| Nav bar | `linear-gradient(#222831, #141a21)` | App headers |
| Card / row idle | `rgba(255,255,255,0.022)` | List rows |
| Row hairline | `rgba(255,255,255,0.05–0.06)` | Dividers / insets |
| Text primary | `#eef1f4` | Titles, body |
| Text secondary | `#869099` / `#8b939c` | Subtitles |
| Text tertiary | `#6f7882` / `#7c838c` | Meta, timestamps |

### Color — network identity (critical, used everywhere as tags)
| Network | Color (oklch) | ≈ hex | Meaning |
|---|---|---|---|
| **Meshtastic** | `oklch(0.74 0.12 205)` | ~`#3fb6c8` cyan | net dot, headers, accents |
| **MeshCore** | `oklch(0.78 0.13 72)` | ~`#e0a23a` amber | net dot, headers, accents |
| OS primary | `oklch(0.82 0.12 165)` | ~`#3fd9a0` mint | toggles-on, send button, unread badge, active tab |
| **Focus ring** | `oklch(0.95 0.02 220)` | near-white | the trackball selection highlight |

The focus highlight is intentionally a near-white ring (BlackBerry-style) so it never
competes with the cyan/amber network tags.

### Color — app icon hues
**Flat solid tiles** — each app icon is a single solid color `oklch(0.64 0.13 H)`
(no gradient, no gloss, no bevel). Hues (H): Messages 165, Meshtastic 205, MeshCore 72,
Contacts 318, App Store 280, Files 242; Terminal & Settings use neutral graphite `#444b56`.

### Typography
- **UI:** `Helvetica Neue, Helvetica, Arial, sans-serif` (period-correct BlackBerry/iOS-6 feel).
- **Mono:** `SF Mono, Menlo, Consolas, monospace` (Terminal, IDs, timestamps, file sizes).
- **Icons:** Material Symbols Rounded, **filled** (`FILL 1`), subset-embedded — see Assets.
- Sizes (at 320×240, do not go smaller): status bar 10–11px; app/nav title 13px;
  row title 11.5–12.5px; row subtitle 9–10px; section labels 9px (letter-spacing .08em,
  uppercase); lock clock 55px/300-weight; big numerals (calculator-style) 33px.

### Radius / elevation (flat, low-cost)
- Tiles 13–14px; small tiles 8–10px; rows 9–11px; pills/toggles 999px; cards 11px.
- **All fills are solid colors** — no gradients, no blur, no drop/inner shadows, and no
  semi-transparent gloss/overlay layers. This is deliberate: the ESP32-S3 + LVGL target
  is not powerful, so the UI avoids alpha compositing, gradients, and glows entirely.
- App icon tile = one solid color `oklch(0.64 0.13 H)` (graphite `#444b56` for neutral apps).
- Focus highlight = a solid **2px ring** in the focus color (`inset 0 0 0 2px <focus>` on
  rows; `0 0 0 2px <focus>` + `scale(1.09)` on launcher tiles). **No glow.**
- Dividers/edges = solid 1px borders (e.g. `border-bottom:1px solid #11161d`).

---

## Screens / Views

### 1. Lock
- **Purpose:** entry; shows time + network presence; unlock with ball-click/Enter (or tap).
- **Layout:** centered column. Top row (inset 14px): left = Meshtastic+MeshCore glyphs +
  "2 networks"; right = "87%" + battery glyph. Center: clock `14:23` (55px/300),
  "Thursday, June 12" (12px). Pill: lock glyph + "Click trackball or press Enter".
  Footer: "LimitlezzOS 1.0".

### 2. Home (app launcher)
- **Purpose:** launch apps. **Single iOS-style grid** (list/carousel were intentionally cut).
- **Layout:** 22px status bar (left: mesh glyph + "7 nodes"; right: signal bars, `14:23`,
  battery). Below: **4-column grid**, 8 apps, ~46px glossy tiles + 10px label.
  Order: **Messages, Meshtastic, MeshCore, Contacts / App Store, Terminal, Files, Settings**.
- **Focus:** near-white ring + 1.09 scale + colored glow on the focused tile; label turns white.

### 3. Messages (the core surface)
- **Purpose:** one unified inbox feeding from both networks.
- **Layout:** nav bar ("Messages", back chevron, compose pencil in mint). Tab row:
  **Direct | Channels** (active tab = white text + mint underline). Filter chip row:
  **All / ● Meshtastic / ● MeshCore** (colored dots; active chip fills with that color;
  shows the 1/2/3 keyboard hint). Scrolling list below.
- **DM row:** 33px circular avatar (network-colored gradient) with a **network dot badge**
  bottom-right (cyan/amber); name + timestamp; last-message snippet; mint unread pill.
  Rows whose network is disabled render at **opacity 0.4**. If a network is off, a dashed
  note appears: *"<Network> disabled in Settings — conversations hidden, history kept."*
- **Channels tab:** broadcast/room rows (Meshtastic channels = cyan tile, MeshCore rooms =
  amber tile) with name, sub ("Primary · 7 nodes" / "Room · 6 members"), last line.
- **Sample data:** Ava Reyes (Meshtastic, 2 unread), Dmitri K (MeshCore, 1), Base-01,
  Ridge Repeater, Sam OK1QRP, Weather-Sensor.

### 4. Conversation
- **Purpose:** read/reply within one network-bound thread.
- **Layout:** nav bar shows contact name + a **network tag** ("● Meshtastic · 2 hops").
  Centered "Encrypted · <Network> · <addr>" caption. Message bubbles: incoming `#1c212a`
  left-aligned; **self** = mint gradient, right-aligned, dark text. Compose bar: rounded
  text input + a **send button that states the outgoing network** ("Meshtastic ▶") tinted
  to that network — reply routing is never ambiguous because the contact is network-bound.

### 5. Meshtastic (network stack manager)
- Cyan-themed nav bar + status dot. **Identity card:** "JES" avatar, "Jess · JESS",
  `!7c3af1d0 · Region US · LongFast`, node count. Tabs **Nodes | Channels**.
  Node rows: short-code tile, name + role badge, `id · distance · last-heard`, SNR
  (color-coded: ≥0 green, ≥-10 amber, else red). Channels: LongFast (PRIMARY), Emergency.

### 6. MeshCore (network stack manager)
- Amber-themed. **Identity card:** key glyph, "Jess [Companion]", `MC·2a9f·e41c · ed25519`,
  contact count. Tabs **Contacts | Rooms**. Rows show a role badge (Repeater / Chat /
  Sensor / Room) with role-specific icon, `id · distance · last`, and an online dot.

### 7. App Store
- **Purpose:** install apps onto the OS (reinforces "runs apps").
- **Layout:** nav bar + search glyph. Featured card (gradient) = "Node Mapper". "Apps &
  utilities" list: icon tile, name, ★ rating · category · size, and a **GET / OPEN /
  UPDATE** pill. Tapping GET shows a brief "…" installing state then flips to OPEN.
- **Includes Calculator and Notes as downloads** (they were intentionally removed as
  built-in apps and live here instead). Plus APRS Bridge, Weather Mesh, Mesh BBS,
  Signal Scope, LoRa Chess.

### 8. Contacts
- Unified node directory across both networks. Rows: avatar + **network dot**, name,
  `id · role`, last-heard + SNR. Tapping opens **Contact detail**: large avatar, name,
  network tag + role, **Message** (mint) + **Trace** (route) actions, and a spec table
  (Node ID, Hardware, Distance, SNR, Battery, Last heard).

### 9. Settings
- **Airtime scheduler card** (top): a split bar (cyan vs amber) showing the radio
  time-slice; label ("Split 50 / 50" / "Meshtastic 100%"); a note explaining that
  disabling one network gives the other full throughput & lower latency.
- **Networks section:** first-class **Meshtastic** and **MeshCore** toggles (cyan/amber).
  Toggling off **grays that network's threads in Messages but keeps history**, and
  rebalances the airtime bar to 100% for the remaining network.
- **Groups:** Radio (Region, Modem preset, TX power — tap to cycle), Connectivity
  (Wi-Fi, GPS toggles), Display (Brightness slider — left/right adjusts when focused;
  Dark mode; Sleep after), Power (Power saving), **Device → System & battery** (opens
  System), About (Device, Firmware).

### 10. System (opened from Settings → Device)
- Battery ring (conic gradient, 87%, 3.94V), "14h 20m left · Discharging · -142 mA",
  "Battery healthy" chip. Stat bars: CPU 12%, RAM 84/512 KB, Flash 6.2/16 MB, Temp 24°C,
  Uptime 3d 04:12. Two cards: LoRa TX/RX (412/1284), Air utilization (3.4%).

### 11. Terminal
- Black console, mono green text, blinking block cursor. Shows `mesh --info`,
  `mesh --nodes`, `mesh --airtime` output. Trackball up/down scrolls.

### 12. Files
- `/sdcard` path bar; rows of folders (config, logs, maps) and files (nodes.db,
  channel.url, firmware.bin, README.txt) with mono sizes.

---

## Interactions & Behavior
- **Navigation stack:** opening an app pushes the current view; Back pops it (Settings →
  System → back returns to Settings; Conversation → back returns to Messages). Home/lock
  reset the stack.
- **Trackball/keys:** Arrow = move focus within the current screen's matrix; Enter =
  activate focused item; Esc = back. The focused list row auto-scrolls into view
  (adjust container `scrollTop`; never use `scrollIntoView`).
- **Messages filters:** All / Meshtastic / MeshCore, also via keys 1/2/3. Filtering is
  instant; disabled-network rows are dimmed (not removed) and annotated.
- **Compose:** typing updates the draft; Enter (or the network-tagged send button)
  appends a right-aligned self bubble and clears the draft; the thread auto-scrolls to
  bottom.
- **Settings toggles:** flip instantly; network toggles drive both the airtime bar and
  the Messages dimming. Value rows cycle through options on activate. Brightness slider
  changes with left/right while focused.
- **App Store install:** GET → "…" (≈1.1s) → OPEN.
- **Transitions:** subtle. Focus ring 120–150ms ease; toggles/airtime-bar 200–300ms;
  carousel/launcher motion uses `cubic-bezier(.2,.85,.25,1)` (only relevant if a
  carousel is ever reintroduced — currently grid-only).

## State Management
Single top-level state object:
- `view` (`'lock'|'home'|'messages'|'convo'|'meshtastic'|'meshcore'|'appstore'|'contacts'|'contact'|'settings'|'system'|'terminal'|'files'`)
- `focus` (int index into current screen's focusable list)
- `navStack` (array of prior views for Back)
- `netMt`, `netMc` (booleans — network enabled; drive Messages dimming + airtime split)
- `msgTab` (`'dms'|'channels'`), `msgFilter` (`'all'|'meshtastic'|'meshcore'`)
- conversation: current thread + `convoMsgs` + `draft`
- `mtTab`, `mcTab` (per-network tab), `contactSel` (detail subject)
- `settings` { region, preset, tx, wifi, gps, bright, dark, timeout, save }
- `storeOv` (per-app install-state overrides)

Data is static/mock (node lists, threads, channels, store apps). In a real build these
come from the Meshtastic & MeshCore stacks (node DB, channel config, contact list,
telemetry) over the device's transport.

## Assets
- **Icons:** Material Symbols Rounded (filled). The mock embeds a **subset** as a
  base64 `@font-face` in `ms-rounded.css` so it renders offline and in exports. In a
  real build, use your platform's icon set (or LVGL's font/symbol system) — the glyph
  names used are: `forum, hub, lan, group, storefront, terminal, folder, settings,
  chevron_left/right, edit, send, search, chat_bubble, route, key, public, graphic_eq,
  cell_tower, wifi, location_on, brightness_high, dark_mode, schedule, bolt, monitoring,
  tag, campaign, router, sensors, person, star, calculate, sticky_note_2, satellite_alt,
  thermostat, dns, videogame_asset, map, arrow_back_ios_new, keyboard_arrow_down, mic, keyboard`.
- **Fonts:** Helvetica (system) + system monospace. No webfont required beyond icons.
- **No bitmap images** — all surfaces are CSS gradients/shadows.

## Files (in this bundle)
- `LimitlezzOS.dc.html` — the full OS: device chrome, lock, home, all app screens,
  navigation engine, and state. **Read the in-screen markup + the logic class for exact
  values.** The 320×240 content is the product; the surrounding device is mock chrome.
- `Launcher.dc.html` — the home-screen app grid component (also supports list/carousel
  variants, but the OS uses **grid only**).
- `ms-rounded.css` — embedded Material Symbols subset (reference for which glyphs are used).
- `support.js` — the prototype runtime (authoring tool; **do not ship**).

To preview the prototype, open `LimitlezzOS.dc.html` in a browser. Use Arrow keys +
Enter (or drag the on-screen trackball) to navigate; Esc to go back.
