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
| neg                    | 5             | 7           | 4 P       | 4 P
| flip                   | 3 P           | 3 P         | 3 P       | 3 P


For my `Uint128` built from a pair of `uint64_t`:

| Test name              |  Clang trunk  | Clang 5.0.0 | GCC trunk | GCC 6.1
| ---------------------- | ------------- | ----------- | --------- | -------
| preinc                 | 3 P           | 5           | 6         | 5
| postinc                | 11            | 13          | 13        | 13
| predec                 | 3 P           | 8           | 6         | 10
| postdec                | 11            | 13          | 13        | 13
| plus                   | 5 P           | 10          | 13        | 15
| pluseq                 | 5 P           | 6           | 6         | 12
| minus                  | 5 P           | 10          | 16        | 18
| minuseq                | 5 P           | 6           | 11        | 12
| mul                    | 11 P          | 11 P        | 11 P      | 11 P
| muleq                  | 11 P          | 11 P        | 11 P      | 11 P
| div                    | 36            | 57          | 78        | 82
| diveq                  | 36            | 57          | 78        | 82
| mod                    | 32            | 53          | 72        | 76
| modeq                  | 32            | 52          | 72        | 79
| xor_                   | 5 P           | 5 P         | 5 P       | 5 P
| xoreq                  | 5 P           | 5 P         | 5 P       | 5 P
| and_                   | 5 P           | 5 P         | 5 P       | 5 P
| andeq                  | 5 P           | 5 P         | 5 P       | 5 P
| or_                    | 5 P           | 5 P         | 5 P       | 5 P
| oreq                   | 5 P           | 5 P         | 5 P       | 5 P
| shl                    | 12 P          | 14          | 26        | 19
| shleq                  | 12 P          | 14          | 26        | 19
| shr                    | 12 P          | 14          | 23        | 19
| shreq                  | 12 P          | 14          | 23        | 19
| clz                    | 7             | 8           | 11        | 9
| lt                     | 6 P           | 9           | 11        | 12
| leq                    | 6 P           | 9           | 11        | 12
| gt                     | 6 P           | 9           | 11        | 12
| geq                    | 6 P           | 9           | 11        | 12
| eq                     | 7             | 7           | 7         | 7
| neq                    | 7             | 7           | 7         | 7
| not_                   | 4 P           | 4 P         | 4 P       | 4 P
| bool_                  | 4 P           | 4 P         | 4 P       | 4 P
| neg                    | 5             | 13          | 13        | 14
| flip                   | 5             | 5           | 5         | 5


For my `Uint256` built from a pair of `Uint128`:

| Test name              |  Clang trunk  | Clang 5.0.0 | GCC trunk | GCC 6.1
| ---------------------- | ------------- | ----------- | --------- | -------
| preinc                 | 5 P           | 11          | 13        | 15
| postinc                | 13            | 21          | 29        | 27
| predec                 | 5 P           | 14          | 17        | 20
| postdec                | 13            | 22          | 30        | 27
| plus                   | 9 P           | 24          | 26        | 30
| pluseq                 | 9 P           | 12          | 18        | 21
| minus                  | 9 P           | 24          | 30        | 26
| minuseq                | 9 P           | 12          | 23        | 22
| mul                    | 60            | 74          | 73        | 84
| muleq                  | 60            | 74          | 73        | 84
| div                    | 175 call      | 166 call    | 230 call  | 225
| diveq                  | 175 call      | 166 call    | 230 call  | 225
| mod                    | 175 call      | 166 call    | 230 call  | 209
| modeq                  | 168 call      | 160 call    | 223 call  | 214
| xor_                   | 9 P           | 9 P         | 9 P       | 9 P
| xoreq                  | 9 P           | 9 P         | 9 P       | 9 P
| and_                   | 9 P           | 9 P         | 9 P       | 9 P
| andeq                  | 9 P           | 9 P         | 9 P       | 9 P
| or_                    | 9 P           | 9 P         | 9 P       | 9 P
| oreq                   | 9 P           | 9 P         | 9 P       | 9 P
| shl                    | 28 P          | 39          | 52        | 60
| shleq                  | 28 P          | 39          | 52        | 60
| shr                    | 28 P          | 40          | 52        | 59
| shreq                  | 28 P          | 40          | 52        | 59
| clz                    | 17            | 19          | 26        | 39
| lt                     | 10 P          | 18          | 21        | 19
| leq                    | 10 P          | 18          | 21        | 19
| gt                     | 10 P          | 18          | 21        | 19
| geq                    | 10 P          | 18          | 21        | 19
| eq                     | 13 P          | 17          | 13 P      | 13 P
| neq                    | 13 P          | 17          | 13 P      | 13 P
| not_                   | 9             | 6 P         | 6 P       | 6 P
| bool_                  | 9             | 6 P         | 6 P       | 6 P
| neg                    | 11            | 27          | 25        | 26
| flip                   | 8             | 8           | 8         | 8
