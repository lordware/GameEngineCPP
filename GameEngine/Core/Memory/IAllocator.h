#pragma once
#include <cstddef>
#include <cstdint>

namespace GameEngine::memory {

class IAllocator {
public:
    virtual ~IAllocator() = default;
    
    virtual void* Allocate(size_t size, size_t alignment = sizeof(void*)) = 0;
    virtual void Deallocate(void* ptr) = 0;
    virtual size_t GetUsedMemory() const = 0;
    virtual size_t GetTotalMemory() const = 0;
    
protected:
    static size_t AlignAddress(size_t address, size_t alignment) {
        return (address + alignment - 1) & ~(alignment - 1);
    }
    
    static void* AlignPointer(void* ptr, size_t alignment) {
        uintptr_t address = reinterpret_cast<uintptr_t>(ptr);
        uintptr_t aligned = AlignAddress(address, alignment);
        return reinterpret_cast<void*>(aligned);
    }
};

}
