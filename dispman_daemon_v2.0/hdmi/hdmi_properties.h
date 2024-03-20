/*!
* TCC Version 1.0
* Copyright (c) Telechips Inc.
* All rights reserved 
*  \file        hdmi_properties.h
*  \brief       HDMI properties header
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

#ifndef __HDMI_PROPERTIES_H__
#define __HDMI_PROPERTIES_H__

#ifdef __cplusplus
extern "C" {
#endif


int hdmi_supportmodeset(const tcc_display_size *dispay_size);
int hdmi_set_DVILUT(int enable);
int HDMI_GetVideoResolution(unsigned int stbmode);
int hdmi_get_native_first(void);
unsigned int hdmi_get_spdif_setting(void);
unsigned int hdmi_get_audio_type(void);
unsigned int hdmi_get_hdmi_link(void);
void hdmi_set_output_detected(unsigned int detected);
void hdmi_set_hdmi_resolution(unsigned int resolution);
void hdmi_set_detected_resolution(unsigned int resolution);
void hdmi_set_detected_mode(unsigned int mode);
color_depth_t hdmi_get_ColorDepth(void);
encoding_t hdmi_get_ColorSpace(void);
int hdmi_set_ColorSpace(int color_space);
int hdmi_get_Colorimetry(void);
int hdmi_get_HDCPEnableStatus(void);
int hdmi_get_PixelAspectRatio(void);
int hdmi_set_PixelAspectRatiobyVIC(int vic, int idx_max);
unsigned int hdmi_get_AudioSamplingRate(void);
unsigned int hdmi_get_AudioChannels(void);
packet_t hdmi_get_AudioOutPacket(void);
int hdmi_AudioOnOffChk(void);
interfaceType_t hdmi_get_AudioInputPort(void);
int hdmi_get_refresh_rate(void);

int hdmi_get_HDCPEnabled(void);
#ifdef __cplusplus
}
#endif

#endif


