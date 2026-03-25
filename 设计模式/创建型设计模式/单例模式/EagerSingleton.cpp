#include <iostream>

using namespace std;

class EagerSingleton {
private:
    // 饿汉：静态指针在类外直接初始化为 new 出来的对象
    static EagerSingleton* instance;

    EagerSingleton() = default;
    EagerSingleton(const EagerSingleton&) = delete;
    EagerSingleton& operator=(const EagerSingleton&) = delete;

public:
    static EagerSingleton* getInstance() {
        return instance;
    }

    void show() const {
        cout << "Eager singleton (instance created at load time)" << endl;
    }
};

// 类外定义并初始化
EagerSingleton* EagerSingleton::instance = new EagerSingleton();

int main() {
    EagerSingleton* s1 = EagerSingleton::getInstance();
    EagerSingleton* s2 = EagerSingleton::getInstance();

    cout << "s1 address: " << s1 << "  s2 address: " << s2 << endl;
    s1->show();
    return 0;
}
