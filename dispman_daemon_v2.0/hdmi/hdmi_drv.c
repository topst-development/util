/*!
* TCC Version 1.0
* Copyright (c) Telechips Inc.
* All rights reserved 
*  \file        hdmi_drv.cpp
*  \brief       HDMI driver control source
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
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <utils/Log.h>
#include <utils/types.h>
#include <utils/properties.h>
#include <hdmi_v2_0/include/hdmi_ioctls.h>
#include <video/tcc/tccfb_ioctrl.h>
#include <hdmi/hdmi_drv.h>
#include <hdmi/hdmi_properties.h>
#include "libs/libdrv/libdrv.h"
#include <poll.h>

#define LOG_TAG "[HDMI_DRV  ]"
#define HDMI_DEV_NAME   "/dev/dw-hdmi-tx"
#define HDMI_APP_DEBUG  0


#if HDMI_APP_DEBUG
#define DPRINTF(args...)    ALOGI(args)
#else
#define DPRINTF(args...)
#endif

static unsigned int scdc_ready = 0;

int HDMI_HPD_Enable(void)
{
        int ret = -1;
        int hpd_enable = 1;
        int hdmi_fd = HDMI_Open();
        DPRINTF("HDMI_HPD_Enable\r\n");
        if(hdmi_fd < 0) {
                ALOGE("%s failed open hdmi fd", __func__);
                goto hdmi_fd_is_invalid;
        }
        ret =  ioctl(hdmi_fd, HDMI_HPD_SET_ENABLE, &hpd_enable);
        if(ret < 0) {
                ALOGE("ioctl(HDMI_HPD_SET_ENABLE) failed!\n");
        }
        HDMI_Close();
        
hdmi_fd_is_invalid:
        return ret;
}

int HDMI_HPD_Disable(void)
{
        int ret = -1;
        int hpd_enable = 0;
        int hdmi_fd = HDMI_Open();
        DPRINTF("HDMI_HPD_Enable\r\n");
        if(hdmi_fd < 0) {
                ALOGE("%s failed open hdmi fd", __func__);
                goto hdmi_fd_is_invalid;
        }
        ret = ioctl(hdmi_fd, HDMI_HPD_SET_ENABLE, &hpd_enable);
        if(ret < 0) {
                ALOGE("ioctl(HDMI_HPD_SET_ENABLE) failed!\n");
        }
        HDMI_Close();
        
hdmi_fd_is_invalid:

        return ret;
}

int HDMI_HPDCheck(void)
{
        int ret = -1;
        int hpd = 0;
        int hdmi_fd = HDMI_Open();
        
        if(hdmi_fd < 0) {
                ALOGE("%s failed open hdmi fd", __func__);
                goto hdmi_fd_is_invalid;
        }
        ret = ioctl(hdmi_fd, HDMI_HPD_GET_STATUS, &hpd);
        if(ret < 0) {
                ALOGE("ioctl(HDMI_HPD_GET_STATUS) failed!\n");
        }
        HDMI_Close();
        
hdmi_fd_is_invalid:
        return hpd;	
}

/**
 * Set HPD callback function.
 *
 * @param    handler    [in]    The pointer to handler.
 * @return    If success to set CallBack handler, return 1;otherwise, return 0.
 */
int HDMI_HPDSetCallback(__attribute__((unused))HPDCallback *handler)
{
    return 1;
}


int HDMI_Set_AvMute(unsigned int mute)
{
        int ret = -1;
        int hdmi_fd = HDMI_Open();
        unsigned int magic = HDMI_AV_MUTE_MAGIC;

        if(mute) {
                magic |= 1;
        }
        if(hdmi_fd < 0) {
                ALOGE("%s failed open hdmi fd", __func__);
                goto hdmi_fd_is_invalid;
        }
        ret = ioctl(hdmi_fd, HDMI_SET_AV_MUTE, &magic);
        if(ret < 0) {
                ALOGE("ioctl(HDMI_SET_AV_MUTE) failed!\n");
        }
        HDMI_Close();
hdmi_fd_is_invalid:
        return ret;     
}

int HDMI_Get_NeedPreConfig(void)
{
        int hdmi_fd = HDMI_Open();
        int need_preconfig = 0;
        if(hdmi_fd < 0) {
                ALOGE("%s failed open hdmi fd", __func__);
                goto hdmi_fd_is_invalid;
        }
        ioctl(hdmi_fd, HDMI_API_GET_NEED_PRE_CONFIG, &need_preconfig);
        HDMI_Close();
hdmi_fd_is_invalid:
        return need_preconfig;
}

int HDMI_Api_PreConfig(void)
{
        int ret = -1;
        int hdmi_fd = HDMI_Open();
        if(hdmi_fd < 0) {
                ALOGE("%s failed open hdmi fd", __func__);
                goto hdmi_fd_is_invalid;
        }
        ret = ioctl(hdmi_fd, HDMI_API_PRE_CONFIG, NULL);
        if(ret < 0) {
                ALOGE("ioctl(HDMI_API_PRE_CONFIG) failed!\n");
        }
        HDMI_Close();
hdmi_fd_is_invalid:
        return ret;
}

int HDMI_Api_Config(videoParams_t *videoParams, audioParams_t *audioParams, productParams_t *productParams, hdcpParams_t *hdcpParam)
{
        int ret = -1;
        int hdmi_fd = HDMI_Open();

        dwc_hdmi_api_data api_data;
        memcpy(&api_data.videoParam, videoParams, sizeof(videoParams_t));
        memcpy(&api_data.audioParam, audioParams, sizeof(audioParams_t));
        memcpy(&api_data.productParam, productParams, sizeof(productParams_t));
        memcpy(&api_data.hdcpParam, hdcpParam, sizeof(hdcpParams_t));     
        if(hdmi_fd < 0) {
                ALOGE("%s failed open hdmi fd", __func__);
                goto hdmi_fd_is_invalid;
        }
        ret = ioctl(hdmi_fd, HDMI_API_CONFIG, &api_data);
        if(ret < 0) {
                ALOGE("ioctl(HDMI_API_CONFIG) failed!\n");
        }
        HDMI_Close();
hdmi_fd_is_invalid:
        return ret;
}

int HDMI_Api_Disable(void)
{
        int ret = -1;
        int hdmi_fd = HDMI_Open();
        unsigned int magic = HDMI_API_DISABLE_MAGIC;
        if(hdmi_fd < 0) {
                ALOGE("%s failed open hdmi fd", __func__);
                goto hdmi_fd_is_invalid;
        }
        ret = ioctl(hdmi_fd, HDMI_API_DISABLE, &magic);
        if(ret < 0) {
                ALOGE("ioctl(HDMI_API_DISABLE) failed!\n");
        }
        HDMI_Close();
hdmi_fd_is_invalid:
        return ret;
}

int HDMI_Audio_Config(audioParams_t audioParams)
{
        int ret = -1;
        int hdmi_fd = HDMI_Open();
        if(hdmi_fd < 0) {
                ALOGE("%s failed open hdmi fd", __func__);
                goto hdmi_fd_is_invalid;
        }
        ret = ioctl(hdmi_fd, HDMI_AUDIO_CONFIG, &audioParams);
        if(ret < 0) {
                ALOGE("ioctl(HDMI_AUDIO_CONFIG) failed!\n");
        }
        HDMI_Close();
hdmi_fd_is_invalid:
        return ret;
}

static int hdmi_check_hdmi_refresh_rate(unsigned int vic)
{
        //The vertical frequencies of the 240p, 480p, and 480i Video Formats are typically adjusted by a factor of
        //exactly 1000/1001 for NTSC video compatibility,
        int refresh_ratemode = hdmi_get_refresh_rate();

        int refresh_rate = 0;

        switch(refresh_ratemode) {
                case 0:
                        // HZ / 1.001
                        switch(vic) {
                                // 23.98/24Hz
                                case 32: // 1920x1080p_24Hz
                                case 93: // 3840x2160p_24Hz
                                        refresh_rate = 23980;
                                        break;
                                        
                                // 29.97/30Hz
                                case 34: // 1920x1080_30Hz
				case 95: // 3840x2160p_30Hz
					refresh_rate = 29970;
					break;
                                        
                                // 59.94/60Hz
                                case 1: // 640x480p_60Hz - 30/36 Not Implemented
                                case 2: // 720x480p_60Hz
                                case 3: // 720x480p_60Hz
                                case 4: // 1280x720p_60Hz
                                case 5: // 1920x1080i_60Hz
                                case 6: // 1440x480i_60Hz - Not Implemented
                                case 7: // 1440x480i_60Hz - Not Implemented
                                case 10: // 2880x480i_60Hz - Not Implemented
                                case 11: // 2880x480i_60Hz - Not Implemented
                                case 14: // 1440x480p_60Hz - Not Implemented
                                case 15: // 1440x480p_60Hz - Not Implemented
                                case 16: // 1920x1080p_60Hz
                                case 35: // 2880x480p - Not Implemented
                                case 36: // 2880x480p - Not Implemented
                                case 97: // 3840x2160p_60Hz
                                        refresh_rate = 59940;
                                        break;
                                
                                // 119.88/120Hz
                                // VIC 46-51, 63

                                // 239.76/240Hz
                                // VIC 56-59
                                default:
                                        break;
                        }
                        break;
                        
                default:
                        if(refresh_ratemode == 125) {
                                // Auto Mode..!
                                switch(vic) {
                                        // 59.94/60Hz
                                        case 1: // 640x480p_60Hz - 30/36 Not Implemented
                                        case 2: // 720x480p_60Hz
                                        case 3: // 720x480p_60Hz
                                        case 6: // 1440x480i_60Hz - Not Implemented
                                        case 7: // 1440x480i_60Hz - Not Implemented
                                        case 10: // 2880x480i_60Hz - Not Implemented
                                        case 11: // 2880x480i_60Hz - Not Implemented
                                        case 14: // 1440x480p_60Hz - Not Implemented
                                        case 15: // 1440x480p_60Hz - Not Implemented
                                        case 35: // 2880x480p - Not Implemented
                                        case 36: // 2880x480p - Not Implemented
                                                refresh_rate = 59940;
                                                break;
                                }
                        }
                        break;
        }
        return refresh_rate;
}


int HDMI_dtd_fill_with_refreshrate(dtd_t *hdmi_dtd, uint32_t code, int refreshRate) {
	int ret = -1;
        int  hdmi_fd = HDMI_Open();
	
	dwc_hdmi_dtd_data dtd_data;
	dtd_data.code = code;
	dtd_data.refreshRate = refreshRate;
        if(hdmi_fd < 0) {
                ALOGE("%s failed open hdmi fd", __func__);
                goto hdmi_fd_is_invalid;
        }
	ret = ioctl(hdmi_fd, HDMI_GET_DTD_INFO, &dtd_data);
        if(ret < 0) {
                ALOGE("ioctl(HDMI_GET_DTD_INFO) failed!\n");
        } else {
		memcpy(hdmi_dtd, &dtd_data.dtd, sizeof(dtd_t));
	}
	HDMI_Close();
hdmi_fd_is_invalid:
	return ret;
}

int HDMI_dtd_fill(dtd_t *hdmi_dtd, uint32_t code) {
        int ret = HDMI_dtd_fill_with_refreshrate(hdmi_dtd, code, hdmi_check_hdmi_refresh_rate(code));
        return ret;
}




int HDMI_set_scrambling(int enable)
{
        int ret = -1;
        int  hdmi_fd = HDMI_Open();
        if(hdmi_fd < 0) {
                ALOGE("%s failed open hdmi fd", __func__);
                goto hdmi_fd_is_invalid;
        }
        ret = ioctl(hdmi_fd, HDMI_VIDEO_SET_SCRAMBLING, &enable);
        if(ret < 0) {
                ALOGE("ioctl(HDMI_VIDEO_SET_SCRAMBLING) failed!\n");
        }
        HDMI_Close();
hdmi_fd_is_invalid:
        return ret;
}

int HDMI_get_scramble_status(void)
{
        int enable = 0;
        int hdmi_fd = HDMI_Open();
        if(hdmi_fd < 0) {
                ALOGE("%s failed open hdmi fd", __func__);
                goto hdmi_fd_is_invalid;
        }
        ioctl(hdmi_fd, HDMI_VIDEO_GET_SCRAMBLE_STATUS, &enable);
        HDMI_Close();
hdmi_fd_is_invalid:
        return enable;
}

void HDMI_set_scdc_ready(unsigned int ready)
{
        scdc_ready = ready;
}

int HDMI_get_CharacterErrorDetection(void)
{
        int ret = -1;
        int hdmi_fd = HDMI_Open();
        if(scdc_ready) {
                if(hdmi_fd < 0) {
                        ALOGE("%s failed open hdmi fd", __func__);
                        goto hdmi_fd_is_invalid;
                }
                ret = ioctl(hdmi_fd, HDMI_VIDEO_GET_ERROR_DETECTION, NULL);
                if(ret < 0) {
                        ALOGE("ioctl(HDMI_VIDEO_GET_ERROR_DETECTION) failed!\n");
                }
        }
        else {
                ret = 0;
        }
hdmi_fd_is_invalid:
        HDMI_Close();
        return ret;
}

int HDMI_Get_PowerStatus(void)
{
        int hdmi_fd = HDMI_Open();
        unsigned int power_on = 0;
        if(hdmi_fd < 0) {
                ALOGE("%s failed open hdmi fd", __func__);
                goto hdmi_fd_is_invalid;
        }
        ioctl(hdmi_fd, HDMI_GET_PWR_STATUS, &power_on);
        HDMI_Close();
hdmi_fd_is_invalid:
        return power_on;
}

int HDMI_Set_PowerStatus(unsigned int power_on)
{
        int ret = -1;
        int hdmi_fd = HDMI_Open();
        if(hdmi_fd < 0) {
                ALOGE("%s failed open hdmi fd", __func__);
                goto hdmi_fd_is_invalid;
        }
        ret = ioctl(hdmi_fd, HDMI_SET_PWR_CONTROL, &power_on);
        if(ret < 0) {
                ALOGE("ioctl(HDMI_SET_PWR_CONTROL) failed!\n");
        }
        HDMI_Close();
hdmi_fd_is_invalid:
        return ret;
}

unsigned int HDMI_SuspendCheck(void)
{
        int hdmi_fd = HDMI_Open();
        unsigned int suspend_check = 0;
        if(hdmi_fd < 0) {
                ALOGE("%s failed open hdmi fd", __func__);
                goto hdmi_fd_is_invalid;
        }
        ioctl(hdmi_fd, HDMI_GET_SUSPEND_STATUS, &suspend_check);
        HDMI_Close();
hdmi_fd_is_invalid:
        return suspend_check;
}

        
int HDMI_GetRunStatus(void)
{
        int hdmi_fd = HDMI_Open();
        unsigned int output_enable = 0;
        if(hdmi_fd < 0) {
                ALOGE("%s failed open hdmi fd", __func__);
                goto hdmi_fd_is_invalid;
        }
        ioctl(hdmi_fd, HDMI_GET_OUTPUT_ENABLE_STATUS, &output_enable);
        HDMI_Close();
hdmi_fd_is_invalid:
        return output_enable;
}

/**
 * Get HDMI HPD status.
 */
int HDMI_SendHPDStatus(void)
{
        return 0;
}


int HDMI_get_scdc_sink_version(unsigned int *version)
{
        int ret = -1;
        int hdmi_fd = HDMI_Open();
        if(hdmi_fd < 0) {
                ALOGE("%s failed open hdmi fd", __func__);
                goto hdmi_fd_is_invalid;
        }
        ret = ioctl(hdmi_fd, HDMI_SCDC_GET_SINK_VERSION, version);
        if(ret < 0) {
                ALOGE("ioctl(HDMI_SCDC_GET_SINK_VERSION) failed!\n");
        }
        HDMI_Close();
hdmi_fd_is_invalid:
        return ret;
}

int HDMI_get_scdc_source_version(unsigned int *version)
{
        int ret = -1;
        int hdmi_fd = HDMI_Open();
        if(hdmi_fd < 0) {
                ALOGE("%s failed open hdmi fd", __func__);
                goto hdmi_fd_is_invalid;
        }
        ret = ioctl(hdmi_fd, HDMI_SCDC_GET_SOURCE_VERSION, version);
        if(ret < 0) {
                ALOGE("ioctl(HDMI_SCDC_GET_SOURCE_VERSION) failed!\n");
        }
        HDMI_Close();
hdmi_fd_is_invalid:
        return ret;
}

int HDMI_set_scdc_source_version(unsigned int version)
{
        int ret = -1;
        int hdmi_fd = HDMI_Open();
        if(hdmi_fd < 0) {
                ALOGE("%s failed open hdmi fd", __func__);
                goto hdmi_fd_is_invalid;
        }
        ret = ioctl(hdmi_fd, HDMI_SCDC_SET_SOURCE_VERSION, &version);
        if(ret < 0) {
                ALOGE("ioctl(HDMI_SCDC_SET_SOURCE_VERSION) failed!\n");
        }
        HDMI_Close();
hdmi_fd_is_invalid:
        return ret;
}



int HDMIDrv_poll_wait(int time_out)
{
        int ret = -1;
        int hdmi_fd = HDMI_Open();
        struct pollfd poll_events;

        if(hdmi_fd < 0) {
                ALOGE("%s failed open hdmi fd", __func__);
                usleep(time_out * 1000);
        } else {
                poll_events.fd = hdmi_fd;
                poll_events.events = POLLIN;
                poll_events.revents = 0;
                
                ret = poll( (struct pollfd *)&poll_events, 1, time_out);
                
                HDMI_Close();
        }

        return ret;
}

int HDMIDrv_set_phy_mask(unsigned int mask)
{
	int ret = -1;
        int hdmi_fd = HDMI_Open();
        if(hdmi_fd < 0) {
                ALOGE("%s failed open hdmi fd", __func__);
                goto hdmi_fd_is_invalid;
        }
	ret = ioctl(hdmi_fd, HDMI_API_PHY_MASK, &mask);
	if(ret < 0) {
                ALOGE("ioctl(HDMI_API_PHY_MASK) failed!\n");
        }
        HDMI_Close();
hdmi_fd_is_invalid:
	return ret;
}

int HDMIDrv_set_edid_machine_id(int machine_id)
{
        int ret = -1;
        int hdmi_proc; 
        char edid_machine_id_buf[255];

        hdmi_proc = open("/proc/hdmi_tx/edid_machine_id", O_RDWR);
        if(hdmi_proc < 0) {
                ALOGE("%s can not open edid_machine_id", __func__);
                goto error_open_proc;                
        }

        sprintf(edid_machine_id_buf, "%d", machine_id);
        ret = write (hdmi_proc, edid_machine_id_buf, strlen(edid_machine_id_buf));
        if(ret < 0) {
                ALOGE("%s can not write edid_machine_id", __func__);
        }
        close(hdmi_proc);
        ret = 0;
        
error_open_proc:
        return ret;
}

int HDMIDrv_get_feature(hdmi_soc_features *soc_features)
{
        int ret = -1;
        int hdmi_fd = HDMI_Open();
        if(hdmi_fd < 0) {
                ALOGE("%s failed open hdmi fd", __func__);
                goto hdmi_fd_is_invalid;
        }
        ret = ioctl(hdmi_fd, HDMI_GET_SOC_FEATURES, soc_features);
        if(ret < 0) {
                ALOGE("ioctl(HDMI_GET_SOC_FEATURES) failed!\n");
        }
        HDMI_Close();
hdmi_fd_is_invalid:
        return ret;

}

int HDMIDrv_get_phy_rx_sense_status(void)
{
	int ret = -1;
	int hdmi_fd = HDMI_Open();
	int value = -1;
	if(hdmi_fd < 0) {
    	ALOGE("%s failed open hdmi fd", __func__);
		goto hdmi_fd_is_invalid;
	}
	
	ret = ioctl(hdmi_fd, HDMI_GET_PHY_RX_SENSE_STATUS, &value);
	if(ret < 0) {
    	ALOGE("ioctl(HDMI_GET_PHY_RX_SENSE_STATUS) failed!\n");
    }
	HDMI_Close();
hdmi_fd_is_invalid:
	return value;
}
