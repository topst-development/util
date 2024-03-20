/*!
* TCC Version 1.0
* Copyright (c) Telechips Inc.
* All rights reserved 
*  \file        hdmi_lut.h
*  \brief       HDMI lut controler header
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

#ifndef __LIB_LUT_H__
#define __LIB_LUT_H__

#ifdef __cplusplus
extern "C" {
#endif

int Lut_enable_bt2020(unsigned int lut_enable) ;

int lut_enable_csc_sdr_to_hdr10(int is_video_ch);
int lut_enable_csc_sdr_to_hlg(int is_video_ch);
int lut_enable_csc_hdr10_to_sdr(int is_video_ch); 
int lut_enable_csc_hlg_to_sdr(int is_video_ch);
int lut_disable_csc(int is_video_ch);
#ifdef __cplusplus
}
#endif

#endif /* __LIB_LUT_H__ */
