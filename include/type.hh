#ifndef __TYPE__
#define __TYPE__
#include <cstddef>
#include <utils.hh>
class Type {
private:
    bool is_basic;
    union {
        Basic_Type basic_type;
        Type *next_level;
    };
    size_t size;
    size_t level;
    size_t ptr_level;

public:
    Type(Basic_Type basic_type, size_t ptr_level = 0);
    Type(Type *next_level, size_t length, size_t ptr_level = 0);
    Type *copy();
    Basic_Type get_basic_type();
    size_t get_level();
    size_t get_length();
    size_t get_size();
    bool get_is_basic();
    Type *get_next_level();
    void print();
    size_t get_ptr_level();
    void set_ptr_level(size_t ptr_level);
    void ptr_level_dec();
    void ptr_level_inc();
    ~Type();
};
#endif