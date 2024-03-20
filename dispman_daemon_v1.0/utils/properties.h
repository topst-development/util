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
int property_supported_resolution_get(char *value);
int property_supported_resolution_set(const char *value);
int property_supported_resolution_clear();
int property_vout_set(const char *value);
int property_list(void (*propfn)(const char *key, const char *value, void *cookie), void *cookie);    
int property_proc_cpuinfo_get(const char* name, char *value);
int display_daemon_lock(void);
void display_daemon_unlock(void);

#ifdef __cplusplus
}
#endif

#endif
