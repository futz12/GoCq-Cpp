#pragma once

class szx_string :
    public std::string
{
public:
    szx_string();
    szx_string(const std::string& s);
    szx_string(const char* s);
    ~szx_string();
public:
    using std::string::replace;
    szx_string& operator=(const std::string &s);
    szx_string& operator=(const char* s);
    const szx_string& replace(const szx_string& sub1, const szx_string& sub2);
    bool begin_with(const szx_string& sub) const;
};

