/****************************************************************************
Copyright (C) 2013 Telechips Inc.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
****************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>

#include <utils/Log.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <linux/fb.h>

#include <sys/stat.h>

#include <utils/Log.h>
#include <utils/properties.h>
#include <libhdmi/libhdmi.h>
#include <libphy/libphy.h>
#include <mach/tccfb_ioctrl.h>
#include <mach/tcc_composite_ioctl.h>
#include <mach/tcc_component_ioctl.h>
#include <mach/tcc_fts_ioctl.h>




//#include <mtdutils.h>
#define FB0_DEVICE	"/dev/fb0"
#define FTS_DEVICE	"/dev/fts"

#define LOG_NDEBUG			0
#define EXTENDDISPLAY_DEBUG     	0
#define EXTENDDISPLAY_THREAD_DEBUG	0

#define LOG_TAG				"-EX_DISP-"
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
static unsigned int uiOutputResizeMode_up = 0, uiOutputResizeMode_down = 0, uiOutputResizeMode_left = 0, uiOutputResizeMode_right = 0;

static char output_resolution[16];
static unsigned int output_display_type;
static unsigned int output_display_mode;

static unsigned int prev_output_second_attach = (unsigned int)-1;

#ifdef HAVE_HDMI_OUTPUT
extern int hdmi_display_init(char hdmi_reset);
extern int hdmi_display_deinit(void);
extern unsigned int hdmi_lcdc_check(void);
extern unsigned int hdmi_suspend_check(void);
extern int hdmi_display_detect_onoff(char onoff);
extern HDMIAudioPort hdmi_get_AudioInputPort(void);
extern ColorDepth hdmi_get_ColorDepth(void);
extern ColorSpace hdmi_get_ColorSpace(void);
extern int hdmi_get_HDCPEnableStatus(void);
extern PixelAspectRatio hdmi_get_PixelAspectRatio(void);
extern int hdmi_AudioInputPortDetect(void);
extern int hdmi_compare_resolution(int width, int height);
extern void hdmi_set_output_detected(unsigned int detected);

extern int hdmi_display_output_set(char onoff);
extern int hdmi_display_output_modeset(char enable);
extern int hdmi_display_cabledetect(void);
extern int hdmi_AudioOnOffChk(void);
extern int HDMI_GetVideoResolution(void);
extern int CECCmdProcess(void);
extern int hdmi_set_video_format_ctrl(unsigned int HdmiVideoFormat, unsigned int Structure_3D);
extern int hdmi_cmd_process(void);
extern int hdmi_api_demo(void);
#endif//

#ifdef TCC_OUTPUT_COMPOSITE
extern int composite_display_init(void);
extern int composite_display_deinit(void);
extern unsigned int composite_lcdc_check(void);
extern unsigned int composite_suspend_check(void);
extern int composite_send_hpd_status(void);
extern int composite_display_detect_onoff(char onoff);
extern int composite_display_output_set(char onoff, char lcdc);
extern int composite_display_output_modeset(char enable);
extern int composite_display_output_attach(char onoff, char lcdc);
extern int composite_display_cabledetect(void);
#endif//

#ifdef TCC_OUTPUT_COMPONENT
extern int component_display_init(void);
extern int component_display_deinit(void);
extern unsigned int component_lcdc_check(void);
extern unsigned int component_suspend_check(void);
extern int component_display_detect_onoff(char onoff);
extern int component_display_output_set(char onoff, char lcdc);
extern int component_display_output_modeset(char enable);
extern int component_display_cabledetect(void);
#endif//

static pthread_t extend_display_thread_id;

static int running = 0;
static int ext_disp_stop = 0;
static int cable_detect = 0;
static int extendthreadready = 0;

struct FB {
    unsigned short *bits;
    unsigned size;
    int fd;
    struct fb_fix_screeninfo fi;
    struct fb_var_screeninfo vi;
};

struct FB fb;

#if defined(NO_DEAMON)
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

static void BlockSignals(void)
{
    sigset_t signal_set;
    /* add all signals */
    sigfillset(&signal_set);
    /* set signal mask */
    pthread_sigmask(SIG_BLOCK, &signal_set, 0);
}
#endif

typedef struct{
	unsigned int OutMode;
	unsigned int SetMode; // 1080p / 720p : ntsc/pal
	unsigned int SubMode; // HDMI or DVI
	unsigned int CECMode;
	unsigned int AspectRatio;
	unsigned int ColorDepth;
	unsigned int ColorSpace;
	unsigned int HDCP1xEnable;
}extend_display_mode;
extend_display_mode CurrentMode;

static extend_display_mode OutputAttachMode;
static unsigned int output_auto_detection = 0;
static unsigned int output_attach_dual = 0;
static unsigned int output_attach_dual_auto = 0;
static unsigned int output_attach_hdmi_cvbs = 0;

static unsigned int output_lcd_cvbs_attach_dual = 0;
static unsigned int stb_use_cvbs_presentation = 0;

static unsigned int gHdmiVideoFormat = 0;
static unsigned int gStructure_3D = 0;

static int fb_open(struct FB *fb)
{
	fb->fd = open(FB0_DEVICE, O_RDWR);

    if (ioctl(fb->fd, FBIOGET_FSCREENINFO, &fb->fi) < 0)
        DPRINTF("%s: FBIOGET_FSCREENINFO failed!\n",__func__);
    if (ioctl(fb->fd, FBIOGET_VSCREENINFO, &fb->vi) < 0)
        DPRINTF("%s: FBIOGET_VSCREENINFO failed!\n",__func__);

	return 0;

}

static void fb_update(struct FB *fb)
{
	// Get FB information
	if (ioctl(fb->fd, FBIOGET_VSCREENINFO, &fb->vi) < 0)
	{
		DPRINTF("%s: FBIOGET_VSCREENINFO failed!\n",__func__);
	}

	fb->vi.activate = FB_ACTIVATE_FORCE;

    if (ioctl(fb->fd, FBIOPUT_VSCREENINFO, &fb->vi) < 0)
    {
		DPRINTF("%s: FBIOPUT_VSCREENINFO failed!\n",__func__);
    }
}

void extend_display_output_second_attach(unsigned int attach)
{
    #if defined(TCC_LCDC_ATTACH_SET_STATE)
    if (ioctl(fb.fd, TCC_LCDC_ATTACH_SET_STATE, &attach) < 0)
    {
		DPRINTF("%s: FBIOPUT_VSCREENINFO failed!\n",__func__);
    }
    #endif
}

int extend_display_SuspendCheck(unsigned int OutMode)
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



int extend_display_check(extend_display_mode *OutMode)
{
	char value[PROPERTY_VALUE_MAX];
	unsigned int OutputMode = 0, uiLcdcCheck = 0, ifixed = 0, uiSuspendCheck = 0;
	int run_check = 1;
	tcc_display_resize OutputResizeMode;
	unsigned int cec_mode;
	extend_display_mode check_output;
	unsigned int output_second_attach;

	// Initialize check_output
	memset(&check_output, 0, sizeof(check_output));

	// Read UI setting value
	memset(value, NULL, PROPERTY_VALUE_MAX);

	OutputResizeMode.resize_up = 0;
	OutputResizeMode.resize_down = 0;
	OutputResizeMode.resize_left = 0;
	OutputResizeMode.resize_right = 0;

	property_get("tcc_sys_output_second_attach", value, "");

	if(atoi(value))
		output_second_attach = 1;
	else
		output_second_attach = 0;

	if(output_second_attach != prev_output_second_attach)
	{
                printf("output_second_attach=%d -> %d, diff(%d)\r\n", prev_output_second_attach, output_second_attach, (output_second_attach != prev_output_second_attach));
                prev_output_second_attach = output_second_attach;

                extend_display_output_second_attach(output_second_attach);
	}

	property_get("persist.sys.extenddisplay_reset", value, "");
	if(atoi(value))
	{
		property_set("persist.sys.extenddisplay_reset", "0");
		return 0;
	}
	property_get("persist.sys.output_mode", value, "");

	if(output_lcd_cvbs_attach_dual)
		OutputMode = OUTPUT_NONE;
	else
		OutputMode = atoi(value);

	#ifdef HAVE_HDMI_OUTPUT
	if ((dispman_daemon_stbmode && ((OutputMode == OUTPUT_HDMI) || (OutputMode == OUTPUT_NONE))) ||
		(!dispman_daemon_stbmode && (OutputMode == OUTPUT_HDMI)))
	{
		memset(value, NULL, PROPERTY_VALUE_MAX);

		check_output.SetMode = HDMI_GetVideoResolution();

		#if 0
			property_get("persist.sys.hdmi_auto_select", value, "");
		#else
			property_get("persist.sys.hdmi_mode", value, "");
		#endif
		check_output.SubMode = atoi(value);

		uiLcdcCheck = hdmi_lcdc_check();

		property_get("persist.sys.hdmi_resize_up", value, "");
		OutputResizeMode.resize_up = atoi(value);
		property_get("persist.sys.hdmi_resize_dn", value, "");
		OutputResizeMode.resize_down = atoi(value);
		property_get("persist.sys.hdmi_resize_lt", value, "");
		OutputResizeMode.resize_left = atoi(value);
		property_get("persist.sys.hdmi_resize_rt", value, "");
		OutputResizeMode.resize_right = atoi(value);

		if (dispman_daemon_stbmode) {
			property_get("persist.sys.hdmi_cec", value, "");
			cec_mode = atoi(value);
			value[0] = '0';
			check_output.CECMode = cec_mode;
			if( (CurrentMode.OutMode != OutputMode) || (CurrentMode.CECMode != cec_mode))
			{
				memset(value, NULL, PROPERTY_VALUE_MAX);
				value[0] = '1'; // HDMI
				property_set("persist.sys.output_mode", value);

				if(cec_mode == 0) {
					check_output.CECMode = 0;
				}else if(cec_mode == 1){
					check_output.CECMode = 1;
				}
			}
		}

		{
			if (dispman_daemon_stbmode) {
				check_output.AspectRatio = hdmi_get_PixelAspectRatio();
				check_output.ColorDepth = hdmi_get_ColorDepth();
				check_output.ColorSpace = hdmi_get_ColorSpace();
				check_output.HDCP1xEnable = hdmi_get_HDCPEnableStatus();
			} else {
				check_output.AspectRatio = 0;
				check_output.ColorDepth = 0;
				check_output.ColorSpace = 0;
				check_output.HDCP1xEnable = 0;
			}
		}

		check_output.OutMode = OUTPUT_HDMI;

	}
	else
	#endif//
	#ifdef TCC_OUTPUT_COMPOSITE
	if(OutputMode == OUTPUT_COMPOSITE)
	{
		check_output.OutMode = OUTPUT_COMPOSITE;

		property_get("persist.sys.composite_mode", value, "");
		check_output.SetMode = atoi(value);
		check_output.SubMode = 0;

		uiLcdcCheck = composite_lcdc_check();

		property_get("persist.sys.composite_resize_up", value, "");
		OutputResizeMode.resize_up = atoi(value);
		property_get("persist.sys.composite_resize_dn", value, "");
		OutputResizeMode.resize_down = atoi(value);
		property_get("persist.sys.composite_resize_lt", value, "");
		OutputResizeMode.resize_left = atoi(value);
		property_get("persist.sys.composite_resize_rt", value, "");
		OutputResizeMode.resize_right = atoi(value);
	}
	else
	#endif//
	#ifdef TCC_OUTPUT_COMPONENT
	if(OutputMode == OUTPUT_COMPONENT)
	{
		check_output.OutMode = OUTPUT_COMPONENT;

		property_get("persist.sys.component_mode", value, "");
		check_output.SetMode = atoi(value);
		check_output.SubMode = 0;

		uiLcdcCheck = component_lcdc_check ();

		property_get("persist.sys.component_resize_up", value, "");
		OutputResizeMode.resize_up = atoi(value);
		property_get("persist.sys.component_resize_dn", value, "");
		OutputResizeMode.resize_down = atoi(value);
		property_get("persist.sys.component_resize_lt", value, "");
		OutputResizeMode.resize_left = atoi(value);
		property_get("persist.sys.component_resize_rt", value, "");
		OutputResizeMode.resize_right = atoi(value);
	}
	else
	#endif//
	{
		memset(&check_output, 0, sizeof(check_output));
	}

	if( (OutputMode != OUTPUT_NONE) &&
		(uiOutputResizeMode_up != OutputResizeMode.resize_up || uiOutputResizeMode_down != OutputResizeMode.resize_down ||
		uiOutputResizeMode_left != OutputResizeMode.resize_left || uiOutputResizeMode_right != OutputResizeMode.resize_right) )
	{
		DPRINTF("[Previous] OutputResizeMode_up = %d, OutputResizeMode_down = %d, OutputResizeMode_left = %d, OutputResizeMode_right = %d [ N e w  ] OutputResizeMode_up = %d, OutputResizeMode_down = %d, OutputResizeMode_left = %d, OutputResizeMode_right = %d",
				uiOutputResizeMode_up, uiOutputResizeMode_down, uiOutputResizeMode_left, uiOutputResizeMode_right, OutputResizeMode.resize_up, OutputResizeMode.resize_down, OutputResizeMode.resize_left, OutputResizeMode.resize_right);

		if(ioctl(fb.fd, TCC_LCDC_SET_OUTPUT_RESIZE_MODE, &OutputResizeMode))
		{
			DPRINTF("%s: TCC_LCDC_SET_OUTPUT_OFFSET failed!\n",__func__);
			return 0;
		}

		//if(output_first_detection_flag)
		//	fb_update(&fb);

		uiOutputResizeMode_up = OutputResizeMode.resize_up;
		uiOutputResizeMode_down = OutputResizeMode.resize_down;
		uiOutputResizeMode_left = OutputResizeMode.resize_left;
		uiOutputResizeMode_right = OutputResizeMode.resize_right;
	}

	//DPRINTF("~ %s %d Output:OUT:%d Set:%d suspend:%d ~ \n",__func__, run_check, check_output.OutMode, check_output.SetMode, uiSuspend);

	if(output_lcd_cvbs_attach_dual)
		check_output.OutMode = OUTPUT_NONE;

	memcpy(OutMode, &check_output, sizeof(check_output));



	uiSuspendCheck = extend_display_SuspendCheck(OutMode->OutMode);

	if(check_output.OutMode && (!uiLcdcCheck) && (!uiSuspendCheck) || output_lcd_cvbs_attach_dual)
		run_check = 1;
	else
		run_check = 0;


	return run_check;
}

void extend_display_device_init()
{
	char value[PROPERTY_VALUE_MAX];

	property_get("tcc.sys.output_mode_stb", value, "");
	dispman_daemon_stbmode = atoi(value);
	#ifdef HAVE_HDMI_OUTPUT
	hdmi_display_init(1);
	#endif//

	#ifdef TCC_OUTPUT_COMPOSITE
	composite_display_init();
	#endif//

	#ifdef TCC_OUTPUT_COMPONENT
	component_display_init();
	#endif//

	fb_open(&fb);

}

void extend_display_device_deinit()
{
	#ifdef HAVE_HDMI_OUTPUT
	hdmi_display_deinit();
	#endif//

	#ifdef TCC_OUTPUT_COMPOSITE
	composite_display_deinit();
	#endif//

	#ifdef TCC_OUTPUT_COMPONENT
	component_display_deinit();
	#endif//

	close(fb.fd);
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
	char lcdc_composite;
	char lcdc_component;
	char value[PROPERTY_VALUE_MAX];
	int result;

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
	property_get("tcc.hwc.use.cvbs.presentation", value, "0");
	stb_use_cvbs_presentation = atoi(value);

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

			memset(value, NULL, PROPERTY_VALUE_MAX);

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
				if(dispman_daemon_stbmode &&
					(output_attach_dual || output_attach_dual_auto || output_attach_hdmi_cvbs))
				{
					hdmi_display_output_set(onoff);

					#if defined(TCC_OUTPUT_COMPOSITE)
					composite_display_detect_onoff(1);
					composite_display_output_attach(onoff, lcdc_composite);
					composite_display_detect_onoff(0);
					#endif
					result = 0;
				}
 				else
					result = hdmi_display_output_set(onoff);
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

					composite_display_detect_onoff(1);
					composite_display_output_attach(onoff, lcdc_composite);
					composite_display_detect_onoff(0);
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

int extend_display_VideoFormatCtrl(void)
{
	unsigned int HdmiVideoFormat, Structure_3D, hdmi_mode;
	char value[PROPERTY_VALUE_MAX];

	memset(value, NULL, PROPERTY_VALUE_MAX);

	if(CurrentMode.OutMode == OUTPUT_HDMI)
	{
	    property_get("persist.sys.hdmi_mode", value, "");
	    hdmi_mode = atoi(value);

	    if(hdmi_mode == 0)
		{
			property_get("tcc.output.hdmi.video.format", value, "");
			HdmiVideoFormat = atoi(value);
			property_get("tcc.output.hdmi.structure.3d", value, "");
			Structure_3D = atoi(value);

			#ifdef HAVE_HDMI_OUTPUT
			if((gHdmiVideoFormat != HdmiVideoFormat) || ( gStructure_3D != Structure_3D))
				hdmi_set_video_format_ctrl(HdmiVideoFormat, Structure_3D);
				property_set("tcc.output.hdmi_3d_enable","1");
			#endif

			gHdmiVideoFormat = HdmiVideoFormat;
			gStructure_3D = Structure_3D;
	    }
	}

	return 0;
}

int extend_display_CableDetect(unsigned int OutMode)
{
	switch(OutMode)
	{
		#ifdef HAVE_HDMI_OUTPUT
		case OUTPUT_HDMI:
			return hdmi_display_cabledetect();
		#endif//

		#ifdef TCC_OUTPUT_COMPOSITE
		case OUTPUT_COMPOSITE:
			return composite_display_cabledetect();
		#endif//

		#ifdef TCC_OUTPUT_COMPONENT
		case OUTPUT_COMPONENT:
			return component_display_cabledetect();
		#endif//
		default:
			break;
	}
	return 0;
}

int extend_display_AutoDetect(void)
{
	extend_display_mode OutputMode;
	char value[PROPERTY_VALUE_MAX];
	int iOutput;

	/* check the cable detection of current output mode */
	if(extend_display_CableDetect(CurrentMode.OutMode))
		return 1;

	/* release the cable detection of currnet output mode */
	extend_display_detect_OnOff(0, CurrentMode.OutMode);

	/* check the cable detection */
	for(iOutput=OUTPUT_HDMI; iOutput<OUTPUT_MAX; iOutput++)
	{
		if (dispman_daemon_stbmode) {
			if(output_attach_dual_auto && (iOutput == OUTPUT_COMPOSITE))
				continue;
		}

		extend_display_detect_OnOff(1, iOutput);

		if(extend_display_CableDetect(iOutput))
		{
			TPRINTF("Old OUTPUT:%d, New OUTPUT:%d Auto-Detection!!\n", CurrentMode.OutMode, iOutput);

			if(iOutput != CurrentMode.OutMode)
			{
				memset(value, NULL, PROPERTY_VALUE_MAX);
				value[0] = '0' + iOutput;
				property_set("persist.sys.output_mode", value);
			}

			break;
		}

		extend_display_detect_OnOff(0, iOutput);
	}

	if(iOutput == OUTPUT_MAX)
	{
		/* open the cable detection of currnet output mode */
		extend_display_detect_OnOff(1, CurrentMode.OutMode);
	}
	else
	{
		/* set the new output mode */
		extend_display_check(&OutputMode);
		CurrentMode = OutputMode;
		return 1;
	}

	return 0;
}

int extend_display_CheckDetect(extend_display_mode *OutputMode)
{
	int cable_detect = 0;

	if(output_auto_detection || output_attach_dual_auto || output_attach_hdmi_cvbs)
	{
		if(output_auto_detection_flag)
		{
			cable_detect = extend_display_AutoDetect();

			if(cable_detect)
				extend_display_check(OutputMode);
		}
		else
		{
			cable_detect = extend_display_CableDetect(CurrentMode.OutMode);
		}
	}
	else
	{
		cable_detect = extend_display_CableDetect(CurrentMode.OutMode);
	}

	return cable_detect;
}

int extend_display_InitFirstOutput(void)
{
	int cable_detect = 0;

	/* reset component output after displaying dual output in bootloader/kernel */
	if(output_first_detection_flag == 0)
	{
		if(CurrentMode.OutMode == OUTPUT_HDMI)
		{
			cable_detect = extend_display_CableDetect(CurrentMode.OutMode);

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
	char value[PROPERTY_VALUE_MAX];

	memset(value, NULL, PROPERTY_VALUE_MAX);

	property_get("persist.sys.composite_mode", value, "");

	if( (output_attach_hdmi_cvbs && OutputAttachMode.SetMode != atoi(value) && CurrentMode.OutMode == OUTPUT_HDMI) ||
		(output_attach_dual_auto && OutputAttachMode.SetMode != atoi(value)) )
	{
		extend_display_detect_OnOff(1, OUTPUT_COMPOSITE);
		extend_display_output_set(OUTPUT_COMPOSITE, 1);
		extend_display_detect_OnOff(0, OUTPUT_COMPOSITE);

		OutputAttachMode.SetMode = atoi(value);
		running = 0;

		return 1;
	}

	return 0;
}

int extend_display_SetDisplayMode(void)
{
	char value[PROPERTY_VALUE_MAX];

	memset(value, NULL, PROPERTY_VALUE_MAX);

	/* get display type - 0: HDMI/CVBS/COMPONENT, 1: HDMI/CVBS, 2: HDMI */
	if(ioctl(fb.fd, TCC_LCDC_GET_DISPLAY_TYPE, &output_display_type) >= 0)
	{
		/* set property for display type */
		value[0] = '0' + output_display_type;
		property_set("tcc.output.display.type", value);
	}

	output_attach_dual = 0;
	output_attach_dual_auto = 0;
	output_attach_hdmi_cvbs = 0;

	property_get("persist.sys.display.mode", value, "");
	output_display_mode = atoi(value);

	if(output_display_mode == 1)
		output_auto_detection = 1;
	else if(output_display_mode == 2)
		output_attach_hdmi_cvbs = 1;
	else if(output_display_mode == 3)
		output_attach_dual_auto = 1;

	if(output_attach_dual || output_attach_dual_auto)
	{
		OutputAttachMode.OutMode = OUTPUT_COMPOSITE;
		OutputAttachMode.SetMode = 0;
		OutputAttachMode.SubMode = 0;
		OutputAttachMode.CECMode = 0;
		OutputAttachMode.AspectRatio = 0;
		OutputAttachMode.ColorDepth = 0;
		OutputAttachMode.ColorSpace = 0;

		output_auto_detection_flag = 1;
	}
	else if(output_attach_hdmi_cvbs)
	{
		property_get("persist.sys.composite_mode", value, "");

		OutputAttachMode.OutMode = OUTPUT_COMPOSITE;
		OutputAttachMode.SetMode = atoi(value);
		OutputAttachMode.SubMode = 0;
		OutputAttachMode.CECMode = 0;
		OutputAttachMode.AspectRatio = 0;
		OutputAttachMode.ColorDepth = 0;
		OutputAttachMode.ColorSpace = 0;

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
	int fts_fd, res;
	char data[512];
	char value[PROPERTY_VALUE_MAX];

	memset(value, NULL, PROPERTY_VALUE_MAX);

	output_resolution[0] = 'T';
	output_resolution[1] = 'C';
	output_resolution[2] = 'C';
	output_resolution[3] = CurrentMode.OutMode;
	output_resolution[4] = CurrentMode.SetMode;
	property_get("persist.sys.hdmi_resolution", value, "");
	output_resolution[5] = atoi(value);
	if(output_resolution[5] == 125)
	{
		property_get("persist.sys.hdmi_detected_res", value, "");
		output_resolution[5] = atoi(value);
	}
	property_get("persist.sys.composite_mode", value, "");
	output_resolution[6] = atoi(value);
	property_get("persist.sys.component_mode", value, "");
	output_resolution[7] = atoi(value);
	output_resolution[8] = 0;//outputmode_1st;
	output_resolution[9] = 0;//outputmode_2nd;
	property_get("persist.sys.hdmi_mode", value, "");
	output_resolution[10] = atoi(value);

	memset(data, 0x00, sizeof(data));
	memcpy(data, output_resolution, sizeof(output_resolution));

	fts_fd = open(FTS_DEVICE, O_RDWR);
	if (fts_fd < 0)
		return -1;

	if (ioctl(fts_fd, OUTPUT_SETTING_SET, data))
		res = -1;
	else
		res = 0;

	close(fts_fd);

	return 0;
}

char extend_display_runing(extend_display_mode OutPutMode)
{
	if (dispman_daemon_stbmode) {
		if(output_attach_dual_auto && (OutPutMode.OutMode == OUTPUT_COMPOSITE))
			return 1;
		else
		{
			if((OutPutMode.OutMode == CurrentMode.OutMode))
				return 1;
			else
				return 0;
		}
	} else {
		if((OutPutMode.OutMode == CurrentMode.OutMode) || output_lcd_cvbs_attach_dual)
			return 1;
		else
			return 0;
	}
}

char lcd_cvbs_CheckDetect()
{
        #if defined(TCC_OUTPUT_COMPOSITE)
	char value[PROPERTY_VALUE_MAX];
	unsigned int lcd_cvbs_check;
	static int before_cvbs_mode = 0;

	property_get("ro.system.cvbs_active", value, ""); //CVBS active check

	if(strcmp(value,"true") == 0)
	{
		property_get("persist.sys.cvbs_power_mode", value, ""); //CVBS power check
		lcd_cvbs_check = atoi(value);

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


static unsigned int switch_report_delay = 100;	//10 second...
static void* extend_display_thread(void* arg)
{
	int cable_detect = 0;
	int hdmi_run_status = 0;
	extend_display_mode OutputMode;
	char value[PROPERTY_VALUE_MAX];

	#if defined(NO_DEAMON)
	// SIG ------------------------------
	sigset_t signal_set;
	sigemptyset(&signal_set);
	sigaddset(&signal_set, SIGINT);				// CTRL+C
	sigaddset(&signal_set, SIGUSR2);			// KILL
	pthread_sigmask(SIG_UNBLOCK, &signal_set, 0);	// BLOCK CTRL+C/KILL
	signal(SIGINT, sighandler);
	signal(SIGUSR2, sighandler);
	// ----------------------------------
	#endif

	memset(value, NULL, PROPERTY_VALUE_MAX);
#ifdef HAVE_HDMI_OUTPUT
	hdmi_set_output_detected(false);
#endif
	running = extend_display_check(&OutputMode);
	CurrentMode = OutputMode;

	TPRINTF("%s Set_Mode:%d Cur_Mode:%d cable:%d running:%d ~~ \n",
				__func__, CurrentMode.OutMode, OutputMode.OutMode, cable_detect, running);

	if (dispman_daemon_stbmode) {
		extend_display_SetDisplayMode();
	}
	lcd_cvbs_CheckDetect();

	while(1)
	{
		// open to cable detect
		extend_display_check(&OutputMode);

		while(!running) {
			usleep(100000);
			running = extend_display_check(&OutputMode);
		}
                extend_display_detect_OnOff(1, OutputMode.OutMode);

		cable_detect = 0;
		CurrentMode = OutputMode;

		TPRINTF("open to cable detect CurrentMode:%d OutputMode:%d cable:%d \n", CurrentMode.OutMode, OutputMode.OutMode, cable_detect);

		if (dispman_daemon_stbmode) {
			extend_display_InitFirstOutput();
			extendthreadready = 1;
		}

		while(running && extend_display_runing(OutputMode))
		{
			TPRINTF("extend display start Mode:%d Mode:%d cable:%d \n", CurrentMode.OutMode, OutputMode.OutMode, cable_detect);

			while((!cable_detect) && extend_display_runing(OutputMode) && running)
			{
				usleep(100000);
				running = extend_display_check(&OutputMode);

				if (dispman_daemon_stbmode) {
					cable_detect = extend_display_CheckDetect(&OutputMode);
				} else {
					if(lcd_cvbs_CheckDetect()){
						cable_detect = extend_display_CableDetect(OUTPUT_COMPOSITE);
						CurrentMode.OutMode = OUTPUT_NONE;
					}
					else
						cable_detect = extend_display_CableDetect(CurrentMode.OutMode);
				}

				if(switch_report_delay != 0) // For MID
					switch_report_delay--;
			}

			CurrentMode.SetMode = OutputMode.SetMode;
			CurrentMode.SubMode = OutputMode.SubMode;
			//CurrentMode.CECMode = OutputMode.CECMode;

			if(output_first_detection_flag == 0)
				output_first_detection_flag = 1;

			TPRINTF("run start Mode:%d Mode:%d cable:%d\n", CurrentMode.OutMode, OutputMode.OutMode, cable_detect);

			if(running && extend_display_runing(OutputMode))
			{
				DPRINTF("output_setting Mode:%d Mode:%d cable:%d\n", CurrentMode.OutMode, OutputMode.OutMode, cable_detect);

				if(output_lcd_cvbs_attach_dual)
					extend_display_output_set(OUTPUT_COMPOSITE, 1);
				else
					extend_display_output_set(CurrentMode.OutMode, 1);

				if (dispman_daemon_stbmode) {
					//extend_display_SaveData(output_resolution, 11);
				}
				#if defined(TCC_OUTPUT_COMPOSITE)
				if(stb_use_cvbs_presentation && CurrentMode.OutMode == OUTPUT_HDMI && !output_lcd_cvbs_attach_dual) {
					composite_display_detect_onoff(1);
					extend_display_output_set(OUTPUT_COMPOSITE, 1);
				}
				#endif
			}

			TPRINTF("ready plug out Mode:%d Mode:%d cable:%d\n", CurrentMode.OutMode, OutputMode.OutMode, cable_detect);

			while(running && (cable_detect) && extend_display_runing(OutputMode))
			{
				usleep(100000);
				running = extend_display_check(&OutputMode);

				if(lcd_cvbs_CheckDetect())
					cable_detect = extend_display_CableDetect(OUTPUT_COMPOSITE);
				else
					cable_detect = extend_display_CableDetect(CurrentMode.OutMode);

				extend_display_AudioInputPortDetect(CurrentMode.OutMode);
				hdmi_run_status = HDMIGetRunStatus();

				if(switch_report_delay != 0) {
					switch_report_delay--;
				} else {
					HDMISendHPDStatus();
                                        #if defined(TCC_OUTPUT_COMPOSITE)
					composite_send_hpd_status();
                                        #endif
				}

				if (!dispman_daemon_stbmode ||
					(dispman_daemon_stbmode && !extend_display_SetAttachOutput()))
				{
					if( (CurrentMode.OutMode != OutputMode.OutMode) ||
					    (CurrentMode.SubMode != OutputMode.SubMode) ||
					    (CurrentMode.SetMode != OutputMode.SetMode) ||
					    (CurrentMode.CECMode != OutputMode.CECMode) ||
					    (CurrentMode.AspectRatio != OutputMode.AspectRatio) ||
					    (CurrentMode.ColorDepth != OutputMode.ColorDepth) ||
					    (CurrentMode.ColorSpace != OutputMode.ColorSpace) ||
					    (CurrentMode.HDCP1xEnable != OutputMode.HDCP1xEnable))
					{
						running = 0;
					}

					if( !cable_detect )
						running = 0;
					if( !hdmi_run_status )
						running = 0;
				}

				#ifdef HAVE_HDMI_OUTPUT
				if (dispman_daemon_stbmode) {
					extend_display_VideoFormatCtrl();
					hdmi_cmd_process();
				}
				if(running && CurrentMode.OutMode == OUTPUT_HDMI) {
					hdmi_api_demo();	
                                }
				//hdmi_AudioOnOffChk();
				CECCmdProcess();
				#endif
			}

			TPRINTF("FINISH option run:%d cable:%d output%d~~~~~~~~~~\n", running , cable_detect , extend_display_runing(OutputMode));

			extend_display_output_set(CurrentMode.OutMode, 0);

			#if defined(TCC_OUTPUT_COMPOSITE)
			if(stb_use_cvbs_presentation && CurrentMode.OutMode == OUTPUT_HDMI && !output_lcd_cvbs_attach_dual) {
				extend_display_output_set(OUTPUT_COMPOSITE, 0);
				composite_display_detect_onoff(0);
			}
			#endif
			if(dispman_daemon_stbmode && (output_auto_detection_flag == 0))
				output_auto_detection_flag = 1;
		}

		// release to cable detect
		extend_display_detect_OnOff(0, CurrentMode.OutMode);

		usleep(500000);
	}

	TPRINTF("~~~~~~ %s Mode:%d Mode:%d cable:%d FINISH~~~~~~~~~~\n", __func__, CurrentMode.OutMode, OutputMode.OutMode, cable_detect);
	return 0;
}

//Temp
static unsigned int extend_display_init_flag = 0;

int extend_display_init()
{
        #if defined(SUPPORT_SAFE_LINK)
        printf("\r\nDisplay Daemon for SafeLink\r\n\r\n");
        #else
	printf("\r\nDisplay Daemon Init\r\n\r\n");
        #endif
#if defined(HAVE_HDMI_OUTPUT) || defined(TCC_OUTPUT_COMPONENT) || defined(TCC_OUTPUT_COMPOSITE)
//	signal(SIGINT, sighandler);

//Temp
	if(extend_display_init_flag == 1)
		return 0;

	extend_display_init_flag = 1;

	extend_display_device_init();

	if (dispman_daemon_stbmode)
		extendthreadready = 0;
	property_set("tcc.sys.output_mode_detected", "0");

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
	running = 0;
	#if defined(HAVE_HDMI_OUTPUT) || defined(TCC_OUTPUT_COMPONENT) || defined(TCC_OUTPUT_COMPOSITE)
	extend_display_device_deinit();
	#endif//
	TPRINTF("~~~~~~~ %s finish", __func__);
    return 0;
}


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

int main()
{
	int retry = 3;

	#if defined(NO_DEAMON)
	// SIG ------------------------------
	sigset_t signal_set;
	#endif


	for(;retry;retry--) {
		if(display_daemon_lock()) {
			break;
		}
		printf("display_daemon is duplicate.. finish \r\n");
		usleep(500000);
	}

	#if defined(NO_DEAMON)
	// SIG ------------------------------
	sigemptyset(&signal_set);
	sigaddset(&signal_set, SIGINT);				// CTRL+C
	sigaddset(&signal_set, SIGUSR2);			// KILL
	pthread_sigmask(SIG_BLOCK, &signal_set, 0);	// BLOCK CTRL+C/KILL
	// ----------------------------------

	#else

	Daemonize();
	#endif


	extend_display_init();

	#if 0
	//test --
	{
		int dataindex=0;
		int ret;
		char data[SR_ATTR_MAX];
		int index, width, height, hz;
		char interlaced;
		dataindex = sprintf(data, "%02d:%04dx%04d@%02d%c ", 1, 1920, 1080, 60, 'P');
		dataindex += sprintf(data+dataindex, "%02d:%04dx%04d@%02d%c ", 2, 1920, 1080, 60, 'I');
		dataindex += sprintf(data+dataindex, "%02d:%04dx%04d@%02d%c ", 3, 1280, 720, 60, 'P');
		dataindex += sprintf(data+dataindex, "%02d:%04dx%04d@%02d%c ", 4, 1280, 720, 60, 'I');
		dataindex += sprintf(data+dataindex, "%02d:%04dx%04d@%02d%c ", 5, 1280, 720, 50, 'P');
		property_supported_resolution_set(data);

		memset(data, 0, sizeof(data));

		property_supported_resolution_get(data);
		printf("data=%s\r\n", data);

		dataindex = 0;
		do {
			index = width = height = hz = interlaced = 0;
			ret = sscanf(data+dataindex, "%02d:%04dx%04d@%02d%c ", &index, &width, &height,&hz, &interlaced);
			if(ret > 0) {
				dataindex+=17;
				printf("read %02d:%04dx%04d@%02d%c (%d)\r\n", index, width, height, hz, interlaced, ret);
			}
		}while(ret > 0);
	}
	#endif
	//---
	while(!ext_disp_stop) {
		usleep(100000);
	}

	extend_display_deinit();
	display_daemon_unlock();
	return 0;
}

#ifdef __cplusplus
}
#endif
