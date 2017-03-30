#pragma once

inline void split(std::string str, std::string splitBy, std::vector<std::string>& tokens)
{
	tokens.push_back(str);
	size_t splitAt;

	size_t splitLen = splitBy.size();

	std::string frag;

	while (true)
	{
		frag = tokens.back();

		splitAt = frag.find(splitBy);

		if (splitAt == std::string::npos)
		{
			break;
		}

		tokens.back() = frag.substr(0, splitAt);

		tokens.push_back(frag.substr(splitAt + splitLen, frag.size() - (splitAt + splitLen)));
	}
}

inline int FloatToScalar(float value, float min, float max)
{
	if (value < 0)
	{
		max += abs(value);
		value = -value;
		min = 0;
	}

	if (min < 0)
	{
		float diff = abs(min);
		max += diff;
		value += diff;
		min = 0;
	}

	return (int)((10.0f - min) * (value - min) / (max - min));
}

inline float ScalarToFloat(int scalar, float min, float max)
{
	return ((max - min) * (scalar / 10.0f)) + min;
}
