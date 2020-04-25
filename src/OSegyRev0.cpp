#include "Exception.hpp"
#include "OSegyRev0.hpp"
#include <ios>
#include <functional>
#include <iostream>
#include <cstring>

using std::ios_base;
using std::function;
using std::get;
using std::holds_alternative;
using std::make_unique;
using std::memcmp;
using std::move;
using std::streampos;
using std::string;

namespace sedaman {
class OSegyRev0::Impl {
public:
	Impl(OSegyRev0 &s, string txt_hdr);
	function<void(Trace &tr)> write_trace;

private:
	OSegyRev0 &sgy;
	streampos first_trace_pos;
};

OSegyRev0::Impl::Impl(OSegyRev0 &s, string txt_hdr)
	: sgy {s}
{
	if (txt_hdr.empty()) {
		txt_hdr = string(CommonSegy::default_text_header, CommonSegy::TEXT_HEADER_SIZE);
		string to_replace("SEG-Y_REV2.0");
		auto pos = txt_hdr.find(to_replace);
		// there are no revision field in text header in revision 0
		txt_hdr.replace(pos, to_replace.size(), string(' ', to_replace.size()));
		CommonSegy::ascii_to_ebcdic(txt_hdr);
	} else if (txt_hdr.size() != CommonSegy::TEXT_HEADER_SIZE) {
		throw Exception(__FILE__, __LINE__, "size of text header should be 3200 bytes");
	}
	sgy.p_file().write(txt_hdr.c_str(), CommonSegy::TEXT_HEADER_SIZE);
	sgy.p_txt_hdrs().push_back(move(txt_hdr));
	if (sgy.p_bin_hdr().format_code == 0)
		sgy.p_bin_hdr().format_code = 1;
	sgy.assign_raw_writers();
	sgy.write_bin_header();
	sgy.assign_sample_writer();
	sgy.assign_bytes_per_sample();
	BinaryHeader zero = {};
	if (memcmp(&sgy.p_bin_hdr(), &zero, sizeof(BinaryHeader))) {
		sgy.p_samp_buf().resize(sgy.p_bin_hdr().samp_per_tr * sgy.p_bytes_per_sample());
		write_trace = [this](Trace &tr) {
			sgy.write_trace_header(tr.header());
			sgy.write_trace_samples(tr);
		};
	} else {
		write_trace = [this](Trace &tr) {
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
	}
}

void OSegyRev0::write_trace(Trace &tr)
{
	pimpl->write_trace(tr);
}

OSegyRev0::OSegyRev0(string name, string th, CommonSegy::BinaryHeader bh)
	: OSegy(move(name), move(bh)), pimpl(make_unique<Impl>(*this, move(th)))
{}

OSegyRev0::~OSegyRev0() = default;
}
