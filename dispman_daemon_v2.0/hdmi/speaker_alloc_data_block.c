/*!
* TCC Version 1.0
* Copyright (c) Telechips Inc.
* All rights reserved 
*  \file        speaker_alloc_data.c
*  \brief       HDMI speaker alloc info source
*  \details   
*  \version     1.0
*  \date        2014-2017
*  \copyright
This source code contains confidential information of Telechips.
Any unauthorized use without a written permission of Telechips including not 
limited to re-distribution in source or binary form is strictly prohibited.
This source code is provided "AS IS"and nothing contained in this source 
code shall constitute any express or implied warranty of any kind, including
without limitation, any warranty of merchantability, fitness for a particular 
purpose or non-infringement of any patent, copyright or other third party 
intellectual property right. No warranty is made, express or implied, regarding 
the information's accuracy, completeness, or performance. 
In no event shall Telechips be liable for any claim, damages or other liability 
arising from, out of or in connection with this source code or the use in the 
source code. 
This source code is provided subject to the terms of a Mutual Non-Disclosure 
Agreement between Telechips and Company.
*/

#include <utils/types.h>
#include <utils/bit_operation.h>
#include "speaker_alloc_data_block.h"

typedef struct speaker_alloc_code {
	unsigned char byte;
	unsigned char code;
} speaker_alloc_code_t;

static speaker_alloc_code_t alloc_codes[] = {
		{1,  0},
		{3,  1},
		{5,  2},
		{7,  3},
		{17, 4},
		{19, 5},
		{21, 6},
		{23, 7},
		{9,  8},
		{11, 9},
		{13, 10},
		{15, 11},
		{25, 12},
		{27, 13},
		{29, 14},
		{31, 15},
		{73, 16},
		{75, 17},
		{77, 18},
		{79, 19},
		{33, 20},
		{35, 21},
		{37, 22},
		{39, 23},
		{49, 24},
		{51, 25},
		{53, 26},
		{55, 27},
		{41, 28},
		{43, 29},
		{45, 30},
		{47, 31},
		{0, 0}
};

void speaker_alloc_data_block_reset(speakerAllocationDataBlock_t * sadb)
{
	sadb->mByte1 = 0;
	sadb->mValid = FALSE;
}

int speaker_alloc_data_block_parse(speakerAllocationDataBlock_t * sadb, u8 * data)
{
	speaker_alloc_data_block_reset(sadb);
	if ((data != 0) && (bit_field(data[0], 0, 5) == 0x03) && (bit_field(data[0], 5, 3) == 0x04)) {
		sadb->mByte1 = data[1];
		sadb->mValid = TRUE;
		return TRUE;
	}
	return FALSE;
}

u8 get_channell_alloc_code(speakerAllocationDataBlock_t * sadb)
{
	int i = 0;
	for(i = 0; alloc_codes[i].byte != 0; i++){
		if(sadb->mByte1 == alloc_codes[i].byte){
			return alloc_codes[i].code;
		}
	}
	return (u8)-1;
}
