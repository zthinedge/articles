#include <iostream>
#include "MemoryPool.h"

int main() {
    MemoryPool pool(64, 1000); // 每个块64字节，初始1000块

    // 分配10个指针
    void* ptrs[10];
    for (int i = 0; i < 10; ++i) {
        ptrs[i] = pool.allocate();
        std::cout << "Allocated: " << ptrs[i] << std::endl;
    }

    // 释放其中5个
    for (int i = 0; i < 5; ++i) {
        pool.deallocate(ptrs[i]);
    }

    // 再次分配，应该复用前面释放的
    for (int i = 0; i < 5; ++i) {
        void* p = pool.allocate();
        std::cout << "Re-allocated (reused): " << p << std::endl;
    }

    std::cout << "Block size: " << pool.getBlockSize() << " bytes" << std::endl;
    return 0;
}