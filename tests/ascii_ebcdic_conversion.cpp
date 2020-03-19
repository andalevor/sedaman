#include "common_segy.hpp"

using sedaman::common_segy;
using std::string;

int main()
{
    //string ascii = common_segy::ebcdic_to_ascii(common_segy::ascii_to_ebcdic(common_segy::default_text_header));
    string ascii(common_segy::default_text_header, common_segy::TEXT_HEADER_SIZE);
    string ebcdic = ascii;
    common_segy::ascii_to_ebcdic(ebcdic);
    string ascii_converted = ebcdic;
    common_segy::ebcdic_to_ascii(ascii_converted);
    if (ascii != ascii_converted)
        return 1;
}
