#include "stdinc.h"

MemAddr & MemAddr::operator=(const MemAddr &rhs)
{
	return MemAddr(rhs.addr);
}

MemAddr & MemAddr::operator=(const uintptr_t &rhs)
{
	return MemAddr(rhs);
}

MemAddr & MemAddr::operator+=(const MemAddr &rhs)
{
	addr += rhs.addr;
	return *this;
}

MemAddr & MemAddr::operator+(const MemAddr &rhs)
{
	return MemAddr(rhs.addr + addr);
}

MemAddr & MemAddr::operator+(const uintptr_t &rhs)
{
	return MemAddr(addr + rhs);
}

MemAddr::operator uintptr_t()
{
	return addr;
}