#!/bin/zsh

set -o pipefail
input="";
while read in; do
    input+=$in'\n';
done;
x=$((RANDOM % 201 - 100))
y=$((RANDOM % 201 - 100))
z=$((RANDOM % 201 - 100))
if [[ $# == 0 ]]; then
    1="./optimized";
fi
main=("${(@f)$(echo $input | time $1 2> /dev/null | AssemblyCompiler/ASMC $x $y $z)}")
if [[ $? == 136 ]]; then
    echo "\033[0;33mfloating point exception\033[m";
    exit -1;
fi
cling=("${(@f)$(echo "#include <stdio.h>\nvolatile int x=$x, y=$y, z=$z;${input}printf(\"x, y, z = %d, %d, %d\\\\n\", x, y, z)" | cling 2>&1)}");
if [[ $? == 136 ]]; then
    echo "\033[0;33mfloating point exception\033[m";
    exit -1;
fi
if [[ $cling =~ "warning" ]]; then
    echo "\033[0;33m${cling[6]}\033[m";
    exit -1;
elif [[ $cling =~ "error" ]]; then
    if [[ $main =~ "CE" ]]; then
        echo "\033[1;32mAC\033[m\n\033[0;33mCompile Error\033[m";
    else
        echo "\033[1;31mWA\033[m\n\033[0;33mCompile Error\033[m";
        exit 1;
    fi
else
    if [[ $main[1] == $cling[6] ]]; then
        echo "\033[1;32mAC\033[m\n\033[0;37m${main[2]}\033[m";
    else
        echo "\033[1;31mWA\033[m\n${main[1]}\n${cling[6]}";
        exit 1;
    fi
fi
