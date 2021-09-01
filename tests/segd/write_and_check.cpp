#include "ISEGD.hpp"
#include "OSEGDRev2_1.hpp"
#include <iostream>

int main(int argc, char** argv)
{
    if (argc < 3)
        return 1;
    try {
        sedaman::ISEGD isgd(argv[1]);
		std::vector<std::shared_ptr<
			sedaman::CommonSEGD::AdditionalGeneralHeader>> v;
		for (auto &i : isgd.general_headerN())
			v.push_back
				(std::make_shared<sedaman::CommonSEGD::GeneralHeaderN>(i));
        sedaman::OSEGDRev2_1
		   	osgd (argv[2], isgd.general_header(),
				  isgd.general_header2().value(),
	 			  isgd.channel_set_headers(), v);
        while (isgd.has_record()) {
            auto trace = isgd.read_trace();
			osgd.write_trace(trace);
        }
    } catch (std::exception& e) {
        std::cerr << "exception on reading and writing\n"
                  << e.what() << '\n';
        return 1;
    }
    try {
        std::fstream ref(argv[1], std::ios_base::binary |
						 std::ios_base::in);
        std::fstream test(argv[2], std::ios_base::binary |
						  std::ios_base::in);
        size_t counter = 0;
        while (ref)
        {
            int first = ref.get();
            int second = test.get();
            if (first != second)
            {
                std::cout << counter << '\n';
                std::cerr << "reference file does not equal to created\n";
                return 1;
            }
            ++counter;
        }
    } catch (std::exception &e) {
        std::cerr << "exception on file comparing\n"
                  << e.what() << '\n';
        return 1;
    }
    return 0;
}
