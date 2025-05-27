cmake_minimum_required(VERSION 3.16)
project(GameEngineTests)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Google Test
find_package(GTest REQUIRED)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/GameEngine)
include_directories(${GTEST_INCLUDE_DIRS})

# Source files for the engine
set(ENGINE_SOURCES
    ${CMAKE_SOURCE_DIR}/GameEngine/Core/Memory/StackAllocator.cpp
    ${CMAKE_SOURCE_DIR}/GameEngine/Core/Memory/PoolAllocator.cpp
    ${CMAKE_SOURCE_DIR}/GameEngine/Core/Memory/HeapAllocator.cpp
)

# Test source files
set(TEST_SOURCES
    Tests/Core/Memory/AllocatorTests.cpp
)

# Create test executable
add_executable(GameEngineTests ${TEST_SOURCES} ${ENGINE_SOURCES})

# Link libraries
target_link_libraries(GameEngineTests ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES} pthread)

# Enable testing
enable_testing()
add_test(NAME AllocatorTests COMMAND GameEngineTests)
