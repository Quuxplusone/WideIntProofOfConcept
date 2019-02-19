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
        elif re.match(r'//template struct Tests<%s>;' % type_name, line):
            source[i] = re.sub(r'//', '', line)

    r = requests.post(
        'https://godbolt.org/api/compiler/%s/compile' % compiler_name,
        json={
            "bypassCache": bypass,
            "source": '\n'.join(source),
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
    for line in r.json()['asm']:
        text = line['text']
        if (not text) or (text[0] in '_.'):
            pass
        elif 'call' in text:
            print text, '## CALL!'
            linecount += 1
            if '__udivti3' in text:
                linecount += 20000
            elif '__umodti3' in text:
                linecount += 30000
            else:
                linecount += 10000
        else:
            print text
            linecount += 1

    return linecount

func_names = [
    'preinc', 'postinc', 'predec', 'postdec',
    'plus', 'pluseq', 'minus', 'minuseq',
    'mul', 'muleq',
    'div', 'diveq', 'mod', 'modeq',
    'xor_', 'xoreq', 'and_', 'andeq', 'or_', 'oreq',
    'shl', 'shleq', 'shr', 'shreq',
    'clz',
    'lt', 'leq', 'gt', 'geq', 'eq', 'neq',
    'not_', 'bool_', 'neg', 'flip',
]
type_names = [
    '__uint128_t', 'Uint128', 'Uint256',
]
compiler_names = [
    'clang_trunk', 'gsnapshot', 'g6', 'clang500',
]

def indicate_perfect_codegen(tn, fn, lc):
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
        (256, 'shl'): 28,        (256, 'shleq'): 28,
        (256, 'shr'): 28,        (256, 'shreq'): 28,
        (256, 'clz'): None,
        (256, 'lt'): 10,         (256, 'leq'): 10,
        (256, 'gt'): 10,         (256, 'geq'): 10,
        (256, 'eq'): 13,         (256, 'neq'): 13,
        (256, 'not_'): 6,        (256, 'bool_'): 6,
        (256, 'neg'): None,      (256, 'flip'): None,
    }
    bitwidth = 128 if ('128' in tn) else 256 if ('256' in tn) else None
    perfect_lc = perfect_dict.get((bitwidth, fn), 1)
    assert lc >= perfect_lc
    if lc == perfect_lc:
        return '%d P' % lc
    elif 10000 <= lc < 20000:
        return '%d call' % (lc - 10000)
    elif 20000 <= lc < 30000:
        return '__udivti3'
    elif 30000 <= lc < 40000:
        return '__umodti3'
    return '%d' % lc

def find_result(tn, fn, cn):
    for tn2, fn2, cn2, lc in results:
        if (tn2, fn2, cn2) == (tn, fn, cn):
            return indicate_perfect_codegen(tn, fn, lc)
    return None


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("--bypass", action="store_true", help="bypass Compiler Explorer's cache")
    options = parser.parse_args()

    results = []
    for fn in func_names:
        for tn in type_names:
            for cn in compiler_names:
                linecount = process(cn, fn, tn, options.bypass)
                print '%s/%s/%s: %s' % (tn, fn, cn, linecount)
                results.append((tn, fn, cn, linecount))

    for tn in type_names:
        print {
            '__uint128_t': 'For the builtin `__uint128_t`:\n',
            'Uint128': 'For my `Uint128` built from a pair of `uint64_t`:\n',
            'Uint256': 'For my `Uint256` built from a pair of `Uint128`:\n',
        }[tn]
        print '| Test name              |  Clang trunk  | Clang 5.0.0 | GCC trunk | GCC 6.1'
        print '| ---------------------- | ------------- | ----------- | --------- | -------'
        for fn in func_names:
            print '| %-22s | %-13s | %-11s | %-9s | %s' % (
                fn,
                find_result(tn, fn, 'clang_trunk'),
                find_result(tn, fn, 'clang500'),
                find_result(tn, fn, 'gsnapshot'),
                find_result(tn, fn, 'g6'),
            )
        print '\n'
