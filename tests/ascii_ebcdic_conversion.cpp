#include "common_segy.hpp"

using sedaman::common_segy;
using std::string;

int main()
{
    string ascii = common_segy::ebcdic_to_ascii(common_segy::ascii_to_ebcdic(common_segy::default_text_header));
    for (decltype (ascii.size()) i = 0; i < ascii.size(); ++i)
        if (ascii[i] != common_segy::default_text_header[i])
            return 1;
}
