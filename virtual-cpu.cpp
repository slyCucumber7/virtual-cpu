#include<iostream>
#include<stdio.h>
#include<math.h>
#include<string>
#include<stdint.h>
#include<vector>
#include<sstream>
#include<fstream>
using namespace std;

struct cell {   //This is used to populate the array; It is a single index's data
    uint8_t data;
};
    
union memAddress{   //Use this to access two adjacent indices as a single memory address in the format of an unsigned 16 bit integer
    uint16_t whole;
    cell left;
    cell right;
};

struct opCode{  
    cell whole;                                       //instruction specifier
    uint8_t First4 = (whole.data & 0b11110000) >> 4;  //instruction
    uint8_t uRBit  =  whole.data & 0b00000001;        //r-bit if unary
    uint8_t nURBit = (whole.data & 0b00001000) >> 3;  //r-bit if non-unary
    uint8_t aField = (whole.data & 0b00000111);       //addressing mode if non-unary
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

void storeOperand(string operand, int leftIndex, int rightIndex, cell memory[]){
    memAddress currentWhole;
    currentWhole.whole = operandToBin(operand);
    memory[leftIndex] = currentWhole.left;
    memory[rightIndex] = currentWhole.right;
}

unsigned int loadOperand(int leftIndex, int rightIndex, cell memory[]){
    memAddress currentWhole;
    currentWhole.left = memory[leftIndex];
    currentWhole.right = memory[rightIndex];
    return currentWhole.whole;
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
    cell memory[memlen];
    
    // cell accumulator[2];
    // cell indexRegister[2];
    // twoByteCounter programCounter {programCounter.index=0};
    // twoByteCounter stackPointer {stackPointer.index=65535};
    // bool statusNZVC[4] = {0,0,0,0}; //this is 4 byes; use uint8_t if you don't like that.
    
    //---------------------------------------------------------------------------------------------------------------------------
    //This section reads a file's contents into a string vector.
    vector<string> list;
    string filename = "pepInstructions.txt";    //note that laptop runs code from the output folder, so you have to put input files in there, or specify an absolute path.
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
        curInstr.whole.data = instructionToBin(list[loadCounter]);
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
            cout << "An invalid/empty instruction was recieved and was skipped." << " Instruction: " << list[loadCounter] << endl;
            break;
        }
    } 
    //end of program loading section.
    //-------------------------------------------------------------------------------------------------------------------------------------------
    for(int i = 0; memory[i].data != 0b0000 && i < memlen; i++){
        printf("Data at %d: %d\n",i,memory[i].data);
    }
    cout << loadOperand(1,2,memory);






    // memory[0].data = 0b00110010;
    // cout << memory[0].data;
    // printf("\nData: %d", memory[0].data);
    
    //cout << memory[0].data;
    
    //Where are we currently?
    //file reading and input parsing work fine
    //input vector to memory array storing seems to work fine
    //we need to double check that non unary instructions are loaded correctly
    //after that, all that is needed is execution loop and instruction function(s)
    //I think zero is being dropped by my binary to decimal converter. Check that out.
    
    
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