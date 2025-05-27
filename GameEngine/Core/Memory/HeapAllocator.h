#pragma once
#include "IAllocator.h"

namespace GameEngine::memory {

class HeapAllocator : public IAllocator {
public:
    explicit HeapAllocator(size_t capacity);
    ~HeapAllocator() override;
    
    void* Allocate(size_t size, size_t alignment = sizeof(void*)) override;
    void Deallocate(void* ptr) override;
    
    size_t GetUsedMemory() const override { return usedMemory; }
    size_t GetTotalMemory() const override { return capacity; }

private:
    struct FreeBlock {
        size_t size;
        FreeBlock* next;
        FreeBlock* prev;
    };
    
    struct AllocHeader {
        size_t size;
        size_t padding;
    };
    
    uint8_t* memory;
    size_t capacity;
    size_t usedMemory;
    FreeBlock* freeHead;
    
    void InitializeFreeList();
    FreeBlock* FindBestFit(size_t size);
    void InsertFreeBlock(FreeBlock* block);
    void RemoveFreeBlock(FreeBlock* block);
    void CoalesceFreeBlocks(FreeBlock* block);
    
    static constexpr size_t MinBlockSize = sizeof(FreeBlock);
    static constexpr size_t HeaderSize = sizeof(AllocHeader);
};

} 
