#pragma once
#include "IAllocator.h"

namespace GameEngine::memory {

class PoolAllocator : public IAllocator {
public:
    PoolAllocator(size_t blockSize, size_t blockCount);
    ~PoolAllocator() override;
    
    void* Allocate(size_t size = 0, size_t alignment = sizeof(void*)) override;
    void Deallocate(void* ptr) override;
    
    size_t GetUsedMemory() const override { return usedBlocks * blockSize; }
    size_t GetTotalMemory() const override { return blockCount * blockSize; }
    
    size_t GetUsedBlocks() const { return usedBlocks; }
    size_t GetFreeBlocks() const { return blockCount - usedBlocks; }
    size_t GetBlockSize() const { return blockSize; }

private:
    union Block {
        Block* next;
        uint8_t data[1];
    };
    
    uint8_t* memory;
    Block* freeHead;
    size_t blockSize;
    size_t blockCount;
    size_t usedBlocks;
    
    void InitializeFreeList();
};

} // namespace GameEngine::Memory
