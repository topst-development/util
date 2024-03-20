/*!
* TCC Version 1.0
* Copyright (c) Telechips Inc.
* All rights reserved 
*  \file        colorimetry_data.h
*  \brief       HDMI clorimetry control header
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

#ifndef COLORIMETRYDATABLOCK_H_
#define COLORIMETRYDATABLOCK_H_

/**
 * @file
 * Colorimetry Data Block class.
 * Holds and parses the Colorimetry data-block information.
 */
#ifdef __cplusplus
        extern "C" {
#endif

typedef struct {
	u8 mByte3;

	u8 mByte4;

	int mValid;

} colorimetryDataBlock_t;

void colorimetry_data_block_reset( colorimetryDataBlock_t * cdb);

int colorimetry_data_block_parse( colorimetryDataBlock_t * cdb, u8 * data);

int supports_xv_ycc709( colorimetryDataBlock_t * cdb);

int supports_xv_ycc601( colorimetryDataBlock_t * cdb);

int supports_s_ycc601( colorimetryDataBlock_t * cdb);

int supports_adobe_ycc601( colorimetryDataBlock_t * cdb);

int supports_adobe_rgb( colorimetryDataBlock_t * cdb);

int supports_BT2020cYCC( colorimetryDataBlock_t * cdb);

int supports_BT2020YCC( colorimetryDataBlock_t * cdb);

int supports_BT2020RGB( colorimetryDataBlock_t * cdb);

int supports_metadata0( colorimetryDataBlock_t * cdb);

int supports_metadata1( colorimetryDataBlock_t * cdb);

int supports_metadata2( colorimetryDataBlock_t * cdb);

int supports_metadata3( colorimetryDataBlock_t * cdb);

#ifdef __cplusplus
}
#endif

#endif	/* COLORIMETRYDATABLOCK_H_ */
