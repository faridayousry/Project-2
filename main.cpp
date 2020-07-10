#include "stdio.h"
#include "stdlib.h"
#include <iostream>
using namespace std;
	
FARIDAAAA
Ahmed!!!

int simulate(unsigned short);

unsigned char Mem[1024];
unsigned int Regs[16];

#define	PC	Regs[15]
#define	LR	Regs[14]

//SHOULD WE DEFINE A PSR (CPSR) program status register to use a flag/store the result of cmp instruction

int main() {
    FILE *fp;
    unsigned short inst_word;

    fp = fopen("test.s.bin","rb");

    if(NULL==fp) {
        printf("Cannot open the file\n");
        exit(0);
    }

    while(fread(&inst_word, 2,1, fp))
    {
        printf("%08x\t%04x\t", PC, inst_word);
        simulate(inst_word);
        PC += 2;
    }
    fclose(fp);
    return 0;
}


int simulate(unsigned short instr)
{
    unsigned char fmt, op, offset5, rd, rs, offset3, rn;

    fmt = (instr) >> 13;

    switch(fmt){
        case 0:             // format 1/2
            op = (instr >> 11) & 3;         //mask to get bits 11 &12 (opcode in format 1)
            rd = instr & 7;                 //mask to get 3 rightmost bits (rd for formats 1,2,4,7,8,9,10) 
            rs = (instr >>  3) & 7;         //mask to get bits 3,4,5 (rs for formats 1,2,4  &  rb for 7,8,9,10)
            offset5 = (instr >> 6) & 0x1F;  //mask to get bits 6-10 (offset5 for formats 1,9,10)
            if(op!=3) {     // format 1
                
                switch(op){
                    case 0: printf("lsl\tr%d, r%d, #%d\n", rd, rs, offset5);	//logical left shift instruction
			    Regs[rd] = (Regs[rs] << offset5 );
			    cout << "\n \t R"<< rd << " has been updated";
				break;
				
                    case 1: printf("lsr\tr%d, r%d, #%d\n", rd, rs, offset5); 	//logical right shift instruction
			    Regs[rd] = (Regs[rs] >> offset5 );
			    cout << "\n \t R"<< rd << " has been updated";
				break;
				
                    case 2: printf("asr\tr%d, r%d, #%d\n", rd, rs, offset5);  	//arithmetic right shift instruction
			    Regs[rd] = (Regs[rs] >> offset5 );			//NEEDS TO CHANGE TO CHECK SIGN BIT!!!!!!!!!!!!!!!!!!!!!1
			    cout << "\n \t R"<< rd << " has been updated";
				break;
		default:
            		printf("UNKNOWN INSTR\n");
                
                
            } else { /*add/sub*/      // format 2
                offset3 = rn = offset5 & 0x07;          //mask to get 3 right most bits of offset5
                if((offset5 & 0x08) == 0){              //if opcode (of format 2) == 0 -> add instruction 
                    printf("add\tr%d, r%d, ", rd, rs);      
                	if((offset5 & 0x10) == 0){          //check if the 'I' flag (immediate') is set:
                    	printf("r%d\n", rn);                //if so -> add format: "ADD rd, rs, rn"
                    	Regs[rd] = Regs[rs] + Regs[rn];         //update registers array 
			 cout << "\n \t R"<< rd << " has been updated";
                    }
                	else {                              //else if 'I' flag (immediate') is not set:
                    	printf("#%d\n", offset3);           //-> add format: "ADD rd, rs, offset"
                    	Regs[rd] = Regs[rs] + offset3;          //update registers array 
			cout << "\n \t R"<< rd << " has been updated";
                    }
                }
                else {        //sub instruction
                    printf("sub\tr%d, r%d, ", rd, rs);
                    if((offset5 & 0x10) == 0){           //check if the 'I' flag (immediate') is set:
                    	printf("r%d\n", rn);                  //if so -> sub format: "SUB rd, rs, rn"
                    	Regs[rd] = Regs[rs] - Regs[rn];		  //update registers array 
			cout << "\n \t R"<< rd << " has been updated";

                    }
                	else {
                    	printf("#%d\n", offset3);          //else if 'I' flag (immediate') is not set: -> sub format: "SUB rd, rs, offset"  
                    	Regs[rd] = Regs[rs] - offset3;           //update registers array 
			cout << "\n \t R"<< rd << " has been updated";
                    }
				}    
            }
            break;
		  
		    
        case 1:		//format 3  (move/compare/add/subtract Immediate)
		op = (instr >> 11) & 3;         //mask to get bits 11 &12 (opcode)
           	rd = (instr >> 8) & 7;                 //mask to get 3 bits f (bits 8,9,19 for rd) 
           	offset8 = instr  & 0xFF;  	//mask to get bits 0-7 (offset8)
		    
		switch(op){
                	case 0: printf("mov\tr%d, #%d\n", rd, offset8);		//MOV imm instruction
			    Regs[rd] =  offset8 ;
			    cout << "\n \t R"<< rd << " has been updated";
				break;
				
			case 1: printf("cmp\tr%d, #%d\n", rd, offset8);		//cmp imm instruction
			    //(Regs[r] =  offset8)? 1;0		//USE CPSR ??
				break;
				
			case 2: printf("add\tr%d, #%d\n", rd, offset8);		//add imm instruction
			    Regs[rd] =  Regs[rd] + offset8 ;
			    cout << "\n \t R"<< rd << " has been updated";
				break;
				
			case 3: printf("sub\tr%d, #%d\n", rd, offset8);		//sub imm instruction
			    Regs[rd] =  Regs[rd] - offset8 ;
			    cout << "\n \t R"<< rd << " has been updated";
				break;
		default:
            		printf("UNKNOWN INSTR\n");
			
		break;    
		    
		    
	case 2:		//formats 4, 5, 6, 7, 8
		op = (instr >> 11) & 3;         //mask to get bits 11 &12 (opcode)
		rd = instr & 7;                 //mask to get 3 rightmost bits (rd for formats 4,5,7,8) 
		rs = (instr >>  3) & 7;         //mask to get bits 3,4,5 (rs for formats 4,5  &  rb for 7,8)
		offset5 = (instr >> 6) & 0x1F;  //mask to get bits 6-10 (offset5 for formats 1,9,10)
				
                switch(op){
                    case 0: 		//formats 4 & 5     (ALU operations & Hi register ops/branch exchange)
                        if ( (instr >> 10) & 1) == 0 {	   //format 4  
                            op = (instr >> 6) & 15;	 //adjust op respective to be bits 6-9 for format 4 
                            
                            switch(op){
                                case 0:		//and instruction (rd = rd & rs)
                                        printf("and\tr%d, r%d\n", rd, rs);
                                        Regs[rd] = Regs[rd] & Regs[rs];
                                            cout << "\n \t R"<< rd << " has been updated";
                                            break;

                                case 1:     //eor instruction (rd = rd eor rs)
                                        printf("eor\tr%d, r%d\n", rd, rs);
                                        Regs[rd] = Regs[rd] eor Regs[rs];
                                            cout << "\n \t R"<< rd << " has been updated";
                                            break;

                                case 2:		//lsl instruction (rd = rd << rs)
                                        printf("lsl\tr%d, r%d\n", rd, rs);
                                        Regs[rd] = Regs[rd] << Regs[rs];
                                            cout << "\n \t R"<< rd << " has been updated";
                                            break;

                                case 3:     //lsr instruction (rd = rd >> rs)
                                        printf("lsr\tr%d, r%d\n", rd, rs);
                                        Regs[rd] = Regs[rd] >> Regs[rs];
                                            cout << "\n \t R"<< rd << " has been updated";
                                            break;

                                 case 4:		//asr instruction (rd = rd asr rs)
                                        printf("asr\tr%d, r%d\n", rd, rs);
                                        Regs[rd] = Regs[rd] asr Regs[rs];
                                            cout << "\n \t R"<< rd << " has been updated";
                                            break;

                                case 5:     //adc instruction (rd = rd + rs + c-bit)
                                        printf("adc\tr%d, r%d\n", rd, rs);
                                        Regs[rd] = Regs[rd] adc Regs[rs];
                                            cout << "\n \t R"<< rd << " has been updated";
                                            break;

                                case 6:		//sbc instruction(rd = rd + rs + NOT c-bit)
                                        printf("sbc\tr%d, r%d\n", rd, rs);
                                        Regs[rd] = Regs[rd] sbc Regs[rs];
                                            cout << "\n \t R"<< rd << " has been updated";
                                            break;

                                case 7:     //ror instruction (rd = rd ror rs)
                                        printf("ror\tr%d, r%d\n", rd, rs);
                                        Regs[rd] = Regs[rd] ror Regs[rs];
                                            cout << "\n \t R"<< rd << " has been updated";
                                            break;

                                case 8:		//tst instruction (rd = rd << rs)
                                        printf("tst\tr%d, r%d\n", rd, rs);
                                        //Regs[rd] = Regs[rd] tst Regs[rs];  - set condition codes
                                            //cout << "\n \t R"<< rd << " has been updated";
                                            break;

                                case 9:     //neg instruction (rd = rd >> rs)
                                        printf("neg\tr%d, r%d\n", rd, rs);
                                        Regs[rd] = -Regs[rs];
                                            cout << "\n \t R"<< rd << " has been updated";
                                            break;

                            }	
                                break;
                            default:
                                        printf("UNKNOWN INSTR\n");
                            }
                        
                        else{				//format 5      //NOT FULLY SURE!!!!!!!! 	[B O N U S]
                            op = (instr >> 8) & 3;	 //adjust op respective to be bits 8-9 for format 5
                            int hs = rs;
                            int hd = rd;
                            int H = (instr >> 6) & 3;   //mask to get H1 and H2 combined in 1 variable

                            switch(op){
                                case 0:		//add instructions hi & lo 

                                        if( H == 1)     //ADD Rd, Hs
                                        {
                                            printf("add\tr%d, r%d\n", rd, hs);
                                            Regs[rd] = Regs[rd] + Regs[hs];
                                                cout << "\n \t R"<< rd << " has been updated";
                                        }

                                        else if( H == 2)     //ADD Hd, Rs
                                        {
                                            printf("add\tr%d, r%d\n", hd, rs);
                                            Regs[hd] = Regs[hd] + Regs[rs];
                                                cout << "\n \t R"<< hd << " has been updated";
                                        }

                                         else    //H = 3 ; ADD Hd, Hs
                                        {
                                            printf("add\tr%d, r%d\n", hd, hs);
                                            Regs[hd] = Regs[hd] + Regs[hs];
                                                cout << "\n \t R"<< hd << " has been updated";
                                        }


                                            break;

                                case 1:    //cmp instructions hi & lo 

                                        if( H == 1)     //cmp Rd, Hs
                                        {
                                            printf("cmp\tr%d, r%d\n", rd, hs);
                                            //Regs[rd] == Regs[hs]? 0:1
                                                //cout << "\n \t R"<< rd << " has been updated";
                                        }

                                        else  if( H == 2)     //cmp hd, rs
                                        {
                                            printf("cmp\tr%d, r%d\n", hd, rs);
                                            //Regs[hd] == Regs[rs]? 0:1
                                                //cout << "\n \t R"<< hd << " has been updated";
                                        }

                                         else     //H == 3 ; cmp hd, hs
                                        {
                                            printf("cmp\tr%d, r%d\n", hd, hs);
                                            //Regs[hd] == Regs[hs]? 0:1
                                                //cout << "\n \t R"<< hd << " has been updated";
                                        }


                                            break;

                                case 2:		//mov instructions hi & lo 
                                        if (H == 1){        //mov rd, hs
                                            printf("mov\tr%d, r%d\n", rd, hs);
                                            Regs[rd] = Regs[hs] ;
                                                cout << "\n \t R"<< rd << " has been updated";
                                        }
                                        else if (H == 2){        //mov hd, rs
                                            printf("mov\tr%d, r%d\n", hd, rs);
                                            Regs[hd] = Regs[rs] ;
                                                cout << "\n \t R"<< hd << " has been updated";
                                        }
                                        else{        // H == 2 ; mov hd, hs
                                            printf("mov\tr%d, r%d\n", hd, hs);
                                            Regs[hd] = Regs[hs] ;
                                                cout << "\n \t R"<< hd << " has been updated";
                                        }

                                                break;

                                case 3:    //bx instructions hi & lo 
                                        if(H == 0){     //BX rs
                                            printf("bx\tr%d\n", rs);
                                            // UPDATE PC   ??????????????????????????????????????????????
                                            //PC = PC + rs ?????
                                                //cout << "\n \t PC has been updated";
                                        }
                                        else{   //H == 11; BX hs
                                            printf("bx\tr%d\n", hs);
                                            // UPDATE PC   ??????????????????????????????????????????????
                                            //PC = PC + hs ?????
                                                //cout << "\n \t PC has been updated";

                                        }

                                                break;

                            }	
                                break;
                            default:
                                        printf("UNKNOWN INSTR\n");
                            }

            }
		        default:
            		printf("UNKNOWN INSTR\n");
				
		break;   
		    
            
		    
	case 3:		//formats 
		break;  
		    
		    
	case 4:		//formats 
		break;      
		    
		    
	case 5:		//formats 
		break;      
		    
		    
	case 6:		//formats 
		break;  
		   
		    
        case 7:		//formats 18 & 19
            if(((instr>>11)&3) == 0) {		//if opcode == 0 ; -> format 18 (unconditional branch)
                int off;			//declare offset11 (bits 0-10)  
                if(instr & 0x400)		    //if sign bit == 1
                    off = (instr & 0x7FF) - 0x800;	//mask to get 11 right most bits (0-10) from instruction
		    						//Offset11 - 1000 000 0000 = 2's complement ?????
		    //WE NEED TO UPDATE PC!!!!!!!!!!!!!!!!
                else
                    off = (instr & 0x7FF);	//else if sign bit == 0
                printf("B\t%d\t\t# Jump to (current instr. addr)+4%+d\n", off,off*2);	
		   
		   //WE NEED TO UPDATE PC!!!!!!!!!!!!!!! 
            }
	    else{				//format 19 (long branch woth link)
		int off;					//CONSIDER SIGN BIT OF  ADDRESS????????????
		off = instr & 0x07FF;		//mask to get offset  (0000 0111 1111 1111)
		    
		if(((instr>>11)&1) == 0){	//High 'H' flag == 0  (instruction 1)
			Regs[15] = Regs[15] + (off << 12);	//PC = PC + (offset << 12)
			Regs[14] = Regs[15];		//LR = PC ????????????????????????????
			//In the first instruction the Offset field contains the upper 11 bits of the target address.
			//This is shifted left by 12 bits and added to the current PC address. The resulting
			//address is placed in LR.
		
		else{				//High 'H' flag == 1 (instruction 2)
			Regs[14] = Regs[14] + (off << 1);	//LR = LR + (offset << 1)
			Regs[15] = Regs[14];			//PC = LR
			
			//In the second instruction the Offset field contains an 11-bit representation lower half of
			//the target address. This is shifted left by 1 bit and added to LR. LR, which now contains
			//the full 23-bit address, is placed in PC, the address of the instruction following the BL
			//is placed in LR and bit 0 of LR is set.
			//The branch offset must take account of the prefetch operation, which causes the PC
			//to be 1 word (4 bytes) ahead of the current instruction
		}
	    }
            break;
        
        default:
            printf("UNKNOWN INSTR!\n");
    }

}
