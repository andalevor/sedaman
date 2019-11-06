#include <fstream>
#include "isegy.hpp"

using std::ifstream;
using std::make_unique;
using std::move;
using std::string;

namespace sedaman {
class isegy::impl {
public:
    void open_isegy(isegy &segy);
};

void isegy::impl::open_isegy(isegy &segy)
{
    ifstream file;
    file.exceptions(ifstream::failbit | ifstream::badbit);
    file.open(segy.file_name(), ifstream::binary);
    char text_buf[segy::TEXT_HEADER_LEN];
    file.read(text_buf, segy::TEXT_HEADER_LEN);
    segy.set_text_hdr(string(text_buf, segy::TEXT_HEADER_LEN));
}

isegy::isegy(string const &file_name)
    : segy(file_name), pimpl(make_unique<impl>())
{
    pimpl->open_isegy(*this);
}

isegy::isegy(string &&file_name)
    : segy(move(file_name)), pimpl(make_unique<impl>())
{
    pimpl->open_isegy(*this);
}

isegy::~isegy() = default;
}
