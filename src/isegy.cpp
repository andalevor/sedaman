#include <cfloat>
#include <climits>
#include <functional>
#include <fstream>
#include "isegy.hpp"
#include "sexception.hpp"
#include "util.hpp"

using std::function;
using std::ifstream;
using std::make_unique;
using std::move;
using std::streampos;
using std::string;
using std::vector;

namespace sedaman {
class isegy::impl {
public:
    void open_isegy(isegy &segy);
    int32_t tr_per_ens;
    int32_t aux_per_ens;
    int32_t samp_per_tr;
    double samp_int;
    double samp_int_orig;
    int32_t samp_per_tr_orig;
    int32_t ens_fold;
    streampos first_trace_pos;
    streampos curr_pos;
    streampos end_of_data;
private:
    void fill_bin_header(isegy &segy, const char *buf);
    void assign_raw_readers(isegy &segy);
    void assign_sample_reader(isegy &segy);
    void read_ext_text_headers(isegy &segy, ifstream &file);
    void assign_bytes_per_sample(isegy &segy);
    function<uint8_t(const char **)> read_8;
    function<uint16_t(const char **)> read_16;
    function<uint16_t(const char **)> read_24;
    function<uint32_t(const char **)> read_32;
    function<uint64_t(const char **)> read_64;
    function<double(isegy &segy, const char **)> read_sample;
    //functions for samples reading
    double dbl_from_ibm_float(const isegy &segy, const char **buf);
    double dbl_from_IEEE_float(const isegy &segy, const char **buf);
    double dbl_from_IEEE_float_not_native(const isegy &segy, const char **buf);
    double dbl_from_IEEE_double(const isegy &segy, const char **buf);
    double dbl_from_IEEE_double_not_native(const isegy &segy, const char **buf);
    double dbl_from_int64(const isegy &segy, const char **buf);
    double dbl_from_int32(const isegy &segy, const char **buf);
    double dbl_from_int24(const isegy &segy, const char **buf);
    double dbl_from_int16(const isegy &segy, const char **buf);
    double dbl_from_int8(const isegy &segy, const char **buf);
    double dbl_from_uint64(const isegy &segy, const char **buf);
    double dbl_from_uint32(const isegy &segy, const char **buf);
    double dbl_from_uint24(const isegy &segy, const char **buf);
    double dbl_from_uint16(const isegy &segy, const char **buf);
    double dbl_from_uint8(const isegy &segy, const char **buf);
};

void isegy::impl::open_isegy(isegy &segy)
{
    ifstream file;
    file.exceptions(ifstream::failbit | ifstream::badbit);
    file.open(segy.file_name(), ifstream::binary);
    char text_buf[segy::TEXT_HEADER_LEN];
    file.read(text_buf, segy::TEXT_HEADER_LEN);
    segy.txt_hdrs().push_back(string(text_buf, segy::TEXT_HEADER_LEN));
    char bin_buf[segy::BIN_HEADER_LEN];
    file.read(bin_buf, segy::BIN_HEADER_LEN);
    fill_bin_header(segy, bin_buf);
    assign_sample_reader(segy);
    assign_bytes_per_sample(segy);
    read_ext_text_headers(segy, file);
    if (segy.bin_hdr().byte_off_of_first_tr) {
        first_trace_pos = static_cast<long>(segy.bin_hdr().byte_off_of_first_tr);
    } else {
        first_trace_pos = file.tellg();
        if (first_trace_pos == -1)
            throw(sexception(__FILE__, __LINE__, "unable to get first byte after headers"));
    }
    if (segy.bin_hdr().num_of_trailer_stanza == -1) {
        if (!segy.bin_hdr().num_of_tr_in_file)
            throw(sexception(__FILE__, __LINE__, "unable to determine end of trace data"));
        // TODO: search for the byte offset of first stanza
    } else if (segy.bin_hdr().num_of_trailer_stanza) {
        file.seekg(segy.bin_hdr().num_of_trailer_stanza * segy::TEXT_HEADER_LEN,
                   std::ios_base::end);
        for (int32_t i = segy.bin_hdr().num_of_trailer_stanza; i; --i) {
            file.read(text_buf, segy::TEXT_HEADER_LEN);
            segy.trail_stnzs().push_back(string(text_buf, segy::TEXT_HEADER_LEN));
        }
    }
    segy.buffer().reserve(static_cast<decltype (segy.buffer().size())>(
                              samp_per_tr * segy.bytes_per_sample()));
}

void isegy::impl::fill_bin_header(isegy &segy, const char *buf)
{
    memcpy(&segy.bin_hdr().endianness, buf + 96, sizeof(int32_t));
    assign_raw_readers(segy);
    segy.bin_hdr().job_id = static_cast<int32_t>(read_32(&buf));
    segy.bin_hdr().line_num = static_cast<int32_t>(read_32(&buf));
    segy.bin_hdr().reel_num = static_cast<int32_t>(read_32(&buf));
    segy.bin_hdr().tr_per_ens = static_cast<int16_t>(read_16(&buf));
    segy.bin_hdr().aux_per_ens = static_cast<int16_t>(read_16(&buf));
    segy.bin_hdr().samp_int = static_cast<int16_t>(read_16(&buf));
    segy.bin_hdr().samp_int_orig = static_cast<int16_t>(read_16(&buf));
    segy.bin_hdr().samp_per_tr = static_cast<int16_t>(read_16(&buf));
    segy.bin_hdr().samp_per_tr_orig = static_cast<int16_t>(read_16(&buf));
    segy.bin_hdr().format_code = static_cast<int16_t>(read_16(&buf));
    segy.bin_hdr().ens_fold = static_cast<int16_t>(read_16(&buf));
    segy.bin_hdr().sort_code = static_cast<int16_t>(read_16(&buf));
    segy.bin_hdr().vert_sum_code = static_cast<int16_t>(read_16(&buf));
    segy.bin_hdr().sw_freq_at_start = static_cast<int16_t>(read_16(&buf));
    segy.bin_hdr().sw_freq_at_end = static_cast<int16_t>(read_16(&buf));
    segy.bin_hdr().sw_length = static_cast<int16_t>(read_16(&buf));
    segy.bin_hdr().sw_type_code = static_cast<int16_t>(read_16(&buf));
    segy.bin_hdr().sw_ch_tr_num = static_cast<int16_t>(read_16(&buf));
    segy.bin_hdr().taper_at_start = static_cast<int16_t>(read_16(&buf));
    segy.bin_hdr().taper_at_end = static_cast<int16_t>(read_16(&buf));
    segy.bin_hdr().taper_type = static_cast<int16_t>(read_16(&buf));
    segy.bin_hdr().corr_traces = static_cast<int16_t>(read_16(&buf));
    segy.bin_hdr().bin_gain_recov = static_cast<int16_t>(read_16(&buf));
    segy.bin_hdr().amp_recov_meth = static_cast<int16_t>(read_16(&buf));
    segy.bin_hdr().measure_system = static_cast<int16_t>(read_16(&buf));
    segy.bin_hdr().impulse_sig_pol = static_cast<int16_t>(read_16(&buf));
    segy.bin_hdr().vib_pol_code = static_cast<int16_t>(read_16(&buf));
    segy.bin_hdr().ext_tr_per_ens = static_cast<int32_t>(read_32(&buf));
    segy.bin_hdr().ext_aux_per_ens = static_cast<int32_t>(read_32(&buf));
    segy.bin_hdr().ext_samp_per_tr = static_cast<int32_t>(read_32(&buf));
    segy.bin_hdr().ext_samp_int = dbl_from_IEEE_double(segy, &buf);
    segy.bin_hdr().ext_samp_int_orig = dbl_from_IEEE_double(segy, &buf);
    segy.bin_hdr().ext_samp_per_tr_orig = static_cast<int32_t>(read_32(&buf));
    segy.bin_hdr().ext_ens_fold = static_cast<int32_t>(read_32(&buf));
    buf += 200; // skip unassigned fields
    segy.bin_hdr().SEGY_rev_major_ver = read_8(&buf);
    segy.bin_hdr().SEGY_rev_minor_ver = read_8(&buf);
    segy.bin_hdr().fixed_tr_length = static_cast<int16_t>(read_16(&buf));
    segy.bin_hdr().ext_text_headers_num = static_cast<int16_t>(read_16(&buf));
    segy.bin_hdr().max_num_add_tr_headers = static_cast<int32_t>(read_32(&buf));
    segy.bin_hdr().time_basis_code = static_cast<int16_t>(read_16(&buf));
    segy.bin_hdr().num_of_tr_in_file = read_64(&buf);
    segy.bin_hdr().byte_off_of_first_tr = read_64(&buf);
    segy.bin_hdr().num_of_trailer_stanza = static_cast<int32_t>(read_32(&buf));

    tr_per_ens = segy.bin_hdr().ext_tr_per_ens ? segy.bin_hdr().ext_tr_per_ens :
                                                 segy.bin_hdr().tr_per_ens;
    aux_per_ens = segy.bin_hdr().ext_aux_per_ens ? segy.bin_hdr().ext_aux_per_ens :
                                                   segy.bin_hdr().aux_per_ens;
    samp_per_tr = segy.bin_hdr().ext_samp_per_tr ? segy.bin_hdr().ext_samp_per_tr :
                                                   segy.bin_hdr().samp_per_tr;
    samp_int = segy.bin_hdr().ext_samp_int != 0.0 ? segy.bin_hdr().ext_samp_int :
                                                    segy.bin_hdr().samp_int;
    samp_int_orig = segy.bin_hdr().ext_samp_int_orig != 0.0 ? segy.bin_hdr().ext_samp_int_orig :
                                                              segy.bin_hdr().samp_int_orig;
    samp_per_tr_orig = segy.bin_hdr().ext_samp_per_tr_orig ? segy.bin_hdr().ext_samp_per_tr_orig :
                                                             segy.bin_hdr().samp_per_tr_orig;
    ens_fold = segy.bin_hdr().ext_ens_fold ? segy.bin_hdr().ext_ens_fold :
                                             segy.bin_hdr().ens_fold;
}

void isegy::impl::assign_raw_readers(isegy &segy)
{
    read_8 = [](const char **buf) {return read<uint8_t>(buf);};
    switch (segy.bin_hdr().endianness) {
    case 0x01020304:
        read_16 = [](const char **buf) {return read<uint16_t>(buf);};
        read_24 = [](const char **buf) {return read<uint16_t>(buf) |
                    (read<uint8_t>(buf) << 16);};
        read_32 = [](const char **buf) {return read<uint32_t>(buf);};
        read_64 = [](const char **buf) {return read<uint64_t>(buf);};
        break;
    case 0:
    case 0x04030201:
        read_16 = [](const char **buf) {return swap(read<uint16_t>(buf));};
        read_24 = [](const char **buf) {return (read<uint8_t>(buf) << 16) |
                    swap(read<uint16_t>(buf));};
        read_32 = [](const char **buf) {return swap(read<uint32_t>(buf));};
        read_64 = [](const char **buf) {return swap(read<uint64_t>(buf));};
        break;
    default:
        throw sexception(__FILE__, __LINE__, "unsupported endianness");
    }
}

void isegy::impl::assign_sample_reader(isegy &segy)
{
    switch (segy.bin_hdr().format_code) {
    case 1:
        segy.pimpl->read_sample = [this] (isegy &segy, const char **buf)
        {return dbl_from_ibm_float(segy, buf);};
        break;
    case 2:
        segy.pimpl->read_sample = [this] (isegy &segy, const char **buf)
        {return dbl_from_int32(segy, buf);};
        break;
    case 3:
        segy.pimpl->read_sample = [this] (isegy &segy, const char **buf)
        {return dbl_from_int16(segy, buf);};
        break;
    case 5:
        if (FLT_RADIX == 2 && DBL_MANT_DIG == 53)
            segy.pimpl->read_sample = [this] (isegy &segy, const char **buf)
            {return dbl_from_IEEE_float(segy, buf);};
        else
            segy.pimpl->read_sample = [this] (isegy &segy, const char **buf)
            {return dbl_from_IEEE_float_not_native(segy, buf);};
        break;
    case 6:
        if (FLT_RADIX == 2 && DBL_MANT_DIG == 53)
            segy.pimpl->read_sample = [this] (isegy &segy, const char **buf)
            {return dbl_from_IEEE_double(segy, buf);};
        else
            segy.pimpl->read_sample = [this] (isegy &segy, const char **buf)
            {return dbl_from_IEEE_double_not_native(segy, buf);};
        break;
    case 7:
        segy.pimpl->read_sample = [this] (isegy &segy, const char **buf)
        {return dbl_from_int24(segy, buf);};
        break;
    case 8:
        segy.pimpl->read_sample = [this] (isegy &segy, const char **buf)
        {return dbl_from_int8(segy, buf);};
        break;
    case 9:
        segy.pimpl->read_sample = [this] (isegy &segy, const char **buf)
        {return dbl_from_int64(segy, buf);};
        break;
    case 10:
        segy.pimpl->read_sample = [this] (isegy &segy, const char **buf)
        {return dbl_from_uint32(segy, buf);};
        break;
    case 11:
        segy.pimpl->read_sample = [this] (isegy &segy, const char **buf)
        {return dbl_from_uint16(segy, buf);};
        break;
    case 12:
        segy.pimpl->read_sample = [this] (isegy &segy, const char **buf)
        {return dbl_from_uint64(segy, buf);};
        break;
    case 15:
        segy.pimpl->read_sample = [this] (isegy &segy, const char **buf)
        {return dbl_from_uint24(segy, buf);};
        break;
    case 16:
        segy.pimpl->read_sample = [this] (isegy &segy, const char **buf)
        {return dbl_from_uint8(segy, buf);};
        break;
    default:
        throw(sexception(__FILE__, __LINE__, "unsupported format"));
    }
}

void isegy::impl::read_ext_text_headers(isegy &segy, ifstream &file)
{
    int num = segy.bin_hdr().ext_text_headers_num;
    if (!num)
        return;
    char buf[segy::TEXT_HEADER_LEN];
    if (num == -1) {
        string end_stanza = "((SEG: EndText))";
        file.read(buf, segy::TEXT_HEADER_LEN);
        while (1) {
            file.read(buf, segy::TEXT_HEADER_LEN);
            segy.txt_hdrs().push_back(string(buf, segy::TEXT_HEADER_LEN));
            if (!end_stanza.compare(0, end_stanza.size(), buf, end_stanza.size()))
                return;
        }
    } else {
        for (int i = segy.bin_hdr().ext_text_headers_num; i; --i) {
            file.read(buf, segy::TEXT_HEADER_LEN);
            segy.txt_hdrs().push_back(string(buf, segy::TEXT_HEADER_LEN));
        }
    }
}

void isegy::impl::assign_bytes_per_sample(isegy &segy)
{
    switch (segy.bin_hdr().format_code) {
    case 1:
    case 2:
    case 4:
    case 5:
    case 10:
        segy.set_bytes_per_sample(4);
        break;
    case 3:
    case 11:
        segy.set_bytes_per_sample(2);
        break;
    case 6:
    case 9:
    case 12:
        segy.set_bytes_per_sample(8);
        break;
    case 7:
    case 15:
        segy.set_bytes_per_sample(3);
        break;
    case 8:
    case 16:
        segy.set_bytes_per_sample(1);
    };
}

double isegy::impl::dbl_from_ibm_float(const isegy &segy, const char **buf)
{
    uint32_t ibm, fraction;
    int sign, exp;

    ibm = segy.pimpl->read_32(buf);
    sign = ibm >> 31 ? -1 : 1;
    exp = ibm >> 24 & 0x7f;
    fraction = ibm & 0x00ffffff;

    return fraction / pow(2, 24) * pow(16, exp - 64) * sign;
}

double isegy::impl::dbl_from_IEEE_float(const isegy &segy, const char **buf)
{
    uint32_t tmp;
    float result;

    tmp = segy.pimpl->read_32(buf);
    memcpy(&result, &tmp, sizeof(result));
    return static_cast<double>(result);
}

double isegy::impl::dbl_from_IEEE_float_not_native(const isegy &segy,
                                                   const char **buf)
{
    uint32_t tmp, fraction;
    int sign, exp;

    tmp = segy.pimpl->read_32(buf);
    sign = tmp >> 31 ? -1 : 1;
    exp = (tmp & 0x7fffffff) >> 23;
    fraction = tmp & 0x7fffff;
    return sign * pow(2, exp - 127) * (1 + fraction / pow(2, 23));
}

double isegy::impl::dbl_from_IEEE_double(const isegy &segy, const char **buf)
{
    uint64_t tmp;
    double result;

    tmp = segy.pimpl->read_64(buf);
    memcpy(&result, &tmp, sizeof(result));
    return result;
}

double isegy::impl::dbl_from_IEEE_double_not_native(const isegy &segy,
                                                    const char **buf)
{
    uint64_t fraction, tmp;
    int sign, exp;

    tmp = segy.pimpl->read_64(buf);
    sign = tmp >> 63 ? -1 : 1;
    exp = (tmp & 0x7fffffffffffffff) >> 52;
    fraction = tmp & 0x000fffffffffffff;
    return sign * pow(2, exp - 1023) * (1 + fraction / pow(2, 52));
}

double isegy::impl::dbl_from_uint64(const isegy &segy, const char **buf)
{
    return static_cast<double>(segy.pimpl->read_64(buf));
}

double isegy::impl::dbl_from_int64(const isegy &segy, const char **buf)
{
    return static_cast<double>(static_cast<int64_t>(segy.pimpl->read_64(buf)));
}

double isegy::impl::dbl_from_uint32(const isegy &segy, const char **buf)
{
    return static_cast<double>(segy.pimpl->read_32(buf));
}

double isegy::impl::dbl_from_int32(const isegy &segy, const char **buf)
{
    return static_cast<double>(static_cast<int32_t>(segy.pimpl->read_32(buf)));
}

double isegy::impl::dbl_from_uint24(const isegy &segy, const char **buf)
{
    return static_cast<double>(segy.pimpl->read_24(buf));
}

double isegy::impl::dbl_from_int24(const isegy &segy, const char **buf)
{
    uint32_t tmp = segy.pimpl->read_24(buf);
    if (tmp & 0x800000)
        tmp |= 0xff000000;
    return static_cast<double>(static_cast<int32_t>(tmp));
}

double isegy::impl::dbl_from_uint16(const isegy &segy, const char **buf)
{
    return static_cast<double>(segy.pimpl->read_16(buf));
}

double isegy::impl::dbl_from_int16(const isegy &segy, const char **buf)
{
    return static_cast<double>(static_cast<int16_t>(segy.pimpl->read_16(buf)));
}

double isegy::impl::dbl_from_uint8(const isegy &segy, const char **buf)
{
    return static_cast<double>(segy.pimpl->read_8(buf));
}

double isegy::impl::dbl_from_int8(const isegy &segy, const char **buf)
{
    return static_cast<double>(static_cast<int8_t>(segy.pimpl->read_8(buf)));
}

vector<string> const &isegy::text_headers()
{
    return txt_hdrs();
}

vector<string> const &isegy::trailer_stanzas()
{
    return trail_stnzs();
}

segy::binary_header const &isegy::binary_header()
{
    return bin_hdr();
}

isegy::isegy(string const &file_name)
    : segy(file_name), pimpl(make_unique<impl>())
{
    pimpl->open_isegy(*this);
}

isegy::isegy(string &&file_name)
    : segy(move(file_name)), pimpl(make_unique<impl>())
{
    pimpl->open_isegy(*this);
}

isegy::~isegy() = default;
}
