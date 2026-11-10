#include "cids.h"
#include "shared_audio_mapping.hpp"
#include "public.sdk/source/vst/vstaudioeffect.h"
#include <algorithm>
#include <array>

namespace SystemInDaw {
namespace ipc = system_in_daw::ipc;
using namespace Steinberg;
using namespace Steinberg::Vst;

class SystemInProcessor final : public AudioEffect {
public:
    SystemInProcessor() {
        // Associate the component with its edit controller in the VST3 factory metadata.
        setControllerClass(ControllerUID);
    }

    tresult PLUGIN_API initialize(FUnknown* context) SMTG_OVERRIDE {
        auto result = AudioEffect::initialize(context);
        if (result == kResultOk) {
            addAudioOutput(STR("SYSTEM IN"), SpeakerArr::kStereo);
        }
        return result;
    }

    tresult PLUGIN_API terminate() SMTG_OVERRIDE {
        reader_.close();
        return AudioEffect::terminate();
    }

    tresult PLUGIN_API setupProcessing(ProcessSetup& setup) SMTG_OVERRIDE {
        if (setup.sampleRate != static_cast<SampleRate>(ipc::kSampleRate))
            return kResultFalse; // Initial bridge format is fixed at 48 kHz.
        const auto result = AudioEffect::setupProcessing(setup);
        // Opening the mapping is deliberately outside process(), never on the audio thread.
        reader_.open();
        return result;
    }

    tresult PLUGIN_API setActive(TBool state) SMTG_OVERRIDE {
        if (!state) reader_.close();
        else reader_.open();
        return AudioEffect::setActive(state);
    }

    tresult PLUGIN_API canProcessSampleSize(int32 size) SMTG_OVERRIDE {
        return size == kSample32 || size == kSample64 ? kResultTrue : kResultFalse;
    }

    tresult PLUGIN_API process(ProcessData& data) SMTG_OVERRIDE {
        if (data.numOutputs < 1 || !data.outputs || data.outputs[0].numChannels < 2)
            return kResultFalse;
        if (data.numSamples < 0 || static_cast<std::size_t>(data.numSamples) > scratch_.size() / 2)
            return kResultFalse;

        auto** out = data.outputs[0].channelBuffers32;
        if (data.symbolicSampleSize == kSample32) {
            if (!out || !out[0] || !out[1]) return kResultFalse;
            std::fill_n(out[0], data.numSamples, 0.0f);
            std::fill_n(out[1], data.numSamples, 0.0f);
            if (reader_.connected()) {
                const auto copied = reader_.read(scratch_.data(), static_cast<uint32>(data.numSamples));
                for (uint32 i = 0; i < copied; ++i) {
                    out[0][i] = scratch_[static_cast<std::size_t>(i) * 2];
                    out[1][i] = scratch_[static_cast<std::size_t>(i) * 2 + 1];
                }
            }
        } else {
            auto** out64 = data.outputs[0].channelBuffers64;
            if (!out64 || !out64[0] || !out64[1]) return kResultFalse;
            std::fill_n(out64[0], data.numSamples, 0.0);
            std::fill_n(out64[1], data.numSamples, 0.0);
            if (reader_.connected()) {
                const auto copied = reader_.read(scratch_.data(), static_cast<uint32>(data.numSamples));
                for (uint32 i = 0; i < copied; ++i) {
                    out64[0][i] = scratch_[static_cast<std::size_t>(i) * 2];
                    out64[1][i] = scratch_[static_cast<std::size_t>(i) * 2 + 1];
                }
            }
        }
        return kResultOk;
    }

private:
    ipc::SharedAudioReader reader_;
    // VST process block sizes are bounded by the host setup. This fixed scratch buffer
    // avoids allocation in process(); oversized blocks are rendered as silence.
    std::array<float, 32768> scratch_ {};
};

FUnknown* createProcessor(void*) {
    return static_cast<IAudioProcessor*>(new SystemInProcessor);
}
}
