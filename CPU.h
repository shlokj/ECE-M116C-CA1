#include <iostream>
#include <bitset>
#include <stdio.h>
#include<stdlib.h>
#include <string>
using namespace std;


enum Operation {
    ADD,
    SUB,
    ADDI,
    XOR,
    ANDI,
    SRA,
    LW,
    SW,
    BLT,
    JALR,
	NOP // when we get 0 do nothing
};

class instruction {
public:
	bitset<32> instr;//instruction
	instruction(bitset<32> fetch); // constructor

};

class CPU {
private:
	int dmemory[4096]; //data memory byte addressable in little endian fashion;
	unsigned long PC; //pc
	int32_t registerFile[32];
	Operation operation;
	int32_t sourceRegister1;
	int32_t sourceRegister2;
	int32_t memoryData;
	uint8_t destinationRegister;
	int32_t aluResult;
	int32_t immediate;

public:
	CPU();
	unsigned long readPC();
	bitset<32> Fetch(bitset<8> *instmem);
	bool Decode(instruction* instr);
	void executeCurrentInstruction();
	void performRegisterWriteback();
	void performMemoryOperations();
	int getRegisterA0() const {
		return registerFile[10];
	}
	int getRegisterA1() const {
		return registerFile[11];
	}
};

// add other functions and objects here
