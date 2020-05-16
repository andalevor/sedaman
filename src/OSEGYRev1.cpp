#include "OSEGYRev1.hpp"
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
using std::memcmp;
using std::move;
using std::streampos;
using std::string;
using std::vector;

namespace sedaman {
class OSEGYRev1::Impl {
public:
    Impl(OSEGYRev1& s, vector<string> txt_hdrs);
    function<void(Trace& tr)> write_trace;

private:
    OSEGYRev1& sgy;
    streampos first_trace_pos;
};

OSEGYRev1::Impl::Impl(OSEGYRev1& s, vector<string> txt_hdrs)
    : sgy { s }
{
    if (txt_hdrs.empty()) {
        string txt_hdr = string(CommonSEGY::default_text_header, CommonSEGY::TEXT_HEADER_SIZE);
        string to_replace("SEG-Y_REV2.0");
        auto pos = txt_hdr.find(to_replace);
        // change revision
        txt_hdr.replace(pos, to_replace.size(), string("SEG Y REV1  "));
        txt_hdrs.push_back(move(txt_hdr));
    } else {
        for (string& s : txt_hdrs)
            if (s.size() != CommonSEGY::TEXT_HEADER_SIZE)
                throw Exception(__FILE__, __LINE__, "size of text header should be 3200 bytes");
        sgy.p_txt_hdrs() = txt_hdrs;
    }
    sgy.p_file().write(txt_hdrs[0].c_str(), CommonSEGY::TEXT_HEADER_SIZE);
    if (sgy.p_bin_hdr().format_code == 0)
        sgy.p_bin_hdr().format_code = 5;
    sgy.assign_raw_writers();
    sgy.write_bin_header();
    sgy.write_ext_text_headers();
    sgy.assign_sample_writer();
    sgy.assign_bytes_per_sample();
    BinaryHeader zero = {};
    if (memcmp(&sgy.p_bin_hdr(), &zero, sizeof(BinaryHeader))) {
        if (sgy.p_bin_hdr().fixed_tr_length) {
            sgy.p_samp_buf().resize(sgy.p_bin_hdr().samp_per_tr * sgy.p_bytes_per_sample());
            write_trace = [this](Trace& tr) {
                sgy.write_trace_header(tr.header());
                sgy.write_trace_samples(tr);
            };
        } else {
            write_trace = [this](Trace& tr) {
                sgy.write_trace_header(tr.header());
                sgy.write_trace_samples_var(tr);
            };
        }
    } else {
        if (sgy.p_bin_hdr().fixed_tr_length) {
            write_trace = [this](Trace& tr) {
                if (sgy.p_file().tellg() == first_trace_pos) {
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
                        sgy.p_bin_hdr().samp_per_tr = samp_num;
                    v = *tr.header().get("SAMP_INT");
                    double samp_int;
                    if (holds_alternative<int16_t>(v))
                        samp_int = get<int16_t>(v);
                    else
                        samp_int = get<double>(v);
                    if (samp_int > INT16_MAX || !static_cast<int16_t>(samp_int))
                        throw Exception(__FILE__, __LINE__, "the sample interval can not be written to rev0");
                    else
                        sgy.p_bin_hdr().samp_int = samp_int;
                    sgy.p_samp_buf().resize(samp_num * sgy.p_bytes_per_sample());
                }
                sgy.write_trace_header(tr.header());
                sgy.write_trace_samples(tr);
            };
        } else {
            write_trace = [this](Trace& tr) {
                if (sgy.p_file().tellg() == first_trace_pos) {
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
                        sgy.p_bin_hdr().samp_per_tr = samp_num;
                    v = *tr.header().get("SAMP_INT");
                    double samp_int;
                    if (holds_alternative<int16_t>(v))
                        samp_int = get<int16_t>(v);
                    else
                        samp_int = get<double>(v);
                    if (samp_int > INT16_MAX || !static_cast<int16_t>(samp_int))
                        throw Exception(__FILE__, __LINE__, "the sample interval can not be written to rev0");
                    else
                        sgy.p_bin_hdr().samp_int = samp_int;
                    sgy.p_samp_buf().resize(samp_num * sgy.p_bytes_per_sample());
                }
                sgy.write_trace_header(tr.header());
                sgy.write_trace_samples_var(tr);
            };
        }
    }
}

void OSEGYRev1::write_trace(Trace& tr)
{
    pimpl->write_trace(tr);
}

OSEGYRev1::OSEGYRev1(string name, vector<string> ths, CommonSEGY::BinaryHeader bh)
    : OSEGY(move(name), move(bh), {})
    , pimpl(make_unique<Impl>(*this, move(ths)))
{
}

OSEGYRev1::~OSEGYRev1() = default;
}
