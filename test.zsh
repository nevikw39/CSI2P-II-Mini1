#!/bin/zsh

for i in $(seq 1 6); do
    echo $i;
    ./main < testcase/test$i.in 2> /dev/null | AssemblyCompiler/ASMC
    cling=$(echo "#include <stdio.h>\nint x = 2, y = 3, z = 5;\n$(cat testcase/test$i.in)\nprintf(\"x, y, z = %d, %d, %d\\\\n\", x, y, z)" | cling 2>&1);
    lines=("${(@f)cling}");
    echo $lines[6];
    echo;
done;
