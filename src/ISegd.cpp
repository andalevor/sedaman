#include "ISegd.hpp"

using std::fstream;
using std::make_unique;
using std::move;
using std::string;

namespace sedaman {
class ISegd::Impl {
public:
    Impl(ISegd& s);
    ISegd& sgy;
};

ISegd::Impl::Impl(ISegd& s)
    : sgy(s)
{
}

ISegd::ISegd(string name)
    : CommonSegd(move(name), fstream::in)
    , pimpl(make_unique<Impl>(*this))
{
}

ISegd::~ISegd() = default;
} //sedaman
