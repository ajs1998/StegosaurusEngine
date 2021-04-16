#pragma once

#include "Core.h"

namespace Steg {

    struct RNG {

    public:

        RNG() = delete;

        RNG(uint32_t seed, uint32_t upperBound);

        RNG(uint32_t seed);

        uint32_t Next();

    private:

        std::default_random_engine generator;

        std::uniform_int_distribution<uint32_t> rand;

    };

}
