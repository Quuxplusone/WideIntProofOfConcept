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

def make_testcase(f, BITS):
    MAX = 2**BITS
    a = random.randint(0, MAX - 1)
    b = random.randint(0, MAX - 1)
    shiftcount = random.randint(0, BITS - 1)
    print >>f, '{'
    print >>f, '    Uint%d a = %s_u%d;' % (BITS, hexit(a), BITS)
    print >>f, '    Uint%d b = %s_u%d;' % (BITS, hexit(b), BITS)
    print >>f, '    int shiftcount = %d;' % shiftcount
    print >>f, '    if (a != a) std::cout << "OOPS! " << a << " != " << a << "\\n";'
    print >>f, '    if (b != b) std::cout << "OOPS! " << b << " != " << b << "\\n";'
    print >>f, '    if (a+b != %s_u%d)' % (hexit((a + b) % MAX), BITS)
    print >>f, '        std::cout << "OOPS! " << a << " + " << b << " = " << (a+b) << " not %s\\n";' % (hexit((a + b) % MAX),)
    print >>f, '    if (a-b != %s_u%d)' % (hexit((MAX + a - b) % MAX), BITS)
    print >>f, '        std::cout << "OOPS! " << a << " - " << b << " = " << (a-b) << " not %s\\n";' % (hexit((MAX + a - b) % MAX),)
    print >>f, '    if (a*b != %s_u%d)' % (hexit((a * b) % MAX), BITS)
    print >>f, '        std::cout << "OOPS! " << a << " * " << b << " = " << (a*b) << " not %s\\n";' % (hexit((a * b) % MAX),)
    print >>f, '    if ((a << shiftcount) != %s_u%d)' % (hexit((a << shiftcount) % MAX), BITS)
    print >>f, '        std::cout << "OOPS! " << a << " << " << shiftcount << " = " << (a << shiftcount) << " not %s\\n";' % (hexit((a << shiftcount) % MAX),)
    print >>f, '    if ((a >> shiftcount) != %s_u%d)' % (hexit(a >> shiftcount), BITS)
    print >>f, '        std::cout << "OOPS! " << a << " >> " << shiftcount << " = " << (a >> shiftcount) << " not %s\\n";' % (hexit(a >> shiftcount),)
    print >>f, '}'

with open('fuzzy.cc', 'w') as f:
    print >>f, '#include "wider.h"'
    print >>f, '#include "wider_io.h"'
    print >>f, 'int main() {'
    print >>f, '    using Uint128 = Wider<uint64_t>;'
    print >>f, '    using Uint256 = Wider<Uint128>;'
    print >>f, '    using Uint512 = Wider<Uint256>;'
    for i in xrange(100):
        make_testcase(f, 128)
        make_testcase(f, 256)
        make_testcase(f, 512)
    print >>f, '}'
