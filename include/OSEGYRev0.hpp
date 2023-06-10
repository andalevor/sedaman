///
/// @file OSEGYRev0.hpp
/// @author Andrei Voronin (andalevor@gmail.com)
/// \brief 
/// @version 0.1
/// \date 2020-04-16
/// 
/// @copyright Copyright (c) 2020
/// 
///
#ifndef SEDAMAN_OSEGYREV0_HPP
#define SEDAMAN_OSEGYREV0_HPP

#include "CommonSEGY.hpp"
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
/// \class OSEGYRev0
///
/// 
class OSEGYRev0 : public OSEGY {
public:
    ///
    /// \brief Construct a new OSEGYRev0 object
    /// Size of text header must be 3200 bytes.
    /// Text header should be set in ebcdic encoding.
    /// CommonSegy::ascii_to_ebcdic could be used for transformation.
    /// By default will be used text header from standard.
    /// 
    /// \param file_name Name of SEGY file.
    /// \param text_header SEGY text header.
    /// \param bin_header Could be used to set some files in binary header.
    ///
    /// \throws std::ifstream::failure In case of file operations falure
    /// \throws sedaman::Exception
    /// 
    OSEGYRev0(std::string file_name, std::string text_header = {},
        CommonSEGY::BinaryHeader bin_header = {},
	   	std::vector<std::pair<std::string, std::map<uint32_t,
	   	std::pair<std::string, Trace::Header::ValueType>>>> tr_hdrs_map =
		CommonSEGY::default_trace_header);
    ///
    /// \brief Writes trace to the end of file.
    /// 
    /// \param trace Trace to write.
    ///
    void write_trace(Trace& trace) override;
    virtual ~OSEGYRev0();

private:
    class Impl;
    std::unique_ptr<Impl> pimpl;
};
} // namespace sedaman

#endif // SEDAMAN_OSEGYREV0_HPP
