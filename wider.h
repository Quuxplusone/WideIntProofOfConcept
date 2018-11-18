#pragma once

#include <stdint.h>
#include <x86intrin.h>

namespace traits {
    template<class T> struct bit_width { static constexpr size_t value = T::bit_width; };
    template<> struct bit_width<uint64_t> { static constexpr size_t value = 64; };
} // namespace traits

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

template<class Int64>
struct Wider {
    Int64 lo;
    Int64 hi;

    static constexpr size_t bit_width = 2 * traits::bit_width<Int64>::value;

    constexpr Wider() = default;
    constexpr explicit Wider(int s) noexcept : lo(s), hi((s < 0) ? -1 : 0) {}
    constexpr operator bool() const noexcept { return bool(lo | hi); }

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

    friend Wider& operator+=(Wider& x, const Wider& y) { (void)producecarry(x, y); return x; }
    friend Wider& operator-=(Wider& x, const Wider& y) { (void)produceborrow(x, y); return x; }
    friend Wider& operator^=(Wider& x, const Wider& y) { x.lo ^= y.lo; x.hi ^= y.hi; return x; }
    friend Wider& operator&=(Wider& x, const Wider& y) { x.lo &= y.lo; x.hi &= y.hi; return x; }
    friend Wider& operator|=(Wider& x, const Wider& y) { x.lo |= y.lo; x.hi |= y.hi; return x; }
    friend Wider operator+(Wider x, const Wider& y) { x += y; return x; }
    friend Wider operator-(Wider x, const Wider& y) { x -= y; return x; }
    friend Wider operator^(Wider x, const Wider& y) { x ^= y; return x; }
    friend Wider operator&(Wider x, const Wider& y) { x &= y; return x; }
    friend Wider operator|(Wider x, const Wider& y) { x |= y; return x; }

    friend bool operator<(Wider x, const Wider& y) { return produceborrow(x, y); }
    friend bool operator>(const Wider& x, const Wider& y) { return (y < x); }
    friend bool operator>=(const Wider& x, const Wider& y) { return !(x < y); }
    friend bool operator<=(const Wider& x, const Wider& y) { return !(y < x); }
    friend bool operator==(const Wider& x, const Wider& y) { return !((x.lo ^ y.lo) | (x.hi ^ y.hi)); }
    friend bool operator!=(const Wider& x, const Wider& y) { return !(x == y); }
    friend bool operator!(const Wider& x) { return !bool(x); }
};

using Uint128 = Wider<uint64_t>;
using Uint256 = Wider<Uint128>;
using Uint512 = Wider<Uint256>;
using Uint1024 = Wider<Uint512>;

template<class T>
struct Tests {
    //static void pluseq(T *p, const T *q)  { *p += *q; }
    //static void plus(T *p, const T *q)       { *p = *p + *q; }
    //static void minuseq(T *p, const T *q) { *p -= *q; }
    //static void minus(T *p, const T *q)      { *p = *p - *q; }
    //static void xoreq(T *p, const T *q)      { *p ^= *q; }
    //static void xor_(T *p, const T *q)       { *p = *p ^ *q; }
    //static void oreq(T *p, const T *q)       { *p |= *q; }
    //static void or_(T *p, const T *q)        { *p = *p | *q; }
    //static void andeq(T *p, const T *q)      { *p &= *q; }
    //static void and_(T *p, const T *q)       { *p = *p & *q; }
    //static bool lt(const T *p, const T *q)   { return *p < *q; }
    //static bool leq(const T *p, const T *q)  { return *p <= *q; }
    //static bool gt(const T *p, const T *q)   { return *p > *q; }
    //static bool geq(const T *p, const T *q)  { return *p >= *q; }
    //static bool eq(const T *p, const T *q)   { return *p == *q; }
    //static bool neq(const T *p, const T *q)  { return *p != *q; }
    //static bool not_(const T *p)             { return !*p; }
    //static bool bool_(const T *p)            { return *p; }
};

template struct Tests<Uint128>;
template struct Tests<__uint128_t>;
//template struct Tests<Uint512>;
