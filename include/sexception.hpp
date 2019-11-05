#ifndef SEDAMAN_SEXCEPTION_HPP
#define SEDAMAN_SEXCEPTION_HPP

#include <exception>
#include <experimental/propagate_const>
#include <memory>
#include <string>

namespace sedaman {
class sexception : public std::exception {
public:
    sexception(std::string const &file_name, int line_num,
               std::string const &message);
    sexception(std::string &&file_name, int line_num, std::string &&message);
    virtual ~sexception();

private:
    class impl;
    std::experimental::propagate_const<std::unique_ptr<impl>> pimpl;
};
}

#endif // SEDAMAN_SEXCEPTION_HPP
