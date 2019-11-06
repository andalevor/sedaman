#include "trace.hpp"

using std::make_unique;
using std::move;
using std::valarray;

namespace sedaman {
class trace::impl {
public:
    impl(const valarray<int32_t> &h, const valarray<double> &s)
        : d_header(h), d_samples(s) {}
    impl(valarray<int32_t> &&h, valarray<double> &&s)
        : d_header(move(h)), d_samples(move(s)) {}
    valarray<int32_t> d_header;
    valarray<double> d_samples;
};

trace::trace(const trace &t)
    : pimpl(make_unique<impl>(t.pimpl->d_header, t.pimpl->d_samples)) {}
trace::trace(trace &&t) noexcept
    : pimpl(move(t.pimpl)) {}
trace::trace(const valarray<int32_t> &h, const valarray<double> &s)
    : pimpl(make_unique<impl>(h, s)) {}
trace::trace(valarray<int32_t> &&h, valarray<double> &&s)
    : pimpl(make_unique<impl>(move(h), move(s))) {}
trace::~trace() = default;

trace &trace::operator=(const trace &o)
{
    if (&o != this)
    {
        pimpl->d_header = o.pimpl->d_header;
        pimpl->d_samples = o.pimpl->d_samples;
    }
    return *this;
}

trace &trace::operator=(trace &&o)
{
    if (&o != this)
    {
        pimpl->d_header = move(o.pimpl->d_header);
        pimpl->d_samples = move(o.pimpl->d_samples);
    }
    return *this;
}

valarray<int32_t> const &trace::header() {return pimpl->d_header;}
valarray<double> const &trace::samples() {return pimpl->d_samples;}
} // namespace sedaman
