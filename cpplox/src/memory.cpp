#include "memory.h"

#include <cstring>
#include <print>
#include <utility>

#include "algorithm.h"
#include "loxexception.h"
namespace lox {
    size_t poolSizes[] = {1 << 0, 1 << 1, 1 << 2, 1 << 3, 1 << 4, 1 << 5, 1 << 6, 1 << 7, 1 << 8, 1 << 9,
                          1 << 10, 1 << 11, 1 << 12, 1 << 13, 1 << 14, 1 << 15, 1 << 16, 1 << 17, 1 << 18, 1 << 19,
                          1 << 20, 1 << 21, 1 << 22, 1 << 23, 1 << 24, 1 << 25, 1 << 26, 1 << 27, 1 << 28, 1 << 29,
                          1 << 30, 1uz << 31};
    size_t getPoolSize(size_t index) {
        return poolSizes[index + 3] - 1;  // take off 1 byte for the size that has to go in the first byte
    }

    // can't use endianess because we want our msb in the first byte
    void writeIntToMemory(std::byte* memory, uint32_t value) {
        memory[0] = std::byte{static_cast<uint8_t>((value >> 24) & 0xFF)};
        memory[1] = std::byte{static_cast<uint8_t>((value >> 16) & 0xFF)};
        memory[2] = std::byte{static_cast<uint8_t>((value >> 8) & 0xFF)};
        memory[3] = std::byte{static_cast<uint8_t>(value & 0xFF)};
    }

    uint32_t readIntFromMemory(std::byte* memory) {
        return (static_cast<uint8_t>(memory[0]) << 24) + (static_cast<uint8_t>(memory[1]) << 16) + (static_cast<uint8_t>(memory[2]) << 8) + static_cast<uint8_t>(memory[3]);
    }

    Arena arena;

    Arena::Arena() {
        lox::ranges::fill(pools, POOL_SENTINEL);
        memory = (std::byte*)malloc(_1GB);
        // lox::ranges::fill_n(memory, _1GB, std::byte{128});  // mark all as free
        if (!memory) {
            throw Exception("Could not allocate arena", nullptr);
        }

        addToFreePool(27, memory);
    }

    void* Arena::allocate(size_t requested) {
        for (size_t index = 0; index < 28; ++index) {
            if (pools[index] == POOL_SENTINEL || getPoolSize(index) < requested) {
                continue;
            }
            // at this point, we should have a pool that fits
            auto pool = popBlockFromPool(index);
            shrinkBlockToRequestedSize(index, pool, requested);
            return static_cast<void*>(pool + 1);
        }

        return nullptr;
    }

    void Arena::shrinkBlockToRequestedSize(size_t poolIndex, std::byte* block, size_t newSize) {
        // while a smaller block could fit this, split this block in half and store its buddy
        while (poolIndex > 0 && getPoolSize(poolIndex - 1) > newSize) {
            poolIndex--;
            auto buddy = getBuddy(poolIndex, block);
            addToFreePool(poolIndex, buddy);
        }
        *block = std::byte{static_cast<uint8_t>(poolIndex)};
    }
    void Arena::verifyNotFreed(std::byte* block) const {
        if (isFree(block)) {
            throw lox::Exception("Trying to free block that has already been freed", nullptr);
        }
    }
    void Arena::verifyFreed(std::byte* block) const {
        if (!isFree(block)) {
            throw lox::Exception("Block is not a free block", nullptr);
        }
    }
    void Arena::verifyAllocatedBlockHasValidPoolSize(std::byte* block) const {
        if (static_cast<uint8_t>(*block) >= 28) {
            throw lox::Exception("We've detected some sort of corruption and can't deallocate this block", nullptr);
        }
    }

    void Arena::deallocate(void* rawBlock) {
        auto blockStart = static_cast<std::byte*>(rawBlock) - 1;
        verifyNotFreed(blockStart);
        verifyAllocatedBlockHasValidPoolSize(blockStart);
        auto poolIndex = static_cast<size_t>(*blockStart);
        // while we have a buddy, combine it and move it up the pool
        auto leftMostBlock = blockStart;
        auto buddy = getBuddy(poolIndex, blockStart);
        while (poolIndex < 27 && isInFreeList(buddy, poolIndex)) {
            removeBlockFromPool(buddy);
            poolIndex++;
            leftMostBlock = std::min(leftMostBlock, buddy);
            buddy = getBuddy(poolIndex, leftMostBlock);
        }
        addToFreePool(poolIndex, leftMostBlock);
    }

    void* Arena::reallocate(void* rawBlock, size_t newSize) {
        if (newSize == 0) {  // assume data has been set correctly
            deallocate(rawBlock);
            return nullptr;
        }
        if (newSize > _1GB) {
            throw lox::Exception("Can not allocate more than one gigabyte in the arena", nullptr);
        }
        auto blockStart = static_cast<std::byte*>(rawBlock) - 1;
        verifyNotFreed(blockStart);
        verifyAllocatedBlockHasValidPoolSize(blockStart);
        auto poolIndex = static_cast<uint32_t>(*blockStart);
        if (isRequestedSizeAlreadyInCorrectPool(poolIndex, newSize)) {
            return rawBlock;
        } else if (getPoolSize(poolIndex) < newSize) {
            auto block = growBlock(blockStart, poolIndex, newSize);
            return static_cast<void*>(block + 1);
        } else {
            auto block = shrinkBlock(blockStart, poolIndex, newSize);
            return static_cast<void*>(block + 1);
        }
        return nullptr;
    }

    std::byte* Arena::growBlock(std::byte* block, size_t poolIndex, size_t newSize) {
        size_t poolSize = getPoolSize(poolIndex);
        size_t targetIndex = poolIndex;
        auto buddy = getBuddy(targetIndex, block);
        // find if we can grow to our block without copying
        while (targetIndex < 27 && getPoolSize(targetIndex + 1) < newSize && isInFreeList(buddy, targetIndex)) {
            targetIndex++;
            buddy = getBuddy(targetIndex, block);
        }
        // if we have a target index that we can grow to
        if (targetIndex < 27 && isInFreeList(buddy, targetIndex)) {
            auto leftmostFreeBlock = block;
            for (size_t index = poolIndex; index <= targetIndex; ++index) {
                auto buddy = getBuddy(index, block);
                leftmostFreeBlock = std::min(buddy, leftmostFreeBlock);
                removeBlockFromPool(buddy);
            }
            if (leftmostFreeBlock < block) {
                memcpy(leftmostFreeBlock + 1, block + 1, poolSize);
            }
            // write new size in the block
            block = leftmostFreeBlock;
            *block = std::byte{static_cast<uint8_t>(targetIndex + 1)};
            return block;
        }

        // we need to copy data over
        auto newBlock = static_cast<std::byte*>(allocate(newSize));
        if (newBlock == nullptr) {
            return nullptr;  // failed to allocate new block
        }
        memcpy(newBlock, block + 1, poolSize);
        deallocate(block + 1);
        return newBlock - 1;
    }

    std::byte* Arena::shrinkBlock(std::byte* block, size_t poolIndex, size_t newSize) {
        if (poolIndex == 0 || getPoolSize(poolIndex - 1) < newSize) {
            return block;
        }
        shrinkBlockToRequestedSize(poolIndex, block, newSize);
        return block;
    }
    bool Arena::isRequestedSizeAlreadyInCorrectPool(uint32_t poolIndex, size_t newSize) const {
        return getPoolSize(poolIndex) >= newSize && (poolIndex == 0 || getPoolSize(poolIndex - 1) <= newSize);
    }

    std::byte* Arena::getBuddy(size_t index, std::byte* block) const {
        uint32_t offset = block - memory;
        uint32_t buddyOffset = offset ^ (1 << (index + 3));
        return memory + buddyOffset;
    }

    bool Arena::isInFreeList(std::byte* block, size_t targetIndex) const {
        auto current = pools[targetIndex];
        while (current != POOL_SENTINEL) {
            if (memory + current == block) {
                return true;
            }

            current = readIntFromMemory(memory + current + 4);
        }
        return false;
    }
    bool Arena::isFree(std::byte* block) const {
        return static_cast<uint8_t>(*block) >= 128;  // if first bit is set
    }

    // this does NOT free the block or write the size, its assumed that the caller will do that
    // if needed (this is becasue sometimes a block is removed and added to a bigger block)
    // which does not need its own size tracking
    void Arena::removeBlockFromPool(std::byte* block) {
        verifyFreed(block);
        auto previousIndex = readIntFromMemory(block) & 0x7FFFFFFF;
        auto nextIndex = readIntFromMemory(block + 4);
        if (previousIndex != POOL_SENTINEL) {
            auto previousBlock = memory + previousIndex;
            writeIntToMemory(previousBlock + 4, nextIndex);
        }
        if (nextIndex != POOL_SENTINEL) {
            auto nextBlock = memory + nextIndex;
            writeIntToMemory(nextBlock, previousIndex);
            nextBlock[0] |= std::byte{1 << 7};  // restore free bit
        }
        for (auto& pool : pools) {
            if (memory + pool == block) {
                pool = nextIndex;
            }
        }
    }

    std::byte* Arena::popBlockFromPool(size_t poolIndex) {
        auto poolOffset = pools[poolIndex];
        auto block = memory + poolOffset;
        removeBlockFromPool(block);
        block[0] = std::byte{static_cast<uint8_t>(poolIndex)};  // clear free bit and set size so we know how to free it later
        return block;
    }

    Arena::~Arena() {
        free(memory);
    }

    void Arena::addToFreePool(size_t poolIndex, std::byte* block) {
        uint32_t firstBlockOffset = pools[poolIndex];
        uint32_t newBlockOffset = static_cast<uint32_t>(block - memory);

        if (firstBlockOffset != POOL_SENTINEL) {
            std::byte* firstBlock = memory + firstBlockOffset;
            writeIntToMemory(firstBlock, newBlockOffset);
            firstBlock[0] |= std::byte{1 << 7};  // restore free bit
        }

        writeIntToMemory(block, POOL_SENTINEL);
        writeIntToMemory(block + 4, firstBlockOffset);
        block[0] |= std::byte{1 << 7};  // no previous link, but mark as free
        pools[poolIndex] = newBlockOffset;
    }
}