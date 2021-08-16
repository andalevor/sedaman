#include "OSEGYRev0.hpp"
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

namespace sedaman {
class OSEGYRev0::Impl {
public:
    Impl(OSEGYRev0& s, string txt_hdr);
    function<void(Trace& tr)> write_trace;

private:
    OSEGYRev0& sgy;
    streampos first_trace_pos;
};

OSEGYRev0::Impl::Impl(OSEGYRev0& s, string txt_hdr)
    : sgy { s }
{
    if (txt_hdr.empty()) {
        txt_hdr = string(CommonSEGY::default_text_header,
						 CommonSEGY::TEXT_HEADER_SIZE);
        string to_replace("SEG-Y_REV2.0");
        auto pos = txt_hdr.find(to_replace);
        // there are no revision field in text header in revision 0
        txt_hdr.replace(pos, to_replace.size(),
					   	string(' ', to_replace.size()));
        CommonSEGY::ascii_to_ebcdic(txt_hdr);
    } else if (txt_hdr.size() != CommonSEGY::TEXT_HEADER_SIZE) {
        throw Exception(__FILE__, __LINE__,
					   	"size of text header should be 3200 bytes");
    }
    sgy.common().file.write(txt_hdr.c_str(), CommonSEGY::TEXT_HEADER_SIZE);
    sgy.common().text_headers.emplace_back(move(txt_hdr));
    if (sgy.common().binary_header.format_code == 0)
        sgy.common().binary_header.format_code = 1;
    sgy.assign_raw_writers();
    sgy.write_bin_header();
    sgy.assign_sample_writer();
    sgy.assign_bytes_per_sample();
    CommonSEGY::BinaryHeader zero = {};
    if (memcmp(&sgy.common().binary_header, &zero,
			   sizeof(CommonSEGY::BinaryHeader))) {
        sgy.common().samp_buf.resize(sgy.common().binary_header.samp_per_tr *
									 sgy.common().bytes_per_sample);
        write_trace = [this](Trace& tr) {
            sgy.write_trace_header(tr.header());
            sgy.write_trace_samples_fix(tr);
        };
    } else {
        write_trace = [this](Trace& tr) {
            if (sgy.common().file.tellg() == first_trace_pos) {
                //throw exception if trace header does not has a samples number
                Trace::Header::Value v = *tr.header().get("SAMP_NUM");
                uint32_t samp_num;
                if (holds_alternative<int16_t>(v))
                    samp_num = get<int16_t>(v);
                else
                    samp_num = get<uint32_t>(v);
                if (samp_num > INT16_MAX)
                    throw Exception(__FILE__, __LINE__,
									"the number of samples is too much for "
									"revision 0");
                else
                    sgy.common().binary_header.samp_per_tr = samp_num;
                v = *tr.header().get("SAMP_INT");
                double samp_int;
                if (holds_alternative<int16_t>(v))
                    samp_int = get<int16_t>(v);
                else
                    samp_int = get<double>(v);
                if (samp_int > INT16_MAX || !static_cast<int16_t>(samp_int))
                    throw Exception(__FILE__, __LINE__,
									"the sample interval can not be "
									"written to rev0");
                else
                    sgy.common().binary_header.samp_int = samp_int;
                sgy.common().samp_buf.resize(samp_num *
											 sgy.common().bytes_per_sample);
            }
            sgy.write_trace_header(tr.header());
            sgy.write_trace_samples_fix(tr);
        };
    }
}

void OSEGYRev0::write_trace(Trace& tr)
{
    pimpl->write_trace(tr);
}

OSEGYRev0::OSEGYRev0(string name, string th, CommonSEGY::BinaryHeader bh)
    : OSEGY { move(name), move(bh), {} }
    , pimpl { make_unique<Impl>(*this, move(th)) }
{
}

OSEGYRev0::~OSEGYRev0() = default;
} // namespace sedaman
