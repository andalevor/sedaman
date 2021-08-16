#include "ISEGD.hpp"
#include <cstring>
#include <iomanip>
#include <iostream>

class Printer {
public:
    explicit Printer(int s)
        : size { s }
    {
    }
    template <typename T>
    void operator()(T v, std::string s)
    {
        std::cout << std::left << std::setw(size) << std::setfill('.') << s
		   	<< " = " << +v << '\n';
    }

private:
    int size;
};

int get_max();

int main(int argc, char* argv[])
{
    if (argc < 2)
        return 1;
    try {
        sedaman::ISEGD segd(argv[1]);
        int max = get_max();
        Printer p(max);
        auto ch_sets = segd.channel_set_headers();
        using name = sedaman::CommonSEGD::ChannelSetHeader::Name;
        auto name_as_string =
		   	sedaman::CommonSEGD::ChannelSetHeader::name_as_string;
        decltype(ch_sets.size()) scan_counter = 0, ch_set_counter;
        for (auto& scan : ch_sets) {
            ch_set_counter = 0;
            std::cout << std::setw(max + 30) << std::setfill('*') << '\0';
            std::cout << "\nScan sequence number: "
			   	<< ++scan_counter << "\n\n";
            for (auto& hdr : scan) {
                std::cout << std::setw(max + 30) << std::setfill('=') << '\0';
                std::cout << "\nChannel set sequence number: "
				   	<< ++ch_set_counter << "\n";
                p(hdr.scan_type_number,
				  name_as_string(name::SCAN_TYPE_NUMBER));
                p(hdr.channel_set_number,
				  name_as_string(name::CHANNEL_SET_NUMBER));
                p(hdr.channel_type, name_as_string(name::CHANNEL_TYPE));
                p(hdr.channel_set_start_time,
				  name_as_string(name::CHANNEL_SET_START_TIME));
                p(hdr.channel_set_end_time,
				  name_as_string(name::CHANNEL_SET_END_TIME));
                p(hdr.descale_multiplier,
				  name_as_string(name::DESCALE_MULTIPLIER));
                p(hdr.number_of_channels,
				  name_as_string(name::NUMBER_OF_CHANNELS));
                p(hdr.subscans_per_ch_set,
				  name_as_string(name::SUBSCANS_PER_CH_SET));
                p(hdr.channel_gain, name_as_string(name::CHANNEL_GAIN));
                p(hdr.alias_filter_freq,
				  name_as_string(name::ALIAS_FILTER_FREQ));
                p(hdr.alias_filter_slope,
				  name_as_string(name::ALIAS_FILTER_SLOPE));
                p(hdr.low_cut_filter_freq,
				  name_as_string(name::LOW_CUT_FILTER_FREQ));
                p(hdr.low_cut_filter_slope,
				  name_as_string(name::LOW_CUT_FILTER_SLOPE));
                p(hdr.first_notch_filter,
				  name_as_string(name::FIRST_NOTCH_FILTER));
                p(hdr.second_notch_filter,
				  name_as_string(name::SECOND_NOTCH_FILTER));
                p(hdr.third_notch_filter,
				  name_as_string(name::THIRD_NOTCH_FILTER));
                p(hdr.ext_ch_set_num, name_as_string(name::EXT_CH_SET_NUM));
                p(hdr.ext_hdr_flag, name_as_string(name::EXT_HDR_FLAG));
                p(hdr.trc_hdr_ext, name_as_string(name::TRC_HDR_EXT));
                p(hdr.vert_stack, name_as_string(name::VERT_STACK));
                p(hdr.streamer_no, name_as_string(name::STREAMER_NO));
                p(hdr.array_forming, name_as_string(name::ARRAY_FORMING));
                if (segd.general_header().add_gen_hdr_blocks &&
				   	segd.general_header2()->segd_rev_major > 2) {
                    p(*hdr.number_of_samples(),
					  name_as_string(name::NUMBER_OF_SAMPLES));
                    p(*hdr.samp_int(), name_as_string(name::SAMPLE_INTERVAL));
                    p(*hdr.filter_phase(), name_as_string(name::FILTER_PHASE));
                    p(*hdr.physical_unit(),
					  name_as_string(name::PHYSICAL_UNIT));
                    p(*hdr.filter_delay(), name_as_string(name::FILTER_DELAY));
                    p(hdr.description()->data(),
					  name_as_string(name::DESCRIPTION));
                }
                std::cout << '\n';
            }
        }
    } catch (std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}

int get_max()
{
    int result = 0;
    for (int i = static_cast<int>(sedaman::CommonSEGD::ChannelSetHeader::Name::
								  SCAN_TYPE_NUMBER);
         i != static_cast<int>(sedaman::CommonSEGD::ChannelSetHeader::Name::
							   DESCRIPTION); ++i) {
        int len = strlen(sedaman::CommonSEGD::ChannelSetHeader::
						 name_as_string(static_cast<sedaman::CommonSEGD::
										ChannelSetHeader::Name>(i)));
        if (result < len)
            result = len;
    }
    return result;
}
