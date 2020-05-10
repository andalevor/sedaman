#include "ISegy.hpp"
#include <cassert>
#include <exception>
#include <iostream>
#include <deque>

int main(int argc, char *argv[])
{
    if (argc < 2)
        return 1;
    try {
        sedaman::ISegy segy(argv[1]);
        std::deque<double> reference;
        bool first = true;
        while (segy.has_next()) {
            sedaman::Trace t = segy.read_trace();
            if (first) {
                for (auto i: t.samples())
                    reference.push_back(i);
                first = false;
                double sum = t.samples().sum();
                assert(sum == -12350);
                continue;
            }
            reference.pop_front();
            reference.push_back(0);
            double sum_ref = 0;
            for (auto v: reference)
                sum_ref += v;
            double sum_curr = t.samples().sum();
            assert(sum_ref == sum_curr);
        }
    } catch (std::exception& e) {
        std::cerr << e.what() << '\n';
    }
    return 0;
}
