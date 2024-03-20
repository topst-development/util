/*!
* TCC Version 1.0
* Copyright (c) Telechips Inc.
* All rights reserved 
*  \file        libddc.c
*  \brief       HDMI TX controller i2c
*  \details   
*  \version     1.0
*  \date        2014-2015
*  \copyright
This source code contains confidential information of Telechips.
Any unauthorized use without a written  permission  of Telechips including not 
limited to re-distribution in source  or binary  form  is strictly prohibited.
This source  code is  provided "AS IS"and nothing contained in this source 
code  shall  constitute any express  or implied warranty of any kind, including
without limitation, any warranty of merchantability, fitness for a   particular 
purpose or non-infringement  of  any  patent,  copyright  or  other third party 
intellectual property right. No warranty is made, express or implied, regarding 
the information's accuracy, completeness, or performance. 
In no event shall Telechips be liable for any claim, damages or other liability 
arising from, out of or in connection with this source  code or the  use in the 
source code. 
This source code is provided subject  to the  terms of a Mutual  Non-Disclosure 
Agreement between Telechips and Company.
*******************************************************************************/


#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include <utils/types.h>
#include <utils/Log.h>
#include <hdmi_v2_0/include/hdmi_ioctls.h>

#include <libdrv/libdrv.h>
#include "libddc.h"

#define LOG_TAG "[LIBDDC   ]"

#define LIBDDC_DEBUG 0
#if LIBDDC_DEBUG
#define DPRINTF(args...)    ALOGD(args)
#else
#define DPRINTF(args...)
#endif

int HDMI_i2cddc_clk_config(void)
{
	// this api is deprecated
	return 0;
}

int HDMI_i2cddc_ddc_write(u8 i2cAddr, u8 addr, u8 len, u8 * data){
        int ret = -1;
        int hdmi_fd = HDMI_Open();
        dwc_hdmi_ddc_transfer_data transfer_data;

        transfer_data.i2cAddr =i2cAddr ;
        transfer_data.addr =addr ;
        transfer_data.len =len ;
        transfer_data.data =data ;
        if(hdmi_fd < 0)
                goto end_process;

        HDMI_i2cddc_clk_config();
        
        ret = ioctl(hdmi_fd, HDMI_DDC_WRITE_DATA, &transfer_data);
        if(ret < 0){
                ALOGE("%s:IOCTL error [%d]\n", __func__, ret);
        }

end_process:
        HDMI_Close();

        return ret;
}

int HDMI_i2cddc_ddc_read(u8 i2cAddr, u8 segment, u8 pointer, u8 addr, u8 len, u8 * data){
        int ret = -1;
        int hdmi_fd = HDMI_Open();

        dwc_hdmi_ddc_transfer_data transfer_data;
        transfer_data.i2cAddr =i2cAddr ;
        transfer_data.segment =segment ;
        transfer_data.pointer =pointer ;
        transfer_data.addr =addr ;
        transfer_data.len =len ;
        transfer_data.data =data ;

        if(hdmi_fd < 0)
                goto end_process;

        HDMI_i2cddc_clk_config();
        
        ret = ioctl(hdmi_fd, HDMI_DDC_READ_DATA, &transfer_data);
        if(ret < 0){
                ALOGE("%s:IOCTL error [%d]\n", __func__, ret);
        }

end_process:
        HDMI_Close();

        return ret;
}

int HDMI_i2cddc_ddc_busclear(void){

	int ret = -1;
	int hdmi_fd = HDMI_Open();

	if(hdmi_fd < 0)
		goto end_process;
	#if defined(HDMI_DDC_BUS_CLEAR)

	ret = ioctl(hdmi_fd, HDMI_DDC_BUS_CLEAR, NULL);
	if(ret < 0){
		ALOGE("%s:IOCTL error [%d]\n", __func__, ret);
	}
	#endif 
end_process:
	HDMI_Close();

	return ret;
}


