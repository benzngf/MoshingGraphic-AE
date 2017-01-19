#include "opFlow.h"

PF_Err calcOpticFlow(PF_InData* in_data, PF_LayerDef* source, PF_LayerDef* target, int frameNum, SeqData* seqData) {
	PF_Err err = PF_Err_NONE;
	AEGP_SuiteHandler	suites(in_data->pica_basicP);
	PF_SampPB	 	samp_pb_src, samp_pb_tar;
	samp_pb_src.src = source;
	samp_pb_src.x_radius = 1;
	samp_pb_src.y_radius = 1;
	samp_pb_src.allow_asynch = false;
	
	tempBlock tempB;
	seqData->frames[frameNum].ready = false;
	if (seqData->blockWidth == 0 || seqData->frameWidth == 0 || seqData->frameHeight == 0) return err;
	int tempBlockWidth = seqData->blockWidth*in_data->downsample_x.num / in_data->downsample_x.den;
	//fill in tempB
	PF_Handle hhh = suites.HandleSuite1()->host_new_handle(sizeof(PF_Pixel)*(tempBlockWidth)*(tempBlockWidth));

	tempB.pxs = static_cast<PF_Pixel*>(DH(hhh)); //allocate tempB data
	int searchWidth;
	int startX, startY;
	

	for (int i = seqData->numHBlock*seqData->numVBlock - 1; i >= 0; i--) {
		searchWidth = tempBlockWidth;
		seqData->frames[frameNum].blocks[i].xDisplace = 0;
		seqData->frames[frameNum].blocks[i].yDisplace = 0; //initialize
		
		int t;
		startX = (i%seqData->numHBlock)*tempBlockWidth;
		startY = (i / seqData->numHBlock)*tempBlockWidth;
		for (int yy = 0; yy <  tempBlockWidth; yy++) {
			for (int xx = 0; xx <  tempBlockWidth; xx++) {
				t = xx + yy* tempBlockWidth;
				suites.Sampling8Suite1()->nn_sample(in_data->effect_ref, (startX + xx) << 16, (startY + yy) << 16, &samp_pb_src, &(tempB.pxs[t]));
			}
		}

		//search, iterate in target
		PF_Pixel out;
		samp_pb_tar.x_radius = 1;
		samp_pb_tar.y_radius = 1;
		samp_pb_tar.allow_asynch = false;
		samp_pb_tar.src = target;
		int distortX = 0, distortY = 0;
		int comparePixel = 0;
		int tempDiff[9] = { -1,-1,-1,-1,-1,-1,-1,-1,-1 };
		int minDiff;
		int moveDir;
		int step;
		bool found;
		distortX = 0;
		distortY = 0;
		
		minDiff = 2147483647;
		while (searchWidth >0) {
			step = 0;
			comparePixel = 0;
			for (int k = 0; k < 9; k++) {
					tempDiff[k] = -1;
			}//initialize tempDiff[]
			moveDir = -1;
			found = false;
			while (step < 8) { //just in case iteration doesn't end
				bool errB = false;
				for (int j = 0; j < 9; j++) {
					//comparing a block
					if (tempDiff[j] == -1) {
						tempDiff[j] = 0;
						for (int yy = 0; yy < tempBlockWidth; yy++) {
							for (int xx = 0; xx < tempBlockWidth; xx++) {
								//sampling a pixel
								suites.Sampling8Suite1()->nn_sample(in_data->effect_ref, (startX + searchWidth*dir[j][0] + xx + distortX) << 16, (startY + searchWidth*dir[j][1] + yy + distortY) << 16, &samp_pb_tar, &out);
								int srcP = xx + yy*tempBlockWidth;
								if (tempB.pxs[srcP].alpha > 0 && out.alpha > 0) {
									tempDiff[j] += ABS((int)out.red - (int)tempB.pxs[srcP].red) + ABS((int)out.green - (int)tempB.pxs[srcP].green) + ABS((int)out.blue - (int)tempB.pxs[srcP].blue);
									comparePixel++;
									
								}
							}
						}
						if (comparePixel != 0) {
							tempDiff[j] *= (tempBlockWidth*tempBlockWidth);
							tempDiff[j] /= comparePixel;
							if (minDiff > tempDiff[j]) {
								minDiff = tempDiff[j];
								moveDir = j;
								//if (j == 4) suites.ANSICallbacksSuite1()->sprintf("die", " sd->numOfFrame= ; TimeTmp = ");
							}
							comparePixel = 0;
						}
						
					}
				}

				if (minDiff == tempDiff[4]) {
					//break if found
					found = true;
					break;
				}
				else {
					switch (moveDir) {
					case 0:
						tempDiff[4] = tempDiff[moveDir];
						tempDiff[0] = -1;
						tempDiff[1] = -1;
						tempDiff[2] = -1;
						tempDiff[3] = -1;
						tempDiff[6] = -1;
						break;
					case 1:
						tempDiff[4] = tempDiff[moveDir];
						tempDiff[0] = -1;
						tempDiff[1] = -1;
						tempDiff[2] = -1;
						break;
					case 2:
						tempDiff[4] = tempDiff[moveDir];
						tempDiff[0] = -1;
						tempDiff[1] = -1;
						tempDiff[2] = -1;
						tempDiff[5] = -1;
						tempDiff[8] = -1;
						break;
					case 3:
						tempDiff[4] = tempDiff[moveDir];
						tempDiff[0] = -1;
						tempDiff[3] = -1;
						tempDiff[6] = -1;
						break;
					case 5:
						tempDiff[4] = tempDiff[moveDir];
						tempDiff[2] = -1;
						tempDiff[5] = -1;
						tempDiff[8] = -1;
						break;
					case 6:
						tempDiff[4] = tempDiff[moveDir];
						tempDiff[0] = -1;
						tempDiff[3] = -1;
						tempDiff[6] = -1;
						tempDiff[7] = -1;
						tempDiff[8] = -1;
						break;
					case 7:
						tempDiff[4] = tempDiff[moveDir];
						tempDiff[6] = -1;
						tempDiff[7] = -1;
						tempDiff[8] = -1;
						break;
					case 8:
						tempDiff[4] = tempDiff[moveDir];
						tempDiff[2] = -1;
						tempDiff[5] = -1;
						tempDiff[6] = -1;
						tempDiff[7] = -1;
						tempDiff[8] = -1;
						break;
					default:
						distortX = 0;
						distortY = 0;
						break;
					}
				}
				distortX += dir[moveDir][0] * searchWidth;
				distortY += dir[moveDir][1] * searchWidth;
				moveDir = -1;
				step++;
			}
			searchWidth /= 2;
		}
		if (found) {
			seqData->frames[frameNum].blocks[i].xDisplace = (short)distortX;
			seqData->frames[frameNum].blocks[i].yDisplace = (short)distortY;
			if (tempBlockWidth > 0) {
				int threshold = minDiff* 40 / tempBlockWidth/tempBlockWidth;
				seqData->frames[frameNum].blocks[i].thres = (unsigned short)threshold;
			}else{
				seqData->frames[frameNum].blocks[i].thres = 0;
			}
			found = false;
		}else{
			seqData->frames[frameNum].blocks[i].xDisplace = (short)0;
			seqData->frames[frameNum].blocks[i].yDisplace = (short)0;
			seqData->frames[frameNum].blocks[i].thres = (unsigned short)30598;//(255*3)*40 - 2
		}


	}
	if (tempBlockWidth != seqData->blockWidth) {//scale the output
		for (int i = seqData->numHBlock*seqData->numVBlock - 1; i >= 0; i--) {
			seqData->frames[frameNum].blocks[i].xDisplace = (short)(seqData->frames[frameNum].blocks[i].xDisplace*(long)DOWNX);
			seqData->frames[frameNum].blocks[i].yDisplace = (short)(seqData->frames[frameNum].blocks[i].yDisplace*(long)DOWNX);
		}
	}
	seqData->frames[frameNum].ready = true;
	suites.HandleSuite1()->host_dispose_handle(hhh);
	
	return err;
}

PF_Point getDIstort(PF_InData* in_data, SeqData* seqData, int x, int y, int frameNum, unsigned short threshold) {
	
	PF_Point ret;
	int disX = x;
	int disY = y;
	int blockn;
	if (!seqData->blockWidth) {
		ret.h = x;
		ret.v = y;
		return ret;
	}
	for (int i = frameNum-1; i >= 0; i--) {
		if (seqData->frames[i].ready) {
			if (disX<0 || disY<0 || disX> seqData->blockWidth* seqData->numHBlock || disY> seqData->blockWidth* seqData->numVBlock) break;
			blockn = (disX / seqData->blockWidth) + (disY / seqData->blockWidth)*seqData->numHBlock;
			if (blockn < 0 || blockn >= seqData->numHBlock*seqData->numVBlock) break;
			if (seqData->frames[i].blocks[blockn].thres > threshold) {
				disX = -100;
				disY = -100;
				break;
			}
			disX +=(int) seqData->frames[i].blocks[blockn].xDisplace;
			disY += (int)seqData->frames[i].blocks[blockn].yDisplace;
		}
	}
	ret.h = disX;
	ret.v = disY;
	return ret;
}