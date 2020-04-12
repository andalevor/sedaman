#include "ISegy.hpp"
#include "OSegy.hpp"
#include <exception>
#include <iostream>

int main(int argc, char *argv[])
{
    if (argc < 2)
        return 1;
    try {
        sedaman::ISegy in(argv[1]);
        sedaman::OSegy out(argv[2], "rev0");
        while (in.has_next()) {
			sedaman::Trace t = in.read_trace();
			out.write_trace(t);
        }
    } catch (std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
    return 0;
}
