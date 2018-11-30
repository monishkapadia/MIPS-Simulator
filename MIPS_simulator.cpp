//============================================================================
// Name        : Lab1_Pipelining.cpp
// Author      : monish
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include<iostream>
#include<string>
#include<vector>
#include<bitset>
#include<fstream>
using namespace std;
#define MemSize 256 // memory size, in reality, the memory size should be 2^32, but for this lab, for the space resaon, we keep it as this large number, but the memory is still 32-bit addressable.

struct IFStruct {
    bitset<32>  PC;
    bool        nop;
};

struct IDStruct {
    bitset<32>  Instr;
    bool        nop;
};

struct EXStruct {
    bitset<32>  Read_data1;
    bitset<32>  Read_data2;
    bitset<16>  Imm;
    bitset<5>   Rs;
    bitset<5>   Rt;
    bitset<5>   Wrt_reg_addr;
    bool        is_I_type;
    bool        rd_mem;
    bool        wrt_mem;
    bool        alu_op;     //1 for addu, lw, sw, 0 for subu
    bool        wrt_enable;
    bool        nop;
};

struct MEMStruct {
    bitset<32>  ALUresult;
    bitset<32>  Store_data;
    bitset<5>   Rs;
    bitset<5>   Rt;
    bitset<5>   Wrt_reg_addr;
    bool        rd_mem;
    bool        wrt_mem;
    bool        wrt_enable;
    bool        nop;
};

struct WBStruct {
    bitset<32>  Wrt_data;
    bitset<5>   Rs;
    bitset<5>   Rt;
    bitset<5>   Wrt_reg_addr;
    bool        wrt_enable;
    bool        nop;
};

struct stateStruct {
    IFStruct    IF;
    IDStruct    ID;
    EXStruct    EX;
    MEMStruct   MEM;
    WBStruct    WB;
};

class RF
{
    public:
        bitset<32> Reg_data;
     	RF()
    	{
			Registers.resize(32);
			Registers[0] = bitset<32> (0);
        }

        bitset<32> readRF(bitset<5> Reg_addr)
        {
            Reg_data = Registers[Reg_addr.to_ulong()];
            return Reg_data;
        }

        void writeRF(bitset<5> Reg_addr, bitset<32> Wrt_reg_data)
        {
            Registers[Reg_addr.to_ulong()] = Wrt_reg_data;
        }

		void outputRF()
		{
			ofstream rfout;
			rfout.open("RFresult.txt",std::ios_base::app);
			if (rfout.is_open())
			{
				rfout<<"State of RF:\t"<<endl;
				for (int j = 0; j<32; j++)
				{
					rfout << Registers[j]<<endl;
				}
			}
			else cout<<"Unable to open file";
			rfout.close();
		}

	private:
		vector<bitset<32> >Registers;
};

class INSMem
{
	public:
        bitset<32> Instruction;
        INSMem()
        {
			IMem.resize(MemSize);
            ifstream imem;
			string line;
			int i=0;
			imem.open("imem.txt");
			if (imem.is_open())
			{
				while (getline(imem,line))
				{
					IMem[i] = bitset<8>(line);
					i++;
				}
			}
            else cout<<"Unable to open file";
			imem.close();
		}

		bitset<32> readInstr(bitset<32> ReadAddress)
		{
			string insmem;
			insmem.append(IMem[ReadAddress.to_ulong()].to_string());
			insmem.append(IMem[ReadAddress.to_ulong()+1].to_string());
			insmem.append(IMem[ReadAddress.to_ulong()+2].to_string());
			insmem.append(IMem[ReadAddress.to_ulong()+3].to_string());
			Instruction = bitset<32>(insmem);		//read instruction memory
			return Instruction;
		}

    private:
        vector<bitset<8> > IMem;
};

class DataMem
{
    public:
        bitset<32> ReadData;
        DataMem()
        {
            DMem.resize(MemSize);
            ifstream dmem;
            string line;
            int i=0;
            dmem.open("dmem.txt");
            if (dmem.is_open())
            {
                while (getline(dmem,line))
                {
                    DMem[i] = bitset<8>(line);
                    i++;
                }
            }
            else cout<<"Unable to open file";
                dmem.close();
        }

        bitset<32> readDataMem(bitset<32> Address)
        {
			string datamem;
            datamem.append(DMem[Address.to_ulong()].to_string());
            datamem.append(DMem[Address.to_ulong()+1].to_string());
            datamem.append(DMem[Address.to_ulong()+2].to_string());
            datamem.append(DMem[Address.to_ulong()+3].to_string());
            ReadData = bitset<32>(datamem);		//read data memory
            return ReadData;
		}

        void writeDataMem(bitset<32> Address, bitset<32> WriteData)
        {
            DMem[Address.to_ulong()] = bitset<8>(WriteData.to_string().substr(0,8));
            DMem[Address.to_ulong()+1] = bitset<8>(WriteData.to_string().substr(8,8));
            DMem[Address.to_ulong()+2] = bitset<8>(WriteData.to_string().substr(16,8));
            DMem[Address.to_ulong()+3] = bitset<8>(WriteData.to_string().substr(24,8));
        }

        void outputDataMem()
        {
            ofstream dmemout;
            dmemout.open("dmemresult.txt");
            if (dmemout.is_open())
            {
                for (int j = 0; j< 1000; j++)
                {
                    dmemout << DMem[j]<<endl;
                }

            }
            else cout<<"Unable to open file";
            dmemout.close();
        }

    private:
		vector<bitset<8> > DMem;
};

void printState(stateStruct state, int cycle)
{
    ofstream printstate;
    printstate.open("stateresult.txt", std::ios_base::app);
    if (printstate.is_open())
    {
        printstate<<"State after executing cycle:\t"<<cycle<<endl;

        printstate<<"IF.PC:\t"<<state.IF.PC.to_ulong()<<endl;
        printstate<<"IF.nop:\t"<<state.IF.nop<<endl;

        printstate<<"ID.Instr:\t"<<state.ID.Instr<<endl;
        printstate<<"ID.nop:\t"<<state.ID.nop<<endl;

        printstate<<"EX.Read_data1:\t"<<state.EX.Read_data1<<endl;
        printstate<<"EX.Read_data2:\t"<<state.EX.Read_data2<<endl;
        printstate<<"EX.Imm:\t"<<state.EX.Imm<<endl;
        printstate<<"EX.Rs:\t"<<state.EX.Rs<<endl;
        printstate<<"EX.Rt:\t"<<state.EX.Rt<<endl;
        printstate<<"EX.Wrt_reg_addr:\t"<<state.EX.Wrt_reg_addr<<endl;
        printstate<<"EX.is_I_type:\t"<<state.EX.is_I_type<<endl;
        printstate<<"EX.rd_mem:\t"<<state.EX.rd_mem<<endl;
        printstate<<"EX.wrt_mem:\t"<<state.EX.wrt_mem<<endl;
        printstate<<"EX.alu_op:\t"<<state.EX.alu_op<<endl;
        printstate<<"EX.wrt_enable:\t"<<state.EX.wrt_enable<<endl;
        printstate<<"EX.nop:\t"<<state.EX.nop<<endl;

        printstate<<"MEM.ALUresult:\t"<<state.MEM.ALUresult<<endl;
        printstate<<"MEM.Store_data:\t"<<state.MEM.Store_data<<endl;
        printstate<<"MEM.Rs:\t"<<state.MEM.Rs<<endl;
        printstate<<"MEM.Rt:\t"<<state.MEM.Rt<<endl;
        printstate<<"MEM.Wrt_reg_addr:\t"<<state.MEM.Wrt_reg_addr<<endl;
        printstate<<"MEM.rd_mem:\t"<<state.MEM.rd_mem<<endl;
        printstate<<"MEM.wrt_mem:\t"<<state.MEM.wrt_mem<<endl;
        printstate<<"MEM.wrt_enable:\t"<<state.MEM.wrt_enable<<endl;
        printstate<<"MEM.nop:\t"<<state.MEM.nop<<endl;

        printstate<<"WB.Wrt_data:\t"<<state.WB.Wrt_data<<endl;
        printstate<<"WB.Rs:\t"<<state.WB.Rs<<endl;
        printstate<<"WB.Rt:\t"<<state.WB.Rt<<endl;
        printstate<<"WB.Wrt_reg_addr:\t"<<state.WB.Wrt_reg_addr<<endl;
        printstate<<"WB.wrt_enable:\t"<<state.WB.wrt_enable<<endl;
        printstate<<"WB.nop:\t"<<state.WB.nop<<endl;
    }
    else cout<<"Unable to open file";
    printstate.close();
}

unsigned long shiftbits(bitset<32> inst, int start)
{
    return ((inst.to_ulong())>>start);
}

bitset<32> signextend (bitset<16> imm)
{
    string sestring;
    if (imm[15]==0){
        sestring = "0000000000000000"+imm.to_string<char,std::string::traits_type,std::string::allocator_type>();
    }
    else{
        sestring = "1111111111111111"+imm.to_string<char,std::string::traits_type,std::string::allocator_type>();
    }
    return (bitset<32> (sestring));

}
bitset<32> branchAddress (bitset<16> imm)
{
    string sestring;
    if (imm[15]==0){
        sestring = "00000000000000"+imm.to_string<char,std::string::traits_type,std::string::allocator_type>()+"00";
    }
    else{
        sestring = "11111111111111"+imm.to_string<char,std::string::traits_type,std::string::allocator_type>()+"00";
    }
    return (bitset<32> (sestring));

}
int main()
{
	int cycle = 0;
	bitset<32> halt;

	bitset<6> opcode;
    bitset<6> funct;
    bitset<16> imm;
	bool RType;
	bool IType;
	bool IsLoad;
	bool IsStore;
	bool IsBranch;
	bool WrtEnable;
	bool stall=0;
	bool prevStall=0;

    bitset<5> RReg1;
    bitset<5> RReg2;

    bitset<32> signext;

    bitset<32> branchAddr;
    RF myRF;
    INSMem myInsMem;
    DataMem myDataMem;
    stateStruct state, newState;

    //Initialization
    state.IF.PC = bitset<32> (0);

    state.IF.nop = 0;
    state.ID.nop = 1;
    state.EX.nop = 1;
    state.MEM.nop = 1;
    state.WB.nop = 1;

    state.EX.alu_op = 1;
    state.EX.is_I_type = 0;
    state.EX.rd_mem = 0;
    state.EX.wrt_enable = 0;
    state.EX.wrt_mem = 0;

    state.MEM.rd_mem = 0;
    state.MEM.wrt_enable = 0;
    state.MEM.wrt_mem = 0;

    state.WB.wrt_enable = 0;

    halt.set();

    while (1) {

        /* --------------------- WB stage --------------------- */
    	if(state.WB.nop == 0){
    		// If writing back to the register
    	    if(state.WB.wrt_enable){
    	    	myRF.writeRF(state.WB.Wrt_reg_addr, state.WB.Wrt_data);
    	    }
    	}


        /* --------------------- MEM stage --------------------- */
    	if(state.MEM.nop == 0){
    		// Check whether it is writing or reading for mem, if not then just forward the result to WB stage
    		//If load-store then do mem-mem
    		if(state.MEM.wrt_mem && (state.MEM.Wrt_reg_addr == state.WB.Wrt_reg_addr)){
    			//cout << "MEM write at: " << state.MEM.ALUresult.to_ulong() << endl;
    			myDataMem.writeDataMem(state.MEM.ALUresult, state.WB.Wrt_data);
    		}
    		// write to dmem
    		else if(state.MEM.wrt_mem){
    			//cout << "MEM write at: " << state.MEM.ALUresult.to_ulong() << endl;
    			myDataMem.writeDataMem(state.MEM.ALUresult, state.MEM.Store_data);
    		}
    		// read from dmem
    		else if(state.MEM.rd_mem){
    			newState.WB.Wrt_data = myDataMem.readDataMem(state.MEM.ALUresult);
    		}
    		// else forward the value in case dmem is not involved
    		else {
    			newState.WB.Wrt_data = state.MEM.ALUresult;
    		}

    		// Forward the values to WB stage
    		newState.WB.Rs = state.MEM.Rs;
    		newState.WB.Rt = state.MEM.Rt;
    		newState.WB.Wrt_reg_addr = state.MEM.Wrt_reg_addr;
    		newState.WB.wrt_enable = state.MEM.wrt_enable;
    		newState.WB.nop = 0;
    	} else if(state.MEM.nop == 1){
    		newState.WB.nop = 1; // To send nop after halt
    	}


        /* --------------------- EX stage --------------------- */
    	if(state.EX.nop == 0){
    		// Sign extend the last bit of Imm
    		signext = signextend (state.EX.Imm);
    		// Forward the values to MEM
    		// Check if I type or R-type add/sub function
    		newState.MEM.Store_data = state.EX.Read_data2;
    		newState.MEM.Rs = state.EX.Rs;
    		newState.MEM.Rt = state.EX.Rt;
    		newState.MEM.Wrt_reg_addr = state.EX.Wrt_reg_addr;
    		newState.MEM.rd_mem = state.EX.rd_mem;
    		newState.MEM.wrt_mem = state.EX.wrt_mem;
    		newState.MEM.wrt_enable = state.EX.wrt_enable;
    		newState.MEM.nop = 0;

    		// check for mem-ex or ex-ex forwarding
    		// mem-ex forwarding -> for Rs
    		if(prevStall){
    			if(state.WB.Wrt_reg_addr == state.EX.Rs && state.WB.wrt_enable){
    				newState.EX.Read_data1 = state.WB.Wrt_data;
    			} else if(state.WB.Wrt_reg_addr == state.EX.Rt && state.WB.wrt_enable){
    				newState.EX.Read_data2 = state.WB.Wrt_data;
    			}
    			newState.MEM = state.MEM;
    			newState.MEM.nop = 1;
    		}
    		else if (state.WB.Wrt_reg_addr == state.EX.Rs){
				newState.MEM.ALUresult = (state.EX.is_I_type == 1)?(bitset<32> (state.WB.Wrt_data.to_ulong() + signext.to_ulong())):((state.EX.alu_op == 1)?(bitset<32> (state.WB.Wrt_data.to_ulong() + state.EX.Read_data2.to_ulong())):(bitset<32> (state.WB.Wrt_data.to_ulong() - state.EX.Read_data2.to_ulong())));
			}
    		// mem-ex forwarding -> for Rt
    		else if (state.WB.Wrt_reg_addr == state.EX.Rt){
				newState.MEM.ALUresult = (state.EX.is_I_type == 1)?(bitset<32> (state.EX.Read_data1.to_ulong() + signext.to_ulong())):((state.EX.alu_op == 1)?(bitset<32> (state.EX.Read_data1.to_ulong() + state.WB.Wrt_data.to_ulong())):(bitset<32> (state.EX.Read_data1.to_ulong() - state.WB.Wrt_data.to_ulong())));
			}
    		// ex-ex forwarding -> for Rs
    		else if(state.MEM.Wrt_reg_addr == state.EX.Rs){
    			newState.MEM.ALUresult = (state.EX.is_I_type == 1)?(bitset<32> (state.MEM.ALUresult.to_ulong() + signext.to_ulong())):((state.EX.alu_op == 1)?(bitset<32> (state.MEM.ALUresult.to_ulong() + state.EX.Read_data2.to_ulong())):(bitset<32> (state.MEM.ALUresult.to_ulong() - state.EX.Read_data2.to_ulong())));
    		}
    		// ex-ex forwarding -> for Rt
    		else if (state.MEM.Wrt_reg_addr == state.EX.Rt){
        		newState.MEM.ALUresult = (state.EX.is_I_type == 1)?(bitset<32> (state.EX.Read_data1.to_ulong() + signext.to_ulong())):((state.EX.alu_op == 1)?(bitset<32> (state.EX.Read_data1.to_ulong() + state.MEM.ALUresult.to_ulong())):(bitset<32> (state.EX.Read_data1.to_ulong() - state.MEM.ALUresult.to_ulong())));
    		}
    		// else normally compute
    		else {
        		newState.MEM.ALUresult = (state.EX.is_I_type == 1)?(bitset<32> (state.EX.Read_data1.to_ulong() + signext.to_ulong())):((state.EX.alu_op == 1)?(bitset<32> (state.EX.Read_data1.to_ulong() + state.EX.Read_data2.to_ulong())):(bitset<32> (state.EX.Read_data1.to_ulong() - state.EX.Read_data2.to_ulong())));
    		}

    	}else if(state.EX.nop == 1){
    		newState.MEM.nop = 1; // To send nop after halt
    	}
    	// To avoid garbage in loops where few state bool's are not passed
    	if(state.EX.nop == 1 && state.WB.nop == 1){
    		newState.EX.alu_op = state.EX.alu_op;
    		newState.EX.is_I_type = state.EX.is_I_type;
    		newState.EX.rd_mem = state.EX.rd_mem;
    		newState.EX.wrt_enable = state.EX.wrt_enable;
    		newState.EX.wrt_mem = state.EX.wrt_mem;

    		newState.MEM.rd_mem = state.MEM.rd_mem;
    		newState.MEM.wrt_enable = state.MEM.wrt_enable;
    		newState.MEM.wrt_mem = state.MEM.wrt_mem;

    		newState.WB.wrt_enable = state.WB.wrt_enable;
    	}

        /* --------------------- ID stage --------------------- */
    	if(state.ID.nop == 0){
    		// Determining the opcode, type, load/store, wrt_enable bit
    		opcode = bitset<6> (shiftbits(state.ID.Instr, 26));
    		RType = (opcode.to_ulong()==0)?1:0;
    		IType = (opcode.to_ulong()!=0 && opcode.to_ulong()!=2)?1:0;
    		IsLoad = (opcode.to_ulong()==35)?1:0;
    		IsStore = (opcode.to_ulong()==43)?1:0;
    		IsBranch = (opcode.to_ulong()==4)?1:0;
    		WrtEnable = (IsStore || IsBranch)?0:1;

    		funct = bitset<6> (shiftbits(state.ID.Instr, 0));
    		RReg1 = bitset<5> (shiftbits(state.ID.Instr, 21));
    		RReg2 = bitset<5> (shiftbits(state.ID.Instr, 16));

    		if(IsBranch && (myRF.readRF(RReg1) == myRF.readRF(RReg2))){
    			newState.EX.nop = 1;
    			newState.ID.nop = 1;
    			branchAddr = branchAddress(bitset<16> (shiftbits(state.ID.Instr, 0)));
    		}
    		/* check stalling by checking:
    		 * 1. prev instr to be IType
    		 * 2. prev register to equal to current register
    		 * 3. Rtype i.e add or sub
    		 * 4. to avoid infinite loop -> check that EX.nop is 0
    		*/
    		if(state.EX.rd_mem && (RReg2 == state.EX.Rt || RReg1 == state.EX.Rt) && RType && !state.EX.nop){
    			stall = 1;
    		}
    		// Forwarding to EX stage
    		if(!prevStall){
				newState.EX.Read_data1 = myRF.readRF(RReg1);
				newState.EX.Read_data2 = myRF.readRF(RReg2);
				newState.EX.Imm = bitset<16> (shiftbits(state.ID.Instr, 0));
				newState.EX.Rs = RReg1;
				newState.EX.Rt = RReg2;
				newState.EX.Wrt_reg_addr = (IType == 1)? RReg2 : bitset<5> (shiftbits(state.ID.Instr, 11));
				newState.EX.is_I_type = IType;
				newState.EX.rd_mem = IsLoad;
				newState.EX.wrt_mem = IsStore;
				newState.EX.alu_op = (funct.to_ulong() == 35)?0:1;
				newState.EX.wrt_enable = WrtEnable;
				newState.EX.nop = 0;
    		}
    		// if stall bit received
    		if(stall){
				newState.ID = state.ID;
			}

    	}else if(state.ID.nop == 1){
    		newState.EX.nop = 1; // To send nop after halt
    	}



        /* --------------------- IF stage --------------------- */
    	if(state.IF.nop == 0){
    		// Forward the instruction to next stage
    		if(IsBranch && (myRF.readRF(RReg1) == myRF.readRF(RReg2))) {
    			newState.ID.Instr = bitset<32> (myInsMem.readInstr(branchAddr));
    			newState.ID.nop = 0;
    		}
    		else {
    			if(prevStall){
    				newState.ID.Instr = bitset<32> (myInsMem.readInstr(bitset<32> (state.IF.PC.to_ulong() - 4)));
    			} else {
    				newState.ID.Instr = bitset<32> (myInsMem.readInstr(state.IF.PC));
    			}
    			newState.IF.nop = 0;
				newState.ID.nop = 0;
				if(newState.ID.Instr == halt){
					state.IF.nop = 1; // To send nop after halt
					newState.IF.nop = 1; // To send nop after halt
					newState.ID.nop = 1; // To send nop after halt
				} else if (prevStall){
					newState.IF.PC = bitset<32> (state.IF.PC.to_ulong());
				} else {
					newState.IF.PC = bitset<32> (state.IF.PC.to_ulong() + 4); // If no halt signal then increment PC
				}
			}
    		prevStall = stall;
    		stall = 0;
    	} else if(state.IF.nop == 1){
    		newState.ID.nop = 1; // To send nop after halt
    	}

    	// IF all NOP then break the loop
        if (state.IF.nop && state.ID.nop && state.EX.nop && state.MEM.nop && state.WB.nop)
            break;
        // print the values in a text file
        printState(newState, cycle); //print states after executing cycle 0, cycle 1, cycle 2 ...
        // copy newState to state for next loop
        state = newState; /*The end of the cycle and updates the current state with the values calculated in this cycle */
        cycle++; // Increment the cycle
    }

    myRF.outputRF(); // dump RF;
	myDataMem.outputDataMem(); // dump data mem
	return 0;
}
