#include "FrameProcessor.h"

#include <chrono>
#include <fstream>
#include <iostream>

FrameProcessor::FrameProcessor(
    FrameBuffer& buffer,
    std::filesystem::path outputDirectory)
    : m_buffer(buffer)
    , m_outputDirectory(std::move(outputDirectory))
{
}

FrameProcessor::~FrameProcessor()
{
    stop();
}

bool FrameProcessor::start()
{
    if (m_running.exchange(true))
        return false;

    try
    {
        std::filesystem::create_directories(m_outputDirectory);
    }
    catch (const std::filesystem::filesystem_error& e)
    {
        std::cerr
            << "Failed to create output directory: "
            << e.what()
            << '\n';

        m_running = false;
        return false;
    }

    m_thread = std::thread(&FrameProcessor::process, this);

    return true;
}

void FrameProcessor::stop()
{
    if (!m_running.exchange(false))
        return;

    if (m_thread.joinable())
        m_thread.join();
}

bool FrameProcessor::isRunning() const
{
    return m_running.load();
}

uint64_t FrameProcessor::processed() const
{
    return m_processed.load();
}

uint64_t FrameProcessor::failed() const
{
    return m_failed.load();
}

void FrameProcessor::process()
{
    while (m_running)
    {
        Frame frame;

        if (!m_buffer.tryPop(frame))
        {
            std::this_thread::sleep_for(
                std::chrono::milliseconds(1));

            continue;
        }

        if (saveFrame(frame))
        {
            ++m_processed;
        }
        else
        {
            ++m_failed;
        }
    }

    // После stop() дочитываем оставшиеся кадры.
    Frame frame;

    while (m_buffer.tryPop(frame))
    {
        if (saveFrame(frame))
        {
            ++m_processed;
        }
        else
        {
            ++m_failed;
        }
    }
}

bool FrameProcessor::saveFrame(const Frame& frame)
{
    const auto filename =
        m_outputDirectory /
        std::format(
            "{:05}_{:05}_{}.bin",
            frame.frameNumber,
            frame.trigNumber,
            frame.deviceTimeStamp
            //,frame.hostTimeStamp
        );

    std::ofstream file(
        filename,
        std::ios::binary | std::ios::trunc);

    if (!file)
    {
        std::cerr
            << "Failed to open file: "
            << filename
            << '\n';

        return false;
    }

    // Заголовок
    file.write(
        reinterpret_cast<const char*>(&frame.frameNumber),
        sizeof(frame.frameNumber));

    file.write(
        reinterpret_cast<const char*>(&frame.width),
        sizeof(frame.width));

    file.write(
        reinterpret_cast<const char*>(&frame.height),
        sizeof(frame.height));

    file.write(
        reinterpret_cast<const char*>(&frame.deviceTimeStamp),
        sizeof(frame.deviceTimeStamp));

    file.write(
        reinterpret_cast<const char*>(&frame.hostTimeStamp),
        sizeof(frame.hostTimeStamp));

    file.write(
        reinterpret_cast<const char*>(&frame.exposureTime),
        sizeof(frame.exposureTime));

    file.write(
        reinterpret_cast<const char*>(&frame.dataSize),
        sizeof(frame.dataSize));

    // Данные изображения
    if (!frame.data.empty())
    {
        file.write(
            reinterpret_cast<const char*>(frame.data.data()),
            static_cast<std::streamsize>(frame.data.size()));
    }

    if (!file)
    {
        std::cerr
            << "Failed to write file: "
            << filename
            << '\n';

        return false;
    }

    return true;
}