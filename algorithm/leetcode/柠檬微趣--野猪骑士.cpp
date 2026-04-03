#include<bits/stdc++.h>
using namespace std;
int a[100005];
int main(){
    int n ;
    cin>>n;
    vector<int> ans(n, -1);
    set<int> s; // 有序集合，自动排序，存储右侧所有元素
    for(int i=0;i<n;i++)cin>>a[i];
    // 从右向左遍历
    for (int i = n - 1; i >= 0; --i) {
        // 查找第一个 严格大于 a[i] 的元素（就是我们要的最小值）
        auto it = s.upper_bound(a[i]);
        if (it != s.end()) {
            ans[i] = *it;
        }
        // 将当前元素加入集合，供左侧元素使用
        s.insert(a[i]);
    }
    for(int i=0;i<n;i++){
        cout<<ans[i]<<" ";
    }
    return 0;
}