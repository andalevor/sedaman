#include "util.hpp"

using sedaman::from_bcd;
using sedaman::to_bcd;

int main()
{
    char buf[64];
    char* ptr = buf;
    to_bcd(&ptr, 243, false, 3);
    char const* cptr = buf;
    int result = from_bcd<int>(&cptr, false, 3);
    if (result != 243)
        return 1;
}
