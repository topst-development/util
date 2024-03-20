/*!
* TCC Version 1.0
* Copyright (c) Telechips Inc.
* All rights reserved 
*  \file        hdmi_drv.h
*  \brief       HDMI driver control header
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

#ifndef __HDMI_DRV_H__
#define __HDMI_DRV_H__
        
#ifdef __cplusplus
        extern "C" {
#endif

typedef void HPDCallback(int);


int HDMI_Open (void);
int HDMI_Close (void);

int HDMI_HPD_Enable(void);
int HDMI_HPD_Disable(void);
int HDMI_HPDCheck(void);
int HDMI_HPDSetCallback(HPDCallback *handler);
int HDMI_Set_AvMute(unsigned int mute);
int HDMI_Get_NeedPreConfig(void);
int HDMI_Api_PreConfig(void);
int HDMI_Api_Config(videoParams_t *videoParams, audioParams_t *audioParams, productParams_t *productParams, hdcpParams_t *hdcpParam);
int HDMI_Api_Disable(void);
int HDMI_Audio_Config(audioParams_t audioParams);
int HDMI_dtd_fill_with_refreshrate(dtd_t *hdmi_dtd, uint32_t code, int refreshRate);
int HDMI_dtd_fill(dtd_t *hdmi_dtd, uint32_t code);

unsigned int HDMI_SuspendCheck(void);
int HDMI_Get_PowerStatus(void);
int HDMI_Set_PowerStatus(unsigned int power_on);

int HDMI_GetRunStatus(void);
int HDMI_SendHPDStatus(void);

int HDMI_set_scrambling(int enable);
int HDMI_get_scramble_status(void);
void HDMI_set_scdc_ready(unsigned int ready);
int HDMI_get_CharacterErrorDetection(void);
int HDMI_get_scdc_sink_version(unsigned int *version);
int HDMI_get_scdc_source_version(unsigned int *version);
int HDMI_set_scdc_source_version(unsigned int version);
int HDMIDrv_poll_wait(int time_out);
int HDMIDrv_set_phy_mask(unsigned int mask);
int HDMIDrv_set_edid_machine_id(int machine_id);
int HDMIDrv_get_feature(hdmi_soc_features *soc_features);
int HDMIDrv_get_phy_rx_sense_status(void);
#ifdef __cplusplus
        }
#endif
#endif

