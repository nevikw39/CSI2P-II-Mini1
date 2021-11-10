#!/bin/zsh

while True; do
    echo -n '.'; 
    input=$(./rand.py);
    echo $input | ./cmp.zsh > /dev/null 2> /dev/null;
    if [[ $? != 0 ]]; then
        echo $input;
        exit 1;
    fi;
done;
