/*!
* TCC Version 1.0
* Copyright (c) Telechips Inc.
* All rights reserved 
*  \file        hdmistaticmeta_data_block.c
*  \brief       HDMI drm metadata conrtl source
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
#include <string.h>
#include <utils/types.h>
#include <utils/bit_operation.h>
#include <hdmi/hdrstaticmeta_data_block.h>

int hdrstaticmetadata_block_parse( hdrstaticmetadata_block_t * std, u8 * data)
{
	memset(std, 0, sizeof(hdrstaticmetadata_block_t));
	if ((data != 0) && (bit_field(data[0], 5, 3) == 0x07) && (bit_field(data[1], 0, 8) == 0x06)) {

                std->supported_eotm = data[2];
                std->Supported_staticmetadatadescriptor = data[3];
                std->DesiredContentMaxLuminancedata = data[4];
                std->DesiredContentMaxFrameaverageLuminancedata = data[5];
                std->DesiredContentMinLuminancedata = data[6];
                        
		std->mValid = TRUE;
		return TRUE;
	}
	return FALSE;
}

int supports_eotf_sdr(hdrstaticmetadata_block_t * std) 
{
        return (std && std->mValid && (std->supported_eotm & (1 << 0)));
}

int supports_eotf_hdr(hdrstaticmetadata_block_t * std) 
{
        return (std && std->mValid && (std->supported_eotm & (1 << 1)));
}

int supports_eotf_st_2084(hdrstaticmetadata_block_t * std) 
{
        return (std && std->mValid && (std->supported_eotm & (1 << 2)));
}

int supports_eotf_hlg(hdrstaticmetadata_block_t * std) 
{
        return (std && std->mValid && (std->supported_eotm & (1 << 3)));
}