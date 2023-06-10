///
/// @file OSEGY.hpp
/// @author Andrei Voronin (andalevor@gmail.com)
/// \brief 
/// @version 0.1
/// \date 2020-04-06
/// 
/// @copyright Copyright (c) 2020
/// 
///

#ifndef SEDAMAN_OSEGY_HPP
#define SEDAMAN_OSEGY_HPP

#include "CommonSEGY.hpp"
#include "Trace.hpp"

///
/// \brief General namespace for sedaman library.
/// \namespace sedaman
///
/// 
namespace sedaman {
///
/// \brief Abstract class for SEGY writing.
/// Declares common methods for all revisions to write information to SEGY
/// files.
/// \class OSEGY
///
/// 
class OSEGY {
public:
    ///
    /// \brief Construct a new OSEGY object
    /// 
    /// \param file_name Name of file to write to.
    /// \param bin_header Could be used to set write parameters.
    /// \param add_tr_hdrs_map Additional trace header maps.
    /// 
    /// \throws std::ifstream::failure In case of file operations falure.
    /// \throws sedaman::Exception
    ///
    OSEGY(std::string file_name, CommonSEGY::BinaryHeader bin_header,
        std::vector<std::pair<std::string, std::map<uint32_t,
	   	std::pair<std::string, Trace::Header::ValueType>>>> tr_hdrs_map);
    ///
    /// \brief Writes trace to the end of file.
    /// 
    /// \param tr Trace to write.
    ///
    virtual void write_trace(Trace& tr) = 0;
    virtual ~OSEGY();

protected:
    CommonSEGY& common();
    void assign_raw_writers();
    void assign_sample_writer();
    void assign_bytes_per_sample();
    void write_bin_header();
    void write_ext_text_headers();
    void write_trailer_stanzas();
    void write_trace_header(Trace::Header const& hdr);
    void write_additional_trace_headers(Trace::Header const& hdr);
    void write_trace_samples_fix(Trace const& t);
    void write_trace_samples_var(Trace const& t);

private:
    class Impl;
    std::unique_ptr<Impl> pimpl;
};
} // namespace sedaman

#endif // SEDAMAN_OSEGY_HPP
