///
/// @file Trace.hpp
/// @author Andrei Voronin (andalevor@gmail.com)
/// \brief 
/// @version 0.1
/// \date 2019-06-06
/// 
/// @copyright Copyright (c) 2019
/// 
///
#ifndef SEDAMAN_TRACE_HPP
#define SEDAMAN_TRACE_HPP

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>
///
/// \brief General namespace for sedaman library.
/// \namespace sedaman
///
/// 
namespace sedaman {
///
/// \brief Class to store trace header and trace samples.
/// \class Trace
///
/// 
class Trace {
public:
    ///
    /// \brief Trace header class
    /// \class Header
    /// 
    ///
    class Header {
    public:
        using Value = std::variant<int64_t, double>;
        ///
        /// \brief Enumiration to set type of additional trace header values.
        /// \enum
        ///
        /// 
        enum class ValueType {
            int8_t,
            uint8_t,
            int16_t,
            uint16_t,
            int24_t,
            uint24_t,
            int32_t,
            uint32_t,
            int64_t,
            uint64_t,
            ibm,
            ieee_single,
            ieee_double
        };
        ///
        /// \brief Construct a new Header object
        /// 
        /// \param hdr Header to copy data from
        ///
        Header(Header const& hdr);
        ///
        /// \brief Construct a new Header object
        /// 
        /// \param hdr Header to move data from
        ///
        Header(Header&& hdr);
        ///
        /// \brief Construct a new Header object
        /// 
        /// \param hdr Header bytes
        ///
        Header(std::unordered_map<std::string, Value> hdr);
        ~Header();
        ///
        /// \brief copy assignment
        /// 
        /// \param other 
        /// \return Header& 
        ///
        Header& operator=(Header const& other);
        ///
        /// \brief move assignment
        /// 
        /// \param other 
        /// \return Header& 
        ///
        Header& operator=(Header&& other) noexcept;
        ///
        /// \brief gets header value by specified key
        /// 
        /// \param key to get assosiated value
        /// \return std::optional<Value> 
        ///
        std::optional<Value> get(std::string key) const;
        ///
        /// \brief sets or adds header value by specified key
        /// 
        /// \param key to get assosiated value
        /// \param v value to set
        ///
        void set(std::string key, Value v);
        ///
        /// \brief Retruns all keys for header
        /// 
        /// \return std::vector<std::string> 
        ///
        std::vector<std::string> keys() const;

    private:
        class Impl;
        std::unique_ptr<Impl> pimpl;
    };
    ///
    /// \brief Construct a new Trace object
    /// 
    /// \param trc Trace to copy data from
    ///
    Trace(Trace const& trc);
    ///
    /// \brief Construct a new Trace object
    /// 
    /// \param trc Trace to move data from
    ///
    Trace(Trace&& trc) noexcept;
    ///
    /// \brief Construct a new Trace object
    /// 
    /// \param hdr Header values
    /// \param smpl Sample values
    ///
    Trace(std::unordered_map<std::string, Header::Value> hdr,
        std::vector<double> smpl);
    ~Trace();
    ///
    /// \brief copy assignment
    /// 
    /// \param other 
    /// \return Trace& 
    ///
    Trace& operator=(Trace const& other);
    ///
    /// \brief move assignment
    /// 
    /// \param other 
    /// \return Trace& 
    ///
    Trace& operator=(Trace&& other) noexcept;
    ///
    /// \brief Trace header getter
    /// 
    /// \return Header& 
    ///
    Header& header();
    ///
    /// \brief Trace header getter
    /// 
    /// \return Header const& 
    ///
    Header const& header_const() const;
    ///
    /// \brief Trace samples getter
    /// 
    /// \return std::vector<double> const& 
    ///
    std::vector<double> const& samples() const;

private:
    class Impl;
    std::unique_ptr<Impl> pimpl;
};
} // namespace sedaman

#endif // SEDAMAN_TRACE_HPP
