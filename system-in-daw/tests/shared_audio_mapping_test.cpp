#include "shared_audio_mapping.hpp"

#include <array>
#include <cmath>
#include <iostream>

int main() {
    using namespace system_in_daw::ipc;

    SharedAudioWriter writer;
    if (!writer.create()) {
        std::cerr << "SharedAudioWriter::create failed\n";
        return 1;
    }
    SharedAudioReader reader;
    if (!reader.open()) {
        std::cerr << "SharedAudioReader::open failed\n";
        return 1;
    }

    std::array<float, 12> input {
        0.1f, -0.1f, 0.2f, -0.2f, 0.3f, -0.3f,
        0.4f, -0.4f, 0.5f, -0.5f, 0.6f, -0.6f
    };
    if (writer.write(input.data(), 6) != 6) {
        std::cerr << "Expected six frames to be written\n";
        return 1;
    }

    std::array<float, 12> output {};
    if (reader.read(output.data(), 6) != 6) {
        std::cerr << "Expected six frames to be read\n";
        return 1;
    }
    for (std::size_t i = 0; i < input.size(); ++i) {
        if (std::abs(input[i] - output[i]) > 1.0e-7f) {
            std::cerr << "Shared-memory sample mismatch at " << i << "\n";
            return 1;
        }
    }

    // A read from an empty queue must be a non-blocking short read.
    if (reader.read(output.data(), 1) != 0) {
        std::cerr << "Expected empty queue to return zero frames\n";
        return 1;
    }

    std::cout << "SharedAudioMappingTest passed\n";
    return 0;
}
