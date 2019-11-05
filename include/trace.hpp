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
#include <vector>

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
    trace(std::vector<int32_t> &h, std::vector<double> &s);
    trace(std::vector<int32_t> &&h, std::vector<double> &&s);
    ~trace();
    std::vector<int32_t> const &header();
    std::vector<double> const &samples();
private:
    class impl;
    std::experimental::propagate_const<std::unique_ptr<impl>> pimpl;
};
}

#endif // SEDAMAN_TRACE_HPP
