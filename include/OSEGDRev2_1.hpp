/// \file OSEGDRev2_1.hpp
/// \brief header file with OSEGDRev2.1 class declaration
/// \author andalevor
/// \date   2020/08/24

#ifndef SEDAMAN_OSEGDRev2_1_HPP
#define SEDAMAN_OSEGDRev2_1_HPP

#include "OSEGD.hpp"

/// \namespace sedaman
/// \brief General namespace for sedaman library.
namespace sedaman {
/// \class OSEGDRev2_1
/// \brief Class for SEGD revision 2.1 writing.
/// Declares methods to write information to SEGD files.
class OSEGDRev2_1 : public OSEGD {
public:
    /// \param file_name Name of SEGY file.
    /// \throws std::ifstream::failure In case of file operations falure
    /// \throws sedaman::Exception
    OSEGDRev2_1(std::string file_name, CommonSEGD::GeneralHeader gh,
        CommonSEGD::GeneralHeader2 gh2,
        std::vector<std::vector<CommonSEGD::ChannelSetHeader>> ch_sets,
        std::vector<std::unique_ptr<CommonSEGD::AdditionalGeneralHeader>>
	   	add_ghs = {});
    void write_trace(Trace& t) override;

private:
    class Impl;
    std::unique_ptr<Impl> pimpl;
};
} // namespace sedaman

#endif // SEDAMAN_OSEGDREV2_1_HPP
