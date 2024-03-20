/*
 * edid_parser.h
 *
 *  Created on: Feb 4, 2015
 */

#ifndef SRC_EDID_PARSER_H_
#define SRC_EDID_PARSER_H_

#ifdef __cplusplus
extern "C" {
#endif


struct parse_standard_timing{
        unsigned int hactive;
        unsigned int vactive;
        unsigned int aspect;
        unsigned int frame_hz;
};


struct parse_vendor_info{
        char manufacturer_name[4];
        unsigned int Product_id;
        unsigned int Serial;
};

typedef struct {
        
        int edid_done;

        int scdc_ready;
        
	/**
	 * Array to hold all the parsed Detailed Timing Descriptors.
	 */
	dtd_t edid_mDtd[32];

	unsigned int edid_mDtdIndex;
	/**
	 * array to hold all the parsed Short Video Descriptors.
	 */
	shortVideoDesc_t edid_mSvd[128];

	shortVideoDesc_t tmpSvd;

	unsigned int edid_mSvdIndex;
	/**
	 * array to hold all the parsed Short Audio Descriptors.
	 */
	shortAudioDesc_t edid_mSad[128];

	unsigned int edid_mSadIndex;

	/**
	 * A string to hold the Monitor Name parsed from EDID.
	 */
	char edid_mMonitorName[13];

	int edid_mYcc444Support;

	int edid_mYcc422Support;

	int edid_mYcc420Support;

	int edid_mBasicAudioSupport;

	int edid_mUnderscanSupport;

        int edid_mSupport_est640x480p60;
	/**
	 *  If Sink is HDMI 2.0
	 */
	int edid_m20Sink;

	hdmivsdb_t edid_mHdmivsdb;

	hdmiforumvsdb_t edid_mHdmiForumvsdb;

	monitorRangeLimits_t edid_mMonitorRangeLimits;

	videoCapabilityDataBlock_t edid_mVideoCapabilityDataBlock;

	colorimetryDataBlock_t edid_mColorimetryDataBlock;

        hdrstaticmetadata_block_t edid_mHdrstaticmetaDataBlock;

	speakerAllocationDataBlock_t edid_mSpeakerAllocationDataBlock;

        /* 
         * The base edid block has up to 4 dtd, and the extension edid block can have up to 6 dts.
         */        
        dtd_t parse_detailed_timing_dtd[10];
        
        struct parse_standard_timing parse_standard_timing[8];
	struct parse_vendor_info parse_vendor_info;
} sink_edid_t;


int edid_parser( u8 * buffer, sink_edid_t *edidExt, u16 edid_size);

/**
 * Initialise the E-EDID reader
 * reset all internal variables
 * reset reader state
 * prepare I2C master
 * commence reading E-EDID memory from sink
 * @param baseAddr base address of controller
 * @param sfrClock external clock supplied to controller
 * @note: this version only works with 25MHz
 */
int edid_parser_Initialize( u16 sfrClock);
/**
 * @param baseAddr base address of controller
 * @return TRUE if successful
 */
int edid_parser_CeaExtReset( sink_edid_t *edidExt);

/**
 * Parses an EDID block of 128 bytes.
 * Each known group (based on location or CEA tag) is parsed using the constructor of its own datatype (class), called in this function accordingly, this function then adds the new valid data structures into the EDID library.
 * @param baseAddr base address of controller
 * @param buffer a pointer to buffer (of 128 bytes)
 * @return TRUE if successful
 */
int edid_parser_ParseBlock( u8 * buffer, sink_edid_t *edidExt);
/**
 * Parse Data Block data structures listed in the CEA Data Block Collection.
 * It identifies the block type and calls the appropriate class constructor to parse it.
 * It adds the new parsed block (object) to the respective vector arrays in the edid object.
 * @param baseAddr base address of controller
 * @param data a buffer array of bytes (pointer to the start of the block).
 * @return the length of the parsed Data Block in the Collection
 */
int edid_parser_ParseDataBlock( u8 * data, sink_edid_t *edidExt);

void edid_parser_updateYcc420(sink_edid_t *edidExt, u8 Ycc420All, u8 LimitedToYcc420All);

#ifdef __cplusplus
}
#endif


#endif
