#!/usr/bin/python3

import random

ops = ['=', '+', '-', '*', '/', '%', "++", "--", "PLUS", "MINUS", '('] + ["VAR", "CONST"] * 69
ops_lvalue = ['(', "VAR"]

def get_var(root: list) -> str:
    if root[0] != '(':
        return root[1]
    return get_var(root[3])

def rand(ops_cur: list, vars: list) -> list:
    root = [random.choice(ops_cur), 0, [], []]
    if not vars:
        while root[0] in "=++-- VAR":
            root[0] = random.choice(ops_cur)
    if root[0] == '=':
        root = ['(', 0, [], ['=', 0, [], []]]
        root[3][2] = rand(ops_lvalue, vars)
        var = get_var(root[3][2])
        vars.remove(var)
        root[3][3] = rand(ops, vars)
    elif root[0] in "+-*/%":
        root[2] = rand(ops, vars)
        root[3] = rand(ops, vars)
    elif root[0] in "++--":
        root = ['(', 0, [], [root[0], 0, [], []]]
        i = random.randint(2, 3)
        root[3][i] = rand(ops_lvalue, vars)
        var = get_var(root[3][i])
        vars.remove(var)
    elif root[0] in "PLUS MINUS":
        root = ['(', 0, [], [root[0], 0, [], []]]
        root[3][3] = ['(', 0, [], rand(ops, vars)]
    elif root[0] == '(':
        root[3] = rand(ops_cur, vars)
    elif root[0] == 'VAR':
        root[1] = random.choice(vars)
    elif root[0] == 'CONST':
        root[1] = random.randrange(100)
    return root

def to_str(root: list) -> str:
    if not root:
        return ""
    if root[0] in "VAR CONST":
        return str(root[1])
    if root[0] == "PLUS":
        return '+' + to_str(root[3])
    if root[0] == "MINUS":
        return '-' + to_str(root[3])
    if root[0] == '(':
        return '(' + to_str(root[3]) + ')'
    return to_str(root[2]) + root[0] + to_str(root[3])

for i in range(200):
    print(to_str(rand(ops + ['='] * 87, ['x', 'y', 'z'])) + ';')
