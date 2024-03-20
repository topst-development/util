/*!
* TCC Version 1.0
* Copyright (c) Telechips Inc.
* All rights reserved 
*  \file        colorimetry_data.c
*  \brief       HDMI clorimetry control source
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
#include <utils/types.h>
#include <utils/bit_operation.h>
#include <hdmi/colorimetry_data_block.h>


void colorimetry_data_block_reset( colorimetryDataBlock_t * cdb)
{
	cdb->mByte3 = 0;
	cdb->mByte4 = 0;
	cdb->mValid = FALSE;
}

int colorimetry_data_block_parse( colorimetryDataBlock_t * cdb, u8 * data)
{
	colorimetry_data_block_reset(cdb);
	if ((data != 0) && (bit_field(data[0], 0, 5) == 0x03) &&
		(bit_field(data[0], 5, 3) == 0x07) && (bit_field(data[1], 0, 8) == 0x05)) {
		cdb->mByte3 = data[2];
		cdb->mByte4 = data[3];
		cdb->mValid = TRUE;
		return TRUE;
	}
	return FALSE;
}

int supports_xv_ycc709( colorimetryDataBlock_t * cdb)
{
	return (bit_field(cdb->mByte3, 1, 1) == 1) ? TRUE : FALSE;
}

int supports_xv_ycc601( colorimetryDataBlock_t * cdb)
{
	return (bit_field(cdb->mByte3, 0, 1) == 1) ? TRUE : FALSE;
}

int supports_s_ycc601( colorimetryDataBlock_t * cdb)
{
	return (bit_field(cdb->mByte3, 2, 1) == 1) ? TRUE : FALSE;
}

int supports_adobe_ycc601( colorimetryDataBlock_t * cdb)
{
	return (bit_field(cdb->mByte3, 3, 1) == 1) ? TRUE : FALSE;
}

int supports_adobe_rgb( colorimetryDataBlock_t * cdb)
{
	return (bit_field(cdb->mByte3, 4, 1) == 1) ? TRUE : FALSE;
}

int supports_BT2020cYCC( colorimetryDataBlock_t * cdb)
{
        return (bit_field(cdb->mByte3, 5, 1) == 1) ? TRUE : FALSE;
}

int supports_BT2020YCC( colorimetryDataBlock_t * cdb)
{
        return (bit_field(cdb->mByte3, 6, 1) == 1) ? TRUE : FALSE;
}

int supports_BT2020RGB( colorimetryDataBlock_t * cdb)
{
        return (bit_field(cdb->mByte3, 7, 1) == 1) ? TRUE : FALSE;
}

int supports_metadata0( colorimetryDataBlock_t * cdb)
{
	return (bit_field(cdb->mByte4, 0, 1) == 1) ? TRUE : FALSE;
}

int supports_metadata1( colorimetryDataBlock_t * cdb)
{
	return (bit_field(cdb->mByte4, 1, 1) == 1) ? TRUE : FALSE;
}

int supports_metadata2( colorimetryDataBlock_t * cdb)
{
	return (bit_field(cdb->mByte4, 2, 1) == 1) ? TRUE : FALSE;
}

int supports_metadata3( colorimetryDataBlock_t * cdb)
{
	return (bit_field(cdb->mByte4, 3, 1) == 1) ? TRUE : FALSE;
}
