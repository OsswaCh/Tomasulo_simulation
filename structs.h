
#pragma once
#include "includes.h"
using namespace std;

/////////////////////////////////////////Hardware input/////////////////////////////////////////
struct hardware_input
{

    // default sizes of the reservation stations
    int adders = 4;
    int multipliers = 2;
    int loaders = 2;
    int stores = 1;
    int branches = 1;
    int nanders = 2;
    int call_ret = 1;

    // default number of cycles
    int load_read_cycles = 4;
    int load_compute_address_cycles = 2;

    int store_write_cycles = 4;
    int store_compute_address_cycles = 2;

    int branch_cycles = 1;
    int call_ret_cycles = 1;
    int add_cycles = 2;
    int mul_cycles = 8;
    int nand_cycles = 1;

    // we may need these??????
    int memory;
    int registers;

    void user_input()
    {
        // hardware input
        cout << "Enter the number of adders: ";
        cin >> adders;
        cout << "Enter the number of multipliers: ";
        cin >> multipliers;
        cout << "Enter the number of loaders: ";
        cin >> loaders;
        cout << "Enter the number of stores: ";
        cin >> stores;
        cout << "Enter the number of nanders: ";
        cin >> nanders;

        // cycles input
        cout << "Enter the number of cycles for load instruction to compute the address: ";
        cin >> load_compute_address_cycles;
        cout << "Enter the number of cycles for load instruction to read from memory: ";
        cin >> load_read_cycles;
        cout << "Enter the number of cycles for store instruction to compute the address: ";
        cin >> store_compute_address_cycles;
        cout << "Enter the number of cycles for store instruction to write to memory: ";
        cin >> store_write_cycles;
        cout << "Enter the number of cycles for branch: ";
        cin >> branch_cycles;
        cout << "Enter the number of cycles for call/ret: ";
        cin >> call_ret_cycles;
        cout << "Enter the number of cycles for add: ";
        cin >> add_cycles;
        cout << "Enter the number of cycles for mul: ";
        cin >> mul_cycles;
        cout << "Enter the number of cycles for nand: ";
        cin >> nand_cycles;
    }
};

/////////////////////////////////////////other classes/////////////////////////////////////////
// first step
class instruction
{

public:
    // intruction type
    enum type
    {
        NO_OP,
        ADD,
        ADDI,
        NAND,
        MUL,
        LOAD,
        STORE,
        BEQ,
        CALL,
        RET
    };

    // instruction components
    instruction::type OP;

    int rs1;
    int rs2;
    int rd;
    int imm;

    string label;
    // used for the tracking of instruction
    short issue_cycle;
    short execution_start_cycle;
    short excecution_end_cycle;
    short write_back_cycle;

    short cycle_count_per_instruction;
    short cycles_left;

    short instruction_status;

    bool finished;

    // after branch
    bool after_branch = false;

    // constructor
    instruction(instruction::type operation = NO_OP, int rs1 = 0, int rs2 = 0, int rd = 0, int imm = 0, short cycles = 0, bool after_branch = false) : OP(operation),
                                                                                                                                                       rs1(rs1), rs2(rs2), rd(rd), imm(imm), cycle_count_per_instruction(cycles), after_branch(after_branch), label("")
    {
        issue_cycle = -1;
        execution_start_cycle = -1;
        excecution_end_cycle = -1;
        write_back_cycle = -1;
        cycles_left = cycles;

        finished = false;
    }

    // destructor
    ~instruction() {}
};

class reservation_station
{

public:
    enum TYPES
    {
        ADDER,
        MUL,
        LOAD,
        STORE,
        NAND,
        CALL_RET,
        BEQ
    };

    // functions to be inherited by other classes
    bool execute_ready();
    bool wb_ready();
    // void execute();
    // void write_back();
    void flush();
    // void branch();
    // void no_branch();
    // void call_ret();

    // void issue(instruction &inst);  // Issue method
    // void update();  // Update method

    // components of reservation station

    reservation_station::TYPES OP; // LOAD, STORE, BEQ, CALL, ADD, NAND, MUL
    string name;
    string Qj, Qk;
    int Vj, Vk;
    int A;
    int cycles;
    int compute_address_cycles;

    instruction *inst;
    int imm;
    int result;

    // flags for the reservation station to keep track of the instruction
    bool computed_effective_A;
    bool busy;              // issues
    bool started_execution; // execution started but didn't finish
    bool executed;          // finished execution but didn't write back
    bool finished;          // wrote back

    // number of reservation stations
    int size;

    // contructors
    reservation_station(reservation_station::TYPES OP = ADDER,
                        string name = "",
                        bool busy = false,
                        string Qj = "",
                        string Qk = "",
                        int A = 0,
                        int Vj = -1,
                        int Vk = -1,
                        int size = 0,
                        int cycles = 0,
                        int compute_address_cycles = 0,
                        instruction *inst = nullptr) : OP(OP), name(name), busy(busy), Qj(Qj), Qk(Qk), A(A), Vj(Vj), Vk(Vk), size(size), cycles(cycles), inst(inst), imm(0), computed_effective_A(false),
                                                       compute_address_cycles(compute_address_cycles), result(0),
                                                       started_execution(false), executed(false), finished(false)

    {
    }

    // destructor
    ~reservation_station()
    {
        inst = nullptr;
    }
};
