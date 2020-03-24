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
#include <variant>
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
        typedef std::variant<int8_t, int16_t, int32_t, int64_t, uint8_t, uint16_t, uint32_t, uint64_t, float, double> Value;
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
        Header(std::unordered_map<std::string, Value> const& hdr);
        ///
        /// \param hdr Header bytes
        ///
        Header(std::unordered_map<std::string, Value>&& hdr);
        ~Header();
        Header& operator=(Header const& other);
        Header& operator=(Header&& other) noexcept;
        ///
        /// \fn get
        /// \brief gets header value by specified key
        /// \param key to get assosiated value
        /// \return value
        /// \throws out_of_range if there is no such key
        ///
        Value const& get(std::string const& key);
        Value const& get(std::string&& key);
        ///
        /// \fn get_mut
        /// \brief gets mutable header value by specified key
        /// \param key to get assosiated value
        /// \return value
        /// \throws out_of_range if there is no such key
        ///
        Value& get_mut(std::string const& key);
        Value& get_mut(std::string&& key);
        ///
        /// \fn insert
        /// \brief inserts new header with given value
        /// \param pair of key and value
        /// \return true if insertion was successful
        /// Inserts only if there was no such header.
        ///
        bool insert(std::pair<std::string, Value> const& pair);
        bool insert(std::pair<std::string, Value>&& pair);

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
    Trace(std::unordered_map<std::string, Header::Value> const& hdr,
        std::valarray<double> const& smpl);
    ///
    /// \param hdr Header values
    /// \param smpl Samples values
    ///
    Trace(std::unordered_map<std::string, Header::Value>&& hdr,
        std::valarray<double>&& smpl);
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
