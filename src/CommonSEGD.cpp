#include "CommonSEGD.hpp"
#include "Exception.hpp"
#include "util.hpp"
#include <cmath>
#include <functional>

using std::array;
using std::fstream;
using std::function;
using std::move;
using std::nullopt;
using std::optional;
using std::string;
using std::vector;

namespace sedaman
{
    CommonSEGD::CommonSEGD(string name, fstream::openmode mode)
        : file_name(move(name)), gen_hdr_buf(vector<char>(CommonSEGD::GEN_HDR_SIZE)), trc_hdr_buf(vector<char>(CommonSEGD::TRACE_HEADER_SIZE))
    {
        fstream fl;
        fl.exceptions(fstream::failbit | fstream::badbit);
        fl.open(file_name, mode);
        file = move(fl);
    }

    CommonSEGD::ChannelSetHeader::ChannelSetHeader(CommonSEGD &common)
    {
        function<uint16_t(char const **)> read_u16 = [](char const **buf) { return swap(read<uint16_t>(buf)); };
        function<int16_t(char const **)> read_i16 = [](char const **buf) { return swap(read<int16_t>(buf)); };
        function<uint32_t(char const **)> read_u24 = [](char const **buf) { return read<uint16_t>(buf) << 8 | read<uint8_t>(buf); };
        function<uint32_t(char const **)> read_u32 = [](char const **buf) { return swap(read<uint32_t>(buf)); };
        char const *buf = common.ch_set_hdr_buf.data();
        if (common.general_header.add_gen_hdr_blocks)
            segd_rev_major = common.general_header2.segd_rev_major;
        if (!common.general_header.add_gen_hdr_blocks || common.general_header2.segd_rev_major < 3)
        {
            scan_type_number = from_bcd<int>(&buf, false, 2);
            channel_set_number = from_bcd<int>(&buf, false, 2);
            channel_set_start_time = read_u16(&buf) * 2;
            channel_set_end_time = read_u16(&buf) * 2;
            descale_multiplier = static_cast<double>(swap(read_i16(&buf))) / pow(2, 10);
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
            vert_stack = *buf++;
            streamer_no = *buf++;
            array_forming = *buf++;
        }
        else
        {
            scan_type_number = from_bcd<int>(&buf, false, 2);
            channel_set_number = read_u16(&buf);
            channel_type = *buf++;
            channel_set_start_time = read_u32(&buf);
            channel_set_end_time = read_u32(&buf);
            p_number_of_samples = read_u32(&buf);
            descale_multiplier = read_u32(&buf);
            number_of_channels = read_u24(&buf);
            p_samp_int = read_u24(&buf);
            array_forming = *buf++;
            trc_hdr_ext = *buf++;
            ext_hdr_flag = *buf >> 4;
            channel_gain = *buf & 0x0f;
            ++buf;
            vert_stack = *buf++;
            streamer_no = *buf++;
            ++buf;
            alias_filter_freq = read_u32(&buf);
            low_cut_filter_freq = read_u32(&buf);
            alias_filter_slope = read_u32(&buf);
            low_cut_filter_slope = read_u32(&buf);
            first_notch_filter = read_u32(&buf);
            second_notch_filter = read_u32(&buf);
            third_notch_filter = read_u32(&buf);
            p_filter_phase = *buf++;
            p_physical_unit = *buf++;
            buf += 2;
            p_filter_delay = read_u32(&buf);
            memcpy(p_description.data(), buf, p_description.size());
        }
    }

    optional<uint32_t> CommonSEGD::ChannelSetHeader::number_of_samples() { return segd_rev_major > 2 ? optional<uint32_t>(p_number_of_samples) : nullopt; }
    optional<uint32_t> CommonSEGD::ChannelSetHeader::samp_int() { return segd_rev_major > 2 ? optional<uint32_t>(p_samp_int) : nullopt; }
    optional<uint8_t> CommonSEGD::ChannelSetHeader::filter_phase() { return segd_rev_major > 2 ? optional<uint8_t>(p_filter_phase) : nullopt; }
    optional<uint8_t> CommonSEGD::ChannelSetHeader::physical_unit() { return segd_rev_major > 2 ? optional<uint8_t>(p_physical_unit) : nullopt; }
    optional<uint32_t> CommonSEGD::ChannelSetHeader::filter_delay() { return segd_rev_major > 2 ? optional<uint8_t>(p_filter_delay) : nullopt; }
    optional<array<char, 27>> CommonSEGD::ChannelSetHeader::description() { return segd_rev_major > 2 ? optional<array<char, 27>>(p_description) : nullopt; }

    static char const *gh1_names[] = {
        "File number",
        "Format code",
        "General constants",
        "Last two digits of year",
        "Number of additional blocks in General Header",
        "Julian day",
        "Hour of dat (Greenwich Mean Time)",
        "Minute of hour",
        "Second of minute",
        "Manufacturer's code",
        "Manufacturer's serial number",
        "Bytes per scan are utilized in the multiplexed formats",
        "Base scan interval",
        "Polarity",
        "Number of scans in a block. It is valid only for multiplexed data",
        "Record type",
        "Record length from time zero",
        "Scan types per record",
        "Number of channel sets per scan type",
        "Number of 32 byte fields added to the end of each scan type header",
        "Extended header length",
        "External header length"};

    char const *CommonSEGD::GeneralHeader::name_as_string(Name n)
    {
        return gh1_names[static_cast<int>(n)];
    }

    static char const *gh2_names[] = {
        "Expanded File Number",
        "Extended Channel Sets per Scan Types",
        "Extended Header Blocks",
        "External Header Blocks",
        "Extended_skew_blocks",
        "SEG-D Revision Number Major",
        "SEG-D Revision Number Minor",
        "The number of 32 bytr blocks to be used to General Trailers",
        "Extended Record Length",
        "Record set number",
        "Extended Number of additional Blocks in the General Header",
        "Dominant Sampling Interval",
        "General Header Block Number"};

    char const *CommonSEGD::GeneralHeader2::name_as_string(Name n)
    {
        return gh2_names[static_cast<int>(n)];
    }

    static char const *ghN_names[] = {
        "Expanded File Number",
        "Source Line Number",
        "Source Point Number",
        "Source Point Index",
        "Phase Control",
        "Type Vibrator",
        "Phase Angle",
        "General Header Block Number",
        "Source Set Number"};

    char const *CommonSEGD::GeneralHeaderN::name_as_string(Name n)
    {
        return ghN_names[static_cast<int>(n)];
    }

    static char const *gh3_names[] = {
        "Time Zero for this record",
        "The total size of the SEG-D record in bytes",
        "The total size of the headers and data in this record in bytes",
        "The total size of the headers in this record in bytes",
        "1 for Extended Recording Mode, 0 for normal record",
        "1 if this record does not contain absolute timestamps",
        "General Header Block Number"};

    char const *CommonSEGD::GeneralHeader3::name_as_string(Name n)
    {
        return gh3_names[static_cast<int>(n)];
    }

    static char const *ghVes_names[] = {
        "Abbreviated vessel or crew name",
        "Vessel or crew name",
        "General header type"};

    char const *CommonSEGD::GeneralHeaderVes::name_as_string(Name n)
    {
        return ghVes_names[static_cast<int>(n)];
    }

    static char const *ghSur_names[] = {
        "Survey Area Name",
        "General header type"};

    char const *CommonSEGD::GeneralHeaderSur::name_as_string(Name n)
    {
        return ghSur_names[static_cast<int>(n)];
    }

    static char const *ghCli_names[] = {
        "Client Area Name",
        "General header type"};

    char const *CommonSEGD::GeneralHeaderCli::name_as_string(Name n)
    {
        return ghCli_names[static_cast<int>(n)];
    }

    static char const *ghJob_names[] = {
        "Abbr Job Identification",
        "Job Identification",
        "General header type"};

    char const *CommonSEGD::GeneralHeaderJob::name_as_string(Name n)
    {
        return ghJob_names[static_cast<int>(n)];
    }

    static char const *ghLin_names[] = {
        "Line Abbreviation",
        "Line Identification",
        "General header type"};

    char const *CommonSEGD::GeneralHeaderLin::name_as_string(Name n)
    {
        return ghLin_names[static_cast<int>(n)];
    }

    static char const *ghVib_names[] = {
        "Expanded File Number",
        "Source Line Number",
        "Source Point Number",
        "Source Point Index",
        "Phase Control",
        "Type Vibrator",
        "Phase Angle",
        "Source Id",
        "Source Set No",
        "Re-shoot Index",
        "Group Index",
        "Depth Index",
        "Offset Cross-line",
        "Offset In-line",
        "Size",
        "Offset Depth",
        "General header type"};

    char const *CommonSEGD::GeneralHeaderVib::name_as_string(Name n)
    {
        return ghVib_names[static_cast<int>(n)];
    }

    static char const *ghExp_names[] = {
        "Expanded File Number",
        "Source Line Number",
        "Source Point Number",
        "Source Point Index",
        "Depth",
        "Charge Length",
        "Soil Type",
        "Source Id",
        "Source Set No",
        "Re-shoot Index",
        "Group Index",
        "Depth Index",
        "Offset Cross-line",
        "Offset In-line",
        "Size",
        "Offset Depth",
        "General header type"};

    char const *CommonSEGD::GeneralHeaderExp::name_as_string(Name n)
    {
        return ghExp_names[static_cast<int>(n)];
    }

    static char const *ghAir_names[] = {
        "Expanded File Number",
        "Source Line Number",
        "Source Point Number",
        "Source Point Index",
        "Depth",
        "Air pressure",
        "Source Id",
        "Source Set No",
        "Re-shoot Index",
        "Group Index",
        "Depth Index",
        "Offset Cross-line",
        "Offset In-line",
        "Size",
        "Offset Depth",
        "General header type"};

    char const *CommonSEGD::GeneralHeaderAir::name_as_string(Name n)
    {
        return ghAir_names[static_cast<int>(n)];
    }

    static char const *ghWat_names[] = {
        "Expanded File Number",
        "Source Line Number",
        "Source Point Number",
        "Source Point Index",
        "Depth",
        "Air pressure",
        "Source Id",
        "Source Set No",
        "Re-shoot Index",
        "Group Index",
        "Depth Index",
        "Offset Cross-line",
        "Offset In-line",
        "Size",
        "Offset Depth",
        "General header type"};

    char const *CommonSEGD::GeneralHeaderWat::name_as_string(Name n)
    {
        return ghWat_names[static_cast<int>(n)];
    }

    static char const *ghEle_names[] = {
        "Expanded File Number",
        "Source Line Number",
        "Source Point Number",
        "Source Point Index",
        "Source type",
        "Moment",
        "Source Id",
        "Source Set No",
        "Re-shoot Index",
        "Group Index",
        "Depth Index",
        "Offset Cross-line",
        "Offset In-line",
        "Size",
        "Offset Depth",
        "General header type"};

    char const *CommonSEGD::GeneralHeaderEle::name_as_string(Name n)
    {
        return ghEle_names[static_cast<int>(n)];
    }

    static char const *ghOth_names[] = {
        "Expanded File Number",
        "Source Line Number",
        "Source Point Number",
        "Source Point Index",
        "Source Id",
        "Source Set No",
        "Re-shoot Index",
        "Group Index",
        "Depth Index",
        "Offset Cross-line",
        "Offset In-line",
        "Size",
        "Offset Depth",
        "General header type"};

    char const *CommonSEGD::GeneralHeaderOth::name_as_string(Name n)
    {
        return ghOth_names[static_cast<int>(n)];
    }

    static char const *ghAdd_names[] = {
        "Time",
        "Source Status",
        "Source Id",
        "Source Moving",
        "Error description",
        "General header type"};

    char const *CommonSEGD::GeneralHeaderAdd::name_as_string(Name n)
    {
        return ghAdd_names[static_cast<int>(n)];
    }

    static char const *ghSaux_names[] = {
        "Source Id",
        "Scan Type Number 1",
        "Channel Set Number 1",
        "Trace Number 1",
        "Scan Type Number 2",
        "Channel Set Number 2",
        "Trace Number 2",
        "Scan Type Number 3",
        "Channel Set Number 3",
        "Trace Number 3",
        "Scan Type Number 4",
        "Channel Set Number 4",
        "Trace Number 4",
        "Scan Type Number 5",
        "Channel Set Number 5",
        "Trace Number 5",
        "General header type"};

    char const *CommonSEGD::GeneralHeaderSaux::name_as_string(Name n)
    {
        return ghSaux_names[static_cast<int>(n)];
    }

    static char const *ghCoord_names[] = {
        "Coordinate Reference System (CRS) identification",
        "General header type"};

    char const *CommonSEGD::GeneralHeaderCoord::name_as_string(Name n)
    {
        return ghCoord_names[static_cast<int>(n)];
    }

    static char const *ghPos1_names[] = {
        "Time of position",
        "Time of measurement/calculation",
        "Vertical error",
        "Horizontal error ellipse semi-major dimension",
        "Horizontal error ellipse semi-minor dimension",
        "Horizontal error ellipse orientation",
        "Position Type",
        "General header type"};

    char const *CommonSEGD::GeneralHeaderPos1::name_as_string(Name n)
    {
        return ghPos1_names[static_cast<int>(n)];
    }

    static char const *ghPos2_names[] = {
        "Coord tuple 1 / CRS A Coord 1",
        "Coord tuple 1 / CRS A Coord 2",
        "Coord tuple 1 / CRS A Coord 3",
        "CRS A CRSREF Stanza ID 1",
        "Position 1 valid",
        "Position 1 quality",
        "General header type"};

    char const *CommonSEGD::GeneralHeaderPos2::name_as_string(Name n)
    {
        return ghPos2_names[static_cast<int>(n)];
    }

    static char const *ghPos3_names[] = {
        "Coord tuple 2 / CRS B Coord 1",
        "Coord tuple 2 / CRS B Coord 2",
        "Coord tuple 2 / CRS B Coord 3",
        "CRS B CRSREF Stanza ID 1",
        "Position 2 valid",
        "Position 2 quality",
        "General header type"};

    char const *CommonSEGD::GeneralHeaderPos3::name_as_string(Name n)
    {
        return ghPos3_names[static_cast<int>(n)];
    }

    static char const *ghRel_names[] = {
        "Offset easting",
        "Offset northing",
        "Offset vertical",
        "Description",
        "General header type"};

    char const *CommonSEGD::GeneralHeaderRel::name_as_string(Name n)
    {
        return ghRel_names[static_cast<int>(n)];
    }

    static char const *ghSen_names[] = {
        "Instrument Test Time",
        "Sensor Sensitivity",
        "Instr Test Result",
        "Serial Number",
        "General header type"};

    char const *CommonSEGD::GeneralHeaderSen::name_as_string(Name n)
    {
        return ghSen_names[static_cast<int>(n)];
    }

    static char const *ghSCa_names[] = {
        "Frequency 1",
        "Amplitude 1",
        "Phase 1",
        "Frequency 2",
        "Amplitude 2",
        "Phase 2",
        "Calibration applied",
        "General header type"};

    char const *CommonSEGD::GeneralHeaderSCa::name_as_string(Name n)
    {
        return ghSCa_names[static_cast<int>(n)];
    }

    static char const *ghTim_names[] = {
        "Time of deployment",
        "Time of retrieval",
        "Timer Offset Deployment",
        "Time Offset Retrieval",
        "Timedrift corrected",
        "Correction method",
        "General header type"};

    char const *CommonSEGD::GeneralHeaderTim::name_as_string(Name n)
    {
        return ghTim_names[static_cast<int>(n)];
    }

    static char const *ghElSR_names[] = {
        "Equipment Dimension X",
        "Equipment Dimension Y",
        "Equipment Dimension Z",
        "Positive terminal",
        "Equipment Offset X",
        "Equipment Offset Y",
        "Equipment Offset Z",
        "General header type"};

    char const *CommonSEGD::GeneralHeaderElSR::name_as_string(Name n)
    {
        return ghElSR_names[static_cast<int>(n)];
    }

    static char const *ghOri_names[] = {
        "Rotation x axis",
        "Rotation Y axis",
        "Rotation Z axis",
        "Reference Orientation",
        "Time Stamp",
        "Orientation Type",
        "Ref Orient Valid",
        "Rot North Applied",
        "General header type"};

    char const *CommonSEGD::GeneralHeaderOri::name_as_string(Name n)
    {
        return ghOri_names[static_cast<int>(n)];
    }

    static char const *ghMeas_names[] = {
        "Timestamp",
        "Measurement Value",
        "Maximum Value",
        "Minimum Value",
        "Quantity Class",
        "Unit Of Measure",
        "Measurement Description",
        "General header type"};

    char const *CommonSEGD::GeneralHeaderMeas::name_as_string(Name n)
    {
        return ghMeas_names[static_cast<int>(n)];
    }

    static char const *ch_sets_names[] = {
        "Scan type number",
        "Channel set number",
        "Channel type",
        "Channel set start time",
        "Channel set end time",
        "Descale multiplier",
        "Number of channels",
        "Subscans per channel set",
        "Channel gain",
        "Alias filter frequency",
        "Alias filter slope",
        "Low cut filter frequency",
        "Low cut filter slope",
        "First notch filter",
        "Second notch filter",
        "Third notch filter",
        "Extended channel set number",
        "Extended header flag",
        "Trace header extension",
        "Verical stack",
        "Streamer no",
        "Array forming",
        "Number of samples",
        "Sample interval",
        "Filter phase",
        "Physical unit",
        "Filter delay",
        "Description"};

    char const *CommonSEGD::ChannelSetHeader::name_as_string(Name n)
    {
        return ch_sets_names[static_cast<int>(n)];
    }
} // namespace sedaman
