/*******************************************************************/
/*                                                                 */
/*                      ADOBE CONFIDENTIAL                         */
/*                   _ _ _ _ _ _ _ _ _ _ _ _ _                     */
/*                                                                 */
/* Copyright 2007 Adobe Systems Incorporated                       */
/* All Rights Reserved.                                            */
/*                                                                 */
/* NOTICE:  All information contained herein is, and remains the   */
/* property of Adobe Systems Incorporated and its suppliers, if    */
/* any.  The intellectual and technical concepts contained         */
/* herein are proprietary to Adobe Systems Incorporated and its    */
/* suppliers and may be covered by U.S. and Foreign Patents,       */
/* patents in process, and are protected by trade secret or        */
/* copyright law.  Dissemination of this information or            */
/* reproduction of this material is strictly forbidden unless      */
/* prior written permission is obtained from Adobe Systems         */
/* Incorporated.                                                   */
/*                                                                 */
/*******************************************************************/

/*
	Skeleton.h
*/

#pragma once

#ifndef SKELETON_H
#define SKELETON_H

typedef unsigned char		u_char;
typedef unsigned short		u_short;
typedef unsigned short		u_int16;
typedef unsigned long		u_long;
typedef short int			int16;
#define PF_TABLE_BITS	12
#define PF_TABLE_SZ_16	4096

#define PF_DEEP_COLOR_AWARE 1	// make sure we get 16bpc pixels; 
								// AE_Effect.h checks for this.

#include "AEConfig.h"

#ifdef AE_OS_WIN
	typedef unsigned short PixelType;
	#include <Windows.h>
#endif

#include "entry.h"
#include "AE_Effect.h"
#include "AE_EffectCB.h"
#include "AE_Macros.h"
#include "Param_Utils.h"
#include "AE_EffectCBSuites.h"
#include "String_Utils.h"
#include "AE_GeneralPlug.h"
#include "AEFX_ChannelDepthTpl.h"
#include "AEGP_SuiteHandler.h"

#include "MoshingGraphic_Strings.h"

/* Versioning information */

#define	MAJOR_VERSION	1
#define	MINOR_VERSION	0
#define	BUG_VERSION		0
#define	STAGE_VERSION	PF_Stage_BETA
#define	BUILD_VERSION	1


/* Parameter defaults */

#define DOWNX in_data->downsample_x.den / in_data->downsample_x.num
#define DOWNY in_data->downsample_y.den / in_data->downsample_y.num

enum {
	SKELETON_INPUT = 0,
	SKELETON_GAIN,
	SKELETON_COLOR,
	SKELETON_NUM_PARAMS
};

enum {
	GAIN_DISK_ID = 1,
	COLOR_DISK_ID,
};

typedef struct GainInfo{
	PF_FpLong	gainF;
} GainInfo, *GainInfoP, **GainInfoH;

#ifdef __cplusplus
	extern "C" {
#endif
		PF_Err
			CCU_ScaleMatrix(
			PF_FloatMatrix           *mP,
			PF_FpLong                     scaleX,
			PF_FpLong                     scaleY,
			PF_FpLong                     aboutX,
			PF_FpLong                     aboutY);

		PF_Err
			CCU_RotateMatrixPlus(
			PF_FloatMatrix          *matrixP,
			PF_InData                    *in_data,
			PF_FpLong                    degreesF,
			PF_FpLong                    aboutXF,
			PF_FpLong                    aboutYF);
		void CCU_SetIdentityMatrix(
			PF_FloatMatrix *matrixP);

		PF_Err
			CCU_TransformPoints(
			PF_FloatMatrix          *matrixP,
			const PF_Point          *point);


DllExport	PF_Err 
EntryPointFunc(	
	PF_Cmd			cmd,
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output,
	void			*extra) ;

#ifdef __cplusplus
}
#endif

#endif // SKELETON_H