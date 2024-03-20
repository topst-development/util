/*!
* TCC Version 1.0
* Copyright (c) Telechips Inc.
* All rights reserved 
*  \file        Log.h
*  \brief       Dislay Daemon log header
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

#ifndef UTILS_LOG_H
#define UTILS_LOG_H

#include <sys/ioctl.h>
#include <string.h>
#include <stdint.h>

#define ALOGW(args...)	{fprintf(stderr, LOG_TAG " " args); fprintf(stderr, "\n");}
#define ALOGV(args...)	{fprintf(stderr, LOG_TAG " " args); fprintf(stderr, "\n");}
#define ALOGD(args...)	{fprintf(stderr, LOG_TAG " " args); fprintf(stderr, "\n");}
#define ALOGE(args...)	{fprintf(stderr, LOG_TAG " " args); fprintf(stderr, "\n");}
#define ALOGI(args...)	{fprintf(stderr, LOG_TAG " " args); fprintf(stderr, "\n");}

//#define DD_DBG
#ifdef DD_DBG
#define DBG(fmt, args...) printf("\e[33m[%s:%d] \e[0m" fmt, __func__, __LINE__, ## args);
#else
#define DBG(fmt, args...)
#endif

#endif


