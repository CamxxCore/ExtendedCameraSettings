#pragma once

class AddressPool
{
private:
	std::map<std::string, uintptr_t> addressMap;
public:
	void insert(std::string key, uintptr_t address);
	uintptr_t & operator[](std::string key);
};

class AddressMgr
{
private:
	std::map<std::string, AddressPool*> items;

public:
	void clear();
	size_t size();
	void insert(std::string category, std::string key, uintptr_t address);
	AddressPool*& get(std::string category);
	AddressPool*& getOrCreate(std::string category);
	~AddressMgr();
};
