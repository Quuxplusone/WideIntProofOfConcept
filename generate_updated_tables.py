#!/usr/bin/env python

import argparse
import json
import re
import requests

with open('wider.h') as f:
    original_source = f.readlines()

def process(compiler_name, function_name, type_name, bypass):
    global original_source
    source = original_source[:]
    for i, line in enumerate(source):
        if re.match(r'    //static .* %s[(]' % function_name, line):
            source[i] = re.sub(r'//', '', line)
        elif line == '//template struct Tests<Uint128>;\n':
            source[i] = 'template struct Tests<%s>;\n' % type_name

    r = requests.post(
        'https://godbolt.org/api/compiler/%s/compile' % compiler_name,
        json={
            "bypassCache": bypass,
            "source": ''.join(source),
            "options": {
                "userArguments": "-std=c++14 -O3",
                "compilerOptions": {},
                "filters": {
                    "binary": False,
                    "execute": False,
                    "labels": True,
                    "directives": True,
                    "commentOnly": True,
                    "trim": False,
                    "intel": False,
                    "demangle": False,
                },
                "tools": []
            }
        },
        headers={
            'Accept': 'application/json',
        },
    )

    linecount = 0
    has_call = False
    has_udivti = False
    has_umodti = False
    for line in r.json()['asm']:
        text = line['text']
        if (not text) or (text[0] in '_.'):
            pass
        elif 'call' in text:
            print (text, '## CALL!')
            linecount += 1
            has_call = True
            if '__udivti3' in text:
                has_udivti = True
            elif '__umodti3' in text:
                has_umodti = True
        else:
            print (text)
            linecount += 1

    if has_umodti:
        linecount += 30000
    elif has_udivti:
        linecount += 20000
    elif has_call:
        linecount += 10000

    return linecount

def indicate_perfect_codegen(r, c, lc):
    perfect_dict = {
        (128, 'preinc'): 3,      (128, 'postinc'): 5,
        (128, 'predec'): 3,      (128, 'postdec'): 5,
        (128, 'plus'): 5,        (128, 'pluseq'): 5,
        (128, 'minus'): 5,       (128, 'minuseq'): 5,
        (128, 'mul'): 11,        (128, 'muleq'): 11,
        (128, 'xor_'): 5,        (128, 'xoreq'): 5,
        (128, 'and_'): 5,        (128, 'andeq'): 5,
        (128, 'or_'): 5,         (128, 'oreq'): 5,
        (128, 'shl'): 12,        (128, 'shleq'): 12,
        (128, 'shr'): 12,        (128, 'shreq'): 12,
        (128, 'clz'): None,
        (128, 'lt'): 6,          (128, 'leq'): 6,
        (128, 'gt'): 6,          (128, 'geq'): 6,
        (128, 'eq'): 6,          (128, 'neq'): 6,
        (128, 'not_'): 4,        (128, 'bool_'): 4,
        (128, 'neg'): 4,         (128, 'flip'): 3,

        (256, 'preinc'): 5,      (256, 'postinc'): 9,
        (256, 'predec'): 5,      (256, 'postdec'): 9,
        (256, 'plus'): 9,        (256, 'pluseq'): 9,
        (256, 'minus'): 9,       (256, 'minuseq'): 9,
        (256, 'mul'): None,      (256, 'muleq'): None,
        (256, 'xor_'): 9,        (256, 'xoreq'): 9,
        (256, 'and_'): 9,        (256, 'andeq'): 9,
        (256, 'or_'): 9,         (256, 'oreq'): 9,
        (256, 'shl'): 26,        (256, 'shleq'): 26,
        (256, 'shr'): 26,        (256, 'shreq'): 26,
        (256, 'clz'): None,
        (256, 'lt'): 10,         (256, 'leq'): 10,
        (256, 'gt'): 10,         (256, 'geq'): 10,
        (256, 'eq'): 13,         (256, 'neq'): 13,
        (256, 'not_'): 6,        (256, 'bool_'): 6,
        (256, 'neg'): None,      (256, 'flip'): 5,
    }
    perfect_lc = perfect_dict.get((c.bitwidth, r.funcname))
    if lc < (perfect_lc or 1):
        print('Uh-oh! %d is less than the perfect %d for %d/%s' % (lc, (perfect_lc or 1), c.bitwidth, r.funcname))
        assert False
    elif lc == perfect_lc:
        return '%d P' % lc
    elif 10000 <= lc < 20000:
        return '%d call' % (lc - 10000)
    elif 20000 <= lc < 30000:
        return '__udivti3'
    elif 30000 <= lc < 40000:
        return '__umodti3'
    return '%d' % lc

def find_result(r, c):
    for tn2, fn2, cn2, lc in results:
        if (tn2, fn2, cn2) == (c.typename, r.funcname, c.compiler):
            return indicate_perfect_codegen(r, c, lc)
    return None


class Row:
    def __init__(self, funcname):
        self.funcname = funcname

class Column:
    def __init__(self, title, compiler, typename):
        self.title = title
        self.compiler = compiler
        self.typename = typename
        self.width = len(self.title)

class Table:
    def __init__(self, bitwidth, caption, columns):
        self.caption = caption
        self.columns = columns
        for c in self.columns:
            c.bitwidth = bitwidth

    def precompute(self, options, rows):
        result = []
        for r in rows:
            for c in self.columns:
                cn = c.compiler
                fn = r.funcname
                tn = c.typename
                linecount = process(cn, fn, tn, options.bypass)
                print('%s/%s/%s: %s' % (tn, fn, cn, linecount))
                result.append((tn, fn, cn, linecount))
        return result

    def produce(self, rows):
        result = self.caption + '\n\n'
        result += '| Test name  | ' + ' | '.join([c.title for c in self.columns]) + ' |\n'
        result += '| ---------- | ' + ' | '.join([('-' * c.width) for c in self.columns]) + ' |\n'
        for r in rows:
            result += '| ' + r.funcname.ljust(10) + ' | '
            result += ' | '.join([
                find_result(r, c).ljust(c.width)
                for c in self.columns
            ])
            result += ' |\n'
        return result

all_tables = [
    Table(
        128, '128-bit math using `__uint128_t`, `unsigned _ExtInt(128)`, and `Wider<uint64_t>`:', [
            Column('Clang `uint128`', 'clang_trunk', '__uint128_t'),
            Column('Clang `_ExtInt`', 'clang_trunk', 'unsigned _ExtInt(128)'),
            Column('Clang `W<u64>`', 'clang_trunk', 'Wider<uint64_t>'),
            Column('GCC `uint128`', 'gsnapshot', '__uint128_t'),
            Column('GCC `W<u64>`', 'gsnapshot', 'Wider<uint64_t>'),
        ]
    ),
    Table(
        256, '256-bit math using `unsigned _ExtInt(256)` and `Wider<Wider<uint64_t>>`:', [
            Column('Clang `_ExtInt`', 'clang_trunk', 'unsigned _ExtInt(256)'),
            Column('Clang `W<W<u64>>`', 'clang_trunk', 'Wider<Wider<uint64_t>>'),
            Column('GCC `W<W<u64>>`', 'gsnapshot', 'Wider<Wider<uint64_t>>'),
        ]
    ),
    Table(
        512, '512-bit math using `unsigned _ExtInt(512)` and `Wider<Wider<Wider<uint64_t>>>`:', [
            Column('Clang `_ExtInt`', 'clang_trunk', 'unsigned _ExtInt(512)'),
            Column('Clang `W<W<W<u64>>>`', 'clang_trunk', 'Wider<Wider<Wider<uint64_t>>>'),
            Column('GCC `W<W<W<u64>>>`', 'gsnapshot', 'Wider<Wider<Wider<uint64_t>>>'),
        ]
    ),
]

all_rows = [Row(x) for x in [
    'preinc', 'postinc', 'predec', 'postdec',
    'plus', 'pluseq', 'minus', 'minuseq',
    'mul', 'muleq',
    'div', 'diveq', 'mod', 'modeq',
    'xor_', 'xoreq', 'and_', 'andeq', 'or_', 'oreq',
    'shl', 'shleq', 'shr', 'shreq',
    'clz',
    'lt', 'leq', 'gt', 'geq', 'eq', 'neq',
    'not_', 'bool_', 'neg', 'flip',
]]


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("--bypass", action="store_true", help="bypass Compiler Explorer's cache")
    options = parser.parse_args()

    results = []
    for table in all_tables:
        results += table.precompute(options, all_rows)

    for table in all_tables:
        print(table.produce(all_rows), '\n')
