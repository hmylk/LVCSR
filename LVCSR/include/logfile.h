/*****************************************************************************/
/*M*
//                        THINKIT INTERNATIONAL PROPRIETARY INFORMATION
//        This software is supplied under the terms of the license agreement
//		or nondisclosure agreement with Thinkit International and may not be copied
//		or disclosed except in accordance with the terms of that agreement.
//            Copyright (c) 2002 ~ 2008 Thinkit International. All Rights Reserved.
//     VSS:
//     $Workfile:: logfile.h                            $
//     $Author:: Savely                               $
//     $Revision:: 8                                  $
//     $Modtime:: 1/05/01 1:20p                       $
//     $NoKeywords: $
//
//     Log file handle module.
M*/
#ifndef _LOGFILE_H
#define _LOGFILE_H


#ifdef WIN32
typedef HANDLE MutexHandel;
#else
typedef pthread_mutex_t MutexHandel;
#endif


class ISDTAPI LogFile{
   private:
      char LogFileName[256];
      FILE *fp;
      int numPrint; //current number of print
      int maximumAccess; // if more print then erase previous log and open new one 
      int logLevel; // logging level
      bool toConsole; // duplicate to console - via printf
	  bool bShowTimeStamp; // add timestamp info in log like tprintf
	   MutexHandel hLogMutex;
      void LogMutex(char* mutexName);
   public:
      int SetLogFile(char *file);
      int SetConsoleMode(bool dup);
      int SetNumberOfAccess(int num);
      int SetLevel(int level);
	  bool SetShowTimeStampInLog(bool flag);

      int logoutfx(char *buf, int level); // print if logLevel >= level
      int logoutf (char *fmt, ...); // standard print:  level = 1
      int logoutf2 (char *fmt, ...); // level = 2 
	  int logoutf3 (char *fmt, ...); // level = 3
      //LogFile(char* name, int maxaccess=400000, int level=1, bool dup=true);
      LogFile(char* name, bool dup=true, bool showtime=false, int maxaccess=40000000, int level=1);
	  /* LogFile* operator=(LogFile* rh)
	  {
	  strcpy(LogFileName,rh->LogFileName);
	  fp = rh->fp;
	  numPrint=rh->numPrint;
	  maximumAccess = rh->maximumAccess;
	  logLevel=rh->logLevel;
	  toConsole = rh->toConsole;
	  bShowTimeStamp = rh->bShowTimeStamp;
	  hLogMutex=rh->hLogMutex;
	  return this;
	  }*/
      LogFile();
      ~LogFile();
};


#endif
