///
/// @file Exception.hpp
/// @author Andrei Voronin (andalevor@gmail.com)
/// \brief 
/// @version 0.1
/// \date 2019-06-06
/// 
/// @copyright Copyright (c) 2019
/// 
///
#ifndef SEDAMAN_SEXCEPTION_HPP
#define SEDAMAN_SEXCEPTION_HPP

#include <exception>
#include <experimental/propagate_const>
#include <memory>
#include <string>

///
/// \brief General namespace for sedaman library.
/// 
///
namespace sedaman {
///
/// \brief Exception to handle errors in SEGY parameters
/// 
///
class Exception : public std::exception {
public:
    ///
    /// \brief Construct a new Exception object
    /// 
    /// \param e 
    ///
    Exception(Exception const& e);
    ///
    /// \brief Construct a new Exception object
    /// 
    /// \param e 
    ///
    Exception(Exception&& e);
    ///
    /// \brief Construct a new Exception object
    /// 
    /// \param file_name Name of file with error.
    /// \param line_num Number of line with error.
    /// \param message Message to describe error.
    ///
    Exception(std::string const& file_name, unsigned long long line_num,
        std::string const& message);
    ///
    /// \brief Construct a new Exception object
    /// 
    /// \param file_name Name of file with error.
    /// \param line_num Number of line with error.
    /// \param message Message to describe error.
    ///
    Exception(std::string&& file_name, unsigned long long line_num,
        std::string&& message);
    ///
    /// \brief message getter
    /// 
    /// \return char const* 
    ///
    char const* what() const noexcept override;
    virtual ~Exception() override;

private:
    class Impl;
    std::experimental::propagate_const<std::unique_ptr<Impl>> pimpl;
};
} // namespace sedaman

#endif // SEDAMAN_SEXCEPTION_HPP
