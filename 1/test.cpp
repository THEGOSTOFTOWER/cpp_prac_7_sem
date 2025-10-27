#include "src/smart_pointer.h"


class Test {
public:
    Test() { std::cout << "Test объект создан" << std::endl; }
    ~Test() { std::cout << "Test объект уничтожен" << std::endl; }

    void show() { std::cout << "Test::show()" << std::endl; }
};

int main() {
    
        SmartPointer<Test> p1(new Test());
        p1->show();
        (*p1).show();
        SmartPointer<Test> p2 = p1;


        std::cout << "p1 == p2: " << (p1 == p2) << std::endl;
        std::cout << p2.use_count() << std::endl;

        p2.reset(new Test());
        p2->show();
        std::cout << p2.use_count() << std::endl;

        std::cout << "p1 == p2: " << (p1 == p2) << std::endl;
    


    return 0;
}