#include "HeapAllocator.h"
#include <cassert>
#include <algorithm>

namespace GameEngine::memory {

HeapAllocator::HeapAllocator(size_t capacity)
    : memory(new uint8_t[capacity])
    , capacity(capacity)
    , usedMemory(0)
    , freeHead(nullptr) {
    
    InitializeFreeList();
}

HeapAllocator::~HeapAllocator() {
    delete[] memory;
}

void* HeapAllocator::Allocate(size_t size, size_t alignment) {
    assert(size > 0);
    assert((alignment & (alignment - 1)) == 0);
    
    size_t totalSize = size + HeaderSize;
    size_t alignedSize = AlignAddress(totalSize, alignment);
    
    if (alignedSize > capacity - usedMemory) {
        return nullptr;
    }
    
    FreeBlock* block = FindBestFit(alignedSize);
    if (block == nullptr) {
        return nullptr;
    }
    
    RemoveFreeBlock(block);
    
    uint8_t* blockStart = reinterpret_cast<uint8_t*>(block);
    uint8_t* dataStart = blockStart + HeaderSize;
    uint8_t* alignedDataStart = reinterpret_cast<uint8_t*>(AlignAddress(reinterpret_cast<uintptr_t>(dataStart), alignment));
    
    size_t padding = alignedDataStart - dataStart;
    size_t requiredSize = HeaderSize + padding + size;
    
    if (block->size >= requiredSize + MinBlockSize) {
        FreeBlock* remainder = reinterpret_cast<FreeBlock*>(blockStart + requiredSize);
        remainder->size = block->size - requiredSize;
        remainder->next = nullptr;
        remainder->prev = nullptr;
        InsertFreeBlock(remainder);
    } else {
        requiredSize = block->size;
    }
    
    AllocHeader* header = reinterpret_cast<AllocHeader*>(alignedDataStart - HeaderSize);
    header->size = requiredSize;
    header->padding = padding;
    
    usedMemory += requiredSize;
    return alignedDataStart;
}

void HeapAllocator::Deallocate(void* ptr) {
    if (ptr == nullptr) {
        return;
    }
    
    AllocHeader* header = reinterpret_cast<AllocHeader*>(static_cast<uint8_t*>(ptr) - HeaderSize);
    size_t blockSize = header->size;
    
    uint8_t* blockStart = reinterpret_cast<uint8_t*>(header) - header->padding;
    FreeBlock* freeBlock = reinterpret_cast<FreeBlock*>(blockStart);
    freeBlock->size = blockSize;
    freeBlock->next = nullptr;
    freeBlock->prev = nullptr;
    
    usedMemory -= blockSize;
    
    InsertFreeBlock(freeBlock);
    CoalesceFreeBlocks(freeBlock);
}

void HeapAllocator::InitializeFreeList() {
    freeHead = reinterpret_cast<FreeBlock*>(memory);
    freeHead->size = capacity;
    freeHead->next = nullptr;
    freeHead->prev = nullptr;
}

HeapAllocator::FreeBlock* HeapAllocator::FindBestFit(size_t size) {
    FreeBlock* bestFit = nullptr;
    FreeBlock* current = freeHead;
    
    while (current != nullptr) {
        if (current->size >= size) {
            if (bestFit == nullptr || current->size < bestFit->size) {
                bestFit = current;
            }
        }
        current = current->next;
    }
    
    return bestFit;
}

void HeapAllocator::InsertFreeBlock(FreeBlock* block) {
    if (freeHead == nullptr) {
        freeHead = block;
        return;
    }
    
    if (block < freeHead) {
        block->next = freeHead;
        freeHead->prev = block;
        freeHead = block;
        return;
    }
    
    FreeBlock* current = freeHead;
    while (current->next != nullptr && current->next < block) {
        current = current->next;
    }
    
    block->next = current->next;
    block->prev = current;
    if (current->next != nullptr) {
        current->next->prev = block;
    }
    current->next = block;
}

void HeapAllocator::RemoveFreeBlock(FreeBlock* block) {
    if (block->prev != nullptr) {
        block->prev->next = block->next;
    } else {
        freeHead = block->next;
    }
    
    if (block->next != nullptr) {
        block->next->prev = block->prev;
    }
}

void HeapAllocator::CoalesceFreeBlocks(FreeBlock* block) {
    uint8_t* blockEnd = reinterpret_cast<uint8_t*>(block) + block->size;
    
    if (block->next != nullptr && reinterpret_cast<uint8_t*>(block->next) == blockEnd) {
        block->size += block->next->size;
        RemoveFreeBlock(block->next);
    }
    
    if (block->prev != nullptr) {
        uint8_t* prevEnd = reinterpret_cast<uint8_t*>(block->prev) + block->prev->size;
        if (prevEnd == reinterpret_cast<uint8_t*>(block)) {
            block->prev->size += block->size;
            RemoveFreeBlock(block);
        }
    }
}

} 
