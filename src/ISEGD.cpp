#include "ISEGD.hpp"
#include "util.hpp"
#include <cmath>
#include <cstring>
#include <functional>
#include <unordered_map>

using std::fstream;
using std::function;
using std::ios_base;
using std::make_unique;
using std::move;
using std::optional;
using std::streamoff;
using std::streampos;
using std::streamsize;
using std::string;
using std::unordered_map;

namespace sedaman {
class ISEGD::Impl {
public:
    Impl(ISEGD& s);
    ISEGD& sgd;
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
    ChannelSetHeader read_ch_set_hdr();
    void fill_buf_from_file(char* buf, streamsize n);
    void file_skip_bytes(streamoff off);
    function<uint16_t(char const**)> read_u16;
    function<int16_t(char const**)> read_i16;
    function<uint32_t(char const**)> read_u24;
    function<int32_t(char const**)> read_i24;
    function<uint32_t(char const**)> read_u32;
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
};

ISEGD::Impl::Impl(ISEGD& s)
    : sgd(s)
    , read_u16([](char const** buf) { return swap(read<uint16_t>(buf)); })
    , read_i16([](char const** buf) { return swap(read<int16_t>(buf)); })
    , read_u24([](char const** buf) { return swap(read<uint16_t>(buf)) | read<uint8_t>(buf) << 16; })
    , read_i24([](char const** buf) {
        uint32_t result = swap(read<uint16_t>(buf)) | read<uint8_t>(buf) << 16;
        return result & 0x800000 ? result | 0xff000000 : result;
    })
    , read_u32([](char const** buf) { return swap(read<uint32_t>(buf)); })
    , read_u64([](char const** buf) { return swap(read<uint64_t>(buf)); })
{
    sgd.p_file().seekg(0, ios_base::end);
    end_of_data = sgd.p_file().tellg();
    sgd.p_file().seekg(0, ios_base::beg);
    curr_pos = sgd.p_file().tellg();
    //while (curr_pos != end_of_data) {
    read_general_headers();
    // read header for each scan type
    // uint16_t ch_sets_per_scan_type_num = sgd.p_general_header().channel_sets_per_scan_type == 0xff ? sgd.p_general_header2().ext_ch_sets_per_scan_type : sgd.p_general_header().channel_sets_per_scan_type;
    //    for (int i = sgd.p_general_header().scan_types_per_record; i; --i) {
    //        sgd.p_ch_sets().push_back({});
    //        for (int j = ch_sets_per_scan_type_num; j; --j) {
    //            sgd.p_ch_sets()[i].push_back(read_ch_set_hdr());
    //        }
    //    }
    //}
}

void ISEGD::Impl::read_general_headers()
{
    fill_buf_from_file(sgd.p_gen_hdr_buf().data(), sgd.p_gen_hdr_buf().size());
    char const* buf = sgd.p_gen_hdr_buf().data();
    read_gen_hdr1(buf);
    if (sgd.general_header().add_gen_hdr_blocks) {
        fill_buf_from_file(sgd.p_gen_hdr_buf().data(), sgd.p_gen_hdr_buf().size());
        char const* buf = sgd.p_gen_hdr_buf().data();
        GeneralHeader2& gh2 = sgd.p_general_header2();
        buf += 10;
        gh2.segd_rev_major = *buf++;
        gh2.segd_rev_minor = *buf++;
        buf = sgd.p_gen_hdr_buf().data();
        gh2.expanded_file_num = read_u24(&buf);
        gh2.ext_ch_sets_per_scan_type = read_u16(&buf);
        if (gh2.segd_rev_major < 3) {
            read_gen_hdr2(buf);
            file_skip_bytes(CommonSEGD::GEN_HDR_SIZE * (sgd.p_general_header().add_gen_hdr_blocks - 1));
        } else {
            read_gen_hdr2_and_3(buf);
            uint16_t add_blks_num = sgd.p_general_header().add_gen_hdr_blocks == 0xf ? gh2.ext_num_add_blks_in_gen_hdr : sgd.p_general_header().add_gen_hdr_blocks;
            for (uint16_t i = add_blks_num - 2; i; --i) {
                fill_buf_from_file(sgd.p_gen_hdr_buf().data(), sgd.p_gen_hdr_buf().size());
                char const* buf = sgd.p_gen_hdr_buf().data();
                read_rev3_add_gen_hdr_blks(buf);
            }
        }
    }
}

void ISEGD::Impl::read_gen_hdr1(char const* buf)
{
    GeneralHeader& gh = sgd.p_general_header();
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
    GeneralHeader2& gh2 = sgd.p_general_header2();
    gh2.segd_rev_minor = gh2.segd_rev_minor / pow(2, 8) * 10;
    gh2.extended_hdr_blocks = read_u16(&buf);
    gh2.external_hdr_blocks = read_u16(&buf);
    buf += 3;
    gh2.gen_trailer_num_of_blocks = read_u16(&buf);
    gh2.ext_record_len = read_u24(&buf);
    gh2.gen_hdr_block_num = *++buf;
    if (sgd.general_header().add_gen_hdr_blocks > 1) {
        fill_buf_from_file(sgd.p_gen_hdr_buf().data(), sgd.p_gen_hdr_buf().size());
        char const* buf = sgd.p_gen_hdr_buf().data();
        GeneralHeaderN& ghN = sgd.p_general_headerN();
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
        file_skip_bytes(CommonSEGD::GEN_HDR_SIZE * (sgd.general_header().add_gen_hdr_blocks - 2));
    }
}

void ISEGD::Impl::read_gen_hdr2_and_3(char const* buf)
{
    GeneralHeader2& gh2 = sgd.p_general_header2();
    gh2.extended_hdr_blocks = read_u24(&buf);
    gh2.extended_skew_blocks = read_u16(&buf);
    buf += 2;
    gh2.gen_trailer_num_of_blocks = read_u32(&buf);
    gh2.record_set_number = read_u16(&buf);
    gh2.ext_num_add_blks_in_gen_hdr = read_u16(&buf);
    gh2.dominant_sampling_int = read_u16(&buf);
    gh2.external_hdr_blocks = read_u24(&buf);
    gh2.gen_hdr_block_num = *++buf;
    fill_buf_from_file(sgd.p_gen_hdr_buf().data(), sgd.p_gen_hdr_buf().size());
    buf = sgd.p_gen_hdr_buf().data();
    GeneralHeader3& gh3 = sgd.p_general_header3();
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
    GeneralHeaderVes& ghv = sgd.p_general_header_ves();
    memcpy(ghv.abbr_vessel_crew_name, buf, sizeof(ghv.abbr_vessel_crew_name));
    buf += sizeof(ghv.abbr_vessel_crew_name);
    memcpy(ghv.vessel_crew_name, buf, sizeof(ghv.vessel_crew_name));
    buf += sizeof(ghv.vessel_crew_name);
    ghv.gen_hdr_block_type = *buf;
}

void ISEGD::Impl::read_survey_area_name(const char* buf)
{
    GeneralHeaderSur& ghs = sgd.p_general_header_sur();
    memcpy(ghs.survey_area_name, buf, sizeof(ghs.survey_area_name));
    buf += sizeof(ghs.survey_area_name);
    ghs.gen_hdr_block_type = *buf;
}

void ISEGD::Impl::read_client_name(const char* buf)
{
    GeneralHeaderCli& ghc = sgd.p_general_header_cli();
    memcpy(ghc.client_name, buf, sizeof(ghc.client_name));
    buf += sizeof(ghc.client_name);
    ghc.gen_hdr_block_type = *buf;
}

void ISEGD::Impl::read_job_id(char const* buf)
{
    GeneralHeaderJob& ghj = sgd.p_general_header_job();
    memcpy(ghj.abbr_job_id, buf, sizeof(ghj.abbr_job_id));
    buf += sizeof(ghj.abbr_job_id);
    memcpy(ghj.job_id, buf, sizeof(ghj.job_id));
    buf += sizeof(ghj.job_id);
    ghj.gen_hdr_block_type = *buf;
}

void ISEGD::Impl::read_line_id(char const* buf)
{
    GeneralHeaderLin& ghl = sgd.p_general_header_lin();
    memcpy(ghl.line_abbr, buf, sizeof(ghl.line_abbr));
    buf += sizeof(ghl.line_abbr);
    memcpy(ghl.line_id, buf, sizeof(ghl.line_id));
    buf += sizeof(ghl.line_id);
    ghl.gen_hdr_block_type = *buf;
}

void ISEGD::Impl::read_vibrator_hdr(char const* buf)
{
    GeneralHeaderVib& ghV = sgd.p_general_header_vib();
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
    GeneralHeaderExp& ghE = sgd.p_general_header_exp();
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
    GeneralHeaderAir& ghA = sgd.p_general_header_air();
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
    GeneralHeaderAir& ghA = sgd.p_general_header_air();
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
    GeneralHeaderEle& ghE = sgd.p_general_header_ele();
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
    GeneralHeaderOth& ghO = sgd.p_general_header_oth();
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
    GeneralHeaderAdd& ghA = sgd.p_general_header_add();
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
    GeneralHeaderSaux& ghS = sgd.p_general_header_saux();
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
    GeneralHeaderCoord& ghC = sgd.p_general_header_coord();
    memcpy(ghC.crs, buf, sizeof(ghC.crs));
    buf += sizeof(ghC.crs);
    ghC.gen_hdr_block_type = *buf++;
}

void ISEGD::Impl::read_position1_hdr(char const* buf)
{
    GeneralHeaderPos1& ghP = sgd.p_general_header_pos1();
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
    GeneralHeaderPos2& ghP = sgd.p_general_header_pos2();
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
    GeneralHeaderPos3& ghP = sgd.p_general_header_pos3();
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
    GeneralHeaderRel& ghR = sgd.p_general_header_rel();
    ghR.offset_east = read_u32(&buf);
    ghR.offset_north = read_u32(&buf);
    ghR.offset_vert = read_u32(&buf);
    memcpy(ghR.description, buf, sizeof(ghR.description));
    buf += sizeof(ghR.description);
    ghR.gen_hdr_block_type = *buf++;
}

void ISEGD::Impl::read_sensor_info_hdr(char const* buf)
{
    GeneralHeaderSen& ghS = sgd.p_general_header_sen();
    ghS.instrument_test_time = read_u64(&buf);
    ghS.sensor_sensitivity = read_u32(&buf);
    ghS.instr_test_result = *buf++;
    memcpy(ghS.serial_number, buf, sizeof(ghS.serial_number));
    buf += sizeof(ghS.serial_number);
    ghS.gen_hdr_block_type = *buf++;
}

void ISEGD::Impl::read_sensor_calibration_hdr(char const* buf)
{
    GeneralHeaderSCa& ghS = sgd.p_general_header_sca();
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
    GeneralHeaderTim& ghT = sgd.p_general_header_tim();
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
    GeneralHeaderElm& ghE = sgd.p_general_header_elm();
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
    GeneralHeaderOri& ghO = sgd.p_general_header_ori();
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
    GeneralHeaderMeas& ghM = sgd.p_general_header_meas();
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

CommonSEGD::ChannelSetHeader ISEGD::Impl::read_ch_set_hdr()
{
    fill_buf_from_file(sgd.p_gen_hdr_buf().data(), sgd.p_gen_hdr_buf().size());
    char const* buf = sgd.p_gen_hdr_buf().data();
    ChannelSetHeader csh = {};
    csh.scan_type_number = from_bcd<int>(&buf, false, 2);
    csh.channel_set_number = from_bcd<int>(&buf, false, 2);
    csh.channel_set_start_time = read_u16(&buf) * 2;
    csh.channel_set_end_time = read_u16(&buf) * 2;
    csh.descale_multiplier = swap(read_u16(&buf)) / pow(2, 10);
    csh.number_of_channels = from_bcd<int>(&buf, false, 4);
    csh.channel_type = from_bcd<int>(&buf, false, 1);
    ++buf;
    csh.samples_per_channel = from_bcd<int>(&buf, false, 1);
    csh.channel_gain = from_bcd<int>(&buf, true, 1);
    csh.alias_filter_freq = from_bcd<int>(&buf, false, 4);
    csh.alias_filter_slope = from_bcd<int>(&buf, false, 4);
    csh.low_cut_filter_freq = from_bcd<int>(&buf, false, 4);
    csh.low_cut_filter_slope = from_bcd<int>(&buf, false, 4);
    csh.first_notch_filter = from_bcd<int>(&buf, false, 4);
    csh.second_notch_filter = from_bcd<int>(&buf, false, 4);
    csh.third_notch_filter = from_bcd<int>(&buf, false, 4);
    return csh;
}

void ISEGD::Impl::fill_buf_from_file(char* buf, streamsize n)
{
    sgd.p_file().read(buf, n);
    curr_pos = sgd.p_file().tellg();
}

void ISEGD::Impl::file_skip_bytes(streamoff off)
{
    sgd.p_file().seekg(off, ios_base::cur);
    curr_pos = sgd.p_file().tellg();
}

CommonSEGD::GeneralHeader ISEGD::general_header()
{
    return p_general_header();
}
optional<CommonSEGD::GeneralHeader2> ISEGD::general_header2()
{
    return p_general_header().add_gen_hdr_blocks ? optional<GeneralHeader2>(p_general_header2()) : std::nullopt;
}
optional<CommonSEGD::GeneralHeaderN> ISEGD::general_headerN()
{
    return p_general_header().add_gen_hdr_blocks > 1 && p_general_header2().segd_rev_major < 3 ? optional<GeneralHeaderN>(p_general_headerN()) : std::nullopt;
}
optional<CommonSEGD::GeneralHeader3> ISEGD::general_header3()
{
    return p_general_header2().segd_rev_major > 2 ? optional<GeneralHeader3>(p_general_header3()) : std::nullopt;
}
optional<CommonSEGD::GeneralHeaderVes> ISEGD::general_header_ves()
{
    return pimpl->add_gen_hdr_blks_map.find(Impl::VESSEL_CREW_ID) != pimpl->add_gen_hdr_blks_map.end() ? optional<GeneralHeaderVes>(p_general_header_ves()) : std::nullopt;
}

ISEGD::ISEGD(string name)
    : CommonSEGD(move(name), fstream::in)
    , pimpl(make_unique<Impl>(*this))
{
}

ISEGD::~ISEGD() = default;
} //sedaman
