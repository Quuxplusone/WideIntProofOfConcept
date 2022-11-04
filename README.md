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

128-bit math using `__uint128_t`, `unsigned _BitInt(128)`, and `Wider<uint64_t>`:

| Test name  | Clang `uint128` | Clang `_BitInt` | Clang `W<u64>` | GCC `uint128` | GCC `W<u64>` |
| ---------- | --------------- | --------------- | -------------- | ------------- | ------------ |
| preinc     | 3 P             | 3 P             | 3 P            | 3 P           | 6            |
| postinc    | 11              | 11              | 11             | 11            | 12           |
| predec     | 3 P             | 3 P             | 3 P            | 3 P           | 6            |
| postdec    | 11              | 11              | 11             | 11            | 13           |
| plus       | 5 P             | 5 P             | 5 P            | 5 P           | 11           |
| pluseq     | 5 P             | 5 P             | 5 P            | 5 P           | 6            |
| minus      | 5 P             | 5 P             | 5 P            | 5 P           | 14           |
| minuseq    | 5 P             | 5 P             | 5 P            | 5 P           | 9            |
| mul        | 11 P            | 11 P            | 11 P           | 11 P          | 11 P         |
| muleq      | 11 P            | 11 P            | 11 P           | 11 P          | 11 P         |
| div        | __udivti3       | __udivti3       | 47             | __udivti3     | 57           |
| diveq      | __udivti3       | __udivti3       | 47             | __udivti3     | 57           |
| mod        | __umodti3       | __umodti3       | 38             | __umodti3     | 53           |
| modeq      | __umodti3       | __umodti3       | 38             | __umodti3     | 53           |
| xor_       | 5 P             | 5 P             | 5 P            | 5 P           | 5 P          |
| xoreq      | 5 P             | 5 P             | 5 P            | 5 P           | 5 P          |
| and_       | 5 P             | 5 P             | 5 P            | 5 P           | 5 P          |
| andeq      | 5 P             | 5 P             | 5 P            | 5 P           | 5 P          |
| or_        | 5 P             | 5 P             | 5 P            | 5 P           | 5 P          |
| oreq       | 5 P             | 5 P             | 5 P            | 5 P           | 5 P          |
| shl        | 13              | 13              | 12 P           | 12 P          | 24           |
| shleq      | 13              | 13              | 12 P           | 12 P          | 24           |
| shr        | 13              | 13              | 12 P           | 12 P          | 23           |
| shreq      | 13              | 13              | 12 P           | 12 P          | 23           |
| clz        | 7               | 1               | 7              | 13            | 11           |
| lt         | 6 P             | 6 P             | 6 P            | 6 P           | 9            |
| leq        | 6 P             | 6 P             | 6 P            | 7             | 9            |
| gt         | 6 P             | 6 P             | 6 P            | 7             | 9            |
| geq        | 6 P             | 6 P             | 6 P            | 6 P           | 9            |
| eq         | 6 P             | 7               | 7              | 7             | 7            |
| neq        | 6 P             | 7               | 7              | 7             | 7            |
| not_       | 4 P             | 4 P             | 4 P            | 4 P           | 4 P          |
| bool_      | 4 P             | 4 P             | 4 P            | 4 P           | 4 P          |
| neg        | 5               | 5               | 5              | 4 P           | 13           |
| flip       | 3 P             | 3 P             | 5              | 3 P           | 5            |
 

256-bit math using `unsigned _ExtInt(256)` and `Wider<Wider<uint64_t>>`:

| Test name  | Clang 13 `_ExtInt` | Clang `W<W<u64>>` | GCC `W<W<u64>>` |
| ---------- | ------------------ | ----------------- | --------------- |
| preinc     | 5 P                | 5 P               | 11              |
| postinc    | 23                 | 13                | 18              |
| predec     | 5 P                | 5 P               | 15              |
| postdec    | 23                 | 13                | 20              |
| plus       | 9 P                | 9 P               | 20              |
| pluseq     | 9 P                | 9 P               | 13              |
| minus      | 9 P                | 9 P               | 24              |
| minuseq    | 9 P                | 9 P               | 17              |
| mul        | 60                 | 63                | 64              |
| muleq      | 62                 | 63                | 64              |
| div        | 1                  | 173 call          | 216 call        |
| diveq      | 1                  | 173 call          | 216 call        |
| mod        | 1                  | 173 call          | 198             |
| modeq      | 1                  | 166 call          | 198             |
| xor_       | 9 P                | 9 P               | 9 P             |
| xoreq      | 9 P                | 9 P               | 9 P             |
| and_       | 9 P                | 9 P               | 9 P             |
| andeq      | 9 P                | 9 P               | 9 P             |
| or_        | 9 P                | 9 P               | 9 P             |
| oreq       | 9 P                | 9 P               | 9 P             |
| shl        | 64                 | 30                | 60              |
| shleq      | 64                 | 30                | 60              |
| shr        | 61                 | 33                | 64              |
| shreq      | 61                 | 33                | 64              |
| clz        | 1                  | 17                | 29              |
| lt         | 10 P               | 10 P              | 15              |
| leq        | 10 P               | 10 P              | 15              |
| gt         | 10 P               | 10 P              | 15              |
| geq        | 10 P               | 10 P              | 15              |
| eq         | 13 P               | 13 P              | 13 P            |
| neq        | 13 P               | 13 P              | 13 P            |
| not_       | 7                  | 9                 | 6 P             |
| bool_      | 7                  | 9                 | 6 P             |
| neg        | 11                 | 11                | 21              |
| flip       | 5 P                | 8                 | 8               |
 

512-bit math using `unsigned _ExtInt(512)` and `Wider<Wider<Wider<uint64_t>>>`:

| Test name  | Clang 13 `_ExtInt` | Clang `W<W<W<u64>>>` | GCC `W<W<W<u64>>>` |
| ---------- | ------------------ | -------------------- | ------------------ |
| preinc     | 9                  | 9                    | 23                 |
| postinc    | 45                 | 25                   | 35                 |
| predec     | 9                  | 9                    | 27                 |
| postdec    | 45                 | 25                   | 38                 |
| plus       | 17                 | 17                   | 42                 |
| pluseq     | 17                 | 17                   | 25                 |
| minus      | 17                 | 17                   | 46                 |
| minuseq    | 17                 | 17                   | 31                 |
| mul        | 274                | 300 call             | 258                |
| muleq      | 275                | 300 call             | 258                |
| div        | 1                  | 467 call             | 512 call           |
| diveq      | 1                  | 467 call             | 512 call           |
| mod        | 1                  | 467 call             | 473                |
| modeq      | 1                  | 456 call             | 479 call           |
| xor_       | 17                 | 17                   | 17                 |
| xoreq      | 17                 | 17                   | 17                 |
| and_       | 17                 | 17                   | 17                 |
| andeq      | 17                 | 17                   | 17                 |
| or_        | 17                 | 17                   | 17                 |
| oreq       | 17                 | 17                   | 17                 |
| shl        | 347                | 133 call             | 137                |
| shleq      | 347                | 133 call             | 137                |
| shr        | 362                | 79                   | 168                |
| shreq      | 362                | 79                   | 168                |
| clz        | 1                  | 39                   | 71                 |
| lt         | 18                 | 18                   | 28                 |
| leq        | 18                 | 18                   | 29                 |
| gt         | 18                 | 18                   | 29                 |
| geq        | 18                 | 18                   | 28                 |
| eq         | 25                 | 21                   | 26                 |
| neq        | 25                 | 21                   | 26                 |
| not_       | 13                 | 13                   | 10                 |
| bool_      | 13                 | 13                   | 10                 |
| neg        | 23                 | 23                   | 37                 |
| flip       | 9                  | 14                   | 14                 |
