# Audio transport contract (prototype v0)

This is the initial in-process transport contract. Cross-process shared memory is not implemented yet.

## Audio payload
- Sample representation: IEEE-754 float32, interleaved by frame.
- Initial validation format: 48,000 frames/second, 2 channels.
- Channel count is explicit metadata; transport code must not assume stereo.
- One frame contains one sample for each channel.

## Producer/consumer contract
The initial AudioRingBuffer is a bounded single-producer/single-consumer queue.
- One producer owns write(); one consumer owns read().
- Allocate and initialize before audio processing starts.
- read() and write() never wait, allocate, lock, or call operating-system APIs.
- A short write is an overrun; the caller counts the unaccepted frames.
- A short read is an underrun; the consumer fills the missing output frames with silence.
- The producer never overwrites unread frames.
- Reset/reconfiguration is only allowed after both sides have stopped.

The queue uses monotonically increasing 64-bit frame counters and acquire/release publication. Capacity must be a power of two and at least two frames.

## Planned process boundary (not implemented)
Use a named shared-memory region for audio payload and a separate control channel for lifecycle and metadata. No mutex waits, message parsing, logging, or handle creation may occur in a VST3 process callback.

The shared-memory header will need:
- Protocol magic and version.
- Session generation identifier.
- Sample rate, channel count, sample format, and buffer capacity.
- Producer/consumer frame counters with explicit ownership.
- Stream state and heartbeat/connection generation.
- Overrun and underrun counters.

Control messages outside the audio callback cover attach/detach, start/stop, format negotiation, reconfiguration, endpoint invalidation, and service restart. A generation change invalidates stale handles and requires clean re-attachment.

## Latency and sizing
Do not publish a latency figure until measured on the target machine. Buffer capacity is a safety bound, not a latency promise. At 48 kHz, 256 frames represent 5.33 ms and 512 frames represent 10.67 ms per buffer. Actual end-to-end latency also includes endpoint periods, scheduling, plugin block sizes, and host buffering.

## Next gates
1. Run the SPSC unit test on Windows CI.
2. Add shared-memory layout and process lifecycle tests.
3. Integrate WASAPI capture with overrun reporting.
4. Evaluate virtual endpoint drivers and licenses before choosing one.
5. Integrate the official VST3 SDK and test in Studio One.
