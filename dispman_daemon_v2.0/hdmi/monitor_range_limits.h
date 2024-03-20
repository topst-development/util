/*!
* TCC Version 1.0
* Copyright (c) Telechips Inc.
* All rights reserved 
*  \file        monitor_range_limits.h
*  \brief       HDMI monitor range info header
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

#ifndef MONITORRANGELIMITS_H_
#define MONITORRANGELIMITS_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file
 * Second Monitor Descriptor
 * Parse and hold Monitor Range Limits information read from EDID
 */
typedef struct {
	u8 mMinVerticalRate;

	u8 mMaxVerticalRate;

	u8 mMinHorizontalRate;

	u8 mMaxHorizontalRate;

	u8 mMaxPixelClock;

	int mValid;
} monitorRangeLimits_t;

void monitor_range_limits_reset(monitorRangeLimits_t * mrl);
#ifdef __cplusplus
}
#endif

#endif	/* MONITORRANGELIMITS_H_ */
