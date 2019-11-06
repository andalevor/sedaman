#include <iostream>
#include <string>
#include "isegy.hpp"

using sedaman::isegy;
using sedaman::segy;
using std::cout;
using std::string;

int main(int argc, char *argv[])
{
    if (argc < 2)
        return 1;
    isegy segy(argv[1]);
    string ascii_hdr = segy::ebcdic_to_ascii(segy.text_hdr());
    for (decltype (ascii_hdr.size()) i = 0; i < ascii_hdr.size(); ++i) {
        cout << ascii_hdr[i];
        if ((i + 1) % 80 == 0)
            cout << "\n";
    }
}
