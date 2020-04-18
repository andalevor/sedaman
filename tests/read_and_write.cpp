#include "ISegy.hpp"
#include "OSegyRev0.hpp"
#include <exception>
#include <iostream>

int main(int argc, char *argv[])
{
    if (argc < 2)
        return 1;
    try {
        sedaman::ISegy in(argv[1]);
		std::string const &text_header = in.text_headers()[0];
		sedaman::CommonSegy::BinaryHeader const &binary_header = in.binary_header();
        sedaman::OSegyRev0 out(argv[2], text_header, binary_header);
        while (in.has_next()) {
			sedaman::Trace t = in.read_trace();
			out.write_trace(t);
        }
    } catch (std::exception& e) {
        std::cerr << "exception on reading and writing\n" << e.what() << '\n';
        return 1;
    }
    try {
		std::fstream ref(argv[1], std::ios_base::binary | std::ios_base::in);
		std::fstream test(argv[2], std::ios_base::binary | std::ios_base::in);
		while (ref) {
			int first = ref.get();
			int second = test.get();
			if (first != second)
				return 1;
		}
    } catch (std::exception& e) {
        std::cerr << "exception on file comparing\n" << e.what() << '\n';
        return 1;
    }
    return 0;
}
