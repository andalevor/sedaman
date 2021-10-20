///
/// @file ISEGY.hpp
/// @author Andrei Voronin (andalevor@gmail.com)
/// \brief header file with ISEGY class declaration
/// @version 0.1
/// \date 2019-06-17
/// 
/// @copyright Copyright (c) 2019
/// 
///

#ifndef SEDAMAN_ISEGY_HPP
#define SEDAMAN_ISEGY_HPP

#include "CommonSEGY.hpp"
#include "Trace.hpp"

///
/// \brief General namespace for sedaman library.
/// \namespace sedaman
///
/// 
namespace sedaman {
///
/// \brief Class for SEGY reading.
/// Defines methods to read information from SEGY files.
/// \class ISEGY
///
/// 
class ISEGY {
public:
    ///
    /// \brief Construct a new ISEGY object
    /// 
    /// \param file_name Name of SEGY file.
    /// \param tr_hdr_over Could be used to override trace header schema from
    /// standard
    /// \param add_hdr_map Could be used to add arbitrary information to
    /// trace headers
    ///
    /// \throws std::ifstream::failure In case of file operations falure
    /// \throws sedaman::Exception
    ///
    ISEGY(std::string file_name, std::vector<std::map<uint32_t,
		  std::pair<std::string, Trace::Header::ValueType>>> tr_hdr_over =
		  {}, std::vector<std::pair<std::string, std::map<uint32_t,
		  std::pair<std::string, Trace::Header::ValueType>>>> add_hdr_map =
		  {});
    ///
    /// \brief Construct a new ISEGY object
    /// 
    /// \param file_name Name of SEGY file.
    /// \param binary_header Could be used to override values in binary header.
    /// \param tr_hdr_over Could be used to override trace header schema from
    /// standard
    /// \param add_hdr_map Could be used to add arbitrary information to
    /// trace headers
    /// 
    /// \throws std::ifstream::failure In case of file operations falure
    /// \throws sedaman::Exception
    ///
    ISEGY(std::string file_name, CommonSEGY::BinaryHeader binary_header,
        std::vector<std::map<uint32_t, std::pair<std::string,
	   	Trace::Header::ValueType>>> tr_hdr_over = {},
        std::vector<std::pair<std::string, std::map<uint32_t,
	   	std::pair<std::string, Trace::Header::ValueType>>>> add_hdr_map =
	   	{});
    ///
    /// \brief creates ISEGY instance internally and returns binary header.
    /// Could be used to get binary header from file to override some values.
    /// 
    /// \param file_name Name of SEGY file.
    /// \return CommonSEGY::BinaryHeader 
    /// 
    /// \throws std::ifstream::failure In case of file operations falure
    /// \throws sedaman::Exception
    ///
    static CommonSEGY::BinaryHeader read_binary_header(std::string file_name);
    ///
    /// \brief segy text headers getter
    /// 
    /// \return std::vector<std::string> const& 
    ///
    std::vector<std::string> const& text_headers();
    ///
    /// \brief segy trailer stanzas getter
    /// 
    /// \return std::vector<std::string> const& 
    ///
    std::vector<std::string> const& trailer_stanzas();
    ///
    /// \brief segy binary header getter
    /// 
    /// \return CommonSEGY::BinaryHeader const& 
    ///
    CommonSEGY::BinaryHeader const& binary_header();
    ///
    /// \brief checks for next trace in file
    /// 
    /// \return true 
    /// \return false 
    ///
    virtual bool has_trace();
    ///
    /// \brief reads header, skips samples
    /// 
    /// \return Trace::Header 
    ///
    virtual Trace::Header read_header();
    ///
    /// \brief reads one trace from file
    /// 
    /// \return Trace 
    ///
    virtual Trace read_trace();
    virtual ~ISEGY();

protected:
    CommonSEGY& common();

private:
    class Impl;
    std::unique_ptr<Impl> pimpl;
};
} // namespace sedaman

#endif // SEDAMAN_ISEGY_HPP
