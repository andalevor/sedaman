#include "CommonSEGD.hpp"
#include "Exception.hpp"
#include "util.hpp"
#include <cmath>
#include <functional>

using std::array;
using std::fstream;
using std::function;
using std::make_unique;
using std::move;
using std::nullopt;
using std::optional;
using std::string;
using std::unique_ptr;
using std::vector;

namespace sedaman
{
    CommonSEGD::CommonSEGD(string file_name, fstream::openmode mode,
                           GeneralHeader gh, GeneralHeader2 gh2, GeneralHeader3 gh3,
                           vector<unique_ptr<AdditionalGeneralHeader>> add_ghs,
                           std::vector<std::vector<ChannelSetHeader>> ch_sets)
        : general_header{gh}, general_header2{gh2}, general_header3{gh3},
          file_name(move(file_name)), gen_hdr_buf{}, trc_hdr_buf{}, channel_sets{ch_sets}
    {
        fstream fl;
        fl.exceptions(fstream::failbit | fstream::badbit);
        fl.open(file_name, mode);
        file = move(fl);
        for (unique_ptr<AdditionalGeneralHeader> &item : add_ghs)
        {
            switch (item->type())
            {
            case AdditionalGeneralHeader::VESSEL_CREW_ID:
                add_gen_hdr_blks_map[AdditionalGeneralHeader::VESSEL_CREW_ID] = make_unique<GeneralHeaderVes>(*dynamic_cast<GeneralHeaderVes *>(item.get()));
                break;
            case AdditionalGeneralHeader::SURVEY_AREA_NAME:
                add_gen_hdr_blks_map[AdditionalGeneralHeader::SURVEY_AREA_NAME] = make_unique<GeneralHeaderSur>(*dynamic_cast<GeneralHeaderSur *>(item.get()));
                break;
            case AdditionalGeneralHeader::CLIENT_NAME:
                add_gen_hdr_blks_map[AdditionalGeneralHeader::CLIENT_NAME] = make_unique<GeneralHeaderCli>(*dynamic_cast<GeneralHeaderCli *>(item.get()));
                break;
            case AdditionalGeneralHeader::JOB_ID:
                add_gen_hdr_blks_map[AdditionalGeneralHeader::JOB_ID] = make_unique<GeneralHeaderJob>(*dynamic_cast<GeneralHeaderJob *>(item.get()));
                break;
            case AdditionalGeneralHeader::LINE_ID:
                add_gen_hdr_blks_map[AdditionalGeneralHeader::LINE_ID] = make_unique<GeneralHeaderLin>(*dynamic_cast<GeneralHeaderLin *>(item.get()));
                break;
            case AdditionalGeneralHeader::VIBRATOR_SOURCE_INFO:
                add_gen_hdr_blks_map[AdditionalGeneralHeader::VIBRATOR_SOURCE_INFO] = make_unique<GeneralHeaderVib>(*dynamic_cast<GeneralHeaderVib *>(item.get()));
                break;
            case AdditionalGeneralHeader::EXPLOSIVE_SOURCE_INFO:
                add_gen_hdr_blks_map[AdditionalGeneralHeader::EXPLOSIVE_SOURCE_INFO] = make_unique<GeneralHeaderExp>(*dynamic_cast<GeneralHeaderExp *>(item.get()));
                break;
            case AdditionalGeneralHeader::AIRGUN_SOURCE_INFO:
                add_gen_hdr_blks_map[AdditionalGeneralHeader::AIRGUN_SOURCE_INFO] = make_unique<GeneralHeaderAir>(*dynamic_cast<GeneralHeaderAir *>(item.get()));
                break;
            case AdditionalGeneralHeader::WATERGUN_SOURCE_INFO:
                add_gen_hdr_blks_map[AdditionalGeneralHeader::WATERGUN_SOURCE_INFO] = make_unique<GeneralHeaderWat>(*dynamic_cast<GeneralHeaderWat *>(item.get()));
                break;
            case AdditionalGeneralHeader::ELECTROMAGNETIC_SOURCE:
                add_gen_hdr_blks_map[AdditionalGeneralHeader::ELECTROMAGNETIC_SOURCE] = make_unique<GeneralHeaderEle>(*dynamic_cast<GeneralHeaderEle *>(item.get()));
                break;
            case AdditionalGeneralHeader::OTHER_SOURCE_TYPE_INFO:
                add_gen_hdr_blks_map[AdditionalGeneralHeader::OTHER_SOURCE_TYPE_INFO] = make_unique<GeneralHeaderOth>(*dynamic_cast<GeneralHeaderOth *>(item.get()));
                break;
            case AdditionalGeneralHeader::ADD_SOURCE_INFO:
                add_gen_hdr_blks_map[AdditionalGeneralHeader::ADD_SOURCE_INFO] = make_unique<GeneralHeaderAdd>(*dynamic_cast<GeneralHeaderAdd *>(item.get()));
                break;
            case AdditionalGeneralHeader::SOU_AUX_CHAN_REF:
                add_gen_hdr_blks_map[AdditionalGeneralHeader::SOU_AUX_CHAN_REF] = make_unique<GeneralHeaderSaux>(*dynamic_cast<GeneralHeaderSaux *>(item.get()));
                break;
            case AdditionalGeneralHeader::SENSOR_INFO_HDR_EXT_BLK:
                add_gen_hdr_blks_map[AdditionalGeneralHeader::SENSOR_INFO_HDR_EXT_BLK] = make_unique<GeneralHeaderSen>(*dynamic_cast<GeneralHeaderSen *>(item.get()));
                break;
            case AdditionalGeneralHeader::SENSOR_CALIBRATION_BLK:
                add_gen_hdr_blks_map[AdditionalGeneralHeader::SENSOR_CALIBRATION_BLK] = make_unique<GeneralHeaderSCa>(*dynamic_cast<GeneralHeaderSCa *>(item.get()));
                break;
            case AdditionalGeneralHeader::TIME_DRIFT_BLK:
                add_gen_hdr_blks_map[AdditionalGeneralHeader::TIME_DRIFT_BLK] = make_unique<GeneralHeaderTim>(*dynamic_cast<GeneralHeaderTim *>(item.get()));
                break;
            case AdditionalGeneralHeader::ELECTROMAG_SRC_REC_DESC_BLK:
                add_gen_hdr_blks_map[AdditionalGeneralHeader::ELECTROMAG_SRC_REC_DESC_BLK] = make_unique<GeneralHeaderElSR>(*dynamic_cast<GeneralHeaderElSR *>(item.get()));
                break;
            case AdditionalGeneralHeader::POSITION_BLK1:
                add_gen_hdr_blks_map[AdditionalGeneralHeader::POSITION_BLK1] = make_unique<GeneralHeaderPos1>(*dynamic_cast<GeneralHeaderPos1 *>(item.get()));
                break;
            case AdditionalGeneralHeader::POSITION_BLK2:
                add_gen_hdr_blks_map[AdditionalGeneralHeader::POSITION_BLK2] = make_unique<GeneralHeaderPos2>(*dynamic_cast<GeneralHeaderPos2 *>(item.get()));
                break;
            case AdditionalGeneralHeader::POSITION_BLK3:
                add_gen_hdr_blks_map[AdditionalGeneralHeader::POSITION_BLK3] = make_unique<GeneralHeaderPos3>(*dynamic_cast<GeneralHeaderPos3 *>(item.get()));
                break;
            case AdditionalGeneralHeader::COORD_REF_SYSTEM:
                add_gen_hdr_blks_map[AdditionalGeneralHeader::COORD_REF_SYSTEM] = make_unique<GeneralHeaderCoord>(*dynamic_cast<GeneralHeaderCoord *>(item.get()));
                break;
            case AdditionalGeneralHeader::RELATIVE_POS_BLK:
                add_gen_hdr_blks_map[AdditionalGeneralHeader::RELATIVE_POS_BLK] = make_unique<GeneralHeaderRel>(*dynamic_cast<GeneralHeaderRel *>(item.get()));
                break;
            case AdditionalGeneralHeader::ORIENT_HDR_BLK:
                add_gen_hdr_blks_map[AdditionalGeneralHeader::ORIENT_HDR_BLK] = make_unique<GeneralHeaderOri>(*dynamic_cast<GeneralHeaderOri *>(item.get()));
                break;
            case AdditionalGeneralHeader::MEASUREMENT_BLK:
                add_gen_hdr_blks_map[AdditionalGeneralHeader::MEASUREMENT_BLK] = make_unique<GeneralHeaderMeas>(*dynamic_cast<GeneralHeaderMeas *>(item.get()));
                break;
            }
        }
    }

    CommonSEGD::ChannelSetHeader::ChannelSetHeader(int stn, uint16_t chsn, uint8_t cht,
                                                   uint32_t chsstm, uint32_t chsetm, double dm, uint32_t noch, int spchs,
                                                   uint8_t chg, uint32_t aff, uint32_t afs, uint32_t lcff, uint32_t lcfs,
                                                   uint32_t fnf, uint32_t snf, uint32_t tnf, uint16_t echsn, uint8_t ehf,
                                                   uint8_t the, uint8_t vs, uint8_t sn, uint8_t af)
        : scan_type_number{stn}, channel_set_number{chsn}, channel_type{cht},
          channel_set_start_time{chsstm}, channel_set_end_time{chsetm}, descale_multiplier{dm},
          number_of_channels{noch}, subscans_per_ch_set{spchs}, channel_gain{chg},
          alias_filter_freq{aff}, alias_filter_slope{afs}, low_cut_filter_freq{lcff},
          low_cut_filter_slope{lcfs}, first_notch_filter{fnf}, second_notch_filter{snf},
          third_notch_filter{tnf}, ext_ch_set_num{echsn}, ext_hdr_flag{ehf},
          trc_hdr_ext{the}, vert_stack{vs}, streamer_no{sn}, array_forming{af}
    {
        segd_rev_more_than_2 = false;
    }
    CommonSEGD::ChannelSetHeader::ChannelSetHeader(int stn, uint16_t chsn, uint8_t cht,
                                                   uint32_t chsstm, uint32_t chsetm, double dm, uint32_t noch, int spchs,
                                                   uint8_t chg, uint32_t aff, uint32_t afs, uint32_t lcff, uint32_t lcfs,
                                                   uint32_t fnf, uint32_t snf, uint32_t tnf, uint16_t echsn, uint8_t ehf,
                                                   uint8_t the, uint8_t vs, uint8_t sn, uint8_t af, uint32_t nos,
                                                   uint32_t si, uint8_t fph, uint8_t phu, uint32_t fd, array<char, 27> d)
        : scan_type_number{stn}, channel_set_number{chsn}, channel_type{cht},
          channel_set_start_time{chsstm}, channel_set_end_time{chsetm}, descale_multiplier{dm},
          number_of_channels{noch}, subscans_per_ch_set{spchs}, channel_gain{chg},
          alias_filter_freq{aff}, alias_filter_slope{afs}, low_cut_filter_freq{lcff},
          low_cut_filter_slope{lcfs}, first_notch_filter{fnf}, second_notch_filter{snf},
          third_notch_filter{tnf}, ext_ch_set_num{echsn}, ext_hdr_flag{ehf},
          trc_hdr_ext{the}, vert_stack{vs}, streamer_no{sn}, array_forming{af},
          p_number_of_samples{nos}, p_samp_int{si}, p_filter_phase{fph},
          p_physical_unit{phu}, p_filter_delay{fd}, p_description{d}
    {
        segd_rev_more_than_2 = true;
    }

    optional<uint32_t> CommonSEGD::ChannelSetHeader::number_of_samples() { return segd_rev_more_than_2 ? optional<uint32_t>(p_number_of_samples) : nullopt; }
    optional<uint32_t> CommonSEGD::ChannelSetHeader::samp_int() { return segd_rev_more_than_2 ? optional<uint32_t>(p_samp_int) : nullopt; }
    optional<uint8_t> CommonSEGD::ChannelSetHeader::filter_phase() { return segd_rev_more_than_2 ? optional<uint8_t>(p_filter_phase) : nullopt; }
    optional<uint8_t> CommonSEGD::ChannelSetHeader::physical_unit() { return segd_rev_more_than_2 ? optional<uint8_t>(p_physical_unit) : nullopt; }
    optional<uint32_t> CommonSEGD::ChannelSetHeader::filter_delay() { return segd_rev_more_than_2 ? optional<uint8_t>(p_filter_delay) : nullopt; }
    optional<array<char, 27>> CommonSEGD::ChannelSetHeader::description() { return segd_rev_more_than_2 ? optional<array<char, 27>>(p_description) : nullopt; }

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
