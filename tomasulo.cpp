
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <queue>
#include <algorithm>
#include <climits>
// #include "tomasulo.h"
// #include "global.h"
using namespace std;

////////////////////////////////////////////////////////

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
                                                                                                                                                       rs1(rs1), rs2(rs2), rd(rd), imm(imm), cycle_count_per_instruction(cycles), after_branch(after_branch)
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

struct CBD
{
    string station_name;
    bool is_empty;
    int reg;
};

CBD cdb;

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

bool reservation_station::execute_ready()
{
    if (!this->busy)
        return false;

    // if FP operation--> return true if Qj and Qk are not empty
    if (this->OP == reservation_station::TYPES::ADDER ||
        this->OP == reservation_station::TYPES::MUL ||
        this->OP == reservation_station::TYPES::NAND)
        if (Qj == "" && Qk == "")
            return true;

    // //if load operation --> return true if Qj is empty and load buffer is not empty and the last element in the load buffer is the current instruction
    // if(this->OP == reservation_station::TYPES::LOAD)
    //    if(Qj=="" && loadBuffer.size() > 0 && loadBuffer[loadBuffer.size()-1]==name)
    //         return true;

    // TODO: add conditions for load, store, and the remaining instructions

    return false;
}

bool reservation_station::wb_ready()
{

    if (!this->executed)
        return false;

    if (this->OP == reservation_station::TYPES::ADDER ||
        this->OP == reservation_station::TYPES::MUL ||
        this->OP == reservation_station::TYPES::NAND ||
        this->OP == reservation_station::TYPES::LOAD)
        if (cdb.is_empty)
            return true;

    if (this->OP == reservation_station::TYPES::STORE)
        return this->Qk == "";

    return false;
}

void reservation_station::flush()
{
    busy = false;
    started_execution = false;
    executed = false;
    finished = false;

    Qj = "";
    Qk = "";
    Vj = -1;
    Vk = -1;
    A = 0;
    computed_effective_A = false;
    inst = nullptr;
    imm = 0;
}

struct reservation_stations
{

    vector<reservation_station> adders;
    vector<reservation_station> multipliers;
    vector<reservation_station> loader;
    vector<reservation_station> stores;
    vector<reservation_station> nanders;
    vector<reservation_station> call_ret;
    vector<reservation_station> branches;
    // vector<LoadBuffer> loadBuffer;
    // vector<StoreBuffer> storeBuffer;

    hardware_input hardware;

    // initialize the reservation stations
    reservation_stations(hardware_input hardware) : hardware(hardware)
    {
        for (int i = 0; i < hardware.adders; i++)
        {
            adders.push_back(reservation_station(reservation_station::TYPES::ADDER, "adder" + to_string(i), false, "", "", 0, -1, -1, hardware.adders, hardware.add_cycles));
        }
        for (int i = 0; i < hardware.multipliers; i++)
        {
            multipliers.push_back(reservation_station(reservation_station::TYPES::MUL, "multiplier" + to_string(i), false, "", "", 0, -1, -1, hardware.multipliers, hardware.mul_cycles));
        }
        for (int i = 0; i < hardware.loaders; i++)
        {
            loader.push_back(reservation_station(reservation_station::TYPES::MUL, "loader" + to_string(i), false, "", "", 0, -1, -1, hardware.loaders, hardware.load_read_cycles, hardware.load_compute_address_cycles));
        }
        for (int i = 0; i < hardware.stores; i++)
        {
            stores.push_back(reservation_station(reservation_station::TYPES::STORE, "store" + to_string(i), false, "", "", 0, -1, -1, hardware.stores, hardware.store_write_cycles, hardware.store_compute_address_cycles));
        }
        for (int i = 0; i < hardware.nanders; i++)
        {
            nanders.push_back(reservation_station(reservation_station::TYPES::NAND, "nander" + to_string(i), false, "", "", 0, -1, -1, hardware.nanders, hardware.nand_cycles));
        }
        for (int i = 0; i < hardware.call_ret; i++)
        {
            call_ret.push_back(reservation_station(reservation_station::TYPES::CALL_RET, "call_ret" + to_string(i), false, "", "", 0, -1, -1, hardware.call_ret, hardware.call_ret_cycles));
        }
        for (int i = 0; i < hardware.branches; i++)
        {
            branches.push_back(reservation_station(reservation_station::TYPES::BEQ, "branches" + to_string(i), false, "", "", 0, -1, -1, hardware.branches, hardware.branch_cycles));
        }
    }

    // check if the reservation station is full
    bool full(reservation_station::TYPES OP)
    {
        if (OP == reservation_station::TYPES::ADDER)
        {
            for (int i = 0; i < hardware.adders; i++)
                if (!adders[i].busy)
                    return false;
            return true;
        }
        else if (OP == reservation_station::TYPES::MUL)
        {
            for (int i = 0; i < hardware.multipliers; i++)
                if (!multipliers[i].busy)
                    return false;
            return true;
        }
        else if (OP == reservation_station::TYPES::LOAD)
        {
            for (int i = 0; i < hardware.loaders; i++)
                if (!loader[i].busy)
                    return false;
            return true;
        }
        else if (OP == reservation_station::TYPES::STORE)
        {
            for (int i = 0; i < hardware.stores; i++)
                if (!stores[i].busy)
                    return false;
            return true;
        }
        else if (OP == reservation_station::TYPES::NAND)
        {
            for (int i = 0; i < hardware.nanders; i++)
                if (!nanders[i].busy)
                    return false;
            return true;
        }
        else if (OP == reservation_station::TYPES::CALL_RET)
        {
            for (int i = 0; i < hardware.call_ret; i++)
                if (!call_ret[i].busy)
                    return false;
            return true;
        }
        else if (OP == reservation_station::TYPES::BEQ)
        {
            for (int i = 0; i < hardware.branches; i++)
                if (!branches[i].busy)
                    return false;
            return true;
        }
    }

    // destructors
    ~reservation_stations()
    {
        adders.clear();
        multipliers.clear();
        loader.clear();
        stores.clear();
        nanders.clear();
        call_ret.clear();
        branches.clear();
    }
};

class RegisterFile
{

private:
    // A struct used to keep track of which unit is reserving which regiter.
    // e.g., registerStat[2].Qi !=nullptr means that r2 is reserved by the a reservation station with
    // address in the pointer registerStat[2].Qi

    // refer to lecture 18
    struct RegisterItem
    {
        reservation_station *Qi;
        int value; // value of the register
    };

public:
    // register status array
    // vector<pair<int, bool>> register_station;

    vector<RegisterItem> register_stat;

    // constructor
    RegisterFile(int size = 16)
    {
        // register_station.resize(size, make_pair(0, false));

        register_stat.resize(size);
    }

    // destructor
    ~RegisterFile()
    {
        // register_station.clear();
    }

    // returns the status of the register
    bool is_busy(int reg)
    {
        // if (reg < 0 || reg >= register_station.size())
        // {
        //     return false;
        // }
        // return register_station[reg].second;

        if (reg >= register_stat.size()) // should be an error

        {
            cout << "ERROR! register porvided to is_busy exceeds the size of the register file\n";
            return false;
        }
        return register_stat[reg].Qi != nullptr;
    }

    // set the frees the register_state of the register reg
    void written_to(int reg)
    {
        // if (reg < 0 || reg >= register_station.size())
        // {
        //     return;
        // }
        // register_station[reg].second = false;

        register_stat[reg].Qi = nullptr;
    }
};
RegisterFile regfile;

// Globals.h
vector<reservation_station *> storeBuffer;
vector<reservation_station *> loadBuffer;

unsigned int data_memory[128];

// int PC;
// int current_cycle;

reservation_stations *res_stations;
vector<instruction> instructions;
vector<instruction> finished_instructions;
vector<reservation_station> after_branch_record;

typedef pair<int, bool> reg_item; // reg value and status
// vector<reg_item> registers;

// Globals.cpp

int PC = 0;

// clock related variables
int current_cycle = 0;
int total_branch_count = 0;
int branch_misprediction_count = 0;

bool program_finished()
{
    for (int i = 0; i < instructions.size(); i++)
    {
        if (instructions[i].write_back_cycle == -1)
        {
            return false;
        }
    }
    return true;
}

bool issue(instruction &inst)
{
    if (inst.OP == instruction::type::ADD || inst.OP == instruction::type::ADDI)
    {
        for (int i = 0; i < res_stations->hardware.adders; i++)
            if (!res_stations->adders[i].busy)
            {
                res_stations->adders[i].busy = true;
                res_stations->adders[i].inst = &inst;

                if (regfile.register_stat[inst.rs1].Qi != nullptr)
                    res_stations->adders[i].Qj = regfile.register_stat[inst.rs1].Qi->name;
                else
                    res_stations->adders[i].Vj = regfile.register_stat[inst.rs1].value;

                if (regfile.register_stat[inst.rs2].Qi != nullptr)
                    res_stations->adders[i].Qk = regfile.register_stat[inst.rs2].Qi->name;
                else
                    res_stations->adders[i].Vk = regfile.register_stat[inst.rs2].value;

                res_stations->adders[i].imm = inst.imm;
                res_stations->adders[i].OP = reservation_station::TYPES::ADDER;
                res_stations->adders[i].inst->issue_cycle = current_cycle;

                return true;
            }
    }
    else if (inst.OP == instruction::type::MUL)
    {
        for (int i = 0; i < res_stations->hardware.multipliers; i++)
            if (!res_stations->multipliers[i].busy)
            {
                res_stations->multipliers[i].busy = true;
                res_stations->multipliers[i].inst = &inst;

                if (regfile.register_stat[inst.rs1].Qi != nullptr)
                    res_stations->multipliers[i].Qj = regfile.register_stat[inst.rs1].Qi->name;
                else
                    res_stations->multipliers[i].Vj = regfile.register_stat[inst.rs1].value;

                if (regfile.register_stat[inst.rs2].Qi != nullptr)
                    res_stations->multipliers[i].Qk = regfile.register_stat[inst.rs2].Qi->name;
                else
                    res_stations->multipliers[i].Vk = regfile.register_stat[inst.rs2].value;

                res_stations->multipliers[i].imm = inst.imm;
                res_stations->multipliers[i].OP = reservation_station::TYPES::MUL;
                res_stations->multipliers[i].inst->issue_cycle = current_cycle;

                return true;
            }
    }
    else if (inst.OP == instruction::type::NAND)
    {
        for (int i = 0; i < res_stations->hardware.nanders; i++)
        {
            if (!res_stations->nanders[i].busy)
            {
                res_stations->nanders[i].busy = true;
                res_stations->nanders[i].inst = &inst;

                if (regfile.register_stat[inst.rs1].Qi != nullptr)
                    res_stations->nanders[i].Qj = regfile.register_stat[inst.rs1].Qi->name;
                else
                    res_stations->nanders[i].Vj = regfile.register_stat[inst.rs1].value;

                if (regfile.register_stat[inst.rs2].Qi != nullptr)
                    res_stations->nanders[i].Qk = regfile.register_stat[inst.rs2].Qi->name;
                else
                    res_stations->nanders[i].Vk = regfile.register_stat[inst.rs2].value;

                res_stations->nanders[i].imm = inst.imm;
                res_stations->nanders[i].OP = reservation_station::TYPES::NAND;
                res_stations->nanders[i].inst->issue_cycle = current_cycle;

                return true;
            }
        }
    }
    else if (inst.OP == instruction::type::LOAD)
    {
        for (int i = 0; i < res_stations->hardware.loaders; i++)
        {
            if (!res_stations->loader[i].busy)
            {
                res_stations->loader[i].busy = true;
                res_stations->loader[i].inst = &inst;

                if (regfile.register_stat[inst.rs1].Qi != nullptr)
                    res_stations->loader[i].Qj = regfile.register_stat[inst.rs1].Qi->name;
                else
                    res_stations->loader[i].Vj = regfile.register_stat[inst.rs1].value;

                res_stations->loader[i].imm = inst.imm;
                res_stations->loader[i].OP = reservation_station::TYPES::LOAD;
                res_stations->loader[i].inst->issue_cycle = current_cycle;

                loadBuffer.push_back(&res_stations->loader[i]);

                return true;
            }
        }
    }
    else if (inst.OP == instruction::type::STORE)
    {
        for (int i = 0; i < res_stations->hardware.stores; i++)
        {
            if (!res_stations->stores[i].busy)
            {
                res_stations->stores[i].busy = true;
                res_stations->stores[i].inst = &inst;

                if (regfile.register_stat[inst.rs1].Qi != nullptr)
                    res_stations->stores[i].Qj = regfile.register_stat[inst.rs1].Qi->name;
                else
                    res_stations->stores[i].Vj = regfile.register_stat[inst.rs1].value;

                if (regfile.register_stat[inst.rs2].Qi != nullptr)
                    res_stations->stores[i].Qk = regfile.register_stat[inst.rs2].Qi->name;
                else
                    res_stations->stores[i].Vk = regfile.register_stat[inst.rs2].value;

                res_stations->stores[i].imm = inst.imm;
                res_stations->stores[i].OP = reservation_station::TYPES::STORE;
                res_stations->stores[i].inst->issue_cycle = current_cycle;

                storeBuffer.push_back(&res_stations->stores[i]);

                return true;
            }
        }
    }
    else if (inst.OP == instruction::type::BEQ)
    {
        for (int i = 0; i < res_stations->hardware.branches; i++)
        {
            if (!res_stations->branches[i].busy)
            {
                res_stations->branches[i].busy = true;
                res_stations->branches[i].inst = &inst;

                if (regfile.register_stat[inst.rs1].Qi != nullptr)
                    res_stations->branches[i].Qj = regfile.register_stat[inst.rs1].Qi->name;
                else
                    res_stations->branches[i].Vj = regfile.register_stat[inst.rs1].value;

                if (regfile.register_stat[inst.rs2].Qi != nullptr)
                    res_stations->branches[i].Qk = regfile.register_stat[inst.rs2].Qi->name;
                else
                    res_stations->branches[i].Vk = regfile.register_stat[inst.rs2].value;

                res_stations->branches[i].imm = inst.imm;
                res_stations->branches[i].OP = reservation_station::TYPES::BEQ;
                res_stations->branches[i].inst->issue_cycle = current_cycle;
                total_branch_count += 1;

                return true;
            }
        }
    }

    /*
    note nadia: I added this but we might need to split it into two functions
    */
    // case of call operation
    else if (inst.OP == instruction::type::CALL || inst.OP == instruction::type::RET)
    {
        for (int i = 0; i < res_stations->hardware.call_ret; i++)
        {
            if (!res_stations->call_ret[i].busy)
            {
                res_stations->call_ret[i].busy = true;
                res_stations->call_ret[i].inst = &inst;

                if (regfile.register_stat[inst.rs1].Qi != nullptr)
                    res_stations->call_ret[i].Qj = regfile.register_stat[inst.rs1].Qi->name;
                else
                    res_stations->call_ret[i].Vj = regfile.register_stat[inst.rs1].value;

                if (regfile.register_stat[inst.rs2].Qi != nullptr)
                    res_stations->call_ret[i].Qk = regfile.register_stat[inst.rs2].Qi->name;
                else
                    res_stations->call_ret[i].Vk = regfile.register_stat[inst.rs2].value;

                res_stations->call_ret[i].imm = inst.imm;
                res_stations->call_ret[i].OP = reservation_station::TYPES::CALL_RET;
                res_stations->call_ret[i].inst->issue_cycle = current_cycle;

                return true;
            }
        }
    }

    return false;
}

void execute()
{

    // add & addi excecution
    for (int i = 0; i < res_stations->hardware.adders; i++)
    {
        if (res_stations->adders[i].executed) // if the instruction is already executed then skip
            continue;

        // case where the execution is not started yet
        if (!res_stations->adders[i].started_execution)
        {
            // check if the reservation station is ready
            if (res_stations->adders[i].execute_ready())
            {
                // update the reservation station
                res_stations->adders[i].started_execution = true;
                res_stations->adders[i].inst->execution_start_cycle = current_cycle;
                res_stations->adders[i].inst->cycles_left = res_stations->adders[i].cycles;
            }
        }
        else // execution started
        {

            /*note osswa check if it should be written back at 1 or 0*/

            // mark the end of the execution
            if (res_stations->adders[i].inst->cycles_left == 0)
            {
                res_stations->adders[i].inst->excecution_end_cycle = current_cycle;
                res_stations->adders[i].executed = true;
            }
            else // decrement the cycles left
                res_stations->adders[i].inst->cycles_left--;
        }
    }

    // mul excecution
    for (int i = 0; i < res_stations->hardware.multipliers; i++)
    {
        if (res_stations->multipliers[i].executed)
            continue;

        // case wher the execution is not started yet
        if (!res_stations->multipliers[i].started_execution)
        {
            // check if the reservation station is ready
            if (res_stations->multipliers[i].execute_ready())
            {
                // update the reservation station
                res_stations->multipliers[i].started_execution = true;
                res_stations->multipliers[i].inst->execution_start_cycle = current_cycle;
                res_stations->multipliers[i].inst->cycles_left = res_stations->multipliers[i].cycles;
            }
        }

        /*note osswa check if it should be written back at 1 or 0
         */
        // mark the end of the execution
        if (res_stations->multipliers[i].inst->cycles_left == 0)
        {
            res_stations->multipliers[i].inst->excecution_end_cycle = current_cycle;
            res_stations->multipliers[i].executed = true;
        }
        else // decrement the cycles left
            res_stations->multipliers[i].inst->cycles_left--;
    }

    // nand excecution
    for (int i = 0; i < res_stations->hardware.nanders; i++)
    {
        if (res_stations->nanders[i].executed)
            continue;

        // case wher the execution is not started yet
        if (!res_stations->nanders[i].started_execution)
        {
            // check if the reservation station is ready
            if (res_stations->nanders[i].execute_ready())
            {
                // update the reservation station
                res_stations->nanders[i].started_execution = true;
                res_stations->nanders[i].inst->execution_start_cycle = current_cycle;
                res_stations->nanders[i].inst->cycles_left = res_stations->nanders[i].cycles;
            }
        }

        /*note osswa check if it should be written back at 1 or 0
         */
        // mark the end of the execution
        if (res_stations->nanders[i].inst->cycles_left == 0)
        {
            res_stations->nanders[i].inst->excecution_end_cycle = current_cycle;
            res_stations->nanders[i].executed = true;
        }
        else // decrement the cycles left
            res_stations->nanders[i].inst->cycles_left--;
    }

    // load excecution
    for (int i = 0; i < res_stations->hardware.loaders; i++)
    {
        if (res_stations->loader[i].executed)
            continue;
        // case wher the execution is not started yet
        if (!res_stations->loader[i].started_execution)
        {
            // check if the reservation station is ready
            if (res_stations->loader[i].execute_ready())
            {
                // update the reservation station
                res_stations->loader[i].started_execution = true;
                res_stations->loader[i].inst->execution_start_cycle = current_cycle;
                res_stations->loader[i].inst->cycles_left = res_stations->loader[i].compute_address_cycles;
            }
        }

        /*note osswa check if it should be written back at 1 or 0
         */
        // mark the end of the execution
        if (res_stations->loader[i].inst->cycles_left == 0)
        {
            if (!res_stations->loader[i].computed_effective_A)
            {

                res_stations->loader[i].A = res_stations->loader[i].Vj + res_stations->loader[i].imm;
                res_stations->loader[i].computed_effective_A = true;
            }
            else
            {
                res_stations->loader[i].inst->excecution_end_cycle = current_cycle;
                res_stations->loader[i].executed = true;
            }
        }

        else // decrement the cycles left
            res_stations->loader[i].inst->cycles_left--;
    }

    // store excecution
    for (int i = 0; i < res_stations->hardware.stores; i++)
    {
        if (res_stations->stores[i].executed)
            continue;
        // case wher the execution is not started yet
        if (!res_stations->stores[i].started_execution)
        {
            // check if the reservation station is ready
            if (res_stations->stores[i].execute_ready())
            {
                // update the reservation station
                res_stations->stores[i].started_execution = true;
                res_stations->stores[i].inst->execution_start_cycle = current_cycle;
                res_stations->stores[i].inst->cycles_left = res_stations->stores[i].cycles;
            }
        }

        else
        {

            /*note osswa check if it should be written back at 1 or 0
             */
            // mark the end of the execution
            if (res_stations->stores[i].inst->cycles_left == 0)
            {
                res_stations->stores[i].inst->excecution_end_cycle = current_cycle;
                res_stations->stores[i].executed = true;
            }
            else // decrement the cycles left
                res_stations->stores[i].inst->cycles_left--;
        }
    }

    // branch excecution
    for (int i = 0; i < res_stations->hardware.branches; i++)
    {
        if (res_stations->branches[i].executed)
            continue;
        // case wher the execution is not started yet
        if (!res_stations->branches[i].started_execution)
        {
            // check if the reservation station is ready
            if (res_stations->branches[i].execute_ready())
            {
                // update the reservation station
                res_stations->branches[i].started_execution = true;
                res_stations->branches[i].inst->execution_start_cycle = current_cycle;
                res_stations->branches[i].inst->cycles_left = res_stations->branches[i].cycles;
            }
        }

        else
        {

            /*note osswa check if it should be written back at 1 or 0
             */
            // mark the end of the execution
            if (res_stations->branches[i].inst->cycles_left == 0)
            {
                res_stations->branches[i].inst->excecution_end_cycle = current_cycle;
                res_stations->branches[i].executed = true;
            }
            else // decrement the cycles left
                res_stations->branches[i].inst->cycles_left--;
        }
    }

    // call_ret excecution
    for (int i = 0; i < res_stations->hardware.call_ret; i++)
    {
        if (res_stations->call_ret[i].executed)
            continue;
        // case wher the execution is not started yet
        if (!res_stations->call_ret[i].started_execution)
        {
            // check if the reservation station is ready
            if (res_stations->call_ret[i].execute_ready())
            {
                // update the reservation station
                res_stations->call_ret[i].started_execution = true;
                res_stations->call_ret[i].inst->execution_start_cycle = current_cycle;
                res_stations->call_ret[i].inst->cycles_left = res_stations->call_ret[i].cycles;
            }
        }

        else
        {

            /*note osswa check if it should be written back at 1 or 0
             */
            // mark the end of the execution
            if (res_stations->call_ret[i].inst->cycles_left == 0)
            {
                res_stations->call_ret[i].inst->excecution_end_cycle = current_cycle;
                res_stations->call_ret[i].executed = true;
            }
            else // decrement the cycles left
                res_stations->call_ret[i].inst->cycles_left--;
        }
    }
}

void reserveCDB(const reservation_station &rs)
{
    cdb.is_empty = false;
    cdb.reg = rs.inst->rd;
    cdb.station_name = rs.name;
}

void write_back()
{

    // add write back
    for (int i = 0; i < res_stations->hardware.adders; i++)
    {
        // check if the reservation station is ready
        if (res_stations->adders[i].wb_ready())
        {
            // update the cycles
            res_stations->adders[i].inst->write_back_cycle = current_cycle;

            if (res_stations->adders[i].inst->OP == instruction::type::ADD)
                res_stations->adders[i].result = res_stations->adders[i].Vj + res_stations->adders[i].Vk;
            else if (res_stations->adders[i].inst->OP == instruction::type::ADDI)
                res_stations->adders[i].result = res_stations->adders[i].Vj + res_stations->adders[i].imm;

            reserveCDB(res_stations->adders[i]);
            // push the finished instruction to the finished instructions vector
            finished_instructions.push_back(*res_stations->adders[i].inst);

            // flush the instruction
            res_stations->adders[i].flush();
        }
    }

    // mul write back
    for (int i = 0; i < res_stations->hardware.multipliers; i++)
    {
        // check if the reservation station is ready
        if (res_stations->multipliers[i].wb_ready())
        {
            // update the cycles
            res_stations->multipliers[i].inst->write_back_cycle = current_cycle;

            // update the rd
            res_stations->multipliers[i].result = res_stations->multipliers[i].Vj * res_stations->multipliers[i].Vk;

            // push the finished instruction to the finished instructions vector
            finished_instructions.push_back(*res_stations->multipliers[i].inst);
            reserveCDB(res_stations->multipliers[i]);

            // flush the instruction
            res_stations->multipliers[i].flush();
        }
    }

    // nand write back
    for (int i = 0; i < res_stations->hardware.nanders; i++)
    {
        // check if the reservation station is ready
        if (res_stations->nanders[i].wb_ready())
        {
            // update the cycles
            res_stations->nanders[i].inst->write_back_cycle = current_cycle;

            // update the rd
            res_stations->nanders[i].result = ~(res_stations->nanders[i].Vj & res_stations->nanders[i].Vk);

            // push the finished instruction to the finished instructions vector
            finished_instructions.push_back(*res_stations->nanders[i].inst);
            reserveCDB(res_stations->nanders[i]);

            // flush the instruction
            res_stations->nanders[i].flush();
        }
    }

    // branch write back

    for (int i = 0; i < res_stations->hardware.branches; i++)
    {
        // check if the reservation station is ready
        if (res_stations->branches[i].wb_ready())
        {
            // update the cycles
            res_stations->branches[i].inst->write_back_cycle = current_cycle;

            // update the rd only if the condition is true & call the branch function
            if (res_stations->branches[i].inst->rs1 == res_stations->branches[i].inst->rs2)
            {
                // branch();
                branch_misprediction_count++; // increment the branch misprediction count since we have a no branch predictor
            }

            // push the finished instruction to the finished instructions vector
            finished_instructions.push_back(*res_stations->branches[i].inst);

            total_branch_count++;

            // flush the instruction
            res_stations->branches[i].flush();

            // update the PC by adding the immidiate calue to the current PC
            PC += res_stations->branches[i].imm;
        }
    }

    // load write back
    for (int i = 0; i < res_stations->hardware.loaders; i++)
    {
        // check if the reservation station is ready
        if (res_stations->loader[i].wb_ready())
        {
            // update the cycles
            res_stations->loader[i].inst->write_back_cycle = current_cycle;

            // note osswa : we might change this
            res_stations->loader[i].A = res_stations->loader[i].Vj + res_stations->loader[i].imm;

            // push the finished instruction to the finished instructions vector
            finished_instructions.push_back(*res_stations->loader[i].inst);
            reserveCDB(res_stations->loader[i]);

            // flush the instruction
            res_stations->loader[i].flush();
        }
    }

    // store write back
    for (int i = 0; i < res_stations->hardware.stores; i++)
    {
        // check if the reservation station is ready
        if (res_stations->stores[i].wb_ready())
        {
            // update the cycles
            res_stations->stores[i].inst->write_back_cycle = current_cycle;

            // note osswa : we might change this
            res_stations->stores[i].result = res_stations->stores[i].Vk;

            // push the finished instruction to the finished instructions vector
            finished_instructions.push_back(*res_stations->stores[i].inst);

            // flush the instruction
            res_stations->stores[i].flush();
        }
    }

    // call & ret write back
    for (int i = 0; i < res_stations->hardware.call_ret; i++)
    {
        // check if the reservation station is ready
        if (res_stations->call_ret[i].wb_ready())
        {
            // check if call or ret; if call calculate the new pc if ret return to the value stored in r1 in the register file
            if (res_stations->call_ret[i].inst->OP == instruction::type::CALL)
            {
                // update the cycles
                res_stations->call_ret[i].inst->write_back_cycle = current_cycle;

                // update the rd
                res_stations->call_ret[i].result = PC + 1;

                // push the finished instruction to the finished instructions vector
                finished_instructions.push_back(*res_stations->call_ret[i].inst);

                // flush the instruction
                res_stations->call_ret[i].flush();

                // update the PC by changing it t the immidiate value
                PC = res_stations->call_ret[i].imm;
            }
            else if (res_stations->call_ret[i].inst->OP == instruction::type::RET)
            {
                // update the cycles
                res_stations->call_ret[i].inst->write_back_cycle = current_cycle;

                // update the rd
                res_stations->call_ret[i].inst->rd = regfile.register_stat[res_stations->call_ret[i].inst->rs1].value;

                // push the finished instruction to the finished instructions vector
                finished_instructions.push_back(*res_stations->call_ret[i].inst);

                // flush the instruction
                res_stations->call_ret[i].flush();

                // update the PC by returning to the value stored in r1 in the register file
                PC = regfile.register_stat[res_stations->call_ret[i].inst->rs1].value;
            }
        }
    }
}

void update()
{
    // loop over every station and update it:
    /**
     * first do one loop to find the reservations that reached a wb stage
     * if the isntruction write back to the regfile, reserve the cdb
     * else, write to the memory or don't write.
     *
     * broadcast the update from the cdb to the stations and the registerstat
     *
     * then do another loop to update the reservation stations that are not in the wb stage.
     *
     *
     *
     */

    // first loop
}

void run()
{
    // main logic

    // TODO: read data into instructions vector here

    while (!program_finished())
    {

        current_cycle++;
        instruction inst = instructions[PC];

        bool issued = issue(inst);

        update();

        PC++;
    }
}

int main()
{

    run();
    return 0;
}
