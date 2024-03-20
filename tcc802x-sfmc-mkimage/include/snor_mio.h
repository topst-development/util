/****************************************************************************
 *   FileName    : snor_mio.h
 *   Description : Serial Flash Multi I/O Driver
 ****************************************************************************
 *
 *   TCC Version 1.0
 *   Copyright (c) Telechips, Inc.
 *   ALL RIGHTS RESERVED
 *
 ****************************************************************************/
#ifndef _SNOR_MIO_H
#define _SNOR_MIO_H

#define _MICOM

#define NOR_FLASH_BASE_ADDR			(0xC0000000)
#define SFMC_BASE_ADDR          	(0x1B000000)

#define SFMC_REG_MAGIC          	(SFMC_BASE_ADDR+0x0000)
#define SFMC_REG_VERSION        	(SFMC_BASE_ADDR+0x0004)
#define SFMC_REG_PARAM          	(SFMC_BASE_ADDR+0x0008)
#define SFMC_REG_STATUS         	(SFMC_BASE_ADDR+0x000C)
#define SFMC_REG_RUN            	(SFMC_BASE_ADDR+0x0010)
#define SFMC_REG_INT_PEND       	(SFMC_BASE_ADDR+0x0014)
#define SFMC_REG_INT_ENB        	(SFMC_BASE_ADDR+0x0018)
#define SFMC_REG_BADDR_MANU     	(SFMC_BASE_ADDR+0x001C)
#define SFMC_REG_BADDR_AUTO     	(SFMC_BASE_ADDR+0x0020)
#define SFMC_REG_MODE           	(SFMC_BASE_ADDR+0x0024)
#define SFMC_REG_TIMING         	(SFMC_BASE_ADDR+0x0028)
#define SFMC_REG_DELAY_SO       	(SFMC_BASE_ADDR+0x002C)
#define SFMC_REG_DELAY_CLK      	(SFMC_BASE_ADDR+0x0030)
#define SFMC_REG_DELAY_WBD0     	(SFMC_BASE_ADDR+0x0034)
#define SFMC_REG_DELAY_WBD1     	(SFMC_BASE_ADDR+0x0038)
#define SFMC_REG_DELAY_RBD0     	(SFMC_BASE_ADDR+0x003C)
#define SFMC_REG_DELAY_RBD1     	(SFMC_BASE_ADDR+0x0040)
#define SFMC_REG_DELAY_WOEBD0   	(SFMC_BASE_ADDR+0x0044)
#define SFMC_REG_DELAY_WOEBD1   	(SFMC_BASE_ADDR+0x0048)
#define SFMC_REG_DELAY_TIMEOUT  	(SFMC_BASE_ADDR+0x004C)
#define SFMC_REG_CODE_TABLE     	(SFMC_BASE_ADDR+0x0800)


#define SFMC_REG_RUN_MAN_IDLE				( 0x0 << 0)
#define SFMC_REG_RUN_MAN_RUN				( 0x1 << 0)
#define SFMC_REG_RUN_MAN_STOP				( 0x3 << 0)

#define SFMC_REG_RUN_AUTO_IDLE				( 0x0 << 4)
#define SFMC_REG_RUN_AUTO_RUN				( 0x1 << 4)
#define SFMC_REG_RUN_AUTO_STOP				( 0x3 << 4)

#define SFMC_REG_RUN_SOFT_RESET				( 0x1 << 8)

#define SFMC_REG_INT_ENB_USER_0 			(	1 << 0)
#define SFMC_REG_INT_ENB_USER_1 			(	1 << 1)
#define SFMC_REG_INT_ENB_USER_2 			(	1 << 2)
#define SFMC_REG_INT_ENB_USER_3 			(	1 << 3)

#define SFMC_REG_INT_ENB_CORE_ERR_EN 		(	1 << 4)
#define SFMC_REG_INT_ENB_TIMEOUT_EN			(	1 << 5)

#define SFMC_REG_MODE_FLASH_RESET			(	1 << 0)
#define SFMC_REG_MODE_MODE_DQUAD			(	1 << 1)	// Dual Quad mode enable (Active High)
#define SFMC_REG_MODE_FIFO_CTRL_EN			(	1 << 2) // Enable DQS Clock
#define SFMC_REG_MODE_FIFO_CTRL_DIS 		(	0)
#define SFMC_REG_MODE_SERIAL_0				( 0x1 << 4) // Serial Mode 0
#define SFMC_REG_MODE_SERIAL_3				( 0x2 << 4) // Serial Mode 3
#define SFMC_REG_MODE_CROSS_WR				(	1 << 6) // Write Data Cross Enable (Octa DTR Mode)
#define SFMC_REG_MODE_CROSS_RD				(	1 << 7) // Read Data Cross Enable (Octa DTR Mode)
#define SFMC_REG_MODE_SIO_OFF_QUAD			( 0xF0<< 8) // Quad Memory
#define SFMC_REG_MODE_SIO_OFF_OCTA			( 0x00<< 8) // Octa or Dual Quad Memory

#define SFMC_REG_TIMING_SEL_P_180			(	1 << 0)	// Select rd_p_180 data
#define SFMC_REG_TIMING_SEL_N_180			(	1 << 1) // Select rd_n_180 data
#define SFMC_REG_TIMING_SEL_PN				(	1 << 2) // Secect p data (must 1 at DTR mode)
#define SFMC_REG_TIMING_SEL_PN_DT(x)		( (x) << 2) // Secect p data (must 1 at DTR mode)

#define SFMC_REG_TIMING_SEL_DQS_FCLK 		(0x0 << 4) // fclk
#define SFMC_REG_TIMING_SEL_DQS_FCLK_PAD	(0x1 << 4) // fclk pad input
#define SFMC_REG_TIMING_SEL_DQS_I_DQS_OCTA	(0x2 << 4) // i_dqs octa only
#define SFMC_REG_TIMING_SEL_DQS_N_FCLK 		(0x3 << 4) // ~fclk
#define SFMC_REG_TIMING_SEL_DQS(x) 			((x) << 4) // ~fclk

#define SFMC_REG_TIMING_READ_LATENCY(x)		(((x) << 8) & 0x00000F00)
#define SFMC_REG_TIMING_CS_TO_CS(x)			(((x) << 16) & 0x000F0000)
#define SFMC_REG_TIMING_SC_EXTND(x)			(((x) << 20) & 0x00300000)

#define SFMC_REG_DELAY_SO_SLDH(x)			(((x) << 0) & 0x000000FF)	// CS Low to SO Latency (0 ~7)
#define SFMC_REG_DELAY_SO_SLCH(x)			(((x) << 8) & 0x00000300)	// CS Low to SCLK Latency (0 ~2)
#define SFMC_REG_DELAY_SO_INV_SCLK(x)		((x) << 10)	// Invert SCLK


#define SFMC_REG_DELAY_CLK_WD(x)			(((x) <<  0) & 0x000000FF)	//SCLK Clock Delay (0xE7) [7:6] Delay ctrl, [5:0] clk_buf sel
#define SFMC_REG_DELAY_CLK_WD_BUF(x)		(((x) <<  0) & 0x0000003F)	//SCLK Clock Delay [5:0] clk_buf sel
#define SFMC_REG_DELAY_CLK_WD_CTRL(x)		(((x) <<  6) & 0x000000C0)	//SCLK Clock Delay [7:6] Delay ctrl


#define SFMC_REG_DELAY_CLK_RD(x)			(((x) <<  8) & 0x0000FF00)	//Read Clock Delay 	[15:14] Delay ctrl, [13:8] clk buf sel

#define SFMC_REG_DELAY_CLK_RD_BUF(x)		(((x) <<  8) & 0x00003F00)	//Read Clock Delay [15:14] clk_buf sel
#define SFMC_REG_DELAY_CLK_RD_CTRL(x)		(((x) << 14) & 0x0000C000)	//Read Clock Delay [13:8] Delay ctrl

#define SFMC_REG_DELAY_CLK_CSN_BD(x)		(((x) << 16) & 0x001F0000)	//CSN Signal Delay
#define SFMC_REG_DELAY_CLK_WR_TAB			(	1 << 21) // CLK_WD tab_delay chg
#define SFMC_REG_DELAY_CLK_RD_TAB			(	1 << 22) // CLK_RD tab_delay chg


#define SFMC_REG_DELAY_WBD0_SO_0(x)			(((x) <<  0) & 0x000000FF)	// Flash Memory SO[0] Write bit delay
#define SFMC_REG_DELAY_WBD0_SO_1(x)			(((x) <<  8) & 0x0000FF00)	// Flash Memory SO[1] Write bit delay
#define SFMC_REG_DELAY_WBD0_SO_2(x)			(((x) << 16) & 0x00FF0000)	// Flash Memory SO[2] Write bit delay
#define SFMC_REG_DELAY_WBD0_SO_3(x)			(((x) << 24) & 0xFF000000)	// Flash Memory SO[3] Write bit delay

#define SFMC_REG_DELAY_WBD1_SO_4(x)			(((x) <<  0) & 0x000000FF)	// Flash Memory SO[4] Write bit delay
#define SFMC_REG_DELAY_WBD1_SO_5(x)			(((x) <<  8) & 0x0000FF00)	// Flash Memory SO[5] Write bit delay
#define SFMC_REG_DELAY_WBD1_SO_6(x)			(((x) << 16) & 0x00FF0000)	// Flash Memory SO[6] Write bit delay
#define SFMC_REG_DELAY_WBD1_SO_7(x)			(((x) << 24) & 0xFF000000)	// Flash Memory SO[7] Write bit delay


#define SFMC_REG_DELAY_RBD0_SO_0(x)			(((x) <<  0) & 0x000000FF)	// Flash Memory SO[0] Read bit delay
#define SFMC_REG_DELAY_RBD0_SO_1(x)			(((x) <<  8) & 0x0000FF00)	// Flash Memory SO[1] Read bit delay
#define SFMC_REG_DELAY_RBD0_SO_2(x)			(((x) << 16) & 0x00FF0000)	// Flash Memory SO[2] Read bit delay
#define SFMC_REG_DELAY_RBD0_SO_3(x)			(((x) << 24) & 0xFF000000)	// Flash Memory SO[3] Read bit delay

#define SFMC_REG_DELAY_RBD1_SO_4(x)			(((x) <<  0) & 0x000000FF)	// Flash Memory SO[4] Read bit delay
#define SFMC_REG_DELAY_RBD1_SO_5(x)			(((x) <<  8) & 0x0000FF00)	// Flash Memory SO[5] Read bit delay
#define SFMC_REG_DELAY_RBD1_SO_6(x)			(((x) << 16) & 0x00FF0000)	// Flash Memory SO[6] Read bit delay
#define SFMC_REG_DELAY_RBD1_SO_7(x)			(((x) << 24) & 0xFF000000)	// Flash Memory SO[7] Read bit delay


#define SFMC_REG_DELAY_WOEBD0_SO_0(x)		(((x) <<  0) & 0x000000FF)	// Flash Memory SO[0] Write Output Enable bit delay
#define SFMC_REG_DELAY_WOEBD0_SO_1(x)		(((x) <<  8) & 0x0000FF00)	// Flash Memory SO[1] Write Output Enable bit delay
#define SFMC_REG_DELAY_WOEBD0_SO_2(x)		(((x) << 16) & 0x00FF0000)	// Flash Memory SO[2] Write Output Enable bit delay
#define SFMC_REG_DELAY_WOEBD0_SO_3(x)		(((x) << 24) & 0xFF000000)	// Flash Memory SO[3] Write Output Enable bit delay

#define SFMC_REG_DELAY_WOEBD1_SO_4(x)		(((x) <<  0) & 0x000000FF)	// Flash Memory SO[4] Write Output Enable bit delay
#define SFMC_REG_DELAY_WOEBD1_SO_5(x)		(((x) <<  8) & 0x0000FF00)	// Flash Memory SO[5] Write Output Enable bit delay
#define SFMC_REG_DELAY_WOEBD1_SO_6(x)		(((x) << 16) & 0x00FF0000)	// Flash Memory SO[6] Write Output Enable bit delay
#define SFMC_REG_DELAY_WOEBD1_SO_7(x)		(((x) << 24) & 0xFF000000)	// Flash Memory SO[7] Write Output Enable bit delay


#define SFMC_REG_DELAY_TIMEOUT_CNT(x)		(((x) << 0) & 0x0000FFFF)	// Timeout Counter [15:0]
#define SFMC_REG_DELAY_TIMEOUT_ENB			(	1 << 16)				// Timeout Enable


#define CMD0_READ				0x0
#define CMD0_WRITE				0x1
#define CMD0_WRITE_BYTE			0x2
#define CMD0_OPERATION			0x3

#define CMD1_OPERATION			0x0
#define CMD1_COMPARE			0x1
#define CMD1_BR_INT				0x2
#define CMD1_WAIT_STOP			0x3

#define CMD2_BRANCH				0x0
#define CMD2_INT				0x1
#define CMD2_WAIT				0x0
#define CMD2_STOP				0x1

#define LA_KEEP_CS				0x0
#define LA_DEASSERT_CS			0x1

#define D_DTR_DISABLE			0x0	// Double Transfer Rate(DTR) mode disable
#define D_DTR_ENABLE			0x1

#define AR_TABLE_WRITE			0x0
#define AR_AHB_WRITE			0x1

#define AA_TABLE_ADDR			0x0
#define AA_AHB_ADDR				0x1

#define DU_REAL_DATA			0x0
#define DU_DUMMY_DATA			0x1

#define IO_NUM_NC				0xF
#define IO_NUM_SINGLE			0x0
#define IO_NUM_DUAL				0x1
#define IO_NUM_QUAD				0x2
#define IO_NUM_OCTA				0x3

#define MK_WRITE_CMD(CODE, DTR, DATA, SIZE, LA, IO_NUM)	{\
				(CODE) = ((CMD0_WRITE_BYTE 	<< 30)	\
						| (LA 				<< 29)	\
						| (DTR			 	<< 28) 	\
						| (SIZE				<< 26)	\
						| (IO_NUM			<< 24)	\
						| (DATA				<<  0));\
}

#define MK_WRITE_ADDR_AHB(CODE, DTR, IO_NUM) {		\
				(CODE) = ((CMD0_WRITE	 	<< 30)	\
						| (LA_KEEP_CS		<< 29)	\
						| (DTR 				<< 28) 	\
						| (AA_AHB_ADDR		<< 27)	\
						| (DU_REAL_DATA		<< 26)	\
						| (IO_NUM			<< 24)	\
						| (0				<<  0));\
}

#define MK_WRITE_ADDR_TABLE(CODE, ADDR, SIZE, IO_NUM)	{	\
				(CODE) = ((CMD0_WRITE_BYTE 	<< 30)	\
						| (LA_KEEP_CS	<< 29)	\
						| (D_DTR_DISABLE 	<< 28) 	\
						| (AA_TABLE_ADDR	<< 27)	\
						| (DU_DUMMY_DATA		<< 26)	\
						| (IO_NUM			<< 24)	\
						| (SIZE 			<< 12)	\
						| (ADDR				<<  0));\
}

#define MK_WRITE_ADDR(CODE, ADDR, SIZE, IO_NUM)	{	\
				(CODE) = ((CMD0_WRITE_BYTE 	<< 30)	\
						| (LA_DEASSERT_CS	<< 29)	\
						| (D_DTR_DISABLE 	<< 28) 	\
						| (AA_TABLE_ADDR	<< 27)	\
						| (DU_DUMMY_DATA		<< 26)	\
						| (IO_NUM			<< 24)	\
						| (SIZE 			<< 12)	\
						| (ADDR				<<  0));\
}

#define MK_WRITE_DUMMY_CYCLE(CODE, DTR, SIZE, IO_NUM) {	\
				(CODE) = ((CMD0_WRITE	 	<< 30)	\
						| (LA_KEEP_CS		<< 29)	\
						| (DTR			 	<< 28) 	\
						| (AA_TABLE_ADDR	<< 27)	\
						| (DU_DUMMY_DATA 	<< 26)	\
						| (IO_NUM			<< 24)	\
						| (SIZE				<< 12));\
}

#define MK_READ_DATA_AHB(CODE, DTR, IO_NUM)	{			\
				(CODE) = ((CMD0_READ	 	<< 30)	\
						| (LA_DEASSERT_CS	<< 29)	\
						| (DTR				<< 28) 	\
						| (AR_AHB_WRITE		<< 27)	\
						| (IO_NUM			<< 24)	\
						| (0				<<  0));\
}

#define MK_READ_DATA(CODE, SIZE, ADDR, IO_NUM)	{	\
				(CODE) = ((CMD0_READ	 	<< 30)	\
						| (LA_DEASSERT_CS	<< 29)	\
						| (D_DTR_DISABLE	<< 28) 	\
						| (AR_TABLE_WRITE	<< 27)	\
						| (IO_NUM			<< 24)	\
						| (SIZE 			<< 12)	\
						| (ADDR				<<  0));\
}

#define MK_WRITE_DATA_AHB(CODE, IO_NUM)	{			\
				(CODE) = ((CMD0_WRITE	 	<< 30)	\
						| (LA_DEASSERT_CS	<< 29)	\
						| (D_DTR_DISABLE	<< 28) 	\
						| (AA_AHB_ADDR		<< 27)	\
						| (IO_NUM			<< 24)	\
						| (0				<<  0));\
}

#define MK_WRITE_DATA(CODE, SIZE, ADDR, IO_NUM)	{	\
				(CODE) = ((CMD0_WRITE	 	<< 30)	\
						| (LA_DEASSERT_CS		<< 29)	\
						| (D_DTR_DISABLE	<< 28) 	\
						| (AA_TABLE_ADDR	<< 27)	\
						| (IO_NUM			<< 24)	\
						| (SIZE 			<< 12)	\
						| (ADDR				<<  0));\
}

#define MK_WRITE_PP_ADDR(CODE, SIZE, ADDR, IO_NUM)	{	\
				(CODE) = ((CMD0_WRITE	 	<< 30)	\
						| (LA_KEEP_CS		<< 29)	\
						| (D_DTR_DISABLE	<< 28) 	\
						| (AA_TABLE_ADDR	<< 27)	\
						| (IO_NUM			<< 24)	\
						| (SIZE 			<< 12)	\
						| (ADDR				<<  0));\
}


#define MK_WAIT_CMD(CODE,DELAY) { 					\
				(CODE) = ((CMD0_OPERATION	<< 30)	\
						| (CMD1_WAIT_STOP	<< 28)	\
						| (CMD2_WAIT		<< 26)	\
						| (DELAY			<<  0));\
}

#define MK_STOP_CMD(CODE) { 						\
				(CODE) = ((CMD0_OPERATION	<< 30)	\
						| (CMD1_WAIT_STOP	<< 28)	\
						| (CMD2_STOP		<< 26)	\
						| (0				<<  0));\
}

#define SET_CODE_TABLE(OFFSET, VAL) 		\
	(*(volatile unsigned int *)(SFMC_REG_CODE_TABLE + (OFFSET<<2)) = VAL)

#define GET_CODE_TABLE(OFFSET) 		\
	(*(volatile unsigned int *)(SFMC_REG_CODE_TABLE + (OFFSET<<2)))

#define GET_CODE_TABLE_ADDR(OFFSET) 		\
	(SFMC_REG_CODE_TABLE + (OFFSET<<2));	\


#define SET_CMD_AUTO_ADDR(OFFSET) 		\
	(*(volatile unsigned int *)(SFMC_REG_BADDR_AUTO) = ((0x800)+(OFFSET<<2)));	\


#define SET_CMD_MANU_ADDR(OFFSET) 		\
	(*(volatile unsigned int *)(SFMC_REG_BADDR_MANU) = ((0x800)+(OFFSET<<2)));	\

#define SET_CMD_RUN(VAL) 		\
	(*(volatile unsigned int *)(SFMC_REG_RUN) = VAL);	\

#define WAIT_CMD_COMPLETE() {		\
	while(1) {	\
		if((*(volatile unsigned int *)(SFMC_REG_RUN) & 0xF)== 0)	\
			break;	\
	};	\
}

typedef struct  {
	unsigned int 	offset;
	unsigned int	size;
} tCODE_TABLE_INFO;

struct SNOR_MIO_PRODUCT_INFO {
	char *name;
	unsigned char 	ManufID;
	unsigned short 	DevID;
	unsigned int 	TotalSector;
	unsigned short 	cmd_read;
	unsigned short 	cmd_read_fast;
	unsigned short 	cmd_write;
	unsigned short 	flags;
};

#define SFMC_BUF_SIZE		256

struct SNOR_MIO_DRV {
    const char 		*name;
	unsigned char 	ManufID;
	unsigned short 	DevID;
    unsigned char 	shift;
    unsigned short 	flags;
    unsigned short 	current_io_mode;
    unsigned short max_read_io;

    unsigned int 		size;
    unsigned int 		page_size;
    unsigned int 		sector_size;
	unsigned int 		sector_count;
    unsigned int 		erase_size;

    unsigned short 		cmd_read;
    unsigned short 		cmd_read_fast;
    unsigned short 		cmd_write;
    unsigned short 		erase_cmd;

	unsigned short 		dt_mode;

	tCODE_TABLE_INFO	sfmc_buf;
	tCODE_TABLE_INFO	sfmc_addr;
	tCODE_TABLE_INFO	rdid;
	tCODE_TABLE_INFO	rdsr;
	tCODE_TABLE_INFO	wrsr;
	tCODE_TABLE_INFO	rdcr;
	tCODE_TABLE_INFO	en4b;
	tCODE_TABLE_INFO	ex4b;
	tCODE_TABLE_INFO	ear_mode;

	//tCODE_TABLE_INFO	en_m_io;
	//tCODE_TABLE_INFO	ex_m_io;
	//tCODE_TABLE_INFO	config_dummy;

	tCODE_TABLE_INFO	write_enable;
	tCODE_TABLE_INFO 	write_disable;
	tCODE_TABLE_INFO	read;
	tCODE_TABLE_INFO	read_fast;
	tCODE_TABLE_INFO 	write;
	tCODE_TABLE_INFO	blk_erase;
	tCODE_TABLE_INFO	sec_erase;
	unsigned int		iSFMC_REG_TIMING;
	unsigned int		iSFMC_REG_DELAY_SO;
	unsigned int		iSFMC_REG_DELAY_CLK;
};

//---------------------------------------------
// Erase commands
//---------------------------------------------
// 3 byte Address Command Set
//---------------------------------------------
#define CMD_ERASE_4K				0x20
#define CMD_ERASE_4K_4B				0x21
#define CMD_ERASE_32K				0x52
#define CMD_ERASE_CHIP				0xc7
#define CMD_ERASE_64K				0xd8
#define CMD_ERASE_64K_4B			0xDC

//---------------------------------------------
// Write commands
//---------------------------------------------
// 3 byte Address Command Set
//---------------------------------------------
#define CMD_PP						0x02	// page program
#define CMD_4PP						0x38	// Quad page program

//---------------------------------------------
// 4 byte Address Command Set
//---------------------------------------------
#define CMD_PP4B					0x12	// page program 4b
#define CMD_4PP4B					0x3E	// Quad page program 4b
#define CMD_8PP4B					0x12ED	// Octa Page program 4b

//---------------------------------------------

#define CMD_WRITE_STATUS			0x01

#define CMD_WRITE_DISABLE			0x04
#define CMD_READ_STATUS				0x05
#define CMD_READ_CONFIG				0x15

#define CMD_QUAD_PAGE_PROGRAM		0x32
#define CMD_READ_STATUS1			0x35
#define CMD_WRITE_ENABLE			0x06
//#define CMD_READ_CONFIG				0x35
#define CMD_FLAG_STATUS				0x70

#define CMD_WRITE_CONFIG2_OPI		0x728D
#define CMD_WRITE_CONFIG2			0x72

#define CONFIG2_SPI_MODE			0x00
#define CONFIG2_STR_OPI_MODE		0x01
#define CONFIG2_DTR_OPI_MODE		0x02



//---------------------------------------------
// Read commands
//---------------------------------------------
// 3 byte Address Command Set
//---------------------------------------------
#define CMD_READ				0x03	// Normal read
#define CMD_FAST_READ			0x0B	// Fast read
#define CMD_FASTDTRD			0x0D	// Fast DT read

#define CMD_DREAD				0x3B	// Dual read
#define CMD_QREAD				0x6B	// Quad read

#define CMD_4READ				0xEB	// Quad STR read SPI/QPI

#define CMD_4DTRD				0xED	// Quad DT read

//---------------------------------------------
// 4 byte Address Command Set
//---------------------------------------------
#define CMD_READ4B				0x13
#define CMD_FAST_READ4B			0x0C
#define CMD_FASTDTRD_4B			0x0E

#define CMD_DREAD4B				0x3C
#define CMD_QREAD4B 			0x6C

#define CMD_4READ4B				0xEC	// Quad STR read SPI/QPI

#define CMD_4DTRD4B				0xEE	// Quad DT read


#define CMD_8READ				0xEC13
#define CMD_8DTRD				0xEE11

//---------------------------------------------

#define CMD_READ_ID					0x9f

#define CMD_EAR						0xC5

#define SNOR_SERIAL_MODE_0			(1 << 0)
#define SNOR_SERIAL_MODE_3			(1 << 1)

#define SNOR_PROTOCOL_SINGLE		(1 << 0)
#define SNOR_PROTOCOL_DUAL			(1 << 1)
#define SNOR_PROTOCOL_QUAD			(1 << 2)
#define SNOR_PROTOCOL_OCTA			(1 << 3)


#if 0		/* 016.11.09 */
/* Enum list - Full read commands */
enum SNOR_READ_CMDS {
	SINGLE_SLOW			= 1 << 0,
	SINGLE_FAST			= 1 << 1,
	SINGLE_FAST_DT		= 1 << 2,
	DUAL_FAST	= 1 << 3,
	QUAD_FAST	= 1 << 4,
	OCTA_FAST	= 1 << 5,

};

enum SNOR_WRITE_CMDS {
	SINGLE_IO_SLOW			= 1 << 0,
	SINGLE_IO_FAST			= 1 << 1,
	DUAL_IO_FAST		= 1 << 2,
	QUAD_IO_FAST		= 1 << 3,
	OCTA_IO_FAST		= 1 << 4,
};
#endif /* 0 */

#if 0		/* 016.11.09 */
/* Read commands array */
static unsigned short SNOR_READ_CMD_array[] = {
	CMD_READ,
	CMD_FAST_READ,
	CMD_FASTDTRD,
	CMD_DREAD,
	CMD_QREAD,
	CMD_READ_OCTA,
};

static unsigned short SNOR_READ_CMD_4B_array[] = {
	CMD_READ4B,
	CMD_FAST_READ4B,
	CMD_FASTDTRD_4B,

	CMD_DREAD,
	CMD_QREAD4B,
	CMD_READ_OCTA,
};

#define RD_SINGLE	(SINGLE_SLOW | SINGLE_FAST)
#define RD_DUAL		(RD_SINGLE | DUAL_FAST)
#define RD_QUAD		(RD_SINGLE | QUAD_FAST)
#define RD_OCTA		(RD_SINGLE | OCTA_FAST)


#define WR_SINGLE	(SINGLE_IO_SLOW | SINGLE_IO_FAST)
#define WR_DUAL		(RD_SINGLE | DUAL_IO_FAST)
#define WR_QUAD		(RD_SINGLE | QUAD_IO_FAST)
#define WR_OCTA		(RD_SINGLE | OCTA_IO_FAST)
#endif /* 0 */

/* sf param flags */
enum {
	SECT_4K		= 1 << 0,
	SECT_32K	= 1 << 1,
	E_FSR		= 1 << 2,
	SST_WR		= 1 << 3,
	WR_QPP		= 1 << 4,
	ADDR_4B		= 1 << 5,
};


//----------------------------------------------------------------
// SNOR MIO Boot header
//----------------------------------------------------------------
#ifdef _MICOM
#define SF_QPI_INIT_HEADER_OFFSET		0x0
#define SF_QPI_1ST_HEADER_OFFSET 		0x100
#define SF_QPI_ROM_AREA_OFFSET 			0x500

#define SF_QPI_SERIAL_NUM_OFFSET 		0x600

#define SF_ROM_COPY_NUM					1
#else
#define SF_QPI_INIT_HEADER_OFFSET		0x0
#define SF_QPI_1ST_HEADER_OFFSET 		0x100
#define SF_QPI_SERIAL_NUM_OFFSET 		0x200

#define SF_QPI_2ND_INIT_HEADER_OFFSET	0x10000
#define SF_QPI_2ND_HEADER_OFFSET 		0x10100
#define SF_QPI_ROM_AREA_OFFSET			0x10200

#define SF_ROM_COPY_NUM					1
#endif

#define SF_QPI_INIT_CODE_MODE_SEL_SPI		(0 << 0)
#define SF_QPI_INIT_CODE_MODE_SEL_QPI		(1 << 0)
#define SF_QPI_INIT_CODE_MODE_SEL_QPI_DUAL	(2 << 0)
#define SF_QPI_INIT_CODE_MODE_SEL_OPI		(3 << 0)

#define SF_QPI_INIT_CODE_PN_STR				(0 << 2)
#define SF_QPI_INIT_CODE_PN_DTR				(1 << 2)

#define SF_QPI_INIT_CODE_MODE_AUTO			(0 << 3)
#define SF_QPI_INIT_CODE_MODE_MANU			(1 << 3)

#define SF_QPI_INIT_CODE_FCLK_DIV_SHIFT		(4)

#define SF_QPI_INIT_CODE_FCLK_SEL_PLL0		(0 << 12)
#define SF_QPI_INIT_CODE_FCLK_SEL_PLL1		(1 << 12)
#define SF_QPI_INIT_CODE_FCLK_SEL_PLL2		(2 << 12)
#define SF_QPI_INIT_CODE_FCLK_SEL_PLL3		(3 << 12)

#define SF_QPI_INIT_CODE_RW_READ			(0 << 31)
#define SF_QPI_INIT_CODE_RW_WRITE			(1 << 31)

#define CODE_VLU_SIZE	47

typedef	struct {
	unsigned int	code;
				//[1:0]   -> sflash_mode_sel        => 0x0 : SPI 0x1: QPI, 0x2: QPI-DUAL 0x3: OPI
			    //[2]     -> str/dtr                => 0x0: STR, 0x1: DTR
			    //[3]     -> auto/manu              => 0x0:AUTO, 0x1:MANU
			    //[11:4]  -> fclk_div(fin=1600MHz)  => 0x1f: 50MHz, 0x1d: 53MHz, 0x1b: 57MHz, 0x19: 61MHz. 0x17:66MHz, 0x15:72MHz, 0x13: 80MHz, 0x11:88MHz,
			    //					  				   0xf: 100MHz, 0xd: 114MHz,  0xb:133MHz, 0x9: 160MHz, 0x7:200MHz, 0x5:266MHz, 0x3: 400MHz, 0x1: 800MHz.
			    //[13:12] -> fclk_sel               => 0x0: PLL0, 0x1: PLL1, 0x2: PLL2, 0x3 : PLL3
			    //[31]    -> read/write             => 0x0: READ, 0x1: WRITE
	unsigned int	timing;
	unsigned int	delay_so;
	unsigned int	dc_clk;
	unsigned int	dc_wbd0;
	unsigned int	dc_wbd1;
	unsigned int	dc_rbd0;
	unsigned int	dc_rbd1;
	unsigned int	dc_woebd0;
	unsigned int	dc_woebd1;
	unsigned int	dc_base_addr_manu_0;
	unsigned int	dc_base_addr_manu_1;
	unsigned int	dc_base_addr_auto;	//0x30
	unsigned int	run_mode;
	//unsigned int	ulReserved[2];
	unsigned int 	ulMID;
	unsigned int 	ulDID;
	unsigned int	code_vlu[CODE_VLU_SIZE];		// 0x40
	unsigned int	ulCRC;
} sSFQPI_InitHeader;



typedef	struct _tad_SFMC_CODE_PARAM_T
{
	unsigned int	code;
	unsigned int	timing;
	unsigned int	delay_so;
	unsigned int	dc_clk;
	unsigned int	codeset_size;
	unsigned int	codeset[];
} sSFMC_CODE_PARAM_T, *PSFMC_CODE_PARAM_T;

typedef struct _tag_ROM_CODE_INFO_T
{
	unsigned int 	ulRomCodeSize;		// [bytes]
	unsigned int	ulRomCodeOffset;	// [Sector, 512bytes = 1 sector]
	unsigned int	ulRomTargetAddrss;	// [Load Address]
	unsigned int	ulRomCodeCRC;			// [CRC]
} ROM_CODE_INFO_T;


enum ROM_CODE_TYPE{
	ROM_CODE_CM4 = 0,
	ROM_CODE_WARM_BOOT,
	ROM_CODE_DRAM_INIT,
	ROM_CODE_BOOT_ROM,
	ROM_CODE_TYPE_MAX,
};

//static unsigned char ROM_CODE_array[] = {
//	ROM_CODE_CM4,
//	ROM_CODE_WARM_BOOT,
//	ROM_CODE_DRAM_INIT,
//	ROM_CODE_BOOT_ROM,
//	ROM_CODE_TYPE_MAX,
//};

typedef	struct {
	unsigned int		Signature;				// @ 0xC0000080
	ROM_CODE_INFO_T		rom_code[ROM_CODE_TYPE_MAX];
	//ROM_CODE_INFO_T		cm4_code;
	//ROM_CODE_INFO_T		warm_code;
	//ROM_CODE_INFO_T		dram_code;
	//ROM_CODE_INFO_T		boot_code;
	unsigned int 		ulReserved[ (30 - (ROM_CODE_TYPE_MAX*4))];
	unsigned int 		ulCRC;
} sSFQPI_BootHeader;	// 128 byte

typedef struct _tag_TCC_ROM_CODE_INFO_T
{
	unsigned int 	ulRomCodeSize;		// [bytes]
	void 			*ulRomCodebuffer;
	unsigned int	ulRomTargetAddrss;	// [Load Address]
	unsigned int	ulRomCodeCRC;		// [CRC]
} TCC_ROM_CODE_INFO_T;

typedef	struct {
	TCC_ROM_CODE_INFO_T		rom_code[ROM_CODE_TYPE_MAX];
	//TCC_ROM_CODE_INFO_T		cm4_code;
	//TCC_ROM_CODE_INFO_T		warm_code;
	//TCC_ROM_CODE_INFO_T		dram_code;
	//TCC_ROM_CODE_INFO_T		boot_code;
} sTCC_FirmwareHeader;

typedef struct
{
	unsigned int	uTcc_start;
	unsigned int	uHardwareID;
	unsigned int	uFirmwareVersion[2];
	unsigned int	uFirmwareCheckSum;	// 128kbyte CRC
	unsigned int	uReserved_1;
	unsigned int	uFirmwareCheckSumEnd;	// 128kbyte ~ end CRC
	unsigned int 	uFirmwareSize;
	unsigned int	uSerialNumber[16];
	unsigned int	uFirmwareBaseAddress;
	unsigned int	uReserved_2;
	unsigned int	uFwdnSig;
	unsigned int	uChipsetName;
	unsigned int	uInitRoutine_StartBase;
	unsigned int	uInitRoutine_StartLimit;
	unsigned int	uSnorSig;
	unsigned int	uSnorLoaderStart;
	unsigned int	uSnorLoaderSize;
	unsigned int	uSnorLoaderBaseAddr;
} sTCCBootRomHeader;


unsigned int SNOR_MIO_GetBootAddress(void);
unsigned int SNOR_MIO_GetBootSize(void);
int SNOR_MIO_Init(void);
void SNOR_MIO_Make_InitHeader(sSFQPI_InitHeader *sfInitHeader);
void SNOR_MIO_Make_InitHeader_for_micom(sSFQPI_InitHeader *sfInitHeader);
void SNOR_MIO_Erase(unsigned int address, unsigned int size);
int SNOR_MIO_Write(unsigned int address, const void *pBuffer, unsigned int length);
int SNOR_MIO_Read(unsigned int address, void *pBuffer, unsigned int length);
void SNOR_MIO_PortConfig(void);
unsigned int SNOR_MIO_GetDataAreaSize(void);
unsigned int SNOR_MIO_GetDataAreaAddress(void);

int SNOR_MIO_DataArea_Read(unsigned long ulLBA_addr, unsigned long ulSector, void *buff);
int SNOR_MIO_DataArea_Write(unsigned long ulLBA_addr, unsigned long ulSector, void *buff);

int FWND_SNOR_MIO_GetSerialNumber(char* Serial);
unsigned int SNOR_MIO_GetTotalSize(void);
void SNOR_MIO_SET_FASTREAD(void);

int FWDN_SNOR_MIO_Write_Firmware( unsigned char *pucRomFile_Buffer, unsigned int uiROMFileSize);
int FWDN_SNOR_MIO_SetSerialFlashSerial(unsigned char *ucData, unsigned int overwrite);
int FWND_SNOR_MIO_GetSerialNorSerial(void);

int IO_I2C_PortConfig(int nCh);
int eeprom_init(void);
void eeprom_firmware_write(unsigned char *ucBuffer, unsigned int write_size);
void eeprom_firmware_read(unsigned char *ucBuffer, unsigned int read_size);


#endif
