#include "ISegy.hpp"
#include <iomanip>
#include <iostream>

template <typename T>
void print(T v, std::string s)
{
    std::cout << std::setw(20) << +v << " = " << s << '\n';
}

int main(int argc, char* argv[])
{
    if (argc < 2)
        return 1;
    try {
        sedaman::ISegy segy(argv[1]);
        auto bin_hdr = segy.binary_header();
        using name = sedaman::CommonSegy::BinaryHeader::Name;
        auto name_as_string = sedaman::CommonSegy::BinaryHeader::name_as_string;
        print(bin_hdr.job_id, name_as_string(name::JOB_ID));
        print(bin_hdr.line_num, name_as_string(name::LINE_NUM));
        print(bin_hdr.reel_num, name_as_string(name::REEL_NUM));
        print(bin_hdr.tr_per_ens, name_as_string(name::TR_PER_ENS));
        print(bin_hdr.aux_per_ens, name_as_string(name::AUX_PER_ENS));
        print(bin_hdr.samp_int, name_as_string(name::SAMP_INT));
        print(bin_hdr.samp_int_orig, name_as_string(name::SAMP_INT_ORIG));
        print(bin_hdr.samp_per_tr, name_as_string(name::SAMP_PER_TR));
        print(bin_hdr.samp_per_tr_orig, name_as_string(name::SAMP_PER_TR_ORIG));
        print(bin_hdr.format_code, name_as_string(name::FORMAT_CODE));
        print(bin_hdr.ens_fold, name_as_string(name::ENS_FOLD));
        print(bin_hdr.sort_code, name_as_string(name::SORT_CODE));
        print(bin_hdr.vert_sum_code, name_as_string(name::VERT_SUM_CODE));
        print(bin_hdr.sw_freq_at_start, name_as_string(name::SW_FREQ_AT_START));
        print(bin_hdr.sw_freq_at_end, name_as_string(name::SW_FREQ_AT_END));
        print(bin_hdr.sw_length, name_as_string(name::SW_LENGTH));
        print(bin_hdr.sw_type_code, name_as_string(name::SW_TYPE_CODE));
        print(bin_hdr.sw_ch_tr_num, name_as_string(name::SW_CH_TR_NUM));
        print(bin_hdr.taper_at_start, name_as_string(name::TAPER_AT_START));
        print(bin_hdr.taper_at_end, name_as_string(name::TAPER_AT_END));
        print(bin_hdr.taper_type, name_as_string(name::TAPER_TYPE));
        print(bin_hdr.corr_traces, name_as_string(name::CORR_TRACES));
        print(bin_hdr.bin_gain_recov, name_as_string(name::BIN_GAIN_RECOV));
        print(bin_hdr.amp_recov_meth, name_as_string(name::AMP_RECOV_METH));
        print(bin_hdr.measure_system, name_as_string(name::MEASURE_SYSTEM));
        print(bin_hdr.impulse_sig_pol, name_as_string(name::IMPULSE_SIG_POL));
        print(bin_hdr.vib_pol_code, name_as_string(name::VIB_POL_CODE));
        print(bin_hdr.ext_tr_per_ens, name_as_string(name::EXT_TR_PER_ENS));
        print(bin_hdr.ext_aux_per_ens, name_as_string(name::EXT_AUX_PER_ENS));
        print(bin_hdr.ext_samp_per_tr, name_as_string(name::EXT_SAMP_PER_TR));
        print(bin_hdr.ext_samp_int, name_as_string(name::EXT_SAMP_INT));
        print(bin_hdr.ext_samp_int_orig, name_as_string(name::EXT_SAMP_INT_ORIG));
        print(bin_hdr.ext_samp_per_tr_orig, name_as_string(name::EXT_SAMP_PER_TR_ORIG));
        print(bin_hdr.ext_ens_fold, name_as_string(name::EXT_ENS_FOLD));
        print(bin_hdr.endianness, name_as_string(name::ENDIANNESS));
        print(bin_hdr.SEGY_rev_major_ver, name_as_string(name::SEGY_REV_MAJOR_VER));
        print(bin_hdr.SEGY_rev_minor_ver, name_as_string(name::SEGY_REV_MINOR_VER));
        print(bin_hdr.fixed_tr_length, name_as_string(name::FIXED_TR_LENGTH));
        print(bin_hdr.ext_text_headers_num, name_as_string(name::EXT_TEXT_HEADERS_NUM));
        print(bin_hdr.max_num_add_tr_headers, name_as_string(name::MAX_NUM_ADD_TR_HEADERS));
        print(bin_hdr.time_basis_code, name_as_string(name::TIME_BASIS_CODE));
        print(bin_hdr.num_of_tr_in_file, name_as_string(name::NUM_OF_TR_IN_FILE));
        print(bin_hdr.byte_off_of_first_tr, name_as_string(name::BYTE_OFF_OF_FIRST_TR));
        print(bin_hdr.num_of_trailer_stanza, name_as_string(name::NUM_OF_TRAILER_STANZA));
    } catch (std::exception &e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
    return 0;
}
