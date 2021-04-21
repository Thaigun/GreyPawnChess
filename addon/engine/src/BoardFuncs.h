#include <string>

namespace BoardFuncs
{
    char getSquareIndex(const char* sqr);
    char getSquareIndex(const std::string& sqr);
    std::string squareToString(char file, char rank);
    std::string squareToString(char square);
}