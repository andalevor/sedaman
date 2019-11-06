///
/// \file segy.hpp
/// \brief header file with segy class declaration
///
/// \author andalevor
///
/// \date   2019/06/07
///

#ifndef SEDAMAN_SEGY_HPP
#define SEDAMAN_SEGY_HPP

#include <cfloat>
#include <climits>
#include <cstdint>
#include <experimental/propagate_const>
#include <memory>
#include <string>

static_assert(CHAR_BIT == 8, "CHAR_BIT != 8\n");
static_assert(FLT_RADIX == 2, "FLT_RADIX != 2\n");
static_assert(DBL_MANT_DIG == 53, "DBL_MAN_DIG != 53\n");

///
/// \namespace sedaman
/// \brief General namespace for sedaman library.
///
namespace sedaman {
///
/// \class segy
/// \brief Class with common SEGY parts.
/// Holds common data and members for isegy and osegy classes.
/// \see isegy
/// \see osegy
///
class segy {
public:
    ///
    /// \class bin_header
    /// \brief SEGY binary header storage class.
    ///
    class binary_header {
    public:
        int32_t  job_id;
        int32_t  line_num;
        int32_t  reel_num;
        int16_t  tr_per_ens;
        int16_t  aux_per_ens;
        int16_t  samp_int;
        int16_t  samp_int_orig;
        int16_t  samp_per_tr;
        int16_t  samp_per_tr_orig;
        int16_t  format_code;
        int16_t  ens_fold;
        int16_t  sort_code;
        int16_t  vert_sum_code;
        int16_t  sw_freq_at_start;
        int16_t  sw_freq_at_end;
        int16_t  sw_length;
        int16_t  sw_type_code;
        int16_t  sw_ch_tr_num;
        int16_t  taper_at_start;
        int16_t  taper_at_end;
        int16_t  taper_type;
        int16_t  corr_traces;
        int16_t  bin_gain_recov;
        int16_t  amp_recov_meth;
        int16_t  measure_system;
        int16_t  impulse_sig_pol;
        int16_t  vib_pol_code;
        int32_t  ext_tr_per_ens;
        int32_t  ext_aux_per_ens;
        int32_t  ext_samp_per_tr;
        double   ext_samp_int;
        double   ext_samp_int_orig;
        int32_t  ext_samp_per_tr_orig;
        int32_t  ext_ens_fold;
        int32_t  endianness;
        uint8_t  SEGY_rev_major_ver;
        uint8_t  SEGY_rev_minor_ver;
        int16_t  fixed_tr_length;
        int16_t  ext_text_headers_num;
        int32_t  max_num_add_tr_headers;
        int16_t  time_basis_code;
        uint64_t num_of_tr_in_file;
        uint64_t byte_off_of_first_tr;
        int32_t  num_of_trailer_stanza;
    };
    ///
    /// \param file_name Name of SEGY file.
    /// \param bin_hdr (optional)SEGY binary header.
    /// \param text_hdr (optional)SEGY text header.
    ///
    segy(std::string const &file_name,
         binary_header const &bin_hdr = binary_header(),
         std::string const &text_hdr = std::string());
    ///
    /// \param file_name Name of SEGY file.
    /// \param bin_hdr (optional)SEGY binary header.
    /// \param text_hdr (optional)SEGY text header.
    ///
    segy(std::string &&file_name,
         binary_header &&bin_hdr = binary_header(),
         std::string &&text_hdr = std::string());
    virtual ~segy();
    ///
    /// \fn text_hdr
    /// \brief segy text header getter
    /// \return text header
    ///
    std::string const &text_hdr();
    ///
    /// \fn ebcdic_to_ascii
    /// \brief Transform ebcdic string to ascii string.
    /// \param ebcdic String to transform.
    /// \return Transformed string.
    ///
    static std::string ebcdic_to_ascii(const std::string &ebcdic);
    static std::string ebcdic_to_ascii(std::string &&ebcdic);
    ///
    /// \fn ascii_to_ebcdic
    /// \brief Transform ascii string to ebcdic string.
    /// \param ascii String to transform.
    /// \return Transformed string.
    ///
    static std::string ascii_to_ebcdic(const std::string &ascii);
    static std::string ascii_to_ebcdic(std::string &&ascii);
    ///
    /// \var default_text_header
    /// \brief Default SEGY text header from standard.
    ///
    static const char *default_text_header;
    ///
    /// \var TEXT_HEADER_LEN
    /// \brief SEGY text header length in bytes
    ///
    static constexpr int TEXT_HEADER_LEN = 3200;
protected:
    std::string const &file_name();
    binary_header const &bin_hdr();
    void set_bin_hdr(binary_header &&b_h);
    void set_text_hdr(std::string &&t_h);
private:
    class impl;
    std::experimental::propagate_const<std::unique_ptr<impl>> pimpl;
};
} // namespace sedaman

#endif // __SEDAMAN_SEGY_HPP__
