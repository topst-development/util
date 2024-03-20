/*!
* TCC Version 1.0
* Copyright (c) Telechips Inc.
* All rights reserved 
*  \file        hdmiforumvsdb.h
*  \brief       HDMI vendor header
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

#ifndef HDMIFORUMVSDB_H_
#define HDMIFORUMVSDB_H_

#ifdef __cplusplus
extern "C" {
#endif

/* HDMI 2.0 HF_VSDB */
typedef struct {
	u32 mIeee_Oui;

	u8 mValid;

	u8 mVersion;

	u8 mMaxTmdsCharRate;

	u8 m3D_OSD_Disparity;

	u8 mDualView;

	u8 mIndependentView;

	u8 mLTS_340Mcs_scramble;

	u8 mRR_Capable;

	u8 mSCDC_Present;

	u8 mDC_30bit_420;

	u8 mDC_36bit_420;

	u8 mDC_48bit_420;

} hdmiforumvsdb_t;

void hdmiforumvsdb_reset(hdmiforumvsdb_t * forumvsdb);

int hdmiforumvsdb_parse(hdmiforumvsdb_t * forumvsdb, u8 * data);
#ifdef __cplusplus
}
#endif

#endif	/* HDMIFORUMVSDB_H_ */
