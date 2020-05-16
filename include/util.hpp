///
/// \file util.hpp
/// \brief header file with utility functions
///
/// \author andalevor
///
/// \date   2019/11/07
///

#ifndef SEDAMAN_UTIL_HPP
#define SEDAMAN_UTIL_HPP

#include <cassert>
#include <cstring>

///
/// \namespace sedaman
/// \brief General namespace for sedaman library.
///
namespace sedaman {

///
/// \fn read
/// \brief reads desired type
/// Reads from buffer necessary quantity of bytes and converts them.
/// \param buf pointer to buffer to read from
/// \return desired value
///
template <typename T>
T read(char const** buf)
{
    T result;
    memcpy(&result, *buf, sizeof(T));
    *buf += sizeof(T);
    return result;
}

///
/// \fn write
/// \brief writes specified value to buffer
/// Writes to buffer value converted of bytes.
/// \param buf pointer to buffer to write to
///
template <typename T>
void write(char** buf, T val)
{
    memcpy(*buf, &val, sizeof(T));
    *buf += sizeof(T);
}

///
/// \fn swap
/// \brief swaps bytes
/// \param val
/// \return swapped value
///
template <typename T>
T swap(T const val)
{
    T result;
    unsigned char* to = reinterpret_cast<unsigned char*>(&result);
    unsigned char const* from = reinterpret_cast<unsigned char const*>(&val) + sizeof(T) - 1;
    for (int counter = sizeof(T); counter; --counter)
        *to++ = *from--;
    return result;
}

template <typename T>
T from_bcd(char const **buf, bool skip_first, int num)
{
    assert(num);
    char const *ptr = *buf;
    T result = 0;
    int first = (static_cast<unsigned>(*ptr) & 0xf0) >> 4;
    int second = static_cast<unsigned>(*ptr) & 0x0f;
    ++ptr;
    int counter = 0;
    if (!skip_first) {
        result += first;
         ++counter;
    }
    if (num != counter) {
        result *= 10;
        result += second;
        ++counter;
    }
    for (int i = num - counter; i > 1; i -= 2) {
        int first = (static_cast<unsigned>(*ptr) & 0xf0) >> 4;
        int second = static_cast<unsigned>(*ptr) & 0x0f;
        ++ptr;
        result *= 10;
        result += first;
        ++counter;
        result *= 10;
        result += second;
        ++counter;
    }
    if (num != counter) {
        int first = (static_cast<unsigned>(*ptr) & 0xf0) >> 4;
        result *= 10;
        result += first;
    }
    *buf += (num + skip_first) / 2;
    return result;
}
} // namespace sedaman

#endif // SEDAMAN_UTIL_HPP
