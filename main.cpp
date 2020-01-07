#include <iostream>
#include <cstdint>
#include <cassert>

using namespace std;

class Entity {
private:
    int x;
private:
    void print() const {
        std::cout << x;
    }
};

template<typename T, int alignedTo>
class TaggedPointer {
private:
    static_assert(
            alignedTo != 0 && ((alignedTo & (alignedTo - 1)) == 0),
            "Alignment parameter must be power of two"
    );

    // for 8 byte alignment tagMask = alignedTo - 1 = 8 - 1 = 7 = 0b111
    // i.e. the lowest three bits are set, which is where the tag is stored
    static const intptr_t tagMask = alignedTo - 1;

    // pointerMask is the exact contrary: 0b...11111000
    // i.e. all bits apart from the three lowest are set, which is where the pointer is stored
    static const intptr_t pointerMask = ~tagMask;

    // save us some reinterpret_casts with a union
    union {
        T *asPointer;
        intptr_t asBits;
    };

public:
    inline TaggedPointer(T *pointer = 0, int tag = 0) {
        set(pointer, tag);
    }

    inline void set(T *pointer, int tag = 0) {
        // make sure that the pointer really is aligned
        assert((reinterpret_cast<intptr_t>(pointer) & tagMask) == 0);
        // make sure that the tag isn't too large
        assert((tag & pointerMask) == 0);
        // Set the pointer value
        asPointer = pointer;
        // Set the tag as well in the empty section of the pointer
        asBits = asBits | tag;
    }

    inline T *getPointer() const {
        return reinterpret_cast<T *>(asBits & pointerMask);
    }

    inline int getTag() const {
        return asBits & tagMask;
    }
};

#include <cassert>
#include <stdint.h>

template<typename T, int alignedTo>
class StoreIntInTagPointer {
private:
    static_assert(
            alignedTo != 0 && ((alignedTo & (alignedTo - 1)) == 0),
            "Alignment parameter must be power of two"
    );
    static_assert(
            alignedTo > 1,
            "Pointer must be at least 2-byte aligned in order to store an int"
    );

    // for 8 byte alignment tagMask = alignedTo - 1 = 8 - 1 = 7 = 0b111
    // i.e. the lowest three bits are set, which is where the tag is stored
    static const intptr_t tagMask = alignedTo - 1;

    // pointerMask is the exact contrary: 0b...11111000
    // i.e. all bits apart from the three lowest are set, which is where the pointer is stored
    static const intptr_t pointerMask = ~tagMask;

    // save us some reinterpret_casts with a union
    union {
        T *asPointer;
        intptr_t asBits;
    };

public:
    inline StoreIntInTagPointer(T *pointer = 0, int tag = 0) {
        setPointer(pointer, tag);
    }

    inline StoreIntInTagPointer(intptr_t number) {
        setInt(number);
    }

    inline void setPointer(T *pointer, int tag = 0) {
        // make sure that the pointer really is aligned
        assert((reinterpret_cast<intptr_t>(pointer) & tagMask) == 0);
        // make sure that the tag isn't too large
        assert(((tag << 1) & pointerMask) == 0);

        // last bit isn't part of tag anymore, but just zero, thus the << 1
        asPointer = pointer;
        asBits |= tag << 1;
    }

    inline void setInt(intptr_t number) {
        // make sure that when we << 1 there will be no data loss
        // i.e. make sure that it's a 31 bit / 63 bit integer
        assert(((number << 1) >> 1) == number);

        // shift the number to the left and set lowest bit to 1
        asBits = (number << 1) | 1;
    }

    inline T *getPointer() const {
        assert(isPointer());

        return reinterpret_cast<T *>(asBits & pointerMask);
    }

    inline int getTag() const {
        assert(isPointer());

        return (asBits & tagMask) >> 1;
    }

    inline intptr_t getInt() const {
        assert(isInt());

        return asBits >> 1;
    }

    inline bool isPointer() const {
        return (asBits & 1) == 0;
    }

    inline bool isInt() const {
        return (asBits & 1) == 1;
    }
};

int main() {
    // Experimenting raw pointer with primary types

    // The enumeration type has an underlying type for it
    // The underlying type is int, how do I know? just ask C++ for that
    // You can't print the int of the enum that simple cout << shape << "\n" ;
    // instead
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

    /*std::cout << entity_ptr.get() << std::endl;

    Entity *rawPtr = entity_ptr.get();
    std::cout << rawPtr << std::endl;*/

    std::cout << "---------------------------------------\n";
    std::cout << "Pointer Tagging:\n";
    std::cout << "---------------------------------------\n";
    int alignedTo = 4;


    double *intptr = new double{123};
    double number = 12;

    std::cout << "Address Before Tagging: " << &number << std::endl;
    TaggedPointer<double, 8> taggedPointer(&number, 2);


    cout << "Address After Tagging: " << taggedPointer.getPointer() << endl; // == &number
    //cout << &number << endl;
    cout << "Tag Value: " << taggedPointer.getTag() << endl;

    std::cout << "---------------------------------------\n";
    std::cout << "Pointer Tagging with integer:\n";
    std::cout << "---------------------------------------\n";
    double newNumber = 17;
    cout << "Storing Pointer Value: " << &newNumber << endl;
    auto k = reinterpret_cast<std::uintptr_t>(&newNumber);
    const char* c = k % 8 == 0? "is aligned " : "is not aligned ";
    cout << c << endl;
    StoreIntInTagPointer<double, 8> StoreIntInTagPointer(&newNumber, 3);
    std::cout << StoreIntInTagPointer.isPointer() << endl; // == true;
    cout << StoreIntInTagPointer.getPointer() << endl; // == &number
    cout << StoreIntInTagPointer.getTag() << endl; // == 3

    StoreIntInTagPointer.setInt(123456789);
    cout << StoreIntInTagPointer.isInt() << endl; // == true
    cout << StoreIntInTagPointer.getInt() << endl; // == 123456789

    return 0;
}
