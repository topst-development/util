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

#include <stdio.h>
#include "libhdcp.h"

int HDCP_Open(void)
{
	return HDCP_ERROR_NOT_SUPPORT;
}

int HDCP_Close(void)
{
	return HDCP_ERROR_NOT_SUPPORT;
}

int HDCP_Enable(hdcp_mode_t mode)
{
	return HDCP_ERROR_NOT_SUPPORT;
}

int HDCP_Disable(void)
{
	return HDCP_ERROR_NOT_SUPPORT;
}

hdcp_status_t HDCP_GetStatus(void)
{
	return HDCP_STATUS_NONE;
}

int HDCP_SetConfig(char *ini_file)
{
	return HDCP_ERROR_NOT_SUPPORT;
}

int HDCP_SetTxVersion(hdcp_version_t version)
{
	return HDCP_SUCCESS;
}

hdcp_version_t HDCP_GetTxVersion(void)
{
	return HDCP_VER_22;
}

hdcp_version_t HDCP_GetRxVersion(void)
{
	return HDCP_VER_22;
}
