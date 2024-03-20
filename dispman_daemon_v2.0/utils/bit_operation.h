/*!
* TCC Version 1.0
* Copyright (c) Telechips Inc.
* All rights reserved 
*  \file        bit_operation.h
*  \brief       Dislay Daemon bit-operation header
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


#ifndef BITOPERATION_H_
#define BITOPERATION_H_
#ifdef __cplusplus
extern "C" {
#endif

/**
 * Concatenate two parts of two 8-bit bytes into a new 16-bit word
 * @param bHi first byte
 * @param oHi shift part of first byte (to be place as most significant
 * bits)
 * @param nHi width part of first byte (to be place as most significant
 * bits)
 * @param bLo second byte
 * @param oLo shift part of second byte (to be place as least
 * significant bits)
 * @param nLo width part of second byte (to be place as least
 * significant bits)
 * @returns 16-bit concatenated word as part of the first byte and part of
 * the second byte
 */
u16 concat_bits(u8 bHi, u8 oHi, u8 nHi, u8 bLo, u8 oLo, u8 nLo);

/** Concatenate two full bytes into a new 16-bit word
 * @param hi
 * @param lo
 * @returns hi as most significant bytes concatenated with lo as least
 * significant bits.
 */
u16 byte_to_word(const u8 hi, const u8 lo);

/** Extract the content of a certain part of a byte
 * @param data 8bit byte
 * @param shift shift from the start of the bit (0)
 * @param width width of the desired part starting from shift
 * @returns an 8bit byte holding only the desired part of the passed on
 * data byte
 */
u8 bit_field(const u16 data, u8 shift, u8 width);

/** Concatenate four 8-bit bytes into a new 32-bit word
 * @param b3 assumed as most significant 8-bit byte
 * @param b2
 * @param b1
 * @param b0 assumed as least significant 8bit byte
 * @returns a 2D word, 32bits, composed of the 4 passed on parameters
 */
u32 byte_to_dword(u8 b3, u8 b2, u8 b1, u8 b0);
#ifdef __cplusplus
}
#endif

#endif	/* BITOPERATION_H_ */
