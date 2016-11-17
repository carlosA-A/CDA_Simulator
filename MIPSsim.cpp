#include <iostream>
#include <bitset>         // std::bitset
#include <fstream>
#include <string>
#include <unordered_map>
#include <utility>      // std::pair


using namespace std;


class Simulator
{   
    private:
        unordered_map <string,string> map;
        int pc_counter;
        int instr_pos;  //Used to acces and traverse string
        string file_name;
        string instruction;
        string format;
        string opcode;
        string dest;
        string src1;
        string src2;
        string addrs;

    public:

    Simulator(string file):file_name{file}{}
    
    void print_data()
    {
        //open up file that contains instructions
        ifstream infile(file_name);
        
        //Read instructions from file
        while(infile>>instruction)
        {

            reset();
            get_type_op(format);
            //cout<<format<<endl;
            //Instruction is category 2, XOR MUL ADD SUB etc
            if(format == "010")
            {   //category 2 type instruction handelling
                cat_two();
            }
            
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

        dest="";
        
        src1="";

        src2="";

        addrs="";
    }
    //used to get bits pertaining to the register being used
    void get_reg(string &reg)

    {
        int i = 0;
        while(i<5)
        {
            reg+=instruction[instr_pos];
            instr_pos++;
            i++;
        
        }
    
    }
    //used to get instruction type and opcode
    void get_type_op(string &format)
    {
    
        //grab 3 leftmost bits to determine type of instructions
        int i = 0;
        while(i < 3)
        {
            format+=instruction[instr_pos];
            instr_pos++;
            i++;
        }

    
    }

    //handle instructions of category 2 type
    void cat_two()
    {

        get_type_op(opcode);

        //Determine what to do depending on the opcode
        if(opcode == "000")
        {
            std::bitset<5> temp1 ;
            std::bitset<5> temp2 ;
            //save result from XOR into map
            auto temp3 = temp1^=temp2;
            map[dest] =temp3.to_string() ;

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
        
            std::bitset<5> temp1 ;
            std::bitset<5> temp2 ;
            //set_bitset(temp1,temp2);

            //save result from AND into map
            auto temp3 = temp1&=temp2;
            map[dest] =temp3.to_string();
        
        }
        else if (opcode == "101")
        {
        
            std::bitset<5> temp1 ;
            std::bitset<5> temp2 ;
            //set_bitset(temp1,temp2);

            //save result from OR into map
            auto temp3 = temp1|=temp2;
            map[dest] =temp3.to_string();
        
        }

        cout<<opcode<<endl;


    }

    pair <int,int> set_bitset()
    {
        //retrieve register location  string ex:00000
        get_reg(src1);
        get_reg(src2);
        
        //grab value in register src1 and src2 stored in map
        string temp_str1 = map[src1];
        string temp_str2 = map[src2];

        bitset<32> temp1(temp_str1);
        bitset<32> temp2(temp_str2);
        
        //return 2 numbers
        pair <int,int> pair_bit = make_pair(get_num(temp1),get_num(temp2));
        //temp1 (temp_str1);
        //temp2 (temp_str2);
        return pair_bit;
    }

    auto get_num(bitset<32> set) -> int
    {
        return static_cast<int>(set.to_ulong());
    
    }




};




int main ()
{
    string file_name;
    cin>>file_name;
    Simulator mips(file_name);
    mips.print_data();

    return 0;
}
