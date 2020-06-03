#include "CommonSEGD.hpp"
#include "Exception.hpp"

using std::fstream;
using std::move;
using std::string;
using std::vector;

namespace sedaman {
CommonSEGD::CommonSEGD(string name, fstream::openmode mode)
    : file_name(move(name))
    , gen_hdr_buf(vector<char>(CommonSEGD::GEN_HDR_SIZE))
{
    fstream fl;
    fl.exceptions(fstream::failbit | fstream::badbit);
    fl.open(file_name, mode);
    file = move(fl);
}

static char const* gh1_bin_names[] = {
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
    "Bumber of 32 byte fields added to the end of each scan type header",
    "Extended header length",
    "External header length"
};

char const* CommonSEGD::GeneralHeader::name_as_string(Name n)
{
    return gh1_bin_names[static_cast<int>(n)];
}

static char const* gh2_bin_names[] = {
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
    "General Header Block Number"
};

char const* CommonSEGD::GeneralHeader2::name_as_string(Name n)
{
    return gh2_bin_names[static_cast<int>(n)];
}

static char const* ghN_bin_names[] = {
    "Expanded File Number",
    "Source Line Number",
    "Source Point Number",
    "Source Point Index",
    "Phase Control",
    "Type Vibrator",
    "Phase Angle",
    "General Header Block Number",
    "Source Set Number"
};

char const* CommonSEGD::GeneralHeaderN::name_as_string(Name n)
{
    return ghN_bin_names[static_cast<int>(n)];
}

static char const* gh3_bin_names[] = {
    "Time Zero for this record",
    "The total size of the SEG-D record in bytes",
    "The total size of the headers and data in this record in bytes",
    "The total size of the headers in this record in bytes",
    "1 for Extended Recording Mode, 0 for normal record",
    "1 if this record does not contain absolute timestamps",
    "General Header Block Number"
};

char const* CommonSEGD::GeneralHeader3::name_as_string(Name n)
{
    return gh3_bin_names[static_cast<int>(n)];
}

static char const* ghVes_bin_names[] = {
    "Abbreviated vessel or crew name",
    "Vessel or crew name",
    "General header type"
};

char const* CommonSEGD::GeneralHeaderVes::name_as_string(Name n)
{
    return ghVes_bin_names[static_cast<int>(n)];
}

static char const* ghSur_bin_names[] = {
    "Survey Area Name",
    "General header type"
};

char const* CommonSEGD::GeneralHeaderSur::name_as_string(Name n)
{
    return ghSur_bin_names[static_cast<int>(n)];
}

static char const* ghCli_bin_names[] = {
    "Client Area Name",
    "General header type"
};

char const* CommonSEGD::GeneralHeaderCli::name_as_string(Name n)
{
    return ghCli_bin_names[static_cast<int>(n)];
}

static char const* ghJob_bin_names[] = {
    "Abbr Job Identification",
    "Job Identification",
    "General header type"
};

char const* CommonSEGD::GeneralHeaderJob::name_as_string(Name n)
{
    return ghJob_bin_names[static_cast<int>(n)];
}

static char const* ghLin_bin_names[] = {
    "Line Abbreviation",
    "Line Identification",
    "General header type"
};

char const* CommonSEGD::GeneralHeaderLin::name_as_string(Name n)
{
    return ghLin_bin_names[static_cast<int>(n)];
}

static char const* ghVib_bin_names[] = {
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
    "General header type"
};

char const* CommonSEGD::GeneralHeaderVib::name_as_string(Name n)
{
    return ghVib_bin_names[static_cast<int>(n)];
}

static char const* ghExp_bin_names[] = {
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
    "General header type"
};

char const* CommonSEGD::GeneralHeaderExp::name_as_string(Name n)
{
    return ghExp_bin_names[static_cast<int>(n)];
}

static char const* ghAir_bin_names[] = {
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
    "General header type"
};

char const* CommonSEGD::GeneralHeaderAir::name_as_string(Name n)
{
    return ghAir_bin_names[static_cast<int>(n)];
}

static char const* ghWat_bin_names[] = {
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
    "General header type"
};

char const* CommonSEGD::GeneralHeaderWat::name_as_string(Name n)
{
    return ghWat_bin_names[static_cast<int>(n)];
}

static char const* ghEle_bin_names[] = {
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
    "General header type"
};

char const* CommonSEGD::GeneralHeaderEle::name_as_string(Name n)
{
    return ghEle_bin_names[static_cast<int>(n)];
}

static char const* ghOth_bin_names[] = {
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
    "General header type"
};

char const* CommonSEGD::GeneralHeaderOth::name_as_string(Name n)
{
    return ghOth_bin_names[static_cast<int>(n)];
}

static char const* ghAdd_bin_names[] = {
    "Time",
    "Source Status",
    "Source Id",
    "Source Moving",
    "Error description",
    "General header type"
};

char const* CommonSEGD::GeneralHeaderAdd::name_as_string(Name n)
{
    return ghAdd_bin_names[static_cast<int>(n)];
}

static char const* ghSaux_bin_names[] = {
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
    "General header type"
};

char const* CommonSEGD::GeneralHeaderSaux::name_as_string(Name n)
{
    return ghSaux_bin_names[static_cast<int>(n)];
}

static char const* ghCoord_bin_names[] = {
    "Coordinate Reference System (CRS) identification",
    "General header type"
};

char const* CommonSEGD::GeneralHeaderCoord::name_as_string(Name n)
{
    return ghCoord_bin_names[static_cast<int>(n)];
}

static char const* ghPos1_bin_names[] = {
    "Time of position",
    "Time of measurement/calculation",
    "Vertical error",
    "Horizontal error ellipse semi-major dimension",
    "Horizontal error ellipse semi-minor dimension",
    "Horizontal error ellipse orientation",
    "Position Type",
    "General header type"
};

char const* CommonSEGD::GeneralHeaderPos1::name_as_string(Name n)
{
    return ghPos1_bin_names[static_cast<int>(n)];
}

static char const* ghPos2_bin_names[] = {
    "Coord tuple 1 / CRS A Coord 1",
    "Coord tuple 1 / CRS A Coord 2",
    "Coord tuple 1 / CRS A Coord 3",
    "CRS A CRSREF Stanza ID 1",
    "Position 1 valid",
    "Position 1 quality",
    "General header type"
};

char const* CommonSEGD::GeneralHeaderPos2::name_as_string(Name n)
{
    return ghPos2_bin_names[static_cast<int>(n)];
}

static char const* ghPos3_bin_names[] = {
    "Coord tuple 2 / CRS B Coord 1",
    "Coord tuple 2 / CRS B Coord 2",
    "Coord tuple 2 / CRS B Coord 3",
    "CRS B CRSREF Stanza ID 1",
    "Position 2 valid",
    "Position 2 quality",
    "General header type"
};

char const* CommonSEGD::GeneralHeaderPos3::name_as_string(Name n)
{
    return ghPos3_bin_names[static_cast<int>(n)];
}

static char const* ghRel_bin_names[] = {
    "Offset easting",
    "Offset northing",
    "Offset vertical",
    "Description",
    "General header type"
};

char const* CommonSEGD::GeneralHeaderRel::name_as_string(Name n)
{
    return ghRel_bin_names[static_cast<int>(n)];
}

static char const* ghSen_bin_names[] = {
    "Instrument Test Time",
    "Sensor Sensitivity",
    "Instr Test Result",
    "Serial Number",
    "General header type"
};

char const* CommonSEGD::GeneralHeaderSen::name_as_string(Name n)
{
    return ghSen_bin_names[static_cast<int>(n)];
}

static char const* ghSCa_bin_names[] = {
    "Frequency 1",
    "Amplitude 1",
    "Phase 1",
    "Frequency 2",
    "Amplitude 2",
    "Phase 2",
    "Calibration applied",
    "General header type"
};

char const* CommonSEGD::GeneralHeaderSCa::name_as_string(Name n)
{
    return ghSCa_bin_names[static_cast<int>(n)];
}

static char const* ghTim_bin_names[] = {
    "Time of deployment",
    "Time of retrieval",
    "Timer Offset Deployment",
    "Time Offset Retrieval",
    "Timedrift corrected",
    "Correction method",
    "General header type"
};

char const* CommonSEGD::GeneralHeaderTim::name_as_string(Name n)
{
    return ghTim_bin_names[static_cast<int>(n)];
}

static char const* ghElm_bin_names[] = {
    "Equipment Dimension X",
    "Equipment Dimension Y",
    "Equipment Dimension Z",
    "Positive terminal",
    "Equipment Offset X",
    "Equipment Offset Y",
    "Equipment Offset Z",
    "General header type"
};

char const* CommonSEGD::GeneralHeaderElm::name_as_string(Name n)
{
    return ghElm_bin_names[static_cast<int>(n)];
}

static char const* ghOri_bin_names[] = {
    "Rotation x axis",
    "Rotation Y axis",
    "Rotation Z axis",
    "Reference Orientation",
    "Time Stamp",
    "Orientation Type",
    "Ref Orient Valid",
    "Rot North Applied",
    "General header type"
};

char const* CommonSEGD::GeneralHeaderOri::name_as_string(Name n)
{
    return ghOri_bin_names[static_cast<int>(n)];
}

static char const* ghMeas_bin_names[] = {
    "Timestamp",
    "Measurement Value",
    "Maximum Value",
    "Minimum Value",
    "Quantity Class",
    "Unit Of Measure",
    "Measurement Description",
    "General header type"
};

char const* CommonSEGD::GeneralHeaderMeas::name_as_string(Name n)
{
    return ghMeas_bin_names[static_cast<int>(n)];
}
} //sedaman
