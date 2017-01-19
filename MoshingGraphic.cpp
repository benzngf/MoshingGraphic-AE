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


#include "MoshingGraphic.h"
#include "opFlow.h"
#include <cstdio>
#define	FLOAT2FIX2(F)			((PF_Fixed)((F) * 65536 + (((F) < 0) ? -1 : 1)))

char uuu[20] = " (」⊙ω⊙)」";
char nya[20] = " (／⊙ω⊙)／";

static PF_Err 
About (	
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output )
{
	AEGP_SuiteHandler suites(in_data->pica_basicP);
	
	suites.ANSICallbacksSuite1()->sprintf(	out_data->return_msg,
											"Moshing Graphics version 0.90b by Xquid\n\nFor any bug reports or questions,\nplease contact me at\n  benzngf@gmail.com  \nThank you~~");
	return PF_Err_NONE;
}

static PF_Err 
GlobalSetup (	
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output )
{
	out_data->my_version = PF_VERSION(	MAJOR_VERSION, 
										MINOR_VERSION,
										BUG_VERSION, 
										STAGE_VERSION, 
										BUILD_VERSION);
	out_data->out_flags = PF_OutFlag_WIDE_TIME_INPUT;
	out_data->out_flags2 = PF_OutFlag2_PARAM_GROUP_START_COLLAPSED_FLAG| PF_OutFlag2_AUTOMATIC_WIDE_TIME_INPUT;
	AEGP_SuiteHandler suites(in_data->pica_basicP);
	suites.ANSICallbacksSuite1()->sprintf(out_data->return_msg,
		"This is a Beta version of \"Moshing Graphics AE\", a data-mosh plugin for After Effects.\n Please delete this plugin immediately if you recieved this plugin but weren't a beta-tester, thanks!\n");
	return PF_Err_NONE;
}

enum PARAMS{
	REFLAYER = 1,
	REFTIME,
	LAYERTIME,
	FOWARDTIME,
	AFTERMOSH,
	THRESHOLD,
	FILLBLANK,
	UTILGSTART,
	REFRAMEBTN,
	REMOSHBTN,
	UTILGEND
};

static PF_Err 
ParamsSetup (	
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output )
{
	PF_Err		err		= PF_Err_NONE;
	PF_ParamDef	def;	

	AEFX_CLR_STRUCT(def);

	def.param_type = PF_Param_LAYER;
	PF_STRCPY(def.name, "Motion Reference Layer");
	def.uu.id = 1;
	def.flags = PF_ParamFlag_CANNOT_TIME_VARY| PF_ParamFlag_SUPERVISE;
	if (err = PF_ADD_PARAM(in_data, -1, &def))
		return err;
	AEFX_CLR_STRUCT(def);

	def.param_type = PF_Param_FLOAT_SLIDER;
	PF_STRCPY(def.name, "Reference Strart-Mosh Time");
	def.uu.id = 2;
	def.flags = PF_ParamFlag_CANNOT_TIME_VARY;
	def.u.fs_d.precision = 0;
	def.u.fs_d.dephault = 0;
	def.u.fs_d.slider_min = 0;
	def.u.fs_d.slider_max = 20;
	def.u.fs_d.valid_max = 1000;
	def.u.fs_d.valid_min = 0;
	def.u.fs_d.display_flags = 0;
	if (err = PF_ADD_PARAM(in_data, -1, &def))
		return err;
	AEFX_CLR_STRUCT(def);

	def.param_type = PF_Param_FLOAT_SLIDER;
	PF_STRCPY(def.name, "Layer Strart-Mosh Time");
	def.uu.id = 3;
	def.flags = PF_ParamFlag_CANNOT_TIME_VARY;
	def.u.fs_d.precision = 0;
	def.u.fs_d.dephault = 0;
	def.u.fs_d.slider_min = 0;
	def.u.fs_d.slider_max = 20;
	def.u.fs_d.valid_max = 1000;
	def.u.fs_d.valid_min = 0;
	def.u.fs_d.display_flags = 0;
	if (err = PF_ADD_PARAM(in_data, -1, &def))
		return err;
	AEFX_CLR_STRUCT(def);

	def.param_type = PF_Param_FLOAT_SLIDER;
	PF_STRCPY(def.name, "Mosh Time");
	def.uu.id = 5;
	def.flags = PF_ParamFlag_CANNOT_TIME_VARY;
	def.u.fs_d.precision = 0;
	def.u.fs_d.dephault = 10;
	def.u.fs_d.slider_min = 0;
	def.u.fs_d.slider_max = 300;
	def.u.fs_d.valid_max = 1000;
	def.u.fs_d.valid_min = 0;
	def.u.fs_d.display_flags = 0;
	if (err = PF_ADD_PARAM(in_data, -1, &def))
		return err;
	AEFX_CLR_STRUCT(def);

	enum Popup_1 { REPEAT = 1, NOMORE, HOLD };
	A_char PopupStr_1[100];
	PF_STRCPY(PopupStr_1, "Repeat Mosh|No More Mosh|Hold Mosh");
	def.param_type = PF_Param_POPUP;
	PF_STRCPY(def.name, "After Mosh Time");
	def.uu.id = 6;
	def.u.pd.num_choices = 3;
	def.u.pd.dephault = HOLD;
	def.u.pd.value = def.u.pd.dephault;
	def.u.pd.u.namesptr = PopupStr_1;
	if (err = PF_ADD_PARAM(in_data, -1, &def))
		return err;
	AEFX_CLR_STRUCT(def);
	
	def.param_type = PF_Param_FLOAT_SLIDER;
	PF_STRCPY(def.name, "Threshold");
	def.uu.id = 7;
	def.flags = 0;
	def.u.fs_d.precision = 1;
	def.u.fs_d.dephault = 100;
	def.u.fs_d.slider_min = 0;
	def.u.fs_d.slider_max = 100;
	def.u.fs_d.valid_max = 100;
	def.u.fs_d.valid_min = 0;
	def.u.fs_d.display_flags = PF_ValueDisplayFlag_PERCENT;
	if (err = PF_ADD_PARAM(in_data, -1, &def))
		return err;
	AEFX_CLR_STRUCT(def);


	enum Popup_2{ FILLREF = 1, FILLTRANS, FILLORIGIN };
	A_char PopupStr_2[100];
	PF_STRCPY(PopupStr_2, "reference layer|transparent|original layer");
	def.param_type = PF_Param_POPUP;
	PF_STRCPY(def.name, "Fill blank with");
	def.uu.id = 10;
	def.u.pd.num_choices = 3;
	def.u.pd.dephault = FILLREF;
	def.u.pd.value = def.u.pd.dephault;
	def.u.pd.u.namesptr = PopupStr_2;
	if (err = PF_ADD_PARAM(in_data, -1, &def))
		return err;
	AEFX_CLR_STRUCT(def);

	def.param_type = PF_Param_GROUP_START;
	PF_STRCPY(def.name, "Utility Buttons...");
	def.uu.id = 23;
	def.flags = PF_ParamFlag_START_COLLAPSED;
	//if (params[TIMEINTERCHK]->u.pd.value == false) def.ui_flags = PF_PUI_DISABLED;
	if (err = PF_ADD_PARAM(in_data, -1, &def))
		return err;
	AEFX_CLR_STRUCT(def);

	def.param_type = PF_Param_BUTTON;
	PF_STRCPY(def.name, "Refresh Rendered Frames");
	char refresh1[50] = "refresh";
	def.u.button_d.u.namesptr = refresh1;
	def.flags |= PF_ParamFlag_SUPERVISE;
	def.uu.id = 9;
	def.flags |= PF_ParamFlag_CANNOT_TIME_VARY;
	if (err = PF_ADD_PARAM(in_data, -1, &def))
		return err;
	AEFX_CLR_STRUCT(def);

	def.param_type = PF_Param_BUTTON;
	PF_STRCPY(def.name, "Refresh Calculated Motion");
	def.u.button_d.u.namesptr = refresh1;
	def.flags |= PF_ParamFlag_SUPERVISE;
	def.uu.id = 11;
	def.flags |= PF_ParamFlag_CANNOT_TIME_VARY;
	if (err = PF_ADD_PARAM(in_data, -1, &def))
		return err;
	AEFX_CLR_STRUCT(def);

	def.param_type = PF_Param_GROUP_END;
	def.uu.id = 24;
	if (err = PF_ADD_PARAM(in_data, -1, &def))
		return err;
	AEFX_CLR_STRUCT(def);

	out_data->num_params = 12;
	return err;
}

static PF_Err
ChangeParam(
PF_InData		*in_data,
PF_ParamDef		*params[],
PF_OutData  *out_data,
PF_LayerDef		*output,
PF_UserChangedParamExtra *extra){
	PF_Err				err = PF_Err_NONE;
	AEGP_SuiteHandler suites(in_data->pica_basicP);
	PF_ParamDef	def;
	AEFX_CLR_STRUCT(def);
	if (extra->param_index == REFLAYER || extra->param_index == REMOSHBTN) {
		SeqData* sd = static_cast<SeqData*> (suites.HandleSuite1()->host_lock_handle(in_data->sequence_data));
		sd->refreshLayer = true;
		suites.HandleSuite1()->host_unlock_handle(in_data->sequence_data);
		out_data->out_flags |= PF_OutFlag_FORCE_RERENDER;
		suites.AdvAppSuite2()->PF_InfoDrawText("Motion Data Refreshed", "\\(^⊙▽⊙^)/");
	}
	if (extra->param_index == REFRAMEBTN){
		out_data->out_flags |= PF_OutFlag_FORCE_RERENDER;
		suites.AdvAppSuite2()->PF_InfoDrawText("Rendered Frames Refreshed", "\\(^⊙ω⊙^)/");
	}
	
	return PF_Err_NONE;
}

PF_Err fillColor(void* refcon, A_long x, A_long y, PF_Pixel *in, PF_Pixel *out) {
	ItData* it = (ItData*)refcon;
	PF_InData* in_data = it->in_data;
	AEGP_SuiteHandler	suites(in_data->pica_basicP);
	int targetX = (int)((float)x * (it->seq_data->frameWidth / (float)(in_data->width)*in_data->downsample_x.den/ in_data->downsample_x.num));
	int targetY = (int)((float)y * (it->seq_data->frameHeight / (float)(in_data->height)*in_data->downsample_y.den / in_data->downsample_y.num));
	PF_Point d;
	d = getDIstort(in_data, it->seq_data, targetX, targetY, it->currentFrame, it->threshold);
	if (!(d.h<0 || d.v<0 || d.h>it->seq_data->frameWidth || d.v>it->seq_data->frameHeight)) {
		for (int i = 0; i < it->loopData; i++) {
			d = getDIstort(in_data, it->seq_data, d.h, d.v, it->seq_data->numOfFrame, it->threshold);
			if (d.h<0 || d.v<0 || d.h>it->seq_data->frameWidth || d.v>it->seq_data->frameHeight) break;
			
		}
	}
	if (d.h != -100 && d.v != -100){
		PF_Fixed x_fixed = -100<<16, y_fixed = -100 << 16;
		x_fixed = FLOAT2FIX(((float)d.h) * ((in_data->width)*in_data->downsample_x.num / in_data->downsample_x.den / ((float)it->seq_data->frameWidth)));
		y_fixed = FLOAT2FIX(((float)d.v) * ((in_data->height)*in_data->downsample_y.num / in_data->downsample_y.den / ((float)it->seq_data->frameHeight)));
		suites.Sampling8Suite1()->subpixel_sample(in_data->effect_ref, x_fixed, y_fixed, &(it->samp_pb), out);
	}
	else {
		out->alpha = 0;
	}
	
	return PF_Err_NONE;
}
static PF_Err
SeqSetup(
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output)
{
	AEGP_SuiteHandler	suites(in_data->pica_basicP);
	out_data->sequence_data = suites.HandleSuite1()->host_new_handle(sizeof(SeqData));
	SeqData* sd = (SeqData*)*out_data->sequence_data;
	AEFX_CLR_STRUCT(*sd);
	sd->ready = false;
	sd->numOfFrame = 10;
	for (int i = 0; i < 10; i++) {
		sd->frames[i].ready = false;
	}
	sd->frameWidth = 0;
	sd->frameHeight = 0;
	sd->numHBlock = 0;
	sd->numVBlock = 0;
	sd->startFrame = 0;
	sd->refreshLayer = true;
	return PF_Err_NONE;
}
static PF_Err
SeqSetdown(
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output)
{
	AEGP_SuiteHandler	suites(in_data->pica_basicP);
	suites.HandleSuite1()->host_dispose_handle(in_data->sequence_data);
	return PF_Err_NONE;
}

static PF_Err
Setup(
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output)
{
	PF_Err				err = PF_Err_NONE;
	AEGP_SuiteHandler	suites(in_data->pica_basicP);
	
	SeqData* sd = static_cast<SeqData*> (suites.HandleSuite1()->host_lock_handle(in_data->sequence_data));
	A_long iTime = (FLOAT2FIX(params[REFTIME]->u.fs_d.value/ 65536.0f)); //target i-frame number
	A_long layerTime = (FLOAT2FIX(params[LAYERTIME]->u.fs_d.value/ 65536.0f)); //source start frame number
	A_long curTime = in_data->current_time/in_data->time_step; //current frame number
	A_long moshLength = (FLOAT2FIX(params[FOWARDTIME]->u.fs_d.value / 65536.0f));

	if (moshLength > sd->numOfFrame) { //we need more frames!

		//suites.ANSICallbacksSuite1()->sprintf(out_data->return_msg,"MORE");
		suites.HandleSuite1()->host_unlock_handle(in_data->sequence_data);
		err = suites.HandleSuite1()->host_resize_handle(sizeof(SeqData)+sizeof(FlowFrame)*(moshLength), &in_data->sequence_data);
		sd = static_cast<SeqData*> (suites.HandleSuite1()->host_lock_handle(in_data->sequence_data));
		for (int i = sd->numOfFrame; i < (moshLength); i++) {
			AEFX_CLR_STRUCT(sd->frames[i]);
			sd->frames[i].ready = false; //calculate opflow later!
		}
		sd->numOfFrame = (moshLength);

	}

	if (sd->startFrame != iTime || sd->refreshLayer) { //something bad happened! data must be flushed
		sd->startFrame = iTime;
		for (int i = 0; i < sd->numOfFrame; i++) {
			sd->frames[i].ready = false;
		}
		
	}
	//initialize some data
	if (sd->refreshLayer) {
		PF_ParamDef lp;
		err = PF_CHECKOUT_PARAM(in_data, REFLAYER, iTime*in_data->time_step, in_data->time_step, in_data->time_scale, &lp);
		if (sd->frameWidth != lp.u.ld.width * DOWNX || sd->frameHeight != lp.u.ld.height * DOWNY) {
			sd->frameWidth = lp.u.ld.width * DOWNX;
			sd->frameHeight = lp.u.ld.height * DOWNY;
			for (sd->blockWidth = 1; true; sd->blockWidth *= 2) {
				sd->numHBlock = (sd->frameWidth / sd->blockWidth) + 1;
				sd->numVBlock = (sd->frameHeight / sd->blockWidth) + 1;
				if (sd->numHBlock*sd->numVBlock <= 15000) break;
			}
			char temp[100] = "";
			//sprintf_s(temp, "BlockWidth: %d, (%d*%d)", sd->blockWidth, sd->numHBlock, sd->numVBlock);
			//suites.AdvAppSuite2()->PF_InfoDrawText(temp, "(^⊙ω⊙^)");
		}
		err = PF_CHECKIN_PARAM(in_data, &lp);
		sd->refreshLayer = false;
	}

	sd->ready = true;

	PF_ParamDef *lp1, *lp2;
	lp1 = new PF_ParamDef();
	lp2 = new PF_ParamDef();
	AEFX_CLR_STRUCT(*lp1);
	AEFX_CLR_STRUCT(*lp2);
	A_long TimeTmp = (FLOAT2FIX(params[REFTIME]->u.fs_d.value*(in_data->time_step) / 65536.0f));
	int calcFrame = 0;
	int previ = -2;
	int itdest = (curTime - layerTime > moshLength) ? moshLength: curTime - layerTime;
	for (int i = 0; i < itdest; i++) { //calculate optical flow!
		if (sd->frames[i].ready == false) {
			if (i >= 1 && previ == i - 1) {
				err = PF_CHECKIN_PARAM(in_data, lp1);
				delete lp1;
				lp1 = lp2;
				lp2 = new PF_ParamDef();
				AEFX_CLR_STRUCT(*lp2);
				err = PF_CHECKOUT_PARAM(in_data, REFLAYER, TimeTmp + in_data->time_step*(i + 1), in_data->time_step, in_data->time_scale, lp2);
			}
			else {
				err = PF_CHECKIN_PARAM(in_data, lp1);
				err = PF_CHECKIN_PARAM(in_data, lp2);
				delete lp1;
				delete lp2;
				lp1 = new PF_ParamDef();
				lp2 = new PF_ParamDef();
				AEFX_CLR_STRUCT(*lp1);
				AEFX_CLR_STRUCT(*lp2);
				err = PF_CHECKOUT_PARAM(in_data, REFLAYER, (TimeTmp + in_data->time_step*(i)), in_data->time_step, in_data->time_scale, lp1);
				err = PF_CHECKOUT_PARAM(in_data, REFLAYER, (TimeTmp + in_data->time_step*(i + 1)), in_data->time_step, in_data->time_scale, lp2);
			}
			previ = i;

			err = calcOpticFlow(in_data, &(lp2->u.ld), &(lp1->u.ld), i, sd);
			calcFrame++;
			char out1[100] = "";
			sprintf_s(out1, "Calculated %d Frames", calcFrame);
			//sprintf_s(out2, "frame size: (%d, %d), %dx%d macro blocks", sd->frameWidth, sd->frameHeight, sd->numHBlock, sd->numVBlock);
			suites.AdvAppSuite2()->PF_InfoDrawText(out1, (calcFrame % 2 == 0) ? uuu : nya);
			//break;
		}
	}
	if (calcFrame >= 1) {
		err = PF_CHECKIN_PARAM(in_data, lp1);
		err = PF_CHECKIN_PARAM(in_data, lp2);
		delete lp1;
		delete lp2;
	}

	suites.HandleSuite1()->host_unlock_handle(in_data->sequence_data);
	return err;
}

static PF_Err 
Render(
PF_InData		*in_data,
PF_OutData		*out_data,
PF_ParamDef		*params[],
PF_LayerDef		*output)
{
	PF_Err				err = PF_Err_NONE;
	AEGP_SuiteHandler	suites(in_data->pica_basicP);
	/*	Put interesting code here */
	if (in_data->sequence_data == nullptr) {
		suites.ANSICallbacksSuite1()->sprintf(out_data->return_msg, "Error: null sequence_data");
		suites.WorldTransformSuite1()->copy_hq(in_data->effect_ref, &(params[0]->u.ld), output, NULL, &output->extent_hint);
		return PF_Err_NONE;
	}
	else {
		SeqData* sd = static_cast<SeqData*> (suites.HandleSuite1()->host_lock_handle(in_data->sequence_data));
		
		A_long iTime = (FLOAT2FIX(params[REFTIME]->u.fs_d.value / 65536.0f)); //target i-frame number
		A_long layerTime = (FLOAT2FIX(params[LAYERTIME]->u.fs_d.value / 65536.0f)); //source start frame number
		A_long curTime = in_data->current_time / in_data->time_step; //current frame number
		A_long moshLength = (FLOAT2FIX(params[FOWARDTIME]->u.fs_d.value / 65536.0f));

		itData it;
		switch (params[AFTERMOSH]->u.pd.value) {
		case 1://repeat
			it.currentFrame = (curTime - layerTime) % moshLength;
			it.loopData = (curTime - layerTime) / moshLength;
			break;
		case 2://nomore
			it.currentFrame = (curTime - layerTime > moshLength)? 0 : curTime - layerTime;
			it.loopData = 0;
			break;
		case 3://hold
			it.currentFrame = (curTime - layerTime > moshLength) ? moshLength : curTime - layerTime;
			it.loopData = 0;
			break;
		}


		it.in_data = in_data;
		it.seq_data = sd;
		it.samp_pb.src = &(params[0]->u.ld);
		it.samp_pb.allow_asynch = false;
		it.threshold = (unsigned short)(FLOAT2FIX(params[THRESHOLD]->u.fs_d.value /100.0f * 120.0f / 65536.0f * 255.0f));
		/*char hey[10];
		sprintf_s(hey, "%u", it.threshold);
		suites.AdvAppSuite2()->PF_InfoDrawText("Threshold", hey);*/

		if (it.currentFrame > 0|| it.loopData>0) {
			suites.Iterate8Suite1()->iterate(in_data, 0, 0, &(params[0]->u.ld), NULL, &it, &fillColor, output);
			switch (params[FILLBLANK]->u.pd.value) {
			case 1://ref layer
				PF_ParamDef checkedOutLayer;
				PF_CHECKOUT_PARAM(in_data, REFLAYER, ((iTime+it.currentFrame)*in_data->time_step), in_data->time_step, in_data->time_scale, &checkedOutLayer);
				PF_FloatMatrix transFormMatrix;
				CCU_SetIdentityMatrix(&transFormMatrix);
				PF_FpLong scaleTempX;
				scaleTempX = ((float)(in_data->width) / (float)sd->frameWidth);
				PF_FpLong scaleTempY;
				scaleTempY = ((float)(in_data->height) / (float)sd->frameHeight);
				CCU_ScaleMatrix(&transFormMatrix, scaleTempX, scaleTempY, 0, 0);
				PF_CompositeMode CompMode;
				CompMode.opacity = 255;
				CompMode.xfer = PF_Xfer_BEHIND;
				suites.WorldTransformSuite1()->transform_world(in_data->effect_ref, in_data->quality, PF_MF_Alpha_PREMUL, PF_Field_FRAME,
					&(checkedOutLayer.u.ld), &CompMode, NULL, &transFormMatrix, 1, true, &output->extent_hint, output);
				err = PF_CHECKIN_PARAM(in_data, &checkedOutLayer);
				break;
			case 3: //original layer
				suites.WorldTransformSuite1()->composite_rect(in_data->effect_ref,&(params[0]->u.ld.extent_hint), 255, &(params[0]->u.ld), 0,0,PF_Field_FRAME, PF_Xfer_BEHIND, output);
				break;
			default:
				break;
			}
		}
		else {
			suites.WorldTransformSuite1()->copy_hq(in_data->effect_ref, &(params[0]->u.ld), output, NULL, &output->extent_hint);
		}
		suites.HandleSuite1()->host_unlock_handle(in_data->sequence_data);
	}
	return err;
}


DllExport	
PF_Err 
EntryPointFunc (
	PF_Cmd			cmd,
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output,
	void			*extra)
{
	PF_Err		err = PF_Err_NONE;
	AEGP_SuiteHandler	suites(in_data->pica_basicP);
	try {
		switch (cmd) {
			case PF_Cmd_USER_CHANGED_PARAM:
				err = ChangeParam(in_data,
					params,out_data,
					output, (PF_UserChangedParamExtra*)extra);
				break;

			case PF_Cmd_ABOUT:
				err = About(in_data,
							out_data,
							params,
							output);
				break;
				
			case PF_Cmd_GLOBAL_SETUP:
				err = GlobalSetup(	in_data,
									out_data,
									params,
									output);
				break;
				
			case PF_Cmd_PARAMS_SETUP:
				err = ParamsSetup(	in_data,
									out_data,
									params,
									output);
				break;
			
			case PF_Cmd_SEQUENCE_SETUP:
				err = SeqSetup(in_data,
					out_data,
					params,
					output);
				break;
			case PF_Cmd_SEQUENCE_RESETUP:
				if (in_data->sequence_data == nullptr) {
					err = SeqSetup(in_data,
						out_data,
						params,
						output);
				}
				break;
			/*case PF_Cmd_SEQUENCE_FLATTEN:
				break;*/
			case PF_Cmd_SEQUENCE_SETDOWN:
				err = SeqSetdown(in_data,
					out_data,
					params,
					output);
				break;
			case PF_Cmd_FRAME_SETUP:
				err = Setup(in_data,
					out_data,
					params,
					output);
				break;
			case PF_Cmd_RENDER:
				err = Render(	in_data,
								out_data,
								params,
								output);
				break;
		}
	}
	catch(PF_Err &thrown_err){
		err = thrown_err;
	}
	return err;
}

