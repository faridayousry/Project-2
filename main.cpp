#include "stdio.h"
#include "stdlib.h"
#include <iostream>
#include <string>
using namespace std;

//CARRY FLAG, lsl & lsr, why simulate returns int?????, output modified value, cspr flag, fseek for BRANCH, instr 14: rlist range

/* BL works in two stages:
 H=0: LR := PC + signextend(offset << 12)
 H=1: PC := LR + (offset << 1)
 LR := oldPC + 3
 */


void simulate(unsigned short, FILE *fp);
void regPrint(unsigned int RList, int& rCount);

unsigned char Mem[1024];        //memory is 1024 bytes?
unsigned int Regs[16];          //each register is 4 bytes?
int carryFlag;

char swiChar;
string swiString;
bool swiTerminate =  false;

#define    PC    Regs[15]
#define    LR    Regs[14]
#define SP  Regs[13]

int CSPR;  //PSR (CPSR) program status register to use a flag/store the result of cmp instruction & other instr(such as branch)
bool carryBit =0;
bool zflag = 0;
int regSize = 4;
int Word = 4;

int main() {
    FILE* fp;
    unsigned short inst_word;
    
    fp = fopen("test.s.bin", "rb");
    
    if (NULL == fp) {
        printf("Cannot open the file\n");
        exit(0);
    }
    
    fread(&SP, 4, 1, fp);        //reading first 32-bits to set as the initial value for the stack pointer
    fread(&PC, 4, 1, fp);        //reading second 32-bits to set as the initial value for the program counter
    
    while (fread(&inst_word, 2, 1, fp))
    {
        if (inst_word == 0xDEAD || swiTerminate == true)     //if terminator instruction is found -> terminate simulator
            return 0;
        
        printf("%08x\t%04x\t", PC, inst_word);
        simulate(inst_word, fp);
        PC += 2;
    }
    fclose(fp);
    return 0;
}




void regPrint(unsigned int RList, int& rCount)
{
    unsigned int Re[8];
    Re[0] = RList & 1;
    Re[1] = (RList >> 1) & 1;
    Re[2] = (RList >> 2) & 1;
    Re[3] = (RList >> 3) & 1;
    Re[4] = (RList >> 4) & 1;
    Re[5] = (RList >> 5) & 1;
    Re[6] = (RList >> 6) & 1;
    Re[7] = (RList >> 7) & 1;
    
    
    for (int i = 0; i < 8; i++) {
        if (Re[i] == 1) {
            rCount++;
            cout << "R" << i;
        }
        if (i == 7 && Re[i] == 1)
            break;
        if (Re[i + 1] == 1)
            cout << ",";
    }
}





void simulate(unsigned short instr, FILE *fp)
{
    unsigned char fmt, op, offset5, rd, rs, offset3, rn;
    
    fmt = (instr) >> 13;
    
    switch (fmt)
    {
        case 0:{             // format 1/2
            op = (instr >> 11) & 3;         //mask to get bits 11 &12 (opcode in format 1)
            rd = instr & 7;                 //mask to get 3 rightmost bits (rd for formats 1,2,4,7,8,9,10)
            rs = (instr >> 3) & 7;         //mask to get bits 3,4,5 (rs for formats 1,2,4  &  rb for 7,8,9,10)
            offset5 = (instr >> 6) & 0x1F;  //mask to get bits 6-10 (offset5 for formats 1,9,10)
            if (op != 3)
            {     // format 1
                switch (op)
                {
                    case 0:
                        printf("lsl\tr%d, r%d, #%d\n", rd, rs, offset5);    //logical left shift instruction
                        Regs[rd] = (Regs[rs] << offset5);
                        cout << "\n \t R" << rd << " has been modified to: "<<Regs[rd];
                        break;
                        
                    case 1:
                        printf("lsr\tr%d, r%d, #%d\n", rd, rs, offset5);     //logical right shift instruction
                        Regs[rd] = (Regs[rs] / (2^offset5));
                        cout << "\n \t R" << rd << " has been modified to: "<<Regs[rd];
                        break;
                        
                    case 2:
                        printf("asr\tr%d, r%d, #%d\n", rd, rs, offset5);      //arithmetic right shift instruction
                        Regs[rd] = (Regs[rs] >> offset5);
                        cout << "\n \t R" << rd << " has been modified to: "<<Regs[rd];
                        break;
                    default:
                        printf("UNKNOWN INSTR\n");
                }
            }
            
            else{ /*add/sub*/      // format 2
                offset3 = rn = offset5 & 0x07;          //mask to get 3 right most bits of offset5
                if ((offset5 & 0x08) == 0) {              //if opcode (of format 2) == 0 -> add instruction
                    printf("add\tr%d, r%d, ", rd, rs);
                    if ((offset5 & 0x10) == 0) {          //check if the 'I' flag (immediate') is set:
                        printf("r%d\n", rn);                //if so -> add format: "ADD rd, rs, rn"
                        Regs[rd] = Regs[rs] + Regs[rn];         //update registers array
                        cout << "\n \t R" << rd << " has been modified to: "<<Regs[rd];
                    }
                    else {                              //else if 'I' flag (immediate') is not set:
                        printf("#%d\n", offset3);           //-> add format: "ADD rd, rs, offset"
                        Regs[rd] = Regs[rs] + offset3;          //update registers array
                        cout << "\n \t R" << rd << " has beenmodified to: "<<Regs[rd];
                    }
                }
                else {        //sub instruction
                    printf("sub\tr%d, r%d, ", rd, rs);
                    if ((offset5 & 0x10) == 0) {           //check if the 'I' flag (immediate') is set:
                        printf("r%d\n", rn);                  //if so -> sub format: "SUB rd, rs, rn"
                        Regs[rd] = Regs[rs] - Regs[rn];          //update registers array
                        cout << "\n \t R" << rd << " has been modified to: "<<Regs[rd];
                    }
                    else {
                        printf("#%d\n", offset3);          //else if 'I' flag (immediate') is not set: -> sub format: "SUB rd, rs, offset"
                        Regs[rd] = Regs[rs] - offset3;           //update registers array
                        cout << "\n \t R" << rd << " has been modified to: "<<Regs[rd];
                    }
                }
            }
            break;
        }
            
        case 1:{        //format 3  (move/compare/add/subtract Immediate)
            op = (instr >> 11) & 3;         //mask to get bits 11 &12 (opcode)
            rd = (instr >> 8) & 7;                 //mask to get 3 bits f (bits 8,9,19 for rd)
            int offset8 = instr & 0xFF;      //mask to get bits 0-7 (offset8)
            
            switch (op)
            {
                case 0:
                    printf("mov\tr%d, #%d\n", rd, offset8);        //MOV imm instruction
                    Regs[rd] = offset8;
                    cout << "\n \t R" << rd << " has been modified to: " << Regs[rd];
                    break;
                    
                case 1:
                    printf("cmp\tr%d, #%d\n", rd, offset8);        //cmp imm instruction
                    if (Regs[rd] >= offset8)  CSPR = 1;
                    else /*if ((Regs[rd] >= 0) && (offset8 >= 0))*/ carryBit = 1;     //if rd < offset & both are unsigned, set carry flag as rd - offset where rd < offset will cause carry
                    //else carryBit =0;
                    break;
                    
                case 2:
                    printf("add\tr%d, #%d\n", rd, offset8);        //add imm instruction
                    Regs[rd] = Regs[rd] + offset8;
                    if ( (Regs[rd] < offset8) || (Regs[rd] < (Regs[rd]-offset8)) ) carryBit = 1;   //if result < either of the operands -> set carry flag
                    else carryBit = 0;
                    cout << "\n \t R" << rd << " has been modified to: " << Regs[rd];
                    break;
                    
                case 3: printf("sub\tr%d, #%d\n", rd, offset8);        //sub imm instruction
                    Regs[rd] = Regs[rd] - offset8;
                    if ( Regs[rd] >  (Regs[rd]+offset8) ) carryBit = 1;   //if result > rd - imm -> set carry flag
                    else carryBit = 0;
                    cout << "\n \t R" << rd << " has been modified to: " << Regs[rd];
                    break;
                default:
                    printf("UNKNOWN INSTR\n");
            }
            break;
        }
            
            
        case 2:{        //formats 4, 5, 6, 7, 8
            op = (instr >> 11) & 3;         //mask to get bits 11 &12 (opcode)
            rd = instr & 7;                 //mask to get 3 rightmost bits (rd for formats 4,5,7,8)
            rs = (instr >> 3) & 7;         //mask to get bits 3,4,5 (rs for formats 4,5  &  rb for 7,8)
            offset5 = (instr >> 6) & 0x1F;  //mask to get bits 6-10 (offset5 for formats 1,9,10)
            
            switch (op)
            {
                case 0:{ //formats 4     (ALU operations & Hi register ops/branch exchange)
                    op = (instr >> 6) & 15;     //adjust op respective to be bits 6-9 for format 4
                    switch (op){
                        case 0:        //and instruction (rd = rd & rs)
                            printf("and\tr%d, r%d\n", rd, rs);
                            Regs[rd] = Regs[rd] & Regs[rs];
                            cout << "\n \t R" << rd << " has been modified to: " << Regs[rd];
                            break;
                            
                        case 1:     //eor instruction (rd = rd (bitwise xor) rs)
                            printf("eor\tr%d, r%d\n", rd, rs);
                            Regs[rd] = Regs[rd] ^ Regs[rs];
                            //carryBit = 0; ?
                            cout << "\n \t R" << rd << " has been modified to: " << Regs[rd];
                            break;
                            
                        case 2:        //lsl instruction (rd = rd << rs)
                            printf("lsl\tr%d, r%d\n", rd, rs);
                            Regs[rd] = Regs[rd] << Regs[rs];
                            cout << "\n \t R" << rd << " has been modified to: " << Regs[rd];
                            break;
                            
                        case 3:     //lsr instruction (rd = rd >> rs)
                            printf("lsr\tr%d, r%d\n", rd, rs);
                            Regs[rd] = (Regs[rs] / (2 ^ Regs[rs]));
                            cout << "\n \t R" << rd << " has been modified to: " << Regs[rd];
                            break;
                            
                        case 4:        //asr instruction (rd = rd asr rs)
                            printf("asr\tr%d, r%d\n", rd, rs);
                            Regs[rd] = Regs[rd] >>  Regs[rs];
                            cout << "\n \t R" << rd << " has been modified to: " << Regs[rd];
                            break;
                            
                        case 5:     //adc instruction (rd = rd + rs + carry flag)
                            printf("adc\tr%d, r%d\n", rd, rs);
                            Regs[rd] = Regs[rd] + Regs[rs] + carryBit;
                            cout << "\n \t R" << rd << " has been modified to: "<<Regs[rd];
                            break;
                            
                        case 6:        //sbc instruction(rd = rd - rs - NOT c-bit)
                            printf("sbc\tr%d, r%d\n", rd, rs);
                            Regs[rd] = Regs[rd] - Regs[rs] - ~carryBit;
                            cout << "\n \t R" << rd << " has been modified to: "<<Regs[rd];
                            break;
                            
                        case 7:{     //ror instruction (rd = rd ror rs)
                            printf("ror\tr%d, r%d\n", rd, rs);
                            for (int i = 0; i < Regs[rs]; i++) {         //rotate right
                                if (Regs[rd] & 0x01){           //if right most bit == 1 (the bit that is about to be shifted)
                                    Regs[rd] = (Regs[rd] >> 1) | 0x80;    //shift rd to the right and add one to the left of rd
                                    carryFlag = 1;   //set c-flag = shifted bit
                                }
                                else {
                                    Regs[rd] = Regs[rd] >> 1;      //else if right most bit == 0; just shift to the right; the zero will be automatically added to the left
                                    carryFlag = 0;
                                }
                            }
                            cout << "\n \t R" << rd << " has been modified to: "<<Regs[rd];
                            break;
                        }
                            
                        case 8:        //tst instruction (flag = rd and rs)
                            printf("tst\tr%d, r%d\n", rd, rs);
                            zflag = ((rd and rs) == rs);
                            break;
                            
                        case 9:     //neg instruction (rd = rd >> rs)
                            printf("neg\tr%d, r%d\n", rd, rs);
                            Regs[rd] = -Regs[rs];
                            cout << "\n \t R" << rd << " has been modified to: "<<Regs[rd];
                            break;
                            
                        case 10:        //cmp instruction (rd - rs)? 1:0)
                            printf("cmp\tr%d, r%d\n", rd, rs);
                            if (Regs[rd] >= Regs[rs])  CSPR = 1;
                            else /*if ((Regs[rd] >= 0) && (Regs[rs] >= 0))*/ carryBit = 1;     //if rd < rs & both are unsigned, set carry flag as rd - rs where rd < rs will cause carry
                            //else carryBit =0;
                            break;
                            
                        case 11:     //cmn instruction (rd + rs)? 1:0)
                            printf("cmn\tr%d, r%d\n", rd, rs);
                            if ( (Regs[rd] < Regs[rs]) || (Regs[rd] < (Regs[rd]-Regs[rd])) ) carryBit = 1;   //if result < either of the operands -> set carry flag
                            else carryBit = 0;
                            break;
                            
                        case 12:        //orr instruction(rd := rd or rs  [bitwise or])
                            printf("orr\tr%d, r%d\n", rd, rs);
                            Regs[rd] = Regs[rd] | Regs[rs];
                            cout << "\n \t R" << rd << " has been modified to: "<<Regs[rd];
                            break;
                            
                        case 13:     //mul instruction (rd = rd * rs)
                            printf("mul\tr%d, r%d\n", rd, rs);
                            Regs[rd] = Regs[rd] * Regs[rs];
                            cout << "\n \t R" << rd << " has been modified to: "<<Regs[rd];
                            break;
                            
                        case 14:        //bic instruction (rd = rd AND NOT rs)
                            printf("bic\tr%d, r%d\n", rd, rs);
                            Regs[rd] = Regs[rd] & (~Regs[rs]);     //set condition codes
                            cout << "\n \t R" << rd << " has been modified to: "<<Regs[rd];
                            break;
                            
                        case 15:     //mvn instruction (rd = not rs)
                            printf("mvn\tr%d, r%d\n", rd, rs);
                            Regs[rd] = ~Regs[rs];                   //SHOULD IT BE 'NOT'/'NEG'/'¬'/'!'/'~' ??????????????????????
                            cout << "\n \t R" << rd << " has been modified to: "<<Regs[rd];
                            break;
                            
                            //break;
                        default:
                            printf("UNKNOWN INSTR\n");
                    }
                }
                case 1:{     //format 6
                    rd = (instr >> 8) & 7;      //mask to get bits 8,9,10 (rd for format 6)
                    int word8 = instr & 0x7f;     //mask to get 8 rightmost bits for immediate offset (Word8)
                    int imm = word8 << 2;           //we shift word8 to the left to output the correct value of the immediate as the assembler has stored into word8 the immediate shifted to the right by 2
                    printf("ldr\tr%d,  #%d\n", rd, imm);    //Load into Rd the word found at the address formed by adding PC + word8
                    Regs[rd] = Mem[Regs[15] + word8];       //Rd = PC + word8
                    Regs[rd] = (Regs[rd] << 8) | Mem[Regs[15] + word8 + 1];
                    Regs[rd] = (Regs[rd] << 8) | Mem[Regs[15] + word8 + 2];
                    Regs[rd] = (Regs[rd] << 8) | Mem[Regs[15] + word8 + 3];  //to get full word (4 bytes)
                    
                    PC += 4 ;  //increment PC by 4 bytes
                    PC = (PC >> 1) & 0xFFFFFFFE;  //bit  of PC forced to 0 to word-align
                    
                    cout << "\n \t R" << rd << " has been modified to: "<<Regs[rd];
                    break;
                }
                default:
                    printf("UNKNOWN INSTR\n");
            }
            if (((instr >> 12) & 1) == 1){          // validate format 7 from the 12bit
                if (((instr >> 9) & 1) == 1){
                    int LB = ((instr >> 10) & 3);
                    int ro = ((instr >> 6) & 7);
                    switch (LB)
                    {
                        case 0:
                            cout << "str\tR" << rd << ", [R" << rs << ", R" << ro << "]\n";
                            
                            Mem[Regs[rs] + Regs[ro]] = Regs[rd];
                            Mem[Regs[rs] + Regs[ro] + 1] = Regs[rd] >> 8;
                            Mem[Regs[rs] + Regs[ro] + 2] = Regs[rd] >> 16;
                            Mem[Regs[rs] + Regs[ro] + 3] = Regs[rd] >> 24;
                            
                            cout << "\n \t memory has been updated";
                            break;
                            
                        case 1:
                            cout << "strb\tR" << rd << ", [R" << rs << ", R" << ro << "]\n";
                            
                            Mem[Regs[rs] + Regs[ro]] = Regs[rd];
                            cout << "\n \t memory has been updated";
                            break;
                            
                        case 2:
                            cout << "ldr\tR" << rd << ", [R" << rs << ", R" << ro << "]\n";
                            
                            Regs[rd] = Mem[Regs[rs] + Regs[ro]];
                            Regs[rd] = Regs[rd] | (Mem[Regs[rs] + Regs[ro] + 1] << 8);
                            Regs[rd] = Regs[rd] | (Mem[Regs[rs] + Regs[ro] + 2] << 16);
                            Regs[rd] = Regs[rd] | (Mem[Regs[rs] + Regs[ro] + 3] << 24);
                            cout << "\t  rd  has been updated";
                            /*printf("ldr\tr%d,[r%d]\n", rd, rs, ro);
                             Regs[rd] = Mem[Regs[rs] + Regs[ro]];
                             cout << "\n \t  rd  has been updated"; */
                            break;
                        case 3:
                            cout << "ldrb\tR" << rd << ", [R" << rs << ", R" << ro << "]\n";
                            Regs[rd] = Mem[Regs[rs] + Regs[ro]];
                            cout << "\n \t R" << rd << " has been modified to: "<<Regs[rd];
                            
                            break;
                        default:
                            printf("UNKNOWN INSTR\n");
                            
                            
                    }
                    
                }
            }
            break;
        }
            
            
            
        case 3:{    // format 9
            offset5 = (instr >> 6) & 0x1F;
            rs = (instr >> 4) & 0x7;
            rd = instr & 0x7;
            int BL = (instr >> 11) & 3;
            switch (BL)
            {
                case 0:
                    Mem[Regs[rs] + offset5] = Regs[rd];
                    Mem[Regs[rs] + offset5 + 1] = Regs[rd] >> 8;
                    Mem[Regs[rs] + offset5 + 2] = Regs[rd] >> 16;
                    Mem[Regs[rs] + offset5 + 3] = Regs[rd] >> 24;
                    cout << "str\trd, [rb, #" << offset5 << "]\n" << endl;
                    
                    break;
                    
                case 1:
                    cout << "strb\trd,[rb, #" << offset5 << "]" << endl;
                    Mem[Regs[rs] + offset5] = Regs[rd];
                    cout << "\n \t memory has been updated";
                    
                    break;
                    
                case 2:
                    cout << "ldr\trd,[rb,#" << offset5 << "]" << endl;
                    Regs[rd] = 0;
                    Regs[rd] = Mem[Regs[rs] + offset5];
                    Regs[rd] = Regs[rd] | (Mem[Regs[rs] + offset5 + 1] << 8);
                    Regs[rd] = Regs[rd] | (Mem[Regs[rs] + offset5 + 2] << 16);
                    Regs[rd] = Regs[rd] | (Mem[Regs[rs] + offset5 + 3] << 24);
                    cout << "\n \t R" << rd << " has been modified to: "<<Regs[rd];
                    
                    
                case 3:
                    cout << "ldrb\trd,[rb,#" << offset5 << "]" << endl;
                    Regs[rd] = Mem[Regs[rs] + offset5];
                    cout << "\n \t R" << rd << " has been modified to: "<<Regs[rd];
                    break;
                    
                    // break;
                default:
                    printf("UNKNOWN INSTR\n");
                    
                    
            }
            break;
        }
            
            
        case 5:{        //formats 13 & 14
            unsigned int L = (instr >> 11) & 1;
            unsigned int R = (instr >> 8) & 1;
            unsigned int RList = instr & 0x007F;
            int rCount;
            
            int  bit10 = ((instr >> 10) & 1);
            int S = ((instr >> 7) & 1);
            
            unsigned int offset8 = (instr & 0x7F) >> 2;
            if (bit10 == 0) {        //format 13
                if (S == 0)
                {
                    SP += offset8; // adding immediate to SP
                    cout << "add\tSP,#" << offset8 << "\t" << endl;
                }
                else if (S == 1)
                {
                    SP -= offset8;  //subtracting imm. from sp
                    cout << "add\tSP,#-" << offset8 << endl;
                }
                else
                {
                    cout << "unkown instruction!" << endl;
                }
            }
            else {               //format 14
                if (L == 0) {         //pushing onto stack
                    if (R == 0) {
                        cout << "push\t{";
                        regPrint(RList, rCount);
                        cout << "}" << endl;
                        SP = SP + (rCount * regSize);
                    }
                    else {   //R == 1
                        cout << "push\t{";
                        regPrint(RList, rCount);
                        cout << ", LR}" << endl;
                        SP = SP + ((rCount + 1) * regSize);
                    }
                }
                else {      //L = 1 (pop)
                    if (R == 0) {
                        cout << "pop\t{";
                        regPrint(RList, rCount);
                        cout << "}" << endl;
                        SP = SP - (rCount * regSize);
                    }
                    else {
                        cout << "push\t{";
                        regPrint(RList, rCount);
                        cout << ", PC}" << endl;
                        SP = SP - ((rCount + 1) * regSize);
                    }
                }
            }
            
            break;
        }
            
            
        case 6:{     //formats 16 and 17
            unsigned int cond = (instr >> 8) & 0xF;
            unsigned int v8 = instr & 0x00FF;
            if (cond == 15) {             // format 17
                cout << "swi\t" << v8 << endl;
                switch(v8){
                    case 0x00:
                        //print char
                        cout << swiChar << endl;
                        break;
                    case 0x01:
                        cout << Regs[0] << endl;
                        break;
                    case 0x02:
                        //print string
                        cout << swiString << endl;
                        break;
                    case 0x03:
                        //reading int
                        cout << "Enter string: ";
                        cin >> Regs[0];
                        break;
                    case 0x04:
                        //reading char
                        cout << "Enter char: ";
                        cin >> swiChar;
                        break;
                    case 0x05:
                        //reading string
                        cout << "Enter string: ";
                        cin >> swiString;
                        break;
                    case 0x11:
                        //terminate
                        swiTerminate = true;
                        break;
                    default:
                        printf("UNKNOWN INSTR\n");
                }
            }
            else {                      // format 16
                switch (cond) {
                    case 0:
                        cout << "beq\tlabel" << v8 << endl;
                        PC = PC + Word;
                        //fseek(fp, SEEK_SET, PC + 2);
                        break;
                    case 1:
                        cout << "bne\tlabel" << v8 << endl;
                        PC = PC + Word;
                        //fseek(fp, SEEK_SET, PC + 2);
                        break;
                    case 2:
                        cout << "bcs\tlabel" << v8 << endl;
                        PC = PC + Word;
                        //fseek(fp, SEEK_SET, PC + 2);
                        break;
                    case 3:
                        cout << "bcc\tlabel" << v8 << endl;
                        PC = PC + Word;
                        //fseek(fp, SEEK_SET, PC + 2);
                        break;
                    case 4:
                        cout << "bmi\tlabel" << v8 << endl;
                        PC = PC + Word;
                        //fseek(fp, SEEK_SET, PC + 2);
                        break;
                    case 5:
                        cout << "bpl\tlabel" << v8 << endl;
                        PC = PC + Word;
                        //fseek(fp, SEEK_SET, PC + 2);
                        break;
                    case 6:
                        cout << "bvs\tlabel" << v8 << endl;
                        PC = PC + Word;
                        //fseek(fp, SEEK_SET, PC + 2);
                        break;
                    case 7:
                        cout << "bvc\tlabel" << v8 << endl;
                        PC = PC + Word;
                        //fseek(fp, SEEK_SET, PC + 2);
                        break;
                    case 8:
                        cout << "bhi\tlabel" << v8 << endl;
                        PC = PC + Word;
                        //fseek(fp, SEEK_SET, PC + 2);
                        break;
                    case 9:
                        cout << "bls\tlabel" << v8 << endl;
                        PC = PC + Word;
                        //fseek(fp, SEEK_SET, PC + 2);
                        break;
                    case 10:
                        cout << "bge\tlabel" << v8 << endl;
                        PC = PC + Word;
                        //fseek(fp, SEEK_SET, PC + 2);
                        break;
                    case 11:
                        cout << "blt\tlabel" << v8 << endl;
                        PC = PC + Word;
                        //fseek(fp, SEEK_SET, PC + 2);
                        break;
                    case 12:
                        cout << "bgt\tlabel" << v8 << endl;
                        PC = PC + Word;
                        //fseek(fp, SEEK_SET, PC + 2);
                        break;
                    case 13:
                        cout << "ble\tlabel" << v8 << endl;
                        PC = PC + Word;
                        //fseek(fp, SEEK_SET, PC + 2);
                        break;
                    case 14:
                        cout << "unidentified instruction" << endl;
                        break;
                        
                    default:
                        printf("UNKNOWN INSTR\n");
                        
                }
            }
            break;
        }
            
            
            
            
        case 7:{        //formats 18 & 19
            if (((instr >> 11) & 3) == 0) {        //if opcode == 0 ; -> format 18 (unconditional branch)
                int off;            //declare offset11 (bits 0-10)
                if (instr & 0x400)            //if sign bit == 1
                    off = (instr & 0x7FF) - 0x800;    //mask to get 11 right most bits (0-10) from instruction
                //Offset11 - 1000 0000 0000 = 2's complement
                //WE NEED TO UPDATE PC!!!!!!!!!!!!!!!!
                else
                    off = (instr & 0x7FF);    //else if sign bit == 0
                printf("B\t%d\t\t# Jump to (current instr. addr)+4%+d\n", off, off * 2);
                
                //WE NEED TO UPDATE PC!!!!!!!!!!!!!!!
            }
            else {                //format 19 (long branch woth link)
                int off;                    //CONSIDER SIGN BIT OF  ADDRESS??????????????????????/
                off = instr & 0x07FF;        //mask to get offset  (0000 0111 1111 1111)
                
                if (((instr >> 11) & 1) == 0)
                {    //High 'H' flag == 0  (instruction 1)
                    Regs[15] = Regs[15] + (off << 12);    //PC = PC + (offset << 12)
                    Regs[14] = Regs[15];        //LR = PC ????????????????????????????
                    //In the first instruction the Offset field contains the upper 11 bits of the target address.
                    //This is shifted left by 12 bits and added to the current PC address. The resulting
                    //address is placed in LR.
                }
                else {                //High 'H' flag == 1 (instruction 2)
                    Regs[14] = Regs[14] + (off << 1);    //LR = LR + (offset << 1)
                    Regs[15] = Regs[14];            //PC = LR
                    
                    //In the second instruction the Offset field contains an 11-bit representation lower half of
                    //the target address. This is shifted left by 1 bit and added to LR. LR, which now contains
                    //the full 23-bit address, is placed in PC, the address of the instruction following the BL
                    //is placed in LR and bit 0 of LR is set.
                    //The branch offset must take account of the prefetch operation, which causes the PC
                    //to be 1 word (4 bytes) ahead of the current instruction
                }
            }
            
            break;
        }
            
        default:
            printf("UNKNOWN INSTR!\n");
    }       //TO close first switch(fmt)
    
}       //to close function simulate

