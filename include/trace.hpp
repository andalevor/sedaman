///
/// \file trace.hpp
/// \brief header file with trace class declaration
///
/// \author andalevor
///
/// \date   2019/06/06
///

#ifndef SEDAMAN_TRACE_HPP
#define SEDAMAN_TRACE_HPP

#include <cstdint>
#include <experimental/propagate_const>
#include <memory>
#include <valarray>

///
/// \namespace sedaman
/// \brief General namespace for sedaman library.
///
namespace sedaman {
///
/// \class trace
/// \brief Class to store trace header and trace samples.
///
class trace {
public:
    trace(const trace &t);
    trace(trace &&t) noexcept;
    trace(const std::valarray<int32_t> &h, const std::valarray<double> &s);
    trace(std::valarray<int32_t> &&h, std::valarray<double> &&s);
    ~trace();
    trace &operator=(const trace &o);
    trace &operator=(trace &&o);
    std::valarray<int32_t> const &header();
    std::valarray<double> const &samples();
private:
    class impl;
    std::experimental::propagate_const<std::unique_ptr<impl>> pimpl;
};
}

#endif // SEDAMAN_TRACE_HPP
