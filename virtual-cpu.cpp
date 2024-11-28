#include<iostream>
#include<stdio.h>
#include<math.h>
#include<string>
#include<stdint.h>
#include<vector>
#include<sstream>
#include<fstream>
using namespace std;

union memAddress{   //Use this to access two adjacent indices as a single memory address in the format of an unsigned 16 bit integer
    uint16_t whole;
    struct{
        uint8_t right;    //LSB
        uint8_t left;     //MSB
    };
};

struct opCode{  
    uint8_t whole;        //instruction specifier
    uint8_t First4;      //instruction bits
    uint8_t uRBit;       //r-bit if unary
    uint8_t nURBit;      //r-bit if non-unary
    uint8_t aField;      //addressing mode if non-unary
    
    opCode(uint8_t whole): //constructor to initialize the members' values
    whole(whole),
    First4((whole & 0b11110000) >> 4),
    uRBit(whole & 0b00000001),
    nURBit((whole & 0b00001000) >> 3),
    aField((whole & 0b00000111)){}
    
    opCode():   //no-args constructor
    whole(),
    First4(),
    uRBit(),
    nURBit(),
    aField(){}
};


struct twoByteCounter{
    uint16_t index;
};

unsigned int hexToDec(string hexIn){
    int totalSum = 0;               //tracks the running sum
    int currentPlace = 0;          //tracks the current place value
    int basePlace = 0;             //holds the base raised to the current place value
    int len = hexIn.length();     //loop length
    int currentInt = 0;                //holds the current character's value as an integer
    for(int i = len-1; i >= 0; i--){
        char currentChar = hexIn[i];
        if(currentChar < '0' || currentChar > '9'){
            currentInt = currentChar - 55;
        }
        else{
            currentInt = currentChar - '0';
        }
        basePlace = pow(16, currentPlace);
        totalSum += currentInt * basePlace;
        currentPlace++;
    }
    return totalSum;
}

unsigned int operandToBin(string operand){
    return hexToDec(operand);
}

unsigned int instructionToBin(string instruction){
    return hexToDec(instruction);
}

void storeOperand(string operand, int leftIndex, int rightIndex, uint8_t memory[]){
    memAddress currentWhole;
    currentWhole.whole = operandToBin(operand);
    memory[leftIndex] = currentWhole.left;
    memory[rightIndex] = currentWhole.right;
}

unsigned int loadOperand(int leftIndex, int rightIndex, uint8_t memory[]){
    memAddress currentWhole;
    currentWhole.left = memory[leftIndex];
    currentWhole.right = memory[rightIndex];
    return currentWhole.whole;
}

void execute(opCode instruction){
    if(instruction.whole == 0b00000000){
        //stop execution
    }
    else if(instruction.whole >= 0b00011000 && instruction.whole <= 0b00011001){
        //bitwise invert r
    }
    else if(instruction.whole >= 0b00011100 && instruction.whole <= 0b00011101){
        //asl r
    }
    else if(instruction.whole >= 0b00011110 && instruction.whole <= 0b00011111){
        //asr r
    }
    else if(instruction.whole >= 0b00100000 && instruction.whole <= 0b00100001){
        //rotate left r
    }
    else if(instruction.whole >= 0b00100010 && instruction.whole <= 0b00100011){
        //rotate right r 
    }
    else if(instruction.whole >= 0b00110000 && instruction.whole <= 0b00110111){
        //decimal input trap
    }
    else if(instruction.whole >= 0b00111000 && instruction.whole <= 0b00111111){
        //decimal output trap
    }
    else if(instruction.whole >= 0b01001000 && instruction.whole <= 0b01001111){
        //character input
    }
    else if(instruction.whole >= 0b01010000 && instruction.whole <= 0b01010111){
        //character output
    }
    else if(instruction.whole >= 0b01110000 && instruction.whole <= 0b01111111){
        //Add to r
    }
    else if(instruction.whole >= 0b10000000 && instruction.whole <= 0b10001111){
        //subtract from r
    }
    else if(instruction.whole >= 0b10010000 && instruction.whole <= 0b10011111){
        //bitwise AND to r
    }
    else if(instruction.whole >= 0b10100000 && instruction.whole <= 0b10101111){
        //bitwise OR to r
    }
    else if(instruction.whole >= 0b11000000 && instruction.whole <= 0b11001111){
        //Load word from memory
    }
    else if(instruction.whole >= 0b11010000 && instruction.whole <= 0b11011111){
        //Load byte from memory
    }
    else if(instruction.whole >= 0b11100000 && instruction.whole <= 0b11101111){
        //Store r to memory
    }
    else if(instruction.whole >= 0b11110000 && instruction.whole <= 0b11111111){
        //Store byte (right half) from r to memory
    }
    else{
        //Instruction is invalid
    }
   








}

// void incrementPC(cell pC[]){    //this is pretty reduntant, but it uses the actual program counter;
//     int currentIndex = loadOperand(0,1,pC);
//     currentIndex++;
//     memAddress temp;
//     temp.whole = currentIndex;
//     pC[0] = temp.left;
//     pC[1] = temp.right;
// };





int main(){
   //registers
   int memlen = 100;
    //cell memory[memlen];
    uint8_t memory[memlen];
    // cell accumulator[2];
    // cell indexRegister[2];
    // twoByteCounter programCounter {programCounter.index=0};
    // twoByteCounter stackPointer {stackPointer.index=65535};
    // bool statusNZVC[4] = {0,0,0,0}; //this is 4 byes; use uint8_t if you don't like that.
    
    //---------------------------------------------------------------------------------------------------------------------------
    //This section reads a file's contents into a string vector.
    vector<string> list;
    string filename = "pepInstructions.txt";    
    string inputBuff;
    ifstream file(filename);
    
    //read file contents to inputBuffer using the extraction operator; add to string vector
    while (file >> inputBuff){
        list.push_back(inputBuff);
    }
    file.close();
    //End of input section.
    //-----------------------------------------------------------------------------------------------------------------------------------------
    //This section loads the contents of the string vector into the memory array.
    opCode curInstr;
    string tempOperand;
    int size = list.size();
    int loadCounter = 0;
    while (loadCounter < size){
        curInstr.whole = instructionToBin(list[loadCounter]);
        memory[loadCounter] = curInstr.whole;
        if(curInstr.First4 < 0b0011){   //unary
            loadCounter++;
        }
        else if(curInstr.First4 >= 0b0011 && curInstr.First4 <= 0b1111 && size - loadCounter >= 3){    //non-unary
            tempOperand = list[loadCounter+1] + list[loadCounter+2];
            storeOperand(tempOperand,loadCounter+1,loadCounter+2,memory);
            loadCounter+=3;
        }
        else{   //invalid instruction
            cout << "An invalid/empty instruction was recieved and was skipped." << " Instruction: " << list[loadCounter] << " Index: " << loadCounter <<  " Value: " << curInstr.First4 << endl;
            break;
        }
    } 
    //end of program loading section.
    //-------------------------------------------------------------------------------------------------------------------------------------------
    for(int i = 0; memory[i] != 0b0000 && i < memlen; i++){    //print loop on mem array to debug
        printf("Data at %d: %d\n",i,memory[i]);
    }
    printf("Loading from memory attempt: %d",loadOperand(1,2,memory));
    //------------------------------------------------------------------------------------------------------------------------------------------------
    //Progress summary
    //Remaining core tasks:
    //implement opCode functionality, 
    //implement execution loop,
    //polish main 
    //------------------------------------------------------------------------------------------------------------------------------------------------
    //Current task?
    //implement opcode functionality.
    //Targets for refactoring: Swap hexToDec function for built-in version, swap opCode struct to a union containing bitfielded uint8_t structs
    //-------------------------------------------------------------------------------------------------------------------------------------------------




    
   
    //----------------------------------------------------
    //execution loop
    


    //testing
   
   //store & load both passed test cases. opCode struct passed all test cases.
   
   
   
   
    // printf("\nSP is at index: %d",stackPointer);
    // programCounter.index++;
    // printf("\nProgram counter is at index: %d",programCounter);
    // printf("\nSize of struct: %d bytes\n", sizeof(opCode));
    // storeOperand("32",0,1,memory);
    // opCode current{current.whole.data = loadOperand(0,1,memory)};
    // printf("\nStruct solution\nExpected value: %d\nActual value: %d\nMemory used (bytes): %d\n", current.whole.data & 0b00000111, current.aField,sizeof(current));

    // //ASR Example (Not Tested)
//     memAddress temp;
//     temp.whole=(loadOperand(0,1,accumulator));
//     int x = temp.whole;
//     x = x >> 1;
//     unsigned int y = x;
//     temp.whole = x;
//     accumulator[0] = temp.left;
//     accumulator[1] = temp.right;


}