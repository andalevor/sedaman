#include "OSEGYRev2.hpp"
#include "Exception.hpp"
#include <cstring>
#include <functional>
#include <ios>
#include <iostream>

using std::function;
using std::get;
using std::holds_alternative;
using std::ios_base;
using std::make_unique;
using std::map;
using std::memcmp;
using std::move;
using std::optional;
using std::pair;
using std::streampos;
using std::string;
using std::vector;

namespace sedaman {
class OSEGYRev2::Impl {
public:
    Impl(OSEGYRev2& s, vector<string> txt_hdrs, vector<string> trlr_stnzs);
    function<void(Trace& tr)> write_trace;

private:
    void set_min_hdrs(Trace& tr);
    OSEGYRev2& sgy;
    streampos first_trace_pos;
};

OSEGYRev2::Impl::Impl(OSEGYRev2& s, vector<string> txt_hdrs, vector<string> trlr_stnzs)
    : sgy { s }
{
    if (txt_hdrs.empty()) {
        string txt_hdr = string(CommonSEGY::default_text_header, CommonSEGY::TEXT_HEADER_SIZE);
        txt_hdrs.push_back(move(txt_hdr));
    } else {
        for (string& s : txt_hdrs)
            if (s.size() != CommonSEGY::TEXT_HEADER_SIZE)
                throw Exception(__FILE__, __LINE__, "size of text header should be 3200 bytes");
        sgy.common().text_headers = move(txt_hdrs);
    }
    if (!trlr_stnzs.empty()) {
        for (string& s : trlr_stnzs)
            if (s.size() != CommonSEGY::TEXT_HEADER_SIZE)
                throw Exception(__FILE__, __LINE__, "size of trailer stanzas should be 3200 bytes");
        sgy.common().trailer_stanzas = move(trlr_stnzs);
    }
    sgy.common().file.write(txt_hdrs[0].c_str(), CommonSEGY::TEXT_HEADER_SIZE);
    if (sgy.common().binary_header.format_code == 0)
        sgy.common().binary_header.format_code = 5;
    sgy.assign_raw_writers();
    sgy.write_bin_header();
    sgy.write_ext_text_headers();
    sgy.assign_sample_writer();
    sgy.assign_bytes_per_sample();
    CommonSEGY::BinaryHeader zero = {};
    if (memcmp(&sgy.common().binary_header, &zero, sizeof(CommonSEGY::BinaryHeader))) {
        if (sgy.common().binary_header.fixed_tr_length) {
            sgy.common().samp_buf.resize(sgy.common().binary_header.samp_per_tr * sgy.common().bytes_per_sample);
            if (sgy.common().binary_header.max_num_add_tr_headers)
                write_trace = [this](Trace& tr) {
                    sgy.write_trace_header(tr.header());
                    sgy.write_additional_trace_headers(tr.header());
                    sgy.write_trace_samples_fix(tr);
                };
            else
                write_trace = [this](Trace& tr) {
                    sgy.write_trace_header(tr.header());
                    sgy.write_trace_samples_fix(tr);
                };
        } else {
            if (sgy.common().binary_header.max_num_add_tr_headers)
                write_trace = [this](Trace& tr) {
                    sgy.write_trace_header(tr.header());
                    sgy.write_additional_trace_headers(tr.header());
                    sgy.write_trace_samples_var(tr);
                };
            else
                write_trace = [this](Trace& tr) {
                    sgy.write_trace_header(tr.header());
                    sgy.write_trace_samples_var(tr);
                };
        }
    } else {
        if (sgy.common().binary_header.fixed_tr_length) {
            if (sgy.common().binary_header.max_num_add_tr_headers)
                write_trace = [this](Trace& tr) {
                    set_min_hdrs(tr);
                    sgy.write_trace_header(tr.header());
                    sgy.write_additional_trace_headers(tr.header());
                    sgy.write_trace_samples_fix(tr);
                };
            else
                write_trace = [this](Trace& tr) {
                    set_min_hdrs(tr);
                    sgy.write_trace_header(tr.header());
                    sgy.write_trace_samples_fix(tr);
                };
        } else {
            if (sgy.common().binary_header.max_num_add_tr_headers)
                write_trace = [this](Trace& tr) {
                    set_min_hdrs(tr);
                    sgy.write_trace_header(tr.header());
                    sgy.write_additional_trace_headers(tr.header());
                    sgy.write_trace_samples_var(tr);
                };
            else
                write_trace = [this](Trace& tr) {
                    set_min_hdrs(tr);
                    sgy.write_trace_header(tr.header());
                    sgy.write_trace_samples_var(tr);
                };
        }
    }
}

void OSEGYRev2::Impl::set_min_hdrs(Trace& tr)
{
    if (sgy.common().file.tellg() == first_trace_pos) {
        // throw exception if trace header does not has a samples number
        Trace::Header::Value v = *tr.header().get("SAMP_NUM");
        uint32_t samp_num;
        if (holds_alternative<int16_t>(v))
            samp_num = get<int16_t>(v);
        else
            samp_num = get<uint32_t>(v);
        if (samp_num > INT16_MAX)
            throw Exception(__FILE__, __LINE__, "the number of samples is too much for revision 0");
        else
            sgy.common().binary_header.samp_per_tr = samp_num;
        v = *tr.header().get("SAMP_INT");
        double samp_int;
        if (holds_alternative<int16_t>(v))
            samp_int = get<int16_t>(v);
        else
            samp_int = get<double>(v);
        if (samp_int > INT16_MAX || !static_cast<int16_t>(samp_int))
            throw Exception(__FILE__, __LINE__, "the sample interval can not be written to rev0");
        else
            sgy.common().binary_header.samp_int = samp_int;
        sgy.common().samp_buf.resize(samp_num * sgy.common().bytes_per_sample);
    }
}

void OSEGYRev2::write_trace(Trace& tr)
{
    pimpl->write_trace(tr);
}

OSEGYRev2::OSEGYRev2(string name, vector<string> ths, CommonSEGY::BinaryHeader bh,
    vector<string> trlr_stnzs,
    vector<pair<string, map<uint32_t, pair<string, CommonSEGY::TrHdrValueType>>>> add_hdr_map)
    : OSEGY(move(name), move(bh), move(add_hdr_map))
    , pimpl { make_unique<Impl>(*this, move(ths), move(trlr_stnzs)) }
{
}

OSEGYRev2::~OSEGYRev2()
{
    common().file.seekg(0, ios_base::end);
    if (common().binary_header.num_of_trailer_stanza)
        write_trailer_stanzas();
}
}
