Wide Integer Math (Proof of Concept)
------------------------------------

This header provides a class template `Wider<T>`, such that `Wider<uint64_t>`
acts like an unsigned 128-bit type, `Wider<Wider<uint64_t>>` acts like an unsigned
256-bit type, and so on. `Wider<T>` overloads all the arithmetic operators,
and also provides a free function `countleadingzeros(w)`.

A sufficiently smart compiler should produce the same codegen for `Wider<uint64_t>`
as it does for the built-in primitive type `__uint128_t`. Any significant differences
in codegen (in either direction) should probably be filed as "missed optimization"
bugs against the relevant compiler.

[As of April 2020, Clang supports](http://blog.llvm.org/2020/04/the-new-clang-extint-feature-provides.html)
a built-in type `unsigned _ExtInt(n)` that implements `n`-bit arithmetic for any `n` at all, even
non-powers of two. A sufficiently smart compiler should produce the same codegen for `Wider<Wider<...>>`
as it does for `unsigned _ExtInt(n)`.

The following tables show naïve instruction counts (not counting labels, but counting the `ret`)
for each member function of `wider_tests::Tests<W>`. The letter "P" indicates codegen
that appears perfect as far as I know. The word "call" indicates failure to completely
inline the code (usually because it's so big).

"`__udivti3`" and "`__umodti3`" indicate that certain arithmetic operations on `__uint128_t`
are delegated to a library function. The library function is macro-optimized with
many special cases (division by small integers, division by integers with many trailing zeros,
et cetera). In contrast, `Wider`'s `operator/` omits these special cases,
resulting in smaller code but almost certainly slower code in common situations.

All numbers are produced by Godbolt Compiler Explorer, using `-O3 -std=c++14`, on GCC trunk and Clang trunk.


| Test name  | Clang `uint128` | Clang `_ExtInt` | Clang `W<u64>` | GCC `uint128` | GCC `W<u64>` |
| ---------- | --------------- | --------------- | -------------- | ------------- | ------------ |
| preinc     | 3 P             | 3 P             | 3 P            | 3 P           | 6            |
| postinc    | 11              | 11              | 11             | 11            | 13           |
| predec     | 3 P             | 3 P             | 3 P            | 3 P           | 6            |
| postdec    | 11              | 11              | 11             | 11            | 13           |
| plus       | 5 P             | 5 P             | 5 P            | 5 P           | 13           |
| pluseq     | 5 P             | 5 P             | 5 P            | 5 P           | 6            |
| minus      | 5 P             | 5 P             | 5 P            | 5 P           | 16           |
| minuseq    | 5 P             | 5 P             | 5 P            | 5 P           | 11           |
| mul        | 11 P            | 11 P            | 11 P           | 12            | 12           |
| muleq      | 11 P            | 11 P            | 11 P           | 12            | 12           |
| div        | __udivti3       | __udivti3       | 49             | __udivti3     | 81           |
| diveq      | __udivti3       | __udivti3       | 49             | __udivti3     | 81           |
| mod        | __umodti3       | __umodti3       | 39             | __umodti3     | 72           |
| modeq      | __umodti3       | __umodti3       | 39             | __umodti3     | 72           |
| xor_       | 5 P             | 5 P             | 5 P            | 5 P           | 5 P          |
| xoreq      | 5 P             | 5 P             | 5 P            | 5 P           | 5 P          |
| and_       | 5 P             | 5 P             | 5 P            | 5 P           | 5 P          |
| andeq      | 5 P             | 5 P             | 5 P            | 5 P           | 5 P          |
| or_        | 5 P             | 5 P             | 5 P            | 5 P           | 5 P          |
| oreq       | 5 P             | 5 P             | 5 P            | 5 P           | 5 P          |
| shl        | 13              | 13              | 12 P           | 12 P          | 25           |
| shleq      | 13              | 13              | 12 P           | 12 P          | 25           |
| shr        | 13              | 13              | 12 P           | 12 P          | 23           |
| shreq      | 13              | 13              | 12 P           | 12 P          | 23           |
| clz        | 7               | unsupported     | 7              | 11            | 11           |
| lt         | 6 P             | 6 P             | 6 P            | 6 P           | 11           |
| leq        | 6 P             | 6 P             | 6 P            | 7             | 11           |
| gt         | 6 P             | 6 P             | 6 P            | 7             | 11           |
| geq        | 6 P             | 6 P             | 6 P            | 6 P           | 11           |
| eq         | 6 P             | 7               | 7              | 7             | 7            |
| neq        | 6 P             | 7               | 7              | 7             | 7            |
| not_       | 4 P             | 4 P             | 4 P            | 4 P           | 4 P          |
| bool_      | 4 P             | 4 P             | 4 P            | 4 P           | 4 P          |
| neg        | 5               | 5               | 5              | 4 P           | 13           |
| flip       | 3 P             | 3 P             | 5              | 3 P           | 5            |


256-bit math using `unsigned _ExtInt(256)` and `Wider<Wider<uint64_t>>`:

| Test name  | Clang `_ExtInt` | Clang `W<W<u64>>` | GCC `W<W<u64>>` |
| ---------- | --------------- | ----------------- | --------------- |
| preinc     | 5 P             | 5 P               | 13              |
| postinc    | 23              | 13                | 29              |
| predec     | 5 P             | 5 P               | 17              |
| postdec    | 23              | 13                | 30              |
| plus       | 9 P             | 9 P               | 26              |
| pluseq     | 9 P             | 9 P               | 18              |
| minus      | 9 P             | 9 P               | 30              |
| minuseq    | 9 P             | 9 P               | 23              |
| mul        | 60              | 60                | 67              |
| muleq      | 62              | 60                | 67              |
| div        | unsupported     | 174 call          | 227 call        |
| diveq      | unsupported     | 174 call          | 227 call        |
| mod        | unsupported     | 174 call          | 227 call        |
| modeq      | unsupported     | 167 call          | 220 call        |
| xor_       | 9 P             | 9 P               | 9 P             |
| xoreq      | 9 P             | 9 P               | 9 P             |
| and_       | 9 P             | 9 P               | 9 P             |
| andeq      | 9 P             | 9 P               | 9 P             |
| or_        | 9 P             | 9 P               | 9 P             |
| oreq       | 9 P             | 9 P               | 9 P             |
| shl        | 64              | 28 P              | 52              |
| shleq      | 64              | 28 P              | 52              |
| shr        | 61              | 28 P              | 53              |
| shreq      | 61              | 28 P              | 53              |
| clz        | unsupported     | 17                | 27              |
| lt         | 10 P            | 10 P              | 21              |
| leq        | 10 P            | 10 P              | 21              |
| gt         | 10 P            | 10 P              | 21              |
| geq        | 10 P            | 10 P              | 21              |
| eq         | 13 P            | 13 P              | 13 P            |
| neq        | 13 P            | 13 P              | 13 P            |
| not_       | 7               | 9                 | 6 P             |
| bool_      | 7               | 9                 | 6 P             |
| neg        | 11              | 11                | 25              |
| flip       | 5 P             | 8                 | 8               |


512-bit math using `unsigned _ExtInt(512)` and `Wider<Wider<Wider<uint64_t>>>`:

| Test name  | Clang `_ExtInt` | Clang `W<W<W<u64>>>` | GCC `W<W<W<u64>>>` |
| ---------- | --------------- | -------------------- | ------------------ |
| preinc     | 9               | 9                    | 33                 |
| postinc    | 45              | 25                   | 67                 |
| predec     | 9               | 9                    | 37                 |
| postdec    | 45              | 25                   | 68                 |
| plus       | 17              | 17                   | 54                 |
| pluseq     | 17              | 17                   | 38                 |
| minus      | 17              | 17                   | 58                 |
| minuseq    | 17              | 17                   | 43                 |
| mul        | 274             | __udivti3            | 320                |
| muleq      | 275             | __udivti3            | 320                |
| div        | unsupported     | 384 call             | 526 call           |
| diveq      | unsupported     | 384 call             | 526 call           |
| mod        | unsupported     | 384 call             | 526 call           |
| modeq      | unsupported     | 373 call             | 515 call           |
| xor_       | 17              | 17                   | 17                 |
| xoreq      | 17              | 17                   | 17                 |
| and_       | 17              | 17                   | 17                 |
| andeq      | 17              | 17                   | 17                 |
| or_        | 17              | 17                   | 17                 |
| oreq       | 17              | 17                   | 17                 |
| shl        | 347             | 67                   | 129                |
| shleq      | 347             | 67                   | 129                |
| shr        | 362             | 72                   | 142                |
| shreq      | 362             | 72                   | 142                |
| clz        | unsupported     | 39                   | 64                 |
| lt         | 18              | 18                   | 41                 |
| leq        | 18              | 18                   | 41                 |
| gt         | 18              | 18                   | 41                 |
| geq        | 18              | 18                   | 41                 |
| eq         | 25              | 21                   | 25                 |
| neq        | 25              | 21                   | 25                 |
| not_       | 13              | 13                   | 10                 |
| bool_      | 13              | 13                   | 10                 |
| neg        | 23              | 23                   | 49                 |
| flip       | 9               | 14                   | 14                 |
