/*****************************************************************************/
/*M*
//                        THINKIT INTERNATIONAL PROPRIETARY INFORMATION
//        This software is supplied under the terms of the license agreement
//		or nondisclosure agreement with Thinkit International and may not be copied
//		or disclosed except in accordance with the terms of that agreement.
//            Copyright (c) 2002 ~ 2008 Thinkit International. All Rights Reserved.
//     VSS:
//     $Workfile:: logmath.h                          $
//     $Author:: Ducy                                 $
//     $Revision:: 8                                  $
//     $Modtime:: 9/06/00 5:02p                       $
//     $NoKeywords: $
//
//
//    LogMath Head File 
//
M*/

#ifndef aaaLOG_MATHEMATICaaa
#define aaaLOG_MATHEMATICaaa

#ifndef PI
#define PI 3.14159265428
#endif
#ifndef TPI
#define TPI (2.0*PI)
#define LOGTPI 1.83787706662904515
#endif

//double
#define LOG10      2.30258509299404 // log(10)
#define LOGZERO         IPPSLOGZERO
#define LOGNULL         LOGZERO
#define LOGSMALL        IPPSLOGSMALL
#define MINLOGEXP       IPPSMINLOGEXP 
#define MAXFWDP         IPPSMAXFWDP  
#define MINFWDP         IPPSMINFWDP   
//float
#define LOG10_F       2.302585 // log(10)
#define LOGZERO_F      IPPSLOGZERO_F
#define LOGSMALL_F     IPPSLOGSMALL_F
#define MINLOGEXP_F    IPPSMINLOGEXP_F
#define MAXFWDP_F      IPPSMAXFWDP_F
#define MINFWDP_F      IPPSMINFWDP_F
#define MINLARG        IPPSMINLARG
#define MINMIX         IPPSMINMIX
#define MINEARG        IPPSMINEARG

class ISDTAPI LOGMath {
   public:
 
      static double Log(const double x) {
         ASSERT3(x>=0,"log(%g)",x);
         return (x<=MINLARG)?LOGZERO:log(x);
      }

      static double Exp(const double x) {
         return (x<=MINEARG)?0.0:exp(x);
      }
	  static float LogAdd(const float x,const float y) 
	  {
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
	  
	  static double LogAdd(const double x,const double y) 
	  {
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
};

#endif
