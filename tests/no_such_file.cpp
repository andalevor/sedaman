#include "ISegy.hpp"
#include <iostream>

int main(int argc, char* argv[])
{
    if (argc < 2)
        return 1;
    try {
        sedaman::ISegy segy(argv[1]);
    } catch (std::exception& e) {
        std::cerr << e.what() << '\n';
        return 0;
    }
    return 1;
}
