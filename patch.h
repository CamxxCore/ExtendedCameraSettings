#pragma once

#include <iterator>

template <typename T>
struct patch
{
public:
	patch()
	{ }

	patch(T * pPlace, std::vector<T> const& data)
	{
		place = pPlace;
		newData = data;
		originalData = std::vector<T>(place, place + newData.size());
	}

	inline void install()
	{
		std::copy(newData.begin(), newData.end(), stdext::checked_array_iterator<T*>(place, newData.size()));

		active = true;
	}

	inline void remove()
	{
		std::copy(originalData.begin(), originalData.end(), stdext::checked_array_iterator<T*>(place, originalData.size()));

		active = false;
	}

public:
	BYTE * place;
	std::vector<T> newData, originalData;
	bool active;
};

typedef patch<BYTE> bytepatch_t;