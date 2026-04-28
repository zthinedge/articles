#include <iostream>
using namespace std;
using Callback1=void(*)();
using Callback2=void(*)(int);
void onFinish() {
    cout << "任务完成后的处理逻辑" << endl;
}
void doTask(Callback1 cb) {//void (*cb)()
    cout << "正在执行任务..." << endl;
    cb();   // 回调
}
void handleResult(int x){
    cout << "计算结果是: " << x << endl;
}
int add(int a,int b,Callback2 cb){//void (*cb)(int)
    int result = a + b;
    cb(result);   // 把结果交给回调函数处理
    return result;
}

int main() {
    doTask(onFinish);
    add(3,5,handleResult);
    return 0;
}