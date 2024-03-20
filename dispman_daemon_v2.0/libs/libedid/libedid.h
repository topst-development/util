/*!
* TCC Version 1.0
* Copyright (c) Telechips Inc.
* All rights reserved 
*  \file        libi2c.c
*  \brief       HDMI TX controller i2c
*  \details   
*  \version     1.0
*  \date        2014-2015
*  \copyright
This source code contains confidential information of Telechips.
Any unauthorized use without a written  permission  of Telechips including not 
limited to re-distribution in source  or binary  form  is strictly prohibited.
This source  code is  provided "AS IS"and nothing contained in this source 
code  shall  constitute any express  or implied warranty of any kind, including
without limitation, any warranty of merchantability, fitness for a   particular 
purpose or non-infringement  of  any  patent,  copyright  or  other third party 
intellectual property right. No warranty is made, express or implied, regarding 
the information's accuracy, completeness, or performance. 
In no event shall Telechips be liable for any claim, damages or other liability 
arising from, out of or in connection with this source  code or the  use in the 
source code. 
This source code is provided subject  to the  terms of a Mutual  Non-Disclosure 
Agreement between Telechips and Company.
*******************************************************************************/


#ifndef _LIBDDC_H_
#define _LIBDDC_H_

#ifdef __cplusplus
extern "C" {
#endif
int HDMI_edid_read( void * edid);
int HDMI_edid_extension_read( int block, u8 * edid_ext);
int API_EDID_Read(unsigned char *ptr_user_edid);

#ifdef __cplusplus
}
#endif

#endif /* _LIBDDC_H_ */
