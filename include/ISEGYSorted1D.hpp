///
/// @file ISEGYSorted1D.hpp
/// @author Andrei Voronin (andalevor@gmail.com)
/// \brief
/// @version 0.1
/// \date 2021-10-18
///
/// @copyright Copyright (c) 2021
///
///

#ifndef SEDAMAN_ISEGYSORTED1D_HPP
#define SEDAMAN_ISEGYSORTED1D_HPP

#include "CommonSEGY.hpp"
#include "ISEGY.hpp"
///
/// \brief General namespace for sedaman library.
/// \namespace sedaman
///
///
namespace sedaman {
///
/// \brief Class for SEGY reading by given sort order
/// \class ISEGYSorted
///
///
class ISEGYSorted1D : public ISEGY {
public:
    ///
    /// \brief Construct a new ISEGYSorted object
    ///
    /// \param file_name Name of SEGY file.
    /// \param hdr_name Name of header to sort by
    /// \param tr_hdr_over Could be used to override trace header schema from
    /// standard
    /// \param add_hdr_map Could be used to add arbitrary information to
    /// trace headers
    ///
    /// \throws std::ifstream::failure In case of file operations falure
    /// \throws sedaman::Exception
    ///
    ISEGYSorted1D(
		std::string file_name, std::string hdr_name,
		std::vector<std::pair<std::string,
		std::map<uint32_t, std::pair<std::string,
		Trace::Header::ValueType>>>>
            hdr_map = CommonSEGY::default_trace_header);
    ///
    /// \brief Construct a new ISEGYSorted object
    ///
    /// \param file_name Name of SEGY file.
    /// \param hdr_name Name of header to sort by
    /// \param binary_header Could be used to override values in binary header.
    /// \param tr_hdr_over Could be used to override trace header schema from
    /// standard
    /// \param add_hdr_map Could be used to add arbitrary information to
    /// trace headers
    ///
    /// \throws std::ifstream::failure In case of file operations falure
    /// \throws sedaman::Exception
    ///
    ISEGYSorted1D(
        std::string file_name, std::string hdr_name,
        CommonSEGY::BinaryHeader binary_header,
        std::vector<std::pair<std::string,
		std::map<uint32_t, std::pair<std::string,
		Trace::Header::ValueType>>>>
            hdr_map = CommonSEGY::default_trace_header);
    ///
    /// \brief checks for next trace in file
    ///
    /// \return true
    /// \return false
    ///
    virtual bool has_trace() override;
    ///
    /// \brief reads header, skips samples
    ///
    /// \return Trace::Header
    ///
    virtual Trace::Header read_header() override;
    ///
    /// \brief reads one trace from file
    ///
    /// \return Trace
    ///
    virtual Trace read_trace() override;
    ///
    /// \brief Get list of keys which could be used to get headers or traces
    ///
    /// \return std::vector<Trace::Header::Value>
    ///
    std::vector<Trace::Header::Value> get_keys();
    ///
    /// \brief Get list of trace headers with given trace header value
    ///
    /// \param value Used to specify header value
    /// \param max_num Maximum number of headers to read
    /// \return std::vector<Trace>
    ///
    std::vector<Trace::Header> get_headers(Trace::Header::Value value,
                                           uint64_t max_num = 1000000);
    ///
    /// \brief Get list of traces with given trace header value
    ///
    /// \param value Used to specify header value
    /// \return std::vector<Trace>
    ///
    std::vector<Trace> get_traces(Trace::Header::Value value,
                                  uint64_t max_num = 100000);
    virtual ~ISEGYSorted1D();

private:
    class Impl;
    std::unique_ptr<Impl> pimpl;
};
} // namespace sedaman

#endif
