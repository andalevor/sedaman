#include "ISegy.hpp"
#include <iostream>
#include <string>

using sedaman::CommonSegy;
using sedaman::ISegy;
using std::cout;
using std::string;

int main(int argc, char* argv[])
{
    if (argc < 2)
        return 1;
    ISegy segy(argv[1]);
    string txt_hdr = segy.text_headers()[0];
    CommonSegy::ebcdic_to_ascii(txt_hdr);
    for (decltype(txt_hdr.size()) i = 0; i < txt_hdr.size(); ++i) {
        cout << txt_hdr[i];
        if ((i + 1) % 80 == 0)
            cout << "\n";
    }
}
