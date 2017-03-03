#ifndef __SEGDLLAAAA__
#define __SEGDLLAAAA__

/*********************PARAMETER DESCRIPTION****************************/ 
//	char *szInFile: 输入待分段文件，必须是线性PCM格式，有无头均可，程序不判断头部信息
//	int nSmpRate: 分段文件的采样率，仅支持两种：8000 或 16000，如果采样率设置不正确，程序返回错误 -1.
//	char *szOutDir: 分段结果输出目录，不需要预先创建，程序自动创建
//	float bic_base: 控制BIC合并的基本门限，典型值为 1.5。
//	float bic_step, 控制BIC迭代合并的步长增量，典型值为 0.1。
//	以上两个参数控制了段的零碎情况，值越大，段越少，反之段越多。
//	int winsize: 预分段音框长度，单位为帧，1帧=0.01秒，典型值为100.
//	int winshift: 音框移动步长，单位为帧，1帧=0.01秒，典型值为 10.
//	以上两个参数对分段结果也有较为明显的影响，主要影响语音分割点，如果发现分段结果中语音被切断的情况较为严重，可以考虑更改值，建议：
//	winsize 最小不要小于 50，最大不要超过200.
//	winshift 最小不要小于10，最大不要超过50. 
//
/**********************************************************************/
extern "C" int 
#ifdef WIN32
__declspec(dllexport) 
#endif
SegWav(char *szInFile, int nSmpRate, char *szOutDir, bool bCutSilence, float bic_base, float bic_step, int winsize, int winshift);

/* 返回值说明：
-1： 采样率设置错误
-2： 无法打开待分段文件
0：  语音长度不足以进行分段
>=1: 一共分出了多少段

在 szOutDir 下面会生成 rawsegment.tlist，里面记录了每一段的时间信息，格式为：“段号: 起始时间 结束时间”，时间单位为秒。 
 */

#endif