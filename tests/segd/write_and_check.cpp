#include "ISEGY.hpp"
#include "OSEGDRev2_1.hpp"
#include <iostream>

int main(int argc, char** argv)
{
    if (argc < 3)
        return 1;
    try {
        sedaman::ISEGY sgy(argv[1]);
        sedaman::OSEGDRev2_1 sgd(argv[2]);
        while (sgy.has_trace()) {
            auto trace = sgy.read_trace();
        }
    } catch (std::exception& e) {
        std::cerr << e.what() << '\n';
    }
}
