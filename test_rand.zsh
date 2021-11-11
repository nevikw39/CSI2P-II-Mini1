#!/bin/zsh

cnt=0;
ub=0;
function onINT()
{
    echo "\n\033[0;37m$cnt($ub)\033[m";
     exit;
}
trap onINT INT
while True; do
    echo -n '.'; 
    input=$(./rand.py);
    echo $input | ./cmp.zsh > /dev/null 2> /dev/null;
    case $? in
    1)
        echo "\n$input";
        exit 1;
        ;;
    255)
        echo "\n\033[0;33mUB\033[m";
        ub=$(($ub+1));
        ;;
    esac
    cnt=$(($cnt+1));
done;
