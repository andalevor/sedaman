#include "ISegy.hpp"
#include <cassert>
#include <exception>
#include <iostream>

int main(int argc, char *argv[])
{
    if (argc < 2)
        return 1;
    int32_t val = 3861;
    try {
        sedaman::ISegy segy(argv[1]);
        while (segy.has_next()) {
            sedaman::Trace::Header hdr = segy.read_header();
            sedaman::Trace::Header::Value v = hdr.get("TRC_SEQ_LINE");
            assert(std::get<int32_t>(v) == val++);
        }
    } catch (std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
    return 0;
}
