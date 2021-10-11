#include "OSEGDRev2_1.hpp"
#include "Exception.hpp"
#include <cstring>

using std::fstream;
using std::make_unique;
using std::map;
using std::move;
using std::pair;
using std::string;
using std::shared_ptr;
using std::vector;

namespace sedaman {
class OSEGDRev2_1::Impl {
public:
    Impl(OSEGDRev2_1& s);
    uint64_t chans_written;

private:
    OSEGDRev2_1& sgd;
};

OSEGDRev2_1::Impl::Impl(OSEGDRev2_1& s)
    : chans_written { 0 }
    , sgd { s }
{
    sgd.assign_raw_writers();
    sgd.assign_sample_writers();
    sgd.write_general_header1();
    sgd.write_general_header2_rev2();
    sgd.write_rev2_add_gen_hdrs();
    sgd.common().ch_set_hdr_buf.resize(CommonSEGD::CH_SET_HDR_SIZE);
    for (vector<CommonSEGD::ChannelSetHeader>& sets :
		 sgd.common().channel_sets) {
        uint16_t ch_sets_per_scan_type_num =
		   	sgd.common().general_header.channel_sets_per_scan_type == 1665 ?
		   	sgd.common().general_header2.ext_ch_sets_per_scan_type :
		   	sgd.common().general_header.channel_sets_per_scan_type;
        if (sets.size() != ch_sets_per_scan_type_num)
            throw Exception(__FILE__, __LINE__,
							"Size of vector with channel sets not equal to "
							"number of channel sets in general header.");
        for (CommonSEGD::ChannelSetHeader& hdr : sets) {
            sgd.write_ch_set_hdr(hdr);
        }
        if (int skews = sgd.common().general_header.skew_blocks) {
            char* buf = sgd.common().gen_hdr_buf;
            memset(buf, 0, CommonSEGD::SKEW_BLOCK_SIZE);
            for (int i = 0; i < skews; ++i)
                sgd.common().file.write(buf, CommonSEGD::SKEW_BLOCK_SIZE);
        }
    }
    uint32_t extended_blocks =
	   	sgd.common().general_header.extended_hdr_blocks == 165 ?
	   	sgd.common().general_header2.extended_hdr_blocks :
	   	sgd.common().general_header.extended_hdr_blocks;
    uint32_t external_blocks =
	   	sgd.common().general_header.external_hdr_blocks == 165 ?
	   	sgd.common().general_header2.external_hdr_blocks :
	   	sgd.common().general_header.external_hdr_blocks;
    for (uint32_t i = 0; i < extended_blocks; ++i)
        sgd.common().file.write(sgd.common().extended_headers[i].data(),
        CommonSEGD::EXTENDED_HEADER_SIZE);
    for (uint32_t i = 0; i < external_blocks; ++i) {
        sgd.common().file.write(sgd.common().external_headers[i].data(),
        CommonSEGD::EXTERNAL_HEADER_SIZE);
    }
}

void OSEGDRev2_1::write_trace(Trace& t)
{
    if (pimpl->chans_written == chans_in_record())
        throw Exception(__FILE__, __LINE__,
						"you tring write more traces than specified "
						"in channel set headers");
    write_trace_header(t.header());
    write_ext_trace_headers(t.header());
    write_trace_samples(t);
    ++pimpl->chans_written;
}

OSEGDRev2_1::OSEGDRev2_1(string file_name, CommonSEGD::GeneralHeader gh,
    CommonSEGD::GeneralHeader2 gh2,
    vector<vector<CommonSEGD::ChannelSetHeader>> ch_sets,
    vector<shared_ptr<CommonSEGD::AdditionalGeneralHeader>> add_ghs,
    vector<vector<char>> extd_hdrs, vector<vector<char>> extl_hdrs,
    vector<map<uint32_t, pair<string, Trace::Header::ValueType>>> trc_hdr_ext)
    : OSEGD { move(file_name), move(gh), move(gh2), {}, move(ch_sets),
	   	move(add_ghs), move(extd_hdrs), move(extl_hdrs), move(trc_hdr_ext) }
    , pimpl { make_unique<Impl>(*this) }
{
}

OSEGDRev2_1::~OSEGDRev2_1() = default;
} // namespace sedaman
