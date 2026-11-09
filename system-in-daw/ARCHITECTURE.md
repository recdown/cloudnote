# Architecture and implementation plan

## Confirmed product contract

### SYSTEM IN
Windows applications → Windows playback endpoint → audio bridge → SYSTEM IN VST3 → DAW track.

### DAW OUT
DAW output → DAW OUT VST3 → audio bridge → Windows virtual recording endpoint → OBS / streaming / communications applications.

DAW OUT is a virtual recording endpoint. It does not replace or modify the physical microphone.

## Scope

The first product version is a low-latency, stable, low-overhead bidirectional bridge.

Required:
- Both directions.
- Multichannel transport and explicit channel mapping.
- Sample-rate and buffer-format handling.
- Connection state and lightweight underrun/overrun counters.
- Recovery after DAW restart, endpoint changes, and stream stop/start.

Out of scope:
- Mixer or routing matrix UI.
- Scene management.
- Network audio.
- Effects, voice changing, or real-time vocal correction.
- A separate cross-plugin Send/Receive routing system.

The DAW owns track routing, sends, mute, gain, mixing, and effect chains.

## Technical direction

1. Validate Windows audio endpoint access in user mode first.
2. Evaluate a mature virtual audio endpoint/driver rather than starting with a custom kernel driver.
3. Keep transport and IPC in user mode.
4. Build native VST3 plugins against the official Steinberg VST3 SDK.
5. Keep real-time audio callbacks free of blocking operations, disk/network I/O, and dynamic allocation.
6. Keep UI and diagnostics outside the real-time processing path.

WASAPI can capture/render audio through existing endpoints; it does not by itself create a new system-wide virtual playback or recording device. A virtual endpoint still requires an appropriate device/driver implementation or a compatible existing driver.

## Planned components

- `SystemIn`: VST3 receiver endpoint inside the DAW.
- `DawOut`: VST3 sender endpoint inside the DAW.
- `AudioBridgeService`: user-mode transport coordinator and shared-buffer/IPC owner.
- `VirtualAudioEndpoint`: endpoint integration, preferably based on a mature driver solution in the first iteration.
- `Installer`: installs the service, VST3 bundle, and endpoint component when licensing/signing requirements are resolved.

## Milestones

- [x] Create isolated development branch and project directory.
- [x] Add initial WASAPI loopback feasibility probe and Windows CI workflow.
- [ ] Verify the probe builds and runs on a Windows runner.
- [ ] Select and document the virtual endpoint strategy.
- [ ] Specify the transport protocol, buffer ownership, channel layout, and recovery behavior.
- [ ] Implement the user-mode bridge service.
- [ ] Implement SYSTEM IN and DAW OUT VST3 plugins.
- [ ] Integrate the endpoint and installer.
- [ ] Validate in Studio One with MOTU M2; then test other hosts.

## Engineering acceptance criteria

- No blocking calls or heap allocation in the audio callback.
- No audio-thread logging, file I/O, or network I/O.
- Explicit handling of sample-rate changes, channel-count changes, stream restarts, and endpoint invalidation.
- Measured round-trip and one-way latency, not guessed figures.
- Long-duration stability tests with underrun/overrun reporting.
