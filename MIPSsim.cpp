#include <iostream>
#include <bitset>         // std::bitset
#include <fstream>
#include <string>
#include <vector>
#include <utility>      // std::pair


using namespace std;


 /* On my honor, I have neither given nor received unauthorized aid on this assignment */
//Create a instruction struct to store data related to each instruction
class Instruc
{
    public:
        string print_instruc; //Name of the instructon that will be printed
        string name; //Store instruction name
        string instruction;//Store instruction
        string cat; //Store instruction category
        string op;  //store opcode
        int src1;   //store src1 index that will be accesed in the vector containing the 32 registers
        int src2;
        int dest;
        int pc;
        int branch;//Only used for instructions that require branching, except Jump
        int addrs; //array pos of of instruction address to be accessed ex: lw r5,144(r0)
        static int next_pc;

        Instruc()
        {
            next_pc+=4;
            pc = Instruc::next_pc;

        }




};

int Instruc::next_pc=60;
class Data:public Instruc
{
    public:
        string instruction;
        int val;
        int pc;

        Data()
        {
            pc = Instruc::next_pc;

        }

};


class Simulator
{   
    public:
        ofstream sim;
        string instruction;
        string format;
        string opcode;
        int pc_counter;
        int dest;
        int src1;
        int src2;
        string addrs;
        int instr_pos;  //Used to acces and traverse string
        string file_name;
        vector <Instruc*> instructions;  //Stores the instructions
        vector <int> regs; //Stores the values for the registers
        vector<Data*> data; //stores the data in memory that will be accesed using lw and sw
        int cycle; 

    Simulator(string file):file_name{file},regs(32)
    {
    
    
    }
    //A preprocessing step that populates the data vector and the instruction vectors with instruction structs that contain details pertaining to each instruction

    void set_instructions()
    {
    
        //open up file that contains instructions
        ifstream infile(file_name);
        
        //Read instructions from file while there are instructions to read
        //When the break instruction is reached stop taking instructions
        //Strat taking in data values
        
        while(infile>>instruction && instruction!="00111100000000000000000000001101")
        {
            reset();
            get_name_op(format);
            
            //Instruction is category 2, XOR MUL ADD SUB etc
            if(format == "010")
            {   //category 2 name instruction handelling

                //get opcode
                get_name_op(opcode);
                if(opcode == "000")
                {
                    save_cat2_inst("XOR");
                }
                else if(opcode == "001")
                {
                    save_cat2_inst("MUL");
                }
                else if(opcode == "010")
                {
                    save_cat2_inst("ADD");
                }
                else if(opcode == "011")
                {
                    save_cat2_inst("SUB");
                }
                else if(opcode == "100")
                {
                    save_cat2_inst("AND");
                }
                else if(opcode == "101")
                {
                    save_cat2_inst("OR");
                }
                else if(opcode == "110")
                {
                    //deal with unsigned nuber
                    save_cat2_inst("ADDU");
                }
                else if(opcode == "111")
                {
                    //deal with unsigned number
                    save_cat2_inst("SUBU");
                }

            }
            //category 1 instructions
            else if (format == "001")
            {
                //get opcode
                get_name_op(opcode);


                if (opcode == "000")
                {
                
                    Instruc *temp_instruc = new Instruc;
                    //temp_instruc->pc = pc_counter;
                    temp_instruc->cat = format;
                    temp_instruc->instruction = instruction;
                    temp_instruc->name = "NOP";
                    temp_instruc->op = opcode;
                    instructions.push_back(temp_instruc);
                
                }
                else if (opcode == "001")
                {
                    Instruc *temp_instruc = new Instruc;
                    //temp_instruc->pc = pc_counter;
                    temp_instruc->cat = format;
                    temp_instruc->instruction = instruction;
                    temp_instruc->name = "J";
                    //Transfor bit string into a bitset and then into an integer value to store in the address field
                    bitset<32> pc (temp_instruc->pc + 4);
                    bitset<32> temp_addrs (instruction.substr(6,26));
                    temp_addrs<<=2;

                    for(int i = 31;i>=28 ;i--)
                    {
                        temp_addrs[i] = pc[i];
                    }
                    
                    temp_instruc->addrs = to_int(temp_addrs);
                    instructions.push_back(temp_instruc);
                }
                else if (opcode == "010")
                {
                    save_inst_16("BEQ");

                }
                else if (opcode == "011")
                {

                    save_inst_16("BNE");
                }

                else if (opcode == "100")
                {

                    save_inst_16("BGTZ");
                }
                else if (opcode == "101")
                {

                    save_inst_16("SW");
                }
                else if (opcode == "110")
                {
                    save_inst_16("LW");
                }
            }

            //CATEGORY name 3 instructions
            else if (format == "100")
            {

                get_name_op(opcode);

                if(opcode == "000")
                {
                    save_inst_16("ORI");
                
                }
                else if(opcode == "001")
                {
                    save_inst_16("XORI");
                
                }
                else if(opcode == "010")
                {
                    save_inst_16("ADDI");
                
                }
                else if(opcode == "011")
                {
                    save_inst_16("SUBI");
                
                }
                else if(opcode == "100")
                {
                    save_inst_16("ANDI");
                
                }
                else if(opcode == "101")
                {
                    save_shif_amnt("SRL");
                
                }
            
                else if(opcode == "110")
                {
                    save_shif_amnt("SRA");
                }
                else if(opcode == "111")
                {
                    save_shif_amnt("SLL");
                
                }
            }
        }
        /////////////Save Break instruction into Instuction vector


        Instruc *temp_instruc = new Instruc;
        //Set category here since the category in format is related to the last instruction before the while loop broke
        format = "001";
        temp_instruc->cat ="001";
        temp_instruc->instruction = instruction;
        temp_instruc->name = "BREAK";
        instructions.push_back(temp_instruc);

        //Read the values in memory
        while(infile>>instruction)
        {
            bitset<32> temp_inst (instruction);
            Data *temp_data = new Data;
            temp_data->instruction = instruction;
            temp_data->val =to_int(temp_inst);

            data.push_back(temp_data);
        
        }
    }

    void simulate()
    {
        sim.open("simulation.txt");
        //location where data becomes accessible in memory
        //it's the spot where data starts becoming available, after 
        //the break command
        int pc_start = 64;
        int mem_start = data[0]->pc;
        cycle = 0;
        bool jump;
        int jump_addrs;
        //Simulate instructions while break instruction is not found
        for(int i =0; ;i++)
        {
            jump = false;
            jump_addrs = 0;
            cycle++;
            //Category one instruction simulation
            if(instructions[i]->name =="BREAK")
            {
                print_simulation(instructions[i]);   
                break;
            }

            if(instructions[i]->cat == "001")
            {
            
                if(instructions[i]->name == "NOP")
                {
                
                }
                else if(instructions[i]->name == "J")
                {   //substract the start of the PC (64) from the
                    //jump address and then divide by 4 to locate
                    //the index of the array, do -1 to offsett the i++
                    //that happens after each loop
                    //ex J 112 would do (112-64)/4 = 12-1 =11
                    jump = true;
                    jump_addrs = ((instructions[i]->addrs -pc_start)/4)-1;
             //       cout<<"JUMP INSTRUCTION: "<<i<<endl;
                }
                else if(instructions[i]->name == "BEQ")
                {
                    if(regs[instructions[i]->src1] == regs[instructions[i]->src2])
                    {
                        jump = true;
                        jump_addrs = ((instructions[i]->branch - pc_start)/4)-1;
                    
                 //       cout<<"BEQ INSTRUCTION: "<<i<<endl;
                    }
                
                }
                else if(instructions[i]->name == "BNE")
                {
                
                    if(regs[instructions[i]->src1] != regs[instructions[i]->src2])
                    {
                        jump = true;
                        jump_addrs = ((instructions[i]->branch - pc_start)/4)-1;
                 //       cout<<"BNE INSTRUCTION: "<<i<<endl;
                    
                    }
                
                
                }
                
            
                else if(instructions[i]->name == "BGTZ")
                {
                    
                    if(regs[instructions[i]->src1] > 0 )

                    {
                        jump = true;
                        jump_addrs = ((instructions[i]->branch - pc_start)/4)-1;
                        //cout<<"BGTZ INSTRUCTION: "<<i<<endl;
                    
                    }
                
                }
            
                else if(instructions[i]->name == "SW")
                {
                    //get index of the memory location where data will be saved
                    int index = (((instructions[i]->addrs + regs[instructions[i]->src1])-mem_start)/4);
                    data[index]->val = regs[instructions[i]->src2];    //save the data in register to memory address
                
                }
                else if(instructions[i]->name == "LW")
                {
                    //get index of the memory location where data will be saved
                    int index = (((instructions[i]->addrs + regs[instructions[i]->src1])-mem_start)/4);
                    regs[instructions[i]->src2] = data[index]->val;    //save the data in register to memory address
                
                }

            }
            else if(instructions[i]->cat == "010")
            {
                //XOR operation
                if(instructions[i]->name == "XOR")
                {
                    auto temp_xor = regs[instructions[i]->src1]^regs[instructions[i]->src2];

                    regs[instructions[i]->dest] = temp_xor;
                }
            
                else if(instructions[i]->name == "MUL")
                {
                
                    auto temp_mul = regs[instructions[i]->src1]*regs[instructions[i]->src2];

                    regs[instructions[i]->dest] = temp_mul;
                
                }
                else if(instructions[i]->name == "ADD")
                {
                
                    auto temp_add = regs[instructions[i]->src1]+regs[instructions[i]->src2];

                    regs[instructions[i]->dest] = temp_add;
                
                }
                else if(instructions[i]->name == "SUB")
                {
                
                    auto temp_sub = regs[instructions[i]->src1]-regs[instructions[i]->src2];

                    regs[instructions[i]->dest] = temp_sub;
                
                }
                else if(instructions[i]->name == "AND")
                {
                
                    auto temp_and = regs[instructions[i]->src1]&regs[instructions[i]->src2];

                    regs[instructions[i]->dest] = temp_and;
                
                }
                else if(instructions[i]->name == "OR")
                {
                
                    auto temp_or = regs[instructions[i]->src1]|regs[instructions[i]->src2];

                    regs[instructions[i]->dest] = temp_or;
                
                }
                else if(instructions[i]->name == "ADDU")
                {
                
                    auto temp_addu = regs[instructions[i]->src1]+regs[instructions[i]->src2];

                    regs[instructions[i]->dest] = temp_addu;
                
                }
                else if(instructions[i]->name == "SUBU")
                {
                
                    auto temp_subu = regs[instructions[i]->src1]-regs[instructions[i]->src2];

                    regs[instructions[i]->dest] = temp_subu;
                
                }
            }
            
            else if(instructions[i]->cat == "100")
            {
                
                if(instructions[i]->name == "ORI")
                {
                
                    auto temp_ori = regs[instructions[i]->src2]|instructions[i]->addrs;

                    regs[instructions[i]->src1] = temp_ori;
                
                }
                else if(instructions[i]->name == "XORI")
                {
                    auto temp_xori = regs[instructions[i]->src2]^instructions[i]->addrs;

                    regs[instructions[i]->src1] = temp_xori;
                
                }
                else if(instructions[i]->name == "ADDI")

                {
                    auto temp_addi = regs[instructions[i]->src2]+instructions[i]->addrs;


                    regs[instructions[i]->src1] = temp_addi;
                
                }

                else if(instructions[i]->name == "SUBI")

                {
                
                    auto temp_subi = regs[instructions[i]->src2]-instructions[i]->addrs;

                    regs[instructions[i]->src1] = temp_subi;
                
                }
                else if(instructions[i]->name == "ANDI")

                {
                
                    auto temp_andi = regs[instructions[i]->src2]&instructions[i]->addrs;

                    regs[instructions[i]->src1] = temp_andi;
                
                }
                else if(instructions[i]->name == "SRL")

                {
                
                    auto temp_srl = regs[instructions[i]->src2]>>instructions[i]->addrs;

                    regs[instructions[i]->src1] = temp_srl;
                
                }
                else if(instructions[i]->name == "SLL")

                {
                
                    auto temp_sll = regs[instructions[i]->src2]<<instructions[i]->addrs;

                    regs[instructions[i]->src1] = temp_sll;
                
                }

                ///ADD SRA
                else if(instructions[i]->name == "SRA")

                {   
                    //Ammount that we will loop
                    auto shift_amnt = instructions[i]->addrs;

                    //bitset with the value that will be shifted
                    bitset<32> inst_shift (regs[instructions[i]->src2]);

                    while(shift_amnt!=0)
                    {
                        if(inst_shift[31] == 1)
                        {
                            inst_shift = inst_shift>>1;
                            inst_shift[31] = 1;

                        }
                        else
                        {
                            inst_shift = inst_shift>>1;

                        }
                        shift_amnt--;
                    }
                
                    auto temp_sra = to_int(inst_shift);

                    regs[instructions[i]->src1] = temp_sra;
                
                }
                
            
            }
            if(jump)
            {
                
                print_simulation(instructions[i]);   
                i = jump_addrs;
            
            }
            else
            {
                print_simulation(instructions[i]);   
            }
        
        }

        //close the file where the simulation output is stored
        sim.close();
    
    }

    //Save the info for a instruction with 2 registers and a 16 bit address
    void save_inst_16(string instr_name)
    {
    
        Instruc *temp_instruc = new Instruc;
        //temp_instruc->pc = pc_counter;
        temp_instruc->cat = format;
        temp_instruc->instruction = instruction;
        temp_instruc->name = instr_name;
        //Get register values to retrieve numbers that will be compared to determine if there is equality
        get_reg(src1);
        temp_instruc->src1 = src1;
        //get the second instruction register

        get_reg(src2);
        temp_instruc->src2 = src2;
        //get 16 bit offset

        string offset = instruction.substr(16,16);

        if(offset[0]=='1' && (instr_name!="XORI" && instr_name!="ANDI" && instr_name!="ORI" && instr_name!="SRL" && instr_name!="SLL" && instr_name!="SRA"))
        {
            extend(offset);
        
        }
        bitset<32> temp_addrs (offset);
        temp_instruc->addrs = to_int(temp_addrs);

        if(instr_name == "BEQ" || instr_name == "BNE"|| instr_name == "BGTZ")
        {
            //used to find the branch location if true
            bitset <32> temp_pc (temp_instruc->pc);
            //Add 4 to PC to reach next pc 
            bitset <32> temp_add (4);
            temp_addrs=temp_addrs<<2;
            //Calculate branch address
            temp_instruc->branch = to_int(temp_pc)+to_int(temp_add)+to_int(temp_addrs);
        }
        instructions.push_back(temp_instruc);
    
    
    }
    //Save data for instructions that require shifting
    void save_shif_amnt(string instr_name)
    {
    
        Instruc *temp_instruc = new Instruc;
        //temp_instruc->pc = pc_counter;
        temp_instruc->cat = format;
        temp_instruc->instruction = instruction;
        temp_instruc->name = instr_name;
        //Get register values to retrieve numbers that will be compared to determine if there is equality
        get_reg(src1);
        temp_instruc->src1 = src1;
        //get the second instruction register

        get_reg(src2);
        temp_instruc->src2 = src2;
        //get 16 bit offset
        bitset<32> temp_addrs (instruction.substr(28,5));
        temp_instruc->addrs = to_int(temp_addrs);
        instructions.push_back(temp_instruc);
    
    }
    //sign extends a negative number
    void extend(string &extnd)
    {
        string ones;

        ones.append(16u,'1');

        //If 16 bit number is negative then sign extend the number so that
        //it can be converted into the appropriate signed integer
        //ex 111111111111111 would be expected to be -1 but if no sign extension
        //is done then the binary number will be transformed into an integer from 000000000000001111111111111
        
        if(extnd[0]=='1')
        {
            extnd=ones.append(extnd);
        
        }
    
    }

    //stores the name of the instruction in the instruction struct
    void save_cat2_inst(string instr_name)
    {
    
    
        Instruc *temp_instruc = new Instruc;
        temp_instruc->cat = format;
        temp_instruc->instruction = instruction;
        temp_instruc->name = instr_name;
        temp_instruc->op = opcode;
        //get destination register
        get_reg(dest);
        temp_instruc->dest = dest;
        //Get src1 from instruction and turn it into an integer value
        //this value will be the location of the register array that 
        //we will access to get the values we will perform
        //operations on
        get_reg(src1);
        temp_instruc->src1 = src1;
        //get the second instruction register

        get_reg(src2);
        temp_instruc->src2 = src2;
        instructions.push_back(temp_instruc);
    
    }
    

    //reset variables used to parse and analyze instruction so as to reuse them with the new incoming instructions
    void reset()
    {
        //reset instruction format
        format="";
        //Reset position of where the string will be accessed
        instr_pos = 0;

        opcode="";

        dest=0;
        
        src1=0;

        src2=0;

        addrs="";

    }
    void print_data()
    {
        sim.open("disassembly.txt");

        string print; 

        for(Instruc* i: instructions)

        {
            sim<< i->instruction<<"\t"<< i->pc<<"\t";

            //Category nanme
            if(i->cat == "001")
            {
                if(i->name == "NOP")
                {
                    print = i->name;
                    i->print_instruc = print;
                    sim<<print<<"\n";
                }
                else if (i->name == "J")
                {
                    print = i->name+" #"+to_string(i->addrs);
                    i->print_instruc = print;
                    sim<<i->name<<" #"<<i->addrs<<"\n";
                }
                else if (i->name == "BREAK")
                {
                    print = i->name;
                    i->print_instruc = print;
                    sim<<i->name<<"\n";
                
                }
                else if (i->name == "SW" ||i->name == "LW")
                {
                
                    print = i->name+" R"+to_string(i->src2)+", "+to_string(i->addrs)+"(R"+to_string(i->src1)+")";
                    i->print_instruc = print;
                    sim<<i->name<<" R"<<i->src2<<", "<<i->addrs<<"(R"<<i->src1<<")"<<"\n";
                }
                else if (i->name == "BEQ" ||i->name == "BNE")
                {
                    print = i->name+" R"+to_string(i->src1)+", "+"R"+to_string(i->src2)+", #"+to_string(i->addrs);
                    i->print_instruc = print;
                    sim<<i->name<<" R"<<i->src1<<", "<<"R"<<i->src2<<", #"<<i->addrs<<"\n";
                }
                else if(i->name == "BGTZ")
                {
                    
                    print = i->name+" R"+to_string(i->src1)+", #"+to_string(i->addrs);
                    i->print_instruc = print;
                    sim<<i->name<<" R"<<i->src1<<", #"<<i->addrs<<"\n";
                
                }
            }
            else if(i->cat == "010")
            {
            
                print = i->name+" R"+to_string(i->dest)+", R"+to_string(i->src1)+", R"+to_string(i->src2);
                i->print_instruc = print;
                sim<<i->name<<" R"<<i->dest<<", R"<<i->src1<<", R"<<i->src2<<"\n";
            
            }
            else if (i->cat == "100")
            {
                print = i->name+" R"+to_string(i->src1)+", "+"R"+to_string(i->src2)+", #"+to_string(i->addrs);
                i->print_instruc = print;
                sim<<i->name<<" R"<<i->src1<<", "<<"R"<<i->src2<<", #"<<i->addrs<<"\n";

            }
        
        }
        for(Data *x: data)
        {
            sim<<x->instruction<<"\t"<<x->pc<<"\t"<<x->val<<"\n";

        } 
        sim.close();
        sim.clear();

    }

    void print_simulation(Instruc* i)
    {
        //simassembly file will be created here
        
        sim<<"\n--------------------"<<"\n";
        sim<<"Cycle "<<cycle<<":\t"<<i->pc<<"\t"<< i->print_instruc<<"\n";
        sim<<"\n";
        sim<<"Registers"<<"\n";
        sim<<"R00:\t";
        for(int j = 0; j < 8;j++)
        {
            sim<<regs[j]<<"\t";
        
        }
        sim<<"\n";
        sim<<"R08:\t";
        for(int j = 8; j < 16;j++)
        {
            sim<<regs[j]<<"\t";
        
        }
        sim<<"\n";
        sim<<"R16:\t";
        for(int j = 16; j < 24;j++)
        {
            sim<<regs[j]<<"\t";
        
        }
        sim<<"\n";
    
        sim<<"R24:\t";
        for(int j = 24; j < 32;j++)
        {
            sim<<regs[j]<<"\t";
        
        }
        sim<<"\n";
        sim<<"\nData"<<"\n";
        for(int j = 0;j < data.size();j++)
        {
           if(j % 8 == 0 && j!= 0 )
           {
               sim<<"\n"<<data[j]->pc<<":\t";
           } 
           if(j % 8 == 0 && j==0)
           {
               sim<<data[j]->pc<<":\t";
           } 
            sim<<data[j]->val<<"\t";

        
        }
        sim<<"\n";

    
    }
    
    
    //used to get bits pertaining to the register being used
    void get_reg(int &reg)

    {
        bitset<32> temp_reg(instruction.substr(instr_pos,5));
        reg+=to_int(temp_reg);
        instr_pos+=5;
        
    
    }
    //used to get instruction name and opcode
    void get_name_op(string &format)
    {
    
        //grab 3 leftmost bits to determine name of instructions
            format+=instruction.substr(instr_pos,3);
            //instruction location after finding name
            instr_pos+=3;
    
    }

    //handle instructions of category 2 name
    void cat_two()
    {


    }


    auto to_int(bitset<32> set) -> int
    {
        return static_cast<int>(set.to_ulong());
    
    }



};




int main (int argc, char *argv[])
{
    if(argc !=2)
    {
        cout<<"ERROR in file input"<<endl;
    
    }
    else
    {
        //Take in filename
        string file_name = argv[1];
        //Create an instance of simulator
        Simulator mips(file_name);
        mips.set_instructions();
        mips.print_data();
        mips.simulate();
    }


    return 0;
}
