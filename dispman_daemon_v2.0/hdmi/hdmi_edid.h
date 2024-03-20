/*!
* TCC Version 1.0
* Copyright (c) Telechips Inc.
* All rights reserved 
*  \file        hdmi_edid.h
*  \brief       HDMI edid loader source
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
#ifndef __HDMI_EDID_H__
#define __HDMI_EDID_H__

#include <utils/types.h> 
#include <utils/bit_operation.h>
#include <hdmi_v2_0/include/hdmi_ioctls.h>
#include <hdmi/edid_type.h>
#include <hdmi/colorimetry_data_block.h>
#include <hdmi/hdrstaticmeta_data_block.h>
#include <hdmi/hdmiforumvsdb.h>
#include <hdmi/hdmivsdb.h>
#include <hdmi/monitor_range_limits.h>
#include <hdmi/short_audio_desc.h>
#include <hdmi/short_video_desc.h>
#include <hdmi/speaker_alloc_data_block.h>
#include <hdmi/video_cap_data_block.h>
#include <hdmi/edid_parser.h>

#ifdef __cplusplus
extern "C" {
#endif

void edid_read_cap(sink_edid_t * sink);

int edid_check_hdmi_resolution(sink_edid_t * sink, int video_idx, int aspect_ratio);
int edid_check_common_resolution(sink_edid_t * sink, int video_idx, int aspect_ratio, int hdmi);
int edid_check_dvi_resolution(sink_edid_t * sink, int video_idx, int aspect_ratio);
encoding_t edid_check_colorspace(sink_edid_t * sink, int hdmi_mode_idx);
int edid_check_support_only_ycc420(sink_edid_t * sink, int hdmi_mode_idx);
int edid_check_support_ycc420(sink_edid_t * sink, int hdmi_mode_idx);
int edid_update_support_hdmi_table(sink_edid_t * sink, videoParams_t *videoParams, hdmi_soc_features *soc_feature);
int edid_update_machin_id(sink_edid_t * sink);

#ifdef __cplusplus
}
#endif


#endif
