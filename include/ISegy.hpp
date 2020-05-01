///
/// \file ISegy.hpp
/// \brief header file with ISegy class declaration
///
/// \author andalevor
///
/// \date   2019/06/17
///

#ifndef SEDAMAN_ISEGY_HPP
#define SEDAMAN_ISEGY_HPP

#include "CommonSegy.hpp"
#include "Trace.hpp"

///
/// \namespace sedaman
/// \brief General namespace for sedaman library.
///
namespace sedaman {
///
/// \class ISegy
/// \brief Class for SEGY reading.
/// Defines methods to read information from SEGY files.
///
class ISegy : CommonSegy {
public:
	///
	/// \param file_name Name of SEGY file.
	/// \throws std::ifstream::failure In case of file operations falure
	/// \throws sedaman::Exception
	///
	explicit ISegy(std::string file_name);
	///
	/// \param file_name Name of SEGY file.
	/// \param binary_header Could be used to override values in binary header.
	/// \throws std::ifstream::failure In case of file operations falure
	/// \throws sedaman::Exception
	///
	ISegy(std::string file_name, BinaryHeader binary_header,
		  std::vector<std::pair<std::string, std::map<uint32_t, std::pair<std::string, TrHdrValueType>>>> add_hdr_map);
	///
	/// \param file_name Name of SEGY file.
	/// \throws std::ifstream::failure In case of file operations falure
	/// \throws sedaman::Exception
	/// \brief creates ISegy instance internally and returns binary header.
	/// Could be used to get binary header from file to override some values.
	///
	static CommonSegy::BinaryHeader read_binary_header(std::string file_name);
	///
	/// \fn text_header
	/// \brief segy text headers getter
	/// \return vector with text headers
	///
	std::vector<std::string> const& text_headers();
	///
	/// \fn trailer_stanzas
	/// \brief segy trailer stanzas getter
	/// \return vector with text headers
	///
	std::vector<std::string> const& trailer_stanzas();
	///
	/// \fn binary_header
	/// \brief segy binary header getter
	/// \return binary header
	///
	CommonSegy::BinaryHeader const& binary_header();
	///
	/// \fn has_next
	/// \brief checks for next trace in file
	/// \return true if there is at least one trace
	///
	bool has_next();
	///
	/// \fn read_header
	/// \brief reads header, skips samples
	/// \return Trace::Header
	///
	Trace::Header read_header();
	///
	/// \fn read_trace
	/// \brief reads one trace from file
	/// \return Trace
	///
	Trace read_trace();
	virtual ~ISegy();

private:
	class Impl;
	std::unique_ptr<Impl> pimpl;
};
} // namespace sedaman

#endif // SEDAMAN_ISEGY_HPP
