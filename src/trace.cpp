#include "trace.hpp"

using std::make_unique;
using std::move;
using std::vector;

namespace sedaman {
class trace::impl {
public:
    impl(vector<int32_t> &h, vector<double> &s)
        : d_header(h), d_samples(s) {}
    impl(vector<int32_t> &&h, vector<double> &&s)
        : d_header(move(h)), d_samples(move(s)) {}
    vector<int32_t> d_header;
    vector<double> d_samples;
};

trace::trace(std::vector<int32_t> &h, std::vector<double> &s)
    : pimpl(make_unique<impl>(h, s)) {}
trace::trace(std::vector<int32_t> &&h, std::vector<double> &&s)
    : pimpl(make_unique<impl>(move(h), move(s))) {}
trace::~trace() = default;

inline vector<int32_t> const &trace::header() {return pimpl->d_header;}
inline vector<double> const &trace::samples() {return pimpl->d_samples;}
} // namespace sedaman
