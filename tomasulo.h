//#include "reservation_stations.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <queue>
#include <algorithm>
using namespace std;


struct Instruction {
    int PC_idx;
    int exec_rem_cycles;
    int issued_cycle;
    int exec_begin_cycle;
    int exec_end_cycle;
    int wb_cycle;
    string op;
    int rs1;
    int rs2;
    int rd;
    int imm;

    Instruction(int PC = 0, string operation = "", int rs1 = 0, int rs2 = 0, int rd = 0, int imm = 0, int cycles = 0)
    {
        PC_idx = PC; this->op = operation; this->rs1 = rs1; this->rs2 = rs2; this->rd = rd; this->imm = imm; exec_rem_cycles = cycles;
    }

    void print() { cout << "PC: " << PC_idx << " OP: " << op << " RS1: " << rs1 << " RS2: " << rs2 << " RD: " << rd; }
};

class rs_station {
public:
    bool load_store_hazard;
    string name;
    bool busy;          // The if this unit is busy
    Instruction inst;  // the instruction
    int Vj;             // Value in the register of the first operand
    string Qj;
    bool not_allowed;
    int exec_time;
    bool finished_exec;
    rs_station() { busy = false; inst = Instruction(); Vj = 0; Qj = ""; not_allowed = false; finished_exec = false; load_store_hazard = false; }
    virtual bool can_exec() { return false; }    // Checks if we can start executing
    virtual void exec() {}       // Exectuting the function
    virtual void wb() {}          // Writing back
    virtual void flush() {}

};


////////////////////////////////////////////////// Classes ///////////////////////////////////////////////////////


class abs_station : public rs_station {
public:
    void issue();
    bool can_exec() { return (Qj == "" && !not_allowed); } 
    void exec();
    void wb();
    void flush();
};

class add_station : public rs_station {
public:
    string op;
    int A; // immediate
    int Vk;
    string Qk;
    bool can_exec() { return (Qj == "" && Qk == "" && !not_allowed); }
    void issue();
    void exec();
    void wb();
    void flush();
};

class beq_station : public rs_station {
public:
    int A; // immediate
    int Vk;             // Value in the rs2
    string Qk;
    bool can_exec() { return (Qj == "" & Qk == ""); }
    void issue();
    void exec();
    void wb();
    void flush();
    void branch();
    void dont_branch();
};

class div_station : public rs_station {
public:
    int Vk;
    string Qk;
    bool can_exec() { return (Qj == "" && Qk == "" && !not_allowed); }
    void issue();
    void exec();
    void wb();
    void flush();
};

class jal_station : public rs_station {
public:
    int A; // immediate
    string op;
    bool can_exec() { return (((Qj == "" && inst.op=="JALR") || inst.op == "JAL") && !not_allowed); };
    void issue();
    void wb();
    void exec();
    void flush();
};

class store_station : public rs_station {
public:
    int A; // immediate
    int Vk;             // Value in the rs2
    string Qk;
    int addr_calc;
    int write_mem;
    int rem_write_mem;
    bool addr_calculated;
    rs_station* load_store_waiting_for;

    void issue();
    bool can_exec() { return (Qj == "" && !not_allowed); } 
    void exec();
    bool can_wb() { return Qk == ""; }
    void wb();
    void check_load_store();
    void flush();
};


class load_station : public rs_station {
public:
    int A; // immediate
    int addr_calc;
    int read_mem;
    bool addr_calculated;
    store_station* store_waiting_for;
    void issue();
    bool can_exec() { return (Qj == "" && !not_allowed && (!addr_calculated || !load_store_hazard)); } 
    void exec();
    void wb();
    void flush();
    void check_store();
};

class neg_station : public rs_station {
public:
    void issue();
    bool can_exec() { return (Qj == "" && !not_allowed); } //WE STILL NEED TO CHECK IF IT IS THE HEAD OF THE LOAD-STORE QUEUE
    void exec();
    void wb();
    void flush();
};

/// <summary>
/// ///////////////////////////////// RS ///////////////////////////////////////////////////////////////
/// </summary>

class reservation_stations {
public:
    vector<load_station> rs_lw;
    vector<store_station> rs_sw;
    vector<beq_station> rs_beq;
    vector<jal_station> rs_jal;
    vector<add_station> rs_add;
    vector<neg_station> rs_neg;
    vector<abs_station> rs_abs;
    vector<div_station> rs_div;

    void init(int lw, int lw_addr_calc, int lw_read_mem, int sw, int sw_addr_calc, int sw_read_mem,
        int beq, int beq_exec, int jal, int jal_exec, int add, int add_exec, int neg, int neg_exec,
        int abs, int abs_exec, int div, int div_exec);
    void issue_load();
    void issue_store();
    void issue_beq();
    void issue_jal();
    void issue_add();
    void issue_neg();
    void issue_abs();
    void issue_div();
    void update_rs(string r, int result);
    void wb_store();
    void wb_beq();
    void execute();
    bool done(); // Returns true if none of the stations is busy
};

vector<Instruction> inst_mem;
vector<string> RegisterStat(8, "");
vector<string> RegisterStat_buffer(8, "");
vector<int> Regs(8);
reservation_stations rs;
vector<int> Mem(65536, 0);
int clk;
bool jump_issued;
bool branch_issued;
vector<rs_station*> load_store_queue;
int PC;
vector<rs_station*> cdb;
vector<Instruction> finished_inst;
vector<rs_station*> after_branch;
int total_branches_num;
int branch_mispredicted;

////////////////////////// Read Memory //////////////////////////////////
void read_memory(string memory_file)
{
    std::ifstream indata;
    indata.open(memory_file);
    if (indata.fail())
    {
        std::cerr << "Error: file could not be opened" << std::endl;
        exit(1);
    }
    std::string line;
    size_t pos = 0;
    int token;
    while(getline(indata,line))
    {
        while ((pos = line.find(":")) != string::npos) {
        token = stoi(line.substr(0, pos));
        line.erase(0, pos + 1);
        Mem[token] = stoi(line);
    }
    }

}

/// <summary>
/// /////////////////////////////// MAIN ///////////////////////////////////////////////////////////////////////////////////////
/// </summary>
vector<int> getUserinput();
void Issue();
void read_instructions(string file_name, int starting_PC, vector<int> userInput);
void run_program(int start_PC);
bool compare_issue_time(rs_station* a, rs_station* b);
bool compare_finished_inst(Instruction a, Instruction b) { return a.issued_cycle < b.issued_cycle; }
void fileMaker();

int main() {
    string file_name;
    cout << "Enter the name of the file which contains the program: ";
    cin >> file_name;


    vector<int> userInput = getUserinput();
    int start_PC = userInput[0];
    rs.init(userInput[1],userInput[2], userInput[3], userInput[4],
    userInput[5], userInput[6], userInput[7], userInput[8],userInput[9],
    userInput[10], userInput[11], userInput[12], userInput[13], userInput[14],
    userInput[15], userInput[16], userInput[17], userInput[18]);
    queue<rs_station*> load_store_queue;

    queue<Instruction> inst_queue;
    read_memory("memory.txt");
    //cout<<endl<<"memory<<\n"<<Mem[0]<<endl<<endl<<Mem[12]<<endl<<"end of memory"<<endl;
    read_instructions(file_name, start_PC, userInput); // Now we have a queue with all the instructions

    run_program(start_PC);

    cout << "The total execution time is " << clk << " cycles.\n";
    cout << "IPC = " << finished_inst.size() / (float)clk << endl;
    cout << "Branch Misprediction Rate= " << branch_mispredicted / (float)total_branches_num << endl;
    sort(finished_inst.begin(), finished_inst.end(), compare_finished_inst);
    fileMaker();

    std::cout<<"\nRegister File\n";
    for (int i=0;i<Regs.size();i++)
    {
       std::cout<<i<<" : "<<Regs[i]<<std::endl;
    }
    return 0;
}

void run_program(int start_PC) {
    //cout << "Starting to run program\n";
    PC = start_PC;
    clk = 0;
    jump_issued = false;
    branch_issued = false;
    total_branches_num = 0;
    branch_mispredicted = 0;
    while ((PC < inst_mem.size()) || !rs.done()) {
        //cout << "Cycle: " << clk << endl;
        //WB
        sort(cdb.begin(), cdb.end(), compare_issue_time);
        if (!cdb.empty()) {
            cdb[0]->wb();
            cdb.erase(cdb.begin());
        }
        rs.wb_store();
        rs.wb_beq();

        //EXEC
        rs.execute();

        //ISSUE
        if (!jump_issued && (PC < inst_mem.size()))
            Issue();
        
        /*cout << "RegisterStat: ";
        for (int i = 0; i < RegisterStat.size(); i++) {
            cout << RegisterStat[i] << " - ";
        }
        cout << endl;*/

        /*cout << "Register File: ";
        for (int i = 0; i < Regs.size(); i++) {
            cout << Regs[i] << " - ";
        }
        cout << endl;*/

        clk++;
    }

}

bool compare_issue_time(rs_station* a, rs_station* b) {
    return a->inst.issued_cycle < b->inst.issued_cycle;
}

void Issue() {
    if (inst_mem[PC].op == "LW") {
        rs.issue_load();
    }
    else if (inst_mem[PC].op == "SW") {
        rs.issue_store();
    }
    else if (inst_mem[PC].op == "BEQ") {
        if (!branch_issued) {
            rs.issue_beq();
        }
    }
    else if (inst_mem[PC].op == "JAL" || inst_mem[PC].op == "JALR") {
        rs.issue_jal();
    }
    else if (inst_mem[PC].op == "ADD" || inst_mem[PC].op == "ADDI") {
        rs.issue_add();
    }
    else if (inst_mem[PC].op == "NEG") {
        rs.issue_neg();
    }
    else if (inst_mem[PC].op == "ABS") {
        rs.issue_abs();
    }
    else if (inst_mem[PC].op == "DIV") {
        rs.issue_div();
    }


}
vector<int> getUserinput()
{
    vector<int> userInput(19,0);
    std::cout<<"Please enter the starting address of your instruction memory: ";
    cin>>userInput[0];
    std::cout<<"Please enter the number of LW stations: ";
    cin>>userInput[1];
    std::cout<<"Please enter the number of cycles to compute the LW address: ";
    cin>>userInput[2];
    std::cout<<"Please enter the number of cycles to read from the memory: ";
    cin>>userInput[3];
    std::cout<<"Please enter the number of SW stations: ";
    cin>>userInput[4];
    std::cout<<"Please enter the number of cycles to compute the SW address: ";
    cin>>userInput[5];
    std::cout<<"Please enter the number of cycles to write to the memory: ";
    cin>>userInput[6];
    std::cout<<"Please enter the number of BEQ stations: ";
    cin>>userInput[7];
    std::cout<<"Please enter the number of cycles to execute BEQ: ";
    cin>>userInput[8];
    std::cout<<"Please enter the number of JAL/JALR stations: ";
    cin>>userInput[9];
    std::cout<<"Please enter the number of cycles to execute JAL/JALR: ";
    cin>>userInput[10];
    std::cout<<"Please enter the number of ADD/ADDI stations: ";
    cin>>userInput[11];
    std::cout<<"Please enter the number of cycles to execute ADD/ADDI: ";
    cin>>userInput[12];
    std::cout<<"Please enter the number of NEG stations: ";
    cin>>userInput[13];
    std::cout<<"Please enter the number of cycles to execute NEG: ";
    cin>>userInput[14];
    std::cout<<"Please enter the number of ABS stations: ";
    cin>>userInput[15];
    std::cout<<"Please enter the number of cycles to execute ABS: ";
    cin>>userInput[16];
    std::cout<<"Please enter the number of DIV stations: ";
    cin>>userInput[17];
    std::cout<<"Please enter the number of cycles to execute DIV: ";
    cin>>userInput[18];
    cout << endl;

    return userInput;
}

void fileMaker()
{
    ofstream myfile;
    myfile.open("results.csv");

    myfile << "PC,time of issue,begining execution,ending execution,writing back\n";
    for (int i = 0; i < finished_inst.size(); i++)
    {
        myfile << to_string(finished_inst[i].PC_idx) + ","
            + to_string(finished_inst[i].issued_cycle) + ","
            + to_string(finished_inst[i].exec_begin_cycle) + ","
            + to_string(finished_inst[i].exec_end_cycle) + ","
            + to_string(finished_inst[i].wb_cycle) + "\n";
    }
    myfile.close();

}

void read_instructions(string file_name, int starting_PC, vector<int> userInput)
{
    std::ifstream indata;
    std::string inst_full;
    std::string inst_op;
    std::string inst_rs1;
    std::string inst_rs2;
    std::string inst_rd;
    std::string inst_imm;
    std::string temp;
    int rs1;
    int rs2;
    int rd;
    int imm;
    int cycles;
    indata.open(file_name);
    if (indata.fail())
    {
        std::cerr << "Error: file could not be opened" << std::endl;
        exit(1);
    }

    int idx = starting_PC;
    while (std::getline(indata, inst_op, ' ')) //reading operation till space
    {

        if (inst_op == "LW")
        {
            std::getline(indata, inst_rd, ','); //reading rd till ','
            inst_rd.erase(0, 1); //removing the 'x'
            rd = stoi(inst_rd);

            std::getline(indata, inst_imm, '('); //reading imm till '('
            inst_imm.erase(0, 1); //removing the ' '
            imm = stoi(inst_imm);

            std::getline(indata, inst_rs1, ')'); //reading rs1 till ')'
            inst_rs1.erase(0, 1); //removing the 'x'
            rs1 = stoi(inst_rs1);

            std::getline(indata, temp); //move to the next instruction

            rs2 = -1; //no rs2 in LW

            cycles = userInput[1]+userInput[2];
        }
        else
        {
            if (inst_op == "SW")
            {
                std::getline(indata, inst_rs2, ','); //reading rs2 till ','
                inst_rs2.erase(0, 1); //removing the 'x'
                rs2 = stoi(inst_rs2);

                std::getline(indata, inst_imm, '('); //reading imm till '('
                inst_imm.erase(0, 1); //removing the ' '
                imm = stoi(inst_imm);

                std::getline(indata, inst_rs1, ')'); //reading rs1 till ')'
                inst_rs1.erase(0, 1); //removing the 'x'
                rs1 = stoi(inst_rs1);

                std::getline(indata, temp); //move to the next instruction

                rd = -1; //no rd in SW

                cycles = userInput[5];
            }
            else
            {
                if (inst_op == "BEQ")
                {
                    std::getline(indata, inst_rs1, ','); //reading rs1 till ','
                    inst_rs1.erase(0, 1); //removing the 'x'
                    rs1 = stoi(inst_rs1);

                    std::getline(indata, inst_rs2, ','); //reading rs2 till ','
                    inst_rs2.erase(0, 2); //removing the ' ' and 'x'
                    rs2 = stoi(inst_rs2);

                    std::getline(indata, inst_imm); //reading imm till ''
                    inst_imm.erase(0, 1); //removing the ' '
                    imm = stoi(inst_imm);

                    rd = -1; //no rd in BEQ

                    cycles = userInput[8];
                }
                else
                {
                    if (inst_op == "JAL")
                    {
                        std::getline(indata, inst_rd, ','); //reading rd till ','
                        inst_rd.erase(0, 1); //removing the 'x'
                        rd = stoi(inst_rd);

                        std::getline(indata, inst_imm); //reading imm till ''
                        inst_imm.erase(0, 1); //removing the ' '
                        imm = stoi(inst_imm);

                        rs1 = -1; //no rs1 in jal
                        rs2 = -1; //no rs2 in jal

                        cycles = userInput[10];
                    }
                    else
                    {
                        if (inst_op == "JALR")
                        {
                            std::getline(indata, inst_rd, ','); //reading rd till ','
                            inst_rd.erase(0, 1); //removing the 'x'
                            rd = stoi(inst_rd);

                            std::getline(indata, inst_rs1); //reading rs1 till ''
                            inst_rs1.erase(0, 2); //removing the ' ' and the 'x'
                            rs1 = stoi(inst_rs1);

                            imm = -999999999; //no imm in jal
                            rs2 = -1; //no rs2 in jal

                            cycles = userInput[10];;
                        }
                        else
                        {
                            if (inst_op == "ADD")
                            {

                                std::getline(indata, inst_rd, ','); //reading rd till ','
                                inst_rd.erase(0, 1); //removing the 'x'
                                rd = stoi(inst_rd);

                                std::getline(indata, inst_rs1, ','); //reading rs1 till ','
                                inst_rs1.erase(0, 2); //removing the ' ' and the 'x'
                                rs1 = stoi(inst_rs1);

                                std::getline(indata, inst_rs2); //reading rs2 till ''
                                inst_rs2.erase(0, 2); //removing the ' ' and 'x'
                                rs2 = stoi(inst_rs2);

                                imm = -999999999; //no imm in add

                                cycles = userInput[12];
                            }
                            else
                            {
                                if (inst_op == "ADDI")
                                {
                                    std::getline(indata, inst_rd, ','); //reading rd till ','
                                    inst_rd.erase(0, 1); //removing the 'x'
                                    rd = stoi(inst_rd);

                                    std::getline(indata, inst_rs1, ','); //reading rs1 till ','
                                    inst_rs1.erase(0, 2); //removing the the ' ' and the 'x' 
                                    rs1 = stoi(inst_rs1);

                                    std::getline(indata, inst_imm); //reading imm till ''
                                    inst_imm.erase(0, 1); //removing the ' '
                                    imm = stoi(inst_imm);

                                    rs2 = -1; //no rs2 in addi

                                    cycles = userInput[12];
                                }
                                else
                                {
                                    if (inst_op == "NEG")
                                    {
                                        std::getline(indata, inst_rd, ','); //reading rd till ','
                                        inst_rd.erase(0, 1); //removing the 'x'
                                        rd = stoi(inst_rd);

                                        std::getline(indata, inst_rs1); //reading rs1 till ''
                                        inst_rs1.erase(0, 2); //removing the ' ' and the 'x'
                                        rs1 = stoi(inst_rs1);

                                        imm = -999999999; //no imm in NEG

                                        rs2 = -1; //no rs2 in NEG

                                        cycles = userInput[14];
                                    }
                                    else
                                    {
                                        if (inst_op == "ABS")
                                        {
                                            std::getline(indata, inst_rd, ','); //reading rd till ','
                                            inst_rd.erase(0, 1); //removing the 'x'
                                            rd = stoi(inst_rd);

                                            std::getline(indata, inst_rs1); //reading rs1 till ''
                                            inst_rs1.erase(0, 2); //removing the ' ' and the 'x'
                                            rs1 = stoi(inst_rs1);

                                            imm = -999999999; //no imm in ABS
                                            rs2 = -1; //no rs2 in ABS

                                            cycles = userInput[16];
                                        }
                                        else
                                        {
                                            if (inst_op == "DIV")
                                            {
                                                std::getline(indata, inst_rd, ','); //reading rd till ','
                                                inst_rd.erase(0, 1); //removing the 'x'
                                                rd = stoi(inst_rd);

                                                std::getline(indata, inst_rs1, ','); //reading rs1 till ','
                                                inst_rs1.erase(0, 2); //removing the ' ' and the 'x'
                                                rs1 = stoi(inst_rs1);

                                                std::getline(indata, inst_rs2); //reading rs2 till ''
                                                inst_rs2.erase(0, 2); //removing the ' ' and 'x'
                                                rs2 = stoi(inst_rs2);

                                                imm = -999999999; //no imm in div

                                                cycles = userInput[18];
                                            }
                                            else
                                            {
                                                std::cout << "Instruction unidentified\n";
                                                exit(1);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

        }
        Instruction inst(idx, inst_op, rs1, rs2, rd, imm, cycles);
        inst_mem.push_back(inst);
        idx++;
    }
    indata.close();

    //for (int i = 0; i < inst_mem.size(); i++) {
    //    inst_mem[i].print();
    //    cout << endl;
    //}
    return;
}


////////////////////////////////// ABS //////////////////////////////////////////////////////////////
void abs_station::issue()
{
    //cout << "Instruction " << inst.PC_idx << " has been issued\n";
    if (!RegisterStat[inst.rs1].empty()) {
        Qj = RegisterStat[inst.rs1];
    }
    else {
        Vj = Regs[inst.rs1];
        Qj = "";
    }

    busy = true;
    inst.issued_cycle = clk;
    if (branch_issued)
    {
        not_allowed = true;
        after_branch.push_back(this);
    }
    RegisterStat[inst.rd] = name;

}

void abs_station::exec()
{
    if (inst.exec_rem_cycles == exec_time) {
        inst.exec_begin_cycle = clk;
        //cout << "Instruction " << inst.PC_idx << " began executing\n";
    }
    if (inst.exec_rem_cycles == 1) {
        inst.exec_end_cycle = clk;
        cdb.push_back(this);
        finished_exec = true;
        //cout << "Instruction " << inst.PC_idx << " ended executing\n";
    }

    inst.exec_rem_cycles--;
}


void abs_station::wb()
{
    inst.wb_cycle = clk;
    //cout << "Instruction " << inst.PC_idx << " is writing back\n";

    int result = abs(Vj);
    rs.update_rs(name, result);

    finished_inst.push_back(inst);
    flush();

}

void abs_station::flush() {

    Qj = "";
    Vj = 0;
    finished_exec = false;
    busy = false;
    not_allowed = false;

}

//////////////////////////////////////////////// ADD //////////////////////////////////////////////////////////////
void add_station::issue()
{
    //cout << "Instruction " << inst.PC_idx << " has been issued\n";

    if (!RegisterStat[inst.rs1].empty())
    {
        Qj = RegisterStat[inst.rs1];
    }
    else
    {
        Vj = Regs[inst.rs1];
        Qj = "";
    }
    if (inst.op == "ADD") {
        if (!RegisterStat[inst.rs2].empty())
        {
            Qk = RegisterStat[inst.rs2];
        }
        else
        {
            Vk = Regs[inst.rs2];
            Qk = "";
        }
    }
    A = inst.imm;
    op = inst.op;
    inst.issued_cycle = clk;
    busy = true;
    RegisterStat[inst.rd] = name;
    if (branch_issued)
    {
        not_allowed = true;
        after_branch.push_back(this);
    }
}

void add_station::exec()
{
    if (inst.exec_rem_cycles == exec_time) {
        inst.exec_begin_cycle = clk;
        //cout << "Instruction " << inst.PC_idx << " began executing\n";
    }
    if (inst.exec_rem_cycles == 1) {
        inst.exec_end_cycle = clk;
        cdb.push_back(this);
        finished_exec = true;
        //cout << "Instruction " << inst.PC_idx << " ended executing\n";

    }

    inst.exec_rem_cycles--;
}

void add_station::wb()
{
    //cout << "Instruction " << inst.PC_idx << " is writing back\n";

    inst.wb_cycle = clk;
    if (op == "ADDI")
    {
        int result = Vj + A;
        rs.update_rs(name, result);
    }
    else
    {
        if (op == "ADD")
        {
            int result = Vj + Vk;
            rs.update_rs(name, result);
        }
    }

    finished_inst.push_back(inst);
    flush();
}

void add_station::flush() {
    A = 0;
    Qj = "";
    Vj = 0;
    Qk = "";
    Vk = 0;
    op = "";
    busy = false;
    not_allowed = false;
    finished_exec = false;
}

/////////////////////////////////////////////////////// BEQ /////////////////////////////////////////////////////////////////////

void beq_station::issue()
{
    //cout << "Instruction " << inst.PC_idx << " has been issued\n";
    inst.issued_cycle = clk;


    if (!RegisterStat[inst.rs1].empty())
    {
        Qj = RegisterStat[inst.rs1];
    }
    else
    {
        Vj = Regs[inst.rs1];
        Qj = "";
    }
    if (!RegisterStat[inst.rs2].empty())
    {
        Qk = RegisterStat[inst.rs2];
    }
    else
    {
        Vk = Regs[inst.rs2];
        Qk = "";
    }

    A = inst.imm;

    branch_issued = true;
    RegisterStat_buffer = RegisterStat;
    busy = true;
}

void beq_station::exec()
{
    if (inst.exec_rem_cycles == exec_time) {
        inst.exec_begin_cycle = clk;
        //cout << "Instruction " << inst.PC_idx << " began executing\n";
    }
    
    if (inst.exec_rem_cycles == 1)
    {
        inst.exec_end_cycle = clk;
        finished_exec = true;
        //cout << "Instruction " << inst.PC_idx << " ended executing\n";

    }

    inst.exec_rem_cycles--;
}

void beq_station::wb()
{
    //cout << "Instruction " << inst.PC_idx << " is writing back\n";

    inst.wb_cycle = clk;

    if (Vk == Vj)
    {
        branch();
        branch_mispredicted++;
    }
    else
    {
        dont_branch();
    }

    finished_inst.push_back(inst);
    flush();
    total_branches_num++;
}

void beq_station::flush() {
    A = 0;
    Qj = "";
    Vj = 0;
    Qk = "";
    Vk = 0;
    busy = false;
    not_allowed = false;
    finished_exec = false;
    jump_issued = false;
}
void beq_station::branch()
{
    for (int i = 0; i < after_branch.size(); i++) {
        after_branch[i]->flush();
    }
    after_branch.clear();
    branch_issued = false;
    PC = inst.PC_idx + A + 1;
    RegisterStat = RegisterStat_buffer;
}

void beq_station::dont_branch()
{
    rs_station* S;

    while (after_branch.size() > 0)
    {
        S = after_branch[0];
        S->not_allowed = false;
        after_branch.erase(after_branch.begin());

    }
    branch_issued = false;
}

//////////////////////////////////////////////////////////// DIV ///////////////////////////////////////////////////////////////////////////////////

void div_station::issue()
{
    //cout << "Instruction " << inst.PC_idx << " has been issued\n";

    if (!RegisterStat[inst.rs1].empty())
    {
        Qj = RegisterStat[inst.rs1];
    }
    else
    {
        Vj = Regs[inst.rs1];
        Qj = "";
    }
    if (!RegisterStat[inst.rs2].empty())
    {
        Qk = RegisterStat[inst.rs2];
    }
    else
    {
        Vk = Regs[inst.rs2];
        Qk = "";
    }

    inst.issued_cycle = clk;
    busy = true;
    RegisterStat[inst.rd] = name;
    if (branch_issued)
    {
        not_allowed = true;
        after_branch.push_back(this);
    }

}


void div_station::exec()
{
    if (inst.exec_rem_cycles == exec_time) {
        inst.exec_begin_cycle = clk;
        //cout << "Instruction " << inst.PC_idx << " began executing\n";
    }
    
    if (inst.exec_rem_cycles == 1) {
        inst.exec_end_cycle = clk;
        cdb.push_back(this);
        finished_exec = true;
        //cout << "Instruction " << inst.PC_idx << " ended executing\n";

    }

    inst.exec_rem_cycles--;
}


void div_station::wb()
{
    //cout << "Instruction " << inst.PC_idx << " is writing back\n";
    int result;
    inst.wb_cycle = clk;
    if( Vk == 0)
    {
        std::cout<<"\nCan not divide by zero\n";
        exit(0);
    }
    else
        result = Vj / Vk;
    rs.update_rs(name, result);

    finished_inst.push_back(inst);
    flush();
}

void div_station::flush() {

    Qj = "";
    Vj = 0;
    Qk = "";
    Vk = 0;
    finished_exec = false;
    busy = false;
    not_allowed = false;
}

////////////////////////////////////////////////////// JAL /////////////////////////////////////////////////////

void jal_station::issue()
{
    //cout << "Instruction " << inst.PC_idx << " has been issued\n";

    if(inst.op == "JALR"){
        if (!RegisterStat[inst.rs1].empty())
        {
            Qj = RegisterStat[inst.rs1];
        }
        else
        {
            Vj = Regs[inst.rs1];
            Qj = "";
        }
    }
    A = inst.imm;
    op = inst.op;
    inst.issued_cycle = clk;
    busy = true;
    RegisterStat[inst.rd] = name;
    jump_issued = true;
    if (branch_issued)
    {
        not_allowed = true;
        after_branch.push_back(this);
    }
}

void jal_station::exec()
{
    if (inst.exec_rem_cycles == exec_time) {
        inst.exec_begin_cycle = clk;
        //cout << "Instruction " << inst.PC_idx << " began executing\n";
    }
    
    if (inst.exec_rem_cycles == 1) {
        inst.exec_end_cycle = clk;
        cdb.push_back(this);
        finished_exec = true;
        //cout << "Instruction " << inst.PC_idx << " ended executing\n";

    }

    inst.exec_rem_cycles--;
}

void jal_station::wb()
{
    //cout << "Instruction " << inst.PC_idx << " is writing back\n";

    inst.wb_cycle = clk;
    if (op == "JAL")
    {

        int result = inst.PC_idx + 1;
        PC = result + A;
        rs.update_rs(name, result);
    }
    else
    {
        if (op == "JALR")
        {
            int result = inst.PC_idx + 1;
            PC = Vj;
            rs.update_rs(name, result);
        }
    }

    finished_inst.push_back(inst);
    flush();
}

void jal_station::flush() {
    A = 0;
    Qj = "";
    Vj = 0;
    op = "";
    busy = false;
    not_allowed = false;
    finished_exec = false;
    jump_issued = false;
}

//////////////////////////////////////////////////// LW //////////////////////////////////////////////////////
void load_station::issue() {
    //cout << "Instruction " << inst.PC_idx << " has been issued\n";

    if (!RegisterStat[inst.rs1].empty()) {
        Qj = RegisterStat[inst.rs1];
    }
    else {
        Vj = Regs[inst.rs1];
        Qj = "";
    }
    A = inst.imm;
    busy = true;
    inst.issued_cycle = clk;

    load_store_hazard = true;

    RegisterStat[inst.rd] = name;
    load_store_queue.push_back(this);
    if (branch_issued)
    {
        not_allowed = true;
        after_branch.push_back(this);
    }
}

void load_station::exec() {
    if (!load_store_hazard) {
        if (inst.exec_rem_cycles == exec_time) {
            inst.exec_begin_cycle = clk;
            //cout << "Instruction " << inst.PC_idx << " began executing\n";
        }
        if(inst.exec_rem_cycles == read_mem+1){
            load_store_queue.erase(load_store_queue.begin());
        }
        if (inst.exec_rem_cycles == read_mem) {
            A = A + Vj;
            addr_calculated = true;
            check_store();
        }
        if (inst.exec_rem_cycles == 1) {
            inst.exec_end_cycle = clk;
            cdb.push_back(this);
            finished_exec = true;
            //cout << "Instruction " << inst.PC_idx << " ended executing\n";

        }

        inst.exec_rem_cycles--;
    }
    else if (addr_calculated) {
        if (store_waiting_for->busy == false) {
            load_store_hazard = false;
            store_waiting_for = NULL;
        }
    }
}

void load_station::wb() {
    //cout << "Instruction " << inst.PC_idx << " is writing back\n";

    inst.wb_cycle = clk;

    rs.update_rs(name, Mem[A]);

    finished_inst.push_back(inst);
    flush();
}

void load_station::flush() {
    A = 0;
    Qj = "";
    Vj = 0;
    busy = false;
    not_allowed = false;
    load_store_hazard = false;
    finished_exec = false;
}

void load_station::check_store() {
    store_waiting_for = NULL;
    for (int i = 0; i < rs.rs_sw.size(); i++)
    {
        if (rs.rs_sw[i].busy && rs.rs_sw[i].addr_calculated && A == rs.rs_sw[i].A) {
            load_store_hazard = true;
            if (store_waiting_for == NULL) store_waiting_for = &rs.rs_sw[i];
            else {
                if (store_waiting_for->inst.issued_cycle < rs.rs_sw[i].inst.issued_cycle) store_waiting_for = &rs.rs_sw[i];
            }
        }
    }

}


///////////////////////////////////////////////////////////////// NEG ///////////////////////////////////////////////////////////////
void neg_station::issue()
{
    //cout << "Instruction " << inst.PC_idx << " has been issued\n";

    if (!RegisterStat[inst.rs1].empty()) {
        Qj = RegisterStat[inst.rs1];
    }
    else {
        Vj = Regs[inst.rs1];
        Qj = "";
    }

    busy = true;
    inst.issued_cycle = clk;

    RegisterStat[inst.rd] = name;
    if (branch_issued)
    {
        not_allowed = true;
        after_branch.push_back(this);
    }
}

void neg_station::exec()
{
    if (inst.exec_rem_cycles == exec_time) {
        inst.exec_begin_cycle = clk;
        //cout << "Instruction " << inst.PC_idx << " began executing\n";
    }
    if (inst.exec_rem_cycles == 1) {
        inst.exec_end_cycle = clk;
        cdb.push_back(this);
        finished_exec = true;
        //cout << "Instruction " << inst.PC_idx << " ended executing\n";

    }

    inst.exec_rem_cycles--;
}


void neg_station::wb()
{
    //cout << "Instruction " << inst.PC_idx << " is writing back\n";

    inst.wb_cycle = clk;

    int result = -Vj;
    rs.update_rs(name, result);

    finished_inst.push_back(inst);
    flush();
}

void neg_station::flush() {

    Qj = "";
    Vj = 0;
    finished_exec = false;
    busy = false;
    not_allowed = false;
}

///////////////////////////////////////////////// SW ////////////////////////////////////////////////////
void store_station::issue() {
    //cout << "Instruction " << inst.PC_idx << " has been issued\n";

    if (!RegisterStat[inst.rs1].empty()) {
        Qj = RegisterStat[inst.rs1];
    }
    else {
        Vj = Regs[inst.rs1];
        Qj = "";
    }

    if (!RegisterStat[inst.rs2].empty()) {
        Qk = RegisterStat[inst.rs2];
    }
    else {
        Vk = Regs[inst.rs1];
        Qk = "";
    }

    A = inst.imm;
    busy = true;
    inst.issued_cycle = clk;

    load_store_hazard = true;
    load_store_queue.push_back(this);

    if (branch_issued)
    {
        not_allowed = true;
        after_branch.push_back(this);
    }
}

void store_station::exec() {
    if (!load_store_hazard) {
        if (inst.exec_rem_cycles == addr_calc) {
            inst.exec_begin_cycle = clk;
            //cout << "Instruction " << inst.PC_idx << " began executing\n";
        }
        if (inst.exec_rem_cycles == 1) {
            A = A + Vj;
            addr_calculated = true;
            load_store_queue.erase(load_store_queue.begin());
            check_load_store();

            inst.exec_end_cycle = clk;
            finished_exec = true;

            rem_write_mem = write_mem;
        }

        inst.exec_rem_cycles--;
    }
}


void store_station::wb() {
    if(load_store_waiting_for == NULL || load_store_waiting_for->busy == false){
        load_store_waiting_for = NULL;
        if(rem_write_mem == 1){
            inst.wb_cycle = clk;
            Mem[A] = Vk;

            finished_inst.push_back(inst);
            flush();
        }
        else rem_write_mem--;
    }
}

void store_station::flush() {
    A = 0;
    Qj = "";
    Vj = 0;
    Qk = "";
    Vk = 0;
    busy = false;
    not_allowed = false;
    load_store_hazard = false;
    finished_exec = false;
}


void store_station::check_load_store() {
    load_store_waiting_for = NULL;
    for (int i = 0; i < rs.rs_sw.size(); i++)
    {
        if (rs.rs_sw[i].busy && rs.rs_sw[i].addr_calculated && A == rs.rs_sw[i].A && rs.rs_sw[i].inst.PC_idx != inst.PC_idx) {
            load_store_hazard = true;
            if (load_store_waiting_for == NULL) load_store_waiting_for = &rs.rs_sw[i];
            else {
                if (load_store_waiting_for->inst.PC_idx < rs.rs_sw[i].inst.PC_idx) load_store_waiting_for = &rs.rs_sw[i];
            }
        }
    }
    for (int i = 0; i < rs.rs_lw.size(); i++)
    {
        if (rs.rs_lw[i].busy && rs.rs_lw[i].addr_calculated && A == rs.rs_lw[i].A) {
            load_store_hazard = true;
            if (load_store_waiting_for == NULL) load_store_waiting_for = &rs.rs_lw[i];
            else {
                if (load_store_waiting_for->inst.PC_idx < rs.rs_lw[i].inst.PC_idx) load_store_waiting_for = &rs.rs_lw[i];
            }
        }
    }

}

/// <summary>
/// /////////////////////////// RS /////////////////////////////////////////////////////////

void reservation_stations::init(int lw, int lw_addr_calc, int lw_read_mem, int sw, int sw_addr_calc, int sw_write_mem,
    int beq, int beq_exec, int jal, int jal_exec, int add, int add_exec, int neg, int neg_exec,
    int abs, int abs_exec, int div, int div_exec)
{
    rs_lw.resize(lw);
    rs_sw.resize(sw);
    rs_beq.resize(beq);
    rs_jal.resize(jal);
    rs_add.resize(add);
    rs_neg.resize(neg);
    rs_abs.resize(abs);
    rs_div.resize(div);

    for (int i = 0; i < rs_lw.size(); i++)
    {
        rs_lw[i].exec_time = lw_addr_calc + lw_read_mem;
        rs_lw[i].addr_calc = lw_addr_calc;
        rs_lw[i].read_mem = lw_read_mem;
        if (rs_lw.size() == 1)
            rs_lw[i].name = "load";
        else
            rs_lw[i].name = "load" + to_string(i + 1);
    }

    for (int i = 0; i < rs_sw.size(); i++)
    {
        rs_sw[i].addr_calc = sw_addr_calc;
        rs_sw[i].write_mem = sw_write_mem;
        if (rs_sw.size() == 1)
            rs_sw[i].name = "store";
        else
            rs_sw[i].name = "store" + to_string(i + 1);
    }

    for (int i = 0; i < rs_beq.size(); i++)
    {
        rs_beq[i].exec_time = beq_exec;
        if (rs_beq.size() == 1)
            rs_beq[i].name = "branch";
        else
            rs_beq[i].name = "branch" + to_string(i + 1);
    }

    for (int i = 0; i < rs_jal.size(); i++)
    {
        rs_jal[i].exec_time = jal_exec;
        if (rs_jal.size() == 1)
            rs_jal[i].name = "jal";
        else
            rs_jal[i].name = "jal" + to_string(i + 1);
    }

    for (int i = 0; i < rs_add.size(); i++)
    {
        rs_add[i].exec_time = add_exec;
        if (rs_add.size() == 1)
            rs_add[i].name = "add";
        else
            rs_add[i].name = "add" + to_string(i + 1);
    }

    for (int i = 0; i < rs_neg.size(); i++)
    {
        rs_neg[i].exec_time = neg_exec;
        if (rs_neg.size() == 1)
            rs_neg[i].name = "negate";
        else
            rs_neg[i].name = "negate" + to_string(i + 1);
    }

    for (int i = 0; i < rs_abs.size(); i++)
    {
        rs_abs[i].exec_time = abs_exec;
        if (rs_abs.size() == 1)
            rs_abs[i].name = "absolute";
        else
            rs_abs[i].name = "absolute" + to_string(i + 1);
    }

    for (int i = 0; i < rs_div.size(); i++)
    {
        rs_div[i].exec_time = div_exec;
        if (rs_div.size() == 1)
            rs_div[i].name = "divide";
        else
            rs_div[i].name = "divide" + to_string(i + 1);
    }
}

void reservation_stations::update_rs(string r, int result)
{
    //Updating RegisterStat
    for (int i = 1; i < RegisterStat.size(); i++) { // We start looping from 1 because we can't write to R0
        if (RegisterStat[i] == r) {
            Regs[i] = result;
            RegisterStat[i] = "";
        }
    }

    // Update RS
    for (int i = 0; i < rs_lw.size(); i++)
    {
        if (rs_lw[i].Qj == r) {
            rs_lw[i].Vj = result;
            rs_lw[i].Qj = "";
        }
    }

    for (int i = 0; i < rs_sw.size(); i++)
    {
        if (rs_sw[i].Qj == r) {
            rs_sw[i].Vj = result;
            rs_sw[i].Qj = "";
        }
        if (rs_sw[i].Qk == r) {
            rs_sw[i].Vk = result;
            rs_sw[i].Qk = "";
        }
    }

    for (int i = 0; i < rs_beq.size(); i++)
    {
        if (rs_beq[i].Qj == r) {
            rs_beq[i].Vj = result;
            rs_beq[i].Qj = "";
        }
        if (rs_beq[i].Qk == r) {
            rs_beq[i].Vk = result;
            rs_beq[i].Qk = "";
        }
    }

    for (int i = 0; i < rs_jal.size(); i++)
    {
        if (rs_jal[i].Qj == r) {
            rs_jal[i].Vj = result;
            rs_jal[i].Qj = "";
        }
    }

    for (int i = 0; i < rs_add.size(); i++)
    {
        if (rs_add[i].Qj == r) {
            rs_add[i].Vj = result;
            rs_add[i].Qj = "";
        }
        if (rs_add[i].Qk == r) {
            rs_add[i].Vk = result;
            rs_add[i].Qk = "";
        }
    }

    for (int i = 0; i < rs_neg.size(); i++)
    {
        if (rs_neg[i].Qj == r) {
            rs_neg[i].Vj = result;
            rs_neg[i].Qj = "";
        }
    }

    for (int i = 0; i < rs_abs.size(); i++)
    {
        if (rs_abs[i].Qj == r) {
            rs_abs[i].Vj = result;
            rs_abs[i].Qj = "";
        }
    }

    for (int i = 0; i < rs_div.size(); i++)
    {
        if (rs_div[i].Qj == r) {
            rs_div[i].Vj = result;
            rs_div[i].Qj = "";
        }

        if (rs_div[i].Qk == r) {
            rs_div[i].Vk = result;
            rs_div[i].Qk = "";
        }
    }
}

bool reservation_stations::done() {

    for (int i = 0; i < rs_lw.size(); i++)
    {
        if (rs_lw[i].busy)
            return false;
    }

    for (int i = 0; i < rs_sw.size(); i++)
    {
        if (rs_sw[i].busy)
            return false;
    }

    for (int i = 0; i < rs_beq.size(); i++)
    {
        if (rs_beq[i].busy)
            return false;
    }

    for (int i = 0; i < rs_jal.size(); i++)
    {
        if (rs_jal[i].busy)
            return false;
    }

    for (int i = 0; i < rs_add.size(); i++)
    {
        if (rs_add[i].busy)
            return false;
    }

    for (int i = 0; i < rs_neg.size(); i++)
    {
        if (rs_neg[i].busy)
            return false;
    }

    for (int i = 0; i < rs_abs.size(); i++)
    {
        if (rs_abs[i].busy)
            return false;
    }

    for (int i = 0; i < rs_div.size(); i++)
    {
        if (rs_div[i].busy)
            return false;
    }
    return true;
}

void reservation_stations::issue_load() {
    for (int i = 0; i < rs_lw.size(); i++) {
        if (!rs_lw[i].busy) {
            rs_lw[i].inst = inst_mem[PC];
            PC++;
            rs_lw[i].issue();
            break;
        }
    }
}

void reservation_stations::issue_store() {
    for (int i = 0; i < rs_sw.size(); i++) {
        if (!rs_sw[i].busy) {
            rs_sw[i].inst = inst_mem[PC];
            PC++;
            rs_sw[i].issue();
            break;
        }
    }
}

void reservation_stations::issue_add()
{
    for (int i = 0; i < rs_add.size(); i++)
    {
        if (!rs_add[i].busy)
        {
            rs_add[i].inst = inst_mem[PC];
            PC++;

            rs_add[i].issue();
            break;
        }
    }
}

void reservation_stations::issue_beq()
{
    for (int i = 0; i < rs_beq.size(); i++)
    {
        if (!rs_beq[i].busy)
        {
            rs_beq[i].inst = inst_mem[PC];
            PC++;

            rs_beq[i].issue();
            break;
        }
    }
}

void reservation_stations::issue_jal()
{
    for (int i = 0; i < rs_jal.size(); i++)
    {
        if (!rs_jal[i].busy)
        {
            rs_jal[i].inst = inst_mem[PC];
            PC++;

            rs_jal[i].issue();
            break;
        }
    }
}

void reservation_stations::issue_neg()
{
    for (int i = 0; i < rs_neg.size(); i++)
    {
        if (!rs_neg[i].busy)
        {
            rs_neg[i].inst = inst_mem[PC];
            PC++;

            rs_neg[i].issue();
            break;
        }
    }
}

void reservation_stations::issue_div()
{
    for (int i = 0; i < rs_div.size(); i++)
    {
        if (!rs_div[i].busy)
        {
            rs_div[i].inst = inst_mem[PC];
            PC++;

            rs_div[i].issue();
            break;
        }
    }
}

void reservation_stations::issue_abs()
{
    for (int i = 0; i < rs_abs.size(); i++)
    {
        if (!rs_abs[i].busy)
        {
            rs_abs[i].inst = inst_mem[PC];
            PC++;

            rs_abs[i].issue();
            break;
        }
    }
}

void reservation_stations::wb_store() {
    for (int i = 0; i < rs_sw.size(); i++) // SW Execution
    {
        if (rs_sw[i].busy == true && rs_sw[i].finished_exec && rs_sw[i].can_wb())
        {
            rs_sw[i].wb();
        }
    }
}

void reservation_stations::wb_beq() {
    for (int i = 0; i < rs_beq.size(); i++) // SW Execution
    {
        if (rs_beq[i].busy == true && rs_beq[i].finished_exec)
        {
            rs_beq[i].wb();
        }
    }
}

void reservation_stations::execute() {
    //Execution for LW & SW
    if (!load_store_queue.empty()) 
    {
        rs_station* load_store_head = load_store_queue.front();
        if (load_store_head->can_exec()) 
        {
            load_store_head->load_store_hazard = false;
        }
    }

    for (int i = 0; i < rs_lw.size(); i++) // LW Execution
    {
        if (rs_lw[i].busy == true && rs_lw[i].can_exec() && !rs_lw[i].finished_exec)
        {
            rs_lw[i].exec();
        }
    }
    for (int i = 0; i < rs_sw.size(); i++) // SW Execution
    {
        if (rs_sw[i].busy == true && rs_sw[i].can_exec() && !rs_sw[i].finished_exec)
        {
            rs_sw[i].exec();
        }
    }

    //Executing Arithmetic Operations
    for (int i = 0; i < rs_add.size(); i++) // ADD/ADDI Execution
    {
        if (rs_add[i].busy == true && rs_add[i].can_exec() && !rs_add[i].finished_exec)
        {
            rs_add[i].exec();
        }
    }

    for (int i = 0; i < rs_div.size(); i++) // ADD/ADDI Execution
    {
        if (rs_div[i].busy == true && rs_div[i].can_exec() && !rs_div[i].finished_exec)
        {
            rs_div[i].exec();
        }
    }

    for (int i = 0; i < rs_abs.size(); i++) // ADD/ADDI Execution
    {
        if (rs_abs[i].busy == true && rs_abs[i].can_exec() && !rs_abs[i].finished_exec)
        {
            rs_abs[i].exec();
        }
    }

    for (int i = 0; i < rs_neg.size(); i++) // ADD/ADDI Execution
    {
        if (rs_neg[i].busy == true && rs_neg[i].can_exec() && !rs_neg[i].finished_exec)
        {
            rs_neg[i].exec();
        }
    }
    for (int i = 0; i < rs_beq.size(); i++) // BEQ Execution
    {
        if (rs_beq[i].busy == true && rs_beq[i].can_exec() && !rs_beq[i].finished_exec)
        {
            rs_beq[i].exec();
        }
    }
    for (int i = 0; i < rs_jal.size(); i++) // JAL/JALR Execution
    {
        if (rs_jal[i].busy == true && rs_jal[i].can_exec() && !rs_jal[i].finished_exec)
        {
            rs_jal[i].exec();
        }
    }
    
}
