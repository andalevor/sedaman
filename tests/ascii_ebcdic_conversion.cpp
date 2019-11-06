#include "segy.hpp"

using sedaman::segy;
using std::string;

int main()
{
    string ascii = segy::ebcdic_to_ascii(segy::ascii_to_ebcdic(segy::default_text_header));
    for (decltype (ascii.size()) i = 0; i < ascii.size(); ++i)
        if (ascii[i] != segy::default_text_header[i])
            return 1;
}
