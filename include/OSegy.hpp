///
/// \file OSegy.hpp
/// \brief header file with OSegy class declaration
///
/// \author andalevor
///
/// \date   2020/04/06
///

#ifndef SEDAMAN_OSEGY_HPP
#define SEDAMAN_OSEGY_HPP

#include "CommonSegy.hpp"
#include "Trace.hpp"

///
/// \namespace sedaman
/// \brief General namespace for sedaman library.
///
namespace sedaman {
///
/// \class OSegy
/// \brief Abstract class for SEGY writing.
/// Declares common methods for all revisions to write information to SEGY files.
///
class OSegy : protected CommonSegy {
public:
	///
	/// \param file_name Name of SEGY file.
	/// \throws std::ifstream::failure In case of file operations falure
	/// \throws sedaman::Exception
	///
	OSegy(std::string file_name, CommonSegy::BinaryHeader bin_header,
		  std::vector<std::pair<std::string, std::map<uint32_t, std::pair<std::string, TrHdrValueType>>>> add_tr_hdrs_map);
	///
	/// \param tr Trace to write.
	/// \brief Writes trace to the end of file.
	///
	virtual void write_trace(Trace &tr) = 0;
	virtual ~OSegy();

protected:
	void assign_raw_writers();
	void assign_sample_writer();
	void assign_bytes_per_sample();
	void write_bin_header();
	void write_ext_text_headers();
	void write_trailer_stanzas();
	void write_trace_header(Trace::Header const &hdr);
	void write_additional_trace_headers(Trace::Header const &hdr);
	void write_trace_samples(Trace const &t);
	void write_trace_samples_var(Trace const &t);

private:
	class Impl;
	std::unique_ptr<Impl> pimpl;
};
} // namespace sedaman

#endif // SEDAMAN_OSEGY_HPP
