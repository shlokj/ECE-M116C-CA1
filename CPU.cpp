#include "CPU.h"

instruction::instruction(bitset<32> fetch)
{
	//cout << fetch << endl;
	instr = fetch;
	//cout << instr << endl;
}

CPU::CPU()
{
	registerFile[0] = 0;
	operation = NOP;
	sourceRegister1 = 0;
	sourceRegister2 = 0;
	memoryData = 0;
	destinationRegister = 0;
	immediate = 0;
	aluResult = 0;
	PC = 0; //set PC to 0
	for (int i = 0; i < 4096; i++) //copy instrMEM
	{
		dmemory[i] = (0);
	}
}

bitset<32> CPU::Fetch(bitset<8> *instmem) {
	bitset<32> instr = ((((instmem[PC + 3].to_ulong()) << 24)) + ((instmem[PC + 2].to_ulong()) << 16) + ((instmem[PC + 1].to_ulong()) << 8) + (instmem[PC + 0].to_ulong()));  //get 32 bit instruction
	PC += 4;//increment PC
	return instr;
}

bool CPU::Decode(instruction* curr) {
	// PC IS INCREMENTED AS SOON AS AN INSTRUCTION IS FETCHED
    uint32_t instructionInt = curr->instr.to_ulong();

	const uint32_t OPCODE_MASK = 0b1111111; // last 7 bits
    uint32_t opcode = instructionInt & OPCODE_MASK;

	const uint32_t RD_MASK = 0b11111 << 7;
    const uint32_t RS1_MASK = 0b11111 << 15;
	const uint32_t RS2_MASK = 0b11111 << 20;
	const uint32_t FUNCT3_MASK = 0b111 << 12;
	const uint32_t IMM_MASK = 0xFFF00000; // in some instructions, the immediate is in the leftmost 12 bits

    uint32_t rd = (instructionInt & RD_MASK) >> 7;
    uint32_t rs1 = (instructionInt & RS1_MASK) >> 15;
	uint32_t rs2 = (instructionInt & RS2_MASK) >> 20;

	sourceRegister1 = rs1;
	sourceRegister2 = rs2;
	destinationRegister = rd;

	// cout << "\ncommons: " << endl;
	// cout << "rd: " << destinationRegister << endl;
	// cout << "rs1: " << rs1 << endl;
	// cout << "rs2: " << rs2 << endl;

    // R-type instructions: ADD, SUB, XOR, SRA
	aluResult = 5;
    if (opcode == 0b0110011) {
		// TESTED WORKING
		const uint32_t FUNCT7_MASK = 0b1111111 << 25; // 7 bits to the left of rs2		
        
        uint32_t funct3 = (instructionInt & FUNCT3_MASK) >> 12;
        uint32_t funct7 = (instructionInt & FUNCT7_MASK) >> 25;
		
		if (funct3 == 0 && funct7 == 0) { // ADD
			// cout << "found ADD" << endl;
			operation = ADD;
		}
		else if (funct3 == 0 && funct7 == 0b0100000) { // SUB
			// cout << "found SUB" << endl;
			operation = SUB;
		}
		else if (funct3 == 0b100 && funct7 == 0) { // XOR
			// cout << "found XOR" << endl;
			operation = XOR;
		}
		else if (funct3 == 0b101 && funct7 == 0b0100000) { // SRA
			// cout << "found SRA" << endl;
			operation = SRA;
		}
		
	}
    else if (opcode == 0b0010011) { // ADDI or ANDI
		// TESTED WORKING        
        uint32_t funct3 = (instructionInt & FUNCT3_MASK) >> 12;
        int32_t imm = (instructionInt & IMM_MASK) >> 20;
		imm = (imm << 20) >> 20;

        if (funct3 == 0b000) {
            // cout << "found ADDI" << endl;
			operation = ADDI;
        } else if (funct3 == 0b111) {
            // cout << "found ANDI" << endl;
			operation = ANDI;
        }

		immediate = imm;
		// cout << "imm: " << imm << endl;
    }
	else if (opcode == 0b0000011) { // LW (the other instructions with this op code aren't included in this assignment, so we don't check funct3)
		// TESTED WORKING
        int32_t imm = static_cast<int32_t>(instructionInt & IMM_MASK) >> 20;

		// cout << "found LW" << endl;
		
		operation = LW;
		immediate = imm;
		// cout << "imm: " << immediate << endl;
	}
	else if (opcode == 0b0100011) { // SW (same explanation as LW)
		// TESTED WORKING
        const uint32_t IMM_MASK_PART1 = 0b1111111 << 25;  // leftmost 7 bits of immediate starting at bit 25
        const uint32_t IMM_MASK_PART2 = 0b11111 << 7;  // rightmost 5 bits of immediate

        int32_t imm = ((instructionInt & IMM_MASK_PART1) >> 25) | ((instructionInt & IMM_MASK_PART2) >> 7); // Assemble the immediate

		// cout << "found SW" << endl;

		operation = SW;
		immediate = imm;
	}
	else if (opcode == 0b1100111) { // JALR (same explanation as LW)
		// TESTED WORKING
        int32_t imm = static_cast<int32_t>(instructionInt & IMM_MASK) >> 20;

		// cout << "found JALR" << endl;
		// cout << "imm: " << imm << endl; 

		operation = JALR;
		immediate = imm;
	}
	else if (opcode == 0b1100011) { // BLT
		int32_t imm_12 = (instructionInt >> 31) & 0x1; // Sign bit
		int32_t imm_11 = (instructionInt >> 7) & 0x1;
		int32_t imm_10_5 = (instructionInt >> 25) & 0x3F;
		int32_t imm_4_1 = (instructionInt >> 8) & 0xF;
		
		int32_t imm = (imm_12 << 12) | (imm_11 << 11) | (imm_10_5 << 5) | (imm_4_1 << 1);

		if (imm_12) {
			imm |= 0xFFFFE000; // Sign-extend to 32 bits by setting the upper 20 bits
		}
		
		// cout << "found BLT" << endl;
		// cout << "imm: " << imm << endl; 

		operation = BLT;
		immediate = imm;
	}

	else if (opcode == 0) {
		// cout << "Opcode zero" << endl;
		operation = NOP;
	}

	// cout << "completing decode. immediate = " << immediate << endl;

    return true;
}

void CPU::executeCurrentInstruction() {
    // cout << "\nExecute current instruction" << endl;
    Operation curOperation = operation;

	// cout << "executing instruction. pc is now " << PC << endl;
	// cout << "the operation is " << curOperation << endl;
    switch (curOperation) {
        case ADD:
            aluResult = registerFile[sourceRegister1] + registerFile[sourceRegister2];
			// cout << "execute add op - set aluResult to " << aluResult << endl;
            break;
        case SUB:
            aluResult = registerFile[sourceRegister1] - registerFile[sourceRegister2];
            break;
        case ADDI:
            aluResult = registerFile[sourceRegister1] + immediate;
			// cout << "execute addi op - set aluResult to " << aluResult << endl;
			// cout << "registerFile[sourceRegister1]: " << registerFile[sourceRegister1] << endl;
			// cout << "immediate: " << immediate << endl;
            break;
        case XOR:
            aluResult = registerFile[sourceRegister1] ^ registerFile[sourceRegister2];
            break;
        case ANDI:
            aluResult = registerFile[sourceRegister1] & immediate;
            break;
        case SRA:
            aluResult = registerFile[sourceRegister1] >> registerFile[sourceRegister2]; // TODO: check to make sure it works
            break;
        case LW:
        case SW:
            // the ALU result simply stores the address we want to read from / write to, so this part is the same for LW and SW
			aluResult = registerFile[sourceRegister1] + immediate;
            break;
        case BLT:
            if (registerFile[sourceRegister1] < registerFile[sourceRegister2]) {
				PC += immediate - 4; // we subtract 4 because we add 4 separately regardless, so this is to counter that
				// cout << "less then condition TRUE, branching" << endl;
			}
			else {
				// cout << "not branching" << endl;
			}
			break;
        case JALR:
			registerFile[destinationRegister] = PC;
			// cout << "in JALR. updating destinationRegister to " << PC << endl;
			PC = registerFile[sourceRegister1] + immediate; // TODO: check to make sure 
			// cout << "registerFile[sourceRegister1]" << registerFile[sourceRegister1] << endl;
			// cout << "immediate" << immediate << endl;
            break;
        case NOP:
            break;
    }
}

void CPU::performMemoryOperations() {
	// only 2 operations for which we have to do memory stuff
	if (operation == LW) { // the alu result contains the memory address we want to load from. load that + 3 bytes because we want 32 bits (4 bytes)
		memoryData = dmemory[aluResult] |  dmemory[aluResult + 1] << 8 | dmemory[aluResult + 2] << 16 | dmemory[aluResult + 3] << 24;
	}
	else if (operation == SW) {
		// the contents of rs2 go to the memory address specified by rs1 plus the offset
		int32_t dataToWrite = registerFile[sourceRegister2];

		// cout << "writing " << dataToWrite << " to memory location " << aluResult << endl;

		dmemory[aluResult] = dataToWrite & 0xFF;
		dmemory[aluResult + 1] = (dataToWrite >> 8) & 0xFF;
		dmemory[aluResult + 2] = (dataToWrite >> 16) & 0xFF;
		dmemory[aluResult + 3] = (dataToWrite >> 24) & 0xFF;
	}
}

void CPU::performRegisterWriteback() {
	if (operation == NOP || operation == SW || operation == BLT || operation == JALR || destinationRegister == 0) {
		return; // x0 can't be written to
	}
	else if (operation == LW) {
		registerFile[destinationRegister] = memoryData;
		// cout << "updated register x" << destinationRegister << " to " << memoryData << endl;
	}
	else {
		registerFile[destinationRegister] = aluResult;
		// cout << "updated register x" << destinationRegister << " to " << aluResult << endl;
	}
}


unsigned long CPU::readPC()
{
	return PC;
}

