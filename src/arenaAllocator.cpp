#include "ArenaAllocator.hpp"

ArenaAllocator::ArenaAllocator(size_t bytes): m_Size(bytes) {
    m_Buffer = static_cast<std::byte*>(malloc(m_Size));
    m_Offset = m_Buffer;
}

ArenaAllocator::~ArenaAllocator() {
    free(m_Buffer);
}