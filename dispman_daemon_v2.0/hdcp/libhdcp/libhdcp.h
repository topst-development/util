/****************************************************************************
Copyright (C) 2019 Telechips Inc.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions andlimitations under the License.
****************************************************************************/

#ifndef _LIBHDCP_H_
#define _LIBHDCP_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	HDCP_STATUS_NONE		= 0,
	HDCP_STATUS_IDLE,			/* HDCP Opened */
	HDCP_STATUS_AUTHENTICATION_FAILED,
	HDCP_STATUS_AUTHENTICATING,
	HDCP_STATUS_AUTHENTICATED,
} hdcp_status_t;

typedef enum {
	HDCP_MODE_HDMI = 0,
	HDCP_MODE_DVI = 1
} hdcp_mode_t;

typedef enum {
	HDCP_VER_NONE = 0,
	HDCP_VER_14,
	HDCP_VER_22,
	HDCP_VER_MAX
} hdcp_version_t;

/* hdcp return values */
#define HDCP_ALREADY_OPENED		(1)
#define HDCP_SUCCESS			(0)
#define HDCP_ERROR_GENERIC		(-1)
#define HDCP_ERROR_NOT_SUPPORT		(-2)
#define HDCP_ERROR_NOT_OPENED		(-3)
#define HDCP_ERROR_FILE_OPERATION	(-10)
#define HDCP_ERROR_MEMORY_ALLOC		(-11)


/**
 * Open HDCP
 * @return On success, it returns 0. On failure, it returns < 0(error value).
 */
int HDCP_Open(void);

/**
 * Close HDCP
 * @return On success, it returns 0. On failure, it returns < 0(error value).
 */
int HDCP_Close(void);

/**
 * Enable HDCP
 * @param[in] mode Ouput mode, 0: HDMI, 1: DVI
 * @return On success, it returns 0. On failure, it returns < 0(error value).
 */
int HDCP_Enable(hdcp_mode_t mode);

/**
 * Disable HDCP
 * @return On success, it returns 0. On failure, it returns < 0(error value).
 */
int HDCP_Disable(void);

/**
 * Get HDCP Stauts
 * @return hdcp_status.
 */
hdcp_status_t HDCP_GetStatus(void);

/**
 * Set HDCP Environment
 * @param[in] ini_file file path of hdcp.ini
 * @return hdcp_status.
 */
int HDCP_SetConfig(char *ini_file);

/**
 * Set HDCP Tx Versoin
 * @param[in] hdcp_version_t
 * @return hdcp_status.
 */
int HDCP_SetTxVersion(hdcp_version_t version);

/**
 * Get HDCP Tx Versoin
 * @return hdcp_status.
 */
hdcp_version_t HDCP_GetTxVersion(void);

/**
 * Get HDCP Rx Versoin
 * @return hdcp_status.
 */
hdcp_version_t HDCP_GetRxVersion(void);

/**
 * The Resetflag set when HDMI is required to restart due to an error in HDCP.
 * @return Need Reset, it returns 1. No need reset, it returns 0.
 */
int HDCP_ResetFlag(void);

#ifdef __cplusplus
}
#endif

#endif
