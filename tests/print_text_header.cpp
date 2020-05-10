#include "ISegy.hpp"
#include <iostream>
#include <string>

int main(int argc, char* argv[])
{
    if (argc < 2)
        return 1;
    try {
        sedaman::ISegy segy(argv[1]);
        std::string txt_hdr = segy.text_headers()[0];
        sedaman::CommonSegy::ebcdic_to_ascii(txt_hdr);
        for (decltype(txt_hdr.size()) i = 0; i < txt_hdr.size(); ++i) {
            std::cout << txt_hdr[i];
            if ((i + 1) % 80 == 0)
                std::cout << "\n";
        }
    } catch (std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
    return 0;
}
