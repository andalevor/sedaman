/// \file Exception.hpp
/// \brief header file with Exception class declaration
/// \author andalevor
/// \date   2019/06/06

#ifndef SEDAMAN_SEXCEPTION_HPP
#define SEDAMAN_SEXCEPTION_HPP

#include <exception>
#include <experimental/propagate_const>
#include <memory>
#include <string>

namespace sedaman
{
    class Exception : public std::exception
    {
    public:
        Exception(Exception const &e);
        Exception(Exception &&e);
        Exception(std::string const &file_name, unsigned long long line_num,
                  std::string const &message);
        Exception(std::string &&file_name, unsigned long long line_num,
                  std::string &&message);
        char const *what() const noexcept override;
        virtual ~Exception() override;

    private:
        class Impl;
        std::experimental::propagate_const<std::unique_ptr<Impl>> pimpl;
    };
} // namespace sedaman

#endif // SEDAMAN_SEXCEPTION_HPP
