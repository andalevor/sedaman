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
/// \brief Class for SEGY reading.
/// Defines methods to write information from SEGY files.
///
class OSegy {
public:
	///
	/// \param file_name Name of SEGY file.
	/// \param revision SEGY standard revision.
	/// \throws std::ifstream::failure In case of file operations falure
	/// \throws sedaman::Exception
	///
	OSegy(std::string file_name, std::string revision = "rev2.0");
	///
	/// \param
	OSegy &write_trace(Trace &tr);
	~OSegy();

private:
	class Impl;
	std::experimental::propagate_const<std::unique_ptr<Impl>> pimpl;
};
} // namespace sedaman

#endif // SEDAMAN_OSEGY_HPP
