#include "OSEGY.hpp"
#include "Exception.hpp"
#include "util.hpp"
#include <cfloat>
#include <climits>
#include <cmath>
#include <functional>
#include <ios>
#include <variant>

using std::abs;
using std::fstream;
using std::function;
using std::get;
using std::ios_base;
using std::make_unique;
using std::map;
using std::move;
using std::optional;
using std::pair;
using std::streampos;
using std::string;
using std::vector;

namespace sedaman {
class OSEGY::Impl {
public:
    Impl(CommonSEGY com)
        : common { move(com) }
    {
    }
    CommonSEGY common;
    void assign_raw_writers();
    void assign_sample_writer();
    void assign_bytes_per_sample();
    void write_bin_header();
    void write_ext_text_headers();
    void write_trailer_stanzas();
    void write_trace_header(Trace::Header const& hdr);
    void write_additional_trace_headers(Trace::Header const& hdr);
    void write_trace_samples_fix(Trace const& t);
    void write_trace_samples_var(Trace const& t);

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
    function<void(char**, double)> write_ibm_float;
    function<void(char**, float)> write_IEEE_float;
    function<void(char**, double)> write_IEEE_double;
};

void OSEGY::Impl::assign_raw_writers()
{
    write_u8 = [](char** buf, uint8_t val) { write<uint8_t>(buf, val); };
    write_i8 = [](char** buf, int8_t val) { write<int8_t>(buf, val); };
    switch (common.binary_header.endianness) {
    case 0x01020304:
        write_u16 = [](char** buf, uint16_t val)
	   	{ write<uint16_t>(buf, val); };
        write_i16 = [](char** buf, int16_t val) { write<int16_t>(buf, val); };
        write_u24 = [](char** buf, uint32_t val) {
            write<uint16_t>(buf, val);
            write<uint8_t>(buf, val >> 16);
        };
        write_i24 = [](char** buf, int32_t val) {
            write<int16_t>(buf, val);
            write<int8_t>(buf, static_cast<int32_t>(val) >> 16);
        };
        write_u32 = [](char** buf, uint32_t val)
	   	{ write<uint32_t>(buf, val); };
        write_i32 = [](char** buf, int32_t val) { write<int32_t>(buf, val); };
        write_u64 = [](char** buf, uint64_t val)
	   	{ write<uint64_t>(buf, val); };
        write_i64 = [](char** buf, int64_t val) { write<int64_t>(buf, val); };
        break;
    case 0:
    case 0x04030201:
        write_u16 = [](char** buf, uint16_t val)
	   	{ write<uint16_t>(buf, swap(val)); };
        write_i16 = [](char** buf, int16_t val)
	   	{ write<int16_t>(buf, swap(val)); };
        write_u24 = [](char** buf, uint32_t val) {
            uint32_t tmp = swap(val) >> 8;
            write<uint16_t>(buf, tmp);
            write<uint8_t>(buf, tmp >> 16);
        };
        write_i24 = [](char** buf, int32_t val) {
            uint32_t tmp = swap(static_cast<uint32_t>(val)) >> 8;
            write<int16_t>(buf, tmp);
            write<int8_t>(buf, tmp >> 16);
        };
        write_u32 = [](char** buf, uint32_t val)
	   	{ write<uint32_t>(buf, swap(val)); };
        write_i32 = [](char** buf, int32_t val)
	   	{ write<int32_t>(buf, swap(val)); };
        write_u64 = [](char** buf, uint64_t val)
	   	{ write<uint64_t>(buf, swap(val)); };
        write_i64 = [](char** buf, int64_t val)
	   	{ write<int64_t>(buf, swap(val)); };
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
        write_IEEE_float = [this](char** buf, double val) {
            uint32_t sign = val < 0 ? 1 : 0;
            double abs_val = abs(val);
            uint32_t exp = static_cast<uint32_t>((log(abs_val) /
												  log(2) + 1 + 127)) & 0xff;
            uint32_t fraction = abs_val / pow(2, static_cast<int>(exp) - 127) *
			   	pow(2, 23) - 1;
            uint32_t result = sign << 31 | exp << 23 | (fraction & 0x007fffff);
            write_u32(buf, result);
        };
        write_IEEE_double = [this](char** buf, double val) {
            uint64_t sign = val < 0 ? 1 : 0;
            double abs_val = abs(val);
            uint64_t exp = static_cast<uint64_t>(log(abs_val) /
												 log(2) + 1 + 1023) & 0x7ff;
            uint64_t fraction = abs_val / pow(2, static_cast<int>(exp) - 1023)
			   	* pow(2, 52) - 1;
            uint64_t result = sign << 63 | exp << 52 |
			   	(fraction & 0xfffffffffffff);
            write_u64(buf, result);
        };
    }
    write_ibm_float = [this](char** buf, double val) {
        uint32_t sign = val < 0 ? 1 : 0;
        double abs_val = abs(val);
        uint32_t exp = static_cast<uint32_t>(log(abs_val) / log(2) / 4 +
											 1 + 64) & 0x7f;
        uint32_t fraction = abs_val / pow(16, static_cast<int>(exp) - 64) *
		   	pow(2, 24);
        uint32_t result = sign << 31 | exp << 24 | (fraction & 0x00ffffff);
        write_u32(buf, result);
    };
}

void OSEGY::Impl::assign_bytes_per_sample()
{
    switch (common.binary_header.format_code) {
    case 1:
    case 2:
    case 4:
    case 5:
    case 10:
        common.bytes_per_sample = 4;
        break;
    case 3:
    case 11:
        common.bytes_per_sample = 2;
        break;
    case 6:
    case 9:
    case 12:
        common.bytes_per_sample = 8;
        break;
    case 7:
    case 15:
        common.bytes_per_sample = 3;
        break;
    case 8:
    case 16:
        common.bytes_per_sample = 1;
        break;
    }
}

void OSEGY::Impl::assign_sample_writer()
{
    switch (common.binary_header.format_code) {
    case 1:
        write_sample = write_ibm_float;
        break;
    case 2:
        write_sample = [this](char** buf, double val) { write_i32(buf, val); };
        break;
    case 3:
        write_sample = [this](char** buf, double val) { write_i16(buf, val); };
        break;
    case 5:
        write_sample = write_IEEE_float;
        break;
    case 6:
        write_sample = write_IEEE_double;
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

void OSEGY::Impl::write_bin_header()
{
    char buf[CommonSEGY::BIN_HEADER_SIZE] = { 0 };
    char* ptr = buf;
    write_i32(&ptr, common.binary_header.job_id);
    write_i32(&ptr, common.binary_header.line_num);
    write_i32(&ptr, common.binary_header.reel_num);
    write_i16(&ptr, common.binary_header.tr_per_ens);
    write_i16(&ptr, common.binary_header.aux_per_ens);
    write_i16(&ptr, common.binary_header.samp_int);
    write_i16(&ptr, common.binary_header.samp_int_orig);
    write_i16(&ptr, common.binary_header.samp_per_tr);
    write_i16(&ptr, common.binary_header.samp_per_tr_orig);
    write_i16(&ptr, common.binary_header.format_code);
    write_i16(&ptr, common.binary_header.ens_fold);
    write_i16(&ptr, common.binary_header.sort_code);
    write_i16(&ptr, common.binary_header.vert_sum_code);
    write_i16(&ptr, common.binary_header.sw_freq_at_start);
    write_i16(&ptr, common.binary_header.sw_freq_at_end);
    write_i16(&ptr, common.binary_header.sw_length);
    write_i16(&ptr, common.binary_header.sw_type_code);
    write_i16(&ptr, common.binary_header.sw_ch_tr_num);
    write_i16(&ptr, common.binary_header.taper_at_start);
    write_i16(&ptr, common.binary_header.taper_at_end);
    write_i16(&ptr, common.binary_header.taper_type);
    write_i16(&ptr, common.binary_header.corr_traces);
    write_i16(&ptr, common.binary_header.bin_gain_recov);
    write_i16(&ptr, common.binary_header.amp_recov_meth);
    write_i16(&ptr, common.binary_header.measure_system);
    write_i16(&ptr, common.binary_header.impulse_sig_pol);
    write_i16(&ptr, common.binary_header.vib_pol_code);
    write_i32(&ptr, common.binary_header.ext_tr_per_ens);
    write_i32(&ptr, common.binary_header.ext_aux_per_ens);
    write_i32(&ptr, common.binary_header.ext_samp_per_tr);
    write_IEEE_double(&ptr, common.binary_header.ext_samp_int);
    write_IEEE_double(&ptr, common.binary_header.ext_samp_int_orig);
    write_i32(&ptr, common.binary_header.ext_samp_per_tr_orig);
    write_i32(&ptr, common.binary_header.ext_ens_fold);
    write_i32(&ptr, common.binary_header.endianness);
    ptr += 200;
    write_u8(&ptr, common.binary_header.SEGY_rev_major_ver);
    write_u8(&ptr, common.binary_header.SEGY_rev_minor_ver);
    write_i16(&ptr, common.binary_header.fixed_tr_length);
    write_i16(&ptr, common.binary_header.ext_text_headers_num);
    write_i32(&ptr, common.binary_header.max_num_add_tr_headers);
    write_i16(&ptr, common.binary_header.time_basis_code);
    write_u64(&ptr, common.binary_header.num_of_tr_in_file);
    write_u64(&ptr, common.binary_header.byte_off_of_first_tr);
    write_i32(&ptr, common.binary_header.num_of_trailer_stanza);
	common.file.seekg(CommonSEGY::TEXT_HEADER_SIZE, ios_base::beg);
    common.file.write(buf, CommonSEGY::BIN_HEADER_SIZE);
}

void OSEGY::Impl::write_trace_header(Trace::Header const& hdr)
{
    char* buf = common.hdr_buf;
    optional<Trace::Header::Value> tmp = hdr.get("TRC_SEQ_LINE");
    write_i32(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("TRC_SEQ_SGY");
    write_i32(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("FFID");
    write_i32(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("CHAN");
    write_i32(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("ESP");
    write_i32(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("ENS_NO");
    write_i32(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("SEQ_NO");
    write_i32(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("TRACE_ID");
    write_i16(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("VERT_SUM");
    write_i16(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("HOR_SUM");
    write_i16(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("DATA_USE");
    write_i16(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("OFFSET");
    write_i32(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("R_ELEV");
    write_i32(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("S_ELEV");
    write_i32(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("S_DEPTH");
    write_i32(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("R_DATUM");
    write_i32(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("S_DATUM");
    write_i32(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("S_WATER");
    write_i32(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("R_WATER");
    write_i32(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("ELEV_SCALAR");
    write_i16(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("COORD_SCALAR");
    write_i16(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("SOU_X");
    write_i32(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("SOU_Y");
    write_i32(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("REC_X");
    write_i32(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("REC_Y");
    write_i32(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("COORD_UNITS");
    write_i16(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("WEATH_VEL");
    write_i16(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("SUBWEATH_VEL");
    write_i16(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("S_UPHOLE");
    write_i16(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("R_UPHOLE");
    write_i16(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("S_STAT");
    write_i16(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("R_STAT");
    write_i16(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("TOT_STAT");
    write_i16(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("LAG_A");
    write_i16(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("LAG_B");
    write_i16(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("DELAY_TIME");
    write_i16(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("MUTE_START");
    write_i16(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("MUTE_END");
    write_i16(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("SAMP_NUM");
    write_i16(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("SAMP_INT");
    write_i16(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("GAIN_TYPE");
    write_i16(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("GAIN_CONST");
    write_i16(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("INIT_GAIN");
    write_i16(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("CORRELATED");
    write_i16(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("SW_START");
    write_i16(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("SW_END");
    write_i16(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("SW_LENGTH");
    write_i16(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("SW_TYPE");
    write_i16(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("SW_TAPER_START");
    write_i16(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("SW_TAPER_END");
    write_i16(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("TAPER_TYPE");
    write_i16(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("ALIAS_FILT_FREQ");
    write_i16(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("ALIAS_FILT_SLOPE");
    write_i16(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("NOTCH_FILT_FREQ");
    write_i16(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("NOTCH_FILT_SLOPE");
    write_i16(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("LOW_CUT_FREQ");
    write_i16(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("HIGH_CUT_FREQ");
    write_i16(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("LOW_CUT_SLOPE");
    write_i16(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("HIGH_CUT_SLOPE");
    write_i16(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("YEAR");
    write_i16(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("DAY");
    write_i16(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("HOUR");
    write_i16(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("MINUTE");
    write_i16(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("SECOND");
    write_i16(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("TIME_BASIS_CODE");
    write_i16(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("TRACE_WEIGHT");
    write_i16(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("GROUP_NUM_ROLL");
    write_i16(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("GROUP_NUM_FIRST");
    write_i16(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("GROUP_NUM_LAST");
    write_i16(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("GAP_SIZE");
    write_i16(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("OVER_TRAVEL");
    write_i16(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("CDP_X");
    write_i32(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("CDP_Y");
    write_i32(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("INLINE");
    write_i32(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("XLINE");
    write_i32(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("SP_NUM");
    write_i32(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("SP_NUM_SCALAR");
    write_i16(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("TR_VAL_UNIT");
    write_i16(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("TRANS_CONST");
    double val = tmp ? get<double>(*tmp) : 1;
    int16_t exp = log10(val);
    write_i32(&buf, val / pow(10, exp));
    write_i16(&buf, exp);
    tmp = hdr.get("TRANS_UNITS");
    write_i16(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("DEVICE_ID");
    write_i16(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("TIME_SCALAR");
    write_i16(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("SOURCE_TYPE");
    write_i16(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("SOU_V_DIR");
    write_i16(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("SOU_X_DIR");
    write_i16(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("SOU_I_DIR");
    write_i16(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("SOURCE_MEASUREMENT");
    val = tmp ? get<double>(*tmp) : 1;
    exp = log10(val);
    write_i32(&buf, val / pow(10, exp));
    write_i16(&buf, exp);
    tmp = hdr.get("SOU_MEAS_UNIT");
    write_i16(&buf, tmp ? get<int64_t>(*tmp) : 0);
    common.file.write(common.hdr_buf, CommonSEGY::TR_HEADER_SIZE);
}

void OSEGY::Impl::write_additional_trace_headers(Trace::Header const& hdr)
{
    char* buf = common.hdr_buf;
    std::memset(buf, 0, CommonSEGY::TR_HEADER_SIZE);
    optional<Trace::Header::Value> tmp = hdr.get("TRC_SEQ_LINE");
    write_u64(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("TRC_SEQ_SGY");
    write_u64(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("FFID");
    write_u64(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("ENS_NO");
    write_u64(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("R_ELEV");
    write_IEEE_double(&buf, tmp ? get<double>(*tmp) : 0);
    tmp = hdr.get("R_DEPTH");
    write_IEEE_double(&buf, tmp ? get<double>(*tmp) : 0);
    tmp = hdr.get("S_DEPTH");
    write_IEEE_double(&buf, tmp ? get<double>(*tmp) : 0);
    tmp = hdr.get("R_DATUM");
    write_IEEE_double(&buf, tmp ? get<double>(*tmp) : 0);
    tmp = hdr.get("S_DATUM");
    write_IEEE_double(&buf, tmp ? get<double>(*tmp) : 0);
    tmp = hdr.get("R_WATER");
    write_IEEE_double(&buf, tmp ? get<double>(*tmp) : 0);
    tmp = hdr.get("S_WATER");
    write_IEEE_double(&buf, tmp ? get<double>(*tmp) : 0);
    tmp = hdr.get("SOU_X");
    write_IEEE_double(&buf, tmp ? get<double>(*tmp) : 0);
    tmp = hdr.get("SOU_Y");
    write_IEEE_double(&buf, tmp ? get<double>(*tmp) : 0);
    tmp = hdr.get("REC_X");
    write_IEEE_double(&buf, tmp ? get<double>(*tmp) : 0);
    tmp = hdr.get("REC_Y");
    write_IEEE_double(&buf, tmp ? get<double>(*tmp) : 0);
    tmp = hdr.get("OFFSET");
    write_IEEE_double(&buf, tmp ? get<double>(*tmp) : 0);
    tmp = hdr.get("SAMP_NUM");
    write_u64(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("NANOSECOND");
    write_u64(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("SAMP_INT");
    write_IEEE_double(&buf, tmp ? get<double>(*tmp) : 0);
    tmp = hdr.get("CABLE_NUM");
    write_u64(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("ADD_TRC_HDR_NUM");
    uint16_t add_trc_hdr_num = tmp ? get<int64_t>(*tmp) : 0;
    add_trc_hdr_num = add_trc_hdr_num ? add_trc_hdr_num :
	   	common.binary_header.max_num_add_tr_headers;
    write_u64(&buf, add_trc_hdr_num);
    tmp = hdr.get("LAST_TRC_FLAG");
    write_u64(&buf, tmp ? get<int64_t>(*tmp) : 0);
    tmp = hdr.get("CDP_X");
    write_IEEE_double(&buf, tmp ? get<double>(*tmp) : 0);
    tmp = hdr.get("CDP_Y");
    write_IEEE_double(&buf, tmp ? get<double>(*tmp) : 0);
    common.file.write(common.hdr_buf, CommonSEGY::TR_HEADER_SIZE);
    for (int i = 0; i < add_trc_hdr_num - 1; ++i) {
        std::memset(common.hdr_buf, 0, CommonSEGY::TR_HEADER_SIZE);
        if (common.add_tr_hdr_map.size()) {
            // for each item in map
            for (auto& item : common.add_tr_hdr_map[i].second) {
                tmp = hdr.get(item.second.first);
                if (tmp == std::nullopt)
                    throw Exception(__FILE__, __LINE__,
                    "no such header in trace");
                char* pos = common.hdr_buf + item.first;
                switch (item.second.second) {
                case Trace::Header::ValueType::int8_t:
                    write_i8(&pos, get<int64_t>(*tmp));
                    break;
                case Trace::Header::ValueType::uint8_t:
                    write_u8(&pos, get<int64_t>(*tmp));
                    break;
                case Trace::Header::ValueType::int16_t:
                    write_i16(&pos, get<int64_t>(*tmp));
                    break;
                case Trace::Header::ValueType::uint16_t:
                    write_u16(&pos, get<int64_t>(*tmp));
                    break;
                case Trace::Header::ValueType::int24_t:
                    write_i24(&pos, get<int64_t>(*tmp));
                    break;
                case Trace::Header::ValueType::uint24_t:
                    write_u24(&pos, get<int64_t>(*tmp));
                    break;
                case Trace::Header::ValueType::int32_t:
                    write_i32(&pos, get<int64_t>(*tmp));
                    break;
                case Trace::Header::ValueType::uint32_t:
                    write_u32(&pos, get<int64_t>(*tmp));
                    break;
                case Trace::Header::ValueType::int64_t:
                    write_i64(&pos, get<int64_t>(*tmp));
                    break;
                case Trace::Header::ValueType::uint64_t:
                    write_u64(&pos, get<int64_t>(*tmp));
                    break;
                case Trace::Header::ValueType::ibm:
                    write_ibm_float(&pos, get<double>(*tmp));
                    break;
                case Trace::Header::ValueType::ieee_single:
                    write_IEEE_float(&pos, get<double>(*tmp));
                    break;
                case Trace::Header::ValueType::ieee_double:
                    write_IEEE_double(&pos, get<double>(*tmp));
                    break;
                }
            }
            // copy additional trace header name
            std::memcpy(common.hdr_buf + 232,
            common.add_tr_hdr_map[i].first.c_str(),
            common.add_tr_hdr_map[i].first.size());
        }
        common.file.write(common.hdr_buf, CommonSEGY::TR_HEADER_SIZE);
    }
}

void OSEGY::Impl::write_ext_text_headers()
{
    int hdrs_num = common.text_headers.size() - 1;
    common.binary_header.ext_text_headers_num = hdrs_num;
    for (int i = 1; i < hdrs_num + 1; ++i)
        common.file.write(common.text_headers[i].c_str(),
						  CommonSEGY::TEXT_HEADER_SIZE);
}

void OSEGY::Impl::write_trailer_stanzas()
{
    int stanzas_num = common.trailer_stanzas.size();
    common.binary_header.num_of_trailer_stanza = stanzas_num;
    for (int i = 0; i < stanzas_num; ++i)
        common.file.write(common.trailer_stanzas[i].c_str(),
						  CommonSEGY::TEXT_HEADER_SIZE);
}

void OSEGY::Impl::write_trace_samples_var(Trace const& t)
{
    Trace::Header::Value v = *t.header().get("SAMP_NUM");
    int64_t samp_num = get<int64_t>(v);
    uint64_t bytes_num = samp_num * common.bytes_per_sample;
    if (bytes_num != common.samp_buf.size())
        common.samp_buf.resize(bytes_num);
    write_trace_samples_fix(t);
}

void OSEGY::Impl::write_trace_samples_fix(Trace const& t)
{
    char* buf = common.samp_buf.data();
    vector<double> const& samples = t.samples();
    for (auto samp : samples)
        write_sample(&buf, samp);
    common.file.write(common.samp_buf.data(), common.samp_buf.size());
}

OSEGY::OSEGY(string name, CommonSEGY::BinaryHeader bh,
    vector<pair<string, map<uint32_t, pair<string,
   	Trace::Header::ValueType>>>> add_hdr_map)
    : pimpl { make_unique<Impl>(CommonSEGY { move(name),
	   	fstream::out | fstream::binary, move(bh), move(add_hdr_map) }) }
{
}

CommonSEGY& OSEGY::common() { return pimpl->common; }
void OSEGY::assign_raw_writers() { pimpl->assign_raw_writers(); }
void OSEGY::assign_sample_writer() { pimpl->assign_sample_writer(); }
void OSEGY::assign_bytes_per_sample() { pimpl->assign_bytes_per_sample(); }
void OSEGY::write_bin_header() { pimpl->write_bin_header(); }
void OSEGY::write_ext_text_headers() { pimpl->write_ext_text_headers(); }
void OSEGY::write_trailer_stanzas() { pimpl->write_trailer_stanzas(); }
void OSEGY::write_trace_header(Trace::Header const& hdr)
{
    pimpl->write_trace_header(hdr);
}
void OSEGY::write_additional_trace_headers(Trace::Header const& hdr)
{
    pimpl->write_additional_trace_headers(hdr);
}
void OSEGY::write_trace_samples_fix(Trace const& t)
{
    pimpl->write_trace_samples_fix(t);
}
void OSEGY::write_trace_samples_var(Trace const& t)
{
    pimpl->write_trace_samples_var(t);
}

OSEGY::~OSEGY() = default;
} // namespace sedaman
