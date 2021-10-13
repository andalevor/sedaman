///
/// @file OSEGDRev2_1.hpp
/// @author Andrei Voronin (andalevor@gmail.com)
/// \brief 
/// @version 0.1
/// \date 2020-08-24
/// 
/// @copyright Copyright (c) 2020
/// 
///
#ifndef SEDAMAN_OSEGDRev2_1_HPP
#define SEDAMAN_OSEGDRev2_1_HPP

#include "OSEGD.hpp"
///
/// \brief General namespace for sedaman library.
/// \namespace sedaman
/// 
///
namespace sedaman {
///
/// \brief Class for SEGD revision 2.1 writing.
/// Declares methods to write information to SEGD files.
/// \class OSEGDRev2_1
/// 
///
class OSEGDRev2_1 : public OSEGD {
public:
    ///
    /// \brief Construct a new osegdrev2 1 object
    /// 
    /// \param file_name Name of SEGD file.
    /// \param gh General header
    /// \param gh2 General header 2
    /// \param ch_sets Channel sets
    /// \param add_ghs Additional general headers
    /// \param extended_headers Extended headers
    /// \param external_headers External headers
    /// \param trc_hdr_ext Trace header extensions
    /// 
    /// \throws std::ifstream::failure In case of file operations falure
    /// \throws sedaman::Exception
    ///
    OSEGDRev2_1(std::string file_name, CommonSEGD::GeneralHeader gh,
        CommonSEGD::GeneralHeader2 gh2,
        std::vector<std::vector<CommonSEGD::ChannelSetHeader>> ch_sets,
        std::vector<std::shared_ptr<CommonSEGD::AdditionalGeneralHeader>>
	   	add_ghs = {},
        std::vector<std::vector<char>> extended_headers = {},
        std::vector<std::vector<char>> external_headers = {},
        std::vector<std::map<uint32_t, std::pair<std::string,
        Trace::Header::ValueType>>> trc_hdr_ext = {}
        );
    ///
    /// \brief Writes givent trace to file.
    /// 
    /// \param t Trace to write
    ///
    void write_trace(Trace& t) override;
    virtual ~OSEGDRev2_1();

private:
    class Impl;
    std::unique_ptr<Impl> pimpl;
};
} // namespace sedaman

#endif // SEDAMAN_OSEGDREV2_1_HPP
