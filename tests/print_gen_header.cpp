#include "ISegd.hpp"
#include <iomanip>
#include <iostream>

template <typename T>
void print(T v, std::string s)
{
    std::cout << std::setw(20) << +v << " = " << s << '\n';
}

int main(int argc, char* argv[])
{
    if (argc < 2)
        return 1;
    try {
        sedaman::ISegd segd(argv[1]);
        auto gen_hdr = segd.general_header();
        using name = sedaman::CommonSegd::GeneralHeader::Name;
        auto name_as_string = sedaman::CommonSegd::GeneralHeader::name_as_string;
        print(gen_hdr.file_number, name_as_string(name::FILE_NUMBER));
        print(gen_hdr.format_code, name_as_string(name::FORMAT_CODE));
        print(gen_hdr.gen_const, name_as_string(name::GENERAL_CONSTANTS));
        print(gen_hdr.year, name_as_string(name::YEAR));
        print(gen_hdr.day, name_as_string(name::DAY));
        print(gen_hdr.hour, name_as_string(name::HOUR));
        print(gen_hdr.minute, name_as_string(name::MINUTE));
        print(gen_hdr.second, name_as_string(name::SECOND));
        print(gen_hdr.manufac_code, name_as_string(name::MANUFACTURERS_CODE));
        print(gen_hdr.manufac_num, name_as_string(name::MANUFACTURERS_NUMBER));
        print(gen_hdr.bytes_per_scan, name_as_string(name::BYTES_PER_SCAN));
        print(gen_hdr.base_scan_int, name_as_string(name::BASE_SCAN_INTERVAL));
        print(gen_hdr.polarity, name_as_string(name::POLARITY));
        print(gen_hdr.scans_per_block, name_as_string(name::SCANS_PER_BLOCK));
        print(gen_hdr.record_type, name_as_string(name::RECORD_TYPE));
        print(gen_hdr.record_length, name_as_string(name::RECORD_LENGTH));
        print(gen_hdr.scan_types_per_record, name_as_string(name::SCAN_TYPES_PER_RECORD));
        print(gen_hdr.channel_sets_per_scan_type, name_as_string(name::CHANNEL_SETS_PER_SCAN_TYPE));
        print(gen_hdr.skew_blocks, name_as_string(name::SKEW_BLOCKS));
        print(gen_hdr.extended_hdr_blocks, name_as_string(name::EXTENDED_HEADER_BLOCKS));
        print(gen_hdr.external_hdr_blocks, name_as_string(name::EXTERNAL_HEADER_BLOCKS));
    } catch (std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
    return 0;
}
