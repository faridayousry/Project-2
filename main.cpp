#include "stdio.h"
#include "stdlib.h"

int simulate(unsigned short);

unsigned char Mem[1024];
unsigned int Regs[16];

#define	PC	Regs[15]
#define	LR	Regs[14]

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
            op = (instr >> 11) & 3;         //mask to get bits 11 &12 (opcode in format 1 & 3)
            rd = instr & 7;                 //mask to get 3 rightmost bits (rd for formats 1,2,4,7,8,9,10) 
            rs = (instr >>  3) & 7;         //mask to get bits 3,4,5 (rs for formats 1,2,4  &  rb for 7,8,9,10)
            offset5 = (instr >> 6) & 0x1F;  //mask to get bits 6-10 (offset5 for formats 1,9,10)
            if(op!=3) {     // format 1
                /*
                switch(op){
                    case 0: printf("lsl\tr%d, r%d, #%d\n", rd, rs, offset5); break;
                    case 1: printf("lsr\tr%d, r%d, #%d\n", rd, rs, offset5); break;
                    case 2: printf("asr\tr%d, r%d, #%d\n", rd, rs, offset5); break;
                
                }*/
            } else { /*add/sub*/      // format 2
                offset3 = rn = offset5 & 0x07;          //mask to get 3 right most bits of offset5
                if((offset5 & 0x08) == 0){              //if opcode (of format 2) == 0 -> add instruction 
                    printf("add\tr%d, r%d, ", rd, rs);      
                	if((offset5 & 0x10) == 0){          //check if the 'I' flag (immediate') is set:
                    	printf("r%d\n", rn);                //if so -> add format: "ADD rd, rs, rn"
                    	Regs[rd] = Regs[rs] + Regs[rn];         //update registers array 
				//OUTPUT "Ri has been updated" !!!!!!!!!!!!!!!11111
                    }
                	else {                              //else if 'I' flag (immediate') is not set:
                    	printf("#%d\n", offset3);           //-> add format: "ADD rd, rs, offset"
                    	Regs[rd] = Regs[rs] + offset3;          //update registers array 
				//OUTPUT "Ri has been updated" !!!!!!!!!!!!!!!11111
                    }
                }
                else {        //sub instruction
                    printf("sub\tr%d, r%d, ", rd, rs);
                    if((offset5 & 0x10) == 0){           //check if the 'I' flag (immediate') is set:
                    	printf("r%d\n", rn);                  //if so -> sub format: "SUB rd, rs, rn"
                    	Regs[rd] = Regs[rs] - Regs[rn];		  //update registers array 
				//OUTPUT "Ri has been updated" !!!!!!!!!!!!!!!11111

                    }
                	else {
                    	printf("#%d\n", offset3);          //else if 'I' flag (immediate') is not set: -> sub format: "SUB rd, rs, offset"  
                    	Regs[rd] = Regs[rs] - offset3;           //update registers array 
				//OUTPUT "Ri has been updated" !!!!!!!!!!!!!!!11111
                    }
				}    
            }
            break;
		  
		    
        case 1:		//formats 
		break;    
		    
		    
	case 2:		//formats 
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
