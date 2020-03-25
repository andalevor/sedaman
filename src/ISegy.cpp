#include "ISegy.hpp"
#include "CommonSegy.hpp"
#include "Exception.hpp"
#include "Trace.hpp"
#include "util.hpp"
#include <bits/stdint-intn.h>
#include <bits/stdint-uintn.h>
#include <cfloat>
#include <fstream>
#include <functional>
#include <ios>
#include <string>
#include <unordered_map>
#include <valarray>
#include <variant>

using std::get;
using std::ios_base;
using std::fstream;
using std::function;
using std::holds_alternative;
using std::make_unique;
using std::move;
using std::streampos;
using std::streamsize;
using std::string;
using std::unordered_map;
using std::valarray;
using std::vector;

namespace sedaman {
class ISegy::Impl {
public:
    Impl(string const& name);
    Impl(string&& name);
    CommonSegy common;
    streampos first_trace_pos;
    streampos curr_pos;
    streampos end_of_data;
    unordered_map<string, Trace::Header::Value> read_trc_header();
    valarray<double> read_trc_smpls();
    valarray<double> read_trc_smpls_var(uint32_t samp_num);

private:
    function<char(char const**)> read_8;
    function<uint16_t(char const**)> read_16;
    function<uint16_t(char const**)> read_24;
    function<uint32_t(char const**)> read_32;
    function<uint64_t(char const**)> read_64;
    function<double(char const**)> read_sample;
    function<double(char const** buf)> dbl_from_IEEE_float;
    function<double(char const** buf)> dbl_from_IEEE_double;
    void initialization();
    void fill_bin_header(char const* buf);
    void assign_raw_readers();
    void assign_sample_reader();
    void read_ext_text_headers();
    void assign_bytes_per_sample();
    void read_trailer_stanzas();
    void read_file(char* buf, streamsize n);
    double dbl_from_ibm_float(char const** buf);
    double dbl_from_IEEE_float_native(char const** buf);
    double dbl_from_IEEE_float_not_native(char const** buf);
    double dbl_from_IEEE_double_native(char const** buf);
    double dbl_from_IEEE_double_not_native(char const** buf);
    double dbl_from_uint64(char const** buf);
    double dbl_from_int64(char const** buf);
    double dbl_from_uint32(char const** buf);
    double dbl_from_int32(char const** buf);
    double dbl_from_uint24(char const** buf);
    double dbl_from_int24(char const** buf);
    double dbl_from_uint16(char const** buf);
    double dbl_from_int16(char const** buf);
    double dbl_from_uint8(char const** buf);
    double dbl_from_int8(char const** buf);
};

ISegy::Impl::Impl(string const& name)
    : common { name, fstream::in | fstream::binary }
{
    initialization();
}

ISegy::Impl::Impl(string&& name)
    : common { move(name), fstream::in | fstream::binary }
{
    initialization();
}

void ISegy::Impl::read_file(char* buf, streamsize n)
{
    common.file.read(buf, n);
    curr_pos = common.file.tellg();
}

void ISegy::Impl::initialization()
{
    char text_buf[CommonSegy::TEXT_HEADER_SIZE];
    common.file.read(text_buf, CommonSegy::TEXT_HEADER_SIZE);
    common.txt_hdrs.push_back(string(text_buf, CommonSegy::TEXT_HEADER_SIZE));
    char bin_buf[CommonSegy::BIN_HEADER_SIZE];
    common.file.read(bin_buf, CommonSegy::BIN_HEADER_SIZE);
    fill_bin_header(bin_buf);
    assign_sample_reader();
    assign_bytes_per_sample();
    read_ext_text_headers();
    if (common.bin_hdr.byte_off_of_first_tr) {
        first_trace_pos = static_cast<long>(common.bin_hdr.byte_off_of_first_tr);
        common.file.seekg(first_trace_pos);
    } else {
        first_trace_pos = common.file.tellg();
    }
    common.samp_per_tr = common.bin_hdr.ext_samp_per_tr ? common.bin_hdr.ext_samp_per_tr : common.bin_hdr.samp_per_tr;
    read_trailer_stanzas();
    common.file.seekg(first_trace_pos);
    curr_pos = first_trace_pos;
    common.samp_buf.resize(static_cast<decltype(common.samp_buf.size())>(common.samp_per_tr * common.bytes_per_sample));
}

void ISegy::Impl::fill_bin_header(char const* buf)
{
    memcpy(&common.bin_hdr.endianness, buf + 96, sizeof(int32_t));
    assign_raw_readers();
    common.bin_hdr.job_id = static_cast<int32_t>(read_32(&buf));
    common.bin_hdr.line_num = static_cast<int32_t>(read_32(&buf));
    common.bin_hdr.reel_num = static_cast<int32_t>(read_32(&buf));
    common.bin_hdr.tr_per_ens = static_cast<int16_t>(read_16(&buf));
    common.bin_hdr.aux_per_ens = static_cast<int16_t>(read_16(&buf));
    common.bin_hdr.samp_int = static_cast<int16_t>(read_16(&buf));
    common.bin_hdr.samp_int_orig = static_cast<int16_t>(read_16(&buf));
    common.bin_hdr.samp_per_tr = static_cast<int16_t>(read_16(&buf));
    common.bin_hdr.samp_per_tr_orig = static_cast<int16_t>(read_16(&buf));
    common.bin_hdr.format_code = static_cast<int16_t>(read_16(&buf));
    common.bin_hdr.ens_fold = static_cast<int16_t>(read_16(&buf));
    common.bin_hdr.sort_code = static_cast<int16_t>(read_16(&buf));
    common.bin_hdr.vert_sum_code = static_cast<int16_t>(read_16(&buf));
    common.bin_hdr.sw_freq_at_start = static_cast<int16_t>(read_16(&buf));
    common.bin_hdr.sw_freq_at_end = static_cast<int16_t>(read_16(&buf));
    common.bin_hdr.sw_length = static_cast<int16_t>(read_16(&buf));
    common.bin_hdr.sw_type_code = static_cast<int16_t>(read_16(&buf));
    common.bin_hdr.sw_ch_tr_num = static_cast<int16_t>(read_16(&buf));
    common.bin_hdr.taper_at_start = static_cast<int16_t>(read_16(&buf));
    common.bin_hdr.taper_at_end = static_cast<int16_t>(read_16(&buf));
    common.bin_hdr.taper_type = static_cast<int16_t>(read_16(&buf));
    common.bin_hdr.corr_traces = static_cast<int16_t>(read_16(&buf));
    common.bin_hdr.bin_gain_recov = static_cast<int16_t>(read_16(&buf));
    common.bin_hdr.amp_recov_meth = static_cast<int16_t>(read_16(&buf));
    common.bin_hdr.measure_system = static_cast<int16_t>(read_16(&buf));
    common.bin_hdr.impulse_sig_pol = static_cast<int16_t>(read_16(&buf));
    common.bin_hdr.vib_pol_code = static_cast<int16_t>(read_16(&buf));
    common.bin_hdr.ext_tr_per_ens = static_cast<int32_t>(read_32(&buf));
    common.bin_hdr.ext_aux_per_ens = static_cast<int32_t>(read_32(&buf));
    common.bin_hdr.ext_samp_per_tr = static_cast<int32_t>(read_32(&buf));
    common.bin_hdr.ext_samp_int = dbl_from_IEEE_double(&buf);
    common.bin_hdr.ext_samp_int_orig = dbl_from_IEEE_double(&buf);
    common.bin_hdr.ext_samp_per_tr_orig = static_cast<int32_t>(read_32(&buf));
    common.bin_hdr.ext_ens_fold = static_cast<int32_t>(read_32(&buf));
    buf += 204; // skip unassigned fields and endianness
    common.bin_hdr.SEGY_rev_major_ver = read_8(&buf);
    common.bin_hdr.SEGY_rev_minor_ver = read_8(&buf);
    common.bin_hdr.fixed_tr_length = static_cast<int16_t>(read_16(&buf));
    common.bin_hdr.ext_text_headers_num = static_cast<int16_t>(read_16(&buf));
    common.bin_hdr.max_num_add_tr_headers = static_cast<int32_t>(read_32(&buf));
    common.bin_hdr.time_basis_code = static_cast<int16_t>(read_16(&buf));
    common.bin_hdr.num_of_tr_in_file = read_64(&buf);
    common.bin_hdr.byte_off_of_first_tr = read_64(&buf);
    common.bin_hdr.num_of_trailer_stanza = static_cast<int32_t>(read_32(&buf));
}

void ISegy::Impl::assign_raw_readers()
{
    read_8 = [](char const** buf) { return read<char>(buf); };
    switch (common.bin_hdr.endianness) {
    case 0x01020304:
        read_16 = [](char const** buf) { return read<uint16_t>(buf); };
        read_24 = [](char const** buf) { return read<uint16_t>(buf) | read<char>(buf) << 16; };
        read_32 = [](char const** buf) { return read<uint32_t>(buf); };
        read_64 = [](char const** buf) { return read<uint64_t>(buf); };
        break;
    case 0:
    case 0x04030201:
        read_16 = [](char const** buf) { return swap(read<uint16_t>(buf)); };
        read_24 = [](char const** buf) { return swap(read<uint16_t>(buf) | read<char>(buf) << 16); };
        read_32 = [](char const** buf) { return swap(read<uint32_t>(buf)); };
        read_64 = [](char const** buf) { return swap(read<uint64_t>(buf)); };
        break;
    default:
        throw Exception(__FILE__, __LINE__, "unsupported endianness");
    }
    if (FLT_RADIX == 2 && DBL_MANT_DIG == 53) {
        dbl_from_IEEE_float = [this](char const** buf) { return dbl_from_IEEE_float_native(buf); };
        dbl_from_IEEE_double = [this](char const** buf) { return dbl_from_IEEE_double_native(buf); };
    } else {
        dbl_from_IEEE_float = [this](char const** buf) { return dbl_from_IEEE_float_not_native(buf); };
        dbl_from_IEEE_double = [this](char const** buf) { return dbl_from_IEEE_double_not_native(buf); };
    }
}

void ISegy::Impl::assign_sample_reader()
{
    switch (common.bin_hdr.format_code) {
    case 1:
        read_sample = [this](char const** buf) { return dbl_from_ibm_float(buf); };
        break;
    case 2:
        read_sample = [this](char const** buf) { return dbl_from_int32(buf); };
        break;
    case 3:
        read_sample = [this](char const** buf) { return dbl_from_int16(buf); };
        break;
    case 5:
        read_sample = [this](char const** buf) { return dbl_from_IEEE_float(buf); };
        break;
    case 6:
        read_sample = [this](char const** buf) { return dbl_from_IEEE_double(buf); };
        break;
    case 7:
        read_sample = [this](char const** buf) { return dbl_from_int24(buf); };
        break;
    case 8:
        read_sample = [this](char const** buf) { return dbl_from_int8(buf); };
        break;
    case 9:
        read_sample = [this](char const** buf) { return dbl_from_int64(buf); };
        break;
    case 10:
        read_sample = [this](char const** buf) { return dbl_from_uint32(buf); };
        break;
    case 11:
        read_sample = [this](char const** buf) { return dbl_from_uint16(buf); };
        break;
    case 12:
        read_sample = [this](char const** buf) { return dbl_from_uint64(buf); };
        break;
    case 15:
        read_sample = [this](char const** buf) { return dbl_from_uint24(buf); };
        break;
    case 16:
        read_sample = [this](char const** buf) { return dbl_from_uint8(buf); };
        break;
    default:
        throw(Exception(__FILE__, __LINE__, "unsupported format"));
    }
}

void ISegy::Impl::read_ext_text_headers()
{
    int num = common.bin_hdr.ext_text_headers_num;
    if (!num)
        return;
    char buf[CommonSegy::TEXT_HEADER_SIZE];
    if (num == -1) {
        string end_stanza = "((SEG: EndText))";
        while (1) {
            common.file.read(buf, CommonSegy::TEXT_HEADER_SIZE);
            common.txt_hdrs.push_back(string(buf, CommonSegy::TEXT_HEADER_SIZE));
            if (!end_stanza.compare(0, end_stanza.size(), buf, end_stanza.size()))
                return;
        }
    } else {
        for (int i = common.bin_hdr.ext_text_headers_num; i; --i) {
            common.file.read(buf, CommonSegy::TEXT_HEADER_SIZE);
            common.txt_hdrs.push_back(string(buf, CommonSegy::TEXT_HEADER_SIZE));
        }
    }
}

void ISegy::Impl::read_trailer_stanzas()
{
    char text_buf[CommonSegy::TEXT_HEADER_SIZE];
    if (common.bin_hdr.num_of_trailer_stanza == -1) {
        if (!common.bin_hdr.num_of_tr_in_file)
            throw(Exception(__FILE__, __LINE__, "unable to determine end of trace data"));
        if (common.bin_hdr.fixed_tr_length) {
            // skip all traces
            common.file.seekg(common.bytes_per_sample * common.samp_per_tr * common.bin_hdr.num_of_tr_in_file, ios_base::cur);
            end_of_data = common.file.tellg();
            string end_stanza = "((SEG: EndText))";
            while (1) {
                common.file.read(text_buf, CommonSegy::TEXT_HEADER_SIZE);
                common.trlr_stnzs.push_back(string(text_buf, CommonSegy::TEXT_HEADER_SIZE));
                if (!end_stanza.compare(0, end_stanza.size(), text_buf, end_stanza.size()))
                    return;
            }
        } else {
            // variable trace length
            char trc_hdr_buf[CommonSegy::TR_HEADER_SIZE];
            for (auto i = common.bin_hdr.num_of_tr_in_file; i; --i) {
                common.file.read(trc_hdr_buf, CommonSegy::TR_HEADER_SIZE);
                // get number of samples from main header
                char const* ptr = trc_hdr_buf + 114;
                int32_t trc_samp_num = read_16(&ptr);
                if (common.bin_hdr.max_num_add_tr_headers) {
                    // if there are additional header(s)
                    // get number of samples from first additional header
                    common.file.read(trc_hdr_buf, CommonSegy::TR_HEADER_SIZE);
                    ptr = trc_hdr_buf + 136;
                    trc_samp_num = read_32(&ptr);
                    ptr = trc_hdr_buf + 156;
                    int16_t add_tr_hdr_num = read_16(&ptr);
                    add_tr_hdr_num = add_tr_hdr_num ? add_tr_hdr_num : common.bin_hdr.max_num_add_tr_headers;
                    // skip addional headers
                    common.file.seekg((add_tr_hdr_num - 1) * CommonSegy::TR_HEADER_SIZE,
                        ios_base::cur);
                }
                // skip trace samples
                common.file.seekg(trc_samp_num * common.bytes_per_sample, ios_base::cur);
            }
            end_of_data = common.file.tellg();
            string end_stanza = "((SEG: EndText))";
            while (1) {
                common.file.read(text_buf, CommonSegy::TEXT_HEADER_SIZE);
                common.trlr_stnzs.push_back(string(text_buf, CommonSegy::TEXT_HEADER_SIZE));
                if (!end_stanza.compare(0, end_stanza.size(), text_buf, end_stanza.size()))
                    return;
            }
        }
    } else if (common.bin_hdr.num_of_trailer_stanza) {
        // go to first trailer stanza
        common.file.seekg(common.bin_hdr.num_of_trailer_stanza * CommonSegy::TEXT_HEADER_SIZE,
            ios_base::end);
        end_of_data = common.file.tellg();
        for (int32_t i = common.bin_hdr.num_of_trailer_stanza; i; --i) {
            common.file.read(text_buf, CommonSegy::TEXT_HEADER_SIZE);
            common.trlr_stnzs.push_back(string(text_buf, CommonSegy::TEXT_HEADER_SIZE));
        }
    }
}

void ISegy::Impl::assign_bytes_per_sample()
{
    switch (common.bin_hdr.format_code) {
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

unordered_map<string, Trace::Header::Value> ISegy::Impl::read_trc_header()
{
    read_file(common.hdr_buf, CommonSegy::TR_HEADER_SIZE);
    char const* buf = common.hdr_buf;
    unordered_map<string, Trace::Header::Value> hdr;
    hdr["TRC_SEQ_LINE"] = static_cast<int32_t>(read_32(&buf));
    hdr["TRC_SEQ_SGY"] = static_cast<int32_t>(read_32(&buf));
    hdr["FFID"] = static_cast<int32_t>(read_32(&buf));
    hdr["CHAN"] = static_cast<int32_t>(read_32(&buf));
    hdr["ESP"] = static_cast<int32_t>(read_32(&buf));
    hdr["ENS_NO"] = static_cast<int32_t>(read_32(&buf));
    hdr["SEQ_NO"] = static_cast<int32_t>(read_32(&buf));
    hdr["TRACE_ID"] = static_cast<int16_t>(read_16(&buf));
    hdr["VERT_SUM"] = static_cast<int16_t>(read_16(&buf));
    hdr["HOR_SUM"] = static_cast<int16_t>(read_16(&buf));
    hdr["DATA_USE"] = static_cast<int16_t>(read_16(&buf));
    hdr["OFFSET"] = static_cast<int32_t>(read_32(&buf));
    hdr["R_ELEV"] = static_cast<int32_t>(read_32(&buf));
    hdr["S_ELEV"] = static_cast<int32_t>(read_32(&buf));
    hdr["S_DEPTH"] = static_cast<int32_t>(read_32(&buf));
    hdr["R_DATUM"] = static_cast<int32_t>(read_32(&buf));
    hdr["S_DATUM"] = static_cast<int32_t>(read_32(&buf));
    hdr["S_WATER"] = static_cast<int32_t>(read_32(&buf));
    hdr["R_WATER"] = static_cast<int32_t>(read_32(&buf));
    hdr["ELEV_SCALAR"] = static_cast<int16_t>(read_16(&buf));
    hdr["COORD_SCALAR"] = static_cast<int16_t>(read_16(&buf));
    hdr["SOU_X"] = static_cast<int32_t>(read_32(&buf));
    hdr["SOU_Y"] = static_cast<int32_t>(read_32(&buf));
    hdr["REC_X"] = static_cast<int32_t>(read_32(&buf));
    hdr["REC_Y"] = static_cast<int32_t>(read_32(&buf));
    hdr["COORD_UNITS"] = static_cast<int16_t>(read_16(&buf));
    hdr["WEATH_VEL"] = static_cast<int16_t>(read_16(&buf));
    hdr["SUBWEATH_VEL"] = static_cast<int16_t>(read_16(&buf));
    hdr["S_UPHOLE"] = static_cast<int16_t>(read_16(&buf));
    hdr["R_UPHOLE"] = static_cast<int16_t>(read_16(&buf));
    hdr["S_STAT"] = static_cast<int16_t>(read_16(&buf));
    hdr["R_STAT"] = static_cast<int16_t>(read_16(&buf));
    hdr["TOT_STAT"] = static_cast<int16_t>(read_16(&buf));
    hdr["LAG_A"] = static_cast<int16_t>(read_16(&buf));
    hdr["LAG_B"] = static_cast<int16_t>(read_16(&buf));
    hdr["DELAY_TIME"] = static_cast<int16_t>(read_16(&buf));
    hdr["MUTE_START"] = static_cast<int16_t>(read_16(&buf));
    hdr["MUTE_END"] = static_cast<int16_t>(read_16(&buf));
    hdr["SAMP_NUM"] = static_cast<int16_t>(read_16(&buf));
    hdr["SAMP_INT"] = static_cast<int16_t>(read_16(&buf));
    hdr["GAIN_TYPE"] = static_cast<int16_t>(read_16(&buf));
    hdr["GAIN_CONST"] = static_cast<int16_t>(read_16(&buf));
    hdr["INIT_GAIN"] = static_cast<int16_t>(read_16(&buf));
    hdr["CORRELATED"] = static_cast<int16_t>(read_16(&buf));
    hdr["SW_START"] = static_cast<int16_t>(read_16(&buf));
    hdr["SW_END"] = static_cast<int16_t>(read_16(&buf));
    hdr["SW_LENGTH"] = static_cast<int16_t>(read_16(&buf));
    hdr["SW_TYPE"] = static_cast<int16_t>(read_16(&buf));
    hdr["SW_TAPER_START"] = static_cast<int16_t>(read_16(&buf));
    hdr["SW_TAPER_END"] = static_cast<int16_t>(read_16(&buf));
    hdr["TAPER_TYPE"] = static_cast<int16_t>(read_16(&buf));
    hdr["ALIAS_FILT_FREQ"] = static_cast<int16_t>(read_16(&buf));
    hdr["ALIAS_FILT_SLOPE"] = static_cast<int16_t>(read_16(&buf));
    hdr["NOTCH_FILT_FREQ"] = static_cast<int16_t>(read_16(&buf));
    hdr["NOTCH_FILT_SLOPE"] = static_cast<int16_t>(read_16(&buf));
    hdr["LOW_CUT_FREQ"] = static_cast<int16_t>(read_16(&buf));
    hdr["HIGH_CUT_FREQ"] = static_cast<int16_t>(read_16(&buf));
    hdr["LOW_CUT_SLOPE"] = static_cast<int16_t>(read_16(&buf));
    hdr["HIGH_CUT_SLOPE"] = static_cast<int16_t>(read_16(&buf));
    hdr["YEAR"] = static_cast<int16_t>(read_16(&buf));
    hdr["DAY"] = static_cast<int16_t>(read_16(&buf));
    hdr["HOUR"] = static_cast<int16_t>(read_16(&buf));
    hdr["MINUTE"] = static_cast<int16_t>(read_16(&buf));
    hdr["SECOND"] = static_cast<int16_t>(read_16(&buf));
    hdr["TIME_BASIS_CODE"] = static_cast<int16_t>(read_16(&buf));
    hdr["TRACE_WEIGHT"] = static_cast<int16_t>(read_16(&buf));
    hdr["GROUP_NUM_ROLL"] = static_cast<int16_t>(read_16(&buf));
    hdr["GROUP_NUM_FIRST"] = static_cast<int16_t>(read_16(&buf));
    hdr["GROUP_NUM_LAST"] = static_cast<int16_t>(read_16(&buf));
    hdr["GAP_SIZE"] = static_cast<int16_t>(read_16(&buf));
    hdr["OVER_TRAVEL"] = static_cast<int16_t>(read_16(&buf));
    hdr["CDP_X"] = static_cast<int32_t>(read_32(&buf));
    hdr["CDP_Y"] = static_cast<int32_t>(read_32(&buf));
    hdr["INLINE"] = static_cast<int32_t>(read_32(&buf));
    hdr["XLINE"] = static_cast<int32_t>(read_32(&buf));
    hdr["SP_NUM"] = static_cast<int32_t>(read_32(&buf));
    hdr["SP_NUM_SCALAR"] = static_cast<int16_t>(read_16(&buf));
    hdr["TR_VAL_UNIT"] = static_cast<int16_t>(read_16(&buf));
    int32_t mant = static_cast<int32_t>(read_32(&buf));
    hdr["TRANS_CONST"] = mant * pow(10, static_cast<int16_t>(read_16(&buf)));
    hdr["TRANS_UNITS"] = static_cast<int16_t>(read_16(&buf));
    hdr["DEVICE_ID"] = static_cast<int16_t>(read_16(&buf));
    hdr["TIME_SCALAR"] = static_cast<int16_t>(read_16(&buf));
    hdr["SOURCE_TYPE"] = static_cast<int16_t>(read_16(&buf));
    hdr["SOU_V_DIR"] = static_cast<int16_t>(read_16(&buf));
    hdr["SOU_X_DIR"] = static_cast<int16_t>(read_16(&buf));
    hdr["SOU_I_DIR"] = static_cast<int16_t>(read_16(&buf));
    mant = static_cast<int32_t>(read_32(&buf));
    hdr["SOURCE_MEASUREMENT"] = mant * pow(10, static_cast<int16_t>(read_16(&buf)));
    hdr["SOU_MEAS_UNIT"] = static_cast<int16_t>(read_16(&buf));
    if (common.bin_hdr.max_num_add_tr_headers > 0) {
        read_file(common.hdr_buf, CommonSegy::TR_HEADER_SIZE);
        char const* buf = common.hdr_buf;
        hdr["TRC_SEQ_LINE"] = read_64(&buf);
        hdr["TRC_SEQ_SGY"] = read_64(&buf);
        hdr["FFID"] = read_64(&buf);
        hdr["ENS_NO"] = read_64(&buf);
        hdr["R_ELEV"] = dbl_from_IEEE_double(&buf);
        hdr["R_DEPTH"] = dbl_from_IEEE_double(&buf);
        hdr["S_DEPTH"] = dbl_from_IEEE_double(&buf);
        hdr["R_DATUM"] = dbl_from_IEEE_double(&buf);
        hdr["S_DATUM"] = dbl_from_IEEE_double(&buf);
        hdr["S_WATER"] = dbl_from_IEEE_double(&buf);
        hdr["R_WATER"] = dbl_from_IEEE_double(&buf);
        hdr["SOU_X"] = dbl_from_IEEE_double(&buf);
        hdr["SOU_Y"] = dbl_from_IEEE_double(&buf);
        hdr["REC_X"] = dbl_from_IEEE_double(&buf);
        hdr["REC_Y"] = dbl_from_IEEE_double(&buf);
        hdr["OFFSET"] = dbl_from_IEEE_double(&buf);
        hdr["SAMP_NUM"] = read_32(&buf);
        hdr["NANOSECOND"] = static_cast<int32_t>(read_32(&buf));
        double samp_int = dbl_from_IEEE_double(&buf);
        if (samp_int)
            hdr["SAMP_INT"] = samp_int;
        hdr["CABLE_NUM"] = static_cast<int32_t>(read_32(&buf));
        uint16_t add_trc_hdr_num = read_16(&buf);
        if (!add_trc_hdr_num)
            hdr["ADD_TRC_HDR_NUM"] = common.bin_hdr.max_num_add_tr_headers;
        else
            hdr["ADD_TRC_HDR_NUM"] = add_trc_hdr_num;
        hdr["LAST_TRC_FLAG"] = static_cast<int16_t>(read_16(&buf));
        hdr["CDP_X"] = dbl_from_IEEE_double(&buf);
        hdr["CDP_Y"] = dbl_from_IEEE_double(&buf);
    }
    return hdr;
}

valarray<double> ISegy::Impl::read_trc_smpls()
{
    read_file(common.samp_buf.data(), common.samp_buf.size());
    char const* buf = common.samp_buf.data();
    valarray<double> result(common.samp_buf.size() / common.bytes_per_sample);
    for (decltype(result.size()) i = 0; i < result.size(); ++i)
        result[i] = read_sample(&buf);
    return result;
}

valarray<double> ISegy::Impl::read_trc_smpls_var(uint32_t samp_num)
{
    if (common.samp_buf.size() != samp_num * common.bytes_per_sample)
        common.samp_buf.resize(samp_num * common.bytes_per_sample);
    return read_trc_smpls();
}

Trace ISegy::read_trace()
{
    unordered_map<string, Trace::Header::Value> hdr = pimpl->read_trc_header();
    auto it = hdr.find("ADD_TRC_HDR_NUM");
    if (it != hdr.end()) {
        int32_t trc_hrs_to_skip;
        Trace::Header::Value v = (*it).second;
        if (holds_alternative<uint16_t>(v))
            trc_hrs_to_skip = get<uint16_t>(v) - 1; // cos we already read one
        else
            trc_hrs_to_skip = get<int32_t>(v) - 1; // cos we already read one
        pimpl->common.file.seekg(CommonSegy::TR_HEADER_SIZE * trc_hrs_to_skip, ios_base::cur);
    }
    valarray<double> samples;
    if (pimpl->common.bin_hdr.fixed_tr_length || pimpl->common.bin_hdr.SEGY_rev_major_ver == 0) {
        samples = pimpl->read_trc_smpls();
    } else {
        uint32_t samp_num;
        Trace::Header::Value v = hdr["SAMP_NUM"];
        if (holds_alternative<uint32_t>(v))
            samp_num = get<uint32_t>(v);
        else
            samp_num = get<int16_t>(v);
        samples = pimpl->read_trc_smpls_var(samp_num);
    }
    return Trace(move(hdr), move(samples));
}

bool ISegy::has_next()
{
    if (pimpl->curr_pos == pimpl->end_of_data)
        return false;
    else
        return true;
}

double ISegy::Impl::dbl_from_ibm_float(char const** buf)
{
    uint32_t ibm = read_32(buf);
    int sign = ibm >> 31 ? -1 : 1;
    int exp = ibm >> 24 & 0x7f;
    uint32_t fraction = ibm & 0x00ffffff;

    return fraction / pow(2, 24) * pow(16, exp - 64) * sign;
}

double ISegy::Impl::dbl_from_IEEE_float_native(char const** buf)
{
    uint32_t tmp = read_32(buf);
    float result;
    memcpy(&result, &tmp, sizeof(result));
    return static_cast<double>(result);
}

double ISegy::Impl::dbl_from_IEEE_float_not_native(char const** buf)
{
    uint32_t tmp = read_32(buf);
    int sign = tmp >> 31 ? -1 : 1;
    int exp = (tmp & 0x7fffffff) >> 23;
    uint32_t fraction = tmp & 0x7fffff;
    return sign * pow(2, exp - 127) * (1 + fraction / pow(2, 23));
}

double ISegy::Impl::dbl_from_IEEE_double_native(char const** buf)
{
    uint64_t tmp = read_64(buf);
    double result;
    memcpy(&result, &tmp, sizeof(result));
    return result;
}

double ISegy::Impl::dbl_from_IEEE_double_not_native(char const** buf)
{
    uint64_t tmp = read_64(buf);
    int sign = tmp >> 63 ? -1 : 1;
    int exp = (tmp & 0x7fffffffffffffff) >> 52;
    uint64_t fraction = tmp & 0x000fffffffffffff;
    return sign * pow(2, exp - 1023) * (1 + fraction / pow(2, 52));
}

double ISegy::Impl::dbl_from_uint64(char const** buf)
{
    return static_cast<double>(read_64(buf));
}

double ISegy::Impl::dbl_from_int64(char const** buf)
{
    return static_cast<double>(static_cast<int64_t>(read_64(buf)));
}

double ISegy::Impl::dbl_from_uint32(char const** buf)
{
    return static_cast<double>(read_32(buf));
}

double ISegy::Impl::dbl_from_int32(char const** buf)
{
    return static_cast<double>(static_cast<int32_t>(read_32(buf)));
}

double ISegy::Impl::dbl_from_uint24(char const** buf)
{
    return static_cast<double>(read_24(buf));
}

double ISegy::Impl::dbl_from_int24(char const** buf)
{
    uint32_t tmp = read_24(buf);
    if (tmp & 0x800000)
        tmp |= 0xff000000;
    return static_cast<double>(static_cast<int32_t>(tmp));
}

double ISegy::Impl::dbl_from_uint16(char const** buf)
{
    return static_cast<double>(read_16(buf));
}

double ISegy::Impl::dbl_from_int16(char const** buf)
{
    return static_cast<double>(static_cast<int16_t>(read_16(buf)));
}

double ISegy::Impl::dbl_from_uint8(char const** buf)
{
    return static_cast<double>(read_8(buf));
}

double ISegy::Impl::dbl_from_int8(char const** buf)
{
    return static_cast<double>(static_cast<int8_t>(read_8(buf)));
}

vector<string> const& ISegy::text_headers() const
{
    return pimpl->common.txt_hdrs;
}

vector<string> const& ISegy::trailer_stanzas() const
{
    return pimpl->common.trlr_stnzs;
}

CommonSegy::BinaryHeader const& ISegy::binary_header() const
{
    return pimpl->common.bin_hdr;
}

ISegy::ISegy(string const& name)
    : pimpl(make_unique<Impl>(name))
{
}
ISegy::ISegy(string&& name)
    : pimpl(make_unique<Impl>(move(name)))
{
}

ISegy::~ISegy() = default;
} // namespace sedaman
