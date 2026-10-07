#pragma once

#include "FrameBuffer.h"

#include <atomic>
#include <cstdint>
#include <filesystem>
#include <thread>

class FrameProcessor
{
public:
    FrameProcessor(
        FrameBuffer& buffer,
        std::filesystem::path outputDirectory);

    ~FrameProcessor();

    FrameProcessor(const FrameProcessor&) = delete;
    FrameProcessor& operator=(const FrameProcessor&) = delete;

    bool start();
    void stop();

    bool isRunning() const;

    uint64_t processed() const;
    uint64_t failed() const;

private:
    void process();

    bool saveFrame(const Frame& frame);

private:
    FrameBuffer& m_buffer;
    std::filesystem::path m_outputDirectory;

    std::thread m_thread;
    std::atomic<bool> m_running{ false };

    std::atomic<uint64_t> m_processed{ 0 };
    std::atomic<uint64_t> m_failed{ 0 };
};