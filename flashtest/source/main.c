/*---------------------------------------------------------------------------------

	Basic template code for starting a DS app

---------------------------------------------------------------------------------*/
#include <nds.h>
#include <stdio.h>

#include <fat.h>

#include <nds/registers_alt.h>

void iSmart_setRomOP(u8 * command)  {
	u32 status ,index;

	do {
		status = REG_ROMCTRL;
	} while(status&0x80000000);  

	REG_AUXSPICNTH = (CARD_CR1_ENABLE)|(CARD_CR1_IRQ);

	for (index = 0; index < 8; index++) {
		CARD_COMMAND[7-index] = command[index];
	}
}


u32 iSmart_Read512BYTE(u32 address, u8* pbuf) {
	u32 status=0 ;

	u8 cmd[8];
	cmd[7] = 0xB5;
	cmd[6] = 0;
	cmd[5] = (address >> 16) & 0xff;
	cmd[4] = (address >> 8) & 0xff;
	cmd[3] = address & 0xff;
	cmd[2] = 0;
	cmd[1] = 0;
	cmd[0] = 0;

	iSmart_setRomOP(cmd);
 
	REG_ROMCTRL = 0xA1586000 ;

	u32 i  = 0 ;
	if(!((u32)pbuf&3)) {
		do{
			status = REG_ROMCTRL;
			if((status & 0x800000)&&(i<0x200))
			{
				((u32*)pbuf)[i] = REG_CARD_DATA_RD;
				i ++ ;
			}
		}while(status & 0x80000000);
	}
	return 0;
}

void  iSmart_ReadFlash(u32 address, u8* pbuf, size_t size) {
	size_t offset = 0;
	while (offset < size) {
		iSmart_Read512BYTE(address + offset, pbuf + offset);
		offset += 0x200;
	}
}


void display_hex(u8 *buffer, int lines)
{
	for (int y=0; y<lines; y++)
	{
		for (int x=0; x<8; x++)
			iprintf("%02x ", buffer[y*8 + x]);
		for (int x=0; x<8; x++)
			if (buffer[y*8 + x] >= 32 && buffer[y*8 + x] <= 127)
				iprintf("%c", buffer[y*8 + x]);
			else
				iprintf(".");
	}
}

#define BLOCKSIZE (16*1024)

//---------------------------------------------------------------------------------
int main(void) {
//---------------------------------------------------------------------------------
	consoleDemoInit();
	iprintf("iSmart flashtest!\n");

	if(fatInitDefault()) {
		sysSetCardOwner(BUS_OWNER_ARM9);

		iprintf("Reading Flash\n");

		u8 *flashbuf = malloc(BLOCKSIZE);


		u32 offset = 0;

		FILE *dump_file = fopen("ismart.bin","wb");

		do {
			iSmart_ReadFlash(0,flashbuf, BLOCKSIZE);
			offset += BLOCKSIZE;
			fwrite(flashbuf,1,BLOCKSIZE,dump_file);
			iprintf(".");
		} while (offset < 0x200000);

		fclose(dump_file);

		free(flashbuf);

		iprintf("\ndone!\n");

	} else {
		iprintf("FAT init failed!\n");
	}


	while(1) {
		swiWaitForVBlank();
		scanKeys();
		int pressed = keysDown();
		if(pressed & KEY_START) break;
	}

}
