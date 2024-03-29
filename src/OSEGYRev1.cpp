#include "OSEGYRev1.hpp"
#include "Exception.hpp"
#include <cstdint>
#include <cstring>
#include <functional>
#include <ios>

using std::function;
using std::get;
using std::holds_alternative;
using std::make_unique;
using std::map;
using std::memcmp;
using std::move;
using std::pair;
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
        string txt_hdr = string(CommonSEGY::default_text_header,
							   	CommonSEGY::TEXT_HEADER_SIZE);
        string to_replace("SEG-Y_REV2.0");
        auto pos = txt_hdr.find(to_replace);
        // change revision
        txt_hdr.replace(pos, to_replace.size(), string("SEG Y REV1  "));
        txt_hdrs.emplace_back(move(txt_hdr));
    } else {
        for (string& s : txt_hdrs)
            if (s.size() != CommonSEGY::TEXT_HEADER_SIZE)
                throw Exception(__FILE__, __LINE__,
							   	"size of text header should be 3200 bytes");
    }
	sgy.common().text_headers = txt_hdrs;
    sgy.common().file.write(txt_hdrs[0].c_str(), CommonSEGY::TEXT_HEADER_SIZE);
    if (sgy.common().binary_header.format_code == 0)
        sgy.common().binary_header.format_code = 5;
    sgy.assign_raw_writers();
    sgy.write_bin_header();
    sgy.write_ext_text_headers();
	first_trace_pos = sgy.common().file.tellg();
    sgy.assign_sample_writer();
    sgy.assign_bytes_per_sample();
    CommonSEGY::BinaryHeader zero = {};
	zero.format_code = 5;
    if (memcmp(&sgy.common().binary_header, &zero,
			   sizeof(CommonSEGY::BinaryHeader))) {
        if (sgy.common().binary_header.fixed_tr_length) {
            sgy.common().samp_buf.resize(static_cast<uint16_t>(
					sgy.common().binary_header.samp_per_tr) *
			   	sgy.common().bytes_per_sample);
            write_trace = [this](Trace& tr) {
                sgy.write_trace_header(tr.header());
                sgy.write_trace_samples_fix(tr);
            };
        } else {
            write_trace = [this](Trace& tr) {
                sgy.write_trace_header(tr.header());
                sgy.write_trace_samples_var(tr);
            };
        }
    } else {
		//if we have empty bin header we can not assume that traces has
		//fixed length
		write_trace = [this](Trace& tr) {
			if (sgy.common().file.tellg() == first_trace_pos) {
				// throw exception if trace header does not has
				// a samples number
				Trace::Header::Value v = *tr.header().get("SAMP_NUM");
				int64_t samp_num = get<int64_t>(v);
				if (samp_num > INT16_MAX)
					throw Exception(__FILE__, __LINE__,
									"the number of samples is too much "
									"for revision 1");
				else
					sgy.common().binary_header.samp_per_tr = samp_num;
				v = *tr.header().get("SAMP_INT");
				double samp_int;
				if (holds_alternative<int64_t>(v))
					samp_int = get<int64_t>(v);
				else
					samp_int = get<double>(v);
				if (samp_int > INT16_MAX ||
					!static_cast<int16_t>(samp_int))
					throw Exception(__FILE__, __LINE__,
									"the sample interval can not be "
									"written to rev1");
				else
					sgy.common().binary_header.samp_int = samp_int;
				sgy.common().samp_buf.resize
					(samp_num * sgy.common().bytes_per_sample);
				sgy.write_bin_header();
			}
			sgy.write_trace_header(tr.header());
			sgy.write_trace_samples_var(tr);
        };
    }
}

void OSEGYRev1::write_trace(Trace& tr)
{
    pimpl->write_trace(tr);
}

OSEGYRev1::OSEGYRev1(string name, vector<string> ths,
					 CommonSEGY::BinaryHeader bh, 
					 vector<pair<string, map<uint32_t,
	   				 pair<string, Trace::Header::ValueType>>>> tr_hdrs_map)
    : OSEGY { move(name), move(bh), move(tr_hdrs_map) }
    , pimpl { make_unique<Impl>(*this, move(ths)) }
{
}

OSEGYRev1::~OSEGYRev1() = default;
} // namespace sedaman
