///
/// \file CommonSegd.hpp
/// \brief header file with CommonSegd class declaration
///
/// \author andalevor
///
/// \date   2020/05/04
///

#ifndef SEDAMAN_COMMON_SEGD_HPP
#define SEDAMAN_COMMON_SEGD_HPP

#include <fstream>
#include <memory>
#include <string>
#include <vector>

///
/// \namespace sedaman
/// \brief General namespace for sedaman library.
///
namespace sedaman {
///
/// \class CommonSegd
/// \brief Class with common SEGD parts.
/// Holds common data and members for ISegd and OSegd classes.
/// \see ISegd
/// \see OSegd
///
class CommonSEGD {
public:
    class GeneralHeader {
    public:
        static constexpr int SIZE = 32;
        int file_number;
        int format_code;
        long long gen_const;
        int year;
        int day;
        int hour;
        int minute;
        int second;
        int manufac_code;
        int manufac_num;
        long bytes_per_scan;
        double base_scan_int;
        int polarity;
        long scans_per_block;
        int record_type;
        int record_length;
        int scan_types_per_record;
        int channel_sets_per_scan_type;
        int skew_blocks;
        int extended_hdr_blocks;
        int external_hdr_blocks;
        ///
        /// \enum
        /// \brief Constants to use with names.
        /// \see names
        ///
        enum class Name {
            FILE_NUMBER,
            FORMAT_CODE,
            GENERAL_CONSTANTS,
            YEAR,
            DAY,
            HOUR,
            MINUTE,
            SECOND,
            MANUFACTURERS_CODE,
            MANUFACTURERS_NUMBER,
            BYTES_PER_SCAN,
            BASE_SCAN_INTERVAL,
            POLARITY,
            SCANS_PER_BLOCK,
            RECORD_TYPE,
            RECORD_LENGTH,
            SCAN_TYPES_PER_RECORD,
            CHANNEL_SETS_PER_SCAN_TYPE,
            SKEW_BLOCKS,
            EXTENDED_HEADER_BLOCKS,
            EXTERNAL_HEADER_BLOCKS
        };
        static char const* name_as_string(Name n);
    };

protected:
    ///
    /// \param file_name Name of file.
    /// \param mode Choose input or output.
    ///
    CommonSEGD(std::string file_name, std::fstream::openmode mode);
    ///
    /// \brief p_file file handler getter
    /// \return reference to a file handler
    ///
    std::fstream& p_file();
    ///
    /// \brief p_general_header general header getter
    /// \return reference to general header
    ///
    GeneralHeader& p_general_header();
    ///
    /// \brief p_gen_hdr_buf access to a buffer for general header
    /// \return reference to buffer
    ///
    std::vector<char>& p_gen_hdr_buf();
    virtual ~CommonSEGD();

private:
    class Impl;
    std::unique_ptr<Impl> pimpl;
};
} // namespace sedaman

#endif // SEDAMAN_COMMON_SEGD_HPP
