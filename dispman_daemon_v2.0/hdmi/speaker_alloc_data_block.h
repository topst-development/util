/*!
* TCC Version 1.0
* Copyright (c) Telechips Inc.
* All rights reserved 
*  \file        speaker_alloc_data.h
*  \brief       HDMI speaker alloc info header
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
#ifndef SPEAKERALLOCATIONDATABLOCK_H_
#define SPEAKERALLOCATIONDATABLOCK_H_


/** 
 * @file
 * SpeakerAllocation Data Block.
 * Holds and parse the Speaker Allocation data block information.
 * For detailed handling of this structure, refer to documentation of the functions
 */

typedef struct {
	u8 mByte1;

	int mValid;
} speakerAllocationDataBlock_t;
//speaker_alloc_data_block_reset
void speaker_alloc_data_block_reset(speakerAllocationDataBlock_t * sadb);

int speaker_alloc_data_block_parse(speakerAllocationDataBlock_t * sadb, u8 * data);

/**
 * @return the Channel Allocation code used in the Audio Info frame to ease the translation process
 */
u8 get_channell_alloc_code(speakerAllocationDataBlock_t * sadb);

#endif				/* SPEAKERALLOCATIONDATABLOCK_H_ */
