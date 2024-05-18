#include "includes.h"
#include "structs.h"
#include "parser.h"
#include <unordered_map>
#include <vector>

////////////////////////////////////////////////////////

int total_clock_cycle_count; // total number of clock cycles

struct
{
    string station_name;
    bool is_empty = true;
    int value;
} cdb;

unsigned int data_memory[128] = {1, 2, 3};

bool reservation_station::execute_ready()
{
    if (!this->busy)
        return false;

    // if FP operation--> return true if Qj and Qk are not empty
    if (this->OP == reservation_station::TYPES::ADDER ||
        this->OP == reservation_station::TYPES::MUL ||
        this->OP == reservation_station::TYPES::NAND ||
        this->OP == reservation_station::TYPES::BEQ

    )
        if (Qj == "" && Qk == "")
            return true;

    if (this->OP == reservation_station::TYPES::LOAD)
        if (Qj == "")
            return true;

    if (this->OP == reservation_station::TYPES::STORE)
        if (Qj == "")
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

    if (this->OP == reservation_station::TYPES::BEQ)
        return this->Qj == "" && this->Qk == "";

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
    vector<pair<int, bool>> register_station;

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
    for (int i = 0; i < res_stations->hardware.adders; i++)
        if (res_stations->adders[i].busy)
            return false;
    for (int i = 0; i < res_stations->hardware.multipliers; i++)
        if (res_stations->multipliers[i].busy)
            return false;
    for (int i = 0; i < res_stations->hardware.nanders; i++)
        if (res_stations->nanders[i].busy)
            return false;
    for (int i = 0; i < res_stations->hardware.loaders; i++)
        if (res_stations->loader[i].busy)
            return false;
    for (int i = 0; i < res_stations->hardware.stores; i++)
        if (res_stations->stores[i].busy)
            return false;
    for (int i = 0; i < res_stations->hardware.branches; i++)
        if (res_stations->branches[i].busy)
            return false;

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

                regfile.register_stat[inst.rd].Qi = &res_stations->adders[i];

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

                regfile.register_stat[inst.rd].Qi = &res_stations->multipliers[i];

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

                regfile.register_stat[inst.rd].Qi = &res_stations->nanders[i];

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

                regfile.register_stat[inst.rd].Qi = &res_stations->loader[i];

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

        else
        {

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
        else
        {

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

        else
        {

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
    // branch excecution
    for (auto &branch : res_stations->branches)
    {
        if (branch.executed)
            continue;

        if (!branch.started_execution && branch.execute_ready())
        {
            branch.started_execution = true;
            branch.inst->execution_start_cycle = current_cycle;
            branch.inst->cycles_left = branch.cycles;
        }
        else if (branch.started_execution)
        {
            if (branch.inst->cycles_left == 0)
            {
                branch.inst->excecution_end_cycle = current_cycle;
                branch.executed = true;
                // Check branch condition
                if (branch.Vj == branch.Vk)
                {
                    // Branch taken
                    PC = PC + 1 + branch.imm;
                }
                else
                {
                    // Branch not taken
                    PC++;
                }
            }
            else
            {
                branch.inst->cycles_left--;
            }
        }
    }
}

void reserveCDB(const reservation_station &rs)
{
    cdb.is_empty = false;
    cdb.value = rs.result;
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
            // add the clocks to the total clock cycles
            total_clock_cycle_count += res_stations->adders[i].cycles;
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
            // add the clocks to the total clock cycles
            total_clock_cycle_count += res_stations->multipliers[i].cycles;
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

            // add the clocks to the total clock cycles
            total_clock_cycle_count += res_stations->nanders[i].cycles;
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
            // add the clocks to the total clock cycles
            total_clock_cycle_count += res_stations->branches[i].cycles;
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

            res_stations->loader[i].result = data_memory[res_stations->loader[i].A];

            // push the finished instruction to the finished instructions vector
            finished_instructions.push_back(*res_stations->loader[i].inst);
            reserveCDB(res_stations->loader[i]);

            // flush the instruction
            res_stations->loader[i].flush();
            // add the clocks to the total clock cycles
            total_clock_cycle_count += res_stations->loader[i].cycles;
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

            // update the rd
            data_memory[res_stations->stores[i].A] = res_stations->stores[i].Vk;

            // push the finished instruction to the finished instructions vector
            finished_instructions.push_back(*res_stations->stores[i].inst);

            // flush the instruction
            res_stations->stores[i].flush();
            // add the clocks to the total clock cycles
            total_clock_cycle_count += res_stations->stores[i].cycles;
        }
    }

    // call & ret write back
    for (int i = 0; i < res_stations->hardware.call_ret; i++)
    {
        // check if the reservation station is ready
        if (res_stations->call_ret[i].wb_ready())
        {
            // check if call or ret; if call calculate the new inst_number if ret return to the value stored in r1 in the register file
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
                // add the clocks to the total clock cycles
                total_clock_cycle_count += res_stations->call_ret[i].cycles;
            }
            else if (res_stations->call_ret[i].inst->OP == instruction::type::RET)
            {
                // update the cycles
                res_stations->call_ret[i].inst->write_back_cycle = current_cycle;

                // update the rd
                // res_stations->call_ret[i].inst->rd = registers[1].first;

                // push the finished instruction to the finished instructions vector
                finished_instructions.push_back(*res_stations->call_ret[i].inst);

                // flush the instruction
                res_stations->call_ret[i].flush();
                // add the clocks to the total clock cycles
                total_clock_cycle_count += res_stations->call_ret[i].cycles;
            }
        }
    }

    for (auto &branch : res_stations->branches)
    {
        if (branch.wb_ready())
        {
            branch.inst->write_back_cycle = current_cycle;
            bool taken = (branch.Vj == branch.Vk);
            if (taken)
            {
                // Branch was predicted not taken, but it is taken
                branch_misprediction_count++;
                PC = branch.inst->execution_start_cycle + 1 + branch.imm;
            }
            else
            {
                // Branch was correctly predicted not taken
                PC++;
            }

            finished_instructions.push_back(*branch.inst);
            total_branch_count++;
            branch.flush();
            total_clock_cycle_count += branch.cycles;
        }
    }
}

void broadcast_cdb_to_stations()
{
    for (int i = 0; i < res_stations->hardware.adders; i++)
    {
        if (res_stations->adders[i].Qj != "" && res_stations->adders[i].Qj == cdb.station_name)
        {
            res_stations->adders[i].Vj = cdb.value;
            res_stations->adders[i].Qj = "";
        }
        if (res_stations->adders[i].Qk != "" && res_stations->adders[i].Qk == cdb.station_name)
        {
            res_stations->adders[i].Vk = cdb.value;
            res_stations->adders[i].Qk = "";
        }
    }

    for (int i = 0; i < res_stations->hardware.multipliers; i++)
    {
        if (res_stations->multipliers[i].Qj != "" && res_stations->multipliers[i].Qj == cdb.station_name)
        {
            res_stations->multipliers[i].Vj = cdb.value;
            res_stations->multipliers[i].Qj = "";
        }
        if (res_stations->multipliers[i].Qk != "" && res_stations->multipliers[i].Qk == cdb.station_name)
        {
            res_stations->multipliers[i].Vk = cdb.value;
            res_stations->multipliers[i].Qk = "";
        }
    }

    for (int i = 0; i < res_stations->hardware.nanders; i++)
    {
        if (res_stations->nanders[i].Qj != "" && res_stations->nanders[i].Qj == cdb.station_name)
        {
            res_stations->nanders[i].Vj = cdb.value;
            res_stations->nanders[i].Qj = "";
        }
        if (res_stations->nanders[i].Qk != "" && res_stations->nanders[i].Qk == cdb.station_name)
        {
            res_stations->nanders[i].Vk = cdb.value;
            res_stations->nanders[i].Qk = "";
        }
    }

    for (int i = 0; i < res_stations->hardware.loaders; i++)
    {
        if (res_stations->loader[i].Qj != "" && res_stations->loader[i].Qj == cdb.station_name)
        {
            res_stations->loader[i].Vj = cdb.value;
            res_stations->loader[i].Qj = "";
        }
    }

    for (int i = 0; i < res_stations->hardware.stores; i++)
    {
        if (res_stations->stores[i].Qj != "" && res_stations->stores[i].Qj == cdb.station_name)
        {
            res_stations->stores[i].Vj = cdb.value;
            res_stations->stores[i].Qj = "";
        }
        if (res_stations->stores[i].Qk != "" && res_stations->stores[i].Qk == cdb.station_name)
        {
            res_stations->stores[i].Vk = cdb.value;
            res_stations->stores[i].Qk = "";
        }
    }

    for (int i = 0; i < res_stations->hardware.call_ret; i++)
    {
        if (res_stations->call_ret[i].Qj != "" && res_stations->call_ret[i].Qj == cdb.station_name)
        {
            res_stations->call_ret[i].Vj = cdb.value;
            res_stations->call_ret[i].Qj = "";
        }
        if (res_stations->call_ret[i].Qk != "" && res_stations->call_ret[i].Qk == cdb.station_name)
        {
            res_stations->call_ret[i].Vk = cdb.value;
            res_stations->call_ret[i].Qk = "";
        }
    }

    for (int i = 0; i < res_stations->hardware.branches; i++)
    {
        if (res_stations->adders[i].Qj != "" && res_stations->branches[i].Qj == cdb.station_name)
        {
            res_stations->branches[i].Vj = cdb.value;
            res_stations->branches[i].Qj = "";
        }
        if (res_stations->branches[i].Qk == cdb.station_name)
        {
            res_stations->branches[i].Vk = cdb.value;
            res_stations->branches[i].Qk = "";
        }
    }
}

void update_regfile()
{
    for (int i = 1; i < regfile.register_stat.size(); i++)
        if (regfile.register_stat[i].Qi != nullptr && regfile.register_stat[i].Qi->name == cdb.station_name)
        {
            regfile.register_stat[i].Qi = nullptr;
            regfile.register_stat[i].value = cdb.value;
            break;
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

    write_back();
    // update reservation stations
    broadcast_cdb_to_stations();

    // update the register file
    update_regfile();

    cdb.is_empty = true;
    cdb.station_name = "";
    cdb.value = 0;

    execute();
}
unordered_map<string, int> labelMap;

// calculate the IPC
void calculateIPC()
{
    double IPC = (double)finished_instructions.size() / (double)total_clock_cycle_count;
    cout << "IPC: " << IPC << endl;
}

void printCycles()
{
    std::ofstream myfile; // Specify the namespace for ofstream
    myfile.open("cycles.csv");
    myfile << "Instruction,Issue,Execution Start,Execution End,Write Back\n";
    for (int i = 0; i < finished_instructions.size(); i++)
    {
        myfile  << std::to_string(finished_instructions[i].issue_cycle) << "," << std::to_string(finished_instructions[i].execution_start_cycle) << "," << std::to_string(finished_instructions[i].excecution_end_cycle) << "," << std::to_string(finished_instructions[i].write_back_cycle) << "\n";
    }
    myfile.close();
}

int main()
{

    // TODO: read data into instructions vector here
    // readInstructions("test.txt", instructions, labelMap); // main logic

    // instructions.push_back(instruction(instruction::type::ADDI, 0, 0, 1, 2)); // ADDI R1, R0, 2
    // // instructions.push_back(instruction(instruction::type::ADD, 1, 0, 2));     // ADD R2, R1, R0
    // // instructions.push_back(instruction(instruction::type::NAND, 1, 2, 3));     // ADD R2, R1, R0
    // instructions.push_back(instruction(instruction::type::STORE, 0, 1, 0, 0)); // STORE R1, 0(R0)
    // instructions.push_back(instruction(instruction::type::ADD, 0, 0, 0));     // ADD R2, R1, R0
    // instructions.push_back(instruction(instruction::type::ADD, 0, 0, 0));     // ADD R2, R1, R0
    // instructions.push_back(instruction(instruction::type::ADD, 0, 0, 0));     // ADD R2, R1, R0
    // instructions.push_back(instruction(instruction::type::ADD, 0, 0, 0));     // ADD R2, R1, R0
    // instructions.push_back(instruction(instruction::type::LOAD, 0, 0, 3, 0)); // LOAD R2, 0(R0)
    // instructions.push_back(instruction(instruction::type::STORE, 0, 0, 0, 0)); // STORE R0, 0(R0)
    // instructions.push_back(instruction(instruction::type::LOAD, 0, 0, 1, 0)); // LOAD R1, 0(R0)
    // instructions.push_back(instruction(instruction::type::BEQ, 1, 2, 0, 2)); // BEQ R1, R2, 2

    readInstructions("test.txt", instructions); // main logic

    hardware_input hw;
    res_stations = new reservation_stations(hw);
    bool finished = false;
    do
    {

        current_cycle++;

        update();
        if (PC < instructions.size())
        {
            // instruction inst = instructions[PC];
            bool issued = issue(instructions[PC]);
            if (issued)
                PC++;
        }
        finished = program_finished();
    } while (!finished);

    for (int i = 0; i < regfile.register_stat.size(); i++)
    {
        cout << "Register " << i << " : " << regfile.register_stat[i].value << endl;
    }

    cout << "MEMORY 0" << data_memory[0] << endl;

    calculateIPC();
    printCycles();
    cout<<"Branch misprediction rate: "<<branch_misprediction_count<<endl; 

}