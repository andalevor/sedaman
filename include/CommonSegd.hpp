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
class CommonSegd {
public:
    class GeneralHeader {
    public:
        static constexpr int SIZE = 32;
        int ffid;
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
        int base_scan_int;
        int polarity;
        long scans;
        int record_type;
        int record_length;
        int scan_types;
        int channel_sets;
        int skew_blocks;
        int extended_hdr_blocks;
        int external_hdr_blocks;
    };

protected:
    ///
    /// \param file_name Name of file.
    /// \param mode Choose input or output.
    ///
    CommonSegd(std::string file_name, std::fstream::openmode mode);
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
    virtual ~CommonSegd();

private:
    class Impl;
    std::unique_ptr<Impl> pimpl;
};
} // namespace sedaman

#endif // SEDAMAN_COMMON_SEGD_HPP
