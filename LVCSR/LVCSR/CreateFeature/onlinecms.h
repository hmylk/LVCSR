#ifndef _ONLINE_CMS_
#define _ONLINE_CMS_

#include "MultiBuffer.h"

/*
static float MeanInit[36] = {
(float)-1.383796e+01,(float)-3.751810e+00,(float)-7.258951e+00,(float)-5.939251e+00,
(float)-5.785870e+00,(float)-4.350086e+00,(float)-5.034603e+00,(float)-3.656827e+00,
(float)-4.231513e+00,(float)-3.311993e+00,(float)-2.946019e+00,(float)-2.167453e+00,
(float)1.296424e-01,(float)-4.414594e-02,(float)-3.119627e-01,(float)-2.932176e-01,
(float)-2.459359e-01,(float)-1.727988e-01,(float)-1.926193e-01,(float)-1.331401e-01,
(float)-1.632906e-01,(float)-1.838062e-01,(float)-1.654139e-01,(float)-1.436262e-01,
(float)8.745933e-02,(float)2.255765e-02,(float)2.933322e-02,(float)7.411232e-02,
(float)3.563987e-02,(float)4.835393e-03,(float)3.859975e-02,(float)1.168547e-03,
(float)4.526256e-02,(float)4.336071e-03,(float)1.053885e-03,(float)3.773227e-03
};
static float VarInit[36] = {
(float)3.312206e+01,(float)3.119009e+01,(float)2.894073e+01,(float)3.178028e+01,
(float)3.054670e+01,(float)3.264526e+01,(float)3.366502e+01,(float)3.308675e+01,
(float)3.067490e+01,(float)3.000567e+01,(float)2.682129e+01,(float)2.437693e+01,
(float)2.002656e+00,(float)2.088994e+00,(float)1.900363e+00,(float)2.422328e+00,
(float)2.557916e+00,(float)2.856883e+00,(float)3.037078e+00,(float)3.105734e+00,
(float)3.020520e+00,(float)2.958490e+00,(float)2.698605e+00,(float)2.524496e+00,
(float)3.110689e-01,(float)3.466244e-01,(float)3.408970e-01,(float)4.415134e-01,
(float)4.785867e-01,(float)5.425810e-01,(float)5.746192e-01,(float)6.010971e-01,
(float)5.683724e-01,(float)5.547509e-01,(float)4.959310e-01,(float)4.477568e-01
};
#define CMS_VEC_SIZE 36;
*/

class OnLineCms {
private:
	float *MeanInit, *VarInit; // will be read from chunkfile pjl-0318
	float *iniMean, *iniVar;
	float *newMean, *newVar;
	float * tmpObv;
	float *tmpMean,*tmpVar;
	int   cms_vsz;
	int   resetTimes;
	int   cms_delayNum;
public:
	void ResetInitCmsValue(int delayNum);
	void OnlineCmsCal(float *obv, int vecSize, int vecSize4, int len=1);
	void OnlineCmsCal(MultiBuffer<float> *obv, int vecSize, int vecSize4, int len=1);

	void cmsIterationUpdate(float * obv, int vecSize, int * pCmsUpdateNum);
	void OnlineCmsDelay(float * obv, int vecSize, int vecSize4, int len, int procLen, int * pCmsUpdateNum);

	OnLineCms(int rTimes=1, char *CMS_bin=NULL);
	~OnLineCms(void);

};
#endif
