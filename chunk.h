//
// Created by jmcarter on 05/01/23.
//

#ifndef CPPLOX_CHUNK_H
#define CPPLOX_CHUNK_H


#include <vector>
#include <cstdint>
#include "vendor/magic_enum.hpp"
#include "value.h"

enum OP : uint8_t {
    CONSTANT,
    RETURN
};

std::ostream& operator<< (std::ostream& os, const OP& code);

struct Chunk {
    struct line_info {
        const int line_no;
        int num_instructions;

        explicit line_info(int line_no) : line_no(line_no), num_instructions(1) {}
    };
    std::vector<uint8_t> code;
    std::vector<Value> constants;
    std::vector<line_info> lines;

    auto writeChunk(uint8_t, int line) -> void;
    auto addConstant(Value value) -> uint8_t;
    auto printChunks() -> void;

    auto getLine(size_t index) const -> int;
};


#endif //CPPLOX_CHUNK_H
