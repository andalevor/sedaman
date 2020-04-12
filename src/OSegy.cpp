#include "CommonSegy.hpp"
#include "Exception.hpp"
#include "OSegy.hpp"
#include "util.hpp"
#include <variant>
#include <cfloat>
#include <cmath>
#include <climits>
#include <functional>
#include <ios>

using std::get;
using std::ios_base;
using std::fstream;
using std::function;
using std::holds_alternative;
using std::make_unique;
using std::optional;
using std::streampos;
using std::string;
using std::valarray;

namespace sedaman {
class OSegy::Impl {
public:
	Impl(string const& name, string const& revision);
	Impl(string&& name, string&& revision);
	void write_trace_header(Trace::Header const &hdr);
	void write_trace_samples(Trace const &t, uint32_t samp_num);
	~Impl();
	CommonSegy common;
	streampos first_trace_pos;
private:
	function<void(char**, uint8_t)> write_u8;
	function<void(char**, uint8_t)> write_i8;
	function<void(char**, uint16_t)> write_u16;
	function<void(char**, uint16_t)> write_i16;
	function<void(char**, uint32_t)> write_u24;
	function<void(char**, uint32_t)> write_i24;
	function<void(char**, uint32_t)> write_u32;
	function<void(char**, uint32_t)> write_i32;
	function<void(char**, uint64_t)> write_u64;
	function<void(char**, uint64_t)> write_i64;
	function<void(char**, double)> write_sample;
	function<void(char**, float)> write_IEEE_float;
	function<void(char**, double)> write_IEEE_double;
	void init_r0();
	void init_r1();
	void init_r20();
	void assign_raw_writers();
	void assign_sample_writer();
	void write_bin_header();
};

OSegy::Impl::Impl(string const& name, string const& revision)
	: common { name, fstream::out | fstream::binary }
{
	if (revision == "rev0")
		init_r0();
	else if (revision == "rev1")
		init_r1();
	else
		init_r20();
}

OSegy::Impl::Impl(string&& name, string&& revision)
	: common { move(name), fstream::out | fstream::binary }
{
	if (revision == "rev0")
		init_r0();
	else if (revision == "rev1")
		init_r1();
	else
		init_r20();
}

OSegy::Impl::~Impl()
{
	common.file.seekg(0, ios_base::beg);
	common.file.write(common.txt_hdrs[0].c_str(), CommonSegy::TEXT_HEADER_SIZE);
	write_bin_header();
}

void OSegy::Impl::init_r0()
{
	string txt_hdr(CommonSegy::default_text_header, CommonSegy::TEXT_HEADER_SIZE);
	CommonSegy::ascii_to_ebcdic(txt_hdr);
	common.file.write(txt_hdr.c_str(), CommonSegy::TEXT_HEADER_SIZE);
	common.txt_hdrs.push_back(txt_hdr);
	assign_raw_writers();
	common.bin_hdr.format_code = 1;
	assign_sample_writer();
	common.bytes_per_sample = 4;
	write_bin_header();
	first_trace_pos = common.file.tellg();
}

OSegy &OSegy::write_trace(Trace &t)
{
	// throw exception if trace header does not has a samples number
	Trace::Header::Value v = *t.header().get("SAMP_NUM");
	uint32_t samp_num;
	if (holds_alternative<int16_t>(v))
		samp_num = get<int16_t>(v);
	else
		samp_num = get<uint32_t>(v);
	if (pimpl->common.file.tellg() == pimpl->first_trace_pos) {
		if (samp_num > INT16_MAX)
			throw Exception(__FILE__, __LINE__, "the number of samples is too much for revision 0");
		else
			pimpl->common.bin_hdr.samp_per_tr = samp_num;
		Trace::Header::Value v = *t.header().get("SAMP_INT");
		double samp_int;
		if (holds_alternative<int16_t>(v))
			samp_int = get<int16_t>(v);
		else
			samp_int = get<double>(v);
		if (samp_int > INT16_MAX || !static_cast<int16_t>(samp_int))
			throw Exception(__FILE__, __LINE__, "the sample interval con not be written to rev0");
		else
			pimpl->common.bin_hdr.samp_int = samp_int;
	}
	pimpl->write_trace_header(t.header());
	pimpl->write_trace_samples(t, samp_num);
	return *this;
}

void OSegy::Impl::init_r1()
{
}

void OSegy::Impl::init_r20()
{
}

void OSegy::Impl::assign_raw_writers()
{
	write_u8 = [](char** buf, uint8_t val) { write<uint8_t>(buf, val); };
	write_i8 = [](char** buf, int8_t val) { write<int8_t>(buf, val); };
	switch (common.bin_hdr.endianness) {
		case 0x01020304:
			write_u16 = [](char** buf, uint16_t val) { write<uint16_t>(buf, val); };
			write_i16 = [](char** buf, int16_t val) { write<int16_t>(buf, val); };
			write_u24 = [](char** buf, uint32_t val) {
				write<uint16_t>(buf, val);
				write<uint8_t>(buf, val >> 16);
			};
			write_i24 = [](char** buf, int32_t val) {
				write<int16_t>(buf, val);
				write<int8_t>(buf, static_cast<int32_t>(val) >> 16);
			};
			write_u32 = [](char** buf, uint32_t val) { write<uint32_t>(buf, val); };
			write_i32 = [](char** buf, int32_t val) { write<int32_t>(buf, val); };
			write_u64 = [](char** buf, uint64_t val) { write<uint64_t>(buf, val); };
			write_i64 = [](char** buf, int64_t val) { write<int64_t>(buf, val); };
			break;
		case 0:
		case 0x04030201:
			write_u16 = [](char** buf, uint16_t val) { write<uint16_t>(buf, swap(val)); };
			write_i16 = [](char** buf, int16_t val) { write<int16_t>(buf, swap(val)); };
			write_u24 = [](char** buf, uint32_t val) {
				uint32_t tmp = swap(val);
				write<uint16_t>(buf, tmp);
				write<uint8_t>(buf, tmp >> 16);
			};
			write_i24 = [](char** buf, int32_t val) {
				uint32_t tmp = swap(static_cast<uint32_t>(val));
				write<int16_t>(buf, tmp);
				write<int8_t>(buf, tmp >> 16);
			};
			write_u32 = [](char** buf, uint32_t val) { write<uint32_t>(buf, swap(val)); };
			write_i32 = [](char** buf, int32_t val) { write<int32_t>(buf, swap(val)); };
			write_u64 = [](char** buf, uint64_t val) { write<uint64_t>(buf, swap(val)); };
			write_i64 = [](char** buf, int64_t val) { write<int64_t>(buf, swap(val)); };
			break;
		default:
			throw Exception(__FILE__, __LINE__, "unsupported endianness");
	}
	if (FLT_RADIX == 2 && DBL_MANT_DIG == 53) {
		write_IEEE_float = [this](char** buf, float val) {
			uint32_t tmp;
			memcpy(&tmp, &val, sizeof(val));
			write_u32(buf, tmp);
		};
		write_IEEE_double = [this](char** buf, double val) {
			uint64_t tmp;
			memcpy(&tmp, &val, sizeof(val));
			write_u64(buf, tmp);
		};
	} else {
		write_IEEE_float = [this](char** buf, float val) {
			uint8_t sign = val < 0 ? 1 : 0;
			uint8_t exp = log(val) / log(2) + 1 + 127;
			uint32_t fraction = val / pow(2, exp - 127) * pow(2, 23) - 1;
			uint32_t result = sign << 31 | exp << 23 | (fraction & 0x007fffff);
			write_u32(buf, result);
		};
		write_IEEE_double = [this](char** buf, double val) {
			uint64_t sign = val < 0 ? 1 : 0;
			uint64_t exp = static_cast<uint16_t>(log(val) / log(2) + 1 + 1023) & 0x7ff;
			uint64_t fraction = val / pow(2, exp - 1023) * pow(2, 52) - 1;
			uint64_t result = sign << 63 | exp << 52 | (fraction & 0xfffffffffffff);
			write_u64(buf, result);
		};
	}
}

void OSegy::Impl::assign_sample_writer()
{
	switch (common.bin_hdr.format_code) {
		case 1:
			write_sample = [this](char** buf, double val) {
				uint8_t sign = val < 0 ? 1 : 0;
				double abs_val = std::abs(val);
				uint8_t exp = static_cast<uint32_t>(log(abs_val) / log(2) / 4 + 1 + 64) & 0x7f;
				uint32_t fraction = abs_val / pow(16, exp - 64) * pow(2, 24);
				uint32_t result = sign << 31 | exp << 24 | (fraction & 0x00ffffff);
				write_u32(buf, result);
			};
			break;
		case 2:
			write_sample = [this](char** buf, double val) { write_i32(buf, val); };
			break;
		case 3:
			write_sample = [this](char** buf, double val) { write_i16(buf, val); };
			break;
		case 5:
			write_sample = [this](char** buf, double val) { write_IEEE_float(buf, val); };
			break;
		case 6:
			write_sample = [this](char** buf, double val) { write_IEEE_double(buf, val); };
			break;
		case 7:
			write_sample = [this](char** buf, double val) { write_i24(buf, val); };
			break;
		case 8:
			write_sample = [this](char** buf, double val) { write_i8(buf, val); };
			break;
		case 9:
			write_sample = [this](char** buf, double val) { write_i64(buf, val); };
			break;
		case 10:
			write_sample = [this](char** buf, double val) { write_u32(buf, val); };
			break;
		case 11:
			write_sample = [this](char** buf, double val) { write_u16(buf, val); };
			break;
		case 12:
			write_sample = [this](char** buf, double val) { write_u64(buf, val); };
			break;
		case 15:
			write_sample = [this](char** buf, double val) { write_u24(buf, val); };
			break;
		case 16:
			write_sample = [this](char** buf, double val) { write_u8(buf, val); };
			break;
		default:
			throw(Exception(__FILE__, __LINE__, "unsupported format"));
	}
}

void OSegy::Impl::write_bin_header()
{
	char buf[CommonSegy::BIN_HEADER_SIZE] = {0};
	char *ptr = buf;
	write_i32(&ptr, common.bin_hdr.job_id);
	write_i32(&ptr, common.bin_hdr.line_num);
	write_i32(&ptr, common.bin_hdr.reel_num);
	write_i16(&ptr, common.bin_hdr.tr_per_ens);
	write_i16(&ptr, common.bin_hdr.aux_per_ens);
	write_i16(&ptr, common.bin_hdr.samp_int);
	write_i16(&ptr, common.bin_hdr.samp_int_orig);
	write_i16(&ptr, common.bin_hdr.samp_per_tr);
	write_i16(&ptr, common.bin_hdr.samp_per_tr_orig);
	write_i16(&ptr, common.bin_hdr.format_code);
	write_i16(&ptr, common.bin_hdr.ens_fold);
	write_i16(&ptr, common.bin_hdr.sort_code);
	write_i16(&ptr, common.bin_hdr.vert_sum_code);
	write_i16(&ptr, common.bin_hdr.sw_freq_at_start);
	write_i16(&ptr, common.bin_hdr.sw_freq_at_end);
	write_i16(&ptr, common.bin_hdr.sw_length);
	write_i16(&ptr, common.bin_hdr.sw_type_code);
	write_i16(&ptr, common.bin_hdr.sw_ch_tr_num);
	write_i16(&ptr, common.bin_hdr.taper_at_start);
	write_i16(&ptr, common.bin_hdr.taper_at_end);
	write_i16(&ptr, common.bin_hdr.taper_type);
	write_i16(&ptr, common.bin_hdr.corr_traces);
	write_i16(&ptr, common.bin_hdr.bin_gain_recov);
	write_i16(&ptr, common.bin_hdr.amp_recov_meth);
	write_i16(&ptr, common.bin_hdr.measure_system);
	write_i16(&ptr, common.bin_hdr.impulse_sig_pol);
	write_i16(&ptr, common.bin_hdr.vib_pol_code);
	write_i32(&ptr, common.bin_hdr.ext_tr_per_ens);
	write_i32(&ptr, common.bin_hdr.ext_aux_per_ens);
	write_i32(&ptr, common.bin_hdr.ext_samp_per_tr);
	write_IEEE_double(&ptr, common.bin_hdr.ext_samp_int);
	write_IEEE_double(&ptr, common.bin_hdr.ext_samp_int_orig);
	write_i32(&ptr, common.bin_hdr.ext_samp_per_tr_orig);
	write_i32(&ptr, common.bin_hdr.ext_ens_fold);
	write_i32(&ptr, common.bin_hdr.endianness);
	write_u8(&ptr, common.bin_hdr.SEGY_rev_major_ver);
	write_u8(&ptr, common.bin_hdr.SEGY_rev_minor_ver);
	write_i16(&ptr, common.bin_hdr.fixed_tr_length);
	write_i16(&ptr, common.bin_hdr.ext_text_headers_num);
	write_i32(&ptr, common.bin_hdr.max_num_add_tr_headers);
	write_i16(&ptr, common.bin_hdr.time_basis_code);
	write_u64(&ptr, common.bin_hdr.num_of_tr_in_file);
	write_u64(&ptr, common.bin_hdr.byte_off_of_first_tr);
	write_i32(&ptr, common.bin_hdr.num_of_trailer_stanza);
	common.file.write(buf, CommonSegy::BIN_HEADER_SIZE);
}

void OSegy::Impl::write_trace_header(Trace::Header const &hdr)
{
	char *buf = common.hdr_buf;
	optional<Trace::Header::Value> tmp = hdr.get("TRC_SEQ_LINE");
	write_i32(&buf, tmp ? get<int32_t>(*tmp) : 0);
	tmp = hdr.get("TRC_SEQ_SGY");
	write_i32(&buf, tmp ? get<int32_t>(*tmp) : 0);
	tmp = hdr.get("FFID");
	write_i32(&buf, tmp ? get<int32_t>(*tmp) : 0);
	tmp = hdr.get("CHAN");
	write_i32(&buf, tmp ? get<int32_t>(*tmp) : 0);
	tmp = hdr.get("ESP");
	write_i32(&buf, tmp ? get<int32_t>(*tmp) : 0);
	tmp = hdr.get("ENS_NO");
	write_i32(&buf, tmp ? get<int32_t>(*tmp) : 0);
	tmp = hdr.get("SEQ_NO");
	write_i32(&buf, tmp ? get<int32_t>(*tmp) : 0);
	tmp = hdr.get("TRACE_ID");
	write_i16(&buf, tmp ? get<int16_t>(*tmp) : 0);
	tmp = hdr.get("VERT_SUM");
	write_i16(&buf, tmp ? get<int16_t>(*tmp) : 0);
	tmp = hdr.get("HOR_SUM");
	write_i16(&buf, tmp ? get<int16_t>(*tmp) : 0);
	tmp = hdr.get("DATA_USE");
	write_i16(&buf, tmp ? get<int16_t>(*tmp) : 0);
	tmp = hdr.get("OFFSET");
	write_i32(&buf, tmp ? get<int32_t>(*tmp) : 0);
	tmp = hdr.get("R_ELEV");
	write_i32(&buf, tmp ? get<int32_t>(*tmp) : 0);
	tmp = hdr.get("S_ELEV");
	write_i32(&buf, tmp ? get<int32_t>(*tmp) : 0);
	tmp = hdr.get("S_DEPTH");
	write_i32(&buf, tmp ? get<int32_t>(*tmp) : 0);
	tmp = hdr.get("R_DATUM");
	write_i32(&buf, tmp ? get<int32_t>(*tmp) : 0);
	tmp = hdr.get("S_DATUM");
	write_i32(&buf, tmp ? get<int32_t>(*tmp) : 0);
	tmp = hdr.get("S_WATER");
	write_i32(&buf, tmp ? get<int32_t>(*tmp) : 0);
	tmp = hdr.get("R_WATER");
	write_i32(&buf, tmp ? get<int32_t>(*tmp) : 0);
	tmp = hdr.get("ELEV_SCALAR");
	write_i16(&buf, tmp ? get<int16_t>(*tmp) : 0);
	tmp = hdr.get("COORD_SCALAR");
	write_i16(&buf, tmp ? get<int16_t>(*tmp) : 0);
	tmp = hdr.get("SOU_X");
	write_i32(&buf, tmp ? get<int32_t>(*tmp) : 0);
	tmp = hdr.get("SOU_Y");
	write_i32(&buf, tmp ? get<int32_t>(*tmp) : 0);
	tmp = hdr.get("REC_X");
	write_i32(&buf, tmp ? get<int32_t>(*tmp) : 0);
	tmp = hdr.get("REC_Y");
	write_i32(&buf, tmp ? get<int32_t>(*tmp) : 0);
	tmp = hdr.get("COORD_UNITS");
	write_i16(&buf, tmp ? get<int16_t>(*tmp) : 0);
	tmp = hdr.get("WEATH_VEL");
	write_i16(&buf, tmp ? get<int16_t>(*tmp) : 0);
	tmp = hdr.get("SUBWEATH_VEL");
	write_i16(&buf, tmp ? get<int16_t>(*tmp) : 0);
	tmp = hdr.get("S_UPHOLE");
	write_i16(&buf, tmp ? get<int16_t>(*tmp) : 0);
	tmp = hdr.get("R_UPHOLE");
	write_i16(&buf, tmp ? get<int16_t>(*tmp) : 0);
	tmp = hdr.get("S_STAT");
	write_i16(&buf, tmp ? get<int16_t>(*tmp) : 0);
	tmp = hdr.get("R_STAT");
	write_i16(&buf, tmp ? get<int16_t>(*tmp) : 0);
	tmp = hdr.get("TOT_STAT");
	write_i16(&buf, tmp ? get<int16_t>(*tmp) : 0);
	tmp = hdr.get("LAG_A");
	write_i16(&buf, tmp ? get<int16_t>(*tmp) : 0);
	tmp = hdr.get("LAG_B");
	write_i16(&buf, tmp ? get<int16_t>(*tmp) : 0);
	tmp = hdr.get("DELAY_TIME");
	write_i16(&buf, tmp ? get<int16_t>(*tmp) : 0);
	tmp = hdr.get("MUTE_START");
	write_i16(&buf, tmp ? get<int16_t>(*tmp) : 0);
	tmp = hdr.get("MUTE_END");
	write_i16(&buf, tmp ? get<int16_t>(*tmp) : 0);
	tmp = hdr.get("SAMP_NUM");
	write_i16(&buf, tmp ? get<int16_t>(*tmp) : 0);
	tmp = hdr.get("SAMP_INT");
	write_i16(&buf, tmp ? get<int16_t>(*tmp) : 0);
	tmp = hdr.get("GAIN_TYPE");
	write_i16(&buf, tmp ? get<int16_t>(*tmp) : 0);
	tmp = hdr.get("GAIN_CONST");
	write_i16(&buf, tmp ? get<int16_t>(*tmp) : 0);
	tmp = hdr.get("INIT_GAIN");
	write_i16(&buf, tmp ? get<int16_t>(*tmp) : 0);
	tmp = hdr.get("CORRELATED");
	write_i16(&buf, tmp ? get<int16_t>(*tmp) : 0);
	tmp = hdr.get("SW_START");
	write_i16(&buf, tmp ? get<int16_t>(*tmp) : 0);
	tmp = hdr.get("SW_END");
	write_i16(&buf, tmp ? get<int16_t>(*tmp) : 0);
	tmp = hdr.get("SW_LENGTH");
	write_i16(&buf, tmp ? get<int16_t>(*tmp) : 0);
	tmp = hdr.get("SW_TYPE");
	write_i16(&buf, tmp ? get<int16_t>(*tmp) : 0);
	tmp = hdr.get("SW_TAPER_START");
	write_i16(&buf, tmp ? get<int16_t>(*tmp) : 0);
	tmp = hdr.get("SW_TAPER_END");
	write_i16(&buf, tmp ? get<int16_t>(*tmp) : 0);
	tmp = hdr.get("TAPER_TYPE");
	write_i16(&buf, tmp ? get<int16_t>(*tmp) : 0);
	tmp = hdr.get("ALIAS_FILT_FREQ");
	write_i16(&buf, tmp ? get<int16_t>(*tmp) : 0);
	tmp = hdr.get("ALIAS_FILT_SLOPE");
	write_i16(&buf, tmp ? get<int16_t>(*tmp) : 0);
	tmp = hdr.get("NOTCH_FILT_FREQ");
	write_i16(&buf, tmp ? get<int16_t>(*tmp) : 0);
	tmp = hdr.get("NOTCH_FILT_SLOPE");
	write_i16(&buf, tmp ? get<int16_t>(*tmp) : 0);
	tmp = hdr.get("LOW_CUT_FREQ");
	write_i16(&buf, tmp ? get<int16_t>(*tmp) : 0);
	tmp = hdr.get("HIGH_CUT_FREQ");
	write_i16(&buf, tmp ? get<int16_t>(*tmp) : 0);
	tmp = hdr.get("LOW_CUT_SLOPE");
	write_i16(&buf, tmp ? get<int16_t>(*tmp) : 0);
	tmp = hdr.get("HIGH_CUT_SLOPE");
	write_i16(&buf, tmp ? get<int16_t>(*tmp) : 0);
	tmp = hdr.get("YEAR");
	write_i16(&buf, tmp ? get<int16_t>(*tmp) : 0);
	tmp = hdr.get("DAY");
	write_i16(&buf, tmp ? get<int16_t>(*tmp) : 0);
	tmp = hdr.get("HOUR");
	write_i16(&buf, tmp ? get<int16_t>(*tmp) : 0);
	tmp = hdr.get("MINUTE");
	write_i16(&buf, tmp ? get<int16_t>(*tmp) : 0);
	tmp = hdr.get("SECOND");
	write_i16(&buf, tmp ? get<int16_t>(*tmp) : 0);
	tmp = hdr.get("TIME_BASIS_CODE");
	write_i16(&buf, tmp ? get<int16_t>(*tmp) : 0);
	tmp = hdr.get("TRACE_WEIGHT");
	write_i16(&buf, tmp ? get<int16_t>(*tmp) : 0);
	tmp = hdr.get("GROUP_NUM_ROLL");
	write_i16(&buf, tmp ? get<int16_t>(*tmp) : 0);
	tmp = hdr.get("GROUP_NUM_FIRST");
	write_i16(&buf, tmp ? get<int16_t>(*tmp) : 0);
	tmp = hdr.get("GROUP_NUM_LAST");
	write_i16(&buf, tmp ? get<int16_t>(*tmp) : 0);
	tmp = hdr.get("GAP_SIZE");
	write_i16(&buf, tmp ? get<int16_t>(*tmp) : 0);
	tmp = hdr.get("OVER_TRAVEL");
	write_i16(&buf, tmp ? get<int16_t>(*tmp) : 0);
	tmp = hdr.get("CDP_X");
	write_i32(&buf, tmp ? get<int32_t>(*tmp) : 0);
	tmp = hdr.get("CDP_Y");
	write_i32(&buf, tmp ? get<int32_t>(*tmp) : 0);
	tmp = hdr.get("INLINE");
	write_i32(&buf, tmp ? get<int32_t>(*tmp) : 0);
	tmp = hdr.get("XLINE");
	write_i32(&buf, tmp ? get<int32_t>(*tmp) : 0);
	tmp = hdr.get("SP_NUM");
	write_i32(&buf, tmp ? get<int32_t>(*tmp) : 0);
	tmp = hdr.get("SP_NUM_SCALAR");
	write_i16(&buf, tmp ? get<int16_t>(*tmp) : 0);
	tmp = hdr.get("TR_VAL_UNIT");
	write_i16(&buf, tmp ? get<int16_t>(*tmp) : 0);
	tmp = hdr.get("TRANS_CONST");
	double val = tmp ? get<double>(*tmp) : 0;
	int16_t exp = log10(val);
	write_i32(&buf, val / pow(10, exp));
	write_i16(&buf, exp);
	tmp = hdr.get("TRANS_UNITS");
	write_i16(&buf, tmp ? get<int16_t>(*tmp) : 0);
	tmp = hdr.get("DEVICE_ID");
	write_i16(&buf, tmp ? get<int16_t>(*tmp) : 0);
	tmp = hdr.get("TIME_SCALAR");
	write_i16(&buf, tmp ? get<int16_t>(*tmp) : 0);
	tmp = hdr.get("SOURCE_TYPE");
	write_i16(&buf, tmp ? get<int16_t>(*tmp) : 0);
	tmp = hdr.get("SOU_V_DIR");
	write_i16(&buf, tmp ? get<int16_t>(*tmp) : 0);
	tmp = hdr.get("SOU_X_DIR");
	write_i16(&buf, tmp ? get<int16_t>(*tmp) : 0);
	tmp = hdr.get("SOU_I_DIR");
	write_i16(&buf, tmp ? get<int16_t>(*tmp) : 0);
	tmp = hdr.get("SOURCE_MEASUREMENT");
	val = tmp ? get<double>(*tmp) : 0;
	exp = log10(val);
	write_i32(&buf, val / pow(10, exp));
	write_i16(&buf, exp);
	tmp = hdr.get("SOU_MEAS_UNIT");
	write_i16(&buf, tmp ? get<int16_t>(*tmp) : 0);
	common.file.write(common.hdr_buf, CommonSegy::TR_HEADER_SIZE);
}

void OSegy::Impl::write_trace_samples(Trace const &t, uint32_t samp_num)
{
	uint64_t bytes_num = samp_num * common.bytes_per_sample;
	if (bytes_num != common.samp_buf.size())
		common.samp_buf.resize(bytes_num);
	char *buf = common.samp_buf.data();
	valarray<double> const &samples = t.samples();
	for (auto samp: samples)
		write_sample(&buf, samp);
	common.file.write(common.samp_buf.data(), common.samp_buf.size());
}

OSegy::OSegy(string const& name, string const& revision)
	: pimpl(make_unique<Impl>(name, revision))
{
}
OSegy::OSegy(string&& name, string&& revision)
	: pimpl(make_unique<Impl>(move(name), move(revision)))
{
}

OSegy::~OSegy() = default;
} // namespace sedaman
