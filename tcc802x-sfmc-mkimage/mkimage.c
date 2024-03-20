#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mkimage.h"
#include "snor_mio.h"

#define HEADER_POS_IMAGE_SIZE		(0x180)
#define HEADER_POS_IMAGE_CRC		(0x184)

#define SNOR_PAGE_SIZE				(256)

char *ROM_CODE_NAME[] = {
	"ROM_CODE_CM4",
	"ROM_CODE_WARM_BOOT",
	"PRE_LOADER",	//"ROM_CODE_DRAM_INIT",
	"ROM_CODE_BOOT_ROM",
	"ROM_CODE_UNKNOWN",
};

static const unsigned long int CRC32_TABLE[256] = {
    0x00000000, 0x90910101, 0x91210201, 0x01B00300,
    0x92410401, 0x02D00500, 0x03600600, 0x93F10701,
    0x94810801, 0x04100900, 0x05A00A00, 0x95310B01,
    0x06C00C00, 0x96510D01, 0x97E10E01, 0x07700F00,
    0x99011001, 0x09901100, 0x08201200, 0x98B11301,
    0x0B401400, 0x9BD11501, 0x9A611601, 0x0AF01700,
    0x0D801800, 0x9D111901, 0x9CA11A01, 0x0C301B00,
    0x9FC11C01, 0x0F501D00, 0x0EE01E00, 0x9E711F01,
    0x82012001, 0x12902100, 0x13202200, 0x83B12301,
    0x10402400, 0x80D12501, 0x81612601, 0x11F02700,
    0x16802800, 0x86112901, 0x87A12A01, 0x17302B00,
    0x84C12C01, 0x14502D00, 0x15E02E00, 0x85712F01,
    0x1B003000, 0x8B913101, 0x8A213201, 0x1AB03300,
    0x89413401, 0x19D03500, 0x18603600, 0x88F13701,
    0x8F813801, 0x1F103900, 0x1EA03A00, 0x8E313B01,
    0x1DC03C00, 0x8D513D01, 0x8CE13E01, 0x1C703F00,
    0xB4014001, 0x24904100, 0x25204200, 0xB5B14301,
    0x26404400, 0xB6D14501, 0xB7614601, 0x27F04700,
    0x20804800, 0xB0114901, 0xB1A14A01, 0x21304B00,
    0xB2C14C01, 0x22504D00, 0x23E04E00, 0xB3714F01,
    0x2D005000, 0xBD915101, 0xBC215201, 0x2CB05300,
    0xBF415401, 0x2FD05500, 0x2E605600, 0xBEF15701,
    0xB9815801, 0x29105900, 0x28A05A00, 0xB8315B01,
    0x2BC05C00, 0xBB515D01, 0xBAE15E01, 0x2A705F00,
    0x36006000, 0xA6916101, 0xA7216201, 0x37B06300,
    0xA4416401, 0x34D06500, 0x35606600, 0xA5F16701,
    0xA2816801, 0x32106900, 0x33A06A00, 0xA3316B01,
    0x30C06C00, 0xA0516D01, 0xA1E16E01, 0x31706F00,
    0xAF017001, 0x3F907100, 0x3E207200, 0xAEB17301,
    0x3D407400, 0xADD17501, 0xAC617601, 0x3CF07700,
    0x3B807800, 0xAB117901, 0xAAA17A01, 0x3A307B00,
    0xA9C17C01, 0x39507D00, 0x38E07E00, 0xA8717F01,
    0xD8018001, 0x48908100, 0x49208200, 0xD9B18301,
    0x4A408400, 0xDAD18501, 0xDB618601, 0x4BF08700,
    0x4C808800, 0xDC118901, 0xDDA18A01, 0x4D308B00,
    0xDEC18C01, 0x4E508D00, 0x4FE08E00, 0xDF718F01,
    0x41009000, 0xD1919101, 0xD0219201, 0x40B09300,
    0xD3419401, 0x43D09500, 0x42609600, 0xD2F19701,
    0xD5819801, 0x45109900, 0x44A09A00, 0xD4319B01,
    0x47C09C00, 0xD7519D01, 0xD6E19E01, 0x46709F00,
    0x5A00A000, 0xCA91A101, 0xCB21A201, 0x5BB0A300,
    0xC841A401, 0x58D0A500, 0x5960A600, 0xC9F1A701,
    0xCE81A801, 0x5E10A900, 0x5FA0AA00, 0xCF31AB01,
    0x5CC0AC00, 0xCC51AD01, 0xCDE1AE01, 0x5D70AF00,
    0xC301B001, 0x5390B100, 0x5220B200, 0xC2B1B301,
    0x5140B400, 0xC1D1B501, 0xC061B601, 0x50F0B700,
    0x5780B800, 0xC711B901, 0xC6A1BA01, 0x5630BB00,
    0xC5C1BC01, 0x5550BD00, 0x54E0BE00, 0xC471BF01,
    0x6C00C000, 0xFC91C101, 0xFD21C201, 0x6DB0C300,
    0xFE41C401, 0x6ED0C500, 0x6F60C600, 0xFFF1C701,
    0xF881C801, 0x6810C900, 0x69A0CA00, 0xF931CB01,
    0x6AC0CC00, 0xFA51CD01, 0xFBE1CE01, 0x6B70CF00,
    0xF501D001, 0x6590D100, 0x6420D200, 0xF4B1D301,
    0x6740D400, 0xF7D1D501, 0xF661D601, 0x66F0D700,
    0x6180D800, 0xF111D901, 0xF0A1DA01, 0x6030DB00,
    0xF3C1DC01, 0x6350DD00, 0x62E0DE00, 0xF271DF01,
    0xEE01E001, 0x7E90E100, 0x7F20E200, 0xEFB1E301,
    0x7C40E400, 0xECD1E501, 0xED61E601, 0x7DF0E700,
    0x7A80E800, 0xEA11E901, 0xEBA1EA01, 0x7B30EB00,
    0xE8C1EC01, 0x7850ED00, 0x79E0EE00, 0xE971EF01,
    0x7700F000, 0xE791F101, 0xE621F201, 0x76B0F300,
    0xE541F401, 0x75D0F500, 0x7460F600, 0xE4F1F701,
    0xE381F801, 0x7310F900, 0x72A0FA00, 0xE231FB01,
    0x71C0FC00, 0xE151FD01, 0xE0E1FE01, 0x7070FF00
};

// SPI Read (STR)
sSFQPI_InitHeader stCODE_FAST_READ3B_for_CHIPBOOT = {
	0x00000090,		// code;	// 57MHz
	0x00040200, 	// timing;
	0x00000400, 	// delay_so;
	0x00000000, 	// dc_clk;
	0x00000000,		// dc_wbd0;
	0x00000000,		// dc_wbd1;
	0x00000000,		// dc_rbd0;
	0x00000000,		// dc_rbd1;
	0x00000000,		// dc_woebd0;
	0x00000000,		// dc_woebd1;
	0x00000814,		// dc_base_addr_manu_0;
	0x00000818,		// dc_base_addr_manu_1;
	0x00000800,		// dc_base_addr_auto;
	0x00000001,		// run_mode;
	0x524F4E53,		// SROM signature
	0x4D4F525F,
					// code
	{
		0x8400000B,		// 0x800	// FAST_READ-3B
		0x48000001,		// 0x804
		0x44001000,		// 0x808
		0x28000000,		// 0x80C
		0xF4000000,		// 0x810	// STOP

		0xF4000000,		// 0x814	// STOP
		0xF4000000,		// 0x818	// STOP
	},

	0x00000077		// crc
};

// SPI Read (STR)
sSFQPI_InitHeader stCODE_FAST_READ4B_for_CHIPBOOT = {
	0x00000090,		// code;	// 57MHz
	0x00040200, 	// timing;
	0x00000400, 	// delay_so;
	0x00000000, 	// dc_clk;
	0x00000000,		// dc_wbd0;
	0x00000000,		// dc_wbd1;
	0x00000000,		// dc_rbd0;
	0x00000000,		// dc_rbd1;
	0x00000000,		// dc_woebd0;
	0x00000000,		// dc_woebd1;
	0x00000814,		// dc_base_addr_manu_0;
	0x00000818,		// dc_base_addr_manu_1;
	0x00000800,		// dc_base_addr_auto;
	0x00000001,		// run_mode;
	0x524F4E53,		// SROM signature
	0x4D4F525F,
					// code
	{
		0x8400000C, 	// 0x800	// FAST_READ-3B
		0x48000000, 	// 0x804
		0x44001000,		// 0x808
		0x28000000,		// 0x80C
		0xF4000000,		// 0x810	// STOP

		0xF4000000,		// 0x814	// STOP
		0xF4000000,		// 0x818	// STOP
	},

	0x00000077		// crc
};

// SPI Read (STR) : TBD
sSFQPI_InitHeader stCODE_FAST_READ = {
	0x00003070,		// code;	//0x7: 116MHz
	0x00020404, 	// timing;
	0x000001FF, 	// delay_so;
	0x0000000F, 	// dc_clk;
	0x00000000,		// dc_wbd0;
	0x00000000,		// dc_wbd1;
	0x00000000,		// dc_rbd0;
	0x00000000,		// dc_rbd1;
	0x00000000,		// dc_woebd0;
	0x00000000,		// dc_woebd1;
	0x00000814,		// dc_base_addr_manu_0;
	0x00000818,		// dc_base_addr_manu_1;
	0x00000800,		// dc_base_addr_auto;
	0x00000001,		// run_mode;
	0x00000000,		// mid
	0x00000000,		// did
					// code
	{
		0x8400000B,		// 0x800	// FAST_READ-3B
		0x48000001,		// 0x804
		0x44001000,		// 0x808
		0x28000000,		// 0x80C
		0xF4000000,		// 0x810	// STOP

		0xF4000000,		// 0x814	// STOP
		0xF4000000,		// 0x818	// STOP
	},

	0x00000077		// crc
};

// SPI Read4B
sSFQPI_InitHeader stCODE_FAST_READ4B = {
	0x00003070,		// code;	//0x7: 116MHz
	0x00020404,		// timing;
	0x000001FF,		// delay_so;
	0x0000000F,		// dc_clk;
	0x00000000,		// dc_wbd0;
	0x00000000,		// dc_wbd1;
	0x00000000,		// dc_rbd0;
	0x00000000,		// dc_rbd1;
	0x00000000,		// dc_woebd0;
	0x00000000,		// dc_woebd1;
	0x00000814,		// dc_base_addr_manu_0;
	0x0000081C,		// dc_base_addr_manu_1;
	0x00000800,		// dc_base_addr_auto;
	0x00000001,		// run_mode;
	0x00000000,		// mid
	0x00000000,		// did
					// code
	{
		0x8400000C,		// 0x800	// FAST_READ_4B
		0x48000000,		// 0x804
		0x44001000,		// 0x808
		0x28000000,		// 0x80C
		0xF4000000,		// 0x810	// STOP

		0xA40000B7,		// 0x814	// EN4B
		0xF4000000,		// 0x818	// STOP
		0xF4000000, 	// 0x81C	// STOP
	},

	0x00000077		// crc
};

// QPI Read 3B (STR)
sSFQPI_InitHeader stCODE_4READ3B = {
	0x00003070,		// code;
	0x00140300,		// timing;
	0x000001FF,		// delay_so;
	0x00000011,		// dc_clk;
	0x00000000,		// dc_wbd0;
	0x00000000,		// dc_wbd1;
	0x00000000,		// dc_rbd0;
	0x00000000,		// dc_rbd1;
	0x00000000,		// dc_woebd0;
	0x00000000,		// dc_woebd1;
	0x00000818,		// dc_base_addr_manu_0;
	0x0000081C,		// dc_base_addr_manu_1;
	0x00000800,		// dc_base_addr_auto;
	0x00000001,		// run_mode;
	0x00000000,		// mid
	0x00000000,		// did
					// code
	{
		0x840000EB,	// 0x800	// 4READ 3B
		0x4A000001,	// 0x804
		0x86000000,	// 0x808
		0x46002000,	// 0x80C
		0x2A000000,	// 0x810
		0xF4000000,	// 0x814	// STOP

		0xF4000000, // 0x818	// STOP
		0xF4000000, // 0x81C	// STOP
	},

	0x00000077		// crc
};

// 4Read4B (STR - QPI)
sSFQPI_InitHeader stCODE_4READ4B = {
	0x00003070,		// code;	//0x7: 116MHz
	0x00020204,		// timing;
	0x000000FF,		// delay_so;
	0x0000000F,		// dc_clk;
	0x00000000,		// dc_wbd0;
	0x00000000,		// dc_wbd1;
	0x00000000,		// dc_rbd0;
	0x00000000,		// dc_rbd1;
	0x00000000,		// dc_woebd0;
	0x00000000,		// dc_woebd1;
	0x00000818,		// dc_base_addr_manu_0;
	0x0000081C,		// dc_base_addr_manu_1;
	0x00000800,		// dc_base_addr_auto;
	0x00000001,		// run_mode;
	0x00000000,		// mid
	0x00000000,		// did
					// code
	{
		0x840000EC,	// 0x800	// 4READ 4B
		0x4A000000,	// 0x804
		0x86000000,	// 0x808
		0x46003000,	// 0x80C
		0x2A000000,	// 0x810
		0xF4000000,	// 0x814	// STOP

		0xA40000B7, // 0x818	// EN4B
		0xF4000000, // 0x81C	// STOP
		0xF4000000, // 0x820	// STOP
	},

	0x00000077		// crc
};

// QPI Read (DTR)
sSFQPI_InitHeader stCODE_4DTRD4B = {
	0x000030A0,		// code;	//0xA: 84HMz
	0x00020114,		// timing;
	0x000000FF,		// delay_so;
	0x0000000F,		// dc_clk;
	//0x0F0F0F0F, 	// dc_wbd0;
	0x00000000,		// dc_wbd0;
	0x00000000,		// dc_wbd1;
	0x00000000,		// dc_rbd0;
	0x00000000,		// dc_rbd1;
	0x00000000,		// dc_woebd0;
	0x00000000,		// dc_woebd1;
	0x00000818,		// dc_base_addr_manu_0;
	0x00000820,		// dc_base_addr_manu_1;
	0x00000800,		// dc_base_addr_auto;
	0x00000001,		// run_mode;
	0x00000000,		// mid
	0x00000000,		// did
					// code
	{
		0x840000EE,	// 0x800	// 4DTRD 4B
		0x5A000000,	// 0x804
		0x96000000,	// 0x808
		0x56008000,	// 0x80C
		0x3A000000,	// 0x810
		0xF4000000,	// 0x814	// STOP
		0xA40000B7, // 0x818	// EN4B
		0xF4000000, // 0x81C	// STOP
		0xF4000000, // 0x820	// STOP
	},

	0x00000077		// crc
};

void clear_input_info(tcc_input_info_x *p_input_info)
{
    if (p_input_info) {
        CLOSE_HANDLE(p_input_info->dest_name, NULL, free);
    }
}

char *jmalloc_string(char *sz)
{
    char *ret_str = NULL;
    unsigned int len = 0;

    if (sz) {
        len = strlen(sz);
        ret_str = (char *)malloc((len * sizeof(char)) + sizeof(char));
        if (ret_str) {
            strcpy(ret_str, sz);
        }
    }
    return ret_str;
}

unsigned int FWUG_CalcCrc8(unsigned char *base, unsigned int length)
{
	unsigned int crcout = 0;
	unsigned int cnt;
	unsigned char	code, tmp;

	for(cnt=0; cnt<length; cnt++)
	{
		code = base[cnt];
		tmp = code^crcout;
		crcout = (crcout>>8)^CRC32_TABLE[tmp&0xFF];
	}
	return crcout;
}

unsigned int PUSH_CRC(unsigned int crc, unsigned char *src, unsigned size)
{
    for ( ; size; size--) {
        crc = CRC32_TABLE[(crc ^ (*(src++))) & 0xFF] ^ (crc >> 8);
    }
    return crc;
}


unsigned int snor_round_addr(unsigned int size)
{
	int res;
	res = ((size + SNOR_PAGE_SIZE - 1) & 0xFFFFFF00);
	return 	res;
}

void SNOR_MIO_BOOT_MakeHeader(sTCC_FirmwareHeader *sImageSource, sSFQPI_BootHeader *sImageHeader,  unsigned int *puiSNorAddress)
{
	unsigned int	i;
	unsigned int	uSNorAddress;

	memset(sImageHeader, 0x00, sizeof(sSFQPI_BootHeader));
	sImageHeader->Signature	= 0x474e4147;

	uSNorAddress = *puiSNorAddress;

	for( i = 0; i < ROM_CODE_TYPE_MAX; ++i) {
		sImageHeader->rom_code[i].ulRomCodeOffset 	= uSNorAddress;
		sImageHeader->rom_code[i].ulRomCodeSize		= sImageSource->rom_code[i].ulRomCodeSize;
		sImageHeader->rom_code[i].ulRomTargetAddrss	= sImageSource->rom_code[i].ulRomTargetAddrss;
		sImageHeader->rom_code[i].ulRomCodeCRC		= sImageSource->rom_code[i].ulRomCodeCRC;
		uSNorAddress = snor_round_addr(sImageHeader->rom_code[i].ulRomCodeOffset + sImageHeader->rom_code[i].ulRomCodeSize);

		if(sImageHeader->rom_code[i].ulRomCodeSize != 0)
		{
			//printf("%s Image Info\n", ROM_CODE_NAME[i]);
			//printf("\tCode Size:   0x%08X (%d byte)\n", sImageHeader->rom_code[i].ulRomCodeSize, sImageHeader->rom_code[i].ulRomCodeSize);
			//printf("\tCode Addr:   0x%08X\n", sImageHeader->rom_code[i].ulRomCodeOffset);
			//printf("\tTarget Addr: 0x%08X\n", sImageHeader->rom_code[i].ulRomTargetAddrss);
			//printf("\tCode CRC:    0x%08X\n", sImageHeader->rom_code[i].ulRomCodeCRC);
		}
	}

	sImageHeader->ulCRC = FWUG_CalcCrc8((unsigned char*)sImageHeader, sizeof(sSFQPI_BootHeader) - 4);

	//printf("\tHeader CRC:  0x%08X\n", sImageHeader->ulCRC);

	*puiSNorAddress = uSNorAddress;
}

unsigned int SNOR_MIO_Get_SFMC_Div(unsigned int mem_pll, unsigned int Clk)
{
	unsigned int pll;
	unsigned int rDiv;
	unsigned int out_clk;
	pll = mem_pll;//tcc_get_pll(PLL_3);
	rDiv = pll / Clk;

	out_clk = ((mem_pll * 1000) / (rDiv +1));

	printf("\tTarget Clk = %dMHz, rDiv = 0x%X, Serial Flash Clk = %d MHz\n", Clk, rDiv, (out_clk/1000));
	return rDiv;
}

int SNOR_MIO_BOOT_Write_Header(sSFQPI_BootHeader *sImageHeader, unsigned char *pucRomFile_Buffer, unsigned int mem_pll, unsigned int snor_size)
{
	sSFQPI_InitHeader sfInitHeader;
	unsigned int 	uiHeaderSize;
	unsigned char	*pucRomFile = NULL;
	unsigned int	uiRomFileIndex;
	unsigned int 	nDiv;

	//=====================================================
	// Write Boot Header
	//=====================================================
	uiHeaderSize = (sizeof(sSFQPI_InitHeader) * 5);
	printf("Header Size: %d byte\n", uiHeaderSize);
	printf("<<SNOR_MAP: 0x%08x++0x%08x>>\n", SF_QPI_INIT_HEADER_OFFSET, uiHeaderSize);

	pucRomFile = malloc(uiHeaderSize);
	if (!pucRomFile) {
		printf("[%s: %d] Low memory(cannot allocate memory for verify)\n", __func__, __LINE__);
		return -1;
	}
	memset(pucRomFile, 0x00, uiHeaderSize);

	//=====================================================
	// Make Header Info
	//=====================================================
	memset(&sfInitHeader, 0x00, sizeof(sSFQPI_InitHeader));

	if(snor_size < 32)
		memcpy(&sfInitHeader, &stCODE_FAST_READ3B_for_CHIPBOOT, sizeof(sSFQPI_InitHeader));
	else
		memcpy(&sfInitHeader, &stCODE_FAST_READ4B_for_CHIPBOOT, sizeof(sSFQPI_InitHeader));

	printf("(0) FAST READ CMD Set for Chipboot (SPI)\n");
	{
		nDiv = SNOR_MIO_Get_SFMC_Div(mem_pll, 115);
		sfInitHeader.code &= ~(0x00000FF0);
		sfInitHeader.code |= ((nDiv) << SF_QPI_INIT_CODE_FCLK_DIV_SHIFT);

		printf("\tCode:        0x%08X\n", sfInitHeader.code);
		printf("\tTiming:      0x%08X\n", sfInitHeader.timing);
		printf("\tDelay_s:     0x%08X\n", sfInitHeader.delay_so);
		printf("\tDc_clk:      0x%08X\n", sfInitHeader.dc_clk);
		//printf("dc_wbd0: 0x%08X\n", sfInitHeader.dc_wbd0);
		//printf("dc_wbd1: 0x%08X\n", sfInitHeader.dc_wbd1);
		//printf("dc_rbd0: 0x%08X\n", sfInitHeader.dc_rbd0);
		//printf("dc_rbd1: 0x%08X\n", sfInitHeader.dc_rbd1);
		//printf("dc_woebd0: 0x%08X\n", sfInitHeader.dc_woebd0);
		//printf("dc_woebd1: 0x%08X\n", sfInitHeader.dc_woebd1);
		printf("\tRun_mode:    0x%08X\n", sfInitHeader.run_mode);
		printf("\tCode_vlu:    0x%08X 0x%08X 0x%08X 0x%08X 0x%08X\n",
									sfInitHeader.code_vlu[0],
									sfInitHeader.code_vlu[1],
									sfInitHeader.code_vlu[2],
									sfInitHeader.code_vlu[3],
									sfInitHeader.code_vlu[4]);
	}

	sfInitHeader.ulCRC = FWUG_CalcCrc8((unsigned char*)&sfInitHeader, sizeof(sSFQPI_InitHeader) - 4);
	printf("\tCMD CRC:     0x%08X\n", sfInitHeader.ulCRC);

	uiRomFileIndex = 0;

	memcpy(&pucRomFile[uiRomFileIndex], &sfInitHeader, sizeof(sSFQPI_InitHeader));
	printf("\tCMD address: 0x%08X\n", (SF_QPI_INIT_HEADER_OFFSET + uiRomFileIndex));
	uiRomFileIndex += 256;

	memcpy(&pucRomFile[uiRomFileIndex], &sImageHeader[0], sizeof(sSFQPI_BootHeader));
	uiRomFileIndex += 256;

	printf("(1) FAST READ CMD Set (SPI)\n");
	memcpy(&sfInitHeader, &stCODE_FAST_READ, sizeof(sSFQPI_InitHeader));
	{
		nDiv = SNOR_MIO_Get_SFMC_Div(mem_pll, 115);
		sfInitHeader.code &= ~(0x00000FF0);
		sfInitHeader.code |= ((nDiv) << SF_QPI_INIT_CODE_FCLK_DIV_SHIFT);

		printf("\tCode:        0x%08X\n", sfInitHeader.code);
		printf("\tTiming:      0x%08X\n", sfInitHeader.timing);
		printf("\tDelay_s:     0x%08X\n", sfInitHeader.delay_so);
		printf("\tDc_clk:      0x%08X\n", sfInitHeader.dc_clk);
		//printf("dc_wbd0: 0x%08X\n", sfInitHeader.dc_wbd0);
		//printf("dc_wbd1: 0x%08X\n", sfInitHeader.dc_wbd1);
		//printf("dc_rbd0: 0x%08X\n", sfInitHeader.dc_rbd0);
		//printf("dc_rbd1: 0x%08X\n", sfInitHeader.dc_rbd1);
		//printf("dc_woebd0: 0x%08X\n", sfInitHeader.dc_woebd0);
		//printf("dc_woebd1: 0x%08X\n", sfInitHeader.dc_woebd1);
		printf("\tRun_mode:    0x%08X\n", sfInitHeader.run_mode);
		printf("\tCode_vlu:    0x%08X 0x%08X 0x%08X 0x%08X 0x%08X\n",
									sfInitHeader.code_vlu[0],
									sfInitHeader.code_vlu[1],
									sfInitHeader.code_vlu[2],
									sfInitHeader.code_vlu[3],
									sfInitHeader.code_vlu[4]);
	}
	sfInitHeader.ulCRC = FWUG_CalcCrc8((unsigned char*)&sfInitHeader, sizeof(sSFQPI_InitHeader) - 4);
	printf("\tCMD CRC:     0x%08X\n", sfInitHeader.ulCRC);

	memcpy(&pucRomFile[uiRomFileIndex], &sfInitHeader, sizeof(sSFQPI_InitHeader));
	printf("\tCMD address: 0x%08X\n", (SF_QPI_INIT_HEADER_OFFSET + uiRomFileIndex));
	uiRomFileIndex += 256;

	printf("(2) 4READ 3B CMD Set (QPI,STR)\n");
	memcpy(&sfInitHeader, &stCODE_4READ3B, sizeof(sSFQPI_InitHeader));
	{
		nDiv = SNOR_MIO_Get_SFMC_Div(mem_pll, 85);
		sfInitHeader.code &= ~(0x00000FF0);
		sfInitHeader.code |= ((nDiv) << SF_QPI_INIT_CODE_FCLK_DIV_SHIFT);

		printf("\tCode:        0x%08X\n", sfInitHeader.code);
		printf("\tTiming:      0x%08X\n", sfInitHeader.timing);
		printf("\tDelay_s:     0x%08X\n", sfInitHeader.delay_so);
		printf("\tDc_clk:      0x%08X\n", sfInitHeader.dc_clk);
		//printf("dc_wbd0: 0x%08X\n", sfInitHeader.dc_wbd0);
		//printf("dc_wbd1: 0x%08X\n", sfInitHeader.dc_wbd1);
		//printf("dc_rbd0: 0x%08X\n", sfInitHeader.dc_rbd0);
		//printf("dc_rbd1: 0x%08X\n", sfInitHeader.dc_rbd1);
		//printf("dc_woebd0: 0x%08X\n", sfInitHeader.dc_woebd0);
		//printf("dc_woebd1: 0x%08X\n", sfInitHeader.dc_woebd1);
		printf("\tRun_mode:    0x%08X\n", sfInitHeader.run_mode);
		printf("\tCode_vlu:    0x%08X 0x%08X 0x%08X 0x%08X 0x%08X\n",
									sfInitHeader.code_vlu[0],
									sfInitHeader.code_vlu[1],
									sfInitHeader.code_vlu[2],
									sfInitHeader.code_vlu[3],
									sfInitHeader.code_vlu[4]);
	}
	sfInitHeader.ulCRC = FWUG_CalcCrc8((unsigned char*)&sfInitHeader, sizeof(sSFQPI_InitHeader) - 4);
	printf("\tCMD CRC:     0x%08X\n", sfInitHeader.ulCRC);

	memcpy(&pucRomFile[uiRomFileIndex], &sfInitHeader, sizeof(sSFQPI_InitHeader));
	printf("\tCMD address: 0x%08X\n", (SF_QPI_INIT_HEADER_OFFSET + uiRomFileIndex));
	uiRomFileIndex += 256;



#if 1		/* 017.02.20 */
	printf("(3) FAST_READ 4B CMD Set (SPI,STR)\n");
	memcpy(&sfInitHeader, &stCODE_FAST_READ4B, sizeof(sSFQPI_InitHeader));
	{
		nDiv = SNOR_MIO_Get_SFMC_Div(mem_pll, 115);
		sfInitHeader.code &= ~(0x00000FF0);
		sfInitHeader.code |= ((nDiv) << SF_QPI_INIT_CODE_FCLK_DIV_SHIFT);

		printf("\tCode: 	   0x%08X\n", sfInitHeader.code);
		printf("\tTiming:	   0x%08X\n", sfInitHeader.timing);
		printf("\tDelay_s:	   0x%08X\n", sfInitHeader.delay_so);
		printf("\tDc_clk:	   0x%08X\n", sfInitHeader.dc_clk);
		//printf("dc_wbd0: 0x%08X\n", sfInitHeader.dc_wbd0);
		//printf("dc_wbd1: 0x%08X\n", sfInitHeader.dc_wbd1);
		//printf("dc_rbd0: 0x%08X\n", sfInitHeader.dc_rbd0);
		//printf("dc_rbd1: 0x%08X\n", sfInitHeader.dc_rbd1);
		//printf("dc_woebd0: 0x%08X\n", sfInitHeader.dc_woebd0);
		//printf("dc_woebd1: 0x%08X\n", sfInitHeader.dc_woebd1);
		printf("\tRun_mode:    0x%08X\n", sfInitHeader.run_mode);
		printf("\tCode_vlu:    0x%08X 0x%08X 0x%08X 0x%08X 0x%08X\n",
									sfInitHeader.code_vlu[0],
									sfInitHeader.code_vlu[1],
									sfInitHeader.code_vlu[2],
									sfInitHeader.code_vlu[3],
									sfInitHeader.code_vlu[4]);
	}
#else
	printf("(3) 4DTRD 4B CMD Set (QPI,DTR)\n");
	memcpy(&sfInitHeader, &stCODE_4DTRD4B, sizeof(sSFQPI_InitHeader));
	{
		nDiv = SNOR_MIO_Get_SFMC_Div(mem_pll, 85);
		sfInitHeader.code &= ~(0x00000FF0);
		sfInitHeader.code |= ((nDiv) << SF_QPI_INIT_CODE_FCLK_DIV_SHIFT);

		printf("\tCode:        0x%08X\n", sfInitHeader.code);
		printf("\tTiming:      0x%08X\n", sfInitHeader.timing);
		printf("\tDelay_s:     0x%08X\n", sfInitHeader.delay_so);
		printf("\tDc_clk:      0x%08X\n", sfInitHeader.dc_clk);
		//printf("dc_wbd0: 0x%08X\n", sfInitHeader.dc_wbd0);
		//printf("dc_wbd1: 0x%08X\n", sfInitHeader.dc_wbd1);
		//printf("dc_rbd0: 0x%08X\n", sfInitHeader.dc_rbd0);
		//printf("dc_rbd1: 0x%08X\n", sfInitHeader.dc_rbd1);
		//printf("dc_woebd0: 0x%08X\n", sfInitHeader.dc_woebd0);
		//printf("dc_woebd1: 0x%08X\n", sfInitHeader.dc_woebd1);
		printf("\tRun_mode:    0x%08X\n", sfInitHeader.run_mode);
		printf("\tCode_vlu:    0x%08X 0x%08X 0x%08X 0x%08X 0x%08X\n",
									sfInitHeader.code_vlu[0],
									sfInitHeader.code_vlu[1],
									sfInitHeader.code_vlu[2],
									sfInitHeader.code_vlu[3],
									sfInitHeader.code_vlu[4]);
	}
#endif /* 0 */
	sfInitHeader.ulCRC = FWUG_CalcCrc8((unsigned char*)&sfInitHeader, sizeof(sSFQPI_InitHeader) - 4);
	printf("\tCMD CRC:     0x%08X\n", sfInitHeader.ulCRC);

	memcpy(&pucRomFile[uiRomFileIndex], &sfInitHeader, sizeof(sSFQPI_InitHeader));
	printf("\tCMD address: 0x%08X\n", (SF_QPI_INIT_HEADER_OFFSET + uiRomFileIndex));
	uiRomFileIndex += 256;

	//SNOR_MIO_Write(SF_QPI_INIT_HEADER_OFFSET, pucRomFile, uiHeaderSize);
	memcpy(&pucRomFile_Buffer[SF_QPI_INIT_HEADER_OFFSET], pucRomFile, uiHeaderSize);

	free(pucRomFile);

	return 0;
}

int SNOR_MIO_BOOT_Write_Image(sTCC_FirmwareHeader *sImageSource, sSFQPI_BootHeader *sImageHeader, unsigned char *pucRomFile_Buffer)
{
	unsigned int	i;
	unsigned int	uSNorAddress;
	unsigned int	uiRomfileSize;
	void	*pucRomFile = NULL;

	for( i = 0; i < ROM_CODE_TYPE_MAX; ++i) {

		uSNorAddress	= sImageHeader->rom_code[i].ulRomCodeOffset;
		uiRomfileSize	= sImageHeader->rom_code[i].ulRomCodeSize;
		pucRomFile		= sImageSource->rom_code[i].ulRomCodebuffer;
		if( uiRomfileSize != 0) {
			printf("<<SNOR_MAP: 0x%08x++0x%08x>>\n", uSNorAddress, uiRomfileSize);
			printf("\t%s rom\n", ROM_CODE_NAME[i]);
			printf("\tCode Size:   0x%08X (%d byte)\n", sImageHeader->rom_code[i].ulRomCodeSize, sImageHeader->rom_code[i].ulRomCodeSize);
			printf("\tCode Addr:   0x%08X\n", sImageHeader->rom_code[i].ulRomCodeOffset);
			printf("\tTarget Addr: 0x%08X\n", sImageHeader->rom_code[i].ulRomTargetAddrss);
			printf("\tCode CRC:    0x%08X\n", sImageHeader->rom_code[i].ulRomCodeCRC);
			printf("\tHeader CRC:  0x%08X\n", sImageHeader->ulCRC);

			memcpy(&pucRomFile_Buffer[uSNorAddress], pucRomFile, uiRomfileSize);
		}
	}

	return 0;
}

int SNOR_MIO_BOOT_MakeImage(unsigned char *pucRomFile_Buffer, unsigned char *pucPreloader_Buffer, unsigned int Preloader_size, unsigned int mem_pll, unsigned int snor_size)
{
	unsigned int	uSNorAddress;
	int				res;
	sSFQPI_BootHeader	sImageHeader;
	sTCC_FirmwareHeader sImageSource;

	//=====================================================
	// TBD :  Image Source Setup
	//=====================================================
	sImageSource.rom_code[ROM_CODE_CM4].ulRomCodebuffer			= 0;
	sImageSource.rom_code[ROM_CODE_CM4].ulRomCodeSize			= 0;
	sImageSource.rom_code[ROM_CODE_CM4].ulRomTargetAddrss		= 0;
	sImageSource.rom_code[ROM_CODE_CM4].ulRomCodeCRC			= 0;

	sImageSource.rom_code[ROM_CODE_WARM_BOOT].ulRomCodebuffer	= 0;
	sImageSource.rom_code[ROM_CODE_WARM_BOOT].ulRomCodeSize		= 0;
	sImageSource.rom_code[ROM_CODE_WARM_BOOT].ulRomTargetAddrss	= 0;
	sImageSource.rom_code[ROM_CODE_WARM_BOOT].ulRomCodeCRC		= 0;

#if 0		/* 017.01.05 */
	sImageSource.rom_code[ROM_CODE_DRAM_INIT].ulRomCodebuffer	= (void *)&MMC_BL1_ROM;
	sImageSource.rom_code[ROM_CODE_DRAM_INIT].ulRomCodeSize		= sizeof(MMC_BL1_ROM);
#else
	sImageSource.rom_code[ROM_CODE_DRAM_INIT].ulRomCodebuffer	= (void *)pucPreloader_Buffer;
	sImageSource.rom_code[ROM_CODE_DRAM_INIT].ulRomCodeSize 	= Preloader_size;
#endif /* 0 */

	sImageSource.rom_code[ROM_CODE_DRAM_INIT].ulRomTargetAddrss	= 0xF0000000;
	sImageSource.rom_code[ROM_CODE_DRAM_INIT].ulRomCodeCRC		= FWUG_CalcCrc8((unsigned char*)(sImageSource.rom_code[ROM_CODE_DRAM_INIT].ulRomCodebuffer),
																		sImageSource.rom_code[ROM_CODE_DRAM_INIT].ulRomCodeSize);

	//printf("\x1b[1;33m[%s:%d]\x1b[0m 0x%08X 0x%08X 0x%08X 0x%08X\n", __func__, __LINE__, MMC_bl_rom[0], MMC_bl_rom[1], MMC_bl_rom[2], MMC_bl_rom[3]);

	sImageSource.rom_code[ROM_CODE_BOOT_ROM].ulRomCodebuffer	= 0;
	sImageSource.rom_code[ROM_CODE_BOOT_ROM].ulRomCodeSize		= 0;
	sImageSource.rom_code[ROM_CODE_BOOT_ROM].ulRomTargetAddrss	= 0;
	sImageSource.rom_code[ROM_CODE_BOOT_ROM].ulRomCodeCRC		= 0;

	//=====================================================
	// Make: Image Header Info
	//=====================================================
	uSNorAddress = SF_QPI_ROM_AREA_OFFSET;
	SNOR_MIO_BOOT_MakeHeader(&sImageSource, &sImageHeader, &uSNorAddress);

	//=====================================================
	// Make: SFMC Read Cmd Header
	//=====================================================
	printf("\n");
	printf("[Make SFMC Read CMD Header...]\n");
	res = SNOR_MIO_BOOT_Write_Header(&sImageHeader, pucRomFile_Buffer, mem_pll, snor_size);
	if(res != 0)
		goto fwdn_snor_err;

	//=====================================================
	// Image Write
	//=====================================================
	printf("\n");
	printf("[Make Image Header...]\n");
	SNOR_MIO_BOOT_Write_Image(&sImageSource, &sImageHeader, pucRomFile_Buffer);

	printf("\n");
	return 0;

fwdn_snor_err:

	return res;

}

int SNOR_MIO_RECOVERY_MakeImage(unsigned char *pucRomFile_Buffer,
        unsigned char *pMicomRom_Buffer, unsigned int MicomRom_size){

    micom_recovery_header_x recovery_header;
    /* Gen Signature */
    memset(&recovery_header, 0x0, sizeof(micom_recovery_header_x));
    recovery_header.signature &=0x00 << 24; // Fixed Single Image
    recovery_header.signature |=0x5a << 16; // Fixed Signature
    recovery_header.signature |=0x01 <<  0; // Fixed Single Image
//    recovery_header.address = 0xC0010000; //Fixed Single Image
    recovery_header.address = 0x00010000; //Fixed Single Image
    recovery_header.crc = FWUG_CalcCrc8(pMicomRom_Buffer, MicomRom_size);
    recovery_header.rev = 0xFFFFFFFF;

    memcpy(pucRomFile_Buffer,&recovery_header, sizeof(micom_recovery_header_x));

    return 0;
}

#define MEM_PLL_500 0x06023283  // 500Mhz Momory
#define MEM_PLL_550 0x060244C3  // 550Mhz Momory
#define MEM_PLL_600 0x06026404  // 600Mhz Memory
#define MEM_PLL_720 0x06013C04  // 720Mhz Memory
#define MEM_PLL_760 0x06012F83  // 760Mhz Memory
#define MEM_PLL_800 0x06013203  // 800Mhz Memory
#define MEM_PLL_900 0x06014B04  // 900Mhz Memory
#define MEM_PLL_933 0x06014DC4  // 933Mhz Memory
#define MEM_PLL_1000 0x06013E83  // 1000Mhz Memory
#define MEM_PLL_1060 0x06014243  // 1060Mhz Memory
#define MEM_PLL_1066 0x06018546  // 1066Mhz Memory
#define MEM_PLL_1100 0x06011701  // 1100Mhz Memory

unsigned int get_pll_value(unsigned int mem_pll)
{
	unsigned int	ret_pll_value = 0xFFFFFFFF;

	switch(mem_pll) {
		case 500:
			ret_pll_value = MEM_PLL_500;
			break;

		case 550:
			ret_pll_value = MEM_PLL_550;
			break;

		case 600:
			ret_pll_value = MEM_PLL_600;
			break;

		case 720:
			ret_pll_value = MEM_PLL_720;
			break;

		case 760:
			ret_pll_value = MEM_PLL_760;
			break;

		case 800:
			ret_pll_value = MEM_PLL_800;
			break;

		case 900:
			ret_pll_value = MEM_PLL_900;
			break;

		case 933:
			ret_pll_value = MEM_PLL_933;
			break;

		case 1000:
			ret_pll_value = MEM_PLL_1000;
			break;

		case 1060:
			ret_pll_value = MEM_PLL_1060;
			break;

		case 1066:
			ret_pll_value = MEM_PLL_1066;
			break;

		case 1100:
			ret_pll_value = MEM_PLL_1100;
			break;

		default:
			ret_pll_value = 0xFFFFFFFF;
			break;
	}

	return ret_pll_value;
}

BOOL write_snor_boot_header(FILE *dest_fd, FILE *preloader_fd, unsigned int mem_pll, unsigned int snor_size)
{
	unsigned int len = 0;
	unsigned char *headers_buf;
	unsigned char *preloader_buf;
	unsigned int *puiPreloader_buf;
	unsigned int	MEM_PLL_VALUE;

    if (dest_fd && preloader_fd) {

		headers_buf = malloc(BOOT_HEADER_SIZE);
		if (!headers_buf) {
			printf("[%s: %d] Low memory(cannot allocate memory for verify)\n", __func__, __LINE__);
			return FALSE;
		}

        memset(headers_buf, 0xFF, BOOT_HEADER_SIZE);

		fseek(preloader_fd, 0, SEEK_END);
		len = ftell(preloader_fd);
		preloader_buf = malloc(len);
		if (!preloader_buf) {
			free(headers_buf);
			printf("[%s: %d] Low memory(cannot allocate memory for verify)\n", __func__, __LINE__);
			return FALSE;
		}
		puiPreloader_buf = (unsigned int *)preloader_buf;

		fseek(preloader_fd, 0, SEEK_SET);
		fread(preloader_buf, 1, len, preloader_fd);
		puiPreloader_buf[1] = 0x1400004C;

		MEM_PLL_VALUE = get_pll_value(mem_pll);
		if(MEM_PLL_VALUE == 0xFFFFFFFF)
		{
			free(headers_buf);
			free(preloader_buf);
			printf("%s - invalid mem pll value (%d) MHz Not supported.\n", __func__, mem_pll);
			return FALSE;
		}

		puiPreloader_buf[2] = MEM_PLL_VALUE;
		SNOR_MIO_BOOT_MakeImage((unsigned char *)headers_buf, (unsigned char *)preloader_buf, len, mem_pll, snor_size);

		fseek(dest_fd, 0, SEEK_SET);
		if (fwrite(headers_buf, 1, BOOT_HEADER_SIZE, dest_fd) != BOOT_HEADER_SIZE) {
			free(headers_buf);
			free(preloader_buf);
			printf("%s - file write fail\n", __func__);
			return FALSE;
		} else {
			free(headers_buf);
			free(preloader_buf);
			printf("%s - success\n", __func__);
			return TRUE;
		}
    }
    return FALSE;
}

BOOL write_recovery_header(FILE *dest_fd, FILE *micom_rom_fd)
{
    unsigned int len = 0;
    unsigned char *headers_buf;
    unsigned char *micom_rom_buf;

    if(dest_fd && micom_rom_fd){
        headers_buf = malloc(RECOVERY_HEADER_SIZE);

        if(!headers_buf){
			printf("[%s: %d] Low memory(cannot allocate memory for verify)\n", __func__, __LINE__);
			return FALSE;
        }

        memset(headers_buf, 0xFF, RECOVERY_HEADER_SIZE);

        fseek(micom_rom_fd, 0, SEEK_END);
        len = ftell(micom_rom_fd);
        micom_rom_buf = malloc(len);

        if(!micom_rom_buf){
			free(headers_buf);
			printf("[%s: %d] Low memory(cannot allocate memory for verify)\n", __func__, __LINE__);
			return FALSE;
        }

        fseek(micom_rom_fd, 0, SEEK_SET);
        fread(micom_rom_buf, 1, len , micom_rom_fd);
        SNOR_MIO_RECOVERY_MakeImage((unsigned char*)headers_buf, (unsigned char*)micom_rom_buf, len);

        fseek(dest_fd, 0, SEEK_END);
        if(fwrite(headers_buf, 1, RECOVERY_HEADER_SIZE, dest_fd) != RECOVERY_HEADER_SIZE){
			free(headers_buf);
			free(micom_rom_buf);
			printf("%s - file write fail\n", __func__);
			return FALSE;
        } else{
			free(headers_buf);
			free(micom_rom_buf);
			printf("%s - success\n", __func__);
            return TRUE;
        }

    }

    return FALSE;

}
BOOL write_micom_rom(FILE *dest_fd, FILE *micom_rom_fd)
{
	unsigned int len = 0;
	unsigned char *micom_rom_buf;

    if (dest_fd && micom_rom_fd) {

		fseek(micom_rom_fd, 0, SEEK_END);
		len = ftell(micom_rom_fd);
		micom_rom_buf = malloc(len);
		if (!micom_rom_buf) {
			printf("[%s: %d] Low memory(cannot allocate memory for verify)\n", __func__, __LINE__);
			return FALSE;
		}

		fseek(micom_rom_fd, 0, SEEK_SET);
		fread(micom_rom_buf, 1, len, micom_rom_fd);

		fseek(dest_fd, MICOM_ROM_OFFSET, SEEK_SET);

		if (fwrite(micom_rom_buf, 1, len, dest_fd) != len) {
			free(micom_rom_buf);
			printf("%s - file write fail\n", __func__);
			return FALSE;
		} else {
			free(micom_rom_buf);
			printf("%s - success\n", __func__);
			return TRUE;
		}
    }
    return FALSE;
}

BOOL write_rom_crc(FILE *out_rom_fd)
{
    if (out_rom_fd) {
        //unsigned char buf[1024];
        unsigned char *rom_buf;
        unsigned int crc = 0;
		unsigned int len;
        size_t read_size = 0;

		fseek(out_rom_fd, 0, SEEK_END);
		len = ftell(out_rom_fd);
		rom_buf = malloc(len);
		if (!rom_buf) {
			printf("[%s: %d] Low memory(cannot allocate memory for verify)\n", __func__, __LINE__);
			return FALSE;
		}

		fseek(out_rom_fd, 0, SEEK_SET);
        read_size = fread(rom_buf, 1, len, out_rom_fd);
		if( read_size != len)
		{
			printf("[%s: %d] File read error\n", __func__, __LINE__);
			free(rom_buf);
			return FALSE;
		}

		memcpy(&rom_buf[HEADER_POS_IMAGE_SIZE], &read_size, 4);
		memset(&rom_buf[HEADER_POS_IMAGE_CRC], 0x00, 4);
        crc = PUSH_CRC(crc, rom_buf, read_size);
        printf("Total Image Size: %d byte\n", (unsigned int)(read_size));
        printf("Total Image CRC: 0x%08X\n", crc);
		memcpy(&rom_buf[HEADER_POS_IMAGE_CRC], &crc, 4);

		fseek(out_rom_fd, 0, SEEK_SET);

		if (fwrite(rom_buf, 1, read_size, out_rom_fd) != read_size) {
			free(rom_buf);
			printf("%s - file write fail\n", __func__);
			return FALSE;
		} else {
			free(rom_buf);
			printf("%s - success\n", __func__);
			return TRUE;
		}
    }
    return FALSE;
}

