/// \file OSEGD.hpp
/// \brief header file with OSEGD class declaration
/// \author andalevor
/// \date   2020/08/24

#ifndef SEDAMAN_OSEGD_HPP
#define SEDAMAN_OSEGD_HPP

#include "CommonSEGD.hpp"
#include "Trace.hpp"

/// \namespace sedaman
/// \brief General namespace for sedaman library.
namespace sedaman {
/// \class OSEGD
/// \brief Abstract class for SEGD writing.
/// Declares common methods for all revisions to write information to SEGD files.
class OSEGD {
public:
    /// \param file_name Name of SEGD file.
    /// \throws std::ifstream::failure In case of file operations falure
    /// \throws sedaman::Exception
    OSEGD(std::string file_name, CommonSEGD::GeneralHeader gh,
        CommonSEGD::GeneralHeader2 gh2, CommonSEGD::GeneralHeader3 gh3,
        std::vector<std::vector<CommonSEGD::ChannelSetHeader>> ch_sets,
        std::vector<std::unique_ptr<CommonSEGD::AdditionalGeneralHeader>> add_ghs = {});
    /// \param tr Trace to write.
    /// \brief Writes trace to the end of file.
    virtual void write_trace(Trace& tr) = 0;
    virtual ~OSEGD();

protected:
    CommonSEGD& common();
    void assign_raw_writers();
    void assign_sample_writers();
    void write_general_header1();
    void write_general_header2_rev2();
    void write_rev2_add_gen_hdrs();
    void write_general_header2_and_3();
    void write_rev3_add_gen_hdrs();
    void write_ch_set_hdr(CommonSEGD::ChannelSetHeader& hdr);
    void write_trace_header(Trace::Header const& hdr);
    void write_ext_trace_header(Trace::Header const& hdr);
    void write_trace_samples(Trace const& trc);
    uint64_t chans_in_record();

private:
    class Impl;
    std::unique_ptr<Impl> pimpl;
};
} // namespace sedaman

#endif // SEDAMAN_OSEGD_HPP
