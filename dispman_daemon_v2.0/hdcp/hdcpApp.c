/****************************************************************************
 *   FileName    : hdcpApp.c
 *   Description :
 ****************************************************************************
 *
 *   TCC Version 1.0
 *   Copyright (c) Telechips Inc.
 *   All rights reserved
 *
 * This source code contains confidential information of Telechips.
 * Any unauthorized use without a written permission of Telechips including not limited
 * to re-distribution in source or binary form is strictly prohibited.
 * This source code is provided ¡°AS IS¡± and nothing contained in this source code shall
 * constitute any express or implied warranty of any kind, including without limitation,
 * any warranty of merchantability, fitness for a particular purpose or non-infringement
 * of any patent, copyright or other third party intellectual property right.
 * No warranty is made, express or implied, regarding the information¡¯s accuracy,
 * completeness, or performance.
 * In no event shall Telechips be liable for any claim, damages or other liability arising
 * from, out of or in connection with this source code or the use in the source code.
 * This source code is provided subject to the terms of a Mutual Non-Disclosure Agreement
 * between Telechips and Company.
 *
 ****************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>

#include <utils/Log.h>
#include <libhdcp.h>
#include "hdcpApp.h"

#undef LOG_TAG
#define LOG_TAG "[HDCPAPP]"

#define HDCP_APP_VERSION	"HDCP_APP_V1.1.0"

#define HDMI_HPD_PROC_NAME	"/proc/hdmi_tx/hpd"
#define HDCP_WAIT_TIME		3000000
#define NANOS			1000000000LL

#define STATE_HPD_UNPLUGGED	0
#define STATE_HPD_PLUGGED	1
#define STATE_CONFIG_CHANGED	2

struct hdcpApp_t {
	pthread_t	hdcp_thread_id;
	int		thread_act;
	int		hotplug;
	int		mode;
	struct timespec startTspec;
};

static struct hdcpApp_t *hdcp_ctx = NULL;
static pthread_cond_t g_cond;
static pthread_mutex_t g_mutex;
static unsigned char connection_step = 0;

static int HDCPGetHPD(void)
{
	FILE *fd;
	int value;

	fd = fopen(HDMI_HPD_PROC_NAME, "r");
	if (!fd) {
		ALOGE("Cannot open %s\n", HDMI_HPD_PROC_NAME);
		return 0;
	}

	fscanf(fd, "%d", &value);

	fclose(fd);
	return value;
}

static void* hdcpApiThread(void* arg)
{
	struct hdcpApp_t *pHandle = (struct hdcpApp_t *)arg;
	struct timeval now;
	struct timespec ts;
	uint32_t msleep = 100;
	hdcp_status_t status;
	hdcp_version_t version;
	char rxver[5];
	int ret = 0;

	if (!pHandle) {
		ALOGE("Invalid Argument!!\n");
		return NULL;
	}

	pthread_mutex_lock(&g_mutex);
	do {
		if (!pHandle->thread_act)
			break;

		gettimeofday(&now, NULL);
		if (now.tv_usec >= (__suseconds_t)(1000000-(msleep*1000))) {
			ts.tv_sec = now.tv_sec + 1;
			ts.tv_nsec = (now.tv_usec-(1000000-(msleep*1000))) * 1000;
		}
		else {
			ts.tv_sec = now.tv_sec;
			ts.tv_nsec = (now.tv_usec+(msleep*1000)) * 1000;
		}
		ret = pthread_cond_timedwait(&g_cond, &g_mutex, &ts);

		if (connection_step == 1) {
			gettimeofday(&now, NULL);
			ts.tv_sec = now.tv_sec;
			do {
				usleep(100*1000);
				gettimeofday(&now, NULL);
				if (now.tv_sec > (ts.tv_sec + 4))
					break;
			} while (1);
			connection_step = 2;
		}

		/* default sleep duration. */
		msleep = 100;
		status = HDCP_GetStatus();

		if ((hdmi_get_HDCPEnabled() != 1) || !HDCPGetHPD()) {
			if (status != HDCP_STATUS_NONE) {
				HDCP_Disable();
				HDCP_Close();
			}
			continue;
		}

		switch(pHandle->hotplug) {
		case STATE_HPD_UNPLUGGED:
		case STATE_CONFIG_CHANGED:
			/* this code from wait_cable_detach api */
			if (status != HDCP_STATUS_NONE) {
				HDCP_Disable();
				HDCP_Close();
			}
			msleep = 1000;
			break;

		case STATE_HPD_PLUGGED:
			/* this code from wait_cable_attach api
			 * It runs once for the first time when hdmi is connected */
			if((status != HDCP_STATUS_AUTHENTICATED) && (status != HDCP_STATUS_AUTHENTICATING)) {
				HDCP_SetConfig("/usr/etc/hdcp.conf");
				HDCP_SetTxVersion(HDCP_VER_22);
				HDCP_Open();
				HDCP_Enable(pHandle->mode);
			}
			else 
				msleep = 50;
			break;
		default:
			// nothing.
			break;
		}
	} while(1);

	pthread_mutex_unlock(&g_mutex);

	return NULL;
}

void *hdcpAppInit(void)
{
	struct hdcpApp_t *pHandle;

	ALOGI("%s !, version : %s\n", __func__, HDCP_APP_VERSION);

	if (hdcp_ctx)
		return hdcp_ctx;

	pHandle = malloc(sizeof(struct hdcpApp_t));
	if (!pHandle) {
		ALOGE("Cannot alloc memory\n");
		return NULL;
	}
	memset(pHandle, 0x0, sizeof(struct hdcpApp_t));
	connection_step = 0;

	pthread_mutex_init(&g_mutex, NULL);
	pthread_cond_init(&g_cond, NULL);
	pHandle->thread_act = 1;
	if (pthread_create(&pHandle->hdcp_thread_id, NULL, &hdcpApiThread, pHandle)) {
		ALOGE("Connot create thread\n");
		free(pHandle);
		return NULL;
	}

	hdcp_ctx = pHandle;

	//HDCP_SetConfig("/vendor/etc/tdd.ini");

	return (void *)pHandle;
}

int hdcpAppCallback(void *arg, int hotplug, int mode)
{
	struct hdcpApp_t *pHandle = (struct hdcpApp_t *)arg;
	if (!pHandle)
		return -1;

	if (connection_step == 0) {
		if (hotplug)
			connection_step = 1;
		else
			return 0;
	}

	pthread_mutex_lock(&g_mutex);
	pHandle->hotplug = hotplug;
	pHandle->mode = mode;
	pthread_cond_signal(&g_cond);
	pthread_mutex_unlock(&g_mutex);

	return 0;
}

int hdcpAppDeinit(void *arg)
{
	struct hdcpApp_t *pHandle = (struct hdcpApp_t *)arg;
	if (!pHandle)
		return -1;

	pHandle->thread_act = 0;
	pthread_join(pHandle->hdcp_thread_id, NULL);
	pthread_cond_destroy(&g_cond);
	pthread_mutex_destroy(&g_mutex);

	HDCP_Disable();
	HDCP_Close();

	free(pHandle);
	pHandle = NULL;
	hdcp_ctx = NULL;
	return 0;
}
