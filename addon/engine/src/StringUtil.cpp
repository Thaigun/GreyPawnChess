#include "StringUtil.h"

#include <string>
#include <sstream>
#include <vector>
#include <iterator>

namespace StringUtil
{
    // Split with custom delimiter, return vector.
    std::vector<std::string> split(const std::string &s, char delim) 
    {
        std::vector<std::string> elems;
        split(s, delim, std::back_inserter(elems));
        return elems;
    }

    // Split the string with default delimiter whitespace.
    std::vector<std::string> split(const std::string &s)
    {
        std::istringstream iss(s);
        std::vector<std::string> tokens;
        std::copy(std::istream_iterator<std::string>(iss),
                std::istream_iterator<std::string>(),
                back_inserter(tokens));
        return tokens;
    }
}