///
/// \file sexception.hpp
/// \brief header file with exception class declaration
///
/// \author andalevor
///
/// \date   2019/06/06
///

#ifndef SEDAMAN_SEXCEPTION_HPP
#define SEDAMAN_SEXCEPTION_HPP

#include <exception>
#include <experimental/propagate_const>
#include <memory>
#include <string>

namespace sedaman {
class sexception : public std::exception {
public:
    sexception(const sexception &e);
    sexception(sexception &&e);
    sexception(std::string const &file_name, unsigned long long line_num,
               std::string const &message);
    sexception(std::string &&file_name, unsigned long long line_num,
               std::string &&message);
    const char* what() const noexcept override;
    virtual ~sexception() override;

private:
    class impl;
    std::experimental::propagate_const<std::unique_ptr<impl>> pimpl;
};
} // namespace sedaman

#endif // SEDAMAN_SEXCEPTION_HPP
