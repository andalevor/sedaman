///
/// \file isegy.hpp
/// \brief header file with isegy class declaration
///
/// \author andalevor
///
/// \date   2019/06/17
///

#ifndef SEDAMAN_ISEGY_HPP
#define SEDAMAN_ISEGY_HPP

#include "common_segy.hpp"
#include "trace.hpp"

///
/// \namespace sedaman
/// \brief General namespace for sedaman library.
///
namespace sedaman {
///
/// \class isegy
/// \brief Class for SEGY reading.
/// Defines methods to read information from SEGY files.
///
class isegy {
public:
    ///
    /// \param file_name Name of SEGY file.
    /// \param bin_hdr (optional)SEGY binary header.
    /// \param text_hdr (optional)SEGY text header.
    /// \throws std::ifstream::failure In case of file operations falure
    ///
    isegy(std::string const &segy_name);
    ///
    /// \param file_name Name of SEGY file.
    /// \param bin_hdr (optional)SEGY binary header.
    /// \param text_hdr (optional)SEGY text header.
    /// \throws std::ifstream::failure In case of file operations falure
    ///
    isegy(std::string &&file_name);
    ///
    /// \fn text_header
    /// \brief segy text headers getter
    /// \return vector with text headers
    ///
    std::vector<std::string> const &text_headers() const;
    ///
    /// \fn trailer_stanzas
    /// \brief segy trailer stanzas getter
    /// \return vector with text headers
    ///
    std::vector<std::string> const &trailer_stanzas() const;
    ///
    /// \fn binary_header
    /// \brief segy binary header getter
    /// \return binary header
    ///
    common_segy::binary_header const &binary_header() const;
    ///
    /// \fn read_trace
    /// \brief reads one trace from file
    /// \return trace
    ///
    //trace read_trace();
    virtual ~isegy();
private:
    class impl;
    std::experimental::propagate_const<std::unique_ptr<impl>> pimpl;
};
} // namespace sedaman

#endif // SEDAMAN_ISEGY_HPP
