#!/bin/sh

make

rm result.txt
index=0
for case in testreturn testlocalvar testloop testnestedloop
do
    cp ../src/${case}.c .
    ./gompiler ${case}.c ${case}.s
    gcc ${case}.s -o ${case}
    ./${case}
    result="$?"
    echo ${result}
    echo "> @testcase ${case} => ${result}" >> result.txt
    index=`expr $index + 1`
done

make clean
