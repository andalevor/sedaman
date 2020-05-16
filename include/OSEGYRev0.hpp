///
/// \file OSegyRev0.hpp
/// \brief header file with OSegyRev0 class declaration
///
/// \author andalevor
///
/// \date   2020/04/16
///

#ifndef SEDAMAN_OSEGYREV0_HPP
#define SEDAMAN_OSEGYREV0_HPP

#include "OSEGY.hpp"

///
/// \namespace sedaman
/// \brief General namespace for sedaman library.
///
namespace sedaman {
///
/// \class OSegyRev0
/// \brief Class for SEGY reading.
/// Defines methods to write information from SEGY files.
///
class OSEGYRev0 : public OSEGY {
public:
    ///
    /// \param file_name Name of SEGY file.
    /// \param text_header SEGY text header.
    /// \param bin_header Could be used to set some files in binary header.
    /// \throws std::ifstream::failure In case of file operations falure
    /// \throws sedaman::Exception
    /// \brief Creates SEGY rev 0 object.
    /// Size of text header must be 3200 bytes.
    /// Text header should be set in ebcdic encoding.
    /// CommonSegy::ascii_to_ebcdic could be used for transformation.
    /// By default will be used text header from standard.
    ///
    OSEGYRev0(std::string file_name, std::string text_header = {},
        CommonSEGY::BinaryHeader bin_header = {});
    ///
    /// \param tr Trace to write.
    /// \brief Writes trace to the end of file.
    ///
    void write_trace(Trace& trace) override;
    virtual ~OSEGYRev0();

private:
    class Impl;
    std::unique_ptr<Impl> pimpl;
};
} // namespace sedaman

#endif // SEDAMAN_OSEGYREV0_HPP
