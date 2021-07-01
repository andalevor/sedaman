#include "OSEGD.hpp"
#include "Exception.hpp"
#include "util.hpp"
#include <functional>
#include <type_traits>

using std::abs;
using std::endian;
using std::fstream;
using std::function;
using std::get;
using std::make_unique;
using std::map;
using std::move;
using std::optional;
using std::string;
using std::unique_ptr;
using std::valarray;
using std::vector;

namespace sedaman {
class OSEGD::Impl {
public:
    uint64_t chans_in_record;
    void assign_raw_writers();
    void assign_sample_writers();
    void write_general_header1();
    void write_general_header2_rev2();
    void write_rev2_add_gen_hdrs();
    void write_general_header2_and_3();
    void write_rev3_add_gen_hdrs();
    void write_ch_set_hdr(CommonSEGD::ChannelSetHeader& hdr);
    void write_trace_header(Trace::Header const& hdr);
    void write_ext_trace_header(Trace::Header const& hdr);
    void write_trace_samples(Trace const& trc);
    Impl(CommonSEGD com)
        : common { move(com) }
    {
    }
    CommonSEGD common;

private:
    function<void(char**, double)> write_sample;
    function<void(char**, uint16_t)> write_u16;
    function<void(char**, uint16_t)> write_i16;
    function<void(char**, uint32_t)> write_u24;
    function<void(char**, uint32_t)> write_i24;
    function<void(char**, uint32_t)> write_u32;
    function<void(char**, uint32_t)> write_i32;
    function<void(char**, uint32_t)> write_u64;
    void write_vessel_crew_id(char* buf, CommonSEGD::GeneralHeaderVes hdr);
    void write_survey_area_name(char* buf, CommonSEGD::GeneralHeaderSur hdr);
    void write_client_name(char* buf, CommonSEGD::GeneralHeaderCli hdr);
    void write_job_id(char* buf, CommonSEGD::GeneralHeaderJob hdr);
    void write_line_id(char* buf, CommonSEGD::GeneralHeaderLin hdr);
    void write_vibrator_hdr(char* buf, CommonSEGD::GeneralHeaderVib hdr);
    void write_explosive_hdr(char* buf, CommonSEGD::GeneralHeaderExp hdr);
    void write_airgun_hdr(char* buf, CommonSEGD::GeneralHeaderAir hdr);
    void write_watergun_hdr(char* buf, CommonSEGD::GeneralHeaderWat hdr);
    void write_electromagnetic_hdr(char* buf, CommonSEGD::GeneralHeaderEle hdr);
    void write_other_source_hdr(char* buf, CommonSEGD::GeneralHeaderOth hdr);
    void write_additional_source_hdr(char* buf, CommonSEGD::GeneralHeaderAdd hdr);
    void write_source_aux_hdr(char* buf, CommonSEGD::GeneralHeaderSaux hdr);
    void write_crs_hdr(char* buf, CommonSEGD::GeneralHeaderCoord hdr);
    void write_position1_hdr(char* buf, CommonSEGD::GeneralHeaderPos1 hdr);
    void write_position2_hdr(char* buf, CommonSEGD::GeneralHeaderPos2 hdr);
    void write_position3_hdr(char* buf, CommonSEGD::GeneralHeaderPos3 hdr);
    void write_relative_position_hdr(char* buf, CommonSEGD::GeneralHeaderRel hdr);
    void write_sensor_info_hdr(char* buf, CommonSEGD::GeneralHeaderSen hdr);
    void write_sensor_calibration_hdr(char* buf, CommonSEGD::GeneralHeaderSCa hdr);
    void write_time_drift_hdr(char* buf, CommonSEGD::GeneralHeaderTim hdr);
    void write_electromagnetic_src_recv_hdr(char* buf, CommonSEGD::GeneralHeaderElSR hdr);
    void write_orientation_hdr(char* buf, CommonSEGD::GeneralHeaderOri hdr);
    void write_measurement_hdr(char* buf, CommonSEGD::GeneralHeaderMeas hdr);
};

void OSEGD::Impl::assign_raw_writers()
{
    if (endian::native == endian::big) {
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
        write_u32 = [](char** buf, int32_t val) { write<int32_t>(buf, val); };
        write_u64 = [](char** buf, uint64_t val) { write<uint64_t>(buf, val); };
    } else {
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
    }
}

void OSEGD::Impl::write_general_header1()
{
    char* ptr = common.gen_hdr_buf;
    memset(ptr, 0, CommonSEGD::GEN_HDR_SIZE);
    common.general_header.file_number > 9999 ? write_u16(&ptr, 0xffff) : to_bcd(&ptr, common.general_header.file_number, false, 4);
    to_bcd(&ptr, common.general_header.format_code, false, 4);
    to_bcd(&ptr, common.general_header.gen_const, false, 12);
    to_bcd(&ptr, common.general_header.year, false, 2);
    *ptr = common.general_header.add_gen_hdr_blocks << 4;
    to_bcd(&ptr, common.general_header.day, true, 3);
    to_bcd(&ptr, common.general_header.hour, false, 2);
    to_bcd(&ptr, common.general_header.minute, false, 2);
    to_bcd(&ptr, common.general_header.second, false, 2);
    to_bcd(&ptr, common.general_header.manufac_code, false, 2);
    to_bcd(&ptr, common.general_header.manufac_num, false, 4);
    to_bcd(&ptr, common.general_header.bytes_per_scan, false, 6);
    *ptr++ = common.general_header.base_scan_int * pow(2, 4);
    to_bcd(&ptr, common.general_header.polarity, false, 1);
    int exp = static_cast<unsigned>(log2(common.general_header.scans_per_block)) & 0x0f;
    *ptr++ |= exp;
    *ptr++ = common.general_header.scans_per_block / pow(2, exp);
    to_bcd(&ptr, common.general_header.record_type, false, 1);
    if (common.general_header.add_gen_hdr_blocks && common.general_header2.ext_record_len)
        write_u16(&ptr, 0xfff0);
    else
        to_bcd(&ptr, common.general_header.record_length, true, 3);
    common.general_header.scan_types_per_record > 99 ? (void)(*ptr++ = static_cast<uint8_t>(0xff)) : to_bcd(&ptr, common.general_header.scan_types_per_record, false, 2);
    to_bcd(&ptr, common.general_header.channel_sets_per_scan_type, false, 2);
    to_bcd(&ptr, common.general_header.skew_blocks, false, 2);
    common.general_header.extended_hdr_blocks > 99 ? (void)(*ptr++ = static_cast<uint8_t>(0xff)) : to_bcd(&ptr, common.general_header.extended_hdr_blocks, false, 2);
    common.general_header.external_hdr_blocks > 99 ? (void)(*ptr++ = static_cast<uint8_t>(0xff)) : to_bcd(&ptr, common.general_header.external_hdr_blocks, false, 2);
    common.file.write(common.gen_hdr_buf, CommonSEGD::GEN_HDR_SIZE);
}

void OSEGD::Impl::write_general_header2_rev2()
{
    char* buf = common.gen_hdr_buf;
    memset(buf, 0, CommonSEGD::GEN_HDR_SIZE);
    write_u24(&buf, common.general_header2.expanded_file_num);
    write_u16(&buf, common.general_header2.ext_ch_sets_per_scan_type);
    write_u16(&buf, common.general_header2.extended_hdr_blocks);
    write_u16(&buf, common.general_header2.external_hdr_blocks);
    ++buf;
    *buf++ = common.general_header2.segd_rev_major;
    *buf++ = common.general_header2.segd_rev_minor;
    write_u16(&buf, common.general_header2.gen_trailer_num_of_blocks);
    write_u24(&buf, common.general_header2.ext_record_len);
    ++buf;
    *buf++ = common.general_header2.gen_hdr_block_num;
    ++buf;
    write_u16(&buf, common.general_header2.sequence_number);
    common.file.write(common.gen_hdr_buf, CommonSEGD::GEN_HDR_SIZE);
}

void OSEGD::Impl::write_rev2_add_gen_hdrs()
{
    for (map<CommonSEGD::AdditionalGeneralHeader::ADD_GEN_HDR_BLKS, unique_ptr<CommonSEGD::AdditionalGeneralHeader>>::iterator it = common.add_gen_hdr_blks_map.begin(),
                                                                                                                               end = common.add_gen_hdr_blks_map.end();
         it != end; ++it) {
        char* buf = common.gen_hdr_buf;
        memset(buf, 0, CommonSEGD::GEN_HDR_SIZE);
        write_u32(&buf, dynamic_cast<CommonSEGD::GeneralHeaderN*>(it->second.get())->expanded_file_number);
        uint32_t integer = dynamic_cast<CommonSEGD::GeneralHeaderN*>(it->second.get())->sou_line_num;
        double frac = dynamic_cast<CommonSEGD::GeneralHeaderN*>(it->second.get())->sou_line_num - integer;
        write_i24(&buf, integer);
        write_u16(&buf, frac * pow(2, 16));
        integer = dynamic_cast<CommonSEGD::GeneralHeaderN*>(it->second.get())->sou_point_num;
        frac = dynamic_cast<CommonSEGD::GeneralHeaderN*>(it->second.get())->sou_point_num - integer;
        write_i24(&buf, integer);
        write_u16(&buf, frac * pow(2, 16));
        *buf++ = dynamic_cast<CommonSEGD::GeneralHeaderN*>(it->second.get())->sou_point_index;
        *buf++ = dynamic_cast<CommonSEGD::GeneralHeaderN*>(it->second.get())->phase_control;
        *buf++ = dynamic_cast<CommonSEGD::GeneralHeaderN*>(it->second.get())->type_vibrator;
        write_i16(&buf, dynamic_cast<CommonSEGD::GeneralHeaderN*>(it->second.get())->phase_angle);
        *buf++ = dynamic_cast<CommonSEGD::GeneralHeaderN*>(it->second.get())->gen_hdr_block_num;
        *buf++ = dynamic_cast<CommonSEGD::GeneralHeaderN*>(it->second.get())->sou_set_num;
        common.file.write(common.gen_hdr_buf, CommonSEGD::GEN_HDR_SIZE);
    }
}

void OSEGD::Impl::write_general_header2_and_3()
{
    char* buf = common.gen_hdr_buf;
    memset(buf, 0, CommonSEGD::GEN_HDR_SIZE);
    write_u24(&buf, common.general_header2.expanded_file_num);
    write_u16(&buf, common.general_header2.ext_ch_sets_per_scan_type);
    write_u24(&buf, common.general_header2.extended_hdr_blocks);
    write_u16(&buf, common.general_header2.extended_skew_blocks);
    *buf++ = common.general_header2.segd_rev_major;
    *buf++ = common.general_header2.segd_rev_minor;
    write_u32(&buf, common.general_header2.gen_trailer_num_of_blocks);
    write_u32(&buf, common.general_header2.ext_record_len);
    write_u16(&buf, common.general_header2.record_set_number);
    write_u16(&buf, common.general_header2.ext_num_add_blks_in_gen_hdr);
    write_u24(&buf, common.general_header2.dominant_sampling_int);
    write_u24(&buf, common.general_header2.external_hdr_blocks);
    *++buf = common.general_header2.gen_hdr_block_num;
    common.file.write(common.gen_hdr_buf, CommonSEGD::GEN_HDR_SIZE);
    buf = common.gen_hdr_buf;
    memset(buf, 0, CommonSEGD::GEN_HDR_SIZE);
    write_u64(&buf, common.general_header3.time_zero);
    write_u64(&buf, common.general_header3.record_size);
    write_u64(&buf, common.general_header3.data_size);
    write_u32(&buf, common.general_header3.header_size);
    *buf++ = common.general_header3.extd_rec_mode;
    *buf++ = common.general_header3.rel_time_mode;
    *++buf = common.general_header3.gen_hdr_block_num;
}

void OSEGD::Impl::write_rev3_add_gen_hdrs()
{
    char* buf;
    for (map<CommonSEGD::AdditionalGeneralHeader::ADD_GEN_HDR_BLKS, unique_ptr<CommonSEGD::AdditionalGeneralHeader>>::iterator it = common.add_gen_hdr_blks_map.begin(),
                                                                                                                               end = common.add_gen_hdr_blks_map.end();
         it != end; ++it) {
        buf = common.gen_hdr_buf;
        memset(buf, 0, CommonSEGD::GEN_HDR_SIZE);
        switch (it->first) {
        case CommonSEGD::AdditionalGeneralHeader::VESSEL_CREW_ID:
            write_vessel_crew_id(buf, *dynamic_cast<CommonSEGD::GeneralHeaderVes*>(it->second.get()));
            break;
        case CommonSEGD::AdditionalGeneralHeader::SURVEY_AREA_NAME:
            write_survey_area_name(buf, *dynamic_cast<CommonSEGD::GeneralHeaderSur*>(it->second.get()));
            break;
        case CommonSEGD::AdditionalGeneralHeader::CLIENT_NAME:
            write_client_name(buf, *dynamic_cast<CommonSEGD::GeneralHeaderCli*>(it->second.get()));
            break;
        case CommonSEGD::AdditionalGeneralHeader::JOB_ID:
            write_job_id(buf, *dynamic_cast<CommonSEGD::GeneralHeaderJob*>(it->second.get()));
            break;
        case CommonSEGD::AdditionalGeneralHeader::LINE_ID:
            write_line_id(buf, *dynamic_cast<CommonSEGD::GeneralHeaderLin*>(it->second.get()));
            break;
        case CommonSEGD::AdditionalGeneralHeader::VIBRATOR_SOURCE_INFO:
            write_vibrator_hdr(buf, *dynamic_cast<CommonSEGD::GeneralHeaderVib*>(it->second.get()));
            break;
        case CommonSEGD::AdditionalGeneralHeader::EXPLOSIVE_SOURCE_INFO:
            write_explosive_hdr(buf, *dynamic_cast<CommonSEGD::GeneralHeaderExp*>(it->second.get()));
            break;
        case CommonSEGD::AdditionalGeneralHeader::AIRGUN_SOURCE_INFO:
            write_airgun_hdr(buf, *dynamic_cast<CommonSEGD::GeneralHeaderAir*>(it->second.get()));
            break;
        case CommonSEGD::AdditionalGeneralHeader::WATERGUN_SOURCE_INFO:
            write_watergun_hdr(buf, *dynamic_cast<CommonSEGD::GeneralHeaderWat*>(it->second.get()));
            break;
        case CommonSEGD::AdditionalGeneralHeader::ELECTROMAGNETIC_SOURCE:
            write_electromagnetic_hdr(buf, *dynamic_cast<CommonSEGD::GeneralHeaderEle*>(it->second.get()));
            break;
        case CommonSEGD::AdditionalGeneralHeader::OTHER_SOURCE_TYPE_INFO:
            write_other_source_hdr(buf, *dynamic_cast<CommonSEGD::GeneralHeaderOth*>(it->second.get()));
            break;
        case CommonSEGD::AdditionalGeneralHeader::ADD_SOURCE_INFO:
            write_additional_source_hdr(buf, *dynamic_cast<CommonSEGD::GeneralHeaderAdd*>(it->second.get()));
            break;
        case CommonSEGD::AdditionalGeneralHeader::SOU_AUX_CHAN_REF:
            write_source_aux_hdr(buf, *dynamic_cast<CommonSEGD::GeneralHeaderSaux*>(it->second.get()));
            break;
        case CommonSEGD::AdditionalGeneralHeader::COORD_REF_SYSTEM:
            write_crs_hdr(buf, *dynamic_cast<CommonSEGD::GeneralHeaderCoord*>(it->second.get()));
            break;
        case CommonSEGD::AdditionalGeneralHeader::POSITION_BLK1:
            write_position1_hdr(buf, *dynamic_cast<CommonSEGD::GeneralHeaderPos1*>(it->second.get()));
            break;
        case CommonSEGD::AdditionalGeneralHeader::POSITION_BLK2:
            write_position2_hdr(buf, *dynamic_cast<CommonSEGD::GeneralHeaderPos2*>(it->second.get()));
            break;
        case CommonSEGD::AdditionalGeneralHeader::POSITION_BLK3:
            write_position3_hdr(buf, *dynamic_cast<CommonSEGD::GeneralHeaderPos3*>(it->second.get()));
            break;
        case CommonSEGD::AdditionalGeneralHeader::RELATIVE_POS_BLK:
            write_relative_position_hdr(buf, *dynamic_cast<CommonSEGD::GeneralHeaderRel*>(it->second.get()));
            break;
        case CommonSEGD::AdditionalGeneralHeader::SENSOR_INFO_HDR_EXT_BLK:
            write_sensor_info_hdr(buf, *dynamic_cast<CommonSEGD::GeneralHeaderSen*>(it->second.get()));
            break;
        case CommonSEGD::AdditionalGeneralHeader::SENSOR_CALIBRATION_BLK:
            write_sensor_calibration_hdr(buf, *dynamic_cast<CommonSEGD::GeneralHeaderSCa*>(it->second.get()));
            break;
        case CommonSEGD::AdditionalGeneralHeader::TIME_DRIFT_BLK:
            write_time_drift_hdr(buf, *dynamic_cast<CommonSEGD::GeneralHeaderTim*>(it->second.get()));
            break;
        case CommonSEGD::AdditionalGeneralHeader::ELECTROMAG_SRC_REC_DESC_BLK:
            write_electromagnetic_src_recv_hdr(buf, *dynamic_cast<CommonSEGD::GeneralHeaderElSR*>(it->second.get()));
            break;
        case CommonSEGD::AdditionalGeneralHeader::ORIENT_HDR_BLK:
            write_orientation_hdr(buf, *dynamic_cast<CommonSEGD::GeneralHeaderOri*>(it->second.get()));
            break;
        case CommonSEGD::AdditionalGeneralHeader::MEASUREMENT_BLK:
            write_measurement_hdr(buf, *dynamic_cast<CommonSEGD::GeneralHeaderMeas*>(it->second.get()));
            break;
        default:
            break;
        }
        common.file.write(buf, CommonSEGD::GEN_HDR_SIZE);
    }
}

void OSEGD::Impl::write_vessel_crew_id(char* buf, CommonSEGD::GeneralHeaderVes hdr)
{
    size_t size = sizeof(hdr.abbr_vessel_crew_name);
    memcpy(buf, hdr.abbr_vessel_crew_name, size);
    buf += size;
    size = sizeof(hdr.vessel_crew_name);
    memcpy(buf, hdr.vessel_crew_name, size);
    buf += size;
    *buf = sizeof(hdr.gen_hdr_block_type);
}

void OSEGD::Impl::write_survey_area_name(char* buf, CommonSEGD::GeneralHeaderSur hdr)
{
    size_t size = sizeof(hdr.survey_area_name);
    memcpy(buf, hdr.survey_area_name, size);
    buf += size;
    *buf = sizeof(hdr.gen_hdr_block_type);
}

void OSEGD::Impl::write_client_name(char* buf, CommonSEGD::GeneralHeaderCli hdr)
{
    size_t size = sizeof(hdr.client_name);
    memcpy(buf, hdr.client_name, size);
    buf += size;
    *buf = sizeof(hdr.gen_hdr_block_type);
}

void OSEGD::Impl::write_job_id(char* buf, CommonSEGD::GeneralHeaderJob hdr)
{
    size_t size = sizeof(hdr.abbr_job_id);
    memcpy(buf, hdr.abbr_job_id, size);
    buf += size;
    size = sizeof(hdr.job_id);
    memcpy(buf, hdr.job_id, size);
    buf += size;
    *buf = sizeof(hdr.gen_hdr_block_type);
}

void OSEGD::Impl::write_line_id(char* buf, CommonSEGD::GeneralHeaderLin hdr)
{
    size_t size = sizeof(hdr.line_abbr);
    memcpy(buf, hdr.line_abbr, size);
    buf += size;
    size = sizeof(hdr.line_id);
    memcpy(buf, hdr.line_id, size);
    buf += size;
    *buf = sizeof(hdr.gen_hdr_block_type);
}

void OSEGD::Impl::write_vibrator_hdr(char* buf, CommonSEGD::GeneralHeaderVib hdr)
{
    write_u32(&buf, hdr.expanded_file_number);
    uint32_t integer = hdr.sou_line_num;
    double frac = hdr.sou_line_num - integer;
    write_i24(&buf, integer);
    write_u16(&buf, frac * pow(2, 16));
    integer = hdr.sou_point_num;
    frac = hdr.sou_point_num - integer;
    write_i24(&buf, integer);
    write_u16(&buf, frac * pow(2, 16));
    *buf++ = hdr.sou_point_index;
    *buf++ = hdr.phase_control;
    *buf++ = hdr.type_vibrator;
    write_i16(&buf, hdr.phase_angle);
    *buf++ = hdr.source_id;
    *buf++ = hdr.source_set_num;
    *buf++ = hdr.reshoot_idx;
    *buf++ = hdr.group_idx;
    *buf++ = hdr.depth_idx;
    *buf++ = hdr.offset_crossline;
    *buf++ = hdr.offset_inline;
    *buf++ = hdr.size;
    *buf++ = hdr.offset_depth;
    *buf = hdr.gen_hdr_block_type;
}

void OSEGD::Impl::write_explosive_hdr(char* buf, CommonSEGD::GeneralHeaderExp hdr)
{
    write_u32(&buf, hdr.expanded_file_number);
    uint32_t integer = hdr.sou_line_num;
    double frac = hdr.sou_line_num - integer;
    write_i24(&buf, integer);
    write_u16(&buf, frac * pow(2, 16));
    integer = hdr.sou_point_num;
    frac = hdr.sou_point_num - integer;
    write_i24(&buf, integer);
    write_u16(&buf, frac * pow(2, 16));
    *buf++ = hdr.sou_point_index;
    write_u16(&buf, hdr.depth);
    *buf++ = hdr.charge_length;
    *buf++ = hdr.soil_type;
    *buf++ = hdr.source_id;
    *buf++ = hdr.source_set_num;
    *buf++ = hdr.reshoot_idx;
    *buf++ = hdr.group_idx;
    *buf++ = hdr.depth_idx;
    *buf++ = hdr.offset_crossline;
    *buf++ = hdr.offset_inline;
    *buf++ = hdr.size;
    *buf++ = hdr.offset_depth;
    *buf = hdr.gen_hdr_block_type;
}

void OSEGD::Impl::write_airgun_hdr(char* buf, CommonSEGD::GeneralHeaderAir hdr)
{
    write_u32(&buf, hdr.expanded_file_number);
    uint32_t integer = hdr.sou_line_num;
    double frac = hdr.sou_line_num - integer;
    write_i24(&buf, integer);
    write_u16(&buf, frac * pow(2, 16));
    integer = hdr.sou_point_num;
    frac = hdr.sou_point_num - integer;
    write_i24(&buf, integer);
    write_u16(&buf, frac * pow(2, 16));
    *buf++ = hdr.sou_point_index;
    write_u16(&buf, hdr.depth);
    write_u16(&buf, hdr.air_pressure);
    *buf++ = hdr.source_id;
    *buf++ = hdr.source_set_num;
    *buf++ = hdr.reshoot_idx;
    *buf++ = hdr.group_idx;
    *buf++ = hdr.depth_idx;
    *buf++ = hdr.offset_crossline;
    *buf++ = hdr.offset_inline;
    *buf++ = hdr.size;
    *buf++ = hdr.offset_depth;
    *buf = hdr.gen_hdr_block_type;
}

void OSEGD::Impl::write_watergun_hdr(char* buf, CommonSEGD::GeneralHeaderWat hdr)
{
    write_u32(&buf, hdr.expanded_file_number);
    uint32_t integer = hdr.sou_line_num;
    double frac = hdr.sou_line_num - integer;
    write_i24(&buf, integer);
    write_u16(&buf, frac * pow(2, 16));
    integer = hdr.sou_point_num;
    frac = hdr.sou_point_num - integer;
    write_i24(&buf, integer);
    write_u16(&buf, frac * pow(2, 16));
    *buf++ = hdr.sou_point_index;
    write_u16(&buf, hdr.depth);
    write_u16(&buf, hdr.air_pressure);
    *buf++ = hdr.source_id;
    *buf++ = hdr.source_set_num;
    *buf++ = hdr.reshoot_idx;
    *buf++ = hdr.group_idx;
    *buf++ = hdr.depth_idx;
    *buf++ = hdr.offset_crossline;
    *buf++ = hdr.offset_inline;
    *buf++ = hdr.size;
    *buf++ = hdr.offset_depth;
    *buf = hdr.gen_hdr_block_type;
}

void OSEGD::Impl::write_electromagnetic_hdr(char* buf, CommonSEGD::GeneralHeaderEle hdr)
{
    write_u32(&buf, hdr.expanded_file_number);
    uint32_t integer = hdr.sou_line_num;
    double frac = hdr.sou_line_num - integer;
    write_i24(&buf, integer);
    write_u16(&buf, frac * pow(2, 16));
    integer = hdr.sou_point_num;
    frac = hdr.sou_point_num - integer;
    write_i24(&buf, integer);
    write_u16(&buf, frac * pow(2, 16));
    *buf++ = hdr.sou_point_index;
    *buf++ = hdr.source_type;
    write_u24(&buf, hdr.moment);
    *buf++ = hdr.source_id;
    *buf++ = hdr.source_set_num;
    *buf++ = hdr.reshoot_idx;
    *buf++ = hdr.group_idx;
    *buf++ = hdr.depth_idx;
    *buf++ = hdr.offset_crossline;
    *buf++ = hdr.offset_inline;
    *buf++ = hdr.size;
    *buf++ = hdr.offset_depth;
    *buf = hdr.gen_hdr_block_type;
}

void OSEGD::Impl::write_other_source_hdr(char* buf, CommonSEGD::GeneralHeaderOth hdr)
{
    write_u32(&buf, hdr.expanded_file_number);
    uint32_t integer = hdr.sou_line_num;
    double frac = hdr.sou_line_num - integer;
    write_i24(&buf, integer);
    write_u16(&buf, frac * pow(2, 16));
    integer = hdr.sou_point_num;
    frac = hdr.sou_point_num - integer;
    write_i24(&buf, integer);
    write_u16(&buf, frac * pow(2, 16));
    *buf++ = hdr.sou_point_index;
    buf += 4;
    *buf++ = hdr.source_id;
    *buf++ = hdr.source_set_num;
    *buf++ = hdr.reshoot_idx;
    *buf++ = hdr.group_idx;
    *buf++ = hdr.depth_idx;
    *buf++ = hdr.offset_crossline;
    *buf++ = hdr.offset_inline;
    *buf++ = hdr.size;
    *buf++ = hdr.offset_depth;
    *buf = hdr.gen_hdr_block_type;
}

void OSEGD::Impl::write_additional_source_hdr(char* buf, CommonSEGD::GeneralHeaderAdd hdr)
{
    write_u64(&buf, hdr.time);
    *buf++ = hdr.source_status;
    *buf++ = hdr.source_id;
    *buf++ = hdr.source_moving;
    size_t size = sizeof(hdr.error_description);
    memcpy(buf, hdr.error_description, size);
    buf += size;
    *buf = hdr.gen_hdr_block_type;
}

void OSEGD::Impl::write_source_aux_hdr(char* buf, CommonSEGD::GeneralHeaderSaux hdr)
{
    *buf++ = hdr.source_id;
    to_bcd(&buf, hdr.scan_type_num_1, false, 2);
    write_u16(&buf, hdr.ch_set_num_1);
    write_u24(&buf, hdr.trace_num_1);
    to_bcd(&buf, hdr.scan_type_num_2, false, 2);
    write_u16(&buf, hdr.ch_set_num_2);
    write_u24(&buf, hdr.trace_num_2);
    to_bcd(&buf, hdr.scan_type_num_3, false, 2);
    write_u16(&buf, hdr.ch_set_num_3);
    write_u24(&buf, hdr.trace_num_3);
    to_bcd(&buf, hdr.scan_type_num_4, false, 2);
    write_u16(&buf, hdr.ch_set_num_4);
    write_u24(&buf, hdr.trace_num_4);
    to_bcd(&buf, hdr.scan_type_num_5, false, 2);
    write_u16(&buf, hdr.ch_set_num_5);
    write_u24(&buf, hdr.trace_num_5);
    *buf = hdr.gen_hdr_block_type;
}

void OSEGD::Impl::write_crs_hdr(char* buf, CommonSEGD::GeneralHeaderCoord hdr)
{
    size_t size = sizeof(hdr.crs);
    memcpy(buf, hdr.crs, size);
    buf += size;
    *buf = hdr.gen_hdr_block_type;
}

void OSEGD::Impl::write_position1_hdr(char* buf, CommonSEGD::GeneralHeaderPos1 hdr)
{
    write_u64(&buf, hdr.time_of_position);
    write_u64(&buf, hdr.time_of_measurement);
    write_u32(&buf, hdr.vert_error);
    write_u32(&buf, hdr.hor_error_semi_major);
    write_u32(&buf, hdr.hor_error_semi_minor);
    write_u16(&buf, hdr.hor_error_orientation);
    *buf++ = hdr.position_type;
    *buf = hdr.gen_hdr_block_type;
}

void OSEGD::Impl::write_position2_hdr(char* buf, CommonSEGD::GeneralHeaderPos2 hdr)
{
    write_u64(&buf, hdr.crs_a_coord1);
    write_u64(&buf, hdr.crs_a_coord2);
    write_u64(&buf, hdr.crs_a_coord3);
    write_u16(&buf, hdr.crs_a_crsref);
    *buf++ = hdr.pos1_valid;
    *buf++ = hdr.pos1_quality;
    buf += 3;
    *buf = hdr.gen_hdr_block_type;
}

void OSEGD::Impl::write_position3_hdr(char* buf, CommonSEGD::GeneralHeaderPos3 hdr)
{
    write_u64(&buf, hdr.crs_b_coord1);
    write_u64(&buf, hdr.crs_b_coord2);
    write_u64(&buf, hdr.crs_b_coord3);
    write_u16(&buf, hdr.crs_b_crsref);
    *buf++ = hdr.pos2_valid;
    *buf++ = hdr.pos2_quality;
    buf += 3;
    *buf = hdr.gen_hdr_block_type;
}

void OSEGD::Impl::write_relative_position_hdr(char* buf, CommonSEGD::GeneralHeaderRel hdr)
{
    write_u32(&buf, hdr.offset_east);
    write_u32(&buf, hdr.offset_north);
    write_u32(&buf, hdr.offset_vert);
    size_t size = sizeof(hdr.description);
    memcpy(buf, hdr.description, size);
    buf += size;
    *buf = hdr.gen_hdr_block_type;
}

void OSEGD::Impl::write_sensor_info_hdr(char* buf, CommonSEGD::GeneralHeaderSen hdr)
{
    write_u64(&buf, hdr.instrument_test_time);
    write_u32(&buf, hdr.sensor_sensitivity);
    *buf++ = hdr.instr_test_result;
    size_t size = sizeof(hdr.serial_number);
    memcpy(buf, hdr.serial_number, size);
    buf += size;
    *buf = hdr.gen_hdr_block_type;
}

void OSEGD::Impl::write_sensor_calibration_hdr(char* buf, CommonSEGD::GeneralHeaderSCa hdr)
{
    write_u32(&buf, hdr.freq1);
    write_u32(&buf, hdr.amp1);
    write_u32(&buf, hdr.phase1);
    write_u32(&buf, hdr.freq2);
    write_u32(&buf, hdr.amp2);
    write_u32(&buf, hdr.phase2);
    *buf++ = hdr.calib_applied;
    buf += 6;
    *buf = hdr.gen_hdr_block_type;
}

void OSEGD::Impl::write_time_drift_hdr(char* buf, CommonSEGD::GeneralHeaderTim hdr)
{
    write_u64(&buf, hdr.time_of_depl);
    write_u64(&buf, hdr.time_of_retr);
    write_u32(&buf, hdr.timer_offset_depl);
    write_u32(&buf, hdr.time_offset_retr);
    *buf++ = hdr.timedrift_corr;
    *buf++ = hdr.corr_method;
    buf += 5;
    *buf = hdr.gen_hdr_block_type;
}

void OSEGD::Impl::write_electromagnetic_src_recv_hdr(char* buf, CommonSEGD::GeneralHeaderElSR hdr)
{
    write_u24(&buf, hdr.equip_dim_x);
    write_u24(&buf, hdr.equip_dim_y);
    write_u24(&buf, hdr.equip_dim_z);
    *buf++ = hdr.pos_term;
    write_u24(&buf, hdr.equip_offset_x);
    write_u24(&buf, hdr.equip_offset_y);
    write_u24(&buf, hdr.equip_offset_z);
    buf += 12;
    *buf = hdr.gen_hdr_block_type;
}

void OSEGD::Impl::write_orientation_hdr(char* buf, CommonSEGD::GeneralHeaderOri hdr)
{
    write_u32(&buf, hdr.rot_x);
    write_u32(&buf, hdr.rot_y);
    write_u32(&buf, hdr.rot_z);
    write_u32(&buf, hdr.ref_orientation);
    write_u64(&buf, hdr.time_stamp);
    *buf++ = hdr.ori_type;
    *buf++ = hdr.ref_orient_valid;
    *buf++ = hdr.rot_applied;
    *buf++ = hdr.rot_north_applied;
    buf += 3;
    *buf = hdr.gen_hdr_block_type;
}

void OSEGD::Impl::write_measurement_hdr(char* buf, CommonSEGD::GeneralHeaderMeas hdr)
{
    write_u64(&buf, hdr.timestamp);
    write_u32(&buf, hdr.measurement_value);
    write_u32(&buf, hdr.maximum_value);
    write_u32(&buf, hdr.minimum_value);
    write_u16(&buf, hdr.quantity_class);
    write_u16(&buf, hdr.unit_of_measure);
    write_u16(&buf, hdr.measurement_description);
    buf += 5;
    *buf = hdr.gen_hdr_block_type;
}

void OSEGD::Impl::write_ch_set_hdr(CommonSEGD::ChannelSetHeader& hdr)
{
    common.ch_set_hdr_buf.assign(common.ch_set_hdr_buf.size(), 0);
    char* buf = common.ch_set_hdr_buf.data();
    if (!common.general_header.add_gen_hdr_blocks || common.general_header2.segd_rev_major < 3) {
        to_bcd(&buf, hdr.scan_type_number, false, 2);
        to_bcd(&buf, hdr.channel_set_number, false, 2);
        write_u16(&buf, hdr.channel_set_start_time / 2);
        write_u16(&buf, hdr.channel_set_end_time / 2);
        uint16_t sign = hdr.descale_multiplier < 0 ? 0x8000 : 0;
        uint16_t val = static_cast<uint16_t>(hdr.descale_multiplier * pow(2, 10)) & 0x7fff;
        write_u16(&buf, val | sign);
        to_bcd(&buf, hdr.number_of_channels, false, 4);
        to_bcd(&buf, hdr.channel_type, false, 1);
        ++buf;
        to_bcd(&buf, log2(hdr.channel_set_number), false, 1);
        to_bcd(&buf, hdr.channel_gain, true, 1);
        to_bcd(&buf, hdr.alias_filter_freq, false, 4);
        to_bcd(&buf, hdr.alias_filter_slope, false, 4);
        to_bcd(&buf, hdr.low_cut_filter_freq, false, 4);
        to_bcd(&buf, hdr.low_cut_filter_slope, false, 4);
        to_bcd(&buf, hdr.first_notch_filter, false, 4);
        to_bcd(&buf, hdr.second_notch_filter, false, 4);
        to_bcd(&buf, hdr.third_notch_filter, false, 4);
        write_u16(&buf, hdr.ext_ch_set_num);
        *buf = hdr.ext_hdr_flag << 4;
        *buf++ |= hdr.trc_hdr_ext & 0xf;
        *buf++ = hdr.vert_stack;
        *buf++ = hdr.streamer_no;
        *buf++ = hdr.array_forming;
    } else {
        to_bcd(&buf, hdr.scan_type_number, false, 2);
        write_u16(&buf, hdr.channel_set_start_time);
        *buf++ = hdr.channel_type;
        write_u32(&buf, hdr.channel_set_start_time);
        write_u32(&buf, hdr.channel_set_end_time);
        write_u32(&buf, *hdr.number_of_samples());
        write_u32(&buf, hdr.descale_multiplier);
        write_u24(&buf, hdr.number_of_channels);
        write_u24(&buf, *hdr.samp_int());
        *buf++ = hdr.array_forming;
        *buf++ = hdr.trc_hdr_ext;
        *buf = hdr.ext_hdr_flag << 4;
        *buf++ |= hdr.channel_gain;
        *buf++ = hdr.vert_stack;
        *buf++ = hdr.streamer_no;
        ++buf;
        write_u32(&buf, hdr.alias_filter_freq);
        write_u32(&buf, hdr.alias_filter_slope);
        write_u32(&buf, hdr.low_cut_filter_freq);
        write_u32(&buf, hdr.low_cut_filter_slope);
        write_u32(&buf, hdr.first_notch_filter);
        write_u32(&buf, hdr.second_notch_filter);
        write_u32(&buf, hdr.third_notch_filter);
        *buf++ = *hdr.filter_phase();
        *buf++ = *hdr.physical_unit();
        buf += 2;
        write_u32(&buf, *hdr.filter_delay());
        memcpy(buf, (*hdr.description()).data(), (*hdr.description()).size());
    }
    common.file.write(common.ch_set_hdr_buf.data(), common.ch_set_hdr_buf.size());
}

void OSEGD::Impl::write_trace_header(Trace::Header const& hdr)
{
    char* buf = common.trc_hdr_buf;
    memset(buf, 0, CommonSEGD::TRACE_HEADER_SIZE);
    optional<Trace::Header::Value> ffid = hdr.get("FFID");
    if (ffid && get<uint32_t>(*ffid) > 9999)
        write_u16(&buf, 0xffff);
    else
        to_bcd(&buf, ffid ? get<int32_t>(*ffid) : 0, false, 4);
    optional<Trace::Header::Value> tmp = hdr.get("SCAN_TYPE_NUM");
    to_bcd(&buf, tmp ? get<int16_t>(*tmp) : 0, false, 2);
    optional<Trace::Header::Value> ch_set = hdr.get("CH_SET_NUM");
    if (ch_set && get<uint16_t>(*ch_set) > 99)
        (void)(*buf++ = static_cast<uint8_t>(0xff));
    else
        to_bcd(&buf, get<uint16_t>(*ch_set), false, 2);
    tmp = hdr.get("TRACE_NUMBER");
    if (tmp && get<int32_t>(*tmp) > 9999)
        write_u16(&buf, 0xffff);
    else
        to_bcd(&buf, tmp ? get<int32_t>(*tmp) : 0, false, 4);
    tmp = hdr.get("FIRST_TIMING_WORD");
    write_u24(&buf, tmp ? get<int32_t>(*tmp) * pow(2, 8) : 0);
    tmp = hdr.get("TR_HDR_EXT");
    *buf++ = tmp ? get<uint8_t>(*tmp) : 0;
    tmp = hdr.get("SAMPLE_SKEW");
    *buf++ = tmp ? get<uint8_t>(*tmp) * pow(2, 8) : 0;
    tmp = hdr.get("TRACE_EDIT");
    *buf++ = tmp ? get<uint8_t>(*tmp) : 0;
    tmp = hdr.get("TIME_BREAK_WIN");
    write_u24(&buf, tmp ? get<int32_t>(*tmp) * pow(2, 8) : 0);
    write_u16(&buf, tmp ? get<uint16_t>(*ch_set) : 0);
    write_u32(&buf, tmp ? get<uint32_t>(*ffid) : 0);
    common.file.write(common.trc_hdr_buf, CommonSEGD::TRACE_HEADER_SIZE);
}

void OSEGD::Impl::write_ext_trace_header(Trace::Header const& hdr)
{
    char* buf = common.trc_hdr_buf;
    memset(buf, 0, CommonSEGD::TRACE_HEADER_SIZE);
    optional<Trace::Header::Value> tmp = hdr.get("R_LINE");
    write_u24(&buf, tmp ? get<uint32_t>(*tmp) : 0);
    tmp = hdr.get("R_POINT");
    write_u24(&buf, tmp ? get<uint32_t>(*tmp) : 0);
    tmp = hdr.get("R_POINT_IDX");
    *buf++ = tmp ? get<uint8_t>(*tmp) : 0;
    if (common.general_header2.segd_rev_major > 1) {
        tmp = hdr.get("SAMP_NUM");
        write_u24(&buf, tmp ? get<uint32_t>(*tmp) : 0);
    } else {
        tmp = hdr.get("RESHOOT_IDX");
        *buf++ = tmp ? get<uint8_t>(*tmp) : 0;
        tmp = hdr.get("GROUP_IDX");
        *buf++ = tmp ? get<uint8_t>(*tmp) : 0;
        tmp = hdr.get("DEPTH_IDX");
        *buf++ = tmp ? get<uint8_t>(*tmp) : 0;
    }
    if (common.general_header2.segd_rev_major > 1) {
        tmp = hdr.get("R_LINE");
        double r_line = tmp ? get<double>(*tmp) : 0;
        uint32_t r_line_int = r_line;
        write_u24(&buf, r_line_int);
        write_u16(&buf, (r_line - r_line_int) * pow(2, 16));
        tmp = hdr.get("R_POINT");
        double r_point = tmp ? get<double>(*tmp) : 0;
        uint32_t r_point_int = r_point;
        write_u24(&buf, r_point_int);
        write_u16(&buf, (r_point - r_point_int) * pow(2, 16));
        tmp = hdr.get("SENSOR_TYPE");
        *buf++ = tmp ? get<uint8_t>(*tmp) : 0;
    }
    if (common.general_header2.segd_rev_major > 2) {
        tmp = hdr.get("TRACE_NUMBER");
        if (tmp && get<int32_t>(*tmp) > 9999)
            write_u24(&buf, get<int32_t>(*tmp));
        tmp = hdr.get("SAMP_NUM");
        write_u32(&buf, tmp ? get<uint32_t>(*tmp) : 0);
        tmp = hdr.get("SENSOR_MOVING");
        *buf++ = tmp ? get<uint8_t>(*tmp) : 0;
        ++buf;
        tmp = hdr.get("PHYSICAL_UNIT");
        *buf++ = tmp ? get<uint8_t>(*tmp) : 0;
    }
}

void OSEGD::Impl::write_trace_samples(Trace const& trc)
{
    if (common.trc_samp_buf.size() != trc.samples().size() * common.bits_per_sample)
        common.trc_samp_buf.resize(trc.samples().size() * common.bits_per_sample);
    char* buf = common.trc_samp_buf.data();
    valarray<double> const& samples = trc.samples();
    for (auto samp : samples)
        write_sample(&buf, samp);
    common.file.write(common.trc_samp_buf.data(), common.trc_samp_buf.size());
}

void OSEGD::Impl::assign_sample_writers()
{
    switch (common.general_header.format_code) {
    case 8015:
        common.bits_per_sample = 20;
        write_sample = [this](char** buf, double val) {
            static double result[4];
            static int counter = 0;
            result[counter++] = val;
            if (counter == 4) {
                counter = 0;
                uint32_t sign = val < 0 ? 1 : 0;
                uint16_t exp[4], frac[4];
                for (int i = 0; i < 4; ++i) {
                    double abs_val = abs(result[i]);
                    exp[i] = static_cast<uint16_t>(log(abs_val) / log(2) + 1 + 15) & 0xf;
                    frac[i] = abs_val / pow(2, exp[i] - 15);
                }
                for (int i = 0; i < 4; i += 2) {
                    **buf = exp[i] << 4 | exp[i + 1];
                    ++*buf;
                }
                for (int i = 0; i < 4; ++i)
                    write_i16(buf, frac[i] | sign << 15);
            }
        };
        break;
    case 8022:
        common.bits_per_sample = 8;
        write_sample = [](char** buf, double val) {
            uint32_t sign = val < 0 ? 1 : 0;
            double abs_val = abs(val);
            uint32_t exp = static_cast<uint32_t>(log(abs_val) / log(2) + 1 + 4) & 0b111;
            uint32_t frac = static_cast<uint32_t>(abs_val / pow(2, exp - 4)) & 0xf;
            **buf++ = sign << 7 | exp << 4 | frac;
        };
        break;
    case 8024:
        common.bits_per_sample = 16;
        write_sample = [this](char** buf, double val) {
            uint32_t sign = val < 0 ? 1 : 0;
            double abs_val = abs(val);
            uint32_t exp = static_cast<uint32_t>(log(abs_val) / log(2) + 1 + 12) & 0b111;
            uint32_t frac = static_cast<uint32_t>(abs_val / pow(2, exp - 12)) & 0xfff;
            write_u16(buf, sign << 15 | exp << 12 | frac);
        };
        break;
    case 8036:
        common.bits_per_sample = 24;
        write_sample = [this](char** buf, double val) { write_i24(buf, val); };
        break;
    case 8038:
        common.bits_per_sample = 32;
        write_sample = [this](char** buf, double val) { write_i32(buf, val); };
        break;
    case 8042:
        common.bits_per_sample = 8;
        write_sample = [](char** buf, double val) {
            uint32_t sign = val < 0 ? 1 : 0;
            double abs_val = abs(val);
            uint32_t exp = static_cast<uint32_t>(log(abs_val) / log(2) / 4 + 1) & 0b11;
            uint32_t frac = static_cast<uint32_t>(abs_val / pow(16, exp) * pow(2, 5)) & 0x1f;
            **buf++ = sign << 7 | exp << 5 | frac;
        };
        break;
    case 8044:
        common.bits_per_sample = 16;
        write_sample = [this](char** buf, double val) {
            uint32_t sign = val < 0 ? 1 : 0;
            double abs_val = abs(val);
            uint32_t exp = static_cast<uint32_t>(log(abs_val) / log(2) / 4 + 1) & 0b11;
            uint32_t frac = static_cast<uint32_t>(abs_val / pow(16, exp) * pow(2, 13)) & 0x1fff;
            write_u16(buf, sign << 15 | exp << 13 | frac);
        };
        break;
    case 8048:
        common.bits_per_sample = 32;
        write_sample = [this](char** buf, double val) {
            uint32_t sign = val < 0 ? 1 : 0;
            double abs_val = abs(val);
            uint32_t exp = static_cast<uint32_t>(log(abs_val) / log(2) / 4 + 1) & 0b01111111;
            uint32_t frac = static_cast<uint32_t>(abs_val / pow(16, exp) * pow(2, 24)) & 0xffffff;
            write_u32(buf, sign << 31 | exp << 24 | frac);
        };
        break;
    case 8058:
        common.bits_per_sample = 32;
        write_sample = [this](char** buf, double val) {
            float tmp = val;
            uint32_t result;
            memcpy(&result, &tmp, sizeof(result));
            write_u32(buf, result);
        };
        break;
    case 8080:
        common.bits_per_sample = 64;
        write_sample = [this](char** buf, double val) {
            uint64_t result;
            memcpy(&result, &val, sizeof(result));
            write_u64(buf, result);
        };
        break;
    case 9036:
        common.bits_per_sample = 24;
        if (endian::native == endian::big)
            write_sample = [this](char** buf, double val) { return write_i24(buf, swap(static_cast<int32_t>(val))); };
        else
            write_sample = [](char** buf, double val) {
                write<int16_t>(buf, val);
                write<int8_t>(buf, static_cast<int32_t>(val) >> 16);
            };
        break;
    case 9038:
        common.bits_per_sample = 32;
        if (endian::native == endian::big)
            write_sample = [this](char** buf, double val) { return write_i32(buf, swap(static_cast<int32_t>(val))); };
        else
            write_sample = [](char** buf, double val) { return write<int32_t>(buf, val); };
        break;
    case 9058:
        common.bits_per_sample = 32;
        if (endian::native == endian::big)
            write_sample = [this](char** buf, double val) {
                uint32_t result;
                float tmp = val;
                memcpy(&result, &tmp, sizeof(result));
                write_u32(buf, swap(result));
            };
        else
            write_sample = [](char** buf, double val) {
                uint32_t result;
                float tmp = val;
                memcpy(&result, &tmp, sizeof(result));
                write<uint32_t>(buf, result);
            };
        break;
    case 9080:
        common.bits_per_sample = 64;
        if (endian::native == endian::big)
            write_sample = [this](char** buf, double val) {
                uint64_t result;
                memcpy(&result, &val, sizeof(result));
                write_u64(buf, swap(result));
            };
        else
            write_sample = [](char** buf, double val) {
                uint64_t result;
                memcpy(&result, &val, sizeof(result));
                write<uint64_t>(buf, result);
            };
        break;
    default:
        throw Exception(__FILE__, __LINE__, "Unsupported format");
        break;
    }
}

uint64_t OSEGD::chans_in_record() { return pimpl->chans_in_record; }
void OSEGD::assign_raw_writers() { pimpl->assign_raw_writers(); }
void OSEGD::assign_sample_writers() { pimpl->assign_sample_writers(); }
void OSEGD::write_general_header1() { pimpl->write_general_header1(); }
void OSEGD::write_general_header2_rev2() { pimpl->write_general_header2_rev2(); }
void OSEGD::write_rev2_add_gen_hdrs() { pimpl->write_rev2_add_gen_hdrs(); }
void OSEGD::write_general_header2_and_3() { pimpl->write_general_header2_and_3(); }
void OSEGD::write_rev3_add_gen_hdrs() { pimpl->write_rev3_add_gen_hdrs(); }
void OSEGD::write_ch_set_hdr(CommonSEGD::ChannelSetHeader& hdr) { pimpl->write_ch_set_hdr(hdr); }
void OSEGD::write_trace_header(Trace::Header const& hdr) { pimpl->write_trace_header(hdr); }
void OSEGD::write_ext_trace_header(Trace::Header const& hdr) { pimpl->write_ext_trace_header(hdr); }
void OSEGD::write_trace_samples(Trace const& trc) { pimpl->write_trace_samples(trc); }

OSEGD::OSEGD(string file_name, CommonSEGD::GeneralHeader gh,
    CommonSEGD::GeneralHeader2 gh2, CommonSEGD::GeneralHeader3 gh3,
    vector<vector<CommonSEGD::ChannelSetHeader>> ch_sets,
    vector<unique_ptr<CommonSEGD::AdditionalGeneralHeader>> add_ghs)
    : pimpl { make_unique<Impl>(CommonSEGD(move(file_name), fstream::out | fstream::binary, move(gh),
        move(gh2), move(gh3), move(add_ghs), move(ch_sets))) }
{
    if (ch_sets.size() != static_cast<size_t>(gh.scan_types_per_record))
        throw Exception(__FILE__, __LINE__, "Size of vector with vectors of channel set header not equal to number of scan types in general header");
}
CommonSEGD& OSEGD::common() { return pimpl->common; }
OSEGD::~OSEGD() = default;
} // namespace sedaman
