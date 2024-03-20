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

#ifdef __cplusplus
extern "C" {
#endif

#include <utils/Log.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>

#include <libhpd/libhpd.h>
#include <libedid/edid.h>
#include <libedid/libedid.h>
#include <libhdmi/libhdmi.h>
#include <libcec/libcec.h>
#include <libphy/libphy.h>
#if defined(TCC_HDMI_HDCP)
#include <libhdcp/libhdcp.h>
#endif

#include <utils/Log.h>
#include <utils/properties.h>
#if defined(HDMI_V1_3)
#include "mach/hdmi_1_3_hdmi.h"
#else
#include "mach/hdmi_1_4_hdmi.h"
#endif//

#include <mach/tccfb_ioctrl.h>
#define FB0_DEVICE	"/dev/fb0"

#define HDMI_APP_DEBUG      0
#define LOG_TAG				"-HDMI_APP-"
#if HDMI_APP_DEBUG
#define DPRINTF(args...)    ALOGI(args)
#else
#define DPRINTF(args...)
#endif

#ifndef TRUE
#define TRUE	1
#endif//
#ifndef FALSE
#define FALSE	0
#endif//

#if defined(NO_HPD_CEC_EDID)
#if defined(SUPPORT_SAFE_LINK)
#define TCC_FORCE_HDMI_RESOLUTION          V1920x720p_60Hz
#else
#define TCC_FORCE_HDMI_RESOLUTION          V1920x1080p_60Hz
#endif
#endif

#if defined(_TCC9200S_) || defined(TCC_HDMI_UI_SIZE_1280_720)
#define TCC_HDMI_DEFALT		v1280x720p_60Hz
#else
#define TCC_HDMI_DEFALT		v1920x1080p_60Hz
#endif

#define USE_VENDOR_ID
#define USE_COLORIMETRY

static int hpd_state = HPD_CABLE_OUT;
typedef struct {
	enum VideoFormat HdmiMode;
	tcc_display_size HdmiSize;
	int interlaced;
}tcc_hdmi;

enum HDMIResolutionIdx
{
	V1920x1080p_60Hz,
	V1920x1080p_50Hz,
	V1920x1080i_60Hz,
	V1920x1080i_50Hz,
	V1280x720p_60Hz,
	V1280x720p_50Hz,
	V720x576p_50Hz,
	V720x480p_60Hz,
	V640x480p_60Hz,
	V1920x720p_60Hz,

	AutoDetectMode = 125,
};


tcc_hdmi tcc_support_hdmi[] = {
	{v1920x1080p_60Hz,		{1920, 1080, 60},	0},		/* 0 */
	{v1920x1080p_50Hz,		{1920, 1080, 50},	0},		/* 1 */
 	{v1920x1080i_60Hz,		{1920, 1080, 60},	1},		/* 2 */
	{v1920x1080i_50Hz,		{1920, 1080, 50},	1},		/* 3 */
	{v1280x720p_60Hz,		{1280, 720, 60},	0},		/* 4 */
	{v1280x720p_50Hz,		{1280, 720, 50},	0},		/* 5 */
	{v720x576p_50Hz,		{720, 576, 50},		0},		/* 6 */
	{v720x480p_60Hz,		{720, 480, 60},		0},		/* 7 */
	{v640x480p_60Hz,		{640, 480, 60},		0},		/* 8 */

	{v1920x720p_60Hz,               {1920, 720, 60}, 0},                    /* 9 */
	{v1920x1080p_60Hz, 		{1920, 1080, 60},	0},		/* 10 */
 	{v1920x1080i_60Hz, 		{1920, 1080, 60},	1},		/* 11 */
	{v1920x1080p_50Hz, 		{1920, 1080, 50},	0},		/* 12 */
	{v1920x1080i_50Hz, 		{1920, 1080, 50},	1},		/* 13 */
	{v1920x1080p_23_976Hz,	{1920, 1080, 23.976},0},	/* 14 */
	{v1920x1080p_24Hz, 		{1920, 1080, 24},	0},		/* 15 */
	{v1920x1080p_25Hz, 		{1920, 1080, 25},	0},		/* 16 */
	{v1920x1080p_30Hz, 		{1920, 1080, 30},	0},		/* 17 */
 	{v1280x720p_60Hz, 		{1280, 720, 60},	0},		/* 18 */
	{v1280x720p_50Hz, 		{1280, 720, 50},	0},		/* 19 */
	{v720x576p_50Hz, 		{720, 576, 50},		0},		/* 20 */
	{v720x576i_50Hz, 		{720, 576, 50},		1},		/* 21 */
	{v720x480p_60Hz, 		{720, 480, 60},		0},		/* 22 */
	{v720x480i_60Hz, 		{720, 480, 60},		1},		/* 23 */
	{v640x480p_60Hz, 		{640, 480, 60},		0},		/* 24 */
	{v1920x1080p_29_97Hz, 	{1920, 1080, 29.97},0},		/* 25 */
        #if defined(HDMI_V1_4)
	{v1280x720p_60Hz_3D, 	{1280, 720, 60},	0},		/* 26 */
	{v1920x1080p_24Hz_3D, 	{1920, 1080, 24},	0},		/* 27 */
	#endif
};


enum SPDIFPort
{
	AUDIO_OUTPORT_DEFAULT,
	AUDIO_OUTPORT_SPDIF_PCM,
	AUDIO_OUTPORT_SPDIF_BITSTREAM,
	AUDIO_OUTPORT_DAI_LPCM,
	AUDIO_OUTPORT_DAI_HBR,
	SPDIF_MAX
};

enum HDMIAudioType
{
	AUDIO_HBR,
	AUDIO_ASP_DDP,
	AUDIO_ASP_DST_AC3,
	AUDIO_ASP_LPCM,
	AUDIO_ASP_PCM,
	AUDIO_MAX
};

enum HDMICecOneTouchPlayType
{
	CEC_IMAGE_VIEW_ON,
	CEC_TEXT_VIEW_ON,
	CEC_ONE_TOUCH_PLAY_MAX
};

typedef struct{
	unsigned int autodetect;
	unsigned int flag;
}TccHdmiResChk;





static pthread_t pthread_cec;
static int running_cec;
static int cec_first_working_status = 0;

static int menu_status_activated = 0;

static enum CECDeviceType devtype = CEC_DEVICE_PLAYER;

static int laddr;
static int paddr;
static int pVendorID;

static void *CECThread(void *arg);
static int CECThreadInit(void);
static int CECThreadDeInit(void);
static int CECFuncStart(void);
static int CECFuncStop(void);
int CECImageViewOn(void);
int CECTextViewOn(void);
int CECOneTouchPlay(int mode);
int CECActiveSource(void);
#if defined(USE_VENDOR_ID)
int CECMenuStatus(int state);
#endif
int CECInActiveSource(void);
int CECStandBy(int broadcast);
int CECCmdProcess(void);


static void *CECThread(void *arg)
{
    unsigned char *buffer = NULL;
    unsigned int cec_func_error = 0;
    unsigned int cec_orignial_address;
	unsigned int cec_new_address;

	running_cec = FALSE;


	printf("CECThread start !!\n");

	while(1)
	{
	    int size;
	    unsigned char lsrc, ldst, opcode;

	    while (!running_cec ) {
			usleep(1000000);
	    }

	    /* set to not valid physical address */
	    paddr = CEC_NOT_VALID_PHYSICAL_ADDRESS;

	    printf("%s\n", __FUNCTION__);

	//    blocksignals();

	    if (!EDIDOpen()) {
	        printf("EDIDInit() failed!\n");
	    }

	    if (!EDIDGetCECPhysicalAddress(&paddr)) {
	        printf("Error: EDIDGetCECPhysicalAddress() failed.\n");
	    }

		//Temporary....
	    if (!EDIDGetCECPhysicalAddress(&pVendorID)) {
	        printf("Error: EDIDGetCECPhysicalAddress() failed.\n");
	    }

	    printf("Device physical address is %X.%X.%X.%X\n",
	          (paddr & 0xF000) >> 12, (paddr & 0x0F00) >> 8,
	          (paddr & 0x00F0) >> 4, paddr & 0x000F);


	    if (!CECOpen()) {
	        printf("CECOpen() failed!!!\n");
	    }

	    if (!CECStart()) {
	        printf("CECStart() failed!!!\n");
	    }

	    /* a logical address should only be allocated when a device \
	       has a valid physical address, at all other times a device \
	       should take the 'Unregistered' logical address (15)
	    */

	    /* if physical address is not valid, device should take \
	       the 'Unregistered' logical address (15)
	    */

	    laddr = CECAllocLogicalAddress(paddr, devtype);

	    if (!laddr) {
	        printf("CECAllocLogicalAddress() failed!!!\n");
	    }

	    buffer = (unsigned char *)malloc(CEC_MAX_FRAME_SIZE);

		if(CECGetReumeStatus() || cec_first_working_status == 0) {
		    CECImageViewOn();
		    CECActiveSource();

			cec_first_working_status = 1;
		}

	    while (running_cec) {
			usleep(50000);

	        size = CECReceiveMessage(buffer, CEC_MAX_FRAME_SIZE, 50000);

	        //printf("CECReceiveMessag size:%d !!\n", size);

			if( size < 0 ) {
				printf("CEC Rx Error!!!!!!\n");
				cec_func_error = 1;
				running_cec = 0;
				continue;
			}

	        if (!size) { // no data available
	            continue;
	        }

	        if (size == 1) continue; // "Polling Message"

	        lsrc = buffer[0] >> 4;

			//printf("CEC lsrc %d laddr:%d  buffer[1]:%d !!\n", lsrc , laddr,  buffer[1]);

	        /* ignore messages with src address == laddr */
	        if (lsrc == laddr) continue;

	        opcode = buffer[1];

	        if (CECIgnoreMessage(opcode, lsrc)) {
	            printf("### ignore message coming from address 15 (unregistered)\n");
	            continue;
	        }

	        if (!CECCheckMessageSize(opcode, size)) {
	            printf("### invalid message size ###\n");
	            continue;
	        }

	        /* check if message broadcast/directly addressed */
	        if (!CECCheckMessageMode(opcode, (buffer[0] & 0x0F) == CEC_MSG_BROADCAST ? 1 : 0)) {
	            printf("### invalid message mode (directly addressed/broadcast) ###\n");
	            continue;
	        }

	        ldst = lsrc;

	//TODO: macroses to extract src and dst logical addresses
	//TODO: macros to extract opcode

	        switch (opcode) {

				case CEC_OPCODE_FEATURE_ABORT:
				{
					printf("rec: CEC_OPCODE_FEATURE_ABORT\r\n");
					continue;
				}

	            case CEC_OPCODE_GIVE_PHYSICAL_ADDRESS:
	            {
	                /* responce with "Report Physical Address" */
	                buffer[0] = (laddr << 4) | CEC_MSG_BROADCAST;
	                buffer[1] = CEC_OPCODE_REPORT_PHYSICAL_ADDRESS;
	                buffer[2] = (paddr >> 8) & 0xFF;
	                buffer[3] = paddr & 0xFF;
	                buffer[4] = devtype;
	                size = 5;
	                break;
	            }

				case CEC_OPCODE_GET_MENU_LANGUAGE:
				{
					buffer[0] = (laddr << 4) | ldst;
					buffer[1] = CEC_OPCODE_SET_MENU_LANGUAGE;
					buffer[2] = 'e';
					buffer[3] = 'n';
					buffer[4] = 'g';
					size = 5;
					break;
				}

	            case CEC_OPCODE_SET_MENU_LANGUAGE:
	            {
	                printf("the menu language will be changed!!!\n");
	                continue;
	            }

				case CEC_OPCODE_GIVE_DEVICE_POWER_STATUS:
				{
					printf("### POWER STATE : ON ###\n");
	                /* responce with "Report Physical Address" */
	                buffer[0] = (laddr << 4) | ldst ;
	                buffer[1] = CEC_OPCODE_REPORT_POWER_STATUS;
	                buffer[2] = CEC_POWER_STATUS_ON;
	                size = 3;
	                break;
				}

		        case CEC_OPCODE_REPORT_PHYSICAL_ADDRESS:// TV
		        case CEC_OPCODE_ACTIVE_SOURCE:        // TV, CEC Switches
		        case CEC_OPCODE_ROUTING_INFORMATION:    // CEC Switches
		        case CEC_OPCODE_SET_SYSTEM_AUDIO_MODE:    // TV
		        case CEC_OPCODE_DEVICE_VENDOR_ID:
		        case CEC_OPCODE_RECORD_OFF:
	                continue;

	            case CEC_OPCODE_ROUTING_CHANGE:        // CEC Switches
	            {
					cec_orignial_address = (((buffer[2] << 8) & 0xFF00) | (buffer[3] & 0xFF));
					cec_new_address = (((buffer[4] << 8) & 0xFF00) | (buffer[5] & 0xFF));

					printf("### cec_orignial_address = 0x%08x\n", cec_orignial_address);
					printf("### cec_new_address = 0x%08x\n", cec_new_address);

					// If a CEC Switch is at the new address, it send a <Routing Information> message to indicate its current active route.
	            	if(cec_new_address == paddr) {
	                /* responce with "Active Source" */
	                buffer[0] = (laddr << 4) | CEC_MSG_BROADCAST;
	                buffer[1] = CEC_OPCODE_ROUTING_INFORMATION;
				    buffer[2] = (paddr >> 8) & 0xFF;
				    buffer[3] = paddr & 0xFF;
				    size = 4;
				    break;
					} else {
						//Don't send <Routing Information>, because new address is not matched with our paddr.
						continue;
					}
	            }

				case CEC_OPCODE_GET_CEC_VERSION:
				{
	                /* responce with "Active Source" */
	                buffer[0] = (laddr << 4) | ldst;
	                buffer[1] = CEC_OPCODE_CEC_VERSION;
	                buffer[2] = 0x04;	//Version 1.3a
	                size = 3;
	                break;
				}

				case CEC_OPCODE_GET_DEVICE_VENDOR_ID:
				{

		#if defined(USE_VENDOR_ID)
					// This is for panasonic TV
					int VendorID = 0x00004C;

	                buffer[0] = (laddr << 4) | CEC_MSG_BROADCAST;
	                buffer[1] = CEC_OPCODE_DEVICE_VENDOR_ID;
	                buffer[2] = 0x00;
	                buffer[3] = (VendorID >> 8) & 0xFF;
	                buffer[4] = VendorID & 0xFF;
	                size = 5;
		#else
	                /* responce with "Active Source" */
	                buffer[0] = (laddr << 4) | CEC_MSG_BROADCAST;
	                buffer[1] = CEC_OPCODE_DEVICE_VENDOR_ID;
	                buffer[2] = 0x00;
	                buffer[3] = (pVendorID >> 8) & 0xFF;
	                buffer[4] = pVendorID & 0xFF;
	                size = 5;
		#endif
					break;
				}

	            case CEC_OPCODE_DECK_CONTROL:
				{
	                if (buffer[2] == CEC_DECK_CONTROL_MODE_STOP) {
	                    printf("### DECK CONTROL : STOP ###\n");
	                }
	                continue;
	            }

	            case CEC_OPCODE_PLAY:
				{
	                if (buffer[2] == CEC_PLAY_MODE_PLAY_FORWARD) {
	                    printf("### PLAY MODE : PLAY ###\n");
	                }
					else if (buffer[2] == CEC_PLAY_MODE_PLAY_STILL) {
						printf("### PAUSE MODE : PAUSE ###\n");

					}
	                continue;
	            }

	            case CEC_OPCODE_STANDBY:
				{
	                printf("### switching device into standby... ###\n");
	                continue;
	            }

				case CEC_OPCODE_USER_CONTROL_PRESSED:
				{

					printf("### USER CONTROL PRESSED ###\n");
					switch(buffer[2])
					{
						case CEC_USER_CONTROL_MODE_FAST_FORWARD:
	                    printf("### FAST FORWARD MODE : FFW ###\n");
							break;
						case CEC_USER_CONTROL_MODE_REWIND:
							printf("### REWIND MODE : REW ###\n");
							break;
						case CEC_USER_CONTROL_MODE_ROOT_MENU:
							printf("### ROOT MENU MODE ###\n");
							break;
						case CEC_USER_CONTROL_MODE_SETUP_MENU:
							printf("### SETUP MENU MODE ###\n");
							break;
						case CEC_USER_CONTROL_MODE_PLAY:
							printf("### PLAY MODE : PLAY ###\n");
							break;
						case CEC_USER_CONTROL_MODE_PAUSE:
							printf("### PAUSE MODE : PAUSE ###\n");
							break;
						case CEC_USER_CONTROL_MODE_STOP:
							printf("### STOP MODE : STOP ###\n");
							break;

						default:
							break;
					}

					continue;
	                }

	            case CEC_OPCODE_USER_CONTROL_RELEASED:
				{
					printf("### USER CONTROL RELEASED ###\n");

 	               continue;
					}

				case CEC_OPCODE_VENDOR_COMMAND:
				{
					printf("CEC_OPCODE_VENDOR_COMMAND\n");
	                //buffer[0] = (laddr << 4) | CEC_MSG_BROADCAST;
					buffer[0] = (laddr << 4) | ldst;
	                buffer[1] = CEC_OPCODE_FEATURE_ABORT;
					buffer[2] = CEC_OPCODE_VENDOR_COMMAND;
					buffer[3] = 3;
	                size = 4;
					break;
					}

				case CEC_OPCODE_GIVE_OSD_NAME:
				{
					printf("CEC_OPCODE_GIVE_OSD_NAME\n");
					if( ldst == CEC_LADDR_UNREGISTERED)
					continue;

	                buffer[0] = (laddr << 4) | ldst;
	                buffer[1] = CEC_OPCODE_SET_OSD_NAME;
	                buffer[2] = 'T';
	                buffer[3] = 'C';
					buffer[4] = 'P';
					buffer[5] = 'L';
					buffer[6] = 'A';
					buffer[7] = 'Y';
					buffer[8] = 'E';
					buffer[9] = 'R';
	                size = 10;
					break;
				}

	            case CEC_OPCODE_REQUEST_ACTIVE_SOURCE:
	            {
	                /* responce with "Active Source" */
	                buffer[0] = (laddr << 4) | CEC_MSG_BROADCAST;
	                buffer[1] = CEC_OPCODE_ACTIVE_SOURCE;
	                buffer[2] = (paddr >> 8) & 0xFF;
	                buffer[3] = paddr & 0xFF;
	                size = 4;
	                break;
	            }

				case CEC_OPCODE_SET_STREAM_PATH:    // CEC Switches
	            {
					int paddr_temp;

					paddr_temp = buffer[2] << 8 | buffer[3];

					//Temporary, we don't support different physical addr with EDID.
					if(!CECCheckPhysicalAddress(paddr_temp))
						continue;

					printf("### SET STREAM PATH : SEND ACTIVE SOURCE ###\n");
	                /* responce with "Active Source" */
	                buffer[0] = (laddr << 4) | CEC_MSG_BROADCAST;
	                buffer[1] = CEC_OPCODE_ACTIVE_SOURCE;
	                buffer[2] = (paddr >> 8) & 0xFF;
	                buffer[3] = paddr & 0xFF;
	                size = 4;

					break;
	            }

				case CEC_OPCODE_MENU_REQUEST:
				{
					printf("### MENU REQUEST ###\n");

					switch(buffer[2])
					{
						case 0:
							printf("memu active\r\n");
	                buffer[0] = (laddr << 4) | ldst ;
	                buffer[1] = CEC_OPCODE_MENU_STATUS;
							buffer[2] = 0;	// active;
	                size = 3;
			                menu_status_activated = 1;
							break;
						case 1:
							printf("memu inactive\r\n");
							/* not response because of SAMSUNG TV : model 42LW6500
	                buffer[0] = (laddr << 4) | ldst ;
	                buffer[1] = CEC_OPCODE_MENU_STATUS;
							buffer[2] = 1;	// inactive;
	                size = 3;
			                */
							continue;

						case 2:
	                buffer[0] = (laddr << 4) | ldst ;
	                buffer[1] = CEC_OPCODE_MENU_STATUS;
							buffer[2] = 0;	// active;
	                size = 3;
			                menu_status_activated = 0;
							printf("memu qurery\r\n");
							break;
					}
					break;
				}

	            default:
	            {
	                /* send "Feature Abort" */
	                buffer[0] = (laddr << 4) | ldst;
	                buffer[1] = CEC_OPCODE_FEATURE_ABORT;
	                buffer[2] = CEC_OPCODE_ABORT;
	                buffer[3] = 0x04; // "refused"
	                size = 4;
	            }
	        }

	//TODO: protect with mutex
	        if (CECSendMessage(buffer, size) != size) {
	            printf("CECSendMessage() failed!!!\n");
	        }

			if(menu_status_activated)
			{
				CECActiveSource();
				menu_status_activated = 0;
			}

		#if defined(USE_VENDOR_ID)
			// This is for panasonic TV
		        if(opcode == CEC_OPCODE_GET_DEVICE_VENDOR_ID  || opcode == CEC_OPCODE_SET_STREAM_PATH)
			{
				CECActiveSource();
				CECMenuStatus( 0 );	// 0 : activated, 1 : deactivated
	    }
		#endif
	    }

	    if(CECGetReumeStatus())
		    CECClrResuemStatus();

    	laddr = CEC_LADDR_UNREGISTERED;

		// If a device loses its Physical address at any time(e.g. it is unplugged),
		// then its local address should be set to 'Unregistered' (15)
	    if (CECSetLogicalAddr(laddr) < 0) {
	        printf("CECSetLogicalAddr() failed!\n");
	    }

		printf("CECThread END!!!\n");

	    if (buffer) {
	        free(buffer);
	    }

	    if (!EDIDClose()) {
	        printf("EDIDClose() failed!\n");
	    }

	    if (!CECStop()) {
	        printf("CECStop() failed!\n");
	    }

	    if (!CECClose()) {
	        printf("CECClose() failed!\n");
	    }

	    if(cec_func_error == 1) {
	    	cec_func_error = 0;
	    	running_cec = 1;
	    }

	}

    return NULL;
}


static int CECThreadInit(void)
{
        #if !defined(NO_HPD_CEC_EDID)
                running_cec = FALSE;
                if (pthread_create(&pthread_cec, NULL, &CECThread, NULL)) {
                        printf("pthread_create() failed!\n");
                        return 0;
                }
        #endif
        return 1;
}

static int CECThreadDeInit(void)
{
        #if !defined(NO_HPD_CEC_EDID)
        running_cec = FALSE;
        if (pthread_join(pthread_cec, NULL)) {
                printf("pthread_join() failed!\n");
                return 0;
        }
        #endif
        return 1;
}


static int CECFuncStart(void)
{
	running_cec = TRUE;

	return 1;
}


static int CECFuncStop(void)
{
	running_cec = FALSE;

	return 1;
}

int CECImageViewOn(void)
{
    int size;
    unsigned char buffer[CEC_MAX_FRAME_SIZE];

    if (laddr < 0)
        return 0;

    /* send "Image View On" */
    buffer[0] = (laddr << 4) | 0; // TV logical address is "0"
    buffer[1] = CEC_OPCODE_IMAGE_VIEW_ON;
    size = 2;

	//TODO: protect with mutex
    if (CECSendMessage(buffer, size) != size) {
        printf("CECSendMessage() failed!!!\n");
        return 0;
    }

    return 1;
}

int CECTextViewOn(void)
{
    int size;
    unsigned char buffer[CEC_MAX_FRAME_SIZE];

    if (laddr < 0)
        return 0;

    /* send "Image View On" */
    buffer[0] = (laddr << 4) | 0; // TV logical address is "0"
    buffer[1] = CEC_OPCODE_TEXT_VIEW_ON;
    size = 2;

//TODO: protect with mutex
    if (CECSendMessage(buffer, size) != size) {
        printf("CECSendMessage() failed!!!\n");
        return 0;
    }

    return 1;
}

int CECOneTouchPlay(int mode)
{
	switch(mode)
	{
		case CEC_IMAGE_VIEW_ON:
			CECImageViewOn();
			break;
		case CEC_TEXT_VIEW_ON:
			CECTextViewOn();
			break;
		default:
			break;
	}

    return 1;
}

#if defined(USE_VENDOR_ID)
int CECMenuStatus(int state)
{
    int size;
    unsigned char buffer[CEC_MAX_FRAME_SIZE];

    if (laddr < 0)
        return 0;

	buffer[0] = (laddr << 4) | 0 ;// TV logical address is "0"
	buffer[1] = CEC_OPCODE_MENU_STATUS;
	buffer[2] = state;	// 0 : activated, 1, deactivated
	size = 3;

//TODO: protect with mutex
    if (CECSendMessage(buffer, size) != size) {
        printf("CECSendMessage() failed!!!\n");
        return 0;
    }

    return 1;
}
#endif

int CECActiveSource(void)
{
    int size;
    unsigned char buffer[CEC_MAX_FRAME_SIZE];

    if (laddr < 0)
        return 0;

    /* send "Active Source" */
    buffer[0] = (laddr << 4) | CEC_MSG_BROADCAST;
    buffer[1] = CEC_OPCODE_ACTIVE_SOURCE;
    buffer[2] = (paddr >> 8) & 0xFF;
    buffer[3] = paddr & 0xFF;
    size = 4;

//TODO: protect with mutex
    if (CECSendMessage(buffer, size) != size) {
        printf("CECSendMessage() failed!!!\n");
        return 0;
    }

    return 1;
}


int CECInActiveSource(void)
{
    int size;
    unsigned char buffer[CEC_MAX_FRAME_SIZE];

    if (laddr < 0)
        return 0;

    /* send "Active Source" */
    buffer[0] = (laddr << 4) | 0; // TV logical address is "0"
    buffer[1] = CEC_OPCODE_INACTIVE_SOURCE;
    buffer[2] = (paddr >> 8) & 0xFF;
    buffer[3] = paddr & 0xFF;
    size = 4;

//TODO: protect with mutex
    if (CECSendMessage(buffer, size) != size) {
        printf("CECSendMessage() failed!!!\n");
        return 0;
    }

	return 1;
}

int CECStandBy(int broadcast)
{
    int size;
    unsigned char buffer[CEC_MAX_FRAME_SIZE];

    if (laddr < 0)
        return 0;

    /* send "Image View On" */
    buffer[0] = (laddr << 4) | broadcast;
    buffer[1] = CEC_OPCODE_STANDBY;
    size = 2;

//TODO: protect with mutex
    if (CECSendMessage(buffer, size) != size) {
        printf("CECSendMessage() failed!!!\n");
        return 0;
    }

    return 1;

}

int CECCmdProcess(void)
{
	char value[PROPERTY_VALUE_MAX];
	unsigned int cec_enable = 0, uiLen = 0;

	memset(value, NULL, PROPERTY_VALUE_MAX);

	uiLen = property_get("persist.sys.hdmi_cec", value, "");

	if( uiLen )
		cec_enable = atoi(value);
	else
		cec_enable = 0;

	if(cec_enable == 1) {

		unsigned int uiCecImageViewOn = 0, uiCecTextViewOn = 0;
		unsigned int uiCecActiveSource = 0, uiCecInActiveSource = 0;
		unsigned int uiCecStandBy = 0;

		property_get("tcc.cec.imageview_on", value, "");
		uiCecImageViewOn = atoi(value);
		if(uiCecImageViewOn)
		{
			CECOneTouchPlay(CEC_IMAGE_VIEW_ON);
			value[0] = '0';
			property_set("tcc.cec.imageview_on", value);
		}

		property_get("tcc.cec.textview_on", value, "");
		uiCecTextViewOn = atoi(value);
		if(uiCecTextViewOn)
		{
			CECOneTouchPlay(CEC_IMAGE_VIEW_ON);
			value[0] = '0';
			property_set("tcc.cec.textview_on", value);
		}

		property_get("tcc.cec.active_source", value, "");
		uiCecActiveSource = atoi(value);
		if(uiCecActiveSource)
		{
			CECActiveSource();
			value[0] = '0';
			property_set("tcc.cec.active_source", value);
		}

		property_get("tcc.cec.in_active_source", value, "");
		uiCecInActiveSource = atoi(value);
		if(uiCecInActiveSource)
		{
			CECInActiveSource();
			value[0] = '0';
			property_set("tcc.cec.in_active_source", value);
		}

		property_get("tcc.cec.standby", value, "");
		uiCecStandBy = atoi(value);
		if(uiCecStandBy == 1)
		{
			CECStandBy(0);
			value[0] = '0';
			property_set("tcc.cec.standby", value);
		}
		else if(uiCecStandBy == 2)
		{
			CECStandBy(CEC_MSG_BROADCAST);
			value[0] = '0';
			property_set("tcc.cec.standby", value);
		}

	}

	return 1;
}

static pthread_t pthread_pbc;
static int pthread_start = 0;

static int PBCThreadInit(void);
static int PBCThreadDeInit(void);
static void *PBCThread(void *arg);


static int PBCThreadInit(void)
{
	if(pthread_start == 0)
	{
		pthread_start = 1;

	    if (pthread_create(&pthread_pbc, NULL, &PBCThread, NULL)) {
	        printf("pthread_create() failed!\n");
	        return 0;
	    }
	}

    return 1;
}

static int PBCThreadDeInit(void)
{
	if(pthread_start == 1)
	{
		pthread_start = 0;

	    if (pthread_join(pthread_pbc, NULL)) {
	        printf("pthread_join() failed!\n");
	        return 0;
	    }
	}

    return 1;
}

static void *PBCThread(void *arg)
{
	char			value[PROPERTY_VALUE_MAX];
	memset(value, NULL, PROPERTY_VALUE_MAX);

	int fd = -1, ret = 0, auto_resolution = 0, uiLen = 0;
	char buff;


	fd = open("/dev/wps_pbc", O_RDONLY );

	if (fd < 0) {
		ALOGE("Cannot open \"%s\": /dev/wps_pbc", __func__ );
		return NULL;
	}

	ALOGE("PBCThread is start \n");

	while(1) {
		usleep(250000);

		if(auto_resolution == 1)
			continue;

		uiLen = property_get("persist.sys.auto_resolution", value, "");

		if( uiLen ) {
			auto_resolution = atoi(value);
		}
		else {
			ALOGE("persist.sys.auto_resolution is NULL \n");
		}

		if( auto_resolution == 0 && uiLen ) {
			ret = read(fd, &buff, 1);

			if( buff == '0' )    { // push button

				ALOGE("button pushed---------------------------------");

				value[0] = '1';
				property_set("persist.sys.auto_resolution", value);
			}
		}
	}

	ALOGE("PBCThread is end");

	close(fd);

	//exit(0);
}


unsigned int hdmi_get_spdif_setting(void);
unsigned int hdmi_get_audio_type(void);
HDMIAudioPort hdmi_get_AudioInputPort(void);
ColorDepth hdmi_get_ColorDepth(void);
ColorSpace hdmi_get_ColorSpace(void);
int hdmi_set_ColorSpace(int color_space);
int hdmi_get_HDCPEnableStatus(void);
PixelAspectRatio hdmi_get_PixelAspectRatio(void);
#if defined(HDMI_V1_4)
HDMI3DVideoStructure hdmi_get_hdmi_3d_format(void);
#endif
SamplingFreq hdmi_get_AudioSamplingRate(void);
HDMIASPType hdmi_get_AudioOutPacket(void);
void hdmi_set_output_detected(unsigned int detected);
void hdmi_set_hdmi_resolution(unsigned int resolution);
void hdmi_set_detected_resolution(unsigned int resolution);

void hdmi_get_video(struct HDMIVideoParameter *HDMIvideo);
void hdmi_get_audio(struct HDMIAudioParameter *HDMIaudio);
void hdmi_set_video(struct HDMIVideoParameter HDMIvideo);
void hdmi_set_audio(struct HDMIAudioParameter HDMIaudio);
int hdmi_video_output_set(struct HDMIVideoParameter *HDMIvideo);
int hdmi_audio_output_set(struct HDMIAudioParameter *HDMIaudio);
int hdmi_check_resolution(struct HDMIVideoParameter *tcc_video, unsigned int *hdmi_mode_idx, TccHdmiResChk *HdmiResChk );
int hdmi_update_resolution(struct HDMIVideoParameter *tcc_video);

struct HDMIVideoParameter video;
struct HDMIAudioParameter audio;

static int gHDMIAudioOutput = 0;

int hdmi_supportmodeset(unsigned int idx)
{
	#define FB_DEV_NAME		"/dev/fb0"

	int fb_fd = -1;
	if ((fb_fd = open(FB_DEV_NAME, O_RDWR)) < 0)	{
		DPRINTF("can not open \"%s\"\n", FB_DEV_NAME);
		return 0;
	}
	ioctl( fb_fd, TCC_LCDC_HDMI_SET_SIZE, &tcc_support_hdmi[idx].HdmiSize);

	close(fb_fd);

	return 0;
}

int hdmi_set_DVILUT(int enable)
{
	#define FB_DEV_NAME		"/dev/fb0"

	int fb_fd = -1;
	lut_ctrl_params lut_ctrl_params_dvi;

	if ((fb_fd = open(FB_DEV_NAME, O_RDWR)) < 0)	{
		DPRINTF("can not open \"%s\"\n", FB_DEV_NAME);
		return 0;
	}

	if(enable)
		lut_ctrl_params_dvi.onoff = 1;
	else
		lut_ctrl_params_dvi.onoff = 0;

	ioctl( fb_fd, TCC_LCDC_SET_LUT_DVI, &lut_ctrl_params_dvi);

	close(fb_fd);

	return 0;
}

#if defined(USE_HDCP_CALL_BACK) && defined(TCC_HDMI_HDCP)
static void HDCPcallback(int state)
{
	int hdcp_state = 0;
	char			value[PROPERTY_VALUE_MAX];

	memset(value, NULL, PROPERTY_VALUE_MAX);


	if(hdmi_get_HDCPEnableStatus()) {
		if( state == HDCP_EVENT_SECOND_AUTH_START)
			hdcp_state = 1;
		else
			hdcp_state = 0;
	} else {
		hdcp_state = 0;
	}

	DPRINTF("%s : state = %d, hdcp_state = %d\n", __func__, state, hdcp_state);

	if(hdcp_state) {
		value[0] = '1';
		property_set("tcc.hdcp.state", value);
	} else {
		value[0] = '0';
		property_set("tcc.hdcp.state", value);
	}

}
#endif

static void HPDcallback(int state)
{
    int hpd_before_state = hpd_state;

    if (state == HPD_CABLE_IN)   {
       hpd_state = HPD_CABLE_IN;
    }
    else if (state == HPD_CABLE_OUT)   {
        hpd_state = HPD_CABLE_OUT;
    }
    else {
        DPRINTF("state is unknown!!!\n");
    }

	if(hpd_before_state != hpd_state)
	{
		DPRINTF("HDP STATE CHANGE :%d", hpd_state);
	}
}

extern unsigned int dispman_daemon_stbmode;

int HDMI_GetVideoResolution(void)
{
	#if defined(NO_HPD_CEC_EDID)
	// NO HPD/CEC/E-EDID
        return TCC_FORCE_HDMI_RESOLUTION;
	#else
	int iret, ifixed;
	char value[PROPERTY_VALUE_MAX];
	memset(value, NULL, PROPERTY_VALUE_MAX);

	property_get("persist.sys.hdmi_resolution", value, "");
	iret = atoi(value);

	memset(value, NULL, PROPERTY_VALUE_MAX);

	// check 720p fixed mode by system property.
	property_get("tcc.all.hdmi.720p.fixed", value, "");
	ifixed = atoi(value);

	if((ifixed == 1) && ((tcc_support_hdmi[iret].HdmiSize.width > 1280)  || (iret == AutoDetectMode)))
	{

		return 2;
	}

	#if defined(_TCC9200S_)
	if(iret == 0)	{
		iret = 1;
	}
	#endif//(_TCC9200S_)


	#if defined(HDMI_V1_4)
	if (dispman_daemon_stbmode)
	{
		int temp_resolution;
		property_get("tcc.video.hdmi_resolution", value, "999");

		temp_resolution = atoi(value);
		if(temp_resolution!=999 && (temp_resolution>=0 && temp_resolution <=26 ))
		{
			//ALOGI("change HDMI resolution to %d from tcc.video.hdmi_resolution",temp_resolution );
			return temp_resolution;
		}
	}
	#endif

	return iret;
	#endif
}


// 0 : HDMI mode , 1 : HDMI,DVI mode auto select
int HDMI_GetDviMode(void)
{
	int iret;
	char value[PROPERTY_VALUE_MAX];
	memset(value, NULL, PROPERTY_VALUE_MAX);

	property_get("persist.sys.hdmi_auto_select", value, "");

	iret = atoi(value);
	DPRINTF("%s :%d !!!\n",__func__, iret);

	return iret;
}



int hdmi_display_init(char hdmi_reset)
{
	struct HDMIVideoParameter tcc_video;
	struct HDMIAudioParameter tcc_audio;
	unsigned int		uiSPDIFSetting = 0;
	SamplingFreq    	iAudioSamplingRate = SF_32KHZ;
	unsigned int		uiHDMIAudioType=0;

	memset((void *)&tcc_video, NULL, sizeof(tcc_video));
	memset((void *)&tcc_audio, NULL, sizeof(tcc_audio));

        #if defined(NO_HPD_CEC_EDID)
        ALOGI("%s NO_HDMI_CEC_EDID\r\n", __func__);
        #else
        ALOGI("%s HDMI FULL FUNCTION\r\n", __func__);
        #endif
	ALOGI("%s HDMI Item:%d", __func__, (sizeof(tcc_support_hdmi)/sizeof(tcc_support_hdmi[0])));

	if (!HPDSetCallback(&HPDcallback)) {
		printf("HPDSetCallback() failed!\n");
		return -1;
	}

	HDMIFBOpen();

	tcc_video.mode = HDMI;
	tcc_video.resolution = TCC_HDMI_DEFALT;
	tcc_video.colorSpace = hdmi_get_ColorSpace();
	tcc_video.colorDepth = hdmi_get_ColorDepth();
	tcc_video.colorimetry = HDMI_COLORIMETRY_NO_DATA;
	tcc_video.pixelAspectRatio = hdmi_get_PixelAspectRatio();
	#if defined(HDMI_V1_4)
	tcc_video.videoSrc = HDMI_SOURCE_EXTERNAL;
	tcc_video.hdmi_3d_format = hdmi_get_hdmi_3d_format();
	#endif

	hdmi_set_video(tcc_video);

	if(hdmi_reset)
		hdmi_supportmodeset((sizeof(tcc_support_hdmi)/sizeof(tcc_support_hdmi[0])) - 1);

	uiSPDIFSetting = hdmi_get_spdif_setting();
	iAudioSamplingRate = hdmi_get_AudioSamplingRate();
	uiHDMIAudioType = hdmi_get_audio_type();

	switch(uiSPDIFSetting)
	{
		case AUDIO_OUTPORT_DEFAULT:
			tcc_audio.inputPort = I2S_PORT;
			tcc_audio.outPacket = HDMI_ASP;
			tcc_audio.formatCode = LPCM_FORMAT;
			tcc_audio.channelNum = CH_2;
			tcc_audio.sampleFreq = iAudioSamplingRate;
			break;
		case AUDIO_OUTPORT_SPDIF_PCM:
		case AUDIO_OUTPORT_SPDIF_BITSTREAM:
			tcc_audio.inputPort = SPDIF_PORT;
			tcc_audio.outPacket = HDMI_ASP;
			tcc_audio.formatCode = LPCM_FORMAT;
			tcc_audio.channelNum = CH_2;
			tcc_audio.sampleFreq = iAudioSamplingRate;
			break;
		case AUDIO_OUTPORT_DAI_LPCM:
			tcc_audio.inputPort = I2S_PORT;
			tcc_audio.outPacket = HDMI_ASP;
			tcc_audio.formatCode = LPCM_FORMAT;
			tcc_audio.channelNum = CH_8;
			tcc_audio.sampleFreq = iAudioSamplingRate;
			break;
		case AUDIO_OUTPORT_DAI_HBR:
			if(uiHDMIAudioType == AUDIO_HBR){
			tcc_audio.inputPort = I2S_PORT;
			tcc_audio.outPacket = HDMI_HBR;
			tcc_audio.formatCode = DTS_HD_FORMAT;
			tcc_audio.channelNum = CH_8;
			tcc_audio.sampleFreq = iAudioSamplingRate;
			}
			else if(uiHDMIAudioType == AUDIO_ASP_LPCM){
				tcc_audio.inputPort = I2S_PORT;
				tcc_audio.outPacket = HDMI_ASP;
				tcc_audio.formatCode = LPCM_FORMAT;
				tcc_audio.channelNum = CH_8;
				tcc_audio.sampleFreq = iAudioSamplingRate;
			}
			else{
				tcc_audio.inputPort = I2S_PORT;
				tcc_audio.outPacket = HDMI_ASP;
				tcc_audio.formatCode = LPCM_FORMAT;
				tcc_audio.channelNum = CH_2;
				tcc_audio.sampleFreq = iAudioSamplingRate;
			}

			break;
		default:
			break;
	}

	tcc_audio.wordLength = WORD_16;			//WORD_24;
	tcc_audio.i2sParam.bpc = I2S_BPC_16;	//I2S_BPC_24;
	tcc_audio.i2sParam.format = I2S_BASIC;
	tcc_audio.i2sParam.clk = I2S_64FS;
	hdmi_set_audio(tcc_audio);

	CECThreadInit();

#if 0
	if(HDMIGetRunStatus() && hdmi_reset)
	{
		HDMIStop();
		HDMISetPowerControl(0);
		HDMI_lcdc_stop();
	}
#endif

    return 0;
}

int hdmi_display_deinit(void)
{
	DPRINTF("%s", __func__);
	HDMIFBClose();
    return 0;
}
unsigned int hdmi_lcdc_check(void)
{
	unsigned int hdmi_check;
	hdmi_check = HDMI_LcdcCheck();
	return hdmi_check;
}

unsigned int hdmi_suspend_check(void)
{
	unsigned int hdmi_suspend;
	hdmi_suspend = HDMI_SuspendCheck();
	//ALOGE("%s :  hdmi_suspend = %d\n", __func__, hdmi_suspend);
	return hdmi_suspend;
}


/*---------- HPD DETECT ON/OFF -------------- */
int hdmi_display_detect_onoff(char onoff)
{
	unsigned int pwr_state;
	DPRINTF("%s onoff:%d !!\n", __func__, onoff);

	if(onoff)
	{
		if (!HDMIOpen()) {
			ALOGE("fail to open hdmi!!\n");
			return NULL;
		}

		HDMISet5vPowerControl(1);

		#if !defined(NO_HPD_CEC_EDID)
		#if defined(TCC_HDMI_HDCP)
		HDMIGetPowerStatus(&pwr_state);

		if(!pwr_state)
			HDMISetPowerControl(1);

		if (!HDCPOpen()) {
                        ALOGE("fail to open hdcp!!\n");
                        return NULL;
                }
		#if defined(USE_HDCP_CALL_BACK)
		if (!HDCPSetCallback(&HDCPcallback)) {
			printf("HPDSetCallback() failed!\n");
			return -1;
		}

		HDCPcallback(0);
		#endif /* USE_HDCP_CALL_BACK */
		#endif /* TCC_HDMI_HDCP */

		if (!HPDOpen()) {
			ALOGE("HPDOpen() failed!\n");
			return NULL;
		}

		HPDStart();

	    if (!EDIDOpen())   {
	        DPRINTF("fail to open edid!!\n");
			return NULL;
	    }

		//PBCThreadInit();
		#if defined(TCC_HDMI_HDCP)
		HDCPStart();
		#endif
		#endif  // NO_HPD_CEC_EDID
	}
	else
	{
		#if !defined(NO_HPD_CEC_EDID)
		#if defined(TCC_HDMI_HDCP)
		HDCPStop();
		#endif

		EDIDClose();

		if (!HPDStop()) {
			ALOGE("HPDStop() failed!\n");
		}

		if (!HPDClose()) {
			ALOGE("HPDClose() failed!\n");
		}

		#if defined(TCC_HDMI_HDCP)
		HDCPClose();

		HDMIGetPowerStatus(&pwr_state);

		if(pwr_state)
			HDMISetPowerControl(0);
		#endif /* TCC_HDMI_HDCP */
		#endif // NO_HPD_CEC_EDID
		HDMISet5vPowerControl(0);


		HDMIClose();

		//PBCThreadDeInit();
	}
	return 0;
}

unsigned int hdmi_get_spdif_setting(void)
{
	char			value[PROPERTY_VALUE_MAX];
	unsigned int	uiSPDIFSetting;
	memset(value, NULL, PROPERTY_VALUE_MAX);

    property_get("persist.sys.spdif_setting", value, "");
	uiSPDIFSetting = atoi(value);

	return uiSPDIFSetting;
}

unsigned int hdmi_get_audio_type(void)
{
	char			value[PROPERTY_VALUE_MAX];
	unsigned int	uiHDMIAudioType=0;
	memset(value, NULL, PROPERTY_VALUE_MAX);

    property_get("tcc.hdmi.audio_type", value, "");
	if(value[0] >='0' && value[0] < '5'){
		uiHDMIAudioType = atoi(value);
	}
	else{
		uiHDMIAudioType = 0;
	}


	return uiHDMIAudioType;
}

unsigned int hdmi_get_hdmi_link(void)
{
	char			value[PROPERTY_VALUE_MAX];
	unsigned int	uiHDMIlink;
	memset(value, NULL, PROPERTY_VALUE_MAX);

    property_get("tcc.audio.hdmi.link", value, "0");
	uiHDMIlink = atoi(value);

	return uiHDMIlink;
}

void hdmi_set_output_detected(unsigned int detected)
{
	char			value[PROPERTY_VALUE_MAX];
	memset(value, NULL, PROPERTY_VALUE_MAX);

	if( detected )
		value[0] = '1';
	else {
		value[0] = '0';
		property_supported_resolution_clear();
	}

	property_set("persist.sys.hdmi_detected", value);
}

void hdmi_set_hdmi_resolution(unsigned int resolution)
{
	char			value[PROPERTY_VALUE_MAX];
	memset(value, NULL, PROPERTY_VALUE_MAX);

	value[0] = '0' + resolution;

	property_set("persist.sys.hdmi_resolution", value);
}

void hdmi_set_detected_resolution(unsigned int resolution)
{
	char			value[PROPERTY_VALUE_MAX];
	memset(value, NULL, PROPERTY_VALUE_MAX);

	value[0] = '0' + resolution;

	property_set("persist.sys.hdmi_detected_res", value);
}

void hdmi_set_detected_mode(unsigned int mode)
{
	char			value[PROPERTY_VALUE_MAX];
	memset(value, NULL, PROPERTY_VALUE_MAX);

	//persist.sys.hdmi_detected_mode : HDMI 0, DVI 1
	if( mode )
		value[0] = '0';
	else
		value[0] = '1';

	property_set("persist.sys.hdmi_detected_mode", value);
}


HDMIAudioPort hdmi_get_AudioInputPort(void)
{
	unsigned int		uiSPDIFSetting = 0;
	HDMIAudioPort	iAudioInputPort;

	uiSPDIFSetting = hdmi_get_spdif_setting();
	//DPRINTF("uiSPDIFSetting = %d", uiSPDIFSetting);
	switch(uiSPDIFSetting)
	{
		case AUDIO_OUTPORT_DEFAULT:
        {
            unsigned int hdmi_link;
            hdmi_link = hdmi_get_hdmi_link();

            if(hdmi_link)
				iAudioInputPort = SPDIF_PORT;
            else
				iAudioInputPort = I2S_PORT;
            break;
		}
        case AUDIO_OUTPORT_DAI_LPCM:
		case AUDIO_OUTPORT_DAI_HBR:
		iAudioInputPort = I2S_PORT;
			break;
		case AUDIO_OUTPORT_SPDIF_PCM:
		case AUDIO_OUTPORT_SPDIF_BITSTREAM:
		iAudioInputPort = SPDIF_PORT;
			break;
		default:
			iAudioInputPort = I2S_PORT;
			break;
	}

	return iAudioInputPort;
}

ColorDepth hdmi_get_ColorDepth(void)
{
	char          value[PROPERTY_VALUE_MAX];
	ColorDepth    iColorDepth;
	unsigned int uiLen = 0, iColorDepthIdx;
	memset(value, NULL, PROPERTY_VALUE_MAX);

	uiLen = property_get("persist.sys.hdmi_color_depth", value, "");

	if( uiLen ) {
		iColorDepthIdx = atoi(value);
		switch(iColorDepthIdx)
		{
			case 0:
				iColorDepth = HDMI_CD_24;
				break;
			case 1:
				iColorDepth = HDMI_CD_30;
				break;
			case 2:
				iColorDepth = HDMI_CD_36;
				break;
			default:
				iColorDepth = HDMI_CD_24;
				break;
		}
	} else {
		iColorDepth = HDMI_CD_24;
	}

	//DPRINTF("iColorDepth = %d", iColorDepth);

	return iColorDepth;
}

ColorSpace hdmi_get_ColorSpace(void)
{
	char          value[PROPERTY_VALUE_MAX];
	ColorSpace    iColorSpace;
	unsigned int  uiLen = 0;
	memset(value, NULL, PROPERTY_VALUE_MAX);

	uiLen = property_get("persist.sys.hdmi_color_space", value, "");

	if(uiLen) {
		iColorSpace = (ColorSpace)atoi(value);
	} else {
		iColorSpace = HDMI_CS_RGB;
	}
	//DPRINTF("iColorSpace = %d", iColorSpace);

	return iColorSpace;
}

int hdmi_set_ColorSpace(int color_space)
{
	char			value[PROPERTY_VALUE_MAX];
	memset(value, NULL, PROPERTY_VALUE_MAX);

	value[0] = '0' + color_space;

	property_set("persist.sys.hdmi_color_space", value);

	return 1;
}


int hdmi_get_HDCPEnableStatus(void)
{
	char          value[PROPERTY_VALUE_MAX];
	int			  iHDCPEnableStatus;
	unsigned int  uiLen = 0;
	memset(value, NULL, PROPERTY_VALUE_MAX);

	uiLen = property_get("persist.sys.hdcp1x_enable", value, "");

	if(uiLen) {
		iHDCPEnableStatus = atoi(value);
		if(iHDCPEnableStatus == 0) {
			iHDCPEnableStatus = 0;
		} else {
			iHDCPEnableStatus = 1;
		}
	} else {
		iHDCPEnableStatus = 1;
	}
	//DPRINTF("iHDCPEnableStatus = %d", iHDCPEnableStatus);

	return iHDCPEnableStatus;
}


PixelAspectRatio hdmi_get_PixelAspectRatio(void)
{
	char          		value[PROPERTY_VALUE_MAX];
	PixelAspectRatio    iPixelAspectRatio;
	unsigned int  		uiLen = 0, iPixelAspectRatioIdx = 0;
	memset(value, NULL, PROPERTY_VALUE_MAX);

	uiLen = property_get("persist.sys.hdmi_aspect_ratio", value, "");

	if(uiLen) {
		iPixelAspectRatioIdx = atoi(value);
		switch(iPixelAspectRatioIdx)
		{
			case 0:
				iPixelAspectRatio = HDMI_PIXEL_RATIO_16_9;
				break;
			case 1:
				iPixelAspectRatio = HDMI_PIXEL_RATIO_4_3;
				break;
			default:
				iPixelAspectRatio = HDMI_PIXEL_RATIO_16_9;
				break;
		}
	} else {
		iPixelAspectRatio = HDMI_PIXEL_RATIO_16_9;
	}
	//DPRINTF("iPixelAspectRatio = %d", iPixelAspectRatio);

	return iPixelAspectRatio;
}

/*
	// 2D Video Format
	HDMI_2D_VIDEO_FORMAT = -1,
	// 3D Frame Packing Structure
	HDMI_3D_FP_FORMAT = 0,
	// 3D Field Alternative Structure
	HDMI_3D_FA_FORMAT,
	// 3D Line Alternative Structure
	HDMI_3D_LA_FORMAT,
	// Side-by-Side(Full)Structure
	HDMI_3D_SSF_FORMAT,
	// 3D L+Depth Structure
	HDMI_3D_LD_FORMAT,
	// 3D L+Depth+Graphics Structure
	HDMI_3D_LDGFX_FORMAT,
	// 3D Top-and-Bottom Structure
	HDMI_3D_TB_FORMAT,
	// HDMI VIC Structure (ex. 4Kx2K)
	HDMI_VIC_FORMAT,
	// Side-by-Side(Half)Structure
	HDMI_3D_SSH_FORMAT,
*/
#if defined(HDMI_V1_4)
HDMI3DVideoStructure hdmi_get_hdmi_3d_format(void)
{
	char          		value[PROPERTY_VALUE_MAX];
	HDMI3DVideoStructure    iHDMI3DVideoFormat;
	unsigned int  		uiLen = 0, iHDMI3DVideoFormatIdx = 0;
	memset(value, NULL, PROPERTY_VALUE_MAX);

	uiLen = property_get("tcc.output.hdmi_3d_format", value, "");

	if(uiLen) {
		iHDMI3DVideoFormatIdx = atoi(value);
		switch(iHDMI3DVideoFormatIdx)
		{
			case 0:
				iHDMI3DVideoFormat = HDMI_2D_VIDEO_FORMAT;
				break;
			case 1:
				iHDMI3DVideoFormat = HDMI_3D_FP_FORMAT;
				break;
			case 2:
				iHDMI3DVideoFormat = HDMI_3D_FA_FORMAT;
				break;
			case 3:
				iHDMI3DVideoFormat = HDMI_3D_LA_FORMAT;
				break;
			case 4:
				iHDMI3DVideoFormat = HDMI_3D_SSF_FORMAT;
				break;
			case 5:
				iHDMI3DVideoFormat = HDMI_3D_LD_FORMAT;
				break;
			case 6:
				iHDMI3DVideoFormat = HDMI_3D_LDGFX_FORMAT;
				break;
			case 7:
				iHDMI3DVideoFormat = HDMI_3D_TB_FORMAT;
				break;
			case 8:
				iHDMI3DVideoFormat = HDMI_VIC_FORMAT;
				break;
			case 9:
				iHDMI3DVideoFormat = HDMI_3D_SSH_FORMAT;
				break;
			default:
				iHDMI3DVideoFormat = HDMI_2D_VIDEO_FORMAT;
				break;
		}
	} else {
		iHDMI3DVideoFormat = HDMI_2D_VIDEO_FORMAT;
	}

	//DPRINTF("iHDMI3DVideoFormat = %d", iHDMI3DVideoFormat);

 	return iHDMI3DVideoFormat;
}
#endif

SamplingFreq hdmi_get_AudioSamplingRate(void)
{
	char          value[PROPERTY_VALUE_MAX];
	SamplingFreq  iAudioSamplingRate;
	memset(value, NULL, PROPERTY_VALUE_MAX);

	property_get("tcc.audio.sampling_rate", value, "");	// SPDIF Planet 20121107
    iAudioSamplingRate = (SamplingFreq)atoi(value);
	//DPRINTF("samping_rate = %d", iAudioSamplingRate);

	return iAudioSamplingRate;
}

ChannelNum hdmi_get_AudioChannels(void)
{
	char          value[PROPERTY_VALUE_MAX];

	// SPDIF Planet 20121107 Start
    property_get("tcc.audio.channels", value, "");

    switch(value[0]) {
        default:
        case '2':
            return CH_2;
        case '3':
            return CH_3;
        case '4':
            return CH_4;
        case '5':
            return CH_5;
        case '6':
            return CH_6;
        case '7':
            return CH_7;
        case '8':
            return CH_8;
    }
	// SPDIF Planet 20121107 End

    return CH_2;
}


HDMIASPType hdmi_get_AudioOutPacket(void)
{
	HDMIASPType		iOutPacket;
	unsigned int	uiSPDIFSetting;

	uiSPDIFSetting = hdmi_get_spdif_setting();
	//DPRINTF("uiSPDIFSetting = %d", uiSPDIFSetting);
	switch(uiSPDIFSetting)
	{
		case AUDIO_OUTPORT_DEFAULT:
		case AUDIO_OUTPORT_SPDIF_PCM:
		case AUDIO_OUTPORT_SPDIF_BITSTREAM:
			iOutPacket = HDMI_ASP;
			break;
		case AUDIO_OUTPORT_DAI_LPCM:
            iOutPacket = HDMI_ASP;
			break;
		case AUDIO_OUTPORT_DAI_HBR:
			if(hdmi_get_audio_type() ==AUDIO_HBR)
			iOutPacket = HDMI_HBR;
			else
				iOutPacket = HDMI_ASP;
            break;
		default:
			iOutPacket = HDMI_ASP;
			break;
	}

	return iOutPacket;
}

int hdmi_AudioOnOffChk(void)
{
	int hdmi_audio_onoff;
	char          value[PROPERTY_VALUE_MAX];

	property_get("tcc.output.hdmi_audio_onoff", value, "");
	hdmi_audio_onoff = atoi(value);

	if( hdmi_audio_onoff == 2 )
	{
		//Audio Stop
		HDMIAudioStop();

		value[0] = '1';
		property_set("tcc.output.hdmi_audio_disable", value);
	}
	else if( hdmi_audio_onoff == 1 )
	{
		// Audio Start
		HDMIAudioStart();

		value[0] = '0';
		property_set("tcc.output.hdmi_audio_disable", value);
	}

	value[0] = '0';
	property_set("tcc.output.hdmi_audio_onoff", value);

	return 1;
}


int hdmi_AudioInputPortDetect(void)
{
	struct HDMIAudioParameter tcc_audio;
	char value[PROPERTY_VALUE_MAX];

	HDMIAudioPort	iAudioInputPort;
	HDMIASPType		iOutPacket;
    SamplingFreq    iAudioSamplingRate;
	unsigned int		uiSPDIFSetting = 0;
    ChannelNum      iChannelNum;
	unsigned int	uiAudioSettingisChanged = 0;
	unsigned int	uiHDMIAudioType=0;
	unsigned int hdmi_audio_off = 0;

	hdmi_get_audio(&tcc_audio);

	iAudioInputPort     = hdmi_get_AudioInputPort();
    iAudioSamplingRate  = hdmi_get_AudioSamplingRate();
	iOutPacket          = hdmi_get_AudioOutPacket();
    iChannelNum         = hdmi_get_AudioChannels();
	uiSPDIFSetting      = hdmi_get_spdif_setting();
	uiHDMIAudioType = hdmi_get_audio_type();

	if( (iAudioInputPort != tcc_audio.inputPort) || (iOutPacket != tcc_audio.outPacket) || (iAudioSamplingRate != tcc_audio.sampleFreq) || (uiSPDIFSetting != gHDMIAudioOutput) )
	{
		switch(uiSPDIFSetting)
		{
			case AUDIO_OUTPORT_DEFAULT:
				//ASP : PCM 2Ch only
   				tcc_audio.inputPort = iAudioInputPort;
				tcc_audio.outPacket = HDMI_ASP;
				tcc_audio.formatCode = LPCM_FORMAT;
				tcc_audio.channelNum = CH_2;
				tcc_audio.sampleFreq = iAudioSamplingRate;
				break;
			case AUDIO_OUTPORT_SPDIF_PCM:
			case AUDIO_OUTPORT_SPDIF_BITSTREAM:
				//SPDIF : PCM 2Ch or AC3, DTS Pass-thru up to 5.1Ch
				tcc_audio.inputPort = SPDIF_PORT;
				tcc_audio.outPacket = HDMI_ASP;
				tcc_audio.formatCode = LPCM_FORMAT;
				tcc_audio.channelNum = CH_2;
				tcc_audio.sampleFreq = iAudioSamplingRate;
				break;
    		case AUDIO_OUTPORT_DAI_LPCM:
    			//not use in this case
				tcc_audio.inputPort = I2S_PORT;
				tcc_audio.outPacket = HDMI_ASP;
				tcc_audio.formatCode = LPCM_FORMAT;
				tcc_audio.channelNum = iChannelNum;
				tcc_audio.sampleFreq = iAudioSamplingRate;
				break;
    		case AUDIO_OUTPORT_DAI_HBR:
				if(uiHDMIAudioType == AUDIO_HBR){
					//DTSHD-MA Pass-thru to 7.1Ch
				tcc_audio.inputPort = I2S_PORT;
				tcc_audio.outPacket = HDMI_HBR;
				tcc_audio.formatCode = DTS_HD_FORMAT;
				tcc_audio.channelNum = CH_8;
				tcc_audio.sampleFreq = iAudioSamplingRate;
				} else if(uiHDMIAudioType == AUDIO_ASP_LPCM || uiHDMIAudioType == AUDIO_ASP_PCM){
					//PCM 2Ch or LPCM Pass-thru up to 7.1Ch
					tcc_audio.inputPort = I2S_PORT;
					tcc_audio.outPacket = HDMI_ASP;
					tcc_audio.formatCode = LPCM_FORMAT;
					tcc_audio.channelNum = iChannelNum;
					tcc_audio.sampleFreq = iAudioSamplingRate;
				} else{
					//DTS, AC3, DDP, AAC Pass-thru up to 5.1Ch
					tcc_audio.inputPort = I2S_PORT;
					tcc_audio.outPacket = HDMI_ASP;
					tcc_audio.formatCode = DTS_FORMAT;
					tcc_audio.channelNum = CH_2;
					tcc_audio.sampleFreq = iAudioSamplingRate;
				}

				break;
			default:
				break;
		}

		gHDMIAudioOutput = uiSPDIFSetting;
		uiAudioSettingisChanged = true;
    }

	if( uiAudioSettingisChanged == true)
	{
		//Audio Stop
		HDMIAudioStop();
		//Audio Setting
		hdmi_audio_output_set(&tcc_audio);

		memset(value, NULL, PROPERTY_VALUE_MAX);
		property_get("tcc.output.hdmi_audio_disable", value, "");
		hdmi_audio_off = atoi(value);

		if( hdmi_audio_off )
		{
			HDMIAudioStop();
		}
		else
		{

			#if defined(CONFIG_VIDEO_HDMI_IN_SENSOR)
			ALOGI("~~~~!!! HDMI-In Audio Start !!!~~~~");
			HDMIAudioStart();
			#else
			if( uiSPDIFSetting == AUDIO_OUTPORT_SPDIF_BITSTREAM || uiSPDIFSetting == AUDIO_OUTPORT_SPDIF_PCM) {
				//Audio Start
				HDMIAudioStart();
			} else {
				//Audio Start
				HDMIAudioStart();
			}
			#endif

		}

        ALOGI("\t======================================");
        ALOGI("\t= AutioType    [%d]", uiHDMIAudioType);
        ALOGI("\t= inputport    [%d]", tcc_audio.inputPort);
		ALOGI("\t= outPacket    [%d]", tcc_audio.outPacket);
        ALOGI("\t= samplingrate [%d]", tcc_audio.sampleFreq);
		ALOGI("\t= Ch           [%d]", tcc_audio.channelNum);
		ALOGI("\t= formatCode   [%d]", tcc_audio.formatCode);
		ALOGI("\t= AudoOutput   [%d]", uiSPDIFSetting);
        ALOGI("\t======================================");

		return 0;
	}


	return 1;
}




void hdmi_get_video(struct HDMIVideoParameter *HDMIvideo)
{
	*HDMIvideo = video;
}

void hdmi_get_audio(struct HDMIAudioParameter *HDMIaudio)
{
	*HDMIaudio = audio;
}

void hdmi_set_video(struct HDMIVideoParameter HDMIvideo)
{
	video = HDMIvideo;

	#if defined(HDMI_V1_4)
	DPRINTF("hdmi_set_video: mode:%d resolution:%d colorDepth:%d colorimetry:%d colorSpace:%d pixelAspectRatio:%d videosrc:%d, hdmi_3d_format:%d\n",
			video.mode, video.resolution, video.colorDepth, video.colorimetry, video.colorSpace, video.pixelAspectRatio, video.videoSrc, video.hdmi_3d_format);
	#else
	DPRINTF("hdmi_set_video: mode:%d resolution:%d colorDepth:%d colorimetry:%d colorSpace:%d pixelAspectRatio:%d \n",
			video.mode, video.resolution, video.colorDepth, video.colorimetry, video.colorSpace, video.pixelAspectRatio);
	#endif
}

void hdmi_set_audio(struct HDMIAudioParameter HDMIaudio)
{
	audio = HDMIaudio;

	DPRINTF("hdmi_set_audio: inputPort:%d outPacket:%d formatCode:%d channelNum:%d sampleFreq:%d wordLength:%d i2sParam:%d \n",
					audio.inputPort, audio.outPacket, audio.formatCode, audio.channelNum,
					audio.sampleFreq, audio.wordLength, audio.i2sParam);
}

int hdmi_video_output_set(struct HDMIVideoParameter *HDMIvideo)
{
	hdmi_set_video(*HDMIvideo);

	#if defined(HDMI_V1_4)

	#else
	if (!HDMISetHDMIMode(HDMIvideo->mode))
	{
		DPRINTF("fail to set video mode!!\n");
		return NULL;
	}
	#endif

	if (!HDMISetVideoMode(HDMIvideo))
	{
		DPRINTF("fail to set video mode!!\n");
		return NULL;
	}

	return 1;
}

int hdmi_audio_output_set(struct HDMIAudioParameter *HDMIaudio)
{
	hdmi_set_audio(*HDMIaudio);

	if (!HDMISetAudioMode(HDMIaudio))
	{
		DPRINTF("fail to set audio mode!!\n");
		return NULL;
	}
	return 1;
}

int hdmi_check_resolution(struct HDMIVideoParameter *tcc_video, unsigned int *hdmi_mode_idx, TccHdmiResChk *HdmiResChk )
{
	#if defined(NO_HPD_CEC_EDID)
        *hdmi_mode_idx = TCC_FORCE_HDMI_RESOLUTION;
	HdmiResChk->flag = 1;
	#else
	int  i = 0, iflag = 0;

	if( HdmiResChk->autodetect )
	{
		for(i =0; i < (sizeof(tcc_support_hdmi)/sizeof(tcc_support_hdmi[0])); i++)
		{
			if( tcc_video->mode == HDMI)
			{
				if(CheckResolution(tcc_support_hdmi[i].HdmiMode, HDMI_PIXEL_RATIO_16_9))
				{
					tcc_video->resolution = tcc_support_hdmi[i].HdmiMode;
					tcc_video->pixelAspectRatio = HDMI_PIXEL_RATIO_16_9;
					iflag = true;
					break;
				}

				if(CheckResolution(tcc_support_hdmi[i].HdmiMode, HDMI_PIXEL_RATIO_4_3))
				{
					tcc_video->resolution = tcc_support_hdmi[i].HdmiMode;
					tcc_video->pixelAspectRatio = HDMI_PIXEL_RATIO_4_3;
					iflag = true;
					break;
				}
			}
			else if( tcc_video->mode == DVI)
			{
				if( CheckResolutionOfDVI( tcc_support_hdmi[i].HdmiMode, HDMI_PIXEL_RATIO_16_9, tcc_support_hdmi[i].HdmiSize.width, tcc_support_hdmi[i].HdmiSize.height, tcc_support_hdmi[i].HdmiSize.frame_hz ))
				{
					tcc_video->resolution = tcc_support_hdmi[i].HdmiMode;
					tcc_video->pixelAspectRatio = HDMI_PIXEL_RATIO_16_9;
					iflag = true;
					break;
				}

				if( CheckResolutionOfDVI( tcc_support_hdmi[i].HdmiMode, HDMI_PIXEL_RATIO_4_3, tcc_support_hdmi[i].HdmiSize.width, tcc_support_hdmi[i].HdmiSize.height, tcc_support_hdmi[i].HdmiSize.frame_hz ))
				{
					tcc_video->resolution = tcc_support_hdmi[i].HdmiMode;
					tcc_video->pixelAspectRatio = HDMI_PIXEL_RATIO_4_3;
					iflag = true;
					break;
				}
			}
		}
	}
	else
	{
		if( tcc_video->mode == HDMI)
		{
			if(CheckResolution(tcc_support_hdmi[*hdmi_mode_idx].HdmiMode, HDMI_PIXEL_RATIO_16_9))
			{
				iflag = true;
			}
		}
		else if( tcc_video->mode == DVI)
		{
			if( CheckResolutionOfDVI( tcc_support_hdmi[*hdmi_mode_idx].HdmiMode, HDMI_PIXEL_RATIO_16_9, tcc_support_hdmi[*hdmi_mode_idx].HdmiSize.width, tcc_support_hdmi[*hdmi_mode_idx].HdmiSize.height, tcc_support_hdmi[*hdmi_mode_idx].HdmiSize.frame_hz ))
			{
				iflag = true;
			}
		}
	}

	if( iflag == true )
	{
		if( HdmiResChk->autodetect )
			*hdmi_mode_idx = i;
		HdmiResChk->flag = true;
	}
	else
	{
		*hdmi_mode_idx = V1280x720p_60Hz;
		HdmiResChk->flag = false;
	}
	#endif // NO_HPD_CEC_EDID
	return 1;
}

/**
 * Check if Rx supports requested 3D format.
 * @param   none
 * @return  Hdmi3DStructure

	See HDMI spec Table 8-19 3D_Structure_ALL
		3D_Structure_ALL_0 : Sink supports "Frame packing" 3D formats
		3D_Structure_ALL_6 : Sink supports "Top-and-Bottom" 3D formats
		3D_Structure_ALL_8 : Sink supports "Side-by-Side(Half) with horizontal sub-sampling" 3D formats
 */
int hdmi_check_3DFormatSupport(void)
{
	#if !defined(NO_HPD_CEC_EDID)
	unsigned int HDMI3DStructure_All;
	unsigned int mode;
	char			value[PROPERTY_VALUE_MAX];
	memset(value, NULL, PROPERTY_VALUE_MAX);

	HDMI3DStructure_All = Check3DFormatSupport();

	ALOGI("%s, HDMI3DStructure_All = %x\n", __func__, HDMI3DStructure_All);

	if(HDMI3DStructure_All & (1 << EDID_3D_STRUCTURE_FP)) {
		ALOGI("Sink supports Frame packing 3D formats\n");
		value[0] = '1';

		property_set("tcc.hdmi.sink_support_3d_fp", value);
	} else {
		value[0] = '0';
		ALOGI("Sink not supports Frame packing 3D formats\n");
		property_set("tcc.hdmi.sink_support_3d_fp", value);
	}

	if(HDMI3DStructure_All & (1 << EDID_3D_STRUCTURE_TB)) {
		ALOGI("Sink supports Top-and-Bottom 3D formats\n");
		value[0] = '1';

		property_set("tcc.hdmi.sink_support_3d_tnb", value);
	} else {
		ALOGI("Sink not supports Top-and-Bottom 3D formats\n");
		value[0] = '0';
		property_set("tcc.hdmi.sink_support_3d_tnb", value);
	}

	if(HDMI3DStructure_All & (1 << EDID_3D_STRUCTURE_TB)) {
		ALOGI("Sink supports Side-by-Side(Half) with horizontal sub-sampling 3D formats\n");
		value[0] = '1';

		property_set("tcc.hdmi.sink_support_3d_sbs", value);
	} else {
		ALOGI("Sink not supports Side-by-Side(Half) with horizontal sub-sampling 3D formats\n");
		value[0] = '0';
		property_set("tcc.hdmi.sink_support_3d_sbs", value);
	}
 	#endif
	//EDID3DFormatSupport(&tcc_video);

	return 1;
}

int hdmi_update_resolution(struct HDMIVideoParameter *tcc_video)
{
	int i = 0;
	int supported = 0;
	int dataindex = 0;
	char data[SR_ATTR_MAX] = {0};

	DPRINTF("hdmi_update_resolution\r\n");

	for(i =0; i < (int)(sizeof(tcc_support_hdmi)/sizeof(tcc_support_hdmi[0])); i++)
	{
		supported = 0;
		if( tcc_video->mode == HDMI)
		{
			if(CheckResolution(tcc_support_hdmi[i].HdmiMode, HDMI_PIXEL_RATIO_16_9))
			{
				supported = 1;
			}
			else if(CheckResolution(tcc_support_hdmi[i].HdmiMode, HDMI_PIXEL_RATIO_4_3))
			{
				supported = 1;
			}
		}
		else if( tcc_video->mode == DVI)
		{
			if( CheckResolutionOfDVI( tcc_support_hdmi[i].HdmiMode, HDMI_PIXEL_RATIO_16_9, tcc_support_hdmi[i].HdmiSize.width, tcc_support_hdmi[i].HdmiSize.height, tcc_support_hdmi[i].HdmiSize.frame_hz ))
			{
				supported = 1;
			}
			else if( CheckResolutionOfDVI( tcc_support_hdmi[i].HdmiMode, HDMI_PIXEL_RATIO_4_3, tcc_support_hdmi[i].HdmiSize.width, tcc_support_hdmi[i].HdmiSize.height, tcc_support_hdmi[i].HdmiSize.frame_hz ))
			{
				supported = 1;
			}
		}

		if(supported) {
			dataindex += sprintf(data+dataindex, "%02d:%04dx%04d@%02d%c ",
			i, tcc_support_hdmi[i].HdmiSize.width,
			tcc_support_hdmi[i].HdmiSize.height, tcc_support_hdmi[i].HdmiSize.frame_hz,
			tcc_support_hdmi[i].interlaced?'I':'P');
		}
		if(tcc_support_hdmi[i].HdmiMode == v640x480p_60Hz) break;
	}

	property_supported_resolution_set(data);

	return 1;
}

int hdmi_compare_resolution(int width, int height)
{
	unsigned int hdmi_mode_idx;
	int ret = 0;
	char automode = 0;
	char value[PROPERTY_VALUE_MAX];

	struct HDMIVideoParameter tcc_video;
	TccHdmiResChk HdmiResChk;

	memset(value, NULL, PROPERTY_VALUE_MAX);

	hdmi_get_video(&tcc_video);

	property_get("persist.sys.hdmi_mode", value, "");
	if(atoi(value) == 0)
		tcc_video.mode = HDMI;
	else
		tcc_video.mode = DVI;
	tcc_video.resolution = TCC_HDMI_DEFALT;
	tcc_video.colorSpace = hdmi_get_ColorSpace();
	tcc_video.colorDepth = hdmi_get_ColorDepth();
	tcc_video.colorimetry = HDMI_COLORIMETRY_NO_DATA;
	tcc_video.pixelAspectRatio = hdmi_get_PixelAspectRatio();

	hdmi_mode_idx = HDMI_GetVideoResolution();

	if(hdmi_mode_idx == AutoDetectMode)
	{
		automode = 1;
		hdmi_mode_idx = 0;
	}

	tcc_video.resolution = tcc_support_hdmi[hdmi_mode_idx].HdmiMode;

	 if(tcc_video.resolution ==v1920x720p_60Hz)
                   tcc_video.mode = DVI;

	if(automode)
	{
		HdmiResChk.autodetect = true;
		hdmi_check_resolution(&tcc_video, &hdmi_mode_idx, &HdmiResChk);
	}

	if((width == tcc_support_hdmi[hdmi_mode_idx].HdmiSize.width) && (height == tcc_support_hdmi[hdmi_mode_idx].HdmiSize.height))
		ret = 1;

	ALOGI("%s, ret=%d, hdmi_setting=%d, cur_width=%d, cur_height=%d, set_width=%d, set_height=%d", __func__, ret, hdmi_mode_idx, width, height, tcc_support_hdmi[hdmi_mode_idx].HdmiSize.width, tcc_support_hdmi[hdmi_mode_idx].HdmiSize.height);

	return ret;
}

int hdmi_display_output_set(char onoff)
{
	unsigned int hdmi_mode_idx, video_diff, audio_diff;
	unsigned int cec_enable, uiLen = 0;
	char automode = 0;
	unsigned int pwr_state = 0;


	char value[PROPERTY_VALUE_MAX];
	memset(value, NULL, PROPERTY_VALUE_MAX);


	struct HDMIVideoParameter tcc_video, tcc_video_before;
	struct HDMIAudioParameter tcc_audio, tcc_audio_before;

	TccHdmiResChk HdmiResChk;


	DPRINTF("%s onoff:%d", __func__, onoff);

	if(onoff)
	{
		int hdmi_output_mode, uiLen;

		#if !defined(TCC_HDMI_HDCP)
        HDMIGetPowerStatus(&pwr_state);


		if(!pwr_state)
			HDMISetPowerControl(1);
		#endif /* TCC_HDMI_HDCP */

		//HDMIDefaultPhyConfig();

		//default is HDMI
		hdmi_get_video(&tcc_video);

		tcc_video.mode = HDMI;

		uiLen = property_get("persist.sys.hdmi_mode", value, "");

		if(uiLen)
			hdmi_output_mode = atoi(value);
		else
			hdmi_output_mode = 0;

		//persist.sys.hdmi_mode - Auto 125, HDMI:0, DVI:1
		if(hdmi_output_mode == 0) {
			tcc_video.mode = HDMI;
		} else if(hdmi_output_mode == 1) {
			tcc_video.mode = DVI;
		} else if(hdmi_output_mode == AutoDetectMode) {
			#if defined(NO_HPD_CEC_EDID)
			tcc_video.mode = HDMI;
			#else
			//For HDMI CTS
			if (!EDIDHDMIModeSupportForCTS(&tcc_video)) {
				tcc_video.mode = DVI;
				ALOGI("RX support DVI mode  tcc_video.mode = %d \n", tcc_video.mode);
			} else {
				tcc_video.mode = HDMI;
				ALOGI("RX support HDMI mode  tcc_video.mode = %d \n", tcc_video.mode);
			}
			#endif
			hdmi_set_detected_mode(tcc_video.mode);
		} else {
			//Default HDMI
			tcc_video.mode = HDMI;
		}

		if(hdmi_get_ColorSpace() == AutoDetectMode)
		{
			#if defined(NO_HPD_CEC_EDID)
			tcc_video.colorSpace = HDMI_CS_YCBCR444;
			#else
			//For HDMI CTS
			tcc_video.colorSpace = CheckColorSpaceWithEDID();
			#endif
		} else {
			tcc_video.colorSpace = hdmi_get_ColorSpace();
		}

		tcc_video.resolution = TCC_HDMI_DEFALT;
		tcc_video.colorDepth = hdmi_get_ColorDepth();
		tcc_video.colorimetry = HDMI_COLORIMETRY_NO_DATA;
		tcc_video.pixelAspectRatio = hdmi_get_PixelAspectRatio();
		#if defined(HDMI_V1_4)
		tcc_video.videoSrc = HDMI_SOURCE_EXTERNAL;
		tcc_video.hdmi_3d_format = hdmi_get_hdmi_3d_format();
		#endif

		hdmi_get_audio(&tcc_audio);

		tcc_audio.inputPort = hdmi_get_AudioInputPort();
		tcc_audio.sampleFreq = hdmi_get_AudioSamplingRate();

		hdmi_mode_idx = HDMI_GetVideoResolution();

		DPRINTF("%s hdmi_mode_idx:%d automode:%d", __func__, hdmi_mode_idx, automode);

		if(hdmi_mode_idx == AutoDetectMode) //auto mode
		{
			automode = 1;
			hdmi_mode_idx = 0;
		}

		tcc_video.resolution = tcc_support_hdmi[hdmi_mode_idx].HdmiMode;

		if(tcc_video.resolution == v1920x720p_60Hz)
			tcc_video.mode = DVI;

		// redkakeru
		hdmi_update_resolution(&tcc_video);

		if(automode)
		{
			HdmiResChk.autodetect = true;
			hdmi_check_resolution(&tcc_video, &hdmi_mode_idx, &HdmiResChk);
			tcc_video.resolution =  tcc_support_hdmi[hdmi_mode_idx].HdmiMode;
			hdmi_set_detected_resolution(hdmi_mode_idx);

			DPRINTF("Auto Detected mode:(%d) Resolution ::(%d) ratio:(%d) \n", tcc_video.mode, tcc_video.resolution, tcc_video.pixelAspectRatio);
		}
		else
		{
			tcc_video.resolution =  tcc_support_hdmi[hdmi_mode_idx].HdmiMode;

			ALOGI("Selected mode:(%d) Resolution (%d) ratio:(%d) \n", tcc_video.mode, tcc_video.resolution, tcc_video.pixelAspectRatio);
		}

		//hdmi_check_3DFormatSupport();

		if(tcc_video.mode == DVI || tcc_video.colorSpace == HDMI_CS_RGB) {
			//if not support YCbCr444/YCbCr422 at Rx, RGB color space is default.
			tcc_video.colorSpace = HDMI_CS_RGB;
			hdmi_set_DVILUT(TRUE);
		} else {
			hdmi_set_DVILUT(FALSE);
		}

		#if defined(USE_COLORIMETRY)
		if(tcc_video.colorSpace == HDMI_CS_YCBCR444 || tcc_video.colorSpace == HDMI_CS_YCBCR422) {
			switch(tcc_video.resolution)
			{
				case v1920x1080p_60Hz:
				case v1920x1080p_50Hz:
			 	case v1920x1080i_60Hz:
				case v1920x1080i_50Hz:
				case v1280x720p_60Hz:
				case v1280x720p_50Hz:
					tcc_video.colorimetry = HDMI_COLORIMETRY_ITU709;
					ALOGI("colorimetry is set to HDMI_COLORIMETRY_ITU709 \n");
					break;
				case v720x576p_50Hz:
				case v720x480p_60Hz:
				case v640x480p_60Hz:
					tcc_video.colorimetry = HDMI_COLORIMETRY_ITU601;
					ALOGI("colorimetry is set to HDMI_COLORIMETRY_ITU601 \n");
					break;
				default:
					tcc_video.colorimetry = HDMI_COLORIMETRY_NO_DATA;
					ALOGI("colorimetry is set to HDMI_COLORIMETRY_NO_DATA \n");
					break;
			}
		}
		#endif

		hdmi_supportmodeset(hdmi_mode_idx);

		HDMIGetVideoMode(&tcc_video_before);
		HDMIGetAudioMode(&tcc_audio_before);

		video_diff = memcmp(&tcc_video, &tcc_video_before, sizeof(struct HDMIVideoParameter));
		audio_diff = memcmp(&tcc_audio, &tcc_audio_before, sizeof(struct HDMIAudioParameter));
		ALOGD("HDMI DIFF  video : %d , audio : %d ", video_diff, audio_diff);

		if(video_diff || audio_diff)
		{
			#if 0
			if(HDMIGetRunStatus())
			{
				HDMIStop();
				HDMI_lcdc_stop();

			}
			#endif

			hdmi_video_output_set(&tcc_video);
			hdmi_audio_output_set(&tcc_audio);

			usleep(100000);

			HDMIStart();

			#if defined(TCC_HDMI_HDCP)
			usleep(1000);
			HDCPEnable();
			#endif /* TCC_HDMI_HDCP */
		}

		hdmi_set_output_detected(true);

		ALOGD("HDMI VIDEO before: mode:%d resolution:%d colorDepth:%d colorimetry:%d colorSpace:%d pixelAspectRatio:%d \n",
						tcc_video_before.mode, tcc_video_before.resolution, tcc_video_before.colorDepth, tcc_video_before.colorimetry, tcc_video_before.colorSpace, tcc_video_before.pixelAspectRatio);

		ALOGD("HDMI AUDIO before: inputPort:%d outPacket:%d formatCode:%d channelNum:%d sampleFreq:%d wordLength:%d i2sParam:%d \n",
						tcc_audio_before.inputPort, tcc_audio_before.outPacket, tcc_audio_before.formatCode, tcc_audio_before.channelNum, tcc_audio_before.sampleFreq, tcc_audio_before.wordLength, tcc_audio_before.i2sParam);


		ALOGD("HDMI VIDEO           : mode:%d resolution:%d colorDepth:%d colorimetry:%d colorSpace:%d pixelAspectRatio:%d \n",
						tcc_video.mode, tcc_video.resolution, tcc_video.colorDepth, tcc_video.colorimetry, tcc_video.colorSpace, tcc_video.pixelAspectRatio);

		ALOGD("HDMI AUDIO           : inputPort:%d outPacket:%d formatCode:%d channelNum:%d sampleFreq:%d wordLength:%d i2sParam:%d \n",
						tcc_audio.inputPort, tcc_audio.outPacket, tcc_audio.formatCode, tcc_audio.channelNum, tcc_audio.sampleFreq, tcc_audio.wordLength, tcc_audio.i2sParam);

        #if !defined(NO_HPD_CEC_EDID)
		uiLen = property_get("persist.sys.hdmi_cec", value, "");

		if( uiLen )
			cec_enable = atoi(value);
		else
			cec_enable = 0;
		if(cec_enable == 1) {
			CECFuncStart();
			ALOGD("CEC enable : %d \n", cec_enable);
		}
        #endif
	}
	else
	{
		unsigned int pwr_state;

		HDMIGetPowerStatus(&pwr_state);
		if(pwr_state)
		{
			#if !defined(NO_HPD_CEC_EDID)
			uiLen = property_get("persist.sys.hdmi_cec", value, "");

			if( uiLen)
				cec_enable = atoi(value);
			else
				cec_enable = 0;

			if(cec_enable == 1) {
				CECFuncStop();
				usleep(500000);
				ALOGD("CEC disable : %d \n", cec_enable);
			}
			#if defined(TCC_HDMI_HDCP)
			HDCPDisable();
			#endif /* TCC_HDMI_HDCP */
			#endif // NO_HPD_CEC_EDID
			HDMIStop();
		}


		HDMI_lcdc_stop();

		//EDIDReset(); - Panasonic TH-P50VT2

		hdmi_set_output_detected(false);

		#if !defined(TCC_HDMI_HDCP)
		HDMIGetPowerStatus(&pwr_state);

		if(pwr_state)
			HDMISetPowerControl(0);
		#endif /* TCC_HDMI_HDCP */
	}
	return 0;
}


int hdmi_display_cabledetect(void)
{
	int hdmi_hpd = 0;


	#if HDMI_APP_DEBUG
	static int prev_hpd = -1;
	#endif

	#if defined(NO_HPD_CEC_EDID)
	hdmi_hpd = 1;
	#else
	hdmi_hpd = HPDCheck();
        #endif // NO_HPD_CEC_EDID
	HPDcallback(hdmi_hpd);

	#if HDMI_APP-DEBUG
	if(prev_hpd != hdmi_hpd) {
	        DPRINTF("%s %d", __func__, hpd_state);
	        prev_hpd = hdmi_hpd;
	}
	#endif

	return hpd_state;
}

int hdmi_set_video_format_ctrl(unsigned int HdmiVideoFormat, unsigned int Structure_3D)
{
	HDMISetVideoFormatCtrl(HdmiVideoFormat, Structure_3D);

	return 0;
}
/*
 tcc.hdmi.screen_status
 0 : hdmi screen off
 1 : hdmi screen on
 If it is not exist, don't work.

 */
 unsigned int g_last_screen_status = 1;
int hdmi_cmd_process(void)
{
	#if !defined(NO_HPD_CEC_EDID)
	char value[PROPERTY_VALUE_MAX];
	unsigned int current_screen_status = 1, uiLen = 0, hdmi_running = 1;

	memset(value, NULL, PROPERTY_VALUE_MAX);

	uiLen = property_get("tcc.hdmi.screen_status", value, "");

	if( uiLen ) {
		current_screen_status = atoi(value);
	} else {
		current_screen_status = 1;
	}

	if( uiLen ) {
		while(hdmi_running) {
			memset(value, NULL, PROPERTY_VALUE_MAX);
			uiLen = property_get("tcc.hdmi.screen_status", value, "");

			if( uiLen )
				current_screen_status = atoi(value);

			if( uiLen && (current_screen_status != g_last_screen_status) ) {
				switch(current_screen_status)
				{
					case 0 :
						HDMIStop();
						ALOGD("HDMIStop()\n");
					break;
					case 1 :
						HDMIStart();
						hdmi_running = 0;
						ALOGE("HDMIStart()\n");
					break;
					default :
						hdmi_running = 0;
					break;
				}

				g_last_screen_status = current_screen_status;
			}

			if(current_screen_status == 1)
				hdmi_running = 0;

			usleep(100000);
		}
	}
	#endif
	return 1;
}

int hdmi_api_demo(void)
{
        int ret = -1;
	#if defined(HDMI_IOC_ACP_PACKET_CONTROL) && defined(HDMI_IOC_ISRC_HEADER_CONTROL)
        int i, current_acp_demo;
        char value[PROPERTY_VALUE_MAX];
        static int previous_acp_demo = 0;

        struct HDMIAcpPacketCtrl acp_packet;
        struct HDMIIsrcHeaderCtrl isrc_header;

        property_get("tcc_hdmi_acp_demo", value, "");
        current_acp_demo = atoi(value);

        if(previous_acp_demo != current_acp_demo) {
                memset(&acp_packet, 0, sizeof(acp_packet));
                previous_acp_demo = current_acp_demo;

                if(!API_EDID_Get_SupportAI()){
                        printf("%s sink did not support acp\r\n", __func__);
                        HDMISetACP(&acp_packet);
                } else {
                        if(current_acp_demo < 100) {
                                switch(current_acp_demo) {
                                        case 0:
                                                /* disable */
                                                ret = 0;
                                                break;
                                        case 10:
                                                /* Generic Audio */
                                                acp_packet.acp.fr_rate = 7; /* 60Hz -> 16ms, 16*8 = 128ms */
                                                acp_packet.acp.enable = 2;
                                                acp_packet.acp.type = 0;
                                                ret = 0;
                                                break;
                                        case 20:
                                                /* IEC60958-Identified Audio */
                                                acp_packet.acp.enable = 2;
                                                acp_packet.acp.type = 1;
                                                ret = 0;
                                                break;
                                        case 30:
                                                /* DVD Audio */
                                                acp_packet.acp.enable = 2;
                                                acp_packet.acp.type = 2;
                                                acp_packet.acp.data[0] = 1;
                                                acp_packet.isrc1.enable = 2;
                                                acp_packet.isrc1.header.valid = 0;
                                                acp_packet.isrc1.header.status = 0;
                                                for(i=1;i<16;i++) {
                                                        acp_packet.isrc1.data[i] = i;
                                                }
                                                ret = 0;
                                                break;
                                        case 31:
                                                /* DVD Audio with ISRC2*/
                                                acp_packet.acp.enable = 2;
                                                acp_packet.acp.type = 2;
                                                acp_packet.acp.data[0] = 1;
                                                acp_packet.isrc1.enable = 2;
                                                acp_packet.isrc1.header.valid = 0;
                                                acp_packet.isrc1.header.status = 0;
                                                acp_packet.isrc2.enable = 1;
                                                for(i=1;i<16;i++) {
                                                        acp_packet.isrc1.data[i] = 16-i;
                                                }
                                                for(i=1;i<16;i++) {
                                                        acp_packet.isrc2.data[i] = i+16;
                                                }
                                                ret = 0;
                                                break;

                                        case 40:
                                                /* Generic Audio */
                                                acp_packet.acp.enable = 2;
                                                acp_packet.acp.type = 3;
                                                acp_packet.acp.data[0] = 0;
                                                ret = 0;
                                                break;
                                        case 41:
                                                /* Generic Audio */
                                                acp_packet.acp.enable = 2;
                                                acp_packet.acp.type = 3;
                                                acp_packet.acp.data[0] = 1;
                                                ret = 0;
                                                break;
                                        case 42:
                                                /* Generic Audio */
                                                acp_packet.acp.enable = 2;
                                                acp_packet.acp.type = 3;
                                                acp_packet.acp.data[0] = 2;
                                                ret = 0;
                                                break;
                                        default:
                                                break;
                                }
                                HDMISetACP(&acp_packet);
                        }else {
                                switch(current_acp_demo) {
                                        case 100:
                                                isrc_header.status = 0;
                                                isrc_header.valid = 0;
                                                HDMISetISRCHeader(&isrc_header);
                                                break;
                                        case 101:
                                                isrc_header.status = 1; /* Starting Position */
                                                isrc_header.valid = 1;
                                                HDMISetISRCHeader(&isrc_header);
                                                break;
                                        case 102:
                                                isrc_header.status = 2; /* Intermediate Position */
                                                isrc_header.valid = 1;
                                                HDMISetISRCHeader(&isrc_header);
                                                break;
                                        case 103:
                                                isrc_header.status = 4; /* Ending Position */
                                                isrc_header.valid = 1;
                                                HDMISetISRCHeader(&isrc_header);
                                                break;
                                }
                        }
                }
        }
	#endif
	return ret;
}

#ifdef __cplusplus
}
#endif
