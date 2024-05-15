/////////////////////////////general notes/////////////////////////////

// cntl + f note + nadia to find my notes left for you
// do the same for any notes you leave for me
// if you need a variable to be changed outside the function you are writing, make sure to write the name of the function and include it in the checklist

//- i made PC a global variable and removed it from the instruction class // i moved them to global.h and global.cpp
// call and ret are handeled as an addition in the execution stage

// we might not need the buffers?
/////////////////////////////checklist/////////////////////////////
/*
- do the clock
- create the load and store buffers
- we may have the cdb as a queue in the run funcition
- we will be doing the actual calculation of the values in the wb stage
- the branching is also not handeled in the execution stage, it is treated as a normal add/sub operation
- for now i am going to put the branching function in the reservation_station class, we might change that later
- i added a after_branch varaible in the instruction class to know if the instruction comes after a branch or not --> set during ussuing (append the instruction to the after_branch vector)
- check if the afterbranch vector is made of instrucitons or reservation stations
- make PC increment in the run function (check the icrement if it should be +1 or no in branch )
- no branch?
- for the load and store make sure if the address should be calculated in the wb or in the buffer (we might not need a buffer)
- make the ready function in the reservation station class
- check the call ret wb functionality

*/
/////////////////////////////code/////////////////////////////

#pragma once
#include "includes.h";

/////////////////////////////////////////other classes/////////////////////////////////////////
// first step
class instruction
{

public:
    // instruction components

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

    // after branch
    bool after_branch = false;

    // intruction type
    enum instruction_type
    {
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

    // constructor
    instruction(string operation = "", int rs1 = 0, int rs2 = 0, int rd = 0, int imm = 0, short cycles = 0, bool after_branch) : rs1(rs1), rs2(rs2), rd(rd), imm(imm), cycle_count_per_instruction(cycles), after_branch(after_branch)
    {
        issue_cycle = INT_MIN;
        execution_start_cycle = INT_MIN;
        excecution_end_cycle = INT_MIN;
        write_back_cycle = INT_MIN;
        cycles_left = cycles;
    }
    // destructor
    ~instruction() {}
};

class reservation_station
{

public:
    // functions to be inherited by other classes
    bool ready();
    void execute();
    void write_back();
    void flush();
    void branch();
    void no_branch();
    void call_ret();

    // components of reservation station
    string OP;
    string name;
    bool busy;
    string Qj, Qk;
    int Vj, Vk;
    int A;
    int cycles;
    instruction *inst;

    // number of reservation stations
    int size;

    // contructors
    reservation_station(string OP = "",
                        string name = "",
                        bool busy = false,
                        string Qj = "",
                        string Qk = "",
                        int A = 0,
                        int Vj = -1,
                        int Vk = -1,
                        int size = 0,
                        int cycles,
                        instruction *inst) : OP(OP), name(name), busy(busy), Qj(Qj), Qk(Qk), A(A), Vj(Vj), Vk(Vk), size(size), cycles(cycles), inst(inst) {}

    // destructor
    ~reservation_station()
    {
        inst = nullptr;
    }
};

class Registers
{
public:
    // register status array
    vector<pair<int, bool>> register_station;

    // constructor
    Registers(int size)
    {
        register_station.resize(size, make_pair(0, false));
    }

    // destructor
    ~Registers()
    {
        register_station.clear();
    }

    // returns the status of the register
    bool is_busy(int reg)
    {
        if (reg < 0 || reg >= register_station.size())
        {
            return false;
        }
        return register_station[reg].second;
    }

    // set the register value to false once the value is written to the register
    void written_to(int reg)
    {
        if (reg < 0 || reg >= register_station.size())
        {
            return;
        }
        register_station[reg].second = false;
    }
};

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
    int laod_cycles = 6;
    int store_cycles = 6;
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
        cout << "Enter the number of cycles for load: ";
        cin >> laod_cycles;
        cout << "Enter the number of cycles for store: ";
        cin >> store_cycles;
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

/////////////////////////////////////////Reservation stations/////////////////////////////////////////

struct reservation_stations
{

    vector<reservation_station> adders;
    vector<reservation_station> multipliers;
    vector<reservation_station> loader;
    vector<reservation_station> stores;
    vector<reservation_station> nanders;

    hardware_input hardware;

    // initialize the reservation stations
    reservation_stations(hardware_input hardware) : hardware(hardware)
    {
        for (int i = 0; i < hardware.adders; i++)
        {
            adders.push_back(reservation_station("ADD", "adder" + to_string(i), false, "", "", 0, -1, -1, hardware.adders, hardware.add_cycles));
        }
        for (int i = 0; i < hardware.multipliers; i++)
        {
            multipliers.push_back(reservation_station("MUL", "multiplier" + to_string(i), false, "", "", 0, -1, -1, hardware.multipliers, hardware.mul_cycles));
        }
        for (int i = 0; i < hardware.loaders; i++)
        {
            loader.push_back(reservation_station("LOAD", "loader" + to_string(i), false, "", "", 0, -1, -1, hardware.loaders, hardware.laod_cycles));
        }
        for (int i = 0; i < hardware.stores; i++)
        {
            stores.push_back(reservation_station("STORE", "store" + to_string(i), false, "", "", 0, -1, -1, hardware.stores, hardware.store_cycles));
        }
        for (int i = 0; i < hardware.nanders; i++)
        {
            nanders.push_back(reservation_station("NAND", "nander" + to_string(i), false, "", "", 0, -1, -1, hardware.nanders, hardware.nand_cycles));
        }
    }

    // check if the reservation station is full
    bool full(string OP)
    {
        if (OP == "ADD")
        {
            for (int i = 0; i < hardware.adders; i++)
            {
                if (!adders[i].busy)
                {
                    return false;
                }
            }
            return true;
        }
        else if (OP == "MUL")
        {
            for (int i = 0; i < hardware.multipliers; i++)
            {
                if (!multipliers[i].busy)
                {
                    return false;
                }
            }
            return true;
        }
        else if (OP == "LOAD")
        {
            for (int i = 0; i < hardware.loaders; i++)
            {
                if (!loader[i].busy)
                {
                    return false;
                }
            }
            return true;
        }
        else if (OP == "STORE")
        {
            for (int i = 0; i < hardware.stores; i++)
            {
                if (!stores[i].busy)
                {
                    return false;
                }
            }
            return true;
        }
        else if (OP == "NAND")
        {
            for (int i = 0; i < hardware.nanders; i++)
            {
                if (!nanders[i].busy)
                {
                    return false;
                }
            }
            return true;
        }
    }

    // check if the reservation station is empty ==> program is done
    bool program_finished(vector<instruction> instructions)
    {
        for (int i = 0; i < instructions.size(); i++)
        {
            if (instructions[i].write_back_cycle == INT_MIN)
            {
                return false;
            }
        }
        return true;
    }

    // destructors
    ~reservation_stations()
    {
        adders.clear();
        multipliers.clear();
        loader.clear();
        stores.clear();
        nanders.clear();
    }
};

/////////////////////////////////////////load store buffers/////////////////////////////////////////

// Buffers
struct LoadStoreBuffer
{
    string name;
    string address;
    bool isBusy;
    instruction *instr;
    string fu;

    LoadStoreBuffer() : isBusy(false), instr(nullptr) {} // Constructor
};

struct LoadStoreBufferType
{
    static const string LOAD;
    static const string STORE;
};

const string LoadStoreBufferType::LOAD = "LOAD";
const string LoadStoreBufferType::STORE = "STORE";

class buffers
{
private:
    LoadStoreBuffer *Load_ReservationStations;
    LoadStoreBuffer *Store_ReservationStations;
    int totalLoad_ReservationStations;
    int totalStore_ReservationStations;

public:
    buffers(int loadStations, int storeStations)
        : totalLoad_ReservationStations(loadStations),
          totalStore_ReservationStations(storeStations)
    {
        Load_ReservationStations = new LoadStoreBuffer[totalLoad_ReservationStations];
        for (int i = 0; i < totalLoad_ReservationStations; i++)
        {
            Load_ReservationStations[i].name = "LOAD" + to_string(i);
        }

        Store_ReservationStations = new LoadStoreBuffer[totalStore_ReservationStations];
        for (int i = 0; i < totalStore_ReservationStations; i++)
        {
            Store_ReservationStations[i].name = "STORE" + to_string(i);
        }
    }

    ~buffers()
    {
        delete[] Load_ReservationStations;
        delete[] Store_ReservationStations;
    }
};

/////////////////////////////////////////Tomasulo/////////////////////////////////////////

/////////////////////////////////////////execution/////////////////////////////////////////

/*Note nadia*/
// i am going to assume that in issuing i am going to have the the instruction be in the reservatoin satation
// also fill in the gj and the other values needed in the reservation sation that are retrieved from the instruction
// you should also update the instuction issue status date to be current cycle
// make sure in the issuing to send the jal and beq to the add reservation station and have a variable or sth to know that it is a branch

void reservation_station::execute()
{

    // add & addi excecution
    for (int i = 0; i < res_stations->hardware.adders; i++)
    {

        // case wher the execution is not started yet
        if (res_stations->adders[i].inst->execution_start_cycle == INT_MIN)
        {
            // check if the reservation station is ready
            if (res_stations->adders[i].ready())
            {
                // update the reservation station
                res_stations->adders[i].inst->execution_start_cycle = current_cycle;
                res_stations->adders[i].inst->cycles_left = res_stations->adders[i].cycles;
            }
        }

        /*note osswa check if it should be written back at 1 or 0
         */

        // mark the end of the execution
        if (res_stations->adders[i].inst->cycles_left == 0)
        {
            res_stations->adders[i].inst->excecution_end_cycle = current_cycle;
        }

        // decrement the cycles left
        res_stations->adders[i].inst->cycles_left--;
    }

    // mul excecution
    for (int i = 0; i < res_stations->hardware.multipliers; i++)
    {
        // case wher the execution is not started yet
        if (res_stations->multipliers[i].inst->execution_start_cycle == INT_MIN)
        {
            // check if the reservation station is ready
            if (res_stations->multipliers[i].ready())
            {
                // update the reservation station
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
        }

        // decrement the cycles left
        res_stations->multipliers[i].inst->cycles_left--;
    }

    // nand excecution
    for (int i = 0; i < res_stations->hardware.nanders; i++)
    {
        // case wher the execution is not started yet
        if (res_stations->nanders[i].inst->execution_start_cycle == INT_MIN)
        {
            // check if the reservation station is ready
            if (res_stations->nanders[i].ready())
            {
                // update the reservation station
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
        }

        // decrement the cycles left
        res_stations->nanders[i].inst->cycles_left--;
    }

    // load excecution
    for (int i = 0; i < res_stations->hardware.loaders; i++)
    {
        // case wher the execution is not started yet
        if (res_stations->loader[i].inst->execution_start_cycle == INT_MIN)
        {
            // check if the reservation station is ready
            if (res_stations->loader[i].ready())
            {
                // update the reservation station
                res_stations->loader[i].inst->execution_start_cycle = current_cycle;
                res_stations->loader[i].inst->cycles_left = res_stations->loader[i].cycles;
            }
        }

        /*note osswa check if it should be written back at 1 or 0
         */
        // mark the end of the execution
        if (res_stations->loader[i].inst->cycles_left == 0)
        {
            res_stations->loader[i].inst->excecution_end_cycle = current_cycle;
        }

        // decrement the cycles left
        res_stations->loader[i].inst->cycles_left--;
    }

    // store excecution
    for (int i = 0; i < res_stations->hardware.stores; i++)
    {
        // case wher the execution is not started yet
        if (res_stations->stores[i].inst->execution_start_cycle == INT_MIN)
        {
            // check if the reservation station is ready
            if (res_stations->stores[i].ready())
            {
                // update the reservation station
                res_stations->stores[i].inst->execution_start_cycle = current_cycle;
                res_stations->stores[i].inst->cycles_left = res_stations->stores[i].cycles;
            }
        }

        /*note osswa check if it should be written back at 1 or 0
         */
        // mark the end of the execution
        if (res_stations->stores[i].inst->cycles_left == 0)
        {
            res_stations->stores[i].inst->excecution_end_cycle = current_cycle;
        }

        // decrement the cycles left
        res_stations->stores[i].inst->cycles_left--;
    }
}

/////////////////////////////////////////flushing/////////////////////////////////////////

void reservation_station::flush()
{

    OP = "";
    name = "";
    busy = false;
    Qj = "";
    Qk = "";
    Vj = -1;
    Vk = -1;
    A = 0;
    inst = nullptr;
}

/////////////////////////////////////////write back/////////////////////////////////////////

void reservation_station::write_back()
{

    // add & addi write back
    for (int i = 0; i < res_stations->hardware.adders; i++)
    {
        // check if the reservation station is ready
        if (res_stations->adders[i].ready())
        {
            // update the cycles
            res_stations->adders[i].inst->write_back_cycle = current_cycle;

            // check if addi or add
            if (res_stations->adders[i].inst->OP == "ADD")
            {
                res_stations->adders[i].inst->rd = res_stations->adders[i].inst->rs1 + res_stations->adders[i].inst->rs2;
            }
            else if (res_stations->adders[i].inst->OP == "ADDI")
            {
                res_stations->adders[i].inst->rd = res_stations->adders[i].inst->rs1 + res_stations->adders[i].inst->imm;
            }

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
        if (res_stations->multipliers[i].ready())
        {
            // update the cycles
            res_stations->multipliers[i].inst->write_back_cycle = current_cycle;

            // update the rd
            res_stations->multipliers[i].inst->rd = res_stations->multipliers[i].inst->rs1 * res_stations->multipliers[i].inst->rs2;

            // push the finished instruction to the finished instructions vector
            finished_instructions.push_back(*res_stations->multipliers[i].inst);

            // flush the instruction
            res_stations->multipliers[i].flush();
        }
    }

    // nand write back
    for (int i = 0; i < res_stations->hardware.nanders; i++)
    {
        // check if the reservation station is ready
        if (res_stations->nanders[i].ready())
        {
            // update the cycles
            res_stations->nanders[i].inst->write_back_cycle = current_cycle;

            // update the rd
            res_stations->nanders[i].inst->rd = ~(res_stations->nanders[i].inst->rs1 & res_stations->nanders[i].inst->rs2);

            // push the finished instruction to the finished instructions vector
            finished_instructions.push_back(*res_stations->nanders[i].inst);

            // flush the instruction
            res_stations->nanders[i].flush();
        }
    }

    // branch write back

    for (int i = 0; i < res_stations->hardware.branches; i++)
    {
        // check if the reservation station is ready
        if (res_stations->branches[i].ready())
        {
            // update the cycles
            res_stations->branches[i].inst->write_back_cycle = current_cycle;

            // update the rd only if the condition is true & call the branch function
            if (res_stations->branches[i].inst->rs1 == res_stations->branches[i].inst->rs2)
            {
                branch();
                branch_misprediction_count++; // increment the branch misprediction count since we have a no branch predictor
            }

            // push the finished instruction to the finished instructions vector
            finished_instructions.push_back(*res_stations->branches[i].inst);

            total_branch_count++;

            // flush the instruction
            res_stations->branches[i].flush();
        }
    }

    // load write back
    for (int i = 0; i < res_stations->hardware.loaders; i++)
    {
        // check if the reservation station is ready
        if (res_stations->loader[i].ready())
        {
            // update the cycles
            res_stations->loader[i].inst->write_back_cycle = current_cycle;

            // note osswa : we might change this
            res_stations->loader[i].inst->rd = res_stations->loader[i].inst->rs1 + res_stations->loader[i].inst->imm;

            // push the finished instruction to the finished instructions vector
            finished_instructions.push_back(*res_stations->loader[i].inst);

            // flush the instruction
            res_stations->loader[i].flush();
        }
    }

    // store write back
    for (int i = 0; i < res_stations->hardware.stores; i++)
    {
        // check if the reservation station is ready
        if (res_stations->stores[i].ready())
        {
            // update the cycles
            res_stations->stores[i].inst->write_back_cycle = current_cycle;

            // note osswa : we might change this
            res_stations->stores[i].inst->rd = res_stations->stores[i].inst->rs1 + res_stations->stores[i].inst->imm;

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
        if (res_stations->call_ret[i].ready())
        {
            // check if call or ret; if call calculate the new pc if ret return to the value stored in r1 in the register file
            if (res_stations->call_ret[i].inst->OP == "CALL")
            {
                // update the cycles
                res_stations->call_ret[i].inst->write_back_cycle = current_cycle;

                // update the rd
                res_stations->call_ret[i].inst->rd = PC + 1;

                // push the finished instruction to the finished instructions vector
                finished_instructions.push_back(*res_stations->call_ret[i].inst);

                // flush the instruction
                res_stations->call_ret[i].flush();
            }
            else if (res_stations->call_ret[i].inst->OP == "RET")
            {
                // update the cycles
                res_stations->call_ret[i].inst->write_back_cycle = current_cycle;

                // update the rd
                res_stations->call_ret[i].inst->rd = registers[1].first;

                // push the finished instruction to the finished instructions vector
                finished_instructions.push_back(*res_stations->call_ret[i].inst);

                // flush the instruction
                res_stations->call_ret[i].flush();
            }
        }
    }
}

/////////////////////////////////////////branching/////////////////////////////////////////

void reservation_station::branch()
{

    for (int i = 0; i < after_branch_record.size(); i++)
    {
        if (after_branch_record[i].inst->after_branch)
        {
            // flush the reservation station
            after_branch_record[i].flush();
        }
    }
    // empty the after branch record
    after_branch_record.clear();

    // set PC to the new value
    PC = PC + A + 1;
}

/////////////////////////////////////////no branch/////////////////////////////////////////
// we might beed this

/////////////////////////////////////////ready function/////////////////////////////////////////

// this function checks if the instructions is ready to be pushed back of not
/*note osswa : might need some tweaking for the laod and store write */

bool reservation_station::ready()
{
    // check if the reservation station is busy
    if (busy)
    {
        return false;
    }

    // check if the reservation station is ready
    if (Qj == "" && Qk == "")
    {
        return true;
    }

    return false;
}
