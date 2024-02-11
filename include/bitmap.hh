#ifndef __BITMAP__
#define __BITMAP__
#include <cassert>
#include <iostream>
#include <stddef.h>
#include <vector>
#define BASIC_T size_t
#define UNIT_LENGTH (sizeof(BASIC_T))

class BitMap {
private:
    size_t size;
    std::vector<BASIC_T> base;

public:
    BitMap(size_t element_num) {
        size = (element_num + UNIT_LENGTH - 1) / UNIT_LENGTH;
        base.resize(size);
    }
    size_t get_element_num() {
        return size * UNIT_LENGTH;
    }
    // BitMap(const BitMap &src) {
    //     size = src.size;
    //     base = b;
    //     for (size_t i = 0; i < this->size; i++)
    //         this->base[i] = src.base[i];
    // }
    // BitMap operator=(const BitMap &src) {
    //     return BitMap(src);
    // }
    BitMap operator|(const BitMap &src) {
        assert(this->size == src.size);
        BitMap res(src.size * UNIT_LENGTH);
        for (size_t i = 0; i < this->size; i++)
            res.base[i] = this->base[i] | src.base[i];
        return res;
    }
    BitMap operator&(const BitMap &src) {
        assert(this->size == src.size);
        BitMap res(src.size * UNIT_LENGTH);
        for (size_t i = 0; i < this->size; i++)
            res.base[i] = this->base[i] & src.base[i];
        return res;
    }
    BitMap operator^(const BitMap &src) {
        assert(this->size == src.size);
        BitMap res(src.size * UNIT_LENGTH);
        for (size_t i = 0; i < this->size; i++)
            res.base[i] = this->base[i] ^ src.base[i];
        return res;
    }
    BitMap operator-(const BitMap &src) {
        assert(this->size == src.size);
        BitMap res(src.size * UNIT_LENGTH);
        for (size_t i = 0; i < this->size; i++)
            res.base[i] = this->base[i] & ~src.base[i];
        return res;
    }
    bool operator==(const BitMap &src) {
        assert(this->size == src.size);
        for (size_t i = 0; i < this->size; i++)
            if ((bool) (this->base[i] != src.base[i]))
                return false;
        return true;
    }
    bool operator!=(const BitMap &src) {
        return !(*this == src);
    }
    BitMap operator~() {
        BitMap res(size * UNIT_LENGTH);
        for (size_t i = 0; i < this->size; i++)
            res.base[i] = ~this->base[i];
        return res;
    }
    void add(size_t id) {
        assert((id / UNIT_LENGTH) < this->size);
        this->base[id / UNIT_LENGTH] |= (BASIC_T) 1 << (id % UNIT_LENGTH);
    }
    void cut(size_t id) {
        assert((id / UNIT_LENGTH) < this->size);
        this->base[id / UNIT_LENGTH] &= ~((BASIC_T) 1 << (id % UNIT_LENGTH));
    }
    void set_empty() {
        for (size_t i = 0; i < this->size; i++)
            this->base[i] = (BASIC_T) 0;
    }
    void set_full() {
        for (size_t i = 0; i < this->size; i++)
            this->base[i] = ~(BASIC_T) 0;
    }
    void print() {
        std::cout << "{";
        bool first = false;
        for (size_t i = this->size - 1; i < this->size; i--) {
            for (size_t j = UNIT_LENGTH - 1; j < UNIT_LENGTH; j--) {
                if (this->if_in((i * UNIT_LENGTH) + j)) {
                    if (first) {
                        std::cout << ", ";
                    }
                    std::cout << (i * UNIT_LENGTH) + j;
                    first = true;
                }
            }
        }
        std::cout << "}";
    }
    bool if_in(size_t id) {
        assert((id / UNIT_LENGTH) < this->size);
        return (bool) ((this->base[id / UNIT_LENGTH] >> (id % UNIT_LENGTH)) & (BASIC_T) 1);
    }
    size_t get_num() {
        size_t num = 0;
        for (size_t i = this->size - 1; i < this->size; i--) {
            for (size_t j = UNIT_LENGTH - 1; j < UNIT_LENGTH; j--) {
                if (this->if_in((i * UNIT_LENGTH) + j)) {
                    num++;
                }
            }
        }
        return num;
    }
};
#endif