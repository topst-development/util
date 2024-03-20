/*!
* TCC Version 1.0
* Copyright (c) Telechips Inc.
* All rights reserved 
*  \file        hdmi_properties.c
*  \brief       HDMI properties source
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
#include <unistd.h>
#include <string.h>
#include <utils/types.h>
#include <utils/Log.h>
#include <utils/properties.h>

#include <video/tcc/tccfb_ioctrl.h>
#include <hdmi_v2_0/include/hdmi_ioctls.h>

#include <hdmi/hdmi_drv.h>
#include <hdmi/hdmi_fb.h>
#include <hdmi/hdmi_edid.h>
#include <hdmi/hdmi_v2.h>
#include <hdmi/hdmi_audio.h>

#define HDMI_APP_DEBUG  0
#define LOG_TAG "[HDMI_APP  ]"

#if HDMI_APP_DEBUG
#define DPRINTF(args...)    ALOGI(args)
#else
#define DPRINTF(args...)
#endif

extern tcc_hdmi_resolution tcc_support_hdmi[];

/**
 Update hdmi size and hz in fb driver. 
 status - ok
*/
int hdmi_supportmodeset(const tcc_display_size *dispay_size)
{
        int fb_fd = FB_GetHandle();
        if (fb_fd < 0)    {
                DPRINTF("can not open \"%s\"\n", FB_DEV_NAME);
                return 0;
        }
        return ioctl( fb_fd, TCC_LCDC_HDMI_SET_SIZE, dispay_size);
}

int hdmi_set_DVILUT(int enable)
{
        #define FB_DEV_NAME             "/dev/graphics/fb0"

        int fb_fd = FB_GetHandle();
        lut_ctrl_params lut_ctrl_params_dvi;
        
        if (fb_fd < 0)    {
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

/**
 get hdmi video resolution 
 status - ok
*/
int HDMI_GetVideoResolution(unsigned int stbmode)
{
        int iret, ifixed;

        iret  = property_get_int("persist.sys.hdmi_resolution", 125);
        if(iret >= vMaxTableItems) {
                iret = AutoDetectMode;
        }
                
        // check 720p fixed mode by system property.
        ifixed = property_get_int("tcc.all.hdmi.720p.fixed", 0);
        if((ifixed == 1) && ((iret == AutoDetectMode) || ((iret < vMaxTableItems) && (tcc_support_hdmi[iret].HdmiSize.width > 1280)) ))
        {
                return 2;
        }
        
        if (stbmode)
        {
                int temp_resolution;
                temp_resolution = property_get_int("tcc.video.hdmi_resolution", 999);
                if(temp_resolution!=999 && (temp_resolution>=0 && temp_resolution <=26 ))
                {
                        //ALOGI("change HDMI resolution to %d from tcc.video.hdmi_resolution",temp_resolution );
                        return temp_resolution;
                }
        }
        
        return iret;
}

/**
 get hdmi video resolution 
 status - ok
*/
int hdmi_get_native_first(void)
{
        int iret;

        iret = property_get_int("persist.hdmi.native.first", 0);
        
        return iret;
}

/**
 check hdmi spdif setting from sysfs. 
 status - ok
*/
unsigned int hdmi_get_spdif_setting(void)
{
        unsigned int uiSPDIFSetting;

        uiSPDIFSetting = property_get_int("persist.sys.spdif_setting", 0);

        return uiSPDIFSetting;
}

/**
 check hdmi audio setting from sysfs. 
 status - ok
*/
unsigned int hdmi_get_audio_type(void)
{
        int  iHDMIAudioType;
        
        iHDMIAudioType = property_get_int("tcc.hdmi.audio_type", 0);

        if(iHDMIAudioType >= 5) {
                iHDMIAudioType = 0;
        }
        
        return (unsigned int)iHDMIAudioType;
}

/**
 check hdmi link setting from sysfs. 
 status - ok
*/
unsigned int hdmi_get_hdmi_link(void)
{
        unsigned int uiHDMIlink;

        uiHDMIlink = property_get_int("tcc.audio.hdmi.link", 0);

        return uiHDMIlink;
}

void hdmi_set_output_detected(unsigned int detected)
{
        char value[PROPERTY_VALUE_MAX];
        memset(value, 0, PROPERTY_VALUE_MAX);

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
        char value[PROPERTY_VALUE_MAX];
        memset(value, 0, PROPERTY_VALUE_MAX);

        value[0] = '0' + resolution;

        property_set("persist.sys.hdmi_resolution", value);
}

void hdmi_set_detected_resolution(unsigned int resolution)
{
        char value[PROPERTY_VALUE_MAX];
        memset(value, 0, PROPERTY_VALUE_MAX);

        if(resolution > 9) {
                value[0] = '0' + (resolution+10)/10 -1;
                resolution %= 10;
                value[1] = '0' + resolution;
        }else {
                value[0] = '0' + resolution;
        }
        
        property_set("persist.sys.hdmi_detected_res", value);
}

void hdmi_set_detected_mode(unsigned int mode)
{
        char value[PROPERTY_VALUE_MAX];
        memset(value, 0, PROPERTY_VALUE_MAX);

        //persist.sys.hdmi_detected_mode : HDMI 0, DVI 1
        if( mode )
                value[0] = '0';
        else
                value[0] = '1';

        property_set("persist.sys.hdmi_detected_mode", value);
}


color_depth_t hdmi_get_ColorDepth(void)
{
        int iColorDepthIdx;
        color_depth_t iColorDepth;

        iColorDepthIdx = property_get_int("persist.sys.hdmi_color_depth", 0);
        
        switch(iColorDepthIdx){
                case 1:
                        iColorDepth = COLOR_DEPTH_10;
                        break;                          
                case 2:
                        iColorDepth = COLOR_DEPTH_12;
                        break;
                case 3:
                        iColorDepth = COLOR_DEPTH_16;
                        break;
                case 0:
                default:
                        iColorDepth = COLOR_DEPTH_8;
                        break;                          
        }
        
        return iColorDepth;
}

encoding_t hdmi_get_ColorSpace(void)
{
        int iencoding;
        encoding_t encoding;

        iencoding = property_get_int("persist.sys.hdmi_color_space", ENC_AUTO);
        if(iencoding > YCC420) {
                iencoding = ENC_AUTO;
        }
        encoding = (encoding_t)iencoding;

        return encoding;
}

int hdmi_set_ColorSpace(int color_space)
{
        char value[PROPERTY_VALUE_MAX];
        memset(value, 0, PROPERTY_VALUE_MAX);

        value[0] = '0' + color_space;

        property_set("persist.sys.hdmi_color_space", value);

        return 1;
}

int hdmi_get_Colorimetry(void)
{
        int Colorimetry;
        
        Colorimetry = property_get_int("persist.sys.hdmi_colorimetry", 125);

        return Colorimetry;
}


int hdmi_get_HDCPEnableStatus(void)
{
        int iHDCPEnableStatus;

        iHDCPEnableStatus = property_get_int("persist.sys.hdcp1x_enable", 0);

        if(iHDCPEnableStatus) {
                iHDCPEnableStatus = 1;
        } else {
                iHDCPEnableStatus = 0;
        }

        return iHDCPEnableStatus;
}

int hdmi_get_HDCPEnabled(void)
{
        int iHDCPEnabled;

         iHDCPEnabled = property_get_int("tcc.hdcp.hdmi.enable", 0);

	if (iHDCPEnabled == 1) {
               	iHDCPEnabled = 1;
	}else {
		iHDCPEnabled = 0;
	}

        return iHDCPEnabled;
}

int hdmi_get_PixelAspectRatio(void)
{
        int property_aspect_ratio;
        unsigned int config_aspect_ratio = 0;

        property_aspect_ratio = property_get_int("persist.sys.hdmi_aspect_ratio", 0);

        switch(property_aspect_ratio)
        {
                case 1:
                        config_aspect_ratio = HDMI_RATIO_4_3;
                        break;
                case 0:
                default:
                        config_aspect_ratio = HDMI_RATIO_16_9;
                        break;                  
        }
        
        return config_aspect_ratio;
}

int hdmi_set_PixelAspectRatiobyVIC(int vic, int idx_max)
{
        char value[PROPERTY_VALUE_MAX];
        int idx, iUpdateVal, iPixelAspectRatio = HDMI_RATIO_MAX;
        memset(value, 0, PROPERTY_VALUE_MAX);

        for(idx = 0; idx < idx_max; idx++) {
                if(vic == tcc_support_hdmi[idx].vic[HDMI_RATIO_4_3]) {
                        iPixelAspectRatio = HDMI_RATIO_4_3;
                   break;
                }
                if(vic == tcc_support_hdmi[idx].vic[HDMI_RATIO_16_9]) {
                        iPixelAspectRatio = HDMI_RATIO_16_9;
                   break;
                }
        }
        
        if(iPixelAspectRatio < HDMI_RATIO_MAX) {
                if(iPixelAspectRatio == HDMI_RATIO_4_3)
                        iUpdateVal = 0;
                else 
                        iUpdateVal = 1;

                value[0] = '0' + iUpdateVal;
                property_set("persist.sys.hdmi_aspect_ratio", value);
        }         

        return iPixelAspectRatio;        
}

/**
 check hdmi audio smpling rate from sysfs. 
 status - ok
*/
unsigned int hdmi_get_AudioSamplingRate(void)
{
        unsigned int val, AudioSamplingRate = 0;

        val = property_get_int("tcc.audio.sampling_rate", 0);     // SPDIF Planet 20121107

        switch (val)
        {
                case SF_32KHZ:
                        AudioSamplingRate = 32000;
                        break;
                case SF_44KHZ:
                        AudioSamplingRate = 44100;
                        break;
                case SF_88KHZ:
                        AudioSamplingRate = 88200;
                        break;
                case SF_176KHZ:
                        AudioSamplingRate = 176000;
                        break;
                case SF_48KHZ:
                        AudioSamplingRate = 48000;
                        break;
                case SF_96KHZ:
                        AudioSamplingRate = 96000;
                        break;
                case SF_192KHZ:
                        AudioSamplingRate = 192000;
                        break;
        }
        return AudioSamplingRate;
}

/**
 check hdmi audio channel from sysfs. 
 status - ok
*/
unsigned int hdmi_get_AudioChannels(void)
{
        int Channel;
        Channel = property_get_int("tcc.audio.channels", 2);

        if(Channel > 8) {
                Channel = 2;
        }
       
        return (unsigned int)((Channel < 2)?2:Channel);
}

/**
 check hdmi audio output packet rate from sysfs. 
 status - ok
*/
packet_t hdmi_get_AudioOutPacket(void)
{
        packet_t iOutPacket;
        unsigned int uiSPDIFSetting;

        uiSPDIFSetting = hdmi_get_spdif_setting();

        switch(uiSPDIFSetting)
        {
                case AUDIO_OUTPORT_DEFAULT:
                case AUDIO_OUTPORT_DAI_LPCM:
                case AUDIO_OUTPORT_SPDIF_PCM:
                case AUDIO_OUTPORT_SPDIF_BITSTREAM:
                default:
                        iOutPacket = AUDIO_SAMPLE;
                break;

                case AUDIO_OUTPORT_DAI_HBR:
                        if(hdmi_get_audio_type() ==AUDIO_HBR)
                                iOutPacket = HBR_STREAM;
                        else
                                iOutPacket = AUDIO_SAMPLE;
                        break;

                break;
        }

        return iOutPacket;
}

/**
 control hdmi audio on/off
 status - ok
*/
int hdmi_AudioOnOffChk(void)
{
        int hdmi_audio_onoff;
        char value[PROPERTY_VALUE_MAX];

        memset(value, 0, PROPERTY_VALUE_MAX);
        hdmi_audio_onoff = property_get_int("tcc.output.hdmi_audio_onoff", 0);

        if( hdmi_audio_onoff == 2 )
        {
                //Audio Stop
                //HDMIAudioStop();

                value[0] = '1';
                property_set("tcc.output.hdmi_audio_disable", value);
        }
        else if( hdmi_audio_onoff == 1 )
        {
                // Audio Start
                //HDMIAudioStart();

                value[0] = '0';
                property_set("tcc.output.hdmi_audio_disable", value);
        }

        value[0] = '0';
        property_set("tcc.output.hdmi_audio_onoff", value);

        return 1;
}


/**
 check hdmi spdif setting from sysfs. 
 status - ok
*/
interfaceType_t hdmi_get_AudioInputPort(void)
{
        unsigned int uiSPDIFSetting = 0;
        interfaceType_t iAudioInputPort;

        uiSPDIFSetting = hdmi_get_spdif_setting();
        switch(uiSPDIFSetting)
        {
                case AUDIO_OUTPORT_DAI_LPCM:
                case AUDIO_OUTPORT_DAI_HBR:
                        iAudioInputPort = I2S;
                        break;
                case AUDIO_OUTPORT_SPDIF_PCM:
                case AUDIO_OUTPORT_SPDIF_BITSTREAM:
                        iAudioInputPort = SPDIF;
                        break;
                case AUDIO_OUTPORT_DEFAULT:
                default:
                        if(hdmi_get_hdmi_link())
                                iAudioInputPort = SPDIF;
                        else
                                iAudioInputPort = I2S;
                        break;
        }

        return iAudioInputPort;
}


/**
 check hdmi refresh rate mode from sysfs. 
 status - ok
*/
int hdmi_get_refresh_rate(void)
{
        return property_get_int("persist.sys.hdmi_refresh_rate", 125); // default auto mode
}
