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

#include "CommonSegd.hpp"
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
class ISegd : CommonSegd {
public:
    ///
    /// \param file_name Name of file to read from.
    ///
    explicit ISegd(std::string file_name);
    virtual ~ISegd();

private:
    class Impl;
    std::unique_ptr<Impl> pimpl;
};
} // namespace sedaman

#endif // SEDAMAN_ISEGD_HPP
