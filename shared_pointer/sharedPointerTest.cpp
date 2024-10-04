#include "sharedPointer.hpp"

class MyClass : public EnableSharedFromThis<MyClass> {
public:
    int age;
    const char* name;
    explicit MyClass(int age_, const char* name_) : age(age_), name(name_) {
        std::cout << "Construct:" << " name: " << name << std::endl;
    };
    void func() {
        std::cout << "shared_from_this addr: " << (void*)shared_from_this().get() << std::endl;
    }
    ~MyClass() {
        std::cout << "Deconstruct:" << " name: " << name <<
                  " this: " << this << std::endl;
    }
};

class MyClassDerived : public MyClass {
public:
    explicit MyClassDerived(int age, const char* name) : MyClass(age, name) {
        std::cout << "MyClass Derived Construct" << " name: " << name << std::endl;
    };

    ~MyClassDerived() {
        std::cout << "MyClass Derived Destruct" << " name: " << name <<
                " this: " << this << std::endl;
    }
};

int main() {
    std::cout << "Demonstrating... " << std::endl;
    SharedPointer<MyClass> p0 = makeShared<MyClass>(12, "class_1");
    SharedPointer<MyClass> p1(new MyClass(19, "class_2"), [](MyClass* p) { delete p; });
    SharedPointer<MyClass> p2 = p0;
    UniquePointer<MyClass> pu = makeUnique<MyClass>(13, "class_3");
    std::cout << "--------------------------------" << std::endl;

    p2->func();

    std::cout << "--------------------------------" << std::endl;
    std::cout << "p0.get(): " << p0.get() << std::endl;
    std::cout << "p1.get(): " << p1.get() << std::endl;
    std::cout << "p2.get(): " << p2.get() << std::endl;
    std::cout << "pu.get(): " << pu.get() << std::endl;

    /** 从UniquePointer转为SharedPointer, 完成后UniquePointer对象会被自动析构 */
    std::cout << "--------------------------------" << std::endl;
    UniquePointer<MyClass> unique = makeUnique<MyClass>(10, "class_4");
    std::cout << "unique.get(): " << unique.get() << std::endl;
    SharedPointer<MyClass> shared(std::move(unique));
    std::cout << "unique.get(): " << unique.get() << std::endl;
    std::cout << "shared.get(): " << shared.get() << std::endl;
    std::cout << "shared.use_count(): " << shared.use_count() << std::endl;
    std::cout << "--------------------------------" << std::endl;

    p2 = p1; // 拷贝赋值

    std::cout << "age: " << staticPointerCast<MyClassDerived>(p0).operator*().age << std::endl;
    std::cout << "name: " << staticPointerCast<MyClassDerived>(p0).operator*().name << std::endl;
    std::cout << "--------------------------------" << std::endl;
    std::cout << "p0: " << &p0 << std::endl;
    std::cout << "p1: " << &p1 << std::endl;
    std::cout << "p2: " << &p2 << std::endl;

    std::cout << "--------------------------------" << std::endl;
    std::cout << "p0.get(): " << p0.get() << std::endl;
    std::cout << "p1.get(): " << p1.get() << std::endl;
    std::cout << "p2.get(): " << p2.get() << std::endl;
    std::cout << "--------------------------------" << std::endl;

    SharedPointer<MyClass> p3(new MyClassDerived(12, "class_5"));
    std::cout << "p3.get(): " << p3.get() << std::endl;
    auto derived = staticPointerCast<MyClassDerived>(p3);
    std::cout << "p3.get(): " << p3.get() << std::endl;
    std::cout << "derived.get(): " << derived.get() << std::endl;
    SharedPointer<MyClass const> base = p3;
    p3 = constPointerCast<MyClass>(base);
    std::cout << "base.get(): " << base.get() << std::endl;
    std::cout << "p3.get(): " << p3.get() << std::endl;
    std::cout << "--------------------------------" << std::endl;

    return 0;
}