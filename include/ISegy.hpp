///
/// \file ISegy.hpp
/// \brief header file with ISegy class declaration
///
/// \author andalevor
///
/// \date   2019/06/17
///

#ifndef SEDAMAN_ISEGY_HPP
#define SEDAMAN_ISEGY_HPP

#include "CommonSegy.hpp"
#include "Trace.hpp"

///
/// \namespace sedaman
/// \brief General namespace for sedaman library.
///
namespace sedaman {
///
/// \class ISegy
/// \brief Class for SEGY reading.
/// Defines methods to read information from SEGY files.
///
class ISegy {
public:
    ///
    /// \param file_name Name of SEGY file.
    /// \param bin_hdr (optional)SEGY binary header.
    /// \param text_hdr (optional)SEGY text header.
    /// \throws std::ifstream::failure In case of file operations falure
    ///
    explicit ISegy(std::string const& segy_name);
    ///
    /// \param file_name Name of SEGY file.
    /// \param bin_hdr (optional)SEGY binary header.
    /// \param text_hdr (optional)SEGY text header.
    /// \throws std::ifstream::failure In case of file operations falure
    ///
    explicit ISegy(std::string&& file_name);
    ///
    /// \fn text_header
    /// \brief segy text headers getter
    /// \return vector with text headers
    ///
    std::vector<std::string> const& text_headers() const;
    ///
    /// \fn trailer_stanzas
    /// \brief segy trailer stanzas getter
    /// \return vector with text headers
    ///
    std::vector<std::string> const& trailer_stanzas() const;
    ///
    /// \fn binary_header
    /// \brief segy binary header getter
    /// \return binary header
    ///
    CommonSegy::BinaryHeader const& binary_header() const;
    ///
    /// \fn has_next
    /// \brief checks for next trace in file
    /// \return true if there is at least one trace
    ///
    bool has_next();
    ///
    /// \fn read_header
    /// \brief reads header, skips samples
    /// \return Trace::Header
    ///
    Trace::Header read_header();
    ///
    /// \fn read_trace
    /// \brief reads one trace from file
    /// \return Trace
    ///
    Trace read_trace();
    ~ISegy();

private:
    class Impl;
    std::experimental::propagate_const<std::unique_ptr<Impl>> pimpl;
};
} // namespace sedaman

#endif // SEDAMAN_ISEGY_HPP
