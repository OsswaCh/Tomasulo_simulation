#pragma once
#include "includes.h"

extern vector<int> loadBuffer;
extern vector<vector<int>> storeBuffer;
extern int PC;
extern int clk;
extern int current_cycle;
extern reservation_stations *res_stations;
extern vector<instruction> instructions;
extern vector<instruction> finished_instructions;
extern vector<reservation_station> after_branch_record;

typedef pair<int, bool> reg_item; // reg value and status

struct LoadStoreBufferType
{
	static string const LOAD;
	static string const STORE;
};