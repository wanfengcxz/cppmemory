//
// Created by 晚风吹行舟 on 2023/8/28.
//

#include <iostream>
#include <complex>
#include <memory>                //std::allocator
#include <string>
#include <cassert>

using namespace std;

/**
 * 1-16节
 */

namespace test01 {

    void test_primitives() {
        cout << "\ntest_primitives().......... \n";


        void *p1 = malloc(512);     // crt
        free(p1);


        complex<int> *p2 = new complex<int>();
        delete p2;

        // 底层调用malloc
        // msvc -> Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.29.30133\crt\src\vcruntime\new_scalar.cpp
        /**
         *
                _CRT_SECURITYCRITICAL_ATTRIBUTE
                void* __CRTDECL operator new(size_t const size)
                {
                    for (;;)
                    {
                        if (void* const block = malloc(size))
                        {
                            return block;
                        }

                        if (_callnewh(size) == 0)
                        {
                            if (size == SIZE_MAX)
                            {
                                __scrt_throw_std_bad_array_new_length();
                            }
                            else
                            {
                                __scrt_throw_std_bad_alloc();
                            }
                        }

                        // The new handler was successful; try to allocate again...
                    }
                }
         */


        void *p3 = ::operator new(512);
        ::operator delete(p3);

#ifdef  _MSC_VER
        //以下兩函數都是 non-static，定要通過 object 調用。以下分配 3 個 ints.
        int *p4 = allocator<int>().allocate(3, (int *) 0);
        allocator<int>().deallocate(p4, 3);
#endif

#ifdef __BORLANDC__
        //以下兩函數都是 non-static，定要通過 object 調用。以下分配 5 個 ints.
    int* p4 = allocator<int>().allocate(5);
    allocator<int>().deallocate(p4,5);
#endif

#ifdef __GNUC__
        // allocator继承new_allocator，allocate这个方法就是在new_allocator中
        void *p4 = allocator<int>().allocate(7);
        allocator<int>().deallocate((int *) p4, 7);


        // 此处allocate底层调用::operator new
        void *p5 = __gnu_cxx::new_allocator<int>().allocate(9);
        __gnu_cxx::new_allocator<int>().deallocate((int *) p5, 9);
#endif
    }
}
namespace test02 {

    void test_new_and_delete() {
        cout << "\ntest_new_and_delete().......... \n";


        complex<int> *p1 = new complex<int>(1, 2);
        delete p1;

        // 等价于 ==>
        complex<int> *p2 = static_cast<complex<int> *>(
                ::operator new(sizeof(complex<int>)));  // 分配内存
        new(p2)complex<int>(1, 2);                      // 调用构造函数

        // gcc11不能通过 msvc14可以通过
//        p2->complex<int>::complex(1, 2);

        // 析构函数都可以被直接调用
        p2->complex<int>::~complex();        // 调用析构函数
//    p2->~complex<int>();    // 这样写也可
//    p2->~complex();    // 这样写也可

        ::operator delete(p2);      // 收回内存

        // --------------------- 总结------------------------
        // gcc比较严格 不能直接调用构造函数 只能使用placement new来做平替
        // msvc则可以

        /** 四种new
         * new expression
         * array new
         * operator new
         * placement new
         */

        int *p4 = new int;
        delete p4;

        int *p5 = new int[3];
        delete[]p5;

        int *p6 = static_cast<int *>(::operator new(4));
//    ::operator delete(p6);

        new(p6)int(2);
    }

}

namespace test03 {

    class Demo1 {
    public:
        int *p;

        Demo1() : p(new int[3]{1, 2, 3}) {
            cout << "ctor" << endl;
        }

        ~Demo1() {
            cout << "d ctor" << endl;
            delete[] p;
        }
    };

    class Demo2 {
    public:

        int x;

        Demo2() : x(2) {
            cout << "ctor" << endl;
        }

        ~Demo2() {
            cout << "d ctor" << endl;
        }
    };

    class Demo3 {
    public:

        int x;

        Demo3() : x(2) {
        }

    };

    void test_array_new_and_delete() {
        cout << "\ntest_array_new_and_delete().......... \n";


        Demo2 *p1 = new Demo2[3];
        // delete底层也是先调用析构函数再调用operator delete
        delete[]p1;
        cout << endl;

        // runtime error  gcc/msvc都会出错
//        Demo2 *p2 = new Demo2[3];
//        delete p2;
        // 单纯delete只调用一次析构函数，即控制台只打印了一次 "d ctor"
        // 然后调用operator delete 显然会报错

        // runtime error 和上面一样
//        Demo2 *p2 = new Demo2[3];
//        ::operator delete(p2);

        // 如果析构函数是trivially，那么可以直接调用operator delete
        Demo3 *p3 = new Demo3[3];
        delete p3;

        Demo3 *p4 = new Demo3[3];
        ::operator delete(p4);

        // runtime error
//        Demo1 *p5 = new Demo1[2];
//        delete p5;

        Demo1 *p6 = new Demo1[2];
        delete[]p6;

        /**
         * 总结
         * 1. 对于class X，若其析构函数是trivially（意思是无意义的）
         * 那么在delete时，就不必要调用析构函数，因此[]加不加都行，甚至可以直接调用::operator delete()
         * 注意c++内置类型默认就是trivially
         * 2. 对于class X，若显示定义了其析构函数（不包括=default），
         * 那么在delete时，需要明确调用析构函数依次析构，再释放这些字节
         * 无论这个析构函数是仅仅打印信息还是释放了一些内存
         */
    }

}

namespace test03 {

    class A {
    public:
        int x;

        A() : x(1) {
            cout << "A default ctor. ";
            cout << "this=" << this << endl;
        }

        ~A() {
            cout << "A d ctor. ";
            cout << "this=" << this << endl;
        }
    };

    void destruct_seq() {
        // 构造时正向调用构造函数，析构时反向调用析构函数
        A *p1 = new A[3];
        delete[]p1;
    }

}

namespace test05 {

    void test_placement_new() {
        cout << "\ntest_placement_new().......... \n";

        // 注意placement new没有对应的placement delete

        // placement new底层也是调用::operator new，但跟之前那个不一样
        complex<int> *p1 = static_cast<complex<int> *>(
                ::operator new(sizeof(complex<int>)));
        new(p1)complex<int>(1, 2);

        complex<int> *p2 = static_cast<complex<int> *>(
                ::operator new(sizeof(complex<int>)));

        // 对于placement new，在底层其实还是调用了operator new + ctor
        // placement new 等价于如下：
        void *p3 = ::operator new(sizeof(complex<int>), p2);    // 这个重载函数直接把p2返回
        complex<int> *p4 = static_cast<complex<int> *>(p3);
//        p4->complex(1,2);


        // placement new在指定地址（内存块）上构建对象 不论是堆内存还是栈内存都可以
        int x = 2;
        int *p5 = &x;
        new(p5)int(3);
        assert(x == 3);
    }

}

// 注意一般不会重载全局::operator new/delete，
// 因为其他程序可能会调用::operator new
//void *operator new(size_t size) {
//    cout << "overload ::operator new" << endl;
//    return malloc(size);
//}
//
//void operator delete(void *ptr) {
//    cout << "overload ::operator delete" << endl;
//    free(ptr);
//}

namespace test06 {

    class Foo {
    public:

        int x;

        Foo() : x(1) {
            cout << "Foo::Foo() ctor. this=" << this << endl;
        }

        explicit Foo(int a) : x(a) {
            cout << "Foo::Foo(int a) ctor. a=" << a << " this=" << this << endl;
        }

        ~Foo() {
            cout << "Foo d ctor. x=" << x << endl;
        }

        // 类内重载new，可以自定义内存分配 一定得是静态函数，所以你不写编译器也会帮你加上
        // 函数内可以调用::operator new（全局），也可以直接使用malloc
        static inline void *operator new(size_t size) {
            cout << "Foo::operator new" << endl;
            return ::operator new(size);
        }

        static inline void operator delete(void *ptr) {
            cout << "Foo::operator delete" << endl;
            ::operator delete(ptr);
        }

        // 同理operator new[]也可以重载

        // 重载placement new
        // 第一个参数必须是size_t

        // 1
        static void *operator new(size_t size, void *start) {
            cout << "Foo::operator new(size_t size, void *start)" << endl;
            return start;
        }

        static void *operator new(size_t size, size_t extra) {
            cout << "Foo::operator new(size_t size, size_t extra)" << endl;
            return malloc(size + extra);
        }

        static void *operator new(size_t size, size_t extra, char init) {
            cout << "Foo::operator new(size_t size, size_t extra, char init)" << endl;
            return malloc(size + extra);
        }

        // 对应1
        static void operator delete(void *ptr, void *start) {
            cout << "Foo::operator delete(void * ptr, void *start)" << endl;
        }

        static void operator delete(void *ptr, size_t extra) {
            cout << "Foo::operator delete(void *ptr, size_t extra)" << endl;
            free(ptr);
        }

        static void operator delete(void *ptr, size_t extra, char init) {
            cout << "Foo::operator delete(void *ptr, size_t extra, char init)" << endl;
            free(ptr);
        }

    };

    void test_overload_operator_new() {
        cout << "\ntest_overload_operator_new().......... \n";


        // 如果重载了全局operator new，那么这里会输出 "overload ::operator new"
        int *p1 = new int;
        delete p1;
        cout << endl << endl;

        // 调用类内重载的operator new
        Foo *p2 = new Foo();
        assert(p2->x == 1);
        delete p2;

        // 也可以通过全局::来强制调用全局new，绕过类内重载的operator new
        // 不过一般没有人这么做
        Foo *p3 = ::new Foo();
        ::delete p3;
    }

    void test_overload_placement_new() {
        cout << "\ntest_overload_placement_new().......... \n";

        // 调用placement 的重载

        Foo *p3 = new(4)Foo;
        Foo *next = p3 + 1;
        new(next)Foo(3);
        assert(next->x == 3), assert(p3->x == 1);

        // 释放时两种方法选一种都可以
        p3->~Foo(), next->~Foo();   /** 这样行不写都可以，因为他其实是广义trivially，
                                    * 他的析构函数不会释放内存，因此不调用也是可以的
                                    * 由于此处构建对象时，new已经被我们修改，虽然我们通过特殊手段看似构建了
                                    * 一个Foo[2]的数组，但是它和你通过系统的array new[]得到的数组在内存布局
                                    * 上是有区别的，例如PPT中提到的数组的长度。系统array new[]产生的对象，
                                    * 必须依次调用析构函数后才能释放，除非他是trivially。
                                    * 所以，此处内存释放时，不必严格依次调用析构函数，然后再释放内存，
                                    * 所以不会报错。但是要注意，如果Foo内调用了new，那么它的析构函数也必须依次
                                    * 调用，否咋会内存泄露
                                    */
        Foo::operator delete(p3, 3);
//        delete p3;

        assert(p3 + 1 == next);
        cout << next->x << endl;
        cout << p3->x << endl;
        cout << endl << endl;

        Foo *p4 = new(10, 'a')Foo;
//        delete p4;
        p4->~Foo();
        Foo::operator delete(p4, 10, 'a');
        cout << endl << endl;

        Foo *p5 = new(10)Foo(2);
        delete p5;
        cout << endl << endl;

        Foo start(2);
        Foo *p2 = new(&start)Foo(3);
        p2->~Foo();
        Foo::operator delete(p2, &start);
    }
}

namespace version1 {

    class Screen {
    public:
        explicit Screen(int x) : i(x), next(nullptr) {};

        int get() const { return i; }

        static void *operator new(size_t);

        static void operator delete(void *);

    private:
        Screen *next;
        static Screen *freeStore;
        static const int screenChunk;
    private:
        int i;
    };

    Screen *Screen::freeStore = nullptr;
    const int Screen::screenChunk = 24;

    /**
     * 为什么要在类内重载operator new呢？
     * 1. 如果连续调用new Screen，会分配好多小的内存块，而这些内存块由于cookie会浪费大量空间
     * 2. 多次调用new Screen时，底层会调用malloc，而malloc属于crt，会触发系统调用，虽然现在
     * malloc已经比较快了（底层也是一个内存池），但是还是浪费时间。
     * 问题： 此处的实现虽然避免了多次调用malloc，但是增加了next字段，
     * 减少了内存利用率，也没那么完美
     *
     * 此处采用链表方式来构建小型内存池。
     * 为什么不用数组的形式来构建？
     * 如果采用数组的形式，在分配的时候确实十分方便，但是在回收的时候非常不方便，难以维护。
     */
    void *Screen::operator new(size_t size) {
        Screen *p;
        if (freeStore == nullptr) {
            p = freeStore = static_cast<Screen *>(
                    ::operator new(size * screenChunk));
            for (size_t i = 0; i < screenChunk - 1; ++i, ++p)
                p->next = p + 1;
            p->next = nullptr;
        }
        p = freeStore;
        freeStore = freeStore->next;
        return p;
    }

    void Screen::operator delete(void *ptr) {
        auto *p = static_cast<Screen *>(ptr);
        p->next = freeStore;
        freeStore = p;
    }

    void test_screen() {
        cout << "\ntest_screen().......... \n";

        size_t const N = 10;
        Screen *p[N];

//        cout << sizeof (Screen) << endl;     // 16 自动padding

        for (int i = 0; i < N; ++i)
            p[i] = new Screen(i);

        // 输出前10个pointers，比较其间隔
        for (int i = 0; i < 10; ++i)
            cout << p[i] << endl;
        /**
         *
         *  0x2a8bc2228d0
            0x2a8bc2228e0
            0x2a8bc2228f0
            0x2a8bc222900
            0x2a8bc222910
            0x2a8bc222920
            0x2a8bc222930
            0x2a8bc222940
            0x2a8bc222950
            0x2a8bc222960
         */

        for (int i = 0; i < 10; ++i)
            delete p[i];
    }

}

namespace version2 {
    class Airplane {
    private:
        struct AirplaneRep {
            int miles;
            char type;
        };
    private:
        union {
            AirplaneRep rep;
            Airplane *next;
        };
    public:
        int getMiles() { return rep.miles; }

        char getType() { return rep.type; }

        void set(int m, int t) {
            rep.miles = m, rep.type = t;
        }

    public:

        /**
         * 仍有一点小问题：
         * 当通过delete释放内存时，实际上只是把那个内存块挂载到headOfFreeList中，
         * 并没有真正释放，这样一来可能delete之后仍有大量内存由Airplane这个类占据着
         */
        static void *operator new(size_t size);

        static void operator delete(void *ptr, size_t);

    public:
        static const int BLOCK_SIZE;
        static Airplane *headOfFreeList;
    };

    const int Airplane::BLOCK_SIZE = 512;
    Airplane *Airplane::headOfFreeList = nullptr;

    void *Airplane::operator new(size_t size) {
        // 应对有继承的情况
        if (sizeof(Airplane) != size) return ::operator new(size);

        Airplane *p;
        if (headOfFreeList == nullptr) {
            headOfFreeList = p = static_cast<Airplane *>(
                    ::operator new(size * BLOCK_SIZE));
            for (int i = 0; i < BLOCK_SIZE - 1; ++i) {
                p[i].next = &p[i + 1];
            }
            p[BLOCK_SIZE - 1].next = nullptr;
        }
        p = headOfFreeList;
        headOfFreeList = p->next;
        return p;
    }

    void Airplane::operator delete(void *ptr, size_t size) {
        if (sizeof(Airplane) != size) ::operator delete(ptr);

        auto *p = static_cast<Airplane *>(ptr);
        p->next = headOfFreeList;
        headOfFreeList = p;
    }

    void test_airplane() {
        cout << "\ntest_airplane().......... \n";

//        cout << sizeof(Airplane) << endl;    // 8

        size_t const N = 15;
        Airplane *p[N];

        for (int i = 0; i < N; ++i)
            p[i] = new Airplane;

        p[1]->set(1000, 'A');
        p[5]->set(2000, 'B');
        p[9]->set(500000, 'C');
        cout << p[1] << ' ' << p[1]->getType() << ' ' << p[1]->getMiles() << endl;
        cout << p[5] << ' ' << p[5]->getType() << ' ' << p[5]->getMiles() << endl;
        cout << p[9] << ' ' << p[9]->getType() << ' ' << p[9]->getMiles() << endl;

        // 输出前10个pointer，比较其间隔
        for (int i = 0; i < 10; ++i)
            cout << p[i] << endl;

        for (int i = 0; i < N; ++i)
            delete p[i];
    }
}

namespace version3 {

    /**
     * 将重复的内存分配操作抽象封装到class allocator当中
     * 避免重复造轮子
     */
    class allocator {
    private:

        struct obj {
            obj *next;
        };

    public:
        void *allocate(size_t size);

        void deallocate(void *ptr);

        void check();

    private:
        obj *freeHead = nullptr;
        const int CHUNK = 5;
    };

    void allocator::check() {
        obj *p = freeHead;
        int count = 0;

        while (p) {
            cout << p << endl;
            p = p->next;
            count++;
        }
        cout << count << endl;
    }

    void *allocator::allocate(size_t size) {
        assert(size >= 8);

        obj *p;
        if (freeHead == nullptr) {
            freeHead = p = static_cast<obj *>(
                    ::operator new(size * CHUNK));
            for (int i = 0; i < CHUNK - 1; ++i) {
                char *tmp = reinterpret_cast<char *>(p) + size;
                p->next = reinterpret_cast<obj *>(tmp);
                p = p->next;
            }
            p->next = nullptr;
        }
        p = freeHead;
        freeHead = freeHead->next;
        return p;
    }

    void allocator::deallocate(void *ptr) {
        obj *p = reinterpret_cast<obj *>(ptr);
        p->next = freeHead;
        freeHead = p;
    }

    class Foo {
    public:
        int x{}, y{};
        static allocator alloc;
    public:
        Foo() = default;

        Foo(int a) : x(a) {}

        void *operator new(size_t size) { return alloc.allocate(size); }

        void operator delete(void *ptr) { alloc.deallocate(ptr); }
    };

/// 将operator new的定义整理为宏 macro

#define DELCARE_POOL_ALLOC() \
public:     \
    void *operator new(size_t size) { return alloc.allocate(size); }    \
    void operator delete(void *ptr) { alloc.deallocate(ptr); }          \
protected:  \
    static allocator alloc;

#define IMPLEMENT_POOL_ALLOC(class_name) \
allocator class_name::alloc;

    class Goo {
    DELCARE_POOL_ALLOC()
    public:
        int x{}, y{}, z{};

        Goo() = default;

    };

    IMPLEMENT_POOL_ALLOC(Goo)

    void test_static_allocator() {
        cout << "\n\ntest_static_allocator().......... \n";

        Foo *p[30];

        // Foo必须大于8个字节，因为一个指针8个字节
//        cout << "sizeof(Foo)= " << sizeof(Foo) << endl;

        for (int i = 0; i < 6; ++i) {
            p[i] = new Foo(i);
            cout << p[i] << ' ' << p[i]->x << endl;
        }
        Foo::alloc.check();


        for (int i = 0; i < 6; ++i) {
            delete p[i];
        }
        Foo::alloc.check();

        /**
            0x23224c22560 0
            0x23224c22568 1
            0x23224c22570 2
            0x23224c22578 3
            0x23224c22580 4
            0x23224c228d0 5
            0x23224c228d8
            0x23224c228e0
            0x23224c228e8
            0x23224c228f0
            4
            0x23224c228d0
            0x23224c22580
            0x23224c22578
            0x23224c22570
            0x23224c22568
            0x23224c22560
            0x23224c228d8
            0x23224c228e0
            0x23224c228e8
            0x23224c228f0
            10

         *
         */
    }

}

namespace test07 {

    void my_new_handler() {
        cout << "my_new_handler" << endl;
        abort();    // 如果abort()被注释确实会一直输出my_new_handler
    }

    void test_new_handler() {

        // 当operator new没有足够内存可以分配给你时，会抛出异常bad_alloc
        // 可以手动指定不让编译器抛出，那么返回的指针就是nullptr
        int *p1 = new(nothrow)int;

        // 但是在异常抛出之前，会多次调用由用户指定的handler函数
        set_new_handler(my_new_handler);
        int *p2 = new int[10000000000000000];   // gcc会输出my_new_handler

        /**
                C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.29.30133\crt\src\vcruntime\delete_scalar.cpp
                _CRT_SECURITYCRITICAL_ATTRIBUTE
                void* __CRTDECL operator new(size_t const size)
                {
                    for (;;)
                    {
                        if (void* const block = malloc(size))
                        {
                            return block;
                        }

                        if (_callnewh(size) == 0)   // 在for循环内部，会重复调用new_handler函数
                        {
                            if (size == SIZE_MAX)
                            {
                                __scrt_throw_std_bad_array_new_length();
                            }
                            else
                            {
                                __scrt_throw_std_bad_alloc();
                            }
                        }

                        // The new handler was successful; try to allocate again...
                    }
                }
         */

    }
}

namespace test08 {

}

int main() {

//    test03::destruct_seq();
//    test05::test_placement_new();
//    test06::test_overload_operator_new();
//    test06::test_overload_placement_new();
//    version1::test_screen();
//    version2::test_airplane();
//    test07::test_new_handler();
    version3::test_static_allocator();
    return 0;
}

