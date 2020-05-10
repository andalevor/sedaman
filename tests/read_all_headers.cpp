#include "ISegy.hpp"
#include <cassert>
#include <exception>
#include <iostream>

int main(int argc, char *argv[])
{
    if (argc < 2)
        return 1;
    int32_t val = 1;
    try {
        sedaman::ISegy segy(argv[1]);
        while (segy.has_next()) {
            sedaman::Trace::Header hdr = segy.read_header();
			std::optional<sedaman::Trace::Header::Value> opt = hdr.get("TRC_SEQ_LINE");
			sedaman::Trace::Header::Value v;
			if (opt)
				v = *opt;
			else
				return 1;
            assert(std::get<int32_t>(v) == val++);
        }
    } catch (std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
    return 0;
}
