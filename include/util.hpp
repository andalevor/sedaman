/// \file util.hpp
/// \brief header file with utility functions
/// \author andalevor
/// \date   2019/11/07

#ifndef SEDAMAN_UTIL_HPP
#define SEDAMAN_UTIL_HPP

#include <bitset>
#include <cassert>
#include <cstring>
#include <vector>

/// \namespace sedaman
/// \brief General namespace for sedaman library.
namespace sedaman {
constexpr int32_t ceil(float num)
{
    return (static_cast<float>(static_cast<int32_t>(num)) == num)
        ? static_cast<int32_t>(num)
        : static_cast<int32_t>(num) + ((num > 0) ? 1 : 0);
}

/// \brief reads desired type
/// Reads from buffer necessary quantity of bytes and converts them.
/// \param buf pointer to buffer to read from
/// \return desired value
template <typename T>
T read(char const** buf)
{
    T result;
    memcpy(&result, *buf, sizeof(T));
    *buf += sizeof(T);
    return result;
}

/// \brief writes specified value to buffer
/// Writes to buffer value converted of bytes.
/// \param buf pointer to buffer to write to
template <typename T>
void write(char** buf, T val)
{
    memcpy(*buf, &val, sizeof(T));
    *buf += sizeof(T);
}

/// \brief swaps bytes
/// \param val
/// \return swapped value
template <typename T>
T swap(T const val)
{
    T result;
    unsigned char* to = reinterpret_cast<unsigned char*>(&result);
    unsigned char const* from = reinterpret_cast<unsigned char const*>(&val) +
	   	sizeof(T) - 1;
    for (int counter = sizeof(T); counter; --counter)
        *to++ = *from--;
    return result;
}

template <typename T>
T from_bcd(char const** buf, bool skip_first, int num)
{
    assert(num);
    char const* ptr = *buf;
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

template <typename T>
void to_bcd(char** buf, T val, bool skip_first, int num)
{
    //int num = std::to_string(val).size();
    assert(num <= 12);
    constexpr int val_size = sizeof(val);
    constexpr int bitset_size = ceil(val_size * 8 / 3.0) * 4 + val_size * 8;
    std::bitset<bitset_size> storage(val);
    std::vector<int> shifts(num);
    for (int i = 0; i < num; ++i)
        shifts[i] = val_size * 8 + i * 4;
    std::vector<std::bitset<bitset_size>> bcd_dig_mask(num);
    for (int i = 0; i < num; ++i)
        bcd_dig_mask[i] = std::bitset<bitset_size>(0xf) << shifts[i];
    std::bitset<bitset_size> new_dig;
    uint8_t bcd_dig = 0;
    // double dabble
    for (int i = 0; i < val_size * 8; ++i) {
        for (int j = 0; j < num; ++j) {
            bcd_dig = ((storage & bcd_dig_mask[j]) >> shifts[j]).to_ullong();
            if (bcd_dig > 4) {
                storage &= ~bcd_dig_mask[j];
                new_dig = bcd_dig + 3;
                storage |= (new_dig << shifts[j]);
            }
        }
        storage <<= 1;
    }
    bcd_dig = ((storage & bcd_dig_mask[num - 1]) >>
			   shifts[num - 1]).to_ullong();
    if (skip_first) {
        **buf |= bcd_dig;
        ++(*buf);
        --num;
    }
    while (num > 1) {
        bcd_dig = ((storage & bcd_dig_mask[num - 1]) >>
				   shifts[num - 1]).to_ullong();
        **buf = bcd_dig << 4;
        --num;
        bcd_dig = ((storage & bcd_dig_mask[num - 1]) >>
				   shifts[num - 1]).to_ullong();
        **buf |= bcd_dig;
        --num;
        ++(*buf);
    }
    if (num) {
        bcd_dig = ((storage & bcd_dig_mask[num - 1]) >>
				   shifts[num - 1]).to_ullong();
        **buf = bcd_dig << 4;
        --num;
    }
}
} // namespace sedaman

#endif // SEDAMAN_UTIL_HPP
