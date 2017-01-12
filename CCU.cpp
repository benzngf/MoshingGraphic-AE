#include "MoshingGraphic.h"

static void
CCU_ConcatMatrix (
                                          const          PF_FloatMatrix          *src_matrixP,
                                          PF_FloatMatrix          *dst_matrixP)
{
          PF_FloatMatrix          tmp;
 
           PF_FpLong          tempF           =          0;
 
             for (register A_long iL = 0; iL < 3; iL++) {
                       for (register A_long jL = 0; jL < 3; jL++) {
                              tempF =          dst_matrixP->mat[iL][0] * src_matrixP->mat[0][jL] +
                              dst_matrixP->mat[iL][1] * src_matrixP->mat[1][jL] +
                              dst_matrixP->mat[iL][2] * src_matrixP->mat[2][jL];
 
                                tmp.mat[iL][jL] = tempF;
                    }
          }
          *dst_matrixP = tmp;
}
 
void
CCU_SetIdentityMatrix (
          PF_FloatMatrix *matrixP)
{
          matrixP->mat[0][0] =
          matrixP->mat[1][1] =
          matrixP->mat[2][2] = 1;
 
          matrixP->mat[0][1] =
          matrixP->mat[0][2] =
          matrixP->mat[1][0] =
          matrixP->mat[1][2] =
          matrixP->mat[2][0] =
          matrixP->mat[2][1] = 0;
}
 
PF_Err
CCU_ScaleMatrix (
          PF_FloatMatrix           *mP,
          PF_FpLong                     scaleX,
          PF_FpLong                     scaleY,
          PF_FpLong                     aboutX,
          PF_FpLong                     aboutY )
{
          PF_Err                              err = PF_Err_NONE;
 
          PF_FloatMatrix          scale;
 
          if (scaleX != 1.0 || scaleY != 1.0)
          {
 
                    scale.mat[0][0] = scaleX; 
                    scale.mat[0][1] = 0; 
                    scale.mat[0][2] = 0;
 
                    scale.mat[1][0] = 0;
                    scale.mat[1][1] = scaleY;
                    scale.mat[1][2] = 0;
 
                    scale.mat[2][0] = (1.0 - scaleX) * (aboutX);
                    scale.mat[2][1] = (1.0 - scaleY) * (aboutY);
                    scale.mat[2][2] = 1;
 
                    CCU_ConcatMatrix(&scale, mP);
          }
          return err;
}
 
PF_Err
CCU_RotateMatrixPlus (
          PF_FloatMatrix          *matrixP,
          PF_InData                    *in_data,
          PF_FpLong                    degreesF,
          PF_FpLong                    aboutXF,
          PF_FpLong                    aboutYF)
{
          AEGP_SuiteHandler          suites(in_data->pica_basicP);
 
          PF_Err                              err                     = PF_Err_NONE;
 
          PF_FloatMatrix          rotate;
 
          PF_FpLong                    radiansF          =          0,
                                                  sF                               =           0,
                                                  cF                               =           0;
 
          if (degreesF){
                    radiansF           =            PF_RAD_PER_DEGREE * degreesF;
                    sF                              =           suites.ANSICallbacksSuite1()->sin(radiansF);
                    cF                              =           suites.ANSICallbacksSuite1()->cos(radiansF);
 
                    rotate.mat[0][0] =          cF;
                    rotate.mat[0][1] =          sF;
                    rotate.mat[0][2] =          0;
 
                    rotate.mat[1][0] =          -sF;
                    rotate.mat[1][1] =          cF;
                    rotate.mat[1][2] =          0;
 
                    rotate.mat[2][0] =          (aboutXF * (1.0 - cF) + aboutYF * sF);
                    rotate.mat[2][1] =          (aboutYF * (1.0 - cF) - aboutXF * sF);
                    rotate.mat[2][2] =          1;
 
                    CCU_ConcatMatrix(&rotate, matrixP);
          }
          return err;
}

PF_Err
CCU_TransformPoints(
PF_FloatMatrix          *matrixP,
const PF_Point          *point)
{
	PF_FloatMatrix transform;
	transform.mat[0][0] = 1;
	transform.mat[0][1] = 0;
	transform.mat[0][2] = 0;

	transform.mat[1][0] = 0;
	transform.mat[1][1] = 1;
	transform.mat[1][2] = 0;

	transform.mat[2][0] = point->h;
	transform.mat[2][1] = point->v;
	transform.mat[2][2] = 1;

	CCU_ConcatMatrix(&transform, matrixP);

	return PF_Err_NONE;
}