/*!
* TCC Version 1.0
* Copyright (c) Telechips Inc.
* All rights reserved 
*  \file        libedid.c
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

#include "../libddc/libddc.h"
#include "libedid.h"

#define LOG_TAG "[LIBEDID   ]"

#define LIBDDC_DEBUG 0
#if LIBDDC_DEBUG
#define DPRINTF(args...)    ALOGD(args)
#else
#define DPRINTF(args...)
#endif



#define EDID_I2C_ADDR           0x50
#define EDID_I2C_SEGMENT_ADDR   0x30

#define EDID_LENGTH 128

static int _edid_checksum(u8 * edid)
{
        int i, checksum = 0;

        for(i = 0; i < EDID_LENGTH; i++)
                checksum += edid[i];

        return checksum % 256; //CEA-861 Spec
}

int HDMI_edid_read( void * edid)
{
        int ret;
        char* edid_pointer = (char*)edid;
        const u8 header[] = {0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00};
        ret = HDMI_i2cddc_ddc_read(EDID_I2C_ADDR, EDID_I2C_SEGMENT_ADDR, 0, 0, 8, (u8 *)edid_pointer);
        if(ret < 0){
                ALOGE("[%s] EDID read header failed", __func__);
                goto end_process;
        }
        ret = memcmp((u8 * ) edid_pointer, (u8 *) header, sizeof(header));
        if(ret){
                ALOGE("[%s] EDID header check failed", __func__);
                ret = -1;
                goto end_process;
        }

        ret = HDMI_i2cddc_ddc_read(EDID_I2C_ADDR, EDID_I2C_SEGMENT_ADDR, 0, 8, 120, (u8 *)(edid_pointer+8));
        if(ret < 0){
                ALOGE("[%s] EDID read failed", __func__);
                goto end_process;
        }

        ret = _edid_checksum((u8 *) edid_pointer);
        if(ret){
                ALOGE("[%s] EDID checksum failed", __func__);
                ret = -1;
                goto end_process;
        }
        ret = 0;
        
end_process:
	if(ret < 0)
		HDMI_i2cddc_ddc_busclear();

        return ret;
}

int HDMI_edid_extension_read( int block, u8 * edid_ext)
{
        int ret = -1;
        /*to incorporate extensions we have to include the following - see VESA E-DDC spec. P 11 */
        u8 start_pointer = block / 2; // pointer to segments of 256 bytes
        u8 start_address = ((block % 2) * 0x80); //offset in segment; first block 0-127; second 128-255
                
        ret = HDMI_i2cddc_ddc_read(EDID_I2C_ADDR, EDID_I2C_SEGMENT_ADDR, start_pointer, start_address, 128, edid_ext);

        if(ret < 0){
                ALOGE("[%s] EDID extension read failed", __func__);
                goto end_process;
        }

        ret = _edid_checksum(edid_ext);
        if(ret){
                ALOGE("[%s] EDID extension checksum failed", __func__);
                ret = -1;
                goto end_process;
        }
end_process:
        return ret;
}



#define EDID_EXTENSION_NUMBER_POS (0x7E)

int API_EDID_Read(unsigned char *ptr_user_edid)
{
        unsigned char edid[EDID_LENGTH];

        
        int extensions = 0;
        int edid_tries = 3;
        int edid_ok;
        int edid_totalsize = 0;
        
        int blkid, blkpos, blknum;
        
        unsigned char *edid_extensions = NULL;
        
        do {
                edid_ok = HDMI_edid_read(edid);
                
                if(!edid_ok) //success!!!
                        break;
                
        }while(edid_tries--);

         if(edid_tries <= 0){
                ALOGE("[%s] Could not read EDID", __func__);
                goto end_process;
         }
         
         // get extension
        extensions = edid[EDID_EXTENSION_NUMBER_POS];
        DPRINTF("EDID Extension blocks %d...\n", extensions);

        if(extensions>3)
        {
                DPRINTF("EDID Extension blocks can't be greater than 3...\n");
                extensions = 3;
        }

        edid_totalsize = (extensions + 1)*EDID_LENGTH;

        // prepare buffer
        edid_extensions = (unsigned char*)malloc(edid_totalsize);
        
        if (edid_extensions == NULL) {
                edid_totalsize = 0;
                goto end_process;
        }

        // copy EDID Block 0
        memcpy(edid_extensions, edid, EDID_LENGTH);

        for ( blkid = 1, blkpos = EDID_LENGTH; blkid <= extensions; blkid++ ,blkpos+=EDID_LENGTH ) {
                // read extension 1 ~ rest of extension
                if (HDMI_edid_extension_read(blkid, edid_extensions+blkpos) < 0)
                {
                        edid_totalsize = 0;
                        goto end_process;
                }
        }

        if(ptr_user_edid) {
                memcpy(ptr_user_edid, edid_extensions, edid_totalsize);
        }
end_process:
        if(edid_extensions)
                free(edid_extensions);
        
        return edid_totalsize;
}

