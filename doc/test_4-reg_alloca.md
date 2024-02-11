# 将语法分析树转换为 ir

## 实验目的

本实验希望学生对寄存器分配  有一定了解

## 实验要求

对 src/reg_alloca/whole_in_mem_alloca.cc 及 src/standrad_ir.cc 文件空缺部分进行更改，完善转换规则。

## 实验原理

1. 全放内存的寄存器分配
2. 基于图的寄存器分配

## 实验步骤

1. 学生观察 include/ir.hh 文件、src/ir.cc、src/standrad_ir.cc 文件,对相关类各成员的含义有所了解。
2. 学生需要补全 src/standrad.cc 及 src/reg_alloca/whole_in_mem_alloca.cc文件中的空缺部分，将对应的 ir 按正确的形式返回。
3. 学生可以通过 script/test.sh 进行测试，测试脚本将生成所有测例的日志，学生自行与各个测例的正确输出进行比对。

## 评测

学生自测无误后，点击评测按钮进行评分。
因学生实现方案的不同可能造成逻辑正确但是输出无法与提供的正确输出完全对应的情况，本实验的成绩仅供学生参考，不影响最后课程实验评分。
