#define NOMINMAX
#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <functiondiscoverykeys_devpkey.h>
#include <wrl/client.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <cmath>
#include <cstdint>
#include <algorithm>
#include <chrono>
#include <thread>
#include <cstring>
#include <stdexcept>
#include <ksmedia.h>

using Microsoft::WRL::ComPtr;

static void Check(HRESULT hr, const char* what) {
    if (FAILED(hr)) {
        std::cerr << what << " failed: 0x" << std::hex
                  << static_cast<unsigned long>(hr) << std::dec << "\n";
        throw std::runtime_error(what);
    }
}

static std::wstring DeviceName(IMMDevice* device) {
    ComPtr<IPropertyStore> props;
    if (FAILED(device->OpenPropertyStore(STGM_READ, &props))) return L"(name unavailable)";
    PROPVARIANT value;
    PropVariantInit(&value);
    std::wstring name = L"(name unavailable)";
    if (SUCCEEDED(props->GetValue(PKEY_Device_FriendlyName, &value)) &&
        value.vt == VT_LPWSTR && value.pwszVal) {
        name = value.pwszVal;
    }
    PropVariantClear(&value);
    return name;
}

static float ReadSample(const BYTE* data, WORD bits, bool isFloat, size_t index) {
    if (isFloat && bits == 32) {
        float v;
        std::memcpy(&v, data + index * sizeof(float), sizeof(float));
        return std::isfinite(v) ? std::clamp(v, -1.0f, 1.0f) : 0.0f;
    }
    if (!isFloat && bits == 16) {
        int16_t v;
        std::memcpy(&v, data + index * sizeof(int16_t), sizeof(int16_t));
        return static_cast<float>(v) / 32768.0f;
    }
    if (!isFloat && bits == 32) {
        int32_t v;
        std::memcpy(&v, data + index * sizeof(int32_t), sizeof(int32_t));
        return static_cast<float>(static_cast<double>(v) / 2147483648.0);
    }
    return 0.0f;
}

int wmain(int argc, wchar_t** argv) {
    const unsigned durationSeconds =
        argc > 1 ? static_cast<unsigned>(std::max(0, _wtoi(argv[1]))) : 0;

    try {
        const HRESULT initHr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        Check(initHr, "CoInitializeEx");
        struct CoUninit { ~CoUninit() { CoUninitialize(); } } coUninit;

        ComPtr<IMMDeviceEnumerator> enumerator;
        Check(CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
                               IID_PPV_ARGS(&enumerator)), "Create MMDeviceEnumerator");

        ComPtr<IMMDevice> device;
        Check(enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device),
              "Get default render endpoint");

        std::wcout << L"SYSTEM IN / WASAPI Loopback Probe\nDevice: "
                   << DeviceName(device.Get()) << L"\n";

        ComPtr<IAudioClient> client;
        Check(device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr,
                               reinterpret_cast<void**>(client.GetAddressOf())),
              "Activate IAudioClient");

        WAVEFORMATEX* mix = nullptr;
        Check(client->GetMixFormat(&mix), "GetMixFormat");
        struct FormatFree {
            WAVEFORMATEX* p;
            ~FormatFree() { if (p) CoTaskMemFree(p); }
        } formatFree{mix};

        const bool extensible =
            mix->wFormatTag == WAVE_FORMAT_EXTENSIBLE &&
            mix->cbSize >= sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX);
        const auto* ext = extensible
            ? reinterpret_cast<const WAVEFORMATEXTENSIBLE*>(mix) : nullptr;
        const bool isFloat =
            mix->wFormatTag == WAVE_FORMAT_IEEE_FLOAT ||
            (ext && ext->SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT);
        const WORD bits = extensible ? ext->Samples.wValidBitsPerSample : mix->wBitsPerSample;

        std::wcout << L"Format: " << mix->nSamplesPerSec << L" Hz, "
                   << mix->nChannels << L" ch, " << mix->wBitsPerSample
                   << L" bit, " << (isFloat ? L"float" : L"integer") << L"\n";

        Check(client->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_LOOPBACK,
                                 1000000, 0, mix, nullptr),
              "Initialize loopback client");

        ComPtr<IAudioCaptureClient> capture;
        Check(client->GetService(IID_PPV_ARGS(&capture)), "Get IAudioCaptureClient");
        Check(client->Start(), "Start loopback capture");
        struct Stop { IAudioClient* p; ~Stop() { if (p) p->Stop(); } } stop{client.Get()};

        std::wcout << L"Capturing. Press Ctrl+C to stop";
        if (durationSeconds) std::wcout << L" (" << durationSeconds << L" seconds)";
        std::wcout << L"...\n";

        const auto start = std::chrono::steady_clock::now();
        auto lastReport = start;
        double sumSquares = 0.0;
        uint64_t samples = 0;
        float peak = 0.0f;
        uint64_t packets = 0;
        uint64_t silentPackets = 0;

        while (!durationSeconds ||
               std::chrono::steady_clock::now() - start < std::chrono::seconds(durationSeconds)) {
            UINT32 packetFrames = 0;
            Check(capture->GetNextPacketSize(&packetFrames), "GetNextPacketSize");
            while (packetFrames) {
                BYTE* data = nullptr;
                UINT32 frames = 0;
                DWORD flags = 0;
                Check(capture->GetBuffer(&data, &frames, &flags, nullptr, nullptr),
                      "GetBuffer");

                if (flags & AUDCLNT_BUFFERFLAGS_SILENT) {
                    ++silentPackets;
                } else {
                    const size_t count = static_cast<size_t>(frames) * mix->nChannels;
                    for (size_t i = 0; i < count; ++i) {
                        const float sample = ReadSample(data, bits, isFloat, i);
                        peak = std::max(peak, std::abs(sample));
                        sumSquares += static_cast<double>(sample) * sample;
                    }
                    samples += count;
                }

                ++packets;
                Check(capture->ReleaseBuffer(frames), "ReleaseBuffer");
                Check(capture->GetNextPacketSize(&packetFrames), "GetNextPacketSize");
            }

            const auto now = std::chrono::steady_clock::now();
            if (now - lastReport >= std::chrono::milliseconds(500)) {
                const double rms = samples
                    ? std::sqrt(sumSquares / static_cast<double>(samples)) : 0.0;
                std::wcout << L"\rPeak " << std::fixed << std::setprecision(4) << peak
                           << L" | RMS " << rms << L" | packets " << packets
                           << L" | silent " << silentPackets << L"     " << std::flush;
                peak = 0.0f;
                sumSquares = 0.0;
                samples = 0;
                lastReport = now;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(3));
        }

        std::wcout << L"\nCapture stopped.\n";
        return 0;
    } catch (const std::exception&) {
        return 1;
    }
}
