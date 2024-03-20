/*!
* TCC Version 1.0
* Copyright (c) Telechips Inc.
* All rights reserved 
*  \file        hdmi_hdr.h
*  \brief       HDMI hdr controler source
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

#include <utils/Log.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <utils/types.h>
#include <utils/bit_operation.h>
#include <utils/Log.h>
#include <utils/properties.h>
#include <libs/libdrv/libdrv.h>
#include <hdmi/hdmi_edid.h>
#include <hdmi/hdmi_hdr.h>
#include <hdmi/hdmi_lut.h>
#include <hdmi_v2_0/include/hdmi_ioctls.h>
#include <video/tcc/tccfb_ioctrl.h>

#define LOG_TAG         "[LIBHDR    ]"

#define LOG_EXTRA_MODE 0

#if LOG_EXTRA_MODE
static int prev_hdmi_extra_mode = -1;
#endif

/**
 * @short Stores drm status of currently playing video contents.
 * 0: The content is SDR.
 * 2: The content is HDR-10.
 * 3: The content is HLG.
 */
static int hdmi_drm_valid;

static int prev_hdmi_drm_valid = -1;

/**
 * @short drm enable is requested.
 * This variable is valid when not in drm seamless mode. 
 */
static int hdmi_drm_need_enable = 0;

/**
 * @short drm enable status.
 */
static int hdmi_drm_enable = 0;
static int hdmi_drm_seamless_mode = 0;
static int (*prev_lut_control_api)(int) = NULL;



static int HDMI_DRM_Get_Valid(void)
{
        int ret = -1;
        int valid = 0;
        int hdmi_fd = HDMI_Open();
        if(hdmi_fd < 0) {
                ALOGE("%s failed open hdmi fd", __func__);
                goto hdmi_fd_is_invalid;
        }
        ret = ioctl(hdmi_fd, HDMI_API_DRM_GET_VALID, &valid);
        if(ret < 0) {
                ALOGE("ioctl(HDMI_SET_PWR_CONTROL) failed!\n");
        }
        HDMI_Close();
hdmi_fd_is_invalid:
        return valid;
}


static int DRM_Set_status(int enable)
{
        int ret = -1;
        int hdmi_fd = HDMI_Open();
        if(hdmi_fd < 0) {
                ALOGE("%s failed open hdmi fd", __func__);
                goto hdmi_fd_is_invalid;
        }
        ALOGI("%s %s", __func__, enable?"Enable":"Disable");
        if(enable) {                        
                ret = ioctl(hdmi_fd, HDMI_API_DRM_SET, NULL);
        } else {
                ret = ioctl(hdmi_fd, HDMI_API_DRM_SET, NULL);
        }
        hdmi_drm_enable = enable;
        if(ret < 0) {
                ALOGE("ioctl(HDMI_SET_PWR_CONTROL) failed!\n");
        }
        HDMI_Close();
hdmi_fd_is_invalid:
        return ret;
}


int HDMI_DRM_Process_Lut( )
{
        int is_video_ch;
        
        int (*lut_control_api)(int) = NULL;

        if(hdmi_drm_enable) {
                is_video_ch = 0;
                switch(hdmi_drm_valid) {
                        case 2:
                                // UI SDR->HDR10
                                lut_control_api = lut_enable_csc_sdr_to_hdr10;
                                break;
                        case 3:
                                // UI SDR->HLG
                                lut_control_api = lut_enable_csc_sdr_to_hlg;
                                break;
                }
        } else  {
                is_video_ch = 1;
                switch(hdmi_drm_valid) {
                        case 0:
                                // BYPASS
                                lut_control_api = lut_disable_csc;
                                break;
                        case 2:
                                // VIDEO HDR10-SDR
                                lut_control_api = lut_enable_csc_hdr10_to_sdr;
                                break;
                        case 3:
                                // VIDEO HLG-SDR
                                lut_control_api = lut_enable_csc_hlg_to_sdr;
                                break;
                }
        }

        if(lut_control_api != NULL && prev_lut_control_api != lut_control_api) {
                ALOGI("%s video contents is %s and hdmi output %s mode\r\n", __func__, !hdmi_drm_valid?"sdr":(hdmi_drm_valid==2)?"hdr-10":"hlg", hdmi_drm_enable?"hdr-10/hlg":"sdr");
                lut_control_api(is_video_ch);
                prev_lut_control_api = lut_control_api;
        }

        return 0;        
}
        

int HDMI_DRM_Set_enable()
{
        int ret = 0;

        ALOGI("%s hdmi_drm_need_enable(%d)", __func__, hdmi_drm_need_enable);
        // After confirm that drm enable is required, enable drm if it required.
        if(hdmi_drm_need_enable) {
                ret = DRM_Set_status(1);
        }

        HDMI_DRM_Process_Lut();
        
        return ret;
}


int HDMI_DRM_Set_Videoparams(videoParams_t *videoParams, sink_edid_t *sink_cap)
{
        int prop_len, hdmi_extra_mode = 0;
        
        char value[PROPERTY_VALUE_MAX];

        // Initialize hdmi_drm_need_enable.
        hdmi_drm_need_enable = 0;
        prev_hdmi_drm_valid = 0;

        prop_len = property_get("persist.sys.hdmi_extra_mode", value, "0");
        if(prop_len > 0) {
                hdmi_extra_mode = atoi(value);
        }

        // The drm setting is also disabled because hdmi is reset
        // before this function is called.
        hdmi_drm_enable = 0;

        hdmi_drm_valid = HDMI_DRM_Get_Valid();
        /*
         * The DVI mode does not supports HDR mode.
         * Because It does not supports YCbCr color space and it does not transfer any packets. 
         * So Daemon ignores drm information of driver in dvi mode.
         */
        if(hdmi_drm_valid && videoParams->mHdmi == DVI) {
                hdmi_drm_valid = 0;
                ALOGI("%s drm information is ignored..", __func__);
        } 
        if(hdmi_extra_mode & 0x3) {
                if((sink_cap && sink_cap->edid_done && supports_eotf_st_2084(&sink_cap->edid_mHdrstaticmetaDataBlock) && ((hdmi_extra_mode & 0x1) && (hdmi_drm_valid == 0x2))) 
                    || (sink_cap && sink_cap->edid_done && supports_eotf_hlg(&sink_cap->edid_mHdrstaticmetaDataBlock) && ((hdmi_extra_mode & 0x2) && (hdmi_drm_valid == 0x3)))) {
                        // is resolution 2160p 
                        // is resolution is 2160p @60?
                        // does TV supports YcbCr420 10-bit  ?
                        // videoParams.mHdrEnable = 1;
                        // need reboot. 
                        if(videoParams->mDtd.mCode == 96 || videoParams->mDtd.mCode == 97) {
                                videoParams->mEncodingIn = videoParams->mEncodingOut = YCC420;
                        }
                        else {
                                videoParams->mEncodingIn = videoParams->mEncodingOut = YCC422;			
                        }
                        videoParams->mColorResolution = COLOR_DEPTH_10;
                        #if defined(CONFIG_HDMI_DOLBYVISION_SUPPORT)
                                /**
                                 * In case of HDR10, 
                                 * HDR10 output with VIOC-path will be used whenever HDR10 content is played. **/
                                #if DOLBYVISION_SUPPORT_HDR10
                                /** If use path of Dolby vision, keep Color space & depth. **/
                                if((videoParams->mDolbyVision & 0x7) > 0) {
                                        videoParams->mEncodingIn = videoParams->mEncodingOut = YCC422;
                                        videoParams->mColorResolution = COLOR_DEPTH_12;                                                         
                                }
                                #else
                                videoParams->mDolbyVision = 0;                               
                                #endif
                        #endif

                        // drm must be enabled
                        hdmi_drm_need_enable = 1;
                        prev_hdmi_drm_valid = hdmi_drm_valid;
                        ALOGI("%s The content being played is %s", __func__, (hdmi_drm_valid ==2)?"HDR-10":"HLG");
                }
        }
        
        return 0;
}


int HDMI_DRM_runtime_check(videoParams_t *videoParams, sink_edid_t *sink_cap)
{
        int ret = 0;
        int prop_len, hdmi_extra_mode = 0;
        char value[PROPERTY_VALUE_MAX];

        if(sink_cap == NULL || sink_cap->edid_done == 0) {
                //ALOGI("HDMI SKIP HDR\r\n");
                return 0;
        }
        
        prop_len = property_get("persist.sys.hdmi_extra_mode", value, "0");
        if(prop_len > 0) {
                hdmi_extra_mode = atoi(value);
        }

        #if LOG_EXTRA_MODE
        if(prev_hdmi_extra_mode != hdmi_extra_mode) {
                prev_hdmi_extra_mode = hdmi_extra_mode;
                ALOGI("%s hdmi_extra_mode=%d\r\n", __func__, hdmi_extra_mode);
        }
        #endif
        
        if(hdmi_extra_mode & HDR_EXTRA_MODE_SEAMLESS) {
               hdmi_drm_seamless_mode = 1; 
        }else {
                hdmi_drm_seamless_mode = 0;
        }

        hdmi_drm_valid = HDMI_DRM_Get_Valid();
        /*
         * The DVI mode does not supports HDR mode.
         * Because It does not supports YCbCr color space and it does not transfer any packets. 
         * So Daemon ignores drm information of driver in dvi mode.
         */
        if(hdmi_drm_valid && videoParams->mHdmi == DVI) {
                hdmi_drm_valid = 0;
        } 

        if(hdmi_drm_seamless_mode) {
                if(hdmi_extra_mode & 3) {
                        if(hdmi_drm_valid) {
                                if(!hdmi_drm_enable || (hdmi_drm_enable && prev_hdmi_drm_valid != hdmi_drm_valid)) {
                                        if(videoParams->mColorResolution > COLOR_DEPTH_8) {
                                                if(supports_eotf_st_2084(&sink_cap->edid_mHdrstaticmetaDataBlock) && ((hdmi_extra_mode & 0x1) && (hdmi_drm_valid == 0x2))) {
                                                        DRM_Set_status(1);
                                                }
                                                else if(supports_eotf_hlg(&sink_cap->edid_mHdrstaticmetaDataBlock) && ((hdmi_extra_mode & 0x2) && (hdmi_drm_valid == 0x3))) {
                                                        DRM_Set_status(1);
                                                }
                                        } else {        
                                                ALOGI("HDMI HDR seamless mode but HDMI depth is not valid - SKIP HDR");                                                
                                        }
                                }
                	}else if(hdmi_drm_enable && !hdmi_drm_valid) {
                                ALOGI("HDMI Drm is not valid");
                                DRM_Set_status(0);
                	}
                }else if(hdmi_drm_enable) {
                       	DRM_Set_status(0);
                }
                
                prev_hdmi_drm_valid = hdmi_drm_valid;
                HDMI_DRM_Process_Lut();
        } else {
                if(hdmi_extra_mode & 0x3) {
                        if(hdmi_drm_valid != prev_hdmi_drm_valid) {
                                if(supports_eotf_st_2084(&sink_cap->edid_mHdrstaticmetaDataBlock) && ((hdmi_extra_mode & 0x1) && (hdmi_drm_valid == 0x2))) {
                                        ALOGI("HDMI re-initialize request for HDR-10");
                                        ret = -1;
                                }else if(supports_eotf_hlg(&sink_cap->edid_mHdrstaticmetaDataBlock) && ((hdmi_extra_mode & 0x2) && (hdmi_drm_valid == 0x3))) {
                                        ALOGI("HDMI re-initialize request for HLG");
                                        ret = -1;
                                }else if(hdmi_drm_enable && prev_hdmi_drm_valid) {
                                        ALOGI("HDMI restore to original state");
                                        ret = -1;
                                }
                        }
                }else if(hdmi_drm_enable && prev_hdmi_drm_valid) {
        	        ALOGI("HDMI extra mode is clear");
                       	ret = -1;
                }
        }
        
        return ret;
}
