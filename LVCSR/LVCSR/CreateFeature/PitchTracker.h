////////////////////////////////////////////////////////////////////////
//
//		PitchTracker.h
//
//		Implemented by Li Ming
//		Speech Processing Lab, Institute of Acoustics, Chinese Academy of Sciences, Beijing, P.R.China
//
//		to class pitchtracker change by zliu
//		Change Date:	May. 2006
//
////////////////////////////////////////////////////////////////////////

#ifndef _PITCHTRACKER_H_
#define	_PITCHTRACKER_H_

#include	"rsrfft.h"

#define		Lowest_F0		68	//Hz	should not below 64Hz for 31.5Hz bandwidth
#define		Highest_F0		480//550//650//	//Hz
#define		Max_Harmonics	15	//	maximum harmonics number
#define		Max_Harmonics_Freq	1250	//Hz	the max harmonics frequency being considered
#define		Band_Shift_Range	3	// spectral band shift number, if the bandwidth is below 31.5Hz, it should increase
#define		Harmonics_Factor	0.915F//0.925F//

#define		Voiced_Threshold	0.3F	//	the shreshold to judge if it is voiced or unvoiced
#define		Max_Allowed_Jump_Ratio_For_Voiced	2.45F//1.45F//	// just leave it
#define		Median_Smooth_Range		5	//	median smooth window size
#define		Linear_Smooth_Window_Size	2	//	the actual window size is (Size * 2 + 1)
#define		Acurate_Search_Lag_Range	6
#define		Do_Acurate_Search	0//1//
#define		Pitch_Buf_Size	5	// 1 second buffer corresponds to 1000
#define		Min_Voiced_Gap	10	//	minimum gap between voiced sections, the gap below the constance is regarded as voiced when DP
#define		Back_Voice_Gap	4	//	if the unvoiced section is only Back_Voice_Gap frame from the last voiced frame, it's regarded as voiced when DP
//#define		DEBUG_PitchTracker	1
#define		NUM_F0	80

#ifndef		PI
#define		PI	3.1415926535F
#endif

struct	PitchTrackGroup
{
	float	**sumHarmonics;
	int		*bandPath;
	int		**backTrack;
	float	**score;
	float   *voicedDegree;
	int		*roughF0;
	int		**fullBandRoughF0;	
	float	*hamming_window;
	int		*unvoiceNo;
	float	*normalizedF0;
	float	*lowFreqEmphasisFactor;
	float	*fft;
	short	*signal;
	float	*fftBuffer;
	int		*F0Point;
	float	*weightHarnm;
	float	frameEnergy;
	float	zeroCrossRate;
	float	maxSumHarmn;

	float	harmonicsFactor;	//	factor of harmonics, it maybe sensitive for different database
	int		frameLen;	//	sample number per frame
	int		curIndex;	//	the current index in the buffer
	int		frameIndex;	//	the frame index, it is only sensitive if it is zero or not
	int		unvoiceCnt;	//	the unvoiced frame counter, it will be set zero if a voiced frame comes
	int		stopBackIndex;
};

struct	Section
{
	int	start;
	int	end;
};


class PitchTrack {
private:
	SplitRadixFFT *pFftTool;


public:
	//--------------Interface----------------
		bool Init(char *config);
		int DoUtterance(char *waveFileName, int **pitch);
		int DoUtterance(short *waveData, int numSamples, int **pitch);
		int DoUtterance(short *waveData, int numSamples, int **pitch, float **voicedDegree, bool initPitch = true);
		void close();

//		short *ReadPCM(char *file,int *smpCount);
		
//--------------------wav parameters-----------------	
		int nSampleRate ;
		int nFrameTime ;
		int nFrameShiftTime;
//----------------------------------------------------
//-------------------old col---------------------
		float	*hamWin;

//------------------------------------------------------------
public:
	void	LinearSmoothArray(int *pitch, int numPitch);
	void	MedianSmoothArray1(int *pitch, int numPitch);
	void    LinearSmoothArray(float*voicedDegrees, int numPitch);
	void	MedianSmoothArray(float*voicedDegrees, int numPitch);
	int		GetCurPitch(PitchTrackGroup *tracker, short *wave, int len, float *voicedDegree);
	//int		GetCurPitch(PitchTrackGroup *tracker, short *wave, int len, float *voicedDegree);
	int		InitPitchTracker(PitchTrackGroup *tracker);
	int		InitPitchPath(PitchTrackGroup *tracker);
	void	GetVoicedDegreeByNCCF(PitchTrackGroup *tracker);
	void	SummateHarmonics(PitchTrackGroup *tracker);
	int		GetCurrentScoreAndBackTrace(PitchTrackGroup *tracker);
	int		TraceBackInBuffer(PitchTrackGroup *tracker);
	void	GenerateHammingWindow(PitchTrackGroup *tracker);
	void	SumHarmonics(PitchTrackGroup *tracker);

    PitchTrack(int nSmpRate, int nFrmTime, int nShiftTime){       
		nSampleRate = 10000000/nSmpRate;
		nFrameTime = nFrmTime/10000;
		nFrameShiftTime = nShiftTime/10000;
		
		
	//----------------------------------------------
		nFFTOrder = ((nSampleRate == 16000) ? 9 : 8);			//#define		FFT_ORDER		((SAMPLE_RATE == 16000) ? 9 : 8)
		nNumFFT=(2<<(nFFTOrder-1));								//#define		Num_FFT			(2<<(FFT_ORDER-1))//256
		nFrameLen= (int)(nSampleRate/1000.0F*nFrameTime);		//#define		Frame_Len		(int)(SAMPLE_RATE / 1000.0F * FRAME_TIME)	//point number per frame
		nFrameShit=(int)(nSampleRate/1000.0F*nFrameShiftTime);	//#define		FRAME_SHIFT		(int)(SAMPLE_RATE / 1000.0F * FRAME_SHIFT_TIME)	//point number per frame
		BandWidth=(float)nSampleRate/nNumFFT;					//#define		Bandwidth	((float)SAMPLE_RATE / Num_FFT)
		nStartBand=(int)(Lowest_F0/BandWidth);					//#define		Start_Band	(int)(Lowest_F0 / Bandwidth)	//	F0 start band
		nEndBand=(int)(Highest_F0/BandWidth);					//#define		End_Band	(int)(Highest_F0 / Bandwidth)	//	F0 end band
		FreqStep=1+BandWidth/(float)Max_Harmonics_Freq;			//#define		Freq_Step			(1 + Bandwidth / (float)Max_Harmonics_Freq)
		nNumCorrSamples=(int)(nSampleRate/(Lowest_F0+30));		//#define		Num_Corr_Samples	(int)(SAMPLE_RATE / (Lowest_F0 + 30))	// sample number for correlation
		BasicEnergy=nNumCorrSamples*100.0F*100.0F;				//#define		Basic_Energy	(Num_Corr_Samples * 100.0F * 100.0F)	//	minimum energy of speech sample
		BasicEnergySquare = BasicEnergy*BasicEnergy;			//#define		Basic_Energy_Square		(Basic_Energy * Basic_Energy)	//	used to prevent from the disturbance of periodical noise

		pFftTool = new SplitRadixFFT(nFFTOrder);
		hamWin = new float [nNumFFT];
	//----------------------------init structure PitchTrackGroup-------------
		tracker = new PitchTrackGroup;
		
		tracker->backTrack = new int * [Pitch_Buf_Size];
		int i = 0;
		for(i = 0; i < Pitch_Buf_Size; i++){
			tracker->backTrack[i] = new int [nEndBand];
		}
		tracker->bandPath = new int [Pitch_Buf_Size];
		tracker->F0Point = new int [NUM_F0];
		tracker->fft = new float [nNumFFT/2];
		tracker->fftBuffer = new float [nNumFFT];
		tracker->fullBandRoughF0 = new int *[Pitch_Buf_Size];
		for(i = 0; i < Pitch_Buf_Size; i++){
			tracker->fullBandRoughF0[i] = new int [nEndBand];
		}
		tracker->hamming_window = new float [Linear_Smooth_Window_Size*2+1];
		tracker->lowFreqEmphasisFactor = new float [nNumFFT/2];
		tracker->normalizedF0 = new float [Pitch_Buf_Size];
		tracker->roughF0 = new int [Pitch_Buf_Size];
		tracker->score = new float *[Pitch_Buf_Size];
		for(i = 0; i < Pitch_Buf_Size; i++){
			tracker->score[i] = new float [nEndBand];
		}
		tracker->signal = new short	[nFrameLen];
		tracker->sumHarmonics = new float *[Pitch_Buf_Size];
		for(i = 0; i < Pitch_Buf_Size; i++){
			tracker->sumHarmonics[i] = new float [nEndBand];
		}
		tracker->unvoiceNo = new int [Pitch_Buf_Size];
		tracker->voicedDegree = new float [Pitch_Buf_Size];
		tracker->weightHarnm = new float [Max_Harmonics+1];
	//-----------------------------------------------------------
		pitches = NULL;
		voicedDegrees=NULL;
    }
    ~PitchTrack(){
		
		if(hamWin)
			delete [] hamWin;
		if(pFftTool)
			delete pFftTool;
//---------------------------------------
		int i = 0;
		for(i = 0; i < Pitch_Buf_Size; i++){
			if(tracker->backTrack[i]) 
				delete [] tracker->backTrack[i];
		}
		if(tracker->backTrack)
			delete [] tracker->backTrack;
		if(tracker->bandPath)
			delete [] tracker->bandPath;
		if(tracker->F0Point)
			delete [] tracker->F0Point;
		if(tracker->fft)
			delete [] tracker->fft;
		if(tracker->fftBuffer)
			delete [] tracker->fftBuffer;
		for( i = 0; i < Pitch_Buf_Size; i++){
			if(tracker->fullBandRoughF0[i]) 
				delete [] tracker->fullBandRoughF0[i];
		}		
		if(tracker->fullBandRoughF0)
			delete [] tracker->fullBandRoughF0;
		if(tracker->hamming_window)
			delete [] tracker->hamming_window;
		if(tracker->lowFreqEmphasisFactor)
			delete [] tracker->lowFreqEmphasisFactor;
		if(tracker->normalizedF0)
			delete [] tracker->normalizedF0;
		if(tracker->roughF0)
			delete [] tracker->roughF0;
		for(i = 0; i < Pitch_Buf_Size; i++){
			if(tracker->score[i]) 
				delete [] tracker->score[i];
		}
		if(tracker->score)
			delete [] tracker->score;
		if(tracker->signal)
			delete [] tracker->signal;
		for(i = 0; i < Pitch_Buf_Size; i++){
			if(tracker->sumHarmonics[i]) 
				delete [] tracker->sumHarmonics[i];
		}
		if(tracker->sumHarmonics)
			delete [] tracker->sumHarmonics;
		if(tracker->unvoiceNo)
			delete [] tracker->unvoiceNo;
		if(tracker->voicedDegree)
			delete [] tracker->voicedDegree;
		if(tracker->weightHarnm)
			delete [] tracker->weightHarnm;
		if(tracker)
			delete tracker;
    }
    
private:
	int *pitches;
	float *voicedDegrees; //add by hongmi for plp_ncc
	PitchTrackGroup	*tracker;
//-------------------pitch parameters-------------
//-----------------------------pitch param form cfg--------------
/* there are too many parameters need to be pre-set. I put them here:
      nSampleRate: 16000,    nFrameTime: 25
      nFrameShiftTime:    10      
*/
//------------------------------------------------
	int nFFTOrder;
	int nNumFFT;
	int nFrameLen;
	int	nFrameShit;
	float	BandWidth;
	int		nStartBand;
	int		nEndBand;
	float	FreqStep;
	int		nNumCorrSamples;
	float	BasicEnergy;
	float	BasicEnergySquare;

};
#endif