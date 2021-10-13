///
/// @file OSEGD.hpp
/// @author Andrei Voronin (andalevor@gmail.com)
/// \brief 
/// @version 0.1
/// \date 2020-08-24
/// 
/// @copyright Copyright (c) 2020
/// 
///
#ifndef SEDAMAN_OSEGD_HPP
#define SEDAMAN_OSEGD_HPP

#include "CommonSEGD.hpp"
#include "Trace.hpp"
///
/// \brief General namespace for sedaman library.
/// \namespace sedaman
/// 
///
namespace sedaman {
///
/// \brief Abstract class for SEGD writing.
/// Declares common methods for all revisions to write information to SEGD
/// files.
/// \class OSEGD
/// 
///
class OSEGD {
public:
    ///
    /// \brief Construct a new OSEGD object
    /// 
    /// \param file_name Name of SEGD file.
    /// \param gh General header.
    /// \param gh2 General header 2.
    /// \param gh3 General header 2
    /// \param ch_sets Channel sets
    /// \param add_ghs Additional general headers
    /// \param extended_headers Extended headers
    /// \param external_headers External headers
    /// \param trc_hdr_ext Trace header extension
    /// 
    /// \throws std::ifstream::failure In case of file operations falure
    /// \throws sedaman::Exception
    ///
    OSEGD(std::string file_name, CommonSEGD::GeneralHeader gh,
        CommonSEGD::GeneralHeader2 gh2, CommonSEGD::GeneralHeader3 gh3,
        std::vector<std::vector<CommonSEGD::ChannelSetHeader>> ch_sets,
        std::vector<std::shared_ptr<CommonSEGD::AdditionalGeneralHeader>>
        add_ghs = {},
        std::vector<std::vector<char>> extended_headers = {},
        std::vector<std::vector<char>> external_headers = {},
        std::vector<std::map<uint32_t, std::pair<std::string,
        Trace::Header::ValueType>>> trc_hdr_ext = {});
    ///
    /// \brief Writes trace to the end of file.
    /// 
    /// \param tr Trace to write.
    ///
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
    void write_ext_trace_headers(Trace::Header const& hdr);
    void write_trace_samples(Trace const& trc);
    uint64_t chans_in_record();

private:
    class Impl;
    std::unique_ptr<Impl> pimpl;
};
} // namespace sedaman

#endif // SEDAMAN_OSEGD_HPP
