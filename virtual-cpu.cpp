#include<iostream>
#include<stdio.h>
#include<math.h>
#include<string>
#include<stdint.h>
#include<vector>
#include<sstream>
#include<fstream>
#include<conio.h>
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

void addR(uint8_t memory[], uint16_t word, uint8_t r[]){
    uint16_t temp = loadWordFromMem(r,0);
    temp += word;
    storeWordToR(temp,r);
}

void subR(uint8_t memory[], uint16_t word, uint8_t r[]){
    uint16_t temp = loadWordFromMem(r,0);
    temp -= word;
    storeWordToR(temp,r);
}

void andR(uint8_t memory[], uint16_t word, uint8_t r[]){
    uint16_t temp = loadWordFromMem(r,0);
    temp &= word;
    storeWordToR(temp,r);
}

void orR(uint8_t memory[], uint16_t word, uint8_t r[]){
    uint16_t temp = loadWordFromMem(r,0);
    temp |= word;
    storeWordToR(temp,r);
}

void lWdR(uint8_t memory[], uint16_t word, uint8_t r[]){
    storeWordToR(word,r);
}

void execute(opCode instruction, uint8_t memory[],uint8_t accumulator[],uint8_t indexRegister[], uint16_t pC, bool &stopEx){
    if(instruction.whole == 0b00000000){
        cout << "\nStop instruction reached; Terminating program.";
        stopEx = true;
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
        //Add word to r (has a-field and r)
        if(instruction.aField == 0b000){    //immediate
            uint16_t word = loadWordFromMem(memory,pC+1);
            if(instruction.nURBit == 0){   
                addR(memory,word,accumulator);
            }
            else{                                     
                addR(memory,word,indexRegister);
            }
        }
        else if(instruction.aField == 0b001){   //direct
            uint16_t address = loadWordFromMem(memory,pC+1);
            uint16_t word = loadWordFromMem(memory,address);
            if(instruction.nURBit == 0){    
                addR(memory,word,accumulator);
            }
            else{                           
                addR(memory,word,indexRegister);
            }
        }
        else{
            cout << "Addressing mode: " << instruction.aField << " is not supported for addR instruction.";
        }
    }
    else if(instruction.whole >= 0b10000000 && instruction.whole <= 0b10001111){
        //subtract word from r
         if(instruction.aField == 0b000){    //immediate
            uint16_t word = loadWordFromMem(memory,pC+1);
            if(instruction.nURBit == 0){   
                subR(memory,word,accumulator);
            }
            else{                                      
                subR(memory,word,indexRegister);
            }
        }
        else if(instruction.aField == 0b001){   //direct
            uint16_t address = loadWordFromMem(memory,pC+1);
            uint16_t word = loadWordFromMem(memory,address);
            if(instruction.nURBit == 0){    
                subR(memory,word,accumulator);
            }
            else{                           
                subR(memory,word,indexRegister);
            }
        }
        else{
            cout << "Addressing mode: " << instruction.aField << " is not supported for subR instruction.";
        }
    }
    else if(instruction.whole >= 0b10010000 && instruction.whole <= 0b10011111){
        //bitwise AND word to r
         if(instruction.aField == 0b000){    //immediate
            uint16_t word = loadWordFromMem(memory,pC+1);
            if(instruction.nURBit == 0){   
                andR(memory,word,accumulator);
            }
            else{                                      
                andR(memory,word,indexRegister);
            }
        }
        else if(instruction.aField == 0b001){   //direct
            uint16_t address = loadWordFromMem(memory,pC+1);
            uint16_t word = loadWordFromMem(memory,address);
            if(instruction.nURBit == 0){    
                andR(memory,word,accumulator);
            }
            else{                           
                andR(memory,word,indexRegister);
            }
        }
        else{
            cout << "Addressing mode: " << instruction.aField << " is not supported for andR instruction.";
        }
    }
    else if(instruction.whole >= 0b10100000 && instruction.whole <= 0b10101111){
        //bitwise OR word to r
         if(instruction.aField == 0b000){    //immediate
            uint16_t word = loadWordFromMem(memory,pC+1);
            if(instruction.nURBit == 0){   
                orR(memory,word,accumulator);
            }
            else{                                      
                orR(memory,word,indexRegister);
            }
        }
        else if(instruction.aField == 0b001){   //direct
            uint16_t address = loadWordFromMem(memory,pC+1);
            uint16_t word = loadWordFromMem(memory,address);
            if(instruction.nURBit == 0){    
                orR(memory,word,accumulator);
            }
            else{                           
                orR(memory,word,indexRegister);
            }
        }
        else{
            cout << "Addressing mode: " << instruction.aField << " is not supported for orR instruction.";
        }
    }
    else if(instruction.whole >= 0b11000000 && instruction.whole <= 0b11001111){
        //Load word from memory to r
         if(instruction.aField == 0b000){    //immediate
            uint16_t word = loadWordFromMem(memory,pC+1);
            if(instruction.nURBit == 0){   
                lWdR(memory,word,accumulator);
            }
            else{                                       
                lWdR(memory,word,indexRegister);
            }
        }
        else if(instruction.aField == 0b001){   //direct
            uint16_t address = loadWordFromMem(memory,pC+1);
            uint16_t word = loadWordFromMem(memory,address);
            if(instruction.nURBit == 0){    
                lWdR(memory,word,accumulator);
            }
            else{                           
                lWdR(memory,word,indexRegister);
            }
        }
        else{
            cout << "Addressing mode: " << instruction.aField << " is not supported for lWdR instruction.";
        }
    }
    else if(instruction.whole >= 0b11010000 && instruction.whole <= 0b11011111){
        //Load byte from memory to r
        if(instruction.aField == 0b000){    //immediate
            if(instruction.nURBit == 0){   
                accumulator[1] = memory[pC+2];
            }
            else{                                       
                indexRegister[1] = memory[pC+2];
            }
        }
        else if(instruction.aField == 0b001){   //direct
            uint16_t address = loadWordFromMem(memory,pC+1);
            uint8_t byte = memory[address];
            if(instruction.nURBit == 0){    
                accumulator[1] = byte;
            }
            else{                           
                indexRegister[1] = byte;
            }
        }
        else{
            cout << "Addressing mode: " << instruction.aField << " is not supported for stBR instruction.";
        }
    }
    else if(instruction.whole >= 0b11100000 && instruction.whole <= 0b11101111){
        //Store word from r to memory
         if(instruction.aField == 0b001){    
            uint16_t address = loadWordFromMem(memory,pC+1);
            if(instruction.nURBit == 0){    
                uint16_t word = loadWordFromMem(accumulator,0);
                storeWordToMem(word,address,memory);
            }
            else{                           
                uint16_t word = loadWordFromMem(indexRegister,0);
                storeWordToMem(word,address,memory);
            }
        }
        else{
            cout << "Addressing mode: " << instruction.aField << " is not supported for stRM instruction.";
        }
    }
    else if(instruction.whole >= 0b11110000 && instruction.whole <= 0b11111111){
        //Store byte (right half) from r to memory
          if(instruction.aField == 0b001){    
            uint16_t address = loadWordFromMem(memory,pC+1);
            if(instruction.nURBit == 0){    
                memory[address] = accumulator[1];
            }
            else{                           
                memory[address] = indexRegister[1];
            }
        }
        else{
            cout << "Addressing mode: " << instruction.aField << " is not supported for stBRM instruction.";
        }
    }
    else{
        //Instruction is invalid
        cout << "Invalid instruction recieved: " << instruction.whole << endl;
    }
   








}



int main(){
     //----------------------------------------------------------------------------------------------------------------------------
    //This section contains the memory array and registers.
    int memlen = 100;
    uint8_t memory[memlen] {0};
    uint8_t accumulator[2] {0};
    uint8_t indexRegister[2] {0};
    uint16_t pC = 0;
    uint16_t sP = 65535;
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
    // for(int i = 0; memory[i] != 0b0000 && i < memlen; i++){    //print loop on mem array to debug
    //     printf("Data at %d: %d\n",i,memory[i]);
    // }
    // printf("Loading from memory attempt: %d",loadOperand(1,2,memory));
    // int16_t x = 32;
    //------------------------------------------------------------------------------------------------------------------------------------------------
    //Current task?
    //print registers on step
    //test opcode functionality, test execution loop functionality
    //Targets for refactoring: Swap hexToDec function for built-in version, swap opCode struct to a union containing bitfielded uint8_t structs, 
    //finish initial version without using status bits, then come back and add status bit usage during polishing stage.
    //-------------------------------------------------------------------------------------------------------------------------------------------------
    //Execution loop
    bool stopEx = false;
    //fetch, decode, increment, execute.
    while(!stopEx && pC < 10){  //remember to change this to 65535 later.
        cout << "Press any key to step execution.\n";
        getch();
        opCode current = memory[pC];
        if(current.First4 < 0b0011){
            pC++;
        }
        else{
            pC+=3;
        }
        execute(current,memory,accumulator,indexRegister,pC,stopEx);
        printf("\nAccumulator: %d\nIndex register: %d\nProgram Counter: %d\nStack Pointer: %d\n",loadWordFromMem(accumulator,0),loadWordFromMem(indexRegister,0),pC,sP);
        //print out the registers
    }


    //End of exectution loop section
   
   
   
   

}