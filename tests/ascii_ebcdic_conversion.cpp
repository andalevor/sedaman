#include "CommonSEGY.hpp"

using sedaman::CommonSEGY;
using std::string;

int main()
{
    string ascii(CommonSEGY::default_text_header, CommonSEGY::TEXT_HEADER_SIZE);
    string ebcdic = ascii;
    CommonSEGY::ascii_to_ebcdic(ebcdic);
    string ascii_converted = ebcdic;
    CommonSEGY::ebcdic_to_ascii(ascii_converted);
    if (ascii != ascii_converted)
        return 1;
    return 0;
}
