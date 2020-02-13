#pragma once

#include <stdint.h>
#include <tuple>
#include <type_traits>
#include <utility>

#ifdef _MSC_VER
#include <intrin.h>
#else // _MSC_VER
#include <x86intrin.h>
#endif // _MSC_VER

namespace wider_traits {

    template<size_t K> using index_constant = std::integral_constant<size_t, K>;

    template<class T> struct bit_width { static constexpr size_t value = T::bit_width; };
    template<> struct bit_width<uint64_t> { static constexpr size_t value = 64; };

    template<class T> struct array_helper {
        template<class... Ints>
        static T from_array(Ints... ints) {
            return T::from_array(std::make_index_sequence<sizeof...(Ints) / 2>(), ints...);
        }
        template<class F>
        static auto with_array(const T& t, const F& f) {
            using Int64 = typename T::half_type;
            return array_helper<Int64>::with_array(t.lo, [&](auto... los) {
                return array_helper<Int64>::with_array(t.hi, [&](auto... his) {
                    return f(los..., his...);
                });
            });
        }
    };
    template<> struct array_helper<uint64_t> {
        static uint64_t from_array(uint64_t x) {
            return x;
        }
        template<class F>
        static auto with_array(uint64_t x, const F& f) {
            return f(x);
        }
    };

    template<size_t I, class... Ts>
    uint64_t& get_helper(Ts&&... ts) {
        std::tuple<Ts&&...> tuple(std::forward<Ts>(ts)...);
        return std::get<I>(tuple);
    }

} // namespace wider_traits

using CarryFlag = bool;

inline CarryFlag producecarry(uint64_t& x, uint64_t y) {
    x += y;
    return (x < y);
}

inline CarryFlag addcarry(CarryFlag cf, uint64_t& x, uint64_t y) {
    return _addcarry_u64(cf, x, y, (unsigned long long*)&x);
}

inline CarryFlag produceborrow(uint64_t& x, uint64_t y) {
    CarryFlag cf = (x < y);
    x -= y;
    return cf;
}

inline CarryFlag subborrow(CarryFlag cf, uint64_t& x, uint64_t y) {
    return _subborrow_u64(cf, x, y, (unsigned long long*)&x);
}

inline uint64_t mulxu(uint64_t a, uint64_t b, uint64_t *rhi) {
#if defined(WIDER_COMPLETELY_STANDARD)
    auto hi = [](uint64_t x) { return x >> 32; };
    auto lo = [](uint64_t x) { return uint32_t(x); };
    uint64_t xl = lo(a);
    uint64_t xh = hi(a);
    uint64_t yl = lo(b);
    uint64_t yh = hi(b);

    uint64_t rhh = xh * yh;
    uint64_t rhl = xh * yl;
    uint64_t rlh = xl * yh;
    uint64_t rll = xl * yl;

    *rhi = rhh + hi(rhl + hi(rll)) + hi(rlh + lo(rhl + hi(rll)));
    return a * b;
#elif defined(_MSC_VER)
    return _umul128(a, b, (unsigned long long*)rhi);
#elif defined(__BMI2__)
    return _mulx_u64(a, b, (unsigned long long*)rhi);
#else
    __uint128_t r = __uint128_t(a) * __uint128_t(b);
    *rhi = (r >> 64);
    return r;
#endif
}

inline uint64_t shl128(uint64_t low, uint64_t high, int n) {
#ifdef _MSC_VER
    return __shiftleft128(low, high, n);
#else
    __uint128_t v = (__uint128_t(high) << 64) | __uint128_t(low);
    return (v << (n & 63)) >> 64;
#endif
}

inline uint64_t shr128(uint64_t low, uint64_t high, int n) {
#ifdef _MSC_VER
    return __shiftright128(low, high, n);
#else
    __uint128_t v = (__uint128_t(high) << 64) | __uint128_t(low);
    return v >> (n & 63);
#endif
}

inline int countleadingzeros(uint64_t x) {
#ifdef _MSC_VER
    return __lzcnt64(x);
#else
    return __builtin_clzll(x);
#endif
}

#ifndef _MSC_VER
inline int countleadingzeros(__uint128_t x) {
    int lo = (63 ^ countleadingzeros(uint64_t(x))) + 64;
    int hi = 63 ^ countleadingzeros(uint64_t(x >> 64));
    return 63 ^ ((x >> 64) ? hi : lo);
}
#endif

template<class Int64>
struct Wider {
    Int64 lo;
    Int64 hi;

    static constexpr size_t bit_width = 2 * wider_traits::bit_width<Int64>::value;
    using half_type = Int64;

    constexpr Wider() = default;
    constexpr explicit Wider(int s) noexcept : lo(s), hi((s < 0) ? -1 : 0) {}
    constexpr explicit Wider(const Int64& s) noexcept : lo(s), hi(0) {}
    constexpr operator bool() const noexcept { return bool(lo | hi); }

    template<size_t... Indices, class... Ints>
    static Wider from_array(std::index_sequence<Indices...>, Ints... ints) {
        Wider w;
        w.lo = wider_traits::array_helper<Int64>::from_array(wider_traits::get_helper<Indices>(ints...)...);
        w.hi = wider_traits::array_helper<Int64>::from_array(wider_traits::get_helper<Indices + sizeof...(Indices)>(ints...)...);
        return w;
    }

    friend CarryFlag producecarry(Wider& x, const Wider& y) {
        return addcarry(producecarry(x.lo, y.lo), x.hi, y.hi);
    }

    friend CarryFlag produceborrow(Wider& x, const Wider& y) {
        return subborrow(produceborrow(x.lo, y.lo), x.hi, y.hi);
    }

    friend CarryFlag addcarry(CarryFlag cf, Wider& x, const Wider& y) {
        cf = addcarry(cf, x.lo, y.lo);
        cf = addcarry(cf, x.hi, y.hi);
        return cf;
    }

    friend CarryFlag subborrow(CarryFlag cf, Wider& x, const Wider& y) {
        cf = subborrow(cf, x.lo, y.lo);
        cf = subborrow(cf, x.hi, y.hi);
        return cf;
    }

    friend Wider mulxu(const Wider& a, const Wider& b, Wider *rhi)
    {
        Wider<Wider> result;
        Wider temp;
        temp.lo = mulxu(a.lo, b.lo, &temp.hi);
        result.lo.lo = temp.lo;
        result.lo.hi = temp.hi;
        temp.lo = mulxu(a.hi, b.hi, &temp.hi);
        result.hi.lo = temp.lo;
        result.hi.hi = temp.hi;
        temp.lo = mulxu(a.lo, b.hi, &temp.hi);
        bool cf = false;
        cf = addcarry(cf, result.lo.hi, temp.lo);
        cf = addcarry(cf, result.hi.lo, temp.hi);
        result.hi.hi += Int64(int(cf));
        temp.lo = mulxu(a.hi, b.lo, &temp.hi);
        cf = false;
        cf = addcarry(cf, result.lo.hi, temp.lo);
        cf = addcarry(cf, result.hi.lo, temp.hi);
        result.hi.hi += Int64(int(cf));
        *rhi = result.hi;
        return result.lo;
    }

    friend Wider operator*(const Wider& a, const Wider& b) {
        Wider result;
        result.lo = mulxu(a.lo, b.lo, &result.hi);
        result.hi += (a.lo * b.hi);
        result.hi += (a.hi * b.lo);
        return result;
    }

    template<size_t... Is, class... Ts>
    static void shift_left(int n, std::index_sequence<Is...>, Ts&&... parts) {
        using wider_traits::get_helper;
        int xx[] = {
            [n](auto I, auto&&... parts) {
                get_helper<I + 1>(parts...) = shl128(
                    get_helper<I>(parts...),
                    get_helper<I + 1>(parts...),
                    n
                );
                return 0;
            }(wider_traits::index_constant<sizeof...(Is) - Is - 1>(), parts...) ...
        };
        (void)xx;
        get_helper<0>(parts...) <<= (n & 63);
    }

    template<size_t... Is, class... Ts>
    static void shift_right(int n, std::index_sequence<Is...>, Ts&&... parts) {
        using wider_traits::get_helper;
        int xx[] = {
            [n](auto I, auto&&... parts) {
                get_helper<I>(parts...) = shr128(
                    get_helper<I>(parts...),
                    get_helper<I + 1>(parts...),
                    n
                );
                return 0;
            }(wider_traits::index_constant<Is>(), parts...) ...
        };
        (void)xx;
        get_helper<sizeof...(Is)>(parts...) >>= (n & 63);
    }

    friend Wider operator<<(const Wider& a, int n) {
        return wider_traits::array_helper<Wider>::with_array(a, [n](auto... parts) {
            shift_left(n, std::make_index_sequence<sizeof...(parts) - 1>(), parts...);
            uint64_t *ps[] = { &parts... };
            for (int shift = 1; shift < int(sizeof...(parts)); shift *= 2) {
                if (n & (shift * 64)) {
                    for (int i = sizeof...(parts) - 1; i >= 0; --i) {
                        if (i >= shift) {
                            *ps[i] = *ps[i - shift];
                        } else {
                            *ps[i] = 0;
                        }
                    }
                }
            }
            return wider_traits::array_helper<Wider>::from_array( parts... );
        });
    }

    friend Wider operator>>(const Wider& a, int n) {
        return wider_traits::array_helper<Wider>::with_array(a, [n](auto... parts) {
            shift_right(n, std::make_index_sequence<sizeof...(parts) - 1>(), parts...);
            uint64_t *ps[] = { &parts... };
            for (int shift = 1; shift < int(sizeof...(parts)); shift *= 2) {
                if (n & (shift * 64)) {
                    for (int i = 0; i < int(sizeof...(parts)); ++i) {
                        if (i + shift < int(sizeof...(parts))) {
                            *ps[i] = *ps[i + shift];
                        } else {
                            *ps[i] = 0;
                        }
                    }
                }
            }
            return wider_traits::array_helper<Wider>::from_array( parts... );
        });
    }

    friend int countleadingzeros(const Wider& x) {
        constexpr int _64 = bit_width/2;
        constexpr int _63 = _64 - 1;
        int lo = (_63 ^ countleadingzeros(x.lo)) + _64;
        int hi = _63 ^ countleadingzeros(x.hi);
        return _63 ^ ((x >> _64) ? hi : lo);
    }

    friend Wider divmod(Wider a, const Wider& b, Wider *remainder) {
        int leading_zeros = countleadingzeros(b);
        Wider subtrahend = b << leading_zeros;
        Wider digit = Wider(1) << leading_zeros;
        Wider quotient{0};
        while (digit) {
            if (subtrahend <= a) {
                a -= subtrahend;
                quotient |= digit;
            }
            subtrahend >>= 1;
            digit >>= 1;
        }
        *remainder = a;
        return quotient;
    }

    friend Wider operator/(const Wider& a, const Wider& b) {
        Wider remainder;
        return divmod(a, b, &remainder);
    }
    friend Wider operator%(const Wider& a, const Wider& b) {
        Wider remainder;
        (void)divmod(a, b, &remainder);
        return remainder;
    }
    friend Wider& operator%=(Wider& a, const Wider& b) {
        (void)divmod(a, b, &a);
        return a;
    }

    Wider& operator++() { *this += Wider(1); return *this; }
    Wider operator++(int) { Wider r = *this; ++*this; return r; }
    Wider& operator--() { *this -= Wider(1); return *this; }
    Wider operator--(int) { Wider r = *this; --*this; return r; }
    friend Wider& operator+=(Wider& x, const Wider& y) { (void)producecarry(x, y); return x; }
    friend Wider& operator-=(Wider& x, const Wider& y) { (void)produceborrow(x, y); return x; }
    friend Wider& operator*=(Wider& x, const Wider& y) { x = (x * y); return x; }
    friend Wider& operator/=(Wider& x, const Wider& y) { x = x / y; return x; }
    friend Wider& operator^=(Wider& x, const Wider& y) { x.lo ^= y.lo; x.hi ^= y.hi; return x; }
    friend Wider& operator&=(Wider& x, const Wider& y) { x.lo &= y.lo; x.hi &= y.hi; return x; }
    friend Wider& operator|=(Wider& x, const Wider& y) { x.lo |= y.lo; x.hi |= y.hi; return x; }
    friend Wider operator+(Wider x, const Wider& y) { x += y; return x; }
    friend Wider operator-(Wider x, const Wider& y) { x -= y; return x; }
    friend Wider operator^(Wider x, const Wider& y) { x ^= y; return x; }
    friend Wider operator&(Wider x, const Wider& y) { x &= y; return x; }
    friend Wider operator|(Wider x, const Wider& y) { x |= y; return x; }
    friend Wider& operator<<=(Wider& x, int y) { x = (x << y); return x; }
    friend Wider& operator>>=(Wider& x, int y) { x = (x >> y); return x; }

    friend bool operator<(Wider x, const Wider& y) { return produceborrow(x, y); }
    friend bool operator>(const Wider& x, const Wider& y) { return (y < x); }
    friend bool operator>=(const Wider& x, const Wider& y) { return !(x < y); }
    friend bool operator<=(const Wider& x, const Wider& y) { return !(y < x); }
    friend bool operator==(const Wider& x, const Wider& y) { return !((x.lo ^ y.lo) | (x.hi ^ y.hi)); }
    friend bool operator!=(const Wider& x, const Wider& y) { return !(x == y); }
    friend bool operator!(const Wider& x) { return !bool(x); }
    friend Wider operator-(const Wider& x) { return Wider(0) - x; }
    friend Wider operator~(const Wider& x) { return Wider(-1) ^ x; }
};

namespace wider_tests {

using Uint128 = Wider<uint64_t>;
using Uint256 = Wider<Uint128>;
using Uint512 = Wider<Uint256>;
using Uint1024 = Wider<Uint512>;

template<class T>
struct Tests {
    //static void preinc(T *p)                 { ++*p; }
    //static void postinc(T *p, T *q)          { *p = (*q)++; }
    //static void predec(T *p)                 { --*p; }
    //static void postdec(T *p, T *q)          { *p = (*q)--; }
    //static void plus(T *p, const T *q)       { *p = *p + *q; }
    //static void pluseq(T *p, const T *q)     { *p += *q; }
    //static void minus(T *p, const T *q)      { *p = *p - *q; }
    //static void minuseq(T *p, const T *q)    { *p -= *q; }
    //static void mul(T *p, const T *q)        { *p = *p * *q; }
    //static void muleq(T *p, const T *q)      { *p *= *q; }
    //static void div(T *p, const T *q)        { *p = *p / *q; }
    //static void diveq(T *p, const T *q)      { *p /= *q; }
    //static void mod(T *p, const T *q)        { *p = *p % *q; }
    //static void modeq(T *p, const T *q)      { *p %= *q; }
    //static void xor_(T *p, const T *q)       { *p = *p ^ *q; }
    //static void xoreq(T *p, const T *q)      { *p ^= *q; }
    //static void and_(T *p, const T *q)       { *p = *p & *q; }
    //static void andeq(T *p, const T *q)      { *p &= *q; }
    //static void or_(T *p, const T *q)        { *p = *p | *q; }
    //static void oreq(T *p, const T *q)       { *p |= *q; }
    //static void shl(T *p, int q)             { *p = *p << q; }
    //static void shleq(T *p, int q)           { *p <<= q; }
    //static void shr(T *p, int q)             { *p = *p >> q; }
    //static void shreq(T *p, int q)           { *p >>= q; }
    //static int clz(T *p)                     { return countleadingzeros(*p); }
    //static bool lt(const T *p, const T *q)   { return *p < *q; }
    //static bool leq(const T *p, const T *q)  { return *p <= *q; }
    //static bool gt(const T *p, const T *q)   { return *p > *q; }
    //static bool geq(const T *p, const T *q)  { return *p >= *q; }
    //static bool eq(const T *p, const T *q)   { return *p == *q; }
    //static bool neq(const T *p, const T *q)  { return *p != *q; }
    //static bool not_(const T *p)             { return !*p; }
    //static bool bool_(const T *p)            { return *p; }
    //static void neg(T *p)                    { *p = -*p; }
    //static void flip(T *p)                   { *p = ~*p; }
};

//template struct Tests<__uint128_t>;
//template struct Tests<Uint128>;
//template struct Tests<Uint256>;
//template struct Tests<Uint512>;

} // namespace wider_tests
