#include "CommonSegy.hpp"

using sedaman::CommonSegy;
using std::string;

int main()
{
    //string ascii = CommonSegy::ebcdic_to_ascii(CommonSegy::ascii_to_ebcdic(CommonSegy::default_text_header));
    string ascii(CommonSegy::default_text_header, CommonSegy::TEXT_HEADER_SIZE);
    string ebcdic = ascii;
    CommonSegy::ascii_to_ebcdic(ebcdic);
    string ascii_converted = ebcdic;
    CommonSegy::ebcdic_to_ascii(ascii_converted);
    if (ascii != ascii_converted)
        return 1;
}
