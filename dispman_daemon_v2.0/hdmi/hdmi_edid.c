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
#include <stdio.h>
#include <stdlib.h>
#include <utils/Log.h> 
#include <utils/types.h> 
#include <utils/bit_operation.h>
#include <hdmi_v2_0/include/hdmi_ioctls.h>
#include <hdmi/hdmi_drv.h>
#include <hdmi/hdmi_edid.h>
#include <video/tcc/tccfb_ioctrl.h>
#include <libedid/libedid.h>
#include <hdmi/hdmi_v2.h>


#define LOG_TAG "[HDMI_EDID ]"


#define LIBDDC_DEBUG 0
#if LIBDDC_DEBUG
#define DPRINTF(args...)    ALOGD(args)
#else
#define DPRINTF(args...)
#endif

struct edid_machine_table {
        char manufacturer_name[4];
        unsigned int Product_id;
        int edid_machine_id;
};

struct edid_machine_table edid_machine_table[] = {
        {"TSB", 0x0210, 0},
        {"LGD", 0xFFFF, 0}               
};

extern tcc_hdmi_resolution tcc_support_hdmi[];

//#define EDID_DUMP_FILE "/nand1/edid.hex"
void edid_dump(__attribute__((unused)) u8* edid, __attribute__((unused)) int offset)
{
        #if defined(USE_EDID_DUMP)
        int i;
        for(i=0;i<EDID_LENGTH;i++) {
                if(!(i%16))
                        DPRINTF("\r\n[0x%03x] ", i + (offset << 8));
                DPRINTF("0x%02x ", edid[i] & 0xFF);
        }
        #endif
}

//#define USE_DUMMY_EDID
unsigned char dummy_edid[] = {
};

void edid_read_cap(sink_edid_t * sink)
{

	int edid_ok = 0;
	int edid_tries = 3;
        int use_dummy_edid = 0;
        u8 *edid_ext = NULL;
        struct edid  *edid = NULL;

        #if defined(USE_DUMMY_EDID)
        if(sizeof(dummy_edid) >= EDID_LENGTH) {
                use_dummy_edid = 1;
        }
        #endif
        
	// Data allocation
	edid = (struct edid *)malloc(sizeof(struct edid));
        if(edid == NULL) {
                ALOGE("[%s] Failed malloc, may be out of memory.", __func__);
                goto end_process;
        }
	memset(edid, 0, sizeof(struct edid));
	edid_ext = (u8 *) malloc(EDID_LENGTH);

        if(edid_ext == NULL) {
                ALOGE("[%s] Failed malloc, may be out of memory.", __func__);
                goto end_process;
        }
	memset(edid_ext, 0, sizeof(EDID_LENGTH));
	memset(sink, 0, sizeof(sink_edid_t));

	edid_parser_CeaExtReset(sink);

	do{
                if(use_dummy_edid) {
                        memcpy((void*)edid, dummy_edid, EDID_LENGTH);
                } else {
		        edid_ok = HDMI_edid_read(edid);
                }

                if(edid_ok == 0) {
        		if(edid_parser((u8 *) edid, sink, EDID_LENGTH) == FALSE){
        			ALOGE( "[%s] Could not parse EDID", __func__);
        			sink->edid_done = 0;
        			goto end_process;
        		}
                        break;
		}
	}while(edid_tries--);


	if(edid_tries <= 0){
		ALOGE( "[%s] Could not read EDID", __func__);
		sink->edid_done = 0;
		goto end_process;
	}
        
	edid_dump((u8*)edid, 0);
        
	if(edid->extensions == 0){
		sink->edid_done = 1;
	}
	else {
		int edid_ext_cnt = 1;
		while(edid_ext_cnt <= edid->extensions){
			DPRINTF("[%s] Process EDID Extension %d", __func__, edid_ext_cnt);
			edid_tries = 3;
			do{
                                memset(edid_ext, 0, sizeof(EDID_LENGTH));
                                if(use_dummy_edid) {
                                        memcpy((void*)edid_ext, &dummy_edid[(edid_ext_cnt << 7)], EDID_LENGTH);
                                } else {
				        edid_ok = HDMI_edid_extension_read( edid_ext_cnt, edid_ext);
                                }

				if(edid_ok == 0){
                                        edid_dump((u8*)edid_ext, edid_ext_cnt);

        				if(edid_parser( edid_ext, sink, EDID_LENGTH) == FALSE){
        					ALOGE( "[%s] Could not parse EDID EXTENSIONS", __func__);
        				}
                                        if(edid_ext_cnt == edid->extensions) {
        				        ALOGE( "[%s] Finish parse EDID EXTENSIONS", __func__);
                                                sink->edid_done = 1;
                                        }
                                        break;
                                }
			}while(edid_tries--);
			edid_ext_cnt++;
		}
	}

end_process:
        if(edid != NULL) {
                free(edid);
                edid = NULL;
        }
        if(edid_ext != NULL) {
                free(edid_ext);
                edid_ext = NULL;
        }
        
}


/**
 * @short Check if the input resolution is supported in detailed timings. 
 * @param[in] sink
 * @param[in] hdmi_resolution Resolution information to compare with detailed timing
 * @return Return true if the input resolution is supported.
 */
static int edid_support_detailed_timing(sink_edid_t * sink, void * hdmi_resolution)
{
        int ret = 0;
        int dtd_loop;
        tcc_hdmi_resolution *tcc_resolution = hdmi_resolution;
        for(dtd_loop=0;dtd_loop<10;dtd_loop++) {
                DPRINTF("[%s] %d == %d \r\n", __func__, sink->parse_detailed_timing_dtd[dtd_loop].mPixelClock, (unsigned int)tcc_resolution->pixel_clock);
                if(  
                     sink->parse_detailed_timing_dtd[dtd_loop].mPixelClock == (unsigned int)tcc_resolution->pixel_clock && 
                     sink->parse_detailed_timing_dtd[dtd_loop].mInterlaced == (unsigned int)tcc_resolution->interlaced &&
                     sink->parse_detailed_timing_dtd[dtd_loop].mHActive == (unsigned int)tcc_resolution->HdmiSize.width &&
                     sink->parse_detailed_timing_dtd[dtd_loop].mHBlanking == (unsigned int)tcc_resolution->hblank &&
                     sink->parse_detailed_timing_dtd[dtd_loop].mVActive == (unsigned int)(tcc_resolution->interlaced?(tcc_resolution->HdmiSize.height>>1):tcc_resolution->HdmiSize.height) &&
                     sink->parse_detailed_timing_dtd[dtd_loop].mHBlanking == (unsigned int)tcc_resolution->vblank) {
                        if(tcc_resolution->vic[HDMI_RATIO_4_3]) {
                                ret |= EDID_SUPPORT_4_3;
                        }if(tcc_resolution->vic[HDMI_RATIO_16_9]) {
                                ret |= EDID_SUPPORT_16_9;
                        }
                        break; 
                }
        }
        return ret;
}

/**
 * @short Check if the input resolution is supported in standard timings. 
 * @param[in] sink
 * @param[in] hdmi_resolution Resolution information to compare with detailed timing
 * @return Return true if the input resolution is supported.
 */
static int edid_support_standard_timing(sink_edid_t * sink, void * hdmi_resolution)
{
        int i, ret = 0;
        struct parse_standard_timing *parse_standard_timing;
        tcc_hdmi_resolution *tcc_resolution = hdmi_resolution;

        //check standard timing
        for (i=0; i < 8; i++) {
                parse_standard_timing = &(sink->parse_standard_timing[i]);

                if( !tcc_resolution->interlaced  && 
                    tcc_resolution->HdmiSize.width == (int)parse_standard_timing->hactive &&
                    tcc_resolution->HdmiSize.height== (int)parse_standard_timing->vactive &&
                    tcc_resolution->HdmiSize.frame_hz == (int)parse_standard_timing->frame_hz) {

                        if(parse_standard_timing->aspect == HDMI_RATIO_4_3 && tcc_resolution->vic[HDMI_RATIO_4_3]) {
                                ret = EDID_SUPPORT_4_3;
                        }
                        else if(parse_standard_timing->aspect == HDMI_RATIO_16_9 && tcc_resolution->vic[HDMI_RATIO_16_9]) {
                                ret = EDID_SUPPORT_16_9;  
                        }
                        break;
                }
        }
        return ret;
}

/**
 * @short Check if the input resolution is supported in hdmi mode
 * @param[in] sink
 * @param[in] video_idx Corresponding index of tcc_support_hdmi
 * @param[in] aspect_ratio Corresponding aspectratio of tcc_support_hdmi
 * @return Return true if the input resolution is supported.
 */
static int edid_check_resolution_is_supported(sink_edid_t * sink, int video_idx, int hdmi)
{
        int i, ret = 0;
        DPRINTF("[%s] video_idx[%d]", __func__, video_idx);
        if(video_idx >= vMaxTableItems) {
                ALOGE("%s out of video index(%d)", __func__, video_idx);
                goto end_process;   
        }

        /* The 640x480@60Hz flag, in the Established Timings area, shall always 
        be set, since the 640x480p format is a mandatory default timing */
        if(video_idx == v640x480p_60Hz && sink->edid_mSupport_est640x480p60) {
                DPRINTF("[%s] video_idx[%d] is supported by v640x480p_60Hz", __func__, video_idx);
                ret = EDID_SUPPORT_4_3;
                goto end_process;
        }

        /* DVI support 2K only */
        if(!hdmi && tcc_support_hdmi[video_idx].not_support_dvi) {
                DPRINTF("[%s] dvi cann't support video_idx[%d]", __func__, video_idx);
                goto end_process;
        }

        /* Check if the input resolution is supported by short video descriptor */        
        for(i = 0; (i < (int)sink->edid_mSvdIndex) && sink->edid_mSvd[i].mCode; i++){
                if(sink->edid_mSvd[i].mCode == (unsigned int)tcc_support_hdmi[video_idx].vic[HDMI_RATIO_4_3]) {
                        ret |= EDID_SUPPORT_4_3;
                }
                if(sink->edid_mSvd[i].mCode == (unsigned int)tcc_support_hdmi[video_idx].vic[HDMI_RATIO_16_9]) {
                        ret |= EDID_SUPPORT_16_9;
                }
        }
        
        if(ret & EDID_SUPPORT) {
                DPRINTF("[%s] video_idx[%d] match aspect-ratio of svd are %d", __func__, video_idx, ret);
                goto end_process;
        }


        /* Check if the input resolution is supported by established timing
           Even though Short Video Descriptors are available in the Version 3 
           CEA Extension, there is still a need to use Detailed Timing Descriptors 
           if full backward compatibility with legacy Sources is desired */
        ret = edid_support_detailed_timing(sink, &tcc_support_hdmi[video_idx]);
        if(ret & EDID_SUPPORT) {
                DPRINTF("[%s] video_idx[%d] is supported by established timing.", __func__, video_idx);
                goto end_process;
        }

        /* Check if the input resolution is supported by stanard timing */
        ret = edid_support_standard_timing(sink, &tcc_support_hdmi[video_idx]);
        if(ret & EDID_SUPPORT) {
                DPRINTF("[%s] video_idx[%d] is supported by standard timing.", __func__, video_idx);
                goto end_process;
        }
       
end_process:
        if(!(ret & EDID_SUPPORT
)) {
                DPRINTF("[%s] video_idx[%d] is not supported by sink.", __func__, video_idx);
        }
        return ret;

}


encoding_t edid_check_colorspace(sink_edid_t * sink, int hdmi_mode_idx)
// CheckColorSpaceWithEDID
{
        encoding_t encoding = RGB;

        if(!sink->edid_done) {
                ALOGE("[%s] failed read edid.", __func__);
                goto end_process;
        }

        if(hdmi_mode_idx < 0 || hdmi_mode_idx >= vMaxTableItems) {
                ALOGE("[%s] out of range", __func__);
                goto end_process;
        }
                
        /* color encoding configuration */
        if((sink->edid_m20Sink == 1) && (tcc_support_hdmi[hdmi_mode_idx].edid_support_status & EDID_SUPPORT_YCC420_ONLY)) {
                /* if current Svd is Limited to YCC420 (EDID) */
                encoding = YCC420;
                DPRINTF("[%s] Setting encoding to Ycc420 because sink support Ycc420 only", __func__);
        }else{
                if((sink->edid_m20Sink == 1) && (tcc_support_hdmi[hdmi_mode_idx].edid_support_status & EDID_SUPPORT_YCC420)){
                        DPRINTF("[%s] Setting encoding to Ycc420", __func__);
                        encoding = YCC420;
                }
                else {
                        //encoding = RGB;
                        if(sink->edid_mYcc444Support == 1){
                                DPRINTF("[%s] edid_check_colorspace: YCC444.", __func__);
                                encoding = YCC444;
                        }
                        else if(sink->edid_mYcc422Support == 1){
                                DPRINTF("[%s] edid_check_colorspace: YCC422.", __func__);
                                encoding = YCC422;
                        }
                        else{
                                DPRINTF("[%s] edid_check_colorspace: RGB.", __func__);
                                encoding = RGB;
                        }
                }
        }
end_process:
        return encoding;
}

int edid_check_support_only_ycc420(sink_edid_t * sink, int hdmi_mode_idx)
{
        int ret = 0;

        if(!sink->edid_done) {
                ALOGE("[%s] failed read edid.", __func__);
                goto end_process;
        }
        
        /* color encoding configuration */
        if((sink->edid_m20Sink == 1) && (tcc_support_hdmi[hdmi_mode_idx].edid_support_status & EDID_SUPPORT_YCC420_ONLY)) {
                /* if current Svd is Limited to YCC420 (EDID) */
                ret = 1;
                DPRINTF("[%s] edid_get_support_only_ycc420 true.", __func__);
        }
        
end_process:
        return ret;
}

int edid_check_support_ycc420(sink_edid_t * sink, int hdmi_mode_idx)
{
        int ret = 0;

        if(!sink->edid_done) {
                ALOGE("[%s] failed read edid ", __func__);
                goto end_process;
        }
                
        /* color encoding configuration */
        if((sink->edid_m20Sink == 1) && (tcc_support_hdmi[hdmi_mode_idx].edid_support_status & EDID_SUPPORT_YCC420)) {
                /* if current Svd supports to YCC420 (EDID) */
                ret = 1;
                DPRINTF("[%s] edid_get_support_only_ycc420 true", __func__);
        }

end_process:
        return ret;
}


/**
 * @short Update resolution information supported by edid information
 * @param[in] sink
 * @param[in] videoParams
 * @return Return 0 
 */
int edid_update_support_hdmi_table(sink_edid_t * sink, videoParams_t *videoParams, hdmi_soc_features *soc_feature)
{
	int table_index;
        unsigned int svd_index;
	unsigned short support_3d_struct;

        if(sink == NULL || videoParams == NULL || soc_feature == NULL) {
                ALOGE("%s parameter is NULL, sink(%p), videoParams(%p), soc_feature(%p)\r\n", __func__, sink, videoParams, soc_feature);
                return -1;
        }
        
        /* Test for a list of all supported resolutions */
	for(table_index = 0; table_index < vMaxTableItems; table_index++)
	{
                tcc_support_hdmi[table_index].edid_support_status = edid_check_resolution_is_supported(sink, table_index, videoParams->mHdmi);

                
                /* Soc supports tmds character ratio below 340 */
                if(soc_feature->max_tmds_mhz > 0 && soc_feature->max_tmds_mhz <= 340) {
                        if(tcc_support_hdmi[table_index].pixel_clock > 340000000) {
                                tcc_support_hdmi[table_index].edid_support_status = 0;        
                        }
                }

                if(!(tcc_support_hdmi[table_index].edid_support_status & EDID_SUPPORT)) {
                        continue;
                }
                
                DPRINTF("[%s] table[%d] is supported", __func__, table_index);


                /* Identify the index that matches the short video descriptor */
                for(svd_index = 0; (svd_index < sink->edid_mSvdIndex) && sink->edid_mSvd[svd_index].mCode; svd_index++){
                        if(sink->edid_mSvd[svd_index].mCode == (unsigned int)tcc_support_hdmi[table_index].vic[HDMI_RATIO_4_3]  || 
                           sink->edid_mSvd[svd_index].mCode == (unsigned int)tcc_support_hdmi[table_index].vic[HDMI_RATIO_16_9] ) {
                           break;
                        }
                }

                /* Found an index that matches a short video descriptor */
                if(svd_index < sink->edid_mSvdIndex) {                
                        /* NATIVE Mode */
                        if(sink->edid_mSvd[svd_index].mNative) {
                                tcc_support_hdmi[table_index].edid_support_status |= EDID_NATIVE_VIC ;
                        }

                        /* HDMI VIC does not support HDMI 3D at the same time HDMI VIC does not support YCC420 */
                        if(!sink->edid_mSvd[svd_index].mHdmiVic) {
                                /* Updae supported hdmi 3d information */
                                if(svd_index < MAX_VIC_WITH_3D && sink->edid_mHdmivsdb.m3dPresent) {
                                        support_3d_struct = get_index_supported_3dstructs(&sink->edid_mHdmivsdb, svd_index);
                                        if(support_3d_struct & 1) {
                                                tcc_support_hdmi[table_index].edid_support_status |= EDID_SUPPORT_3D_FB;
                                        }
                                        if(support_3d_struct & (1 << 6)) {
                                                tcc_support_hdmi[table_index].edid_support_status |= EDID_SUPPORT_3D_TAB;
                                        }
                                        if(support_3d_struct & (1 << 8)) {
                                                tcc_support_hdmi[table_index].edid_support_status |= EDID_SUPPORT_3D_SBS;
                                        }
                                }
                        }
                }

                if(sink->edid_mSvd[svd_index].mHdmiVic) {
                        tcc_support_hdmi[table_index].edid_support_status |= EDID_HDMI_VIC;
                } else {
                        /* HDMI VIC does not support YCC420 at same time */
                
                        /* Soc supports YCbCr420 colorspace */
                        if(soc_feature->support_feature_1 & (1 <<0 )) {
                                switch(tcc_support_hdmi[table_index].vic[HDMI_RATIO_16_9]) {
                                        case 96:
                                        case 97:
                                        case 101:
                                        case 102:
                                        case 106:
                                        case 107:
                                                if(sink->edid_mSvd[svd_index].mYcc420) {
                                                        tcc_support_hdmi[table_index].edid_support_status |= EDID_SUPPORT_YCC420;
                                                }
                                                if(sink->edid_mSvd[svd_index].mLimitedToYcc420) {
                                                        tcc_support_hdmi[table_index].edid_support_status |= EDID_SUPPORT_YCC420_ONLY;
                                                }
                                                break;                                             
                                } 
                        }
                }
	}
}



/**
 * @short Update the machine id after checking the TV model in edid 
 * @param[in] sink
 * @return Return 0 
 */
int edid_update_machin_id(sink_edid_t * sink)
{
        int loop;
        int edid_machine_table_max = sizeof(edid_machine_table) / sizeof(struct edid_machine_table);
        int edid_machine_id = 0;

        if(sink->edid_done) {
                for(loop = 0; loop < edid_machine_table_max; loop++) {
                        if(!strcmp(sink->parse_vendor_info.manufacturer_name, edid_machine_table[loop].manufacturer_name)) {  
                                if(edid_machine_table[loop].Product_id == 0xFFFF) {
                                        edid_machine_id = edid_machine_table[loop].edid_machine_id;
                                        break;
                                }else if(sink->parse_vendor_info.Product_id == edid_machine_table[loop].Product_id) {
                                        edid_machine_id = edid_machine_table[loop].edid_machine_id;
                                        break;
                                }
                        }
                }
        }

        HDMIDrv_set_edid_machine_id(edid_machine_id);
        return 0;
}



