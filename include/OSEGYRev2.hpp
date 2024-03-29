///
/// @file OSEGYRev2.hpp
/// @author Andrei Voronin (andalevor@gmail.com)
/// \brief 
/// @version 0.1
/// \date 2020-04-23
/// 
/// @copyright Copyright (c) 2020
/// 
///
#ifndef SEDAMAN_OSEGYREV2_HPP
#define SEDAMAN_OSEGYREV2_HPP

#include "OSEGY.hpp"
///
/// \brief General namespace for sedaman library.
/// \namespace sedaman
/// 
///
namespace sedaman {
///
/// \brief Class for SEGY reading.
/// Defines methods to write information to SEGY files.
/// \class OSEGYRev2
/// 
///
class OSEGYRev2 : public OSEGY {
public:
    ///
    /// \brief Creates SEGY rev 2 object.
    /// Size of text header must be 3200 bytes.
    /// First string in vector will be the main text header. Other will be
	/// considered extended.
    /// CommonSegy::ascii_to_ebcdic could be used for transformation.
    /// By default will be used text header from standard.
    /// For additional trace headers reading add_hdr_map could be set.
    /// Key - offset starting second additional trace header. Offset starts
	/// from 0.
    /// Header value should not overlap 240 bytes.
    /// Maps value is a pair of header values name and header values type.
    /// 
    /// \param file_name Name of SEGY file.
    /// \param text_headers Vector with SEGY text header and extended text
	/// headers.
    /// \param bin_header Could be used to set write parameters.
    /// \param trailer_stanzas Vector with SEGY trailer stanzas.
    /// \param add_hdr_map Map for additional trace headers reading.
    ///
    /// \throws std::ifstream::failure In case of file operations falure
    /// \throws sedaman::Exception
    /// 
    OSEGYRev2(std::string file_name, std::vector<std::string> text_headers =
			  {}, CommonSEGY::BinaryHeader bin_header = {},
	  		  std::vector<std::string> trailer_stanzas = {},
	  		  std::vector<std::pair<std::string, std::map<uint32_t,
			  std::pair<std::string, Trace::Header::ValueType>>>> tr_hdr_map =
			  CommonSEGY::default_trace_header);
    ///
    /// \brief Writes trace to the end of file.
    /// 
    /// \param trace Trace to write.
    ///
    void write_trace(Trace& trace) override;
    virtual ~OSEGYRev2();

private:
    class Impl;
    std::unique_ptr<Impl> pimpl;
};
} // namespace sedaman

#endif // SEDAMAN_OSEGYREV2_HPP
