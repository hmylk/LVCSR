
#ifndef  __HANDLEFILE__H
#define  __HANDLEFILE__H

#include <stdio.h>
#ifdef WIN32
#pragma warning(disable:4996)
#include <windows.h>
#endif
#include<iostream>
#include <string>
#include <cstdio>
#include <vector>
using namespace std;

namespace THINKIT
{
	class  READ_FILE
	{
	public:
		READ_FILE(FILE* aFileTemp)
		{
			if (aFileTemp!=NULL)
			{
				p_fp = aFileTemp;
			}
			else
			{
				p_fp = NULL;
			}
		}
		unsigned int GetHeadSize();

	private:
		void freadOrDie (void * ptr, size_t size, size_t count);
		// ----------------------------------------------------------------------------
		// fgetbyte(): read a byte value
		// ----------------------------------------------------------------------------
		char fgetbyte ();

		// ----------------------------------------------------------------------------
		// fgetshort(): read a short value
		// ----------------------------------------------------------------------------
		short fgetshort ();


		// ----------------------------------------------------------------------------
		// fgetint(): read an int value
		// ----------------------------------------------------------------------------
		int fgetint ();
		string fgetTag ();

		bool fcheckTag (const char * expectedTag);
		bool fcompareTag (const string & readTag, const string & expectedTag);
		bool fcompareTagNoExit(const string & readTag, const string & expectedTag);
		bool fcheckTagNoExit( const char * expectedTag);
		FILE * p_fp;
	};
};
#endif