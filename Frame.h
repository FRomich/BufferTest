#pragma once
#include <cstdint>
#include <vector>

struct Frame {
    /// Numbers frame
    uint32_t frameNumber = 0;
    uint32_t trigNumber = 0;

    uint32_t width = 0;
    uint32_t height = 0;

    uint64_t deviceTimeStamp = 0;
    int64_t  hostTimeStamp = 0;

    double exposureTime = 0.0;

    uint64_t dataSize = 0;
    std::vector<uint8_t> data;
};