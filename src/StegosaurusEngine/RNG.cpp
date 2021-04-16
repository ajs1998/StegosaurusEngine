#include "include/RNG.h"

using namespace Steg;

RNG::RNG(uint32_t seed, uint32_t upperBound) : generator(seed), rand(0, upperBound) {}

RNG::RNG(uint32_t seed) : generator(seed), rand() {}

uint32_t RNG::Next() {
    return rand(generator);
}
