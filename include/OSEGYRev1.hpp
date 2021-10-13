///
/// @file OSEGYRev1.hpp
/// @author Andrei Voronin (andalevor@gmail.com)
/// \brief 
/// @version 0.1
/// \date 2020-04-21
/// 
/// @copyright Copyright (c) 2020
/// 
///

#ifndef SEDAMAN_OSEGYREV1_HPP
#define SEDAMAN_OSEGYREV1_HPP

#include "OSEGY.hpp"
///
/// \brief General namespace for sedaman library.
/// \namespace sedaman
/// 
///
namespace sedaman {
///
/// \brief Class for SEGY reading.
/// Defines methods to write information from SEGY files.
/// \class OSEGYRev1
/// 
///
class OSEGYRev1 : public OSEGY {
public:
    /// \param file_name Name of SEGY file.
    /// \param text_headers Vector with SEGY text header and extended text
	/// headers.
    /// \param bin_header Could be used to set write parameters.
    /// \throws std::ifstream::failure In case of file operations falure
    /// \throws sedaman::Exception
    /// \brief Creates SEGY rev 1 object.
    /// Size of text header must be 3200 bytes.
    /// First string in vector will be the main text header. Other will be
	/// considered extended.
    /// CommonSegy::ascii_to_ebcdic could be used for transformation.
    /// By default will be used text header from standard.
    
    ///
    /// \brief Creates SEGY rev 1 object.
    /// Size of text header must be 3200 bytes.
    /// First string in vector will be the main text header. Other will be
	/// considered extended.
    /// CommonSegy::ascii_to_ebcdic could be used for transformation.
    /// By default will be used text header from standard.
    /// 
    /// \param file_name Name of SEGY file.
    /// \param text_headers Vector with SEGY text header and extended text
	/// headers.
    /// \param bin_header Could be used to set some files in binary header.
    ///
    /// \throws std::ifstream::failure In case of file operations falure
    /// \throws sedaman::Exception
    /// 
    OSEGYRev1(std::string file_name, std::vector<std::string> text_headers =
			  {}, CommonSEGY::BinaryHeader bin_header = {});
    ///
    /// \brief Writes trace to the end of file.
    /// 
    /// \param trace Trace to write.
    ///
    void write_trace(Trace& trace) override;
    virtual ~OSEGYRev1();

private:
    class Impl;
    std::unique_ptr<Impl> pimpl;
};
} // namespace sedaman

#endif // SEDAMAN_OSEGYREV1_HPP
