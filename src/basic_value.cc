#include <basic_value.hh>
Basic_Value::Basic_Value() {
    this->type = nullptr;
}
Basic_Value::Basic_Value(I32CONST_t int_num) {
    this->int_num = int_num;
    this->type = new Type(I32);
}
Basic_Value::Basic_Value(F32CONST_t float_num) {
    this->float_num = float_num;
    this->type = new Type(F32);
}
Basic_Value::Basic_Value(const Basic_Value &val) {
    this->type = val.type->copy();
    this->int_num = val.int_num;
}
I32CONST_t Basic_Value::get_i32_val() {
    assert(this->get_basic_type() == I32);
    return this->int_num;
}
F32CONST_t Basic_Value::get_f32_val() {
    assert(this->get_basic_type() == F32);
    return this->float_num;
}
void Basic_Value::print() {
    assert(type->get_is_basic());
    this->type->print();
    switch (this->type->get_basic_type()) {
    case I32:
        std::cout << " " << this->int_num;
        break;
    case F32:
        std::cout << " " << this->float_num;
        break;
    }
}
std::ostream &operator<<(std::ostream &os, const Basic_Value &val) {
    assert(val.type->get_is_basic());
    val.type->print();
    switch (val.type->get_basic_type()) {
    case I32:
        std::cout << " " << val.int_num;
        break;
    case F32:
        std::cout << " " << val.float_num;
        break;
    }
    return os;
}
Basic_Type Basic_Value::get_basic_type() {
    return this->type->get_basic_type();
}
Type *Basic_Value::get_type() { return this->type; }
Basic_Value &Basic_Value ::operator=(const Basic_Value &src) {
    if (!src.type) {
        this->type = nullptr;
        return *this;
    }
    this->type = src.type->copy();
    this->int_num = src.int_num;
    return *this;
}
Basic_Value Basic_Value::operator+(const Basic_Value &src2) {
    Basic_Type type1 = this->get_basic_type();
    Basic_Type type2 = src2.type->get_basic_type();
    if (type1 == I32) {
        if (type2 == F32)
            return Basic_Value((F32CONST_t) this->int_num + src2.float_num);
        if (type2 == I32)
            return Basic_Value(this->int_num + src2.int_num);
        assert(0);
    }
    if (type1 == F32) {
        if (type2 == F32)
            return Basic_Value(this->float_num + src2.float_num);
        if (type2 == I32)
            return Basic_Value(this->float_num + (F32CONST_t) src2.float_num);
        assert(0);
    }
    assert(0);
}
Basic_Value Basic_Value::operator-(const Basic_Value &src2) {
    Basic_Type type1 = this->get_basic_type();
    Basic_Type type2 = src2.type->get_basic_type();
    if (type1 == I32) {
        if (type2 == F32)
            return Basic_Value((F32CONST_t) this->int_num - src2.float_num);
        if (type2 == I32)
            return Basic_Value(this->int_num - src2.int_num);
    }
    if (type1 == F32) {
        if (type2 == F32)
            return Basic_Value(this->float_num - src2.float_num);
        if (type2 == I32)
            return Basic_Value(this->float_num - (F32CONST_t) src2.float_num);
    }
    assert(0);
}
Basic_Value Basic_Value::operator*(const Basic_Value &src2) {
    Basic_Type type1 = this->get_basic_type();
    Basic_Type type2 = src2.type->get_basic_type();
    if (type1 == I32) {
        if (type2 == F32)
            return Basic_Value((F32CONST_t) this->int_num * src2.float_num);
        if (type2 == I32)
            return Basic_Value(this->int_num * src2.int_num);
    }
    if (type1 == F32) {
        if (type2 == F32)
            return Basic_Value(this->float_num * src2.float_num);
        if (type2 == I32)
            return Basic_Value(this->float_num * (F32CONST_t) src2.float_num);
    }
    assert(0);
}
Basic_Value Basic_Value::operator/(const Basic_Value &src2) {
    Basic_Type type1 = this->get_basic_type();
    Basic_Type type2 = src2.type->get_basic_type();
    if (type1 == I32) {
        if (type2 == F32)
            return Basic_Value((F32CONST_t) this->int_num / src2.float_num);
        if (type2 == I32)
            return Basic_Value(this->int_num / src2.int_num);
    }
    if (type1 == F32) {
        if (type2 == F32)
            return Basic_Value(this->float_num / src2.float_num);
        if (type2 == I32)
            return Basic_Value(this->float_num / (F32CONST_t) src2.float_num);
    }
    assert(0);
}
Basic_Value Basic_Value::operator%(const Basic_Value &src2) {
    Basic_Type type1 = this->get_basic_type();
    Basic_Type type2 = src2.type->get_basic_type();
    assert(type1 == I32);
    assert(type2 == I32);
    return Basic_Value(this->int_num % src2.int_num);
}
Basic_Value Basic_Value::operator<(const Basic_Value &src2) {
    Basic_Type type1 = this->get_basic_type();
    Basic_Type type2 = src2.type->get_basic_type();
    if (type1 == I32) {
        if (type2 == F32)
            return Basic_Value(I32CONST_t(this->int_num < src2.float_num));
        if (type2 == I32)
            return Basic_Value(I32CONST_t(this->int_num < src2.int_num));
    }
    if (type1 == F32) {
        if (type2 == F32)
            return Basic_Value(I32CONST_t(this->float_num < src2.float_num));
        if (type2 == I32)
            return Basic_Value(I32CONST_t(this->float_num < (F32CONST_t) src2.float_num));
    }
    assert(0);
}
Basic_Value Basic_Value::operator>(const Basic_Value &src2) {
    Basic_Type type1 = this->get_basic_type();
    Basic_Type type2 = src2.type->get_basic_type();
    if (type1 == I32) {
        if (type2 == F32)
            return Basic_Value(I32CONST_t(this->int_num > src2.float_num));
        if (type2 == I32)
            return Basic_Value(I32CONST_t(this->int_num > src2.int_num));
    }
    if (type1 == F32) {
        if (type2 == F32)
            return Basic_Value(I32CONST_t(this->float_num > src2.float_num));
        if (type2 == I32)
            return Basic_Value(I32CONST_t(this->float_num > (F32CONST_t) src2.float_num));
    }
    assert(0);
}
Basic_Value Basic_Value::operator<=(const Basic_Value &src2) {
    Basic_Type type1 = this->get_basic_type();
    Basic_Type type2 = src2.type->get_basic_type();
    if (type1 == I32) {
        if (type2 == F32)
            return Basic_Value(I32CONST_t(this->int_num <= src2.float_num));
        if (type2 == I32)
            return Basic_Value(I32CONST_t(this->int_num <= src2.int_num));
    }
    if (type1 == F32) {
        if (type2 == F32)
            return Basic_Value(I32CONST_t(this->float_num <= src2.float_num));
        if (type2 == I32)
            return Basic_Value(I32CONST_t(this->float_num <= (F32CONST_t) src2.float_num));
    }
    assert(0);
}
Basic_Value Basic_Value::operator>=(const Basic_Value &src2) {
    Basic_Type type1 = this->get_basic_type();
    Basic_Type type2 = src2.type->get_basic_type();
    if (type1 == I32) {
        if (type2 == F32)
            return Basic_Value(I32CONST_t(this->int_num >= src2.float_num));
        if (type2 == I32)
            return Basic_Value(I32CONST_t(this->int_num >= src2.int_num));
    }
    if (type1 == F32) {
        if (type2 == F32)
            return Basic_Value(I32CONST_t(this->float_num >= src2.float_num));
        if (type2 == I32)
            return Basic_Value(I32CONST_t(this->float_num >= (F32CONST_t) src2.float_num));
    }
    assert(0);
}
Basic_Value Basic_Value::operator==(const Basic_Value &src2) {
    Basic_Type type1 = this->get_basic_type();
    Basic_Type type2 = src2.type->get_basic_type();
    if (type1 == I32) {
        if (type2 == F32)
            return Basic_Value(I32CONST_t(this->int_num == src2.float_num));
        if (type2 == I32)
            return Basic_Value(I32CONST_t(this->int_num == src2.int_num));
    }
    if (type1 == F32) {
        if (type2 == F32)
            return Basic_Value(I32CONST_t(this->float_num == src2.float_num));
        if (type2 == I32)
            return Basic_Value(I32CONST_t(this->float_num == (F32CONST_t) src2.float_num));
    }
    assert(0);
}
Basic_Value Basic_Value::operator!=(const Basic_Value &src2) {
    Basic_Type type1 = this->get_basic_type();
    Basic_Type type2 = src2.type->get_basic_type();
    if (type1 == I32) {
        if (type2 == F32)
            return Basic_Value(I32CONST_t(this->int_num != src2.float_num));
        if (type2 == I32)
            return Basic_Value(I32CONST_t(this->int_num != src2.int_num));
    }
    if (type1 == F32) {
        if (type2 == F32)
            return Basic_Value(I32CONST_t(this->float_num != src2.float_num));
        if (type2 == I32)
            return Basic_Value(I32CONST_t(this->float_num != (F32CONST_t) src2.float_num));
    }
    assert(0);
}
Basic_Value Basic_Value::operator!() {
    Basic_Type type1 = this->get_basic_type();
    if (type1 == I32) {
        return Basic_Value((I32CONST_t) !this->int_num);
    }
    return Basic_Value((I32CONST_t) !this->float_num);
}

Basic_Value Basic_Value::operator||(const Basic_Value &src2) {
    Basic_Type type1 = this->get_basic_type();
    Basic_Type type2 = src2.type->get_basic_type();
    if (type1 == I32) {
        if (type2 == F32)
            return Basic_Value(I32CONST_t(this->int_num || src2.float_num));
        if (type2 == I32)
            return Basic_Value(I32CONST_t(this->int_num || src2.int_num));
    }
    if (type1 == F32) {
        if (type2 == F32)
            return Basic_Value(I32CONST_t(this->float_num || src2.float_num));
        if (type2 == I32)
            return Basic_Value(I32CONST_t(this->float_num || (F32CONST_t) src2.float_num));
    }
    assert(0);
}

Basic_Value Basic_Value::operator&&(const Basic_Value &src2) {
    Basic_Type type1 = this->get_basic_type();
    Basic_Type type2 = src2.type->get_basic_type();
    if (type1 == I32) {
        if (type2 == F32)
            return Basic_Value(I32CONST_t(this->int_num && src2.float_num));
        if (type2 == I32)
            return Basic_Value(I32CONST_t(this->int_num && src2.int_num));
    }
    if (type1 == F32) {
        if (type2 == F32)
            return Basic_Value(I32CONST_t(this->float_num && src2.float_num));
        if (type2 == I32)
            return Basic_Value(I32CONST_t(this->float_num && (F32CONST_t) src2.float_num));
    }
    assert(0);
}
Basic_Value Basic_Value::operator-() {
    Basic_Type type1 = this->get_basic_type();
    if (type1 == I32) {
        return Basic_Value(-this->int_num);
    }
    return Basic_Value(-this->float_num);
}
Basic_Value::~Basic_Value() { delete type; }