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
        std::cout << std::left << std::setw(size) << std::setfill('.') << s << " = " << +v << '\n';
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
        Printer p(get_max());
        auto gen_hdr = segd.general_header();
        using name = sedaman::CommonSEGD::GeneralHeader::Name;
        auto name_as_string = sedaman::CommonSEGD::GeneralHeader::name_as_string;
        std::cout << "General Header #1:\n";
        p(gen_hdr.file_number, name_as_string(name::FILE_NUMBER));
        p(gen_hdr.format_code, name_as_string(name::FORMAT_CODE));
        p(gen_hdr.gen_const, name_as_string(name::GENERAL_CONSTANTS));
        p(gen_hdr.year, name_as_string(name::YEAR));
        p(gen_hdr.add_gen_hdr_blocks, name_as_string(name::GENERAL_HEADER_BLOCKS));
        p(gen_hdr.day, name_as_string(name::DAY));
        p(gen_hdr.hour, name_as_string(name::HOUR));
        p(gen_hdr.minute, name_as_string(name::MINUTE));
        p(gen_hdr.second, name_as_string(name::SECOND));
        p(gen_hdr.manufac_code, name_as_string(name::MANUFACTURERS_CODE));
        p(gen_hdr.manufac_num, name_as_string(name::MANUFACTURERS_NUMBER));
        p(gen_hdr.bytes_per_scan, name_as_string(name::BYTES_PER_SCAN));
        p(gen_hdr.base_scan_int, name_as_string(name::BASE_SCAN_INTERVAL));
        p(gen_hdr.polarity, name_as_string(name::POLARITY));
        p(gen_hdr.scans_per_block, name_as_string(name::SCANS_PER_BLOCK));
        p(gen_hdr.record_type, name_as_string(name::RECORD_TYPE));
        p(gen_hdr.record_length, name_as_string(name::RECORD_LENGTH));
        p(gen_hdr.scan_types_per_record, name_as_string(name::SCAN_TYPES_PER_RECORD));
        p(gen_hdr.channel_sets_per_scan_type, name_as_string(name::CHANNEL_SETS_PER_SCAN_TYPE));
        p(gen_hdr.skew_blocks, name_as_string(name::SKEW_BLOCKS));
        p(gen_hdr.extended_hdr_blocks, name_as_string(name::EXTENDED_HEADER_BLOCKS));
        p(gen_hdr.external_hdr_blocks, name_as_string(name::EXTERNAL_HEADER_BLOCKS));
        if (gen_hdr.add_gen_hdr_blocks) {
            auto gen_hdr2 = *segd.general_header2();
            using name = sedaman::CommonSEGD::GeneralHeader2::Name;
            auto name_as_string = sedaman::CommonSEGD::GeneralHeader2::name_as_string;
            std::cout << "\nGeneral Header #2:\n";
            p(gen_hdr2.expanded_file_num, name_as_string(name::EXPANDED_FILE_NUMBER));
            p(gen_hdr2.ext_ch_sets_per_scan_type, name_as_string(name::EXT_CH_SETS_PER_SCAN_TYPE));
            p(gen_hdr2.extended_hdr_blocks, name_as_string(name::EXTEDNDED_HEADER_BLOCKS));
            if (gen_hdr2.segd_rev_major == 2)
                p(gen_hdr2.external_hdr_blocks, name_as_string(name::EXTERNAL_HEADER_BLOCKS));
            else
                p(gen_hdr2.extended_skew_blocks, name_as_string(name::EXTENDED_SKEW_BLOCKS));
            p(gen_hdr2.segd_rev_major, name_as_string(name::SEGD_REVISION_MAJOR));
            p(gen_hdr2.segd_rev_minor, name_as_string(name::SEGD_REVISION_MINOR));
            p(gen_hdr2.gen_trailer_num_of_blocks, name_as_string(name::EXPANDED_FILE_NUMBER));
            p(gen_hdr2.ext_record_len, name_as_string(name::EXTENDED_RECORD_LENGTH));
            if (gen_hdr2.segd_rev_major > 2) {
                p(gen_hdr2.record_set_number, name_as_string(name::RECORD_SET_NUMBER));
                p(gen_hdr2.ext_num_add_blks_in_gen_hdr, name_as_string(name::EXT_NUM_ADD_BLKS_IN_GEN_HDR));
                p(gen_hdr2.dominant_sampling_int, name_as_string(name::DOMINANT_SAMPLING_INT));
            }
            p(gen_hdr2.gen_hdr_block_num, name_as_string(name::GEN_HEADER_BLOCK_NUM));
            if (gen_hdr.add_gen_hdr_blocks > 1 && gen_hdr2.segd_rev_major < 3) {
                auto gen_hdrN = *segd.general_headerN();
                using name = sedaman::CommonSEGD::GeneralHeaderN::Name;
                auto name_as_string = sedaman::CommonSEGD::GeneralHeaderN::name_as_string;
                std::cout << "\nGeneral Header #N:\n";
                p(gen_hdrN.expanded_file_number, name_as_string(name::EXPANDED_FILE_NUMBER));
                p(gen_hdrN.sou_line_num, name_as_string(name::SOURCE_LINE_NUMBER));
                p(gen_hdrN.sou_point_num, name_as_string(name::SOURCE_POINT_NUMBER));
                p(gen_hdrN.sou_point_index, name_as_string(name::SOURCE_POINT_INDEX));
                p(gen_hdrN.phase_control, name_as_string(name::PHASE_CONTROL));
                p(gen_hdrN.type_vibrator, name_as_string(name::TYPE_VIBRATOR));
                p(gen_hdrN.phase_angle, name_as_string(name::PHASE_ANGLE));
                p(gen_hdrN.gen_hdr_block_num, name_as_string(name::GEN_HEADER_BLOCK_NUM));
                p(gen_hdrN.sou_set_num, name_as_string(name::SOURCE_SET_NUMBER));
            }
            if (gen_hdr2.segd_rev_major > 2) {
                auto gen_hdr3 = *segd.general_header3();
                using name = sedaman::CommonSEGD::GeneralHeader3::Name;
                auto name_as_string = sedaman::CommonSEGD::GeneralHeader3::name_as_string;
                std::cout << "\nGeneral Header #3:\n";
                p(gen_hdr3.time_zero, name_as_string(name::TIME_ZERO));
                p(gen_hdr3.record_size, name_as_string(name::RECORD_SIZE));
                p(gen_hdr3.data_size, name_as_string(name::DATA_SIZE));
                p(gen_hdr3.header_size, name_as_string(name::HEADER_SIZE));
                p(gen_hdr3.extd_rec_mode, name_as_string(name::EXTD_REC_MODE));
                p(gen_hdr3.rel_time_mode, name_as_string(name::REL_TIME_MODE));
                p(gen_hdr3.gen_hdr_block_num, name_as_string(name::GEN_HEADER_BLOCK_NUM));
            }
        }
    } catch (std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
    return 0;
}

int get_max()
{
    int result = 0;
    for (int i = static_cast<int>(sedaman::CommonSEGD::GeneralHeader::Name::FILE_NUMBER);
         i != static_cast<int>(sedaman::CommonSEGD::GeneralHeader::Name::EXTERNAL_HEADER_BLOCKS); ++i) {
        int len = strlen(sedaman::CommonSEGD::GeneralHeader::name_as_string(static_cast<sedaman::CommonSEGD::GeneralHeader::Name>(i)));
        if (result < len)
            result = len;
    }
    for (int i = static_cast<int>(sedaman::CommonSEGD::GeneralHeader2::Name::EXPANDED_FILE_NUMBER);
         i != static_cast<int>(sedaman::CommonSEGD::GeneralHeader2::Name::GEN_HEADER_BLOCK_NUM); ++i) {
        int len = strlen(sedaman::CommonSEGD::GeneralHeader2::name_as_string(static_cast<sedaman::CommonSEGD::GeneralHeader2::Name>(i)));
        if (result < len)
            result = len;
    }
    for (int i = static_cast<int>(sedaman::CommonSEGD::GeneralHeaderN::Name::EXPANDED_FILE_NUMBER);
         i != static_cast<int>(sedaman::CommonSEGD::GeneralHeaderN::Name::SOURCE_SET_NUMBER); ++i) {
        int len = strlen(sedaman::CommonSEGD::GeneralHeaderN::name_as_string(static_cast<sedaman::CommonSEGD::GeneralHeaderN::Name>(i)));
        if (result < len)
            result = len;
    }
    return result;
}
