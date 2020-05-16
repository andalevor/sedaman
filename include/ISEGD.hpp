///
/// \file ISegd.hpp
/// \brief header file with ISegd class declaration
///
/// \author andalevor
///
/// \date   2020/05/04
///

#ifndef SEDAMAN_ISEGD_HPP
#define SEDAMAN_ISEGD_HPP

#include "CommonSEGD.hpp"
#include "Trace.hpp"

///
/// \namespace sedaman
/// \brief General namespace for sedaman library.
///
namespace sedaman {
///
/// \class ISegd
/// \brief Class for SEGD reading.
/// Defines methods to read information from SEGD files.
///
class ISEGD : CommonSEGD {
public:
    ///
    /// \param file_name Name of file to read from.
    ///
    explicit ISEGD(std::string file_name);
    ///
    /// \brief General header getter
    /// \return return constant reference to general header
    ///
    GeneralHeader const& general_header();
    virtual ~ISEGD();

private:
    class Impl;
    std::unique_ptr<Impl> pimpl;
};
} // namespace sedaman

#endif // SEDAMAN_ISEGD_HPP
