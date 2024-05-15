#pragma once
#include "includes.h"


extern vector<int> loadBuffer;
extern vector<vector<int>> storeBuffer;
extern int PC;
extern int clk;
extern int current_cycle;
extern reservation_stations* res_stations;
extern vector<instruction> instructions;
extern vector<reservation_station> after_branch_record;

typedef pair<int, bool> reg_item; // reg value and status

struct LoadBufferEntry 
{
    int address;
    bool ready;
    int result;
};

struct StoreBufferEntry
{
    int address;
    int data;
    bool committed;
};