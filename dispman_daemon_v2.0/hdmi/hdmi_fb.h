/*!
* TCC Version 1.0
* Copyright (c) Telechips Inc.
* All rights reserved 
*  \file        hdmi_fb.h
*  \brief       HDMI frame buffer header
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
#ifndef __HDMI_FB_H__
#define __HDMI_FB_H__

#ifdef __cplusplus
extern "C" {
#endif

int FB_Open(void);
int FB_Close(void);
int FB_GetHandle(void);
unsigned int FB_LcdcCheck(void);
int FB_video_generator_prepare(void);
int FB_video_generator_config(videoParams_t *videoParam);
int FB_video_generator_extra(videoParams_t *videoParam);
int FB_video_generator_stop(void);
int FB_video_set_r2ymd(unsigned int r2ymd);

#ifdef __cplusplus
}
#endif
#endif
