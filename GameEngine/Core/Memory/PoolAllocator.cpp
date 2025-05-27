#include "PoolAllocator.h"
#include <cassert>
#include <algorithm>

namespace GameEngine::memory {

PoolAllocator::PoolAllocator(size_t blockSize, size_t blockCount)
    : memory(new uint8_t[blockSize * blockCount])
    , freeHead(nullptr)
    , blockSize(std::max(blockSize, sizeof(Block*)))
    , blockCount(blockCount)
    , usedBlocks(0) {
    
    InitializeFreeList();
}

PoolAllocator::~PoolAllocator() {
    delete[] memory;
}

void* PoolAllocator::Allocate(size_t size, size_t alignment) {
    if (freeHead == nullptr) {
        return nullptr;
    }
    
    Block* block = freeHead;
    freeHead = freeHead->next;
    ++usedBlocks;
    
    return block;
}

void PoolAllocator::Deallocate(void* ptr) {
    if (ptr == nullptr) {
        return;
    }
    
    assert(ptr >= memory && ptr < memory + (blockSize * blockCount));
    assert(((reinterpret_cast<uintptr_t>(ptr) - reinterpret_cast<uintptr_t>(memory)) % blockSize) == 0);
    
    Block* block = static_cast<Block*>(ptr);
    block->next = freeHead;
    freeHead = block;
    --usedBlocks;
}

void PoolAllocator::InitializeFreeList() {
    for (size_t i = 0; i < blockCount; ++i) {
        Block* block = reinterpret_cast<Block*>(memory + i * blockSize);
        block->next = (i == blockCount - 1) ? nullptr : reinterpret_cast<Block*>(memory + (i + 1) * blockSize);
    }
    freeHead = reinterpret_cast<Block*>(memory);
}

} 
