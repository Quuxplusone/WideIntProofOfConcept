#!/usr/bin/env python

import random

BITS = 128
MAX = 2**BITS

def hexit(n):
    result = hex(n)
    assert result[:2] == '0x'
    if result[-1] == 'L':
        result = result[:-1]
    return result

def make_testcase(f):
    a = random.randint(0, MAX - 1)
    b = random.randint(0, MAX - 1)
    shiftcount = random.randint(0, BITS - 1)
    print >>f, '{'
    print >>f, '    Uint128 a = ' + hexit(a) + '_u128;'
    print >>f, '    Uint128 b = ' + hexit(b) + '_u128;'
    print >>f, '    int shiftcount = ' + str(shiftcount) + ';'
    print >>f, '    if (a != a) std::cout << "OOPS! " << a << " != " << a << "\\n";'
    print >>f, '    if (b != b) std::cout << "OOPS! " << b << " != " << b << "\\n";'
    print >>f, '    if (a+b != ' + hexit((a + b) % MAX) + '_u128)'
    print >>f, '        std::cout << "OOPS! " << a << " + " << b << " = " << (a+b) << " not ' + hexit((a + b) % MAX) + '\\n";'
    print >>f, '    if (a-b != ' + hexit((MAX + a - b) % MAX) + '_u128)'
    print >>f, '        std::cout << "OOPS! " << a << " - " << b << " = " << (a-b) << " not ' + hexit((MAX + a - b) % MAX) + '\\n";'
    print >>f, '    if ((a << shiftcount) != ' + hexit((a << shiftcount) % MAX) + '_u128)'
    print >>f, '        std::cout << "OOPS! " << a << " << " << shiftcount << " = " << (a << shiftcount) << " not ' + hexit((a << shiftcount) % MAX) + '\\n";'
    print >>f, '    if ((a >> shiftcount) != ' + hexit(a >> shiftcount) + '_u128)'
    print >>f, '        std::cout << "OOPS! " << a << " >> " << shiftcount << " = " << (a >> shiftcount) << " not ' + hexit(a >> shiftcount) + '\\n";'
    print >>f, '}'

with open('fuzzy.cc', 'w') as f:
    print >>f, '#include "wider.h"'
    print >>f, '#include "wider_io.h"'
    print >>f, 'int main() {'
    print >>f, '    using Uint128 = Wider<uint64_t>;'
    for i in xrange(1000):
        make_testcase(f)
    print >>f, '}'
