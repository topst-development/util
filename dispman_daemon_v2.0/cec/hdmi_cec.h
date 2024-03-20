/*!
* TCC Version 1.0
* Copyright (c) Telechips Inc.
* All rights reserved 
*  \file        hdmi_cec.h
*  \brief       HDMI CEC application
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

#ifndef HDMI_V2_0_CEC_DEAMON
#define HDMI_V2_0_CEC_DEAMON

#ifdef __cplusplus
extern "C" {
#endif


//#include <hdmi_v2_0_cec/include/hdmi_cec.h>

#define CMD_SIZE 50
#define POLL_TIME 2*1000*1000*1000

/**
 * @enum CECDeviceType
 * Type of CEC device
 */
enum CECDeviceType {
        CEC_DEVICE_TV = 0,
        CEC_DEVICE_RECODER,
        CEC_DEVICE_RESERVED,
        CEC_DEVICE_TUNER,
        CEC_DEVICE_PLAYBACK,
        CEC_DEVICE_AUDIO,
        CEC_DEVICE_FREE,
        CEC_DEVICE_UNREGISTED,
};


struct cec_buffer{
        char        send_buf[16];
        char        recv_buf[16];
        unsigned    int size;
#if 0
        unsigned        src_address;
        unsigned        dst_address;
#endif
};


typedef enum {
        CEC_VERSION_11 = 0,
        CEC_VERSION_12,
        CEC_VERSION_12A,
        CEC_VERSION_13,
        CEC_VERSION_13A,
        CEC_VERSION_14A,
} cec_version_e;


typedef struct {
        //char                    hdmi_edid_name[CMD_SIZE];
        //int                     hdmi_edid_driver;
        char                    hdmi_cec_name[CMD_SIZE];
        char                    vendor_info_name[4];
        int                     hdmi_cec_driver;
        int                     TVvendorID;
        int                     vendorID;

        pthread_t               thread_id;
        int                     thread_run;

        struct cec_buffer       buffer;

        int                     p_address;
        int                     src_l_address;
        int                     dst_l_address;
        enum CECDeviceType      devtype;
        int                     running_cec;
        char                    menu_language[3];                
}hdmitx_cec_t;

#define CEC_MAX_FRAME_SIZE                16
#define CEC_NOT_VALID_PHYSICAL_ADDRESS    0xFFFF
#define CEC_COMMON_PHYSICAL_ADDRESS       0x1000

/** CEC broadcast address (as destination address) */
#define CEC_MSG_BROADCAST                 0x0F
/** CEC unregistered address (as initiator address) */
#define CEC_LADDR_UNREGISTERED            0x0F

/*
 * CEC Messages
 */
//@{
/** @name Messages for the One Touch Play Feature */
#define CEC_OPCODE_ACTIVE_SOURCE            0x82
#define CEC_OPCODE_IMAGE_VIEW_ON            0x04
#define CEC_OPCODE_TEXT_VIEW_ON             0x0D
//@}

//@{
/** @name Messages for the Routing Control Feature */
#define CEC_OPCODE_INACTIVE_SOURCE          0x9D
#define CEC_OPCODE_REQUEST_ACTIVE_SOURCE    0x85
#define CEC_OPCODE_ROUTING_CHANGE           0x80
#define CEC_OPCODE_ROUTING_INFORMATION      0x81
#define CEC_OPCODE_SET_STREAM_PATH          0x86
//@}

//@{
/** @name Messages for the Standby Feature */
#define CEC_OPCODE_STANDBY                  0x36
//@}

//@{
/** @name Messages for the One Touch Record Feature */
#define CEC_OPCODE_RECORD_OFF               0x0B
#define CEC_OPCODE_RECORD_ON                0x09
#define CEC_OPCODE_RECORD_STATUS            0x0A
#define CEC_OPCODE_RECORD_TV_SCREEN         0x0F
//@}

//@{
/** @name Messages for the Timer Programming Feature */
#define CEC_OPCODE_CLEAR_ANALOGUE_TIMER     0x33
#define CEC_OPCODE_CLEAR_DIGITAL_TIMER      0x99
#define CEC_OPCODE_CLEAR_EXTERNAL_TIMER     0xA1
#define CEC_OPCODE_SET_ANALOGUE_TIMER       0x34
#define CEC_OPCODE_SET_DIGITAL_TIMER        0x97
#define CEC_OPCODE_SET_EXTERNAL_TIMER       0xA2
#define CEC_OPCODE_SET_TIMER_PROGRAM_TITLE  0x67
#define CEC_OPCODE_TIMER_CLEARED_STATUS     0x43
#define CEC_OPCODE_TIMER_STATUS             0x35
//@}

//@{
/** @name Messages for the System Information Feature */
#define CEC_OPCODE_CEC_VERSION              0x9E
#define CEC_OPCODE_GET_CEC_VERSION          0x9F
#define CEC_OPCODE_GIVE_PHYSICAL_ADDRESS    0x83
#define CEC_OPCODE_GET_MENU_LANGUAGE        0x91
//#define CEC_OPCODE_POLLING_MESSAGE
#define CEC_OPCODE_REPORT_PHYSICAL_ADDRESS  0x84
#define CEC_OPCODE_SET_MENU_LANGUAGE        0x32
//@}

//@{
/** @name Messages for the Deck Control Feature */
#define CEC_OPCODE_DECK_CONTROL             0x42
#define CEC_OPCODE_DECK_STATUS              0x1B
#define CEC_OPCODE_GIVE_DECK_STATUS         0x1A
#define CEC_OPCODE_PLAY                     0x41
//@}

//@{
/** @name Messages for the Tuner Control Feature */
#define CEC_OPCODE_GIVE_TUNER_DEVICE_STATUS 0x08
#define CEC_OPCODE_SELECT_ANALOGUE_SERVICE  0x92
#define CEC_OPCODE_SELECT_DIGITAL_SERVICE   0x93
#define CEC_OPCODE_TUNER_DEVICE_STATUS      0x07
#define CEC_OPCODE_TUNER_STEP_DECREMENT     0x06
#define CEC_OPCODE_TUNER_STEP_INCREMENT     0x05
//@}

//@{
/** @name Messages for the Vendor Specific Commands Feature */
#define CEC_OPCODE_DEVICE_VENDOR_ID         0x87
#define CEC_OPCODE_GET_DEVICE_VENDOR_ID     0x8C
#define CEC_OPCODE_VENDOR_COMMAND           0x89
#define CEC_OPCODE_VENDOR_COMMAND_WITH_ID   0xA0
#define CEC_OPCODE_VENDOR_REMOTE_BUTTON_DOWN 0x8A
#define CEC_OPCODE_VENDOR_REMOVE_BUTTON_UP  0x8B
//@}

//@{
/** @name Messages for the OSD Display Feature */
#define CEC_OPCODE_SET_OSD_STRING           0x64
//@}

//@{
/** @name Messages for the Device OSD Transfer Feature */
#define CEC_OPCODE_GIVE_OSD_NAME            0x46
#define CEC_OPCODE_SET_OSD_NAME             0x47
//@}

//@{
/** @name Messages for the Device Menu Control Feature */
#define CEC_OPCODE_MENU_REQUEST             0x8D
#define CEC_OPCODE_MENU_STATUS              0x8E
#define CEC_OPCODE_USER_CONTROL_PRESSED     0x44
#define CEC_OPCODE_USER_CONTROL_RELEASED    0x45
//@}

//@{
/** @name Messages for the Remote Control Passthrough Feature */
//@}

//@{
/** @name Messages for the Power Status Feature */
#define CEC_OPCODE_GIVE_DEVICE_POWER_STATUS 0x8F
#define CEC_OPCODE_REPORT_POWER_STATUS      0x90
//@}

//@{
/** @name Messages for General Protocol messages */
#define CEC_OPCODE_FEATURE_ABORT            0x00
#define CEC_OPCODE_ABORT                    0xFF
//@}

//@{
/** @name Operands for Abort Reason*/
#define CEC_ABORT_RESAON_UNRECOGNIZED                   0x00
#define CEC_ABORT_RESAON_NOT_INCURRENT_MODE_TO_RESPOND  0x01
#define CEC_ABORT_RESAON_CANNOT_PROVIDE_SOURCE          0x02
#define CEC_ABORT_RESAON_INVALID                        0x03
#define CEC_ABORT_RESAON_REFUSED                        0x04
//@}


//@{
/** @name Messages for the System Audio Control Feature */
#define CEC_OPCODE_GIVE_AUDIO_STATUS                    0x71
#define CEC_OPCODE_GIVE_SYSTEM_AUDIO_MODE_STATUS        0x7D
#define CEC_OPCODE_REPORT_AUDIO_STATUS                  0x7A
#define CEC_OPCODE_SET_SYSTEM_AUDIO_MODE                0x72
#define CEC_OPCODE_SYSTEM_AUDIO_MODE_REQUEST            0x70
#define CEC_OPCODE_SYSTEM_AUDIO_MODE_STATUS             0x7E
//@}

//@{
/** @name Messages for the Audio Rate Control Feature */
#define CEC_OPCODE_SET_AUDIO_RATE                       0x9A
//@}


//@{
/** @name CEC Operands */

//TODO: not finished

#define CEC_DECK_CONTROL_MODE_STOP                      0x03
#define CEC_PLAY_MODE_PLAY_FORWARD                      0x24
#define CEC_PLAY_MODE_PLAY_STILL                        0x25        // PAUSE

// ...
#define CEC_USER_CONTROL_MODE_SELECT                    0x00
#define CEC_USER_CONTROL_MODE_UP                        0x01
#define CEC_USER_CONTROL_MODE_DOWN                      0x02
#define CEC_USER_CONTROL_MODE_LEFT                      0x03
#define CEC_USER_CONTROL_MODE_RIGHT                     0x04
#define CEC_USER_CONTROL_MODE_RIGHT_UP                  0x05
#define CEC_USER_CONTROL_MODE_RIGHT_DOWN                0x06
#define CEC_USER_CONTROL_MODE_LEFT_UP                   0x07
#define CEC_USER_CONTROL_MODE_LEFT_DOWN                 0x08
#define CEC_USER_CONTROL_MODE_ROOT_MENU                 0x09
#define CEC_USER_CONTROL_MODE_SETUP_MENU                0x0A
#define CEC_USER_CONTROL_MODE_CONTENT_MENU              0x0B
#define CEC_USER_CONTROL_MODE_FAVORITE_MENU             0x0C
#define CEC_USER_CONTROL_MODE_EXIT                      0x0D     
#define CEC_USER_CONTROL_MODE_NUMBER_0                  0x20
#define CEC_USER_CONTROL_MODE_NUMBER_1                  0x21
#define CEC_USER_CONTROL_MODE_NUMBER_2                  0x22
#define CEC_USER_CONTROL_MODE_NUMBER_3                  0x23
#define CEC_USER_CONTROL_MODE_NUMBER_4                  0x24
#define CEC_USER_CONTROL_MODE_NUMBER_5                  0x25
#define CEC_USER_CONTROL_MODE_NUMBER_6                  0x26
#define CEC_USER_CONTROL_MODE_NUMBER_7                  0x27
#define CEC_USER_CONTROL_MODE_NUMBER_8                  0x28
#define CEC_USER_CONTROL_MODE_NUMBER_9                  0x29
#define CEC_USER_CONTROL_MODE_DOT                       0x2A
#define CEC_USER_CONTROL_MODE_CHANNEL_UP                0x30
#define CEC_USER_CONTROL_MODE_CHANNEL_DOWN              0x31
#define CEC_USER_CONTROL_MODE_INPUT_SELECT              0x34
#define CEC_USER_CONTROL_MODE_VOLUME_UP                 0x41
#define CEC_USER_CONTROL_MODE_VOLUME_DOWN               0x42
#define CEC_USER_CONTROL_MODE_MUTE                      0x43
#define CEC_USER_CONTROL_MODE_PLAY                      0x44
#define CEC_USER_CONTROL_MODE_STOP                      0x45
#define CEC_USER_CONTROL_MODE_PAUSE                     0x46
#define CEC_USER_CONTROL_MODE_RECORD                    0x47
#define CEC_USER_CONTROL_MODE_REWIND                    0x48
#define CEC_USER_CONTROL_MODE_FAST_FORWARD              0x49
#define CEC_USER_CONTROL_MODE_EJECT                     0x4A
#define CEC_USER_CONTROL_MODE_FORWARD                   0x4B
#define CEC_USER_CONTROL_MODE_BACKWARD                  0x4C

#define CEC_POWER_STATUS_ON                             0x00
#define CEC_POWER_STATUS_STANBY                         0x01
#define CEC_POWER_STATUS_INTRANSITIONSTANBYON           0x02
#define CEC_POWER_STATUS_INTRANSITIONONTOSTANBY         0x03
int hdmitx_cmd_handle(hdmitx_cec_t* dev, int cmd, int flag);
int init_cec_parameter(void) ;
int start_cec_process(unsigned short p_address, char *manufacturer_name);
int stop_cec_process(void);
int hdmi_get_cec_status(void);
void CEC_Func_Running(int running);
int hdmi_cec_control_TV(int OnOff);
#ifdef __cplusplus
}
#endif


#endif
