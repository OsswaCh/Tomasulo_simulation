#pragma once
#include "includes.h"
#include "structs.h"

// ADD, MUL, NAND
void parseRInstruction(instruction &temp, std::string line)
{
    temp.rd = stoi(line.substr(0, line.find(',')));
    line.erase(0, line.find(',') + 3);
    temp.rs1 = stoi(line.substr(0, line.find(',')));
    line.erase(0, line.find(',') + 3);
    temp.rs2 = stoi(line.substr(0, line.find(',')));
}

// ADDI
void parseIInstruction(instruction &temp, std::string line)
{

    int rd = stoi(line.substr(0, line.find(',')));
    temp.rd = rd;
    line.erase(0, line.find(',') + 3);
    int rs1 = stoi(line.substr(0, line.find(',')));
    temp.rs1 = rs1;
    line.erase(0, line.find(',') + 2);

    // Extract the imm value
    int imm = stoi(line);
    temp.imm = imm;
}

// BEQ
void parseBInstruction(instruction &temp, std::string line)
{
    temp.rd = stoi(line.substr(0, line.find(',')));
    line.erase(0, line.find(',') + 3);
    temp.rs1 = stoi(line.substr(0, line.find(',')));
    line.erase(0, line.find(',') + 2);
    temp.rs2 = stoi(line.substr(0, line.find(',')));

    // Extract the imm value
    int imm = stoi(line);
    temp.imm = imm;
}

// LOAD
void parseLoadInstruction(instruction &temp, std::string line)
{
    temp.rd = stoi(line.substr(0, line.find(',')));
    line.erase(0, line.find(',') + 1);
    temp.imm = stoi(line.substr(0, line.find('(')));
    line.erase(0, line.find('(') + 2);
    temp.rs1 = stoi(line.substr(0, line.find(')')));
}

// STORE
void parseStoreInstruction(instruction &temp, std::string line)
{
    temp.rd = stoi(line.substr(0, line.find(',')));
    line.erase(0, line.find(',') + 1);
    temp.imm = stoi(line.substr(0, line.find('(')));
    line.erase(0, line.find('(') + 2);
    temp.rs1 = stoi(line.substr(0, line.find(')')));
}

void parseCallInstruction(instruction &temp, std::string line)
{
    temp.imm = stoi(line);
}

void readInstructions(std::string filename, std::vector<instruction> &instructions)
{
    cout << "FILE NAME: " << filename << endl;
    std::ifstream file(filename);
    std::string line;

    if (!file.is_open())
    {
        std::cout << "File not opened";
        exit(1);
    }

    int inst_number = 0;

    while (getline(file, line))
    {
        std::cout << line << std::endl;
        instruction temp;
        string inst_name = line.substr(0, line.find(' '));

        auto label_last_index = line.find(":");
        if (label_last_index != string::npos) // if label is present
        {
            string label = line.substr(0, label_last_index);
            line.erase(0, label_last_index + 2);
            temp.label = label;
        }

        line.erase(0, line.find(' ') + 2);

        try
        {
            //////////////////////////////

            if (inst_name == "ADD" || inst_name == "NAND" || inst_name == "MUL")
            {
                if (inst_name == "ADD")
                    temp.OP = instruction::type::ADD;
                else if (inst_name == "NAND")
                    temp.OP = instruction::type::NAND;
                else if (inst_name == "MUL")
                    temp.OP = instruction::type::MUL;
                parseRInstruction(temp, line);
                std::cout << "Instruction: OP" << temp.OP << " rd " << temp.rd << " rs1 " << temp.rs1 << " rs2 " << temp.rs2 << std::endl;
            }
            else if (inst_name == "ADDI")
            {
                temp.OP = instruction::type::ADDI;
                parseIInstruction(temp, line);
                std::cout << "Instruction: OP" << temp.OP << " rd " << temp.rd << " rs1 " << temp.rs1 << " rs2 " << temp.rs2 << " imm " << temp.imm << std::endl;
            }
            else if (inst_name == "BEQ")
            {
                temp.OP = instruction::type::BEQ;
                parseBInstruction(temp, line);
                std::cout << "Instruction: OP" << temp.OP << " rd " << temp.rd << " rs1 " << temp.rs1 << " rs2 " << temp.rs2 << " imm " << temp.imm << std::endl;
            }
            else if (inst_name == "LOAD")
            {
                temp.OP = instruction::type::LOAD;
                parseLoadInstruction(temp, line);
                std::cout << "Instruction: OP" << temp.OP << " rd " << temp.rd << " rs1 " << temp.rs1 << " imm " << temp.imm << std::endl;
            }
            else if (inst_name == "STORE")
            {
                temp.OP = instruction::type::STORE;
                parseStoreInstruction(temp, line);
                std::cout << "Instruction: OP" << temp.OP << " rd " << temp.rd << " rs1 " << temp.rs1 << " imm " << temp.imm << std::endl;
            }
            else if (inst_name == "RET")
            {

                temp.OP = instruction::type::RET;
            }
            else if (inst_name == "CALL")
            {
                temp.OP = instruction::type::CALL;
                parseCallInstruction(temp, line);
                std::cout << "Instruction: OP" << temp.OP << " imm " << temp.imm << std::endl;
            }
            else
            {
                std::cout << "Unknown instruction: " << inst_name << std::endl;
                exit(1); // Exit the program with an error status code.
            }

            inst_number++;
            instructions.push_back(temp);
        }
        catch (const std::exception &e)
        {
            std::cerr << "An exception occurred: " << e.what() << std::endl;
            exit(1);
        }
    }
}

// void printInstructions()
// {
//     for (int i = 0; i < instructions.size(); i++)
//     {
//         std::cout << instructions[i].OP << " " << instructions[i].rs1 << " " << instructions[i].rs2 << " " << instructions[i].rd << " " << instructions[i].imm << std::endl;
//     }
// }

std::string decToBinary(int n)
{
    std::vector<int> binaryNum(32);
    std::vector<int> arr(32);
    std::string output = "";
    // counter for binary array
    int i = 0;
    while (i < 32)
    {
        // storing remainder in binary array and the rest with 0
        if (n <= 0)
        {
            binaryNum[i] = 0;
            i++;
            continue;
        }
        else
        {
            binaryNum[i] = n % 2;
            n = n / 2;
            i++;
        }
    }
    for (int j = i - 1; j >= 0; j--)
        arr[32 - 1 - j] = binaryNum[j];
    for (int i = 0; i < 32; i++)
        output += std::to_string(arr[i]);
    return output;
}
std::string decToHex(int n)
{
    // ans string to store hexadecimal number
    std::string ans = "";

    while (n != 0)
    {
        int rem = 0;
        char ch;
        rem = n % 16;
        if (rem < 10)
        {
            ch = rem + 48;
        }
        else
        {
            ch = rem + 55;
        }
        ans += ch;
        n = n / 16;
    }
    int i = 0, j = ans.size() - 1;
    while (i <= j)
    {
        std::swap(ans[i], ans[j]);
        i++;
        j--;
    }
    if (ans.empty())
    {
        ans = "0";
    }
    return ans;
}

void printRegisterFile()
{
    // std::cout << "Register File:" << std::endl;
    // for (int i = 0; i < 32; i++)
    // {
    //     std::cout << "x" << i << ": " << registerFile[i] << "  | Binary : " << decToBinary(registerFile[i]) << "  | Hex: " << decToHex(registerFile[i]) << std::endl;
    // }
}