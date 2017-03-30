#include "stdinc.h"

void AddressPool::insert(std::string key, uintptr_t address)
{
	map.insert(std::make_pair(key, address));
}

uintptr_t & AddressPool::operator[] (std::string key)
{
	return map[key];
}

void AddressMgr::clear()
{
	items.clear();
}

size_t AddressMgr::size()
{
	return items.size();
}

void AddressMgr::insert(std::string key, std::string addressName, uintptr_t address)
{
	get(key)->insert(addressName, address);
}

AddressPool* & AddressMgr::get(std::string key)
{
	return items[key];
}

AddressPool* & AddressMgr::getOrCreate(std::string key)
{
	auto & result = get(key);

	if (!result)
	{
		result = new AddressPool();

		items.insert(std::make_pair(key, result));
	}

	return result;
}

AddressMgr::~AddressMgr()
{
	for (auto it = items.begin(); it != items.end(); it++)
	{
		delete it->second;
	}
}