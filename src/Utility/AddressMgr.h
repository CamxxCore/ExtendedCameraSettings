#pragma once

#include "MemAddr.h"

class AddressPool
{
public:
	std::map<std::string, MemAddr> map;
	void insert(std::string key, MemAddr address);
	MemAddr & operator[](std::string key);
};

class AddressMgr
{
private:
	std::map<std::string, AddressPool*> items;

public:
	void clear();
	size_t size() const;
	void insert(std::string category, std::string key, MemAddr address);
	AddressPool*& get(std::string category);
	AddressPool*& getOrCreate(std::string category);
	~AddressMgr();
};

extern AddressMgr g_addresses;