#include "includes.h"
#pragma once

void addLoadInstruction(int address)
 {
    for (auto& entry : loadBuffer) 
    {
        if (!entry.ready) {
            entry.address = address;
            entry.ready = false;
            entry.result = 0; // Initialize result
            return;
        }
    }
    cout << "Load buffer full. Unable to add load instruction." << std::endl;
}