/****************************************************************************
Copyright (C) 2013 Telechips Inc.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
****************************************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>

#include "libhdmi.h"
#include "libphy/libphy.h"
#include "libddc/libddc.h"


#include <utils/Log.h>
#include <mach/tccfb_ioctrl.h>

/**
 * @brief HDMI device name.
 * User should change this.
 */
#define HDMI_DEV_NAME   "/dev/hdmi"
/**
 * @brief AUDIO device name.
 * User should change this.
 */
#define AUDIO_DEV_NAME  "/dev/audio"

#define LOG_TAG 		"LIBHDMI"
#define HDMI_DEBUG     0
#if HDMI_DEBUG
#define DPRINTF(args...)    ALOGD(args)
#else
#define DPRINTF(args...)
#endif

//! Structure for video timing parameters
static const struct hdmi_video_params
{
    /** [H Blank] */
    unsigned int    HBlank;

    /** [V Blank] */
    unsigned int    VBlank;

    /**
     * [H Total : V Total] @n
     * For more information, refer HDMI register map.
     */
    unsigned int    HVLine;

    /**
     * [H Sync polarity : H Sync end point : H Sync start point]@n
     * For more information, refer HDMI register map.
     */
    unsigned int    HSYNCGEN;

    /**
     * [V Sync start line num + V Sync end line num] @n
     * For more information, refer HDMI register map.
     */
    unsigned int    VSYNCGEN;

    /** CEA VIC */
    unsigned char   AVI_VIC;
    /** CEA VIC for 16:9 pixel ratio */
    unsigned char   AVI_VIC_16_9;

    /** 0 - progresive, 1 - interlaced */
    unsigned char   interlaced;

    /** Pixel repetition if double, set 1 */
    unsigned char   repetition;

    /** V Sync polarity */
    unsigned char   polarity;

    /**
     * In case of interlaced mode, @n
     * [end point of bottom field's active region : start point of that]@n
     * For more information, refer HDMI register map.
     */
    unsigned int    VBLANK_F;

    /**
     * In case of interlaced mode, @n
     * [start line of bottom field's V Sync : end line of that]@n
     * For more information, refer HDMI register map.
     */
    unsigned int    VSYNCGEN2;

    /**
     * In case of interlaced mode, @n
     * [start transition point of bottom field's V Sync : end transition of that]@n
     * For more information, refer HDMI register map.
     */
    unsigned int    VSYNCGEN3;

    /** Pixel frequency */
    enum PixelFreq PixelClock; // pixel clock
} HDMIVideoParams[] =
{
    { 0xA0 , 0x16A0D, 0x32020D, 0x11B80E, 0xA00C , 1 , 1 , 0, 0, 1, 0       , 0       , 0       , PIXEL_FREQ_25_200,  },  // v640x480p_60Hz
    { 0x8A , 0x16A0D, 0x35A20D, 0x11300E, 0x900F , 2 , 3 , 0, 0, 1, 0       , 0       , 0       , PIXEL_FREQ_27_027,  },  // v720x480p_60Hz
    { 0x172, 0xF2EE , 0x6722EE, 0x2506C , 0x500A , 4 , 4 , 0, 0, 0, 0       , 0       , 0       , PIXEL_FREQ_74_250,  },  // v1280x720p_60Hz
    { 0x118, 0xB232 , 0x898465, 0x20856 , 0x2007 , 5 , 5 , 1, 0, 0, 0x232A49, 0x234239, 0x4A44A4, PIXEL_FREQ_74_250,  },  // v1920x1080i_60Hz
    { 0x114, 0xB106 , 0x6B420D, 0x128024, 0x4007 , 6 , 7 , 1, 1, 1, 0x10691D, 0x10A10D, 0x380380, PIXEL_FREQ_27_027,  },  // v720x480i_60Hz
    { 0x114, 0xB106 , 0x6B4106, 0x128024, 0x4007 , 8 , 9 , 0, 1, 1, 0       , 0       , 0       , PIXEL_FREQ_27_027,  },  // v720x240p_60Hz
    { 0x228, 0xB106 , 0xD6820D, 0x15084A, 0x4007 , 10, 11, 1, 1, 1, 0x10691D, 0x10A10D, 0x700700, PIXEL_FREQ_54_054,  },  // v2880x480i_60Hz
    { 0x228, 0xB106 , 0x6B4106, 0x15084A, 0x4007 , 12, 13, 0, 1, 1, 0       , 0       , 0       , PIXEL_FREQ_54_054,  },  // v2880x240p_60Hz
    { 0x114, 0x16A0D, 0x6B420D, 0x12681E, 0x900F , 14, 15, 0, 1, 1, 0       , 0       , 0       , PIXEL_FREQ_54_054,  },  // v1440x480p_60Hz
    { 0x118, 0x16C65, 0x898465, 0x20856 , 0x4009 , 16, 16, 0, 0, 0, 0       , 0       , 0       , PIXEL_FREQ_148_500, },  // v1920x1080p_60Hz
    { 0x90 , 0x18A71, 0x360271, 0x11280A, 0x500A , 17, 18, 0, 0, 1, 0       , 0       , 0       , PIXEL_FREQ_27,      },  // v720x576p_50Hz
    { 0x2BC, 0xF2EE , 0x7BC2EE, 0x779B6 , 0x500A , 19, 19, 0, 0, 0, 0       , 0       , 0       , PIXEL_FREQ_74_250,  },  // v1280x720p_50Hz
    { 0x2D0, 0xB232 , 0xA50465, 0x8EA0E , 0x2007 , 20, 20, 1, 0, 0, 0x232A49, 0x234239, 0x738738, PIXEL_FREQ_74_250,  },  // v1920x1080i_50Hz
    { 0x120, 0xC138 , 0x6C0271, 0x125016, 0x2005 , 21, 22, 1, 1, 1, 0x138951, 0x13A13D, 0x378378, PIXEL_FREQ_27,      },  // v720x576i_50Hz
    { 0x120, 0xC138 , 0x6C0138, 0x125016, 0x3006 , 23, 24, 0, 1, 1, 0       , 0       , 0       , PIXEL_FREQ_27,      },  // v720x288p_50Hz
    { 0x240, 0xC138 , 0xD80271, 0x14A82E, 0x2005 , 25, 26, 1, 1, 1, 0x138951, 0x13A13D, 0x6F06F0, PIXEL_FREQ_54,      },  // v2880x576i_50Hz
    { 0x240, 0xC138 , 0xD80138, 0x14A82E, 0x2005 , 27, 28, 0, 1, 1, 0       , 0       , 0       , PIXEL_FREQ_54,      },  // v2880x288p_50Hz
    { 0x120, 0x18A71, 0x6C0271, 0x125816, 0x500A , 29, 30, 0, 1, 1, 0       , 0       , 0       , PIXEL_FREQ_54,      },  // v1440x576p_50Hz
    { 0x2D0, 0x16C65, 0xA50465, 0x8EA0E , 0x4009 , 31, 31, 0, 0, 0, 0       , 0       , 0       , PIXEL_FREQ_148_500, },  // v1920x1080p_50Hz

	{ 0x33E, 0x16C65, 0xABE465, 0xAA27C , 0x4009 , 32, 32, 0, 0, 0, 0       , 0       , 0       , PIXEL_FREQ_74_176,  },  // v1920x1080p_23.976Hz
    { 0x33E, 0x16C65, 0xABE465, 0xAA27C , 0x4009 , 32, 32, 0, 0, 0, 0       , 0       , 0       , PIXEL_FREQ_74_250,  },  // v1920x1080p_24Hz
    { 0x2D0, 0x16C65, 0xA50465, 0x8EA0E , 0x4009 , 33, 33, 0, 0, 0, 0       , 0       , 0       , PIXEL_FREQ_74_250,  },  // v1920x1080p_25Hz
    { 0x118, 0x16C65, 0x898465, 0x20856 , 0x4009 , 34, 34, 0, 0, 0, 0       , 0       , 0       , PIXEL_FREQ_74_250,  },  // v1920x1080p_30Hz

    { 0x172, 0xF5DC , 0x6725DC, 0x2506C , 0x500A , 4 , 4 , 0, 0, 0, 0       , 0       , 0       , PIXEL_FREQ_148_500, },  // v1280x720p_60Hz, 3D Frame packing

	{ 0x228, 0x16A0D, 0xD6820D, 0x14D83E, 0x900F , 35, 36, 0, 0, 1, 0       , 0       , 0       , PIXEL_FREQ_108_108, },  // v2880x480p_60Hz
    { 0x240, 0x18A71, 0xD80271, 0x14B82E, 0x500A , 37, 38, 0, 1, 1, 0       , 0       , 0       , PIXEL_FREQ_108,     },  // v2880x576p_50Hz
    { 0x180, 0x2AA71, 0x9004E2, 0x3181E , 0x1701C, 39, 39, 0, 0, 0, 0x2712C6, 0x28728F, 0x4a44a4, PIXEL_FREQ_72,      },  // v1920x1080i_50Hz(1250)
    { 0x2D0, 0xB232 , 0xA50465, 0x8EA0E , 0x2007 , 40, 40, 1, 0, 0, 0x232A49, 0x234239, 0x738738, PIXEL_FREQ_148_500, },  // v1920x1080i_100Hz
    { 0x2BC, 0xF2EE , 0x7BC2EE, 0x779B6 , 0x500A , 41, 41, 0, 0, 0, 0       , 0       , 0       , PIXEL_FREQ_148_500, },  // v1280x720p_100Hz
    { 0x90 , 0x18A71, 0x360271, 0x11280A, 0x500A , 42, 43, 0, 0, 1, 0       , 0       , 0       , PIXEL_FREQ_54,      },  // v720x576p_100Hz
    { 0x120, 0xC138 , 0x6C0271, 0x125016, 0x2005 , 44, 45, 1, 1, 1, 0x138951, 0x13A13D, 0x378378, PIXEL_FREQ_54,      },  // v720x576i_100Hz
    { 0x118, 0xB232 , 0x898465, 0x20856 , 0x2007 , 46, 46, 1, 0, 0, 0x232A49, 0x234239, 0x4A44A4, PIXEL_FREQ_148_500, },  // v1920x1080i_120Hz
    { 0x172, 0xF2EE , 0x6722EE, 0x2506C , 0x500A , 47, 47, 0, 0, 0, 0       , 0       , 0       , PIXEL_FREQ_148_500, },  // v1280x720p_120Hz
    { 0x8A , 0x16A0D, 0x35A20D, 0x11300E, 0x900F , 48, 49, 0, 0, 1, 0       , 0       , 0       , PIXEL_FREQ_54_054,  },  // v720x480p_120Hz
    { 0x114, 0xB106 , 0x6B420D, 0x128024, 0x4007 , 50, 51, 1, 1, 1, 0x10691D, 0x10A10D, 0x380380, PIXEL_FREQ_54_054,  },  // v720x480i_120Hz
    { 0x90 , 0x18A71, 0x360271, 0x11280A, 0x500A , 52, 53, 0, 0, 1, 0       , 0       , 0       , PIXEL_FREQ_108,     },  // v720x576p_200Hz
    { 0x120, 0xC138 , 0x6C0271, 0x125016, 0x2005 , 54, 55, 1, 1, 1, 0x138951, 0x13A13D, 0x378378, PIXEL_FREQ_108,     },  // v720x576i_200Hz
    { 0x8A , 0x16A0D, 0x35A20D, 0x11300E, 0x900F , 56, 57, 0, 0, 1, 0       , 0       , 0       , PIXEL_FREQ_108_108, },  // v720x480p_240Hz
    { 0x114, 0xB106 , 0x6B420D, 0x128024, 0x4007 , 58, 59, 1, 1, 1, 0x10691D, 0x10A10D, 0x380380, PIXEL_FREQ_108_108, },  // v720x480i_240Hz
    //{ 0x1B0, 0xDB1B , 0x70031B, 0x40040 , 0x500A ,  0,  0, 0, 0, 1, 0       , 0       , 0       , PIXEL_FREQ_84_75,   },  // v1360x768p_60Hz
    { 0x1A0, 0xF31E , 0x6F031E, 0x134048, 0x2005 ,  0,  0, 0, 0, 1, 0       , 0       , 0       , PIXEL_FREQ_84_75,   },  // v1360x768p_60Hz
    { 0xBE , 0xDB1B , 0x61431B, 0xE42C  , 0x2005 ,  0,  0, 0, 0, 1, 0       , 0       , 0       , PIXEL_FREQ_74_250,  },  // v1366x768p_60Hz
    { 0x140, 0x13326, 0x540326, 0x1070A0, 0x2008 , 1 , 1 , 0, 0, 0, 0       , 0       , 0       , PIXEL_FREQ_65,      },  // v1024x768p_60Hz
    { 0xFA , 0x29A33, 0x450233, 0x1D056 , 0x2005 , 1 , 1 , 0, 0, 0, 0       , 0       , 0       , PIXEL_FREQ_37_293,  },  // v854x480p_60Hz

    { 0x118, 0x16C65, 0x898465, 0x20856 , 0x4009 , 34, 34, 0, 0, 0, 0       , 0       , 0       , PIXEL_FREQ_74_176,  },  // v1920x1080p_30Hz

};


#if defined(TELECHIPS)	//LCDC

#if (0)
#define DEFAULT_HDMI_LCDC_TIMING	{ 0,  1,  1,  0,  0,  1,  0,  1,  0,  61 ,    639 ,   59 ,     37 ,    0,  0,  5,  479 ,   29,  8,   5,   479 ,    29,  8, /*640x480p @ 60Hz       25.200MHz   */ }
#else
#define DEFAULT_HDMI_LCDC_TIMING	{ 0,  0,  0,  0,  0,  1,  0,  1,  0,  43 ,    1919,   147,     87 ,    0,  0,  4,  1079,   35,  3,   4,   1079,    35,  3, /*1920x1080p @ 60Hz     148.5MHz    */ }
#endif

static const struct lcdc_timimg_parms_t LCDCTimimgParams[] = 
{                                            		/*  ID  IV  IH  IP  DP  NI TV  TFT STN  LPW     LPC   	LSWC    LEWC   	VDB VDF FPW    FLC  FSWC FEWC FPW2   FLC2  FSWC2 FEWC2 */
  /* v640x480p_60Hz         PIXEL_FREQ_25_200,  */    { 0,  1,  1,  0,  0,  1,  0,  1,  0,  61 ,    639 ,   59 ,     37 , 	 0,   0,  5,  479 ,   29,  8,   5,   479 ,    29,  8, /*640x480p @ 60Hz       25.200MHz   */ },
  /* v720x480p_60Hz         PIXEL_FREQ_27_027,  */    { 0,  1,  1,  0,  0,  1,  0,  1,  0,  61 ,    719 ,   59 ,     15 ,    0,  0,  5,  479 ,   29,  8,   5,   479 ,    29,  8, /*720x480p @ 60Hz       27.027MHz   */ },
  /* v1280x720p_60Hz        PIXEL_FREQ_74_250,  */    { 0,  0,  0,  0,  0,  1,  0,  1,  0,  39 ,    1279,   219,     109,    0,  0,  4,  719 ,   19,  4,   4,   719 ,    19,  4, /*1280x720p @ 60Hz      74.25MHz    */ },
  /* v1920x1080i_60Hz       PIXEL_FREQ_74_250,  */    { 0,  0,  0,  0,  0,  0,  1,  0,  0,  43 ,    1919,   147,     87 ,    0,  0,  9,  1079,   29,  3,   9,   1079,    31,  3, /*1920x1080i @ 60Hz     74.25MHz    */ },
  /* v720x480i_60Hz         PIXEL_FREQ_27_027,  */    { 0,  1,  1,  0,  1,  0,  1,  0,  0,  123,    1439,   113,     37 ,    0,  0,  5,  479 ,   29,  7,   5,   479 ,    31,  7, /*720x480i @ 60Hz       27.027MHz   */ },
  /* v720x240p_60Hz         PIXEL_FREQ_27_027,  */    DEFAULT_HDMI_LCDC_TIMING,
  /* v2880x480i_60Hz        PIXEL_FREQ_54_054,  */    DEFAULT_HDMI_LCDC_TIMING,
  /* v2880x240p_60Hz        PIXEL_FREQ_54_054,  */    DEFAULT_HDMI_LCDC_TIMING,
  /* v1440x480p_60Hz        PIXEL_FREQ_54_054,  */    {0, 0, 0, 0, 1, 1, 0, 1, 0, 123, 1439, 120, 30, 0, 0, 5, 479, 32, 5, 5, 479, 32, 5, /*1440x480p @ 60Hz 54.054MHz*/ },
  /* v1920x1080p_60Hz       PIXEL_FREQ_148_500, */    { 0,  0,  0,  0,  0,  1,  0,  1,  0,  43 ,    1919,   147,     87 ,    0,  0,  4,  1079,   35,  3,   4,   1079,    35,  3, /*1920x1080p @ 60Hz     148.5MHz    */ },
  /* v720x576p_50Hz         PIXEL_FREQ_27,      */    { 0,  1,  1,  0,  0,  1,  0,  1,  0,  63 ,    719 ,   67 ,     11 ,    0,  0,  4,  575 ,   38,  4,   4,   575 ,    38,  4, /*720x576p @ 50Hz       27MHz       */ },
  /* v1280x720p_50Hz        PIXEL_FREQ_74_250,  */    { 0,  0,  0,  0,  0,  1,  0,  1,  0,  39 ,    1279,   219,     439,    0,  0,  4,  719 ,   19,  4,   4,   719 ,    19,  4, /*1280x720p @ 50Hz      74.25MHz    */ },
  /* v1920x1080i_50Hz       PIXEL_FREQ_74_250,  */    { 0,  0,  0,  0,  0,  0,  1,  0,  0,  43 ,    1919,   174,     500,    0,  0,  9,  1079,   29,  3,   9,   1079,    31,  3, /*1920x1080i @ 50Hz     74.25MHz    */ },
  /* v720x576i_50Hz         PIXEL_FREQ_27,      */    { 0,  1,  1,  0,  1,  0,  1,  0,  0,  125,    1439,   137,     23 ,    0,  0,  5,  575 ,   37,  3,   5,   575 ,    39,  3, /*720x576i @ 50Hz       27MHz       */ },
  /* v720x288p_50Hz         PIXEL_FREQ_27,      */    DEFAULT_HDMI_LCDC_TIMING,
  /* v2880x576i_50Hz        PIXEL_FREQ_54,      */    DEFAULT_HDMI_LCDC_TIMING,
  /* v2880x288p_50Hz        PIXEL_FREQ_54,      */    DEFAULT_HDMI_LCDC_TIMING,
  /* v1440x576p_50Hz        PIXEL_FREQ_54,      */    { 0,  0,  1,  0,  1,  1,  0,  1,  0,  125,    1439,   137,     23 ,    0,  0,  4,  575 ,   38,  4,   4,   575 ,    38,  4, /*1440x576p @ 50Hz      54MHz       */ },
  /* v1920x1080p_50Hz       PIXEL_FREQ_148_500, */    { 0,  0,  0,  0,  0,  1,  0,  1,  0,  43 ,    1919,   174,     500,    0,  0,  4,  1079,   35,  3,   4,   1079,    35,  3, /*1920x1080p @ 50Hz     148.5MHz    */ },
  /* v1920x1080p_23.976Hz   PIXEL_FREQ_74_176,  */    { 0,  0,  0,  0,  0,  1,  0,  1,  0,  43 ,    1919,   500,     284,    0,  0,  4,  1079,   35,  3,   4,   1079,    35,  3, /*1920x1080p @ 24Hz     74.176MHz    */ },
  /* v1920x1080p_24Hz       PIXEL_FREQ_74_250,  */    { 0,  0,  0,  0,  0,  1,  0,  1,  0,  43 ,    1919,   500,     284,    0,  0,  4,  1079,   35,  3,   4,   1079,    35,  3, /*1920x1080p @ 24Hz     74.25MHz    */ },
  /* v1920x1080p_25Hz       PIXEL_FREQ_74_250,  */    { 0,  0,  0,  0,  0,  1,  0,  1,  0,  43 ,    1919,   430,     244,    0,  0,  4,  1079,   35,  3,   4,   1079,    35,  3, /*1920x1080p @ 25Hz     74.25MHz    */ },
  /* v1920x1080p_30Hz       PIXEL_FREQ_74_250,  */    { 0,  0,  0,  0,  0,  1,  0,  1,  0,  43 ,    1919,   147,     87 ,    0,  0,  4,  1079,   35,  3,   4,   1079,    35,  3, /*1920x1080p @ 30Hz     74.25MHz    */ },

  /* v1280x720p_60Hz_3D     PIXEL_FREQ_148_500, */    { 0,  0,  0,  0,  0,  1,  0,  1,  0,  39 ,    1279,   219,     109,    0,  0,  4,  1469 ,   19,  4,   4,   1469 ,    19,  4, /*1280x720p @ 60Hz    148.5MHz, 3D Frame packing */ },

  /* v2880x480p_60Hz        PIXEL_FREQ_108_108, */    DEFAULT_HDMI_LCDC_TIMING,
  /* v2880x576p_50Hz        PIXEL_FREQ_108,     */    DEFAULT_HDMI_LCDC_TIMING,
  /* v1920x1080i_50Hz(1250) PIXEL_FREQ_72,      */    DEFAULT_HDMI_LCDC_TIMING,
  /* v1920x1080i_100Hz      PIXEL_FREQ_148_500, */    DEFAULT_HDMI_LCDC_TIMING,
  /* v1280x720p_100Hz       PIXEL_FREQ_148_500, */    DEFAULT_HDMI_LCDC_TIMING,
  /* v720x576p_100Hz        PIXEL_FREQ_54,      */    DEFAULT_HDMI_LCDC_TIMING,
  /* v720x576i_100Hz        PIXEL_FREQ_54,      */    DEFAULT_HDMI_LCDC_TIMING,
  /* v1920x1080i_120Hz      PIXEL_FREQ_148_500, */    DEFAULT_HDMI_LCDC_TIMING,
  /* v1280x720p_120Hz       PIXEL_FREQ_148_500, */    { 0,  0,  0,  0,  0,  1,  0,  1,  0,  39 ,    1279,   219,     109,    0,  0,  4,  719 ,   19,  4,   4,   719 ,    19,  4, /*1280x720p @ 60Hz      74.25MHz    */ },
  /* v720x480p_120Hz        PIXEL_FREQ_54_054,  */    DEFAULT_HDMI_LCDC_TIMING,
  /* v720x480i_120Hz        PIXEL_FREQ_54_054,  */    DEFAULT_HDMI_LCDC_TIMING,
  /* v720x576p_200Hz        PIXEL_FREQ_108,     */    DEFAULT_HDMI_LCDC_TIMING,
  /* v720x576i_200Hz        PIXEL_FREQ_108,     */    DEFAULT_HDMI_LCDC_TIMING,
  /* v720x480p_240Hz        PIXEL_FREQ_108_108, */    DEFAULT_HDMI_LCDC_TIMING,
  /* v720x480i_240Hz        PIXEL_FREQ_108_108, */    DEFAULT_HDMI_LCDC_TIMING,
  #if 0 
  /* v1360x768p_60Hz        PIXEL_FREQ_84_75,   */    { 0,  0,  0,  0,  0,  1,  0,  1,  0, 111 ,   1359,    63,    255 ,    0,  0,  5,   767,   2,  17,   5,    767,    2,  17, /* 1360x768p @ 60Hz     84.75MHz    */ },
  #endif
  /* v1360x768p_60Hz        PIXEL_FREQ_84_75,   */    { 0,  0,  0,  0,  0,  1,  0,  1,  0, 135 ,   1359,    71,    207 ,    0,  0,  4,   767,   2,  21,   4,    767,    2,  21, /* 1360x768p @ 60Hz     84.75MHz    */ },
  /* v1366x768p_60Hz        PIXEL_FREQ_74_250,  */    { 0,  0,  0,  0,  0,  1,  0,  1,  0,  88 ,   1365,    43,     56 ,    0,  0,  5,   767,   2,  17,   5,    767,    2,  17, /* 1366x768p @ 60Hz     84.75MHz    */ },
  /* v1024x768p_60Hz        PIXEL_FREQ_65    ,  */    { 0,  1,  1,  0,  0,  1,  0,  1,  0,  135,   1023,   159,     23 ,    0,  0,  5,   767,  28,   2,   5,    767 ,  28,   2, /*1024x768p @ 60Hz     65.000MHz   */ },
  /*  v854x768p_60Hz        PIXEL_FREQ_37_293,  */    { 0,  0,  0,  0,  0,  1,  0,  1,  0,   47,    853,   115,     85 ,    0,  0, 19,   479,  33,  28,  19,    479,   33,  28, /*  854x480p @ 60Hz     37.293MHz   */ },  
  /* v1920x1080p_30Hz       PIXEL_FREQ_74_250,  */    { 0,  0,  0,  0,  0,  1,  0,  1,  0,  43 ,    1919,   147,     87 ,    0,  0,  4,  1079,   35,  3,   4,   1079,    35,  3, /*1920x1080p @ 30Hz     74.25MHz    */ },
};
#endif

/** HDMI device file descriptor */
static int hdmi_fd = -1;
/** Audio device file descriptor */
static int audio_fd = -1;

/**
 * Initialize HDMI library. Open HDMI device file and audio device file.
 * @return If success, return 1; Otherwise, return 0.
 */
int HDMIOpen(void)
{
    if (hdmi_fd == -1 && (hdmi_fd = open(HDMI_DEV_NAME, O_RDWR)) < 0)
    {
        ALOGE("can not open \"%s\"\n", HDMI_DEV_NAME);
        return 0;
    }

    if (audio_fd == -1 && (audio_fd = open(AUDIO_DEV_NAME, O_RDWR)) < 0)
    {
        ALOGE("can not open \"%s\"\n", AUDIO_DEV_NAME);
        return 0;
    }

    return 1;
}

/**
 * Finalize HDMI library. Close HDMI device file and audio device file.
 * @return If success, return 1; Otherwise, return 0.
 */
int HDMIClose(void)
{
    int retval = 1;

    if (audio_fd != -1)
    {
        if (close(audio_fd) < 0)
        {
            ALOGE("can not close \"%s\"\n", AUDIO_DEV_NAME);
            retval = 0;
        }
        else
            audio_fd = -1;
    }

    if (hdmi_fd != -1)
    {
        if (close(hdmi_fd) < 0)
        {
            ALOGE("can not close \"%s\"\n", HDMI_DEV_NAME);
            retval = 0;
        }
        else
            hdmi_fd = -1;
    }

    return retval;
}

#if defined(TELECHIPS)
/**
 * Configurate LCDC.
 * @param   hdmi_video_format [in]    lcdc timing to set
 * @return  If success, return 1; Otherwise, return 0.
 */

#define FB_DEV_NAME		"/dev/fb0"

/** framebuffer device file descriptor */
static int fb_fd = -1;
int HDMIFBOpen(void)
{
	if (fb_fd == -1 && (fb_fd = open(FB_DEV_NAME, O_RDWR)) < 0)
	{
		ALOGE("can not open \"%s\"\n", FB_DEV_NAME);
		return 0;
	}
	return 1;
}

int HDMIFBClose(void)
{
	if (fb_fd != -1)
	{
		if (close(fb_fd) < 0)
		{
			ALOGE("can not close \"%s\"\n", FB_DEV_NAME);
		}
		fb_fd = -1;
	}
	return 1;
}


int HDMILcdcConfig(enum VideoFormat hdmi_video_format)
{
	LCDC_HDMI_MODE mode;

    struct device_lcdc_timing_params device;

    // check file
    if (fb_fd == -1)    {
        ALOGE("HDMI device File is not available\n");
        return 0;
    }

	if (ioctl(fb_fd,TCC_LCDC_HDMI_START, NULL)) {
		ALOGE("ioctl(HDMI_IOC_SET_LCDC_TIMING) failed!\n");
		return 0;
	}


    // set lcdc
    memcpy((void*)&device,(const void*)&(LCDCTimimgParams[hdmi_video_format]),sizeof(device));

    // set video parameters
    if (ioctl(fb_fd,TCC_LCDC_HDMI_TIMING, &device)) {
        DPRINTF("ioctl(HDMI_IOC_SET_LCDC_TIMING) failed!\n");
        return 0;
    }

    return 1;
}


int HDMI_lcdc_stop(void)
{
	if(fb_fd != -1) {
		if (ioctl(fb_fd, TCC_LCDC_HDMI_END, NULL) ) {
			DPRINTF("ioctl(TCC_LCDC_HDMI_END) failed!\n");
			return 0;
		}	
	}
	return 1;
}

unsigned int HDMI_LcdcCheck(void)
{
	int hdmi_check  = 0;
	
	if (ioctl(fb_fd, TCC_LCDC_HDMI_CHECK, &hdmi_check) ) {
		DPRINTF("%s TCC_LCDC_HDMI_CHECK failed!\n",__func__);
		return 0;
	}	

	return hdmi_check;
}

unsigned int HDMI_SuspendCheck(void)
{
	int suspend_check = 0;
	
	if (ioctl(hdmi_fd, HDMI_IOC_GET_SUSPEND_STATUS, &suspend_check) ) {
		DPRINTF("%s HDMI_IOC_GET_SUSPEND_STATUS failed!\n",__func__);
		return 0;
	}	

	return suspend_check;
}

unsigned int HDMI_OutputModeCheck(void)
{
	unsigned int OutputMode  = 0;
	
	if (ioctl(fb_fd, TCC_LCDC_OUTPUT_MODE_CHECK, &OutputMode) ) {
		DPRINTF("%s failed!\n",__func__);
		return 0;
	}	
	return OutputMode;
}

#endif /*TELECHIPS*/


/**
 * Set requested HDMI/DVI mode.
 * @param   mode [in]    HDMI/DVI mode to set
 * @return  If success, return 1; Otherwise, return 0
 */
int HDMISetHDMIMode(const enum HDMIMode mode)
{

    // check file
    if (hdmi_fd == -1) {
        DPRINTF("HDMI device File is not available\n");
        return 0;
    }

    // set mode
    if (ioctl(hdmi_fd,HDMI_IOC_SET_HDMIMODE, &mode)) {
        DPRINTF("ioctl(HDMI_IOC_SET_HDMIMODE) failed!\n");
        return 0;
    }

    return 1;
}

/**
 * Configurate PHY.
 * @param   hdmi_video_mode [in]    Video mode to set
 * @return  If success, return 1; Otherwise, return 0.
 */
int HDMIPhyConfig(const struct HDMIVideoParameter * const hdmi_video_mode)
{
    unsigned char phy_ready = 0;
    int i = 0;
	DPRINTF("HDMIPhyConfig mode:%d resolution:%d !\n",hdmi_video_mode->mode, hdmi_video_mode->resolution);

    if (hdmi_fd == -1)
    {
        DPRINTF("Open file descriptor first! (hdmi_fd)\n");
        return 0;
    }

	// Check HDMI PHY STAUS
    if (ioctl(hdmi_fd,HDMI_IOC_GET_PHYREADY,&phy_ready))
    {
        DPRINTF("ioctl(HDMI_IOC_GET_PHYREADY) failed!\n");
        return 0;
    }

	if(!phy_ready)
	{
		if(ioctl(hdmi_fd, HDMI_IOC_SET_PHYRESET, NULL))
		{
			DPRINTF("ioctl(HDMI_IOC_SET_PHYRESET) failed!\n");
			return 0;
		}
	}

    if (!PHYConfig(HDMIVideoParams[hdmi_video_mode->resolution].PixelClock,
          hdmi_video_mode->colorDepth))
    {
        DPRINTF("phy config failed!\n");
        return 0;
    }

    do
    {
        if (ioctl(hdmi_fd,HDMI_IOC_GET_PHYREADY,&phy_ready))
        {
            DPRINTF("ioctl(HDMI_IOC_GET_PHYREADY) failed!\n");
            break;
        }
        if (i++ == 100) break;
    } while (!phy_ready);

    if (!phy_ready)
    {
        DPRINTF("phy is not ready!!!\n");
    }
    else
    {
        DPRINTF("phy configured cnt:%d \n", i);
    }

    return 1;
}

/**
 * Configure PHY as default mode (640x480\@60Hz, 24 bit color depth).@n
 * Before PHY produce any TMDS clock, HDMI Link does not work.
 * @return If success, return 1; Otherwise, return 0.
 */
//TODO: only for 45nm PHY
int HDMIDefaultPhyConfig(void)
{
    unsigned char phy_ready = 0;
    int i = 0;

    if ( hdmi_fd == -1 )
    {
        DPRINTF("Open file descriptor first! (hdmi_fd)\n");
        return 0;
    }

	// Check HDMI PHY STAUS
    if (ioctl(hdmi_fd,HDMI_IOC_GET_PHYREADY,&phy_ready))
    {
        DPRINTF("ioctl(HDMI_IOC_GET_PHYREADY) failed!\n");
        return 0;
    }

	if(!phy_ready)
	{
		if(ioctl(hdmi_fd, HDMI_IOC_SET_PHYRESET, NULL))
		{
			DPRINTF("ioctl(HDMI_IOC_SET_PHYRESET) failed!\n");
			return 0;
		}
	}

    if (!PHYConfig(PIXEL_FREQ_148_500,HDMI_CD_24))
    {
        DPRINTF("phy config failed!\n");
        return 0;
    }

    do
    {
        if (ioctl(hdmi_fd,HDMI_IOC_GET_PHYREADY,&phy_ready))
        {
            DPRINTF("ioctl(HDMI_IOC_GET_PHYREADY) failed!\n");
            break;
        }
        if (i++ == 100) break;
    }
    while (!phy_ready);

    if (!phy_ready)    {
        DPRINTF("phy is not ready!!!\n");
    }
    else    {
        DPRINTF("phy configured.\n");
    }

    return 1;
}

/**
 * Set requested video mode.
 * @param   hdmi_video_mode [in]   requested video mode to set
 * @return  If success, return 1; Otherwise, return 0
 */
int HDMISetVideoMode(const struct HDMIVideoParameter * const hdmi_video_mode)
{
    struct device_video_params device;
	enum PixelLimit pxl_lmt = HDMI_FULL_RANGE;

    if (!hdmi_video_mode)
    {
        DPRINTF("bad args: hdmi_video_mode\n");
        return 0;
    }

    // check file
    if (hdmi_fd == -1)
    {
        DPRINTF("HDMI device File is not available\n");
        return 0;
    }

    // set phy
    if (!HDMIPhyConfig(hdmi_video_mode))
    {
        DPRINTF("fail to config PHY!\n");
        return 0;
    }

    // parsing video parameters
    memcpy((void*)&device,(const void*)&(HDMIVideoParams[hdmi_video_mode->resolution]),sizeof(device));

    // set pixel aspect ratio
    // !! must be setting before 'HDMI_IOC_SET_VIDEOMODE'
    DPRINTF("ioctl(HDMI_IOC_SET_PIXEL_ASPECT_RATIO)\n");
    if (ioctl(hdmi_fd,HDMI_IOC_SET_PIXEL_ASPECT_RATIO, &hdmi_video_mode->pixelAspectRatio)) {
        DPRINTF("ioctl(HDMI_IOC_SET_PIXEL_ASPECT_RATIO) failed!\n");
        return 0;
    }

    // set video parameters
    DPRINTF("ioctl(HDMI_IOC_SET_VIDEOMODE)\n");
    if (ioctl(hdmi_fd,HDMI_IOC_SET_VIDEOMODE, &device) ) {
        DPRINTF("ioctl(HDMI_IOC_SET_VIDEOMODE) failed!\n");
        return 0;
    }

    // set video format information
    DPRINTF("ioctl(HDMI_IOC_SET_VIDEOFORMAT_INFO)\n");
    if (ioctl(hdmi_fd,HDMI_IOC_SET_VIDEOFORMAT_INFO, &hdmi_video_mode->resolution) ) {
        DPRINTF("ioctl(HDMI_IOC_SET_VIDEOFORMAT_INFO) failed!\n");
        return 0;
    }

	// set colorimetry
	if (ioctl(hdmi_fd,HDMI_IOC_SET_COLORIMETRY, &hdmi_video_mode->colorimetry)) {
		DPRINTF("ioctl(HDMI_IOC_SET_COLORIMETRY) failed!\n");
		return 0;
	}

    // set color space
    DPRINTF("ioctl(HDMI_IOC_SET_COLORSPACE)\n");
    if (ioctl(hdmi_fd,HDMI_IOC_SET_COLORSPACE, &hdmi_video_mode->colorSpace)) {
        DPRINTF("ioctl(HDMI_IOC_SET_COLORSPACE) failed!\n");
        return 0;
    }

	// set pixel limitation.
	switch(hdmi_video_mode->colorSpace) 
	{
		case HDMI_CS_RGB:		/** RGB color space */
			if (hdmi_video_mode->resolution == v640x480p_60Hz)
				pxl_lmt = HDMI_FULL_RANGE;
			else
				pxl_lmt = HDMI_RGB_LIMIT_RANGE;
			break;
		case HDMI_CS_YCBCR444:	/** YCbCr 4:4:4 color space */
		case HDMI_CS_YCBCR422:	/** YCbCr 4:2:2 color space */
			pxl_lmt = HDMI_YCBCR_LIMIT_RANGE;
			break;
	}
	
	if (!HDMISetPixelLimit(pxl_lmt))
	{
		DPRINTF("fail to set pixel limitation!!\n");
		return 0;
	}
	
#if defined(TELECHIPS)
	// set lcdc
	if (!HDMILcdcConfig(hdmi_video_mode->resolution))
	{
		DPRINTF("fail to config LCDC!\n");
		return 0;
	}
#endif

    // set color space
    DPRINTF("ioctl(HDMI_IOC_SET_COLORSPACE)\n");
    if (ioctl(hdmi_fd,HDMI_IOC_SET_COLORSPACE, &hdmi_video_mode->colorSpace)) {
        DPRINTF("ioctl(HDMI_IOC_SET_COLORSPACE) failed!\n");
        return 0;
    }

    return 1;
}

/**
 * Set requested audio mode.
 * @param   hdmi_audio_mode [in]    Audio mode to set
 * @return If success, return 1;Otherwise, return 0
 */
int HDMISetAudioMode(const struct HDMIAudioParameter * const hdmi_audio_mode)
{
    // check paramter
    if (hdmi_audio_mode == NULL)
    {
        DPRINTF("bad args: hdmi_audio_mode\n");
        return 0;
    }

    // check file
    if (audio_fd == -1)
    {
        DPRINTF("AUDIO device File is not available\n");
        return 0;
    }

    // setting audio input port parameters
    switch (hdmi_audio_mode->inputPort)
    {
        int wordlength,codingtype;
        case I2S_PORT:
        {

            if (ioctl(audio_fd,AUDIO_IOC_RESET_I2S_CUV,NULL))
            {
                DPRINTF("error to set AUDIO_IOC_RESET_I2S_CUV\n");
                return 0;
            }
			
            // set CUV
            // sample freq
            if (ioctl(audio_fd,AUDIO_IOC_SET_I2S_CUV_SET_SAMPLEFREQ,&hdmi_audio_mode->sampleFreq))
            {
                DPRINTF("error to set sample freq on I2S CUV\n");
                return 0;
            }

            // channel number
            if (ioctl(audio_fd,AUDIO_IOC_SET_I2S_CUV_SET_CHANNELNUMBER,&hdmi_audio_mode->channelNum))
            {
                DPRINTF("error to set channel number on I2S CUV\n");
                return 0;
            }

            if (hdmi_audio_mode->formatCode == LPCM_FORMAT)
            {
                codingtype = CUV_LPCM;
            }
            else
            {
                codingtype = CUV_NLPCM;
            }

            if (ioctl(audio_fd,AUDIO_IOC_SET_I2S_CUV_SET_CODINGTYPE, &codingtype))
            {
                DPRINTF("error to set coding type on I2S CUV\n");
                return 0;
            }

            if (ioctl(audio_fd,AUDIO_IOC_SET_AUDIOFORMATCODE_INFO, &hdmi_audio_mode->formatCode))
            {
                DPRINTF("error to set coding type on I2S CUV\n");
                return 0;
            }

//Check: don't care
            // word length
            if (codingtype == CUV_LPCM)
            {
                switch(hdmi_audio_mode->wordLength)
                {
                    case WORD_16:
                    {
                        wordlength = CUV_WL_20_16;
                        break;
                    }
                    case WORD_17:
                    {
                        wordlength = CUV_WL_20_17;
                        break;
                    }
                    case WORD_18:
                    {
                        wordlength = CUV_WL_20_18;
                        break;
                    }
                    case WORD_19:
                    {
                        wordlength = CUV_WL_20_19;
                        break;
                    }
                    case WORD_20:
                    {
                        wordlength = CUV_WL_24_20;
                        break;
                    }
                    case WORD_21:
                    {
                        wordlength = CUV_WL_24_21;
                        break;
                    }
                    case WORD_22:
                    {
                        wordlength = CUV_WL_24_22;
                        break;
                    }
                    case WORD_23:
                    {
                        wordlength = CUV_WL_24_23;
                        break;
                    }
                    case WORD_24:
                    {
                        wordlength = CUV_WL_24_24;
                        break;
                    }
                    default:
                    {
                        wordlength = CUV_WL_24_NOT_DEFINED;
                        break;
                    }
                } // switch

                if (ioctl(audio_fd,AUDIO_IOC_SET_I2S_CUV_SET_WORDLENGTH ,&wordlength))
                {
                    DPRINTF("error to set word length on I2S CUV\n");
                    return 0;
                }

                if (ioctl(audio_fd,AUDIO_IOC_SET_I2S_CUV_SET_WORDLENGTH_INFO ,&hdmi_audio_mode->wordLength))
                {
                    DPRINTF("error to set word length info on I2S CUV\n");
                    return 0;
                }

            } // if (LPCM)

            if (ioctl(audio_fd,AUDIO_IOC_SET_I2S_PARAMETER,&hdmi_audio_mode->i2sParam))
            {
                DPRINTF("error to set i2s parameters\n");
                return 0;
            }
            break;
        }
        case SPDIF_PORT:
        {
            if (hdmi_audio_mode->formatCode == LPCM_FORMAT)
            {
                codingtype = SPDIF_LPCM;
            }
            else
            {
                codingtype = SPDIF_NLPCM;
            }

			if (ioctl(audio_fd, AUDIO_IOC_SET_SPDIF_SET_SAMPLEFREQ , &hdmi_audio_mode->sampleFreq))
            {
                DPRINTF("error to set sample freq on SPDIF\n");
                return 0;
            }
			
            if (ioctl(audio_fd, AUDIO_IOC_SET_SPDIF_SET_CODINGTYPE ,&codingtype))
            {
                DPRINTF("error to set coding type on SPDIF\n");
                return 0;
            }
            break;
        }
        case DSD_PORT:
        {
            //TODO: implement
            break;
        }
        default:
            DPRINTF("not available arg on input port\n");
            return 0;
    }

    // set input port
    if (ioctl(audio_fd, AUDIO_IOC_SET_AUDIOINPUT ,&hdmi_audio_mode->inputPort) )
    {
        DPRINTF("error to set input port on audio\n");
        return 0;
    }

    // set audio channel num on audio sample packet and audio infoframe
    if (ioctl(hdmi_fd, HDMI_IOC_SET_AUDIOCHANNEL ,&hdmi_audio_mode->channelNum))
    {
        DPRINTF("error to set audio channel number on HDMI\n");
        return 0;
    }

    // set audio clock recovery packet and audio infoframe sample freq
    if (ioctl(hdmi_fd, HDMI_IOC_SET_AUDIOSAMPLEFREQ  ,&hdmi_audio_mode->sampleFreq))
    {
        DPRINTF("error to set audio sample freq on HDMI\n");
        return 0;
    }

    // get hdmi audio parameters

    if (hdmi_audio_mode->outPacket == HDMI_ASP)
    {
        // reset sampling freq fields in AUI
        if (ioctl(hdmi_fd, HDMI_IOC_RESET_AUISAMPLEFREQ, NULL))
        {
            DPRINTF("fail to reset sampling freq field in AUI InfoFrame\n");
            return 0;
        }
    }

    // set audio packet type
    if (ioctl(hdmi_fd, HDMI_IOC_SET_AUDIOPACKETTYPE ,&hdmi_audio_mode->outPacket))
    {
        DPRINTF("error to set audio packet type on HDMI\n");
        return 0;
    }

	if (hdmi_audio_mode->inputPort == I2S_PORT)
	{
        if (ioctl(audio_fd,AUDIO_IOC_UPDATE_I2S_CUV,NULL))
        {
            DPRINTF("error to update CUV fields\n");
            return 0;
        }
	}
	

    return 1;
}

/**
 * Set speaker allocation information.
 * @param   speaker [in]    Value to set. @n
 *                          For the values, refer CEA-861 Spec.
 * @return If success, return 1;Otherwise, return 0
 */
int HDMISetSpeakerAllocation(const unsigned int speaker)
{
    if ( hdmi_fd == -1 )
    {
        DPRINTF("Open file descriptor first!:hdmi_fd\n");
        return 0;
    }

    if (ioctl(hdmi_fd, HDMI_IOC_SET_SPEAKER_ALLOCATION,&speaker))
    {
        DPRINTF("error to set speaker allocation on HDMI\n");
        return 0;
    }
    return 1;
}

/**
 * Enable/Disable blue screen mode.
 * @param   enable [in]    1 to enable blue screen mode @n
 *                         0 to disable blue screen mode
 * @return If success, return 1;Otherwise, return 0
 */
int HDMISetBlueScreen(const unsigned char enable)
{
    if ( hdmi_fd == -1 )
    {
        DPRINTF("Open file descriptor first!:hdmi_fd\n");
        return 0;
    }

    // set blue screen
    if (ioctl(hdmi_fd,HDMI_IOC_SET_BLUESCREEN, &enable)) {
        DPRINTF("ioctl(HDMI_IOC_SET_BLUESCREEN) failed!\n");
        return 0;
    }
    return 1;
}

/**
 * Set pixel limitation.
 * @param   mode [in]    Pixel limitation infomation of video.
 * @return If success, return 1;Otherwise, return 0
 */
int HDMISetPixelLimit(const enum PixelLimit mode)
{
    if ( hdmi_fd == -1 )
    {
        DPRINTF("Open file descriptor first!:hdmi_fd\n");
        return 0;
    }
    // set pixel limitation
    if (ioctl(hdmi_fd,HDMI_IOC_SET_PIXEL_LIMIT, &mode)) {
        DPRINTF("ioctl(HDMI_IOC_SET_PIXEL_LIMIT) failed!\n");
        return 0;
    }
    return 1;
}

/**
 * Enable HDMI output.
 * @return If success, return 1;Otherwise, return 0
 */
int HDMIStart(void)
{
	unsigned int enable = 1;
    DPRINTF("%s()\n", __FUNCTION__);

    if ( hdmi_fd == -1 )
    {
        DPRINTF("Open file descriptor first!:hdmi_fd\n");
        return 0;
    }
    
    //ioctl(hdmi_fd,HDMI_IOC_STOP_HDMI, NULL);

    if (ioctl(hdmi_fd,HDMI_IOC_SET_AUDIO_ENABLE,&enable)) {
        DPRINTF("ioctl(HDMI_IOC_SET_AUDIO_ENABLE) failed!\n");
        return 0;
    }	

	if (ioctl(audio_fd, AUDIO_IOC_ENABLE_I2S_CLK_CON, NULL)) {
        DPRINTF("ioctl(AUDIO_IOC_ENABLE_I2S_CLK_CON) failed!\n");
        return 0;
	}

    // set hdmi start
    if (ioctl(hdmi_fd,HDMI_IOC_START_HDMI, NULL)) {
        DPRINTF("ioctl(HDMI_IOC_START_HDMI) failed!\n");
        return 0;
    }

    // update UI
    if (ioctl(fb_fd,TCC_LCDC_HDMI_UI_UPDATE, NULL)) {
        DPRINTF("ioctl(TCC_LCDC_HDMI_UI_UPDATE) failed!\n");
        return 0;
    }

    return 1;
}

/**
 * Disable HDMI output.
 * @return If success, return 1;Otherwise, return 0
 */
int HDMIStop(void)
{
	unsigned int enable = 0;
    DPRINTF("%s()\n", __FUNCTION__);

    if ( hdmi_fd == -1 )
    {
        DPRINTF("Open file descriptor first!:hdmi_fd\n");
        return 0;
    }

    if (ioctl(hdmi_fd,HDMI_IOC_SET_AUDIO_ENABLE,&enable)) {
        DPRINTF("ioctl(HDMI_IOC_SET_AUDIO_ENABLE) failed!\n");
        return 0;
    }	

    // set hdmi stop
    if (ioctl(hdmi_fd,HDMI_IOC_STOP_HDMI, NULL)) {
        DPRINTF("ioctl(HDMI_IOC_STOP_HDMI) failed!\n");
        return 0;
    }

	// [I2S_CLK_CON.i2s_en]
	//  You must set i2s_en, after other registers are configured. 
	//  when you want to reset the i2s, this register is 0 1. 
    if (ioctl(audio_fd,AUDIO_IOC_DISABLE_I2S_CLK_CON,NULL))
    {
        DPRINTF("error to set I2S CLK disable\n");
        return 0;
    }

    return 1;
}

int HDMIAudioStart(void)
{
	unsigned int enable = 1;
	DPRINTF("%s()\n", __FUNCTION__);

	if( hdmi_fd == -1)
    {
        DPRINTF("Open file descriptor first!:hdmi_fd\n");
        return 0;
    }

    if (ioctl(hdmi_fd,HDMI_IOC_SET_AUDIO_ENABLE,&enable)) {
        DPRINTF("ioctl(HDMI_IOC_SET_AUDIO_ENABLE) failed!\n");
        return 0;
    }

	if (ioctl(audio_fd, AUDIO_IOC_ENABLE_I2S_CLK_CON, NULL)) {
        DPRINTF("ioctl(AUDIO_IOC_ENABLE_I2S_CLK_CON) failed!\n");
        return 0;
	}

    return 1;

}

int HDMIAudioStop(void)
{
	unsigned int enable = 0;
	DPRINTF("%s()\n", __FUNCTION__);

	if( audio_fd == -1)
	{
		return 0;
	}


	// [I2S_CLK_CON.i2s_en]
	//  You must set i2s_en, after other registers are configured. 
	//  when you want to reset the i2s, this register is 0 1. 
    if (ioctl(audio_fd,AUDIO_IOC_DISABLE_I2S_CLK_CON,NULL))
    {
        DPRINTF("error to set I2S CLK disable\n");
        return 0;
    }

	if( hdmi_fd == -1)
    {
        DPRINTF("Open file descriptor first!:hdmi_fd\n");
        return 0;
    }

    if (ioctl(hdmi_fd,HDMI_IOC_SET_AUDIO_ENABLE,&enable)) {
        DPRINTF("ioctl(HDMI_IOC_SET_AUDIO_ENABLE) failed!\n");
        return 0;
    }

    return 1;

}


/**
 * Enable/Disable audio mute.
 * @param   enable [in]    1 to enable audio mute @n
 *                         0 to disable audio mute
 * @return If success, return 1;Otherwise, return 0
 */
int HDMISetAudioMute(const unsigned char enable)
{
	unsigned char parm;

    if ( hdmi_fd == -1 )
    {
        DPRINTF("Open file descriptor first!:hdmi_fd\n");
        return 0;
    }

	if (enable)	parm = 0;
	else		parm = 1;

    if (ioctl(hdmi_fd,HDMI_IOC_SET_AUDIO_ENABLE,&parm)) {
        DPRINTF("ioctl(HDMI_IOC_SET_AUDIO_ENABLE) failed!\n");
        return 0;
    }
    return 1;
}

/**
 * Enable/Disable A/V mute mode.
 * @param   enable [in]    1 to enable A/V mute mode @n
 *                         0 to disable A/V mute mode
 * @return If success, return 1;Otherwise, return 0
 */
int HDMISetAVMute(const unsigned char enable)
{
    if ( hdmi_fd == -1 )
    {
        DPRINTF("Open file descriptor first!:hdmi_fd\n");
        return 0;
    }

    // set AV mute
    if (ioctl(hdmi_fd,HDMI_IOC_SET_AVMUTE, &enable)) {
        DPRINTF("ioctl(HDMI_IOC_SET_AVMUTE) failed!\n");
        return 0;
    }
    return 1;
}

/**
 * Get HDMI Video Mode Configuration.
 * @param   hdmi_video_mode [out]  video mode
 * @return  If success, return 1; Otherwise, return 0
 */
int HDMIGetVideoMode(struct HDMIVideoParameter *hdmi_video_mode)
{
    // check file
    if (hdmi_fd == -1)
    {
        DPRINTF("HDMI device File is not available\n");
        return 0;
    }

	if (hdmi_video_mode == NULL)
		return 0;

    // set video parameters
    DPRINTF("ioctl(HDMI_IOC_GET_VIDEOCONFIG)\n");
    if (ioctl(hdmi_fd,HDMI_IOC_GET_VIDEOCONFIG, hdmi_video_mode) ) {
        DPRINTF("ioctl(HDMI_IOC_GET_VIDEOCONFIG) failed!\n");
        return 0;
    }

    return 1;
}

/**
 * Get HDMI Running status.
 * @return  If HDMI running(started), return 1; Otherwise, return 0 (include Fail)
 */
int HDMIGetRunStatus(void)
{
	int flag = 0;
	
    // check file
    if (hdmi_fd == -1)
    {
        DPRINTF("HDMI device File is not available\n");
        return 0;
    }

    // set video parameters
    DPRINTF("ioctl(HDMI_IOC_GET_HDMISTART_STATUS)\n");
    if (ioctl(hdmi_fd,HDMI_IOC_GET_HDMISTART_STATUS, &flag) ) {
        DPRINTF("ioctl(HDMI_IOC_GET_HDMISTART_STATUS) failed!\n");
        return 0;
    }

    return flag;
}


/**
 * Get pixel limitation.
 * @param   mode [out]    Pixel limitation infomation of video.
 * @return If success, return 1;Otherwise, return 0
 */
int HDMIGetPixelLimit(enum PixelLimit *mode)
{
    if ( hdmi_fd == -1 )
    {
        DPRINTF("Open file descriptor first!:hdmi_fd\n");
        return 0;
    }

	if (mode == NULL)
		return 0;

    // get pixel limitation
    if (ioctl(hdmi_fd,HDMI_IOC_GET_PIXEL_LIMIT, mode)) {
        DPRINTF("ioctl(HDMI_IOC_GET_PIXEL_LIMIT) failed!\n");
        return 0;
    }

    return 1;
}

/**
 * Get speaker allocation information.
 * @param   speaker [out]    get value @n
 *                          For the values, refer CEA-861 Spec.
 * @return If success, return 1;Otherwise, return 0
 */
int HDMIGetSpeakerAllocation(unsigned int *speaker)
{
    if ( hdmi_fd == -1 )
    {
        DPRINTF("Open file descriptor first!:hdmi_fd\n");
        return 0;
    }

    if (ioctl(hdmi_fd, HDMI_IOC_GET_SPEAKER_ALLOCATION,speaker))
    {
        DPRINTF("error to set speaker allocation on HDMI\n");
        return 0;
    }

    return 1;
}



/**
 * Get requested audio mode.
 * @param   hdmi_audio_mode [out]    Audio mode
 * @return If success, return 1;Otherwise, return 0
 */
int HDMIGetAudioMode(struct HDMIAudioParameter *hdmi_audio_mode)
{
    // check paramter
    if (hdmi_audio_mode == NULL)
    {
        DPRINTF("bad args: hdmi_audio_mode\n");
        return 0;
    }

    // check file
    if (audio_fd == -1)
    {
        DPRINTF("AUDIO device File is not available\n");
        return 0;
    }

    // get input port
    if (ioctl(audio_fd, AUDIO_IOC_GET_AUDIOINPUT ,&hdmi_audio_mode->inputPort) )
    {
        DPRINTF("error to get input port on audio\n");
        return 0;
    }

    if (ioctl(audio_fd, AUDIO_IOC_GET_AUDIOFORMATCODE_INFO ,&hdmi_audio_mode->formatCode) )
    {
        DPRINTF("error to get formatcode on audio\n");
        return 0;
    }

	// getting audio input port parameters
    switch (hdmi_audio_mode->inputPort)
    {
        int wordlength,codingtype;
        case I2S_PORT:
        {
            // set CUV
            // sample freq
            if (ioctl(audio_fd,AUDIO_IOC_GET_I2S_CUV_SET_SAMPLEFREQ,&hdmi_audio_mode->sampleFreq))
            {
                DPRINTF("error to get sample freq on I2S CUV\n");
                return 0;
            }

            // channel number
            if (ioctl(audio_fd,AUDIO_IOC_GET_I2S_CUV_SET_CHANNELNUMBER,&hdmi_audio_mode->channelNum))
            {
                DPRINTF("error to get channel number on I2S CUV\n");
                return 0;
            }

            if (ioctl(audio_fd,AUDIO_IOC_GET_I2S_CUV_SET_CODINGTYPE, &codingtype))
            {
                DPRINTF("error to get coding type on I2S CUV\n");
                return 0;
            }
//Check: don't care
            // word length
            if (codingtype == CUV_LPCM)
            {
                if (ioctl(audio_fd,AUDIO_IOC_GET_I2S_CUV_SET_WORDLENGTH_INFO ,&hdmi_audio_mode->wordLength))
                {
                    DPRINTF("error to get word length info on I2S CUV\n");
                    return 0;
                }
				
            } // if (LPCM)

            if (ioctl(audio_fd,AUDIO_IOC_GET_I2S_PARAMETER,&hdmi_audio_mode->i2sParam))
            {
                DPRINTF("error to get i2s parameters\n");
                return 0;
            }
            break;
        }
        case SPDIF_PORT:
        {
            break;
        }
        case DSD_PORT:
        {
            //TODO: implement
            break;
        }
        default:
            DPRINTF("not available arg on input port\n");
            return 0;
    }

    // get audio channel num on audio sample packet and audio infoframe
    if (ioctl(hdmi_fd, HDMI_IOC_GET_AUDIOCHANNEL ,&hdmi_audio_mode->channelNum))
    {
        DPRINTF("error to get audio channel number on HDMI\n");
        return 0;
    }

    // get audio clock recovery packet and audio infoframe sample freq
    if (ioctl(hdmi_fd, HDMI_IOC_GET_AUDIOSAMPLEFREQ  ,&hdmi_audio_mode->sampleFreq))
    {
        DPRINTF("error to get audio sample freq on HDMI\n");
        return 0;
    }

    // get audio packet type
    if (ioctl(hdmi_fd, HDMI_IOC_GET_AUDIOPACKETTYPE ,&hdmi_audio_mode->outPacket))
    {
        DPRINTF("error to get audio packet type on HDMI\n");
        return 0;
    }

    return 1;
}


/**
 * Get HDMI Power status.
 * @param   pwr_status [out];    1 - power on status, 0 - power off status
 * @return If success, return 1;Otherwise, return 0
 */
int HDMIGetPowerStatus(unsigned int *pwr_status)
{
	unsigned int pwrstatus = 0;
	
    // check file
    if (hdmi_fd == -1)    {
        ALOGE("HDMI device File is not available\n");
        return 0;
    }

    // set video parameters
    if (ioctl(hdmi_fd,HDMI_IOC_GET_PWR_STATUS, &pwrstatus) ) {
	ALOGE("ioctl(HDMI_IOC_GET_PWR_STATUS 0x%x  hdmi_fd:0x%x failed!\n",HDMI_IOC_GET_PWR_STATUS, hdmi_fd);
		
        return 0;
    }

    DPRINTF("ioctl(HDMI_IOC_GET_PWR_STATUS power : %d)\n", pwrstatus);

  *pwr_status = pwrstatus;

    return 1;
}


/**
 * Set HDMI Power control.
 * @param   pwr_cmd [in];    1 - power on, 0 - power off
 * @return If success, return 1;Otherwise, return 0
 */
int HDMISetPowerControl(unsigned int pwr_cmd)
{
    // check file
    if (hdmi_fd == -1)    {
        ALOGE("HDMI device File is not available\n");
        return 0;
    }
 //if(!pwr_cmd)
	 //PHYPowerDown();
 
    // set video parameters
    DPRINTF("ioctl(HDMI_IOC_SET_PWR_CONTROL)\n");
    if (ioctl(hdmi_fd,HDMI_IOC_SET_PWR_CONTROL, &pwr_cmd) ) {
        DPRINTF("ioctl(HDMI_IOC_SET_PWR_CONTROL) failed!\n");
        return 0;
    }

    return 1;
}

int HDMISet5vPowerControl(unsigned int pwr_cmd)
{
    // check file
    if (hdmi_fd == -1)    {
        ALOGE("HDMI device File is not available\n");
        return 0;
    }
	
    // set video parameters
    DPRINTF("ioctl(HDMI_IOC_SET_PWR_V5_CONTROL)\n");
    if (ioctl(hdmi_fd,HDMI_IOC_SET_PWR_V5_CONTROL, &pwr_cmd) ) {
        DPRINTF("ioctl(HDMI_IOC_SET_PWR_V5_CONTROL) failed!\n");
        return 0;
    }

    return 1;
}


void HDMIGetLCDSize(enum VideoFormat videofmt ,unsigned int *width, unsigned int *height)
{	
	unsigned int lcd_width = 0, lcd_height = 0;

	lcd_width = LCDCTimimgParams[videofmt].lpc + 1;
	lcd_height = LCDCTimimgParams[videofmt].flc + 1;

	// double pixel
	if(LCDCTimimgParams[videofmt].dp == 1)
	{
		lcd_width >>= 1;		
	}

	*width = lcd_width;
	*height = lcd_height;
		
}

int HDMISetVideoFormatCtrl(unsigned int HDMIVideoFormat, unsigned int Structure_3D)
{
	struct HDMIVideoFormatCtrl VideoFormatCtrl;

    if ( hdmi_fd == -1 )
    {
        DPRINTF("Open file descriptor first!:hdmi_fd\n");
        return 0;
    }

	VideoFormatCtrl.video_format = HDMIVideoFormat;
	VideoFormatCtrl.structure_3D = Structure_3D;
	VideoFormatCtrl.ext_data_3D = 0;
	
    // get pixel limitation
    if (ioctl(hdmi_fd,HDMI_IOC_VIDEO_FORMAT_CONTROL, &VideoFormatCtrl)) {
        DPRINTF("ioctl(HDMI_IOC_VIDEO_FORMAT_CONTROL) failed!\n");
        return 0;
    }

	return 1;
}




