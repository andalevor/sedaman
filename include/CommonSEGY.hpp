///
/// @file CommonSEGY.hpp
/// @author Andrei Voronin (andalevor@gmail.com)
/// \brief 
/// @version 0.1
/// \date 2019-06-07
/// 
/// @copyright Copyright (c) 2019
/// 
///

#ifndef SEDAMAN_COMMON_SEGY_HPP
#define SEDAMAN_COMMON_SEGY_HPP

#include <fstream>
#include <map>
#include <string>
#include <vector>
#include "Trace.hpp"

///
/// \brief General namespace for sedaman library.
/// \namespace sedaman
///
/// 
namespace sedaman {
///
/// \brief Class with common SEGY parts.
/// Holds common data and members for ISEGY and OSEGY classes.
/// 
/// \see ISEGY
/// \see OSEGY
///
/// 
class CommonSEGY {
public:
    ///
    /// \brief SEGY binary header storage class.
    /// \class BinaryHeader
    ///
    /// 
    class BinaryHeader {
    public:
        int32_t job_id;
        int32_t line_num;
        int32_t reel_num;
        int16_t tr_per_ens;
        int16_t aux_per_ens;
        int16_t samp_int;
        int16_t samp_int_orig;
        int16_t samp_per_tr;
        int16_t samp_per_tr_orig;
        int16_t format_code;
        int16_t ens_fold;
        int16_t sort_code;
        int16_t vert_sum_code;
        int16_t sw_freq_at_start;
        int16_t sw_freq_at_end;
        int16_t sw_length;
        int16_t sw_type_code;
        int16_t sw_ch_tr_num;
        int16_t taper_at_start;
        int16_t taper_at_end;
        int16_t taper_type;
        int16_t corr_traces;
        int16_t bin_gain_recov;
        int16_t amp_recov_meth;
        int16_t measure_system;
        int16_t impulse_sig_pol;
        int16_t vib_pol_code;
        int32_t ext_tr_per_ens;
        int32_t ext_aux_per_ens;
        int32_t ext_samp_per_tr;
        double ext_samp_int;
        double ext_samp_int_orig;
        int32_t ext_samp_per_tr_orig;
        int32_t ext_ens_fold;
        int32_t endianness;
        uint8_t SEGY_rev_major_ver;
        uint8_t SEGY_rev_minor_ver;
        int16_t fixed_tr_length;
        int16_t ext_text_headers_num;
        int32_t max_num_add_tr_headers;
        int16_t time_basis_code;
        uint64_t num_of_tr_in_file;
        uint64_t byte_off_of_first_tr;
        int32_t num_of_trailer_stanza;
        ///
        /// \brief Constants to use with names.
        /// \enum
        /// \see names
        ///
        /// 
        enum class Name {
            JOB_ID,
            LINE_NUM,
            REEL_NUM,
            TR_PER_ENS,
            AUX_PER_ENS,
            SAMP_INT,
            SAMP_INT_ORIG,
            SAMP_PER_TR,
            SAMP_PER_TR_ORIG,
            FORMAT_CODE,
            ENS_FOLD,
            SORT_CODE,
            VERT_SUM_CODE,
            SW_FREQ_AT_START,
            SW_FREQ_AT_END,
            SW_LENGTH,
            SW_TYPE_CODE,
            SW_CH_TR_NUM,
            TAPER_AT_START,
            TAPER_AT_END,
            TAPER_TYPE,
            CORR_TRACES,
            BIN_GAIN_RECOV,
            AMP_RECOV_METH,
            MEASURE_SYSTEM,
            IMPULSE_SIG_POL,
            VIB_POL_CODE,
            EXT_TR_PER_ENS,
            EXT_AUX_PER_ENS,
            EXT_SAMP_PER_TR,
            EXT_SAMP_INT,
            EXT_SAMP_INT_ORIG,
            EXT_SAMP_PER_TR_ORIG,
            EXT_ENS_FOLD,
            ENDIANNESS,
            SEGY_REV_MAJOR_VER,
            SEGY_REV_MINOR_VER,
            FIXED_TR_LENGTH,
            EXT_TEXT_HEADERS_NUM,
            MAX_NUM_ADD_TR_HEADERS,
            TIME_BASIS_CODE,
            NUM_OF_TR_IN_FILE,
            BYTE_OFF_OF_FIRST_TR,
            NUM_OF_TRAILER_STANZA
        };
        static char const* name_as_string(Name n);
    } binary_header;
    ///
    /// \brief Transform text header from ebcdic to ascii.
    /// 
    /// \param ebcdic String in ebcdic encoding.
    ///
    static void ebcdic_to_ascii(std::string& ebcdic);
    ///
    /// \brief Transform text header from ascii to ebcdic.
    /// 
    /// \param ascii String in ascii encoding.
    ///
    static void ascii_to_ebcdic(std::string& ascii);
    ///
    /// \brief Default SEGY text header from standard.
    ///
    static char const* default_text_header;
	///
	/// \brief Default SEGY trace header mapping
	///
	static std::vector<std::pair<std::string,
	   	std::map<uint32_t, std::pair<std::string, Trace::Header::ValueType>>>>
		default_trace_header;
    ///
    /// \brief SEGY text header length in bytes
    ///
    /// 
    static constexpr int TEXT_HEADER_SIZE = 3200;
    ///
    /// \brief SEGY binary header length in bytes
    ///
    /// 
    static constexpr int BIN_HEADER_SIZE = 400;
    ///
    /// \brief SEGY trace header length in bytes
    ///
    /// 
    static constexpr int TR_HEADER_SIZE = 240;
    ///
    /// \brief Can be used to browse headers or get there description.
    ///
    /// 
    static std::map<std::string, std::string> trace_header_description;
    ///
    /// \brief Construct a new Common S E G Y object
    /// 
    /// \param name Name of SEGY file.
    /// \param mode Used to switch between input and output
    /// \param bh Can be used to override binary header values.
	/// Usefull for OSegy.
    /// \param tr_hdrs_map Each item of vector is pair of header name
	/// and map.
    ///
    CommonSEGY(std::string name, std::ios_base::openmode mode, BinaryHeader bh,
        std::vector<std::pair<std::string, std::map<uint32_t,
	   	std::pair<std::string, Trace::Header::ValueType>>>> tr_hdrs_map);
    std::string file_name;
    std::fstream file;
    std::vector<std::string> text_headers;
    std::vector<std::string> trailer_stanzas;
    std::vector<char> samp_buf;
    char hdr_buf[TR_HEADER_SIZE];
    int bytes_per_sample;
    int32_t samp_per_tr;
    std::vector<std::pair<std::string, std::map<uint32_t,
	   	std::pair<std::string, Trace::Header::ValueType>>>> tr_hdr_map;
};
} // namespace sedaman

#endif // SEDAMAN_COMMON_SEGY_HPP
