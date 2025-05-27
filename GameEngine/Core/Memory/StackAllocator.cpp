#include "StackAllocator.h"
#include <cassert>
#include <algorithm>

namespace GameEngine::memory {

StackAllocator::StackAllocator(size_t capacity)
    : memory(new uint8_t[capacity])
    , capacity(capacity)
    , currentOffset(0) {
}

StackAllocator::~StackAllocator() {
    delete[] memory;
}

void* StackAllocator::Allocate(size_t size, size_t alignment) {
    assert(size > 0);
    assert((alignment & (alignment - 1)) == 0);
    
    size_t currentAddress = reinterpret_cast<size_t>(memory + currentOffset);
    size_t alignedAddress = AlignAddress(currentAddress, alignment);
    size_t alignmentOffset = alignedAddress - currentAddress;
    
    if (currentOffset + alignmentOffset + size > capacity) {
        return nullptr;
    }
    
    currentOffset += alignmentOffset + size;
    return reinterpret_cast<void*>(alignedAddress);
}

void StackAllocator::Deallocate(void* ptr) {
}

void StackAllocator::FreeToMarker(Marker marker) {
    assert(marker <= currentOffset);
    currentOffset = marker;
}

void StackAllocator::Reset() {
    currentOffset = 0;
}

}
