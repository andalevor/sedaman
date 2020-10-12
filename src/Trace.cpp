#include "Trace.hpp"

using std::make_unique;
using std::move;
using std::nullopt;
using std::optional;
using std::pair;
using std::string;
using std::unordered_map;
using std::valarray;
using std::vector;

namespace sedaman {
class Trace::Header::Impl {
public:
    Impl(unordered_map<string, Header::Value> hdr)
        : d_hdr { move(hdr) }
    {
    }
    unordered_map<string, Header::Value> d_hdr;
};

class Trace::Impl {
public:
    Impl(Trace::Header h, valarray<double> s)
        : d_header { move(h) }
        , d_samples { move(s) }
    {
    }
    Impl(unordered_map<string, Header::Value> hdr, valarray<double> s)
        : d_header { move(hdr) }
        , d_samples { move(s) }
    {
    }
    Trace::Header d_header;
    valarray<double> d_samples;
};

Trace::Header const& Trace::header() const { return pimpl->d_header; }
valarray<double> const& Trace::samples() const { return pimpl->d_samples; }

optional<Trace::Header::Value> Trace::Header::get(string key) const
{
    auto it = pimpl->d_hdr.find(move(key));
    return it == pimpl->d_hdr.end() ? nullopt : optional<Value>(it->second);
}
void Trace::Header::set(string key, Value v) { pimpl->d_hdr[move(key)] = v; }

vector<string> Trace::Header::keys() const
{
    vector<string> result;
    for (auto it = pimpl->d_hdr.cbegin(),
              end = pimpl->d_hdr.cend();
         it != end; ++it)
        result.push_back(it->first);
    return result;
}

Trace::Header::Header(Header const& hdr)
    : pimpl { make_unique<Impl>(hdr.pimpl->d_hdr) }
{
}

Trace::Header::Header(Header&& hdr)
    : pimpl { make_unique<Impl>(move(hdr.pimpl->d_hdr)) }
{
}

Trace::Header::Header(unordered_map<string, Value> hdr)
    : pimpl { make_unique<Impl>(move(hdr)) }
{
}

Trace::Header::~Header() = default;

Trace::Header& Trace::Header::operator=(Header const& o)
{
    if (&o != this)
        pimpl->d_hdr = o.pimpl->d_hdr;
    return *this;
}

Trace::Header& Trace::Header::operator=(Header&& o) noexcept
{
    if (&o != this)
        pimpl->d_hdr = move(o.pimpl->d_hdr);
    return *this;
}

Trace::Trace(Trace const& t)
    : pimpl { make_unique<Impl>(t.pimpl->d_header, t.pimpl->d_samples) }
{
}

Trace::Trace(Trace&& t) noexcept
    : pimpl { make_unique<Impl>(move(t.pimpl->d_header), move(t.pimpl->d_samples)) }
{
}

Trace::Trace(unordered_map<string, Header::Value> hdr, valarray<double> s)
    : pimpl { make_unique<Impl>(move(hdr), move(s)) }
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
