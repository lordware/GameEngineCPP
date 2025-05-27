#pragma once
#include "IAllocator.h"
#include <memory>

namespace GameEngine::memory {

class StackAllocator : public IAllocator {
public:
    using Marker = size_t;
    
    explicit StackAllocator(size_t capacity);
    ~StackAllocator() override;
    
    void* Allocate(size_t size, size_t alignment = sizeof(void*)) override;
    void Deallocate(void* ptr) override;
    
    size_t GetUsedMemory() const override { return currentOffset; }
    size_t GetTotalMemory() const override { return capacity; }
    
    Marker GetMarker() const { return currentOffset; }
    void FreeToMarker(Marker marker);
    void Reset();

private:
    uint8_t* memory;
    size_t capacity;
    size_t currentOffset;
};

}
