"""
PlatformIO native simulator SDL2 configuration.

Linux/macOS use sdl2-config or pkg-config when available. Windows uses a local
SDL2 development bundle installed by scripts/ensure_sdl2_windows.ps1, or an
SDL2_DIR environment variable that points at x86_64-w64-mingw32.
"""
import os
import shutil
import subprocess
from pathlib import Path

Import("env")

SDL2_VERSION = "2.32.10"


def fail(message):
    print("")
    print("SDL2 setup error:")
    print(message)
    print("")
    env.Exit(1)


def append_flags(command):
    output = subprocess.check_output(command, text=True).strip()
    env.Append(**env.ParseFlags(output))


project_dir = Path(env.subst("$PROJECT_DIR"))

if os.name == "nt":
    candidates = []
    env_dir = os.environ.get("SDL2_DIR")
    if env_dir:
        candidates.append(Path(env_dir))
    candidates.append(project_dir / ".deps" / f"SDL2-{SDL2_VERSION}" / "x86_64-w64-mingw32")
    candidates.append(project_dir / ".deps" / "SDL2" / "x86_64-w64-mingw32")

    sdl_root = None
    for candidate in candidates:
        if (
            (candidate / "include" / "SDL2" / "SDL.h").exists()
            and (candidate / "lib").is_dir()
            and (candidate / "bin" / "SDL2.dll").exists()
        ):
            sdl_root = candidate
            break

    if sdl_root is None:
        fail(
            "Windows native builds need SDL2. Run:\n"
            "  powershell -ExecutionPolicy Bypass -File scripts/ensure_sdl2_windows.ps1\n"
            "Then rerun:\n"
            "  pio run -e native"
        )

    env.Append(
        CPPDEFINES=["SDL_MAIN_HANDLED"],
        CPPPATH=[str(sdl_root / "include" / "SDL2")],
        LIBPATH=[str(sdl_root / "lib")],
        LIBS=["SDL2"],
    )

    dll = sdl_root / "bin" / "SDL2.dll"

    def copy_sdl2_dll(source, target, env):
        build_dir = Path(env.subst("$BUILD_DIR"))
        build_dir.mkdir(parents=True, exist_ok=True)
        shutil.copy2(dll, build_dir / "SDL2.dll")

    env.AddPostAction("$BUILD_DIR/${PROGNAME}${PROGSUFFIX}", copy_sdl2_dll)
else:
    if shutil.which("sdl2-config"):
        append_flags(["sdl2-config", "--cflags"])
        append_flags(["sdl2-config", "--libs"])
    elif shutil.which("pkg-config"):
        append_flags(["pkg-config", "--cflags", "sdl2"])
        append_flags(["pkg-config", "--libs", "sdl2"])
    else:
        fail(
            "Install SDL2 development tools first. Examples:\n"
            "  macOS: brew install sdl2\n"
            "  Debian/Ubuntu: sudo apt install libsdl2-dev"
        )
