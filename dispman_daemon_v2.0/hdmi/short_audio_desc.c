/*!
* TCC Version 1.0
* Copyright (c) Telechips Inc.
* All rights reserved 
*  \file        short_audio_desc.c
*  \brief       HDMI audio descript source
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
#include "short_audio_desc.h"


void sad_reset( shortAudioDesc_t * sad)
{
	sad->mFormat = 0;
	sad->mMaxChannels = 0;
	sad->mSampleRates = 0;
	sad->mByte3 = 0;
}

int sad_parse( shortAudioDesc_t * sad, u8 * data)
{
	sad_reset(sad);
	if (data != 0) {
		sad->mFormat = bit_field(data[0], 3, 4);
		sad->mMaxChannels = bit_field(data[0], 0, 3) + 1;
		sad->mSampleRates = bit_field(data[1], 0, 7);
		sad->mByte3 = data[2];
		return TRUE;
	}
	return FALSE;
}

int sad_support32k( shortAudioDesc_t * sad)
{
	return (bit_field(sad->mSampleRates, 0, 1) == 1) ? TRUE : FALSE;
}

int sad_support44k1( shortAudioDesc_t * sad)
{
	return (bit_field(sad->mSampleRates, 1, 1) == 1) ? TRUE : FALSE;
}

int sad_support48k( shortAudioDesc_t * sad)
{
	return (bit_field(sad->mSampleRates, 2, 1) == 1) ? TRUE : FALSE;
}

int sad_support88k2( shortAudioDesc_t * sad)
{
	return (bit_field(sad->mSampleRates, 3, 1) ==
		1) ? TRUE : FALSE;
}

int sad_support96k( shortAudioDesc_t * sad)
{
	return (bit_field(sad->mSampleRates, 4, 1) == 1) ? TRUE : FALSE;
}

int sad_support176k4( shortAudioDesc_t * sad)
{
	return (bit_field(sad->mSampleRates, 5, 1) == 1) ? TRUE : FALSE;
}

int sad_support192k( shortAudioDesc_t * sad)
{
	return (bit_field(sad->mSampleRates, 6, 1) == 1) ? TRUE : FALSE;
}

int sad_support16bit( shortAudioDesc_t * sad)
{
	if (sad->mFormat == 1) {
		return (bit_field(sad->mByte3, 0, 1) == 1) ? TRUE : FALSE;
	}
	return FALSE;
}

int sad_support20bit( shortAudioDesc_t * sad)
{
	if (sad->mFormat == 1) {
		return (bit_field(sad->mByte3, 1, 1) == 1) ? TRUE : FALSE;
	}
	return FALSE;
}

int sad_support24bit( shortAudioDesc_t * sad)
{
	if (sad->mFormat == 1) {
		return (bit_field(sad->mByte3, 2, 1) == 1) ? TRUE : FALSE;
	}
	return FALSE;
}
