#include "Exception.hpp"
#include <sstream>

using std::make_unique;
using std::move;
using std::ostringstream;
using std::string;

namespace sedaman
{
    class Exception::Impl
    {
    public:
        Impl(string const &f_n, unsigned long long l_n, string const &msg);
        Impl(string &&f_n, unsigned long long l_n, string &&msg);
        string join_message(string const &f_n, unsigned long long l_n,
                            string const &msg);

        string d_message;
    };

    Exception::Impl::Impl(string const &f_n, unsigned long long l_n,
                          string const &msg)
    {
        d_message = join_message(f_n, l_n, msg);
    }

    Exception::Impl::Impl(string &&f_n, unsigned long long l_n, string &&msg)
    {
        d_message = join_message(f_n, l_n, msg);
    }

    string Exception::Impl::join_message(string const &f_n, unsigned long long l_n,
                                         string const &msg)
    {
        ostringstream ss;

        ss << f_n << " " << l_n << " " << msg;
        return ss.str();
    }

    char const *Exception::what() const noexcept
    {
        return pimpl->d_message.c_str();
    }
    Exception::Exception(Exception const &e)
    {
        this->pimpl->d_message = e.pimpl->d_message;
    }
    Exception::Exception(Exception &&e)
    {
        this->pimpl = move(e.pimpl);
    }
    Exception::Exception(string const &file_name, unsigned long long line_num,
                         string const &message)
        : pimpl(make_unique<Impl>(file_name, line_num, message))
    {
    }

    Exception::Exception(string &&file_name, unsigned long long line_num,
                         string &&message)
        : pimpl(make_unique<Impl>(move(file_name), line_num, move(message)))
    {
    }

    Exception::~Exception() = default;
} // namespace sedaman
