/*
  Implementation of MultiBuffer for easy handling of two/three discontinuous buffers
  syq, May 10th, 2010
  You may feel uneasy to handle use the data stored in two or more buffers, as they should be continuous
  So just enjoy this class.
*/


#ifndef _Multi_Buffer_
#define _Multi_Buffer_

// You need to put all templates in the header file.
// There should be no .cpp file for templated classes.

template<class T>
class MultiBuffer {
private:
	int vecsize;
	T * buffer1;
	int size1;
	T * buffer2;
	int size2;
	T * buffer3;
	int size3;
	int size1_2;
	int size1_2_3;
public:

	MultiBuffer(void) {};
	MultiBuffer(int ndim, T *firstbuffer, int firstsize, T *secondbuffer, int secondsize, T *thirdbuffer=NULL, int thirdsize=0);
	int Initialize(int ndim, T *firstbuffer, int firstsize, T *secondbuffer, int secondsize, T *thirdbuffer=NULL, int thirdsize=0);
	~MultiBuffer(void) {};
	T&	operator[](int index); // obsolete
	T*	operator+(int index); // obsolete
	T* get(int index);
	T& get(int index, int index2);
	T* getBuffer(int index, int len, bool &bforcenew, T* (&dest));
	int framenum(void) {return size1_2_3;}
	int framesize(void) {return vecsize;}
};

//extern LogFile *ApiLog;

template <class T>
MultiBuffer<T>::MultiBuffer(int ndim, T *firstbuffer, int firstsize, T *secondbuffer, int secondsize, T *thirdbuffer, int thirdsize)
{
	Initialize(ndim,firstbuffer,firstsize,secondbuffer,secondsize,thirdbuffer,thirdsize);
}

template <class T>
int MultiBuffer<T>::Initialize(int ndim, T *firstbuffer, int firstsize, T *secondbuffer, int secondsize, T *thirdbuffer, int thirdsize)
{
	vecsize = ndim;
	buffer1 = firstbuffer;
	size1   = firstsize;
	buffer2 = secondbuffer;
	size2   = secondsize;
	buffer3 = thirdbuffer;
	size3   = thirdsize;
	size1_2 = size1 + size2;
	size1_2_3 = size1_2 + size3;
	return 0;
}

template <class T>
T& MultiBuffer<T>::operator[](int index)
{
	if (index>=0 && index<size1 && buffer1) return buffer1[index*vecsize];
	else if (index>=size1 && index<size1_2 && buffer2) return buffer2[(index-size1)*vecsize];
	else if (index>=size1_2 && index<size1_2_3 && buffer3) return buffer3[(index-size1_2)*vecsize];
	else if (index<0)
	{
		if (buffer1 && size1>0) return buffer1[0];
		else if (buffer2 && size2>0) return buffer2[0];
		else if (buffer3 && size3>0) return buffer3[0];
		else return 0;
	}
	else if (index>=size1_2_3)
	{
		if (buffer3 && size3>0) return buffer3[(size3-1)*vecsize];
		else if (buffer2 && size2>0) return buffer2[(size2-1)*vecsize];
		else if (buffer1 && size1>0) return buffer1[(size1-1)*vecsize];
		else return 0;
	}
}

template <class T>
T* MultiBuffer<T>::operator+(int index)
{
	if (index>=0 && index<size1 && buffer1) return buffer1+index*vecsize;
	else if (index>=size1 && index<size1_2 && buffer2) return buffer2+(index-size1)*vecsize;
	else if (index>=size1_2 && index<size1_2_3 && buffer3) return buffer3+(index-size1_2)*vecsize;
	else if (index<0)
	{
		if (buffer1 && size1>0) return buffer1;
		else if (buffer2 && size2>0) return buffer2;
		else if (buffer3 && size3>0) return buffer3;
		else return NULL;
	}
	else if (index>=size1_2_3)
	{
		if (buffer3 && size3>0) return buffer3+(size3-1)*vecsize;
		else if (buffer2 && size2>0) return buffer2+(size2-1)*vecsize;
		else if (buffer1 && size1>0) return buffer1+(size1-1)*vecsize;
		else return NULL;
	}
}

template <class T>
T* MultiBuffer<T>::get(int index)
{
	if (index>=0 && index<size1 && buffer1) return buffer1+index*vecsize;
	else if (index>=size1 && index<size1_2 && buffer2) return buffer2+(index-size1)*vecsize;
	else if (index>=size1_2 && index<size1_2_3 && buffer3) return buffer3+(index-size1_2)*vecsize;
	else if (index<0)
	{
		if (buffer1 && size1>0) return buffer1;
		else if (buffer2 && size2>0) return buffer2;
		else if (buffer3 && size3>0) return buffer3;
		else return NULL;
	}
	else if (index>=size1_2_3)
	{
		if (buffer3 && size3>0) return buffer3+(size3-1)*vecsize;
		else if (buffer2 && size2>0) return buffer2+(size2-1)*vecsize;
		else if (buffer1 && size1>0) return buffer1+(size1-1)*vecsize;
		else return NULL;
	}
	else return NULL;
}

template <class T>
T& MultiBuffer<T>::get(int index, int index2)
{
	static T t;
	if (index>=0 && index<size1 && buffer1) return buffer1[index*vecsize+index2];
	else if (index>=size1 && index<size1_2 && buffer2) return buffer2[(index-size1)*vecsize+index2];
	else if (index>=size1_2 && index<size1_2_3 && buffer3) return buffer3[(index-size1_2)*vecsize+index2];
	else if (index<0)
	{
		if (buffer1 && size1>0) return buffer1[index2];
		else if (buffer2 && size2>0) return buffer2[index2];
		else if (buffer3 && size3>0) return buffer3[index2];
		//else return 0;
	}
	else if (index>=size1_2_3)
	{
		if (buffer3 && size3>0) return buffer3[(size3-1)*vecsize+index2];
		else if (buffer2 && size2>0) return buffer2[(size2-1)*vecsize+index2];
		else if (buffer1 && size1>0) return buffer1[(size1-1)*vecsize+index2];
		//else return 0;
	}
	return t;
}

template <class T>
T* MultiBuffer<T>::getBuffer(int index, int len, bool &bforcenew, T* (&dest))
{
	// only support two buffers
	if (index<0 || index>=size1_2_3 || len<=0) return NULL;
	if (bforcenew && dest==NULL)
		dest = new T[len*vecsize];
	if (index>=0 && index<size1 && buffer1)
	{
		if (index+len<=size1)
		{
			if (!bforcenew) return buffer1+index*vecsize;
			else
			{
				memmove(dest, buffer1+index*vecsize, len*vecsize*sizeof(T));
			}
		}
		else if (index+len<=size1_2)
		{
			if (!dest) dest = new T[len*vecsize];
			memmove(dest, buffer1+index*vecsize, (size1-index)*vecsize*sizeof(T));
			memmove(dest+(size1-index)*vecsize, buffer2, (len-size1+index)*vecsize*sizeof(T));
			bforcenew = true;
		}
	}
	else if (index>=size1 && index<size1_2 && buffer2)
	{
		if (index+len<=size1_2)
		{
			if (!bforcenew) return buffer2+(index-size1)*vecsize;
			else
			{
				memmove(dest, buffer2+(index-size1)*vecsize, len*vecsize*sizeof(T));
			}
		}
		else if (index+len<=size1_2_3)
		{
			if (!dest) dest = new T[len*vecsize];
			memmove(dest, buffer2+(index-size1)*vecsize, (size1_2-index)*vecsize*sizeof(T));
			memmove(dest+(size1_2-index)*vecsize, buffer3, (len-size1_2+index)*vecsize*sizeof(T));
			bforcenew = true;
		}
	}
	//else if (index>=size1_2 && index<size1_2_3 && buffer3) return buffer3[(index-size1_2)*vecsize];
	return dest;
}

#endif
