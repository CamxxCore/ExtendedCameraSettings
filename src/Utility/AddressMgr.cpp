#include "stdafx.h"

void AddressPool::insert(std::string key, MemAddr address)
{
	map.insert(std::make_pair(key, address));
}

MemAddr & AddressPool::operator[](std::string key)
{
	return map[key];
}

void AddressMgr::clear()
{
	items.clear();
}

size_t AddressMgr::size() const
{
	return items.size();
}

void AddressMgr::insert(std::string key, std::string addressName, MemAddr address)
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
	for (auto it = items.begin(); it != items.end(); ++it)
	{
		delete it->second;
	}
}
