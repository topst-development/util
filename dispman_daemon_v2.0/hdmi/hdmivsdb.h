/*!
* TCC Version 1.0
* Copyright (c) Telechips Inc.
* All rights reserved 
*  \file        hdmivsdb.c
*  \brief       HDMI vendor v1.4 header
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

#ifndef HDMIVSDB_H_
#define HDMIVSDB_H_


#define MAX_HDMI_VIC		16
#define MAX_HDMI_3DSTRUCT	16
#define MAX_VIC_WITH_3D		16

#ifdef __cplusplus
extern "C" {
#endif


/** For detailed handling of this structure, refer to documentation of the functions */
typedef struct {
	u16 mPhysicalAddress;

	int mSupportsAi;

	int mDeepColor30;

	int mDeepColor36;

	int mDeepColor48;

	int mDeepColorY444;

	int mDviDual;

	u16 mMaxTmdsClk;

	u16 mVideoLatency;

	u16 mAudioLatency;

	u16 mInterlacedVideoLatency;

	u16 mInterlacedAudioLatency;

	u32 mId;

	u8 mContentTypeSupport;

	u8 mImageSize;

	int mHdmiVicCount;

	u8 mHdmiVic[MAX_HDMI_VIC];

	int m3dPresent;

	int mVideo3dStruct[MAX_VIC_WITH_3D][MAX_HDMI_3DSTRUCT];	/* row index is the VIC number */

	int mDetail3d[MAX_VIC_WITH_3D][MAX_HDMI_3DSTRUCT];	/* index is the VIC number */

	int mValid;

} hdmivsdb_t;

void hdmivsdb_reset( hdmivsdb_t * vsdb);

/**
 * Parse an array of data to fill the hdmivsdb_t data strucutre
 * @param *vsdb pointer to the structure to be filled
 * @param *data pointer to the 8-bit data type array to be parsed
 * @return Success, or error code:
 * @return 1 - array pointer invalid
 * @return 2 - Invalid datablock tag
 * @return 3 - Invalid minimum length
 * @return 4 - HDMI IEEE registration identifier not valid
 * @return 5 - Invalid length - latencies are not valid
 * @return 6 - Invalid length - Interlaced latencies are not valid
 */
int hdmivsdb_parse( hdmivsdb_t * vsdb, u8 * data);

u16 get_index_supported_3dstructs( hdmivsdb_t * vsdb, u8 index);

u16 get_3dstruct_indexes( hdmivsdb_t * vsdb, u8 struct3d);

#ifdef __cplusplus
}
#endif


#endif	/* HDMIVSDB_H_ */


