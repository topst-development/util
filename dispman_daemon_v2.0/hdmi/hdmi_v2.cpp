/*!
* TCC Version 1.0
* Copyright (c) Telechips Inc.
* All rights reserved 
*  \file        hdmi_v2.cpp
*  \brief       HDMI application source
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
#include <signal.h>
#include <pthread.h>
#include <string.h>

#include <utils/types.h>
#include <utils/Log.h>
#include <utils/properties.h>
#include <hdmi_v2_0/include/hdmi_ioctls.h>
#include <video/tcc/tccfb_ioctrl.h>
#include <hdmi/hdmi_drv.h>
#include <hdmi/hdmi_fb.h>
#include <hdmi/hdmi_edid.h>
#include <hdmi/hdmi_properties.h>
#include <hdmi/hdmi_hdr.h>
#include <hdmi/hdmi_lut.h>
#include <hdmi/hdmi_v2.h>
#include <hdmi/hdmi_audio.h>

#include <cec/hdmi_cec.h>
#if defined(CONFIG_HDMI_DOLBYVISION_SUPPORT)
#include <dolbyvision_vsdb.h>
#endif

/**
 * HPD_CABLE_OUT indicates HDMI cable out.
 */
#define HPD_CABLE_OUT   0

/**
 * HPD_CABLE_IN indicates HDMI cable in.
 */
#define HPD_CABLE_IN    1

#define HDMI_APP_DEBUG  0
#define HDMI_APP_EDID_DEBUG 1
#define LOG_TAG         "[HDMI_APP  ]"

#if HDMI_APP_DEBUG
#define DPRINTF(args...) ALOGI(args)
#else
#define DPRINTF(args...)
#endif

#if HDMI_APP_EDID_DEBUG
#define EDPRINTF(args...) ALOGI(args)
#else
#define EDPRINTF(args...)
#endif

#define SCDC_SOURCE_VERSION 1


enum {
        stage_check_property = 1,
        stage_match_edid,
};


static int hpd_state = HPD_CABLE_OUT;

#define COMPATIBILITY_WAIT_TIME 400
tcc_hdmi_resolution tcc_support_hdmi[] = {
        /**
         * idx           vic4:3      refresh_rate      width        Hz          vblank not_support_dvi
         *  |               |  vic16:9  |  pixel_clock   |   height  |    hblank  | interlaced       
         *  |               |     |     |        |       |      |    |      |     |  |              */
        {v1920x1080p_60Hz,{  0,   16}, 60000, 148500000, {1920, 1080, 60},  280,  45, 0, 0, 0},
        {v1920x1080p_50Hz,{  0,   31}, 50000, 148500000, {1920, 1080, 50},  720,  45, 0, 0, 0},
        {v1920x1080i_60Hz,{  0,    5}, 60000,  74250000, {1920, 1080, 60},  280,  22, 1, 0, 0},
        {v1920x1080i_50Hz,{  0,   20}, 50000,  74250000, {1920, 1080, 50},  720,  22, 1, 0, 0},
        {v1280x720p_60Hz, {  0,    4}, 60000,  74250000, {1280,  720, 60},  370,  30, 0, 0, 0},
        {v1280x720p_50Hz, {  0,   19}, 50000,  74250000, {1280,  720, 50},  700,  30, 0, 0, 0},
        {v720x576p_50Hz,  { 17,   18}, 50000,  27000000, { 720,  576, 50},  144,  49, 0, 0, 0},
        {v720x480p_60Hz,  {  2,    3}, 60000,  27000000, { 720,  480, 60},  138,  45, 0, 0, 0},
        {v640x480p_60Hz,  {  1,    0}, 60000,  25175000, { 640,  480, 60},  160,  45, 0, 0, 0},
        // Add HDMI V2.0	
        {v1920x1080_30Hz, {  0,   34}, 30000,  74250000, {1920, 1080, 30},  280,  45, 0, 0, 0},
        {v3840x2160p_30Hz,{  0,   95}, 30000, 297000000, {3840, 2160, 30},  560,  90, 0, 1, 0},
        {v3840x2160p_60Hz,{  0,   97}, 60000, 594000000, {3840, 2160, 60},  560,  90, 0, 1, 0},

        {v1920x1080p_24Hz,{  0,   32}, 24000,  74250000, {1920, 1080, 24},  830,  45, 0, 0, 0},

        // Added 2016.12.05
        {v3840x2160p_24Hz,{  0,   93}, 24000, 297000000, {3840, 2160, 24}, 1660,  90, 0, 1, 0},
        {v3840x2160p_25Hz,{  0,   94}, 25000, 297000000, {3840, 2160, 25}, 1440,  90, 0, 1, 0},
        {v3840x2160p_50Hz,{  0,   96}, 50000, 594000000, {3840, 2160, 50}, 1440,  90, 0, 1, 0},

        // Custom 
        {v1920x720p_60Hz, {  0, 1024}, 60000,  88200000, {1920,  720, 60},   64,  21, 0, 0, 0},

};

/* This variable to store hdmi feature supported by soc */
static hdmi_soc_features soc_feature;

#if defined(TCC_HDMI_UI_SIZE_1280_720)
#define TCC_HDMI_DEFAULT_VIC		v1280x720p_60Hz
#define TCC_HDMI_DEFAULT_ASPEC          1       // 16:9
#else
#define TCC_HDMI_DEFAULT_VIC		v1920x1080p_60Hz
#define TCC_HDMI_DEFAULT_ASPEC          1       // 16:9
#endif


extern unsigned int dispman_daemon_stbmode;
static int hdmi_fd = -1;
        
static videoParams_t videoParams;
static audioParams_t audioParams;
static productParams_t productParams;
static hdcpParams_t hdcpParams;


static sink_edid_t sink_cap;
static unsigned int gHDMIAudioOutput = 0;
static struct timespec hdmi_setting_time;
static int hdmi_compatibility_check = 0;
static int hdmi_outpustarter_powerdown = 0;

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
                ALOGI("state is unknown!!!\n");
        }

        if(hpd_before_state != hpd_state)
        {
                ALOGI("[%s] HPD STATE CHANGE :%d", __func__, hpd_state);
        }
}

int hdmi_get_hdmi_mode(void)
{
	return videoParams.mHdmi;
}

static void hdmi_video_params_reset(void)
{
        memset((void *)&videoParams, 0, sizeof(videoParams));
        videoParams.mHdmi = HDMI;
        videoParams.mEncodingOut = RGB;
        videoParams.mEncodingIn = RGB;
        videoParams.mColorResolution = COLOR_DEPTH_8;
        videoParams.mActiveFormatAspectRatio = 8;
        videoParams.mExtColorimetry = (ext_colorimetry_t)~0;
        videoParams.mEndTopBar =(unsigned short)(-1);
        videoParams.mStartBottomBar =(unsigned short)(-1);
        videoParams.mEndLeftBar =(unsigned short)(-1);
        videoParams.mStartRightBar = (unsigned short)(-1);
        videoParams.mHdmi20 = 0;
}  

static void hdmi_product_params_reset(void)
{
        productParams.mVendorNameLength = 0;
        productParams.mProductNameLength = 0;
        productParams.mSourceType = (u8) (-1);
        productParams.mOUI = (u8) (-1);
        productParams.mVendorPayloadLength = 0;
        memset(productParams.mVendorName, 0, sizeof(productParams.mVendorName));
        memset(productParams.mProductName, 0, sizeof(productParams.mProductName));
        memset(productParams.mVendorPayload, 0, sizeof(productParams.mVendorPayload));
}

static void hdmi_hdcp_params_reset(void)
{
	hdcpParams.bypass = TRUE;
	hdcpParams.mHdcp14Enable = 0;
	hdcpParams.mEnable11Feature = 0;
	hdcpParams.mRiCheck = 1;
	hdcpParams.mI2cFastMode = 0;
	hdcpParams.mEnhancedLinkVerification = 0;
	hdcpParams.maxDevices = 0;
	if (hdcpParams.mKsvListBuffer != NULL)
		free(hdcpParams.mKsvListBuffer);
	hdcpParams.mKsvListBuffer = NULL;
}

/**
 * @short Remove the supported resolution information.
 */
static void hdmi_clean_support_hdmi_table(void) 
{
        int index;
        for(index = 0; index < vMaxTableItems; index++) {
                tcc_support_hdmi[index].edid_support_status = 0;
        }
}

static unsigned int hdmi_calculate_actual_tmds_bit_ratio(encoding_t encoding, color_depth_t color_depth, unsigned int pixelclock, int framepacking_mode)
{
        unsigned int dtd_pixelclock, actual_pixelclock;
        
        dtd_pixelclock = pixelclock;
        actual_pixelclock = pixelclock;
        
        if(framepacking_mode) {
                dtd_pixelclock = pixelclock << 1;
        }
        
        if(dtd_pixelclock > 0) {
                actual_pixelclock = dtd_pixelclock;
                switch(encoding) 
                {
                        case RGB:
                        case YCC444:
                            switch(color_depth) 
                            {
                                case COLOR_DEPTH_10:
                                        // 1.25x
                                        actual_pixelclock = (dtd_pixelclock * 125)/100;
                                        break;
                                case COLOR_DEPTH_12:
                                        // 1.5x
                                        actual_pixelclock = (dtd_pixelclock * 15)/10;
                                        break;
                                case COLOR_DEPTH_16:
                                        // 2x
                                        actual_pixelclock = (dtd_pixelclock << 1);
                                        break;
                                default:
                                        break;
                            }
                            break;
                        case YCC420:
                            switch(color_depth) 
                            {
                                case COLOR_DEPTH_8:
                                        // 0.5x
                                        actual_pixelclock = (dtd_pixelclock >> 1);
                                        break;
                                case COLOR_DEPTH_10:
                                        // 0.625x
                                        actual_pixelclock = (dtd_pixelclock * 625)/1000;
                                        break;
                                case COLOR_DEPTH_12:
                                        // 0.75x
                                        actual_pixelclock = (dtd_pixelclock * 75)/100;
                                        break;
                                default:
                                        break;
                            }
                            break;

                        default:
                                // Nothing
                                break;
                }
        }
        return actual_pixelclock;
}


static unsigned int hdmi_get_actual_tmds_bit_ratio(void)
{
        int framepacking_mode = 0;
        unsigned int actual_pixelclock;
        if(videoParams.mHdmiVideoFormat == 2 && videoParams.m3dStructure == 0) {
                framepacking_mode = 1;
        }
        
        actual_pixelclock = hdmi_calculate_actual_tmds_bit_ratio(videoParams.mEncodingOut, (color_depth_t)videoParams.mColorResolution, videoParams.mDtd.mPixelClock, framepacking_mode);

        ALOGI("actual pixel clock is about %dKHz\r\n", actual_pixelclock);
        return actual_pixelclock;
}

static void hdmi_api_make_packet(void) 
{
        hdmi_product_params_reset();
        productParams.mVendorName[0] = 'T';
        productParams.mVendorName[1] = 'C';
        productParams.mVendorName[2] = 'C';
        productParams.mVendorNameLength = 3;

        productParams.mProductName[0] = 'T';
        productParams.mProductName[1] = 'C';
        productParams.mProductName[2] = 'C';
        productParams.mProductName[3] = '8';
        productParams.mProductName[4] = '9';
        productParams.mProductName[5] = '8';
        productParams.mProductName[6] = 'x';
        productParams.mProductNameLength = 7;
        productParams.mSourceType = 0x01; // Digital STB
        // fc_vsdsize max size is 27
        productParams.mVendorPayloadLength = 0;

        /**
         * INFO: HDMI 2.0 3D MODE>
        if(0) {
                productParams.mOUI = 0x00D85DC4;
                productParams.mVendorPayload[0] = 0x1;
        }
        */
        productParams.mOUI = 0x00030C00;
        if((videoParams.mDtd.mCode >= 93 && videoParams.mDtd.mCode <= 95) || videoParams.mDtd.mCode == 98)
        {
                productParams.mVendorPayload[0] = 1 << 5;
		productParams.mVendorPayloadLength = 5;
                switch(videoParams.mDtd.mCode)
                {
                        case 93: // 3840x2160p24Hz
                                productParams.mVendorPayload[1] = 3;
                                break;
                        case 94: // 3840x2160p25Hz
                                productParams.mVendorPayload[1] = 2;
                                break;
                        case 95: // 3840x2160p30Hz
                                productParams.mVendorPayload[1] = 1;
                                break;
                        case 98: // 4096x2160p24Hz
                                productParams.mVendorPayload[1] = 4;
                                break;
                 }                        
        } else if (videoParams.mHdmiVideoFormat == 2) {
		u8 struct_3d = videoParams.m3dStructure;
                
		// frame packing || tab || sbs
		if ((struct_3d == 0) || (struct_3d == 6) || (struct_3d == 8)) {
                        /** 
                         * INFO: HDMI 2.0 3D 
                        if(0) {
                                // Futher Used
                                productParams.mVendorPayload[0] = 0x1; // version 1
                                productParams.mVendorPayload[1] = 0x1; // 3D valid
                                productParams.mVendorPayload[2] = ((struct_3d & 0xF) << 4) | (videoParams.m3dExtData & 0xF);
                                productParams.mOUI = 0x01D85DC4;
                                productParams.mVendorPayloadLength = 6;
                        }
                        */
			productParams.mVendorPayload[0] = videoParams.mHdmiVideoFormat << 5;
			productParams.mVendorPayload[1] = struct_3d << 4;
			productParams.mVendorPayload[2] = videoParams.m3dExtData << 4;
                        productParams.mOUI = 0x00030C00;
                        productParams.mVendorPayloadLength = 6;
		}
        }
	/* Support DolbyVision  */
        if(videoParams.mDolbyVision & (2 << 3)) {
                /* refre to dolbyvision 6.1.1 */
                productParams.mVendorPayloadLength = 24;
	}
        /* Do not send Vendor Specific InfoFrmae except HDMI_VIC, HDMI_3D or DolbyVision */
        if(productParams.mVendorPayloadLength == 0) {
                memset(productParams.mVendorPayload, 0, sizeof(productParams.mVendorPayload));
        }
}

static int hdmi_api_output_set(void)
{ 
        hdmi_api_make_packet();

        if(hdmi_get_actual_tmds_bit_ratio() >= 340000) {
                videoParams.mScrambling = 1;
        }else if(sink_cap.edid_m20Sink &&
        (sink_cap.edid_mHdmiForumvsdb.mValid) &&
        (sink_cap.edid_mHdmiForumvsdb.mSCDC_Present) &&
        (sink_cap.edid_mHdmiForumvsdb.mLTS_340Mcs_scramble)){
                videoParams.mScrambling = 1;
        } else {
                videoParams.mScrambling = 0;
        }
        /** 
         * The 8-bit I2C slave addresses of the EDID are 0xA0/0xA1 and the address of 
         * SCDC are 0xA8/0xA9. 
         * I thought that 2k TV would not respond to SCDC address, but I found the 2k TV 
         * that responding to the SCDC address. The 2k tv initializes some of the edids when 
         * it receives the tmds character ratio or scramble command through the scdc address.
         * Then an edid checksum error will occur when the source reads edid.
         * To prevent this, i changed the source to use scdc address only if the sink 
         * supports scdc address. */
        videoParams.mScdcPresent = sink_cap.edid_mHdmiForumvsdb.mSCDC_Present;
        if (HDMI_Api_Config(&videoParams, &audioParams, &productParams, &hdcpParams) < 0){
                DPRINTF("fail to set api mode!!\n");
                return -1;
        }
        return 0;
}

static int hdmi_audio_output_set(void)
{	
        if (!HDMI_Audio_Config(audioParams))
        {
                DPRINTF("fail to set audio mode!!\n");
                return 0;
        }
        return 1;
}


static void hdmi_display_set_power(int enable) 
{
        if(enable) {
	        if(!HDMI_Get_PowerStatus()) {
                        HDMI_Set_PowerStatus(1);
                        if(hdmi_outpustarter_powerdown) {
                                usleep(300000);
                                hdmi_outpustarter_powerdown = 0;
                        } else {
                                usleep(200000);
                        }
                }
        } else {
                if(HDMI_Get_PowerStatus()) {
                        HDMI_Set_PowerStatus(0);
                }
        }
}

void hdmi_audio_params_reset()
{
	audioParams.mInterfaceType = I2S;
	audioParams.mCodingType = PCM;
	audioParams.mChannelAllocation = 0x0;
	audioParams.mSampleSize = 16;
	audioParams.mDataWidth = 16;
	audioParams.mSamplingFrequency = 32000;
	audioParams.mLevelShiftValue = 0;
	audioParams.mDownMixInhibitFlag = 0;
	audioParams.mIecCopyright = 1;
	audioParams.mIecCgmsA = 3;
	audioParams.mIecPcmMode = 0;
	audioParams.mIecCategoryCode = 0;
	audioParams.mIecSourceNumber = 1;
	audioParams.mIecClockAccuracy = 0;
	audioParams.mPacketType = AUDIO_SAMPLE;
	audioParams.mClockFsFactor = 64;
	audioParams.mDmaBeatIncrement = DMA_UNSPECIFIED_INCREMENT;
	audioParams.mDmaThreshold = 0;
	audioParams.mDmaHlock = 0;
	audioParams.mGpaInsertPucv = 0;
	audioParams.mAudioType = AUDIO_ASP_PCM;

}

#if defined(CONFIG_SUPPORT_UPDATE_HDMI_PHYREGS)
void update_hdmi_phyregs(void)
{
        char *phyreg_buff = NULL;
        int hdmiphy_fb = open("/proc/hdmi_tx/phy_regs", O_RDWR);
        int userphy_fb = open("/nand1/hdmiphy.reg", O_RDONLY);

        off_t file_size, read_size;


        if(userphy_fb) {
                file_size = lseek(userphy_fb, 0, SEEK_END);
                if(file_size > 0 ){
                        lseek(userphy_fb, 0, SEEK_SET);
                        phyreg_buff = (char*)malloc(file_size+1);
                        if(phyreg_buff) {
                                read_size = read (userphy_fb, phyreg_buff, file_size);
                                phyreg_buff[read_size] = '\0';  
                                
                        }
                        close(userphy_fb);
                }
        }

        if(hdmiphy_fb) {
                if(phyreg_buff)
                        write (hdmiphy_fb, phyreg_buff, strlen(phyreg_buff));
                close(hdmiphy_fb);
        }

        if(phyreg_buff) 
                free(phyreg_buff);
}
#endif

int hdmi_display_init(void)
{
        
	memset((void *)&audioParams, 0, sizeof(audioParams));
	
	if (!HDMI_HPDSetCallback(&HPDcallback)) {
		DPRINTF("HPDSetCallback() failed!\n");
		return -1;
	}		

        // Open Drivers 
        HDMI_Open();
        FB_Open();
        hdmi_video_params_reset();
        hdmi_audio_params_reset();
        hdmi_product_params_reset();
        hdmi_hdcp_params_reset();

        videoParams.mEncodingOut = videoParams.mEncodingIn = hdmi_get_ColorSpace();
        videoParams.mColorResolution = hdmi_get_ColorDepth();
       

        hdmi_supportmodeset(&tcc_support_hdmi[TCC_HDMI_DEFAULT_VIC].HdmiSize);

        /* Read supported feature by soc */
        HDMIDrv_get_feature(&soc_feature);
        if(HDMI_Get_PowerStatus()) {
                /* we have to wait END_VSYNC from player */
                hdmi_set_output_detected(false);
                /* Some tv,Grudig Vision9, needs enough waiting delay before hdmi power off*/
                usleep(1000000);
                HDMI_Set_AvMute(1); 
                /* CEA8610F F.3.6 Video Timing Transition (AVMUTE Recommendation) */
                usleep(90000);
                HDMI_Api_Disable();				
                FB_video_generator_stop();
                
                // cleanup edid information. 
                memset(&sink_cap, 0, sizeof(sink_cap));
                usleep(100000); // Wait FB STOP
                HDMI_Set_PowerStatus(0);
                usleep(1000000); // Wait PHY disable
                hdmi_outpustarter_powerdown = 1;                
        }
        #if defined(CONFIG_SUPPORT_UPDATE_HDMI_PHYREGS)
        update_hdmi_phyregs();
        #endif
        init_cec_parameter();
        DPRINTF("finish hdmi_display_init\r\n");
        return 0;
}

/**
 deinit hdmi display
 status - ok
*/
int hdmi_display_deinit(void)
{
        DPRINTF("%s", __func__);

        if(hdmi_fd > 0) 
                close(hdmi_fd);
        hdmi_fd = -1;
        //HDMIFBClose();
        return 0;
}

/**
 check hdmi status in fb driver. 
 status - ok
*/
unsigned int hdmi_lcdc_check(void)
{
	unsigned int hdmi_check;
	hdmi_check = FB_LcdcCheck();
	return hdmi_check;
}

/**
 check hdmi suspend status in fb driver. 
 status - ok
*/
unsigned int hdmi_suspend_check(void)
{
	unsigned int hdmi_suspend;
	hdmi_suspend = HDMI_SuspendCheck();
	return hdmi_suspend;
}


/*---------- HPD DETECT ON/OFF -------------- */
int hdmi_display_detect_onoff(char onoff)
{
	DPRINTF("[%s] onoff:%d !!\n", __func__, onoff);
        /*  HPD interrupt model
                00 - gpio 
                01 - hdi link  */
        if(soc_feature.support_feature_1 & (1 << 3)) {
        	if(onoff)
        	{
        	        hdmi_display_set_power(1);
        	        HDMI_HPD_Enable();
        	}
        	else
        	{
        	        HDMI_HPD_Disable();
        	        hdmi_display_set_power(0);
        	}
        }
	return 0;
}



int hdmi_AudioInputPortDetect(void)
{
        char value[PROPERTY_VALUE_MAX];

        interfaceType_t	iAudioInputPort = hdmi_get_AudioInputPort();
        packet_t iOutPacket = hdmi_get_AudioOutPacket();
        unsigned int iAudioSamplingRate = hdmi_get_AudioSamplingRate();
        unsigned int uiSPDIFSetting = hdmi_get_spdif_setting();
        unsigned int iChannelNum = hdmi_get_AudioChannels();
        unsigned int uiHDMIAudioType = hdmi_get_audio_type();
        unsigned int hdmi_audio_off = 0;
        unsigned int uiAudioSettingisChanged = false;

	if(iAudioInputPort != audioParams.mInterfaceType)
		uiAudioSettingisChanged = true;
	
	if(iOutPacket != audioParams.mPacketType)
		uiAudioSettingisChanged = true;
	
	if(iAudioSamplingRate != audioParams.mSamplingFrequency)
		uiAudioSettingisChanged = true;
	
	if(uiSPDIFSetting != gHDMIAudioOutput)
		uiAudioSettingisChanged = true;

	if(uiSPDIFSetting == AUDIO_OUTPORT_DAI_HBR && uiHDMIAudioType !=audioParams.mAudioType)
		uiAudioSettingisChanged = true;
	if(uiAudioSettingisChanged)
        {
		hdmi_audio_params_reset();
			 
		switch(uiSPDIFSetting)
		{
			case AUDIO_OUTPORT_DEFAULT:
				audioParams.mInterfaceType = iAudioInputPort;
				audioParams.mChannelAllocation = 0x0;
				audioParams.mCodingType = PCM;
				audioParams.mClockFsFactor = 64;
			break;
			
			case AUDIO_OUTPORT_SPDIF_PCM:
			case AUDIO_OUTPORT_SPDIF_BITSTREAM:
				audioParams.mInterfaceType = SPDIF;
				audioParams.mChannelAllocation = 0x0;
				audioParams.mCodingType = PCM;
				audioParams.mClockFsFactor = 128;
			break;
			
			case AUDIO_OUTPORT_DAI_LPCM:
				//not use in this case
				audioParams.mInterfaceType = I2S;
				audioParams.mChannelAllocation = iChannelNum;
				audioParams.mCodingType = PCM;
				audioParams.mClockFsFactor = 64;
			break;
			
			case AUDIO_OUTPORT_DAI_HBR:
				if(uiHDMIAudioType == AUDIO_HBR){
					//DTSHD-MA Pass-thru to 7.1Ch
					audioParams.mChannelAllocation = 0x2F;
					audioParams.mCodingType = DTS_HD;
					audioParams.mDataWidth = 21;
				} else if(uiHDMIAudioType == AUDIO_ASP_LPCM || uiHDMIAudioType == AUDIO_ASP_PCM){
					audioParams.mChannelAllocation = iChannelNum;
					audioParams.mCodingType = PCM;
				} else{
					//DTS, AC3, DDP, AAC Pass-thru up to 5.1Ch
					audioParams.mChannelAllocation = 0x0;
					audioParams.mCodingType = DTS;
					audioParams.mDataWidth = 21;
				}

				audioParams.mInterfaceType = I2S;
				audioParams.mClockFsFactor = 64;
			break;
			
			default:
			break;
		}
			
		audioParams.mPacketType = iOutPacket;							
		audioParams.mSamplingFrequency = iAudioSamplingRate;							
		audioParams.mAudioType = uiHDMIAudioType;
		gHDMIAudioOutput = uiSPDIFSetting;
			
                hdmi_audio_output_set();

                memset(value, 0, PROPERTY_VALUE_MAX);
                property_get("tcc.output.hdmi_audio_disable", value, "");
                hdmi_audio_off = atoi(value);

                if( hdmi_audio_off )
                {
                        //HDMIAudioStop();
                }
                else
                {

                        #if defined(CONFIG_VIDEO_HDMI_IN_SENSOR)
                        ALOGI("~~~~!!! HDMI-In Audio Start !!!~~~~");
                        HDMIAudioStart();
                        #else
                        if( uiSPDIFSetting == AUDIO_OUTPORT_SPDIF_BITSTREAM || uiSPDIFSetting == AUDIO_OUTPORT_SPDIF_PCM) {
                                //Audio Start
                                //HDMIAudioStart();
                        } else {
                                //Audio Start
                                //HDMIAudioStart();
                        }
                        #endif

                }

                ALOGI("\t======================================");
                ALOGI("\t= AutioType    [%d]", uiHDMIAudioType);
                ALOGI("\t= inputport    [%d]", audioParams.mInterfaceType);
                ALOGI("\t= outPacket    [%d]", audioParams.mPacketType);
                ALOGI("\t= samplingrate [%d]", audioParams.mSamplingFrequency);
                ALOGI("\t= Ch           [%d]", audioParams.mChannelAllocation);
                ALOGI("\t= formatCode   [%d]", audioParams.mCodingType);
                ALOGI("\t= AudoOutput   [%d]", uiSPDIFSetting);
                ALOGI("\t======================================");

                return 0;
        }


        return 1;
}

static void print_edidinfo(void)
{
        dtd_t temp_dtd;
        char h3d_info[125];
        unsigned short support_3d_struct;
        int vic, dt_loop, st_loop, svd_loop, table_index, ar_index;

        if(!sink_cap.edid_done) {
                EDPRINTF("=====================================================================");
                EDPRINTF(" NO E-EDID INFO");
                EDPRINTF("=====================================================================");
                return;
		}
        EDPRINTF("\r\n");
        EDPRINTF("=====================================================================");
        EDPRINTF(" E-EDID INFO");
        if(sink_cap.parse_vendor_info.manufacturer_name[0] != 0) {
            EDPRINTF(" PNP ID %s%04x", sink_cap.parse_vendor_info.manufacturer_name, sink_cap.parse_vendor_info.Product_id);
            EDPRINTF(" Serial 0x%x", sink_cap.parse_vendor_info.Serial);
        }

        EDPRINTF("---------------------------------------------------------------------");
        EDPRINTF("Detailed Timing");
        for(dt_loop = 0; dt_loop < 10 ; dt_loop++) {
                EDPRINTF("[%d] %dx%d (blank %d, %d) interlaced(%d) pixel is %dHz", dt_loop, 
                        sink_cap.parse_detailed_timing_dtd[dt_loop].mHActive,
                        sink_cap.parse_detailed_timing_dtd[dt_loop].mVActive,
                        sink_cap.parse_detailed_timing_dtd[dt_loop].mHBlanking,
                        sink_cap.parse_detailed_timing_dtd[dt_loop].mHBlanking,
                        sink_cap.parse_detailed_timing_dtd[dt_loop].mInterlaced,
                        sink_cap.parse_detailed_timing_dtd[dt_loop].mPixelClock);
        }
        EDPRINTF("---------------------------------------------------------------------");
        EDPRINTF("Standard Timing");
        for(st_loop = 0; st_loop < 8 ; st_loop++) {
                struct parse_standard_timing  * parse_standard_timing = &(sink_cap.parse_standard_timing[st_loop]);
                EDPRINTF("[%d] %dx%d at %dHz", st_loop, parse_standard_timing->hactive, parse_standard_timing->vactive, parse_standard_timing->frame_hz);
        }
        EDPRINTF("---------------------------------------------------------------------");
        EDPRINTF("Short Video Descriptor");
        for(svd_loop = 0; (svd_loop < (int)sink_cap.edid_mSvdIndex) && sink_cap.edid_mSvd[svd_loop].mCode ; svd_loop++) {
                if(!HDMI_dtd_fill(&temp_dtd, sink_cap.edid_mSvd[svd_loop].mCode)) {
                        memset(h3d_info, 0, sizeof(h3d_info));
                        if(sink_cap.edid_mHdmivsdb.m3dPresent && svd_loop < MAX_VIC_WITH_3D) {
                                support_3d_struct = get_index_supported_3dstructs(&sink_cap.edid_mHdmivsdb, svd_loop);
                                if(support_3d_struct & 1) {
                                        strcat(h3d_info, "'FP' ");
                                }
                                if(support_3d_struct & (1 << 6)) {
                                        strcat(h3d_info, "'TB' ");
                                }
                                if(support_3d_struct & (1 << 8)) {
                                        strcat(h3d_info, "'SBS' ");
                                }
                                if(0 == (support_3d_struct & 0x141)) {
                                        strcat(h3d_info, "'None' ");
                                }
                        }
                        else {
                                strcat(h3d_info, "'None' ");
                        }
                        if(sink_cap.edid_mSvd[svd_loop].mHdmiVic) {
                                EDPRINTF("[%02d] SVD<%d> %dx%d HDMI_VIC\r\n     3D Supports 'None'", 
                                        svd_loop, 
                                        sink_cap.edid_mSvd[svd_loop].mCode, (int)temp_dtd.mHActive, (int)temp_dtd.mVActive);
                        } else {
                                EDPRINTF("[%02d] SVD<%d> %dx%d%s\r\n     3D Supports %s", 
                                        svd_loop,
                                        sink_cap.edid_mSvd[svd_loop].mCode, (int)temp_dtd.mHActive, (int)temp_dtd.mVActive,
                                        (sink_cap.edid_m20Sink && sink_cap.edid_mSvd[svd_loop].mLimitedToYcc420)?"YCC420 ONLY":(sink_cap.edid_m20Sink && sink_cap.edid_mSvd[svd_loop].mYcc420)?" YCC420 Support ":" ", h3d_info);
                        }
                }
        }
                
        if(sink_cap.edid_mHdmivsdb.mValid) {
                EDPRINTF("---------------------------------------------------------------------");
                EDPRINTF("HDMI1.x VSDB");
                EDPRINTF(" Max_TMDS_Charater_rate %dMHz", sink_cap.edid_mHdmivsdb.mMaxTmdsClk * 5);
                EDPRINTF(" DC_48_%d DC_36_%d DC_30_%d", sink_cap.edid_mHdmivsdb.mDeepColor48, sink_cap.edid_mHdmivsdb.mDeepColor36, sink_cap.edid_mHdmivsdb.mDeepColor30);
                EDPRINTF(" DC_YCC444 %d", sink_cap.edid_mHdmivsdb.mDeepColorY444);
        }
        if(sink_cap.edid_mHdmiForumvsdb.mValid) {
                EDPRINTF("---------------------------------------------------------------------");
        	EDPRINTF("HDMI2.0 VSDB V%d", sink_cap.edid_mHdmiForumvsdb.mVersion);
        	EDPRINTF(" Max_TMDS_Charater_rate %dMHz", sink_cap.edid_mHdmiForumvsdb.mMaxTmdsCharRate * 5);
        	EDPRINTF(" SCDC_%d RR_%d LT340_SCRM_%d", sink_cap.edid_mHdmiForumvsdb.mSCDC_Present, sink_cap.edid_mHdmiForumvsdb.mRR_Capable, sink_cap.edid_mHdmiForumvsdb.mLTS_340Mcs_scramble);
        	EDPRINTF(" Indep_View_%d Dual_View_%d 3D_OSD_%d", sink_cap.edid_mHdmiForumvsdb.mIndependentView, sink_cap.edid_mHdmiForumvsdb.mDualView, sink_cap.edid_mHdmiForumvsdb.m3D_OSD_Disparity);
        	EDPRINTF(" DC48_420_%d DC36_420_%d DC30_420_%d", sink_cap.edid_mHdmiForumvsdb.mDC_48bit_420, sink_cap.edid_mHdmiForumvsdb.mDC_36bit_420, sink_cap.edid_mHdmiForumvsdb.mDC_30bit_420);
        }        

        EDPRINTF("---------------------------------------------------------------------");
        EDPRINTF("Support by this Board");
	for(table_index = 0; table_index < vMaxTableItems; table_index++) {

                if(!(tcc_support_hdmi[table_index].edid_support_status & EDID_SUPPORT)) {
                        //EDPRINTF("E:  [%d] edid_support_status(0x%x) : 0x%x", table_index, tcc_support_hdmi[table_index].edid_support_status, EDID_SUPPORT);
                        continue;
                }

                for(ar_index = HDMI_RATIO_4_3; ar_index < HDMI_RATIO_MAX; ar_index++) {                        
                        //EDPRINTF("T:  [%d] ar[%d] edid_support_status(0x%x) : 0x%x", table_index, ar_index, tcc_support_hdmi[table_index].edid_support_status, EDID_SUPPORT);
                        if(tcc_support_hdmi[table_index].edid_support_status  & ((ar_index==HDMI_RATIO_4_3)?EDID_SUPPORT_4_3:EDID_SUPPORT_16_9)) {
                                
                                vic = tcc_support_hdmi[table_index].vic[ar_index];
                                if(!HDMI_dtd_fill(&temp_dtd, vic)) {
                                        memset(h3d_info, 0, sizeof(h3d_info));
                                        if(tcc_support_hdmi[table_index].edid_support_status & EDID_SUPPORT_3D_FB) {
                                                strcat(h3d_info, "'FP' ");
                                        }
                                        if(tcc_support_hdmi[table_index].edid_support_status & EDID_SUPPORT_3D_TAB) {
                                                strcat(h3d_info, "'TB' ");
                                        }
                                        if(tcc_support_hdmi[table_index].edid_support_status & EDID_SUPPORT_3D_SBS) {
                                                strcat(h3d_info, "'SBS' ");
                                        }
                                        if(!(tcc_support_hdmi[table_index].edid_support_status & (EDID_SUPPORT_3D_FB | EDID_SUPPORT_3D_TAB | EDID_SUPPORT_3D_SBS))) {
                                                strcat(h3d_info, "'None' ");
                                        }

                                        if(tcc_support_hdmi[table_index].edid_support_status & EDID_HDMI_VIC) {
                                                EDPRINTF("[%02d]SVD<%d> %dx%d %dHz HDMI_VIC%s\r\n     3D Supports %s", 
                                                        table_index,
                                                        vic,  (int)temp_dtd.mHActive, (int)temp_dtd.mVActive,
                                                        tcc_support_hdmi[table_index].HdmiSize.frame_hz,
                                                        (sink_cap.edid_m20Sink && (tcc_support_hdmi[table_index].edid_support_status & EDID_SUPPORT_YCC420_ONLY))?"YCC420 ONLY":(sink_cap.edid_m20Sink && (tcc_support_hdmi[table_index].edid_support_status & EDID_SUPPORT_YCC420))?" YCC420 Support ":" ", h3d_info);
                                        } else {
                                                EDPRINTF("[%02d]SVD<%d> %dx%d %dHz%s\r\n     3D Supports %s", 
                                                        table_index,
                                                        vic,  (int)temp_dtd.mHActive, (int)temp_dtd.mVActive,
                                                        tcc_support_hdmi[table_index].HdmiSize.frame_hz,
                                                        (sink_cap.edid_m20Sink && (tcc_support_hdmi[table_index].edid_support_status & EDID_SUPPORT_YCC420_ONLY))?"YCC420 ONLY":(sink_cap.edid_m20Sink && (tcc_support_hdmi[table_index].edid_support_status & EDID_SUPPORT_YCC420))?" YCC420 Support ":" ", h3d_info);
                                        }
                                        

                                }
                        }
                }
        }
        EDPRINTF("=====================================================================");

}

static void print_videoinfo(videoParams_t *pVideo)
{
        if(pVideo) {
                unsigned int pixelclock = pVideo->mDtd.mPixelClock * 1000;
                unsigned int total = ((pVideo->mDtd.mHActive + pVideo->mDtd.mHBlanking) * (pVideo->mDtd.mVActive + pVideo->mDtd.mVBlanking));
                unsigned int hz = (pixelclock / total);
                
                EDPRINTF("\n");
                EDPRINTF("=====================================================================");
                EDPRINTF("EXTENDDISP VIDEO INFO");
                EDPRINTF(" VIC=%d %dx%d%c %dHz", pVideo->mDtd.mCode, 
                                                pVideo->mDtd.mHActive, (pVideo->mDtd.mInterlaced)?(pVideo->mDtd.mVActive*2):pVideo->mDtd.mVActive,
                                                (pVideo->mDtd.mInterlaced)?'i':'p', hz);
                EDPRINTF(" Aspect Ratio %d:%d, ", pVideo->mDtd.mHImageSize, pVideo->mDtd.mVImageSize);
                EDPRINTF(" %d-bpp %s %s ", pVideo->mColorResolution,
                                        (pVideo->mEncodingOut==RGB)?"RGB":(pVideo->mEncodingOut==YCC444)?"YCC444":(pVideo->mEncodingOut==YCC422)?"YCC422":"YCC420", 
                                        pVideo->mHdmi == HDMI ? "HDMI" : "DVI");
                EDPRINTF(" colorimetry %d", pVideo->mColorimetry);
                if( pVideo->mColorimetry == EXTENDED_COLORIMETRY) {
                        EDPRINTF(" ExtendColorimetry %d", (pVideo->mExtColorimetry == COLORMETRY_INVALID)?0:pVideo->mExtColorimetry);
                }
                EDPRINTF("=====================================================================");
        }
}

static int hdmi_check_auto_resolution(void)
{
        /* Configure using Native SVD or HDMI_VIC */
        int table_index;
        dtd_t temp_dtd;
        int native_first;
        int aspect_ratio, vic_aspect_ratio;
        int current_resolution, biggest_resolution;
        int current_vic, biggest_vic, biggest_hz;
        int biggest_table_index = -1;

        /* Initializes the variable */        
        native_first = hdmi_get_native_first();
        aspect_ratio = hdmi_get_PixelAspectRatio();
        
        /* Initializes the variable that stores the maximum value. */
        biggest_vic = biggest_hz = biggest_resolution = 0;
        for(table_index = 0; table_index < vMaxTableItems; table_index++) {
                DPRINTF("[%s] Previous biggest refresh rate vic = %d, index=%d\r\n", __func__, biggest_vic, biggest_table_index);
                
                /* Verify that the resolution is supported by edid. */
                if(!(tcc_support_hdmi[table_index].edid_support_status & EDID_SUPPORT)) {
                        DPRINTF("video index[%d] is not supported", table_index);
                        continue;
                }

                DPRINTF("[%s] edid_support_status=0x%x, base_status=0x%x, aspect_ratio(%d)\r\n", __func__, (tcc_support_hdmi[table_index].edid_support_status & EDID_SUPPORT), ((aspect_ratio==HDMI_RATIO_4_3)?EDID_SUPPORT_4_3:EDID_SUPPORT_16_9), aspect_ratio)
                if(tcc_support_hdmi[table_index].edid_support_status  & ((aspect_ratio==HDMI_RATIO_4_3)?EDID_SUPPORT_4_3:EDID_SUPPORT_16_9)) {
                        vic_aspect_ratio = aspect_ratio;
                } else {
                        if(aspect_ratio == HDMI_RATIO_4_3) {
                                vic_aspect_ratio = HDMI_RATIO_16_9;
                        } else {
                                vic_aspect_ratio = HDMI_RATIO_4_3;
                        }
                }

                /* Select the vic that matches the aspect ratio. */
                current_vic = tcc_support_hdmi[table_index].vic[vic_aspect_ratio];
                
                /* If native_first, find native resolution. */
                if(native_first && (tcc_support_hdmi[table_index].edid_support_status & EDID_NATIVE_VIC)) {
                        /* Check if that vic currently supports dtd normally. */
                        if(!HDMI_dtd_fill(&temp_dtd, current_vic)) {
                                biggest_vic = current_vic;
                                biggest_table_index = table_index;
                                biggest_hz = tcc_support_hdmi[table_index].refresh_rate;
                                DPRINTF("[%s] Find native vic=%d\r\n", __func__, biggest_vic);
                                break;
                        }
                }

                /* Determine the biggest resolution video mode and if it's supported by HDMI TX */
                if(!HDMI_dtd_fill(&temp_dtd, current_vic)) {
                        current_resolution = temp_dtd.mHActive * temp_dtd.mVActive;
                        DPRINTF("[%s] vic[%d] %dx%d %d\r\n", __func__, current_vic, (int)temp_dtd.mHActive, (int)temp_dtd.mVActive, (int)temp_dtd.mPixelClock);

                        if(current_resolution == biggest_resolution) {
                                if(tcc_support_hdmi[table_index].refresh_rate > biggest_hz) {
                                        biggest_resolution = current_resolution;
                                        biggest_vic = current_vic;
                                        biggest_hz = tcc_support_hdmi[table_index].refresh_rate;
                                        biggest_table_index = table_index;
                                        DPRINTF("[%s] Update biggest refresh rate vic = %d, index=%d\r\n", __func__, biggest_vic, biggest_table_index);
                                }
                                else {
                                        /* Nothing */
                                }
                        } else if(current_resolution > biggest_resolution){
                                biggest_resolution = current_resolution;
                                biggest_vic = current_vic;
                                biggest_hz = tcc_support_hdmi[table_index].refresh_rate;
                                biggest_table_index = table_index;
                                DPRINTF("[%s] Update biggest vic = %d, index=%d\r\n", __func__, biggest_vic, biggest_table_index);
                        }
                } 
        }

        
        /* If the supported resolutions are not found, the default resolution will be used. */
        if(biggest_vic == 0 || (HDMI_dtd_fill(&temp_dtd, biggest_vic) < 0)) {
                DPRINTF("[%s] failed last check.. biggest_vic = %d\r\n", __func__, biggest_vic);
                biggest_table_index = -1;
                goto end_process;
        }

        /* parse configs */
        memcpy(&videoParams.mDtd, &temp_dtd, sizeof(dtd_t));

end_process:

        return biggest_table_index;
}

static int hdmi_check_colorspace(int hdmi_mode_idx)
{ 
        int force_autodetectmode= 0;

        if(hdmi_mode_idx < vMaxTableItems) {
                switch(tcc_support_hdmi[hdmi_mode_idx].vic[HDMI_RATIO_16_9]) 
                {
                        case 96:
                        case 97:
                                if(edid_check_support_only_ycc420(&sink_cap, hdmi_mode_idx) || !sink_cap.scdc_ready) {
                                        force_autodetectmode = 1; 
                                        ALOGI("hdmi_check_colorspace force auto detect mode\r\n");
                                }
                                break;
                }

                // Color Space                
                if(videoParams.mHdmi == DVI) {
                        videoParams.mEncodingIn = videoParams.mEncodingOut = RGB;
                        DPRINTF("Selected Encoding = RGB (DVI MODE)\r\n");
                } else {                  
                        if(force_autodetectmode || (unsigned int)hdmi_get_ColorSpace() == AutoDetectMode)
                        {
                                //For HDMI CTS
                                videoParams.mEncodingIn = videoParams.mEncodingOut = edid_check_colorspace(&sink_cap, hdmi_mode_idx);
                                DPRINTF("[%s] Auto Detected  Encoding = %s\r\n", __func__, 
                                        (videoParams.mEncodingOut==RGB)?"RGB":(videoParams.mEncodingOut==YCC444)?"YCC444":(videoParams.mEncodingOut==YCC422)?"YCC422":"YCC420");
                        } else {
                                videoParams.mEncodingIn = videoParams.mEncodingOut = hdmi_get_ColorSpace();
                                DPRINTF("[%s] Selected Encoding = %s\r\n", __func__, 
                                        (videoParams.mEncodingOut==RGB)?"RGB":(videoParams.mEncodingOut==YCC444)?"YCC444":(videoParams.mEncodingOut==YCC422)?"YCC422":"YCC420");
                        }
                }
        }
        return 0;
}

/**
 * @short Check user selected color depth
 * @return Return - Zero indicates success. Nonezero indicates failure.
*/
static int hdmi_check_color_depth(int hdmi_mode_idx)
{        
        int framepacking_mode = 0;
        unsigned int max_tmds_clock_KHz, actual_pixelclock_KHz;
                
        // Get MaxTmdsClockRate
        if(sink_cap.edid_mHdmiForumvsdb.mValid == TRUE) {
                max_tmds_clock_KHz = sink_cap.edid_mHdmiForumvsdb.mMaxTmdsCharRate * 5000;
                DPRINTF("hdmi2.0 max_tmds_clock=%dKHz", max_tmds_clock_KHz);
        } else if(sink_cap.edid_mHdmivsdb.mValid == TRUE) {
                max_tmds_clock_KHz = sink_cap.edid_mHdmivsdb.mMaxTmdsClk * 5000;
                DPRINTF("hdmi1.x max_tmds_clock=%dKHz", max_tmds_clock_KHz);
        }

        // Check FramePacking Mode
        if(videoParams.mHdmiVideoFormat == 2 && videoParams.m3dStructure == 0) {
                framepacking_mode = 1;
        }
        
        // parse Required ColorDepth
        if(videoParams.mColorResolution == COLOR_DEPTH_8)
                videoParams.mColorResolution = hdmi_get_ColorDepth();

        if(videoParams.mColorResolution > COLOR_DEPTH_8) {
                switch(tcc_support_hdmi[hdmi_mode_idx].vic[HDMI_RATIO_16_9]) 
                {
                        case 96: // 2160p @50
                        case 97: // 2160p @60
                                switch(videoParams.mEncodingOut) {
                                        case RGB:
                                        case YCC444:
                                                videoParams.mColorResolution = COLOR_DEPTH_8;
                                                // Notsupport
                                                break;
                                        case YCC420:
                                                if((videoParams.mColorResolution == COLOR_DEPTH_12 && !sink_cap.edid_mHdmiForumvsdb.mDC_36bit_420) ||
                                                   (videoParams.mColorResolution == COLOR_DEPTH_10 && !sink_cap.edid_mHdmiForumvsdb.mDC_30bit_420)) { 
                                                        videoParams.mColorResolution = COLOR_DEPTH_8;
                                                        ALOGW("Force COLOR_DEPTH_8\r\n");
                                                }
                                                break;
                                        default:
                                                // Nothing
                                                break;
                                }
                                break;
                        default:
                                // 2K
                                if(videoParams.mEncodingOut == YCC444 && sink_cap.edid_mHdmivsdb.mDeepColorY444 != 1) {
                                    videoParams.mColorResolution = COLOR_DEPTH_8;
                                }else if(videoParams.mEncodingOut != YCC422) {
                                        if((videoParams.mColorResolution == COLOR_DEPTH_12 && !sink_cap.edid_mHdmivsdb.mDeepColor36) ||
                                           (videoParams.mColorResolution == COLOR_DEPTH_10 && !sink_cap.edid_mHdmivsdb.mDeepColor30)) { 
                                                videoParams.mColorResolution = COLOR_DEPTH_8;
                                        }
                                }
                                break;
                }
        }

        /* Maximum depth that Soc can support.
                                                          00 - reserved 
                                                          01 - 30-bit
                                                          10 - 36-bit */
        if(videoParams.mEncodingOut != YCC422) {
                switch((soc_feature.support_feature_1 >> 1) & 0x3) {
                        case 1:
                                /* hdmi depth is limited by 30-bit */
                                if(videoParams.mColorResolution == COLOR_DEPTH_12) {
                                        videoParams.mColorResolution = COLOR_DEPTH_10;
                                }
                                break;
                        case 2: 
                                /* hdmi depth is limited by 36-bit */
                                break;
                        default:
                                /* hdmi depth is limited by 36-bit */
                                videoParams.mColorResolution = COLOR_DEPTH_8;
                                break;
                }
        }
        
        // CHECK MAX TMDS CLOCK
        switch(videoParams.mColorResolution){
                case COLOR_DEPTH_12:
                        actual_pixelclock_KHz = hdmi_calculate_actual_tmds_bit_ratio(videoParams.mEncodingOut, COLOR_DEPTH_12, videoParams.mDtd.mPixelClock, framepacking_mode);
                        if(actual_pixelclock_KHz > max_tmds_clock_KHz) {
                                videoParams.mColorResolution = COLOR_DEPTH_8;
                                ALOGI("12-bit actual pixel clock (%dKHz) is over SINK's max_tmds_clock(%dKHz)", actual_pixelclock_KHz, max_tmds_clock_KHz);
                        }
                        break;
                case COLOR_DEPTH_10:
                        actual_pixelclock_KHz = hdmi_calculate_actual_tmds_bit_ratio(videoParams.mEncodingOut, COLOR_DEPTH_10, videoParams.mDtd.mPixelClock, framepacking_mode);
                        if(actual_pixelclock_KHz > max_tmds_clock_KHz) {
                                videoParams.mColorResolution = COLOR_DEPTH_8;
                                ALOGI("10-bit actual pixel clock (%dKHz) is over SINK's max_tmds_clock(%dKHz)", actual_pixelclock_KHz, max_tmds_clock_KHz);
                        }
                        break;
                default:
                        actual_pixelclock_KHz = hdmi_calculate_actual_tmds_bit_ratio(videoParams.mEncodingOut, COLOR_DEPTH_8, videoParams.mDtd.mPixelClock, framepacking_mode);
                        if(actual_pixelclock_KHz > max_tmds_clock_KHz) {
                                ALOGI("warring 8-bit actual pixel clock (%dKHz) is over SINK's max_tmds_clock(%dKHz)", actual_pixelclock_KHz, max_tmds_clock_KHz);
                        }
                        break;
        }
        
        return 0;
}

/**
 * @short Check user selected colormetry
 * @return Return - Zero indicates success. Nonezero indicates failure.
*/
static int hdmi_check_colorimetry(videoParams_t *pvideoParams)
{
        int auto_colorimetry = 0;
        int user_colorimetry = hdmi_get_Colorimetry();

        //ALOGI("hdmi check colorimetry is %d\r\n", user_colorimetry);
        
        // Initialize Colorimetry
        pvideoParams->mColorimetry = 0;
        pvideoParams->mExtColorimetry = XV_YCC601;
        pvideoParams->mColorimetryDataBlock = 0;
        
        switch(user_colorimetry)
        {

                case 0:
                        // ITU601
                        if(pvideoParams->mEncodingOut == RGB) {
                                auto_colorimetry = 1;
                        }
                        else {
                                pvideoParams->mColorimetry = ITU601;
                        }
                        break;
                case 1:
                        // ITU709
                        if(pvideoParams->mEncodingOut == RGB) {
                                auto_colorimetry = 1;
                        }
                        else {
                                pvideoParams->mColorimetry = ITU709;
                        }
                        break;
                case 2:
                        // XV_YCC601
                        if(pvideoParams->mEncodingOut == RGB) {
                                auto_colorimetry = 1;
                        }
                        else {
                                pvideoParams->mColorimetry = EXTENDED_COLORIMETRY;
                                pvideoParams->mExtColorimetry = XV_YCC601;
                        }
                        break;
                case 3:
                        // XV_YCC709
                        if(pvideoParams->mEncodingOut == RGB) {
                                auto_colorimetry = 1;
                        }
                        else {
                                pvideoParams->mColorimetry = EXTENDED_COLORIMETRY;
                                pvideoParams->mExtColorimetry = XV_YCC709;
                        }
                        break;
                case 4:
                        // S_YCC601
                        if(pvideoParams->mEncodingOut == RGB) {
                                auto_colorimetry = 1;
                        }
                        else {
                                pvideoParams->mColorimetry = EXTENDED_COLORIMETRY;
                                pvideoParams->mExtColorimetry = S_YCC601;
                        }
                        break;
                case 5:
                        // ADOBE_YCC601
                        if(pvideoParams->mEncodingOut == RGB) {
                                auto_colorimetry = 1;
                        }
                        else {
                                pvideoParams->mColorimetry = EXTENDED_COLORIMETRY;
                                pvideoParams->mExtColorimetry = ADOBE_YCC601;
                        }
                        break;
                case 6:
                        // ADOBE_RGB
                        if(pvideoParams->mEncodingOut == RGB) {
                                pvideoParams->mColorimetry = EXTENDED_COLORIMETRY;
                                pvideoParams->mExtColorimetry = ADOBE_RGB;
                        }
                        else {
                                auto_colorimetry = 1;
                        }
                        break;
                case 7:
                        // ITU-R BT.2020 Y'CC'BCC'RC
                        if(pvideoParams->mEncodingOut == RGB) {
                                auto_colorimetry = 1;
                        }
                        else {
                                pvideoParams->mColorimetry = EXTENDED_COLORIMETRY;
                                pvideoParams->mExtColorimetry = BT2020YCCBCR;
                        }
                        break;
                case 8:
                        // ITU-R BT.2020 Y'CC'BCC'RC
                        pvideoParams->mColorimetry = EXTENDED_COLORIMETRY;
                        pvideoParams->mExtColorimetry = BT2020YCBCR;
                        break;
                default:
                        auto_colorimetry = 1;
                        break;
        }

        if(auto_colorimetry) {
                switch(pvideoParams->mDtd.mCode) {
                        case 1:
                        case 2:
                        case 3:
                        case 6:
                        case 7:
                        case 17:
                        case 18:  
                        case 21:
                        case 22:
                                // below 720p
                                if(pvideoParams->mEncodingOut != RGB) {
                                        pvideoParams->mColorimetry = ITU601;
                                }
                                break;
                        case 95:
                        case 97:
                                // 4k
                                if(videoParams.mColorResolution > COLOR_DEPTH_8) {
                                        // ITU-R BT.2020 Y'CC'BCC'RC
                                        pvideoParams->mColorimetry = EXTENDED_COLORIMETRY;
                                        pvideoParams->mExtColorimetry = BT2020YCBCR;
                                }else {
                                        if(pvideoParams->mEncodingOut != RGB) {
                                               pvideoParams->mColorimetry = ITU709; 
                                        }
                                }
                                break;
                        default:
                                // 720p~2k
                                if(pvideoParams->mEncodingOut != RGB) {
                                        pvideoParams->mColorimetry = ITU709;
                                }
                                break;
                }
        }

        if(supports_metadata0(&sink_cap.edid_mColorimetryDataBlock)) {
                pvideoParams->mColorimetryDataBlock = TRUE;
        }else {
                pvideoParams->mColorimetryDataBlock = FALSE;
        }
        
        return 0;
}




/**
 * @short Check user selected hdmi 3d mode.
 * @param stage Steps to check hdmi 3d.
 *              stage_check_property : This Step to read hdmi 3d properties from system. 
 *              stage_match_edid : This step ensures that the TV supports hdmi 3d mode.
 *                                 If the TV doesnt support hdmi 3d then source does not 
 *                                 transmit 3d video format to that TV.
 * @return Return - Zero indicates success. Nonezero indicates failure.
*/
static int hdmi_check_hdmi3d_mode(int stage)
{
        int tmp_val;
        switch(stage) {
                case stage_check_property:
                        if(videoParams.mHdmi == HDMI) {
                                videoParams.mHdmiVideoFormat  = (unsigned char)(property_get_int("tcc.output.hdmi.video.format", 0) & 0xFF);

                                if(videoParams.mHdmiVideoFormat == 2) {
                                        tmp_val = property_get_int("tcc.output.hdmi.structure.3d", 0);
                                        switch(tmp_val) {
                                                case 0:
                                                        // Frame-Packing
                                                        videoParams.m3dStructure = 0;
                                                        break;
                                                case 1: 
                                                        // Top-and-Bottom
                                                        videoParams.m3dStructure = 6;
                                                        break;
                                                case 2:
                                                        // Side-by-Side(Half)
                                                        videoParams.m3dStructure = 8;
                                                        break;
                                        }

                                        videoParams.m3dExtData = 0;
                                }
                        }
                        break;
                case stage_match_edid:
                        if(videoParams.mHdmiVideoFormat == 2) {
                                if(videoParams.mHdmi != HDMI || !sink_cap.edid_done || !sink_cap.edid_mHdmivsdb.m3dPresent) {
                                        ALOGI("Revert hdmi 3d mode\r\n");
                                        videoParams.mHdmiVideoFormat = 0;
                                        videoParams.m3dStructure = 0;
                                        videoParams.m3dExtData = 0;
                                }
                        }
                        break;
        }
        return 0;
}

/*!
 * @brief               dolbyvision function.
 * @details             This function check to support dolbyvision or not from sink.
 * @version             1.01
 * @return              Status of Video resolution 
 * @retval              -1              Normal Mode
 * @retval              0               Video resolution is not restricted with dolbyvision mode
 * @retval              96 or 97        Video resolution is restricted to 4k 30Hz with dolbyvision mode
 */
static int hdmi_check_dolbyvision_path(void)
{
        int ret = -1;
	#if defined(CONFIG_HDMI_DOLBYVISION_SUPPORT)
	unsigned int max_tmds_clock_KHz;
	int support_dv = support_sink_dv();   //0 for HDR10 testing...
 	int support_dv_path;

        /* Get MaxTmdsClockRate */
        if(sink_cap.edid_mHdmiForumvsdb.mValid == TRUE) {
                max_tmds_clock_KHz = sink_cap.edid_mHdmiForumvsdb.mMaxTmdsCharRate * 5000;
                ALOGI("hdmi2.0 max_tmds_clock=%dKHz", max_tmds_clock_KHz);
        } else if(sink_cap.edid_mHdmivsdb.mValid == TRUE) {
                max_tmds_clock_KHz = sink_cap.edid_mHdmivsdb.mMaxTmdsClk * 5000;
                ALOGI("hdmi1.x max_tmds_clock=%dKHz", max_tmds_clock_KHz);
        }else {
                max_tmds_clock_KHz = 0;
        }

        /**
        * If max tmds clock of sink is 0 then this sink only support 8-bit depth.
        * In this case, this sink can not support Dolby vision.
        * Because Dolby vision is only available on the YCC422 12-bit. */
        support_dv_path = (max_tmds_clock_KHz > 0) ? sink_cap.edid_mYcc422Support : 0;

	#if defined(_TCC8980_) //TCC898x doesn't support dolby output on 480p resolution which use inverted H/VSync Polarity.
        if(videoParams.mDtd.mCode == 1 || videoParams.mDtd.mCode == 2 || videoParams.mDtd.mCode ==3 ) {
        	support_dv = 0;
        }
	#endif

        /**
         * Dolby-IP can't support Interlace output for HDMI. **/
        if(videoParams.mDtd.mInterlaced) {
                support_dv = 0;
                support_dv_path = 0;
        }

	if(support_dv || support_dv_path) {
                ret = 0;
		if(support_dv) {
			libdolbyvision_version_print();
		}
		if(videoParams.mDtd.mCode == 96 || videoParams.mDtd.mCode == 97) {
                        ret = videoParams.mDtd.mCode;
			HDMI_dtd_fill(&videoParams.mDtd, 95);
	    	}
		if(support_sink_dv_ycc422() || !support_dv) {
		    videoParams.mDolbyVision = 1;
		    // If display just uses dolbyvision path then it must use ycc444 format. 
		}else {
		    videoParams.mDolbyVision = 2;
                    /* Tunneling mode needs RGB Full-Range */
                    videoParams.mRgbQuantizationRange = 2;
		}
		videoParams.mEncodingIn = videoParams.mEncodingOut = YCC422;
		videoParams.mColorResolution = COLOR_DEPTH_12;
	}

	// Update TV mode. 
	if(support_dv) {
		videoParams.mDolbyVision |= (2 << 3); // TV mode is DolbyVision
	}
	/**
	 *	In case of HDR10, 
	 * HDR10 output with VIOC-path will be changed whenever HDR10 content playback is started.
	else if(sink_cap.edid_mHdrstaticmetaDataBlock.mValid && sink_cap.edid_mHdrstaticmetaDataBlock.supported_eotm & 4) {
		videoParams.mDolbyVision |= (1 << 3); // TV mode is HDR-10
	}
	*/
	if(support_dv || support_dv_path)
		return ret;
	#endif
	return -1;
}

static int hdmi_update_supported_audio(void)
{
	int i, ret;
	char audio_descriptor_data[SR_ATTR_MAX], *p;
        char *audio_descriptor_data_pointer;
        int audio_descriptor_data_pointer_offset;

	memset(audio_descriptor_data, 0, sizeof(audio_descriptor_data));

	audio_descriptor_data_pointer = audio_descriptor_data;

	if(sink_cap.edid_done) {
		ret = sprintf(audio_descriptor_data_pointer, "%02x ", (sink_cap.edid_mSadIndex));
                if(ret < 0) {
                        goto failed_sprintf;
                }
                audio_descriptor_data_pointer_offset = ret;
		audio_descriptor_data_pointer = audio_descriptor_data + audio_descriptor_data_pointer_offset;

		for(i=0; i<(int)sink_cap.edid_mSadIndex; i++) {
                        if((audio_descriptor_data_pointer_offset + 4) < PROPERTY_VALUE_MAX) {
        			ret = sprintf(audio_descriptor_data_pointer, "%02x ", sink_cap.edid_mSad[i].mFormat);
                                #if 0
        			ret = sprintf(p, "%x %x %x %x ", \
        						sink_cap.edid_mSad[i].mFormat, \
        						sink_cap.edid_mSad[i].mMaxChannels, \
        						sink_cap.edid_mSad[i].mSampleRates, \
        						sink_cap.edid_mSad[i].mByte3);
                                #endif
                                audio_descriptor_data_pointer_offset += ret;
		                audio_descriptor_data_pointer = audio_descriptor_data + audio_descriptor_data_pointer_offset;
                                //printf("audio block sprintf ret : %d \n", ret);
                        }
		}

        }
	else {
                ret = sprintf(audio_descriptor_data_pointer, "%02x ", 0);
                if(ret < 0) {
                        goto failed_sprintf;
                }
		audio_descriptor_data_pointer += ret;
	}


	sprintf(audio_descriptor_data_pointer, "\n");

	property_set("tcc.hdmi.supported.audio", audio_descriptor_data);

#if 0
	property_get("tcc.hdmi.supported.audio", buffer, "");

	p = &buffer[0];

	sscanf(p, "%02x ", &totalNum);
	printf("total audio format : %d \n", totalNum);
	p += 3;

	for(i=0; i<totalNum; i++) {
		ret = sscanf(p, "%02x ", &fmt);
		printf("fmt : %d\n", fmt);
		p += 3;
	}
        #endif
failed_sprintf:
        return 1;
}

static void hdmi_clean_supported_list(void) {
        char data[PROPERTY_VALUE_MAX];

        memset(data, 0, sizeof(data));
		data[0] = '0';
		data[1] = '\n';
        property_set("tcc.hdmi.supported_resolution", data);
        property_set("tcc.hdmi.supported_3d_mode", data);
        property_set("tcc.hdmi.supported_hdr", data);
        property_set("tcc.hdmi.supported.audio", data);
}

/**
 * @short Update supported resolution 
 *        This is old interface. It will be deprecate.
 * @param none
 */
static int hdmi_update_resolution(void)
{
	int table_index;
	int dataindex = 0;
	char data[SR_ATTR_MAX] = {0};
		
	DPRINTF("hdmi_update_resolution\r\n");
	for(table_index = 0; table_index < vMaxTableItems; table_index++)
	{
                if(tcc_support_hdmi[table_index].edid_support_status & EDID_SUPPORT) {
			dataindex += sprintf(data+dataindex, "%02d:%04dx%04d@%02d%c ", 
			table_index, tcc_support_hdmi[table_index].HdmiSize.width, 
			tcc_support_hdmi[table_index].HdmiSize.height, tcc_support_hdmi[table_index].HdmiSize.frame_hz, 
			tcc_support_hdmi[table_index].interlaced?'I':'P');
		}
	}

	property_supported_resolution_set(data);

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
static int hdmi_update_supported_resolution(void)
{
        int table_index, aspect_ratio, current_aspect_ratio;
	int resolution_data_index = 0;
	int h3d_data_index = 0;

	unsigned short update_3d_mode;

	char resolution_data[SR_ATTR_MAX];
	char h3d_mode_data[SR_ATTR_MAX];

	memset(resolution_data, 0, sizeof(resolution_data));
	memset(h3d_mode_data, 0, sizeof(h3d_mode_data));

        aspect_ratio = hdmi_get_PixelAspectRatio();
	DPRINTF("hdmi_update_supported_resolution\r\n");
	for(table_index = 0; table_index < vMaxTableItems; table_index++)
	{
	        if(tcc_support_hdmi[table_index].edid_support_status & EDID_SUPPORT) {
                        DPRINTF("[%s] edid_support_status=0x%x, base_status=0x%x, aspect_ratio(%d)\r\n", __func__, (tcc_support_hdmi[table_index].edid_support_status & EDID_SUPPORT), ((aspect_ratio==HDMI_RATIO_4_3)?EDID_SUPPORT_4_3:EDID_SUPPORT_16_9), aspect_ratio)
                        if(tcc_support_hdmi[table_index].edid_support_status  & ((aspect_ratio==HDMI_RATIO_4_3)?EDID_SUPPORT_4_3:EDID_SUPPORT_16_9)) {
                                current_aspect_ratio = aspect_ratio;
                        } else {
                                if(aspect_ratio == HDMI_RATIO_4_3) {
                                        current_aspect_ratio = HDMI_RATIO_16_9;
                                } else {
                                        current_aspect_ratio = HDMI_RATIO_4_3;
                                }
                        }
                        
                        // Update Supported Resolution 
                        if(resolution_data_index < (PROPERTY_VALUE_MAX -4)) {
		                resolution_data_index += sprintf(resolution_data+resolution_data_index, "%02X ", tcc_support_hdmi[table_index].vic[current_aspect_ratio]);
                        }
                        
                        // Update Supported 3D Mode
                        update_3d_mode = 0;
                        if(tcc_support_hdmi[table_index].edid_support_status  & EDID_SUPPORT_3D_FB) {
                                update_3d_mode |= 1;
                        }   
                        if(tcc_support_hdmi[table_index].edid_support_status  & EDID_SUPPORT_3D_TAB) {
                                update_3d_mode |= (1 << 6);
                        }
                        if(tcc_support_hdmi[table_index].edid_support_status  & EDID_SUPPORT_3D_SBS) {
                                update_3d_mode |= (1 << 8);
                        }
                        h3d_data_index += sprintf(h3d_mode_data+h3d_data_index, "%04x ", update_3d_mode);
                }
                else  {
                        resolution_data_index += sprintf(resolution_data+resolution_data_index, "00 ");
                        h3d_data_index += sprintf(h3d_mode_data+h3d_data_index, "0000 ");
                }
	}
        resolution_data_index += sprintf(resolution_data+resolution_data_index, "\n");
        h3d_data_index += sprintf(h3d_mode_data+h3d_data_index, "\n");
	property_set("tcc.hdmi.supported_resolution", resolution_data);
        property_set("tcc.hdmi.supported_3d_mode", h3d_mode_data);
        return 1;
}


static int hdmi_update_supported_staticmetadata(void)
{      
	char staticmetadata[SR_ATTR_MAX], supported_drm_info[SR_ATTR_MAX];

        memset(staticmetadata, 0, sizeof(staticmetadata));
        memset(supported_drm_info, 0, sizeof(supported_drm_info));
        
	DPRINTF("hdmi_update_supported_staticmetadata\r\n");

        if(sink_cap.edid_done && sink_cap.edid_mHdrstaticmetaDataBlock.mValid) {
                sprintf(staticmetadata, "%01x %02x %02x %02x %02x %02x\n",
                        sink_cap.edid_mHdrstaticmetaDataBlock.mValid,
                        sink_cap.edid_mHdrstaticmetaDataBlock.supported_eotm,
                        sink_cap.edid_mHdrstaticmetaDataBlock.Supported_staticmetadatadescriptor,
                        sink_cap.edid_mHdrstaticmetaDataBlock.DesiredContentMaxLuminancedata,
                        sink_cap.edid_mHdrstaticmetaDataBlock.DesiredContentMaxFrameaverageLuminancedata,
                        sink_cap.edid_mHdrstaticmetaDataBlock.DesiredContentMinLuminancedata);
        }
        else {
                sprintf(staticmetadata, "%01x %02x %02x %02x %02x %02x\n", 0, 0, 0, 0, 0, 0);
        }

        if(sink_cap.edid_done) {
                #if defined(CONFIG_HDMI_DOLBYVISION_SUPPORT)
                if(sink_cap.edid_mHdrstaticmetaDataBlock.mValid && support_sink_dv())
                        sprintf(supported_drm_info, "SDR,HDR,DV");
                else if(sink_cap.edid_mHdrstaticmetaDataBlock.mValid && !support_sink_dv())
                        sprintf(supported_drm_info, "SDR,HDR");
                else if(!sink_cap.edid_mHdrstaticmetaDataBlock.mValid && support_sink_dv())
                        sprintf(supported_drm_info, "SDR,DV");
                #else
                if(sink_cap.edid_mHdrstaticmetaDataBlock.mValid)
                        sprintf(supported_drm_info, "SDR,HDR");		
                #endif
                else
                        sprintf(supported_drm_info, "SDR");
        }
        else {
                sprintf(supported_drm_info, "SDR");
        }
	
        #if 0
        ALOGI("hdmi_update_supported_staticmetadata [%01x %02x %02x %02x %02x %02x]\r\n", 
                        sink_cap.edid_mHdrstaticmetaDataBlock.mValid,
                        sink_cap.edid_mHdrstaticmetaDataBlock.supported_eotm,
                        sink_cap.edid_mHdrstaticmetaDataBlock.Supported_staticmetadatadescriptor,
                        sink_cap.edid_mHdrstaticmetaDataBlock.DesiredContentMaxLuminancedata,
                        sink_cap.edid_mHdrstaticmetaDataBlock.DesiredContentMaxFrameaverageLuminancedata,
                        sink_cap.edid_mHdrstaticmetaDataBlock.DesiredContentMinLuminancedata);
        #endif
        property_set("tcc.hdmi.supported_hdr", staticmetadata);
        property_set("tcc.hdmi.supported_drms", supported_drm_info);
        return 1;
}



      
static unsigned int get_time_diff_ms(struct timespec *prev_time, struct timespec *current_time) {
        unsigned int diff_time_ms;
        long diff_sec, diff_nano;
        if (current_time->tv_nsec >= prev_time->tv_nsec) {
                diff_sec = current_time->tv_sec - prev_time->tv_sec;
                diff_nano = current_time->tv_nsec - prev_time->tv_nsec;
                diff_nano = (diff_nano > 1000000)?(diff_nano/1000000):0;
                diff_time_ms = (1000 * diff_sec) + diff_nano;
        } else {
                diff_sec = current_time->tv_sec - prev_time->tv_sec - 1;
                diff_nano = current_time->tv_nsec - prev_time->tv_nsec;
                diff_nano += 1000000000;
                diff_nano = (diff_nano > 1000000)?(diff_nano/1000000):0;
                diff_time_ms = (1000 * diff_sec) + diff_nano;
        }
        return diff_time_ms;
}



static int hdmi_display_output_compatibility_check(void)
{
        struct timespec hdmi_current_time;
        unsigned int diff_time_ms;
        unsigned int wait_compatibility_time_ms = COMPATIBILITY_WAIT_TIME;

        if(hdmi_compatibility_check && wait_compatibility_time_ms > 0) {
                clock_gettime(CLOCK_REALTIME, &hdmi_current_time);
                diff_time_ms = get_time_diff_ms(&hdmi_setting_time, &hdmi_current_time);
                if(hdmi_compatibility_check && diff_time_ms > wait_compatibility_time_ms) {
                        //HDMIDrv_request_reset_phy();
                        hdmi_compatibility_check = 0;
                }
        }
        return 0;
}

static int hdmi_display_check_printlog(void)
{        
        int hdmi_printlog = 0;
        char value[PROPERTY_VALUE_MAX];

        hdmi_printlog = property_get_int("persist.sys.hdmi_printlog", 0);
        if(hdmi_printlog == 1) {
                value[0] = '0';
                value[1] = '\0';
                property_set("persist.sys.hdmi_printlog", value); 
                print_edidinfo();
                print_videoinfo(&videoParams);        
        }
        return hdmi_printlog;
}

int hdmi_display_runtime_check(void)
{
        int runtime_status = 0;
        if(sink_cap.edid_m20Sink && sink_cap.edid_mHdmiForumvsdb.mSCDC_Present) {
                HDMI_get_CharacterErrorDetection();
        }
        hdmi_display_output_compatibility_check();

        /* If soc supports hdr-drm, daemon enable to hdmi-drm function */
        if(soc_feature.support_feature_1 & (1 << 4)) {
                if(HDMI_DRM_runtime_check(&videoParams, &sink_cap) < 0) {
                        hdmi_display_output_set(0, 0);
                        runtime_status = -1;
                        ALOGI("HDMI required to restart\r\n");
                }
        }
        hdmi_display_check_printlog();

        return runtime_status;
}

static void hdmi_display_output_compatibility_check_time(void)
{
        if(videoParams.mDtd.mCode == 97) {
                hdmi_compatibility_check = 1;
                clock_gettime(CLOCK_REALTIME, &hdmi_setting_time);
        }
}

static int hdmi_display_output_scdc_check_before_unmute(void)
{
        struct timespec hdmi_current_time, hdmi_prev_time;
        unsigned int diff_time_ms;
        unsigned int wait_time_ms = 90; // 90ms
        //unsigned int wait_time_ms = 1000; // 1sec

        clock_gettime(CLOCK_REALTIME, &hdmi_prev_time);
        do {
                clock_gettime(CLOCK_REALTIME, &hdmi_current_time);
                diff_time_ms = get_time_diff_ms(&hdmi_prev_time, &hdmi_current_time);
                if(diff_time_ms > wait_time_ms ) {
                        ALOGI("hdmi_display_output_scdc_check_before_unmute %dms\r\n", diff_time_ms);
                        break;
                }
                if(sink_cap.edid_m20Sink && sink_cap.edid_mHdmiForumvsdb.mSCDC_Present) {
                        HDMI_get_CharacterErrorDetection();
                }
                usleep(10000);
        }while(1);

        /* If soc supports hdr-drm, daemon enable to hdmi-drm function */
        if(soc_feature.support_feature_1 & (1 << 4)) {
                HDMI_DRM_Set_enable();
        }

        HDMI_Set_AvMute(0);
        return 0;
}

int hdmi_display_output_set(char onoff, unsigned int hdmi_hw_cts_mode)
{
        int hdmi_mode_idx;
        int video_diff, audio_diff;
        char automode = 0;
        int PixelAspectRatio;

        char value[PROPERTY_VALUE_MAX];
        memset(value, 0, PROPERTY_VALUE_MAX);

        videoParams_t tcc_video_before;
        audioParams_t tcc_audio_before;

        memcpy(&tcc_video_before, &videoParams, sizeof(videoParams));
        memcpy(&tcc_audio_before, &audioParams, sizeof(audioParams));

        DPRINTF("[%s] onoff:%d", __func__, onoff);

	
	HDMI_set_scdc_ready(0);
	
	if(onoff)
	{
                int hdmi_output_mode;
		videoParams.mHdmi = HDMI;

		hdmi_output_mode = property_get_int("persist.sys.hdmi_mode", 0);
                
		// prepare
		if(!HDMI_Get_PowerStatus() && HDMI_Get_NeedPreConfig()) {
                        videoParams_t pre_videoParams;
                        HDMI_dtd_fill_with_refreshrate(&pre_videoParams.mDtd, 1, 60000);
                        pre_videoParams.mEncodingIn = RGB;
                        pre_videoParams.mEncodingOut = RGB;
                        hdmi_check_colorimetry(&pre_videoParams);
                        FB_video_generator_prepare();
                        FB_video_generator_config(&pre_videoParams);
                        FB_video_generator_extra(&pre_videoParams);
                        HDMI_Api_PreConfig();   
		}
                #if defined(CONFIG_HDMI_DOLBYVISION_SUPPORT)
                dolbyvision_vsdb_reset();
                #endif
                if((soc_feature.support_feature_1 & (1 << 3)) == 0) {  
                        hdmi_display_set_power(1);
                }
                usleep(10000); // wait for stable.
		
		edid_read_cap(&sink_cap);
 
                edid_update_machin_id(&sink_cap);
                start_cec_process(sink_cap.edid_done?sink_cap.edid_mHdmivsdb.mPhysicalAddress:CEC_COMMON_PHYSICAL_ADDRESS, sink_cap.parse_vendor_info.manufacturer_name);

		if(sink_cap.edid_m20Sink && sink_cap.edid_mHdmiForumvsdb.mSCDC_Present) {
			unsigned int version;
			if(HDMI_get_scdc_sink_version(&version) < 0) {
			        sink_cap.scdc_ready  = 0;
			        ALOGI("HDMI sink not support scdc\r\n");
			}
			else  {
			        sink_cap.scdc_ready = 1;
			        HDMI_set_scdc_ready(1);
			        DPRINTF("HDMI SINK SCDC VERSION is %d\r\n", version);
			}
			// Update Source Version
			HDMI_set_scdc_source_version(SCDC_SOURCE_VERSION);
		}
		else {
                        sink_cap.scdc_ready  = 0;
                        ALOGI("HDMI sink is not HDMI2.0\r\n");
		}

                               
		//persist.sys.hdmi_mode - Auto 125, HDMI:0, DVI:1
		if(hdmi_output_mode == 0) {
			videoParams.mHdmi = HDMI;
		} else if(hdmi_output_mode == 1) {
			videoParams.mHdmi = DVI; 
		} else if(hdmi_output_mode == AutoDetectMode) {
		        if(sink_cap.edid_mHdmivsdb.mId == 0x000C03) {
                                videoParams.mHdmi = HDMI;
                        }
                        else {
                                videoParams.mHdmi = DVI;
                        }
		} else {
			//Default HDMI
			videoParams.mHdmi = HDMI;
		}
                hdmi_set_detected_mode(videoParams.mHdmi);
                
        	/* Update supported resolution information */
                edid_update_support_hdmi_table(&sink_cap, &videoParams, &soc_feature);

		if(hdmi_display_check_printlog())
                        print_edidinfo();

                DPRINTF("[%s] set default dtd fill vic=%d, ref = 0\r\n", __func__, tcc_support_hdmi[TCC_HDMI_DEFAULT_VIC].vic[TCC_HDMI_DEFAULT_ASPEC]);
                // Set Default Value. 
                HDMI_dtd_fill(&videoParams.mDtd, tcc_support_hdmi[TCC_HDMI_DEFAULT_VIC].vic[TCC_HDMI_DEFAULT_ASPEC]);

		#if defined(HDMI_V1_4)
		//tcc_video.videoSrc = HDMI_SOURCE_EXTERNAL;
		//tcc_video.hdmi_3d_format = hdmi_get_hdmi_3d_format();
		#endif

		audioParams.mInterfaceType = hdmi_get_AudioInputPort();
		audioParams.mSamplingFrequency = hdmi_get_AudioSamplingRate();
		audioParams.mPacketType= hdmi_get_AudioOutPacket();
		audioParams.mAudioType = hdmi_get_audio_type();

		hdmi_mode_idx = HDMI_GetVideoResolution(dispman_daemon_stbmode);
		
		ALOGI("[%s] hdmi_mode_idx is %d", __func__, hdmi_mode_idx);

                
		if(hdmi_mode_idx >= vMaxTableItems)
		{
		        ALOGI("%s Auto Mode", __func__);
			automode = 1;
			hdmi_mode_idx = 0;
		}

                switch(automode) {
                        case 0:
                                /** 
                                 * Process Manual Mode
                                 */
        		        PixelAspectRatio = hdmi_get_PixelAspectRatio();
                                DPRINTF("[%s] HDMI_dtd_fill tcc_support_hdmi[%d].vic[%d]=%d\r\n", __func__, hdmi_mode_idx, PixelAspectRatio, tcc_support_hdmi[hdmi_mode_idx].vic[PixelAspectRatio]);

                                /* 1. Check Dvi limitation. */
                                if(videoParams.mHdmi == DVI && tcc_support_hdmi[hdmi_mode_idx].not_support_dvi == 1) {
                                        /* Nothing. Goto Auto Mode */
                                        ALOGE("[%s] Failed Manual mode wit DVI... Change to Auto Detect mode\r\n", __func__);
                                } else /* 2. Check whether the selected resolution is available. */ 
                                if(tcc_support_hdmi[hdmi_mode_idx].edid_support_status & EDID_SUPPORT) {
                                        /**
                                         * 2.1 Check Aspect-Ratio 
                                         */
                                        DPRINTF("[%s] edid_support_status=0x%x, base_status=0x%x, aspect_ratio(%d)\r\n", 
                                                        __func__, 
                                                        (tcc_support_hdmi[hdmi_mode_idx].edid_support_status & EDID_SUPPORT), 
                                                        ((PixelAspectRatio==HDMI_RATIO_4_3)?EDID_SUPPORT_4_3:EDID_SUPPORT_16_9), 
                                                        PixelAspectRatio);
                                        if(!(tcc_support_hdmi[hdmi_mode_idx].edid_support_status  & ((PixelAspectRatio==HDMI_RATIO_4_3)?EDID_SUPPORT_4_3:EDID_SUPPORT_16_9))) {
                                                if(PixelAspectRatio == HDMI_RATIO_4_3) {
                                                        PixelAspectRatio = HDMI_RATIO_16_9;
                                                } else {
                                                        PixelAspectRatio = HDMI_RATIO_4_3;
                                                }
                                        }
                                        HDMI_dtd_fill(&videoParams.mDtd, tcc_support_hdmi[hdmi_mode_idx].vic[PixelAspectRatio]);
                                        break;
                                } else /* 3. Force Manual Mode */
                                {
                                        if(tcc_support_hdmi[hdmi_mode_idx].vic[PixelAspectRatio] == 0) {
                                                if(PixelAspectRatio == HDMI_RATIO_4_3) {
                                                        PixelAspectRatio = HDMI_RATIO_16_9;
                                                } else {
                                                        PixelAspectRatio = HDMI_RATIO_4_3;
                                                }
                                        }
                                        HDMI_dtd_fill(&videoParams.mDtd, tcc_support_hdmi[hdmi_mode_idx].vic[PixelAspectRatio]);    
                                        break;
                                }
                        case 1:
                                hdmi_mode_idx = hdmi_check_auto_resolution();
                                
                                /* Process Default Mode */
                                if((hdmi_mode_idx < 0) || (hdmi_mode_idx >= vMaxTableItems)) {
                                        /* Resolution could not be set automatically */
                                        hdmi_mode_idx  = TCC_HDMI_DEFAULT_VIC;
                                        HDMI_dtd_fill(&videoParams.mDtd, tcc_support_hdmi[hdmi_mode_idx].vic[hdmi_mode_idx]);
                                        ALOGE("[%s] force default resolution. failed auto resolution\r\n", __func__);
                                } else {
                                        DPRINTF("[%s] Auto Detected mode: %s, vic is %d , mode index=%d\r\n", __func__, (videoParams.mHdmi == HDMI)?"HDMI":"DVI", videoParams.mDtd.mCode, hdmi_mode_idx);
                                }
                                break;
                }

                if(videoParams.mHdmi == HDMI) {
                        videoParams.mHdmi20 = sink_cap.edid_m20Sink;
                }
                               
                DPRINTF("[%s] Selected mode: %s, hdmi_mode_idx (%d) vic (%d) hdmi2(%d)\r\n", __func__, (videoParams.mHdmi == HDMI)?"HDMI":"DVI", hdmi_mode_idx, videoParams.mDtd.mCode, videoParams.mHdmi20);

                hdmi_check_colorspace(hdmi_mode_idx);
                hdmi_check_hdmi3d_mode(stage_check_property);
                /* If soc supports dolbyvision, daemon enable to dolbyvision path */
                if(soc_feature.support_feature_1 & (1 << 5)) {
                        hdmi_check_dolbyvision_path();
                }
                /* If soc supports hdr-drm, daemon enable to hdmi-drm function */
                if(soc_feature.support_feature_1 & (1 << 4)) {
                        HDMI_DRM_Set_Videoparams(&videoParams, &sink_cap);
                }
                hdmi_check_color_depth(hdmi_mode_idx);
                hdmi_check_colorimetry(&videoParams);

                // redkakeru
                hdmi_update_resolution();
                hdmi_update_supported_resolution();
                hdmi_update_supported_staticmetadata();
                hdmi_update_supported_audio();

                hdmi_supportmodeset(&tcc_support_hdmi[hdmi_mode_idx].HdmiSize);
                hdmi_set_detected_resolution(hdmi_mode_idx);

                video_diff = memcmp(&videoParams, &tcc_video_before, sizeof(videoParams));
                audio_diff = memcmp(&audioParams, &tcc_audio_before, sizeof(audioParams));
                DPRINTF("[%s] HDMI DIFF  video : %d , audio : %d ", __func__, video_diff, audio_diff);

                if(video_diff || audio_diff)
                {
			hdmi_audio_params_reset();
					
                        if(video_diff) {
                                if(HDMI_Get_NeedPreConfig()) {
                                        ALOGI("Disable HDMI\r\n");
                                        HDMI_Api_Disable();				
                                        FB_video_generator_stop();
                                }
                                
                                if(hdmi_display_check_printlog())
                                        print_videoinfo(&videoParams);
                                FB_video_generator_prepare();
                                FB_video_generator_config(&videoParams);
                                /****************************************************************
                                UI replication with SBS/TNB regardless whether the TV supports 3d 
                                mode. The following code,hdmi_check_hdmi3d_mode(stage_match_edid),
                                to provide this functionality
                                ----------------------------------------------------------------
                                ****************************************************************/
                                hdmi_check_hdmi3d_mode(stage_match_edid);
                                if(videoParams.mColorimetry == EXTENDED_COLORIMETRY && videoParams.mExtColorimetry == BT2020YCBCR) {
                                        DPRINTF("[%s] Enable Lut for BT2020\r\n", __func__);
                                        Lut_enable_bt2020(1);
                                }
                                else {
                                        DPRINTF("[%s] Disable Lut for BT2020\r\n", __func__);
                                        Lut_enable_bt2020(0);
                                }
                                FB_video_generator_extra(&videoParams);
                                hdmi_api_output_set();
                                hdmi_display_output_compatibility_check_time();
                                hdmi_display_output_scdc_check_before_unmute();
                        } else if(audio_diff) {
                                // audio diff only. 
                                hdmi_audio_output_set();
                        }
                        //HDMIStart();

                }

                hdmi_set_output_detected(true);

                if(videoParams.mHdmiVideoFormat == 2) {
                        property_set("tcc.output.hdmi_3d_enable","1");
                }else {
                        property_set("tcc.output.hdmi_3d_enable","0");
                }

        }
        else
        {
                /* we have to wait END_VSYNC from player */
                hdmi_set_output_detected(false);
                usleep(200000);

                hdmi_compatibility_check = 0;
                HDMI_Set_AvMute(1);
				
		// CEA8610F F.3.6 Video Timing Transition (AVMUTE Recommendation)
		usleep(90000);
		stop_cec_process();


                HDMI_Api_Disable();				
                FB_video_generator_stop();
		Lut_enable_bt2020(0);			
                hdmi_video_params_reset();
                hdmi_audio_params_reset();
                hdmi_product_params_reset();
                hdmi_hdcp_params_reset();
                // cleanup edid information. 
                memset(&sink_cap, 0, sizeof(sink_cap));

                hdmi_clean_supported_list();
                hdmi_clean_support_hdmi_table();
                if((soc_feature.support_feature_1 & (1 << 3)) == 0) {  
                        hdmi_display_set_power(0);
                }
        }
        return 0;
}


int hdmi_display_cabledetect(void)
{
	int hdmi_hpd = 0;
	hdmi_hpd = HDMI_HPDCheck();
	HPDcallback(hdmi_hpd);

	return hpd_state;
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

	return 1;
}

int hdmi_sink_support_hdmi3d()
{
        return (sink_cap.edid_done && sink_cap.edid_mHdmivsdb.m3dPresent && (videoParams.mHdmi != DVI))?1:0;
}
