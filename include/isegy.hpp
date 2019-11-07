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

#include "segy.hpp"

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
class isegy : public segy {
public:
    ///
    /// \param file_name Name of SEGY file.
    /// \param bin_hdr (optional)SEGY binary header.
    /// \param text_hdr (optional)SEGY text header.
    /// \throws std::ifstream::failure In case of file operations falure
    ///
    isegy(std::string const &file_name);
    ///
    /// \param file_name Name of SEGY file.
    /// \param bin_hdr (optional)SEGY binary header.
    /// \param text_hdr (optional)SEGY text header.
    /// \throws std::ifstream::failure In case of file operations falure
    ///
    isegy(std::string &&file_name);
    ///
    /// \fn text_hdr
    /// \brief segy text header getter
    /// \return text header
    ///
    std::string const &text_header();
    virtual ~isegy();
private:
    class impl;
    std::experimental::propagate_const<std::unique_ptr<impl>> pimpl;
};
} // namespace sedaman

#endif // SEDAMAN_ISEGY_HPP
