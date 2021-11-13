#!/usr/bin/python3

import random

ops = ['=', '+', '-', '*', '/', '%', "++", "--", "PLUS", "MINUS", '(']
ops_term = ["VAR", "CONST"]
ops_lvalue = ['(', "VAR"]


def cal(root: list) -> int:
    if root[0] == "CONST":
        return root[1]
    elif root[0] in "PLUS(":
        return cal(root[3])
    elif root[0] == "MINUS":
        return -cal(root[3])
    elif root[0] == '+':
        return cal(root[2]) + cal(root[3])
    elif root[0] == '-':
        return cal(root[2]) - cal(root[3])
    elif root[0] == '*':
        return cal(root[2]) * cal(root[3])
    elif root[0] == '/':
        return int(cal(root[2]) / cal(root[3]))
    elif root[0] == '%':
        return cal(root[2]) % cal(root[3])
    else:
        raise Exception()


def rand(ops_cur: list, vars: list, vars_lvalue: list, d=1, lvalue=False) -> list:
    root = [random.choice(ops_cur), 0, [], []]
    if not vars_lvalue:
        while root[0] in "=++-- VAR":
            root[0] = random.choice(ops_cur)
    if not vars:
        while root[0] == "VAR":
            root[0] = random.choice(ops_cur)
    if root[0] == '=':
        root = ['(', 0, [], ['=', 0, [], []]]
        root[3][2] = rand(ops_lvalue, vars, vars_lvalue, d + 1, True)
        root[3][3] = rand(ops * (69 // d ** 3) + ops_term * d, vars, vars_lvalue, d + 1)
    elif root[0] in "+-*/%":
        root[2] = rand(ops * (69 // d ** 3) + ops_term * d, vars, vars_lvalue, d + 1)
        root[3] = rand(ops * (69 // d ** 3) + ops_term * d, vars, vars_lvalue, d + 1)
        if root[0] in "/%":
            try:
                while not cal(root[3]):
                    root[3] = rand(ops * (69 // d ** 3) + ops_term * d, vars, vars_lvalue, d + 1)
            except:
                pass
    elif root[0] in "++--":
        root = ['(', 0, [], [root[0], 0, [], []]]
        i = random.randint(2, 3)
        root[3][i] = rand(ops_lvalue, vars, vars_lvalue, d + 1, True)
    elif root[0] in "PLUS MINUS":
        root = ['(', 0, [], [root[0], 0, [], []]]
        root[3][3] = ['(', 0, [], rand(ops * (69 // d ** 3) + ops_term * d, vars, vars_lvalue, d + 1)]
    elif root[0] == '(':
        root[3] = rand(ops_cur, vars, vars_lvalue, d + 1, lvalue)
    elif root[0] == 'VAR':
        root[1] = random.choice(vars_lvalue if lvalue else vars)
        if lvalue:
            vars_lvalue.remove(root[1])
            vars.remove(root[1])
        elif root[1] in vars_lvalue:
            vars_lvalue.remove(root[1])
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


for i in range(20):
    print(to_str(rand(ops + ['='] * 87, ['x', 'y', 'z'], ['x', 'y', 'z'])) + ';')
