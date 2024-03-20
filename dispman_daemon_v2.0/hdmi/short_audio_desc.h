/*!
* TCC Version 1.0
* Copyright (c) Telechips Inc.
* All rights reserved 
*  \file        short_audio_desc.h
*  \brief       HDMI audio descript header
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
#ifndef SHORTAUDIODESC_H_
#define SHORTAUDIODESC_H_
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file
 * Short Audio Descriptor.
 * Found in Audio Data Block (shortAudioDesc_t *sad, CEA Data Block Tage Code 1).
 * Parse and hold information from EDID data structure
 */
/** For detailed handling of this structure, refer to documentation of the functions */
typedef struct {
	u8 mFormat;

	u8 mMaxChannels;

	u8 mSampleRates;

	u8 mByte3;
} shortAudioDesc_t;

void sad_reset(shortAudioDesc_t * sad);

/**
 * Parse Short Audio Descriptor
 */
int sad_parse( shortAudioDesc_t * sad, u8 * data);

/**
 *@return the sample rates byte, where bit 7 is always 0 and rates are sorted respectively starting with bit 6:
 * 192 kHz  176.4 kHz  96 kHz  88.2 kHz  48 kHz  44.1 kHz  32 kHz
 */
//u8 sad_GetSampleRates( shortAudioDesc_t * sad);

int sad_support32k( shortAudioDesc_t * sad);

int sad_support44k1( shortAudioDesc_t * sad);

int sad_support48k( shortAudioDesc_t * sad);

int sad_support88k2( shortAudioDesc_t * sad);

int sad_support96k( shortAudioDesc_t * sad);

int sad_support176k4( shortAudioDesc_t * sad);

int sad_support192k( shortAudioDesc_t * sad);

int sad_support16bit( shortAudioDesc_t * sad);

int sad_support20bit( shortAudioDesc_t * sad);

int sad_support24bit( shortAudioDesc_t * sad);
#ifdef __cplusplus
}
#endif

#endif	/* SHORTAUDIODESC_H_ */
