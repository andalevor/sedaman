#include "ISEGY.hpp"
#include <deque>
#include <exception>
#include <iostream>
#include <numeric>

int main(int argc, char *argv[])
{
    if (argc < 2)
        return 1;
    double sum_ref = 0;
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
                reference = std::deque<double>(t.samples().begin(),
											   t.samples().end());
                first = false;
                double sum = std::accumulate(t.samples().begin(),
											 t.samples().end(), 0.0);
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
            double sum_ref = std::accumulate(reference.begin(),
											 reference.end(), 0.0);
            double sum_curr = std::accumulate(t.samples().begin(),
											  t.samples().end(), 0.0);
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
