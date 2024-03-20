/*!
* TCC Version 1.0
* Copyright (c) Telechips Inc.
* All rights reserved 
*  \file        timers.h
*  \brief       Dislay Daemon timer header
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


//
// Timer functions.
//
#ifndef _LIBS_UTILS_TIMERS_H
#define _LIBS_UTILS_TIMERS_H

#include <stdint.h>
#include <sys/types.h>
#include <sys/time.h>

typedef int64_t nsecs_t;       // nano-seconds

nsecs_t systemTime(void);

#endif // TIMERS_H
