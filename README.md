Wide Integer Math (Proof of Concept)
------------------------------------

All numbers produced from Godbolt Compiler Explorer, using `-O3 -std=c++11`.

For the builtin `__uint128_t`:

| Test name              |  Clang trunk  | Clang 5.0.0 | GCC trunk | GCC 6.1
| ---------------------- | ------------- | ----------- | --------- | -------
| plus                   | OK            | extra mov   | OK        | OK
| pluseq                 | OK            | extra movs  | OK        | OK
| minus                  | OK            | extra mov   | OK        | OK
| minuseq                | OK            | extra mov   | OK        | OK
| xor_                   | OK            | OK          | OK        | OK
| xoreq                  | OK            | OK          | OK        | OK
| or_                    | OK            | OK          | OK        | OK
| oreq                   | OK            | OK          | OK        | OK
| and_                   | OK            | OK          | OK        | OK
| andeq                  | OK            | OK          | OK        | OK
| lt                     | OK            | OK          | OK        | bad
| leq                    | OK            | OK          | extra mov | bad
| gt                     | OK            | OK          | extra mov | bad
| geq                    | OK            | OK          | OK        | bad
| eq                     | weird SSE     | weird SSE   | OK        | OK
| neq                    | weird SSE     | weird SSE   | OK        | OK
| not_                   | OK            | OK          | OK        | OK
| bool_                  | OK            | OK          | OK        | OK


For my `Uint128` built from a pair of `uint64_t`:

| Test name              |  Clang trunk  | Clang 5.0.0 | GCC trunk | GCC 6.1
| ---------------------- | ------------- | ----------- | --------- | -------
| plus                   | OK            | extra mov   | extra mov | bad
| pluseq                 | OK            | bad         | bad       | bad
| minus                  | extra mov     | extra mov   | bad       | bad
| minuseq                | extra mov     | bad         | bad       | bad
| xor_                   | OK            | OK          | OK        | OK
| xoreq                  | OK            | OK          | OK        | OK
| or_                    | OK            | OK          | OK        | OK
| oreq                   | OK            | OK          | OK        | OK
| and_                   | OK            | OK          | OK        | OK
| andeq                  | OK            | OK          | OK        | OK
| lt                     | bad           | bad         | bad       | bad
| leq                    | bad           | bad         | bad       | bad
| gt                     | bad           | bad         | bad       | bad
| geq                    | bad           | bad         | bad       | bad
| eq                     | OK            | OK          | OK        | OK
| neq                    | OK            | OK          | OK        | OK
| not_                   | OK            | OK          | OK        | OK
| bool_                  | OK            | OK          | OK        | OK

For my `Uint256` built from a pair of `__uint128_t`:

| Test name              |  Clang trunk  | Clang 5.0.0 | GCC trunk | GCC 6.1
| ---------------------- | ------------- | ----------- | --------- | -------
| plus                   | OK            | extra movs  | bad       | bad
| pluseq                 | OK            | bad         | bad       | bad
| minus                  | OK            | extra movs  | bad       | bad
| minuseq                | OK            | bad         | bad       | bad
| xor_                   | OK            | OK          | OK        | OK
| xoreq                  | OK            | OK          | OK        | OK
| or_                    | OK            | OK          | OK        | OK
| oreq                   | OK            | OK          | OK        | OK
| and_                   | OK            | OK          | OK        | OK
| andeq                  | OK            | OK          | OK        | OK
| lt                     | OK            | bad         | bad       | bad
| leq                    | OK            | bad         | bad       | bad
| gt                     | OK            | bad         | bad       | bad
| geq                    | OK            | bad         | bad       | bad
| eq                     | OK            | bad         | OK        | OK
| neq                    | OK            | bad         | OK        | OK
| not_                   | OK            | OK          | OK        | OK
| bool_                  | OK            | OK          | OK        | OK
