#include "FrameProcessor.h"

#include <chrono>
#include <fstream>
#include <iostream>

FrameProcessor::FrameProcessor(
    FrameBuffer& buffer,
    DataBase& db,
    int sessionId,
    int cameraId,
    std::filesystem::path outputDirectory)
    : m_buffer(buffer)
    , m_database(db)
    , m_sessionId(sessionId)
    , m_cameraId(cameraId)
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
    if (frame.width == 0 || frame.height == 0)
    {
        std::cerr << "Invalid dimensions: "
            << frame.width << "x" << frame.height << '\n';
        return false;
    }

    const size_t width = frame.width;
    const size_t height = frame.height;

    if (frame.data.size() < width * height)
    {
        std::cerr << "Invalid Mono8 frame data\n";
        return false;
    }

    const auto timestamp = std::to_string(frame.deviceTimeStamp);

    const auto timestampPart =
        timestamp.substr(timestamp.size() - 13, 6);

    const auto filename =
        m_outputDirectory /
        std::format(
            "{}_{:05}_{:05}.bmp",
            timestampPart,
            frame.frameNumber,
            frame.trigNumber
        );

   // BMP требует выравнивания каждой строки по 4 байтам.
    const uint32_t rowSize = (width + 3u) & ~3u;
    const uint32_t paddedImageSize = rowSize * height;
    const uint32_t paletteSize = 256 * 4;
    const uint32_t pixelOffset = 14 + 40 + paletteSize;
    const uint32_t fileSize = pixelOffset + paddedImageSize;

    std::ofstream file(filename, std::ios::binary);

    if (!file)
    {
        std::cerr << "Cannot create BMP: "
            << filename.string() << '\n';
        return false;
    }

    auto write16 = [&file](uint16_t value)
        {
            const char bytes[] = {
                static_cast<char>(value),
                static_cast<char>(value >> 8)
            };
            file.write(bytes, sizeof(bytes));
        };

    auto write32 = [&file](uint32_t value)
        {
            const char bytes[] = {
                static_cast<char>(value),
                static_cast<char>(value >> 8),
                static_cast<char>(value >> 16),
                static_cast<char>(value >> 24)
            };
            file.write(bytes, sizeof(bytes));
        };

    // BITMAPFILEHEADER — 14 байт.
    file.put('B');
    file.put('M');
    write32(fileSize);
    write16(0);
    write16(0);
    write32(pixelOffset);

    // BITMAPINFOHEADER — 40 байт.
    write32(40);
    write32(width);
    write32(height);       // Положительная высота: строки снизу вверх.
    write16(1);            // Planes
    write16(8);            // 8 бит на пиксель
    write32(0);            // BI_RGB
    write32(paddedImageSize);
    write32(2835);         // 72 DPI
    write32(2835);
    write32(256);          // Цветовых элементов палитры
    write32(256);

    // Палитра: B, G, R, Reserved.
    for (uint32_t i = 0; i < 256; ++i)
    {
        file.put(static_cast<char>(i));
        file.put(static_cast<char>(i));
        file.put(static_cast<char>(i));
        file.put(0);
    }

    // BMP хранит строки снизу вверх.
    std::vector<char> row(rowSize, 0);

    for (uint32_t y = height; y > 0; --y)
    {
        const size_t sourceOffset =
            static_cast<size_t>(y - 1) * width;

        for (uint32_t x = 0; x < width; ++x)
            row[x] = static_cast<char>(frame.data[sourceOffset + x]);

        file.write(row.data(), row.size());
    }

    file.close();

    if (!file)
    {
        std::cerr << "Error writing BMP: "
            << filename.string() << '\n';
        return false;
    }

    if (!m_database.insertFrame(
        m_sessionId,
        m_cameraId,
        frame,
        std::filesystem::absolute(filename).string()))
    {
        std::cerr << "Database error: "
            << m_database.lastError() << '\n';
        return false;
    }

    return true;
}