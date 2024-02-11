#include "utils.hh"
#include <cstddef>
#include <type.hh>
Type::Type(Basic_Type basic_type, size_t ptr_level) {
    this->is_basic = true;
    this->basic_type = basic_type;
    this->size = 1;
    this->level = 0;
    this->ptr_level = ptr_level;
}
Type::Type(Type *next_level, size_t length, size_t ptr_level) {
    this->is_basic = false;
    this->next_level = next_level;
    this->size = length * next_level->size;
    this->level = next_level->level + 1;
    this->ptr_level = ptr_level;
}
Type *Type::copy() {
    if (this->is_basic)
        return new Type(this->basic_type, this->ptr_level);
    return new Type(this->next_level->copy(), this->get_length(), this->ptr_level);
}
Basic_Type Type::get_basic_type() {
    assert(this->is_basic);
    return this->basic_type;
}
void Type::set_ptr_level(size_t ptr_level) {
    this->ptr_level = ptr_level;
}
void Type::ptr_level_inc() {
    this->ptr_level++;
}
void Type::ptr_level_dec() {
    this->ptr_level--;
}
size_t Type::get_ptr_level() {
    return this->ptr_level;
}
void Type::print() {
    if (this->is_basic) {
        switch (this->basic_type) {
        case I32:
            std::cout << " i32";
            break;
        case F32:
            std::cout << " f32";
            break;
        }
    }
    else {
        std::cout << " [" << this->get_length() << " X ";
        this->next_level->print();
        std::cout << "]";
    }
    for (size_t i = 0; i < this->ptr_level; i++)
        std::cout << "*";
}
size_t Type::get_level() {
    return this->level;
}
size_t Type::get_length() {
    if (this->is_basic)
        return 1;
    return this->size / this->next_level->size;
}
size_t Type::get_size() {
    return this->size;
}
bool Type::get_is_basic() {
    return this->is_basic;
}
Type *Type::get_next_level() {
    assert(!this->is_basic);
    return this->next_level;
}
Type::~Type() {
    if (this->is_basic) return;
    delete this->next_level;
}