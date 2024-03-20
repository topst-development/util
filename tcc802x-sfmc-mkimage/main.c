#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "jatom.h"
#include "mkimage.h"

static void help_msg(char *prog_name)
{
	printf(	"------------------------------------------------------------\n"
			"|  TCC_MK_SNOR_BOOT: Mark up SNOR Boot Image\n"
			"|  [USAGE]\n"
			"|  -c : mem pll value (MHz, ex 800, 1066)\n"
			"|  -o : output file name\n"
			"|  -m : micom rom file\n"
			"|  -p : pre-loader rom file\n"
			"|  -s : SNOR Size (unit: MByte)\n"
			"|\n"
			"| * example \n"
			"| $ %s -c 1066 -s 4 -m micom.bin -p pre_loader.bin -o snor_boot.rom\n"
			"|\n"
			"| * Date: 2017.02.20\n"
			"------------------------------------------------------------\n", prog_name);
}

static BOOL parse_args(int argc, char *argv[], tcc_input_info_x *p_input_info)
{
    BOOL ret = FALSE;
    static struct option long_options[] = {
        {"mem_pll", 1, 0, 'c'},
        {"dest_name", 1, 0, 'o'},
		{"micom_name", 1, 0, 'm'},
		{"snor_size", 1, 0, 's'},
		{"preloader_name", 1, 0, 'p'},
        {0, 0, 0, 0}
    };

    if (!p_input_info) {
        return FALSE;
    }

    while (1) {
        int c = 0;
        int option_index = 0;

        c = getopt_long(argc, argv, "c:o:m:p:s:", long_options, &option_index);
        if (c == -1) { break; }

        switch (c) {
        case 0:
            break;
		case 'c':
			p_input_info->mem_pll = (unsigned int)atol(optarg);
			break;
		case 's':
			p_input_info->snor_size = (unsigned int)atol(optarg);
			break;
        case 'o':
            p_input_info->dest_name = jmalloc_string(optarg);
            break;
		case 'm':
			p_input_info->micom_rom_name = jmalloc_string(optarg);
			break;
		case 'p':
			p_input_info->preloader_rom_name = jmalloc_string(optarg);
			break;
        default:
            printf("invalid argument: optarg[%s]\n", optarg);
            break;
        }
    }

	if( (p_input_info->dest_name != 0) && (p_input_info->micom_rom_name != 0)
		&& (p_input_info->preloader_rom_name != 0) && (p_input_info->mem_pll != 0) && (p_input_info->snor_size != 0)) {
		ret = TRUE;
	}
    return ret;
}

static BOOL exist_file(char *name)
{
    FILE *fd = NULL;
    return FALSE;
    if (name) {
        fd = fopen(name, "r");
        if (fd) {
            fclose(fd);
            return TRUE;
        } else {
        }
    }
    return FALSE;
}

static BOOL tcc_sfmc_mk_bootrom(tcc_input_info_x *p_input_info)
{
    BOOL ret = FALSE;
    if (p_input_info) {
        char *dest_name = p_input_info->dest_name;
        char *preloader_name = p_input_info->preloader_rom_name;
        char *micom_rom_name = p_input_info->micom_rom_name;

        if (dest_name) {
            if (!exist_file(dest_name)) {
                FILE *dest_fd = NULL;
                FILE *preloader_fd = NULL;
                FILE *micom_rom_fd = NULL;

				preloader_fd = fopen(preloader_name, "rb");
				micom_rom_fd = fopen(micom_rom_name, "rb");
                dest_fd 	 = fopen(dest_name, "w+b");

                if (dest_fd && preloader_fd && micom_rom_fd) {
					//===================================
					// Write Boot Header (60Kbyte)
					//===================================
					ret = write_snor_boot_header(dest_fd, preloader_fd, p_input_info->mem_pll, p_input_info->snor_size);
					CLOSE_HANDLE(preloader_fd, NULL, fclose);
					if(ret == FALSE)
						return ret;

					//===================================
					// Make recovery header (4Kbyte)
					//===================================
                    ret = write_recovery_header(dest_fd, micom_rom_fd);
					if(ret == FALSE)
						return ret;

					//===================================
					// micom rom write
					//===================================
					ret = write_micom_rom(dest_fd, micom_rom_fd);
					CLOSE_HANDLE(micom_rom_fd, NULL, fclose);
					if(ret == FALSE)
						return ret;

					ret = write_rom_crc(dest_fd);
					if(ret == FALSE)
						return ret;

					CLOSE_HANDLE(dest_fd, NULL, fclose);
                }
            } else {
				printf("cannot make file !! (%s) \n",  dest_name);
                printf("(%s) file exist!!!\n", dest_name);
            }
        }
    }
    return ret;
}

int main(int argc, char *argv[])
{
    int ret = -1;
    tcc_input_info_x param;

    memset(&param, 0x0, sizeof(tcc_input_info_x));

	if (parse_args(argc, argv, &param) == TRUE) {
		printf("out file: (%s)\n", param.dest_name );
		printf("micom rom file: (%s)\n", param.micom_rom_name );
		printf("preloader rom file: (%s)\n", param.preloader_rom_name );
		printf("mem pll: (%d MHz)\n", param.mem_pll);
		printf("snor size: (%d MByte)\n", param.snor_size);

	    if (!tcc_sfmc_mk_bootrom(&param)) {
	        printf("make fail!!! \n");
	    } else {
	        ret = 0;
	    }
	} else {
		help_msg(argv[0]);
	}

    return ret;
}

