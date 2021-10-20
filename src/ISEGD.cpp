#include "ISEGD.hpp"
#include "Exception.hpp"
#include "Trace.hpp"
#include "util.hpp"
#include <cfloat>
#include <cmath>
#include <cstring>
#include <functional>
#include <type_traits>

using std::array;
using std::endian;
using std::fstream;
using std::function;
using std::get;
using std::ios_base;
using std::make_unique;
using std::map;
using std::move;
using std::nullopt;
using std::optional;
using std::pair;
using std::streamoff;
using std::streampos;
using std::streamsize;
using std::string;
using std::to_string;
using std::unique_ptr;
using std::unordered_map;
using std::vector;

namespace sedaman {
class ISEGD::Impl {
public:
    Impl(CommonSEGD com);
    unordered_map<string, Trace::Header::Value> read_trace_header();
    void read_trace_header_ext
        (unordered_map<string, Trace::Header::Value>& hdr);
    vector<double> read_trace_samples(unordered_map<string,
									  Trace::Header::Value>& hdr);
    void read_headers_before_traces();
    void read_trailer();
    CommonSEGD common;
    streampos curr_pos;
    streampos end_of_data;
    uint64_t chans_in_record;
    uint64_t chans_read;

private:
    void read_general_headers();
    CommonSEGD::ChannelSetHeader read_ch_set_hdr();
    void fill_buf_from_file(char* buf, streamsize n);
    void file_skip_bytes(streamoff off);
    void check_trc_hdr_ext();
    function<double(char const**)> read_sample;
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
    function<double(char const** buf)> dbl_from_ibm_float;
    function<double(char const** buf)> dbl_from_IEEE_float;
    function<double(char const** buf)> dbl_from_IEEE_double;
    void read_gen_hdr1(char const* buf);
    void read_gen_hdr2_rev2(char const* buf);
    void read_rev2_add_gen_hdr_blks(char const* buf);
    void read_gen_hdr2_and_3(char const* buf);
    void read_rev3_add_gen_hdr_blks(char const* buf);
    unique_ptr<CommonSEGD::AdditionalGeneralHeader>
 		read_vessel_crew_id(char const* buf);
    unique_ptr<CommonSEGD::AdditionalGeneralHeader>
	 	read_survey_area_name(char const* buf);
    unique_ptr<CommonSEGD::AdditionalGeneralHeader>
	   	read_client_name(const char* buf);
    unique_ptr<CommonSEGD::AdditionalGeneralHeader>
	   	read_job_id(char const* buf);
    unique_ptr<CommonSEGD::AdditionalGeneralHeader>
	   	read_line_id(char const* buf);
    unique_ptr<CommonSEGD::AdditionalGeneralHeader>
	   	read_vibrator_hdr(char const* buf);
    unique_ptr<CommonSEGD::AdditionalGeneralHeader>
	   	read_explosive_hdr(char const* buf);
    unique_ptr<CommonSEGD::AdditionalGeneralHeader>
	   	read_airgun_hdr(char const* buf);
    unique_ptr<CommonSEGD::AdditionalGeneralHeader>
	   	read_watergun_hdr(char const* buf);
    unique_ptr<CommonSEGD::AdditionalGeneralHeader>
	   	read_electromagnetic_hdr(char const* buf);
    unique_ptr<CommonSEGD::AdditionalGeneralHeader>
	   	read_other_source_hdr(char const* buf);
    unique_ptr<CommonSEGD::AdditionalGeneralHeader>
	   	read_additional_source_hdr(char const* buf);
    unique_ptr<CommonSEGD::AdditionalGeneralHeader>
	   	read_source_aux_hdr(char const* buf);
    unique_ptr<CommonSEGD::AdditionalGeneralHeader>
		read_crs_hdr(char const* buf);
    unique_ptr<CommonSEGD::AdditionalGeneralHeader>
	   	read_position1_hdr(char const* buf);
    unique_ptr<CommonSEGD::AdditionalGeneralHeader>
	   	read_position2_hdr(char const* buf);
    unique_ptr<CommonSEGD::AdditionalGeneralHeader>
	   	read_position3_hdr(char const* buf);
    unique_ptr<CommonSEGD::AdditionalGeneralHeader>
	   	read_relative_position_hdr(char const* buf);
    unique_ptr<CommonSEGD::AdditionalGeneralHeader>
	   	read_sensor_info_hdr(char const* buf);
    unique_ptr<CommonSEGD::AdditionalGeneralHeader>
	   	read_sensor_calibration_hdr(char const* buf);
    unique_ptr<CommonSEGD::AdditionalGeneralHeader>
	   	read_time_drift_hdr(char const* buf);
    unique_ptr<CommonSEGD::AdditionalGeneralHeader>
	   	read_electromagnetic_src_recv_hdr(char const* buf);
    unique_ptr<CommonSEGD::AdditionalGeneralHeader>
	   	read_orientation_hdr(char const* buf);
    unique_ptr<CommonSEGD::AdditionalGeneralHeader>
	   	read_measurement_hdr(char const* buf);
    void assign_raw_readers();
    void assign_sample_reading();
};

bool ISEGD::has_record()
{
    return pimpl->curr_pos != pimpl->end_of_data;
}

bool ISEGD::has_trace()
{
    return pimpl->chans_in_record != pimpl->chans_read;
}

Trace ISEGD::read_trace()
{
    unordered_map<string, Trace::Header::Value> hdr =
	   	pimpl->read_trace_header();
    pimpl->read_trace_header_ext(hdr);
    vector<double> samples = pimpl->read_trace_samples(hdr);
    ++pimpl->chans_read;
    if (pimpl->chans_in_record == pimpl->chans_read) {
        pimpl->chans_in_record = pimpl->chans_read = 0;
        pimpl->common.channel_sets.clear();
        if (has_record())
            pimpl->read_headers_before_traces();
    }
    return Trace(move(hdr), move(samples));
}

ISEGD::Impl::Impl(CommonSEGD com)
    : common { move(com) }
{
    common.file.seekg(0, ios_base::end);
    end_of_data = common.file.tellg();
    common.file.seekg(0, ios_base::beg);
    curr_pos = common.file.tellg();
    assign_raw_readers();
    read_headers_before_traces();
}

void ISEGD::Impl::read_headers_before_traces()
{
    read_general_headers();
    assign_sample_reading();
    // read header for each scan type
    uint16_t ch_sets_per_scan_type_num =
	   	common.general_header.channel_sets_per_scan_type ==	1665 ?
	   	common.general_header2.ext_ch_sets_per_scan_type :
	   	common.general_header.channel_sets_per_scan_type;
    if (!common.general_header.add_gen_hdr_blocks ||
	   	common.general_header2.segd_rev_major < 3)
        common.ch_set_hdr_buf.resize(CommonSEGD::CH_SET_HDR_SIZE);
    else
        common.ch_set_hdr_buf.resize(CommonSEGD::CH_SET_HDR_R3_SIZE);
    for (int i = 0; i < common.general_header.scan_types_per_record; ++i) {
        common.channel_sets.push_back({});
        for (int j = ch_sets_per_scan_type_num; j; --j) {
            CommonSEGD::ChannelSetHeader hdr = read_ch_set_hdr();
            chans_in_record += hdr.number_of_channels;
            common.channel_sets[i].emplace_back(move(hdr));
        }
        uint16_t skew_blks = common.general_header.skew_blocks == 165 ?
		   	common.general_header2.extended_skew_blocks :
		   	common.general_header.skew_blocks;
        file_skip_bytes(skew_blks * CommonSEGD::SKEW_BLOCK_SIZE);
    }
    uint32_t extended_blocks = common.general_header.extended_hdr_blocks == 165
	   	? common.general_header2.extended_hdr_blocks :
	   	common.general_header.extended_hdr_blocks;
    uint32_t external_blocks = common.general_header.external_hdr_blocks == 165
	   	? common.general_header2.external_hdr_blocks :
	   	common.general_header.external_hdr_blocks;
    common.extended_headers.resize(extended_blocks);
    for (int i = 0; i < CommonSEGD::EXTENDED_HEADER_SIZE; ++i) {
        common.extended_headers[i].resize(CommonSEGD::EXTENDED_HEADER_SIZE);
        fill_buf_from_file(common.extended_headers[i].data(),
            CommonSEGD::EXTENDED_HEADER_SIZE);
    }
    common.external_headers.resize(external_blocks);
    for (int i = 0; i < CommonSEGD::EXTERNAL_HEADER_SIZE; ++i) {
        common.external_headers[i].resize(CommonSEGD::EXTERNAL_HEADER_SIZE);
        fill_buf_from_file(common.external_headers[i].data(),
            CommonSEGD::EXTERNAL_HEADER_SIZE);
    }
}

void ISEGD::Impl::assign_raw_readers()
{
    read_u8 = [](char const** buf) { return read<uint8_t>(buf); };
    read_i8 = [](char const** buf) { return read<int8_t>(buf); };
    if (endian::native == endian::big) {
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
    } else {
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

void ISEGD::Impl::read_general_headers()
{
    fill_buf_from_file(common.gen_hdr_buf, CommonSEGD::GEN_HDR_SIZE);
    char const* buf = common.gen_hdr_buf;
    read_gen_hdr1(buf);
    if (common.general_header.add_gen_hdr_blocks) {
        fill_buf_from_file(common.gen_hdr_buf, CommonSEGD::GEN_HDR_SIZE);
        char const* buf = common.gen_hdr_buf;
        CommonSEGD::GeneralHeader2& gh2 = common.general_header2;
        buf += 10;
        gh2.segd_rev_major = read_u8(&buf);
        gh2.segd_rev_minor = read_u8(&buf);
        buf = common.gen_hdr_buf;
        if (gh2.segd_rev_major < 3) {
            read_gen_hdr2_rev2(buf);
            for (int i = common.general_header.add_gen_hdr_blocks - 1;
				 i; --i) {
                fill_buf_from_file(common.gen_hdr_buf,
								   CommonSEGD::GEN_HDR_SIZE);
                read_rev2_add_gen_hdr_blks(common.gen_hdr_buf);
            }
        } else {
            read_gen_hdr2_and_3(buf);
            uint16_t add_blks_num = common.general_header.add_gen_hdr_blocks ==
			   	0xf ? gh2.ext_num_add_blks_in_gen_hdr :
			   	common.general_header.add_gen_hdr_blocks;
            for (uint16_t i = add_blks_num - 2; i; --i) {
                fill_buf_from_file(common.gen_hdr_buf,
								   CommonSEGD::GEN_HDR_SIZE);
                read_rev3_add_gen_hdr_blks(common.gen_hdr_buf);
            }
        }
    }
}

void ISEGD::Impl::read_gen_hdr1(char const* buf)
{
    CommonSEGD::GeneralHeader& gh = common.general_header;
    gh.file_number = from_bcd<int>(&buf, false, 4);
    gh.format_code = from_bcd<int>(&buf, false, 4);
    gh.gen_const = from_bcd<long long>(&buf, false, 12);
    gh.year = from_bcd<int>(&buf, false, 2);
    gh.add_gen_hdr_blocks = from_bcd<int>(&buf, false, 1);
    gh.day = from_bcd<int>(&buf, true, 3);
    gh.hour = from_bcd<int>(&buf, false, 2);
    gh.minute = from_bcd<int>(&buf, false, 2);
    gh.second = from_bcd<int>(&buf, false, 2);
    gh.manufac_code = from_bcd<int>(&buf, false, 2);
    gh.manufac_num = from_bcd<int>(&buf, false, 4);
    gh.bytes_per_scan = from_bcd<long>(&buf, false, 6);
    gh.base_scan_int = read_u8(&buf) / pow(2, 4);
    gh.polarity = from_bcd<int>(&buf, false, 1);
    int exp = static_cast<unsigned>(read_u8(&buf)) & 0x0f;
    gh.scans_per_block = static_cast<unsigned>(read_u8(&buf)) * pow(2, exp);
    gh.record_type = from_bcd<int>(&buf, false, 1);
    gh.record_length = from_bcd<int>(&buf, true, 3);
    gh.scan_types_per_record = from_bcd<int>(&buf, false, 2);
    gh.channel_sets_per_scan_type = from_bcd<int>(&buf, false, 2);
    gh.skew_blocks = from_bcd<int>(&buf, false, 2);
    gh.extended_hdr_blocks = from_bcd<int>(&buf, false, 2);
    gh.external_hdr_blocks = from_bcd<int>(&buf, false, 2);
}

void ISEGD::Impl::read_gen_hdr2_rev2(char const* buf)
{
    CommonSEGD::GeneralHeader2& gh2 = common.general_header2;
    gh2.expanded_file_num = read_u24(&buf);
    gh2.ext_ch_sets_per_scan_type = read_u16(&buf);
    gh2.extended_hdr_blocks = read_u16(&buf);
    gh2.external_hdr_blocks = read_u16(&buf);
    buf += 3;
    gh2.gen_trailer_num_of_blocks = read_u16(&buf);
    gh2.ext_record_len = read_u24(&buf);
    gh2.gen_hdr_block_num = *++buf;
    buf += 2;
    gh2.sequence_number = read_u16(&buf);
}

void ISEGD::Impl::read_rev2_add_gen_hdr_blks(char const* buf)
{
    CommonSEGD::GeneralHeaderN ghN;
    ghN.expanded_file_number = read_u24(&buf);
    ghN.sou_line_num = read_i24(&buf);
    ghN.sou_line_num += read_u16(&buf) / pow(2, 16);
    ghN.sou_point_num = read_i24(&buf);
    ghN.sou_point_num += read_u16(&buf) / pow(2, 16);
    ghN.sou_point_index = read_u8(&buf);
    ghN.phase_control = read_u8(&buf);
    ghN.type_vibrator = read_u8(&buf);
    ghN.phase_angle = read_i16(&buf);
    ghN.gen_hdr_block_num = read_u8(&buf);
    ghN.sou_set_num = read_u8(&buf);
    common.add_gen_hdr_blks_map[static_cast<CommonSEGD::
		AdditionalGeneralHeader::ADD_GEN_HDR_BLKS>(ghN.gen_hdr_block_num)] =
	   	make_unique<CommonSEGD::GeneralHeaderN>(ghN);
}

void ISEGD::Impl::read_gen_hdr2_and_3(char const* buf)
{
    CommonSEGD::GeneralHeader2& gh2 = common.general_header2;
    gh2.expanded_file_num = read_u24(&buf);
    gh2.ext_ch_sets_per_scan_type = read_u16(&buf);
    gh2.extended_hdr_blocks = read_u24(&buf);
    gh2.extended_skew_blocks = read_u16(&buf);
    buf += 2;
    gh2.gen_trailer_num_of_blocks = read_u32(&buf);
    gh2.record_set_number = read_u16(&buf);
    gh2.ext_num_add_blks_in_gen_hdr = read_u16(&buf);
    gh2.dominant_sampling_int = read_u16(&buf);
    gh2.external_hdr_blocks = read_u24(&buf);
    gh2.gen_hdr_block_num = *++buf;
    fill_buf_from_file(common.gen_hdr_buf, CommonSEGD::GEN_HDR_SIZE);
    buf = common.gen_hdr_buf;
    CommonSEGD::GeneralHeader3& gh3 = common.general_header3;
    gh3.time_zero = read_u64(&buf);
    gh3.record_size = read_u64(&buf);
    gh3.data_size = read_u64(&buf);
    gh3.header_size = read_u32(&buf);
    gh3.extd_rec_mode = read_u8(&buf);
    gh3.rel_time_mode = read_u8(&buf);
    gh3.gen_hdr_block_num = *++buf;
}

void ISEGD::Impl::read_rev3_add_gen_hdr_blks(char const* buf)
{
    int hdr_blk_type = *(buf + 31);
    switch (hdr_blk_type) {
    case CommonSEGD::AdditionalGeneralHeader::VESSEL_CREW_ID:
        common.add_gen_hdr_blks_map[CommonSEGD::AdditionalGeneralHeader::
			VESSEL_CREW_ID] = read_vessel_crew_id(buf);
        break;
    case CommonSEGD::AdditionalGeneralHeader::SURVEY_AREA_NAME:
        common.add_gen_hdr_blks_map[CommonSEGD::AdditionalGeneralHeader::
			SURVEY_AREA_NAME] = read_survey_area_name(buf);
        break;
    case CommonSEGD::AdditionalGeneralHeader::CLIENT_NAME:
        common.add_gen_hdr_blks_map[CommonSEGD::AdditionalGeneralHeader::
			CLIENT_NAME] = read_client_name(buf);
        break;
    case CommonSEGD::AdditionalGeneralHeader::JOB_ID:
        common.add_gen_hdr_blks_map[CommonSEGD::AdditionalGeneralHeader::
			JOB_ID] = read_job_id(buf);
        break;
    case CommonSEGD::AdditionalGeneralHeader::LINE_ID:
        common.add_gen_hdr_blks_map[CommonSEGD::AdditionalGeneralHeader::
			LINE_ID] = read_line_id(buf);
        break;
    case CommonSEGD::AdditionalGeneralHeader::VIBRATOR_SOURCE_INFO:
        common.add_gen_hdr_blks_map[CommonSEGD::AdditionalGeneralHeader::
			VIBRATOR_SOURCE_INFO] = read_vibrator_hdr(buf);
        break;
    case CommonSEGD::AdditionalGeneralHeader::EXPLOSIVE_SOURCE_INFO:
        common.add_gen_hdr_blks_map[CommonSEGD::AdditionalGeneralHeader::
			EXPLOSIVE_SOURCE_INFO] = read_explosive_hdr(buf);
        break;
    case CommonSEGD::AdditionalGeneralHeader::AIRGUN_SOURCE_INFO:
        common.add_gen_hdr_blks_map[CommonSEGD::AdditionalGeneralHeader::
			AIRGUN_SOURCE_INFO] = read_airgun_hdr(buf);
        break;
    case CommonSEGD::AdditionalGeneralHeader::WATERGUN_SOURCE_INFO:
        common.add_gen_hdr_blks_map[CommonSEGD::AdditionalGeneralHeader::
			WATERGUN_SOURCE_INFO] = read_watergun_hdr(buf);
        break;
    case CommonSEGD::AdditionalGeneralHeader::ELECTROMAGNETIC_SOURCE:
        common.add_gen_hdr_blks_map[CommonSEGD::AdditionalGeneralHeader::
			ELECTROMAGNETIC_SOURCE] = read_electromagnetic_hdr(buf);
        break;
    case CommonSEGD::AdditionalGeneralHeader::OTHER_SOURCE_TYPE_INFO:
        common.add_gen_hdr_blks_map[CommonSEGD::AdditionalGeneralHeader::
			OTHER_SOURCE_TYPE_INFO] = read_other_source_hdr(buf);
        break;
    case CommonSEGD::AdditionalGeneralHeader::ADD_SOURCE_INFO:
        common.add_gen_hdr_blks_map[CommonSEGD::AdditionalGeneralHeader::
			ADD_SOURCE_INFO] = read_additional_source_hdr(buf);
        break;
    case CommonSEGD::AdditionalGeneralHeader::SOU_AUX_CHAN_REF:
        common.add_gen_hdr_blks_map[CommonSEGD::AdditionalGeneralHeader::
			SOU_AUX_CHAN_REF] = read_source_aux_hdr(buf);
        break;
    case CommonSEGD::AdditionalGeneralHeader::SENSOR_INFO_HDR_EXT_BLK:
        common.add_gen_hdr_blks_map[CommonSEGD::AdditionalGeneralHeader::
			SENSOR_INFO_HDR_EXT_BLK] = read_sensor_info_hdr(buf);
        break;
    case CommonSEGD::AdditionalGeneralHeader::SENSOR_CALIBRATION_BLK:
        common.add_gen_hdr_blks_map[CommonSEGD::AdditionalGeneralHeader::
			SENSOR_CALIBRATION_BLK] = read_sensor_calibration_hdr(buf);
        break;
    case CommonSEGD::AdditionalGeneralHeader::TIME_DRIFT_BLK:
        common.add_gen_hdr_blks_map[CommonSEGD::AdditionalGeneralHeader::
			TIME_DRIFT_BLK] = read_time_drift_hdr(buf);
        break;
    case CommonSEGD::AdditionalGeneralHeader::ELECTROMAG_SRC_REC_DESC_BLK:
        common.add_gen_hdr_blks_map[CommonSEGD::AdditionalGeneralHeader::
			ELECTROMAG_SRC_REC_DESC_BLK] =
		   	read_electromagnetic_src_recv_hdr(buf);
        break;
    case CommonSEGD::AdditionalGeneralHeader::POSITION_BLK1:
        common.add_gen_hdr_blks_map[CommonSEGD::AdditionalGeneralHeader::
			POSITION_BLK1] = read_position1_hdr(buf);
        break;
    case CommonSEGD::AdditionalGeneralHeader::POSITION_BLK2:
        common.add_gen_hdr_blks_map[CommonSEGD::AdditionalGeneralHeader::
			POSITION_BLK2] = read_position2_hdr(buf);
        break;
    case CommonSEGD::AdditionalGeneralHeader::POSITION_BLK3:
        common.add_gen_hdr_blks_map[CommonSEGD::AdditionalGeneralHeader::
			POSITION_BLK3] = read_position3_hdr(buf);
        break;
    case CommonSEGD::AdditionalGeneralHeader::COORD_REF_SYSTEM:
        common.add_gen_hdr_blks_map[CommonSEGD::AdditionalGeneralHeader::
			COORD_REF_SYSTEM] = read_crs_hdr(buf);
        break;
    case CommonSEGD::AdditionalGeneralHeader::RELATIVE_POS_BLK:
        common.add_gen_hdr_blks_map[CommonSEGD::AdditionalGeneralHeader::
			RELATIVE_POS_BLK] = read_relative_position_hdr(buf);
        break;
    case CommonSEGD::AdditionalGeneralHeader::ORIENT_HDR_BLK:
        common.add_gen_hdr_blks_map[CommonSEGD::AdditionalGeneralHeader::
			ORIENT_HDR_BLK] = read_orientation_hdr(buf);
        break;
    case CommonSEGD::AdditionalGeneralHeader::MEASUREMENT_BLK:
        common.add_gen_hdr_blks_map[CommonSEGD::AdditionalGeneralHeader::
			MEASUREMENT_BLK] = read_measurement_hdr(buf);
        break;
    }
}

unique_ptr<CommonSEGD::AdditionalGeneralHeader>
ISEGD::Impl::read_vessel_crew_id(char const* buf)
{
    CommonSEGD::GeneralHeaderVes ghv;
    memcpy(ghv.abbr_vessel_crew_name, buf, sizeof(ghv.abbr_vessel_crew_name));
    buf += sizeof(ghv.abbr_vessel_crew_name);
    memcpy(ghv.vessel_crew_name, buf, sizeof(ghv.vessel_crew_name));
    buf += sizeof(ghv.vessel_crew_name);
    ghv.gen_hdr_block_type = *buf;
    return make_unique<CommonSEGD::GeneralHeaderVes>(move(ghv));
}

unique_ptr<CommonSEGD::AdditionalGeneralHeader>
ISEGD::Impl::read_survey_area_name(const char* buf)
{
    CommonSEGD::GeneralHeaderSur ghs;
    memcpy(ghs.survey_area_name, buf, sizeof(ghs.survey_area_name));
    buf += sizeof(ghs.survey_area_name);
    ghs.gen_hdr_block_type = *buf;
    return make_unique<CommonSEGD::GeneralHeaderSur>(move(ghs));
}

unique_ptr<CommonSEGD::AdditionalGeneralHeader>
ISEGD::Impl::read_client_name(const char* buf)
{
    CommonSEGD::GeneralHeaderCli ghc;
    memcpy(ghc.client_name, buf, sizeof(ghc.client_name));
    buf += sizeof(ghc.client_name);
    ghc.gen_hdr_block_type = *buf;
    return make_unique<CommonSEGD::GeneralHeaderCli>(move(ghc));
}

unique_ptr<CommonSEGD::AdditionalGeneralHeader>
ISEGD::Impl::read_job_id(char const* buf)
{
    CommonSEGD::GeneralHeaderJob ghj;
    memcpy(ghj.abbr_job_id, buf, sizeof(ghj.abbr_job_id));
    buf += sizeof(ghj.abbr_job_id);
    memcpy(ghj.job_id, buf, sizeof(ghj.job_id));
    buf += sizeof(ghj.job_id);
    ghj.gen_hdr_block_type = *buf;
    return make_unique<CommonSEGD::GeneralHeaderJob>(move(ghj));
}

unique_ptr<CommonSEGD::AdditionalGeneralHeader>
ISEGD::Impl::read_line_id(char const* buf)
{
    CommonSEGD::GeneralHeaderLin ghl;
    memcpy(ghl.line_abbr, buf, sizeof(ghl.line_abbr));
    buf += sizeof(ghl.line_abbr);
    memcpy(ghl.line_id, buf, sizeof(ghl.line_id));
    buf += sizeof(ghl.line_id);
    ghl.gen_hdr_block_type = *buf;
    return make_unique<CommonSEGD::GeneralHeaderLin>(move(ghl));
}

unique_ptr<CommonSEGD::AdditionalGeneralHeader>
ISEGD::Impl::read_vibrator_hdr(char const* buf)
{
    CommonSEGD::GeneralHeaderVib ghV;
    ghV.expanded_file_number = read_u24(&buf);
    ghV.sou_line_num = read_u24(&buf);
    ghV.sou_line_num += read_u16(&buf) / pow(2, 16);
    ghV.sou_point_num = read_u24(&buf);
    ghV.sou_point_num += read_u16(&buf) / pow(2, 16);
    ghV.sou_point_index = read_u8(&buf);
    ghV.phase_control = read_u8(&buf);
    ghV.type_vibrator = read_u8(&buf);
    ghV.phase_angle = read_u16(&buf);
    ghV.source_id = read_u8(&buf);
    ghV.source_set_num = read_u8(&buf);
    ghV.reshoot_idx = read_u8(&buf);
    ghV.group_idx = read_u8(&buf);
    ghV.depth_idx = read_u8(&buf);
    ghV.offset_crossline = read_u8(&buf);
    ghV.offset_inline = read_u8(&buf);
    ghV.size = read_u8(&buf);
    ghV.offset_depth = read_u8(&buf);
    ghV.gen_hdr_block_type = *buf;
    return make_unique<CommonSEGD::GeneralHeaderVib>(move(ghV));
}

unique_ptr<CommonSEGD::AdditionalGeneralHeader>
ISEGD::Impl::read_explosive_hdr(char const* buf)
{
    CommonSEGD::GeneralHeaderExp ghE;
    ghE.expanded_file_number = read_u24(&buf);
    ghE.sou_line_num = read_u24(&buf);
    ghE.sou_line_num += read_u16(&buf) / pow(2, 16);
    ghE.sou_point_num = read_u24(&buf);
    ghE.sou_point_num += read_u16(&buf) / pow(2, 16);
    ghE.sou_point_index = read_u8(&buf);
    ghE.depth = read_u16(&buf);
    ghE.charge_length = read_u8(&buf);
    ghE.soil_type = read_u8(&buf);
    ghE.source_id = read_u8(&buf);
    ghE.source_set_num = read_u8(&buf);
    ghE.reshoot_idx = read_u8(&buf);
    ghE.group_idx = read_u8(&buf);
    ghE.depth_idx = read_u8(&buf);
    ghE.offset_crossline = read_u8(&buf);
    ghE.offset_inline = read_u8(&buf);
    ghE.size = read_u8(&buf);
    ghE.offset_depth = read_u8(&buf);
    ghE.gen_hdr_block_type = *buf;
    return make_unique<CommonSEGD::GeneralHeaderExp>(move(ghE));
}

unique_ptr<CommonSEGD::AdditionalGeneralHeader>
ISEGD::Impl::read_airgun_hdr(char const* buf)
{
    CommonSEGD::GeneralHeaderAir ghA;
    ghA.expanded_file_number = read_u24(&buf);
    ghA.sou_line_num = read_u24(&buf);
    ghA.sou_line_num += read_u16(&buf) / pow(2, 16);
    ghA.sou_point_num = read_u24(&buf);
    ghA.sou_point_num += read_u16(&buf) / pow(2, 16);
    ghA.sou_point_index = read_u8(&buf);
    ghA.depth = read_u16(&buf);
    ghA.air_pressure = read_u16(&buf);
    ghA.source_id = read_u8(&buf);
    ghA.source_set_num = read_u8(&buf);
    ghA.reshoot_idx = read_u8(&buf);
    ghA.group_idx = read_u8(&buf);
    ghA.depth_idx = read_u8(&buf);
    ghA.offset_crossline = read_u8(&buf);
    ghA.offset_inline = read_u8(&buf);
    ghA.size = read_u8(&buf);
    ghA.offset_depth = read_u8(&buf);
    ghA.gen_hdr_block_type = *buf;
    return make_unique<CommonSEGD::GeneralHeaderAir>(move(ghA));
}

unique_ptr<CommonSEGD::AdditionalGeneralHeader>
ISEGD::Impl::read_watergun_hdr(char const* buf)
{
    CommonSEGD::GeneralHeaderWat ghA;
    ghA.expanded_file_number = read_u24(&buf);
    ghA.sou_line_num = read_u24(&buf);
    ghA.sou_line_num += read_u16(&buf) / pow(2, 16);
    ghA.sou_point_num = read_u24(&buf);
    ghA.sou_point_num += read_u16(&buf) / pow(2, 16);
    ghA.sou_point_index = read_u8(&buf);
    ghA.depth = read_u16(&buf);
    ghA.air_pressure = read_u16(&buf);
    ghA.source_id = read_u8(&buf);
    ghA.source_set_num = read_u8(&buf);
    ghA.reshoot_idx = read_u8(&buf);
    ghA.group_idx = read_u8(&buf);
    ghA.depth_idx = read_u8(&buf);
    ghA.offset_crossline = read_u8(&buf);
    ghA.offset_inline = read_u8(&buf);
    ghA.size = read_u8(&buf);
    ghA.offset_depth = read_u8(&buf);
    ghA.gen_hdr_block_type = *buf;
    return make_unique<CommonSEGD::GeneralHeaderWat>(move(ghA));
}

unique_ptr<CommonSEGD::AdditionalGeneralHeader>
ISEGD::Impl::read_electromagnetic_hdr(char const* buf)
{
    CommonSEGD::GeneralHeaderEle ghE;
    ghE.expanded_file_number = read_u24(&buf);
    ghE.sou_line_num = read_u24(&buf);
    ghE.sou_line_num += read_u16(&buf) / pow(2, 16);
    ghE.sou_point_num = read_u24(&buf);
    ghE.sou_point_num += read_u16(&buf) / pow(2, 16);
    ghE.sou_point_index = read_u8(&buf);
    ghE.source_type = read_u8(&buf);
    ghE.moment = read_u24(&buf);
    ghE.source_id = read_u8(&buf);
    ghE.source_set_num = read_u8(&buf);
    ghE.reshoot_idx = read_u8(&buf);
    ghE.group_idx = read_u8(&buf);
    ghE.depth_idx = read_u8(&buf);
    ghE.offset_crossline = read_u8(&buf);
    ghE.offset_inline = read_u8(&buf);
    ghE.size = read_u8(&buf);
    ghE.offset_depth = read_u8(&buf);
    ghE.gen_hdr_block_type = *buf;
    return make_unique<CommonSEGD::GeneralHeaderEle>(move(ghE));
}

unique_ptr<CommonSEGD::AdditionalGeneralHeader>
ISEGD::Impl::read_other_source_hdr(char const* buf)
{
    CommonSEGD::GeneralHeaderOth ghO;
    ghO.expanded_file_number = read_u24(&buf);
    ghO.sou_line_num = read_u24(&buf);
    ghO.sou_line_num += read_u16(&buf) / pow(2, 16);
    ghO.sou_point_num = read_u24(&buf);
    ghO.sou_point_num += read_u16(&buf) / pow(2, 16);
    ghO.sou_point_index = read_u8(&buf);
    buf += 4;
    ghO.source_id = read_u8(&buf);
    ghO.source_set_num = read_u8(&buf);
    ghO.reshoot_idx = read_u8(&buf);
    ghO.group_idx = read_u8(&buf);
    ghO.depth_idx = read_u8(&buf);
    ghO.offset_crossline = read_u8(&buf);
    ghO.offset_inline = read_u8(&buf);
    ghO.size = read_u8(&buf);
    ghO.offset_depth = read_u8(&buf);
    ghO.gen_hdr_block_type = *buf;
    return make_unique<CommonSEGD::GeneralHeaderOth>(move(ghO));
}

unique_ptr<CommonSEGD::AdditionalGeneralHeader>
ISEGD::Impl::read_additional_source_hdr(char const* buf)
{
    CommonSEGD::GeneralHeaderAdd ghA;
    ghA.time = read_u64(&buf);
    ghA.source_status = read_u8(&buf);
    ghA.source_id = read_u8(&buf);
    ghA.source_moving = read_u8(&buf);
    memcpy(ghA.error_description, buf, sizeof(ghA.error_description));
    buf += sizeof(ghA.error_description);
    ghA.gen_hdr_block_type = *buf;
    return make_unique<CommonSEGD::GeneralHeaderAdd>(move(ghA));
}

unique_ptr<CommonSEGD::AdditionalGeneralHeader>
ISEGD::Impl::read_source_aux_hdr(char const* buf)
{
    CommonSEGD::GeneralHeaderSaux ghS;
    ghS.source_id = read_u8(&buf);
    ghS.scan_type_num_1 = from_bcd<uint8_t>(&buf, false, 2);
    ghS.ch_set_num_1 = read_u16(&buf);
    ghS.trace_num_1 = read_u24(&buf);
    ghS.scan_type_num_2 = from_bcd<uint8_t>(&buf, false, 2);
    ghS.ch_set_num_2 = read_u16(&buf);
    ghS.trace_num_2 = read_u24(&buf);
    ghS.scan_type_num_3 = from_bcd<uint8_t>(&buf, false, 2);
    ghS.ch_set_num_3 = read_u16(&buf);
    ghS.trace_num_3 = read_u24(&buf);
    ghS.scan_type_num_4 = from_bcd<uint8_t>(&buf, false, 2);
    ghS.ch_set_num_4 = read_u16(&buf);
    ghS.trace_num_4 = read_u24(&buf);
    ghS.scan_type_num_5 = from_bcd<uint8_t>(&buf, false, 2);
    ghS.ch_set_num_5 = read_u16(&buf);
    ghS.trace_num_5 = read_u24(&buf);
    ghS.gen_hdr_block_type = *buf;
    return make_unique<CommonSEGD::GeneralHeaderSaux>(move(ghS));
}

unique_ptr<CommonSEGD::AdditionalGeneralHeader>
ISEGD::Impl::read_crs_hdr(char const* buf)
{
    CommonSEGD::GeneralHeaderCoord ghC;
    memcpy(ghC.crs, buf, sizeof(ghC.crs));
    buf += sizeof(ghC.crs);
    ghC.gen_hdr_block_type = *buf;
    return make_unique<CommonSEGD::GeneralHeaderCoord>(move(ghC));
}

unique_ptr<CommonSEGD::AdditionalGeneralHeader>
ISEGD::Impl::read_position1_hdr(char const* buf)
{
    CommonSEGD::GeneralHeaderPos1 ghP;
    ghP.time_of_position = read_u64(&buf);
    ghP.time_of_measurement = read_u64(&buf);
    ghP.vert_error = read_u32(&buf);
    ghP.hor_error_semi_major = read_u32(&buf);
    ghP.hor_error_semi_minor = read_u32(&buf);
    ghP.hor_error_orientation = read_u16(&buf);
    ghP.position_type = read_u8(&buf);
    ghP.gen_hdr_block_type = *buf;
    return make_unique<CommonSEGD::GeneralHeaderPos1>(move(ghP));
}

unique_ptr<CommonSEGD::AdditionalGeneralHeader>
ISEGD::Impl::read_position2_hdr(char const* buf)
{
    CommonSEGD::GeneralHeaderPos2 ghP;
    ghP.crs_a_coord1 = read_u64(&buf);
    ghP.crs_a_coord2 = read_u64(&buf);
    ghP.crs_a_coord3 = read_u64(&buf);
    ghP.crs_a_crsref = read_u16(&buf);
    ghP.pos1_valid = read_u8(&buf);
    ghP.pos1_quality = read_u8(&buf);
    buf += 3;
    ghP.gen_hdr_block_type = *buf;
    return make_unique<CommonSEGD::GeneralHeaderPos2>(move(ghP));
}

unique_ptr<CommonSEGD::AdditionalGeneralHeader>
ISEGD::Impl::read_position3_hdr(char const* buf)
{
    CommonSEGD::GeneralHeaderPos3 ghP;
    ghP.crs_b_coord1 = read_u64(&buf);
    ghP.crs_b_coord2 = read_u64(&buf);
    ghP.crs_b_coord3 = read_u64(&buf);
    ghP.crs_b_crsref = read_u16(&buf);
    ghP.pos2_valid = read_u8(&buf);
    ghP.pos2_quality = read_u8(&buf);
    buf += 3;
    ghP.gen_hdr_block_type = *buf;
    return make_unique<CommonSEGD::GeneralHeaderPos3>(move(ghP));
}

unique_ptr<CommonSEGD::AdditionalGeneralHeader>
ISEGD::Impl::read_relative_position_hdr(char const* buf)
{
    CommonSEGD::GeneralHeaderRel ghR;
    ghR.offset_east = read_u32(&buf);
    ghR.offset_north = read_u32(&buf);
    ghR.offset_vert = read_u32(&buf);
    memcpy(ghR.description, buf, sizeof(ghR.description));
    buf += sizeof(ghR.description);
    ghR.gen_hdr_block_type = *buf;
    return make_unique<CommonSEGD::GeneralHeaderRel>(move(ghR));
}

unique_ptr<CommonSEGD::AdditionalGeneralHeader>
ISEGD::Impl::read_sensor_info_hdr(char const* buf)
{
    CommonSEGD::GeneralHeaderSen ghS;
    ghS.instrument_test_time = read_u64(&buf);
    ghS.sensor_sensitivity = read_u32(&buf);
    ghS.instr_test_result = read_u8(&buf);
    memcpy(ghS.serial_number, buf, sizeof(ghS.serial_number));
    buf += sizeof(ghS.serial_number);
    ghS.gen_hdr_block_type = *buf;
    return make_unique<CommonSEGD::GeneralHeaderSen>(move(ghS));
}

unique_ptr<CommonSEGD::AdditionalGeneralHeader>
ISEGD::Impl::read_sensor_calibration_hdr(char const* buf)
{
    CommonSEGD::GeneralHeaderSCa ghS;
    ghS.freq1 = read_u32(&buf);
    ghS.amp1 = read_u32(&buf);
    ghS.phase1 = read_u32(&buf);
    ghS.freq2 = read_u32(&buf);
    ghS.amp2 = read_u32(&buf);
    ghS.phase2 = read_u32(&buf);
    ghS.calib_applied = read_u8(&buf);
    buf += 6;
    ghS.gen_hdr_block_type = *buf;
    return make_unique<CommonSEGD::GeneralHeaderSCa>(move(ghS));
}

unique_ptr<CommonSEGD::AdditionalGeneralHeader>
ISEGD::Impl::read_time_drift_hdr(char const* buf)
{
    CommonSEGD::GeneralHeaderTim ghT;
    ghT.time_of_depl = read_u64(&buf);
    ghT.time_of_retr = read_u64(&buf);
    ghT.timer_offset_depl = read_u32(&buf);
    ghT.time_offset_retr = read_u32(&buf);
    ghT.timedrift_corr = read_u8(&buf);
    ghT.corr_method = read_u8(&buf);
    buf += 5;
    ghT.gen_hdr_block_type = *buf;
    return make_unique<CommonSEGD::GeneralHeaderTim>(move(ghT));
}

unique_ptr<CommonSEGD::AdditionalGeneralHeader>
ISEGD::Impl::read_electromagnetic_src_recv_hdr(char const* buf)
{
    CommonSEGD::GeneralHeaderElSR ghE;
    ghE.equip_dim_x = read_u24(&buf);
    ghE.equip_dim_y = read_u24(&buf);
    ghE.equip_dim_z = read_u24(&buf);
    ghE.pos_term = read_u8(&buf);
    ghE.equip_offset_x = read_u24(&buf);
    ghE.equip_offset_y = read_u24(&buf);
    ghE.equip_offset_z = read_u24(&buf);
    buf += 12;
    ghE.gen_hdr_block_type = *buf;
    return make_unique<CommonSEGD::GeneralHeaderElSR>(move(ghE));
}

unique_ptr<CommonSEGD::AdditionalGeneralHeader>
ISEGD::Impl::read_orientation_hdr(char const* buf)
{
    CommonSEGD::GeneralHeaderOri ghO;
    ghO.rot_x = read_u32(&buf);
    ghO.rot_y = read_u32(&buf);
    ghO.rot_z = read_u32(&buf);
    ghO.ref_orientation = read_u32(&buf);
    ghO.time_stamp = read_u64(&buf);
    ghO.ori_type = read_u8(&buf);
    ghO.ref_orient_valid = read_u8(&buf);
    ghO.rot_applied = read_u8(&buf);
    ghO.rot_north_applied = read_u8(&buf);
    buf += 3;
    ghO.gen_hdr_block_type = *buf;
    return make_unique<CommonSEGD::GeneralHeaderOri>(move(ghO));
}

unique_ptr<CommonSEGD::AdditionalGeneralHeader>
ISEGD::Impl::read_measurement_hdr(char const* buf)
{
    CommonSEGD::GeneralHeaderMeas ghM;
    ghM.timestamp = read_u64(&buf);
    ghM.measurement_value = read_u32(&buf);
    ghM.maximum_value = read_u32(&buf);
    ghM.minimum_value = read_u32(&buf);
    ghM.quantity_class = read_u16(&buf);
    ghM.unit_of_measure = read_u16(&buf);
    ghM.measurement_description = read_u16(&buf);
    buf += 5;
    ghM.gen_hdr_block_type = *buf;
    return make_unique<CommonSEGD::GeneralHeaderMeas>(move(ghM));
}

void ISEGD::Impl::assign_sample_reading()
{
    switch (common.general_header.format_code) {
    case 8015:
        common.bits_per_sample = 20;
        read_sample = [this](char const** buf) {
            static double result[4];
            static int counter = 0;
            if (counter == 4)
                counter = 0;
            if (!counter) {
                int exp[4];
                for (int i = 0; i < 4; i += 2) {
                    exp[i] = **buf >> 4;
                    exp[i + 1] = **buf++ & 0xf;
                }
                for (int i = 0; i < 4; ++i)
                    result[i] = read_i16(buf) * pow(2, exp[i] - 15);
            }
            return result[counter++];
        };
        break;
    case 8022:
        common.bits_per_sample = 8;
        read_sample = [](char const** buf) {
            int sign = **buf & 0b10000000 ? -1 : 1;
            int exp = **buf & 0b01110000;
            return sign * (**buf++ & 0xf) * pow(2, exp - 4);
        };
        break;
    case 8024:
        common.bits_per_sample = 16;
        read_sample = [](char const** buf) {
            int sign = **buf & 0b10000000 ? -1 : 1;
            int exp = **buf & 0b01110000;
            uint16_t frac = **buf++ & 0xf;
            frac = (frac << 8) & **buf++;
            return sign * frac * pow(2, exp - 12);
        };
        break;
    case 8036:
        common.bits_per_sample = 24;
        read_sample = [this](char const** buf) { return read_i24(buf); };
        break;
    case 8038:
        common.bits_per_sample = 32;
        read_sample = [this](char const** buf) { return read_i32(buf); };
        break;
    case 8042:
        common.bits_per_sample = 8;
        read_sample = [](char const** buf) {
            int sign = **buf & 0b10000000 ? -1 : 1;
            int exp = **buf & 0b01100000;
            return sign * (**buf++ & 0x1f) / pow(2, 5) * pow(16, exp);
        };
        break;
    case 8044:
        common.bits_per_sample = 16;
        read_sample = [](char const** buf) {
            int sign = **buf & 0b10000000 ? -1 : 1;
            int exp = **buf & 0b01100000;
            uint16_t frac = **buf++ & 0x1f;
            frac = (frac << 8) & **buf++;
            return sign * frac / pow(2, 13) * pow(16, exp);
        };
        break;
    case 8048:
        common.bits_per_sample = 32;
        read_sample = [this](char const** buf) {
            int sign = **buf & 0b10000000 ? -1 : 1;
            int exp = **buf++ & 0b01111111;
            return sign * read_u24(buf) / pow(2, 24) * pow(16, exp);
        };
        break;
    case 8058:
        common.bits_per_sample = 32;
        read_sample = [this](char const** buf) {
            uint32_t tmp = read_u32(buf);
            float result;
            memcpy(&result, &tmp, sizeof(tmp));
            return result;
        };
        break;
    case 8080:
        common.bits_per_sample = 64;
        read_sample = [this](char const** buf) {
            uint64_t tmp = read_u64(buf);
            double result;
            memcpy(&result, &tmp, sizeof(tmp));
            return result;
        };
        break;
    case 9036:
        common.bits_per_sample = 24;
        if (endian::native == endian::big)
            read_sample = [this](char const** buf)
		   	{ return swap(read_i24(buf)); };
        else
            read_sample = [](char const** buf) {
                uint32_t result = read<uint16_t>(buf) << 8 |
				   	read<uint8_t>(buf);
                return result & 0x800000 ? result | 0xff000000 : result;
            };
        break;
    case 9038:
        common.bits_per_sample = 32;
        if (endian::native == endian::big)
            read_sample = [this](char const** buf)
		   	{ return swap(read_i32(buf)); };
        else
            read_sample = [](char const** buf) { return read<int32_t>(buf); };
        break;
    case 9058:
        common.bits_per_sample = 32;
        if (endian::native == endian::big)
            read_sample = [this](char const** buf) {
                uint32_t tmp = read_u32(buf);
                tmp = swap(tmp);
                float result;
                memcpy(&result, &tmp, sizeof(tmp));
                return result;
            };
        else
            read_sample = [](char const** buf) {
                uint32_t tmp = read<uint32_t>(buf);
                float result;
                memcpy(&result, &tmp, sizeof(tmp));
                return result;
            };
        break;
    case 9080:
        common.bits_per_sample = 64;
        if (endian::native == endian::big)
            read_sample = [this](char const** buf) {
                uint64_t tmp = read_u64(buf);
                tmp = swap(tmp);
                double result;
                memcpy(&result, &tmp, sizeof(tmp));
                return result;
            };
        else
            read_sample = [](char const** buf) {
                uint64_t tmp = read<uint64_t>(buf);
                double result;
                memcpy(&result, &tmp, sizeof(tmp));
                return result;
            };
        break;
    default:
        throw Exception(__FILE__, __LINE__, "Unsupported format");
    }
}

CommonSEGD::ChannelSetHeader ISEGD::Impl::read_ch_set_hdr()
{
    fill_buf_from_file(common.ch_set_hdr_buf.data(),
					   common.ch_set_hdr_buf.size());
    char const* buf = common.ch_set_hdr_buf.data();
    int scan_type_number, subscans_per_ch_set = 0;
    uint8_t channel_type, channel_gain, ext_hdr_flag, trc_hdr_ext, vert_stack,
		   	streamer_no, array_forming, filter_phase = 0, physical_unit = 0;
    uint16_t channel_set_number, ext_ch_set_num = 0;
    uint32_t channel_set_start_time, channel_set_end_time, number_of_channels,
        alias_filter_freq, alias_filter_slope, low_cut_filter_freq,
	   	low_cut_filter_slope, first_notch_filter, second_notch_filter,
	   	third_notch_filter, number_of_samples = 0, samp_int = 0,
	   	filter_delay = 0;
    double descale_multiplier;
    array<char, 27> description;
    if (!common.general_header.add_gen_hdr_blocks ||
	   	common.general_header2.segd_rev_major < 3) {
        scan_type_number = from_bcd<int>(&buf, false, 2);
        channel_set_number = from_bcd<int>(&buf, false, 2);
        channel_set_start_time = read_u16(&buf) * 2;
        channel_set_end_time = read_u16(&buf) * 2;
        uint16_t val = read_u16(&buf);
        descale_multiplier = ((val & 0x8000) ? -1.0 : 1.0) * (val & 0x7fff) /
		   	pow(2, 10);
        number_of_channels = from_bcd<int>(&buf, false, 4);
        channel_type = from_bcd<int>(&buf, false, 1);
        ++buf;
        subscans_per_ch_set = pow(2, from_bcd<int>(&buf, false, 1));
        channel_gain = from_bcd<int>(&buf, true, 1);
        alias_filter_freq = from_bcd<int>(&buf, false, 4);
        alias_filter_slope = from_bcd<int>(&buf, false, 4);
        low_cut_filter_freq = from_bcd<int>(&buf, false, 4);
        low_cut_filter_slope = from_bcd<int>(&buf, false, 4);
        first_notch_filter = from_bcd<int>(&buf, false, 4);
        second_notch_filter = from_bcd<int>(&buf, false, 4);
        third_notch_filter = from_bcd<int>(&buf, false, 4);
        ext_ch_set_num = read_u16(&buf);
        ext_hdr_flag = *buf >> 4;
        trc_hdr_ext = *buf & 0x0f;
        ++buf;
        vert_stack = read_u8(&buf);
        streamer_no = read_u8(&buf);
        array_forming = read_u8(&buf);
    } else {
        scan_type_number = from_bcd<int>(&buf, false, 2);
        channel_set_number = read_u16(&buf);
        channel_type = read_u8(&buf);
        channel_set_start_time = read_u32(&buf);
        channel_set_end_time = read_u32(&buf);
        number_of_samples = read_u32(&buf);
        descale_multiplier = read_u32(&buf);
        number_of_channels = read_u24(&buf);
        samp_int = read_u24(&buf);
        array_forming = read_u8(&buf);
        trc_hdr_ext = read_u8(&buf);
        ext_hdr_flag = *buf >> 4;
        channel_gain = *buf & 0x0f;
        ++buf;
        vert_stack = read_u8(&buf);
        streamer_no = read_u8(&buf);
        ++buf;
        alias_filter_freq = read_u32(&buf);
        low_cut_filter_freq = read_u32(&buf);
        alias_filter_slope = read_u32(&buf);
        low_cut_filter_slope = read_u32(&buf);
        first_notch_filter = read_u32(&buf);
        second_notch_filter = read_u32(&buf);
        third_notch_filter = read_u32(&buf);
        filter_phase = read_u8(&buf);
        physical_unit = read_u8(&buf);
        buf += 2;
        filter_delay = read_u32(&buf);
        memcpy(description.data(), buf, description.size());
    }
    return CommonSEGD::ChannelSetHeader(
        scan_type_number, channel_set_number, channel_type,
        channel_set_start_time, channel_set_end_time, descale_multiplier,
        number_of_channels, subscans_per_ch_set, channel_gain,
	   	alias_filter_freq, alias_filter_slope, low_cut_filter_freq,
	   	low_cut_filter_slope, first_notch_filter, second_notch_filter,
	   	third_notch_filter, ext_ch_set_num, ext_hdr_flag, trc_hdr_ext,
	   	vert_stack, streamer_no, array_forming, number_of_samples, samp_int,
        filter_phase, physical_unit, filter_delay, description);
}

void ISEGD::Impl::check_trc_hdr_ext()
{
    if (common.trace_header_extension.size()) {
        // for now all channel sets should have the same trace header extension
        // number
        for (auto& scan_type : common.channel_sets)
            for (auto& ch_set : scan_type)
                if (ch_set.trc_hdr_ext &&
                ch_set.trc_hdr_ext != common.trace_header_extension.size())
                    throw Exception(__FILE__, __LINE__,
                        "Number of trace header extension in channel set " +
                        to_string(ch_set.channel_set_number) +
                        " not equal to number of maps in provided vector");
        int first = 1, size = 0;
        uint32_t prev;
        for (auto& hdr_map : common.trace_header_extension) {
            for (auto& i : hdr_map) {
                if (first) {
                    prev = i.first;
                    first = 0;
                } else {
                    switch (i.second.second) {
                    case Trace::Header::ValueType::int8_t:
                        size = 1;
                        break;
                    case Trace::Header::ValueType::uint8_t:
                        size = 1;
                        break;
                    case Trace::Header::ValueType::int16_t:
                        size = 2;
                        break;
                    case Trace::Header::ValueType::uint16_t:
                        size = 2;
                        break;
                    case Trace::Header::ValueType::int24_t:
                        size = 3;
                        break;
                    case Trace::Header::ValueType::uint24_t:
                        size = 3;
                        break;
                    case Trace::Header::ValueType::int32_t:
                        size = 4;
                        break;
                    case Trace::Header::ValueType::uint32_t:
                        size = 4;
                        break;
                    case Trace::Header::ValueType::int64_t:
                        size = 8;
                        break;
                    case Trace::Header::ValueType::uint64_t:
                        size = 8;
                        break;
                    case Trace::Header::ValueType::ibm:
                        size = 4;
                        break;
                    case Trace::Header::ValueType::ieee_single:
                        size = 4;
                        break;
                    case Trace::Header::ValueType::ieee_double:
                        size = 8;
                        break;
                    default:
                        throw Exception(__FILE__, __LINE__,
                            "impossible, unexpected type in "
                            "TrHdrValueType");
                    }
                    if (i.first - prev < static_cast<uint32_t>(size))
                        throw Exception(__FILE__, __LINE__,
                            string("overlapping type/offset in "
                                   "trace headers extension map: ")
                                + to_string(i.first));
                    if (i.first + size > CommonSEGD::TRACE_HEADER_EXT_SIZE)
                        throw Exception(__FILE__, __LINE__,
                            string("trace header extension map will write over "
                            "buffer bound"));
                    prev = i.first;
                }
            }
        }
    }
}

unordered_map<string, Trace::Header::Value> ISEGD::Impl::read_trace_header()
{
    unordered_map<string, Trace::Header::Value> hdr;
    fill_buf_from_file(common.trc_hdr_buf, CommonSEGD::TRACE_HEADER_SIZE);
    char const* buf = common.trc_hdr_buf;
    hdr["FFID"] = from_bcd<uint32_t>(&buf, false, 4);
    hdr["SCAN_TYPE_NUM"] = from_bcd<uint16_t>(&buf, false, 2);
    hdr["CH_SET_NUM"] = from_bcd<uint16_t>(&buf, false, 2);
    uint32_t trc_num = from_bcd<uint32_t>(&buf, false, 4);
    hdr["TRACE_NUMBER"] = trc_num;
    hdr["FIRST_TIMING_WORD"] = read_u24(&buf) / pow(2, 8);
    uint8_t tr_hdr_ext = read_u8(&buf);
    hdr["TR_HDR_EXT"] = tr_hdr_ext;
    hdr["SAMPLE_SKEW"] = read_u8(&buf) / pow(2, 8);
    hdr["TRACE_EDIT"] = static_cast<uint8_t>(read_u8(&buf));
    hdr["TIME_BREAK_WIN"] = read_u24(&buf) / pow(2, 8);
    uint16_t ext_ch_set_num = read_u16(&buf);
    if (ext_ch_set_num)
        hdr["CH_SET_NUM"] = ext_ch_set_num;
    uint32_t ext_file_num = read_u24(&buf);
    if (ext_file_num)
        hdr["FFID"] = ext_file_num;

    return hdr;
}

void ISEGD::Impl::read_trace_header_ext(unordered_map<string,
                                        Trace::Header::Value>& hdr)
{
    int trc_hdr_ext = get<int64_t>(hdr["TR_HDR_EXT"]);
    for (int i = 0; i < trc_hdr_ext; ++i) {
        fill_buf_from_file(common.trc_ext_hdr_buf,
            CommonSEGD::TRACE_HEADER_EXT_SIZE);
        char const* buf = common.trc_ext_hdr_buf;
        if (common.trace_header_extension.size()) {
            for (auto& p : common.trace_header_extension[i]) {
                char const* pos = common.trc_ext_hdr_buf + p.first;
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
                    hdr[p.second.first] = static_cast<int64_t>(read_u64(&buf));
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
        } else if (i == 0) {
            // if there are no trace header map, read first according to
            // standard and skip all other
            hdr["R_LINE"] = read_u24(&buf);
            hdr["R_POINT"] = read_u24(&buf);
            hdr["R_POINT_IDX"] = read_u8(&buf);
            if (common.general_header2.segd_rev_major < 3) {
                hdr["SAMP_NUM"] = read_u24(&buf);
            } else {
                hdr["RESHOOT_IDX"] = read_u8(&buf);
                hdr["GROUP_IDX"] = read_u8(&buf);
                hdr["DEPTH_IDX"] = read_u8(&buf);
            }
            if (common.general_header2.segd_rev_major > 1) {
                double ext_r_line = read_u24(&buf);
                ext_r_line += read_u16(&buf) / pow(2, 16);
                if (ext_r_line != 0.0)
                    hdr["R_LINE"] = ext_r_line;
                double ext_r_point = read_u24(&buf);
                ext_r_point += read_u16(&buf) / pow(2, 16);
                if (ext_r_point != 0.0)
                    hdr["R_POINT"] = ext_r_point;
                hdr["SENSOR_TYPE"] = read_u8(&buf);
            }
            if (common.general_header2.segd_rev_major > 2) {
                uint32_t ext_tr_num = read_u24(&buf);
                if (get<int64_t>(hdr["TRACE_NUMBER"]) == 16665)
                    hdr["TRACE_NUMBER"] = ext_tr_num;
                hdr["SAMP_NUM"] = read_u32(&buf);
                hdr["SENSOR_MOVING"] = read_u8(&buf);
                ++buf;
                hdr["PHYSICAL_UNIT"] = read_u8(&buf);
            }
        }
    }
}

vector<double> ISEGD::Impl::read_trace_samples(unordered_map<string,
											   Trace::Header::Value>& hdr)
{
    uint32_t samp_num;
    CommonSEGD::ChannelSetHeader curr_ch_set =
	   	common.channel_sets[get<int64_t>(hdr["SCAN_TYPE_NUM"]) - 1]
		[get<int64_t>(hdr["CH_SET_NUM"]) - 1];
    if (hdr.find("SAMP_NUM") != hdr.end())
        samp_num = get<int64_t>(hdr["SAMP_NUM"]);
    else if (curr_ch_set.number_of_samples())
        samp_num = *curr_ch_set.number_of_samples();
    else
        samp_num = curr_ch_set.subscans_per_ch_set;
    if (common.trc_samp_buf.size() != (samp_num * common.bits_per_sample) / 8)
        common.trc_samp_buf.resize((samp_num * common.bits_per_sample) / 8);
    fill_buf_from_file(common.trc_samp_buf.data(), common.trc_samp_buf.size());
    char const* buf = common.trc_samp_buf.data();
    vector<double> result(samp_num);
    double descale = pow(2, curr_ch_set.descale_multiplier);
    for (uint32_t i = 0; i < samp_num; ++i)
        result[i] = read_sample(&buf) * descale;
    return result;
}

void ISEGD::Impl::fill_buf_from_file(char* buf, streamsize n)
{
    common.file.read(buf, n);
    curr_pos = common.file.tellg();
}

void ISEGD::Impl::file_skip_bytes(streamoff off)
{
    common.file.seekg(off, ios_base::cur);
    curr_pos = common.file.tellg();
}

void ISEGD::Impl::read_trailer()
{
    if (common.general_header.add_gen_hdr_blocks &&
	   	common.general_header2.gen_trailer_num_of_blocks)
        file_skip_bytes(CommonSEGD::GEN_TRLR_SIZE *
					   	common.general_header2.gen_trailer_num_of_blocks);
}

CommonSEGD::GeneralHeader ISEGD::general_header()
{
    return pimpl->common.general_header;
}

optional<CommonSEGD::GeneralHeader2> ISEGD::general_header2()
{
    return pimpl->common.general_header.add_gen_hdr_blocks ?
	   	optional<CommonSEGD::GeneralHeader2>(pimpl->common.general_header2) :
	   	std::nullopt;
}

vector<CommonSEGD::GeneralHeaderN> ISEGD::general_headerN()
{
    if (!pimpl->common.general_header.add_gen_hdr_blocks ||
	   	pimpl->common.general_header2.segd_rev_major > 2)
        return {};
    vector<CommonSEGD::GeneralHeaderN> result;
    for (map<int, unique_ptr<CommonSEGD::AdditionalGeneralHeader>>::iterator it
         = pimpl->common.add_gen_hdr_blks_map.begin(),
         end = pimpl->common.add_gen_hdr_blks_map.end();
         it != end; ++it) {
        result.push_back(*dynamic_cast<CommonSEGD::GeneralHeaderN*>
						 (it->second.get()));
    }
    return result;
}

optional<CommonSEGD::GeneralHeader3> ISEGD::general_header3()
{
    return pimpl->common.general_header.add_gen_hdr_blocks &&
	   	pimpl->common.general_header2.segd_rev_major > 2 ?
	   	optional<CommonSEGD::GeneralHeader3>(pimpl->common.general_header3) :
	   	std::nullopt;
}

optional<CommonSEGD::GeneralHeaderVes> ISEGD::general_header_vessel_crew_id()
{
    map<int, unique_ptr<CommonSEGD::AdditionalGeneralHeader>>::iterator it =
		   	pimpl->common.add_gen_hdr_blks_map.find(
				CommonSEGD::AdditionalGeneralHeader::VESSEL_CREW_ID);
    return it != pimpl->common.add_gen_hdr_blks_map.end() ?
	   	optional<CommonSEGD::GeneralHeaderVes>(
			*dynamic_cast<CommonSEGD::GeneralHeaderVes*>(it->second.get())) :
	   	nullopt;
}

optional<CommonSEGD::GeneralHeaderSur> ISEGD::general_header_survey_name()
{
    map<int, unique_ptr<CommonSEGD::AdditionalGeneralHeader>>::iterator it =
		   	pimpl->common.add_gen_hdr_blks_map.find(
				CommonSEGD::AdditionalGeneralHeader::SURVEY_AREA_NAME);
    return it != pimpl->common.add_gen_hdr_blks_map.end() ?
	   	optional<CommonSEGD::GeneralHeaderSur>(
			*dynamic_cast<CommonSEGD::GeneralHeaderSur*>(it->second.get())) :
	   	nullopt;
}

optional<CommonSEGD::GeneralHeaderCli> ISEGD::general_header_client_name()
{
    map<int, unique_ptr<CommonSEGD::AdditionalGeneralHeader>>::iterator it =
		   	pimpl->common.add_gen_hdr_blks_map.find(
				CommonSEGD::AdditionalGeneralHeader::CLIENT_NAME);
    return it != pimpl->common.add_gen_hdr_blks_map.end() ?
	   	optional<CommonSEGD::GeneralHeaderCli>(
			*dynamic_cast<CommonSEGD::GeneralHeaderCli*>(it->second.get())) :
	   	nullopt;
}

optional<CommonSEGD::GeneralHeaderJob> ISEGD::general_header_job_id()
{
    map<int, unique_ptr<CommonSEGD::AdditionalGeneralHeader>>::iterator it =
		   	pimpl->common.add_gen_hdr_blks_map.find(
				CommonSEGD::AdditionalGeneralHeader::JOB_ID);
    return it != pimpl->common.add_gen_hdr_blks_map.end() ?
	   	optional<CommonSEGD::GeneralHeaderJob>(
			*dynamic_cast<CommonSEGD::GeneralHeaderJob*>(it->second.get())) :
	   	nullopt;
}

optional<CommonSEGD::GeneralHeaderLin> ISEGD::general_header_line_id()
{
    map<int, unique_ptr<CommonSEGD::AdditionalGeneralHeader>>::iterator it =
		   	pimpl->common.add_gen_hdr_blks_map.find(
				CommonSEGD::AdditionalGeneralHeader::LINE_ID);
    return it != pimpl->common.add_gen_hdr_blks_map.end() ?
	   	optional<CommonSEGD::GeneralHeaderLin>(
			*dynamic_cast<CommonSEGD::GeneralHeaderLin*>(it->second.get())) :
	   	nullopt;
}

optional<CommonSEGD::GeneralHeaderVib> ISEGD::general_header_vibrator_info()
{
    map<int, unique_ptr<CommonSEGD::AdditionalGeneralHeader>>::iterator it =
		   	pimpl->common.add_gen_hdr_blks_map.find(
				CommonSEGD::AdditionalGeneralHeader::VIBRATOR_SOURCE_INFO);
    return it != pimpl->common.add_gen_hdr_blks_map.end() ?
	   	optional<CommonSEGD::GeneralHeaderVib>(
			*dynamic_cast<CommonSEGD::GeneralHeaderVib*>(it->second.get())) :
	   	nullopt;
}

optional<CommonSEGD::GeneralHeaderExp> ISEGD::general_header_explosive_info()
{
    map<int, unique_ptr<CommonSEGD::AdditionalGeneralHeader>>::iterator it =
		   	pimpl->common.add_gen_hdr_blks_map.find(
				CommonSEGD::AdditionalGeneralHeader::EXPLOSIVE_SOURCE_INFO);
    return it != pimpl->common.add_gen_hdr_blks_map.end() ?
	   	optional<CommonSEGD::GeneralHeaderExp>(
			*dynamic_cast<CommonSEGD::GeneralHeaderExp*>(it->second.get())) :
	   	nullopt;
}

optional<CommonSEGD::GeneralHeaderAir> ISEGD::general_header_airgun_info()
{
    map<int, unique_ptr<CommonSEGD::AdditionalGeneralHeader>>::iterator it =
		   	pimpl->common.add_gen_hdr_blks_map.find(
				CommonSEGD::AdditionalGeneralHeader::AIRGUN_SOURCE_INFO);
    return it != pimpl->common.add_gen_hdr_blks_map.end() ?
	   	optional<CommonSEGD::GeneralHeaderAir>(
			*dynamic_cast<CommonSEGD::GeneralHeaderAir*>(it->second.get())) :
	   	nullopt;
}

optional<CommonSEGD::GeneralHeaderWat> ISEGD::general_header_watergun_info()
{
    map<int, unique_ptr<CommonSEGD::AdditionalGeneralHeader>>::iterator it =
		   	pimpl->common.add_gen_hdr_blks_map.find(
				CommonSEGD::AdditionalGeneralHeader::WATERGUN_SOURCE_INFO);
    return it != pimpl->common.add_gen_hdr_blks_map.end() ?
	   	optional<CommonSEGD::GeneralHeaderWat>(
			*dynamic_cast<CommonSEGD::GeneralHeaderWat*>(it->second.get())) :
	   	nullopt;
}

optional<CommonSEGD::GeneralHeaderEle>
ISEGD::general_header_electromagnetic_info()
{
    map<int, unique_ptr<CommonSEGD::AdditionalGeneralHeader>>::iterator it =
		   	pimpl->common.add_gen_hdr_blks_map.find(
				CommonSEGD::AdditionalGeneralHeader::ELECTROMAGNETIC_SOURCE);
    return it != pimpl->common.add_gen_hdr_blks_map.end() ?
	   	optional<CommonSEGD::GeneralHeaderEle>(
			*dynamic_cast<CommonSEGD::GeneralHeaderEle*>(it->second.get())) :
	   	nullopt;
}

optional<CommonSEGD::GeneralHeaderOth>
ISEGD::general_header_other_source_info()
{
    map<int, unique_ptr<CommonSEGD::AdditionalGeneralHeader>>::iterator it =
		   	pimpl->common.add_gen_hdr_blks_map.find(
				CommonSEGD::AdditionalGeneralHeader::OTHER_SOURCE_TYPE_INFO);
    return it != pimpl->common.add_gen_hdr_blks_map.end() ?
	   	optional<CommonSEGD::GeneralHeaderOth>(
			*dynamic_cast<CommonSEGD::GeneralHeaderOth*>(it->second.get())) :
	   	nullopt;
}

optional<CommonSEGD::GeneralHeaderAdd> ISEGD::general_header_add_source_info()
{
    map<int, unique_ptr<CommonSEGD::AdditionalGeneralHeader>>::iterator it =
		   	pimpl->common.add_gen_hdr_blks_map.find(
				CommonSEGD::AdditionalGeneralHeader::ADD_SOURCE_INFO);
    return it != pimpl->common.add_gen_hdr_blks_map.end() ?
	   	optional<CommonSEGD::GeneralHeaderAdd>(
			*dynamic_cast<CommonSEGD::GeneralHeaderAdd*>(it->second.get())) :
	   	nullopt;
}

optional<CommonSEGD::GeneralHeaderSaux>
ISEGD::general_header_sou_aux_chan_ref()
{
    map<int, unique_ptr<CommonSEGD::AdditionalGeneralHeader>>::iterator it =
		   	pimpl->common.add_gen_hdr_blks_map.find(
				CommonSEGD::AdditionalGeneralHeader::SOU_AUX_CHAN_REF);
    return it != pimpl->common.add_gen_hdr_blks_map.end() ?
	   	optional<CommonSEGD::GeneralHeaderSaux>(
			*dynamic_cast<CommonSEGD::GeneralHeaderSaux*>(it->second.get())) :
	   	nullopt;
}

optional<CommonSEGD::GeneralHeaderSen>
ISEGD::general_header_sen_info_hdr_ext_blk()
{
    map<int, unique_ptr<CommonSEGD::AdditionalGeneralHeader>>::iterator it =
		   	pimpl->common.add_gen_hdr_blks_map.find(
				CommonSEGD::AdditionalGeneralHeader::SENSOR_INFO_HDR_EXT_BLK);
    return it != pimpl->common.add_gen_hdr_blks_map.end() ?
	   	optional<CommonSEGD::GeneralHeaderSen>(
			*dynamic_cast<CommonSEGD::GeneralHeaderSen*>(it->second.get())) :
	   	nullopt;
}
optional<CommonSEGD::GeneralHeaderSCa> ISEGD::general_header_sen_calib_blk()
{
    map<int, unique_ptr<CommonSEGD::AdditionalGeneralHeader>>::iterator it =
		   	pimpl->common.add_gen_hdr_blks_map.find(
				CommonSEGD::AdditionalGeneralHeader::SENSOR_CALIBRATION_BLK);
    return it != pimpl->common.add_gen_hdr_blks_map.end() ?
	   	optional<CommonSEGD::GeneralHeaderSCa>(
			*dynamic_cast<CommonSEGD::GeneralHeaderSCa*>(it->second.get())) :
	   	nullopt;
}

optional<CommonSEGD::GeneralHeaderTim> ISEGD::general_header_time_drift_blk()
{
    map<int, unique_ptr<CommonSEGD::AdditionalGeneralHeader>>::iterator it =
		   	pimpl->common.add_gen_hdr_blks_map.find(
				CommonSEGD::AdditionalGeneralHeader::TIME_DRIFT_BLK);
    return it != pimpl->common.add_gen_hdr_blks_map.end() ?
	   	optional<CommonSEGD::GeneralHeaderTim>(
			*dynamic_cast<CommonSEGD::GeneralHeaderTim*>(it->second.get())) :
	   	nullopt;
}

optional<CommonSEGD::GeneralHeaderElSR>
ISEGD::general_header_elemag_src_rec_desc_blk()
{
    map<int, unique_ptr<CommonSEGD::AdditionalGeneralHeader>>::iterator it =
		   	pimpl->common.add_gen_hdr_blks_map.find(
				CommonSEGD::AdditionalGeneralHeader::
				ELECTROMAG_SRC_REC_DESC_BLK);
    return it != pimpl->common.add_gen_hdr_blks_map.end() ?
	   	optional<CommonSEGD::GeneralHeaderElSR>(
			*dynamic_cast<CommonSEGD::GeneralHeaderElSR*>(it->second.get())) :
	   	nullopt;
}

optional<CommonSEGD::GeneralHeaderPos1> ISEGD::general_header_position_blk_1()
{
    map<int, unique_ptr<CommonSEGD::AdditionalGeneralHeader>>::iterator it =
		   	pimpl->common.add_gen_hdr_blks_map.find(
				CommonSEGD::AdditionalGeneralHeader::POSITION_BLK1);
    return it != pimpl->common.add_gen_hdr_blks_map.end() ?
	   	optional<CommonSEGD::GeneralHeaderPos1>(
			*dynamic_cast<CommonSEGD::GeneralHeaderPos1*>(it->second.get())) :
	   	nullopt;
}

optional<CommonSEGD::GeneralHeaderPos2> ISEGD::general_header_position_blk_2()
{
    map<int, unique_ptr<CommonSEGD::AdditionalGeneralHeader>>::iterator it =
		   	pimpl->common.add_gen_hdr_blks_map.find(
				CommonSEGD::AdditionalGeneralHeader::POSITION_BLK2);
    return it != pimpl->common.add_gen_hdr_blks_map.end() ?
	   	optional<CommonSEGD::GeneralHeaderPos2>(
			*dynamic_cast<CommonSEGD::GeneralHeaderPos2*>(it->second.get())) :
	   	nullopt;
}

optional<CommonSEGD::GeneralHeaderPos3> ISEGD::general_header_position_blk_3()
{
    map<int, unique_ptr<CommonSEGD::AdditionalGeneralHeader>>::iterator it =
		   	pimpl->common.add_gen_hdr_blks_map.find(
				CommonSEGD::AdditionalGeneralHeader::POSITION_BLK3);
    return it != pimpl->common.add_gen_hdr_blks_map.end() ?
	   	optional<CommonSEGD::GeneralHeaderPos3>(
			*dynamic_cast<CommonSEGD::GeneralHeaderPos3*>(it->second.get())) :
	   	nullopt;
}

optional<CommonSEGD::GeneralHeaderCoord> ISEGD::general_header_coord_ref_blk()
{
    map<int, unique_ptr<CommonSEGD::AdditionalGeneralHeader>>::iterator it =
		   	pimpl->common.add_gen_hdr_blks_map.find(
				CommonSEGD::AdditionalGeneralHeader::COORD_REF_SYSTEM);
    return it != pimpl->common.add_gen_hdr_blks_map.end() ?
	   	optional<CommonSEGD::GeneralHeaderCoord>(
			*dynamic_cast<CommonSEGD::GeneralHeaderCoord*>(it->second.get())) :
	   	nullopt;
}

optional<CommonSEGD::GeneralHeaderRel> ISEGD::general_header_relative_pos_blk()
{
    map<int, unique_ptr<CommonSEGD::AdditionalGeneralHeader>>::iterator it =
		   	pimpl->common.add_gen_hdr_blks_map.find(
				CommonSEGD::AdditionalGeneralHeader::RELATIVE_POS_BLK);
    return it != pimpl->common.add_gen_hdr_blks_map.end() ?
	   	optional<CommonSEGD::GeneralHeaderRel>(
			*dynamic_cast<CommonSEGD::GeneralHeaderRel*>(it->second.get())) :
	   	nullopt;
}

optional<CommonSEGD::GeneralHeaderOri> ISEGD::general_header_orient_hdr_blk()
{
    map<int, unique_ptr<CommonSEGD::AdditionalGeneralHeader>>::iterator it =
		   	pimpl->common.add_gen_hdr_blks_map.find(
				CommonSEGD::AdditionalGeneralHeader::ORIENT_HDR_BLK);
    return it != pimpl->common.add_gen_hdr_blks_map.end() ?
	   	optional<CommonSEGD::GeneralHeaderOri>(
			*dynamic_cast<CommonSEGD::GeneralHeaderOri*>(it->second.get())) :
	   	nullopt;
}

optional<CommonSEGD::GeneralHeaderMeas> ISEGD::general_header_measurement_blk()
{
    map<int, unique_ptr<CommonSEGD::AdditionalGeneralHeader>>::iterator it =
		   	pimpl->common.add_gen_hdr_blks_map.find(
				CommonSEGD::AdditionalGeneralHeader::MEASUREMENT_BLK);
    return it != pimpl->common.add_gen_hdr_blks_map.end() ?
	   	optional<CommonSEGD::GeneralHeaderMeas>(
			*dynamic_cast<CommonSEGD::GeneralHeaderMeas*>(it->second.get())) :
	   	nullopt;
}

vector<vector<char>> ISEGD::extended_headers()
{
    return pimpl->common.extended_headers;
}

vector<vector<char>> ISEGD::external_headers()
{
    return pimpl->common.external_headers;
}

vector<vector<CommonSEGD::ChannelSetHeader>> const&
ISEGD::channel_set_headers()
{
    return pimpl->common.channel_sets;
}

ISEGD::ISEGD(string name, vector<map<uint32_t, pair<string,
    Trace::Header::ValueType>>> tr_hdr_ext)
    : pimpl { make_unique<Impl>(CommonSEGD(move(name),
										   fstream::in | fstream::binary,
                                           {}, {}, {}, {}, {}, {}, {},
                                           tr_hdr_ext)) }
{
}

ISEGD::~ISEGD() = default;
} // namespace sedaman
