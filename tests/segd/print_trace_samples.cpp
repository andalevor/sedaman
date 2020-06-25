#include "ISEGD.hpp"
#include <iostream>
#include <iomanip>

int main(int argc, char *argv[])
{
    if (argc < 2)
        return 1;
    try
    {
        sedaman::ISEGD sgd(argv[1]);
        sedaman::Trace trc = sgd.read_trace();
        for (decltype(trc.samples().size()) i = 0;
             i < trc.samples().size(); ++i)
            std::cout << "Sample #: " << std::setw(5) << i << " = "
                      << trc.samples()[i] << '\n';
    }
    catch (std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
}