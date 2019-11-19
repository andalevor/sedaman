///
/// \file config.hpp
/// \brief header file with config class declaration
///
/// \author andalevor
///
/// \date   2019/11/19
///

#ifndef SEDAMAN_CONFIG_HPP
#define SEDAMAN_CONFIG_HPP

#include <experimental/propagate_const>
#include <memory>
#include <string>
#include <utility>

///
/// \namespace sedaman
/// \brief General namespace for sedaman library.
///
namespace sedaman {
class config {
public:
    config();
    explicit config(std::string const &file_name);
    explicit config(std::string &&file_name) noexcept;
    std::pair<int, int> get_pair(int32_t num, std::string header_name);
    ~config();
private:
    class impl;
    std::experimental::propagate_const<std::unique_ptr<impl>> pimpl;
};
} // namespace sedaman

#endif // SEDAMAN_CONFIG_HPP
