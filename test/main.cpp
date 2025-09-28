
#include <iostream>

#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest.h"

int main(int argc, char **argv)
{
    doctest::Context context;
    int res = context.run();
    if (context.shouldExit())
        return res;
    std::cout << "done with result: " << res << "!" << std::endl;
    return res;
}