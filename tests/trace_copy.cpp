#include "trace.hpp"

using std::vector;
using sedaman::trace;

vector<int32_t> make_header() {
    vector<int32_t> v(96);
    for (size_t i = 0; i < v.size(); ++i)
        v[i] = i;
    return v;
}

vector<double> make_samples() {
    vector<double> v(3000);
    for (size_t i = 0; i < v.size(); ++i)
        v[i] = i;
    return v;
}

int main()
{
    vector<int32_t> h = make_header();
    vector<double> s = make_samples();
    trace t(h, s);
    t.header();
    t.samples();
}
