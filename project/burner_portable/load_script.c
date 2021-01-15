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

int glamomcu_load_init_script(char *fname)
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

                            if (VERBOSE) printf("READ_MASK(0x%04x, 0x%04x) 0x%x;\n", addr, mask,rval);

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

#define GLAMO_RAM_LOAD_CHUNK    (64 * 1024)
#define GLAMO_RAM_SAVE_CHUNK    (64 * 1024)

#define GLAMO_RAM_ADDR_986X    (0)



static int spi_write_burn(uint32_t ctrlLen, uint8_t* pCtrlBuf, uint32_t dataLen, uint8_t* pDataBuf)
{
	uint32_t data_size = ctrlLen + dataLen;
	uint8_t *data ;

	if (data_size > 0)
	{
	
		data = malloc(data_size*sizeof(uint8_t));
		data_size = 0;
		if (ctrlLen > 0 && pCtrlBuf != NULL)
		{
			memcpy(&data[0], pCtrlBuf, ctrlLen);
			data_size += ctrlLen;
		}
		
		if (dataLen > 0 && pDataBuf != NULL)
		{
			memcpy(&data[data_size], pDataBuf, dataLen);
			data_size += dataLen;
		}
		
		burnport_write_data( data, data_size);

		free(data);
	}

	
	return 0;
}


static void spi_write_vram(unsigned long dest, const void *src, unsigned long size)
{
#define SECTION_SIZE       (0x10000 - 1)

	unsigned char wrBuf[8];

    {
        unsigned int secSize    = 0;
        uint32_t     error;
        uint32_t     srcAddress = (uint32_t)src;

        while (size)
        {
            uint32_t wrLen = 5;

            secSize = (size > SECTION_SIZE)
                      ? SECTION_SIZE
                      : size;

            wrBuf[0] = 4;
            wrBuf[1] = (uint8_t)(dest & 0x000000FF);
            wrBuf[2] = (uint8_t)((dest & 0x0000FF00) >> 8);
            wrBuf[3] = (uint8_t)((dest & 0x00FF0000) >> 16);
            wrBuf[4] = (uint8_t)((dest & 0xFF000000) >> 24);


            error    = burnport_write_data(wrLen, wrBuf, secSize, (uint8_t*)srcAddress);
            if (error != 0)
            {
                printf("\n%s:%s:Ack Error! Error: 0x%08x\n", __TIME__, __FUNCTION__, error);
            }

            size       -= secSize;
            srcAddress += secSize;
            dest       += secSize;
        }
    }
}



int glamomcu_load_ram(const char *file)
{
    int ret;
    uint8_t buf[GLAMO_RAM_LOAD_CHUNK];
    uint32_t len;
    uint32_t i;
    uint32_t chunk;
    unsigned long t;
    FILE *f;
    size_t file_size;

    
    printf("load file <%s> to ram\n", file);


    f = fopen(file, "rb");
    if (f == NULL)
    {
        printf("cannot open %s for reading \n", (const char *)file);
		return -1;
    }

    if (fseek(f, 0, SEEK_END))
    {
        printf("cannot seek file %s\n",  (const char *)file);
		return -1;
    }

    file_size = ftell(f);

    if (fseek(f, 0, SEEK_SET))
    {
        printf("cannot seek file %s\n", (const char *)file);
		fclose(f);
		return -1;
    }
/*
    if (options.size_valid)
    {
        if (options.size > file_size)
        {
            error("specified size %d > file size %d\n", options.size, file_size);
            fclose(f);
            return -1;
        }
        len = options.size;
    }
    else 
        len = (uint32_t)file_size;

    if (len > glamomcu_target.ram_size - options.addr)
    {
        error("size %d > ram size %d\n", len, glamomcu_target.ram_size - options.addr);
        fclose(f);
        return EINVAL;
    }
*/
	len = (uint32_t)file_size;
	/*
	get_ite_chip_id();

	dummy_fake();
	usleep(1000);
	dummy_fake2();
	usleep(1000);

	get_ite_chip_id();
	usleep(1000);
*/

    chunk = (GLAMO_RAM_LOAD_CHUNK > (32 * 1024)) ?  (32 * 1024) : GLAMO_RAM_LOAD_CHUNK;
  //  t     = progress_start("RAM load: data");

    for (i = 0; i < len; i += chunk)
    {
        if (i + chunk > len)
            chunk = len - i;

        if (fread(buf, 1, chunk, f) != chunk)
        {
            printf("cannot read file %s (%s)\n",
                  (const char *)file);
            fclose(f);
            return -1;
        }

      spi_write_vram(GLAMO_RAM_ADDR_986X + i, buf, chunk);

       // progress_report(i + chunk, len);
    }
    //progress_stop(t, "RAM load: data", len);

    if (fclose(f) == -1)
    {
        printf("cannot close file %s (%s)\n",
              (const char *)file);
        return -1;
    }

    return 0;
}









void showMessage(char* msg)
{
	printf("%s",msg);
}
//int main(void) {
//    load_script("test.scr");
//}


