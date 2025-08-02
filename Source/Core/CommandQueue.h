#pragma once
#include "Commands.h"

template <size_t Capacity = 64>
class CommandQueue
{
public:
    CommandQueue() = default;

    bool push(const Command& c)
    {
        const auto nextWrite = (writeIndex + 1) % Capacity;
        if (nextWrite == readIndex)
            return false; // full

        buffer[writeIndex] = c;
        writeIndex = nextWrite;
        return true;
    }

    bool pop(Command& out)
    {
        if (readIndex == writeIndex)
            return false; // empty

        out = buffer[readIndex];
        readIndex = (readIndex + 1) % Capacity;
        return true;
    }

    void clear()
    {
        readIndex = writeIndex = 0;
    }

    bool isEmpty() const noexcept { return readIndex == writeIndex; }
    bool isFull()  const noexcept { return ((writeIndex + 1) % Capacity) == readIndex; }

private:
    std::array<Command, Capacity> buffer{};
    std::atomic<size_t> writeIndex{ 0 };
    std::atomic<size_t> readIndex{ 0 };
};
