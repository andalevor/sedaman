#include "CommonSEGY.hpp"
#include "ISEGY.hpp"
#include <iostream>

template <typename T, typename U>
void check(T first, U second)
{
    if (first != second)
    {
        std::cerr << first << " not equal to " << second << '\n';
        exit(1);
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2)
        return 1;
    try
    {
        sedaman::ISEGY segy(argv[1]);
        sedaman::CommonSEGY::BinaryHeader bin_hdr = segy.binary_header();
        check(bin_hdr.job_id, 9999);
        check(bin_hdr.line_num, 9999);
        check(bin_hdr.reel_num, 1);
        check(bin_hdr.tr_per_ens, 40);
        check(bin_hdr.samp_int, 2000);
        check(bin_hdr.samp_per_tr, 501);
        check(bin_hdr.samp_per_tr_orig, 2001);
        check(bin_hdr.format_code, 1);
        check(bin_hdr.ens_fold, 1);
        check(bin_hdr.sort_code, 1);
        check(bin_hdr.vert_sum_code, 1);
        check(bin_hdr.corr_traces, 2);
        check(bin_hdr.bin_gain_recov, 1);
        check(bin_hdr.amp_recov_meth, 4);
        check(bin_hdr.measure_system, 1);
        check(bin_hdr.SEGY_rev_major_ver, 1);
        check(bin_hdr.fixed_tr_length, 1);
    }
    catch (std::exception &e)
    {
        std::cerr << e.what() << '\n';
        return 1;
    }
    return 0;
}
