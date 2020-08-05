/// \file C_API.hpp
/// \brief header file with C API
/// \author andalevor
/// \date   2020/06/17

#ifndef SEDAMAN_C_API_HPP
#define SEDAMAN_C_API_HPP

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C"
{
#endif
	/// \struct sedaman_CommonSegy_BinaryHeader
	/// \brief CommonSegy::BinaryHeader wrapper
	/// \see CommonSegy::BinaryHeader
	typedef struct sedaman_CommonSEGY_BinaryHeader sedaman_CommonSEGY_BinaryHeader;
	/// \struct sedaman_Trace
	/// \brief Trace wrapper
	/// \see Trace
	typedef struct sedaman_Trace sedaman_Trace;
	/// \enum sedaman_ISEGY_Header_Value_Types
	/// \brief Type of value in variant
	/// It's needed for variant unboxing
	/// \see Trace::Header::Value
	typedef enum sedaman_Trace_Header_Value_Types
	{
		NONE,
		I8,
		I16,
		I32,
		I64,
		U8,
		U16,
		U32,
		U64,
		F32,
		F64
	} sedaman_Trace_Header_Value_Types;
	/// \union sedaman_ISEGY_Header_Value
	/// \brief Value in variant
	/// It's needed to store value from variant
	/// \see Trace::Header::Value
	typedef union sedaman_Trace_Header_Value sedaman_Trace_Header_Value;
	union sedaman_Trace_Header_Value
	{
		int8_t i8;
		int16_t i16;
		int32_t i32;
		int64_t i64;
		uint8_t u8;
		uint16_t u16;
		uint32_t u32;
		uint64_t u64;
		float f32;
		double f64;
	};
	/// \struct sedaman_ISEGY_Header_Value_Holder
	/// \brief Holds value and values type
	/// \see Trace::Header::Value
	typedef struct sedaman_Trace_Header_Value_Holder
	{
		sedaman_Trace_Header_Value_Types type;
		sedaman_Trace_Header_Value val;
	} sedaman_Trace_Header_Value_Holder;
	/// \brief Frees resources
	/// Trace destructor wrapper
	/// \param trc
	void sedaman_Trace_delete(sedaman_Trace *trc);
	/// \brief Gets value from trace header by given key
	/// Trace::Header::get wrapper
	/// \param t
	/// \param s
	/// \see Trace::Header::get
	/// \return sedaman_Trace_Header_Value_Holder
	sedaman_Trace_Header_Value_Holder sedaman_Trace_Header_get_value(sedaman_Trace *t, char const *s);
	/// \brief Gets trace sample value by given sample number
	/// \param trc
	/// \param num
	/// \return double
	double sedaman_Trace_get_sample(sedaman_Trace *trc, size_t num);
	/// \brief Gets samples number
	/// \param trc
	/// \return size_t
	size_t sedaman_Trace_samples_num(sedaman_Trace *trc);
	/// \struct sedaman_ISEGY
	/// \brief ISEGY wrapper
	/// \see ISEGY
	typedef struct sedaman_ISEGY sedaman_ISEGY;
	/// \brief Creates new sedaman_ISEGY instance
	/// ISEGY constructor wrapper
	/// \param file_name
	/// \param err
	/// \see ISEGY::ISEGY
	/// \return sedaman_ISEGY*
	sedaman_ISEGY *sedaman_ISEGY_new(char const *file_name, char *err);
	/// \brief Frees resources
	/// ISEGY destructor wrapper
	/// \see ISEGY::~ISEGY
	/// \param sgy
	void sedaman_ISEGY_delete(sedaman_ISEGY *sgy);
	/// \brief Return true if there is a trace to read
	/// ISEGY::has_trace wrapper
	/// \see ISEGY::has_trace
	/// \param sgy
	/// \return true
	/// \return false
	bool sedaman_ISEGY_has_trace(sedaman_ISEGY *sgy);
	/// \brief Reads trace from segy
	/// ISEGY::read_trace wrapper
	/// \param sgy
	/// \param err
	/// \see ISEGY::read_trace
	/// \return sedaman_Trace*
	sedaman_Trace *sedaman_ISEGY_read_trace(sedaman_ISEGY *sgy, char const **err);
	/// \struct sedaman_ISEGD
	/// \brief ISEGD wrapper
	/// \see ISEGD
	typedef struct sedaman_ISEGD sedaman_ISEGD;
	/// \brief Creates new sedaman_ISEGD instance
	/// ISEGD constructor wrapper
	/// \param file_name
	/// \param err
	/// \see ISEGD::ISEGD
	/// \return sedaman_ISEGD*
	sedaman_ISEGD *sedaman_ISEGD_new(char const *file_name, char const **err);
	/// \brief Frees resources
	/// ISEGD destructor wrapper
	/// \see ISEGD::~ISEGD
	/// \param sgd
	void sedaman_ISEGD_delete(sedaman_ISEGD *sgd);
	/// \brief Return true if there is a record to read
	/// \see ISEGD::has_record
	/// \param sgd
	/// \return true
	/// \return false
	bool sedaman_ISEGD_has_record(sedaman_ISEGD *sgd);
	/// \brief Return true if there is a trace to read
	/// ISEGD::has_trace wrapper
	/// \param sgd
	/// \see ISEGD::has_trace
	/// \return true
	/// \return false
	bool sedaman_ISEGD_has_trace(sedaman_ISEGD *sgd);
	/// \brief Reads trace from segd
	/// ISEGD::read_trace wrapper
	/// \param sgd
	/// \param err
	/// \see ISEGD::read_trace
	/// \return sedaman_Trace*
	sedaman_Trace *sedaman_ISEGD_read_trace(sedaman_ISEGD *sgd, char const **err);
	/// \struct sedaman_OSEGY
	/// \brief OSEGY wrapper
	/// \see OSEGY
	typedef struct sedaman_OSEGYRev0 sedaman_OSEGYRev0;
	/// \brief Creates new sedaman_OSEGYRev0 instance
	/// One of OSEGYRev0 constructor wrappers
	/// \param file_name
	/// \param err
	/// \see OSEGYRev0::OSEGYRev0
	/// \return sedaman_OSEGYRev0*
	sedaman_OSEGYRev0 *sedaman_OSEGYRev0_new(char const *file_name, char const **err);
	/// \brief Creates new sedaman_OSEGYRev0 instance
	/// One of OSEGYRev0 constructor wrappers
	/// \param file_name
	/// \param text_header
	/// \param err
	/// \see OSEGYRev0::OSEGYRev0
	/// \return sedaman_OSEGYRev0*
	sedaman_OSEGYRev0 *sedaman_OSEGYRev0_new_with_text_header(char const *file_name, char const *text_header, char const **err);
	/// \brief Creates new sedaman_OSEGYRev0 instance
	/// One of OSEGYRev0 constructor wrappers
	/// \param file_name
	/// \param text_header
	/// \param bin_header
	/// \param err
	/// \see OSEGYRev0::OSEGYRev0
	/// \return sedaman_OSEGYRev0*
	sedaman_OSEGYRev0 *sedaman_OSEGYRev0_new_with_text_and_bin_headers(char const *file_name, char const *text_header, sedaman_CommonSEGY_BinaryHeader *bin_header, char const **err);
	/// \brief Frees resources
	/// OSEGYRev0 destructor wrapper
	/// \see OSEGYRev0::~OSEGYRev0
	/// \param sgy
	void sedaman_OSEGYRev0_delete(sedaman_OSEGYRev0 *sgy);
	/// \brief Writes trace to segy
	/// OSEGYRev0::write_trace wrapper
	/// \param sgy
	/// \param err
	/// \see OSEGYRev0::write_trace
	void sedaman_OSEGYRev0_write_trace(sedaman_OSEGYRev0 *sgy, sedaman_Trace *trc, char const **err);
#ifdef __cplusplus
}
#endif

#endif // SEDAMAN_C_API_HPP
