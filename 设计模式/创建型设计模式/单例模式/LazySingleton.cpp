#include <iostream>
#include <mutex>

using namespace std;

// 0: non-thread-safe lazy singleton
// 1: thread-safe lazy singleton by mutex
// 2: thread-safe lazy singleton by C++11 local static (Meyers' Singleton)
#define SINGLETON_MODE 2

class LazySingleton {
private:
    LazySingleton() = default;
    LazySingleton(const LazySingleton&) = delete;
    LazySingleton& operator=(const LazySingleton&) = delete;

#if SINGLETON_MODE == 0 || SINGLETON_MODE == 1
    static LazySingleton* instance;
#endif

#if SINGLETON_MODE == 1
    static mutex mtx;
#endif

public:
    static LazySingleton* getInstance() {
#if SINGLETON_MODE == 0
        if (instance == nullptr) {
            instance = new LazySingleton();
        }
        return instance;
#elif SINGLETON_MODE == 1
        lock_guard<mutex> lock(mtx);
        if (instance == nullptr) {
            instance = new LazySingleton();
        }
        return instance;
#elif SINGLETON_MODE == 2
        static LazySingleton instance;
        return &instance;
#else
#error "Unsupported SINGLETON_MODE. Use 0, 1, or 2."
#endif
    }

    void show() {
#if SINGLETON_MODE == 0
        cout << "Lazy singleton (non-thread-safe)" << endl;
#elif SINGLETON_MODE == 1
        cout << "Lazy singleton (thread-safe by mutex)" << endl;
#elif SINGLETON_MODE == 2
        cout << "Lazy singleton (thread-safe by local static)" << endl;
#endif
    }
};

#if SINGLETON_MODE == 0 || SINGLETON_MODE == 1
LazySingleton* LazySingleton::instance = nullptr;
#endif

#if SINGLETON_MODE == 1
mutex LazySingleton::mtx;
#endif

int main() {
    LazySingleton* s1 = LazySingleton::getInstance();
    LazySingleton* s2 = LazySingleton::getInstance();

    cout << "s1 address: " << s1 << "  s2 address: " << s2 << endl;
    s1->show();
    return 0;
}
