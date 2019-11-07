///
/// \file util.hpp
/// \brief header file with isegy class declaration
///
/// \author andalevor
///
/// \date   2019/11/07
///

#ifndef SEDAMAN_UTIL_HPP
#define SEDAMAN_UTIL_HPP

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
template<typename T>
T read(const char **buf)
{
    T result;
    memcpy(&result, *buf, sizeof(T));
    *buf += sizeof(T);
    return result;
}
///
/// \fn swap
/// \brief swaps bytes
/// \param val
/// \return swapped value
///
template<typename T>
T swap(const T val)
{
    T result;
    char *to = reinterpret_cast<char *>(&result);
    const char *from = reinterpret_cast<const char *>(&val) + sizeof(T) - 1;
    for (int counter = sizeof(T); counter; --counter)
        *to++ = *from--;
    return result;
}
} // namespace sedaman

#endif // SEDAMAN_UTIL_HPP
