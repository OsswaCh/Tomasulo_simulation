#include <iostream>
#include <vector>
using namespace std;

typedef pair<string, int> reg_item;

/////////////////////////////////////////other classes/////////////////////////////////////////
class instruction
{

public:
    // instruction components
    int PC;
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
    instruction(int PC = 0, string operation = "", int rs1 = 0, int rs2 = 0, int rd = 0, int imm = 0, short cycles = 0) : PC(PC), rs1(rs1), rs2(rs2), rd(rd), imm(imm), cycle_count_per_instruction(cycles)
    {
        issue_cycle = INT_MIN;
        execution_start_cycle = INT_MIN;
        excecution_end_cycle = INT_MIN;
        write_back_cycle = INT_MIN;
    }
};

class reservation_station
{

public:
    // functions to be inherited by other classes
    virtual bool ready();
    virtual void excecute();
    virtual void write_back();
    virtual void flush();

    // components of reservation station
    string OP;
    string name;
    bool busy;
    string Qj, Qk;
    int Vj, Vk;
    int A;

    // register status array
    vector<reg_item> registers;

    // contructors
    reservation_station(string OP = "",
                        string name = "",
                        bool busy = false,
                        string Qj = "",
                        string Qk = "",
                        int A = 0,
                        int Vj = -1,
                        int Vk = -1) : OP(OP), name(name), busy(busy), Qj(Qj), Qk(Qk), Vj(Vj), Vk(Vk){};
};

////////////////////////////////////////inherited classes////////////////////////////////////////

class add_reservation_station : public reservation_station
{
    // make sure to check if things are allowed to excecute or not

    bool ready()
    {
        return (Qj == "" && Qk == "");
    }
    void excecute()
    {
    }
};
