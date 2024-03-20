/*!
* TCC Version 1.0
* Copyright (c) Telechips Inc.
* All rights reserved 
*  \file        hdmiforumvsdb.c
*  \brief       HDMI vendor source
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
#include <stdio.h>
#include <utils/types.h>
#include <utils/bit_operation.h>
#include "hdmiforumvsdb.h"
#include <utils/Log.h>
#define LOG_TAG         "[HDMIVSDB  ]"
void hdmiforumvsdb_reset( hdmiforumvsdb_t * forumvsdb)
{
        forumvsdb->mValid = FALSE;
        forumvsdb->mIeee_Oui = 0;
        forumvsdb->mVersion = 0;
        forumvsdb->mMaxTmdsCharRate = 0;
        forumvsdb->mSCDC_Present = FALSE;
        forumvsdb->mRR_Capable = FALSE;
        forumvsdb->mLTS_340Mcs_scramble = FALSE;
        forumvsdb->mIndependentView = FALSE;
        forumvsdb->mDualView = FALSE;
        forumvsdb->m3D_OSD_Disparity = FALSE;
        forumvsdb->mDC_30bit_420 = FALSE;
        forumvsdb->mDC_36bit_420 = FALSE;
        forumvsdb->mDC_48bit_420 = FALSE;
}

int hdmiforumvsdb_parse( hdmiforumvsdb_t * forumvsdb, u8 * data)
{
	u16 blockLength;
	hdmiforumvsdb_reset(forumvsdb);
	if (data == 0) {
		return FALSE;
	}
	if (bit_field(data[0], 5, 3) != 0x3) {
		ALOGE("Invalid datablock tag");
		return FALSE;
	}
	blockLength = bit_field(data[0], 0, 5);
	if (blockLength < 7) {
		ALOGE("Invalid minimum length");
		return FALSE;
	}
	if (byte_to_dword(0x00, data[3], data[2], data[1]) != 0xC45DD8) {
		ALOGE("HDMI IEEE registration identifier not valid");
		return FALSE;
	}
	forumvsdb->mVersion = bit_field(data[4], 0, 7);
	forumvsdb->mMaxTmdsCharRate = bit_field(data[5], 0, 7);
	forumvsdb->mSCDC_Present = bit_field(data[6], 7, 1);
	forumvsdb->mRR_Capable = bit_field(data[6], 6, 1);
	forumvsdb->mLTS_340Mcs_scramble = bit_field(data[6], 3, 1);
	forumvsdb->mIndependentView = bit_field(data[6], 2, 1);
	forumvsdb->mDualView = bit_field(data[6], 1, 1);
	forumvsdb->m3D_OSD_Disparity = bit_field(data[6], 0, 1);
	forumvsdb->mDC_30bit_420 = bit_field(data[7], 0, 1);
	forumvsdb->mDC_36bit_420 = bit_field(data[7], 1, 1);
	forumvsdb->mDC_48bit_420 = bit_field(data[7], 2, 1);
	forumvsdb->mValid = TRUE;
	return TRUE;
}

