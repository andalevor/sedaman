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
    /// \return return constant reference to second general header
    ///
    GeneralHeader general_header();
    ///
    /// \brief General header 2 getter
    /// \return return second general header or std::nullopt if SEGD revision less than 1.0
    ///
    std::optional<GeneralHeader2> general_header2();
    ///
    /// \brief General header N getter
    /// \return return second general header or std::nullopt if SEGD revision less than 1.0
    ///
    std::optional<GeneralHeaderN> general_headerN();
    ///
    /// \brief General header 3 getter
    /// \return return third general header or std::nullopt if SEGD revision less than 3.0
    ///
    std::optional<GeneralHeader3> general_header3();
    ///
    /// \brief General header 3 getter
    /// \return return third general header or std::nullopt if SEGD revision less than 3.0
    ///
    std::optional<GeneralHeaderVes> general_header_ves();
    virtual ~ISEGD();

private:
    class Impl;
    std::unique_ptr<Impl> pimpl;
};
} // namespace sedaman

#endif // SEDAMAN_ISEGD_HPP
