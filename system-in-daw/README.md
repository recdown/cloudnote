# SYSTEM IN / DAW OUT

Windows ↔ DAW audio bridge project.

## Product definition

- **SYSTEM IN**: Windows system playback audio → DAW.
- **DAW OUT**: DAW audio → Windows virtual recording endpoint.

The bridge transports audio. Track processing, mixing, sends, mute and routing remain the DAW's responsibility.

## Current status

This repository currently contains an early Windows feasibility probe only. It is **not** a working DAW bridge yet.

Implemented in this initial commit:
- A WASAPI loopback capture probe for the default Windows render endpoint.
- A Windows x64 GitHub Actions build workflow.

Not implemented yet:
- Virtual playback/recording endpoints.
- The IPC/audio transport engine.
- SYSTEM IN or DAW OUT VST3 plugins.
- Device installation or a product installer.
- End-to-end latency and stability validation in Studio One.

## Build artifact

The workflow builds `SystemInProbe.exe` on a Windows runner and uploads it as a GitHub Actions artifact. This executable only measures loopback capture activity; it does not route audio into a DAW.

## Build locally on Windows

Requires CMake 3.24+, Visual Studio 2022 C++ build tools, and a Windows SDK.

```powershell
cmake -S system-in-daw -B system-in-daw/build -A x64
cmake --build system-in-daw/build --config Release
.\system-in-daw\build\Release\SystemInProbe.exe 30
```

See [ARCHITECTURE.md](ARCHITECTURE.md) for the agreed architecture and implementation boundaries.
