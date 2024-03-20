/*!
* TCC Version 1.0
* Copyright (c) Telechips Inc.
* All rights reserved 
*  \file        edid_parser.c
*  \brief       HDMI edid parser source
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
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <utils/Log.h>
#include <utils/types.h> 
#include <utils/bit_operation.h>
#include <hdmi_v2_0/include/hdmi_ioctls.h>
#include "edid_type.h"
#include "colorimetry_data_block.h"
#include "hdrstaticmeta_data_block.h"
#include "hdmiforumvsdb.h"
#include "hdmivsdb.h"
#include "monitor_range_limits.h"
#include "short_audio_desc.h"
#include "short_video_desc.h"
#include "speaker_alloc_data_block.h"
#include "video_cap_data_block.h"
#include "edid_parser.h"
#if defined(CONFIG_HDMI_DOLBYVISION_SUPPORT)
#include <dolbyvision_vsdb.h>
#endif

#define HDMI_APP_DEBUG  0
#define LOG_TAG         "[HDMI_EDID ]"
#if HDMI_APP_DEBUG
#define DPRINTF(args...)    ALOGI(args)
#else
#define DPRINTF(args...)
#endif


const int DTD_SIZE = 0x12;
static int videoParams_GetCeaVicCode(int hdmi_vic_code)
{
        int vic = -1;
        switch(hdmi_vic_code)
        {
        case 1:
                vic = 95;
                break;
        case 2:
                vic = 94;
                break;
        case 3:
                vic = 93;
                break;
        case 4:
                vic = 98;
                break;
        }
        
        return vic;
}


static int dtd_parse(dtd_t * dtd, u8 data[18])
{
        int ret = -1;
        
        dtd->mCode = 0;
        dtd->mPixelRepetitionInput = 0;
        dtd->mLimitedToYcc420 = 0;
        dtd->mYcc420 = 0;

        dtd->mPixelClock = byte_to_word(data[1], data[0]);      /*  [10000Hz] */
        if (dtd->mPixelClock < 0x01) {  /* 0x0000 is defined as reserved */
                //ALOGI("[%s] invalid dts\r\n", __func__);
                goto end_process;
        }
        dtd->mPixelClock *= 10000; 
        dtd->mHActive = concat_bits(data[4], 4, 4, data[2], 0, 8);
        dtd->mHBlanking = concat_bits(data[4], 0, 4, data[3], 0, 8);
        dtd->mHSyncOffset = concat_bits(data[11], 6, 2, data[8], 0, 8);
        dtd->mHSyncPulseWidth = concat_bits(data[11], 4, 2, data[9], 0, 8);
        dtd->mHImageSize = concat_bits(data[14], 4, 4, data[12], 0, 8);
        dtd->mHBorder = data[15];

        dtd->mVActive = concat_bits(data[7], 4, 4, data[5], 0, 8);
        dtd->mVBlanking = concat_bits(data[7], 0, 4, data[6], 0, 8);
        dtd->mVSyncOffset = concat_bits(data[11], 2, 2, data[10], 4, 4);
        dtd->mVSyncPulseWidth = concat_bits(data[11], 0, 2, data[10], 0, 4);
        dtd->mVImageSize = concat_bits(data[14], 0, 4, data[13], 0, 8);
        dtd->mVBorder = data[16];

        if (bit_field(data[17], 4, 1) != 1) {   /* if not DIGITAL SYNC SIGNAL DEF */
                //ALOGE("Invalid DTD Parameters");
                goto end_process;
        }
        if (bit_field(data[17], 3, 1) != 1) {   /* if not DIGITAL SEPATATE SYNC */
                //ALOGE("Invalid DTD Parameters");
                goto end_process;
        }
        /* no stereo viewing support in HDMI */
        dtd->mInterlaced = bit_field(data[17], 7, 1) == 1;
        dtd->mVSyncPolarity = bit_field(data[17], 2, 1) == 1;
        dtd->mHSyncPolarity = bit_field(data[17], 1, 1) == 1;
        ret = 0;
        
end_process:
        return ret;
}


int _edid_struture_parser( struct edid * edid, sink_edid_t * sink)
{
        /** 
         * variales for Vendor Information.
         */
	unsigned char tmp;
        
        dtd_t tmpDtd;

        /** 
         * variales for detailed/standard timing.
         */
        int loop, standard_width, standard_height, standard_hz, standard_ar, detailed_index = 0;
        struct std_timing  * standard_timing;

	if(edid->header[0] != 0){
		ALOGE("[%s] Invalid Header\n", __func__);
		return -1;
	}
        
        /**
         * parse detailed timing 
         */
        memset(sink->parse_detailed_timing_dtd, 0, sizeof(sink->parse_detailed_timing_dtd));
        for (loop=0; loop < 4; loop++) {
		struct detailed_timing * detailed_timing = &(edid->detailed_timings[loop]);
		if(detailed_timing->pixel_clock == 0){
			struct detailed_non_pixel * npixel = &(detailed_timing->data.other_data);

			switch (npixel->type){
        			case EDID_DETAIL_MONITOR_NAME:
        				DPRINTF("[%s] Monitor name: %s\n", __func__, npixel->data.str.str);
        				break;
        			case EDID_DETAIL_MONITOR_RANGE:
        				break;

			}
		}
		else { //Detailed Timing Definition
		        if(!dtd_parse(&tmpDtd, (u8*)detailed_timing) ) {
                                sink->parse_detailed_timing_dtd[detailed_index++] = tmpDtd;
                        }
		}
        }

        /**
         * parse standard timing 
         */
        for (loop = 0; loop < 8; loop++) {
                standard_timing = &(edid->standard_timings[loop]);
        
                /* it is necessary to fill each unused byte, of the byte pairs, with 0x01 as padding. */
                if(standard_timing->vfreq_aspect == 0 && (standard_timing->hsize == 0 || standard_timing->hsize == 1)) {
                        
                        continue;
                }

                standard_height = 0;
                /* Value Stored (in hex) =(Horizontal addressable pixels ¡Ë¢çA 8) - 31 */
                standard_width = (standard_timing->hsize * 8) + 248;
                /* Value Stored (in binary) = Field Refresh Rate (in Hz) - 60 */
                standard_hz = (standard_timing->vfreq_aspect & 0x3F) + 60;
                
                switch(standard_timing->vfreq_aspect >> 6) {
                        #if 0
                        case 0:
                                // 16:10
                                standard_height = (10*standard_width)/16;
                                break;
                        case 2: 
                                // 5:4
                                standard_height = (4*standard_width)/5;
                                break;
                        #endif
                        case 1:
                                // 4:3
                                standard_height = (3*standard_width)/4;
                                standard_ar = 0; /* HDMI_RATIO_4_3 */
                                break;

                        case 3:
                                // 16:9
                                standard_height = (9*standard_width)/16;
                                standard_ar = 1; /* HDMI_RATIO_16_9 */
                                break;
                        default:
                                //ALOGI("[%s] standard_width = %d, vfreq_aspect = %d", __func__, standard_width, standard_timing->vfreq_aspect);
                                break;
                }
        
                if(standard_height > 0) {
                        sink->parse_standard_timing[loop].hactive = standard_width;
                        sink->parse_standard_timing[loop].vactive = standard_height;
                        sink->parse_standard_timing[loop].frame_hz = standard_hz;
                        sink->parse_standard_timing[loop].aspect = standard_ar;
                }
        }


        // Parse Vendor Information
        memset(sink->parse_vendor_info.manufacturer_name, 0, sizeof(sink->parse_vendor_info.manufacturer_name));
        tmp = bit_field(edid->mfg_id[0], 2, 5);
        if(tmp > 0)
            sink->parse_vendor_info.manufacturer_name[0] = '@'+tmp;
        tmp = bit_field(edid->mfg_id[0], 0, 2) << 3;
        tmp |= bit_field(edid->mfg_id[1], 5, 3);
        if(tmp > 0)
            sink->parse_vendor_info.manufacturer_name[1] = '@'+tmp;
        tmp = bit_field(edid->mfg_id[1], 0, 5);
        if(tmp > 0)
            sink->parse_vendor_info.manufacturer_name[2] = '@'+tmp;

        sink->parse_vendor_info.Product_id = edid->prod_code[0] | (edid->prod_code[1] << 8);

        sink->parse_vendor_info.Serial = edid->serial;
	return TRUE;
}

int _edid_cea_extension_parser( u8 * buffer, sink_edid_t * edidExt, u16 edid_size)
{
	int i;
	dtd_t tmpDtd;
	u8 offset = buffer[2];
        int detailed_index = 10;
        //int number_of_native_dtd;

	if (buffer[1] < 0x03){
		ALOGE("Invalid version for CEA Extension block, only rev 3 or higher is supported");
		return -1;
	}

	edidExt->edid_mYcc422Support = bit_field(buffer[3],	4, 1) == 1;
	edidExt->edid_mYcc444Support = bit_field(buffer[3],	5, 1) == 1;
	edidExt->edid_mBasicAudioSupport = bit_field(buffer[3], 6, 1) == 1;
	edidExt->edid_mUnderscanSupport = bit_field(buffer[3], 7, 1) == 1;
        //number_of_native_dtd = bit_field(buffer[3], 0, 4);

	DPRINTF("EDID YCC422(%d). YCC444(%d) \r\n", edidExt->edid_mYcc422Support, edidExt->edid_mYcc444Support);
	if (offset != 4) {
		for (i = 4; i < offset; i += edid_parser_ParseDataBlock(buffer + i, edidExt)) ;
	}

        for(i = 0; i< 10;i++) {
                if(edidExt->parse_detailed_timing_dtd[i].mPixelClock == 0) {
                        detailed_index = i;
                        break;
                }
        }
        
        /*
         * Read Detailed Timing on Extension EDID Block. 
         */
        for (i = offset; i < (int)(edid_size - DTD_SIZE) && detailed_index < 10 ; i += DTD_SIZE) {
		if (!dtd_parse(&tmpDtd, buffer + i)) {
                        edidExt->parse_detailed_timing_dtd[detailed_index++] = tmpDtd;
		}
	}

        /**
         * Parse hdmi vic and add it to end of short video description list.
         */
        for(i = 0; i < edidExt->edid_mHdmivsdb.mHdmiVicCount ; i++) {
                int CeaVicCode;
                shortVideoDesc_t tmpSvd;

                CeaVicCode = videoParams_GetCeaVicCode(edidExt->edid_mHdmivsdb.mHdmiVic[i]);
                if(CeaVicCode < 0)  {
                        // SKIP.. It is Invalid CeaVic Code. 
                        continue;
                }
                tmpSvd.mCode = CeaVicCode;
                tmpSvd.mYcc420 = 0;
                tmpSvd.mNative = 0;
                tmpSvd.mHdmiVic = 1;
                edidExt->edid_mSvd[edidExt->edid_mSvdIndex++] = tmpSvd;
        }
        
	return TRUE;
}

int edid_parser( u8 * buffer, sink_edid_t *edidExt, u16 edid_size)
{
	int ret = 0;
	switch (buffer[0]){
	case 0x00:
		ret = _edid_struture_parser((struct edid *) buffer, edidExt);
		break;
	case CEA_EXT:
		ret = _edid_cea_extension_parser(buffer, edidExt, edid_size);
		break;
	case VTB_EXT:
	case DI_EXT:
	case LS_EXT:
	case MI_EXT:
	default:
		DPRINTF("Block 0x%02x not supported\n", buffer[0]);
	}
	return ret;
}

int edid_parser_CeaExtReset( sink_edid_t *edidExt)
{
	unsigned i;
	edidExt->edid_m20Sink = FALSE;
	for (i = 0; i < sizeof(edidExt->edid_mMonitorName); i++) {
		edidExt->edid_mMonitorName[i] = 0;
	}
	edidExt->edid_mBasicAudioSupport = FALSE;
	edidExt->edid_mUnderscanSupport = FALSE;
	edidExt->edid_mYcc422Support = FALSE;
	edidExt->edid_mYcc444Support = FALSE;
	edidExt->edid_mYcc420Support = FALSE;
	edidExt->edid_mDtdIndex = 0;
	edidExt->edid_mSadIndex = 0;
	edidExt->edid_mSvdIndex = 0;
        
	hdmivsdb_reset(&edidExt->edid_mHdmivsdb);
	hdmiforumvsdb_reset(&edidExt->edid_mHdmiForumvsdb);
	monitor_range_limits_reset(&edidExt->edid_mMonitorRangeLimits);
	video_cap_data_block_reset(&edidExt->edid_mVideoCapabilityDataBlock);
	colorimetry_data_block_reset(&edidExt->edid_mColorimetryDataBlock);
	speaker_alloc_data_block_reset(&edidExt->edid_mSpeakerAllocationDataBlock);
	return TRUE;
}

int edid_parser_ParseDataBlock(u8 * data, sink_edid_t *edidExt)
{
        int i, c;
	u8 tag = bit_field(data[0], 5, 3);
	u8 length = bit_field(data[0], 0, 5);
	shortAudioDesc_t tmpSad;
	shortVideoDesc_t tmpSvd;
	tmpSvd.mLimitedToYcc420 = 0;
	tmpSvd.mYcc420 = 0;
	u8 tmpYcc420All = 0;
	u8 tmpLimitedYcc420All = 0;
	u32 ieeeId;
        unsigned int edid_cnt;
	#if defined(CONFIG_HDMI_DOLBYVISION_SUPPORT)
	dolbyvision_vsdb_parse(data);
	#endif
	switch (tag) {
	case 0x1:		/* Audio Data Block */
		//LOGGER(SNPS_DEBUG,"EDID: Audio datablock parsing\n");
		for (c = 1; c < (length + 1); c += 3) {
			sad_parse(&tmpSad, data + c);
			if (edidExt->edid_mSadIndex < (sizeof(edidExt->edid_mSad) / sizeof(shortAudioDesc_t))) {
				edidExt->edid_mSad[edidExt->edid_mSadIndex++] = tmpSad;
			} else {
				DPRINTF("buffer full - SAD ignored\n");
			}
		}
                #if defined(CONFIG_DUMP_EDID_AUDIOBLOCK)
		printf("Audio datablock parsing dump \n");
		printf("CEA Short Audio Descriptor total number : %d, length : %d \n", edidExt->edid_mSadIndex, length);
		{
			int i = 0;

			for(i=0; i<edidExt->edid_mSadIndex; i++) {
				printf("Audio format code = 0x%x, Max Channel : 0x%x, SampleRates : 0x%x, Byte 3 : 0x%x \n", \
						edidExt->edid_mSad[i].mFormat, \
						edidExt->edid_mSad[i].mMaxChannels, \
						edidExt->edid_mSad[i].mSampleRates, \
						edidExt->edid_mSad[i].mByte3);
			}
		}
                #endif
		break;
	case 0x2:		/* Video Data Block */
		//LOGGER(SNPS_DEBUG,"EDID: Video datablock parsing\n");
		for (c = 1; c < (length + 1); c++) {
			svd_parse(&tmpSvd, data[c]);
			if (edidExt->edid_mSvdIndex < (sizeof(edidExt->edid_mSvd) / sizeof(shortVideoDesc_t))) {
				edidExt->edid_mSvd[edidExt->edid_mSvdIndex++] = tmpSvd;
			} else {
				DPRINTF("buffer full - SVD ignored\n");
			}
		}
		break;
	case 0x3:		/* Vendor Specific Data Block HDMI or HF */
		//LOGGER(SNPS_DEBUG,"EDID: VSDB HDMI and HDMI-F\n ");
		ieeeId = byte_to_dword(0x00, data[3], data[2], data[1]);
		if (ieeeId == 0x000C03) {	/* HDMI */
			if (hdmivsdb_parse(&edidExt->edid_mHdmivsdb, data) != TRUE) {
				DPRINTF("HDMI Vendor Specific Data Block corrupt\n");
				break;
			}
			DPRINTF("EDID HDMI VSDB parsed\n");
		} else {
			if (ieeeId == 0xC45DD8) {	/* HDMI-F */
				DPRINTF("Sink is HDMI 2.0 because haves HF-VSDB\n");
				edidExt->edid_m20Sink = TRUE;
				if (hdmiforumvsdb_parse(&edidExt->edid_mHdmiForumvsdb, data) != TRUE) {
					DPRINTF("HDMI Vendor Specific Data Block corrupt\n");
					break;
				}
                                
			} else {
				DPRINTF("Vendor Specific Data Block not parsed ieeeId: 0x%x\n",
						ieeeId);
			}
		}
		break;
	case 0x4:		/* Speaker Allocation Data Block */
		//LOGGER(SNPS_DEBUG,"SAD block parsing");
		if (speaker_alloc_data_block_parse(&edidExt->edid_mSpeakerAllocationDataBlock, data) != TRUE) {
			ALOGE("Speaker Allocation Data Block corrupt\n");
		}
		break;
	case 0x7:{
		//LOGGER(SNPS_DEBUG,"EDID CEA Extended field 0x07\n");
		u8 extendedTag = data[1];
		switch (extendedTag) {
		case 0x00:	/* Video Capability Data Block */
			//LOGGER(SNPS_DEBUG,"Video Capability Data Block\n");
			if (video_cap_data_block_parse(&edidExt->edid_mVideoCapabilityDataBlock, data) != TRUE) {
				ALOGE("Video Capability Data Block corrupt\n");
			}
			break;
                case 0x04:      /* HDMI Video Data Block */
                        //LOGGER(SNPS_NOTICE,"HDMI Video Data Block\n");
                        break;
		case 0x05:	/* Colorimetry Data Block */
			//LOGGER(SNPS_DEBUG,"Colorimetry Data Block");
			if (colorimetry_data_block_parse(&edidExt->edid_mColorimetryDataBlock, data) != TRUE) {
				ALOGE("Colorimetry Data Block corrupt\n");
			}
			break;
                case 0x06:      /* HDR Static Metadata Data Block */
                        if (hdrstaticmetadata_block_parse(&edidExt->edid_mHdrstaticmetaDataBlock, data) != TRUE) {
                                ALOGE("HDR Static Metadata Data Block corrupt\n");
                        }
                        break;
		case 0x12:	/* HDMI Audio Data Block */
			//LOGGER(SNPS_NOTICE,"HDMI Audio Data Block\n");
			break;
		case 0xe:
			/** If it is a YCC420 VDB then VICs can ONLY be displayed in YCC 4:2:0 */
                        /** 7.5.10 YCBCR 4:2:0 Video Data Block */
			/** If Sink has YCC Datablocks it is HDMI 2.0 */
			edidExt->edid_m20Sink = TRUE;
			tmpLimitedYcc420All = (bit_field(data[0], 0, 5) == 1 ? 1 : 0);
			edid_parser_updateYcc420(edidExt, tmpYcc420All, tmpLimitedYcc420All);

			for (i = 0; i < (bit_field(data[0], 0, 5) - 1); i++) {
				/** Lenght includes the tag byte*/
				tmpSvd.mCode = data[2 + i];
				tmpSvd.mNative = 0;
				tmpSvd.mLimitedToYcc420 = 1;
				for (edid_cnt = 0;edid_cnt < edidExt->edid_mSvdIndex;edid_cnt++) {
					if (edidExt->edid_mSvd[edid_cnt].mCode ==tmpSvd.mCode) {
						edidExt->edid_mSvd[edid_cnt] =	tmpSvd;
						goto concluded;
					}
				}
				if (edidExt->edid_mSvdIndex < (sizeof(edidExt->edid_mSvd) /  sizeof(shortVideoDesc_t)))
				{
					edidExt->edid_mSvd[edidExt->edid_mSvdIndex] = tmpSvd;
					edidExt->edid_mSvdIndex++;
				} else {
					ALOGE("buffer full - YCC 420 DTD ignored");
				}
				concluded: ;
			}
			break;
		case 0x0f:
			/** If it is a YCC420 CDB then VIC can ALSO be displayed in YCC 4:2:0 */
			edidExt->edid_m20Sink = TRUE;
			//LOGGER(SNPS_NOTICE,"YCBCR 4:2:0 Capability Map Data Block");
			int svdNr = 0;
			int icnt = 0;
			int data_index;
			/* If YCC420 CMDB is bigger than 1, then there is SVD info to parse */
			if(bit_field(data[0], 0, 5) > 1){
                                for(data_index = 2; data_index < (length+1); data_index++) {
                                        for (icnt = 0; icnt < 8; icnt++) {
                                                /** Lenght includes the tag byte*/
                                                if((bit_field(data[data_index], icnt, 1) & 0x01)){
                                                        svdNr = icnt + (8*(data_index-2));
                                                        tmpSvd.mCode = edidExt->edid_mSvd[svdNr].mCode;
                                                        tmpSvd.mYcc420 = 1;
                                                        edidExt->edid_mSvd[svdNr] = tmpSvd;
                                                        //ALOGI("YCC420 can = svd[%d] %d\r\n", svdNr, tmpSvd.mCode);
                                                }
                                        }
                                }
				/* Otherwise, all SVDs present at the Video Data Block support YCC420*/
			}else
			{
				tmpYcc420All = (bit_field(data[0], 0, 5) == 1 ? 1 : 0);
				edid_parser_updateYcc420(edidExt, tmpYcc420All, tmpLimitedYcc420All);
			}
			break;
		default:
			//LOGGER(SNPS_WARN,"Extended Data Block not parsed %d\n", extendedTag);
			break;
		}
		break;
	}
	default:
		//LOGGER(SNPS_WARN,"Data Block not parsed %d\n", tag);
		break;
	}
	return length + 1;
}


/**
 * @short Update the YCC420 block processing result to SVD.
 * @param[in] edidExt
 * @param[in] Ycc420All If true, sink can support YCC420.
 *            (i.e, do not indicate which RGB, YCBCR 4:4:4, and/or YCBCR 
 *            4:2:2 modes are supported)
 * @param[in] LimitedToYcc420All If true, sink can only support YCC420.
 *            (i.e., do not support RGB, YCBCR 4:4:4, or YCBCR 4:2:2 sampling modes)
 */
void edid_parser_updateYcc420(sink_edid_t *edidExt, u8 Ycc420All, u8 LimitedToYcc420All)
{
	u16 edid_cnt = 0;
	for (edid_cnt = 0;edid_cnt < edidExt->edid_mSvdIndex;edid_cnt++) {
                /* HDMI VIC mode does not support YCC420 */
                if(edidExt->edid_mSvd[edid_cnt].mHdmiVic) {
                        continue;
                }
                
		switch (edidExt->edid_mSvd[edid_cnt].mCode){
		case 96:
		case 97:
		case 101:
		case 102:
		case 106:
		case 107:
			Ycc420All == 1 ? edidExt->edid_mSvd[edid_cnt].mYcc420 = Ycc420All : 0;
			LimitedToYcc420All == 1 ? edidExt->edid_mSvd[edid_cnt].mLimitedToYcc420 = LimitedToYcc420All : 0;
			break;
		}
	}
}
