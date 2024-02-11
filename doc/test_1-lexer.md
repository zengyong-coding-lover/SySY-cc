# 词法分析和语法分析

## 实验目的

本实验希望学生对编译器的词法和语法分析有一定了解

## 实验要求

对 grammar/SysY.l 和 grammar/SysY.y 文件空缺部分进行更改，完善词法规则和语法规则。

## 实验原理

1. [Flex 的使用](http://westes.github.io/flex/manual)。
2. [Bison 的使用](https://www.gnu.org/software/bison/manual/bison.html)。

## 实验步骤

1. 学生观察 include/ast_tree.hh 文件、src/ast_tree.cc 文件、及输出中的词法分析部分，对相关类各成员的含义有所了解。
2. 学生需要补全 grammar/SysY.l 和 grammar/SysY.y 文件中的词法规则和语义规则，将对应的语法分析树按正确的形式返回。
3. 学生可以通过 script/test.sh 进行测试，测试脚本将生成所有测例的日志，学生自行与各个测例的正确输出进行比对。

## 评测

学生自测无误后，点击评测按钮进行评分。
因学生实现方案的不同可能造成逻辑正确但是输出无法与提供的正确输出完全对应的情况，本实验的成绩仅供学生参考，不影响最后课程实验评分。
