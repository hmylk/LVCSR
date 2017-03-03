/******************************DOCUMENT*COMMENT***********************************
*D
*D 文件名称            : logmath.h
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

#ifndef aaaLOG_MATHEMATICaaa
#define aaaLOG_MATHEMATICaaa
#include <math.h>

#ifndef PI
#define PI 3.14159265428
#endif
#ifndef TPI
#define TPI (2.0*PI)
#endif

#define LOGZERO         (-1.0e+10)
#define LOGSMALL        (-0.9e+10)
#define LOGNULL         (-2.0e+10)

#define MINEARG         (-708.3)     /* lowest exp() arg  = log(MINLARG) */
#define MINLARG         (2.45E-308)  /* lowest log() arg  = exp(MINEARG) */
#define MINFWDP         10.0         /* minimum forward probability */
#define MINMIX          (-11.5129254649702) /* log(1.0e-5) */

#define MINLOGEXP		-log(-LOGZERO)		// 2004.01.05 plu : add

class LOGMath 
{
public:
	static double Add(const double x,const double y);
	static double Sub(const double x,const double y);
	static double Log(const double x);
	static double Exp(const double x);
	static double LogAdd(const double x,const double y);
	static double LogSub(const double x,const double y);

	// 2003.11.25  : add
	static float Add(const float x,const float y);
	static float Sub(const float x,const float y);
	static float Log(const float x);
	static float Exp(const float x);
	static float LogAdd(const float x,const float y);
	static float LogSub(const float x,const float y);	  
};

#endif
