#include <gtest/gtest.h>
#include "../../../GameEngine/Core/Memory/StackAllocator.h"
#include "../../../GameEngine/Core/Memory/PoolAllocator.h"
#include "../../../GameEngine/Core/Memory/HeapAllocator.h"
#include <cstring>

namespace GameEngine::Tests {

class StackAllocatorTest : public ::testing::Test {
protected:
    void SetUp() override {
        allocator = std::make_unique<memory::StackAllocator>(1024);
    }

    void TearDown() override {
        allocator.reset();
    }

    std::unique_ptr<memory::StackAllocator> allocator;
};

TEST_F(StackAllocatorTest, AllocateBasicMemory) {
    void* ptr = allocator->Allocate(64, 8);
    ASSERT_NE(ptr, nullptr);
    EXPECT_EQ(allocator->GetUsedMemory(), 64);
}

TEST_F(StackAllocatorTest, AllocateWithAlignment) {
    void* ptr = allocator->Allocate(32, 16);
    ASSERT_NE(ptr, nullptr);
    EXPECT_EQ(reinterpret_cast<uintptr_t>(ptr) % 16, 0);
}

TEST_F(StackAllocatorTest, AllocateSequentialMemory) {
    void* ptr1 = allocator->Allocate(32, 8);
    void* ptr2 = allocator->Allocate(32, 8);
    
    ASSERT_NE(ptr1, nullptr);
    ASSERT_NE(ptr2, nullptr);
    EXPECT_GT(reinterpret_cast<uintptr_t>(ptr2), reinterpret_cast<uintptr_t>(ptr1));
    EXPECT_EQ(allocator->GetUsedMemory(), 64);
}

TEST_F(StackAllocatorTest, ExceedCapacity) {
    void* ptr = allocator->Allocate(2048, 8);
    EXPECT_EQ(ptr, nullptr);
    EXPECT_EQ(allocator->GetUsedMemory(), 0);
}

TEST_F(StackAllocatorTest, Reset) {
    allocator->Allocate(64, 8);
    allocator->Allocate(32, 8);
    EXPECT_EQ(allocator->GetUsedMemory(), 96);
    
    allocator->Reset();
    EXPECT_EQ(allocator->GetUsedMemory(), 0);
}

TEST_F(StackAllocatorTest, SaveAndRestoreMarker) {
    auto marker = allocator->GetMarker();
    allocator->Allocate(64, 8);
    allocator->Allocate(32, 8);
    EXPECT_EQ(allocator->GetUsedMemory(), 96);
    
    allocator->FreeToMarker(marker);
    EXPECT_EQ(allocator->GetUsedMemory(), 0);
}

class PoolAllocatorTest : public ::testing::Test {
protected:
    void SetUp() override {
        allocator = std::make_unique<memory::PoolAllocator>(64, 16);
    }

    void TearDown() override {
        allocator.reset();
    }

    std::unique_ptr<memory::PoolAllocator> allocator;
};

TEST_F(PoolAllocatorTest, AllocateBlock) {
    void* ptr = allocator->Allocate();
    ASSERT_NE(ptr, nullptr);
    EXPECT_EQ(allocator->GetUsedBlocks(), 1);
    EXPECT_EQ(allocator->GetFreeBlocks(), 15);
}

TEST_F(PoolAllocatorTest, AllocateMultipleBlocks) {
    void* ptr1 = allocator->Allocate();
    void* ptr2 = allocator->Allocate();
    void* ptr3 = allocator->Allocate();
    
    ASSERT_NE(ptr1, nullptr);
    ASSERT_NE(ptr2, nullptr);
    ASSERT_NE(ptr3, nullptr);
    EXPECT_NE(ptr1, ptr2);
    EXPECT_NE(ptr2, ptr3);
    EXPECT_EQ(allocator->GetUsedBlocks(), 3);
    EXPECT_EQ(allocator->GetFreeBlocks(), 13);
}

TEST_F(PoolAllocatorTest, DeallocateBlock) {
    void* ptr = allocator->Allocate();
    ASSERT_NE(ptr, nullptr);
    EXPECT_EQ(allocator->GetUsedBlocks(), 1);
    
    allocator->Deallocate(ptr);
    EXPECT_EQ(allocator->GetUsedBlocks(), 0);
    EXPECT_EQ(allocator->GetFreeBlocks(), 16);
}

TEST_F(PoolAllocatorTest, AllocateAllBlocks) {
    void* ptrs[16];
    for (int i = 0; i < 16; ++i) {
        ptrs[i] = allocator->Allocate();
        ASSERT_NE(ptrs[i], nullptr);
    }
    
    EXPECT_EQ(allocator->GetUsedBlocks(), 16);
    EXPECT_EQ(allocator->GetFreeBlocks(), 0);
    
    void* overflow = allocator->Allocate();
    EXPECT_EQ(overflow, nullptr);
}

TEST_F(PoolAllocatorTest, ReuseBlockAfterDeallocation) {
    void* ptr1 = allocator->Allocate();
    allocator->Deallocate(ptr1);
    
    void* ptr2 = allocator->Allocate();
    EXPECT_EQ(ptr1, ptr2);
}

class HeapAllocatorTest : public ::testing::Test {
protected:
    void SetUp() override {
        allocator = std::make_unique<memory::HeapAllocator>(4096);
    }

    void TearDown() override {
        allocator.reset();
    }

    std::unique_ptr<memory::HeapAllocator> allocator;
};

TEST_F(HeapAllocatorTest, AllocateBasicMemory) {
    void* ptr = allocator->Allocate(64, 8);
    ASSERT_NE(ptr, nullptr);
    EXPECT_GT(allocator->GetUsedMemory(), 0);
}

TEST_F(HeapAllocatorTest, AllocateWithAlignment) {
    void* ptr = allocator->Allocate(32, 16);
    ASSERT_NE(ptr, nullptr);
    EXPECT_EQ(reinterpret_cast<uintptr_t>(ptr) % 16, 0);
}

TEST_F(HeapAllocatorTest, DeallocateMemory) {
    void* ptr = allocator->Allocate(64, 8);
    ASSERT_NE(ptr, nullptr);
    size_t usedBefore = allocator->GetUsedMemory();
    
    allocator->Deallocate(ptr);
    EXPECT_LT(allocator->GetUsedMemory(), usedBefore);
}

TEST_F(HeapAllocatorTest, AllocateMultipleSizes) {
    void* ptr1 = allocator->Allocate(32, 8);
    void* ptr2 = allocator->Allocate(128, 8);
    void* ptr3 = allocator->Allocate(256, 8);
    
    ASSERT_NE(ptr1, nullptr);
    ASSERT_NE(ptr2, nullptr);
    ASSERT_NE(ptr3, nullptr);
    EXPECT_NE(ptr1, ptr2);
    EXPECT_NE(ptr2, ptr3);
}

TEST_F(HeapAllocatorTest, CoalesceAfterDeallocation) {
    void* ptr1 = allocator->Allocate(64, 8);
    void* ptr2 = allocator->Allocate(64, 8);
    void* ptr3 = allocator->Allocate(64, 8);
    
    allocator->Deallocate(ptr1);
    allocator->Deallocate(ptr3);
    allocator->Deallocate(ptr2);
    
    void* bigPtr = allocator->Allocate(192, 8);
    ASSERT_NE(bigPtr, nullptr);
}

TEST_F(HeapAllocatorTest, ExceedCapacity) {
    void* ptr = allocator->Allocate(8192, 8);
    EXPECT_EQ(ptr, nullptr);
}

} 
