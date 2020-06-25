/// \file ISEGY.hpp
/// \brief header file with ISEGY class declaration
/// \author andalevor
/// \date   2019/06/17

#ifndef SEDAMAN_ISEGY_HPP
#define SEDAMAN_ISEGY_HPP

#include "CommonSEGY.hpp"
#include "Trace.hpp"

/// \namespace sedaman
/// \brief General namespace for sedaman library.
namespace sedaman
{
    /// \class ISEGY
    /// \brief Class for SEGY reading.
    /// Defines methods to read information from SEGY files.
    class ISEGY
    {
    public:
        /// \param file_name Name of SEGY file.
        /// \throws std::ifstream::failure In case of file operations falure
        /// \throws sedaman::Exception
        ISEGY(std::string file_name, std::vector<std::map<uint32_t, std::pair<std::string, CommonSEGY::TrHdrValueType>>> tr_hdr_over = {},
              std::vector<std::pair<std::string, std::map<uint32_t, std::pair<std::string, CommonSEGY::TrHdrValueType>>>> add_hdr_map = {});
        /// \param file_name Name of SEGY file.
        /// \param binary_header Could be used to override values in binary header.
        /// \throws std::ifstream::failure In case of file operations falure
        /// \throws sedaman::Exception
        ISEGY(std::string file_name, CommonSEGY::BinaryHeader binary_header,
              std::vector<std::map<uint32_t, std::pair<std::string, CommonSEGY::TrHdrValueType>>> tr_hdr_over = {},
              std::vector<std::pair<std::string, std::map<uint32_t, std::pair<std::string, CommonSEGY::TrHdrValueType>>>> add_hdr_map = {});
        /// \param file_name Name of SEGY file.
        /// \throws std::ifstream::failure In case of file operations falure
        /// \throws sedaman::Exception
        /// \brief creates ISegy instance internally and returns binary header.
        /// Could be used to get binary header from file to override some values.
        static CommonSEGY::BinaryHeader read_binary_header(std::string file_name);
        /// \brief segy text headers getter
        /// \return vector with text headers
        std::vector<std::string> const &text_headers();
        /// \brief segy trailer stanzas getter
        /// \return vector with text headers
        std::vector<std::string> const &trailer_stanzas();
        /// \brief segy binary header getter
        /// \return binary header
        CommonSEGY::BinaryHeader const &binary_header();
        /// \brief checks for next trace in file
        /// \return true if there is at least one trace
        bool has_trace();
        /// \brief reads header, skips samples
        /// \return Trace::Header
        Trace::Header read_header();
        /// \brief reads one trace from file
        /// \return Trace
        Trace read_trace();
        virtual ~ISEGY();

    private:
        class Impl;
        std::unique_ptr<Impl> pimpl;
    };
} // namespace sedaman

#endif // SEDAMAN_ISEGY_HPP
