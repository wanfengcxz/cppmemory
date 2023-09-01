//
// Created by 晚风吹行舟 on 2023/8/31.
//
#include <iostream>
#include <vector>
#include <ext/pool_allocator.h>

using namespace std;

namespace test01 {

    class A {

    };

    void test_allocator() {
        cout << "\n\n---------------test_allocator-------------\n";

        vector<int> v1; // 用的最原始的allocator，什么都不做
        vector<int, __gnu_cxx::__pool_alloc<int>> v2_pool;

        /// 需要附带<ext/pool_allocator.h>这个头文件才能使用
        cout << sizeof(__gnu_cxx::__pool_alloc<int>) << endl;   // 1

    }

}

int main() {
    test01::test_allocator();

    return 0;
}