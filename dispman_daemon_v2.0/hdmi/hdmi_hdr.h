/*!
* TCC Version 1.0
* Copyright (c) Telechips Inc.
* All rights reserved 
*  \file        hdmi_hdr.h
*  \brief       HDMI hdr controler header
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
#ifndef __LIG_HDR_H__
#define __LIG_HDR_H__

#ifdef __cplusplus
extern "C" {
#endif

#define HDR_EXTRA_MODE_HDR      1
#define HDR_EXTRA_MODE_HLG      2
#define HDR_EXTRA_MODE_SEAMLESS 8

int HDMI_DRM_Set_enable();
int HDMI_DRM_Set_Videoparams(videoParams_t *videoParams, sink_edid_t *sink_cap);
int HDMI_DRM_runtime_check(videoParams_t *videoParams, sink_edid_t *sink_cap);

#ifdef __cplusplus
}
#endif

#endif /* __LIG_HDR_H__ */
