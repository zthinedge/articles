#ifndef MEMORY_POOL_H
#define MEMORY_POOL_H
#include <cstddef>  // size_t
#include <new>      // bad_alloc

class MemoryPool {
public:
    // 构造函数：块大小和初始块数量
    explicit MemoryPool(size_t block_size, size_t initial_blocks = 1024);
    
    // 禁用拷贝和移动（内存池通常不应该拷贝）
    MemoryPool(const MemoryPool&) = delete;
    MemoryPool& operator=(const MemoryPool&) = delete;
    MemoryPool(MemoryPool&&) = delete;
    MemoryPool& operator=(MemoryPool&&) = delete;
    
    // 析构函数：释放所有内存
    ~MemoryPool();

    // 分配一个块
    void* allocate();

    // 释放一个块
    void deallocate(void* ptr);

    // 获取块大小
    size_t getBlockSize() const { return block_size_; }

private:
    struct Block {
        Block* next;
    };

    Block* head_;              // 空闲链表头
    char* memory_;             // 大块内存起始地址
    size_t block_size_;        // 每个块的大小（包括链表指针）
    size_t total_blocks_;      // 当前总块数
    size_t used_blocks_;       // 已使用块数

    // 内部：扩展内存池（当空闲块不足时）
    void expand(size_t additional_blocks = 1024);
};

#endif