#pragma once

#include "wider.h"
#include <iostream>
#include <stdint.h>
#include <stdio.h>

namespace wider_detail {

template<class T> constexpr void multiply_by(T& t, wider_traits::index_constant<10>) { t *= 10; }
template<class T> constexpr void multiply_by(T& t, wider_traits::index_constant<16>) { t <<= 4; }

template<class T, int Base>
constexpr T parse_it2(const char *first, const char *last) {
    T result = T(0);
    for (const char *p = first; p != last; ++p) {
        char c = *p;
        if ('0' <= c && c <= '9') {
            multiply_by(result, wider_traits::index_constant<Base>{});
            result += T(c - '0');
        } else if (Base == 16 && 'a' <= c && c <= 'f') {
            result <<= 4;
            result += T(c - 'a' + 10);
        } else if (Base == 16 && 'A' <= c && c <= 'F') {
            result <<= 4;
            result += T(c - 'A' + 10);
        }
    }
    return result;
}

template<class T>
constexpr T parse_it(const char *first, const char *last) {
    if (last - first >= 2 && first[0] == '0' && first[1] == 'x') {
        return wider_detail::parse_it2<T, 16>(first + 2, last);
    } else {
        // Wide multiplication by 10 is not yet implemented.
        return T(wider_detail::parse_it2<uint64_t, 10>(first, last));
    }
}

inline void print_it(std::ostream& os, const uint64_t& value, bool trailing) {
    char buf[17];
    if (trailing) {
        snprintf(buf, sizeof buf, "%016llx", (unsigned long long)value);
    } else {
        snprintf(buf, sizeof buf, "%llx", (unsigned long long)value);
    }
    std::ostream os2(os.rdbuf());
    os2 << buf;
}

template<class T>
void print_it(std::ostream& os, const Wider<T>& value, bool trailing) {
    if (trailing) {
        wider_detail::print_it(os, value.hi, true);
        wider_detail::print_it(os, value.lo, true);
    } else if (value.hi) {
        wider_detail::print_it(os, value.hi, false);
        wider_detail::print_it(os, value.lo, true);
    } else {
        wider_detail::print_it(os, value.lo, false);
    }
}

} // namespace wider_detail

template<char... Cs>
constexpr auto operator""_u128() {
    using Uint128 = Wider<uint64_t>;
    const char arr[] = { Cs... };
    return wider_detail::parse_it<Uint128>(arr, arr + sizeof...(Cs));
}

template<char... Cs>
constexpr auto operator""_u256() {
    using Uint128 = Wider<uint64_t>;
    using Uint256 = Wider<Uint128>;
    const char arr[] = { Cs... };
    return wider_detail::parse_it<Uint256>(arr, arr + sizeof...(Cs));
}

template<char... Cs>
constexpr auto operator""_u512() {
    using Uint128 = Wider<uint64_t>;
    using Uint256 = Wider<Uint128>;
    using Uint512 = Wider<Uint256>;
    const char arr[] = { Cs... };
    return wider_detail::parse_it<Uint512>(arr, arr + sizeof...(Cs));
}

template<class T>
std::ostream& operator<<(std::ostream& os, const Wider<T>& value)
{
    os << "0x";
    wider_detail::print_it(os, value, false);
    return os;
}

template<class T>
std::istream& operator>>(std::istream& is, Wider<T>& value)
{
    std::string input;
    is >> input;
    value = wider_detail::parse_it<Wider<T>>(input.data(), input.data() + input.size());
    return is;
}
