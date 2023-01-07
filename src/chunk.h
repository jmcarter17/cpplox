#ifndef CPPLOX_CHUNK_H
#define CPPLOX_CHUNK_H


#include <vector>
#include <cstdint>
#include "value.h"
#include "fmt/format.h"
#include "magic_enum.hpp"

enum class OP : uint8_t {
    CONSTANT,
    NEGATE,
    ADD,
    SUBTRACT,
    MULTIPLY,
    DIVIDE,
    RETURN
};

template<typename E>
constexpr auto to_integral(E e) -> typename std::underlying_type<E>::type {
    return static_cast<typename std::underlying_type<E>::type>(e);
}

template<>
struct fmt::formatter<OP> {
    template<typename ParseContext>
    constexpr auto parse(ParseContext &ctx) {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(OP const &opcode, FormatContext &ctx) {
        return fmt::format_to(ctx.out(), "OP::{}", magic_enum::enum_name(opcode));
    }
};

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
    auto writeChunk(OP opcode, int line) -> void;
    auto addConstant(Value value) -> uint8_t;
    [[nodiscard]] auto getLine(size_t index) const -> int;
    void writeConstant(Value d, int line);
};


#endif //CPPLOX_CHUNK_H
