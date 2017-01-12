#include "MoshingGraphic.h"

class FlowBlock
{
public:
	FlowBlock(int _width, int startX, int startY, PF_LayerDef& source, PF_LayerDef& target, PF_InData		*in) { //Parent construction function
		for (int i = 0; i < 4; i++) child[i] = nullptr;
		in_data = in;
		width = _width;
		AEGP_SuiteHandler	suites(in_data->pica_basicP);
		//gets the source block
		sourceBlock = (PF_Pixel*)malloc(sizeof(PF_Pixel)*width*width);
		int i = 0;
		PF_SampPB	 	samp_pb;
		samp_pb.src = &source;
		samp_pb.allow_asynch = false;
		for (int yy = 0; yy < width / 2; yy++) {
			for (int xx = 0; xx < width / 2; xx++) {
				i = xx + yy*width;
				suites.Sampling8Suite1()->nn_sample(in_data->effect_ref, startX + xx, startY + yy, &samp_pb, &(sourceBlock[i]));
			}
		}
		//perform search in target block

		//recursive call to child
		for (int i = 0; i < 4; i++) child[i] = new FlowBlock(width / 2, startX + (width / 2)*(i % 2), startY + (width / 2)*(i / 2), sourceBlock, target);
	}
	FlowBlock(int _width, int startX, int startY, PF_Pixel* source, PF_LayerDef& target) {
		if (source != nullptr) {
			sourceBlock = source;
			width = _width;
			AEGP_SuiteHandler	suites(in_data->pica_basicP);
			for (int i = 0; i < 4; i++) child[i] = nullptr;
			//perform search in target block

			//recursive call to child


		}
		else {}//error!
	}
	PF_Point GetDistort(int x, int y) { // local block coordinate
		int block = (x / width) + (y / width) * 2;
		if (block <= 3) {

		}
		else { //error
		}
	}
protected:
	PF_InData		*in_data;
	static int dir[9][2]; //(-1,-1) to (1,1)
private:
	int width;//asume square blocks
	int startX, startY;
	static int maxWidth; //used to get stuff from source block
	PF_Pixel* sourceBlock;
	PF_FpLong average_diff;
	FlowBlock* child[4];
	PF_FpLong search(PF_LayerDef& source, PF_LayerDef& target) {
		PF_Err	 err;
		AEGP_SuiteHandler	suites(in_data->pica_basicP);
		PF_Pixel  out;
		PF_SampPB	 	samp_pb;
		samp_pb.src = &target;
		samp_pb.allow_asynch = false;
		int prevDir = 0;
		for (int i = 0; i < 9; i++) {
			//sampling a pixel
			for (int yy = 0; yy < width / 2; yy++) {
				for (int xx = 0; xx < width / 2; xx++) {
					suites.Sampling8Suite1()->nn_sample(in_data->effect_ref, startX + width*dir[i][0] / 2 + xx, startY + width*dir[i][1] / 2 + yy, &samp_pb, &out);

				}
			}
			//err = PF_AREA_SAMPLE(startX+width*dir[i][0]/2, startY + width*dir[i][1] / 2, &samp_pb, &(out[i]));

		}

	}
};
class FlowFrame {
public:
	FlowFrame(PF_LayerDef& source, PF_LayerDef& target) {

	}
	~FlowFrame() {

	}
	PF_Point GetDistort(PF_Point p) {

	}
private:
	int width, height;
	FlowBlock* blocks; //array
};
