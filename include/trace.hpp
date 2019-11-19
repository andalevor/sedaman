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
    ///
    /// \param trc Trace to copy data from
    ///
    trace(const trace &trc);
    ///
    /// \param trc Trace to move data from
    ///
    trace(trace &&trc) noexcept;
    ///
    /// \param hdr Header values
    /// \param smpl Samples values
    ///
    trace(const std::valarray<int32_t> &hdr, const std::valarray<double> &smpl);
    ///
    /// \param hdr Header values
    /// \param smpl Samples values
    ///
    trace(std::valarray<int32_t> &&hdr, std::valarray<double> &&smpl);
    ~trace();
    trace &operator=(const trace &other);
    trace &operator=(trace &&other) noexcept;
    ///
    /// \fn header
    /// \brief Trace header getter
    /// \return Header values
    ///
    std::valarray<int32_t> const &header();
    ///
    /// \fn samples
    /// \brief Trace samples getter
    /// \return Samples values
    ///
    std::valarray<double> const &samples();
    int32_t seq_num_in_line();
    int32_t seq_num_in_segy();
    int32_t ffid();
private:
    class impl;
    std::experimental::propagate_const<std::unique_ptr<impl>> pimpl;
};
} // namespace sedaman

#endif // SEDAMAN_TRACE_HPP
