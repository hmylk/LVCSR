/******************************DOCUMENT*COMMENT***********************************
*D
*D �ļ�����            : FFT.H
*D
*D ��Ŀ����            : HCCL-LVCSR
*D
*D �汾��              : 1.1.0001
*D
*D �ļ�����            :
*D
*D
*D �ļ��޸ļ�¼
*D ------------------------------------------------------------------------------ 
*D �汾��       �޸�����       �޸���     �Ķ�����
*D ------------------------------------------------------------------------------ 
*D 1.1.0001     2004.06.01     plu        �����ļ�
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