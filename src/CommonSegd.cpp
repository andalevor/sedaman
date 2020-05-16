#include "CommonSegd.hpp"
#include "Exception.hpp"

using std::fstream;
using std::make_unique;
using std::move;
using std::string;
using std::vector;

namespace sedaman {
class CommonSegd::Impl {
public:
    explicit Impl(string name)
        : file_name(move(name))
        , gen_hdr_buf(CommonSegd::GeneralHeader::SIZE)
    {
    }
    string file_name;
    fstream file;
    GeneralHeader gen_hdr;
    vector<char> gen_hdr_buf;
};

CommonSegd::CommonSegd(string name, fstream::openmode mode)
    : pimpl(make_unique<Impl>(move(name)))
{
    fstream fl;
    fl.exceptions(fstream::failbit | fstream::badbit);
    fl.open(pimpl->file_name, mode);
    pimpl->file = move(fl);
}

static char const* bin_names[] = {
    "File number",
    "Format code",
    "General constants",
    "Last two digits of year",
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

char const* CommonSegd::GeneralHeader::name_as_string(Name n)
{
    return bin_names[static_cast<int>(n)];
}

CommonSegd::~CommonSegd() = default;

std::fstream& CommonSegd::p_file() { return pimpl->file; }
CommonSegd::GeneralHeader& CommonSegd::p_general_header() { return pimpl->gen_hdr; }
vector<char>& CommonSegd::p_gen_hdr_buf() { return pimpl->gen_hdr_buf; }
} //sedaman
