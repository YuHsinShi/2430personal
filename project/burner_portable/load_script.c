#include <stdio.h>
#include <stdlib.h>
//#include "host.h"

#define VERBOSE 1
#define TIMEOUT 10000

#define DELAY(us)                               printf("delay %d us\n", us)
#define MEMORY_STORE(dst_addr, src_addr, len)   printf("memory write 0x%08x -> 0x%08x (len: %d)\n", src_addr, dst_addr, len)



//extern int read_scr(char *fname);
//extern int cmd_len;
//extern int *cmd;

enum {
    WAIT_CMD      = 0xffffffff,
    DATA_CMD      = 0xfffffffe,
    DATA_WAIT0_CMD= 0xfffffffd,
    DATA_WAIT1_CMD= 0xfffffffc,
    CALL_CMD      = 0xfffffffb,
    WRITE_MASK_CMD= 0xfffffffa,
    GOTO_CMD      = 0xfffffff9,
    READ_MASK_CMD = 0xfffffff8,
    SKIP_CMD      = 0xfffffff7,
    BEQ_CMD       = 0xffffffe0,
    BNE_CMD       = 0xffffffe1,
    BGT_CMD       = 0xffffffe2,
    BGTE_CMD      = 0xffffffe3,
    BLT_CMD       = 0xffffffe4,
    BLTE_CMD      = 0xffffffe5,
};

int load_script(char *fname,char delayus)
{
    int idx = 0;
	int result = 0;
//	int rval = 0;
	extern int* cmd;
	extern int  cmd_len;

    if (VERBOSE)
    {
        printf("initialize by script file <%s>\n", fname);
    }

    result=read_scr(fname);
	
	if(result!=0)
	{
		if(cmd) free(cmd);
		return result;
	}

    while(idx < cmd_len) {
        static int rval = 0;

        switch(cmd[idx])
        {

           case WRITE_MASK_CMD: // WRITE_MASK(addr, data, mask)
                        {
                            int addr = cmd[idx+1];
                            int val  = cmd[idx+2];
                            int mask = cmd[idx+3];
                            int data;

							HOST_ReadRegister(addr,&data,1);
							HOST_WriteRegister(addr, (data & mask) | (val & ~mask));

                            if (VERBOSE) printf("WRITE_MASK(0x%04x, 0x%04x, 0x%04x);\n", addr, val, mask);
                        }
                        idx+=4;
                        break;
		   
		   case WAIT_CMD: // WAIT(n)
                        {
                            int n = cmd[idx+1];
							int x;
							/*
							if(delayus == 1)
							{
								x =  n/1000;
								
								if ((n%1000) != 0)
								{
									x += 1;
								}

								Sleep(x);
							}
							else
							{
								Sleep(n);
							}
							*/
							usleep(n);
                            if(VERBOSE) printf("WAIT(%d);\n", n);

                        }

                        idx+=2;

                        break;

            case DATA_CMD: // DATA(base, height, width, pitch)
                        {
                            int base   = cmd[idx+1];
                            int height = cmd[idx+2];
                            int width  = cmd[idx+3];
                            int pitch  = cmd[idx+4];
                            char *src = (char*)cmd[idx+5];
							#if 0
                            // memcpy from src to base for height*width with pitch
                            int i;
                            for (i = 0; i < height; i++)
							{	
							    HOST_WriteBlockMemory(base, (unsigned int)(&src), width);
								src  += width;
								base += pitch;
							}
							#else
								printf("ERROR: not support cmd \n");
							#endif 
                            idx += (height*width) / sizeof(int);
                        }
			
                        idx+=5;
                        break;

            case DATA_WAIT0_CMD: // DATA_WAIT0(addr, mask)
                        {
                            int addr = cmd[idx+1];
                            int mask = cmd[idx+2];
                            int data;
                            int timeout = 0;

                            do {
//								data = HOST_ReadRegister(addr);
								HOST_ReadRegister(addr,&data,1);

                                if (timeout++ > TIMEOUT) break;
                                else DELAY(1);
                            } while(data & mask);

                            if (VERBOSE) printf("DATA_WAIT0(0x%04x, 0x%04x);\n", addr, mask);
                        }
                        idx+=3;
                        break;

            case DATA_WAIT1_CMD: // DATA_WAIT1(addr, mask)
                        {
                            int addr = cmd[idx+1];
                            int mask = cmd[idx+2];
                            int data;
                            int timeout = 0;

                            do {
//								data = HOST_ReadRegister(addr);
								HOST_ReadRegister(addr,&data,1);

                                if (timeout++ > TIMEOUT) break;
                                else DELAY(1);
                            } while(!(data & mask));

                            if (VERBOSE) printf("DATA_WAIT1(0x%04x, 0x%04x);\n", addr, mask);
                        }
                        idx+=3;
                        break;


            case READ_MASK_CMD: // READ_MASK(addr, mask)
                        {
                            int addr = cmd[idx+1];
                            int mask = cmd[idx+2];
                            unsigned int data ;
	
//                          data = HOST_ReadRegister(addr);
						    HOST_ReadRegister(addr,&data,1);

                            rval = ((unsigned int)data) & mask;

                            if (VERBOSE) printf("READ_MASK(0x%04x, 0x%04x);\n", addr, mask);

                        }
                        idx+=3;

                        break;
            case SKIP_CMD: // SKIP(offset)

                        {
                            int offset = cmd[idx+1];
                            idx += offset;
                            if (VERBOSE) printf("SKIP(0x%04x);\n", offset);
                        }
                        idx += 2;

                        break;

            case BEQ_CMD: // BEQ(val, offset)
                        {
                            int val = cmd[idx+1];
                            int offset = cmd[idx+2];
                            if ((unsigned)val == (unsigned)rval) idx += offset;
                            if (VERBOSE) printf("BEQ(0x%04x, 0x%04x);\n", val, offset);
                        }
                        idx += 3;
                        break;

            case BNE_CMD: // BNE(val, offset)

                        {
                            int val = cmd[idx+1];
                            int offset = cmd[idx+2];
                            if ((unsigned)val != (unsigned)rval) idx += offset;
                            if (VERBOSE) printf("BNE(0x%04x, 0x%04x);\n", val, offset);
                        }

                        idx += 3;

                        break;

            case BGT_CMD: // BGT(val, offset)
                        {
                            int val = cmd[idx+1];
                            int offset = cmd[idx+2];
                            if ((unsigned)rval > (unsigned)val) idx += offset;
                            if (VERBOSE) printf("BGT(0x%04x, 0x%04x);\n", val, offset);
                        }
                        idx += 3;
                        break;

            case BGTE_CMD: // BGTE(val, offset)
                        {
                            int val = cmd[idx+1];
                            int offset = cmd[idx+2];
                            if ((unsigned)rval >= (unsigned)val) idx += offset;
                            if (VERBOSE) printf("BGTE(0x%04x, 0x%04x);\n", val, offset);
                        }
                        idx += 3;
                        break;

            case BLT_CMD: // BLT(val, offset)
                        {
                            int val = cmd[idx+1];
                            int offset = cmd[idx+2];
                            if ((unsigned)rval < (unsigned)val) idx += offset;
                            if (VERBOSE) printf("BLT(0x%04x, 0x%04x);\n", val, offset);
                        }
                        idx += 3;
                        break;

            case BLTE_CMD: // BLTE(val, offset)
                        {
                            int val = cmd[idx+1];
                            int offset = cmd[idx+2];
                            if ((unsigned)rval <= (unsigned)val) idx += offset;
                            if (VERBOSE) printf("BLTE(0x%04x, 0x%04x);\n", val, offset);
                        }
                        idx += 3;
                        break;

            default: // WRITE(addr, data)
                        {
                            int addr=cmd[idx];
                            int data=cmd[idx+1];

                            HOST_WriteRegister(addr, data);

                            if (VERBOSE) printf("WRITE(0x%04x, 0x%04x);\n", addr, data);
                        }
                        idx+=2;

                        break;
        }
    }
	
	if(cmd) free(cmd);	
    return 0;	
}


//int main(void) {
//    load_script("test.scr");
//}


