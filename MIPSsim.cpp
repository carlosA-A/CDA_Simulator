#include <iostream>
#include <bitset>         // std::bitset
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <utility>      // std::pair


using namespace std;


//Create a instruction struct to store data related to each instruction
class Instruc
{
    public:

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


    Simulator(string file):file_name{file},regs(32)
    {}
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
                    bitset<32> pc (120);
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
        //temp_instruc->pc = pc_counter;
        temp_instruc->cat = format;
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
        //location where data becomes accessible in memory
        //it's the spot where data starts becoming available, after 
        //the break command
        int pc_start = 64;
        
        //Simulate instructions while break instruction is not found
        for(int i =0;instructions[i]->name !="BREAK";i++)
        {
            //Category one instruction simulation
            if(instructions[i]->cat == "001")
            {
            
                if(instructions[i]->name == "NOP")
                {
                    continue;
                
                }
                else if(instructions[i]->name == "J")
                {   //substract the start of the PC (64) from the
                    //jump address and then divide by 4 to locate
                    //the index of the array, do -1 to offsett the i++
                    //that happens after each loop
                    //ex J 112 would do (112-64)/4 = 12-1 =11
                    i = ((instructions[i]->addrs -pc_start)/4)-1;
                }
                else if(instructions[i]->name == "BEQ")
                {
                    
                
                }
            
            
            }
            
        
        }
    
    }

    //return the index where the jump/branch instruction must go
    //auto find_index(int offset) -> decltype(offset)
    //{
    //    for(int i = 0; i < instructions.size(); i++)
    //    {
    //        if(instructions[i]->pc == offset)
    //    
    //    }
    //
    //}






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

        if(offset[0]=='1' && instr_name!="XORI")
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
            temp_addrs<<=2;
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
        for(Instruc* i: instructions)

        {
            cout<< i->instruction<<"  "<< i->pc<<"  ";

            //Category nanme
            if(i->cat == "001")
            {
                if(i->name == "NOP")
                {
                    cout<<i->name<<endl;
                }
                else if (i->name == "J")
                {
                    cout<<i->name<<" "<<i->addrs<<endl;
                }
                else if (i->name == "BREAK")
                {
                    cout<<i->name<<endl;
                
                }
                else if (i->name == "SW" ||i->name == "LW")
                {
                
                    cout<<i->name<<" R"<<i->src2<<", "<<i->addrs<<"(R"<<i->src1<<")"<<endl;
                }
                else if (i->name == "BEQ" ||i->name == "BNE")
                {
                    cout<<i->name<<" R"<<i->src1<<", "<<"R"<<i->src2<<", #"<<i->addrs<<endl;
                }
                else if(i->name == "BGTZ")
                {
                    
                    cout<<i->name<<" R"<<i->src1<<", #"<<i->addrs<<endl;
                
                }
            }
            else if(i->cat == "010")
            {
            
                cout<<i->name<<" R"<<i->dest<<", R"<<i->src1<<", R"<<i->src2<<endl;
            
            }
            else if (i->cat == "100")
            {
                
                cout<<i->name<<" R"<<i->src1<<", "<<"R"<<i->src2<<", #"<<i->addrs<<endl;

            }
        
        }
        for(Data *x: data)
        {
            cout<<x->instruction<<"  "<<x->pc<<"  "<<x->val<<endl;

        } 
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




int main ()
{
    string file_name;
    cin>>file_name;
    Simulator mips(file_name);
    mips.set_instructions();
    mips.print_data();

    return 0;
}
