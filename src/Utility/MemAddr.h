#pragma once

class MemAddr {
  public:
    MemAddr() : MemAddr(nullptr) {}

    MemAddr(uintptr_t address) : addr(address) {}

    MemAddr(void * ptr) : MemAddr(reinterpret_cast<uintptr_t>(ptr)) {}

    uintptr_t addr;

    operator uintptr_t&() {
        return addr;
    }

    operator uintptr_t() const {
        return addr;
    }

    MemAddr add(MemAddr m) const {
        return *this + m;
    }
};
