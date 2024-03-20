/*!
* TCC Version 1.0
* Copyright (c) Telechips Inc.
* All rights reserved 
*  \file        short_video_desc.c
*  \brief       HDMI video descript source
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
#include "short_video_desc.h"

void svd_reset(shortVideoDesc_t * svd)
{
	svd->mNative = 0;
	svd->mCode = 0;
}

int svd_parse(shortVideoDesc_t * svd, u8 data)
{
	svd_reset(svd);
        svd->mCode = bit_field(data, 0, 7);
        if(svd->mCode >= 1 && svd->mCode <= 64)
	        svd->mNative = (bit_field(data, 7, 1) == 1) ? 1 : 0;
	svd->mLimitedToYcc420 = 0;
	svd->mYcc420 = 0;
        svd->mHdmiVic = 0;
	return 0;
}
