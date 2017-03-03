/******************************DOCUMENT*COMMENT***********************************
*D
*D 文件名称            : logmath.cpp
*D
*D 项目名称            : HCCL-LVCSR
*D
*D 版本号              : 1.1.0001
*D
*D 文件描述            :
*D         LogMath Head File 
*D
*D
*D 文件修改记录
*D ------------------------------------------------------------------------------ 
*D 版本号       修改日期       修改人     改动内容
*D ------------------------------------------------------------------------------ 
*D 1.1.0001     2003.09.18             创建文件
*D*******************************************************************************/
#include "comm.h"
#include "logmath.h"

double LOGMath2::Add(const double x,const double y) 
{
	double z=x+y;
	return (z<LOGSMALL)?LOGZERO:z;
}

double LOGMath2::Sub(const double x,const double y) 
{
	double z=x-y;
	return (z<LOGSMALL)?LOGZERO:z;
}

double LOGMath2::Log(const double x) 
{
	ASSERT3(x>=0,"log(%g)",x);
	return (x<=MINLARG)?LOGZERO:log(x);
}

double LOGMath2::Exp(const double x) 
{
	return (x<=MINEARG)?0.0:exp(x);
}

double LOGMath2::LogAdd(const double x,const double y) 
{
	/*// plu 2004.01.05_16:12:58
	const double MINLOGEXP= -log(-LOGZERO);
	*/// plu 2004.01.05_16:12:58
	double tmp=x,diff=y-x;
	
	if (x<y) 
	{ 
		tmp = y; 
		diff= -diff;
	}
	
	if (diff<MINLOGEXP) 
		return (tmp<LOGSMALL)?LOGZERO:tmp;
	
	return tmp+log(1.0+exp(diff));
}

double LOGMath2::LogSub(const double x,const double y) 
{
	/*// plu 2004.01.05_16:13:04
	const double MINLOGEXP= -log(-LOGZERO);
	*/// plu 2004.01.05_16:13:04
	double diff=y-x,z;
	
	if (diff<MINLOGEXP) 
		return  (x<LOGSMALL)?LOGZERO:x;
	
	z = 1.0-exp(diff);
	ASSERT3(z>=0,"log(%g)",z);
	
	return (z<=MINLARG) ? LOGZERO : x+log(z);
}


// 2003.11.25  : add
float LOGMath2::Add(const float x,const float y) 
{
	float z=x+y;
	return (float) ((z<LOGSMALL)?LOGZERO:z);
}

float LOGMath2::Sub(const float x,const float y) 
{
	double z=x-y;
	return (float)((z<LOGSMALL)?LOGZERO:z);
}

float LOGMath2::Log(const float x) 
{
	ASSERT3(x>=0,"log(%g)",x);
	return (float) ((x<=MINLARG)?LOGZERO:log(x));
}

float LOGMath2::Exp(const float x) 
{
	return (float) ((x<=MINEARG)?0.0:exp(x));
}

float LOGMath2::LogAdd(const float x,const float y) 
{
	/*// plu 2004.01.05_16:13:11
	const double MINLOGEXP= -log(-LOGZERO);
	*/// plu 2004.01.05_16:13:11
	double tmp=x,diff=y-x;
	
	if (x<y) 
	{ 
		tmp = y; 
		diff= -diff;
	}
	
	if (diff<MINLOGEXP) 
		return (float) ((tmp<LOGSMALL)?LOGZERO:tmp);
	
	return (float) (tmp+log(1.0+exp(diff)));
}

float LOGMath2::LogSub(const float x,const float y) 
{
	/*// plu 2004.01.05_16:12:50
	const float MINLOGEXP= (float)-log(-LOGZERO);
	*/// plu 2004.01.05_16:12:50
	double diff=y-x,z;
	
	if (diff<MINLOGEXP) 
		return  (float)((x<LOGSMALL)?LOGZERO:x);
	
	z = 1.0-exp(diff);
	ASSERT3(z>=0,"log(%g)",z);
	
	return (float) ((z<=MINLARG) ? LOGZERO : x+log(z));
}
