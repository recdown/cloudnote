#pragma once
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <stdexcept>

namespace system_in_daw {

// Bounded single-producer/single-consumer queue of interleaved float frames.
// Construct before starting callbacks. read/write do not allocate, lock, or call OS APIs.
// Exactly one thread may call write(), and exactly one other thread may call read().
class AudioRingBuffer final {
public:
    AudioRingBuffer(std::uint32_t capacityFrames, std::uint32_t channels)
        : capacityFrames_(capacityFrames), mask_(capacityFrames - 1),
          channels_(channels),
          samples_(new float[static_cast<std::size_t>(capacityFrames) * channels]) {
        if (capacityFrames < 2 || (capacityFrames & (capacityFrames - 1)) != 0 || channels == 0)
            throw std::invalid_argument("capacityFrames must be a power of two >= 2; channels must be non-zero");
    }

    AudioRingBuffer(const AudioRingBuffer&) = delete;
    AudioRingBuffer& operator=(const AudioRingBuffer&) = delete;

    // Returns accepted frames. A short write is an overrun the caller must count.
    std::uint32_t write(const float* source, std::uint32_t frames) noexcept {
        if (!source || frames == 0) return 0;
        const auto w = writeFrame_.load(std::memory_order_relaxed);
        const auto r = readFrame_.load(std::memory_order_acquire);
        const auto used = w - r;
        const auto freeFrames = used >= capacityFrames_ ? 0u : capacityFrames_ - static_cast<std::uint32_t>(used);
        const auto accepted = frames < freeFrames ? frames : freeFrames;
        copyIntoRing(w, source, accepted);
        writeFrame_.store(w + accepted, std::memory_order_release);
        return accepted;
    }

    // Returns copied frames. A short read is an underrun; caller supplies silence.
    std::uint32_t read(float* destination, std::uint32_t frames) noexcept {
        if (!destination || frames == 0) return 0;
        const auto r = readFrame_.load(std::memory_order_relaxed);
        const auto w = writeFrame_.load(std::memory_order_acquire);
        const auto available = w - r;
        const auto copied = available < frames ? static_cast<std::uint32_t>(available) : frames;
        copyFromRing(r, destination, copied);
        readFrame_.store(r + copied, std::memory_order_release);
        return copied;
    }

    std::uint32_t channels() const noexcept { return channels_; }
    std::uint32_t capacityFrames() const noexcept { return capacityFrames_; }

    // Diagnostic snapshot only; never use this to coordinate producer/consumer.
    std::uint64_t readableFrames() const noexcept {
        const auto w = writeFrame_.load(std::memory_order_acquire);
        const auto r = readFrame_.load(std::memory_order_acquire);
        const auto used = w - r;
        return used > capacityFrames_ ? capacityFrames_ : used;
    }

private:
    void copyIntoRing(std::uint64_t frame, const float* src, std::uint32_t count) noexcept {
        const auto offset = static_cast<std::uint32_t>(frame) & mask_;
        const auto first = count < capacityFrames_ - offset ? count : capacityFrames_ - offset;
        const auto firstSamples = static_cast<std::size_t>(first) * channels_;
        const auto totalSamples = static_cast<std::size_t>(count) * channels_;
        const auto start = static_cast<std::size_t>(offset) * channels_;
        for (std::size_t i = 0; i < firstSamples; ++i) samples_[start + i] = src[i];
        for (std::size_t i = firstSamples; i < totalSamples; ++i) samples_[i - firstSamples] = src[i];
    }

    void copyFromRing(std::uint64_t frame, float* dst, std::uint32_t count) noexcept {
        const auto offset = static_cast<std::uint32_t>(frame) & mask_;
        const auto first = count < capacityFrames_ - offset ? count : capacityFrames_ - offset;
        const auto firstSamples = static_cast<std::size_t>(first) * channels_;
        const auto totalSamples = static_cast<std::size_t>(count) * channels_;
        const auto start = static_cast<std::size_t>(offset) * channels_;
        for (std::size_t i = 0; i < firstSamples; ++i) dst[i] = samples_[start + i];
        for (std::size_t i = firstSamples; i < totalSamples; ++i) dst[i] = samples_[i - firstSamples];
    }

    const std::uint32_t capacityFrames_;
    const std::uint32_t mask_;
    const std::uint32_t channels_;
    std::unique_ptr<float[]> samples_;
    alignas(64) std::atomic<std::uint64_t> writeFrame_{0};
    alignas(64) std::atomic<std::uint64_t> readFrame_{0};
};
} // namespace system_in_daw
