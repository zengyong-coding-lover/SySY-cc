#ifndef _TODO_
#define _TODO_
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <list>
#include <map>
#include <queue>
#include <stack>
#include <string>
#include <vector>

// #define ERROR(str) std::cout << str << std::endl;

typedef long int I32CONST_t;
typedef double F32CONST_t;
typedef enum Basic_Type {
    I32,
    F32,
} Basic_Type;

typedef enum LANG_LEVEL {
    AST,
    IR,
    LOWIR,
    ASM,
} LANG_LEVEL;

#define ERROR(str)                                                                                         \
    do {                                                                                                   \
        std::cerr << (str ": ") << __PRETTY_FUNCTION__ << " " << __FILE__ << ":" << __LINE__ << std::endl; \
        exit(-1);                                                                                          \
    } while (0)

#define ASSERT(cond, str)                \
    do {                                 \
        if (!(cond)) ERROR("error: " str); \
    } while (0)

#define TODO() ERROR("TODO")
#endif
