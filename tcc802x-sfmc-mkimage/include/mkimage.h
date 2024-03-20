
#ifndef __MKIMAGE_H__
#define __MKIMAGE_H__

#define ALIGN_SIZE 4
#define BOOT_HEADER_OFFSET	(0)
#define BOOT_HEADER_SIZE (60 * 1024)

#define RECOVERY_HEADER_OFFSET (60 *1024)
#define RECOVERY_HEADER_SIZE (4 *1024)

#define MICOM_ROM_OFFSET (64 * 1024)

#include "jatom.h"

typedef struct _tcc_input_info_x {
    char *dest_name;
    char *micom_rom_name;
    char *preloader_rom_name;
	int 	mem_pll;
	int 	snor_size;
} tcc_input_info_x;

typedef struct _micom_recovery_header_x {
    unsigned int signature;
    unsigned int address;
    unsigned int crc;
    unsigned int rev;
}micom_recovery_header_x;

extern char *jmalloc_string(char *sz);
extern BOOL write_snor_boot_header(FILE *dest_fd, FILE *preloader_fd, unsigned int mem_pll, unsigned int snor_size);
extern BOOL write_recovery_header(FILE *dest_fd, FILE *micom_rom_fd);
extern BOOL write_micom_rom(FILE *dest_fd, FILE *micom_rom_fd);
extern void clear_input_info(tcc_input_info_x *p_input_info);
extern BOOL write_rom_crc(FILE *out_rom_fd);


#endif /* __MKIMAGE_H__ */

