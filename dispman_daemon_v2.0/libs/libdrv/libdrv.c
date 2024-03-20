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
#include <utils/Log.h>

#include "libdrv.h"
#define HDMI_DEV_NAME      "/dev/dw-hdmi-tx"
#define LOG_TAG "[LIBDRV    ]"
/**
 * HDMI file descriptor
 */
static int hdmi_fd = -1;

/**
 * Reference count of HDMI file descriptor
 */
static unsigned int ref_cnt = 0;

int HDMI_Open (void)
{
        int ret = 1;

        // check already open??
        if (ref_cnt > 0)
        {
                ref_cnt++;
                return hdmi_fd;
        }

        hdmi_fd = open(HDMI_DEV_NAME, O_RDWR);
        
        // open
        if (hdmi_fd > 0) {
                ref_cnt++;
        }
        else {
                ALOGE("Cannot open HDMI : %s \n\n",HDMI_DEV_NAME);
        }
        
        return hdmi_fd;
}

int HDMI_Close (void)
{
        int ret = 1;
        
        // check if fd is available
        if (ref_cnt == 0)
        {
                ALOGE("I2C_DDC is not available!!!!\n\n");
                return 1;
        }

        // close
        if (ref_cnt > 1)
        {
                ref_cnt--;
                return 1;
        }

        if (close(hdmi_fd) < 0)
        {
                ALOGE("Cannot close HDMI : %s \n\n",HDMI_DEV_NAME);
                ret = 0;
        }

        ref_cnt--;
        hdmi_fd = -1;

        return ret;
}


