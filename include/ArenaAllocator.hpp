#pragma once
#include <string>


class ArenaAllocator {
public:
    ArenaAllocator(size_t bytes);

    template<typename T>
    T* alloc() {
        void* offset = m_Offset;
        m_Offset += sizeof(T);
        return static_cast<T*>(offset);
    }

    ArenaAllocator(const ArenaAllocator& other) = delete;
    ArenaAllocator& operator=(const ArenaAllocator& other) = delete;

    ~ArenaAllocator();
private:
    size_t m_Size;
    std::byte* m_Buffer;
    std::byte* m_Offset;
};