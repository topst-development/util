/*!
* TCC Version 1.0
* Copyright (c) Telechips Inc.
* All rights reserved 
*  \file        hdmi_audio.cpp
*  \brief       HDMI Audio application source
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

#include <utils/types.h>

#include <hdmi_v2_0/include/hdmi_ioctls.h>
#include <utils/Log.h>
#include <utils/properties.h>

#include "hdmi_audio.h"

#define LOG_NDEBUG					0
#define HDMI_SOUND_DEBUG     		0
#define HDMI_SOUND_THREAD_DEBUG		0

#define LOG_TAG				""
#if HDMI_SOUND_DEBUG
#define DPRINTF(args...)    ALOGD(args)
#else
#define DPRINTF(args...)
#endif

#if HDMI_SOUND_THREAD_DEBUG
#define TPRINTF(args...)    ALOGD(args)
#else
#define TPRINTF(args...)
#endif


#define CMD_SIZE 50

typedef struct {
	char            hdmi_tx_name[CMD_SIZE];
	int 		hdmi_tx_driver;
	
	audioParams_t 	current_parameter;
	pthread_t	thread_id;
}hdmitx_audio_t;


int hdmitx_open_device(hdmitx_audio_t* audio)
{
	strcpy(audio->hdmi_tx_name, "/dev/dw-hdmi-tx");
	audio->hdmi_tx_driver = open(audio->hdmi_tx_name,O_RDWR);
	if(audio->hdmi_tx_driver < 0){
		ALOGE("Could not open %s device....\n",audio->hdmi_tx_name);
		return audio->hdmi_tx_driver;
	}
	return 0;
}

int hdmitx_close_device(hdmitx_audio_t* audio)
{
	close(audio->hdmi_tx_driver);
	ALOGI("close %s device....\n",audio->hdmi_tx_name);
	return 0;
}

void hdmitx_audio_reset(audioParams_t* params)
{
	params->mInterfaceType = I2S;
	params->mCodingType = PCM;
	params->mChannelAllocation = 0;
	params->mSampleSize = 16;
	params->mSamplingFrequency = 44100;
	params->mLevelShiftValue = 0;
	params->mDownMixInhibitFlag = 0;
	params->mIecCopyright = 1;
	params->mIecCgmsA = 3;
	params->mIecPcmMode = 0;
	params->mIecCategoryCode = 0;
	params->mIecSourceNumber = 1;
	params->mIecClockAccuracy = 0;
	params->mPacketType = AUDIO_SAMPLE;
	params->mClockFsFactor = 64;
	params->mDmaBeatIncrement = DMA_UNSPECIFIED_INCREMENT;
	params->mDmaThreshold = 0;
	params->mDmaHlock = 0;
	params->mGpaInsertPucv = 0;

}

unsigned int hdmitx_get_port_setting(void)
{
	char			value[PROPERTY_VALUE_MAX];
	unsigned int	PortSetting;
	
	memset(value, (int)NULL, PROPERTY_VALUE_MAX);
	
    property_get("persist.sys.spdif_setting", value, "");
	PortSetting = atoi(value);

	return PortSetting;
}

unsigned int hdmitx_get_audio_type(void)
{
	char			value[PROPERTY_VALUE_MAX];
	unsigned int	AudioType=0;
	
	memset(value, (int)NULL, PROPERTY_VALUE_MAX);
	
    property_get("tcc.hdmi.audio_type", value, "");
	if(value[0] >='0' && value[0] < '5'){
		AudioType = atoi(value);
	}
	else{
		AudioType = 0;
	}
	return AudioType;
}

unsigned int hdmitx_get_audio_Channels(void)
{
	char          	value[PROPERTY_VALUE_MAX];
	unsigned int	Channel;
	
	memset(value, (int)NULL, PROPERTY_VALUE_MAX);

    property_get("tcc.audio.channels", value, "");

	Channel = atoi(value);

    return Channel;
}

unsigned int hdmi_get_audio_SamplingRate(void)
{
	char          value[PROPERTY_VALUE_MAX];
	SamplingFreq  AudioSamplingRate;
	
	memset(value, (int)NULL, PROPERTY_VALUE_MAX);

	property_get("tcc.audio.sampling_rate", value, "");	// SPDIF Planet 20121107
    AudioSamplingRate = (SamplingFreq)atoi(value);

 	switch (AudioSamplingRate)
    {
        case SF_32KHZ:
			return 32000;

        case SF_44KHZ:
			return 44100;

        case SF_88KHZ:
			return 88200;

        case SF_176KHZ:
			return 176000;

        case SF_48KHZ:
			return 48000;

        case SF_96KHZ:
			return 96000;

        case SF_192KHZ:
			return 192000;

        default:
            return 0;
    }
}

void hdmitx_get_audio_Type(audioParams_t* user)
{
	unsigned int		PortSetting = 0;
	unsigned int		AudioType = 0;
	unsigned int		AudioChannel = 0;
	unsigned int		AudioSampleRate = 0;

	PortSetting 	=	hdmitx_get_port_setting();
	AudioType		=	hdmitx_get_audio_type();
	AudioChannel	=	hdmitx_get_audio_Channels();
	AudioSampleRate	=	hdmi_get_audio_SamplingRate();

	switch(PortSetting)
	{
		case AUDIO_OUTPORT_DEFAULT:
			user->mInterfaceType		= I2S;
			user->mPacketType			= AUDIO_SAMPLE;
			user->mChannelAllocation	= 2;
			user->mCodingType			= PCM;
            break;

        case AUDIO_OUTPORT_DAI_LPCM:
			user->mInterfaceType		= I2S;
			user->mPacketType			= AUDIO_SAMPLE;
			user->mChannelAllocation	= AudioChannel;
			user->mCodingType			= PCM;			
			break;
			
		case AUDIO_OUTPORT_DAI_HBR:
			user->mInterfaceType	= I2S;
			if(AudioType == AUDIO_HBR)
			{
				user->mPacketType			= HBR_STREAM;
				user->mChannelAllocation	= 8;
				user->mCodingType			= DTS;
				
			}
			else if(AudioType == AUDIO_ASP_LPCM || AudioType == AUDIO_ASP_PCM)
			{
				user->mPacketType			= AUDIO_SAMPLE;
				user->mChannelAllocation	= AudioChannel;
				user->mCodingType			= PCM;					
			}
			else
			{
				user->mPacketType			= AUDIO_SAMPLE;
				user->mChannelAllocation	= 2;
				user->mCodingType			= DTS;				
			}
			break;
			
		case AUDIO_OUTPORT_SPDIF_PCM:
			user->mInterfaceType		= SPDIF;
			user->mPacketType			= AUDIO_SAMPLE;
			user->mChannelAllocation	= 2;
			user->mCodingType			= PCM;
			break;
			
		case AUDIO_OUTPORT_SPDIF_BITSTREAM:
			user->mInterfaceType		= SPDIF;
			user->mPacketType			= AUDIO_SAMPLE;
			user->mChannelAllocation	= 2;
			user->mCodingType			= PCM;			
			break;
			
		default:
			user->mInterfaceType		= I2S;
			user->mPacketType			= AUDIO_SAMPLE;
			user->mChannelAllocation	= 2;
			user->mCodingType			= PCM;			
			break;
	}

	user->mSamplingFrequency	=	AudioSampleRate;

	return;
}



int hdmitx_audio_check_parm(audioParams_t* cfg)
{
	int return_value = 0;
	audioParams_t user;
	
	hdmitx_get_audio_Type(&user);
	

	if((user.mInterfaceType != cfg->mInterfaceType) && (user.mInterfaceType != INTERFACE_NOT_DEFINED))
	{
		cfg->mInterfaceType = user.mInterfaceType;

		if(cfg->mInterfaceType == I2S)
		{
			cfg->mClockFsFactor = 64;

		}
		else if(cfg->mInterfaceType == SPDIF)
		{
			cfg->mClockFsFactor = 512;
		}
			
		return_value = -1;
	}
	
	if((user.mCodingType != cfg->mCodingType) && (user.mCodingType != CODING_NOT_DEFINED))
	{
		cfg->mCodingType = user.mCodingType;
		return_value = -1;
	}

	if((user.mChannelAllocation != cfg->mChannelAllocation) && (user.mChannelAllocation != 0))
	{
		cfg->mChannelAllocation = user.mChannelAllocation;
		return_value = -1;		
	}

	if((user.mPacketType != cfg->mPacketType) && (user.mPacketType != PACKET_NOT_DEFINED))
	{
		cfg->mPacketType = user.mPacketType;
		return_value = -1;		
	}
	
	if((user.mSamplingFrequency != cfg->mSamplingFrequency) && (user.mSamplingFrequency != 0))
	{
		cfg->mSamplingFrequency = user.mSamplingFrequency;
		return_value = -1;		
	}

	return return_value;
}

static void* hdmitx_audio_thread(void* arg)
{
	hdmitx_audio_t* audio = (hdmitx_audio_t*)arg;
	int ret;

	while(1)
	{
		if(hdmitx_audio_check_parm(&audio->current_parameter) < 0)
		{
			ret = ioctl(audio->hdmi_tx_driver, HDMI_AUDIO_CONFIG, &audio->current_parameter);
			if(ret) {
				ALOGE("Failed HDMI_AUDIO_CONFIG IOCTL [%d]\n", ret);
			}
		}
		usleep(500000);
	}
	return 0;
}

int hdmitx_init_audio(hdmitx_audio_t* audio)
{
	int ret;

	audioParams_t* parm = &audio->current_parameter;
	
	ret = ioctl(audio->hdmi_tx_driver, HDMI_AUDIO_INIT, NULL);
	if(ret) {
            ALOGE("Failed HDMI_AUDIO_INIT IOCTL [%d]\n", ret);
            return -1;
    }

	ret = ioctl(audio->hdmi_tx_driver, HDMI_AUDIO_CONFIG, parm);
	if(ret) {
            ALOGE("Failed HDMI_AUDIO_CONFIG IOCTL [%d]\n", ret);
            return -1;
    }
	
	if (pthread_create(&audio->thread_id, NULL, &hdmitx_audio_thread, (void *)audio)){
		ALOGE("hdmi audio thread init fail....\n");
		return -1;
	}
	
	return 0;
}

int hdmitx_deinit_audio(hdmitx_audio_t* audio)
{
	return hdmitx_close_device(audio);
}

int main()
{
	hdmitx_audio_t audio;

	if(hdmitx_open_device(&audio) < 0)
	{
		ALOGE("close hdmi audio deamon.... \n");
		return 0;
	}

	hdmitx_audio_reset(&audio.current_parameter);

	if(hdmitx_init_audio(&audio) < 0)
	{
		ALOGE("close hdmi audio deamon.... \n");
		return 0;

	}

	while(1) {
                usleep(100000);
        }
	
	return 0;
		
}
