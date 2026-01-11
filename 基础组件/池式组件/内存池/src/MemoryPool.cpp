#include "MemoryPool.h"
#include <cstdlib>   // malloc, free
#include <stdexcept> // std::bad_alloc

MemoryPool::MemoryPool(size_t block_size, size_t initial_blocks)
    : head_(nullptr), memory_(nullptr), block_size_(block_size + sizeof(Block*)), 
      total_blocks_(0), used_blocks_(0) {
    if (block_size_ < sizeof(Block*)) {
        block_size_ = sizeof(Block*); // 至少能放下一个指针
    }
    expand(initial_blocks);
}

MemoryPool::~MemoryPool() {
    // 注意：这里假设只分配了一次大块内存（实际情况可多次扩展，这里简化）
    // 如果有多次 expand，需要记录所有内存块并逐个 free
    std::free(memory_);
}

void MemoryPool::expand(size_t additional_blocks) {
    // 计算需要分配的内存大小
    size_t alloc_size = block_size_ * additional_blocks;
    char* new_memory = static_cast<char*>(std::malloc(alloc_size));
    if (!new_memory) {
        throw std::bad_alloc();
    }

    // 将新内存加入空闲链表
    char* current = new_memory;
    for (size_t i = 0; i < additional_blocks - 1; ++i) {
        Block* block = reinterpret_cast<Block*>(current);
        block->next = reinterpret_cast<Block*>(current + block_size_);
        current += block_size_;
    }
    // 最后一个块
    reinterpret_cast<Block*>(current)->next = head_;

    head_ = reinterpret_cast<Block*>(new_memory);
    total_blocks_ += additional_blocks;

    // 如果是第一次分配，记录起始地址（简化：这里只支持一次分配）
    if (!memory_) {
        memory_ = new_memory;
    }
    // 实际项目中应使用链表管理多个大块内存，这里为简化示例仅一次分配
}

void* MemoryPool::allocate() {
    if (!head_) {
        expand(); // 自动扩展
    }
    Block* block = head_;
    head_ = head_->next;
    ++used_blocks_;
    return block;
}

void MemoryPool::deallocate(void* ptr) {
    if (!ptr) return;
    Block* block = static_cast<Block*>(ptr);
    block->next = head_;
    head_ = block;
    --used_blocks_;
}