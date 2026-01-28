#include <Bytecode/Chunk.hpp>
#include <Bytecode/Instruction.hpp>
#include <Bytecode/CompiledFunction.hpp>
#include <VirtualMachine/VirtualMachine.hpp>

#include <chrono>
#include <iostream>

using namespace Fig;

int main()
{
    /*
        func fib(x)
        {
            if x <= 1
            {
                return x;
            }
            return fib(x - 1) + fib(x - 2);
        }
    */

    // ---------------- fib ----------------

    Instructions fib_ins{
        /*  0 */ {OpCode::LOAD_LOCAL, 0},   // x
        /*  1 */ {OpCode::LOAD_CONST, 0},   // 1
        /*  2 */ {OpCode::LTET},               // x <= 1
        /*  3 */ {OpCode::JUMP_IF_FALSE, 2}, // false -> jump to 6

        /*  4 */ {OpCode::LOAD_LOCAL, 0}, // return x
        /*  5 */ {OpCode::RETURN},

        /*  6 */ {OpCode::LOAD_LOCAL, 0}, // x
        /*  7 */ {OpCode::LOAD_CONST, 0}, // 1
        /*  8 */ {OpCode::SUB},              // x - 1
        /*  9 */ {OpCode::LOAD_CONST, 2}, // fib
        /* 10 */ {OpCode::CALL, 1},       // fib(x-1)

        /* 11 */ {OpCode::LOAD_LOCAL, 0}, // x
        /* 12 */ {OpCode::LOAD_CONST, 1}, // 2
        /* 13 */ {OpCode::SUB},              // x - 2
        /* 14 */ {OpCode::LOAD_CONST, 2}, // fib
        /* 15 */ {OpCode::CALL, 1},       // fib(x-2)

        /* 16 */ {OpCode::ADD},
        /* 17 */ {OpCode::RETURN},
    };

    std::vector<Object> fib_consts{
        Object((int64_t) 1), // 0
        Object((int64_t) 2), // 1
        Object(),            // 2 fib (回填)
    };

    CompiledFunction fib_fn{
        {},
        u8"fib",
        1, // posArgCount
        0,
        false,
        0, // localCount
        1  // slotCount = 参数 x
    };

    // fib 自引用
    fib_consts[2] = Object(Function(&fib_fn));

    Chunk fib_chunk{fib_ins, fib_consts, {}, ChunkAddressInfo{}};

    fib_fn.chunk = fib_chunk;

    // ---------------- main ----------------

    Instructions main_ins{
        {OpCode::LOAD_CONST, 0}, // 30
        {OpCode::LOAD_CONST, 1}, // fib
        {OpCode::CALL, 1},
        {OpCode::RETURN},
    };

    std::vector<Object> main_consts{
        Object((int64_t)251), // 0
        Object(Function(&fib_fn)),       // 1
    };

    Chunk main_chunk{main_ins, main_consts, {}, ChunkAddressInfo{}};

    CompiledFunction main_fn{main_chunk, u8"main", 0, 0, false, 0, 0};

    CallFrame entry{.ip = 0, .base = 0, .fn = main_fn};

    VirtualMachine vm(entry);

    using Clock = std::chrono::high_resolution_clock;

    auto start = Clock::now();
    Object result = vm.Execute();
    auto end = Clock::now();

    auto duration_secs = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();
    auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::cout << result.toString().toBasicString() << "\n";
    std::cout << "cost: " << duration_secs << "s. " << duration_ms << "ms" << "\n";
}
