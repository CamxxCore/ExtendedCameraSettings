#pragma once

class MemAddr
{
public:
	MemAddr() : MemAddr(nullptr) {}

	MemAddr(uintptr_t address) : addr(address) {}

	MemAddr(void * obj) : MemAddr(reinterpret_cast<uintptr_t>(obj)) {}

	MemAddr add(MemAddr m) const
	{
		return MemAddr(addr + m.addr);
	}

	uintptr_t addr;

	operator uintptr_t();
};
