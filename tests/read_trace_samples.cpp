#include "ISegy.hpp"
#include <cassert>
#include <exception>
#include <iostream>

int main(int argc, char *argv[])
{
    if (argc < 2)
        return 1;
    try {
        sedaman::ISegy segy(argv[1]);
        sedaman::Trace t = segy.read_trace();
        double sum = 0;
        for (auto v: t.samples())
            sum += v;
        assert(sum == 0.00023024588769262472);
    } catch (std::exception& e) {
        std::cout << e.what() << '\n';
    }
    return 0;
}
