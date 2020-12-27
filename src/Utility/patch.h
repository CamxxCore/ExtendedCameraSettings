#pragma once

#define JMPREL_8 0xEB // relative jump short 8bits
#define JMPREL_16_32 0xE9
#define JNZREL_8 0x75 // relative jump short if != 0 8bits
#define JZEREL_16_32 0x0F84 // relative jump short if 0
#define JNZREL_16_32 0x0F85 // relative jump short 16/ 32
#define JBEREL_16_32 0x0F86 // relative jump short if < or ==

#define NOP 0x90

#include <iterator>

template <typename T>
struct patch {
    patch(): place(nullptr), active(false)
    { }

    patch(T * pPlace, std::vector<T> const& data): active(false) {
        place = pPlace;
        newData = data;
        originalData = std::vector<T>(place, place + newData.size());
    }

    void install() {
        std::copy(newData.begin(), newData.end(), stdext::checked_array_iterator<T*>(place, newData.size()));

        active = true;
    }

    void remove() {
        std::copy(originalData.begin(), originalData.end(), stdext::checked_array_iterator<T*>(place, originalData.size()));

        active = false;
    }

    BYTE * place;
    std::vector<T> newData, originalData;
    bool active;
};

typedef patch<BYTE> bytepatch_t;
