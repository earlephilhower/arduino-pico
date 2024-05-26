/**
 *
 * \file
 *
 * \brief WINC Application Interface Internal Types.
 *
 * Copyright (c) 2017 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */
 
#ifndef __ECC_TYPES_H__
#define __ECC_TYPES_H__

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
INCLUDES
*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

#include "m2m_types.h"

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
MACROS
*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/



#define ECC_LARGEST_CURVE_SIZE						(32)
/*!<	
	The size of the the largest supported EC. For now, assuming
	the 256-bit EC is the largest supported curve type.
*/


#define ECC_POINT_MAX_SIZE							ECC_LARGEST_CURVE_SIZE
/*!<
	Maximum size of one coordinate of an EC point.
*/


#define ECC_POINT_MAX_SIZE_WORDS					(ECC_POINT_MAX_SIZE / 4)
/*!<
	SIZE in 32-bit words.
*/

#if 0
#define ECC_NUM_SUPP_CURVES							((sizeof(gastrECCSuppList)) / (sizeof(tstrEllipticCurve)))
#endif
/*!<
*/


/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
DATA TYPES
*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/


/*!
@enum	\
	tenuEcNamedCurve

@brief	EC Named Curves

	Defines a list of supported ECC named curves.
*/
typedef enum EcNamedCurve{
	EC_SECP192R1		= 19,
	/*!<
		It is defined by NIST as P192 and by the SEC Group as secp192r1.
	*/
	EC_SECP256R1		= 23,
	/*!<
		It is defined by NIST as P256 and by the SEC Group as secp256r1.
	*/
	EC_SECP384R1		= 24,
	/*!<
		It is defined by NIST as P384 and by the SEC Group as secp384r1.
	*/
	EC_SECP521R1		= 25,
	/*!<
		It is defined by NIST as P521 and by the SEC Group as secp521r1.
	*/
	EC_UNKNOWN			= 255
}tenuEcNamedCurve;


/*!
@struct	\
	tstrECPoint

@brief	Elliptic Curve point representation	
*/
typedef struct EcPoint{
	uint8	X[ECC_POINT_MAX_SIZE];
	/*!<
		The X-coordinate of the ec point.
	*/
	uint8	Y[ECC_POINT_MAX_SIZE];
	/*!<
		The Y-coordinate of the ec point.
	*/
	uint16	u16Size;
	/*!<
		Point size in bytes (for each of the coordinates).
	*/
	uint16	u16PrivKeyID;
	/*!<
		ID for the corresponding private key.
	*/
}tstrECPoint;


/*!
@struct	\
	tstrECDomainParam
	
@brief	ECC Curve Domain Parameters

	The structure defines the ECC domain parameters for curves defined over prime finite fields.
*/
typedef struct EcDomainParam{
	uint32		p[ECC_POINT_MAX_SIZE_WORDS];
	uint32		a[ECC_POINT_MAX_SIZE_WORDS];
	uint32		b[ECC_POINT_MAX_SIZE_WORDS];
	tstrECPoint	G;
}tstrECDomainParam;


/*!
@struct	\
	tstrEllipticCurve

@brief
	Definition of an elliptic curve
*/
typedef struct{
	tenuEcNamedCurve	enuType;
	tstrECDomainParam	strParam;
}tstrEllipticCurve;


typedef enum{
	ECC_REQ_NONE,
	ECC_REQ_CLIENT_ECDH,
	ECC_REQ_SERVER_ECDH,
	ECC_REQ_GEN_KEY,
	ECC_REQ_SIGN_GEN,
	ECC_REQ_SIGN_VERIFY
}tenuEccREQ;


typedef struct{
	tstrECPoint	strPubKey;
	uint8		au8Key[ECC_POINT_MAX_SIZE];
}tstrEcdhReqInfo;


typedef struct{
	uint32	u32nSig;
}tstrEcdsaVerifyReqInfo;


typedef struct{
	uint16	u16CurveType;
	uint16	u16HashSz;
}tstrEcdsaSignReqInfo;


typedef struct{
	uint16		u16REQ;
	uint16		u16Status;
	uint32		u32UserData;
	uint32		u32SeqNo;
	union{
		tstrEcdhReqInfo			strEcdhREQ;
		tstrEcdsaSignReqInfo	strEcdsaSignREQ;
		tstrEcdsaVerifyReqInfo	strEcdsaVerifyREQ;
	};
}tstrEccReqInfo;


/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
GLOBALS
*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

#if 0
static tstrEllipticCurve	gastrECCSuppList[] = {
	{
		EC_SECP256R1,
		{
			{0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0x00000000, 0x00000000, 0x00000001, 0xFFFFFFFF},
			{0xFFFFFFFC, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0x00000000, 0x00000000, 0x00000001, 0xFFFFFFFF},
			{0x27D2604B, 0x3BCE3C3E, 0xCC53B0F6, 0x651D06B0, 0x769886BC, 0xB3EBBD55, 0xAA3A93E7, 0x5AC635D8},
			{
				{
					0x6B, 0x17, 0xD1, 0xF2, 0xE1, 0x2C, 0x42, 0x47, 0xF8, 0xBC, 0xE6, 0xE5, 0x63, 0xA4, 0x40, 0xF2,
					0x77, 0x03, 0x7D, 0x81, 0x2D, 0xEB, 0x33, 0xA0, 0xF4, 0xA1, 0x39, 0x45, 0xD8, 0x98, 0xC2, 0x96
				},
				{
					0x4F, 0xE3, 0x42, 0xE2, 0xFE, 0x1A, 0x7F, 0x9B, 0x8E, 0xE7, 0xEB, 0x4A, 0x7C, 0x0F, 0x9E, 0x16,
					0x2B, 0xCE, 0x33, 0x57, 0x6B, 0x31, 0x5E, 0xCE, 0xCB, 0xB6, 0x40, 0x68, 0x37, 0xBF, 0x51, 0xF5
				},
				32
			}
		}
	}
};
#endif

/*!<
	List of supported Elliptic Curves ordered by security level (most secure curve is at index ZERO).
*/



#endif /* __ECC_TYPES_H__ */
