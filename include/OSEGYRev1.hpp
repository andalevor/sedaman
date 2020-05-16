///
/// \file OSegyRev1.hpp
/// \brief header file with OSegyRev1 class declaration
///
/// \author andalevor
///
/// \date   2020/04/21
///

#ifndef SEDAMAN_OSEGYREV1_HPP
#define SEDAMAN_OSEGYREV1_HPP

#include "OSEGY.hpp"

///
/// \namespace sedaman
/// \brief General namespace for sedaman library.
///
namespace sedaman {
///
/// \class OSegyRev1
/// \brief Class for SEGY reading.
/// Defines methods to write information from SEGY files.
///
class OSEGYRev1 : public OSEGY {
public:
    ///
    /// \param file_name Name of SEGY file.
    /// \param text_headers Vector with SEGY text header and extended text headers.
    /// \param bin_header Could be used to set some files in binary header.
    /// \throws std::ifstream::failure In case of file operations falure
    /// \throws sedaman::Exception
    /// \brief Creates SEGY rev 1 object.
    /// Size of text header must be 3200 bytes.
    /// First string in vector will be the main text header. Other will be considered extended.
    /// CommonSegy::ascii_to_ebcdic could be used for transformation.
    /// By default will be used text header from standard.
    ///
    OSEGYRev1(std::string file_name, std::vector<std::string> text_headers = {},
        CommonSEGY::BinaryHeader bin_header = {});
    ///
    /// \param tr Trace to write.
    /// \brief Writes trace to the end of file.
    ///
    void write_trace(Trace& trace) override;
    virtual ~OSEGYRev1();

private:
    class Impl;
    std::unique_ptr<Impl> pimpl;
};
} // namespace sedaman

#endif // SEDAMAN_OSEGYREV1_HPP
