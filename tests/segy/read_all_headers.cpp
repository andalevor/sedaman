#include "ISEGY.hpp"
#include <exception>
#include <iostream>

int main(int argc, char *argv[])
{
    if (argc < 2)
        return 1;
    int32_t val = 1;
    try
    {
        sedaman::ISEGY segy(argv[1]);
        while (segy.has_trace())
        {
            sedaman::Trace::Header hdr = segy.read_header();
            std::optional<sedaman::Trace::Header::Value> opt =
			   	hdr.get("TRC_SEQ_LINE");
            sedaman::Trace::Header::Value v;
            if (opt) {
                v = *opt;
			} else {
				std::cerr << "No such header\n";
                return 1;
			}
            if (std::get<int64_t>(v) != val++)
            {
                std::cerr << std::get<int64_t>(v) << " not equal to "
				   	<< val - 1 << '\n';
                return 1;
            }
        }
    }
    catch (std::exception &e)
    {
        std::cerr << e.what() << '\n';
        return 1;
    }
    return 0;
}
