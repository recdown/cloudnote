# Architecture and implementation plan

## Confirmed product contract
SYSTEM IN: Windows applications → Windows playback endpoint → audio bridge → SYSTEM IN VST3 → DAW track.
DAW OUT: DAW output → DAW OUT VST3 → audio bridge → Windows virtual recording endpoint → OBS / streaming / communications applications.
DAW OUT is a virtual recording endpoint. It does not replace or modify the physical microphone.

## Scope
The first product version is a low-latency, stable, low-overhead bidirectional bridge.
Required: both directions; multichannel transport and explicit channel mapping; sample-rate and buffer-format handling; connection state and underrun/overrun counters; recovery after DAW restart, endpoint changes, and stream stop/start.
Out of scope: mixer/routing matrix UI; scene management; network audio; effects, voice changing, or real-time vocal correction; separate cross-plugin Send/Receive routing.
The DAW owns track routing, sends, mute, gain, mixing, and effect chains.

## Technical direction
1. Validate Windows endpoint access in user mode first.
2. Evaluate a mature virtual audio endpoint/driver before considering a custom kernel driver.
3. Keep transport and IPC in user mode.
4. Build native VST3 plugins against the official Steinberg VST3 SDK.
5. Keep real-time callbacks free of blocking operations, disk/network I/O, and dynamic allocation.
6. Keep UI and diagnostics outside the real-time path.

WASAPI accesses existing endpoints; it does not create a system-wide virtual playback or recording device. A virtual endpoint still requires an appropriate driver or compatible existing driver.

## Components
- SystemIn: VST3 receiver endpoint inside the DAW.
- DawOut: VST3 sender endpoint inside the DAW.
- AudioBridgeService: user-mode transport coordinator and shared-buffer/IPC owner.
- VirtualAudioEndpoint: integration with a selected virtual endpoint driver.
- Installer: service, VST3 bundle, and endpoint component when licensing/signing requirements are resolved.

## Milestones
- [x] Isolated development branch and project directory.
- [x] Initial WASAPI loopback probe and Windows CI workflow.
- [x] User-confirmed capture from MOTU M Series: 48 kHz, stereo, float32, active signal.
- [x] First SPSC transport primitive and unit-test source added.
- [ ] Run ring-buffer unit test on Windows CI.
- [ ] Specify and implement cross-process shared-memory protocol.
- [ ] Select and document virtual endpoint strategy.
- [ ] Implement user-mode bridge service.
- [ ] Implement SYSTEM IN and DAW OUT VST3 plugins.
- [ ] Integrate endpoint and installer.
- [ ] Validate in Studio One with MOTU M2; then test other hosts.

## Acceptance criteria
- No blocking calls or heap allocation in audio callbacks.
- No audio-thread logging, file I/O, or network I/O.
- Explicit handling of sample-rate/channel changes, stream restarts, and endpoint invalidation.
- Measured latency, not guessed figures.
- Long-duration stability tests with underrun/overrun reporting.
