#include "ISEGY.hpp"
#include "Trace.hpp"
#include <exception>
#include <iostream>

int main(int argc, char *argv[])
{
    if (argc < 2)
        return 1;
    try
    {
        sedaman::ISEGY segy(argv[1]);
        int counter = 0;
        while (segy.has_trace())
        {
            sedaman::Trace t = segy.read_trace();
            ++counter;
        }
        if (counter != 160)
            return 1;
    }
    catch (std::exception &e)
    {
        std::cout << e.what() << '\n';
        return 1;
    }
    return 0;
}
