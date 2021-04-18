#include "BoardFuncs.h"

namespace BoardFuncs
{
    char getSquareIndex(const char* sqr)
    {
        // char 'a' equals to 97, followed by 'b'=97, 'c'=99...
        char file = sqr[0] - (char)97;
        // char '0' equals to 48, followed by '1'=49, '2'=50...
        char rank = sqr[1] - (char)49;
        return 8 * rank + file;
    }

    std::string squareToString(char file, char rank) 
    {
        std::string square(2, 0);
        square[0] = file + 97;
        square[1] = file + 49;
        return square;
    }

    std::string squareToString(char square)
    {
        return squareToString(square % 8, square / 8);
    }
}