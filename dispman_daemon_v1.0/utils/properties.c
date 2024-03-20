/*
 * Copyright (C) 2006 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifdef __cplusplus
extern "C" {
#endif

#define LOG_TAG "properties"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <fcntl.h>
#include "properties.h"


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
	 {"tcc_hdmi_pllmode"                     		, "tcc_hdmi_pllmode"},
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
	 {"tcc.video.hdmi_resolution"                   , "tcc_video_hdmi_resolution"},
	 {"tcc.sys.output_mode_detected"                , "tcc_output_mode_detected"},
	 {"tcc.sys.output_dispdev_width"                , "tcc_output_dispdev_width"},
	 {"tcc.sys.output_dispdev_height"               , "tcc_output_dispdev_height"},
	 {"tcc.sys.output_panel_width"                	, "tcc_output_panel_width"},
	 {"tcc.sys.output_panel_height"               	, "tcc_output_panel_height"},
	 {"tcc.sys.output_mode_plugout"                 , "tcc_output_mode_plugout"},
	 // STB CONTROL
	 {"tcc.sys.output_mode_stb"                     , "tcc_output_mode_stb"},

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
	 {"persist.sys.hdmi_aspect_ratio"               , "persist_hdmi_aspect_ratio"},
	 {"persist.sys.hdmi_detected"                   , "persist_hdmi_detected"},
	 {"persist.sys.hdmi_detected_mode"              , "persist_hdmi_detected_mode"},
	 {"persist.sys.composite_mode"                  , "persist_composite_mode"},
	 {"persist.sys.composite_resize_up"             , "persist_composite_resize_up"},
	 {"persist.sys.composite_resize_dn"             , "persist_composite_resize_dn"},
	 {"persist.sys.composite_resize_lt"             , "persist_composite_resize_lt"},
	 {"persist.sys.composite_resize_rt"             , "persist_composite_resize_rt"},
	 {"persist.sys.composite_detected"				, "persist_composite_detected"},
	 {"persist.sys.component_mode"                  , "persist_component_mode"},
	 {"persist.sys.component_resize_up"             , "persist_component_resize_up"},
	 {"persist.sys.component_resize_dn"             , "persist_component_resize_dn"},
	 {"persist.sys.component_resize_lt"             , "persist_component_resize_lt"},
	 {"persist.sys.component_resize_rt"             , "persist_component_resize_rt"},
	 {"persist.sys.component_detected"				, "persist_component_detected"},
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
         {"tcc_sys_output_second_attach"                , "tcc_sys_output_second_attach"},
         {"tcc_hdmi_acp_demo"                         	, "tcc_hdmi_acp_demo"},
};

#define PROPERTIES_COUNT (sizeof(sysfs_properites) / sizeof(struct sysfs_properites_type))

#define DISPMAN_SYSFS "/sys/class/tcc_dispman/tcc_dispman"
#define VOUT_RDMA_SYFS "/sys/devices/platform/tcc-vout-video.10/vioc_rdma"

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
		//printf("write %s to %s (fd=%d)\r\n", sysfspath, val, sysfs_fd);
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

		int read_count, sysfs_fd = open(sysfspath, O_RDONLY);

	  	read_count = read (sysfs_fd, val, PROPERTY_VALUE_MAX);
		val[read_count] = '\0';
		//printf("read %s from %s (read_count=%d)\r\n", sysfspath, val, read_count);

	  	close(sysfs_fd);
	}

	return 0;
}

int property_get(const char *key, char *value, const char *default_value)
{
    if (strlen(key) >= PROPERTY_KEY_MAX) return -1;

    pthread_mutex_lock(&gPropertyFdLock);

	getfromsysfs(key, value);

    pthread_mutex_unlock(&gPropertyFdLock);

    return strlen(value);
}

int property_set(const char *key, const char *value)
{
    if (strlen(key) >= PROPERTY_KEY_MAX) return -1;
    if (strlen(value) >= PROPERTY_VALUE_MAX) return -1;

    pthread_mutex_lock(&gPropertyFdLock);

	sendtosysfs(key, value);

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
		if ( fp_proc = fopen( "/proc/cpuinfo", "r")) {
			while( fgets( proc_buff, 512, fp_proc)) {
				//ALOGD("ReadLine = %s\r\n", proc_buff);
				if(!strncmp(name, proc_buff, strlen(name))) {
					//ALOGD("find [%s]\r\n", proc_buff);
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

static int daemon_lock_fd, daemon_locked;

int display_daemon_lock(void)
{
        int ret = -1;

        daemon_lock_fd = open(DISPMAN_SYSFS "/tcc_dispman_lock_file", O_RDWR | O_CREAT, 0666);
        if(daemon_lock_fd > 0) {
                ret = lockf(daemon_lock_fd, F_TLOCK, 0);
                if(!ret)
                        daemon_locked = 1;
                else
                        daemon_locked = 0;
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


#ifdef __cplusplus
}
#endif

