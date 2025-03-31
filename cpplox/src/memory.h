#ifndef CLOXCPP_MEMORY_H_
#define CLOXCPP_MEMORY_H_

#include <atomic>
#include <new>

#include "array.h"
#include "common.h"
#include "loxexception.h"

namespace lox {
    constexpr uint32_t _1GB = 1024 * 1024 * 1024;
    constexpr uint32_t POOL_SENTINEL = _1GB;
    class Arena {
    public:
        Arena();
        ~Arena();

        void* allocate(size_t size);
        void deallocate(void* data);
        void* reallocate(void* data, size_t newSize);

    private:
        void addToFreePool(size_t poolIndex, std::byte* memory);
        std::byte* popBlockFromPool(size_t poolIndex);
        std::byte* getBuddy(size_t poolIndex, std::byte* memory) const;
        bool isInFreeList(std::byte* block, size_t poolIndex) const;
        bool isFree(std::byte* block) const;
        void removeBlockFromPool(std::byte* block);
        void verifyNotFreed(std::byte* block) const;
        void verifyFreed(std::byte* block) const;
        void verifyAllocatedBlockHasValidPoolSize(std::byte* block) const;
        std::byte* growBlock(std::byte* block, size_t poolIndex, size_t newSize);
        std::byte* shrinkBlock(std::byte* block, size_t poolIndex, size_t newSize);
        bool isRequestedSizeAlreadyInCorrectPool(uint32_t poolIndex, size_t newSize) const;
        void shrinkBlockToRequestedSize(size_t poolIndex, std::byte* block, size_t newSize);
        Array<uint32_t, 28> pools;  // pools[0] is for 8 bytes, all the way up to pools[27] for 1 gig
        std::byte* memory = nullptr;
    };

    extern Arena arena;

    template <typename T>
    [[nodiscard]] T* reallocate(T* pointer, size_t oldSize, size_t newSize) {
        T* result = nullptr;
        if (pointer == nullptr || (oldSize == 0 && newSize != 0)) {
            result = reinterpret_cast<T*>(arena.allocate(newSize));
        } else if (newSize == 0 && oldSize != 0) {
            arena.deallocate(pointer);
            return nullptr;
        } else if (newSize != 0 && oldSize != 0) {
            result = reinterpret_cast<T*>(arena.reallocate(pointer, newSize));
        }
        if (!result)
            throw BadAllocException{"Memory realloc failed", std::bad_alloc{}};
        return result;
    }

    template <typename T>
    T* allocate(size_t size = sizeof(T)) {
        return reallocate<T>(nullptr, 0, size);
    }

    template <typename T>
    void deallocate(T* p) {
        // just put a 1 in the old size to force a deallocation
        std::ignore = reallocate(p, 1, 0);
    }

    template <typename T>
    class SharedPtr {
    public:
        SharedPtr() {}
        SharedPtr(T* inPtr) {
            ctrlBlock = allocate<ControlBlock>();
            ctrlBlock.refCount++;
            ctrlBlock.rawPtr = inPtr;
        }

        SharedPtr(const SharedPtr& sp) {
            this->ctrlBlock = sp.ctrlBlock;
            if (this->ctrlBlock) {
                this->ctrlBlock->IncrementRef();
            }
        }

        SharedPtr& operator=(const SharedPtr& sp) {
            this->ctrlBlock = sp.ctrlBlock;
            if (this->ctrlBlock) {
                this->ctrlBlock->IncrementRef();
            }
            return *this;
        }

        friend auto operator<=>(const SharedPtr& lhs, const SharedPtr& rhs) = default;

        SharedPtr(SharedPtr&& sp) {
            this->ctrlBlock = sp.ctrlBlock;
            sp.ctrlBlock = nullptr;
        }
        SharedPtr& operator=(SharedPtr&& sp) {
            if (this == &sp) {
                return *this;
            }
            if (this->ctrlBlock) {
                this->ctrlBlock.DecrementRef();  // got to remove the old one
            }
            this->ctrlBlock = sp.ctrlBlock;
            sp->ctrlBlock = nullptr;
            return *this;
        }

        ~SharedPtr() {
            if (ctrlBlock) {
                ctrlBlock->DecrementRef();
            }
        }

        static SharedPtr<T> Make() {
            return SharedPtr(allocate<T>());
        }

    private:
        class ControlBlock {
        private:
            std::atomic<size_t> refCount = 0;
            T* rawPtr = nullptr;

        public:
            void DecrementRef() {
                if (refCount.fetch_sub(1) == 1) {
                    deallocate(rawPtr);
                    deallocate<ControlBlock>(this);
                }
            }

            void IncrementRef() {
                refCount++;
            }
        };
        ControlBlock* ctrlBlock = nullptr;
    };

}
#endif