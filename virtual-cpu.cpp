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

void storeWordToMem(uint16_t word, int leftIndex,uint8_t memory[]){
    memAddress currentWhole;
    currentWhole.whole = word;
    memory[leftIndex] = currentWhole.left;
    memory[leftIndex+1] = currentWhole.right;
}

uint16_t loadWordFromMem(uint8_t memory[], uint16_t leftIndex){
    uint16_t output;
    memAddress temp;
    temp.left = memory[leftIndex];
    temp.right = memory[leftIndex + 1];
    output = temp.whole;
    return output;
}


unsigned int loadOperand(int leftIndex, int rightIndex, uint8_t memory[]){
    memAddress currentWhole;
    currentWhole.left = memory[leftIndex];
    currentWhole.right = memory[rightIndex];
    return currentWhole.whole;
}

void storeWordToR(uint16_t word, uint8_t r[]){
    memAddress temp;
    temp.whole = word;
    r[0] = temp.left;
    r[1] = temp.right;
}

void bitwiseInvert(uint8_t r[]){
uint16_t temp = loadOperand(0,1,r);
temp = ~temp;
storeWordToR(temp,r);
}

void aslR(uint8_t r[]){
    int temp = loadOperand(0,1,r);
    temp == temp << 1;
    uint16_t temp2 = temp;
    storeWordToR(temp,r);
}

void asrR(uint8_t r[]){
    int temp = loadOperand(0,1,r);
    temp == temp >> 1;
    uint16_t temp2 = temp;
    storeWordToR(temp,r);
}

uint16_t rotL(uint16_t value, unsigned int count){
    const uint16_t mask = CHAR_BIT * sizeof(value) - 1;
    count &= mask;
    return (value << count) | (value >> (-count & mask)) ;
}

uint16_t rotR(uint16_t value, unsigned int count){
    const uint16_t mask = CHAR_BIT * sizeof(value) - 1;
    count &= mask;
    return (value >> count) | (value << (-count & mask)) ;
}

void rotLR(uint8_t r[]){
    uint16_t temp = loadOperand(0,1,r);
    temp = rotL(temp,1);
    storeWordToR(temp,r);
}

void rotRR(uint8_t r[]){
    uint16_t temp = loadOperand(0,1,r);
    temp = rotR(temp,1);
    storeWordToR(temp,r);
}

void decI(uint8_t memory[], uint16_t pC){
    int16_t userIn;
    cout << "Please enter a decimal input:\nNote that x < -32,768 or > 32,767 will result in an overflow.\n";
    cin >> userIn;                                      //grab deci. Limit for acceptable range is: -32,768 to 32,767
    storeWordToMem(userIn, pC+1,memory);     //store deci 
}

void decO(uint8_t memory[], uint16_t leftIndex){
    int16_t output;
    output = loadWordFromMem(memory,leftIndex);
    cout << output;
}

void charIn(uint8_t memory[], uint16_t address){
    char c;
    cout << "Please enter a character input: \n";
    cin >> c;
    uint8_t temp = c;
    memory[address] = c;
}

void charOut(uint8_t memory[], uint16_t address){
    char c = memory[address];
    cout << c;
}


//return bool keepExecuting? 
void execute(opCode instruction, uint8_t memory[],uint8_t accumulator[],uint8_t indexRegister[], uint16_t pC){
    if(instruction.whole == 0b00000000){
        //stop execution
    }
    else if(instruction.whole >= 0b00011000 && instruction.whole <= 0b00011001){
        //bitwise invert r
        if(instruction.uRBit == 0){         
            bitwiseInvert(accumulator);
        }
        else if(instruction.uRBit == 1){    
            bitwiseInvert(indexRegister);
        }
        else{
            cout << "Failed to determine r-bit on bitwise invert instruction.";
        }
    }
    else if(instruction.whole >= 0b00011100 && instruction.whole <= 0b00011101){
        //asl r
        if(instruction.uRBit == 0){         
            aslR(accumulator);
        }
        else if(instruction.uRBit == 1){    
            aslR(indexRegister);
        }
        else{
            cout << "Failed to determine r-bit on asl r instruction.";
        }
    }
    else if(instruction.whole >= 0b00011110 && instruction.whole <= 0b00011111){
        //asr r
        if(instruction.uRBit == 0){         
            asrR(accumulator);
        }
        else if(instruction.uRBit == 1){    
            asrR(indexRegister);
        }
        else{
            cout << "Failed to determine r-bit on asr r instruction.";
        }
    }
    else if(instruction.whole >= 0b00100000 && instruction.whole <= 0b00100001){
        //rotate left r
         if(instruction.uRBit == 0){         
            rotLR(accumulator);
        }
        else if(instruction.uRBit == 1){    
            rotLR(indexRegister);
        }
        else{
            cout << "Failed to determine r-bit on rotate left r instruction.";
        }
    }
    else if(instruction.whole >= 0b00100010 && instruction.whole <= 0b00100011){
        //rotate right r 
         if(instruction.uRBit == 0){         
            rotRR(accumulator);
        }
        else if(instruction.uRBit == 1){    
            rotRR(indexRegister);
        }
        else{
            cout << "Failed to determine r-bit on rotate right r instruction.";
        }
    }
    else if(instruction.whole >= 0b00110000 && instruction.whole <= 0b00110111){
        //decimal input trap (has a-field,no r)
       decI(memory,pC);
    }
    else if(instruction.whole >= 0b00111000 && instruction.whole <= 0b00111111){
        //decimal output trap
        if(instruction.aField == 0b000){    //immediate
            decO(memory,pC+1);  //using the address after pC as operand
        }
        else if(instruction.aField == 0b001){   //direct
            uint16_t address = loadWordFromMem(memory,pC+1);
            decO(memory,address);   //interpreting pC +1 and pC + 2 together as an address; using that address as operand
        }
        else{
            cout << "Addressing mode: " << instruction.aField << " is not supported by deco instruction.";
        }
    }
    else if(instruction.whole >= 0b01001000 && instruction.whole <= 0b01001111){
        //character input
        if(instruction.aField == 0b001){
            uint16_t address = loadWordFromMem(memory,pC+1);
            charIn(memory,address);
        }
        else{
            cout << "Addressing mode: " << instruction.aField << " is not supported for charIn instruction.";
        }
    }
    else if(instruction.whole >= 0b01010000 && instruction.whole <= 0b01010111){
        //character output
        if(instruction.aField == 0b000){    //immediate
            charOut(memory,pC+2);
        }
        else if(instruction.aField == 0b001){   //direct
            uint16_t address = loadWordFromMem(memory,pC+1);
            charOut(memory,address);
        }
        else{
            cout << "Addressing mode: " << instruction.aField << " is not supported for charOut instruction.";
        }
    }
    else if(instruction.whole >= 0b01110000 && instruction.whole <= 0b01111111){
        //Add to r (has a-field and r)
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
     //----------------------------------------------------------------------------------------------------------------------------
    //This section contains the memory array and registers.
    int memlen = 100;
    uint8_t memory[memlen];
    uint8_t accumulator[2];
    uint8_t indexRegister[2];
    uint16_t pC = 0;
    // twoByteCounter programCounter {programCounter.index=0};
    // twoByteCounter stackPointer {stackPointer.index=65535};
    // bool statusNZVC[4] = {0,0,0,0}; //this is 4 byes; use uint8_t if you don't like that.
    //End of register section.
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
    int16_t x = 32;
    //------------------------------------------------------------------------------------------------------------------------------------------------
    //Progress summary
    //Remaining core tasks:
    //implement opCode functionality, 
    //implement execution loop,
    //polish main 
    //------------------------------------------------------------------------------------------------------------------------------------------------
    //Current task?
    //implement opcode functionality.
    //Targets for refactoring: Swap hexToDec function for built-in version, swap opCode struct to a union containing bitfielded uint8_t structs, 
    //note that rotate instructions do NOT currently make use of a status register
    //finish initial version without using status bits, then come back and add status bit usage during polishing stage.
    //-------------------------------------------------------------------------------------------------------------------------------------------------




    
   
    //----------------------------------------------------
    //Execution loop
    


    //End of exectution loop section
   
   
   
   

}