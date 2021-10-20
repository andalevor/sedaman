#include "ISEGYSorted1D.hpp"
#include "Exception.hpp"
#include <iostream>
using std::make_unique;
using std::map;
using std::move;
using std::optional;
using std::pair;
using std::streampos;
using std::string;
using std::vector;

namespace sedaman {
class ISEGYSorted1D::Impl {
public:
    Impl(ISEGYSorted1D &s, string hdr_name);
    map<Trace::Header::Value, vector<streampos>>::iterator map_cur, map_end;
    vector<streampos>::iterator vec_cur, vec_end;
    map<Trace::Header::Value, vector<streampos>> trc_map;
    Trace::Header::ValueType hdr_type;

private:
    ISEGYSorted1D &sgy;
};

ISEGYSorted1D::Impl::Impl(ISEGYSorted1D &s, string hdr_name) : sgy{s} {
    while (s.ISEGY::has_trace()) {
        streampos pos = s.common().file.tellg();
        Trace::Header hdr = s.ISEGY::read_header();
        optional<Trace::Header::Value> opt_val = hdr.get(hdr_name);
        if (std::nullopt == opt_val)
            throw Exception(__FILE__, __LINE__, "no such header in trace");
        trc_map[*opt_val].push_back(move(pos));
    }
    map_cur = trc_map.begin();
    map_end = trc_map.end();
    vec_cur = map_cur->second.begin();
    vec_end = map_cur->second.end();
    Trace::Header::Value key = map_cur->first;
}

bool ISEGYSorted1D::has_trace() { return pimpl->map_cur != pimpl->map_end; }

Trace::Header ISEGYSorted1D::read_header() {
    common().file.seekg(*pimpl->vec_cur);
    Trace::Header hdr = ISEGY::read_header();
    ++pimpl->vec_cur;
    if (pimpl->vec_cur == pimpl->vec_end) {
        ++pimpl->map_cur;
        if (pimpl->map_cur != pimpl->map_end) {
            pimpl->vec_cur = pimpl->map_cur->second.begin();
            pimpl->vec_end = pimpl->map_cur->second.end();
        }
    }
    return hdr;
}

Trace ISEGYSorted1D::read_trace() {
    common().file.seekg(*pimpl->vec_cur);
    Trace trc = ISEGY::read_trace();
    ++pimpl->vec_cur;
    if (pimpl->vec_cur == pimpl->vec_end) {
        ++pimpl->map_cur;
        if (pimpl->map_cur != pimpl->map_end) {
            pimpl->vec_cur = pimpl->map_cur->second.begin();
            pimpl->vec_end = pimpl->map_cur->second.end();
        }
    }
    return trc;
}

vector<Trace::Header::Value> ISEGYSorted1D::get_keys() {
    vector<Trace::Header::Value> result;
    for (map<Trace::Header::Value, vector<streampos>>::iterator
             cur = pimpl->trc_map.begin(),
             end = pimpl->trc_map.end();
         cur != end; ++cur) {
        result.push_back(cur->first);
    }
    return result;
}

vector<Trace::Header> ISEGYSorted1D::get_headers(Trace::Header::Value v,
                                                 uint64_t max_num) {
    vector<Trace::Header> result;
    map<Trace::Header::Value, vector<streampos>>::iterator mit =
        pimpl->trc_map.find(v);
    if (mit != pimpl->trc_map.end()) {
        for (vector<streampos>::iterator vit = mit->second.begin(),
                                         vend = mit->second.end();
             vit != vend && result.size() != max_num; ++vit) {
            common().file.seekg(*vit);
            Trace::Header hdr = ISEGY::read_header();
            result.push_back(move(hdr));
        }
    }
    return result;
}

vector<Trace> ISEGYSorted1D::get_traces(Trace::Header::Value v,
                                        uint64_t max_num) {
    vector<Trace> result;
    map<Trace::Header::Value, vector<streampos>>::iterator mit =
        pimpl->trc_map.find(v);
    if (mit != pimpl->trc_map.end()) {
        for (vector<streampos>::iterator vit = mit->second.begin(),
                                         vend = mit->second.end();
             vit != vend && result.size() != max_num; ++vit) {
            common().file.seekg(*vit);
            Trace trc = ISEGY::read_trace();
            result.push_back(move(trc));
        }
    }
    return result;
}

ISEGYSorted1D::ISEGYSorted1D(
    string file_name, string hdr_name,
    vector<map<uint32_t, pair<string, Trace::Header::ValueType>>> tr_hdr_over,
    vector<pair<string, map<uint32_t, pair<string, Trace::Header::ValueType>>>>
        add_hdr_map)
    : ISEGY(move(file_name), move(tr_hdr_over), move(add_hdr_map)),
      pimpl(make_unique<Impl>(*this, move(hdr_name))) {}

ISEGYSorted1D::ISEGYSorted1D(
    string file_name, string hdr_name, CommonSEGY::BinaryHeader bin_hdr,
    vector<map<uint32_t, pair<string, Trace::Header::ValueType>>> tr_hdr_over,
    vector<pair<string, map<uint32_t, pair<string, Trace::Header::ValueType>>>>
        add_hdr_map)
    : ISEGY(move(file_name), move(bin_hdr), move(tr_hdr_over),
            move(add_hdr_map)),
      pimpl(make_unique<Impl>(*this, move(hdr_name))) {}

ISEGYSorted1D::~ISEGYSorted1D() = default;
} // namespace sedaman