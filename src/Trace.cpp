#include "Trace.hpp"

using std::make_unique;
using std::move;
using std::pair;
using std::string;
using std::unordered_map;
using std::valarray;
using std::vector;

namespace sedaman {
class Trace::Header::Impl {
public:
    Impl(vector<char> const& h, unordered_map<string, pair<int, ValueType>> const& hdr_map)
        : d_data(h)
        , d_hdr_map(hdr_map)
    {
    }
    Impl(vector<char>&& h, unordered_map<string, pair<int, ValueType>>&& hdr_map)
        : d_data(move(h))
        , d_hdr_map(move(hdr_map))
    {
    }
    vector<char> d_data;
    unordered_map<string, pair<int, ValueType>> d_hdr_map;
};

class Trace::Impl {
public:
    Impl(Trace::Header const& h, valarray<double> const& s)
        : d_header(h)
        , d_samples(s)
    {
    }
    Impl(Trace::Header&& h, valarray<double>&& s)
        : d_header(move(h))
        , d_samples(move(s))
    {
    }
    Impl(vector<char> const& h, valarray<double> const& s,
        unordered_map<string, pair<int, Header::ValueType>> const& hdr_map)
        : d_header(h, hdr_map)
        , d_samples(s)
    {
    }
    Impl(vector<char>&& h, valarray<double>&& s,
        unordered_map<string, pair<int, Header::ValueType>>&& hdr_map)
        : d_header(move(h), move(hdr_map))
        , d_samples(move(s))
    {
    }
    Trace::Header d_header;
    valarray<double> d_samples;
};

Trace::Header const& Trace::header() const { return pimpl->d_header; }
valarray<double> const& Trace::samples() const { return pimpl->d_samples; }

Trace::Header::Header(Header const& hdr)
    : pimpl(make_unique<Impl>(hdr.pimpl->d_data, hdr.pimpl->d_hdr_map))
{
}
Trace::Header::Header(Header&& hdr)
    : pimpl(make_unique<Impl>(move(hdr.pimpl->d_data), move(hdr.pimpl->d_hdr_map)))
{
}
Trace::Header::Header(vector<char> const& h,
    unordered_map<string, pair<int, ValueType>> const& hdr_map)
    : pimpl(make_unique<Impl>(h, hdr_map))
{
}
Trace::Header::Header(vector<char>&& h,
    unordered_map<string, pair<int, ValueType>>&& hdr_map)
    : pimpl(make_unique<Impl>(move(h), move(hdr_map)))
{
}
Trace::Header::~Header() = default;

Trace::Header& Trace::Header::operator=(Header const& o)
{
    if (&o != this) {
        pimpl->d_data = o.pimpl->d_data;
        pimpl->d_hdr_map = o.pimpl->d_hdr_map;
    }
    return *this;
}

Trace::Header& Trace::Header::operator=(Header&& o) noexcept
{
    if (&o != this) {
        pimpl->d_data = move(o.pimpl->d_data);
        pimpl->d_hdr_map = move(o.pimpl->d_hdr_map);
    }
    return *this;
}

Trace::Trace(Trace const& t)
    : pimpl(make_unique<Impl>(t.pimpl->d_header, t.pimpl->d_samples))
{
}
Trace::Trace(Trace&& t) noexcept
    : pimpl(make_unique<Impl>(move(t.pimpl->d_header), move(t.pimpl->d_samples)))
{
}
Trace::Trace(vector<char> const& h, valarray<double> const& s,
    unordered_map<string, pair<int, Header::ValueType>> const& hdr_map)
    : pimpl(make_unique<Impl>(h, s, hdr_map))
{
}
Trace::Trace(vector<char>&& h, valarray<double>&& s,
    unordered_map<string, pair<int, Header::ValueType>>&& hdr_map)
    : pimpl(make_unique<Impl>(move(h), move(s), move(hdr_map)))
{
}
Trace::~Trace() = default;

Trace& Trace::operator=(Trace const& o)
{
    if (&o != this) {
        pimpl->d_header = o.pimpl->d_header;
        pimpl->d_samples = o.pimpl->d_samples;
    }
    return *this;
}

Trace& Trace::operator=(Trace&& o) noexcept
{
    if (&o != this) {
        pimpl->d_header = move(o.pimpl->d_header);
        pimpl->d_samples = move(o.pimpl->d_samples);
    }
    return *this;
}
} // namespace sedaman
