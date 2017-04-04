#pragma once

class MemAddr
{
public:

	MemAddr(uintptr_t address) : addr(address) {}

	MemAddr() : MemAddr(NULL) {}

	operator uintptr_t();

	MemAddr & operator=(const MemAddr &rhs);

	MemAddr & operator=(const uintptr_t &rhs);

	MemAddr & operator+=(const MemAddr &rhs);

	MemAddr & operator+(const MemAddr &rhs);

	MemAddr & operator+(const uintptr_t &rhs);

	inline MemAddr add(MemAddr m)
	{
		return MemAddr(addr + m.addr);
	}

	uintptr_t addr;
};
