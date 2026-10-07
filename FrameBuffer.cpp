#include "FrameBuffer.h"

FrameBuffer::FrameBuffer(size_t capacity)
    : m_capacity(capacity)
{
}

bool FrameBuffer::push(Frame frame)
{
    std::lock_guard lock(m_mutex);

    ++m_pushed;

    if (m_capacity == 0)
    {
        ++m_dropped;
        return false;
    }

    if (m_queue.size() >= m_capacity)
    {
        // Buffer is full: drop the oldest frame.
        m_queue.pop_front();
        ++m_dropped;
    }

    m_queue.push_back(std::move(frame));

    return true;
}

bool FrameBuffer::tryPop(Frame& frame)
{
    std::lock_guard lock(m_mutex);

    if (m_queue.empty())
        return false;

    frame = std::move(m_queue.front());
    m_queue.pop_front();

    ++m_popped;

    return true;
}

void FrameBuffer::clear()
{
    std::lock_guard lock(m_mutex);

    m_queue.clear();
}

size_t FrameBuffer::size() const
{
    std::lock_guard lock(m_mutex);

    return m_queue.size();
}

size_t FrameBuffer::capacity() const
{
    std::lock_guard lock(m_mutex);

    return m_capacity;
}

void FrameBuffer::setCapacity(size_t capacity)
{
    std::lock_guard lock(m_mutex);

    m_capacity = capacity;

    while (m_queue.size() > m_capacity)
    {
        m_queue.pop_front();
        ++m_dropped;
    }
}

uint64_t FrameBuffer::pushed() const
{
    std::lock_guard lock(m_mutex);
    return m_pushed;
}

uint64_t FrameBuffer::dropped() const
{
    std::lock_guard lock(m_mutex);
    return m_dropped;
}

uint64_t FrameBuffer::popped() const
{
    std::lock_guard lock(m_mutex);
    return m_popped;
}