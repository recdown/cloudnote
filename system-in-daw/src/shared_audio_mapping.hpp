#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>

namespace system_in_daw::ipc {

inline constexpr wchar_t kMappingName[] = L"Local\\SystemInDawOut.SystemIn.v1";
inline constexpr std::uint32_t kMagic = 0x53494E31; // "SIN1"
inline constexpr std::uint32_t kVersion = 1;
inline constexpr std::uint32_t kSampleRate = 48000;
inline constexpr std::uint32_t kChannels = 2;
inline constexpr std::uint32_t kCapacityFrames = 16384;

struct alignas(64) SharedAudioHeader {
    std::uint32_t magic;
    std::uint32_t version;
    std::uint32_t sampleRate;
    std::uint32_t channels;
    std::uint32_t capacityFrames;
    std::uint32_t reserved[11];
    alignas(64) volatile LONG64 writeFrame;
    alignas(64) volatile LONG64 readFrame;
    alignas(64) volatile LONG64 overrunFrames;
    alignas(64) volatile LONG64 underrunFrames;
};

inline constexpr std::size_t kMappingBytes =
    sizeof(SharedAudioHeader) +
    static_cast<std::size_t>(kCapacityFrames) * kChannels * sizeof(float);

class SharedAudioReader final {
public:
    SharedAudioReader() = default;
    ~SharedAudioReader() { close(); }
    SharedAudioReader(const SharedAudioReader&) = delete;
    SharedAudioReader& operator=(const SharedAudioReader&) = delete;

    bool open() noexcept {
        if (view_) return true;
        HANDLE mapping = OpenFileMappingW(FILE_MAP_READ | FILE_MAP_WRITE, FALSE, kMappingName);
        if (!mapping) return false;
        void* view = MapViewOfFile(mapping, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, kMappingBytes);
        if (!view) {
            CloseHandle(mapping);
            return false;
        }
        auto* header = static_cast<SharedAudioHeader*>(view);
        if (header->magic != kMagic || header->version != kVersion ||
            header->sampleRate != kSampleRate || header->channels != kChannels ||
            header->capacityFrames != kCapacityFrames) {
            UnmapViewOfFile(view);
            CloseHandle(mapping);
            return false;
        }
        mapping_ = mapping;
        view_ = view;
        header_ = header;
        samples_ = reinterpret_cast<float*>(static_cast<std::byte*>(view) + sizeof(SharedAudioHeader));
        return true;
    }

    void close() noexcept {
        if (view_) UnmapViewOfFile(view_);
        if (mapping_) CloseHandle(mapping_);
        mapping_ = nullptr;
        view_ = nullptr;
        header_ = nullptr;
        samples_ = nullptr;
    }

    bool connected() const noexcept { return header_ != nullptr; }

    // Called only by the single SYSTEM IN plug-in consumer on the audio thread.
    // Copies available frames without waiting, allocating, or calling OS APIs.
    std::uint32_t read(float* destination, std::uint32_t requestedFrames) noexcept {
        if (!header_ || !destination || requestedFrames == 0) return 0;
        const auto write = static_cast<std::uint64_t>(
            InterlockedCompareExchange64(&header_->writeFrame, 0, 0));
        const auto read = static_cast<std::uint64_t>(
            InterlockedCompareExchange64(&header_->readFrame, 0, 0));
        const auto available64 = write - read;
        const auto available = static_cast<std::uint32_t>(
            std::min<std::uint64_t>(available64, kCapacityFrames));
        const auto count = std::min(requestedFrames, available);
        const auto offset = static_cast<std::uint32_t>(read) & (kCapacityFrames - 1);
        const auto first = std::min(count, kCapacityFrames - offset);
        const auto firstSamples = static_cast<std::size_t>(first) * kChannels;
        const auto totalSamples = static_cast<std::size_t>(count) * kChannels;
        const auto start = static_cast<std::size_t>(offset) * kChannels;
        std::memcpy(destination, samples_ + start, firstSamples * sizeof(float));
        if (totalSamples > firstSamples) {
            std::memcpy(destination + firstSamples, samples_,
                        (totalSamples - firstSamples) * sizeof(float));
        }
        InterlockedExchange64(&header_->readFrame, static_cast<LONG64>(read + count));
        if (count < requestedFrames)
            InterlockedExchangeAdd64(&header_->underrunFrames, static_cast<LONG64>(requestedFrames - count));
        return count;
    }

private:
    HANDLE mapping_ {nullptr};
    void* view_ {nullptr};
    SharedAudioHeader* header_ {nullptr};
    float* samples_ {nullptr};
};

class SharedAudioWriter final {
public:
    SharedAudioWriter() = default;
    ~SharedAudioWriter() { close(); }
    SharedAudioWriter(const SharedAudioWriter&) = delete;
    SharedAudioWriter& operator=(const SharedAudioWriter&) = delete;

    bool create() noexcept {
        if (view_) return true;
        HANDLE mapping = CreateFileMappingW(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE,
            static_cast<DWORD>((kMappingBytes >> 32) & 0xffffffffu),
            static_cast<DWORD>(kMappingBytes & 0xffffffffu), kMappingName);
        if (!mapping) return false;
        void* view = MapViewOfFile(mapping, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, kMappingBytes);
        if (!view) {
            CloseHandle(mapping);
            return false;
        }
        auto* header = static_cast<SharedAudioHeader*>(view);
        // This mapping has a single service owner. Initialize its protocol before
        // publishing the magic value so readers never accept a partial header.
        std::memset(view, 0, kMappingBytes);
        header->version = kVersion;
        header->sampleRate = kSampleRate;
        header->channels = kChannels;
        header->capacityFrames = kCapacityFrames;
        InterlockedExchange64(&header->writeFrame, 0);
        InterlockedExchange64(&header->readFrame, 0);
        InterlockedExchange64(&header->overrunFrames, 0);
        InterlockedExchange64(&header->underrunFrames, 0);
        MemoryBarrier();
        header->magic = kMagic;
        mapping_ = mapping;
        view_ = view;
        header_ = header;
        samples_ = reinterpret_cast<float*>(static_cast<std::byte*>(view) + sizeof(SharedAudioHeader));
        return true;
    }

    void close() noexcept {
        if (view_) UnmapViewOfFile(view_);
        if (mapping_) CloseHandle(mapping_);
        mapping_ = nullptr;
        view_ = nullptr;
        header_ = nullptr;
        samples_ = nullptr;
    }

    // Single producer: called by the WASAPI capture thread.
    std::uint32_t write(const float* source, std::uint32_t frames) noexcept {
        if (!header_ || !source || frames == 0) return 0;
        const auto write = static_cast<std::uint64_t>(
            InterlockedCompareExchange64(&header_->writeFrame, 0, 0));
        const auto read = static_cast<std::uint64_t>(
            InterlockedCompareExchange64(&header_->readFrame, 0, 0));
        const auto used = write - read;
        const auto freeFrames = used >= kCapacityFrames ? 0u :
            kCapacityFrames - static_cast<std::uint32_t>(used);
        const auto count = std::min(frames, freeFrames);
        const auto offset = static_cast<std::uint32_t>(write) & (kCapacityFrames - 1);
        const auto first = std::min(count, kCapacityFrames - offset);
        const auto firstSamples = static_cast<std::size_t>(first) * kChannels;
        const auto totalSamples = static_cast<std::size_t>(count) * kChannels;
        const auto start = static_cast<std::size_t>(offset) * kChannels;
        std::memcpy(samples_ + start, source, firstSamples * sizeof(float));
        if (totalSamples > firstSamples) {
            std::memcpy(samples_, source + firstSamples,
                        (totalSamples - firstSamples) * sizeof(float));
        }
        MemoryBarrier();
        InterlockedExchange64(&header_->writeFrame, static_cast<LONG64>(write + count));
        if (count < frames)
            InterlockedExchangeAdd64(&header_->overrunFrames, static_cast<LONG64>(frames - count));
        return count;
    }

private:
    HANDLE mapping_ {nullptr};
    void* view_ {nullptr};
    SharedAudioHeader* header_ {nullptr};
    float* samples_ {nullptr};
};

} // namespace system_in_daw::ipc
