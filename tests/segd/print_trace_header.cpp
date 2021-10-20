#include "ISEGD.hpp"
#include <algorithm>
#include <iomanip>
#include <iostream>

class Printer {
public:
    explicit Printer(int s)
        : size { s }
    {
    }
    void operator()(sedaman::Trace::Header::Value v, std::string s)
    {
        if (std::holds_alternative<int64_t>(v))
            std::cout << std::left << std::setw(size) << std::setfill('.')
			   	<< s << " = " << +std::get<int64_t>(v) << '\n';
        else std::cout << std::fixed << std::left << std::setw(size) <<
			   	std::setfill('.') << s << " = " << +std::get<double>(v)
			   	<< '\n';
    }

private:
    int size;
};

int get_max(std::vector<std::string> const& keys);

int main(int argc, char* argv[])
{
    if (argc < 2)
        return 1;
    try {
        sedaman::ISEGD segd(argv[1]);
        sedaman::Trace trc = segd.read_trace();
        std::vector<std::string> keys = trc.header().keys();
        std::sort(keys.begin(), keys.end());
        int max = get_max(keys);
        Printer p(max);
        for (auto& k : keys)
            p(*trc.header().get(k), k);
    } catch (std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}

int get_max(std::vector<std::string> const& keys)
{
    int max = 0;
    for (auto& k : keys) {
        if (max < k.size())
            max = k.size();
    }
    return max;
}
