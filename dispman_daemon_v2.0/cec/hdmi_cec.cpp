/*!
* TCC Version 1.0
* Copyright (c) Telechips Inc.
* All rights reserved 
*  \file        hdmi_cec.cpp
*  \brief       HDMI CEC application source
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
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <utils/Timers.h>

#include <utils/types.h>

#include <hdmi_v2_0_cec/include/hdmi_cec_ioctl.h>
#include <utils/Log.h>
#include <utils/properties.h>
#include <utils/bit_operation.h>

#include <hdmi/edid_type.h>
#include <cec/hdmi_cec.h>
#include <hdmi/hdmi_edid.h>


#define LOG_TAG "[HDMICEC   ]"

#define LIBDDC_DEBUG 0
#if LIBDDC_DEBUG
#define CEC_PRINTF(args...)    ALOGD(args)
#else
#define CEC_PRINTF(args...)
#endif

#define LIBCEC_DEBUG_MESSAGE		1

#define LIBCEC_CHECK_STATE_OF_SINK	0	// Disable because CECT 9.3-1

static struct {
    enum CECDeviceType devtype;
    unsigned char laddr;
} laddresses[] = {
    { CEC_DEVICE_TV,		0  },
    { CEC_DEVICE_RECODER,	1  },
    { CEC_DEVICE_RECODER,	2  },
    { CEC_DEVICE_TUNER,		3  },
    { CEC_DEVICE_PLAYBACK,	4  },
    { CEC_DEVICE_AUDIO,		5  },
    { CEC_DEVICE_TUNER,		6  },
    { CEC_DEVICE_TUNER,		7  },
    { CEC_DEVICE_PLAYBACK,	8  },
    { CEC_DEVICE_RECODER,	9  },
    { CEC_DEVICE_TUNER,		10 },
    { CEC_DEVICE_PLAYBACK,	11 },
    { CEC_DEVICE_RESERVED,	12 },
    { CEC_DEVICE_RESERVED,	13 },
    { CEC_DEVICE_FREE,		14 },
    { CEC_DEVICE_UNREGISTED,	15 },
};


static hdmitx_cec_t cecParm;


int hdmitx_open_cec_device(hdmitx_cec_t* dev)
{
	if(dev->hdmi_cec_driver <= 0)
	{
		strcpy(dev->hdmi_cec_name, "/dev/dw-hdmi-cec");
		dev->hdmi_cec_driver = open(dev->hdmi_cec_name,O_RDWR);
		if(dev->hdmi_cec_driver < 0){
			ALOGE("Could not open %s device....\n",dev->hdmi_cec_name);
			return dev->hdmi_cec_driver;
		}
	}

	dev->devtype = CEC_DEVICE_PLAYBACK;
	dev->vendorID = 0x0000;

	return 0;
}

int hdmitx_close_cec_device(hdmitx_cec_t* dev)
{
	if(dev->hdmi_cec_driver > 0)	
	{
		close(dev->hdmi_cec_driver);
		dev->hdmi_cec_driver = -1;
	}
	return 0;
}

static int hdmitx_cec_start(hdmitx_cec_t* dev)
{
	int ret;
        
	ret = ioctl(dev->hdmi_cec_driver, CEC_IOC_START, NULL);
	if(ret) {
            ALOGE("Failed CEC_IOC_START IOCTL [%d]\n", ret);
    }
	return ret;
	
}


static int hdmitx_cec_stop(hdmitx_cec_t* dev)
{
	int ret,parm;

	parm = FALSE;

	ret = ioctl(dev->hdmi_cec_driver, CEC_IOC_STOP, &parm);
	if(ret < 0) {
            ALOGE("Failed CEC_IOC_START IOCTL [%d]\n", ret);
    }
	return ret;
	
}


static int hdmitx_cec_send_msg(hdmitx_cec_t* dev)
{

	int ret;
	
	ret = ioctl(dev->hdmi_cec_driver, CEC_IOC_SENDDATA, &dev->buffer);

#ifdef LIBCEC_DEBUG_MESSAGE
	printf("\r\n[cec message][tx-->] [cnt=%d] [data]",dev->buffer.size);
	for(int i = 0; i < (int)dev->buffer.size; i++)	printf("[%d]0x%02x",i,dev->buffer.send_buf[i]);
	printf("\r\n");

	if(ret < 0)	printf("[cec Message][tx<--] [NAck]\r\n");
#endif	
	return ret;
}

static int hdmitx_cec_recv_msg(hdmitx_cec_t* dev)
{
	int msg_size;
	
	msg_size = ioctl(dev->hdmi_cec_driver, CEC_IOC_RECVDATA, &dev->buffer.recv_buf);

	return msg_size;
}

int hdmitx_init_edid(hdmitx_cec_t* dev)
{
        int ret = -1;
        sink_edid_t* sink;

	sink = (sink_edid_t *) malloc(sizeof(sink_edid_t));
                
        edid_read_cap(sink);
        if(!sink->edid_done) {
                ALOGE("hdmitx_init_edid failed\r\n");
                goto end_process;
        }
	if(dev->p_address != sink->edid_mHdmivsdb.mPhysicalAddress)
		dev->p_address = sink->edid_mHdmivsdb.mPhysicalAddress;
        CEC_PRINTF("Device physical address is %X.%X.%X.%X\n",
          (dev->p_address & 0xF000) >> 12, (dev->p_address & 0x0F00) >> 8,
          (dev->p_address & 0x00F0) >> 4, dev->p_address & 0x000F);
        ret = 0;

end_process:
        free(sink);
        return ret;
}

static int hdmitx_cec_Ignore_msg(unsigned char opcode, unsigned char lsrc)
{
	int retval = 0;

	/* if a message coming from address 15 (unregistered) */
	if (lsrc == CEC_LADDR_UNREGISTERED)
	{
		switch (opcode)
		{
			case CEC_OPCODE_DECK_CONTROL:
			case CEC_OPCODE_PLAY:
			case CEC_OPCODE_GIVE_DECK_STATUS:
			case CEC_OPCODE_GIVE_OSD_NAME:
			case CEC_OPCODE_MENU_REQUEST:
				retval = 1;
				break;
				
			default:
				break;
		}
	}

	return retval;
}

static int hdmitx_cec_Check_msg_size(unsigned char opcode, int size)
{
    int retval = 0;

    switch (opcode) {
	case CEC_OPCODE_SET_OSD_NAME:
	case CEC_OPCODE_REPORT_POWER_STATUS:
        case CEC_OPCODE_PLAY:
        case CEC_OPCODE_DECK_CONTROL:
	case CEC_OPCODE_USER_CONTROL_PRESSED:
//        case CEC_OPCODE_SET_MENU_LANGUAGE:
            if (size != 3) retval = 1;
            break;
	case CEC_OPCODE_USER_CONTROL_RELEASED:
            if (size != 2) retval = 1;
            break;
        case CEC_OPCODE_SET_STREAM_PATH:
        case CEC_OPCODE_FEATURE_ABORT:
            if (size != 4) retval = 1;
            break;
        default:
            break;
    }

    return retval;

}

static int hdmitx_cec_report_physical_address(hdmitx_cec_t* dev)
{

	if (dev->src_l_address < 0)
		return 0;
	
        dev->buffer.send_buf[0] = (dev->src_l_address << 4) | CEC_MSG_BROADCAST;
        dev->buffer.send_buf[1] = CEC_OPCODE_REPORT_PHYSICAL_ADDRESS;
        dev->buffer.send_buf[2] = (dev->p_address >> 8) & 0xFF;
        dev->buffer.send_buf[3] = dev->p_address & 0xFF;
        dev->buffer.send_buf[4] = dev->devtype;
        dev->buffer.size = 5;

	if(hdmitx_cec_send_msg(dev) < 0) return 0;

	return 1;
}


static int hdmitx_get_logical_address(hdmitx_cec_t* dev)
{

	if(dev->hdmi_cec_driver == -1)
	{
		ALOGE("%s : first, driver open please.",__func__);
		return -1;
	}

	if (dev->p_address == CEC_NOT_VALID_PHYSICAL_ADDRESS)
	{
		return -1;
	}	

	dev->src_l_address = CEC_LADDR_UNREGISTERED;

	for (int index = 0; index < (int)sizeof(laddresses)/(int)sizeof(laddresses[0]);index++)
	{

		if (laddresses[index].devtype == dev->devtype)
		{
			unsigned char _laddr = laddresses[index].laddr;

			dev->buffer.send_buf[0] = _laddr << 4 | _laddr;
			dev->buffer.size = 1;

			if(hdmitx_cec_send_msg(dev) < 0 && dev->src_l_address == CEC_LADDR_UNREGISTERED)
			{
				dev->src_l_address = _laddr;
				break;
			}

		}
	}
	
	if(dev->src_l_address == CEC_LADDR_UNREGISTERED)
	{
		ALOGE("%s: cec logical address is unregistered.....\n",__func__);
		return -1;
	}

	if (ioctl(dev->hdmi_cec_driver, CEC_IOC_SETLADDR, &dev->src_l_address)) {
		ALOGE("%s: ioctl(CEC_IOC_SETLA) failed!\n",__func__);
		return -1;
	}

	return 0;
    

}

static int hdmitx_cec_image_view_on(hdmitx_cec_t* dev)
{
	dev->buffer.send_buf[0] = dev->src_l_address << 4 | CEC_DEVICE_TV;
	dev->buffer.send_buf[1] = CEC_OPCODE_IMAGE_VIEW_ON;
	dev->buffer.size = 2;
	
	hdmitx_cec_send_msg(dev);

	return 0;
	
}

static int hdmitx_cec_text_view_on(hdmitx_cec_t* dev)
{
	dev->buffer.send_buf[0] = dev->src_l_address << 4 | CEC_DEVICE_TV;
	dev->buffer.send_buf[1] = CEC_OPCODE_TEXT_VIEW_ON;
	dev->buffer.size = 2;
	
	hdmitx_cec_send_msg(dev);

	return 0;
	
}

static int hdmitx_cec_active_source(hdmitx_cec_t* dev)
{
	if (dev->src_l_address< 0)
		return 0;

	/* send "Active Source" */
	dev->buffer.send_buf[0] = dev->src_l_address << 4 | CEC_MSG_BROADCAST;
	dev->buffer.send_buf[1] = CEC_OPCODE_ACTIVE_SOURCE;
	dev->buffer.send_buf[2] = (dev->p_address>> 8) & 0xFF;
	dev->buffer.send_buf[3] = dev->p_address& 0xFF;
	dev->buffer.size = 4;	

	if(hdmitx_cec_send_msg(dev) < 0) return 0;

	return 1;
}

static int hdmitx_cec_inactive_source(hdmitx_cec_t* dev)
{
	if (dev->src_l_address< 0)
		return 0;

	/* send "Active Source" */
	dev->buffer.send_buf[0] = (dev->src_l_address << 4) | dev->dst_l_address;
	dev->buffer.send_buf[1] = CEC_OPCODE_INACTIVE_SOURCE;
	dev->buffer.send_buf[2] = (dev->p_address >> 8) & 0xFF;
	dev->buffer.send_buf[3] = dev->p_address & 0xFF;
	dev->buffer.size = 4;

	//TODO: protect with mutex
	if(hdmitx_cec_send_msg(dev) < 0) return 0;

	return 1;
}

static int hdmitx_cec_standBy(hdmitx_cec_t* dev,int broadcast)
{
	if (dev->src_l_address< 0)
		return 0;

	/* send "Image View On" */
	dev->buffer.send_buf[0] = (dev->src_l_address << 4) | broadcast;
	dev->buffer.send_buf[1] = CEC_OPCODE_STANDBY;
	dev->buffer.size = 2;

	//TODO: protect with mutex
	if(hdmitx_cec_send_msg(dev) < 0) return 0;

    return 1;

}

static int hdmitx_cec_MenuStatus(hdmitx_cec_t* dev, int state)
{
	if (dev->src_l_address < 0)
		return 0;

	dev->buffer.send_buf[0] = (dev->src_l_address << 4) | CEC_DEVICE_TV;// TV logical address is "0"
	dev->buffer.send_buf[1] = CEC_OPCODE_MENU_STATUS;
	dev->buffer.send_buf[2] = state;	// 0 : activated, 1, deactivated
	dev->buffer.size = 3;

	if(hdmitx_cec_send_msg(dev) < 0) return 0;

    return 1;
}
static int hdmitx_check_physical_address(hdmitx_cec_t* dev,int paddr)
{

        if (hdmitx_init_edid(dev) < 0) {
                ALOGE("Error: hdmitx_init_edid() failed.\n");
        }

        CEC_PRINTF("%s, paddr_edid = %d, paddr = %d\n", __func__, dev->p_address, paddr);

        if(dev->p_address== paddr )
                return 1;

        return 0;
}


static int hdmitx_set_vendorID(hdmitx_cec_t* dev)
{

	if(dev->buffer.recv_buf[1] == CEC_OPCODE_DEVICE_VENDOR_ID || 
		dev->buffer.recv_buf[1] == CEC_OPCODE_VENDOR_COMMAND_WITH_ID)
	{
		dev->TVvendorID = dev->buffer.recv_buf[3] << 8 | dev->buffer.recv_buf[4];
	}

	return 0;
}

/**
 * This API is not used. ( Samsung Vendor ID Error )
 
static int hdmitx_get_vendorID(hdmitx_cec_t* dev)
{
	if (dev->src_l_address < 0)
		return 0;

	dev->buffer.send_buf[0] = (dev->src_l_address << 4) | CEC_DEVICE_TV;
	dev->buffer.send_buf[1] = CEC_OPCODE_GET_DEVICE_VENDOR_ID;
	dev->buffer.size = 2;
	
	if(hdmitx_cec_send_msg(dev) < 0) return 0;

	return 1;
}

*/

static int hdmitx_cec_set_menu_language(hdmitx_cec_t* dev)
{

	if (dev->src_l_address < 0)
		return 0;

        dev->buffer.send_buf[0] = (dev->src_l_address << 4) | dev->dst_l_address;
        dev->buffer.send_buf[1] = CEC_OPCODE_SET_MENU_LANGUAGE;
        dev->buffer.send_buf[2] = dev->menu_language[0];//'e';
        dev->buffer.send_buf[3] = dev->menu_language[1];//'n';
        dev->buffer.send_buf[4] = dev->menu_language[2];//'g';
        dev->buffer.size = 5;

	if(hdmitx_cec_send_msg(dev) < 0) return 0;

	return 1;
}

static int hdmitx_cec_report_power_status(hdmitx_cec_t* dev)
{

	if (dev->src_l_address < 0)
		return 0;

        dev->buffer.send_buf[0] = dev->src_l_address << 4 | dev->dst_l_address;
        dev->buffer.send_buf[1] = CEC_OPCODE_REPORT_POWER_STATUS;
        dev->buffer.send_buf[2] = CEC_POWER_STATUS_ON;
        dev->buffer.size = 3;

	if(hdmitx_cec_send_msg(dev) < 0) return 0;

	return 1;

}
/**
 * This API is not used. 
static int hdmitx_cec_user_control_pressed(hdmitx_cec_t* dev, unsigned char key_value)
{

	if (dev->src_l_address < 0)
		return 0;

        dev->buffer.send_buf[0] = (dev->src_l_address << 4) | dev->dst_l_address;
        dev->buffer.send_buf[1] = CEC_OPCODE_USER_CONTROL_PRESSED;
        dev->buffer.send_buf[2] = key_value;
        dev->buffer.size = 3;

	if(hdmitx_cec_send_msg(dev) < 0) return 0;

	return 1;
}
*/

static int hdmitx_cec_routing_information(hdmitx_cec_t* dev)
{

	if (dev->src_l_address < 0)
		return 0;

	dev->buffer.send_buf[0] = (dev->src_l_address << 4) | CEC_MSG_BROADCAST;
	dev->buffer.send_buf[1] = CEC_OPCODE_ROUTING_INFORMATION;
	dev->buffer.send_buf[2] = (dev->p_address >> 8) & 0xFF;
	dev->buffer.send_buf[3] = dev->p_address & 0xFF;
	dev->buffer.size = 4;

	if(hdmitx_cec_send_msg(dev) < 0) return 0;

	return 1;
}

static int hdmitx_cec_version(hdmitx_cec_t* dev)
{

	if (dev->src_l_address < 0)
		return 0;

        dev->buffer.send_buf[0] = (dev->src_l_address << 4) | dev->dst_l_address;
        dev->buffer.send_buf[1] = CEC_OPCODE_CEC_VERSION;
        dev->buffer.send_buf[2] = CEC_VERSION_14A;
        dev->buffer.size = 3;

	if(hdmitx_cec_send_msg(dev) < 0) return 0;

	return 1;
}

static int hdmitx_cec_device_vendor_id(hdmitx_cec_t* dev)
{

	if (dev->src_l_address < 0)
		return 0;
	
        dev->buffer.send_buf[0] = dev->src_l_address << 4 | CEC_MSG_BROADCAST;
        dev->buffer.send_buf[1] = CEC_OPCODE_DEVICE_VENDOR_ID;
        dev->buffer.send_buf[2] = 0x00;
        dev->buffer.send_buf[3] = (dev->vendorID >> 8) & 0xFF;
        dev->buffer.send_buf[4] = dev->vendorID  & 0xFF;
        dev->buffer.size = 5;
	
	if(hdmitx_cec_send_msg(dev) < 0) return 0;

	return 1;
}

static int hdmitx_cec_set_osd_name(hdmitx_cec_t* dev)
{

	if (dev->src_l_address < 0)
		return 0;
	
        dev->buffer.send_buf[0] = dev->src_l_address << 4 | dev->dst_l_address;
        dev->buffer.send_buf[1] = CEC_OPCODE_SET_OSD_NAME;
        dev->buffer.send_buf[2] = 'T';
        dev->buffer.send_buf[3] = 'C';
        dev->buffer.send_buf[4] = 'P';
        dev->buffer.send_buf[5] = 'L';
        dev->buffer.send_buf[6] = 'A';
        dev->buffer.send_buf[7] = 'Y';
        dev->buffer.send_buf[8] = 'E';
        dev->buffer.send_buf[9] = 'R';
        dev->buffer.size = 10;
	
	if(hdmitx_cec_send_msg(dev) < 0) return 0;

	return 1;
}

static int hdmitx_cec_deck_status(hdmitx_cec_t* dev)
{

	if (dev->src_l_address < 0)
		return 0;
	
        dev->buffer.send_buf[0] = (dev->src_l_address << 4) | dev->dst_l_address;
        dev->buffer.send_buf[1] = CEC_OPCODE_DECK_STATUS;
	dev->buffer.send_buf[2] = 0x11;
        dev->buffer.size = 3;
	
	if(hdmitx_cec_send_msg(dev) < 0) return 0;

	return 1;
}

static int hdmitx_cec_simplelink_alive(hdmitx_cec_t* dev)
{

	if (dev->src_l_address < 0)
		return 0;
	
        dev->buffer.send_buf[0] = dev->src_l_address << 4 | CEC_DEVICE_TV;
        dev->buffer.send_buf[1] = CEC_OPCODE_VENDOR_COMMAND;
        dev->buffer.send_buf[2] = 2;
        dev->buffer.send_buf[3] = 5;
        dev->buffer.size = 4;
	
	if(hdmitx_cec_send_msg(dev) < 0) return 0;

	return 1;
}

static int hdmitx_cec_simplelink_ack(hdmitx_cec_t* dev)
{

	if (dev->src_l_address < 0)
		return 0;
	
	dev->buffer.send_buf[0] = dev->src_l_address << 4 | CEC_DEVICE_TV;
	dev->buffer.send_buf[1] = CEC_OPCODE_VENDOR_COMMAND;
	dev->buffer.send_buf[2] = 5;
	dev->buffer.send_buf[3] = 1;
	dev->buffer.size = 4;
	
	if(hdmitx_cec_send_msg(dev) < 0) return 0;

	return 1;
}



static int hdmitx_cec_feature_abort(hdmitx_cec_t* dev, unsigned char abort_opcode,unsigned char abort_operand)
{

	if (dev->src_l_address < 0)
		return 0;
	
        dev->buffer.send_buf[0] = (dev->src_l_address << 4) | dev->dst_l_address;
        dev->buffer.send_buf[1] = CEC_OPCODE_FEATURE_ABORT;
        dev->buffer.send_buf[2] = abort_opcode;
        dev->buffer.send_buf[3] = abort_operand;
        dev->buffer.size = 4;
	
	if(hdmitx_cec_send_msg(dev) < 0) return 0;

	return 1;
}

static int hdmitx_cec_request_tv_status(hdmitx_cec_t* dev)
{

	if (dev->src_l_address < 0)
		return 0;
	
        dev->buffer.send_buf[0] = (dev->src_l_address << 4) | CEC_DEVICE_TV;
        dev->buffer.send_buf[1] = CEC_OPCODE_GIVE_DEVICE_POWER_STATUS;
        dev->buffer.size = 2;
	
	if(hdmitx_cec_send_msg(dev) < 0) return 0;

	return 1;
}

static int hdmitx_cec_check_menu_language(hdmitx_cec_t* dev)
{
	
	if((dev->buffer.recv_buf[2]=='e' && dev->buffer.recv_buf[3]=='n' && dev->buffer.recv_buf[4]=='g') ||
		(dev->buffer.recv_buf[2]=='c' && dev->buffer.recv_buf[3]=='h' && dev->buffer.recv_buf[4]=='n') ||
		(dev->buffer.recv_buf[2]=='f' && dev->buffer.recv_buf[3]=='r' && dev->buffer.recv_buf[4]=='a') ||
		(dev->buffer.recv_buf[2]=='j' && dev->buffer.recv_buf[3]=='p' && dev->buffer.recv_buf[4]=='n') ||
		(dev->buffer.recv_buf[2]=='k' && dev->buffer.recv_buf[3]=='o' && dev->buffer.recv_buf[4]=='r'))
		return 1;

	return 0;
}

static int hdmitx_cec_Check_msg_mode(unsigned char opcode, int broadcast)
{
    int retval = 0;

    switch (opcode) {
	case CEC_OPCODE_REQUEST_ACTIVE_SOURCE:
	case CEC_OPCODE_SET_STREAM_PATH:
	case CEC_OPCODE_ROUTING_INFORMATION:
            if (!broadcast) retval = 1;
            break;
	    
	case CEC_OPCODE_INACTIVE_SOURCE:	    
	case CEC_OPCODE_GIVE_PHYSICAL_ADDRESS:
	case CEC_OPCODE_DECK_STATUS:
	case CEC_OPCODE_DECK_CONTROL:
	case CEC_OPCODE_GIVE_DECK_STATUS:
	case CEC_OPCODE_CEC_VERSION:
	case CEC_OPCODE_GET_CEC_VERSION:
	case CEC_OPCODE_PLAY:
	case CEC_OPCODE_DEVICE_VENDOR_ID:
	case CEC_OPCODE_GET_DEVICE_VENDOR_ID:
	case CEC_OPCODE_USER_CONTROL_PRESSED:
	case CEC_OPCODE_USER_CONTROL_RELEASED:
	case CEC_OPCODE_FEATURE_ABORT:
	case CEC_OPCODE_ABORT:
	case CEC_OPCODE_SET_OSD_NAME:
	case CEC_OPCODE_GIVE_OSD_NAME:
	case CEC_OPCODE_GIVE_DEVICE_POWER_STATUS:
	case CEC_OPCODE_REPORT_POWER_STATUS:
	case CEC_OPCODE_MENU_REQUEST:
	case CEC_OPCODE_MENU_STATUS:
	case CEC_OPCODE_SET_MENU_LANGUAGE:
            if (broadcast) retval = 1;
            break;
        default:
            break;
    }

    return retval;
}

void hdmitx_cec_set_connect_status(char status)
{
	char	value[PROPERTY_VALUE_MAX];
	
	if(status)	value[0] = '1';
	else		value[0] = '0';

	property_set("tcc.cec.connection", value);
}

/**
 * This API is not used. use for test
 
void hdmitx_cec_status_check(hdmitx_cec_t* dev, int check)
{
	unsigned int	uiCec_Value = 0;
	char	value[PROPERTY_VALUE_MAX];

	memset(value, 0, PROPERTY_VALUE_MAX);

	if(check)
	{
		property_get("tcc.cec.connect_check", value, "");
		uiCec_Value = atoi(value);
		if(uiCec_Value > 0)
		{

			hdmitx_cec_request_tv_status(dev);

			for(int cnt = 0; cnt < 3 ; cnt++)
			{
				if(hdmitx_cec_recv_msg(dev) > 0 )
				{
					hdmitx_cec_set_connect_status(1);
					break;
				}
				else
				{
					hdmitx_cec_set_connect_status(0);
				}
				
		                usleep(15000);
			}

			value[0] = '0';
			property_set("tcc.cec.connect_check", value);
			uiCec_Value = 0;

		}
		
	}
	else
	{
		value[0] = '0';
		property_set("tcc.cec.connection", value);
	}
}

*/

int hdmitx_cec_cmd_process(hdmitx_cec_t* dev)
{
	
	unsigned int	uiCec_Value = 0;

	char	value[PROPERTY_VALUE_MAX];

	memset(value, 0, PROPERTY_VALUE_MAX);

	property_get("tcc.cec.imageview_on", value, "");
	uiCec_Value = atoi(value);
	if(uiCec_Value)
	{
		hdmitx_cec_image_view_on(dev);
		value[0] = '0';
		property_set("tcc.cec.imageview_on", value);
	}

	property_get("tcc.cec.textview_on", value, "");
	uiCec_Value = atoi(value);
	if(uiCec_Value)
	{
		hdmitx_cec_text_view_on(dev);
		value[0] = '0';
		property_set("tcc.cec.textview_on", value);
	}		

	property_get("tcc.cec.active_source", value, "");
	uiCec_Value = atoi(value);
	if(uiCec_Value)
	{
		hdmitx_cec_active_source(dev);
		value[0] = '0';
		property_set("tcc.cec.active_source", value);
	}

	property_get("tcc.cec.in_active_source", value, "");
	uiCec_Value = atoi(value);
	if(uiCec_Value)
	{
		hdmitx_cec_inactive_source(dev);
		value[0] = '0';
		property_set("tcc.cec.in_active_source", value);
	}

	property_get("tcc.cec.standby", value, "");
	uiCec_Value = atoi(value);
	if(uiCec_Value == 1)
	{
		hdmitx_cec_standBy(dev,0);
		value[0] = '0';
		property_set("tcc.cec.standby", value);
	}
	else if(uiCec_Value == 2)
	{
		hdmitx_cec_standBy(dev,CEC_MSG_BROADCAST);
		value[0] = '0';
		property_set("tcc.cec.standby", value);
	}

	property_get("tcc.cec.request_tv_status", value, "");
	uiCec_Value = atoi(value);	
	if(uiCec_Value == 1)
	{
		hdmitx_cec_request_tv_status(dev);
		value[0] = '0';
		property_set("tcc.cec.request_tv_status", value);		
	}

	property_get("tcc.cec.menu_status", value, "");
	uiCec_Value = atoi(value);
	if(uiCec_Value == 1)
	{
		hdmitx_cec_MenuStatus(dev,0);
		value[0] = '0';
		property_set("tcc.cec.menu_status", value);
	}
	else if(uiCec_Value == 2)
	{
		hdmitx_cec_MenuStatus(dev,1);
		value[0] = '0';
		property_set("tcc.cec.menu_status", value);
	}

	return 1;
}

static unsigned int hdmitx_cec_check_sink_status(hdmitx_cec_t* dev)
{
	unsigned int sink_status = FALSE;
	unsigned int Initator;
	int size;

	/** Send PING **/
        dev->buffer.send_buf[0] = (dev->src_l_address << 4) | CEC_DEVICE_TV;
        dev->buffer.size = 1;
	if(hdmitx_cec_send_msg(dev) < 0) return sink_status;


	if(!dev->running_cec) return sink_status;
	

	/**Send Command "Give Power Status"**/
	hdmitx_cec_request_tv_status(dev);
	
	/**Check Status**/
	for(int count = 0; count < 10; count++)
	{
		if((size = hdmitx_cec_recv_msg(dev)) > 1)
		{
			Initator = dev->buffer.recv_buf[0] >> 4;
			
			if(Initator == CEC_DEVICE_TV && dev->buffer.recv_buf[1] == CEC_OPCODE_REPORT_POWER_STATUS)
			{
#ifdef LIBCEC_DEBUG_MESSAGE
			printf("\r\n[cec message][<--rx] [cnt=%d] [data]",size);
			for(int index = 0; index < size; index++)	printf("[%d]0x%02x",index, dev->buffer.recv_buf[index]);
			printf("\r\n");
#endif				
				if(dev->buffer.recv_buf[2] == 0x0 || dev->buffer.recv_buf[2] == 0x2)
				{
					printf("[%s]Sink Power Status ON\n",__func__);
					sink_status = TRUE;
				}
				else
				{
					printf("[%s]Sink Power Status OFF\n",__func__);
				}
				break;
			}
		}

		if(!dev->running_cec)
		{
			sink_status = FALSE;
			break;
		}
		
		usleep(15000);
	}

	return sink_status;
}

static void* hdmitx_cec_thread(void* arg)
{
	hdmitx_cec_t* dev = (hdmitx_cec_t*)arg;

	unsigned int cec_func_error = 0;
#if LIBDDC_DEBUG
	unsigned int cec_orignial_address;
#endif
	unsigned int cec_new_address;

#if LIBCEC_CHECK_STATE_OF_SINK
	unsigned int cec_status_check = 0;
#endif

	char	value[PROPERTY_VALUE_MAX];

	dev->running_cec = 0;

	hdmitx_cec_set_connect_status(0);

	ALOGI("CECThread START!!!\n");

	while(dev->thread_run)
	{
		int size;
		unsigned char Receive_Initiator,Receive_Destination, Receive_opcode;
#if LIBCEC_CHECK_STATE_OF_SINK		
		nsecs_t poll_start_ns = systemTime();
#endif

		while (dev->thread_run && !dev->running_cec ) {
			usleep(1000000);
		}

		if(dev->thread_run && hdmitx_open_cec_device(dev) < 0) {
			usleep(500000);
			continue;
		}

		if(!dev->thread_run) break;

		
		
		hdmitx_cec_start(dev);

		hdmitx_check_physical_address(dev,dev->p_address);

		hdmitx_get_logical_address(dev);

		hdmitx_cec_report_physical_address(dev);

		if(hdmitx_cec_check_sink_status(dev))
		{
			hdmitx_cec_image_view_on(dev);
			hdmitx_cec_text_view_on(dev);
			hdmitx_cec_active_source(dev);
			hdmitx_cec_device_vendor_id(dev);					
		}


		if(hdmitx_cec_check_menu_language(dev) == 0)
		{
			dev->menu_language[0] = 'e';
			dev->menu_language[1] = 'n';
			dev->menu_language[2] = 'g';
		}

		while (dev->thread_run && dev->running_cec) {
			usleep(50000);
			hdmitx_cec_cmd_process(dev);
						
			size = hdmitx_cec_recv_msg(dev);

			if( size < 0 ) {
				ALOGE("CEC Rx Error!!!!!!\n");
				cec_func_error = 1;
				dev ->running_cec = FALSE;
				continue;
			}

#if LIBCEC_CHECK_STATE_OF_SINK
			if(size == 0) {

			if(cec_status_check) {
				hdmitx_cec_set_connect_status(0);
				cec_status_check = FALSE;
			}

			if(systemTime() -  poll_start_ns > POLL_TIME) {
				hdmitx_cec_request_tv_status(dev);
				cec_status_check = TRUE;
				poll_start_ns = systemTime();
			}
			continue;
			} else{
				hdmitx_cec_set_connect_status(1);
				cec_status_check = FALSE;
				poll_start_ns = systemTime();
			}
#else
			if(size == 0)	continue;
#endif

			if(size == 1)	continue; //PoLLING MSG


			hdmitx_set_vendorID(dev);

			Receive_Initiator = dev->buffer.recv_buf[0] >> 4;
			Receive_Destination = dev->buffer.recv_buf[0] & 0x0F;

			/* ignore messages with src address == laddr */
			if (Receive_Initiator == dev->src_l_address) continue;

			Receive_opcode = dev->buffer.recv_buf[1];

			if (hdmitx_cec_Ignore_msg(Receive_opcode, Receive_Initiator)) {
				CEC_PRINTF("### ignore message coming from address 15 (unregistered)\n");
				continue;
			}

			if (hdmitx_cec_Check_msg_size(Receive_opcode, size)) {
				CEC_PRINTF("### invalid message size ###\n");
				continue;
			}

			/* check if message broadcast/directly addressed */
			if (hdmitx_cec_Check_msg_mode(Receive_opcode, Receive_Destination == CEC_MSG_BROADCAST ? 1 : 0)) {
				CEC_PRINTF("### invalid message mode (directly addressed/broadcast) ###\n");
				continue;
			}

			dev->dst_l_address = Receive_Initiator;
			
#ifdef LIBCEC_DEBUG_MESSAGE
			printf("\r\n[cec message][<--rx] [cnt=%d] [data]",size);
			for(int index = 0; index < size; index++)	printf("[%d]0x%02x",index, dev->buffer.recv_buf[index]);
			printf("\r\n");
#endif

			switch(Receive_opcode)
			{
				case CEC_OPCODE_GIVE_PHYSICAL_ADDRESS:
					hdmitx_cec_report_physical_address(dev);
					break;

				case CEC_OPCODE_GET_MENU_LANGUAGE:
					hdmitx_cec_set_menu_language(dev);
					break;

				case CEC_OPCODE_SET_MENU_LANGUAGE:
					if((hdmitx_cec_check_menu_language(dev) > 0) && dev->dst_l_address == CEC_DEVICE_TV)
					{
						ALOGI("The menu language has changed, %c%c%c to %c%c%c\n",
							dev->menu_language[0],dev->menu_language[1],dev->menu_language[2],
							dev->buffer.recv_buf[2],dev->buffer.recv_buf[3],dev->buffer.recv_buf[4]);
						dev->menu_language[0] = dev->buffer.recv_buf[2];
						dev->menu_language[1] = dev->buffer.recv_buf[3];
						dev->menu_language[2] = dev->buffer.recv_buf[4];
					}
					else
					{
						ALOGI("The menu language has not changed. keep '%c%c%c'\n",dev->menu_language[0],dev->menu_language[1],dev->menu_language[2]);
					}
					break;
					
				case CEC_OPCODE_GIVE_DEVICE_POWER_STATUS:
					hdmitx_cec_report_power_status(dev);
					break;

				case CEC_OPCODE_REPORT_POWER_STATUS:
					/* send Power On message */
					if (dev->buffer.recv_buf[2] == 0 || dev->buffer.recv_buf[2] == 0x2)
						value[0] = '1';
					else
						value[0] = '0';
					
					property_set("tcc.cec.tv_status",value);
					break;					

				case CEC_OPCODE_REPORT_PHYSICAL_ADDRESS:// TV
				case CEC_OPCODE_ACTIVE_SOURCE:        // TV, CEC Switches
				case CEC_OPCODE_ROUTING_INFORMATION:    // CEC Switches
				case CEC_OPCODE_SET_SYSTEM_AUDIO_MODE:    // TV
				case CEC_OPCODE_DEVICE_VENDOR_ID:
				case CEC_OPCODE_RECORD_OFF:
					break;//continue;

				case CEC_OPCODE_ROUTING_CHANGE:        // CEC Switches
				#if LIBDDC_DEBUG
					cec_orignial_address = (((dev->buffer.recv_buf[2] << 8) & 0xFF00) | (dev->buffer.recv_buf[3] & 0xFF));
				#endif
					cec_new_address = (((dev->buffer.recv_buf[4] << 8) & 0xFF00) | (dev->buffer.recv_buf[5] & 0xFF));

					CEC_PRINTF("### cec_orignial_address = 0x%08x\n", cec_orignial_address);
					CEC_PRINTF("### cec_new_address = 0x%08x\n", cec_new_address);

					// If a CEC Switch is at the new address, it send a <Routing Information> message to indicate its current active route.
					if(cec_new_address == (unsigned int)dev->p_address) {
						/* responce with "Active Source" */
						hdmitx_cec_routing_information(dev);
					}
					//Don't send <Routing Information>, because new address is not matched with our paddr.
					break;

				case CEC_OPCODE_GET_CEC_VERSION:
					/* responce with "Active Source" */
					hdmitx_cec_version(dev);
					break;

				case CEC_OPCODE_GET_DEVICE_VENDOR_ID:
					hdmitx_cec_device_vendor_id(dev);

					if((dev->dst_l_address == CEC_DEVICE_TV) && 
						(dev->TVvendorID == 0x8045 || (strcmp(dev->vendor_info_name,"MEI") == 0)))
					{
						hdmitx_cec_active_source(dev);
						hdmitx_cec_MenuStatus(dev,0);
					}
					break;

				case CEC_OPCODE_DECK_CONTROL:
					if (dev->buffer.recv_buf[2] == CEC_DECK_CONTROL_MODE_STOP) {
						CEC_PRINTF("### DECK CONTROL : STOP ###\n");
					}
					break;

				case CEC_OPCODE_PLAY:
					if (dev->buffer.recv_buf[2] == CEC_PLAY_MODE_PLAY_FORWARD) {
						CEC_PRINTF("### PLAY MODE : PLAY ###\n");
					}
					else if (dev->buffer.recv_buf[2] == CEC_PLAY_MODE_PLAY_STILL) {
						CEC_PRINTF("### PAUSE MODE : PAUSE ###\n");
					}
					break;
					
				case CEC_OPCODE_STANDBY:
					CEC_PRINTF("### switching device into standby... ###\n");
					break;

				case CEC_OPCODE_USER_CONTROL_PRESSED:
					CEC_PRINTF("### USER CONTROL PRESSED ###\n");
					switch(dev->buffer.recv_buf[2])
					{
						case CEC_USER_CONTROL_MODE_FAST_FORWARD:
							CEC_PRINTF("### FAST FORWARD MODE : FFW ###\n");
							break;
						case CEC_USER_CONTROL_MODE_REWIND:
							CEC_PRINTF("### REWIND MODE : REW ###\n");
							break;
						case CEC_USER_CONTROL_MODE_ROOT_MENU:
							CEC_PRINTF("### ROOT MENU MODE ###\n");
							break;
						case CEC_USER_CONTROL_MODE_SETUP_MENU:
							CEC_PRINTF("### SETUP MENU MODE ###\n");
							break;
						case CEC_USER_CONTROL_MODE_PLAY:
							CEC_PRINTF("### PLAY MODE : PLAY ###\n");
							break;
						case CEC_USER_CONTROL_MODE_PAUSE:
							CEC_PRINTF("### PAUSE MODE : PAUSE ###\n");
							break;
						case CEC_USER_CONTROL_MODE_STOP:
							CEC_PRINTF("### STOP MODE : STOP ###\n");
							break;
						default:
							break;
					}

					dev->buffer.send_buf[0] = (dev->src_l_address << 4) | dev->dst_l_address;
					dev->buffer.size = 1;
					hdmitx_cec_send_msg(dev);
					break;

				case CEC_OPCODE_USER_CONTROL_RELEASED:
					CEC_PRINTF("### USER CONTROL RELEASED ###\n");
					break;

				case CEC_OPCODE_VENDOR_COMMAND:
					CEC_PRINTF("CEC_OPCODE_VENDOR_COMMAND\n");
						
					if(dev->buffer.recv_buf[2] == 0x1)
					{
						hdmitx_cec_report_power_status(dev);

						hdmitx_cec_simplelink_alive(dev);

						
					}
					else if(dev->buffer.recv_buf[2] == 0x4)
					{
						hdmitx_cec_simplelink_ack(dev);
										
					}
					else
					{
						hdmitx_cec_feature_abort(dev,CEC_OPCODE_VENDOR_COMMAND, CEC_ABORT_RESAON_INVALID);
					}
					break;
										
				case CEC_OPCODE_GIVE_OSD_NAME:
					hdmitx_cec_set_osd_name(dev);
					break;				

				case CEC_OPCODE_REQUEST_ACTIVE_SOURCE:
					/* responce with "Active Source" */
					hdmitx_cec_active_source(dev);
					break;

				case CEC_OPCODE_INACTIVE_SOURCE:
					hdmitx_cec_inactive_source(dev);
					break;

				case CEC_OPCODE_SET_STREAM_PATH:    // CEC Switches
				{
					int paddr_temp;

					paddr_temp = dev->buffer.recv_buf[2] << 8 | dev->buffer.recv_buf[3];

					//Temporary, we don't support different physical addr with EDID.
					if(!hdmitx_check_physical_address(dev,paddr_temp))
					continue;

					/* responce with "Active Source" */
					hdmitx_cec_active_source(dev);
					hdmitx_cec_MenuStatus(dev,0);	// 0 : activated, 1 : deactivated
                                                
					break;						
				}

				case CEC_OPCODE_MENU_REQUEST:
					CEC_PRINTF("### MENU REQUEST ###\n");

					switch(dev->buffer.recv_buf[2])
					{
						case 0:
							CEC_PRINTF("memu active\r\n");
							hdmitx_cec_MenuStatus(dev,0); // active;
							hdmitx_cec_active_source(dev);
							break;

						case 1:
							CEC_PRINTF("memu inactive\r\n");
							/* not response because of SAMSUNG TV : model 42LW6500
							hdmitx_cec_MenuStatus(dev,1); // inactive; */
							break;;

						case 2:
							CEC_PRINTF("memu qurery\r\n");
							hdmitx_cec_MenuStatus(dev,0); // active;
							break;
					}
					break;

				case CEC_OPCODE_GIVE_DECK_STATUS:
					/* respond with "Deck Status" */
					hdmitx_cec_deck_status(dev);
					break;						


				case CEC_OPCODE_ABORT:
				case CEC_OPCODE_FEATURE_ABORT:
				default:
					/* send "Feature Abort" */
					hdmitx_cec_feature_abort(dev, CEC_OPCODE_ABORT, CEC_ABORT_RESAON_UNRECOGNIZED);
					hdmitx_cec_feature_abort(dev, CEC_OPCODE_ABORT, CEC_ABORT_RESAON_UNRECOGNIZED);
					break;
			}
		}

		hdmitx_cec_set_connect_status(0);

		ALOGI("CECThread END!!!\n");

		hdmitx_cec_stop(dev);

		hdmitx_close_cec_device(dev);
        	    
		if(cec_func_error == 1) {
			cec_func_error = 0;
			dev->running_cec = 1;
		}

		value[0] = '0';
		property_set("tcc.cec.tv_status",value);

	}

	CEC_PRINTF("call pthread_exit\r\n");
	pthread_exit(NULL);
	return 0;
}

int hdmitx_init_cec(hdmitx_cec_t* dev)
{
	if(dev->thread_id != 0) {
		ALOGE(" CEC is already init..!!\r\n");
		return 0;
	}

	CEC_PRINTF("hdmitx_init_cec\r\n");
	dev->hdmi_cec_driver = -1;
	dev->thread_run = 1;
	if (pthread_create(&dev->thread_id, NULL, &hdmitx_cec_thread, (void *)dev)){
		dev->thread_id = 0;
		dev->thread_run = 0;
		ALOGE("hdmi cec thread init fail....\n");
		return -1;
	}

	return 0;
}

int hdmitx_deinit_cec(hdmitx_cec_t* dev)
{
	dev->running_cec = FALSE;
        
	if(dev->thread_id == 0) {
		ALOGE(" CEC is already de-init..!!\r\n");
		return 0;
	}

	dev->thread_run = 0;
	CEC_PRINTF("wait thread_join(0x%x)\r\n", dev->thread_id);
	if (pthread_join(dev->thread_id, NULL)) {
		ALOGE("pthread_join() failed!\n");
		return -1;
	}
	
	CEC_PRINTF("hdmitx_deinit_cec finish\r\n");
	dev->thread_id = 0;

	return 0;
}


int hdmitx_cec_suspend_check(hdmitx_cec_t* dev)
{
	int ret;

	ret = ioctl(dev->hdmi_cec_driver, CEC_IOC_GETRESUMESTATUS, NULL);
	return ret; // return value 0 : Resume, return value 1 : Suspend

}

int hdmitx_cmd_handle(hdmitx_cec_t* dev, int cmd, int flag)
{

	switch(cmd)
	{
		case CEC_OPCODE_IMAGE_VIEW_ON:
		case CEC_OPCODE_TEXT_VIEW_ON:
		{

			if(!dev->thread_run)
			{
				hdmitx_open_cec_device(dev);
				
				hdmitx_cec_start(dev);

				hdmitx_get_logical_address(dev);

				hdmitx_cec_image_view_on(dev);

				hdmitx_cec_stop(dev);

				hdmitx_close_cec_device(dev);

				property_set("tcc.cec.imageview_on", "0");

			}

			if(!dev->running_cec && dev->thread_run)
				property_set("tcc.cec.imageview_on", "0");
		}
		break;

		case CEC_OPCODE_ACTIVE_SOURCE:
		{
			hdmitx_cec_active_source(dev);
		}
		break;

		case CEC_OPCODE_INACTIVE_SOURCE:
		{
			hdmitx_cec_inactive_source(dev);
		}
		break;


		case CEC_OPCODE_STANDBY:
		{
			if(hdmitx_cec_suspend_check(dev))
			{
				hdmitx_cec_standBy(dev,flag);
			}
		}
		break;

		default:
		{
			ALOGE("command is not support.....\n");
		}
		
	}

	return 0;
	
}

int init_cec_parameter(void) 
{
        memset(&cecParm, 0, sizeof(cecParm));
        return 0;
}
        
int start_cec_process(unsigned short p_address, char *manufacturer_name)
{

        cecParm.p_address = p_address;
        if(manufacturer_name)
                memcpy(cecParm.vendor_info_name, manufacturer_name, sizeof(cecParm.vendor_info_name));	
	ALOGI(" >> CEC Device physical address is %X.%X.%X.%X\n",
	(cecParm.p_address & 0xF000) >> 12, (cecParm.p_address & 0x0F00) >> 8,
	(cecParm.p_address & 0x00F0) >> 4, cecParm.p_address & 0x000F);
        
	if(hdmitx_init_cec(&cecParm) < 0)
	{
		ALOGE("close hdmi cec deamon.... \n");
		return -1;
	}

	return 0;
		
}

int stop_cec_process(void)
{
	if(hdmitx_deinit_cec(&cecParm) < 0)
	{
		ALOGE("close hdmi cec deamon.... \n");
		return -1;
	}

	return 0;
		
}

int set_cec_status(hdmitx_cec_t* dev,int status)
{
	return ioctl(dev->hdmi_cec_driver, CEC_IOC_STATUS, &status);
}

int hdmi_get_cec_status(void)
{
        int cec_enable = property_get_int("persist.sys.hdmi_cec", 0);

        set_cec_status(&cecParm, cec_enable);

        return cec_enable;
}

void CEC_Func_Running(int running)
{
        if(running)
                cecParm.running_cec = TRUE;
        else
                cecParm.running_cec = FALSE;
}

int hdmi_cec_control_TV(int OnOff)
{
        int cmd = 0x00;

        if(OnOff) 	cmd = CEC_OPCODE_IMAGE_VIEW_ON;
        else		cmd = CEC_OPCODE_STANDBY;

        hdmitx_cmd_handle(&cecParm,cmd,CEC_MSG_BROADCAST);

        return 0;
}

