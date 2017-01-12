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


static PF_Err 
About (	
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output )
{
	AEGP_SuiteHandler suites(in_data->pica_basicP);
	
	suites.ANSICallbacksSuite1()->sprintf(	out_data->return_msg,
											"Hello, thank you for using my plugin!!\n\nFor any bug reports, questions,\nor any features wanted,\nplease cantact me at\n  benzngf@gmail.com  \nThank you~~");
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
	out_data->out_flags = 0L;
	out_data->out_flags2 = PF_OutFlag2_PARAM_GROUP_START_COLLAPSED_FLAG;
	
	return PF_Err_NONE;
}

enum PARAMS{
	REFLAYER = 1,
	REFTIME,
	LAYERTIME,
	FOWARDCB,
	FOWARDTIME,
	BACKWARDCB,
	BACKWARDTIME,
	BLOCKSIZE,
	APPLYBTN,
	FILLBLANK
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
	PF_STRCPY(def.name, "Motion reference layer");
	def.uu.id = 1;
	def.flags = PF_ParamFlag_CANNOT_TIME_VARY;
	if (err = PF_ADD_PARAM(in_data, -1, &def))
		return err;
	AEFX_CLR_STRUCT(def);

	def.param_type = PF_Param_FLOAT_SLIDER;
	PF_STRCPY(def.name, "Reference i-frame time");
	def.uu.id = 2;
	def.flags = PF_ParamFlag_CANNOT_TIME_VARY;
	def.u.fs_d.precision = 2;
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
	PF_STRCPY(def.name, "Layer i-frame time");
	def.uu.id = 3;
	def.flags = PF_ParamFlag_CANNOT_TIME_VARY;
	def.u.fs_d.precision = 2;
	def.u.fs_d.dephault = 0;
	def.u.fs_d.slider_min = 0;
	def.u.fs_d.slider_max = 20;
	def.u.fs_d.valid_max = 1000;
	def.u.fs_d.valid_min = 0;
	def.u.fs_d.display_flags = 0;
	if (err = PF_ADD_PARAM(in_data, -1, &def))
		return err;
	AEFX_CLR_STRUCT(def);

	def.param_type = PF_Param_CHECKBOX;
	PF_STRCPY(def.name, "");
	def.uu.id = 4;
	def.flags = PF_ParamFlag_CANNOT_TIME_VARY;
	def.u.bd.dephault = true;
	A_char name[10];
	PF_STRCPY(name, "Mosh foward");
	def.u.bd.u.nameptr = name;
	if (err = PF_ADD_PARAM(in_data, -1, &def))
		return err;
	AEFX_CLR_STRUCT(def);

	def.param_type = PF_Param_FLOAT_SLIDER;
	PF_STRCPY(def.name, "Mosh foward time");
	def.uu.id = 5;
	def.flags = PF_ParamFlag_CANNOT_TIME_VARY;
	def.u.fs_d.precision = 2;
	def.u.fs_d.dephault = 1;
	def.u.fs_d.slider_min = 0;
	def.u.fs_d.slider_max = 10;
	def.u.fs_d.valid_max = 20;
	def.u.fs_d.valid_min = 0;
	def.u.fs_d.display_flags = 0;
	if (err = PF_ADD_PARAM(in_data, -1, &def))
		return err;
	AEFX_CLR_STRUCT(def);

	def.param_type = PF_Param_CHECKBOX;
	PF_STRCPY(def.name, "");
	def.uu.id = 6;
	def.flags = PF_ParamFlag_CANNOT_TIME_VARY;
	def.u.bd.dephault = false;
	PF_STRCPY(name, "Mosh backward");
	def.u.bd.u.nameptr = name;
	if (err = PF_ADD_PARAM(in_data, -1, &def))
		return err;
	AEFX_CLR_STRUCT(def);

	def.param_type = PF_Param_FLOAT_SLIDER;
	PF_STRCPY(def.name, "Mosh backward time");
	def.uu.id = 7;
	def.flags = PF_ParamFlag_CANNOT_TIME_VARY;
	def.u.fs_d.precision = 2;
	def.u.fs_d.dephault = 1;
	def.u.fs_d.slider_min = 0;
	def.u.fs_d.slider_max = 10;
	def.u.fs_d.valid_max = 20;
	def.u.fs_d.valid_min = 0;
	def.u.fs_d.display_flags = 0;
	if (err = PF_ADD_PARAM(in_data, -1, &def))
		return err;
	AEFX_CLR_STRUCT(def);

	def.param_type = PF_Param_FLOAT_SLIDER;
	PF_STRCPY(def.name, "Block size");
	def.uu.id = 8;
	def.flags = PF_ParamFlag_CANNOT_TIME_VARY;
	def.u.fs_d.precision = 0;
	def.u.fs_d.dephault = 2;
	def.u.fs_d.slider_min = 2;
	def.u.fs_d.slider_max = 20;
	def.u.fs_d.valid_max = 50;
	def.u.fs_d.valid_min = 2;
	def.u.fs_d.display_flags = 0;
	if (err = PF_ADD_PARAM(in_data, -1, &def))
		return err;
	AEFX_CLR_STRUCT(def);

	def.param_type = PF_Param_BUTTON;
	PF_STRCPY(def.name, "Apply");
	def.u.button_d.u.namesptr = def.name;
	def.flags |= PF_ParamFlag_SUPERVISE;
	def.uu.id = 9;
	def.flags |= PF_ParamFlag_CANNOT_TIME_VARY;
	if (err = PF_ADD_PARAM(in_data, -1, &def))
		return err;
	AEFX_CLR_STRUCT(def);

	enum Popup_2{ FILLREF, FILLTRANS, FILLORIGIN };
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

	out_data->num_params = 11;
	return err;
}

//DEBUG FUNCTION
//END DEBUG FUNCTION

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
	
	if (extra->param_index == APPLYBTN){
		
		out_data->out_flags |= PF_OutFlag_FORCE_RERENDER;
	}
	
	return PF_Err_NONE;
}

PF_Err fillColor(void* refcon, A_long x, A_long y, PF_Pixel *in, PF_Pixel *out) {
	ItData* it = (ItData*)refcon;
	AEGP_SuiteHandler	suites(it->in_data->pica_basicP);
	//int targetX = (int)((float)x * (it->seq_data->frameWidth / (float)(it->in_data->width)));
	//int targetY = (int)((float)y * (it->seq_data->frameHeight / (float)(it->in_data->height)));
	PF_Point d = getDIstort(it->in_data, it->seq_data, x, y, it->currentFrame);
	//d.h = (A_long)(((float)d.h) * ((it->in_data->width) / ((float) it->seq_data->frameWidth)));
	//d.v = (A_long)(((float)d.v) * ((it->in_data->height) / ((float) it->seq_data->frameHeight)));
	
	/*if (x % 50 == 0 && y % 50 == 0) {
		char mesg[500];
		sprintf_s(mesg, "(%d, %d)->(%d, %d)", x, y, d.h, d.v);
		strcat_s(it->mesg, mesg);
	}*/
	suites.Sampling8Suite1()->nn_sample(it->in_data->effect_ref, (d.h)<<16 , (d.v) <<16 , &(it->samp_pb), out);
	
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

	if (curTime - layerTime > sd->numOfFrame) { //we need more frames!

		//suites.ANSICallbacksSuite1()->sprintf(out_data->return_msg,"MORE");
		suites.HandleSuite1()->host_unlock_handle(in_data->sequence_data);
		err = suites.HandleSuite1()->host_resize_handle(sizeof(SeqData)+sizeof(FlowFrame)*(curTime - layerTime), &in_data->sequence_data);
		sd = static_cast<SeqData*> (suites.HandleSuite1()->host_lock_handle(in_data->sequence_data));
		for (int i = sd->numOfFrame; i < (curTime - layerTime); i++) {
			AEFX_CLR_STRUCT(sd->frames[i]);
			sd->frames[i].ready = false; //calculate opflow later!
		}
		sd->numOfFrame = (curTime - layerTime);

	}

	if (sd->startFrame != iTime) { //something bad happened! data must be flushed
		for (int i = 0; i < sd->numOfFrame; i++) {
			sd->frames[i].ready = false;
		}
	}

	//initialize some data
	PF_ParamDef lp;
	err = PF_CHECKOUT_PARAM(in_data, REFLAYER, iTime*in_data->time_step, in_data->time_step, in_data->time_scale, &lp);
	if ( sd->frameWidth != lp.u.ld.width || sd->frameHeight != lp.u.ld.height) {
		sd->frameWidth = lp.u.ld.width;
		sd->frameHeight = lp.u.ld.height;
		for (sd->blockWidth = 1; true; sd->blockWidth *= 2) {
			sd->numHBlock = (sd->frameWidth / sd->blockWidth) + 1;
			sd->numVBlock = (sd->frameHeight / sd->blockWidth) + 1;
			if (sd->numHBlock*sd->numVBlock <= 15000) break;
		}
	}
	err = PF_CHECKIN_PARAM(in_data, &lp);

	sd->ready = true;
	//out_data->sequence_data = in_data->sequence_data;
	//suites.ANSICallbacksSuite1()->sprintf(out_data->return_msg,  "framesetup: %llu",suites.HandleSuite1()->host_get_handle_size(in_data->sequence_data));
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
		suites.ANSICallbacksSuite1()->sprintf(out_data->return_msg, "null sequence_data");
		return PF_Err_NONE;
	}
	else {
		SeqData* sd = static_cast<SeqData*> (suites.HandleSuite1()->host_lock_handle(in_data->sequence_data));
		//suites.ANSICallbacksSuite1()->sprintf(out_data->return_msg, "render: %llu", suites.HandleSuite1()->host_get_handle_size(in_data->sequence_data));
		A_long iTime = (FLOAT2FIX(params[REFTIME]->u.fs_d.value / 65536.0f)); //target i-frame number
		A_long layerTime = (FLOAT2FIX(params[LAYERTIME]->u.fs_d.value / 65536.0f)); //source start frame number
		A_long curTime = in_data->current_time / in_data->time_step; //current frame number

		PF_ParamDef lp1, lp2;
		AEFX_CLR_STRUCT(lp1);
		AEFX_CLR_STRUCT(lp2);
		A_long TimeTmp = (FLOAT2FIX(params[REFTIME]->u.fs_d.value*(in_data->time_step) / 65536.0f));
		bool nor = false;
		//suites.ANSICallbacksSuite1()->sprintf(out_data->return_msg, " sd->numOfFrame=%d ; TimeTmp = %d; numHBlock = %d; numVBlock = %d; blockWidth = %d;", sd->numOfFrame, TimeTmp, sd->numHBlock, sd->numVBlock, sd->blockWidth);
		int previ = -2;
		for (int i = 0; i < curTime; i++) { //calculate optical flow!
			if (sd->frames[i].ready == false) {
				/*if (i >= 1 && previ == i - 1) {
					err = PF_CHECKIN_PARAM(in_data, &lp1);
					lp1 = lp2;
					err = PF_CHECKOUT_PARAM(in_data, REFLAYER, TimeTmp + in_data->time_step*(i + 1), in_data->time_step, in_data->time_scale, &lp2);
				}
				else {
					err = PF_CHECKIN_PARAM(in_data, &lp1);
					err = PF_CHECKIN_PARAM(in_data, &lp2);*/
					err = PF_CHECKOUT_PARAM(in_data, REFLAYER, (TimeTmp + in_data->time_step*(i)), in_data->time_step, in_data->time_scale, &lp1);
					err = PF_CHECKOUT_PARAM(in_data, REFLAYER, (TimeTmp + in_data->time_step*(i + 1)), in_data->time_step, in_data->time_scale, &lp2);
				/*}
				previ = i;
				*/
				err = calcOpticFlow(in_data, &(lp2.u.ld), &(lp1.u.ld), i, sd);
				if (params[FILLBLANK]->u.pd.value == 1) {
					suites.WorldTransformSuite1()->copy_hq(in_data->effect_ref, &lp1.u.ld, output, NULL, &output->extent_hint);
					nor = true;
				}else if (params[FILLBLANK]->u.pd.value == 2){
					suites.WorldTransformSuite1()->copy_hq(in_data->effect_ref, &lp2.u.ld, output, NULL, &output->extent_hint);
					nor = true;
				}
				err = PF_CHECKIN_PARAM(in_data, &lp1);
				err = PF_CHECKIN_PARAM(in_data, &lp2);
				break;
			}
		}
		
		
		

		itData it;
		it.currentFrame = curTime - layerTime;
		it.in_data = in_data;
		it.seq_data = sd;
		it.samp_pb.src = &(params[0]->u.ld);
		it.samp_pb.allow_asynch = false;
		strncpy_s(it.mesg, "", 1500);
		if (!nor) {
			if (it.currentFrame > 0) {
				suites.Iterate8Suite1()->iterate(in_data, 0, 0, &(params[0]->u.ld), NULL, &it, &fillColor, output);
				//PF_Point d = getDIstort(it.in_data, it.seq_data, 10, 11, it.currentFrame);
				//suites.ANSICallbacksSuite1()->sprintf(out_data->return_msg, "(10, 11)->(%d, %d); LayerTime = %d; curTime = %d", d.h, d.v, layerTime, curTime);
				//suites.ANSICallbacksSuite1()->sprintf(out_data->return_msg, it.mesg);
			}
			else {
				//suites.WorldTransformSuite1()->copy_hq(in_data->effect_ref, &(params[0]->u.ld), output, NULL, &output->extent_hint);
			}
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
				err = SeqSetup(in_data,
					out_data,
					params,
					output);
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

