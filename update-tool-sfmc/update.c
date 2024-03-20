
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/stat.h>
#include <getopt.h>
#include <sys/reboot.h>

#include "update.h"

int firmware_update(tcc_update_param_t *param);
int update_micom_firmware_area(tcc_update_param_t *param);

unsigned int check_crc(int fd);

const unsigned	CRC32_TABLE[256] = {
	0x00000000,	0x90910101,	0x91210201,	0x01B00300,
	0x92410401,	0x02D00500,	0x03600600,	0x93F10701,
	0x94810801,	0x04100900,	0x05A00A00,	0x95310B01,
	0x06C00C00,	0x96510D01,	0x97E10E01,	0x07700F00,
	0x99011001,	0x09901100,	0x08201200,	0x98B11301,
	0x0B401400,	0x9BD11501,	0x9A611601,	0x0AF01700,
	0x0D801800,	0x9D111901,	0x9CA11A01,	0x0C301B00,
	0x9FC11C01,	0x0F501D00,	0x0EE01E00,	0x9E711F01,
	0x82012001,	0x12902100,	0x13202200,	0x83B12301,
	0x10402400,	0x80D12501,	0x81612601,	0x11F02700,
	0x16802800,	0x86112901,	0x87A12A01,	0x17302B00,
	0x84C12C01,	0x14502D00,	0x15E02E00,	0x85712F01,
	0x1B003000,	0x8B913101,	0x8A213201,	0x1AB03300,
	0x89413401,	0x19D03500,	0x18603600,	0x88F13701,
	0x8F813801,	0x1F103900,	0x1EA03A00,	0x8E313B01,
	0x1DC03C00,	0x8D513D01,	0x8CE13E01,	0x1C703F00,
	0xB4014001,	0x24904100,	0x25204200,	0xB5B14301,
	0x26404400,	0xB6D14501,	0xB7614601,	0x27F04700,
	0x20804800,	0xB0114901,	0xB1A14A01,	0x21304B00,
	0xB2C14C01,	0x22504D00,	0x23E04E00,	0xB3714F01,
	0x2D005000,	0xBD915101,	0xBC215201,	0x2CB05300,
	0xBF415401,	0x2FD05500,	0x2E605600,	0xBEF15701,
	0xB9815801,	0x29105900,	0x28A05A00,	0xB8315B01,
	0x2BC05C00,	0xBB515D01,	0xBAE15E01,	0x2A705F00,
	0x36006000,	0xA6916101,	0xA7216201,	0x37B06300,
	0xA4416401,	0x34D06500,	0x35606600,	0xA5F16701,
	0xA2816801,	0x32106900,	0x33A06A00,	0xA3316B01,
	0x30C06C00,	0xA0516D01,	0xA1E16E01,	0x31706F00,
	0xAF017001,	0x3F907100,	0x3E207200,	0xAEB17301,
	0x3D407400,	0xADD17501,	0xAC617601,	0x3CF07700,
	0x3B807800,	0xAB117901,	0xAAA17A01,	0x3A307B00,
	0xA9C17C01,	0x39507D00,	0x38E07E00,	0xA8717F01,
	0xD8018001,	0x48908100,	0x49208200,	0xD9B18301,
	0x4A408400,	0xDAD18501,	0xDB618601,	0x4BF08700,
	0x4C808800,	0xDC118901,	0xDDA18A01,	0x4D308B00,
	0xDEC18C01,	0x4E508D00,	0x4FE08E00,	0xDF718F01,
	0x41009000,	0xD1919101,	0xD0219201,	0x40B09300,
	0xD3419401,	0x43D09500,	0x42609600,	0xD2F19701,
	0xD5819801,	0x45109900,	0x44A09A00,	0xD4319B01,
	0x47C09C00,	0xD7519D01,	0xD6E19E01,	0x46709F00,
	0x5A00A000,	0xCA91A101,	0xCB21A201,	0x5BB0A300,
	0xC841A401,	0x58D0A500,	0x5960A600,	0xC9F1A701,
	0xCE81A801,	0x5E10A900,	0x5FA0AA00,	0xCF31AB01,
	0x5CC0AC00,	0xCC51AD01,	0xCDE1AE01,	0x5D70AF00,
	0xC301B001,	0x5390B100,	0x5220B200,	0xC2B1B301,
	0x5140B400,	0xC1D1B501,	0xC061B601,	0x50F0B700,
	0x5780B800,	0xC711B901,	0xC6A1BA01,	0x5630BB00,
	0xC5C1BC01,	0x5550BD00,	0x54E0BE00,	0xC471BF01,
	0x6C00C000,	0xFC91C101,	0xFD21C201,	0x6DB0C300,
	0xFE41C401,	0x6ED0C500,	0x6F60C600,	0xFFF1C701,
	0xF881C801,	0x6810C900,	0x69A0CA00,	0xF931CB01,
	0x6AC0CC00,	0xFA51CD01,	0xFBE1CE01,	0x6B70CF00,
	0xF501D001,	0x6590D100,	0x6420D200,	0xF4B1D301,
	0x6740D400,	0xF7D1D501,	0xF661D601,	0x66F0D700,
	0x6180D800,	0xF111D901,	0xF0A1DA01,	0x6030DB00,
	0xF3C1DC01,	0x6350DD00,	0x62E0DE00,	0xF271DF01,
	0xEE01E001,	0x7E90E100,	0x7F20E200,	0xEFB1E301,
	0x7C40E400,	0xECD1E501,	0xED61E601,	0x7DF0E700,
	0x7A80E800,	0xEA11E901,	0xEBA1EA01,	0x7B30EB00,
	0xE8C1EC01,	0x7850ED00,	0x79E0EE00,	0xE971EF01,
	0x7700F000,	0xE791F101,	0xE621F201,	0x76B0F300,
	0xE541F401,	0x75D0F500,	0x7460F600,	0xE4F1F701,
	0xE381F801,	0x7310F900,	0x72A0FA00,	0xE231FB01,
	0x71C0FC00,	0xE151FD01,	0xE0E1FE01,	0x7070FF00
};

unsigned int calc_crc32(unsigned char *base, unsigned int length, unsigned int crcin)
{
    unsigned int cnt;
    unsigned int code;

    for(cnt = 0; cnt<length; cnt++){

        code = (unsigned char)(base[cnt]^crcin);
        crcin = (crcin>>8)^CRC32_TABLE[code&0xFF];

    }

    return crcin;
}

#if 0

/*
 * Check rom's CRC
 */
unsigned int CalCRC_ROMFile(unsigned int *pBuffer,unsigned int size,unsigned int crcout, unsigned int mode)
{
	unsigned int cnt, i, code, tmp;
	unsigned int CrcRegion;

	CrcRegion = (size >> 2);
	for (cnt = 0; cnt < CrcRegion; cnt++) {
		code = pBuffer[cnt];

		if (mode==0 || mode==1) {
			if (cnt == 4 || cnt == 5) {
				continue;
			}
		}
		if (mode == 1) {
			if (cnt == 6) {
				code = 0x00000000;
			}
		}

		for (i = 0; i < 4; i++) {
			tmp = code^crcout;
			crcout = (crcout>>8)^CRC32_TABLE[tmp&0xFF];
			code = code >> 8;
		}
	}

	return crcout;
}

unsigned int check_crc(int fd)
{
	unsigned int    i;
	unsigned int    uiROMFileSize;
	unsigned int    uiVerifyCRC;
	unsigned int    uiTempCRC;
	unsigned int    uiCRCSize;
	unsigned int    uiCnt;
	unsigned int    uiMode;
	unsigned int    uiBufSize;
	unsigned char  *buf;

	uiROMFileSize =  0x00;
	uiVerifyCRC = 0x00;
	uiTempCRC = 0x00;
	uiBufSize = (4 * 1024 * 1024);

	buf = (unsigned char *)malloc(uiBufSize);
	memset(buf, 0x00, uiBufSize);
	read(fd, buf, 32);	//header size

	uiTempCRC |= ( buf[27] & 0x000000FF) << 24;
	uiTempCRC |= ( buf[26] & 0x000000FF) << 16;
	uiTempCRC |= ( buf[25] & 0x000000FF) << 8;
	uiTempCRC |= ( buf[24] & 0x000000FF) ;

	uiROMFileSize |= ( buf[31] & 0x000000FF) << 24;
	uiROMFileSize |= ( buf[30] & 0x000000FF) << 16;
	uiROMFileSize |= ( buf[29] & 0x000000FF) << 8;
	uiROMFileSize |= ( buf[28] & 0x000000FF) ;

	lseek(fd, 0L, SEEK_SET);
	if (uiBufSize < uiROMFileSize ) {
		uiCnt = (uiROMFileSize + ( uiBufSize - 1)) / uiBufSize;
		for (i = 0; i < uiCnt ; i++) {
			if (i == (uiCnt -1))
				uiCRCSize = (uiROMFileSize  - uiBufSize * (uiCnt - 1));
			else
				uiCRCSize = uiBufSize;

			read(fd, buf, uiCRCSize);

			if ( i == 0 )
				uiMode = 1;
			else
				uiMode = 2;

			uiVerifyCRC = CalCRC_ROMFile((unsigned int *)buf, uiCRCSize, uiVerifyCRC, uiMode);
		}
	} else {
		read(fd, buf, uiROMFileSize);
		uiMode = 1;
		uiVerifyCRC = CalCRC_ROMFile((unsigned int *)buf, uiROMFileSize, uiVerifyCRC, uiMode);
	}

	lseek(fd, 0L, SEEK_SET);
	if ( uiTempCRC != uiVerifyCRC )
		return -1;	// CRC FAIL
	else
		return 0;
}
#endif

char *tcc_malloc_string(char *sz)
{
    char *ret = NULL;
    unsigned int len = 0;

    if (sz) {
        len = strlen(sz);
        ret = (char *)malloc((len * sizeof(char)) + sizeof(char));
        if (ret) {
            strcpy(ret, sz);
        }
    }
    return ret;
}

size_t strlcpy(char *dst, char const *src, size_t s)
{
	size_t i= 0;

	if(!s) {
		return strlen(src);
	}

	for(i= 0; ((i< s-1) && src[i]); i++) {
		dst[i]= src[i];
	}

	dst[i]= 0;

	return i + strlen(src+i);
}

static int intr_parse_args(int argc, char *argv[], tcc_update_param_t *param)
{
    int ret = -1;
	static struct option long_opt[] = {
		{"rom_file", 1, NULL, 0},
		{0, 0, 0, 0}
	};

    if (!param) {
        goto exit;
    }

    param->update_type = -1;
	param->dev_type = DEV_NAND;		/* default NAND */

    while (1) {
        int c = 0;
        int option_idx = 0;

        c = getopt_long(argc, argv, "f:d:", long_opt, &option_idx);
        if (c == -1) { break; }

        switch (c) {
        case 'f':
            param->rom_file = tcc_malloc_string(optarg);
            break;
		case 'd':
			param->dev_name = tcc_malloc_string(optarg);
			break;

        default:
            printf("invalid argument\n");
			goto exit;
            break;
        }
    }

	ret = 0;

exit:
    return ret;
}

static void help_msg(void)
{
	printf(	"\n"
			"  Usage:\n"
			"    update-tools -f <file>\n -d /dev/tcc_sfmc"
			"\n"
			"  Options:\n"
			"    -f <file>     firmware rom file\n"
			"                  <file> /mnt/SD0p1/boot.rom,  /mnt/SD0p1/boot.img, etc...\n"
			"\n"
			);
}

int main(int argc, char *argv[])
{
	tcc_update_param_t param;

	memset(&param, 0, sizeof(tcc_update_param_t));

	if (intr_parse_args(argc, argv, &param) == 0) {
		firmware_update(&param);
	} else {
		help_msg();
	}

    CLOSE_HANDLE(param.rom_file, NULL, free);
	return 0;
}


/*-------------------------------------------------------------------------
 * UPDATE FUNTIONS
 *-------------------------------------------------------------------------
 */
/*
 * update main function
 */
int firmware_update(tcc_update_param_t *param)
{
	int ret = -1;

	if (!param) {
		return ret;
	}

	/* display info. */
	printf("\nsource rom : %s\n", param->rom_file);
	ret = update_micom_firmware_area(param);
	return ret;
}

/*
 * update firmware function
 */
int update_micom_firmware_area(tcc_update_param_t *param)
{
    int ret = -1;
    int rom_fd = -1;
    int dev_fd = -1;
    unsigned char *rom_buff = NULL;
    unsigned char *vrif_buff = NULL;
    unsigned char *sfmcinfo_buff = NULL;
    unsigned int len;
    struct stat st;
	int sfmc_access_size =0;
	struct sfmc_ioctl_info sfmc_info;

    struct micom_bootsec_info *bootsec_info;

	printf("Update micom firmware!!!\n");

	//===================================================
	// open rom file
	//===================================================
	rom_fd = open(param->rom_file, O_RDONLY|O_NDELAY);
	if (rom_fd == -1) {
		printf("[%s] cannot open file.\n", param->rom_file);
		goto exit;
	}
	fstat(rom_fd, &st);
	len = st.st_size;

	rom_buff = (unsigned char *)malloc(len);
	memset(rom_buff, 0x0, len);
	read(rom_fd, rom_buff, len);

	vrif_buff = (unsigned char *)malloc(len);
	memset(vrif_buff, 0x0, len);

    sfmcinfo_buff = (unsigned char *)malloc(SFMC_MICOM_HEADER_SIZE);
    memset(sfmcinfo_buff , 0x00, SFMC_MICOM_HEADER_SIZE);
    bootsec_info = (struct micom_bootsec_info*)sfmcinfo_buff;

	//===================================================
	// sfmc dev open
	//===================================================
	/* open dev */
	dev_fd = open(param->dev_name, O_RDWR|O_NDELAY);
	if (dev_fd == -1) {
		printf("[sfmc] cannot open %s\n", param->dev_name);
		goto exit;
	}

	printf("Update %s (%d bytes) --> ", param->rom_file, len);

	//===================================================
	// Read and Backup Header
	//===================================================
    sfmc_info.buf = sfmcinfo_buff;
	sfmc_info.address = 0;						// 0 : micom firmware Header Info address. (Serial flash 0xC000F000)
												// must 4096 byte unit.
												// address ex: 0x0000
	sfmc_access_size = (SFMC_MICOM_HEADER_SIZE);				// 4096byte Erase
	sfmc_info.sector_count = (sfmc_access_size / SFMC_SECTOR_SIZE);			// 1 sector = 4k byte
	sfmc_info.data_size = sfmc_access_size;

    ret = ioctl(dev_fd, IOCTL_SFMC_READ_PAGE_MICOM_HEADER, &sfmc_info);

    if(ret) { 
        printf("FAIL TO READ HEADER SECTION\n");
        goto exit;
    }else printf("HEADER SECTION READ OK\n");


	//===================================================
	// Header Erase
	//===================================================
	sfmc_info.buf = NULL;
	sfmc_info.address = 0;						// 0 : micom firmware Header Info address. (Serial flash 0xC000F000)
												// must 4096 byte unit.
												// address ex: 0x0000
	sfmc_access_size = (SFMC_MICOM_HEADER_SIZE);				// 4096byte Erase
	sfmc_info.sector_count = (sfmc_access_size / SFMC_SECTOR_SIZE);			// 1 sector = 4k byte
	sfmc_info.data_size = sfmc_access_size;

	printf("\n** Header erase address: %d, erase size: %d, erase sector count: %d\n",
									sfmc_info.address,
									sfmc_access_size,
									sfmc_info.sector_count);

    ret = ioctl(dev_fd, IOCTL_SFMC_ERASE_4K_MICOM_HEADER, &sfmc_info);
    if (ret) {
        printf("FAIL\n");
        goto exit;
    } else {
        printf("SUCCESS\n");
    }
    
	//===================================================
	// MICOM Firmware Area Erase
	//===================================================
	sfmc_info.buf = NULL;
	sfmc_info.address = 0;						// 0 : micom firmware address. (Serial flash 0xC0010000)
												// must 4096 byte unit.
												// address ex: 0x0000, 0x1000, 0x2000 ...
	sfmc_access_size = (2 * 1024 * 1024);		// 2MByte Erase
	sfmc_info.sector_count = (sfmc_access_size / SFMC_SECTOR_SIZE);			// 1 sector = 4k byte
	sfmc_info.data_size = sfmc_access_size;

	printf("\n1) erase address: %d, erase size: %d, erase sector count: %d\n",
									sfmc_info.address,
									sfmc_access_size,
									sfmc_info.sector_count);

    ret = ioctl(dev_fd, IOCTL_SFMC_ERASE_4K_MICOM_ROM, &sfmc_info);
    if (ret) {
        printf("FAIL\n");
        goto exit;
    } else {
        printf("SUCCESS\n");
    }

	//===================================================
	//  MICOM Firmware Area Write
	//===================================================
	sfmc_info.buf = rom_buff;
	sfmc_info.address = 0;				// 0 : micom firmware address. (Serial flash 0xC0010000)
	sfmc_access_size = len;				// firmware file size
	sfmc_info.sector_count = (sfmc_access_size / SFMC_SECTOR_SIZE);			// 1 sector = 4k byte
	sfmc_info.data_size = sfmc_access_size;

	printf("\n2) Write address: %d, size: %d, sector count: %d\n",
									sfmc_info.address,
									sfmc_access_size,
									sfmc_info.sector_count);

    ret = ioctl(dev_fd, IOCTL_SFMC_WRITE_PAGE_MICOM_ROM, &sfmc_info);
    if (ret) {
        printf("nFAIL\n");
        goto exit;
    } else {
        printf("SUCCESS\n");
    }

	//===================================================
	//  MICOM Firmware Area Read
	//===================================================
	sfmc_info.buf = vrif_buff;
	sfmc_info.address = 0;				// 0 : micom firmware address. (Serial flash 0xC0010000)
	sfmc_access_size = len;				// firmware file size
	sfmc_info.sector_count = (sfmc_access_size / SFMC_SECTOR_SIZE);			// 1 sector = 4k byte
	sfmc_info.data_size = sfmc_access_size;

	printf("\n3) Read address: %d, size: %d, sector count: %d\n",
									sfmc_info.address,
									sfmc_access_size,
									sfmc_info.sector_count);

    ret = ioctl(dev_fd, IOCTL_SFMC_READ_PAGE_MICOM_ROM, &sfmc_info);
    if (ret) {
        printf("FAIL\n");
        goto exit;
    } else {
        printf("SUCCESS\n");
    }

	//===================================================
	// Data compare
	//===================================================
	if(memcmp(vrif_buff, rom_buff, len)) {
		printf("Data compare fail!!\n");
	} else {
		printf("Data compare Success!!!\n");
	}



	//===================================================
	//  Header Update
	//===================================================
    // TODO update multiple header section 
    
    bootsec_info->crc = calc_crc32(rom_buff, len, 0x0);
    printf("CRC32 : 0x%08X \n" , bootsec_info->crc);
    
	//===================================================
	//  Header Write
	//===================================================

	sfmc_info.buf = sfmcinfo_buff;
	sfmc_info.address = 0;				   // 0 : micom firmware Header Info address. (Serial flash 0xC000F000)
	sfmc_access_size = SFMC_MICOM_HEADER_SIZE;				// Header Info file size
	sfmc_info.sector_count = (sfmc_access_size / SFMC_SECTOR_SIZE);			// 1 sector = 4k byte
	sfmc_info.data_size = sfmc_access_size;

	printf("\n** Header Write address: %d, size: %d, sector count: %d\n",
									sfmc_info.address,
									sfmc_access_size,
									sfmc_info.sector_count);

    ret = ioctl(dev_fd, IOCTL_SFMC_WRITE_PAGE_MICOM_HEADER, &sfmc_info);
    if (ret) {
        printf("nFAIL\n");
        goto exit;
    } else {
        printf("SUCCESS\n");
    }

	//===================================================
	sync();

    free(rom_buff);
    free(vrif_buff);
    free(sfmcinfo_buff);
    close(rom_fd); close(dev_fd);
	//reboot(RB_AUTOBOOT);
	return 0;
exit:
    free(rom_buff);
    free(vrif_buff);
    free(sfmcinfo_buff);
    close(rom_fd); close(dev_fd);
	return ret;
}


/* end of file */
