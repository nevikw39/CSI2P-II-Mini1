#!/bin/zsh

input="";
while read in; do
    input+=$in'\n';
done;
optimized=("${(@f)$(echo $input | time ./optimized 2> /dev/null | AssemblyCompiler/ASMC)}")
cling=("${(@f)$(echo "#include <stdio.h>\nint x=2, y=3, z=5;${input}printf(\"x, y, z = %d, %d, %d\\\\n\", x, y, z)" | cling 2>&1)}");
if [[ $? == 136 ]]; then
    echo "\033[0;33mfloating point exception\033[m";
    exit -1;
fi
if [[ $cling =~ "warning" ]]; then
    echo "\033[0;33m${cling[6]}\033[m";
    exit -1;
elif [[ $cling =~ "error" ]]; then
    if [[ $optimized =~ "CE" ]]; then
        echo "\033[1;32mAC\033[m\n\033[0;33mCompile Error\033[m";
    else
        echo "\033[1;31mWA\033[m\n\033[0;33mCompile Error\033[m";
        exit 1;
    fi
else
    if [[ $optimized[1] == $cling[6] ]]; then
        echo "\033[1;32mAC\033[m\n\033[0;37m${optimized[2]}\033[m";
    else
        echo "\033[1;31mWA\033[m\n${optimized[1]}\n${cling[6]}";
        exit 1;
    fi
fi
