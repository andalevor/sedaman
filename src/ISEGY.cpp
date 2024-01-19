#include "ISEGY.hpp"
#include "CommonSEGY.hpp"
#include "Exception.hpp"
#include "Trace.hpp"
#include "util.hpp"
#include <cfloat>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <functional>
#include <ios>
#include <string>
#include <unordered_map>

using std::fstream;
using std::function;
using std::get;
using std::ios_base;
using std::make_unique;
using std::map;
using std::move;
using std::pair;
using std::streamoff;
using std::streampos;
using std::streamsize;
using std::string;
using std::unordered_map;
using std::vector;

namespace sedaman {
class ISEGY::Impl {
public:
    Impl(string name, vector<pair<string, map<uint32_t,
 		 pair<string, Trace::Header::ValueType>>>> hdr_map)
		: common { move(name), fstream::in | fstream::binary, {},
		   	move(hdr_map) }
    {
        initialization(false);
    }

    Impl(string name, CommonSEGY::BinaryHeader bh,
        vector<pair<string, map<uint32_t, pair<string,
	   	Trace::Header::ValueType>>>> hdr_map)
        : common { move(name), fstream::in | fstream::binary,
		   	move(bh), move(hdr_map) }
    {
        initialization(true);
    }

    CommonSEGY common;
    streampos first_trace_pos;
    streampos curr_pos;
    streampos end_of_data;
    unordered_map<string, Trace::Header::Value> read_trc_header();
    function<vector<double>(unordered_map<string, Trace::Header::Value>&)>
	   	read_trc_smpls;
    vector<double> read_trc_smpls_fix();
    vector<double> read_trc_smpls_var(unordered_map<string,
									  Trace::Header::Value>& hdr);
    void file_skip_bytes(streamoff off);
    vector<map<uint32_t, pair<string, Trace::Header::ValueType>>>
	   	tr_hdr_default_io_map();

private:
    function<uint8_t(char const**)> read_u8;
    function<int8_t(char const**)> read_i8;
    function<uint16_t(char const**)> read_u16;
    function<int16_t(char const**)> read_i16;
    function<uint32_t(char const**)> read_u24;
    function<int32_t(char const**)> read_i24;
    function<uint32_t(char const**)> read_u32;
    function<int32_t(char const**)> read_i32;
    function<uint64_t(char const**)> read_u64;
    function<int64_t(char const**)> read_i64;
    function<double(char const**)> read_sample;
    function<double(char const** buf)> dbl_from_ibm_float;
    function<double(char const** buf)> dbl_from_IEEE_float;
    function<double(char const** buf)> dbl_from_IEEE_double;
    void initialization(bool override_bin_hdr);
    void fill_bin_header(char const* buf, bool override_bin_hdr);
    void assign_raw_readers();
    void assign_sample_reader();
    void read_ext_text_headers();
    void assign_bytes_per_sample();
    void read_trailer_stanzas();
    void fill_buf_from_file(char* buf, streamsize n);
};

void ISEGY::Impl::initialization(bool override_bin_hdr)
{
    char text_buf[CommonSEGY::TEXT_HEADER_SIZE];
    common.file.read(text_buf, CommonSEGY::TEXT_HEADER_SIZE);
    common.text_headers.emplace_back(text_buf, CommonSEGY::TEXT_HEADER_SIZE);
    char bin_buf[CommonSEGY::BIN_HEADER_SIZE];
    common.file.read(bin_buf, CommonSEGY::BIN_HEADER_SIZE);
	fill_bin_header(bin_buf, override_bin_hdr);
    assign_sample_reader();
    assign_bytes_per_sample();
    read_ext_text_headers();
	char const *pos = common.hdr_buf;
	read_i32(&pos);
    if (common.binary_header.byte_off_of_first_tr) {
        first_trace_pos =
		   	static_cast<long>(common.binary_header.byte_off_of_first_tr);
        common.file.seekg(first_trace_pos);
    } else {
        first_trace_pos = common.file.tellg();
    }
    common.samp_per_tr = common.binary_header.ext_samp_per_tr ?
	   	common.binary_header.ext_samp_per_tr :
		/* static cast for the case you will get SEGY prior rev2 with samples
		 * more then int16_t can hold without wrap. i got one */
	   	static_cast<uint16_t>(common.binary_header.samp_per_tr);
    read_trailer_stanzas();
    common.file.seekg(first_trace_pos);
    curr_pos = first_trace_pos;
    common.samp_buf.resize(static_cast<decltype(common.samp_buf.size())>(
        common.samp_per_tr * common.bytes_per_sample));
	if (common.binary_header.fixed_tr_length ||
		common.binary_header.SEGY_rev_major_ver == 0)
		read_trc_smpls =
			[this](unordered_map<string,
				   Trace::Header::Value>& hdr)
			{ (void)hdr; return read_trc_smpls_fix(); };
	else
		read_trc_smpls =
			[this](unordered_map<string,
				   Trace::Header::Value>& hdr)
			{ return read_trc_smpls_var(hdr); };
}

void ISEGY::Impl::fill_buf_from_file(char* buf, streamsize n)
{
    common.file.read(buf, n);
    curr_pos = common.file.tellg();
}

void ISEGY::Impl::file_skip_bytes(streamoff off)
{
    common.file.seekg(off, ios_base::cur);
    curr_pos = common.file.tellg();
}

void ISEGY::Impl::fill_bin_header(char const* buf, bool override_bin_hdr)
{
    memcpy(&common.binary_header.endianness, buf + 96, sizeof(int32_t));
    assign_raw_readers();
    if (override_bin_hdr)
		return;
    common.binary_header.job_id = read_i32(&buf);
    common.binary_header.line_num = read_i32(&buf);
    common.binary_header.reel_num = read_i32(&buf);
    common.binary_header.tr_per_ens = read_i16(&buf);
    common.binary_header.aux_per_ens = read_i16(&buf);
    common.binary_header.samp_int = read_i16(&buf);
    common.binary_header.samp_int_orig = read_i16(&buf);
    common.binary_header.samp_per_tr = read_i16(&buf);
    common.binary_header.samp_per_tr_orig = read_i16(&buf);
    common.binary_header.format_code = read_i16(&buf);
    common.binary_header.ens_fold = read_i16(&buf);
    common.binary_header.sort_code = read_i16(&buf);
    common.binary_header.vert_sum_code = read_i16(&buf);
    common.binary_header.sw_freq_at_start = read_i16(&buf);
    common.binary_header.sw_freq_at_end = read_i16(&buf);
    common.binary_header.sw_length = read_i16(&buf);
    common.binary_header.sw_type_code = read_i16(&buf);
    common.binary_header.sw_ch_tr_num = read_i16(&buf);
    common.binary_header.taper_at_start = read_i16(&buf);
    common.binary_header.taper_at_end = read_i16(&buf);
    common.binary_header.taper_type = read_i16(&buf);
    common.binary_header.corr_traces = read_i16(&buf);
    common.binary_header.bin_gain_recov = read_i16(&buf);
    common.binary_header.amp_recov_meth = read_i16(&buf);
    common.binary_header.measure_system = read_i16(&buf);
    common.binary_header.impulse_sig_pol = read_i16(&buf);
    common.binary_header.vib_pol_code = read_i16(&buf);
    common.binary_header.ext_tr_per_ens = read_i32(&buf);
    common.binary_header.ext_aux_per_ens = read_i32(&buf);
    common.binary_header.ext_samp_per_tr = read_i32(&buf);
    common.binary_header.ext_samp_int = dbl_from_IEEE_double(&buf);
    common.binary_header.ext_samp_int_orig = dbl_from_IEEE_double(&buf);
    common.binary_header.ext_samp_per_tr_orig = read_i32(&buf);
    common.binary_header.ext_ens_fold = read_i32(&buf);
    buf += 204; // skip unassigned fields and endianness
    common.binary_header.SEGY_rev_major_ver = read_u8(&buf);
    common.binary_header.SEGY_rev_minor_ver = read_u8(&buf);
    common.binary_header.fixed_tr_length = read_i16(&buf);
    common.binary_header.ext_text_headers_num = read_i16(&buf);
	if (common.binary_header.SEGY_rev_major_ver > 1) {
		common.binary_header.max_num_add_tr_headers = read_i32(&buf);
		common.binary_header.time_basis_code = read_i16(&buf);
		common.binary_header.num_of_tr_in_file = read_i64(&buf);
		common.binary_header.byte_off_of_first_tr = read_u64(&buf);
		common.binary_header.num_of_trailer_stanza = read_i32(&buf);
	}
}

void ISEGY::Impl::assign_raw_readers()
{
    read_u8 = [](char const** buf) { return read<uint8_t>(buf); };
    read_i8 = [](char const** buf) { return read<int8_t>(buf); };
    switch (common.binary_header.endianness) {
    case 0x01020304:
        read_u16 = [](char const** buf) { return read<uint16_t>(buf); };
        read_i16 = [](char const** buf) { return read<int16_t>(buf); };
        read_u24 = [](char const** buf) { return read<uint16_t>(buf) |
		   	read<uint8_t>(buf) << 16; };
        read_i24 = [](char const** buf) {
            uint32_t result = read<uint16_t>(buf) | read<uint8_t>(buf) << 16;
            return result & 0x800000 ? result | 0xff000000 : result;
        };
        read_u32 = [](char const** buf) { return read<uint32_t>(buf); };
        read_i32 = [](char const** buf) { return read<int32_t>(buf); };
        read_u64 = [](char const** buf) { return read<uint64_t>(buf); };
        read_i64 = [](char const** buf) { return read<int64_t>(buf); };
        break;
    case 0:
    case 0x04030201:
        read_u16 = [](char const** buf) { return swap(read<uint16_t>(buf)); };
        read_i16 = [](char const** buf) { return swap(read<int16_t>(buf)); };
        read_u24 = [](char const** buf) { return swap(read<uint16_t>(buf)) << 8
		   	| read<uint8_t>(buf); };
        read_i24 = [](char const** buf) {
            uint32_t result = swap(read<uint16_t>(buf)) << 8 |
			   	read<uint8_t>(buf);
            return result & 0x800000 ? result | 0xff000000 : result;
        };
        read_u32 = [](char const** buf) { return swap(read<uint32_t>(buf)); };
        read_i32 = [](char const** buf) { return swap(read<int32_t>(buf)); };
        read_u64 = [](char const** buf) { return swap(read<uint64_t>(buf)); };
        read_i64 = [](char const** buf) { return swap(read<int64_t>(buf)); };
        break;
    default:
        throw Exception(__FILE__, __LINE__, "unsupported endianness");
    }
    if (FLT_RADIX == 2 && DBL_MANT_DIG == 53) {
        dbl_from_IEEE_float = [this](char const** buf) {
            uint32_t tmp = read_u32(buf);
            float result;
            memcpy(&result, &tmp, sizeof(result));
            return static_cast<double>(result);
        };
        dbl_from_IEEE_double = [this](char const** buf) {
            uint64_t tmp = read_u64(buf);
            double result;
            memcpy(&result, &tmp, sizeof(result));
            return result;
        };
    } else {
        dbl_from_IEEE_float = [this](char const** buf) {
            uint32_t tmp = read_u32(buf);
            int sign = tmp >> 31 ? -1 : 1;
            int exp = (tmp & 0x7fffffff) >> 23;
            uint32_t fraction = tmp & 0x7fffff;
            return sign * pow(2, exp - 127) * (1 + fraction / pow(2, 23));
        };
        dbl_from_IEEE_double = [this](char const** buf) {
            uint64_t tmp = read_u64(buf);
            int sign = tmp >> 63 ? -1 : 1;
            int exp = (tmp & 0x7fffffffffffffff) >> 52;
            uint64_t fraction = tmp & 0x000fffffffffffff;
            return sign * pow(2, exp - 1023) * (1 + fraction / pow(2, 52));
        };
    }
    dbl_from_ibm_float = [this](char const** buf) {
        uint32_t ibm = read_u32(buf);
        int sign = ibm >> 31 ? -1 : 1;
        int exp = ibm >> 24 & 0x7f;
        double fraction = ibm & 0x00ffffff;

        return fraction / pow(2, 24) * pow(16, exp - 64) * sign;
    };
}

void ISEGY::Impl::assign_sample_reader()
{
    switch (common.binary_header.format_code) {
    case 1:
        read_sample = dbl_from_ibm_float;
        break;
    case 2:
        read_sample = [this](char const** buf) { return read_i32(buf); };
        break;
    case 3:
        read_sample = [this](char const** buf) { return read_i16(buf); };
        break;
    case 5:
        read_sample = dbl_from_IEEE_float;
        break;
    case 6:
        read_sample = dbl_from_IEEE_double;
        break;
    case 7:
        read_sample = [this](char const** buf) { return read_i24(buf); };
        break;
    case 8:
        read_sample = [this](char const** buf) { return read_i8(buf); };
        break;
    case 9:
        read_sample = [this](char const** buf) { return read_i64(buf); };
        break;
    case 10:
        read_sample = [this](char const** buf) { return read_u32(buf); };
        break;
    case 11:
        read_sample = [this](char const** buf) { return read_u16(buf); };
        break;
    case 12:
        read_sample = [this](char const** buf) { return read_u64(buf); };
        break;
    case 15:
        read_sample = [this](char const** buf) { return read_u24(buf); };
        break;
    case 16:
        read_sample = [this](char const** buf) { return read_u8(buf); };
        break;
    default:
        throw(Exception(__FILE__, __LINE__, "unsupported format"));
    }
}

void ISEGY::Impl::read_ext_text_headers()
{
    int num = common.binary_header.ext_text_headers_num;
    if (!num)
        return;
    char buf[CommonSEGY::TEXT_HEADER_SIZE];
    if (num == -1) {
        string end_stanza = "((SEG: EndText))";
        while (1) {
            common.file.read(buf, CommonSEGY::TEXT_HEADER_SIZE);
            common.text_headers.emplace_back(buf,
											 CommonSEGY::TEXT_HEADER_SIZE);
            if (!end_stanza.compare(0, end_stanza.size(), buf,
								   	end_stanza.size()))
                return;
        }
    } else {
        for (int i = common.binary_header.ext_text_headers_num; i; --i) {
            common.file.read(buf, CommonSEGY::TEXT_HEADER_SIZE);
            common.text_headers.emplace_back(buf,
											 CommonSEGY::TEXT_HEADER_SIZE);
        }
    }
}

void ISEGY::Impl::read_trailer_stanzas()
{
    char text_buf[CommonSEGY::TEXT_HEADER_SIZE];
    if (common.binary_header.num_of_trailer_stanza == -1) {
        if (!common.binary_header.num_of_tr_in_file)
            throw(Exception(__FILE__, __LINE__,
						   	"unable to determine end of trace data"));
        if (common.binary_header.fixed_tr_length) {
            // skip all traces
            common.file.seekg(common.bytes_per_sample * common.samp_per_tr *
							  common.binary_header.num_of_tr_in_file,
							  ios_base::cur);
            end_of_data = common.file.tellg();
            string end_stanza = "((SEG: EndText))";
            while (1) {
                common.file.read(text_buf, CommonSEGY::TEXT_HEADER_SIZE);
                common.trailer_stanzas
					.emplace_back(text_buf,	CommonSEGY::TEXT_HEADER_SIZE);
                if (!end_stanza.compare(0, end_stanza.size(), text_buf,
									   	end_stanza.size()))
                    return;
            }
        } else {
            // variable trace length
            char trc_hdr_buf[CommonSEGY::TR_HEADER_SIZE];
            for (auto i = common.binary_header.num_of_tr_in_file; i; --i) {
                common.file.read(trc_hdr_buf, CommonSEGY::TR_HEADER_SIZE);
                // get number of samples from main header
                char const* ptr = trc_hdr_buf + 114;
                uint32_t trc_samp_num = read_i16(&ptr);
                if (common.binary_header.max_num_add_tr_headers) {
                    // if there are additional header(s)
                    // get number of samples from first additional header
                    common.file.read(trc_hdr_buf, CommonSEGY::TR_HEADER_SIZE);
                    ptr = trc_hdr_buf + 136;
                    trc_samp_num = read_u32(&ptr);
                    ptr = trc_hdr_buf + 156;
                    uint16_t add_tr_hdr_num = read_u16(&ptr);
                    add_tr_hdr_num = add_tr_hdr_num ? add_tr_hdr_num :
					   	common.binary_header.max_num_add_tr_headers;
                    // skip addional headers
                    common.file.seekg((add_tr_hdr_num - 1) *
									  CommonSEGY::TR_HEADER_SIZE,
									  ios_base::cur);
                }
                // skip trace samples
                common.file.seekg(trc_samp_num * common.bytes_per_sample,
								  ios_base::cur);
            }
            end_of_data = common.file.tellg();
            string end_stanza = "((SEG: EndText))";
            while (1) {
                common.file.read(text_buf, CommonSEGY::TEXT_HEADER_SIZE);
                common.trailer_stanzas
					.emplace_back(text_buf, CommonSEGY::TEXT_HEADER_SIZE);
                if (!end_stanza.compare(0, end_stanza.size(), text_buf,
									   	end_stanza.size()))
                    return;
            }
        }
    } else {
        // go to first trailer stanza
        common.file.seekg(common.binary_header.num_of_trailer_stanza *
						  CommonSEGY::TEXT_HEADER_SIZE, ios_base::end);
        end_of_data = common.file.tellg();
        for (int32_t i = common.binary_header.num_of_trailer_stanza; i; --i) {
            common.file.read(text_buf, CommonSEGY::TEXT_HEADER_SIZE);
            common.trailer_stanzas.emplace_back(text_buf,
											   	CommonSEGY::TEXT_HEADER_SIZE);
        }
    }
}

void ISEGY::Impl::assign_bytes_per_sample()
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

unordered_map<string, Trace::Header::Value> ISEGY::Impl::read_trc_header()
{
    unordered_map<string, Trace::Header::Value> hdr;
	for (decltype(common.binary_header.max_num_add_tr_headers) i = 0;
		 i < common.binary_header.max_num_add_tr_headers + 1; ++i) {
		if (static_cast<decltype(common.tr_hdr_map.size())>(i) <
		   	common.tr_hdr_map.size()) {
			fill_buf_from_file(common.hdr_buf, CommonSEGY::TR_HEADER_SIZE);
			for (auto& p : common.tr_hdr_map[i].second) {
				char const* pos = common.hdr_buf + p.first;
				switch (p.second.second) {
					case Trace::Header::ValueType::int8_t:
						hdr[p.second.first] = read_i8(&pos);
						break;
					case Trace::Header::ValueType::uint8_t:
						hdr[p.second.first] = read_u8(&pos);
						break;
					case Trace::Header::ValueType::int16_t:
						hdr[p.second.first] = read_i16(&pos);
						break;
					case Trace::Header::ValueType::uint16_t:
						hdr[p.second.first] = read_u16(&pos);
						break;
					case Trace::Header::ValueType::int24_t:
						hdr[p.second.first] = read_i24(&pos);
						break;
					case Trace::Header::ValueType::uint24_t:
						hdr[p.second.first] = read_u24(&pos);
						break;
					case Trace::Header::ValueType::int32_t:
						hdr[p.second.first] = read_i32(&pos);
						break;
					case Trace::Header::ValueType::uint32_t:
						hdr[p.second.first] = read_u32(&pos);
						break;
					case Trace::Header::ValueType::int64_t:
						hdr[p.second.first] = read_i64(&pos);
						break;
					case Trace::Header::ValueType::uint64_t:
						hdr[p.second.first] = static_cast<int64_t>(read_u64(&pos));
						break;
					case Trace::Header::ValueType::ibm:
						hdr[p.second.first] = dbl_from_ibm_float(&pos);
						break;
					case Trace::Header::ValueType::ieee_single:
						hdr[p.second.first] = dbl_from_IEEE_float(&pos);
						break;
					case Trace::Header::ValueType::ieee_double:
						hdr[p.second.first] = dbl_from_IEEE_double(&pos);
						break;
				}
			}
		} else {
			file_skip_bytes(CommonSEGY::TR_HEADER_SIZE);
		}
	}
    return hdr;
}

Trace::Header ISEGY::read_header()
{
    unordered_map<string, Trace::Header::Value> hdr = pimpl->read_trc_header();
    if (pimpl->common.binary_header.fixed_tr_length ||
	   	pimpl->common.binary_header.SEGY_rev_major_ver == 0) {
        pimpl->file_skip_bytes(pimpl->common.samp_per_tr *
							   pimpl->common.bytes_per_sample);
    } else {
        Trace::Header::Value v = hdr["SAMP_NUM"];
        uint64_t samp_num = get<int64_t>(v);
        pimpl->file_skip_bytes(samp_num * pimpl->common.bytes_per_sample);
    }
    return Trace::Header(hdr);
}

vector<double> ISEGY::Impl::read_trc_smpls_fix()
{
    fill_buf_from_file(common.samp_buf.data(), common.samp_buf.size());
    char const* buf = common.samp_buf.data();
    vector<double> result(common.samp_buf.size() / common.bytes_per_sample);
    for (decltype(result.size()) i = 0; i < result.size(); ++i)
        result[i] = read_sample(&buf);
    return result;
}

vector<double> ISEGY::Impl::read_trc_smpls_var(unordered_map<string,
											   Trace::Header::Value>& hdr)
{
    Trace::Header::Value v = hdr["SAMP_NUM"];
    size_t samp_num = get<int64_t>(v);
    if (common.samp_buf.size() != samp_num * common.bytes_per_sample)
        common.samp_buf.resize(samp_num * common.bytes_per_sample);
    return read_trc_smpls_fix();
}

Trace ISEGY::read_trace()
{
    unordered_map<string, Trace::Header::Value> hdr = pimpl->read_trc_header();
    vector<double> samples = pimpl->read_trc_smpls(hdr);
    return Trace(move(hdr), move(samples));
}

bool ISEGY::has_trace()
{
    if (pimpl->curr_pos == pimpl->end_of_data)
        return false;
    else
        return true;
}

vector<string> const& ISEGY::text_headers()
{
    return pimpl->common.text_headers;
}

vector<string> const& ISEGY::trailer_stanzas()
{
    return pimpl->common.trailer_stanzas;
}

CommonSEGY::BinaryHeader const& ISEGY::binary_header()
{
    return pimpl->common.binary_header;
}

ISEGY::ISEGY(string name, vector<pair<string, map<uint32_t, pair<string,
   	Trace::Header::ValueType>>>> hdr_map)
    : pimpl { make_unique<Impl>(move(name), move(hdr_map)) }
{
}

ISEGY::ISEGY(string name, CommonSEGY::BinaryHeader bh,
    vector<pair<string, map<uint32_t, pair<string,
   	Trace::Header::ValueType>>>> hdr_map)
	: pimpl { make_unique<Impl>(move(name), move(bh), move(hdr_map)) }
{
}

CommonSEGY::BinaryHeader ISEGY::read_binary_header(std::string file_name)
{
    ISEGY s(file_name);
    return s.binary_header();
}

CommonSEGY& ISEGY::common()
{
    return pimpl->common;
}

ISEGY::~ISEGY() = default;
} // namespace sedaman
