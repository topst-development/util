/*!
* TCC Version 1.0
* Copyright (c) Telechips Inc.
* All rights reserved 
*  \file        hdmistaticmeta_data_block.h
*  \brief       HDMI drm metadata conrtl header
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

#ifndef __HDR_STATIC_DATA_BLOCK_H__
#define __HDR_STATIC_DATA_BLOCK_H__

/**
 * @file
 * HDR Static Metadata Data Block class.
 * Holds and parses the HDR Static Metadata Data Block information.
 */
#ifdef __cplusplus
        extern "C" {
#endif

typedef struct {
	u8 supported_eotm;
        u8 Supported_staticmetadatadescriptor;
	u8 DesiredContentMaxLuminancedata;
        u8 DesiredContentMaxFrameaverageLuminancedata;
        u8 DesiredContentMinLuminancedata;
        u8 mValid;
} hdrstaticmetadata_block_t;

int hdrstaticmetadata_block_parse( hdrstaticmetadata_block_t * std, u8 * data);
int supports_eotf_sdr(hdrstaticmetadata_block_t * std);
int supports_eotf_hdr(hdrstaticmetadata_block_t * std);
int supports_eotf_st_2084(hdrstaticmetadata_block_t * std) ;
int supports_eotf_hlg(hdrstaticmetadata_block_t * std);

#ifdef __cplusplus
}
#endif

#endif	/* __HDR_STATIC_DATA_BLOCK_H__ */
