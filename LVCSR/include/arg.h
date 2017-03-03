/*****************************************************************************/
/*M*
//                        THINKIT INTERNATIONAL PROPRIETARY INFORMATION
//        This software is supplied under the terms of the license agreement
//		or nondisclosure agreement with Thinkit International and may not be copied
//		or disclosed except in accordance with the terms of that agreement.
//            Copyright (c) 2002 ~ 2008 Thinkit International. All Rights Reserved.
//     VSS:
//     $Workfile:: arg.h                              $
//     $Author:: Ducy                                 $
//     $Revision:: 7                                  $
//     $Modtime:: 7/28/00 3:35p                       $
//     $NoKeywords: $
//
//
//
//    Read User Config Module
//
M*/
#ifndef LVCSR_ARG_H
#define LVCSR_ARG_H
/* Vargin */
#include <vector>

#define DEFAULT_PASS    ""
#define DEFAULT_CONFIG  "" //"lvcsr.cfg"
#define DEFAULT_PROGNAME  "program"
#define DEFAULT_GROUPNAME  "group"

class Arg {
   
   public:

      char256 pass, cfgFile, workDir, progName, groupName;
      bool exam;
      std::vector<char *> clparam;

      Arg (int argc, char *argv[], char *prog=NULL, char *group=NULL) {
         pass[0]=cfgFile[0]=workDir[0]=progName[0]=groupName[0]='\0';
         exam=false;
         //printf("中科院声学所LVCSR系统.\n");
         for (int i=1; i<argc; i++) {
            if (strrchr(argv[i],'=')) { 
             clparam.push_back(argv[i]);
         } else if (!strcmp(argv[i],"-e")) {
            exam=true;
         } else if (!strcmp(argv[i],"-h")) {
            printf("\nUsage:  %s {-<option> [<parameter>]} {<varname>=<value>} \n\n",argv[0]);
            printf("  -e                     examine configurations\n");
            printf("  -d        <dir>        chdir to a specified dir \n");
            printf("  -config   <file>       read  configuration file\n");
            printf("  -program  <prgname>    set program section name\n");
            printf("  -group    <grpname>    set group section name\n");
            printf("  -pass     <passname>   set pass name\n");
            printf("  <varname>=<value>      set variable value\n");
            printf("  -h                     this help\n\n");
            printf("Please, read \"ISDT Reference Manual\" for more details.\n");
            printf("Compiling date: %s\n\n",__DATE__);
            exit(0);
         } else if (!strcmp(argv[i],"-d")) {
            if (workDir[0]) continue;
            if ( ++i < argc)
             strcpy(workDir,argv[i]);
         } else if (!strcmp(argv[i],"-pass")) {
            if (pass[0]) continue;
            if ( ++i < argc)
               strcpy(pass, argv[i]);
         } else if (strcmp(argv[i],"-config")==0) {
            if (cfgFile[0]) continue;
            if ( ++i < argc)
               strcpy(cfgFile,argv[i]);
         } else if (!strcmp(argv[i],"-program")) {
            if (progName[0]) continue;
            if ( ++i < argc)
               strcpy(progName,argv[i]);
         } else if (!strcmp(argv[i],"-group")) {
            if (groupName[0]) continue;
            if ( ++i < argc)
               strcpy(groupName,argv[i]);
         } 
      }

      
      if (!cfgFile[0]) {
         printf("No configuration file is defined\n");
         printf("Please, ask for usage \"%s -h\"\n",argv[0]);
         exit (0);
      }
      if (!progName[0]) if (prog) strcpy(progName,prog);
      if (!progName[0]) strcpy(progName,DEFAULT_PROGNAME);
      if (!groupName[0]) if (group) strcpy(groupName,group);
      if (!groupName[0]) strcpy(groupName,DEFAULT_GROUPNAME);
      if (workDir[0]) chdir(workDir);
      };
  
      ~Arg(void) {};
};


#endif