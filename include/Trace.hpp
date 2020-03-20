///
/// \file Trace.hpp
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
#include <unordered_map>
#include <valarray>
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
class Trace {
public:
    ///
    /// \class header
    /// \brief Trace header class
    ///
    class Header {
    public:
        enum class ValueType {
            i8,
            i16,
            i32,
            i64,
            u8,
            u16,
            u32,
            u64,
            f32,
            f64
        };
        ///
        /// \param hdr Header to copy data from
        ///
        Header(Header const& hdr);
        ///
        /// \param hdr Header to move data from
        ///
        Header(Header&& hdr);
        ///
        /// \param hdr Header bytes
        ///
        Header(std::vector<char> const& hdr,
            std::unordered_map<std::string, std::pair<int, ValueType>> const& hdr_map);
        ///
        /// \param hdr Header bytes
        ///
        Header(std::vector<char>&& hdr,
            std::unordered_map<std::string, std::pair<int, ValueType>>&& hdr_map);
        ~Header();
        Header& operator=(Header const& other);
        Header& operator=(Header&& other) noexcept;

    private:
        class Impl;
        std::experimental::propagate_const<std::unique_ptr<Impl>> pimpl;
    };
    ///
    /// \param trc Trace to copy data from
    ///
    Trace(Trace const& trc);
    ///
    /// \param trc Trace to move data from
    ///
    Trace(Trace&& trc) noexcept;
    ///
    /// \param hdr Header values
    /// \param smpl Samples values
    ///
    Trace(std::vector<char> const& hdr, std::valarray<double> const& smpl,
        std::unordered_map<std::string, std::pair<int, Header::ValueType>> const& hdr_map);
    ///
    /// \param hdr Header values
    /// \param smpl Samples values
    ///
    Trace(std::vector<char>&& hdr, std::valarray<double>&& smpl,
        std::unordered_map<std::string, std::pair<int, Header::ValueType>>&& hdr_map);
    ~Trace();
    Trace& operator=(Trace const& other);
    Trace& operator=(Trace&& other) noexcept;
    ///
    /// \fn header
    /// \brief Trace header getter
    /// \return Reference to trace header
    ///
    Header const& header() const;
    ///
    /// \fn samples
    /// \brief Trace samples getter
    /// \return Samples values
    ///
    std::valarray<double> const& samples() const;

private:
    class Impl;
    std::experimental::propagate_const<std::unique_ptr<Impl>> pimpl;
};
} // namespace sedaman

#endif // SEDAMAN_TRACE_HPP
