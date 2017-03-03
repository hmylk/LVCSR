/******************************DOCUMENT*COMMENT***********************************
*D
*D 文件名称            : config.h
*D
*D 项目名称            : HCCL-LVCSR
*D
*D 版本号              : 1.1.0001
*D
*D 文件描述            :
*D         Read User Config Module Head
*D         The LVCSR Group
*D
*D
*D 文件修改记录
*D ------------------------------------------------------------------------------ 
*D 版本号       修改日期       修改人     改动内容
*D ------------------------------------------------------------------------------ 
*D 1.1.0001     2003.09.18             创建文件
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
