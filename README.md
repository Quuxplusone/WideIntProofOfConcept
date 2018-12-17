Wide Integer Math (Proof of Concept)
------------------------------------

All numbers are produced by Godbolt Compiler Explorer, using `-O3 -std=c++14`.

Each number indicates a naïve instruction count
(not counting labels, but counting the `ret`) for the given member function
of `wider_tests::Tests<T>`.
"P" indicates perfect codegen. "sse" indicates that I'm surprised at
the use of SSE instructions.

For the builtin `__uint128_t`:

| Test name              |  Clang trunk  | Clang 5.0.0 | GCC trunk | GCC 6.1
| ---------------------- | ------------- | ----------- | --------- | -------
| plus                   | 5 P           | 6           | 5 P       | 5 P
| pluseq                 | 5 P           | 7           | 5 P       | 5 P
| minus                  | 5 P           | 6           | 5 P       | 5 P
| minuseq                | 5 P           | 6           | 5 P       | 5 P
| mul                    | 11 P          | 11 P        | 12        | 15
| muleq                  | 11 P          | 11 P        | 12        | 15
| xor_                   | 5 P           | 5 P         | 5 P       | 5 P
| xoreq                  | 5 P           | 5 P         | 5 P       | 5 P
| or_                    | 5 P           | 5 P         | 5 P       | 5 P
| oreq                   | 5 P           | 5 P         | 5 P       | 5 P
| and_                   | 5 P           | 5 P         | 5 P       | 5 P
| andeq                  | 5 P           | 5 P         | 5 P       | 5 P
| shl                    | 13            | 13          | 12 P      | 12 P
| shleq                  | 13            | 13          | 12 P      | 12 P
| shr                    | 13            | 13          | 12 P      | 12 P
| shreq                  | 13            | 13          | 12 P      | 12 P
| lt                     | 6 P           | 6 P         | 6 P       | 11
| leq                    | 6 P           | 6 P         | 7         | 11
| gt                     | 6 P           | 6 P         | 7         | 11
| geq                    | 6 P           | 6 P         | 6 P       | 11
| eq                     | 6 sse         | 6 sse       | 7 P       | 7 P
| neq                    | 6 sse         | 6 sse       | 7 P       | 7 P
| not_                   | 4 P           | 4 P         | 4 P       | 4 P
| bool_                  | 4 P           | 4 P         | 4 P       | 4 P

For my `Uint128` built from a pair of `uint64_t`:

| Test name              |  Clang trunk  | Clang 5.0.0 | GCC trunk | GCC 6.1
| ---------------------- | ------------- | ----------- | --------- | -------
| plus                   | 5 P           | 10          | 13        | 15
| pluseq                 | 5 P           | 6           | 6         | 12
| minus                  | 6             | 10          | 16        | 18
| minuseq                | 6             | 6           | 11        | 12
| mul                    | 11 P          | 11 P        | 11 P      | 11 P
| muleq                  | 11 P          | 11 P        | 11 P      | 11 P
| xor_                   | 5 sse         | 5 sse       | 5 sse     | 5 sse
| xoreq                  | 5 sse         | 5 sse       | 5 sse     | 5 sse
| or_                    | 5 sse         | 5 sse       | 5 sse     | 5 sse
| oreq                   | 5 sse         | 5 sse       | 5 sse     | 5 sse
| and_                   | 5 sse         | 5 sse       | 5 sse     | 5 sse
| andeq                  | 5 sse         | 5 sse       | 5 sse     | 5 sse
| shl                    | 12 P          | 14          | 25        | 19
| shleq                  | 12 P          | 14          | 25        | 19
| shr                    | 12 P          | 14          | 22        | 17
| shreq                  | 12 P          | 14          | 22        | 17
| lt                     | 8             | 9           | 11        | 12
| leq                    | 8             | 9           | 9         | 12
| gt                     | 8             | 9           | 9         | 12
| geq                    | 8             | 9           | 9         | 12
| eq                     | 7 P           | 7 P         | 7 P       | 7 P
| neq                    | 7 P           | 7 P         | 7 P       | 7 P
| not_                   | 4 P           | 4 P         | 4 P       | 4 P
| bool_                  | 4 P           | 4 P         | 4 P       | 4 P

For my `Uint256` built from a pair of `__uint128_t`:

| Test name              |  Clang trunk  | Clang 5.0.0 | GCC trunk | GCC 6.1
| ---------------------- | ------------- | ----------- | --------- | -------
| plus                   | 9 P           | 24          | 26        | 30
| pluseq                 | 9 P           | 12          | 18        | 21
| minus                  | 10            | 24          | 30        | 26
| minuseq                | 10            | 12          | 23        | 22
| mul                    | 60            | 74          | 62        | 84
| muleq                  | 60            | 74          | 62        | 84
| xor_                   | 9 sse         | 9 sse       | 9 P       | 9 P
| xoreq                  | 9 sse         | 9 sse       | 9 P       | 9 P
| or_                    | 9 sse         | 9 sse       | 9 P       | 9 P
| oreq                   | 9 sse         | 9 sse       | 9 P       | 9 P
| and_                   | 9 sse         | 9 sse       | 9 P       | 9 P
| andeq                  | 9 sse         | 9 sse       | 9 P       | 9 P
| shl                    | 28 P          | 39          | 68        | 61
| shleq                  | 28 P          | 39          | 68        | 61
| shr                    | 28 P          | 40          | 68        | 61
| shreq                  | 28 P          | 40          | 68        | 61
| lt                     | 10 P          | 18          | 21        | 19
| leq                    | 10 P          | 18          | 21        | 19
| gt                     | 10 P          | 18          | 21        | 19
| geq                    | 10 P          | 18          | 21        | 19
| eq                     | 13 sse        | 17 sse      | 13 P      | 13 P
| neq                    | 13 sse        | 17 sse      | 13 P      | 13 P
| not_                   | 9 sse         | 6 P         | 6 P       | 6 P
| bool_                  | 9 sse         | 6 P         | 6 P       | 6 P
