
#ifndef __FIRMUPDATE_H__
#define __FIRMUPDATE_H__

#ifdef __cplusplus
extern "C" {
#endif

#define CLOSE_HANDLE(H, N, F) do { if ((H) != (N)) { (F)(H); (H) = (N); } } while (0)


#define DEV_NAME				"/dev/tcc_sfmc"
#define NDD_DEV_MAJOR       	252
#define TCC_SFMC_MAGIC 			'S'

#define IOCTL_SFMC_ERASE_4K_MICOM_ROM		_IO(TCC_SFMC_MAGIC ,1)
#define IOCTL_SFMC_WRITE_PAGE_MICOM_ROM 	_IO(TCC_SFMC_MAGIC ,2)
#define IOCTL_SFMC_READ_PAGE_MICOM_ROM		_IO(TCC_SFMC_MAGIC ,3)

#define IOCTL_SFMC_ERASE_4K_MICOM_HEADER	_IO(TCC_SFMC_MAGIC ,4)
#define IOCTL_SFMC_WRITE_PAGE_MICOM_HEADER 	_IO(TCC_SFMC_MAGIC ,5)
#define IOCTL_SFMC_READ_PAGE_MICOM_HEADER	_IO(TCC_SFMC_MAGIC ,6)

typedef struct _tcc_update_param {
	char *rom_file;
	char *rom_type;
	int kernel_size;
	char *dev_name;
	char *dev_misc_name;

#define UPDATE_BOOTLOADER	0
#define UPDATE_KERNEL		1
#define GOTO_RECOVERY		2
#define MODE_CLEAR 			3
	int update_type;

#define DEV_NAND			0
	int dev_type;
} tcc_update_param_t;


struct sfmc_ioctl_info {
	unsigned int	address;
	unsigned int	sector_count;
	unsigned int	data_size;
	unsigned char 	*buf;
};

struct micom_bootsec_info {
    unsigned int signature;
    unsigned int address;
    unsigned int crc;
    unsigned int rev;
};


#define SFMC_MICOM_HEADER_SIZE		4096

#define SFMC_SECTOR_SIZE			4096

#ifdef __cplusplus
}
#endif

#endif /*__FIRMUPDATE_H__*/

