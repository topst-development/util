/*!
* TCC Version 1.0
* Copyright (c) Telechips Inc.
* All rights reserved 
*  \file        hdmi_v2.h
*  \brief       HDMI application header
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

#ifndef __HDMI_APP_H__
#define __HDMI_APP_H__

#ifdef __cplusplus
extern "C" {
#endif

enum 
{
        v1920x1080p_60Hz = 0,
        v1920x1080p_50Hz,
        v1920x1080i_60Hz,
        v1920x1080i_50Hz,
        v1280x720p_60Hz,
        v1280x720p_50Hz,
        v720x576p_50Hz,
        v720x480p_60Hz,
        v640x480p_60Hz,
        // Add HDMI V2.0	
        v1920x1080_30Hz,
        v3840x2160p_30Hz,       // no_10
        v3840x2160p_60Hz,
        v1920x1080p_24Hz,
        v3840x2160p_24Hz,
        v3840x2160p_25Hz,
        v3840x2160p_50Hz,       // no_15
        v1920x720p_60Hz,
        vMaxTableItems,
        AutoDetectMode = 125,
};

enum 
{
        HDMI_RATIO_4_3=0,
        HDMI_RATIO_16_9,
        HDMI_RATIO_MAX
};

#define EDID_SUPPORT_4_3                (1 <<  0)
#define EDID_SUPPORT_16_9               (1 <<  1)
#define EDID_SUPPORT                    (EDID_SUPPORT_4_3 | EDID_SUPPORT_16_9)


#define EDID_SUPPORT_YCC420             (1 <<  5)
#define EDID_SUPPORT_YCC420_ONLY        (1 <<  6)
#define EDID_NATIVE_VIC                 (1 <<  7)
#define EDID_HDMI_VIC                   (1 <<  8)

#define EDID_SUPPORT_3D_FB              (1 << 10)
#define EDID_SUPPORT_3D_TAB             (1 << 11)
#define EDID_SUPPORT_3D_SBS             (1 << 12)

typedef struct {
        const int idx; 
        const int vic[2];     // 0(4:3), 1(16:9)
        const int refresh_rate;
        const int pixel_clock;
        const tcc_display_size HdmiSize;
        const int hblank;
        const int vblank;
        const int interlaced;
        const int not_support_dvi;
        unsigned int edid_support_status;
}tcc_hdmi_resolution;

int hdmi_display_init(void);
int hdmi_display_deinit(void);
unsigned int hdmi_lcdc_check(void);
unsigned int hdmi_suspend_check(void);
int hdmi_display_detect_onoff(char onoff);
int hdmi_AudioInputPortDetect(void);
int hdmi_display_runtime_check(void);
int hdmi_display_output_set(char onoff, unsigned int hdmi_hw_cts_mode);
int hdmi_display_output_modeset(char enable);
int hdmi_display_cabledetect(void);
int hdmi_set_video_format_ctrl(unsigned int HdmiVideoFormat, unsigned int Structure_3D);
int hdmi_cmd_process(void);
int hdmi_sink_support_hdmi3d();

extern int hdmi_get_hdmi_mode(void);
#ifdef __cplusplus
}
#endif

#endif //(__HDMI_APP_H__)
