/*!
* TCC Version 1.0
* Copyright (c) Telechips Inc.
* All rights reserved 
*  \file        hdmi_lut.h
*  \brief       HDMI lut controler source
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
#include <hdmi_v2_0/include/hdmi_ioctls.h>
#include <video/tcc/tccfb_ioctrl.h>
#include <video/tcc/tcc_lut_ioctl.h>
/* Fix Compile error on vioc_lut.h*/
#define __iomem
#include <video/tcc/vioc_lut.h>

#define LOG_TAG         "[LIBLUT    ]"

#define LUT_DEV_NAME      "/dev/tcc_lut"

#define LIBLUT_DEBUG 0
#if LIBLUT_DEBUG
#define DPRINTF(args...)    ALOGD(args)
#else
#define DPRINTF(args...)
#endif

static int lut_support_preset_mode = 0;

int Lut_enable_bt2020(unsigned int lut_enable) {   
        #if 1
        return 0;
        #else
        int ret = -1;
        int lut_fd;
        
        struct VIOC_LUT_ONOFF_SET lut_cmd = {
                .lut_onoff = lut_enable,
                .lut_number = LUT_COMP0
        };

        if(!lut_support_preset_mode)  {
                lut_fd = open(LUT_DEV_NAME, O_RDWR);
                if (lut_fd < 0)    {
                        ALOGI("LUT device File is not available\r\n");
                        goto end_process;
                }
                ALOGI("FB_video_enable_lut_for_bt2020 %d\r\n", lut_enable);
                ret = ioctl(lut_fd, TCC_LUT_ONOFF, &lut_cmd);
                if (ret) {
                        ALOGE( "Failed TCC_LUT_ONOFF IOCTL [%d]\n", ret);
                }
                close(lut_fd);
        }

end_process:

        return ret;
        #endif
}

static int lut_csc_preset_api(unsigned int preset_id, int is_video_ch, int enable)
{
        #if 1
        return 0;
        #else
        int ret = -1;
        int lut_fd = -1;
                
        struct VIOC_LUT_CSC_PRESET_SET lut_csc_preset_cmd = {
                .lut_number = LUT_COMP0,
        };

        struct VIOC_LUT_PLUG_IN_SET lut_plugin_cmd = {
                .lut_number = LUT_COMP0,
        };
        
        lut_support_preset_mode = property_get_int("persist.sys.hdmi_support_preset_mode", 0);
        
        if(lut_support_preset_mode)  {
                lut_csc_preset_cmd.enable = enable;
                lut_plugin_cmd.enable = enable;
                
                if(is_video_ch) {
                        lut_plugin_cmd.lut_plug_in_ch = VIOC_LUT_RDMA_03;
                } else {
                        lut_plugin_cmd.lut_plug_in_ch = VIOC_LUT_RDMA_00;
                }

                lut_csc_preset_cmd.preset_id = preset_id;
                
                lut_fd = open(LUT_DEV_NAME, O_RDWR);
                if (lut_fd < 0)    {
                        ALOGE("LUT device File is not available\r\n");
                        goto cannot_open_lut_fd;
                }
                ALOGI("%s TCC_LUT_CSC_PRESET_SET\r\n", __func__);
                ret = ioctl(lut_fd, TCC_LUT_CSC_PRESET_SET, &lut_csc_preset_cmd);
                if (ret) {
                        ALOGE( "Failed TCC_LUT_CSC_PRESET_SET IOCTL [%d]\n", ret);
                        goto failed_lut_preset;
                }
                ALOGI("%s TCC_LUT_PLUG_IN\r\n", __func__);
                ret = ioctl(lut_fd, TCC_LUT_PLUG_IN, &lut_plugin_cmd);
                if (ret) {
                        ALOGE( "Failed TCC_LUT_PLUG_IN IOCTL [%d]\n", ret);
                }
        }
        
failed_lut_preset:
        if(lut_fd > 0) {
                close(lut_fd);
        }
        
cannot_open_lut_fd:
        ALOGI("%s Finish\r\n", __func__);
        
        return ret;
        #endif
}

int lut_enable_csc_sdr_to_hdr10(int is_video_ch) 
{
        //ALOGI("%s for %s", __func__, is_video_ch?"Video":"UI");
        return lut_csc_preset_api(LUT_CSC_PRESET_SDR_TO_HDR10, is_video_ch, 1);
}

int lut_enable_csc_sdr_to_hlg(int is_video_ch) 
{
        //ALOGI("%s for %s", __func__, is_video_ch?"Video":"UI");
        return lut_csc_preset_api(LUT_CSC_PRESET_SDR_TO_HLG, is_video_ch, 1);
}

int lut_enable_csc_hdr10_to_sdr(int is_video_ch) 
{
        //ALOGI("%s for %s", __func__, is_video_ch?"Video":"UI");
        return lut_csc_preset_api(LUT_CSC_PRESET_HDR10_TO_SDR, is_video_ch, 1);

}

int lut_enable_csc_hlg_to_sdr(int is_video_ch) 
{
        //ALOGI("%s for %s", __func__, is_video_ch?"Video":"UI");
        return lut_csc_preset_api(LUT_CSC_PRESET_HLG_TO_SDR, is_video_ch, 1);

}

int lut_disable_csc(__attribute__((unused))int is_video_ch)
{
        //ALOGI("%s for %s", __func__, is_video_ch?"Video":"UI");
        return lut_csc_preset_api(0, 0, 0);
}

