

#ifndef UTILS_LOG_H
#define UTILS_LOG_H

#include <sys/ioctl.h>
#include <string.h>

#ifndef uint32_t
#define uint32_t	unsigned int
#endif

#ifndef true
#define true	1
#endif

#ifndef false
#define false	0
#endif


#define ALOGW(args...)	{printf( LOG_TAG " " args); printf("\n");}
#define ALOGV(args...)	{printf( LOG_TAG " " args); printf("\n");}
#define ALOGD(args...)	{printf( LOG_TAG " " args); printf("\n");}
#define ALOGE(args...)	{printf( LOG_TAG " " args); printf("\n");}
#define ALOGI(args...)	{printf( LOG_TAG " " args); printf("\n");}



#endif


