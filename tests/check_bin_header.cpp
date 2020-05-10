#include "CommonSegy.hpp"
#include "ISegy.hpp"
#include <cassert>
#include <iostream>

int main(int argc, char *argv[])
{
    if (argc < 2)
        return 1;
    try {
        sedaman::ISegy segy(argv[1]);
        sedaman::CommonSegy::BinaryHeader bin_hdr = segy.binary_header();
        assert(bin_hdr.job_id == 9999);
        assert(bin_hdr.line_num == 9999);
        assert(bin_hdr.reel_num == 1);
        assert(bin_hdr.tr_per_ens == 40);
        assert(bin_hdr.samp_int == 2000);
        assert(bin_hdr.samp_per_tr == 501);
        assert(bin_hdr.samp_per_tr_orig == 2001);
        assert(bin_hdr.format_code == 1);
        assert(bin_hdr.ens_fold == 1);
        assert(bin_hdr.sort_code == 1);
        assert(bin_hdr.vert_sum_code == 1);
        assert(bin_hdr.corr_traces == 2);
        assert(bin_hdr.bin_gain_recov == 1);
        assert(bin_hdr.amp_recov_meth == 4);
        assert(bin_hdr.measure_system == 1);
        assert(bin_hdr.SEGY_rev_major_ver == 1);
        assert(bin_hdr.fixed_tr_length == 1);
    } catch(std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
    return 0;
}
