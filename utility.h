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
	return (int)((1.0f / (max - min)) * (value - min) * 10);
}

inline float ScalarToFloat(int scalar, float min, float max)
{
	return ((max - min) * (scalar / 10.0f)) + min;
}
