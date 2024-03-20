/*!
* TCC Version 1.0
* Copyright (c) Telechips Inc.
* All rights reserved 
*  \file        properties.cpp
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

#ifndef __CUTILS_PROPERTIES_H
#define __CUTILS_PROPERTIES_H

#ifdef __cplusplus
extern "C" {
#endif

/* System properties are *small* name value pairs managed by the
** property service.  If your data doesn't fit in the provided
** space it is not appropriate for a system property.
**
** WARNING: system/bionic/include/sys/system_properties.h also defines
**          these, but with different names.  (TODO: fix that)
*/
#define PROPERTY_KEY_MAX    60
#define PROPERTY_VALUE_MAX  93

#define PROPERTY_MAX        100

#define SR_ATTR_MAX			500

// support resolution HEADER
#define SR_SIZE_DATA_INDEX		2
#define SR_SIZE_DATA_WIDTH		4		
#define SR_SIZE_DATA_HEIGHT		4		
#define SR_SIZE_DATA_INTERLACED	1		
#define SR_SIZE_DATA_HZ			3

/* property_get: returns the length of the value which will never be
** greater than PROPERTY_VALUE_MAX - 1 and will always be zero terminated.
** (the length does not include the terminating zero).
**
** If the property read fails or returns an empty value, the default
** value is used (if nonnull).
*/
int property_get(const char *key, char *value, const char *default_value);
int property_set(const char *key, const char *value);
int property_get_int(const char *key, const int default_value);
int property_vsync_lock_set(int lock);
int property_supported_resolution_get(char *value);
int property_supported_resolution_set(const char *value);
int property_supported_resolution_clear();
int property_supported_edid_drm_info_set(const char *value);
int property_vout_set(const char *value);
int property_list(void (*propfn)(const char *key, const char *value, void *cookie), void *cookie);    
int property_proc_cpuinfo_get(const char* name, char *value);
int display_daemon_lock(void);
void display_daemon_unlock(void);

#ifdef __cplusplus
}
#endif

#endif
