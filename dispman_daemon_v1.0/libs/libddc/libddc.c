/****************************************************************************
Copyright (C) 2013 Telechips Inc.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
****************************************************************************/

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

#include "libddc.h"

#include <utils/Log.h>
#define LOG_TAG "LIBDDC"

#define LIBDDC_DEBUG 0
#if LIBDDC_DEBUG
#define DPRINTF(args...)    ALOGD(args)
#else
#define DPRINTF(args...)
#endif

/**
 * @brief DDC device name.
 * User should change this.
 */
#define DDC_DEV_NAME	"/dev/i2c-ddc"
/**
 * DDC file descriptor
 */
static int ddc_fd = -1;

/**
 * Reference count of DDC file descriptor
 */
static unsigned int ref_cnt = 0;

/**
 * Check if DDC file is already opened or not
 * @return  If DDC file is already opened, return 1;Otherwise, return 0.
 */
static int DDCFileAvailable()
{
        return (ddc_fd < 0) ? 0 : 1;
}

/**
 * Initialze DDC library. Open DDC device
 * @return  If succeed in opening DDC device or it is already opened, return 1;@n
 *         Otherwise, return 0
 */
int DDCOpen()
{
        int ret = 0;

        do {
                /* check already open?? */
                if(ddc_fd != -1) {
                        ref_cnt++;
                        ret = 1;
                        break;
                }

                ddc_fd = open(DDC_DEV_NAME, O_RDWR);
                if(ddc_fd < 0) {
                        ALOGE("Cannot open I2C_DDC : %s \n\n",DDC_DEV_NAME);
                        break;
                }
                ref_cnt = 1;
                ret = 1;
        } while(0);
        return ret;
}

/**
 * Finalize DDC library. Close DDC device
 * @return  If succeed in closing DDC device or it is being used yet, return 1;@n
 *          Otherwise, return 0
 */
int DDCClose()
{
        int ret = 0;
        do {
                if(ref_cnt == 0) {
                        ALOGE("I2C_DDC is not available!!!!\n\n");
                        ret = 1;
                        break;
                }

                if(ref_cnt > 1) {
                        ref_cnt--;
                        ret = 1;
                        break;
                }
                if(close(ddc_fd) < 0) {
                        ALOGE("Cannot close I2C_DDC : %s \n\n", DDC_DEV_NAME);
                        break;
                }
                ret = 1;
                ddc_fd = -1;
                ref_cnt = 0;
        } while(0);

        return ret;
}

/**
 * Read data though DDC. For more information of DDC, refer DDC Spec.
 * @param   addr    [in]    Device address
 * @param   offset  [in]    Byte offset
 * @param   size    [in]    Sizes of data
 * @param   buffer  [out]   Pointer to buffer to store data
 * @return  If succeed in reading, return 1;Otherwise, return 0
 */
int DDCRead(unsigned char addr, unsigned char offset,
                unsigned int size, unsigned char* buffer)
{
        int ret = 0;
        struct i2c_msg msgs[2];
        struct i2c_rdwr_ioctl_data msgset;

        do {
                if (!DDCFileAvailable()) {
                        ALOGE("I2C_DDC is not available!!!!\n\n");
                        break;
                }

                // set offset
                msgs[0].addr = addr>>1;
                msgs[0].flags = 0;
                msgs[0].len = 1;
                msgs[0].buf = &offset;

                // read data
                msgs[1].addr = addr>>1;
                msgs[1].flags = I2C_M_RD;
                msgs[1].len = size;
                msgs[1].buf = buffer;

                // set rdwr ioctl data
                msgset.nmsgs = 2;
                msgset.msgs = msgs;

                if(ioctl(ddc_fd, I2C_RDWR, &msgset) < 0) {
                        ALOGE("ddc error:");
                        break;
                }
                ret = 1;
        } while(0);
        return ret;
}

/**
 * Read data though E-DDC. For more information of E-DDC, refer E-DDC Spec.
 * @param   segpointer  [in]    Segment pointer
 * @param   segment     [in]    Segment number
 * @param   addr        [in]    Device address
 * @param   offset      [in]    Byte offset
 * @param   size        [in]    Sizes of data
 * @param   buffer      [out]   Pointer to buffer to store data
 * @return  If succeed in reading, return 1;Otherwise, return 0
 */
int EDDCRead(unsigned char segpointer, unsigned char segment, unsigned char addr,
  unsigned char offset, unsigned int size, unsigned char* buffer)
{
        int ret = 0;
        struct i2c_rdwr_ioctl_data msgset;
        struct i2c_msg msgs[3];

        do {
                if (!DDCFileAvailable()) {
                        ALOGE("I2C_DDC is not available!!!!\n\n");
                        break;
                }

	        DPRINTF("EDID READ START segpointer:0x%x segment:0x%x add:0x%x offset:0x%x size:%d!!\n", segpointer, segment, addr, offset, size);

                // set segment pointer
                msgs[0].addr  = segpointer>>1;
                msgs[0].flags = I2C_M_IGNORE_NAK;
                msgs[0].len   = 1;
                msgs[0].buf   = &segment;

                // set offset
                msgs[1].addr  = addr>>1;
                msgs[1].flags = 0;
                msgs[1].len   = 1;
                msgs[1].buf   = &offset;

                // read data
                msgs[2].addr  = addr>>1;
                msgs[2].flags = I2C_M_RD;
                msgs[2].len   = size;
                msgs[2].buf   = buffer;

                msgset.nmsgs = 3;
                msgset.msgs  = msgs;

                // eddc read
                if (ioctl(ddc_fd, I2C_RDWR, &msgset) < 0) {
                        ALOGE("ioctl(I2C_RDWR) failed!!!\n");
                        break;
                }
                ret = 1;
        } while(0);

	DPRINTF("EDID READ end ret: %d !!\n",ret);

        return ret;
}


