#include <functional>
#include <fstream>
#include "isegy.hpp"
#include "sexception.hpp"
#include "util.hpp"

using std::function;
using std::ifstream;
using std::make_unique;
using std::move;
using std::string;

namespace sedaman {
//functions for samples reading
static double dbl_from_ibm_float(const isegy &segy, const char **buf);
static double dbl_from_IEEE_float(const isegy &segy, const char **buf);
static double dbl_from_IEEE_double(const isegy &segy, const char **buf);
static double dbl_from_int64(const isegy &segy, const char **buf);
static double dbl_from_int32(const isegy &segy, const char **buf);
static double dbl_from_int24(const isegy &segy, const char **buf);
static double dbl_from_int16(const isegy &segy, const char **buf);
static double dbl_from_int8(const isegy &segy, const char **buf);
static double dbl_from_uint64(const isegy &segy, const char **buf);
static double dbl_from_uint32(const isegy &segy, const char **buf);
static double dbl_from_uint24(const isegy &segy, const char **buf);
static double dbl_from_uint16(const isegy &segy, const char **buf);
static double dbl_from_uint8(const isegy &segy, const char **buf);

class isegy::impl {
public:
    void open_isegy(isegy &segy);
    double samp_int;
    int32_t samp_per_tr;
private:
    void fill_bin_header(isegy &segy, const char *buf);
    void assign_raw_readers(isegy &segy);
    function<uint8_t(const char **)> read_8;
    function<uint16_t(const char **)> read_16;
    function<uint32_t(const char **)> read_32;
    function<uint64_t(const char **)> read_64;
};

void isegy::impl::open_isegy(isegy &segy)
{
    ifstream file;
    file.exceptions(ifstream::failbit | ifstream::badbit);
    file.open(segy.file_name(), ifstream::binary);
    char text_buf[segy::TEXT_HEADER_LEN];
    file.read(text_buf, segy::TEXT_HEADER_LEN);
    segy.set_text_hdr(string(text_buf, segy::TEXT_HEADER_LEN));
    char bin_buf[segy::BIN_HEADER_LEN];
    file.read(bin_buf, segy::BIN_HEADER_LEN);
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

    if (segy.bin_hdr().ext_samp_int != 0.0)
        samp_int = segy.bin_hdr().ext_samp_int;
    else
        samp_int = segy.bin_hdr().samp_int;

    if (segy.bin_hdr().ext_samp_per_tr)
        samp_per_tr = segy.bin_hdr().ext_samp_per_tr;
    else
        samp_per_tr = segy.bin_hdr().samp_per_tr;
}

void isegy::impl::assign_raw_readers(isegy &segy)
{
    switch (segy.bin_hdr().endianness) {
    case 0x01020304:
        read_8 = [](const char **buf) {return read<uint8_t>(buf);};
        read_16 = [](const char **buf) {return read<uint16_t>(buf);};
        read_32 = [](const char **buf) {return read<uint32_t>(buf);};
        read_64 = [](const char **buf) {return read<uint64_t>(buf);};
        break;
    case 0:
    case 0x04030201:
        read_8 = [](const char **buf) {return swap(read<uint8_t>(buf));};
        read_16 = [](const char **buf) {return swap(read<uint16_t>(buf));};
        read_32 = [](const char **buf) {return swap(read<uint32_t>(buf));};
        read_64 = [](const char **buf) {return swap(read<uint64_t>(buf));};
        break;
    default:
        throw sexception(__FILE__, __LINE__, "unsupported endianness");
    }
}

std::string const &isegy::text_header()
{
    return segy::text_hdr();
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
