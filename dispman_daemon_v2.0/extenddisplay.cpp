/*!
* TCC Version 1.0
* Copyright (c) Telechips Inc.
* All rights reserved 
*  \file        Extenddisplay.cpp
*  \brief       Display Daemon application
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
#include <unistd.h>
#include <signal.h>
#include <pthread.h>

#include <utils/Log.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <sys/stat.h>


#include <utils/types.h>
#include <utils/Log.h>
#include <utils/properties.h>
#include <utils/dispman_fb.h>
#include <video/tcc/tccfb_ioctrl.h>

#ifdef HAVE_HDMI_OUTPUT
#include <hdmi_v2_0/include/hdmi_ioctls.h>
#include <hdmi/hdmi_v2.h>
#include <hdmi/hdmi_drv.h>
#include <hdmi/hdmi_fb.h>
#include <hdmi/hdmi_edid.h>
#include <hdmi/hdmi_properties.h>
#include <cec/hdmi_cec.h>
#endif
#define CONFIG_DISPMAN_DAEMONIZE
#define DAEMON_VERSION  "AVN_2.0.7"

#define FB0_DEVICE	"/dev/fb0"
#define FTS_DEVICE	"/dev/fts"

#define LOG_NDEBUG			0
#define EXTENDDISPLAY_DEBUG     	0
#define EXTENDDISPLAY_THREAD_DEBUG	0

#define LOG_TAG "[EX_DISP   ]"

#if defined(TCC_HDCP)
#include <libhdcp/libhdcp.h>
#include <hdcpApp.h>
#endif

#if EXTENDDISPLAY_DEBUG
#define DPRINTF(args...)    ALOGD(args)
#else
#define DPRINTF(args...)
#endif

#if EXTENDDISPLAY_THREAD_DEBUG
#define TPRINTF(args...)    ALOGD(args)
#else
#define TPRINTF(args...)
#endif

// DISPLAY_OUTPUT_STB
unsigned int dispman_daemon_stbmode = 0;
static unsigned int output_auto_detection_flag = 0;
static unsigned int output_first_detection_flag = 0;
static unsigned int output_lcdc_always_on_flag = 0;
static tcc_display_resize saved_resizemode;

//static char output_resolution[16];
static unsigned int output_display_type;
static unsigned int output_display_mode;



#ifdef TCC_OUTPUT_COMPOSITE
#include <composite.h>
#endif

#ifdef TCC_OUTPUT_COMPONENT
#include <component.h>
#endif

#if defined(TCC_HDCP)
static void* hdcpHandle = NULL;
#endif

static pthread_t extend_display_thread_id;
static int ext_disp_stop = 0;
static int extendthreadready = 0;

struct FB {
        unsigned short *bits;
        unsigned size;
        int fd;
        struct fb_fix_screeninfo fi;
        struct fb_var_screeninfo vi;
};

static struct FB g_fb;

static void sighandler(int sig) 
{
	DPRINTF("sighandler-%d\r\n", sig);
	switch(sig)
	{
		case SIGINT:
		case SIGUSR2:
                        printf("SIGINT or SIGUSR2\r\n");
			ext_disp_stop = 1;
			break;
	}
}

#if 0
// This BlockSignals is deprecated.
static void BlockSignals(void)
{
    sigset_t signal_set;
    /* add all signals */
    sigfillset(&signal_set);
    /* set signal mask */
    pthread_sigmask(SIG_BLOCK, &signal_set, 0);
}
#endif

struct extend_display_mode{
	unsigned int OutMode;
	unsigned int SetMode; // 1080p / 720p : ntsc/pal
	unsigned int SubMode; // HDMI or DVI
	unsigned int CECMode;
	unsigned int AspectRatio;
	unsigned int ColorDepth;
	unsigned int ColorSpace;
	unsigned int Colorimetry;
	unsigned int HDCP1xEnable;
	unsigned int HdmiVideoFormat;
	unsigned int Hdmi3dStructure;
};

static struct extend_display_mode extend_display_mode;
static struct extend_display_mode extend_display_attachmode;
static unsigned int output_auto_detection = 0;
static unsigned int output_attach_dual = 0;
static unsigned int output_attach_dual_auto = 0;
static unsigned int output_attach_hdmi_cvbs = 0;

static unsigned int output_lcd_cvbs_attach_dual = 0;
static unsigned int stb_use_cvbs_presentation = 0;

// support hdmi hw cts - hpd is always 1
#if defined(HAVE_HDMI_OUTPUT)
unsigned int hdmi_hw_cts_mode = 0;
#endif

static int fb_open(struct FB *fb)
{
	fb->fd = open(FB0_DEVICE, O_RDWR);

        if(fb->fd < 0) {
                ALOGE("%s failed open hdmi fd", __func__);
                goto fb_fd_is_invalid;
        }
        if (ioctl(fb->fd, FBIOGET_FSCREENINFO, &fb->fi) < 0) {
                ALOGE("%s: FBIOGET_FSCREENINFO failed!\n",__func__);
        }
        if (ioctl(fb->fd, FBIOGET_VSCREENINFO, &fb->vi) < 0) {
                ALOGE("%s: FBIOGET_VSCREENINFO failed!\n",__func__);
        }
fb_fd_is_invalid:
        return 0;

}

static int extend_display_SuspendCheck(unsigned int OutMode)
{
	switch(OutMode)
	{
		#ifdef HAVE_HDMI_OUTPUT
		case OUTPUT_HDMI:
                        return hdmi_suspend_check();
		#endif//
		
		#ifdef TCC_OUTPUT_COMPOSITE
		case OUTPUT_COMPOSITE:			
			return composite_suspend_check();
		#endif//

		#ifdef TCC_OUTPUT_COMPONENT
		case OUTPUT_COMPONENT:
			return component_suspend_check();

		#endif//
		default:
			break;
	}
	return 0;
}


static int extend_display_check_user_need_reset(void) {
        int reset = 0;
    
	reset = property_get_int("persist.sys.extenddisplay_reset", 0);
	if(reset)
	{
		property_set("persist.sys.extenddisplay_reset", "0");
	}
        return reset;
}


static int extend_display_read_userconfig(struct extend_display_mode *display_mode)
{
        int reset;
	unsigned int OutputMode;
        unsigned int fb_suspend_status = 0;
        unsigned int extend_display_suspend_status;

        int extend_display_running = 0;

	char value[PROPERTY_VALUE_MAX];
	struct extend_display_mode user_display_mode;
	tcc_display_resize current_resize;

        memset(value, 0, PROPERTY_VALUE_MAX);
        memset(&user_display_mode, 0, sizeof(user_display_mode));
        memset(&current_resize, 0, sizeof(current_resize));
      


        reset = extend_display_check_user_need_reset();
	if(reset) {
		return 0;
	}

	if(output_lcd_cvbs_attach_dual) {
                OutputMode = OUTPUT_NONE;
        } else {
                OutputMode = property_get_int("persist.sys.output_mode", 0);
        }
	//ALOGI("[%s][%d] OutputMode=%d\r\n", __func__, __LINE__, OutputMode);
	#ifdef HAVE_HDMI_OUTPUT
	if(property_get_int("persist.hdmi.hw.cts", 0)) {
                hdmi_hw_cts_mode = 1;
        } else {
                hdmi_hw_cts_mode = 0;
        }
        
	if (OutputMode == OUTPUT_HDMI) {
		memset(value, 0, PROPERTY_VALUE_MAX);

		user_display_mode.SetMode = HDMI_GetVideoResolution(dispman_daemon_stbmode);

                /* Hdmi_mode is either DVI or HDMI */

		user_display_mode.SubMode = property_get_int("persist.sys.hdmi_mode", 0);

                /* Get suspend status of FB driver */
		fb_suspend_status = hdmi_lcdc_check();

		current_resize.resize_up = property_get_int("persist.sys.hdmi_resize_up", 0);
		current_resize.resize_down = property_get_int("persist.sys.hdmi_resize_dn", 0);
		current_resize.resize_left = property_get_int("persist.sys.hdmi_resize_lt", 0);
		current_resize.resize_right = property_get_int("persist.sys.hdmi_resize_rt", 0);

		if (dispman_daemon_stbmode) {
			user_display_mode.CECMode = hdmi_get_cec_status();
			if( (extend_display_mode.OutMode != OutputMode) || (extend_display_mode.CECMode != user_display_mode.CECMode)) {
				value[0] = '1';
				value[1] = '\0';
				property_set("persist.sys.output_mode", value);
			}
		}
		user_display_mode.AspectRatio = hdmi_get_PixelAspectRatio();
		user_display_mode.ColorDepth = hdmi_get_ColorDepth();
		user_display_mode.ColorSpace = hdmi_get_ColorSpace();
		user_display_mode.HDCP1xEnable = hdmi_get_HDCPEnableStatus();
		user_display_mode.Colorimetry = hdmi_get_Colorimetry();
                
                user_display_mode.HdmiVideoFormat = property_get_int("tcc.output.hdmi.video.format", 0);
                
                if(user_display_mode.HdmiVideoFormat == 2) {
			int tmp_val = property_get_int("tcc.output.hdmi.structure.3d", 0);
                        switch(tmp_val) {
                                case 0:
                                        // Frame-Packing
                                        user_display_mode.Hdmi3dStructure = 0;
                                break;
                                case 1: 
                                        // Top-and-Bottom
                                        user_display_mode.Hdmi3dStructure = 6;
                                        break;
                                case 2:
                                        // Side-by-Side(Half)
                                        user_display_mode.Hdmi3dStructure = 8;
                                        break;
                                }
                }
		user_display_mode.OutMode = OUTPUT_HDMI;
	}
	else 
	#endif//
	#ifdef TCC_OUTPUT_COMPOSITE
	if(OutputMode == OUTPUT_COMPOSITE)
	{
		user_display_mode.OutMode = OUTPUT_COMPOSITE;

		user_display_mode.SetMode = property_get_int("persist.sys.composite_mode", 0);
		user_display_mode.SubMode = 0;
                
		fb_suspend_status = composite_lcdc_check();	

		current_resize.resize_up = property_get_int("persist.sys.composite_resize_up", 0);
		current_resize.resize_down = property_get_int("persist.sys.composite_resize_dn", 0);
		current_resize.resize_left = property_get_int("persist.sys.composite_resize_lt", 0);
		current_resize.resize_right = property_get_int("persist.sys.composite_resize_rt", 0);
	}
	else
	#endif//
	#ifdef TCC_OUTPUT_COMPONENT
	if(OutputMode == OUTPUT_COMPONENT)
	{
		user_display_mode.OutMode = OUTPUT_COMPONENT;

		user_display_mode.SetMode = property_get_int("persist.sys.component_mode", 0);
		user_display_mode.SubMode = 0;

		fb_suspend_status = component_lcdc_check ();

		current_resize.resize_up = property_get_int("persist.sys.component_resize_up", 0);
		current_resize.resize_down = property_get_int("persist.sys.component_resize_dn", 0);
		current_resize.resize_left = property_get_int("persist.sys.component_resize_lt", 0);
		current_resize.resize_right = property_get_int("persist.sys.component_resize_rt", 0);

	}
	#endif//

	if( (OutputMode != OUTPUT_NONE) && memcmp(&saved_resizemode,  &current_resize, sizeof(current_resize))) {
		DPRINTF("[Previous] saved resize_up = %d, saved resize_down = %d, saved resize_left = %d, saved resize_right = %d\r\n"
                        "[ N e w  ] new   resize_up = %d, new   resize_down = %d, new   resize_left = %d, new   resize_right = %d",
                saved_resizemode.resize_up, saved_resizemode.resize_down, saved_resizemode.resize_left, saved_resizemode.resize_right, 
                current_resize.resize_up, current_resize.resize_down, current_resize.resize_left, current_resize.resize_right);

                if(g_fb.fd < 0) {
                        ALOGE("%s FB is not opend", __func__);
                        goto end_process;
                }
                
		if(ioctl(g_fb.fd, TCC_LCDC_SET_OUTPUT_RESIZE_MODE, &current_resize)) 
		{
			ALOGE("[%s] TCC_LCDC_SET_OUTPUT_OFFSET failed!\n",__func__);
			goto end_process;
		}


                memcpy(&saved_resizemode, &current_resize, sizeof(current_resize));
	}

        //DPRINTF("~ %s %d Output:OUT:%d Set:%d suspend:%d ~ \n",__func__, run_check, check_output.OutMode, check_output.SetMode, uiSuspend);

        if(output_lcd_cvbs_attach_dual)
                user_display_mode.OutMode = OUTPUT_NONE;

        if(display_mode) {
                memcpy(display_mode, &user_display_mode, sizeof(user_display_mode));
        }
	
        extend_display_suspend_status = extend_display_SuspendCheck(user_display_mode.OutMode);

        if(extend_display_suspend_status && output_attach_hdmi_cvbs)
                output_lcdc_always_on_flag=0;

        if((user_display_mode.OutMode && (!fb_suspend_status) && (!extend_display_suspend_status)) || output_lcd_cvbs_attach_dual)
                extend_display_running = 1;

end_process:
        return extend_display_running;
}

static void extend_display_wait_ms(__attribute__((unused))struct extend_display_mode *display_mode, int time_out_ms)
{
        int OutMode = 0;
        
        if(display_mode) {
                OutMode = display_mode->OutMode;
        }
        
        switch(OutMode)
        {
                case OUTPUT_HDMI:
                        HDMIDrv_poll_wait(time_out_ms);
                        break;
                default:
                        usleep(time_out_ms * 1000);
                        break;
        }
}

static int extend_display_check_output_underrun(void)
{
	return property_get_int("tcc.sys.output_underrun", 0);
}

void extend_display_device_init()
{
	dispman_daemon_stbmode = property_get_int("tcc.sys.output_mode_stb", 0);
	#ifdef HAVE_HDMI_OUTPUT
	hdmi_display_init();
	#endif /* HAVE_HDMI_OUTPUT */

	#ifdef TCC_OUTPUT_COMPOSITE
	composite_display_init();
	#endif /* TCC_OUTPUT_COMPOSITE */
	
	#ifdef TCC_OUTPUT_COMPONENT
	component_display_init();
	#endif /* TCC_OUTPUT_COMPONENT */

	fb_open(&g_fb);
	
}

void extend_display_device_deinit()
{
	#ifdef HAVE_HDMI_OUTPUT
	hdmi_display_deinit();
	#endif /* HAVE_HDMI_OUTPUT */

	#ifdef TCC_OUTPUT_COMPOSITE
	composite_display_deinit();
	#endif /* TCC_OUTPUT_COMPOSITE */
	
	#ifdef TCC_OUTPUT_COMPONENT
	component_display_deinit();
	#endif /* TCC_OUTPUT_COMPONENT */

	close(g_fb.fd);
}

int extend_display_detect_OnOff(char onoff, unsigned int OutMode)
{
	switch(OutMode)
	{
		#ifdef HAVE_HDMI_OUTPUT
		case OUTPUT_HDMI:
			return hdmi_display_detect_onoff(onoff);
		#endif//
		
		#ifdef TCC_OUTPUT_COMPOSITE
		case OUTPUT_COMPOSITE:			
			return composite_display_detect_onoff(onoff);
		#endif//
		
		#ifdef TCC_OUTPUT_COMPONENT
		case OUTPUT_COMPONENT:
			return component_display_detect_onoff(onoff);
		#endif//

		default:
			break;
	}
	return 0;
}

int extend_display_output_set(unsigned int OutMode, char onoff)
{
	int result;
        char lcdc_composite;
        char lcdc_component;
	static int oneshot = 1;
	
	if(!output_lcd_cvbs_attach_dual)
	{
		if(onoff)
			property_set("tcc.sys.output_mode_detected", "1");
		else
			property_set("tcc.sys.output_mode_detected", "0");
	}
			
	if (dispman_daemon_stbmode) {
		if(output_auto_detection || output_attach_dual_auto || output_attach_hdmi_cvbs)
		{
			if(onoff == 0)
			{
				property_set("tcc.sys.output_mode_plugout", "1");
				usleep(100000);
			}

		}
	}
        stb_use_cvbs_presentation = property_get_int("tcc.hwc.use.cvbs.presentation", 0);
	if(stb_use_cvbs_presentation) {
		lcdc_composite = COMPOSITE_LCDC_1;
		lcdc_component = COMPONENT_LCDC_0;
	} else {
		lcdc_composite = COMPOSITE_LCDC_0;
		lcdc_component = COMPONENT_LCDC_0;
	}


	if (dispman_daemon_stbmode) {
		if((output_attach_dual || output_attach_dual_auto) && onoff)
		{
			char value[PROPERTY_VALUE_MAX];

			memset(value, 0, PROPERTY_VALUE_MAX);
			
			if(OutMode == OUTPUT_COMPOSITE)
			{
				value[0] = '0' + OUTPUT_COMPOSITE; 
				property_set("persist.sys.output_attach_sub", value);
			}
			else
			{
				value[0] = '0' + OutMode; 
				property_set("persist.sys.output_attach_main", value);
				value[0] = '0' + OUTPUT_COMPOSITE; 
				property_set("persist.sys.output_attach_sub", value);
			}
		}
	}
	
	switch(OutMode)
	{
		#ifdef HAVE_HDMI_OUTPUT
		case OUTPUT_HDMI:
				#ifdef TCC_OUTPUT_COMPOSITE
				if(dispman_daemon_stbmode && 
					(output_attach_dual || output_attach_dual_auto || output_attach_hdmi_cvbs))
				{
					hdmi_display_output_set(onoff, hdmi_hw_cts_mode);
					if (oneshot && onoff) {
						composite_display_detect_onoff(1);
						composite_display_output_attach(onoff, lcdc_composite);
						composite_display_detect_onoff(0);
						oneshot = 0;
					}

					result = 0;
				}

 				else
				#endif
					result = hdmi_display_output_set(onoff, hdmi_hw_cts_mode);
				break;
		#endif
		
		#ifdef TCC_OUTPUT_COMPOSITE
		case OUTPUT_COMPOSITE:			
				if(dispman_daemon_stbmode && output_attach_dual_auto)
					result = composite_display_output_attach(onoff, lcdc_composite);
				else {
					if(output_lcd_cvbs_attach_dual)
						result = composite_display_output_attach(onoff, lcdc_composite);
					else
						result = composite_display_output_set(onoff, lcdc_composite);
				}

				if(onoff) {
					int dataindex = 0;
					char data[SR_ATTR_MAX] = {0};
					dataindex += sprintf(data+dataindex, "%02d:%04dx%04d@%02d%c ", 0, 720, 480, 60, 'I');
					dataindex += sprintf(data+dataindex, "%02d:%04dx%04d@%02d%c ", 1, 720, 576, 50, 'I');
					property_supported_resolution_set(data);
				}
				else {
					property_supported_resolution_clear();
				}
				break;
		#endif
		
		#ifdef TCC_OUTPUT_COMPONENT
		case OUTPUT_COMPONENT:
				if(dispman_daemon_stbmode && (output_attach_dual || output_attach_dual_auto))
				{
 					component_display_output_set(onoff, lcdc_component);					
					if (oneshot && onoff) {
						composite_display_detect_onoff(1);
						composite_display_output_attach(onoff, lcdc_composite);
						composite_display_detect_onoff(0);
						oneshot = 0;
					}
					result = 0;
				}
 				else
					result = component_display_output_set(onoff, lcdc_component);

				if(onoff) {
					int dataindex = 0;
					char data[SR_ATTR_MAX] = {0};
					dataindex += sprintf(data+dataindex, "%02d:%04dx%04d@%02d%c ", 0, 1280, 720, 60, 'P');
					dataindex += sprintf(data+dataindex, "%02d:%04dx%04d@%02d%c ", 1, 1920, 1080, 60, 'I');
					property_supported_resolution_set(data);
				}
				else {
					property_supported_resolution_clear();
				}
				break;
		#endif

		default:
			break;
	}

	return result;
}

int extend_display_AudioInputPortDetect(unsigned int OutMode)
{
	switch(OutMode)
	{
		#ifdef HAVE_HDMI_OUTPUT
		case OUTPUT_HDMI:
			return hdmi_AudioInputPortDetect();
		#endif//
		
		#ifdef TCC_OUTPUT_COMPOSITE
		case OUTPUT_COMPOSITE:			
			return 1;
		#endif//

		#ifdef TCC_OUTPUT_COMPONENT
		case OUTPUT_COMPONENT:
			return 1;


		#endif//
		default:
			break;
	}
	return 0;
}

static int extend_display_CableDetect(unsigned int OutMode)
{
        int cableDectect = 0;
 
	switch(OutMode)
	{
		#ifdef HAVE_HDMI_OUTPUT
		case OUTPUT_HDMI:
			int hdmi_hw_cts_mode;
			hdmi_hw_cts_mode = property_get_int("persist.hdmi.hw.cts", 0);
                	if(hdmi_hw_cts_mode == 1 ) {
                                cableDectect = 1;
                                break;
                        } else if(hdmi_hw_cts_mode == 2) {
                                cableDectect = 0;
                                break;
			}
			cableDectect =  hdmi_display_cabledetect();
                        //ALOGI("[%s][%d] cable_detect = %d\r\n", __func__, __LINE__, cableDectect);
			break;
		#endif//
		
		#ifdef TCC_OUTPUT_COMPOSITE
		case OUTPUT_COMPOSITE:			
			cableDectect =  composite_display_cabledetect();
                        //ALOGI("[%s][%d] cable_detect = %d\r\n", __func__, __LINE__, cableDectect);
			break;
		#endif//

		#ifdef TCC_OUTPUT_COMPONENT
		case OUTPUT_COMPONENT:
			cableDectect = component_display_cabledetect();
                        //ALOGI("[%s][%d] cable_detect = %d\r\n", __func__, __LINE__, cableDectect);
			break;
		#endif//
		default:
			break;
	}
	return cableDectect;
}

static int extend_display_AutoDetect(void)
{
	struct extend_display_mode display_mode;
	char value[PROPERTY_VALUE_MAX];
	unsigned int output_mode;

	/* check the cable detection of current output mode */
	if(extend_display_CableDetect(extend_display_mode.OutMode))
		return 1;

	/* release the cable detection of currnet output mode */
	extend_display_detect_OnOff(0, extend_display_mode.OutMode);

	/* check the cable detection */
	for(output_mode=OUTPUT_HDMI; output_mode<OUTPUT_MAX; output_mode++)
	{
		if (dispman_daemon_stbmode) {
			if(output_attach_dual_auto && (output_mode == OUTPUT_COMPOSITE))
				continue;
		}

		extend_display_detect_OnOff(1, output_mode);
		
		if(extend_display_CableDetect(output_mode))
		{
			TPRINTF("Old OUTPUT:%d, New OUTPUT:%d Auto-Detection!!\n", extend_display_mode.OutMode, output_mode);
			
			if(output_mode != extend_display_mode.OutMode)
			{
				memset(value, 0, PROPERTY_VALUE_MAX);
				value[0] = '0' + output_mode;
				property_set("persist.sys.output_mode", value);
			}

			break;
		}

		extend_display_detect_OnOff(0, output_mode);
	}
		
	if(output_mode == OUTPUT_MAX){
		/* open the cable detection of currnet output mode */
		extend_display_detect_OnOff(1, extend_display_mode.OutMode);
 	} else {
		/* set the new output mode */
		extend_display_read_userconfig(&display_mode);
		extend_display_mode = display_mode;
		return 1;
	}

	return 0;
}

static int extend_display_checkdetect(struct extend_display_mode *display_mode)
{
	int cable_detect = 0;

	if(output_auto_detection || output_attach_dual_auto || output_attach_hdmi_cvbs) {
		if(output_auto_detection_flag) {
			cable_detect = extend_display_AutoDetect();
                        
                        //ALOGI("[%s][%d] cable_detect = %d\r\n", __func__, __LINE__, cable_detect);
			if(cable_detect)
				extend_display_read_userconfig(display_mode);
		}
		else {
			cable_detect = extend_display_CableDetect(extend_display_mode.OutMode);
                        //ALOGI("[%s][%d] cable_detect = %d\r\n", __func__, __LINE__, cable_detect);
		}
	}
	else {       
		cable_detect = extend_display_CableDetect(extend_display_mode.OutMode);
                //ALOGI("[%s][%d] cable_detect = %d\r\n", __func__, __LINE__, cable_detect);
	}

	return cable_detect;
}

int extend_display_InitFirstOutput(void)
{
	int cable_detect = 0;

	/* reset component output after displaying dual output in bootloader/kernel */
	if(output_first_detection_flag == 0)
	{
		if(extend_display_mode.OutMode == OUTPUT_HDMI)
		{
			cable_detect = extend_display_CableDetect(extend_display_mode.OutMode);
			
			if( (cable_detect == 0) && (output_display_type == 0) && (output_auto_detection == 0) && 
				(output_attach_dual == 0) && (output_attach_dual_auto == 0) && (output_attach_hdmi_cvbs == 0) )
			{
				extend_display_detect_OnOff(1, OUTPUT_COMPONENT);
				extend_display_output_set(OUTPUT_COMPONENT, 0);
				extend_display_detect_OnOff(0, OUTPUT_COMPONENT);
			}

			return 1;
		}
	}

	return 0;
}

int extend_display_SetAttachOutput(void)
{
        int imode;
	char value[PROPERTY_VALUE_MAX];

	memset(value, 0, PROPERTY_VALUE_MAX);

	imode  = property_get_int("persist.sys.composite_mode", 0);
        
	if( (output_attach_hdmi_cvbs && extend_display_attachmode.SetMode != (unsigned int)imode && extend_display_mode.OutMode == OUTPUT_HDMI) || 
		(output_attach_dual_auto && extend_display_attachmode.SetMode != (unsigned int)imode) )
	{
		extend_display_detect_OnOff(1, OUTPUT_COMPOSITE);
		extend_display_output_set(OUTPUT_COMPOSITE, 1);
		extend_display_detect_OnOff(0, OUTPUT_COMPOSITE);

                extend_display_attachmode.SetMode = imode;
		return 1;
	}

	return 0;
}

int extend_display_SetDisplayMode(void)
{
	char value[PROPERTY_VALUE_MAX];

	memset(value, 0, PROPERTY_VALUE_MAX);

        if(g_fb.fd > 0) {
        	/* get display type - 0: HDMI/CVBS/COMPONENT, 1: HDMI/CVBS, 2: HDMI */
        	if(ioctl(g_fb.fd, TCC_LCDC_GET_DISPLAY_TYPE, &output_display_type) >= 0) 
        	{
        		/* set property for display type */
        		value[0] = '0' + output_display_type;
        		property_set("tcc.output.display.type", value);
	        }
	} else {
                ALOGE("%s FB is not opend", __func__);
        }
	output_attach_dual = 0;
	output_attach_dual_auto = 0;
	output_attach_hdmi_cvbs = 0;

	output_display_mode = property_get_int("persist.sys.display.mode", 0);

	if(output_display_mode == 1)
		output_auto_detection = 1;
	else if(output_display_mode == 2)
		output_attach_hdmi_cvbs = 1;
	else if(output_display_mode == 3)
		output_attach_dual_auto = 1;

	if(output_attach_dual || output_attach_dual_auto)
	{
		memset(&extend_display_attachmode, 0, sizeof(extend_display_attachmode));
		extend_display_attachmode.OutMode = OUTPUT_COMPOSITE;
		output_auto_detection_flag = 1;
	}
	else if(output_attach_hdmi_cvbs)
	{
		memset(&extend_display_attachmode, 0, sizeof(extend_display_attachmode));
		extend_display_attachmode.OutMode = OUTPUT_COMPOSITE;
		extend_display_attachmode.SetMode = property_get_int("persist.sys.composite_mode", 0);
		output_auto_detection_flag = 1;
	}
	else if(output_auto_detection)
	{
		/* B140019 : set auto detection for the exception handling 
		* (board is booted without HDMI plug-in) */
		output_auto_detection_flag = 1;
	}
		
	return 0;
}

int extend_display_SaveData(char *output_data, char data_size)
{
	return 0;
}
static int extend_display_check_display_mode_change(struct extend_display_mode *display_mode)
{
        int mode_change = 1;
        
	if (dispman_daemon_stbmode) {
			if(output_attach_dual_auto && (display_mode->OutMode == OUTPUT_COMPOSITE)) {
				mode_change = 0;
			} else {
				if(display_mode->OutMode == extend_display_mode.OutMode) {
					mode_change = 0;
			}
                }
	}
        if((display_mode->OutMode == extend_display_mode.OutMode) || output_lcd_cvbs_attach_dual)	
                mode_change = 0;

        return mode_change;	
}

static char extend_display_check_lcd_cvbs_mode()
{
        #if defined(TCC_OUTPUT_COMPOSITE)
	char value[PROPERTY_VALUE_MAX];
	unsigned int lcd_cvbs_check;
		
	property_get("ro.system.cvbs_active", value, ""); //CVBS active check

	if(strcmp(value,"true") == 0)
	{
		lcd_cvbs_check = property_get_int("persist.sys.cvbs_power_mode", 0); //CVBS power check

		if(lcd_cvbs_check)
		{
			output_lcd_cvbs_attach_dual = 1;
			output_auto_detection_flag = 1;

			composite_display_detect_onoff(1);
			extend_display_output_set(OUTPUT_COMPOSITE, 1);
		}
		else
		{
			extend_display_output_set(OUTPUT_COMPOSITE, 0);
			composite_display_detect_onoff(0);
			
			output_lcd_cvbs_attach_dual = 0;
			output_auto_detection_flag = 0;
		}		

		if(output_lcd_cvbs_attach_dual)
		{
			return 1;
		}
		else
			return 0;
	}
	else
	{
		return 0 ;
	}
        #else
        return 0;
        #endif
}


static int extend_display_wait_cable_attach(struct extend_display_mode *display_mode) 
{
        int cable_detect = 0;
        int extend_display_running;

        if(display_mode == NULL) {
                extend_display_running = 0;
                goto end_process;
        }
        ALOGI("[%s][%d]\r\n", __func__, __LINE__);
        extend_display_running = extend_display_read_userconfig(display_mode);
        //ALOGI("[%s][%d] extend_display_running = %d\r\n", __func__, __LINE__, extend_display_running);
        while(!cable_detect && extend_display_running && !extend_display_check_display_mode_change(display_mode)) {
                extend_display_wait_ms(display_mode, 100);
                extend_display_running = extend_display_read_userconfig(display_mode);
                if(!extend_display_running) {
                        ALOGI("[%s][%d] extenddispaly is not running", __func__, __LINE__);
                        break;
                }
        	if (dispman_daemon_stbmode) {
                        cable_detect = extend_display_checkdetect(display_mode);
                        //ALOGI("[%s][%d] cable_detect = %d\r\n", __func__, __LINE__, cable_detect);

        	} else {
                        if(extend_display_check_lcd_cvbs_mode()){
                                cable_detect = extend_display_CableDetect(OUTPUT_COMPOSITE);
                                extend_display_mode.OutMode = OUTPUT_NONE;
                                //ALOGI("[%s][%d] cable_detect = %d\r\n", __func__, __LINE__, cable_detect);
                        }
                        else {
                                cable_detect = extend_display_CableDetect(extend_display_mode.OutMode);
                                //ALOGI("[%s][%d] cable_detect = %d\r\n", __func__, __LINE__, cable_detect);
                        }
        	}
	}		       

        extend_display_mode.SetMode = display_mode->SetMode;
        extend_display_mode.SubMode = display_mode->SubMode;
	
	if(output_first_detection_flag == 0)
                output_first_detection_flag = 1;

        ALOGI("[%s][%d] run start Mode:%d Mode:%d cable:%d \n", __func__, __LINE__, extend_display_mode.OutMode, display_mode->OutMode, cable_detect);
        
        if(extend_display_running && !extend_display_check_display_mode_change(display_mode))
        {				
        	ALOGI("[%s][%d] output_setting Mode:%d Mode:%d cable:%d\n", __func__, __LINE__, extend_display_mode.OutMode, display_mode->OutMode, cable_detect);

        	if(output_lcd_cvbs_attach_dual)
        		extend_display_output_set(OUTPUT_COMPOSITE, 1);
        	else
        		extend_display_output_set(extend_display_mode.OutMode, 1);

        	if(stb_use_cvbs_presentation && extend_display_mode.OutMode == OUTPUT_HDMI && !output_lcd_cvbs_attach_dual) {
        		#ifdef TCC_OUTPUT_COMPOSITE
        		composite_display_detect_onoff(1);
        		#endif
        		extend_display_output_set(OUTPUT_COMPOSITE, 1);
        	}

        }

	#if defined(TCC_HDCP)
	if (extend_display_mode.OutMode == OUTPUT_HDMI)
		hdcpAppCallback(hdcpHandle, 1/* plugged */, hdmi_get_hdmi_mode() ? HDCP_MODE_HDMI : HDCP_MODE_DVI);
	#endif

end_process:
        return extend_display_running;
}

static int extend_display_wait_cable_detach(struct extend_display_mode *display_mode) 
{       
        int cable_detect;
        int hdmi_run_status;
        int extend_display_running;

        if(display_mode == NULL) {
                extend_display_running = 0;
                goto end_process;
        }
        ALOGI("[%s][%d] enter\r\n", __func__, __LINE__);
        extend_display_running = extend_display_read_userconfig(display_mode);
        while(extend_display_running && !extend_display_check_display_mode_change(display_mode)) {
		extend_display_wait_ms(display_mode, 100);

		extend_display_running = extend_display_read_userconfig(display_mode);
		
		/* CEC Process */
		if(property_get_int("tcc.cec.imageview_on", 0))
			hdmi_cec_control_TV(1);
	
		if(extend_display_check_lcd_cvbs_mode())
			cable_detect = extend_display_CableDetect(OUTPUT_COMPOSITE);
		else
			cable_detect = extend_display_CableDetect(extend_display_mode.OutMode);

                if(!cable_detect) {
                        ALOGI("[%s][%d] cable is detached", __func__, __LINE__);
                        break;
                }
               
		extend_display_AudioInputPortDetect(extend_display_mode.OutMode);
                hdmi_run_status = HDMI_GetRunStatus();


		
	        if (!dispman_daemon_stbmode || 
                        (dispman_daemon_stbmode && !extend_display_SetAttachOutput()))
                {                              
			#ifdef HAVE_HDMI_OUTPUT     
			// CECMode is not necessry to compare.
			extend_display_mode.CECMode = display_mode->CECMode;
                        /****************************************************************
                        UI replication with SBS/TNB regardless whether the TV supports 3d 
                        mode. The following code has been annotated to provide this 
                        functionality
                        ----------------------------------------------------------------
                        if(!hdmi_sink_support_hdmi3d()) {
                                display_mode->HdmiVideoFormat = extend_display_mode.HdmiVideoFormat;
                                display_mode->Hdmi3dStructure = extend_display_mode.Hdmi3dStructure;
                        }
                        ****************************************************************/
			#endif
			if(memcmp(&extend_display_mode, display_mode, sizeof(extend_display_mode))) {
                                ALOGI("[%s][%d] may be user configuration is changed\r\n", __func__, __LINE__);
			        // different between previous and current mode.
				extend_display_running = 0;
                                break;
			}else {
			        //ALOGI("[%s][%d] user configuration is not changed", __func__, __LINE__);
                        }
                        

                        #ifdef HAVE_HDMI_OUTPUT 
                        if(extend_display_mode.OutMode == OUTPUT_HDMI) {
                                if(extend_display_running) {
                                        if(hdmi_run_status) {
                                                if(hdmi_display_runtime_check() < 0) {
                                                     extend_display_running = 0;
                                                     ALOGI("[%s][%d] Result of HDMI runtime check is non zero\r\n", __func__, __LINE__);
                                                     break;
                                                }
                                        }
                                        else  {
                                                extend_display_running = 0;
                                                ALOGI("[%s][%d] HDMI is not running status\r\n", __func__, __LINE__);
                                                break;
                                        }
                                }  
                        }
                        CEC_Func_Running(hdmi_get_cec_status());
                        #endif
		}
	}

#if defined(TCC_HDCP)
	if (extend_display_mode.OutMode == OUTPUT_HDMI) {
		if (cable_detect)
			hdcpAppCallback(hdcpHandle, 2/* state changed */, \
					hdmi_get_hdmi_mode() ? HDCP_MODE_HDMI : HDCP_MODE_DVI);
		else
			hdcpAppCallback(hdcpHandle, 0/* unpluged */, 0);
	} else {
		hdcpAppCallback(hdcpHandle, 0/* unpluged */, 0);
	}
#endif

end_process:
        return extend_display_running;
}

static void* extend_display_thread(__attribute__((unused))void* arg)
{
        int extend_display_running;
	struct extend_display_mode display_mode;


	// SIG ------------------------------
	sigset_t signal_set;
	sigemptyset(&signal_set);
	sigaddset(&signal_set, SIGINT);				// CTRL+C
	sigaddset(&signal_set, SIGUSR2);			// KILL
	pthread_sigmask(SIG_UNBLOCK, &signal_set, 0);	// BLOCK CTRL+C/KILL
	signal(SIGINT, sighandler);
	signal(SIGUSR2, sighandler);
	// ----------------------------------

	extend_display_device_init();
	property_set("tcc.sys.output_mode_detected", "0");
	property_set("persist.sys.extenddisplay_reset", "0");
        #ifdef HAVE_HDMI_OUTPUT
	hdmi_set_output_detected(false);
        #endif

	TPRINTF("%s Set_Mode:%d Cur_Mode:%d cable:%d running:%d ~~ \n",
				__func__, CurrentMode.OutMode, display_mode.OutMode, cable_detect, running);

	if (dispman_daemon_stbmode) {
		extend_display_SetDisplayMode();
	}
	extend_display_check_lcd_cvbs_mode();
	
	while(1)
	{
                do {
			usleep(100000);
                        extend_display_running = extend_display_read_userconfig(&display_mode);
		}while(!extend_display_running);

		extend_display_detect_OnOff(1, display_mode.OutMode);
                /* Update extend_display_mode from user config. */
		extend_display_mode = display_mode;

         	#if defined(TCC_HDCP)
		if (!hdcpHandle)
			hdcpHandle = hdcpAppInit();
		#endif

		if (dispman_daemon_stbmode) {
			extend_display_InitFirstOutput();
			extendthreadready = 1;
		}
                
		while(extend_display_running && !extend_display_check_display_mode_change(&display_mode))
		{
		        extend_display_running = extend_display_wait_cable_attach(&display_mode);
                        if(extend_display_running) {
                                ALOGI("[%s][%d] cable is attached\r\n", __func__, __LINE__);
                                /*
                                * Unlock vsync_lock after end of changing (if it was locked).
                                */
                                property_vsync_lock_set(0);
                                extend_display_running = extend_display_wait_cable_detach(&display_mode);
                                extend_display_output_set(extend_display_mode.OutMode, 0);
                                /*
                                * Lock tcc_vsync after END_VSYNC and disable disp
                                * locking period: changing output (resoultion, outputmode etc.)
                                */
                                property_vsync_lock_set(1);
				
				#if defined(TCC_OUTPUT_COMPOSITE)
        			if(stb_use_cvbs_presentation && extend_display_mode.OutMode == OUTPUT_HDMI && !output_lcd_cvbs_attach_dual) {
        				extend_display_output_set(OUTPUT_COMPOSITE, 0);
        				composite_display_detect_onoff(0);
        			}
				#endif


			        if(dispman_daemon_stbmode && (output_auto_detection_flag == 0))
					output_auto_detection_flag = 1;

                                // Re-initialize the output device when underrun occurs on it.
                                if(extend_display_check_output_underrun()) {
                                        extend_display_running = 0;
                                }
                        }
		}

		#if defined(TCC_HDCP)
		if (hdcpHandle) {
			hdcpAppDeinit(hdcpHandle);
			hdcpHandle = NULL;
		}
		#endif

		// release to cable detect		
		extend_display_detect_OnOff(0, extend_display_mode.OutMode);
		usleep(200000);
	}

	return 0;
}

//Temp
static unsigned int extend_display_init_flag = 0;

int extend_display_init()
{
        ALOGI("[%s] Init Extend Display", __func__);
        #if defined(HAVE_HDMI_OUTPUT) || defined(TCC_OUTPUT_COMPONENT) || defined(TCC_OUTPUT_COMPOSITE)
	if(extend_display_init_flag == 1)
		return 0;

        ALOGI("[%s] run extenddisplay thread", __func__);
	extend_display_init_flag = 1;
	if (pthread_create(&extend_display_thread_id, NULL, &extend_display_thread, NULL))	{
		return -1;
	}

	if (dispman_daemon_stbmode) {
		while(!extendthreadready)
			usleep(1000);
	}

	property_set("persist.sys.extenddisplay_reset", "0");

#endif//
    return 0;
}

int extend_display_deinit()
{
	#if defined(HAVE_HDMI_OUTPUT) || defined(TCC_OUTPUT_COMPONENT) || defined(TCC_OUTPUT_COMPOSITE)
	extend_display_device_deinit();
	#endif//        
    return 0;
}

#if defined(CONFIG_DISPMAN_DAEMONIZE)
static void Daemonize()
{
	pid_t pid = 0;

	// create child process
	pid = fork();

	// fork failed
	if (pid < 0)
	{
		fprintf(stderr, "fork failed\n");
		exit(1);
	}

	// parent process
	if (pid > 0)
	{
		// exit parent process for daemonize
		_exit(0);
	}
	
	// umask the file mode
	umask(0);

	// set new session
	if (setsid() < 0)
	{
		fprintf(stderr ,"set new session failed\n");
		exit(1);
	}

	// change the current working directory for safety
	if (chdir("/") < 0)
	{
		fprintf(stderr, "change directory failed\n");
		exit(1);
	}
}
#endif

int main()
{        
        int retry = 3;
        #if !defined(CONFIG_DISPMAN_DAEMONIZE)
	// SIG ------------------------------
	sigset_t signal_set;
	sigemptyset(&signal_set);
	sigaddset(&signal_set, SIGINT);				// CTRL+C
	sigaddset(&signal_set, SIGUSR2);			// KILL
        pthread_sigmask(SIG_BLOCK, &signal_set, 0);	// BLOCK CTRL+C/KILL
	// ----------------------------------
        #endif
        
        if(!display_daemon_lock()) {
                fprintf(stderr, "display_daemon is duplicate.. finish \r\n");

                return -1;
        }

	for(;retry;retry--) {
		if(display_daemon_lock()) {
			break;
		}
		usleep(500000);
	}
        if(retry < 0) {
                fprintf(stderr, "display_daemon is duplicate.. finish \r\n");
                return -1;
        }
        
        #if defined(CONFIG_DISPMAN_DAEMONIZE)
        Daemonize();
        #endif
        

        fprintf(stderr, "display_daemon %s\r\n", DAEMON_VERSION);
	extend_display_init();

	//---
	while(!ext_disp_stop) {
		usleep(100000);
	}
	
	extend_display_deinit();
        display_daemon_unlock();
	return 0;
}
