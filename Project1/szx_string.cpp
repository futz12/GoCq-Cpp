#include "pch.h"
#include "szx_string.h"

szx_string::szx_string()
{
}

szx_string::szx_string(const std::string& s)
	: std::string(s)
{
}

szx_string::szx_string(const char* s)
	: std::string(s)
{
}

szx_string::~szx_string()
{
}

szx_string& szx_string::operator=(const std::string& s)
{
	// TODO: 在此处插入 return 语句
	std::string::operator=(s);
	return *this;
}

szx_string& szx_string::operator=(const char* s)
{
	// TODO: 在此处插入 return 语句
	std::string::operator=(s);
	return *this;
}

const szx_string& szx_string::replace(const szx_string& sub1, const szx_string& sub2)
{
	// TODO: 在此处插入 return 语句
	for (std::string::size_type pos(0); pos != std::string::npos; pos += sub2.length())
	{
		if ((pos = find(sub1, pos)) != std::string::npos)
			replace(pos, sub1.length(), sub2);
		else break;
	}
	return *this;
}

bool szx_string::begin_with(const szx_string& sub) const
{
	return substr(0, sub.length()) == sub;
}
