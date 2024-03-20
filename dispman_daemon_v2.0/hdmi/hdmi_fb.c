/*!
* TCC Version 1.0
* Copyright (c) Telechips Inc.
* All rights reserved 
*  \file        hdmi_fb.h
*  \brief       HDMI frame buffer header
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
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>

#include <utils/Log.h>
#include <utils/types.h>
#include <utils/properties.h>
#include <hdmi_v2_0/include/hdmi_ioctls.h>
#include <video/tcc/tccfb_ioctrl.h>

#define FB_DEV_NAME      "/dev/fb0"
#define HDMI_APP_DEBUG  0
#define LOG_TAG         "[HDMI_FB   ]"

#if HDMI_APP_DEBUG
#define DPRINTF(args...)    ALOGI(args)
#else
#define DPRINTF(args...)
#endif

#ifndef TRUE
#define TRUE	1
#endif
#ifndef FALSE
#define FALSE	0
#endif

#ifdef CONFIG_HDMI_DOLBYVISION_SUPPORT
#include "pmap.h"
#include <sys/mman.h>
#include <errno.h>
#include <control_path_wrapper.h>

#define SZ_REG_BUFFER_MAX (600*1024)
#define SZ_MD_BUFFER_MAX	(4*1024)

typedef struct hdr_10_infoframe_s
{
    uint8_t infoframe_type_code      ;
    uint8_t infoframe_version_number ;
    uint8_t length_of_info_frame     ;
    uint8_t data_byte_1              ;
    uint8_t data_byte_2              ;
    uint8_t display_primaries_x_0_LSB;
    uint8_t display_primaries_x_0_MSB;
    uint8_t display_primaries_y_0_LSB;
    uint8_t display_primaries_y_0_MSB;
    uint8_t display_primaries_x_1_LSB;
    uint8_t display_primaries_x_1_MSB;
    uint8_t display_primaries_y_1_LSB;
    uint8_t display_primaries_y_1_MSB;
    uint8_t display_primaries_x_2_LSB;
    uint8_t display_primaries_x_2_MSB;
    uint8_t display_primaries_y_2_LSB;
    uint8_t display_primaries_y_2_MSB;
    uint8_t white_point_x_LSB        ;
    uint8_t white_point_x_MSB        ;
    uint8_t white_point_y_LSB        ;
    uint8_t white_point_y_MSB        ;
    uint8_t max_display_mastering_luminance_LSB;
    uint8_t max_display_mastering_luminance_MSB;
    uint8_t min_display_mastering_luminance_LSB;
    uint8_t min_display_mastering_luminance_MSB;
    uint8_t max_content_light_level_LSB        ;
    uint8_t max_content_light_level_MSB        ;
    uint8_t max_frame_average_light_level_LSB  ;
    uint8_t max_frame_average_light_level_MSB  ;
} hdr_10_infoframe_t;

static void DV_Set_HDR10_InfoFrame(hdr_10_infoframe_t* src, unsigned int bHDR10)
{
    #define MERGE_BIT(msb,lsb) (int)(((msb << 7) & 0xff00) | (lsb & 0xff))
	int hdmi_fd = -1;

	if(0 <= (hdmi_fd = 0; open("/dev/dw-hdmi-tx", O_RDWR))
	{
		DRM_Packet_t info;
	    memset(&info, 0x0, sizeof(DRM_Packet_t));

		if(bHDR10)
		{
			info.mInfoFrame.version 				= src->infoframe_version_number;
			info.mInfoFrame.length 					= src->length_of_info_frame;
			info.mDescriptor_type1.EOTF 			= 2;
			info.mDescriptor_type1.Descriptor_ID 	= 0;

			info.mDescriptor_type1.disp_primaries_x[0] 	= MERGE_BIT(src->display_primaries_x_0_MSB, src->display_primaries_x_0_LSB);
			info.mDescriptor_type1.disp_primaries_x[1] 	= MERGE_BIT(src->display_primaries_x_1_MSB, src->display_primaries_x_1_LSB);
			info.mDescriptor_type1.disp_primaries_x[2] 	= MERGE_BIT(src->display_primaries_x_2_MSB, src->display_primaries_x_2_LSB);
			info.mDescriptor_type1.disp_primaries_y[0] 	= MERGE_BIT(src->display_primaries_y_0_MSB, src->display_primaries_y_0_LSB);
			info.mDescriptor_type1.disp_primaries_y[1] 	= MERGE_BIT(src->display_primaries_y_1_MSB, src->display_primaries_y_1_LSB);
			info.mDescriptor_type1.disp_primaries_y[2] 	= MERGE_BIT(src->display_primaries_y_2_MSB, src->display_primaries_y_2_LSB);
			info.mDescriptor_type1.white_point_x           		= MERGE_BIT(src->white_point_x_MSB, src->white_point_x_LSB);
			info.mDescriptor_type1.white_point_y          		= MERGE_BIT(src->white_point_y_MSB, src->white_point_y_LSB);
			info.mDescriptor_type1.max_disp_mastering_luminance =  MERGE_BIT(src->max_display_mastering_luminance_MSB, src->max_display_mastering_luminance_LSB);
			info.mDescriptor_type1.min_disp_mastering_luminance =  MERGE_BIT(src->min_display_mastering_luminance_MSB, src->min_display_mastering_luminance_LSB);

			info.mDescriptor_type1.max_content_light_level      = MERGE_BIT(src->max_content_light_level_MSB, src->max_content_light_level_LSB);
			info.mDescriptor_type1.max_frame_avr_light_level    = MERGE_BIT(src->max_frame_average_light_level_MSB, src->max_frame_average_light_level_LSB);
		}

		if (0 < ioctl(hdmi_fd, HDMI_API_DRM_CONFIG, &info)){
			ALOGE("DV_Set_HDR10_InfoFrame :: HDR info send error!!(%s)", strerror(errno));
		}
		close(hdmi_fd);
	}

	#undef MERGE_BIT
}

static int DV_Get_Regs(unsigned int *pReg_PhyAddr, unsigned int *pMD_PhyAddr, unsigned int tv_type, unsigned int width, unsigned int height)
{
	int ret = 0;
	int mTmem_fd = -1;
	pmap_t mDV_pmap;
	unsigned char *pReg_mem[2], *pMD_mem[2];
	unsigned int szReg_mem, szMD_mem;
	unsigned int out_format = 0;
	
	hdr_10_infoframe_t pHDR10_InfoFrame;
	unsigned int szHDR10_InfoFrame;
	
	if( tv_type == 2 )
		out_format = DOVI;
	else
		out_format = (tv_type == 1) ? HDR10 : SDR;

	mTmem_fd = open("/dev/tmem", O_RDWR);
	if (mTmem_fd < 0) {
		ALOGE("can't open[%s] '%s'", strerror(errno), "/dev/tmem");
		return -1;
	}

	szReg_mem = SZ_REG_BUFFER_MAX;
	szMD_mem = SZ_MD_BUFFER_MAX;
	pmap_get_info("dolby_regs", &mDV_pmap);

	pReg_mem[0] = (unsigned char *)(mDV_pmap.base + mDV_pmap.size - szReg_mem - szMD_mem);
	pReg_mem[1] = (unsigned char *)mmap(NULL, szReg_mem, PROT_READ | PROT_WRITE, MAP_SHARED, mTmem_fd, (unsigned int)pReg_mem[0]);
	if(pReg_mem[1] == MAP_FAILED)
		goto Error;

	pMD_mem[0] = (unsigned char *)(mDV_pmap.base + mDV_pmap.size - szMD_mem);
	pMD_mem[1] = (unsigned char *)mmap(NULL, szMD_mem, PROT_READ | PROT_WRITE, MAP_SHARED, mTmem_fd, (unsigned int)pMD_mem[0]);

	if(pMD_mem[1] == MAP_FAILED){
		munmap((void*)pReg_mem[1], SZ_REG_BUFFER_MAX);		
		goto Error;
	}

	{
		if( 0 > (ret = DV_CP_Iint(	SDR,			// in_format :: FORMAT_DOVI = 0, FORMAT_HDR10 = 1, FORMAT_SDR = 2
									out_format,		// out_format :: FORMAT_DOVI = 0, FORMAT_HDR10 = 1, FORMAT_SDR = 2
									BPP_8, 			// src_bpp :: 8 or 10
									YUV420,			// src_chroma_format :: 0=420, 1=422
									FULL_RANGE, 	// src_yuv_range :: SIGNAL_RANGE_NARROW = 0, SIGNAL_RANGE_FULL = 1, SIGNAL_RANGE_SDI = 2 
									width, 			// width
									height, 		// height
									OSD_OFF,		// graphics_top
									OSD_ON,			// graphica_bottom
									NULL
				)
			)
		){
			ALOGE("@@@@@@@@@@ DV_CP_Iint Error(%d)", ret);
		}

		if( 0 > (ret = DV_CP_Commit( NULL,
									NULL,
									pReg_mem[1],
									&szReg_mem,
									(unsigned char*)(pMD_mem[1]),
									&szMD_mem,
									(unsigned char*)&pHDR10_InfoFrame,
									&szHDR10_InfoFrame))
		){
			ALOGE("@@@@@@@@@@ DV_CP_Commit Error(%d) ", ret);
		}

		if( 0 > (ret = DV_CP_Destory())){
			ALOGE("@@@@@@@@@@ DV_CP_Destory Error(%d)", ret);
		}

		*pReg_PhyAddr 	= (unsigned int)(pReg_mem[0]);
		if( szReg_mem )
			*pMD_PhyAddr	= (unsigned int)(pMD_mem[0]);
		else
			*pMD_PhyAddr	= 0x00;

		//ALOGI("DV_CP_Commit :: SDR to %d, %d x %d, Reg(0x%x-%d)/Meta(0x%x-%d) \n", out_format, width, height, *pReg_PhyAddr, szReg_mem, *pMD_PhyAddr, szMD_mem);

		if(0)
		{
			unsigned int *p = (unsigned int *)pReg_mem[1];
			ALOGE("0x12500000 Regs Start ::\n"
					"0x%8x 0x%8x 0x%8x 0x%8x \n"
					"0x%8x 0x%8x 0x%8x 0x%8x \n"
					"0x%8x 0x%8x 0x%8x 0x%8x \n",
							p[0], p[1], p[2], p[3],
							p[4], p[5], p[6], p[7],
							p[8], p[9], p[10], p[11]);

			p += (0x3C000/4);
			ALOGE("0x1253C000 ::\n"
					"0x%8x 0x%8x 0x%8x 0x%8x \n"
					"0x%8x 0x%8x 0x%8x 0x%8x \n"
					"0x%8x 0x%8x 0x%8x 0x%8x \n",
							p[0], p[1], p[2], p[3],
							p[4], p[5], p[6], p[7],
							p[8], p[9], p[10], p[11]);

			p += (0x4000/4);
			ALOGE("0x12540000 ::\n"
					"0x%8x 0x%8x 0x%8x 0x%8x \n"
					"0x%8x 0x%8x 0x%8x 0x%8x \n"
					"0x%8x 0x%8x 0x%8x 0x%8x \n",
							p[0], p[1], p[2], p[3],
							p[4], p[5], p[6], p[7],
							p[8], p[9], p[10], p[11]);	

			ALOGI("ZzaU :: Resolution %d x %d", width, height);

			ALOGI("ZzaU :: pmem(0x%x-0x%x): DV Reg(0x%x / 0x%x), Md(0x%x / 0x%x)", mDV_pmap.base, mDV_pmap.size, pReg_mem[0], pReg_mem[1], pMD_mem[0], pMD_mem[1]);
			ALOGI("ZzaU :: DV 0x%x - 0x%x / 0x%x - 0x%x", *pReg_PhyAddr, szReg_mem, *pMD_PhyAddr, szMD_mem);

			p = (unsigned int *)pMD_mem[1];

			ALOGE("metadata(%d/0x%x) Start ::\n"
					"0x%8x 0x%8x 0x%8x 0x%8x \n"
					"0x%8x 0x%8x 0x%8x 0x%8x \n"
					"0x%8x 0x%8x 0x%8x 0x%8x \n", szMD_mem, szMD_mem,
							p[0], p[1], p[2], p[3],
							p[4], p[5], p[6], p[7],
							p[8], p[9], p[10], p[11]);

			ALOGE("metadata End ::\n"
					"0x%8x 0x%8x 0x%8x 0x%8x \n"
					"0x%8x 0x%8x 0x%8x 0x%8x \n"
					"0x%8x 0x%8x 0x%8x 0x%8x \n",
							p[szMD_mem-12], p[szMD_mem-11], p[szMD_mem-10], p[szMD_mem-9],
							p[szMD_mem-8], p[szMD_mem-7], p[szMD_mem-6], p[szMD_mem-5],
							p[szMD_mem-4], p[szMD_mem-3], p[szMD_mem-2], p[szMD_mem-1]);

		}

		DV_Set_HDR10_InfoFrame((hdr_10_infoframe_t*)&pHDR10_InfoFrame, (out_format == HDR10) ? 1 : 0);
		munmap((void*)pReg_mem[1], SZ_REG_BUFFER_MAX);
		munmap((void*)pMD_mem[1], SZ_MD_BUFFER_MAX);
	}

	close(mTmem_fd);

	return ret;

Error:
	close(mTmem_fd);

	ALOGE("@@@@@@@@@@ mmap for DV has Error:: 0x%x - 0x%x", pReg_mem[0], szReg_mem);
	*pReg_PhyAddr 	= 0x00;
	*pMD_PhyAddr	= 0x00;
	ret = -1;

	return ret;
}
#endif

static int fb_fd = -1;


int FB_Open(void)
{
        ALOGI("FB_Open\r\n");
        if (fb_fd == -1 && (fb_fd = open(FB_DEV_NAME, O_RDWR)) < 0)
        {
                fb_fd = -1;
                ALOGE("can not open \"%s\"\n", FB_DEV_NAME);
        }
        return fb_fd;
}

int FB_Close(void)
{
        if (fb_fd != -1)
        {
                if (close(fb_fd) < 0)
                {
                        ALOGE("can not close \"%s\"\n", FB_DEV_NAME);
                }
                fb_fd = -1;
        }
        return 1;
}

int FB_GetHandle(void)
{
        if(fb_fd == -1) {
                FB_Open();
        }
        return fb_fd;
}

unsigned int FB_LcdcCheck(void)
{
        int hdmi_check = 0;

        if (ioctl(fb_fd, TCC_LCDC_HDMI_CHECK, &hdmi_check) < 0 ) {
                DPRINTF("%s TCC_LCDC_HDMI_CHECK failed!\n",__func__);
                return 0;
        }

        return hdmi_check;
}

int FB_video_generator_prepare(void) {
        int ret;

        // check file
        if (fb_fd < 0)    {
                ALOGE("HDMI device File is not available\r\n");
                return 0;
        }
        
        ret = ioctl(fb_fd, TCC_LCDC_HDMI_START, NULL);
        if (ret) {
                ALOGE( "Failed TCC_LCDC_HDMI_START IOCTL [%d]\n", ret);
        }
        return ret;
}

static unsigned int FB_video_get_vactive(videoParams_t *video)
{
        unsigned int vactive = 0;

        if(video != NULL) {
                vactive = video->mDtd.mVActive;
                if(video->mHdmiVideoFormat == 2 && video->m3dStructure == 0) {
                        /* Check HDMI 3D-Frame Packing */
                        if(video->mDtd.mInterlaced) {
                                vactive = (vactive << 2) + (3*video->mDtd.mVBlanking+2);
                        } else {
                                vactive = (vactive << 1) + video->mDtd.mVBlanking;
                        }
                } else if(video->mDtd.mInterlaced) {
                        /* Check HDMI Interlaced Mode */
                        vactive <<= 1;
                }
        }

        //printf("%s active=%d\r\n", __func__, vactive);
        return vactive;
}


int FB_video_generator_config(videoParams_t *videoParam) {

	int vactive, ret = -1;
        struct lcdc_timimg_parms_t lcdc_timimg_parms;
	                
        // check file
        if (fb_fd < 0)    {
                ALOGE( "HDMI device File is not available\r\n");
                goto end_process;
        }

        memset(&lcdc_timimg_parms, 0, sizeof(lcdc_timimg_parms));
        lcdc_timimg_parms.iv = videoParam->mDtd.mVSyncPolarity?0:1;  /** 0 for Active low, 1 active high */
        lcdc_timimg_parms.ih = videoParam->mDtd.mHSyncPolarity?0:1;  /** 0 for Active low, 1 active high */
        lcdc_timimg_parms.dp = videoParam->mDtd.mPixelRepetitionInput;        
        
        if(videoParam->mDtd.mInterlaced)
                lcdc_timimg_parms.tv = 1;
        else    
                lcdc_timimg_parms.ni = 1;      
        lcdc_timimg_parms.tft = 1;
        
        lcdc_timimg_parms.lpw = videoParam->mDtd.mHSyncPulseWidth-1;
        lcdc_timimg_parms.lpc = videoParam->mDtd.mHActive-1;
        lcdc_timimg_parms.lswc = (videoParam->mDtd.mHBlanking - (videoParam->mDtd.mHSyncOffset+videoParam->mDtd.mHSyncPulseWidth))-1;
        lcdc_timimg_parms.lewc = videoParam->mDtd.mHSyncOffset-1;

        vactive = FB_video_get_vactive(videoParam);
        
        // Check 3D Frame Packing
        if(videoParam->mHdmiVideoFormat == 2 && videoParam->m3dStructure == 0) {
                lcdc_timimg_parms.framepacking = 1;
        }
        else if(videoParam->mHdmiVideoFormat == 2 && (videoParam->m3dStructure == 6 || videoParam->m3dStructure == 8)) {
                // SBS is 2, TAB is 3
                lcdc_timimg_parms.framepacking = (videoParam->m3dStructure == 8)?2:3;
        }else {
                lcdc_timimg_parms.framepacking = 0;
        }
        ALOGI("video_generator_config VIC[%d]%dx%d\r\n", videoParam->mDtd.mCode, videoParam->mDtd.mHActive, videoParam->mDtd.mVActive);
        
        if(videoParam->mDtd.mInterlaced){
                lcdc_timimg_parms.fpw = (videoParam->mDtd.mVSyncPulseWidth << 1)-1;    
                lcdc_timimg_parms.fswc = ((videoParam->mDtd.mVBlanking - (videoParam->mDtd.mVSyncOffset + videoParam->mDtd.mVSyncPulseWidth)) << 1)-1;
                lcdc_timimg_parms.fewc = (videoParam->mDtd.mVSyncOffset << 1);
                lcdc_timimg_parms.fswc2 = lcdc_timimg_parms.fswc+1;
                lcdc_timimg_parms.fewc2 = lcdc_timimg_parms.fewc-1;
        }
        else {
                lcdc_timimg_parms.fpw = videoParam->mDtd.mVSyncPulseWidth-1;     
                lcdc_timimg_parms.fswc = (videoParam->mDtd.mVBlanking - (videoParam->mDtd.mVSyncOffset + videoParam->mDtd.mVSyncPulseWidth))-1;
                lcdc_timimg_parms.fewc = videoParam->mDtd.mVSyncOffset-1;
                lcdc_timimg_parms.fswc2 = lcdc_timimg_parms.fswc;
                lcdc_timimg_parms.fewc2 = lcdc_timimg_parms.fewc;
        } 

        /* Common Timing Parameters */
        lcdc_timimg_parms.flc = vactive-1;
        lcdc_timimg_parms.fpw2 = lcdc_timimg_parms.fpw;
        lcdc_timimg_parms.flc2 = lcdc_timimg_parms.flc;
        

	#if defined(CONFIG_HDMI_DOLBYVISION_SUPPORT)
        if((videoParam->mDolbyVision & 0x7) > 0) {
			unsigned int mReg_PhyAddr, mMD_PhyAddr;

			lcdc_timimg_parms.format = ((videoParam->mDolbyVision >> 3) & 0x7); 

			if(lcdc_timimg_parms.dp)
				DV_Get_Regs(&mReg_PhyAddr, &mMD_PhyAddr, lcdc_timimg_parms.format, (lcdc_timimg_parms.lpc + 1)/2, lcdc_timimg_parms.flc + 1);
			else
				DV_Get_Regs(&mReg_PhyAddr, &mMD_PhyAddr, lcdc_timimg_parms.format, (lcdc_timimg_parms.lpc + 1), lcdc_timimg_parms.flc + 1);

			lcdc_timimg_parms.dv_reg_phyaddr = mReg_PhyAddr;
			lcdc_timimg_parms.dv_md_phyaddr = mMD_PhyAddr;
		}
		else
		{
			lcdc_timimg_parms.dv_reg_phyaddr = 0x00;
			lcdc_timimg_parms.dv_md_phyaddr = 0x00;
		}
	#endif
        
        // set video parameters
        if(fb_fd) {
                ret = ioctl(fb_fd, TCC_LCDC_HDMI_TIMING, &lcdc_timimg_parms);
                if (ret) {
                        ALOGE("Failed TCC_LCDC_HDMI_TIMING IOCTL [%d]\n", ret);
                }        
        }
end_process:
	return ret;
}


int FB_video_generator_extra(videoParams_t *videoParam) {
        int ret;
        struct tcc_fb_extra_data tcc_fb_extra_data;

        memset(&tcc_fb_extra_data, 0, sizeof(tcc_fb_extra_data));

        // check file
        if (fb_fd < 0)    {
                ALOGE("HDMI device File is not available\r\n");
                return 0;
        }

        switch(videoParam->mEncodingOut)
        {
                case YCC444:
                        // YCC444
                        if(videoParam->mColorResolution > COLOR_DEPTH_8) {
                                tcc_fb_extra_data.pxdw = 23;
                        }
                        else {
                                tcc_fb_extra_data.pxdw = 12;
                        }
                        tcc_fb_extra_data.swapbf = 2;
                        break;
                case YCC422:
                        // YCC422
                        tcc_fb_extra_data.pxdw = 21;
                        tcc_fb_extra_data.swapbf = 5;
                        break;
                case YCC420:
                        // YCC420
                        if(videoParam->mColorResolution > COLOR_DEPTH_8) {
                                tcc_fb_extra_data.pxdw = 27;
                        }
                        else {
                                tcc_fb_extra_data.pxdw = 26;
                        }        
                        tcc_fb_extra_data.swapbf = 0;
                        break;
                case RGB:
                default:
                        if(videoParam->mColorResolution > COLOR_DEPTH_8) {
                                tcc_fb_extra_data.pxdw = 23; // RGB101010 Format
                        }
                        else {
                                tcc_fb_extra_data.pxdw = 12;
                        }
                        tcc_fb_extra_data.swapbf = 0;
                        break;
        }
        
        // R2Y
        if(videoParam->mEncodingOut != RGB) {
                tcc_fb_extra_data.r2y = 1;
                switch(videoParam->mColorimetry) 
                {
                                case ITU709:
                                        tcc_fb_extra_data.r2ymd = 3;//2;
                                        break;
                                case EXTENDED_COLORIMETRY:
                                        switch(videoParam->mExtColorimetry) {
                                                case XV_YCC601:
                                                case S_YCC601:
                                                case ADOBE_YCC601:
                                                default:
                                                        tcc_fb_extra_data.r2ymd = 1;//0;
                                                        break;
                                                case XV_YCC709:
                                                        tcc_fb_extra_data.r2ymd = 3;//2;
                                                        break;
                                                case BT2020YCCBCR:
                                                case BT2020YCBCR:
                                                        tcc_fb_extra_data.r2ymd = 5;//4;
                                                        break;
                                        }
                                        break;
                                case ITU601:
                                default:
                                        tcc_fb_extra_data.r2ymd = 1;//0;
                                        break;
                }
        }
        else {
                tcc_fb_extra_data.r2y = 0;
        }

        ret = ioctl(fb_fd, TCC_LCDC_HDMI_EXTRA_SET, &tcc_fb_extra_data);
        if (ret) {
                ALOGE( "Failed TCC_LCDC_HDMI_EXTRA_SET IOCTL [%d]\n", ret);
        }
        return ret;
}


int FB_video_generator_stop(void) {
        int ret;

        // check file
        if (fb_fd < 0)    {
                ALOGE("HDMI device File is not available\r\n");
                return 0;
        }
        
        ret = ioctl(fb_fd, TCC_LCDC_HDMI_END, NULL);
        if (ret) {
                ALOGE( "Failed TCC_LCDC_HDMI_END IOCTL [%d]\n", ret);
        }
        return ret;
}


int FB_video_set_r2ymd(unsigned int r2ymd) {      
        int ret;

        // check file
        if (fb_fd < 0)    {
                ALOGE("HDMI device File is not available\r\n");
                return 0;
        }

        r2ymd |= (TCC_LCDC_SET_HDMI_R2YMD_MAGIC << 16);
        ret = ioctl(fb_fd, TCC_LCDC_SET_HDMI_R2YMD, &r2ymd);
        if (ret) {
                ALOGE( "Failed TCC_LCDC_SET_HDMI_R2YMD IOCTL [%d]\n", ret);
        }
        return ret;

}

