Wide Integer Math (Proof of Concept)
------------------------------------

All numbers are produced by Godbolt Compiler Explorer, using `-O3 -std=c++14`.

Each number indicates a naïve instruction count
(not counting labels, but counting the `ret`) for the given member function
of `wider_tests::Tests<T>`.
"P" indicates perfect codegen.
"call" indicates failure to completely inline the code (because it's so big).

"`__udivti3`" and "`__umodti3`" indicate that certain arithmetic operations on `__uint128_t`
are delegated to a library function. The library function is macro-optimized with
many special cases (division by small integers, division by integers with many trailing zeros,
et cetera). In contrast, `Wider`'s `operator/` omits these special cases,
resulting in smaller code but almost certainly slower code in common situations.

For the builtin `__uint128_t`:

| Test name              |  Clang trunk  | Clang 5.0.0 | GCC trunk | GCC 6.1
| ---------------------- | ------------- | ----------- | --------- | -------
| preinc                 | 3 P           | 5           | 3 P       | 3 P
| postinc                | 11            | 11          | 11        | 11
| predec                 | 3 P           | 5           | 3 P       | 3 P
| postdec                | 11            | 11          | 11        | 11
| plus                   | 5 P           | 7           | 5 P       | 5 P
| pluseq                 | 5 P           | 6           | 5 P       | 5 P
| minus                  | 5 P           | 6           | 5 P       | 5 P
| minuseq                | 5 P           | 6           | 5 P       | 5 P
| mul                    | 11 P          | 11 P        | 12        | 15
| muleq                  | 11 P          | 11 P        | 12        | 15
| div                    | __udivti3     | __udivti3   | __udivti3 | __udivti3
| diveq                  | __udivti3     | __udivti3   | __udivti3 | __udivti3
| mod                    | __umodti3     | __umodti3   | __umodti3 | __umodti3
| modeq                  | __umodti3     | __umodti3   | __umodti3 | __umodti3
| xor_                   | 5 P           | 5 P         | 5 P       | 5 P
| xoreq                  | 5 P           | 5 P         | 5 P       | 5 P
| and_                   | 5 P           | 5 P         | 5 P       | 5 P
| andeq                  | 5 P           | 5 P         | 5 P       | 5 P
| or_                    | 5 P           | 5 P         | 5 P       | 5 P
| oreq                   | 5 P           | 5 P         | 5 P       | 5 P
| shl                    | 13            | 13          | 12 P      | 12 P
| shleq                  | 13            | 13          | 12 P      | 12 P
| shr                    | 13            | 13          | 12 P      | 12 P
| shreq                  | 13            | 13          | 12 P      | 12 P
| clz                    | 7             | 8           | 11        | 9
| lt                     | 6 P           | 6 P         | 6 P       | 11
| leq                    | 6 P           | 6 P         | 7         | 11
| gt                     | 6 P           | 6 P         | 7         | 11
| geq                    | 6 P           | 6 P         | 6 P       | 11
| eq                     | 6 P           | 6 P         | 7         | 7
| neq                    | 6 P           | 6 P         | 7         | 7
| not_                   | 4 P           | 4 P         | 4 P       | 4 P
| bool_                  | 4 P           | 4 P         | 4 P       | 4 P
| neg                    | 7             | 7           | 4 P       | 4 P
| flip                   | 3 P           | 3 P         | 3 P       | 3 P


For my `Uint128` built from a pair of `uint64_t`:

| Test name              |  Clang trunk  | Clang 5.0.0 | GCC trunk | GCC 6.1
| ---------------------- | ------------- | ----------- | --------- | -------
| preinc                 | 3 P           | 5           | 5         | 5
| postinc                | 11            | 13          | 15        | 13
| predec                 | 8             | 8           | 8         | 10
| postdec                | 13            | 13          | 15        | 13
| plus                   | 5 P           | 10          | 13        | 15
| pluseq                 | 5 P           | 6           | 6         | 12
| minus                  | 6             | 10          | 16        | 18
| minuseq                | 6             | 6           | 11        | 12
| mul                    | 11 P          | 11 P        | 11 P      | 11 P
| muleq                  | 11 P          | 11 P        | 11 P      | 11 P
| div                    | 39            | 57          | 87        | 82
| diveq                  | 39            | 57          | 87        | 82
| mod                    | 32            | 53          | 82        | 76
| modeq                  | 32            | 52          | 81        | 79
| xor_                   | 5 P           | 5 P         | 5 P       | 5 P
| xoreq                  | 5 P           | 5 P         | 5 P       | 5 P
| and_                   | 5 P           | 5 P         | 5 P       | 5 P
| andeq                  | 5 P           | 5 P         | 5 P       | 5 P
| or_                    | 5 P           | 5 P         | 5 P       | 5 P
| oreq                   | 5 P           | 5 P         | 5 P       | 5 P
| shl                    | 12 P          | 14          | 26        | 19
| shleq                  | 12 P          | 14          | 26        | 19
| shr                    | 12 P          | 14          | 27        | 19
| shreq                  | 12 P          | 14          | 27        | 19
| clz                    | 9             | 9           | 12        | 10
| lt                     | 6 P           | 9           | 11        | 12
| leq                    | 6 P           | 9           | 9         | 12
| gt                     | 6 P           | 9           | 9         | 12
| geq                    | 6 P           | 9           | 9         | 12
| eq                     | 7             | 7           | 7         | 7
| neq                    | 7             | 7           | 7         | 7
| not_                   | 4 P           | 4 P         | 4 P       | 4 P
| bool_                  | 4 P           | 4 P         | 4 P       | 4 P
| neg                    | 10            | 13          | 13        | 14
| flip                   | 5             | 5           | 5         | 5


For my `Uint256` built from a pair of `Uint128`:

| Test name              |  Clang trunk  | Clang 5.0.0 | GCC trunk | GCC 6.1
| ---------------------- | ------------- | ----------- | --------- | -------
| preinc                 | 5 P           | 11          | 13        | 15
| postinc                | 13            | 21          | 29        | 27
| predec                 | 10            | 14          | 17        | 20
| postdec                | 18            | 22          | 30        | 27
| plus                   | 9 P           | 24          | 26        | 30
| pluseq                 | 9 P           | 12          | 18        | 21
| minus                  | 10            | 24          | 30        | 26
| minuseq                | 10            | 12          | 23        | 22
| mul                    | 60            | 74          | 62        | 84
| muleq                  | 60            | 74          | 62        | 84
| div                    | 177 call      | 166 call    | 236       | 225
| diveq                  | 177 call      | 166 call    | 236       | 225
| mod                    | 177 call      | 166 call    | 226       | 209
| modeq                  | 170 call      | 160 call    | 226       | 214
| xor_                   | 9 P           | 9 P         | 9 P       | 9 P
| xoreq                  | 9 P           | 9 P         | 9 P       | 9 P
| and_                   | 9 P           | 9 P         | 9 P       | 9 P
| andeq                  | 9 P           | 9 P         | 9 P       | 9 P
| or_                    | 9 P           | 9 P         | 9 P       | 9 P
| oreq                   | 9 P           | 9 P         | 9 P       | 9 P
| shl                    | 28 P          | 39          | 67        | 60
| shleq                  | 28 P          | 39          | 67        | 60
| shr                    | 28 P          | 40          | 67        | 59
| shreq                  | 28 P          | 40          | 67        | 59
| clz                    | 21            | 21          | 32        | 25
| lt                     | 10 P          | 18          | 21        | 19
| leq                    | 10 P          | 18          | 21        | 19
| gt                     | 10 P          | 18          | 21        | 19
| geq                    | 10 P          | 18          | 21        | 19
| eq                     | 13 P          | 17          | 13 P      | 13 P
| neq                    | 13 P          | 17          | 13 P      | 13 P
| not_                   | 9             | 6 P         | 6 P       | 6 P
| bool_                  | 9             | 6 P         | 6 P       | 6 P
| neg                    | 16            | 27          | 25        | 26
| flip                   | 8             | 8           | 8         | 8
