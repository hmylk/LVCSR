/******************************DOCUMENT*COMMENT***********************************
*D
*D 文件名称            : FFT.H
*D
*D 项目名称            : HCCL-LVCSR
*D
*D 版本号              : 1.1.0001
*D
*D 文件描述            :
*D
*D
*D 文件修改记录
*D ------------------------------------------------------------------------------ 
*D 版本号       修改日期       修改人     改动内容
*D ------------------------------------------------------------------------------ 
*D 1.1.0001     2004.06.01     plu        创建文件
*D*******************************************************************************/
#ifndef _FFT_H_
#define _FFT_H_

typedef float real;

#define PI  3.1415926535897932
#define PI8 0.392699081698724 /* PI / 8.0 */
#define RT2 1.4142135623731  /* sqrt(2.0) */
#define IRT2 0.707106781186548  /* 1.0/sqrt(2.0) */

#define signum(i) (i < 0 ? -1 : i == 0 ? 0 : 1)

int fft_pow(float *orig, float *power, long p_fftNum);

int  FAST(real*, int);
void FR2TR(int, real*, real*);
void FR4TR(int, int, real*, real*, real*, real*);
void FORD1(int, real*);
void FORD2(int, real*);
int  fastlog2(int);


#endif // _FFT_H_