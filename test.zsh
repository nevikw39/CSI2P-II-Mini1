#!/bin/zsh

for i in $(seq 1 6); do
    echo $i;
    cat testcase/test$i.in | ./cmp.zsh || exit 1;
    echo;
done;
