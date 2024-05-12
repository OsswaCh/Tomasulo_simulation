#include <iostream>
#include <vector>
#include <string>
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
    bool ready();
    void execute();
    void write_back();
    void flush();

    // components of reservation station
    string OP;
    string name;
    bool busy;
    string Qj, Qk;
    int Vj, Vk;
    int A;

    // number of reservation stations
    int size;

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
                        int Vk = -1,
                        int size = 0) : OP(OP), name(name), busy(busy), Qj(Qj), Qk(Qk), A(A), Vj(Vj), Vk(Vk), size(size) {}
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
    int laod_cycles = 6;  // 2 for address calculation, 4 for memory access
    int store_cycles = 6; // 2 for address calculation, 4 for memory access
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
            adders.push_back(reservation_station("ADD", "adder" + to_string(i), false, "", "", 0, -1, -1, hardware.adders));
        }
        for (int i = 0; i < hardware.multipliers; i++)
        {
            multipliers.push_back(reservation_station("MUL", "multiplier" + to_string(i), false, "", "", 0, -1, -1, hardware.multipliers));
        }
        for (int i = 0; i < hardware.loaders; i++)
        {
            loader.push_back(reservation_station("LOAD", "loader" + to_string(i), false, "", "", 0, -1, -1, hardware.loaders));
        }
        for (int i = 0; i < hardware.stores; i++)
        {
            stores.push_back(reservation_station("STORE", "store" + to_string(i), false, "", "", 0, -1, -1, hardware.stores));
        }
        for (int i = 0; i < hardware.nanders; i++)
        {
            nanders.push_back(reservation_station("NAND", "nander" + to_string(i), false, "", "", 0, -1, -1, hardware.nanders));
        }
    }
};
