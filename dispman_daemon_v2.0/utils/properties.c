/*!
* TCC Version 1.0
* Copyright (c) Telechips Inc.
* All rights reserved 
*  \file        properites.c
*  \brief       HDMI android properties emul source
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

#define LOG_TAG "[PROP      ]"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <fcntl.h>
#include <ctype.h>
#include "properties.h"
#include <utils/Log.h>

/*
 * The Linux simulator provides a "system property server" that uses IPC
 * to set/get/list properties.  The file descriptor is shared by all
 * threads in the process, so we use a mutex to ensure that requests
 * from multiple threads don't get interleaved.
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <pthread.h>


//    char sendBuf[1+PROPERTY_KEY_MAX];
//    char recvBuf[1+PROPERTY_VALUE_MAX];

//typedef struct {
//    char key[1+PROPERTY_KEY_MAX];
//    char val[1+PROPERTY_VALUE_MAX];
//} prop_t;

//static prop_t	g_props[PROPERTY_MAX];
//static int		g_props_cnt=0;


//static pthread_once_t gInitOnce = PTHREAD_ONCE_INIT;
static pthread_mutex_t gPropertyFdLock = PTHREAD_MUTEX_INITIALIZER;


struct sysfs_properites_type {
    char key[1+PROPERTY_KEY_MAX];
	char sysfs[1+PROPERTY_KEY_MAX];
};

struct sysfs_properites_type  sysfs_properites[] = {
	 {"tcc_hdmi_pllmode"                     	, "tcc_hdmi_pllmode"},
	 {"tcc.all.hdmi.720p.fixed"                     , "tcc_hdmi_720p_fixed"},
	 {"tcc.audio.hdmi.link"                         , "tcc_audio_hdmi_link"},      
	 {"tcc.audio.sampling_rate"                     , "tcc_audio_sampling_rate"},
	 {"tcc.audio.channels"                          , "tcc_audio_channels"},
	 {"tcc.cec.imageview_on"                        , "tcc_cec_imageview_on"},
	 {"tcc.cec.textview_on"                         , "tcc_cec_textview_on"},
	 {"tcc.cec.active_source"                       , "tcc_cec_active_source"},
	 {"tcc.cec.in_active_source"                    , "tcc_cec_in_active_source"},
	 {"tcc.cec.standby"                             , "tcc_cec_standby"},
	 {"tcc.hdmi.audio_type"                         , "tcc_hdmi_audio_type"},
	 {"tcc.output.display.type"                     , "tcc_output_display_type"},
	 {"tcc.output.hdmi_3d_format"                   , "tcc_output_hdmi_3d_format"},
	 {"tcc.output.hdmi_audio_onoff"                 , "tcc_output_hdmi_audio_onoff"},
	 {"tcc.output.hdmi_audio_disable"               , "tcc_output_hdmi_audio_disable"},
	 {"tcc.output.hdmi.video.format"                , "tcc_output_hdmi_video_format"},
	 {"tcc.output.hdmi.structure.3d"                , "tcc_output_hdmi_structure_3d"},
 	 {"tcc.hdmi.supported.audio"                    , "tcc_output_hdmi_supported_audio"},
	 {"tcc.hdmi.supported_resolution"               , "tcc_output_hdmi_supported_resolution"},
	 {"tcc.hdmi.supported_3d_mode"                  , "tcc_output_hdmi_supported_3d_mode"},
	 {"tcc.hdmi.supported_hdr"                      , "tcc_output_hdmi_supported_hdr"},
	 {"tcc.video.hdmi_resolution"                   , "tcc_video_hdmi_resolution"},
	 {"tcc.sys.output_mode_detected"                , "tcc_output_mode_detected"},
	 {"tcc.sys.output_dispdev_width"                , "tcc_output_dispdev_width"},
	 {"tcc.sys.output_dispdev_height"               , "tcc_output_dispdev_height"},
	 {"tcc.sys.output_panel_width"                	, "tcc_output_panel_width"},
	 {"tcc.sys.output_panel_height"               	, "tcc_output_panel_height"},	 
	 {"tcc.sys.output_mode_plugout"                 , "tcc_output_mode_plugout"},
	 {"tcc.2d.compression"                          , "tcc_2d_compression"},
	 // STB CONTROL
	 {"tcc.sys.output_mode_stb"                     , "tcc_output_mode_stb"},
         {"tcc.sys.output_underrun"                     , "tcc_sys_output_underrun"},


	 {"persist.sys.hdcp1x_enable"                   , "persist_hdcp1x_enable"},
	 {"persist.sys.display.mode"                    , "persist_display_mode"},
	 {"persist.sys.extenddisplay_reset"             , "persist_extenddisplay_reset"},
	 {"persist.sys.output_attach_main"              , "persist_output_attach_main"},
	 {"persist.sys.output_attach_sub"               , "persist_output_attach_sub"},
	 {"persist.sys.output_mode"                     , "persist_output_mode"},
	 {"persist.sys.auto_resolution"                 , "persist_auto_resolution"},
	 {"persist.sys.spdif_setting"                   , "persist_spdif_setting"},
	 {"persist.sys.hdmi_mode"                       , "persist_hdmi_mode"},
	 {"persist.sys.hdmi_resize_up"                  , "persist_hdmi_resize_up"},
	 {"persist.sys.hdmi_resize_dn"                  , "persist_hdmi_resize_dn"},
	 {"persist.sys.hdmi_resize_lt"                  , "persist_hdmi_resize_lt"},
	 {"persist.sys.hdmi_resize_rt"                  , "persist_hdmi_resize_rt"},
	 {"persist.sys.hdmi_cec"                        , "persist_hdmi_cec"},
	 {"persist.sys.hdmi_resolution"                 , "persist_hdmi_resolution"},
	 {"persist.sys.hdmi_detected_res"               , "persist_hdmi_detected_res"},
	 {"persist.sys.hdmi_auto_select"                , "persist_hdmi_auto_select"},
	 {"persist.sys.hdmi_color_depth"                , "persist_hdmi_color_depth"},
	 {"persist.sys.hdmi_color_space"                , "persist_hdmi_color_space"},
	 {"persist.sys.hdmi_colorimetry"                , "persist_hdmi_colorimetry"},
	 {"persist.sys.hdmi_aspect_ratio"               , "persist_hdmi_aspect_ratio"},
	 {"persist.sys.hdmi_detected"                   , "persist_hdmi_detected"},
	 {"persist.sys.hdmi_detected_mode"              , "persist_hdmi_detected_mode"},
	 {"persist.sys.hdmi_printlog"                   , "persist_hdmi_printlog"},
	 {"persist.sys.hdmi_extra_mode"                 , "persist_hdmi_extra_mode"},
	 {"persist.sys.composite_mode"                  , "persist_composite_mode"},
	 {"persist.sys.composite_resize_up"             , "persist_composite_resize_up"},
	 {"persist.sys.composite_resize_dn"             , "persist_composite_resize_dn"},
	 {"persist.sys.composite_resize_lt"             , "persist_composite_resize_lt"},
	 {"persist.sys.composite_resize_rt"             , "persist_composite_resize_rt"},
	 {"persist.sys.composite_detected"		, "persist_composite_detected"},
	 {"persist.sys.component_mode"                  , "persist_component_mode"},
	 {"persist.sys.component_detected_mode"         , "persist_component_detected_mode"},
	 {"persist.sys.component_resize_up"             , "persist_component_resize_up"},
	 {"persist.sys.component_resize_dn"             , "persist_component_resize_dn"},
	 {"persist.sys.component_resize_lt"             , "persist_component_resize_lt"},   
	 {"persist.sys.component_resize_rt"             , "persist_component_resize_rt"},
	 {"persist.sys.component_detected"		, "persist_component_detected"},
	 {"persist.sys.supported_resolution_count"      , "persist_supported_resolution_count"},
	 {"color_enhance_lcd_hue"                       , "color_enhance_lcd_hue"},
	 {"color_enhance_lcd_brightness"                , "color_enhance_lcd_brightness"},
	 {"color_enhance_lcd_contrast"                  , "color_enhance_lcd_contrast"},
	 {"color_enhance_hdmi_hue"                      , "color_enhance_hdmi_hue"},
	 {"color_enhance_hdmi_brightness"               , "color_enhance_hdmi_brightness"},
	 {"color_enhance_hdmi_contrast"                 , "color_enhance_hdmi_contrast"},
	 {"color_enhance_composite_hue"                 , "color_enhance_composite_hue"},
	 {"color_enhance_composite_brightness"          , "color_enhance_composite_brightness"},
	 {"color_enhance_composite_contrast"            , "color_enhance_composite_contrast"},
	 {"color_enhance_component_hue"                 , "color_enhance_component_hue"},
	 {"color_enhance_component_brightness"          , "color_enhance_component_brightness"},
	 {"color_enhance_component_contrast"            , "color_enhance_component_contrast"},
	 {"persist.hdmi.native.first"                   , "persist_hdmi_native_first"},
	 {"persist.hdmi.hw.cts"                         , "persist_hdmi_hw_cts"},
	 {"persist.sys.hdmi_refresh_rate"               , "persist_hdmi_refresh_mode"}, 
	 {"tcc.cec.connection"                          , "tcc_cec_connection"},
	 {"tcc.cec.menu_status"                         , "tcc_cec_menu_status"},
	 {"tcc.cec.request_tv_status"                   , "tcc_cec_request_tv_status"},
	 {"tcc.cec.tv_status"                           , "tcc_cec_tv_status"},
	 
	 {"tcc.hdcp.hdmi.enable"						, "tcc_hdcp_hdmi_enable"}, //hdcp
};

#define PROPERTIES_COUNT (sizeof(sysfs_properites) / sizeof(struct sysfs_properites_type))

#define DISPMAN_SYSFS "/sys/class/tcc_dispman/tcc_dispman"
#define VOUT_RDMA_SYFS "/sys/devices/platform/tcc-vout-video.10/vioc_rdma"
#define VSYNC_SYSFS    "/sys/class/misc/tcc_vsync0"

static int sendtosysfs(const char *key, const char *val)
{
	int index;
	char sysfspath[512] = {0};

	for(index=0;index < PROPERTIES_COUNT; index++)
	{
		if(!strcmp(sysfs_properites[index].key, key)) {
			sprintf(sysfspath, "%s/%s", DISPMAN_SYSFS, sysfs_properites[index].sysfs);
			break;
		}
	}

	if(index < PROPERTIES_COUNT) {
		int sysfs_fd = open(sysfspath, O_RDWR);
		if (sysfs_fd < 0) {
			//ALOGE("%s: failed open %s \n", __func__, sysfspath);
			return -1;
		}

		//ALOGI("write %s to %s (fd=%d)\r\n", sysfspath, val, sysfs_fd);
	  	write (sysfs_fd, val, strlen(val));

	  	close(sysfs_fd);
	}

	return 0;
}

static int getfromsysfs(const char *key, char *val)
{
	int index;
	char sysfspath[512] = {0};

	for(index=0;index < PROPERTIES_COUNT; index++)
	{
		if(!strcmp(sysfs_properites[index].key, key)) {
			sprintf(sysfspath, "%s/%s", DISPMAN_SYSFS, sysfs_properites[index].sysfs);
			break;
		}
	}

	if(index < PROPERTIES_COUNT) {
		int read_count, sysfs_fd;

		sysfs_fd = open(sysfspath, O_RDONLY);
		if (sysfs_fd < 0) {
			//ALOGE("%s: failed open %s \n", __func__, sysfspath);
			return -1;
		}

	  	read_count = read (sysfs_fd, val, PROPERTY_VALUE_MAX);
		val[read_count] = '\0';
		//ALOGI("read %s from %s (read_count=%d)\r\n", sysfspath, val, read_count);

	  	close(sysfs_fd);
	}

	return 0;
}

int property_get(const char *key, char *value, const char *default_value)
{
        int len = -1;
        if (strlen(key) >= PROPERTY_KEY_MAX) 
                goto endif_property_get;

        pthread_mutex_lock(&gPropertyFdLock);

        getfromsysfs(key, value);
        	
        pthread_mutex_unlock(&gPropertyFdLock);
        len = strlen(value);
endif_property_get:
        return len;
}

int property_set(const char *key, const char *value)
{
        int ret = -1;
        if (strlen(key) >= PROPERTY_KEY_MAX) 
                goto endif_property_set;
        if (strlen(value) >= PROPERTY_VALUE_MAX) 
                goto endif_property_set;

        pthread_mutex_lock(&gPropertyFdLock);

        sendtosysfs(key, value);

        pthread_mutex_unlock(&gPropertyFdLock);
        ret = 0;
endif_property_set:
    return ret;
}

int property_get_int(const char *key, const int default_value)
{
        int len;
        int property = default_value;
        char value[PROPERTY_VALUE_MAX];

        memset(value, 0, PROPERTY_VALUE_MAX);
        len = property_get(key, value, "");
        if(len > 0) {
                property = atoi(value);
                if(property < 0) {
                        property = default_value;
                }
        }
        return property;
}

int property_vsync_lock_set(int lock)
{
	int sysfs_fd;
	char sysfspath[512] = {0};
	char *value;

	if (lock)
		value = "1";
	else
		value = "0";

	pthread_mutex_lock(&gPropertyFdLock);
	sprintf(sysfspath, "%s/%s", DISPMAN_SYSFS, "tcc_vsync_lock");
	sysfs_fd = open(sysfspath, O_RDWR);
	write (sysfs_fd, value, strlen(value));
	close(sysfs_fd);
	pthread_mutex_unlock(&gPropertyFdLock);

    return 0;
}

int property_supported_resolution_get(char *value)
{
	int read_count, sysfs_fd;
	char sysfspath[512] = {0};

	pthread_mutex_lock(&gPropertyFdLock);

	sprintf(sysfspath, "%s/%s", DISPMAN_SYSFS, "persist_supported_resolution");

	sysfs_fd = open(sysfspath, O_RDWR);
	if (sysfs_fd < 0) {
		printf("%s:E:open\n", __func__);
		return -1;
	}

  	read_count = read (sysfs_fd, value, PROPERTY_VALUE_MAX);
	value[read_count] = '\0';
  	close(sysfs_fd);
		
        pthread_mutex_unlock(&gPropertyFdLock);
	
        return strlen(value);
}

int property_supported_resolution_set(const char *value)
{
	int sysfs_fd;
	char sysfspath[512] = {0};
	
        if (strlen(value) >= SR_ATTR_MAX) return -1;

        pthread_mutex_lock(&gPropertyFdLock);

	sprintf(sysfspath, "%s/%s", DISPMAN_SYSFS, "persist_supported_resolution");
	sysfs_fd = open(sysfspath, O_RDWR);
	if (sysfs_fd < 0) {
		printf("%s:E:open\n", __func__);
		return -1;
	}

  	write (sysfs_fd, value, strlen(value));
  	close(sysfs_fd);
	
        pthread_mutex_unlock(&gPropertyFdLock);
	
        return 0;
}

int property_supported_resolution_clear()
{
        int sysfs_fd;
        char sysfspath[512] = {0};
        char *value = "unknown";

        if (strlen(value) >= SR_ATTR_MAX) return -1;

    pthread_mutex_lock(&gPropertyFdLock);

        sprintf(sysfspath, "%s/%s", DISPMAN_SYSFS, "persist_supported_resolution");
        sysfs_fd = open(sysfspath, O_RDWR);
        if (sysfs_fd < 0) {
                printf("%s:E:open\n", __func__);
                return -1;
        }

        write (sysfs_fd, value, strlen(value));
        close(sysfs_fd);

        pthread_mutex_unlock(&gPropertyFdLock);

        return 0;
}

int property_supported_edid_drm_info_set(const char *value)
{
	int sysfs_fd;
	char sysfspath[512] = {0};

        if (strlen(value) >= SR_ATTR_MAX) return -1;

        pthread_mutex_lock(&gPropertyFdLock);

        sprintf(sysfspath, "%s/%s", DISPMAN_SYSFS, "persist_hdmi_edid_drm_info");
        sysfs_fd = open(sysfspath, O_RDWR);
        if (sysfs_fd < 0) {
                printf("%s:E:open\n", __func__);
                return -1;
	}

        write (sysfs_fd, value, strlen(value));
        close(sysfs_fd);

        pthread_mutex_unlock(&gPropertyFdLock);

        return 0;
}

int property_vout_set(const char *value)
{
        int sysfs_fd;

        if (strlen(value) >= SR_ATTR_MAX) return -1;

        pthread_mutex_lock(&gPropertyFdLock);

        sysfs_fd = open(VOUT_RDMA_SYFS, O_RDWR);
	if (sysfs_fd < 0) {
		printf("%s:E:open\n", __func__);
		return -1;
	}

  	write (sysfs_fd, value, strlen(value));
  	close(sysfs_fd);

        pthread_mutex_unlock(&gPropertyFdLock);

        return 0;
}

int property_proc_cpuinfo_get(const char* name, char *value)
{
	FILE *fp_proc;
	char *proc_buff;
	char *val_ptr = NULL;

	pthread_mutex_lock(&gPropertyFdLock);

	proc_buff = malloc(512);

	if(proc_buff) {
                fp_proc = fopen( "/proc/cpuinfo", "r");
		if ( fp_proc ) {
			while( fgets( proc_buff, 512, fp_proc)) {
				//ALOGI("ReadLine = %s\r\n", proc_buff);
				if(!strncmp(name, proc_buff, strlen(name))) {
					//ALOGI("find [%s]\r\n", proc_buff);
					val_ptr = strchr(proc_buff, ':');
					val_ptr++;
					break;
				}
			}
			fclose( fp_proc);
		}
		if(val_ptr) {
			while ((isspace(*val_ptr) || isblank(*val_ptr )) && *val_ptr != '\0') val_ptr++;
			strcpy(value, val_ptr);
		}
		free(proc_buff);
	}

    pthread_mutex_unlock(&gPropertyFdLock);

    return strlen(value);
}

static int daemon_lock_fd = 0;
static int daemon_locked = 0;

int display_daemon_lock(void)
{
        int ret = -1;
        
        daemon_lock_fd = open(DISPMAN_SYSFS "/tcc_dispman_lock_file", O_RDWR | O_CREAT, 0666);
        if(daemon_lock_fd > 0) {
                if(lockf(daemon_lock_fd, F_TLOCK, 0) < 0) {
                        daemon_locked = 0;
                } else {
                        daemon_locked = 1;
                }
        }
        return daemon_locked;
}

void display_daemon_unlock(void)
{
        if(daemon_lock_fd > 0) {
                if(daemon_locked) {
                        lockf(daemon_lock_fd, F_ULOCK, 0);
                }

                close(daemon_lock_fd);
        }
        daemon_locked = 0;
        daemon_lock_fd = -1;
}
