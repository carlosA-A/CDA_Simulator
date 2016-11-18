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

        string type; //Store instruction type
        string op;  //store opcode
        int src1;   //store src1 index that will be accesed in the vector containing the 32 registers
        int src2;
        int dest;
        static int pc;
        int addrs; //array pos of of instruction address to be accessed ex: lw r5,144(r0)
    Instruc()
    {
        pc+=4;
    }




};

int Instruc::pc = 60;

class Simulator
{   
    public:
        string instruction;
        string format;
        string opcode;
        int dest;
        int src1;
        int src2;
        string addrs;
        int instr_pos;  //Used to acces and traverse string
        string file_name;
        vector <Instruc*> instructions;  //Stores the instructions
        vector <int> regs; //Stores the values for the registers
        vector<int> data; //stores the data in memory that will be accesed using lw and sw


    Simulator(string file):file_name{file}{}
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
            get_type_op(format);
            
            //Instruction is category 2, XOR MUL ADD SUB etc
            if(format == "010")
            {   //category 2 type instruction handelling

                //get opcode
                get_type_op(opcode);
                cout<<"format 1 "<<format<< "OPCODE 1 "<<opcode<<endl;
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
                    save_cat2_inst("ADDU");
                }
                else if(opcode == "111")
                {
                    save_cat2_inst("SUBU");
                }

            }
            //category 1 instructions
            else if (format == "001")
            {
                //get opcode
                get_type_op(opcode);

                cout<< "format 2 "<<format<<"OPCODE 2 "<<opcode<<endl;

                if (opcode == "000")
                {
                
                    Instruc *temp_instruc = new Instruc;
                    temp_instruc->type = "NOP";
                    temp_instruc->op = opcode;
                    instructions.push_back(temp_instruc);
                
                }
                else if (opcode == "001")
                {
                    Instruc *temp_instruc = new Instruc;
                    temp_instruc->type = "J";
                    //Transfor bit string into a bitset and then into an integer value to store in the address field
                    bitset<32> temp_addrs (instruction.substr(7,26));
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

            //CATEGORY TYPE 3 instructions
            else if (format == "100")
            {

                get_type_op(opcode);
                cout<< "format 3 "<<format<<" OPCODE 3 "<<opcode<<endl;

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
                    save_inst_16("SRL");
                
                }
            
                else if(opcode == "110")
                {
                    save_inst_16("SRA");
                
                }
                else if(opcode == "111")
                {
                    save_inst_16("SLL");
                
                }
            }
        }


        /////////////Save Break instruction insto a struct!!!
    
        Instruc *temp_instruc = new Instruc;
        temp_instruc->type = "BREAK";
        instructions.push_back(temp_instruc);

        //Read the values in memory
        while(infile>>instruction)
        {
            bitset<32> temp_data (instruction);
            data.push_back(to_int(temp_data));
        
        }
    }
    //Save the info for a instruction with 2 registers and a 16 bit address
    void save_inst_16(string instr_name)
    {
    
        Instruc *temp_instruc = new Instruc;
        temp_instruc->type = instr_name;
        //Get register values to retrieve numbers that will be compared to determine if there is equality
        get_reg(src1);
        temp_instruc->src1 = src1;
        //get the second instruction register

        get_reg(src2);
        temp_instruc->src2 = src2;
        //get 16 bit offset
        bitset<32> temp_addrs (instruction.substr(7,16));
        temp_instruc->addrs = to_int(temp_addrs);
        instructions.push_back(temp_instruc);
    
    
    }

    //stores the name of the instruction in the instruction struct
    void save_cat2_inst(string instr_name)
    {
    
    
        Instruc *temp_instruc = new Instruc;
        temp_instruc->type = instr_name;
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
    void print_data()
    {
        for(Instruc* i: instructions)
        {
            cout<<i->type<<endl;
        
        }
        for(int x: data)
        {
            cout<<x<<endl;

        } 
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
    //used to get bits pertaining to the register being used
    void get_reg(int &reg)

    {
        bitset<32> temp_reg(instruction.substr(instr_pos,5));
        reg+=to_int(temp_reg);
        instr_pos+=5;
        
    
    }
    //used to get instruction type and opcode
    void get_type_op(string &format)
    {
    
        //grab 3 leftmost bits to determine type of instructions
            format+=instruction.substr(instr_pos,3);
            //instruction location after finding type
            instr_pos+=3;
    
    }

    //handle instructions of category 2 type
    void cat_two()
    {

        get_type_op(opcode);

        //Determine what to do depending on the opcode
        if(opcode == "000")
        {

        }
        //else if(opcode == "001")
        //{
        //
        //    std::bitset<5> temp1 ;
        //    std::bitset<5> temp2 ;
        //    set_bitset(temp1,temp2);

        //    //save result from MUL into map
        //    auto temp3 = temp1*temp2;
        //    map[dest] =temp3.to_string() ;
        //
        //}
        //else if (opcode == "010")
        //{
        //
        //    std::bitset<5> temp1 ;
        //    std::bitset<5> temp2 ;
        //    set_bitset(temp1,temp2);

        //    //save result from ADD into map
        //    auto temp3 = temp1+temp2;
        //    map[dest] =temp3.to_string();
        //
        //
        //}
        //else if (opcode == "011")
        //{
        //
        //    std::bitset<5> temp1 ;
        //    std::bitset<5> temp2 ;
        //    set_bitset(temp1,temp2);

        //    //save result from SUB into map
        //    auto temp3 = temp1-temp2;
        //    map[dest] =temp3.to_string();
        //
        //
        //}
        else if (opcode == "100")
        {
        
        
        }
        else if (opcode == "101")
        {
        
        
        }

        cout<<opcode<<endl;


    }

    //pair <int,int> set_bitset()
    //{
    //    //retrieve register location  string ex:00000
    //    get_reg(src1);
    //    get_reg(src2);
    //    
    //    //grab value in register src1 and src2 stored in map
    //    string temp_str1 = map[src1];
    //    string temp_str2 = map[src2];

    //    bitset<32> temp1(temp_str1);
    //    bitset<32> temp2(temp_str2);
    //    
    //    //return 2 numbers
    //    pair <int,int> pair_bit = make_pair(to_int(temp1),to_int(temp2));
    //    //temp1 (temp_str1);
    //    //temp2 (temp_str2);
    //    return pair_bit;
    //}

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
