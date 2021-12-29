#pragma once

#include <string>
#include <sstream>
#include <vector>

namespace StringUtil
{
    // Split with custom delimiter, populate given vector.
    template <typename Out>
    void split(const std::string &s, char delim, Out result) 
    {
        std::istringstream iss(s);
        std::string item;
        while (std::getline(iss, item, delim)) {
            *result++ = item;
        }
    }

    // Split with custom delimiter, return vector.
    std::vector<std::string> split(const std::string &s, char delim);

    // Split the string with default delimiter whitespace.
    std::vector<std::string> split(const std::string &s);
}