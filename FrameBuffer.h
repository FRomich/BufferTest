#pragma once

#include "Frame.h"
#include <mutex>
#include <deque>

class FrameBuffer
{
public:
	explicit FrameBuffer(size_t capacity = 100);

	bool push(Frame frame);
	bool tryPop(Frame& frame);

	void clear();

	size_t size() const;
	size_t capacity() const;

	void setCapacity(size_t capacity);

	uint64_t pushed() const;
	uint64_t dropped() const;
	uint64_t popped() const;

private:
	/// <summary>
	/// Protects the queue and counters.
	/// </summary>
	mutable std::mutex m_mutex;
	/// <summary>
	/// Frame queue.
	/// </summary>
	std::deque<Frame> m_queue;
	/// <summary>
	/// Maximum queue capacity.
	/// </summary>
	size_t m_capacity;

	uint64_t m_pushed = 0;
	uint64_t m_dropped = 0;
	uint64_t m_popped = 0;
};
