#include "Exception.hpp"
#include "OSegy.hpp"
#include "util.hpp"
#include <cfloat>
#include <climits>
#include <cmath>
#include <ios>
#include <functional>
#include <variant>

using std::get;
using std::ios_base;
using std::fstream;
using std::function;
using std::holds_alternative;
using std::make_unique;
using std::move;
using std::optional;
using std::streampos;
using std::string;
using std::valarray;

namespace sedaman {
class OSegy::Impl {
public:
	Impl(OSegy &s);
	void assign_raw_writers();
	void assign_sample_writer();
	void assign_bytes_per_sample();
	void write_bin_header();
	void write_ext_text_headers();
	void write_trailer_stanzas();
	void write_trace_header(Trace::Header const &hdr);
	void write_additional_trace_header(Trace::Header const &hdr);
	void write_trace_samples(Trace const &t);
	void write_trace_samples_var(Trace const &t);

private:
	OSegy &sgy;
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
};

OSegy::Impl::Impl(OSegy &s)
	: sgy { s }
{
}

void OSegy::Impl::assign_raw_writers()
{
	write_u8 = [](char** buf, uint8_t val) { write<uint8_t>(buf, val); };
	write_i8 = [](char** buf, int8_t val) { write<int8_t>(buf, val); };
	switch (sgy.p_bin_hdr().endianness) {
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

void OSegy::Impl::assign_bytes_per_sample()
{
	switch (sgy.p_bin_hdr().format_code) {
		case 1:
		case 2:
		case 4:
		case 5:
		case 10:
			sgy.p_bytes_per_sample() = 4;
			break;
		case 3:
		case 11:
			sgy.p_bytes_per_sample() = 2;
			break;
		case 6:
		case 9:
		case 12:
			sgy.p_bytes_per_sample() = 8;
			break;
		case 7:
		case 15:
			sgy.p_bytes_per_sample() = 3;
			break;
		case 8:
		case 16:
			sgy.p_bytes_per_sample() = 1;
			break;
	}
}

void OSegy::Impl::assign_sample_writer()
{
	switch (sgy.p_bin_hdr().format_code) {
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
	write_i32(&ptr, sgy.p_bin_hdr().job_id);
	write_i32(&ptr, sgy.p_bin_hdr().line_num);
	write_i32(&ptr, sgy.p_bin_hdr().reel_num);
	write_i16(&ptr, sgy.p_bin_hdr().tr_per_ens);
	write_i16(&ptr, sgy.p_bin_hdr().aux_per_ens);
	write_i16(&ptr, sgy.p_bin_hdr().samp_int);
	write_i16(&ptr, sgy.p_bin_hdr().samp_int_orig);
	write_i16(&ptr, sgy.p_bin_hdr().samp_per_tr);
	write_i16(&ptr, sgy.p_bin_hdr().samp_per_tr_orig);
	write_i16(&ptr, sgy.p_bin_hdr().format_code);
	write_i16(&ptr, sgy.p_bin_hdr().ens_fold);
	write_i16(&ptr, sgy.p_bin_hdr().sort_code);
	write_i16(&ptr, sgy.p_bin_hdr().vert_sum_code);
	write_i16(&ptr, sgy.p_bin_hdr().sw_freq_at_start);
	write_i16(&ptr, sgy.p_bin_hdr().sw_freq_at_end);
	write_i16(&ptr, sgy.p_bin_hdr().sw_length);
	write_i16(&ptr, sgy.p_bin_hdr().sw_type_code);
	write_i16(&ptr, sgy.p_bin_hdr().sw_ch_tr_num);
	write_i16(&ptr, sgy.p_bin_hdr().taper_at_start);
	write_i16(&ptr, sgy.p_bin_hdr().taper_at_end);
	write_i16(&ptr, sgy.p_bin_hdr().taper_type);
	write_i16(&ptr, sgy.p_bin_hdr().corr_traces);
	write_i16(&ptr, sgy.p_bin_hdr().bin_gain_recov);
	write_i16(&ptr, sgy.p_bin_hdr().amp_recov_meth);
	write_i16(&ptr, sgy.p_bin_hdr().measure_system);
	write_i16(&ptr, sgy.p_bin_hdr().impulse_sig_pol);
	write_i16(&ptr, sgy.p_bin_hdr().vib_pol_code);
	write_i32(&ptr, sgy.p_bin_hdr().ext_tr_per_ens);
	write_i32(&ptr, sgy.p_bin_hdr().ext_aux_per_ens);
	write_i32(&ptr, sgy.p_bin_hdr().ext_samp_per_tr);
	write_IEEE_double(&ptr, sgy.p_bin_hdr().ext_samp_int);
	write_IEEE_double(&ptr, sgy.p_bin_hdr().ext_samp_int_orig);
	write_i32(&ptr, sgy.p_bin_hdr().ext_samp_per_tr_orig);
	write_i32(&ptr, sgy.p_bin_hdr().ext_ens_fold);
	write_i32(&ptr, sgy.p_bin_hdr().endianness);
	ptr += 200;
	write_u8(&ptr, sgy.p_bin_hdr().SEGY_rev_major_ver);
	write_u8(&ptr, sgy.p_bin_hdr().SEGY_rev_minor_ver);
	write_i16(&ptr, sgy.p_bin_hdr().fixed_tr_length);
	write_i16(&ptr, sgy.p_bin_hdr().ext_text_headers_num);
	write_i32(&ptr, sgy.p_bin_hdr().max_num_add_tr_headers);
	write_i16(&ptr, sgy.p_bin_hdr().time_basis_code);
	write_u64(&ptr, sgy.p_bin_hdr().num_of_tr_in_file);
	write_u64(&ptr, sgy.p_bin_hdr().byte_off_of_first_tr);
	write_i32(&ptr, sgy.p_bin_hdr().num_of_trailer_stanza);
	sgy.p_file().write(buf, CommonSegy::BIN_HEADER_SIZE);
}

void OSegy::Impl::write_trace_header(Trace::Header const &hdr)
{
	char *buf = sgy.p_hdr_buf();
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
	sgy.p_file().write(sgy.p_hdr_buf(), CommonSegy::TR_HEADER_SIZE);
}

void OSegy::Impl::write_additional_trace_header(Trace::Header const &hdr)
{
	// TODO: add writing arbitrary headers
	char *buf = sgy.p_hdr_buf();
	std::memset(buf, 0, CommonSegy::TR_HEADER_SIZE);
	optional<Trace::Header::Value> tmp = hdr.get("TRC_SEQ_LINE");
	write_u64(&buf, tmp ? get<uint64_t>(*tmp) : 0);
	tmp = hdr.get("TRC_SEQ_SGY");
	write_u64(&buf, tmp ? get<uint64_t>(*tmp) : 0);
	tmp = hdr.get("FFID");
	write_u64(&buf, tmp ? get<uint64_t>(*tmp) : 0);
	tmp = hdr.get("ENS_NO");
	write_u64(&buf, tmp ? get<uint64_t>(*tmp) : 0);
	tmp = hdr.get("R_ELEV");
	write_u64(&buf, tmp ? get<double>(*tmp) : 0);
	tmp = hdr.get("R_DEPTH");
	write_u64(&buf, tmp ? get<double>(*tmp) : 0);
	tmp = hdr.get("S_DEPTH");
	write_u64(&buf, tmp ? get<double>(*tmp) : 0);
	tmp = hdr.get("R_DATUM");
	write_u64(&buf, tmp ? get<double>(*tmp) : 0);
	tmp = hdr.get("S_DATUM");
	write_u64(&buf, tmp ? get<double>(*tmp) : 0);
	tmp = hdr.get("R_WATER");
	write_u64(&buf, tmp ? get<double>(*tmp) : 0);
	tmp = hdr.get("S_WATER");
	write_u64(&buf, tmp ? get<double>(*tmp) : 0);
	tmp = hdr.get("SOU_X");
	write_u64(&buf, tmp ? get<double>(*tmp) : 0);
	tmp = hdr.get("SOU_Y");
	write_u64(&buf, tmp ? get<double>(*tmp) : 0);
	tmp = hdr.get("REC_X");
	write_u64(&buf, tmp ? get<double>(*tmp) : 0);
	tmp = hdr.get("REC_Y");
	write_u64(&buf, tmp ? get<double>(*tmp) : 0);
	tmp = hdr.get("OFFSET");
	write_u64(&buf, tmp ? get<double>(*tmp) : 0);
	tmp = hdr.get("SAMP_NUM");
	write_u64(&buf, tmp ? get<uint32_t>(*tmp) : 0);
	tmp = hdr.get("NANOSECOND");
	write_u64(&buf, tmp ? get<int32_t>(*tmp) : 0);
	tmp = hdr.get("SAMP_INT");
	write_u64(&buf, tmp ? get<double>(*tmp) : 0);
	tmp = hdr.get("CABLE_NUM");
	write_u64(&buf, tmp ? get<int32_t>(*tmp) : 0);
	tmp = hdr.get("ADD_TRC_HDR_NUM");
	uint16_t add_trc_hdr_num = tmp ? get<uint16_t>(*tmp) : 0;
	add_trc_hdr_num = add_trc_hdr_num ? add_trc_hdr_num : sgy.p_bin_hdr().max_num_add_tr_headers;
	write_u64(&buf, add_trc_hdr_num);
	tmp = hdr.get("LAST_TRC_FLAG");
	write_u64(&buf, tmp ? get<int16_t>(*tmp) : 0);
	tmp = hdr.get("CDP_X");
	write_u64(&buf, tmp ? get<double>(*tmp) : 0);
	tmp = hdr.get("CDP_Y");
	write_u64(&buf, tmp ? get<double>(*tmp) : 0);
	sgy.p_file().write(sgy.p_hdr_buf(), CommonSegy::TR_HEADER_SIZE);
}

void OSegy::Impl::write_ext_text_headers()
{
	int hdrs_num = sgy.p_txt_hdrs().size() - 1;
	sgy.p_bin_hdr().ext_text_headers_num = hdrs_num;
	for (int i = 0; i < hdrs_num; ++i)
		sgy.p_file().write(sgy.p_txt_hdrs()[i + 1].c_str(), CommonSegy::TEXT_HEADER_SIZE);
}

void OSegy::Impl::write_trailer_stanzas()
{
	int stanzas_num = sgy.p_trlr_stnzs().size();
	sgy.p_bin_hdr().num_of_trailer_stanza = stanzas_num;
	for (int i = 0; i < stanzas_num; ++i)
		sgy.p_file().write(sgy.p_trlr_stnzs()[i].c_str(), CommonSegy::TEXT_HEADER_SIZE);
}

void OSegy::Impl::write_trace_samples_var(Trace const &t)
{
	Trace::Header::Value v = *t.header().get("SAMP_NUM");
	uint32_t samp_num;
	if (holds_alternative<int16_t>(v))
		samp_num = get<int16_t>(v);
	else
		samp_num = get<uint32_t>(v);
	uint64_t bytes_num = samp_num * sgy.p_bytes_per_sample();
	if (bytes_num != sgy.p_samp_buf().size())
		sgy.p_samp_buf().resize(bytes_num);
	write_trace_samples(t);
}

void OSegy::Impl::write_trace_samples(Trace const &t)
{
	char *buf = sgy.p_samp_buf().data();
	valarray<double> const &samples = t.samples();
	for (auto samp: samples)
		write_sample(&buf, samp);
	sgy.p_file().write(sgy.p_samp_buf().data(), sgy.p_samp_buf().size());
}

OSegy::OSegy(string name, CommonSegy::BinaryHeader bh)
	: CommonSegy { move(name), fstream::out | fstream::binary, move(bh) },
	pimpl(make_unique<Impl>(*this))
{
}

void OSegy::assign_raw_writers() { pimpl->assign_raw_writers(); }
void OSegy::assign_sample_writer() { pimpl->assign_sample_writer(); }
void OSegy::assign_bytes_per_sample() { pimpl->assign_bytes_per_sample(); }
void OSegy::write_bin_header() { pimpl->write_bin_header(); }
void OSegy::write_ext_text_headers() { pimpl->write_ext_text_headers(); }
void OSegy::write_trailer_stanzas() { pimpl->write_trailer_stanzas(); }
void OSegy::write_trace_header(Trace::Header const &hdr) {
	pimpl->write_trace_header(hdr);
}
void OSegy::write_additional_trace_header(Trace::Header const &hdr)
{
	pimpl->write_additional_trace_header(hdr);
}
void OSegy::write_trace_samples(Trace const &t) {
	pimpl->write_trace_samples(t);
}
void OSegy::write_trace_samples_var(Trace const &t) {
	pimpl->write_trace_samples_var(t);
}

OSegy::~OSegy() = default;
} // namespace sedaman
