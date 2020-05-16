#include "ISEGD.hpp"
#include "util.hpp"
#include <cmath>

using std::fstream;
using std::make_unique;
using std::move;
using std::string;

namespace sedaman {
class ISEGD::Impl {
public:
    Impl(ISEGD& s);
    ISEGD& sgd;
private:
    void read_gen_hdr();
};

ISEGD::Impl::Impl(ISEGD& s)
    : sgd(s)
{
    read_gen_hdr();
}

void ISEGD::Impl::read_gen_hdr()
{
    sgd.p_file().read(sgd.p_gen_hdr_buf().data(), sgd.p_gen_hdr_buf().size());
    char const* buf = sgd.p_gen_hdr_buf().data();
    GeneralHeader& gh = sgd.p_general_header();
    gh.file_number = from_bcd<int>(&buf, false, 4);
    gh.format_code = from_bcd<int>(&buf, false, 4);
    gh.gen_const = from_bcd<long long>(&buf, false, 12);
    gh.year = from_bcd<int>(&buf, false, 2);
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

CommonSEGD::GeneralHeader const& ISEGD::general_header() { return p_general_header(); }

ISEGD::ISEGD(string name)
    : CommonSEGD(move(name), fstream::in)
    , pimpl(make_unique<Impl>(*this))
{
}

ISEGD::~ISEGD() = default;
} //sedaman
