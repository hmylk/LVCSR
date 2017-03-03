/*****************************************************************************/
/*M*
//                        THINKIT INTERNATIONAL PROPRIETARY INFORMATION
//        This software is supplied under the terms of the license agreement
//		or nondisclosure agreement with Thinkit International and may not be copied
//		or disclosed except in accordance with the terms of that agreement.
//            Copyright (c) 2002 ~ 2008 Thinkit International. All Rights Reserved.
//     VSS:
//     $Workfile:: lvcsrcfg.h                         $
//     $Author:: Vit                                  $
//     $Revision:: 9                                  $
//     $Modtime:: 15-01-01 10:50a                     $
//     $NoKeywords: $
//
//
//
M*/

#ifndef LVCSR_CONFIGURATION_H
#define LVCSR_CONFIGURATION_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "arg.h"
#include "isdtexception.h"

#define CFG_PNAME_LEN       256
#define CFG_PVALUE_LEN      256
#define CFG_SNAME_LEN       40
#define CFG_PASS_SEPARATOR  '.'
#define CFG_GLOBAL_NAME     "global"
#define FILE_NAME_LEN       256
#define FILE_STRING_LEN     256

#define UNDEFINED            0
#define CFG_GLOBAL_SECTION   1
#define CFG_GROUP_SECTION    2
#define CFG_TOOL_SECTION     4
#define CFG_TOOLDEF_SECTION  8
#define ENVIROMENT_PARAM     16
#define CMD_LINE_PARAM       32
#define UNDEFINED_OBLIGATORY 64


struct string_less
{
  bool operator() (const char *f, const char *s) const
  { return (strcmp(f, s) < 0);}
};

class LvcsrConfig 
{ 
  struct Parameter 
  { struct  Parameter  *next;
    char    name[CFG_PNAME_LEN];
    char    value[CFG_PVALUE_LEN];
    char    actual_value[CFG_PVALUE_LEN];
    bool    was_retrieved;
    int     type;
  };

  class CfgFile 
  {  char  fileName[FILE_NAME_LEN];
     FILE *fileID;
     char  last_str[FILE_STRING_LEN];
     bool  last_str_flag;
     int   cfgLineCounter;
    
    public:
     void  OpenFile ( char *fname );
     char *GetFileName();
     bool  ReadString (char *str);
          
     void  RestoreLastString ();
     int   GetLineNumber();
     bool  EndOfFile (void);

     CfgFile  ( char *fname );
     ~CfgFile ();
  };
   friend class CfgFile;
   protected:
      char tool_section[CFG_SNAME_LEN];
      char group_section[CFG_SNAME_LEN];
      char tool_pass[CFG_SNAME_LEN];
      FILE *envFile;
      FILE *lvcsrEnvFile;
      std::vector<Parameter> subst; 
      Parameter *plist, *nolist;
      bool exam;

      void Substitute(char *dst, char *src);
      char *GetPValue(char *name);
      Parameter* PListInsertParameter (char *name,char *value,int stype);
      Parameter* PListFindParameter (char *name );
      Parameter* SeeActualParameter (char *name );
      void PListDeleteList(void);
      void AddToNolist(char *name, int type = UNDEFINED, char *defString="");
      bool FileFindNextSection(CfgFile &cf, int &stype);
      bool FileReadParameter (CfgFile &cf, char *name, char *value);
      void StrToSectionName(char *sname, char *str);
      
   public:
      void GetParameter(char *name, int&   num, int   def);
      void GetParameter(char *name, float& num, float def);
      void GetParameter(char *name, bool&  bln, bool  def);
      void GetParameter(char *name, char  *str, char *def);

      // =%d 
      void GetParameter(char *name,int& num);
      // =true/false
      void GetParameter(char *name,bool& bln);
      // =%g
      void GetParameter(char *name,float& num);
      // =%s
      void GetParameter(char *name,char *str);

      // =%d %d %d
      //bool GetParameter(char *name,int& num1,int& num2,int& num3);
      // =%g %g
      //bool GetParameter(char *name,float& num1,float& num2);
      // =%g %g %g
      //bool GetParameter(char *name,float& num1,float& num2,float& num3);
      // =%g,%g %g,%g
      //bool GetParameter(char *name,float& n1,float& n2,float& n3,float& n4);
      // =%s %s
      //bool GetParameter(char *name,char *str1, char *str2);


      void SetNewConfig( char *tool=NULL, char *group=NULL, 
                         char *pass=NULL, char *cfgfile=NULL,
                         std::vector<char *> *cl = NULL);

      LvcsrConfig( char *tool=NULL, char *group=NULL, 
                   char *pass=NULL, char *cfgfile=NULL, 
                   std::vector<char *> *cl = NULL  );
      LvcsrConfig( Arg *arg );

      ~LvcsrConfig(void);
};

#endif
