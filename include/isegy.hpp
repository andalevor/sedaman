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

namespace sedaman {
class isegy : public segy {
public:
    ///
    /// \param file_name Name of SEGY file.
    /// \param bin_hdr (optional)SEGY binary header.
    /// \param text_hdr (optional)SEGY text header.
    ///
    isegy(std::string const &file_name);
    ///
    /// \param file_name Name of SEGY file.
    /// \param bin_hdr (optional)SEGY binary header.
    /// \param text_hdr (optional)SEGY text header.
    ///
    isegy(std::string &&file_name);
    virtual ~isegy();
private:
    class impl;
    std::experimental::propagate_const<std::unique_ptr<impl>> pimpl;
};
}

#endif // SEDAMAN_ISEGY_HPP
