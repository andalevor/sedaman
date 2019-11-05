#include "sexception.hpp"

using std::make_unique;
using std::move;
using std::string;

namespace sedaman {
class sexception::impl {
public:
    impl(string const &f_n, int l_n, string const &msg)
        : file_name(f_n), line_num(l_n), message(msg) {}

    impl(string &&f_n, int l_n, string &&msg)
        : file_name(move(f_n)), line_num(l_n),
          message(move(msg)) {}

    string file_name;
    int line_num;
    string message;
};

sexception::sexception(string const &file_name, int line_num,
                       string const &message)
    : pimpl(make_unique<impl>(file_name, line_num, message)) {}

sexception::sexception(string &&file_name, int line_num,
                       string &&message)
    : pimpl(make_unique<impl>(move(file_name), line_num, move(message))) {}

sexception::~sexception() = default;
} // namespace sedaman
