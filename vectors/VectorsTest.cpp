#include "Vectors.hpp"

template <class T>
void printVector(Vectors<T> const& vec, std::string name = "defaultVector") {
    printf("&Vectors of %s: %p\n", name.c_str(), &vec);
    for (int i = 0; i < vec.size(); i++) {
        std::cout << name << "[" << i << "]" << ": " << vec[i] << std::endl;
    }
}

int main() {
    Vectors<int> arr(15);
    for (int i = 0; i < arr.size(); i++) {
        arr[i] = i;
    }
    arr.erase(arr.begin() + 1, arr.begin() + 5);
    std::cout << "-----------------------------" << std::endl;
    printVector(arr);
    std::cout << "-----------------------------" << std::endl;

    arr.resize(2);
    printVector(arr);
    std::cout << "-----------------------------" << std::endl;

    Vectors<int> bar = arr;
    bar[0] = 10000;
    printVector(bar);
    std::cout << "-----------------------------" << std::endl;

    Vectors<int> car;
    car = bar;
    car.resize(4);
    car[3] = 200;
    printVector(car);
    std::cout << "-----------------------------" << std::endl;

    Vectors<int> this_vec_test;
    std::cout << "This push_back..." << std::endl;
    this_vec_test.reserve(100000);
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < 100000; i++) {
        this_vec_test.push_back(i);
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration_one = end - start;

    std::vector<int> vec_test;
    std::cout << "Standard push_back..." << std::endl;
    vec_test.reserve(100000);
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 100000; i++) {
        vec_test.push_back(i);
    }
    end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration_two = end - start;

    std::cout << "duration_one: " << duration_one.count() << "\n"
              << "duration_two: " << duration_two.count() << std::endl;

    std::cout << "-----------------------------" << std::endl;

    struct S {
        int x, y;
        void print() const {
            std::cout << "Output Struct... x: " << x << " y: " << y << std::endl;
        }
    };
    Vectors<S> vec_struct;
    vec_struct.emplace_back().x = 20;
    for (auto &s: vec_struct) {
        s.print();
    }
    std::cout << "-----------------------------" << std::endl;
    Vectors<int> insert_arr(10);
    insert_arr.insert(insert_arr.begin() + 2, 3,15);
    printVector(insert_arr, "insert_arr");

    std::cout << "-----------------------------" << std::endl;
    std::cout << "sizeof(this_vector): " << sizeof(Vectors<int>) << std::endl;
    std::cout << "sizeof(standard_vector): " << sizeof(std::vector<int>) << std::endl;
    std::cout << "-----------------------------" << std::endl;
    
    arr.resize(0);
    return 0;
}