# UAE4ALL

UAE4All is an optimized "Lite" Commodore Amiga 500 emulator based on the E-UAE core. It emulates Commodore Amiga 500 hardware with OCS (Original Chip Set), 512 KB / 1 MB / 2 MB Chip RAM, and up to 4 floppy drives (currently restricted to 2 - DF0 & DF1).

This repository provides both a **standalone** emulator build (optimized for MiyooCFW handhelds and Linux) and a **Libretro core** (`uae4all_libretro.so`) for RetroArch / Libretro frontends. Remember to provide `uae4all_libretro.info` file in RetroArch **`core_info`** directory for core to be detactable.

Emulation can utilize multiple Motorola 68000 CPU cores:
- **UAE core**: Full C-based core with the highest game compatibility (default for standalone release).
- **Cyclone**: Highly optimized ARM assembly core for maximum performance on ARM architectures.
- **FAME / FAME-C**: Fast M68k emulation library for x86 and ARM platforms.

---

## Flavors

### Standalone (MiyooCFW & Linux)
Revamped version of UAE4ALL for MiyooCFW supported devices (Bittboy, PocketGo, Powkiddy V90 / Q90 / Q20, etc.) and native Linux.
- Based on the GCW0 port by zear & Nebuleon (with contributions from smurline and goldmojo for other platforms).
- Upstream stock emulator for MiyooCFW 1.3.3 was based on [zear's repository](https://github.com/zear/uae4all), originally ported by @jamesofarrell with FAME core (see "james_zear" branch).
- Current releases default to the UAE core for wider game compatibility, while still supporting Cyclone and FAME cores.

### Libretro Core (`uae4all_libretro`)
Downstream implementation based on [Chips-fr/uae4all-rpi](https://github.com/Chips-fr/uae4all-rpi), adapted into this codebase to produce the `uae4all_libretro.so` core for RetroArch and other Libretro frontends.

---

## Changelog (Standalone / MiyooCFW)

**UAE4ALL rev. 1.2**
- Added extra "2MB RAM" option in menu (increased memory to enable extra features in particular games).
- Added virtual button mapping to L2/R2 and L3/R3 (more keyboard bindings at your will).
- Enabled music in menu (via `SDL_mixer`, negligible in-game performance impact).

**UAE4ALL rev. 1.1a**
- Added log warning about missing `kick.rom` file when not found in `/mnt/.uae4all` directory.

**UAE4ALL rev. 1.1**
- "Status bar" option is now configurable in the main menu and `uae4all.cfg` (`STATUS_BAR 0` or `-1`).
- Added hotkeys to increase/decrease throttle (`SELECT` + `X` / `Y`).

**UAE4ALL rev. 1.0**
- Load DF0 as first argument parameter (allows loading ADF/ROM directly from console frontend).
- Show/hide status bar option (editable through `*.cfg` or emu GUI).
- Disabled splash screen by default (faster initial load speed).
- Semi-darkmode for all notifications.
- Fixed A/B button mapping for V90/Q90 in menu screen (cohesive with Gmenu2X / SimpleMenu).
- Frame throttling fine tuning now works in "auto" frameskip mode.
- Added lib7z savestate compression method.
- Compatibility increase: switched default m68k emulation to UAE core.

---

## BIOS / Kickstart ROM Requirements

The emulator requires an Amiga 500 Kickstart 1.3 BIOS ROM to run games.

### Standalone
Place your Kickstart 1.3 ROM named `kick.rom` into:
- `$HOME/.uae4all/kick.rom` (on console), or
- the local working directory alongside the executable.

### Libretro
Place the Kickstart ROM in the RetroArch **`system`** directory using the exact filename below:

| System | Version | Filename | Size | MD5 |
|---|---|---|---|---|
| Amiga 500 | Kickstart v1.3 rev 34.005 | **`kick34005.A500`** | 262,144 bytes | `82a21c1890cae844b3df741f2762d48d` |

---

## Compiling Instructions

### 1. Build Environment Setup

#### Cross-compiling with Docker (MiyooCFW)
You can use the official MiyooCFW toolchain Docker container:
```bash
git clone https://github.com/Apaczer/uae4all
cd uae4all
docker run --volume ./:/src/ -it miyoocfw/toolchain-shared-uclibc:latest
cd /src
```

Alternatively, set up a native Debian 9 toolchain environment for MiyooCFW.

---

### 2. Standalone Build (`Makefile`)

- **Cross-compile Standalone MiyooCFW binary**:
  ```bash
  make clean
  make -j$(nproc) platform=miyoo
  ```

- **Distribution IPK package** (for Gmenu2X / MiyooCFW):
  ```bash
  make -j$(nproc) platform=miyoo gm2xpkg-ipk
  ```

- **Native Linux build**:
  ```bash
  make -j$(nproc)
  ```

---

### 3. Libretro Core Build (`Makefile.libretro`)

- **Cross-compile core for MiyooCFW**:
  ```bash
  make -j$(nproc) -f Makefile.libretro platform=miyoo
  ```

- **Native build**:
  ```bash
  make -j$(nproc) -f Makefile.libretro
  ```

---

### 4. M68k CPU Core Selection

Both makefiles allow selecting the desired M68k CPU core by passing flags:

| Flag | Description |
|---|---|
| `UAE_CORE=1` | UAE C-based core (highest compatibility, default for standalone) |
| `CYCLONE_CORE=1` (*) | Cyclone ARM assembly core (fastest on ARM, default for Miyoo Libretro) |
| `FAME_CORE=1` | FAME M68k emulation library (x86 assembly) |
| `FAME_CORE=1 FAME_CORE_C=1` | FAME C implementation (default for native Libretro, to heavy for cross-compiling locally) |

Example:
```bash
make -j$(nproc) UAE_CORE=1
# or for libretro:
make -j$(nproc) -f Makefile.libretro platform=miyoo CYCLONE_CORE=1
```

*) *The CYCLONE is an experimental core, currently missing Savestate implementation and maybe less accurate than others*

---

## Profile-Guided Optimization (PGO)

All release builds for standalone have Profile-Guided Optimization applied for an approximate 5–10% performance boost:

1. Set `PROFILE = YES` in `Makefile`.
2. Compile the binary.
3. Run the emulator on the target device for a few minutes with representative gameplay (exit normally through the GUI menu to write profiling data).
4. Copy the generated `*.gcda` files from `/mnt/profile` back into the `/src` directory in the repository.
5. Set `PROFILE = APPLY` in `Makefile`.
6. Recompile for the final optimized build.

---

## Controls

### Standalone (MiyooCFW)

- B: Joystick fire button
- Y: Mouse left button
- X: Mouse right button
- L1: Switches between joystick and mouse control (when using mouse press R to change the mouse speed)
- R1: Virtual keyboard, use the B button to press a key (A/X/Y/L2/R2/L3/R3 can be used to map a key to, return to the main menu to reset keymaps).
- RESET: Brings up the UAE4all menu
- SELECT + R1: Quick load state.
- SELECT + L1: Quick save state.
- START: SuperThrottle on/off
- SELECT + Y: increase Throttle (only under v20220507 release or later)
- SELECT + X: decrease Throttle (only under v20220507 release or later)

### Libretro Core (RetroPad)

| RetroPad Button | Action |
|---|---|
| **D-Pad** | Joystick directions / Mouse movement (in mouse mode) |
| **B** | Fire button 1 / Red |
| **A** | Fire button 2 / Blue |
| **L2** | Left mouse button |
| **R2** | Right mouse button |
| **L** | Switch to previous floppy disk (DF0:) |
| **R** | Switch to next floppy disk (DF0:) |
| **Select** | Toggle virtual keyboard |
| **Start** | Toggle mouse emulation |

- **Mouse Emulation**: Pressing `Start` toggles mouse mode, where the D-Pad and Fire buttons control mouse pointer and buttons.
- **Dual Joystick Support**: Automatically switches between mouse and second joystick when mouse or 2nd joystick inputs are triggered.
- **Multi-Disk Switching**: `L` and `R` buttons switch the inserted disk on `DF0:` for multi-disk games. Disks should follow the standard naming scheme with `(Disk X of Y)` (e.g. `Game (Disk 1 of 2).adf`).

---

## Compatibility List

Differences observed between UAE core and FAME core:

**Titles with better performance on FAME:**
- *Zool II* – more optimized / higher framerate.
- *Jim Power in Mutant Planet* – more optimized / higher framerate.
- *Xenon 2: Megablast* – crashes after sprite collision with bomb on UAE core.

**Titles running better on UAE core (compared to FAME):**
- *Superfrog* – smooth video with "auto" frameskip (no video jitter).
- *Alien Breed* – smooth video with "auto" frameskip (no video jitter).
- *James Pond* – boots and runs without crashing (CR PDX by TIC).
- *The Great Giana Sisters* – boots and runs without crashing.
- *Nicky II* – boots and runs without crashing.
- *The Addams Family Mansion Mayhem* – no sprite freezes during gameplay.
- *Moonstone: A Hard Days Knight* – passes loading screen when disk B is read in DF1.

**General UAE4ALL emulation known quirks:**
- *James Pond* – player sprite can pass through walls.
- *Moonstone: A Hard Days Knight* – requires mounting disk 1 (DF0) and disk 2 (DF1) simultaneously.

---

## FAQ

**Q: What is the difference between this version and stock UAE4ALL (MiyooCFW 1.3.3)?**  
A: This revision provides higher compatibility (UAE core option), user-friendly features (command-line direct ADF loading, throttle hotkeys, status bar toggle, 2MB RAM option, virtual buttons), bug fixes, and lib7z savestates. The source code is also maintained and open for community contributions.

**Q: How do I set up the standalone emulator?**  
A: Use `./uae4all` by adding exec permission and copy the [`./data/`](https://github.com/Apaczer/uae4all/tree/master/data) folder alongside it. Place your Kickstart ROM at `$HOME/.uae4all/kick.rom`.

**Q: How do I set up the Libretro core in RetroArch?**  
A: Copy `uae4all_libretro.so` to your RetroArch cores folder and place `kick34005.A500` into your RetroArch `system/` directory.

---

## Credits & Upstream

- UAE & E-UAE authors for base Amiga emulation.
- Chui & fox68k for the original UAE4ALL.
- zear & Nebuleon for the GCW0 port.
- smurline & goldmojo for additional platform changes.
- @jamesofarrell for the initial MiyooCFW port.
- [Chips-fr](https://github.com/Chips-fr/uae4all-rpi) for the Libretro port.
- [Apaczer](https://github.com/Apaczer/uae4all) for MiyooCFW improvements, standalone updates, and Libretro integration.
