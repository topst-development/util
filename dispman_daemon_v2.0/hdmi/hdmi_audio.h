/*!
* TCC Version 1.0
* Copyright (c) Telechips Inc.
* All rights reserved 
*  \file        hdmi_audio.h
*  \brief       HDMI Audio application header
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
#ifndef HDMI_V2_0_AUDIO_DEAMON
#define HDMI_V2_0_AUDIO_DEAMON

enum HDMIAudioType
{
	AUDIO_HBR,
	AUDIO_ASP_DDP,
	AUDIO_ASP_DST_AC3,
	AUDIO_ASP_LPCM,
	AUDIO_ASP_PCM,
	AUDIO_MAX
};

enum SamplingFreq
{
        /** 32KHz sampling frequency */
        SF_32KHZ = 0,
        /** 44.1KHz sampling frequency */
        SF_44KHZ,
        /** 88.2KHz sampling frequency */
        SF_88KHZ,
        /** 176.4KHz sampling frequency */
        SF_176KHZ,
        /** 48KHz sampling frequency */
        SF_48KHZ,
        /** 96KHz sampling frequency */
        SF_96KHZ,
        /** 192KHz sampling frequency */
        SF_192KHZ
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


#endif
