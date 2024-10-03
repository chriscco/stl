#include "shared_pointer/sharedPointer.hpp"

class MyClass : public EnableSharedFromThis<MyClass> {
public:
    int age;
    const char* name;
    explicit MyClass(int age_, const char* name_) : age(age_), name(name_) {
        std::cout << "construct:" << " name: " << name << std::endl;
    };

    void func() {
        std::cout << "shared_from_this addr: " << (void*)shared_from_this().get() << std::endl;
    }

    ~MyClass() {
        std::cout << "deconstruct:" << " name: " << name <<
                  " this: " << this << std::endl;
    }
};

class MyClassDerived : public MyClass {
    explicit MyClassDerived(int age, const char* name) : MyClass(age, name) {
        std::cout << "MyClass Derived Construct\n";
    };

    ~MyClassDerived() {
        std::cout << "MyClass Derived Destruct\n";
    }
};

int main() {
    std::cout << "demonstrating... " << std::endl;
    SharedPointer<MyClass> p0 = makeShared<MyClass>(12, "class_1");
    SharedPointer<MyClass> p1(new MyClass(19, "class_2"), [](MyClass* p) { delete p; });
    SharedPointer<MyClass> p2 = p0;
    UniquePointer<MyClass> pu = makeUnique<MyClass>(13, "class_3");
    p2->func();

    std::cout << "--------------------------------" << std::endl;
    std::cout << "p0.get(): " << p0.get() << std::endl;
    std::cout << "p1.get(): " << p1.get() << std::endl;
    std::cout << "p2.get(): " << p2.get() << std::endl;
    std::cout << "pu.get(): " << pu.get() << std::endl;

    p2 = p1; // 拷贝赋值
    SharedPointer<MyClass> pp(pu.get(), [](MyClass* p) { // uniquePointer -> sharedPointer
        // std::cout << "p_shared: " << p->name << std::endl;
        delete p;
    });

    pu.release();

    std::cout << "age: " << staticPointerCast<MyClassDerived>(p0).operator*().age << std::endl;
    std::cout << "name: " << staticPointerCast<MyClassDerived>(p0).operator*().name << std::endl;
    std::cout << "p0: " << &p0 << std::endl;
    std::cout << "p1: " << &p1 << std::endl;
    std::cout << "p2: " << &p2 << std::endl;
    std::cout << "pp: " << &pp << std::endl;

    std::cout << "--------------------------------" << std::endl;
    std::cout << "p0.get(): " << p0.get() << std::endl;
    std::cout << "p1.get(): " << p1.get() << std::endl;
    std::cout << "p2.get(): " << p2.get() << std::endl;
    std::cout << "pp.get(): " << pp.get() << std::endl;

    return 0;
}