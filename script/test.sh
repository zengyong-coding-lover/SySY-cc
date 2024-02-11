#!/bin/sh

out=out

compiler="./build/SysY_Compiler-release"
assembler="./assembler/assembler-$(arch)"
linker="./linker/linker-$(arch)"

testdir=$1
tests=$(find $testdir -name "*.sy")

make VERSION=release

total=0
pass=0

rm -rf $out
for test in $tests; do
    total=$((total + 1))

    mkdir -p $out/${test%/*}

    name="${test%.sy}"
    src="$test"

    cclog="$out/$name.compiler.log"
    asm="$out/$name.s"
    $compiler $src $asm > $cclog 2>&1
    if [ ! $? -eq 0 ]; then
        echo "X $test, @cc"
        continue
    fi

    aslog="/dev/null"
    rel="$out/$name.o"
    $assembler $asm $rel > $aslog 2>&1
    if [ ! $? -eq 0 ]; then
        echo "X $test, @as"
        continue
    fi

    ldlog="/dev/null"
    exe="$out/$name"
    $linker $rel -o $exe > $ldlog 2>&1
    if [ ! $? -eq 0 ]; then
        echo "X $test, @ld"
        continue
    fi

    is="$name.in"
    os="$out/$name.out"
    qemu-riscv32 $exe < $is > $os
    output=$?
    sed -i -e '$a\' "$os"
    echo $output >> $os

    ans="$name.out"
    diff -bB $ans $os > /dev/null 2>&1
    diff=$?
    if [ $diff -eq 0 ]; then
        echo "* $test"
        pass=$((pass + 1))
    else
        echo "X $test, @qemu"
    fi
done

echo "Pass: $pass/(total: $total)"

