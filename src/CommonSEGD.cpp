#include "CommonSEGD.hpp"
#include "Exception.hpp"

using std::fstream;
using std::make_unique;
using std::move;
using std::string;
using std::vector;

namespace sedaman {
class CommonSEGD::Impl {
public:
    explicit Impl(string name)
        : file_name(move(name))
        , gen_hdr2({})
        , gen_hdr_buf(CommonSEGD::GEN_HDR_SIZE)
    {
    }
    string file_name;
    fstream file;
    GeneralHeader gen_hdr;
    GeneralHeader2 gen_hdr2;
    GeneralHeaderN gen_hdrN;
    GeneralHeader3 gen_hdr3;
    GeneralHeaderVes gen_hdr_ves;
    GeneralHeaderSur gen_hdr_sur;
    GeneralHeaderCli gen_hdr_cli;
    GeneralHeaderJob gen_hdr_job;
    GeneralHeaderLin gen_hdr_lin;
    GeneralHeaderVib gen_hdr_vib;
    GeneralHeaderExp gen_hdr_exp;
    GeneralHeaderAir gen_hdr_air;
    GeneralHeaderWat gen_hdr_wat;
    GeneralHeaderEle gen_hdr_ele;
    GeneralHeaderOth gen_hdr_oth;
    GeneralHeaderAdd gen_hdr_add;
    GeneralHeaderSaux gen_hdr_saux;
    GeneralHeaderCoord gen_hdr_coord;
    GeneralHeaderPos1 gen_hdr_pos1;
    GeneralHeaderPos2 gen_hdr_pos2;
    GeneralHeaderPos3 gen_hdr_pos3;
    GeneralHeaderRel gen_hdr_rel;
    vector<char> gen_hdr_buf;
    vector<vector<ChannelSetHeader>> ch_sets;
};

CommonSEGD::CommonSEGD(string name, fstream::openmode mode)
    : pimpl(make_unique<Impl>(move(name)))
{
    fstream fl;
    fl.exceptions(fstream::failbit | fstream::badbit);
    fl.open(pimpl->file_name, mode);
    pimpl->file = move(fl);
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

CommonSEGD::~CommonSEGD() = default;

fstream& CommonSEGD::p_file() { return pimpl->file; }
CommonSEGD::GeneralHeader& CommonSEGD::p_general_header() { return pimpl->gen_hdr; }
CommonSEGD::GeneralHeader2& CommonSEGD::p_general_header2() { return pimpl->gen_hdr2; }
CommonSEGD::GeneralHeaderN& CommonSEGD::p_general_headerN() { return pimpl->gen_hdrN; }
CommonSEGD::GeneralHeader3& CommonSEGD::p_general_header3() { return pimpl->gen_hdr3; }
CommonSEGD::GeneralHeaderVes& CommonSEGD::p_general_header_ves() { return pimpl->gen_hdr_ves; }
CommonSEGD::GeneralHeaderSur& CommonSEGD::p_general_header_sur() { return pimpl->gen_hdr_sur; }
CommonSEGD::GeneralHeaderCli& CommonSEGD::p_general_header_cli() { return pimpl->gen_hdr_cli; }
CommonSEGD::GeneralHeaderJob& CommonSEGD::p_general_header_job() { return pimpl->gen_hdr_job; }
CommonSEGD::GeneralHeaderLin& CommonSEGD::p_general_header_lin() { return pimpl->gen_hdr_lin; }
CommonSEGD::GeneralHeaderVib& CommonSEGD::p_general_header_vib() { return pimpl->gen_hdr_vib; }
CommonSEGD::GeneralHeaderExp& CommonSEGD::p_general_header_exp() { return pimpl->gen_hdr_exp; }
CommonSEGD::GeneralHeaderAir& CommonSEGD::p_general_header_air() { return pimpl->gen_hdr_air; }
CommonSEGD::GeneralHeaderWat& CommonSEGD::p_general_header_wat() { return pimpl->gen_hdr_wat; }
CommonSEGD::GeneralHeaderEle& CommonSEGD::p_general_header_ele() { return pimpl->gen_hdr_ele; }
CommonSEGD::GeneralHeaderOth& CommonSEGD::p_general_header_oth() { return pimpl->gen_hdr_oth; }
CommonSEGD::GeneralHeaderAdd& CommonSEGD::p_general_header_add() { return pimpl->gen_hdr_add; }
CommonSEGD::GeneralHeaderSaux& CommonSEGD::p_general_header_saux() { return pimpl->gen_hdr_saux; }
CommonSEGD::GeneralHeaderCoord& CommonSEGD::p_general_header_coord() { return pimpl->gen_hdr_coord; }
CommonSEGD::GeneralHeaderPos1& CommonSEGD::p_general_header_pos1() { return pimpl->gen_hdr_pos1; }
CommonSEGD::GeneralHeaderPos2& CommonSEGD::p_general_header_pos2() { return pimpl->gen_hdr_pos2; }
CommonSEGD::GeneralHeaderPos3& CommonSEGD::p_general_header_pos3() { return pimpl->gen_hdr_pos3; }
CommonSEGD::GeneralHeaderRel& CommonSEGD::p_general_header_rel() { return pimpl->gen_hdr_rel; }
vector<char>& CommonSEGD::p_gen_hdr_buf() { return pimpl->gen_hdr_buf; }
vector<vector<CommonSEGD::ChannelSetHeader>>& CommonSEGD::p_ch_sets() { return pimpl->ch_sets; }
} //sedaman
