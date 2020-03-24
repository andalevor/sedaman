///
/// \file CommonSegy.hpp
/// \brief header file with CommonSegy class declaration
///
/// \author andalevor
///
/// \date   2019/06/07
///

#ifndef SEDAMAN_COMMON_SEGY_HPP
#define SEDAMAN_COMMON_SEGY_HPP

#include <cstdint>
#include <fstream>
#include <map>
#include <string>
#include <vector>

///
/// \namespace sedaman
/// \brief General namespace for sedaman library.
///
namespace sedaman {
///
/// \class CommonSegy
/// \brief Class with common SEGY parts.
/// Holds common data and members for ISegy and OSegy classes.
/// \see ISegy
/// \see OSegy
///
class CommonSegy {
public:
    ///
    /// \class bin_header
    /// \brief SEGY binary header storage class.
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
        /// \enum
        /// \brief Constants to use with names.
        /// \see names
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
    };
    ///
    /// \fn ebcdic_to_ascii
    /// \brief Transform text header from ebcdic to ascii.
    /// \param ebcdic String in ebcdic encoding.
    ///
    static void ebcdic_to_ascii(std::string ebcdic);
    ///
    /// \fn ascii_to_ebcdic
    /// \brief Transform text header from ascii to ebcdic.
    /// \param ascii String in ascii encoding.
    ///
    static void ascii_to_ebcdic(std::string ascii);
    ///
    /// \var default_text_header
    /// \brief Default SEGY text header from standard.
    ///
    static char const* default_text_header;
    ///
    /// \var TEXT_HEADER_SIZE
    /// \brief SEGY text header length in bytes
    ///
    static constexpr auto TEXT_HEADER_SIZE = 3200;
    ///
    /// \var BIN_HEADER_SIZE
    /// \brief SEGY binary header length in bytes
    ///
    static constexpr auto BIN_HEADER_SIZE = 400;
    ///
    /// \var TR_HEADER_SIZE
    /// \brief SEGY trace header length in bytes
    ///
    static constexpr auto TR_HEADER_SIZE = 240;
    ///
    /// \brief trace_header_description
    ///
    static std::map<std::string, std::string> trace_header_description;
    ///
    /// \param file_name Name of SEGY file.
    ///
    CommonSegy(std::string const& name, std::ios_base::openmode mode);
    ///
    /// \param file_name Name of SEGY file.
    ///
    CommonSegy(std::string&& name, std::ios_base::openmode mode);
    ///
    /// \var file_name
    /// \brief Name of file on disk.
    ///
    std::string file_name;
    ///
    /// \var file
    /// \brief File handler.
    ///
    std::fstream file;
    ///
    /// \var txt_hdrs
    /// \brief All text headers
    ///
    std::vector<std::string> txt_hdrs;
    ///
    /// \var trlr_stzs
    /// \brief All trailer stanzas
    ///
    std::vector<std::string> trlr_stnzs;
    ///
    /// \var bin_hdr
    /// \brief Binary header
    ///
    BinaryHeader bin_hdr;
    ///
    /// \var samp_buf
    /// \brief Buffer for samples
    ///
    std::vector<char> samp_buf;
    ///
    /// \var hdr_buf
    /// \brief Buffer for headers
    /// Size of buffer should be set by ISegy or OSegy
    ///
    char hdr_buf[TR_HEADER_SIZE];
    ///
    /// \var bytes_per_sample
    /// \brief Number of bytes per sample. Used for buffer size calculations.
    ///
    int bytes_per_sample;
    ///
    /// \var samp_per_tr
    /// \brief Number of samples. Used for buffer size calculations.
    ///
    int32_t samp_per_tr;
};
} // namespace sedaman

#endif // SEDAMAN_COMMON_SEGY_HPP
