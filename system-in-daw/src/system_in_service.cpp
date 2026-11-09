#define NOMINMAX
#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <functiondiscoverykeys_devpkey.h>
#include <wrl/client.h>
#include <ksmedia.h>

#include "shared_audio_mapping.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <thread>

using Microsoft::WRL::ComPtr;
using namespace system_in_daw::ipc;

static void Check(HRESULT hr, const char* operation) {
    if (FAILED(hr)) throw std::runtime_error(operation);
}

static float DecodeSample(const BYTE* data, WORD containerBits, bool isFloat,
                          std::size_t sampleIndex) {
    if (isFloat && containerBits == 32) {
        float value = 0.0f;
        std::memcpy(&value, data + sampleIndex * sizeof(float), sizeof(float));
        return std::isfinite(value) ? std::clamp(value, -1.0f, 1.0f) : 0.0f;
    }
    if (!isFloat && containerBits == 16) {
        std::int16_t value = 0;
        std::memcpy(&value, data + sampleIndex * sizeof(value), sizeof(value));
        return static_cast<float>(value) / 32768.0f;
    }
    if (!isFloat && containerBits == 32) {
        std::int32_t value = 0;
        std::memcpy(&value, data + sampleIndex * sizeof(value), sizeof(value));
        return static_cast<float>(static_cast<double>(value) / 2147483648.0);
    }
    return 0.0f;
}

int wmain() {
    try {
        const HRESULT init = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        Check(init, "CoInitializeEx");
        struct CoUninit { ~CoUninit() { CoUninitialize(); } } coUninit;

        SharedAudioWriter writer;
        if (!writer.create()) throw std::runtime_error("Unable to create SYSTEM IN shared audio mapping");

        ComPtr<IMMDeviceEnumerator> enumerator;
        Check(CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
                               IID_PPV_ARGS(&enumerator)), "Create MMDeviceEnumerator");
        ComPtr<IMMDevice> device;
        Check(enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device),
              "Get default playback endpoint");

        ComPtr<IAudioClient> client;
        Check(device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr,
                               reinterpret_cast<void**>(client.GetAddressOf())),
              "Activate IAudioClient");
        WAVEFORMATEX* mix = nullptr;
        Check(client->GetMixFormat(&mix), "GetMixFormat");
        struct FormatFree { WAVEFORMATEX* value; ~FormatFree() { if (value) CoTaskMemFree(value); } } freeFormat{mix};

        if (mix->nSamplesPerSec != kSampleRate || mix->nChannels == 0 ||
            mix->wBitsPerSample != 16 && mix->wBitsPerSample != 32) {
            throw std::runtime_error("Initial SYSTEM IN service requires 48 kHz and 16/32-bit endpoint audio");
        }

        const auto* ext = mix->wFormatTag == WAVE_FORMAT_EXTENSIBLE
            ? reinterpret_cast<const WAVEFORMATEXTENSIBLE*>(mix) : nullptr;
        const bool isFloat = mix->wFormatTag == WAVE_FORMAT_IEEE_FLOAT ||
            (ext && ext->SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT);
        if (isFloat && mix->wBitsPerSample != 32)
            throw std::runtime_error("Unsupported floating-point endpoint format");

        Check(client->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_LOOPBACK,
                                 1000000, 0, mix, nullptr), "Initialize loopback");
        ComPtr<IAudioCaptureClient> capture;
        Check(client->GetService(IID_PPV_ARGS(&capture)), "Get IAudioCaptureClient");
        Check(client->Start(), "Start loopback capture");
        struct Stop { IAudioClient* client; ~Stop() { if (client) client->Stop(); } } stop{client.Get()};

        std::array<float, 16384> stereo {};
        std::uint64_t framesPublished = 0;
        std::uint64_t framesDropped = 0;
        std::cout << "SYSTEM IN bridge capture active at 48 kHz. Close this process to stop.\n";

        for (;;) {
            UINT32 packetFrames = 0;
            Check(capture->GetNextPacketSize(&packetFrames), "GetNextPacketSize");
            while (packetFrames > 0) {
                BYTE* data = nullptr;
                UINT32 frames = 0;
                DWORD flags = 0;
                Check(capture->GetBuffer(&data, &frames, &flags, nullptr, nullptr), "GetBuffer");
                if (frames > stereo.size() / 2) {
                    capture->ReleaseBuffer(frames);
                    throw std::runtime_error("WASAPI packet exceeded the preallocated conversion buffer");
                }

                if (flags & AUDCLNT_BUFFERFLAGS_SILENT) {
                    std::fill_n(stereo.data(), static_cast<std::size_t>(frames) * 2, 0.0f);
                } else {
                    for (UINT32 frame = 0; frame < frames; ++frame) {
                        const auto left = DecodeSample(data, mix->wBitsPerSample, isFloat,
                                                       static_cast<std::size_t>(frame) * mix->nChannels);
                        const auto right = mix->nChannels > 1
                            ? DecodeSample(data, mix->wBitsPerSample, isFloat,
                                           static_cast<std::size_t>(frame) * mix->nChannels + 1)
                            : left;
                        stereo[static_cast<std::size_t>(frame) * 2] = left;
                        stereo[static_cast<std::size_t>(frame) * 2 + 1] = right;
                    }
                }
                const auto written = writer.write(stereo.data(), frames);
                framesPublished += written;
                framesDropped += frames - written;
                Check(capture->ReleaseBuffer(frames), "ReleaseBuffer");
                Check(capture->GetNextPacketSize(&packetFrames), "GetNextPacketSize");
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
        }
    } catch (const std::exception& error) {
        std::cerr << "SYSTEM IN service error: " << error.what() << "\n";
        return 1;
    }
}
