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
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <valarray>
#include <variant>
#include <vector>

///
/// \namespace sedaman
/// \brief General namespace for sedaman library.
///
namespace sedaman
{
    ///
    /// \class trace
    /// \brief Class to store trace header and trace samples.
    ///
    class Trace
    {
    public:
        ///
        /// \class header
        /// \brief Trace header class
        ///
        class Header
        {
        public:
            typedef std::variant<int8_t, int16_t, int32_t, int64_t, uint8_t, uint16_t, uint32_t, uint64_t, float, double> Value;
            ///
            /// \param hdr Header to copy data from
            ///
            Header(Header const &hdr);
            ///
            /// \param hdr Header to move data from
            ///
            Header(Header &&hdr);
            ///
            /// \param hdr Header bytes
            ///
            Header(std::unordered_map<std::string, Value> hdr);
            ~Header();
            Header &operator=(Header const &other);
            Header &operator=(Header &&other) noexcept;
            ///
            /// \fn get
            /// \brief gets header value by specified key
            /// \param key to get assosiated value
            /// \return optional value
            ///
            std::optional<Value> get(std::string key) const;
            ///
            /// \fn set
            /// \brief sets or adds header value by specified key
            /// \param key to get assosiated value
            /// \param v value to set
            ///
            void set(std::string key, Value v);

        private:
            class Impl;
            std::unique_ptr<Impl> pimpl;
        };
        ///
        /// \param trc Trace to copy data from
        ///
        Trace(Trace const &trc);
        ///
        /// \param trc Trace to move data from
        ///
        Trace(Trace &&trc) noexcept;
        ///
        /// \param hdr Header values
        /// \param smpl Samples values
        ///
        Trace(std::unordered_map<std::string, Header::Value> hdr,
              std::valarray<double> smpl);
        ~Trace();
        Trace &operator=(Trace const &other);
        Trace &operator=(Trace &&other) noexcept;
        ///
        /// \fn header
        /// \brief Trace header getter
        /// \return Reference to trace header
        ///
        Header const &header() const;
        ///
        /// \fn samples
        /// \brief Trace samples getter
        /// \return Samples values
        ///
        std::valarray<double> const &samples() const;

    private:
        class Impl;
        std::unique_ptr<Impl> pimpl;
    };
} // namespace sedaman

#endif // SEDAMAN_TRACE_HPP
