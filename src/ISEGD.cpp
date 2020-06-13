#include "ISEGD.hpp"
#include "Exception.hpp"
#include "Trace.hpp"
#include "util.hpp"
#include <cmath>
#include <cstring>
#include <functional>
#include <unordered_map>

using std::fstream;
using std::function;
using std::get;
using std::ios_base;
using std::make_unique;
using std::move;
using std::optional;
using std::streamoff;
using std::streampos;
using std::streamsize;
using std::string;
using std::unordered_map;
using std::vector;

namespace sedaman {
class ISEGD::Impl {
public:
    Impl(CommonSEGD com);
    CommonSEGD common;
    streampos curr_pos;
    streampos end_of_data;
    enum ADD_GEN_HDR_BLKS {
        VESSEL_CREW_ID = 0x10,
        SURVEY_AREA_NAME = 0x11,
        CLIENT_NAME = 0x12,
        JOB_ID = 0x13,
        LINE_ID = 0x14,
        VIBRATOR_SOURCE_INFO = 0x15,
        EXPLOSIVE_SOURCE_INFO = 0x16,
        AIRGUN_SOURCE_INFO = 0x17,
        WATERGUN_SOURCE_INFO = 0x18,
        ELECTROMAGNETIC_SOURCE = 0x19,
        OTHER_SOURCE_TYPE_INFO = 0x1f,
        ADD_SOURCE_INFO = 0x20,
        SOU_AUX_CHAN_REF = 0x21,
        CH_SET_DESCRIP_BLK1 = 0x30,
        CH_SET_DESCRIP_BLK2 = 0x31,
        CH_SET_DESCRIP_BLK3 = 0x32,
        TRACE_HDR_EXT1 = 0x40,
        SENSOR_INFO_HDR_EXT_BLK = 0x41,
        TIME_STAMP_HDR_BLK = 0x42,
        SENSOR_CALIBRATION_BLK = 0x43,
        TIME_DRIFT_BLK = 0x44,
        ELECTROMAG_SRC_REC_DESC_BLK = 0x45,
        POSITION_BLK1 = 0x50,
        POSITION_BLK2 = 0x51,
        POSITION_BLK3 = 0x52,
        COORD_REF_SYSTEM = 0x55,
        RELATIVE_POS_BLK = 0x56,
        ORIENT_HDR_BLK = 0x60,
        MEASUREMENT_BLK = 0x61,
        GEN_TRAILER_DESC_BLK = 0x70
    };
    unordered_map<ADD_GEN_HDR_BLKS, bool> add_gen_hdr_blks_map;

private:
    void read_general_headers();
    CommonSEGD::ChannelSetHeader read_ch_set_hdr();
    void fill_buf_from_file(char* buf, streamsize n);
    void file_skip_bytes(streamoff off);
    function<double(char const**)> read_sample;
    function<uint16_t(char const**)> read_u16;
    function<int16_t(char const**)> read_i16;
    function<uint32_t(char const**)> read_u24;
    function<int32_t(char const**)> read_i24;
    function<uint32_t(char const**)> read_u32;
    function<int32_t(char const**)> read_i32;
    function<uint64_t(char const**)> read_u64;
    void read_gen_hdr1(char const* buf);
    void read_gen_hdr2(char const* buf);
    void read_gen_hdr2_and_3(char const* buf);
    void read_rev3_add_gen_hdr_blks(char const* buf);
    void read_vessel_crew_id(char const* buf);
    void read_survey_area_name(char const* buf);
    void read_client_name(const char* buf);
    void read_job_id(char const* buf);
    void read_line_id(char const* buf);
    void read_vibrator_hdr(char const* buf);
    void read_explosive_hdr(char const* buf);
    void read_airgun_hdr(char const* buf);
    void read_watergun_hdr(char const* buf);
    void read_electromagnetic_hdr(char const* buf);
    void read_other_source_hdr(char const* buf);
    void read_additional_source_hdr(char const* buf);
    void read_source_aux_hdr(char const* buf);
    void read_crs_hdr(char const* buf);
    void read_position1_hdr(char const* buf);
    void read_position2_hdr(char const* buf);
    void read_position3_hdr(char const* buf);
    void read_relative_position_hdr(char const* buf);
    void read_sensor_info_hdr(char const* buf);
    void read_sensor_calibration_hdr(char const* buf);
    void read_time_drift_hdr(char const* buf);
    void read_electromagnetic_src_recv_hdr(char const* buf);
    void read_orientation_hdr(char const* buf);
    void read_measurement_hdr(char const* buf);
    Trace::Header read_header();
    void assign_raw_readers();
    void assign_sample_reading();
};

static bool is_big_endian(void);

ISEGD::Impl::Impl(CommonSEGD com)
    : common(move(com))
{
    common.file.seekg(0, ios_base::end);
    end_of_data = common.file.tellg();
    common.file.seekg(0, ios_base::beg);
    curr_pos = common.file.tellg();
    assign_raw_readers();
    //while (curr_pos != end_of_data) {
    read_general_headers();
    assign_sample_reading();
    // read header for each scan type
    uint16_t ch_sets_per_scan_type_num = common.general_header.channel_sets_per_scan_type == 1665 ? common.general_header2.ext_ch_sets_per_scan_type : common.general_header.channel_sets_per_scan_type;
    for (int i = 0; i < common.general_header.scan_types_per_record; ++i) {
        common.ch_sets.push_back({});
        for (int j = ch_sets_per_scan_type_num; j; --j) {
            common.ch_sets[i].push_back(read_ch_set_hdr());
        }
        uint16_t skew_blks = common.general_header.skew_blocks == 165 ? common.general_header2.extended_skew_blocks : common.general_header.skew_blocks;
        file_skip_bytes(skew_blks * CommonSEGD::SKEW_BLOCK_SIZE);
    }
    uint32_t extended_blocks = common.general_header.extended_hdr_blocks == 165 ? common.general_header2.extended_hdr_blocks : common.general_header.extended_hdr_blocks;
    uint32_t external_blocks = common.general_header.external_hdr_blocks == 165 ? common.general_header2.external_hdr_blocks : common.general_header.external_hdr_blocks;
    file_skip_bytes(extended_blocks * CommonSEGD::EXTENDED_HEADER_SIZE + external_blocks * CommonSEGD::EXTERNAL_HEADER_SIZE);
    //  }
}

void ISEGD::Impl::assign_raw_readers()
{
    if (is_big_endian()) {
        read_u16 = [](char const** buf) { return read<uint16_t>(buf); };
        read_i16 = [](char const** buf) { return read<int16_t>(buf); };
        read_u24 = [](char const** buf) { return read<uint16_t>(buf) << 8 | read<uint8_t>(buf); };
        read_i24 = [](char const** buf) {
            uint32_t result = read<uint16_t>(buf) << 8 | read<uint8_t>(buf);
            return result & 0x800000 ? result | 0xff000000 : result;
        };
        read_u32 = [](char const** buf) { return read<uint32_t>(buf); };
        read_i32 = [](char const** buf) { return read<int32_t>(buf); };
        read_u64 = [](char const** buf) { return read<uint64_t>(buf); };
    } else {
        read_u16 = [](char const** buf) { return swap(read<uint16_t>(buf)); };
        read_i16 = [](char const** buf) { return swap(read<int16_t>(buf)); };
        read_u24 = [](char const** buf) { return swap(read<uint16_t>(buf)) | read<uint8_t>(buf) << 16; };
        read_i24 = [](char const** buf) {
            uint32_t result = swap(read<uint16_t>(buf)) | read<uint8_t>(buf) << 16;
            return result & 0x800000 ? result | 0xff000000 : result;
        };
        read_u32 = [](char const** buf) { return swap(read<uint32_t>(buf)); };
        read_i32 = [](char const** buf) { return swap(read<int32_t>(buf)); };
        read_u64 = [](char const** buf) { return swap(read<uint64_t>(buf)); };
    }
}

void ISEGD::Impl::read_general_headers()
{
    fill_buf_from_file(common.gen_hdr_buf.data(), common.gen_hdr_buf.size());
    char const* buf = common.gen_hdr_buf.data();
    read_gen_hdr1(buf);
    if (common.general_header.add_gen_hdr_blocks) {
        fill_buf_from_file(common.gen_hdr_buf.data(), common.gen_hdr_buf.size());
        char const* buf = common.gen_hdr_buf.data();
        CommonSEGD::GeneralHeader2& gh2 = common.general_header2;
        buf += 10;
        gh2.segd_rev_major = *buf++;
        gh2.segd_rev_minor = *buf++;
        buf = common.gen_hdr_buf.data();
        gh2.expanded_file_num = read_u24(&buf);
        gh2.ext_ch_sets_per_scan_type = read_u16(&buf);
        if (gh2.segd_rev_major < 3) {
            read_gen_hdr2(buf);
        } else {
            read_gen_hdr2_and_3(buf);
            uint16_t add_blks_num = common.general_header.add_gen_hdr_blocks == 0xf ? gh2.ext_num_add_blks_in_gen_hdr : common.general_header.add_gen_hdr_blocks;
            for (uint16_t i = add_blks_num - 2; i; --i) {
                fill_buf_from_file(common.gen_hdr_buf.data(), common.gen_hdr_buf.size());
                char const* buf = common.gen_hdr_buf.data();
                read_rev3_add_gen_hdr_blks(buf);
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
    gh.base_scan_int = *buf++ / pow(2, 4);
    gh.polarity = from_bcd<int>(&buf, false, 1);
    int exp = static_cast<unsigned>(*buf++) & 0x0f;
    gh.scans_per_block = static_cast<unsigned>(*buf++) * pow(2, exp);
    gh.record_type = from_bcd<int>(&buf, false, 1);
    gh.record_length = from_bcd<int>(&buf, true, 3);
    gh.scan_types_per_record = from_bcd<int>(&buf, false, 2);
    gh.channel_sets_per_scan_type = from_bcd<int>(&buf, false, 2);
    gh.skew_blocks = from_bcd<int>(&buf, false, 2);
    gh.extended_hdr_blocks = from_bcd<int>(&buf, false, 2);
    gh.external_hdr_blocks = from_bcd<int>(&buf, false, 2);
}

void ISEGD::Impl::read_gen_hdr2(char const* buf)
{
    CommonSEGD::GeneralHeader2& gh2 = common.general_header2;
    gh2.segd_rev_minor = gh2.segd_rev_minor / pow(2, 8) * 10;
    gh2.extended_hdr_blocks = read_u16(&buf);
    gh2.external_hdr_blocks = read_u16(&buf);
    buf += 3;
    gh2.gen_trailer_num_of_blocks = read_u16(&buf);
    gh2.ext_record_len = read_u24(&buf);
    gh2.gen_hdr_block_num = *++buf;
    if (common.general_header.add_gen_hdr_blocks > 1) {
        fill_buf_from_file(common.gen_hdr_buf.data(), common.gen_hdr_buf.size());
        char const* buf = common.gen_hdr_buf.data();
        CommonSEGD::GeneralHeaderN& ghN = common.general_headerN;
        ghN.expanded_file_number = read_u24(&buf);
        ghN.sou_line_num = read_i24(&buf);
        ghN.sou_line_num += read_u16(&buf) / pow(2, 16);
        ghN.sou_point_num = read_i24(&buf);
        ghN.sou_point_num += read_u16(&buf) / pow(2, 16);
        ghN.sou_point_index = *buf++;
        ghN.phase_control = *buf++;
        ghN.type_vibrator = *buf++;
        ghN.phase_angle = read_i16(&buf);
        ghN.gen_hdr_block_num = *buf++;
        ghN.sou_set_num = *buf++;
        file_skip_bytes(CommonSEGD::GEN_HDR_SIZE * (common.general_header.add_gen_hdr_blocks - 2));
    }
}

void ISEGD::Impl::read_gen_hdr2_and_3(char const* buf)
{
    CommonSEGD::GeneralHeader2& gh2 = common.general_header2;
    gh2.extended_hdr_blocks = read_u24(&buf);
    gh2.extended_skew_blocks = read_u16(&buf);
    buf += 2;
    gh2.gen_trailer_num_of_blocks = read_u32(&buf);
    gh2.record_set_number = read_u16(&buf);
    gh2.ext_num_add_blks_in_gen_hdr = read_u16(&buf);
    gh2.dominant_sampling_int = read_u16(&buf);
    gh2.external_hdr_blocks = read_u24(&buf);
    gh2.gen_hdr_block_num = *++buf;
    fill_buf_from_file(common.gen_hdr_buf.data(), common.gen_hdr_buf.size());
    buf = common.gen_hdr_buf.data();
    CommonSEGD::GeneralHeader3& gh3 = common.general_header3;
    gh3.time_zero = read_u64(&buf);
    gh3.record_size = read_u64(&buf);
    gh3.data_size = read_u64(&buf);
    gh3.header_size = read_u32(&buf);
    gh3.extd_rec_mode = *buf++;
    gh3.rel_time_mode = *buf++;
    gh3.gen_hdr_block_num = *++buf;
}

void ISEGD::Impl::read_rev3_add_gen_hdr_blks(char const* buf)
{
    int hdr_blk_type = *(buf + 31);
    switch (hdr_blk_type) {
    case VESSEL_CREW_ID:
        read_vessel_crew_id(buf);
        add_gen_hdr_blks_map[VESSEL_CREW_ID] = true;
        break;
    case SURVEY_AREA_NAME:
        read_survey_area_name(buf);
        add_gen_hdr_blks_map[SURVEY_AREA_NAME] = true;
        break;
    case CLIENT_NAME:
        read_client_name(buf);
        add_gen_hdr_blks_map[CLIENT_NAME] = true;
        break;
    case JOB_ID:
        read_job_id(buf);
        add_gen_hdr_blks_map[JOB_ID] = true;
        break;
    case LINE_ID:
        read_line_id(buf);
        add_gen_hdr_blks_map[LINE_ID] = true;
        break;
    case VIBRATOR_SOURCE_INFO:
        read_vibrator_hdr(buf);
        add_gen_hdr_blks_map[VIBRATOR_SOURCE_INFO] = true;
        break;
    case EXPLOSIVE_SOURCE_INFO:
        read_explosive_hdr(buf);
        add_gen_hdr_blks_map[EXPLOSIVE_SOURCE_INFO] = true;
        break;
    case AIRGUN_SOURCE_INFO:
        read_airgun_hdr(buf);
        add_gen_hdr_blks_map[AIRGUN_SOURCE_INFO] = true;
        break;
    case WATERGUN_SOURCE_INFO:
        read_watergun_hdr(buf);
        add_gen_hdr_blks_map[WATERGUN_SOURCE_INFO] = true;
        break;
    case ELECTROMAGNETIC_SOURCE:
        read_electromagnetic_hdr(buf);
        add_gen_hdr_blks_map[ELECTROMAGNETIC_SOURCE] = true;
        break;
    case OTHER_SOURCE_TYPE_INFO:
        read_other_source_hdr(buf);
        add_gen_hdr_blks_map[OTHER_SOURCE_TYPE_INFO] = true;
        break;
    case ADD_SOURCE_INFO:
        read_additional_source_hdr(buf);
        add_gen_hdr_blks_map[ADD_SOURCE_INFO] = true;
        break;
    case SOU_AUX_CHAN_REF:
        read_source_aux_hdr(buf);
        add_gen_hdr_blks_map[SOU_AUX_CHAN_REF] = true;
        break;
    case SENSOR_INFO_HDR_EXT_BLK:
        read_sensor_info_hdr(buf);
        add_gen_hdr_blks_map[SENSOR_INFO_HDR_EXT_BLK] = true;
        break;
    case SENSOR_CALIBRATION_BLK:
        read_sensor_calibration_hdr(buf);
        add_gen_hdr_blks_map[SENSOR_CALIBRATION_BLK] = true;
        break;
    case TIME_DRIFT_BLK:
        read_time_drift_hdr(buf);
        add_gen_hdr_blks_map[TIME_DRIFT_BLK] = true;
        break;
    case ELECTROMAG_SRC_REC_DESC_BLK:
        read_electromagnetic_src_recv_hdr(buf);
        add_gen_hdr_blks_map[ELECTROMAG_SRC_REC_DESC_BLK] = true;
        break;
    case POSITION_BLK1:
        read_position1_hdr(buf);
        add_gen_hdr_blks_map[POSITION_BLK1] = true;
        break;
    case POSITION_BLK2:
        read_position2_hdr(buf);
        add_gen_hdr_blks_map[POSITION_BLK2] = true;
        break;
    case POSITION_BLK3:
        read_position3_hdr(buf);
        add_gen_hdr_blks_map[POSITION_BLK3] = true;
        break;
    case COORD_REF_SYSTEM:
        read_crs_hdr(buf);
        add_gen_hdr_blks_map[COORD_REF_SYSTEM] = true;
        break;
    case RELATIVE_POS_BLK:
        read_relative_position_hdr(buf);
        add_gen_hdr_blks_map[RELATIVE_POS_BLK] = true;
        break;
    case ORIENT_HDR_BLK:
        read_orientation_hdr(buf);
        add_gen_hdr_blks_map[ORIENT_HDR_BLK] = true;
        break;
    case MEASUREMENT_BLK:
        read_measurement_hdr(buf);
        add_gen_hdr_blks_map[MEASUREMENT_BLK] = true;
        break;
    }
}

void ISEGD::Impl::read_vessel_crew_id(char const* buf)
{
    CommonSEGD::GeneralHeaderVes& ghv = common.general_header_ves;
    memcpy(ghv.abbr_vessel_crew_name, buf, sizeof(ghv.abbr_vessel_crew_name));
    buf += sizeof(ghv.abbr_vessel_crew_name);
    memcpy(ghv.vessel_crew_name, buf, sizeof(ghv.vessel_crew_name));
    buf += sizeof(ghv.vessel_crew_name);
    ghv.gen_hdr_block_type = *buf;
}

void ISEGD::Impl::read_survey_area_name(const char* buf)
{
    CommonSEGD::GeneralHeaderSur& ghs = common.general_header_sur;
    memcpy(ghs.survey_area_name, buf, sizeof(ghs.survey_area_name));
    buf += sizeof(ghs.survey_area_name);
    ghs.gen_hdr_block_type = *buf;
}

void ISEGD::Impl::read_client_name(const char* buf)
{
    CommonSEGD::GeneralHeaderCli& ghc = common.general_header_cli;
    memcpy(ghc.client_name, buf, sizeof(ghc.client_name));
    buf += sizeof(ghc.client_name);
    ghc.gen_hdr_block_type = *buf;
}

void ISEGD::Impl::read_job_id(char const* buf)
{
    CommonSEGD::GeneralHeaderJob& ghj = common.general_header_job;
    memcpy(ghj.abbr_job_id, buf, sizeof(ghj.abbr_job_id));
    buf += sizeof(ghj.abbr_job_id);
    memcpy(ghj.job_id, buf, sizeof(ghj.job_id));
    buf += sizeof(ghj.job_id);
    ghj.gen_hdr_block_type = *buf;
}

void ISEGD::Impl::read_line_id(char const* buf)
{
    CommonSEGD::GeneralHeaderLin& ghl = common.general_header_lin;
    memcpy(ghl.line_abbr, buf, sizeof(ghl.line_abbr));
    buf += sizeof(ghl.line_abbr);
    memcpy(ghl.line_id, buf, sizeof(ghl.line_id));
    buf += sizeof(ghl.line_id);
    ghl.gen_hdr_block_type = *buf;
}

void ISEGD::Impl::read_vibrator_hdr(char const* buf)
{
    CommonSEGD::GeneralHeaderVib& ghV = common.general_header_vib;
    ghV.expanded_file_number = read_u24(&buf);
    ghV.sou_line_num = read_u24(&buf);
    ghV.sou_line_num += read_u16(&buf) / pow(2, 16);
    ghV.sou_point_num = read_u24(&buf);
    ghV.sou_point_num += read_u16(&buf) / pow(2, 16);
    ghV.sou_point_index = *buf++;
    ghV.phase_control = *buf++;
    ghV.type_vibrator = *buf++;
    ghV.phase_angle = read_u16(&buf);
    ghV.source_id = *buf++;
    ghV.source_set_num = *buf++;
    ghV.reshoot_idx = *buf++;
    ghV.group_idx = *buf++;
    ghV.depth_idx = *buf++;
    ghV.offset_crossline = *buf++;
    ghV.offset_inline = *buf++;
    ghV.size = *buf++;
    ghV.offset_depth = *buf++;
    ghV.gen_hdr_block_type = *buf++;
}

void ISEGD::Impl::read_explosive_hdr(char const* buf)
{
    CommonSEGD::GeneralHeaderExp& ghE = common.general_header_exp;
    ghE.expanded_file_number = read_u24(&buf);
    ghE.sou_line_num = read_u24(&buf);
    ghE.sou_line_num += read_u16(&buf) / pow(2, 16);
    ghE.sou_point_num = read_u24(&buf);
    ghE.sou_point_num += read_u16(&buf) / pow(2, 16);
    ghE.sou_point_index = *buf++;
    ghE.depth = read_u16(&buf);
    ghE.charge_length = *buf++;
    ghE.soil_type = *buf++;
    ghE.source_id = *buf++;
    ghE.source_set_num = *buf++;
    ghE.reshoot_idx = *buf++;
    ghE.group_idx = *buf++;
    ghE.depth_idx = *buf++;
    ghE.offset_crossline = *buf++;
    ghE.offset_inline = *buf++;
    ghE.size = *buf++;
    ghE.offset_depth = *buf++;
    ghE.gen_hdr_block_type = *buf++;
}

void ISEGD::Impl::read_airgun_hdr(char const* buf)
{
    CommonSEGD::GeneralHeaderAir& ghA = common.general_header_air;
    ghA.expanded_file_number = read_u24(&buf);
    ghA.sou_line_num = read_u24(&buf);
    ghA.sou_line_num += read_u16(&buf) / pow(2, 16);
    ghA.sou_point_num = read_u24(&buf);
    ghA.sou_point_num += read_u16(&buf) / pow(2, 16);
    ghA.sou_point_index = *buf++;
    ghA.depth = read_u16(&buf);
    ghA.air_pressure = read_u16(&buf);
    ghA.source_set_num = *buf++;
    ghA.reshoot_idx = *buf++;
    ghA.group_idx = *buf++;
    ghA.depth_idx = *buf++;
    ghA.offset_crossline = *buf++;
    ghA.offset_inline = *buf++;
    ghA.size = *buf++;
    ghA.offset_depth = *buf++;
    ghA.gen_hdr_block_type = *buf++;
}

void ISEGD::Impl::read_watergun_hdr(char const* buf)
{
    CommonSEGD::GeneralHeaderWat& ghA = common.general_header_wat;
    ghA.expanded_file_number = read_u24(&buf);
    ghA.sou_line_num = read_u24(&buf);
    ghA.sou_line_num += read_u16(&buf) / pow(2, 16);
    ghA.sou_point_num = read_u24(&buf);
    ghA.sou_point_num += read_u16(&buf) / pow(2, 16);
    ghA.sou_point_index = *buf++;
    ghA.depth = read_u16(&buf);
    ghA.air_pressure = read_u16(&buf);
    ghA.source_set_num = *buf++;
    ghA.reshoot_idx = *buf++;
    ghA.group_idx = *buf++;
    ghA.depth_idx = *buf++;
    ghA.offset_crossline = *buf++;
    ghA.offset_inline = *buf++;
    ghA.size = *buf++;
    ghA.offset_depth = *buf++;
    ghA.gen_hdr_block_type = *buf++;
}

void ISEGD::Impl::read_electromagnetic_hdr(char const* buf)
{
    CommonSEGD::GeneralHeaderEle& ghE = common.general_header_ele;
    ghE.expanded_file_number = read_u24(&buf);
    ghE.sou_line_num = read_u24(&buf);
    ghE.sou_line_num += read_u16(&buf) / pow(2, 16);
    ghE.sou_point_num = read_u24(&buf);
    ghE.sou_point_num += read_u16(&buf) / pow(2, 16);
    ghE.sou_point_index = *buf++;
    ghE.source_type = *buf++;
    ghE.moment = read_u24(&buf);
    ghE.source_set_num = *buf++;
    ghE.reshoot_idx = *buf++;
    ghE.group_idx = *buf++;
    ghE.depth_idx = *buf++;
    ghE.offset_crossline = *buf++;
    ghE.offset_inline = *buf++;
    ghE.size = *buf++;
    ghE.offset_depth = *buf++;
    ghE.gen_hdr_block_type = *buf++;
}

void ISEGD::Impl::read_other_source_hdr(char const* buf)
{
    CommonSEGD::GeneralHeaderOth& ghO = common.general_header_oth;
    ghO.expanded_file_number = read_u24(&buf);
    ghO.sou_line_num = read_u24(&buf);
    ghO.sou_line_num += read_u16(&buf) / pow(2, 16);
    ghO.sou_point_num = read_u24(&buf);
    ghO.sou_point_num += read_u16(&buf) / pow(2, 16);
    ghO.sou_point_index = *buf++;
    buf += 4;
    ghO.source_set_num = *buf++;
    ghO.reshoot_idx = *buf++;
    ghO.group_idx = *buf++;
    ghO.depth_idx = *buf++;
    ghO.offset_crossline = *buf++;
    ghO.offset_inline = *buf++;
    ghO.size = *buf++;
    ghO.offset_depth = *buf++;
    ghO.gen_hdr_block_type = *buf++;
}

void ISEGD::Impl::read_additional_source_hdr(char const* buf)
{
    CommonSEGD::GeneralHeaderAdd& ghA = common.general_header_add;
    ghA.time = read_u64(&buf);
    ghA.source_status = *buf++;
    ghA.source_id = *buf++;
    ghA.source_moving = *buf++;
    memcpy(ghA.error_description, buf, sizeof(ghA.error_description));
    buf += sizeof(ghA.error_description);
    ghA.gen_hdr_block_type = *buf++;
}

void ISEGD::Impl::read_source_aux_hdr(char const* buf)
{
    CommonSEGD::GeneralHeaderSaux& ghS = common.general_header_saux;
    ghS.source_id = *buf++;
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
    ghS.gen_hdr_block_type = *buf++;
}

void ISEGD::Impl::read_crs_hdr(char const* buf)
{
    CommonSEGD::GeneralHeaderCoord& ghC = common.general_header_coord;
    memcpy(ghC.crs, buf, sizeof(ghC.crs));
    buf += sizeof(ghC.crs);
    ghC.gen_hdr_block_type = *buf++;
}

void ISEGD::Impl::read_position1_hdr(char const* buf)
{
    CommonSEGD::GeneralHeaderPos1& ghP = common.general_header_pos1;
    ghP.time_of_position = read_u64(&buf);
    ghP.time_of_measurement = read_u64(&buf);
    ghP.vert_error = read_u32(&buf);
    ghP.hor_error_semi_major = read_u32(&buf);
    ghP.hor_error_semi_minor = read_u32(&buf);
    ghP.hor_error_orientation = read_u16(&buf);
    ghP.position_type = *buf++;
    ghP.gen_hdr_block_type = *buf++;
}

void ISEGD::Impl::read_position2_hdr(char const* buf)
{
    CommonSEGD::GeneralHeaderPos2& ghP = common.general_header_pos2;
    ghP.crs_a_coord1 = read_u64(&buf);
    ghP.crs_a_coord2 = read_u64(&buf);
    ghP.crs_a_coord3 = read_u64(&buf);
    ghP.crs_a_crsref = read_u16(&buf);
    ghP.pos1_valid = *buf++;
    ghP.pos1_quality = *buf++;
    buf += 3;
    ghP.gen_hdr_block_type = *buf++;
}

void ISEGD::Impl::read_position3_hdr(char const* buf)
{
    CommonSEGD::GeneralHeaderPos3& ghP = common.general_header_pos3;
    ghP.crs_b_coord1 = read_u64(&buf);
    ghP.crs_b_coord2 = read_u64(&buf);
    ghP.crs_b_coord3 = read_u64(&buf);
    ghP.crs_b_crsref = read_u16(&buf);
    ghP.pos2_valid = *buf++;
    ghP.pos2_quality = *buf++;
    buf += 3;
    ghP.gen_hdr_block_type = *buf++;
}

void ISEGD::Impl::read_relative_position_hdr(char const* buf)
{
    CommonSEGD::GeneralHeaderRel& ghR = common.general_header_rel;
    ghR.offset_east = read_u32(&buf);
    ghR.offset_north = read_u32(&buf);
    ghR.offset_vert = read_u32(&buf);
    memcpy(ghR.description, buf, sizeof(ghR.description));
    buf += sizeof(ghR.description);
    ghR.gen_hdr_block_type = *buf++;
}

void ISEGD::Impl::read_sensor_info_hdr(char const* buf)
{
    CommonSEGD::GeneralHeaderSen& ghS = common.general_header_sen;
    ghS.instrument_test_time = read_u64(&buf);
    ghS.sensor_sensitivity = read_u32(&buf);
    ghS.instr_test_result = *buf++;
    memcpy(ghS.serial_number, buf, sizeof(ghS.serial_number));
    buf += sizeof(ghS.serial_number);
    ghS.gen_hdr_block_type = *buf++;
}

void ISEGD::Impl::read_sensor_calibration_hdr(char const* buf)
{
    CommonSEGD::GeneralHeaderSCa& ghS = common.general_header_sca;
    ghS.freq1 = read_u32(&buf);
    ghS.amp1 = read_u32(&buf);
    ghS.phase1 = read_u32(&buf);
    ghS.freq2 = read_u32(&buf);
    ghS.amp2 = read_u32(&buf);
    ghS.phase2 = read_u32(&buf);
    ghS.calib_applied = *buf++;
    buf += 6;
    ghS.gen_hdr_block_type = *buf++;
}

void ISEGD::Impl::read_time_drift_hdr(char const* buf)
{
    CommonSEGD::GeneralHeaderTim& ghT = common.general_header_tim;
    ghT.time_of_depl = read_u64(&buf);
    ghT.time_of_retr = read_u64(&buf);
    ghT.timer_offset_depl = read_u32(&buf);
    ghT.time_offset_retr = read_u32(&buf);
    ghT.timedrift_corr = *buf++;
    ghT.corr_method = *buf++;
    buf += 5;
    ghT.gen_hdr_block_type = *buf++;
}

void ISEGD::Impl::read_electromagnetic_src_recv_hdr(char const* buf)
{
    CommonSEGD::GeneralHeaderElSR& ghE = common.general_header_elsr;
    ghE.equip_dim_x = read_u24(&buf);
    ghE.equip_dim_y = read_u24(&buf);
    ghE.equip_dim_z = read_u24(&buf);
    ghE.pos_term = *buf++;
    ghE.equip_offset_x = read_u24(&buf);
    ghE.equip_offset_y = read_u24(&buf);
    ghE.equip_offset_z = read_u24(&buf);
    buf += 12;
    ghE.gen_hdr_block_type = *buf++;
}

void ISEGD::Impl::read_orientation_hdr(char const* buf)
{
    CommonSEGD::GeneralHeaderOri& ghO = common.general_header_ori;
    ghO.rot_x = read_u32(&buf);
    ghO.rot_y = read_u32(&buf);
    ghO.rot_z = read_u32(&buf);
    ghO.ref_orientation = read_u32(&buf);
    ghO.time_stamp = read_u64(&buf);
    ghO.ori_type = *buf++;
    ghO.ref_orient_valid = *buf++;
    ghO.rot_applied = *buf++;
    ghO.rot_north_applied = *buf++;
    buf += 3;
    ghO.gen_hdr_block_type = *buf++;
}

void ISEGD::Impl::read_measurement_hdr(char const* buf)
{
    CommonSEGD::GeneralHeaderMeas& ghM = common.general_header_meas;
    ghM.timestamp = read_u64(&buf);
    ghM.measurement_value = read_u32(&buf);
    ghM.maximum_value = read_u32(&buf);
    ghM.minimum_value = read_u32(&buf);
    ghM.quantity_class = read_u16(&buf);
    ghM.unit_of_measure = read_u16(&buf);
    ghM.measurement_description = read_u16(&buf);
    buf += 5;
    ghM.gen_hdr_block_type = *buf++;
}

void ISEGD::Impl::assign_sample_reading()
{
    switch (common.general_header.format_code) {
    case 8015:
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
        read_sample = [](char const** buf) {
            int sign = **buf & 0b10000000 ? -1 : 1;
            int exp = **buf & 0b01110000;
            return sign * (**buf++ & 0xf) * pow(2, exp - 4);
        };
        break;
    case 8024:
        read_sample = [](char const** buf) {
            int sign = **buf & 0b10000000 ? -1 : 1;
            int exp = **buf & 0b01110000;
            uint16_t frac = **buf++ & 0xf;
            frac = (frac << 8) & **buf++;
            return sign * frac * pow(2, exp - 12);
        };
        break;
    case 8036:
        read_sample = [this](char const** buf) { return read_i24(buf); };
        break;
    case 8038:
        read_sample = [this](char const** buf) { return read_i32(buf); };
        break;
    case 8042:
        read_sample = [](char const** buf) {
            int sign = **buf & 0b10000000 ? -1 : 1;
            int exp = **buf & 0b01100000;
            return sign * (**buf++ & 0x1f) / pow(2, 5) * pow(16, exp);
        };
        break;
    case 8044:
        read_sample = [](char const** buf) {
            int sign = **buf & 0b10000000 ? -1 : 1;
            int exp = **buf & 0b01100000;
            uint16_t frac = **buf++ & 0x1f;
            frac = (frac << 8) & **buf++;
            return sign * frac / pow(2, 13) * pow(16, exp);
        };
        break;
    case 8048:
        read_sample = [this](char const** buf) {
            int sign = **buf & 0b10000000 ? -1 : 1;
            int exp = **buf++ & 0b01111111;
            return sign * read_u24(buf) / pow(2, 24) * pow(16, exp);
        };
        break;
    case 8058:
        read_sample = [this](char const** buf) {
            uint32_t tmp = read_u32(buf);
            float result;
            memcpy(&result, &tmp, sizeof(tmp));
            return result;
        };
        break;
    case 8080:
        read_sample = [this](char const** buf) {
            uint64_t tmp = read_u64(buf);
            double result;
            memcpy(&result, &tmp, sizeof(tmp));
            return result;
        };
        break;
    case 9036:
        if (is_big_endian())
            read_sample = [this](char const** buf) { return swap(read_i24(buf)); };
        else
            read_sample = [](char const** buf) {
                uint32_t result = read<uint16_t>(buf) << 8 | read<uint8_t>(buf);
                return result & 0x800000 ? result | 0xff000000 : result;
            };
        break;
    case 9038:
        if (is_big_endian())
            read_sample = [this](char const** buf) { return swap(read_i32(buf)); };
        else
            read_sample = [](char const** buf) { return read<int32_t>(buf); };
        break;
    case 9058:
        if (is_big_endian())
            read_sample = [this](char const** buf) {
                uint32_t tmp = read_u32(buf);
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
        if (is_big_endian())
            read_sample = [this](char const** buf) {
                uint64_t tmp = read_u64(buf);
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
    if (!common.general_header.add_gen_hdr_blocks || common.general_header2.segd_rev_major < 3)
        common.ch_set_hdr_buf.resize(CommonSEGD::CH_SET_HDR_SIZE);
    else
        common.ch_set_hdr_buf.resize(CommonSEGD::CH_SET_HDR_R3_SIZE);
    fill_buf_from_file(common.ch_set_hdr_buf.data(), common.ch_set_hdr_buf.size());
    return CommonSEGD::ChannelSetHeader(common);
}

Trace::Header ISEGD::Impl::read_header()
{
    unordered_map<string, Trace::Header::Value> hdr;
    fill_buf_from_file(common.trc_hdr_buf.data(), common.trc_hdr_buf.size());
    char const* buf = common.trc_hdr_buf.data();
    hdr["FFID"] = from_bcd<int32_t>(&buf, false, 4);
    hdr["SCAN_TYPE_NUM"] = from_bcd<int16_t>(&buf, false, 2);
    hdr["CH_SET_NUM"] = from_bcd<int16_t>(&buf, false, 2);
    hdr["TRACE_NUMBER"] = from_bcd<int32_t>(&buf, false, 4);
    hdr["FIRST_TIMING_WORD"] = read_u24(&buf) / pow(2, 8);
    int tr_hdr_ext = *buf++;
    hdr["TR_HDR_EXT"] = tr_hdr_ext;
    hdr["SAMPLE_SKEW"] = *buf++ / pow(2, 8);
    hdr["TRACE_EDIT"] = *buf++;
    hdr["TIME_BREAK_WIN"] = read_u24(&buf) / pow(2, 8);
    uint16_t ext_ch_set_num = read_u16(&buf);
    if (ext_ch_set_num)
        hdr["CH_SET_NUM"] = ext_ch_set_num;
    uint32_t ext_file_num = read_u24(&buf);
    if (ext_file_num)
        hdr["FFID"] = ext_file_num;
    if (tr_hdr_ext || (common.general_header.add_gen_hdr_blocks && common.general_header2.segd_rev_major > 2)) {
        fill_buf_from_file(common.gen_hdr_buf.data(), common.gen_hdr_buf.size());
        hdr["R_LINE"] = read_u24(&buf);
        hdr["R_POINT"] = read_u24(&buf);
        hdr["R_POINT_IDX"] = *buf++;
        hdr["RESHOOT_IDX"] = *buf++;
        hdr["GROUP_IDX"] = *buf++;
        hdr["DEPTH_IDX"] = *buf++;
        double ext_r_line = read_u24(&buf);
        ext_r_line /= read_u16(&buf) / pow(2, 16);
        if (ext_r_line != 0.0)
            hdr["R_LINE"] = ext_r_line;
        double ext_r_point = read_u24(&buf);
        ext_r_point /= read_u16(&buf) / pow(2, 16);
        if (ext_r_point != 0.0)
            hdr["R_POINT"] = ext_r_point;
        hdr["SENSOR_TYPE"] = *buf++;
        uint32_t ext_tr_num = read_u24(&buf);
        if (ext_tr_num)
            hdr["TRACE_NUMBER"] = read_u24(&buf);
        hdr["SAMP_NUM"] = read_u32(&buf);
        hdr["SENSOR_MOVING"] = *buf++;
        ++buf;
        hdr["PHYSICAL_UNIT"] = *buf++;
        --tr_hdr_ext;
    }
    file_skip_bytes(tr_hdr_ext * CommonSEGD::TRACE_HEADER_EXT_SIZE);
    return Trace::Header(move(hdr));
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

bool is_big_endian(void)
{
    union {
        uint32_t i;
        char c[4];
    } bint = { 0x01020304 };

    return bint.c[0] == 1;
}

CommonSEGD::GeneralHeader ISEGD::general_header()
{
    return pimpl->common.general_header;
}
optional<CommonSEGD::GeneralHeader2> ISEGD::general_header2()
{
    return pimpl->common.general_header.add_gen_hdr_blocks ? optional<CommonSEGD::GeneralHeader2>(pimpl->common.general_header2) : std::nullopt;
}
optional<CommonSEGD::GeneralHeaderN> ISEGD::general_headerN()
{
    return pimpl->common.general_header.add_gen_hdr_blocks > 1 && pimpl->common.general_header2.segd_rev_major < 3 ? optional<CommonSEGD::GeneralHeaderN>(pimpl->common.general_headerN) : std::nullopt;
}
optional<CommonSEGD::GeneralHeader3> ISEGD::general_header3()
{
    return pimpl->common.general_header.add_gen_hdr_blocks && pimpl->common.general_header2.segd_rev_major > 2 ? optional<CommonSEGD::GeneralHeader3>(pimpl->common.general_header3) : std::nullopt;
}
optional<CommonSEGD::GeneralHeaderVes> ISEGD::general_header_vessel_crew_id()
{
    return pimpl->add_gen_hdr_blks_map.find(Impl::VESSEL_CREW_ID) != pimpl->add_gen_hdr_blks_map.end() ? optional<CommonSEGD::GeneralHeaderVes>(pimpl->common.general_header_ves) : std::nullopt;
}
optional<CommonSEGD::GeneralHeaderSur> ISEGD::general_header_survey_name()
{
    return pimpl->add_gen_hdr_blks_map.find(Impl::SURVEY_AREA_NAME) != pimpl->add_gen_hdr_blks_map.end() ? optional<CommonSEGD::GeneralHeaderSur>(pimpl->common.general_header_sur) : std::nullopt;
}
optional<CommonSEGD::GeneralHeaderCli> ISEGD::general_header_client_name()
{
    return pimpl->add_gen_hdr_blks_map.find(Impl::CLIENT_NAME) != pimpl->add_gen_hdr_blks_map.end() ? optional<CommonSEGD::GeneralHeaderCli>(pimpl->common.general_header_cli) : std::nullopt;
}
optional<CommonSEGD::GeneralHeaderJob> ISEGD::general_header_job_id()
{
    return pimpl->add_gen_hdr_blks_map.find(Impl::JOB_ID) != pimpl->add_gen_hdr_blks_map.end() ? optional<CommonSEGD::GeneralHeaderJob>(pimpl->common.general_header_job) : std::nullopt;
}
optional<CommonSEGD::GeneralHeaderLin> ISEGD::general_header_line_id()
{
    return pimpl->add_gen_hdr_blks_map.find(Impl::LINE_ID) != pimpl->add_gen_hdr_blks_map.end() ? optional<CommonSEGD::GeneralHeaderLin>(pimpl->common.general_header_lin) : std::nullopt;
}
optional<CommonSEGD::GeneralHeaderVib> ISEGD::general_header_vibrator_info()
{
    return pimpl->add_gen_hdr_blks_map.find(Impl::VIBRATOR_SOURCE_INFO) != pimpl->add_gen_hdr_blks_map.end() ? optional<CommonSEGD::GeneralHeaderVib>(pimpl->common.general_header_vib) : std::nullopt;
}
optional<CommonSEGD::GeneralHeaderExp> ISEGD::general_header_explosive_info()
{
    return pimpl->add_gen_hdr_blks_map.find(Impl::EXPLOSIVE_SOURCE_INFO) != pimpl->add_gen_hdr_blks_map.end() ? optional<CommonSEGD::GeneralHeaderExp>(pimpl->common.general_header_exp) : std::nullopt;
}
optional<CommonSEGD::GeneralHeaderAir> ISEGD::general_header_airgun_info()
{
    return pimpl->add_gen_hdr_blks_map.find(Impl::AIRGUN_SOURCE_INFO) != pimpl->add_gen_hdr_blks_map.end() ? optional<CommonSEGD::GeneralHeaderAir>(pimpl->common.general_header_air) : std::nullopt;
}
optional<CommonSEGD::GeneralHeaderWat> ISEGD::general_header_watergun_info()
{
    return pimpl->add_gen_hdr_blks_map.find(Impl::WATERGUN_SOURCE_INFO) != pimpl->add_gen_hdr_blks_map.end() ? optional<CommonSEGD::GeneralHeaderWat>(pimpl->common.general_header_wat) : std::nullopt;
}
optional<CommonSEGD::GeneralHeaderEle> ISEGD::general_header_electromagnetic_info()
{
    return pimpl->add_gen_hdr_blks_map.find(Impl::ELECTROMAGNETIC_SOURCE) != pimpl->add_gen_hdr_blks_map.end() ? optional<CommonSEGD::GeneralHeaderEle>(pimpl->common.general_header_ele) : std::nullopt;
}
optional<CommonSEGD::GeneralHeaderOth> ISEGD::general_header_other_source_info()
{
    return pimpl->add_gen_hdr_blks_map.find(Impl::OTHER_SOURCE_TYPE_INFO) != pimpl->add_gen_hdr_blks_map.end() ? optional<CommonSEGD::GeneralHeaderOth>(pimpl->common.general_header_oth) : std::nullopt;
}
optional<CommonSEGD::GeneralHeaderAdd> ISEGD::general_header_add_source_info()
{
    return pimpl->add_gen_hdr_blks_map.find(Impl::ADD_SOURCE_INFO) != pimpl->add_gen_hdr_blks_map.end() ? optional<CommonSEGD::GeneralHeaderAdd>(pimpl->common.general_header_add) : std::nullopt;
}
optional<CommonSEGD::GeneralHeaderSaux> ISEGD::general_header_sou_aux_chan_ref()
{
    return pimpl->add_gen_hdr_blks_map.find(Impl::SOU_AUX_CHAN_REF) != pimpl->add_gen_hdr_blks_map.end() ? optional<CommonSEGD::GeneralHeaderSaux>(pimpl->common.general_header_saux) : std::nullopt;
}
optional<CommonSEGD::GeneralHeaderSen> ISEGD::general_header_sen_info_hdr_ext_blk()
{
    return pimpl->add_gen_hdr_blks_map.find(Impl::SENSOR_INFO_HDR_EXT_BLK) != pimpl->add_gen_hdr_blks_map.end() ? optional<CommonSEGD::GeneralHeaderSen>(pimpl->common.general_header_sen) : std::nullopt;
}
optional<CommonSEGD::GeneralHeaderSCa> ISEGD::general_header_sen_calib_blk()
{
    return pimpl->add_gen_hdr_blks_map.find(Impl::SENSOR_CALIBRATION_BLK) != pimpl->add_gen_hdr_blks_map.end() ? optional<CommonSEGD::GeneralHeaderSCa>(pimpl->common.general_header_sca) : std::nullopt;
}
optional<CommonSEGD::GeneralHeaderTim> ISEGD::general_header_time_drift_blk()
{
    return pimpl->add_gen_hdr_blks_map.find(Impl::TIME_DRIFT_BLK) != pimpl->add_gen_hdr_blks_map.end() ? optional<CommonSEGD::GeneralHeaderTim>(pimpl->common.general_header_tim) : std::nullopt;
}
optional<CommonSEGD::GeneralHeaderElSR> ISEGD::general_header_elemag_src_rec_desc_blk()
{
    return pimpl->add_gen_hdr_blks_map.find(Impl::ELECTROMAG_SRC_REC_DESC_BLK) != pimpl->add_gen_hdr_blks_map.end() ? optional<CommonSEGD::GeneralHeaderElSR>(pimpl->common.general_header_elsr) : std::nullopt;
}
optional<CommonSEGD::GeneralHeaderPos1> ISEGD::general_header_position_blk_1()
{
    return pimpl->add_gen_hdr_blks_map.find(Impl::POSITION_BLK1) != pimpl->add_gen_hdr_blks_map.end() ? optional<CommonSEGD::GeneralHeaderPos1>(pimpl->common.general_header_pos1) : std::nullopt;
}
optional<CommonSEGD::GeneralHeaderPos2> ISEGD::general_header_position_blk_2()
{
    return pimpl->add_gen_hdr_blks_map.find(Impl::POSITION_BLK2) != pimpl->add_gen_hdr_blks_map.end() ? optional<CommonSEGD::GeneralHeaderPos2>(pimpl->common.general_header_pos2) : std::nullopt;
}
optional<CommonSEGD::GeneralHeaderPos3> ISEGD::general_header_position_blk_3()
{
    return pimpl->add_gen_hdr_blks_map.find(Impl::POSITION_BLK3) != pimpl->add_gen_hdr_blks_map.end() ? optional<CommonSEGD::GeneralHeaderPos3>(pimpl->common.general_header_pos3) : std::nullopt;
}
optional<CommonSEGD::GeneralHeaderCoord> ISEGD::general_header_coord_ref_blk()
{
    return pimpl->add_gen_hdr_blks_map.find(Impl::COORD_REF_SYSTEM) != pimpl->add_gen_hdr_blks_map.end() ? optional<CommonSEGD::GeneralHeaderCoord>(pimpl->common.general_header_coord) : std::nullopt;
}
optional<CommonSEGD::GeneralHeaderRel> ISEGD::general_header_relative_pos_blk()
{
    return pimpl->add_gen_hdr_blks_map.find(Impl::RELATIVE_POS_BLK) != pimpl->add_gen_hdr_blks_map.end() ? optional<CommonSEGD::GeneralHeaderRel>(pimpl->common.general_header_rel) : std::nullopt;
}
optional<CommonSEGD::GeneralHeaderOri> ISEGD::general_header_orient_hdr_blk()
{
    return pimpl->add_gen_hdr_blks_map.find(Impl::ORIENT_HDR_BLK) != pimpl->add_gen_hdr_blks_map.end() ? optional<CommonSEGD::GeneralHeaderOri>(pimpl->common.general_header_ori) : std::nullopt;
}
optional<CommonSEGD::GeneralHeaderMeas> ISEGD::general_header_measurement_blk()
{
    return pimpl->add_gen_hdr_blks_map.find(Impl::MEASUREMENT_BLK) != pimpl->add_gen_hdr_blks_map.end() ? optional<CommonSEGD::GeneralHeaderMeas>(pimpl->common.general_header_meas) : std::nullopt;
}

vector<vector<CommonSEGD::ChannelSetHeader>> const& ISEGD::channel_set_headers()
{
    return pimpl->common.ch_sets;
}

ISEGD::ISEGD(string name)
    : pimpl(make_unique<Impl>(CommonSEGD(move(name), fstream::in | fstream::binary)))
{
}

ISEGD::~ISEGD() = default;
} //sedaman
