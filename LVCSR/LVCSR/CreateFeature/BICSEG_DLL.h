#ifndef __SEGDLLAAAA__
#define __SEGDLLAAAA__

/*********************PARAMETER DESCRIPTION****************************/ 
//	char *szInFile: ������ֶ��ļ�������������PCM��ʽ������ͷ���ɣ������ж�ͷ����Ϣ
//	int nSmpRate: �ֶ��ļ��Ĳ����ʣ���֧�����֣�8000 �� 16000��������������ò���ȷ�����򷵻ش��� -1.
//	char *szOutDir: �ֶν�����Ŀ¼������ҪԤ�ȴ����������Զ�����
//	float bic_base: ����BIC�ϲ��Ļ������ޣ�����ֵΪ 1.5��
//	float bic_step, ����BIC�����ϲ��Ĳ�������������ֵΪ 0.1��
//	�����������������˶ε����������ֵԽ�󣬶�Խ�٣���֮��Խ�ࡣ
//	int winsize: Ԥ�ֶ����򳤶ȣ���λΪ֡��1֡=0.01�룬����ֵΪ100.
//	int winshift: �����ƶ���������λΪ֡��1֡=0.01�룬����ֵΪ 10.
//	�������������Էֶν��Ҳ�н�Ϊ���Ե�Ӱ�죬��ҪӰ�������ָ�㣬������ֶַν�����������жϵ������Ϊ���أ����Կ��Ǹ���ֵ�����飺
//	winsize ��С��ҪС�� 50�����Ҫ����200.
//	winshift ��С��ҪС��10�����Ҫ����50. 
//
/**********************************************************************/
extern "C" int 
#ifdef WIN32
__declspec(dllexport) 
#endif
SegWav(char *szInFile, int nSmpRate, char *szOutDir, bool bCutSilence, float bic_base, float bic_step, int winsize, int winshift);

/* ����ֵ˵����
-1�� ���������ô���
-2�� �޷��򿪴��ֶ��ļ�
0��  �������Ȳ����Խ��зֶ�
>=1: һ���ֳ��˶��ٶ�

�� szOutDir ��������� rawsegment.tlist�������¼��ÿһ�ε�ʱ����Ϣ����ʽΪ�����κ�: ��ʼʱ�� ����ʱ�䡱��ʱ�䵥λΪ�롣 
 */

#endif