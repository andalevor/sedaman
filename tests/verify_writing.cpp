#include "ISegy.hpp"
#include "OSegyRev0.hpp"
#include <exception>
#include <iostream>

int main(int argc, char* argv[])
{
    if (argc < 3)
        return 1;
    try {
        sedaman::ISegy in(argv[1]);
        std::string const& text_header = in.text_headers()[0];
        sedaman::CommonSegy::BinaryHeader const& binary_header = in.binary_header();
        sedaman::OSegyRev0 out(argv[2], text_header, binary_header);
        while (in.has_next()) {
            sedaman::Trace t = in.read_trace();
            out.write_trace(t);
        }
    } catch (std::exception& e) {
        std::cerr << "exception on reading and writing\n"
                  << e.what() << '\n';
        return 1;
    }
    try {
        std::fstream ref(argv[1], std::ios_base::binary | std::ios_base::in);
        std::fstream test(argv[2], std::ios_base::binary | std::ios_base::in);
        size_t counter = 0;
        while (ref) {
            int first = ref.get();
            int second = test.get();
            if (first != second) {
                std::cout << counter << '\n';
                std::cerr << "reference file does not equal to created\n";
                return 1;
            }
            ++counter;
        }
    } catch (std::exception& e) {
        std::cerr << "exception on file comparing\n"
                  << e.what() << '\n';
        return 1;
    }
    return 0;
}
