#include <iostream>

using namespace std;

class Entity {
private:
    int x;
private:
    void print() const {
        std::cout << x;
    }
};

int main() {

    // Experimenting raw pointer with primary types
    int results = 0;
    for (size_t i = 0; i < 1000000; i++) {
        // Quick tast to check that data alignment is always happening -
        // We could simply notice that the value of the pointer (memory address to which the pointer is refering) is always of multiple of 8 = 3^2
        // the first three bits are always zeros and we can consider this a free space in the pointer value itself that could be used for tagging
        int x = 12;
        int *ptr = new int(x);
        auto k = reinterpret_cast<std::uintptr_t>(ptr);
        results += k % 8;
        delete ptr;
    }
    // If my assumption is correct result should always be 0

    cout << results << endl;
    // Experimenting raw pointer with used-defined types
    int results1 = 0;
    for (size_t i = 0; i < 1000000; i++) {
        int x = 12;
        Entity *ptr = new Entity();
        auto k = reinterpret_cast<std::uintptr_t>(ptr);
        results1 += k % 8;
        delete ptr;
    }
    // If my assumption is correct result should always be 0
    cout << results1 << endl;

    // Experimenting shared pointer
    int results2 = 0;
    for (size_t i = 0; i < 1000000; i++) {
        std::shared_ptr<Entity> entity_ptr = std::make_shared<Entity>();
        auto k = reinterpret_cast<std::uintptr_t>(entity_ptr.get());
        //cout << entity_ptr.use_count() << endl;
        results2 += k % 8;
    }
    // If my assumption is correct results2 should always be 0
    cout << results2 << endl;

    std::shared_ptr<Entity> entity_ptr = std::make_shared<Entity>();

    std::cout << entity_ptr.get() << std::endl;

    Entity* rawPtr = entity_ptr.get();
    std::cout << rawPtr << std::endl;
    return 0;
}
