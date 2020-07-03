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
                    }
                	else {                              //else if 'I' flag (immediate') is not set:
                    	printf("#%d\n", offset3);           //-> add format: "ADD rd, rs, offset"
                    	Regs[rd] = Regs[rs] + offset3;          //update registers array 
                    }
                }
                else {        //sub instruction
                    printf("sub\tr%d, r%d, ", rd, rs);
                    if((offset5 & 0x10) == 0){           //check if the 'I' flag (immediate') is set:
                    	printf("r%d\n", rn);                  //if so -> sub format: "SUB rd, rs, rn"
                    	Regs[rd] = Regs[rs] - Regs[rn];
                    }
                	else {
                    	printf("#%d\n", offset3);          //else if 'I' flag (immediate') is not set:
                    	Regs[rd] = Regs[rs] - offset3;         //-> sub format: "SUB rd, rs, offset"  
                    }
				}
                
                    
                
            }
            break;
        /*    
        case 7:
            if(((instr>>11)&3) == 0) {
                int off;
                if(instr & 0x400)
                    off = (instr & 0x7FF) - 0x800;
                else
                    off = (instr & 0x7FF);
                printf("B\t%d\t\t# Jump to (current instr. addr)+4%+d\n", off,off*2);
            }
            break;
        */
        default:
            printf("UNKNOWN INSTR!\n");
    }

}