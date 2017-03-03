/******************************DOCUMENT*COMMENT***********************************
*D
*D �ļ�����            : config.h
*D
*D ��Ŀ����            : HCCL-LVCSR
*D
*D �汾��              : 1.1.0001
*D
*D �ļ�����            :
*D         Read User Config Module Head
*D         The LVCSR Group
*D
*D
*D �ļ��޸ļ�¼
*D ------------------------------------------------------------------------------ 
*D �汾��       �޸�����       �޸���     �Ķ�����
*D ------------------------------------------------------------------------------ 
*D 1.1.0001     2003.09.18             �����ļ�
*D*******************************************************************************/

#ifndef aaaCONFIGURATIONaaa
#define aaaCONFIGURATIONaaa
#include "comm.h"

//typedef struct aConfigEnv;
struct aConfigEnv {
   aConfigEnv *next;
   stringbuf env;
   stringbuf def;
};
  
class Config {
   protected:

      aConfigEnv *envs;
      FILE *fenv;
      char *value;
      bool exam;

      char *GetEnv(char *env);
      
   public:
      // =%d 
      void ReadConfig(char *line,int& num);
      // =%d %d
      void ReadConfig(char *line,int& num1,int& num2);
      // =true/false
      void ReadConfig(char *line,bool& bln);
      // =%g
      void ReadConfig(char *line,float& num);
      // =%g %g
      void ReadConfig(char *line,float& num1,float& num2);
      // =%g %g %g
      void ReadConfig(char *line,float& num1,float& num2,float& num3);
      // =%g,%g %g,%g
      void ReadConfig(char *line,float& n1,float& n2,float& n3,float& n4);
      // =%s
      void ReadConfig(char *line,char *str);
      // =%s %s
      void ReadConfig(char *line,char *str1,char *str2);
      
      Config(int argc,char *argv[],char *head=NULL);
      ~Config(void);
};

#endif
