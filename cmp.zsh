#!/bin/zsh

lines="";
while read line; do
    lines+=$line'\n';
done;
echo $lines | ./main 2> /dev/null | AssemblyCompiler/ASMC;
cling=$(echo "#include <stdio.h>\nint x = 2, y = 3, z = 5;${lines}printf(\"x, y, z = %d, %d, %d\\\\n\", x, y, z)" | cling 2>&1);
if [[ $cling =~ "error" ]]; then
    echo "Compile Error";
else
    lines=("${(@f)cling}");
    echo $lines[6];
fi
