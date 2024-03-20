/*!
* TCC Version 1.0
* Copyright (c) Telechips Inc.
* All rights reserved 
*  \file        video_cap_data_block.h
*  \brief       HDMI video capable info header
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
#include "video_cap_data_block.h"


void video_cap_data_block_reset(videoCapabilityDataBlock_t * vcdb)
{
	vcdb->mQuantizationRangeSelectable = FALSE;
	vcdb->mPreferredTimingScanInfo = 0;
	vcdb->mItScanInfo = 0;
	vcdb->mCeScanInfo = 0;
	vcdb->mValid = FALSE;
}

int video_cap_data_block_parse(videoCapabilityDataBlock_t * vcdb, u8 * data)
{
	video_cap_data_block_reset(vcdb);
	/* check tag code and extended tag */
	if ((data != 0) && (bit_field(data[0], 5, 3) == 0x7) &&
		(bit_field(data[1], 0, 8) == 0x0) && (bit_field(data[0], 0, 5) == 0x2)) {	/* so far VCDB is 2 bytes long */
		vcdb->mCeScanInfo = bit_field(data[2], 0, 2);
		vcdb->mItScanInfo = bit_field(data[2], 2, 2);
		vcdb->mPreferredTimingScanInfo = bit_field(data[2], 4, 2);
		vcdb->mQuantizationRangeSelectable = (bit_field(data[2], 6, 1) == 1) ? TRUE : FALSE;
		vcdb->mValid = TRUE;
		return TRUE;
	}
	return FALSE;
}
