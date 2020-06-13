#include "ISEGY.hpp"
#include <deque>
#include <exception>
#include <iostream>

int main(int argc, char *argv[])
{
    if (argc < 2)
        return 1;
    double sum_ref = -0.00000595875118516262934804939011002034021657891571521759033203125;
    try
    {
        sedaman::ISEGY segy(argv[1]);
        std::deque<double> reference;
        bool first = true;
        while (segy.has_trace())
        {
            sedaman::Trace t = segy.read_trace();
            if (first)
            {
                for (auto i : t.samples())
                    reference.push_back(i);
                first = false;
                double sum = t.samples().sum();
                if (sum != sum_ref)
                {
                    std::cerr << "first trace:\n"
                              << sum << " not equal to " << sum_ref << '\n';
                    return 1;
                }
                continue;
            }
            reference.pop_front();
            reference.push_back(0);
            double sum_ref = 0;
            for (auto v : reference)
                sum_ref += v;
            double sum_curr = t.samples().sum();
            if (sum_curr != sum_ref)
            {
                std::cerr << sum_curr << " not equal to " << sum_ref << '\n';
                return 1;
            }
        }
    }
    catch (std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
    return 0;
}
