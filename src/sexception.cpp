#include <sstream>
#include "sexception.hpp"

using std::make_unique;
using std::move;
using std::string;
using std::ostringstream;

namespace sedaman {
class sexception::impl {
public:
    impl(string const &f_n, int l_n, string const &msg);
    impl(string &&f_n, int l_n, string &&msg);
    string join_message(const string &f_n, int l_n, const string &msg);

    string d_message;
};

sexception::impl::impl(string const &f_n, int l_n, string const &msg)
{
    d_message = join_message(f_n, l_n, msg);
}

sexception::impl::impl(string &&f_n, int l_n, string &&msg)
{
    d_message = join_message(f_n, l_n, msg);
}

string sexception::impl::join_message(const string &f_n, int l_n, const string &msg)
{
    ostringstream ss;

    ss << f_n << " " << l_n << " " << msg;
    return ss.str();
}

const char* sexception::what() const noexcept
{
    return pimpl->d_message.c_str();
}

sexception::sexception(string const &file_name, int line_num,
                       string const &message)
    : pimpl(make_unique<impl>(file_name, line_num, message)) {}

sexception::sexception(string &&file_name, int line_num,
                       string &&message)
    : pimpl(make_unique<impl>(move(file_name), line_num, move(message))) {}

sexception::~sexception() = default;
} // namespace sedaman
