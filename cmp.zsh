#!/bin/zsh

input="";
while read in; do
    input+=$in'\n';
done;
main=("${(@f)$(echo $input | time ./main 2> /dev/null | AssemblyCompiler/ASMC)}")
cling=("${(@f)$(echo "#include <stdio.h>\nint x=2, y=3, z=5;${input}printf(\"x, y, z = %d, %d, %d\\\\n\", x, y, z)" | cling -w 2>&1)}");
if [[ $cling =~ "error" ]]; then
    echo "\033[0;33mCompile Error\033[m";
    if [[ $main =~ "CE" ]]; then
        echo "\033[1;32mAC\033[m";
        exit;
    else
        echo "\033[1;31mWA\033[m";
        exit 1;
    fi
else
    if [[ $main[1] == $cling[6] ]]; then
        echo "\033[0;37m${main[2]}\033[m";
        echo "\033[1;32mAC\033[m";
        exit;
    else
        echo "\033[1;31mWA\033[m";
        exit 1;
    fi
fi
