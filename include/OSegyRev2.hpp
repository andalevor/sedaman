///
/// \file OSegyRev2.hpp
/// \brief header file with OSegyRev1 class declaration
///
/// \author andalevor
///
/// \date   2020/04/23
///

#ifndef SEDAMAN_OSEGYREV2_HPP
#define SEDAMAN_OSEGYREV2_HPP

#include "OSegy.hpp"

///
/// \namespace sedaman
/// \brief General namespace for sedaman library.
///
namespace sedaman {
///
/// \class OSegyRev2
/// \brief Class for SEGY reading.
/// Defines methods to write information from SEGY files.
///
class OSegyRev2 : public OSegy {
public:
    ///
    /// \param file_name Name of SEGY file.
    /// \param text_headers Vector with SEGY text header and extended text headers.
    /// \param bin_header Could be used to set some files in binary header.
    /// \param trailer_stanzas Vector with SEGY trailer stanzas.
    /// \param add_hdr_map Map for additional trace headers reading.
    /// \throws std::ifstream::failure In case of file operations falure
    /// \throws sedaman::Exception
    /// \brief Creates SEGY rev 2 object.
    /// Size of text header must be 3200 bytes.
    /// First string in vector will be the main text header. Other will be considered extended.
    /// CommonSegy::ascii_to_ebcdic could be used for transformation.
    /// By default will be used text header from standard.
    /// For additional trace headers reading add_hdr_map could be set.
    /// Key - offset starting second additional trace header. Offset starts from 0.
    /// Header value should not overlap 240 bytes.
    /// Maps value is a pair of header values name and header values type.
    ///
    OSegyRev2(std::string file_name, std::vector<std::string> text_headers = {},
        CommonSegy::BinaryHeader bin_header = {},
        std::vector<std::string> trailer_stanzas = {},
        std::vector<std::pair<std::string, std::map<uint32_t, std::pair<std::string, TrHdrValueType>>>> add_hdr_map = {});
    ///
    /// \param tr Trace to write.
    /// \brief Writes trace to the end of file.
    ///
    void write_trace(Trace& trace) override;
    virtual ~OSegyRev2();

private:
    class Impl;
    std::unique_ptr<Impl> pimpl;
};
} // namespace sedaman

#endif // SEDAMAN_OSEGYREV2_HPP
