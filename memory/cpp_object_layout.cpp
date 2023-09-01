#include <iostream>

using std::cout;
using std::endl;

// 基于 https://blog.csdn.net/sinat_21107433/article/details/108552472

// base
class Base1 {
public:
    Base1() {
        cout << "Base1::Base1()" << endl;
        a = 1;
        b = 2;
    }

    void func_1() {
        cout << "Base1::func_1()" << endl;
    }

    ~Base1() {
        cout << "Base1::~Base1()" << endl;
    }

private:
    int a;
    int b;
};

void test1() {
    cout << "------------ test1 -----------" << endl;

    Base1 base1;
    cout << "sizeof Base1 class: " << sizeof(Base1) << endl;
    cout << "sizeof Base1 object: " << sizeof(base1) << endl;
}


// 有虚函数
class Base2 {
public:
    Base2() {
        cout << "Base2::Base2()" << endl;
        a = 1;
        b = 2;
    }

    void func_1() {
        cout << "Base2::func_1()" << endl;
    }

    virtual void func_2() {
        cout << "Base2::func_2()" << endl;
    }

    virtual void func_3() {
        cout << "Base2::func_3()" << endl;
    }

    ~Base2() {
        cout << "Base2::~Base2()" << endl;
    }

private:
    int a;
    int b;
};

void test2_1() {
    cout << "------------- test2_1 -----------" << endl;

    Base2 base2;

    // 无论几个虚函数，都只增加8个字节（一个虚表指针）
    cout << "sizeof Base2 class with virtual fun: " << sizeof(Base2) << endl;
    cout << "sizeof Base2 object with virtual fun: " << sizeof(base2) << endl;
}

void test2_2() {
    cout << "-------------- test2_2 -------------" << endl;

    Base2 base;
    Base2 *pBase = &base;
    cout << "base Size: " << sizeof(base) << endl;
    cout << "sizeof long long: " << sizeof(long long) << endl;

    // 虚函数表地址放在对象开始处
    cout << std::hex;
    cout << "address of virtual fun table: 0x" << *(reinterpret_cast<long long *>(pBase)) << endl;

    // 然后才存放其他成员变量
    cout << std::dec;
    cout << "a=" << *(reinterpret_cast<int *>(pBase) + 2) << endl;
    cout << "b=" << *(reinterpret_cast<int *>(pBase) + 3) << endl;


    typedef void(*pFunc)();
    pFunc fun;

    for (int i = 0; i < 2; i++) {
        fun = (pFunc) *(reinterpret_cast<long long *>(*reinterpret_cast<long long *>(pBase)) + i);
        fun();
    }
}

// 继承但不覆写
class Derived : public Base2 {
public:
    Derived() {
        c = 3;
        d = 4;
    }

    virtual void func_4() {
        cout << "Derived::func_4()" << endl;
    };

    virtual void func_5() {
        cout << "Derived::func_5()" << endl;
    };
private:
    int c;
    int d;
};

void test3() {
    cout << "------------- test3 -------------" << endl;

    Derived derived;
    Derived *p = &derived;
    cout << "Derived object size: " << sizeof(derived) << endl;

    // 虚函数表地址放在对象开始处
    cout << std::hex;
    cout << "address of virtual fun table: 0x" << *(reinterpret_cast<long long *>(p)) << endl;

    // 然后才存放其他成员变量
    cout << std::dec;
    cout << "a=" << *(reinterpret_cast<int *>(p) + 2) << endl;
    cout << "b=" << *(reinterpret_cast<int *>(p) + 3) << endl;
    cout << "c=" << *(reinterpret_cast<int *>(p) + 4) << endl;
    cout << "d=" << *(reinterpret_cast<int *>(p) + 5) << endl;

    typedef void(*pFunc)();
    pFunc fun;

    for (int i = 0; i < 4; i++) {
        fun = (pFunc) *(reinterpret_cast<long long *>(*reinterpret_cast<long long *>(p)) + i);
        fun();
    }

    /*
     * 结论：
     * 具有单继承关系的子类，其类对象中保存了一个指向虚函数表的指针，
     * 虚函数表依次存放基类的虚函数和自己（子类）定义的虚函数，接着保存基类的成员和自己定义的成员。
     */
}

// 继承，子类覆写父类虚函数
class Derived2 : public Base2 {
public:
    Derived2() {
        e = 5;
        f = 6;
    }

    // 覆写func_3
    virtual void func_3() {
        cout << "Derived2::func_3()" << endl;
    };

    virtual void func_6() {
        cout << "Derived2::func_6()" << endl;
    };
private:
    int e;
    int f;
};

void test4() {
    cout << "------------------- test4 ------------" << endl;

    Derived2 derived;
    Derived2 *p = &derived;
    cout << "Derived2 Size : " << sizeof(derived) << endl;

    // 虚函数表地址放在对象开始处
    cout << std::hex;
    cout << "address of virtual fun table: 0x" << *(reinterpret_cast<long long *>(p)) << endl;

    // 然后才存放其他成员变量
    cout << std::dec;
    cout << "a=" << *(reinterpret_cast<int *>(p) + 2) << endl;
    cout << "b=" << *(reinterpret_cast<int *>(p) + 3) << endl;
    cout << "e=" << *(reinterpret_cast<int *>(p) + 4) << endl;
    cout << "f=" << *(reinterpret_cast<int *>(p) + 5) << endl;

    typedef void(*pFunc)();
    pFunc fun;

    for (int i = 0; i < 3; i++) {
        fun = (pFunc) *(reinterpret_cast<long long *>(*reinterpret_cast<long long *>(p)) + i);
        fun();
    }

    /*
     * Base2::func_3()这个虚函数被derived给覆盖了
     */

}

// 多继承 略

int main() {
    test1();
    test2_1();
    test2_2();
    test3();
    test4();

    return 0;
}

