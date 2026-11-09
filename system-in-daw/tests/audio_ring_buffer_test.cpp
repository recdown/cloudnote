#include "audio_ring_buffer.hpp"
#include <iostream>
#include <stdexcept>
using system_in_daw::AudioRingBuffer;

static void require(bool ok, const char* message) {
    if (!ok) throw std::runtime_error(message);
}

int main() {
    try {
        AudioRingBuffer ring(8, 2);
        const float a[] = {1,10, 2,20, 3,30, 4,40, 5,50, 6,60};
        require(ring.write(a, 6) == 6, "initial write");
        float first[8]{};
        require(ring.read(first, 4) == 4, "initial read");
        for (int i = 0; i < 8; ++i) require(first[i] == a[i], "initial data mismatch");

        const float b[] = {7,70, 8,80, 9,90, 10,100, 11,110, 12,120};
        require(ring.write(b, 6) == 6, "wrapped write");
        require(ring.readableFrames() == 8, "full capacity");
        require(ring.write(b, 1) == 0, "must reject write when full");

        float out[16]{};
        require(ring.read(out, 8) == 8, "full read");
        const float expected[] = {5,50, 6,60, 7,70, 8,80, 9,90, 10,100};
        for (int i = 0; i < 12; ++i) require(out[i] == expected[i], "wrap data mismatch");
        require(ring.read(out, 1) == 0, "empty read");
        std::cout << "AudioRingBuffer tests passed\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "AudioRingBuffer test failed: " << e.what() << '\n';
        return 1;
    }
}
