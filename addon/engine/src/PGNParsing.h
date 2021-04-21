#pragma once

#include <string>
#include <vector>

namespace PGNParsing
{
    std::vector<std::string> getMoveList(std::string pgnString);
}