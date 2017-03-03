/////////////////////////////////////////////////////////////////////////////////////////
//
//    PitchTracker.h
//
//    Implemented by Li Ming
//    Speech Processing Lab, Institute of Acoustics, Chinese Academy of Sciences, Beijing, P.R.China
//
//    to class pitchtracker change by zliu
//    Change Date:  May. 2006
//
/////////////////////////////////////////////////////////////////////////////////////////

//#include "stdafx.h"

//#include "readwave.h"
#include "config.h"

#include  <stdio.h>
#include  <assert.h>
#include  <stdlib.h>
#include  <malloc.h>
#include  <math.h>
#include  <string.h>
#include  "rsrfft.h"
#include  "PitchTracker.h"


void  PitchTrack::SumHarmonics(PitchTrackGroup *tracker) {
  int   band, noHarmonics;
  int   F0, F0Index, i;
  float *sumHarF0;
  float *sumHarF0_ori;
  int   *argmaxF0;
  float   *fftSum;
  short *wave;
//--------------------defined to cfg--------------
  sumHarF0 = new float [Highest_F0];
  sumHarF0_ori = new float [Highest_F0];
  argmaxF0 = new  int [nEndBand];
  fftSum = new float [nNumFFT / 2];
//--------------------------------------------------

  wave = new short [tracker->frameLen];
  memcpy(wave, tracker->signal, sizeof(short) * tracker->frameLen);

  //  get the summmation of harmonics
  for (F0 = 0; F0 < Highest_F0; F0++) {
    sumHarF0[F0] = 0;
    sumHarF0_ori[F0] = 0;
  }

  fftSum[0] = tracker->fft[0];
  for (i = 1; i < nNumFFT / 2; i++) {
    fftSum[i] = fftSum[i - 1] + tracker->fft[i];
  }

  int harmnIdx, harmnFreq;
  int low, high;
  float avgE;

  for (F0Index = 0; F0Index < NUM_F0; F0Index++) {
    F0 = tracker->F0Point[F0Index];
    for (noHarmonics = 1; noHarmonics <= Max_Harmonics; noHarmonics++) {
      if (F0 * noHarmonics > Max_Harmonics_Freq)
        break;

      harmnFreq = F0 * noHarmonics;
      harmnIdx = (int)(harmnFreq / BandWidth + 0.5F);
      low = (int)((harmnFreq - F0 * 2 / 3) / BandWidth) - 1;
      high = (int)((harmnFreq + F0 * 2 / 3) / BandWidth);
      low = (low < 1) ? 1 : low;
      high = (high < nNumFFT / 2) ? high : nNumFFT / 2;
      avgE = (fftSum[high] - fftSum[low]) / (high - low);
      assert((high - low) != 0);

      sumHarF0[F0] += tracker->weightHarnm[noHarmonics] * (tracker->fft[harmnIdx] / avgE);
      sumHarF0_ori[F0] += tracker->weightHarnm[noHarmonics] * tracker->fft[harmnIdx];
    }
    //  float cor;
    //  cor = NormalizedCorrelation(tracker, wave, F0)*(0.5F+0.1F*nSampleRate/F0);
    //  if (cor < 0.1F) cor = 0.1F; else  cor = 0.2F;
    //  sumHarF0[F0] *= cor;
  }
  //  store the best pitch candidate for each band
  for (band = nStartBand; band < nEndBand; band++) {
    argmaxF0[band] = 0;
    float max = -1;
    for (F0 = (int)(band * BandWidth + 0.5F); F0 < (int)((band + 1) * BandWidth + 0.5F) && F0 < Highest_F0; F0++) {
      if (F0 < Lowest_F0)
        continue;
      if (sumHarF0[F0] > max) {
        max = sumHarF0[F0];
        argmaxF0[band] = F0;
        tracker->fullBandRoughF0[tracker->curIndex][band] = F0;
      }
    }
    assert(max >= 0);
    tracker->sumHarmonics[tracker->curIndex][band] = max;
  }
  delete []wave;

  float *sumBHarmn;
  int   frame, idx, midIdx;
//------------------------------------
  sumBHarmn = new float [nEndBand];
  for (band = nStartBand; band < nEndBand; band++) {
    sumBHarmn[band] = 0;

    for (frame = -5; frame <= 0; frame++) {
      idx = tracker->curIndex + frame;
      idx = (idx + Pitch_Buf_Size) % Pitch_Buf_Size;
      assert(idx >= 0 && idx < Pitch_Buf_Size);
      sumBHarmn[band] += tracker->sumHarmonics[idx][band];
    }
  }

  midIdx = tracker->curIndex - 2;
  midIdx = (midIdx + Pitch_Buf_Size) % Pitch_Buf_Size;

  float max = -1;
  for (band = nStartBand; band < nEndBand; band++) {
    if (sumBHarmn[band] > max) {
      max = sumBHarmn[band];
      tracker->roughF0[midIdx] = tracker->fullBandRoughF0[midIdx][band];
      tracker->roughF0[tracker->curIndex] = tracker->roughF0[midIdx];
    }
  }
  max = -1;
  for (F0 = 0; F0 < Highest_F0; F0++) {
    if (sumHarF0_ori[F0] > max)
      max = sumHarF0_ori[F0];
  }
  tracker->maxSumHarmn = max;
//-------------------------------------------
  delete []sumHarF0;
  delete []sumHarF0_ori;
  delete []fftSum;
  delete []argmaxF0;
  delete []sumBHarmn;
}

int PitchTrack::GetCurPitch(PitchTrackGroup *tracker, short *wave, int len, float *voicedDegree) {
  float *fft_buf;
  float re, im;
  int   i, curPitch;
  float energy, crossRate;

  fft_buf = tracker->fftBuffer;
  assert(len <= nNumFFT);

  for (i = 0; i < len; i++) {
    tracker->signal[i] = wave[i];
    fft_buf[i] = wave[i];
  }
  for (i = len; i < nNumFFT; i++) {
    fft_buf[i] = 0;
  }

  // pre-emphasize
  for (i = len - 1; i > 0; i--)  fft_buf[i] -= fft_buf[i - 1] * 0.97F;

  // hamming window
  for (i = 0; i < len; i++) fft_buf[i] *= hamWin[i];

//  SplitRadixFFT fftTool(nFFTOrder);
  pFftTool->XForm(fft_buf);
  fft_buf[0] *= fft_buf[0];
  for (i = 1; i < nNumFFT / 2; i++) {
    re = fft_buf[i];
    im = fft_buf[nNumFFT - i];
    fft_buf[i] = re * re + im * im;
  }
  for (i = 0; i < nNumFFT / 2; i++) {
    tracker->fft[i] = fft_buf[i];
  }
  energy = 0;
  crossRate = 0;
  for (i = 0; i < len - 1; i++) {
    energy = energy + ((float)wave[i] * wave[i]);
    crossRate = crossRate + ((int)wave[i] * wave[i + 1] < 0);
  }
  energy = (float)log10(energy + 1) * 10;
  crossRate = crossRate / len;
  tracker->frameEnergy = energy;
  tracker->zeroCrossRate = crossRate;
  //  following is the main steps for the pitch tracking
  //  but first you should initialize the PitchTracker, then use it
#if 0//1//
  //  get the summation score of harmonics
  SummateHarmonics(tracker);

  //  get the voiced degree to roughly judge if it is voiced or unvoiced
  GetVoicedDegreeByNCCF(tracker);

  //  get the current score and trace back path
  GetCurrentScoreAndBackTrace(tracker);

  //  trace back again in the whole buffer
  TraceBackInBuffer(tracker);

  //  median smooth pitch value to get rid of some gross errors further
//  MedianSmoothPitchValue(tracker);

  //  use linear smooth method to get a smooth pitch contour
//  LinearSmoothPitchValue(tracker);

  curPitch = tracker->roughF0[tracker->curIndex];

  if (voicedDegree != NULL)
    *voicedDegree = tracker->voicedDegree[tracker->curIndex];

#else
  //  get the summation score of harmonics
  SumHarmonics(tracker);

  //  get the voiced degree to roughly judge if it is voiced or unvoiced
  GetVoicedDegreeByNCCF(tracker);

  int midIdx;

  midIdx = tracker->curIndex - 2;
  midIdx = (midIdx + Pitch_Buf_Size) % Pitch_Buf_Size;

  curPitch = tracker->roughF0[midIdx];

  if (voicedDegree != NULL)
    *voicedDegree = tracker->voicedDegree[midIdx];
#endif

  // increase index
  tracker->unvoiceNo[tracker->curIndex] = tracker->unvoiceCnt;
  tracker->curIndex++;
  tracker->curIndex = tracker->curIndex % Pitch_Buf_Size;
  tracker->frameIndex++;

  return  curPitch;
}

int   PitchTrack::InitPitchTracker(PitchTrackGroup *tracker) {
  int   index, band, t;

  tracker->frameLen = nFrameLen;

  //  initialize the pitch value in the buffer, they are random values
  for (index = 0; index < Pitch_Buf_Size; index++) 
  {
    tracker->voicedDegree[index] = Voiced_Threshold - 0.1F;
    for (band = 0; band < nEndBand; band++) {
      //tracker->sumHarmonics[index][band] = (float)rand();
	  tracker->sumHarmonics[index][band] = (float)1;
      tracker->fullBandRoughF0[index][band] = (int)((band + 0.5F) * BandWidth);
    }
  }

  if (InitPitchPath(tracker) == 0)
    return  0;

  // the hamming window is used for linear smoothing
  GenerateHammingWindow(tracker);

  //  initialize the parameters
  tracker->unvoiceCnt = Pitch_Buf_Size;
  tracker->curIndex = 0;
  tracker->frameIndex = 0;
  tracker->harmonicsFactor = Harmonics_Factor;

  //  normalize the pitch value in the buffer
  int   sumPitch = 0;
  for (t = 0; t < Pitch_Buf_Size; t++) {
    sumPitch = sumPitch + tracker->roughF0[t];
  }

  for (t = 0; t < Pitch_Buf_Size; t++) {
    tracker->normalizedF0[t] = tracker->roughF0[t] / (float)sumPitch;
  }

  //  initialize the low frequency emphasis factor
  for (band = 0; band < nNumFFT / 2; band++) {
    tracker->lowFreqEmphasisFactor[band] = ((float)cos((band + 0.5F) * BandWidth / Max_Harmonics_Freq * PI) + 2);
  }

  //  get the pitch candidatas at the FreqStep resolution
  float floatF0 = Lowest_F0;
  int i = 0;
  for (i = 0; i < NUM_F0; i++) {
    tracker->F0Point[i] = (int)(floatF0 + 0.5F);
    floatF0 = floatF0 * FreqStep;
  }

  float weight = 1;
  for (i = 1; i <= Max_Harmonics; i++) {
    tracker->weightHarnm[i] = weight;
    weight = weight * tracker->harmonicsFactor;
  }
  return  1;
}

int   PitchTrack::InitPitchPath(PitchTrackGroup *tracker) {
  int   index, band;
  int   startIndex, endIndex;
  int   localBand, lastBand;

  startIndex = 0;
  endIndex = Pitch_Buf_Size - 1;

  //  the following is just to give the initial pitch path in buffer, they are not meaningful values
  for (index = startIndex; index <= endIndex; index++) {
    for (band = 0; band < nEndBand; band++) {
      tracker->backTrack[index][band] = -1;
      if (index == startIndex)
        tracker->score[index][band] = tracker->sumHarmonics[startIndex][band];
      else
        tracker->score[index][band] = 0;
    }
  }

  // Dydamic programming
  for (index = startIndex + 1; index <= endIndex; index++) {
    for (localBand = nStartBand;  localBand < nEndBand; localBand++) {
      float maxTransScore = -1;
      int   argmaxBand = -1;
      int   shiftNo;
      for (lastBand = localBand - Band_Shift_Range, shiftNo = 0; lastBand <= localBand + Band_Shift_Range; lastBand++, shiftNo++) {
        if (lastBand < nStartBand || lastBand >= nEndBand)
          continue;
        if (lastBand < 0 || lastBand >= nNumFFT / 2) {
          printf("nBandShiftRange = %d is out of FFT range!!!", Band_Shift_Range);
          return  0;
        }

        float transScore;

        transScore = tracker->score[index - 1][lastBand];
        if (transScore > maxTransScore) {
          maxTransScore = transScore;
          argmaxBand = lastBand;
        }
      }
      if (argmaxBand == -1) {
        printf("there is something wrong1 in InitPitchPath()!\n");
        return  0;
      }
      tracker->score[index][localBand] = tracker->sumHarmonics[index][localBand] + maxTransScore;
      tracker->backTrack[index][localBand] = argmaxBand;

      if (argmaxBand == -1) {
        printf("Error in InitBandPath() trace path!\n");
        return  0;
      }
    }
  }

  float finalMaxScore = -1;
  int   finalArgmaxBand = -1;
  for (band = nStartBand;  band < nEndBand; band++) {
    if (tracker->score[endIndex][band] > finalMaxScore) {
      finalMaxScore = tracker->score[endIndex][band];
      finalArgmaxBand = band;
    }
  }
  if (finalArgmaxBand == -1) {
    printf("there is something wrong2 in InitBandPath!\n");
    return  0;
  }

  //  trace back
  tracker->bandPath[endIndex] = finalArgmaxBand;
  for (index = endIndex; index > startIndex; index--) {
    tracker->bandPath[index - 1] = tracker->backTrack[index][ tracker->bandPath[index] ];
    if (tracker->bandPath[index - 1] < nStartBand || tracker->bandPath[index - 1] >= nEndBand) {
      printf("there is something wrong3 in InitPitchPath()!\n");
      return  0;
    }
    if (tracker->bandPath[index] < nStartBand || tracker->bandPath[index - 1] >= nEndBand) {
      printf("there is something wrong4 in InitPitchPath()!\n");
      return  0;
    }
  }

  //  find the roughF0
  for (index = startIndex; index <= endIndex; index++) {
    tracker->roughF0[index] = tracker->fullBandRoughF0[index][tracker->bandPath[index]];
  }

  return  1;
}

void  PitchTrack::SummateHarmonics(PitchTrackGroup *tracker) {
  int   band, noHarmonics;
  int   F0, F0Index, i;
  short *wave;

  wave = new short [tracker->frameLen];
  memcpy(wave, tracker->signal, sizeof(short) * tracker->frameLen);

  //emphasize the low freqency which is good for telephone speech
  for (band = 0; band < nNumFFT / 2; band++) {
    tracker->fft[band] *= tracker->lowFreqEmphasisFactor[band];
  }

  float *sumHarF0;
  float *sumHarF0_ori;
  int   *argmaxF0;
  float   *fftSum;
//--------------------defined to cfg--------------
  sumHarF0 = new float [Highest_F0];
  sumHarF0_ori = new float [Highest_F0];
  argmaxF0 = new  int [nEndBand];
  fftSum = new float [nNumFFT / 2];

  //  get the summmation of harmonics
  for (F0 = 0; F0 < Highest_F0; F0++) {
    sumHarF0[F0] = 0;
    sumHarF0_ori[F0] = 0;
  }

  fftSum[0] = tracker->fft[0];
  for (i = 1; i < nNumFFT / 2; i++) {
    fftSum[i] = fftSum[i - 1] + tracker->fft[i];
  }

  for (F0Index = 0; F0Index < NUM_F0; F0Index++) {
    F0 = tracker->F0Point[F0Index];
    for (noHarmonics = 1; noHarmonics <= Max_Harmonics; noHarmonics++) {
      if (F0 * noHarmonics > Max_Harmonics_Freq)
        break;
#if 1//0//
      sumHarF0[F0] += tracker->weightHarnm[noHarmonics] * tracker->fft[(int)(F0 * noHarmonics / BandWidth + 0.5F)];
#else
      int   harmnIdx, harmnFreq;
      int   low, high;
      float avgE;

      harmnFreq = F0 * noHarmonics;
      harmnIdx = (int)(harmnFreq / BandWidth + 0.5F);
      low = (int)((harmnFreq - F0 * 2 / 3) / BandWidth) - 1;
      high = (int)((harmnFreq + F0 * 2 / 3) / BandWidth);
      low = (low < 1) ? 1 : low;
      high = (high < nNumFFT / 2) ? high : nNumFFT / 2 - 1;
      avgE = (fftSum[high] - fftSum[low]) / (high - low);
      assert((high - low) != 0);

      sumHarF0[F0] += tracker->weightHarnm[noHarmonics] * (tracker->fft[harmnIdx] / avgE);
      sumHarF0_ori[F0] += tracker->weightHarnm[noHarmonics] * tracker->fft[harmnIdx];
#endif
    }
    //  sumHarF0[F0] *= NormalizedCorrelation(tracker, wave, F0);
  }
  //  store the best pitch candidate for each band
  for (band = nStartBand; band < nEndBand; band++) {
    argmaxF0[band] = 0;
    float max = -1;
    for (F0 = (int)(band * BandWidth + 0.5F); F0 < (int)((band + 1) * BandWidth + 0.5F) && F0 < Highest_F0; F0++) {
      if (band == nStartBand && F0 < Lowest_F0)
        continue;
      if (sumHarF0[F0] > max) {
        max = sumHarF0[F0];
        argmaxF0[band] = F0;
        tracker->fullBandRoughF0[tracker->curIndex][band] = F0;
      }
    }
    assert(max >= 0);
    tracker->sumHarmonics[tracker->curIndex][band] = max;
  }

  //  get the best pitch candidate among all bands before DP
  float maxSum = -1;
  tracker->roughF0[tracker->curIndex] = 0;
  for (band = nStartBand; band < nEndBand; band++) {
    if (tracker->sumHarmonics[tracker->curIndex][band] > maxSum) {
      maxSum = tracker->sumHarmonics[tracker->curIndex][band];
      tracker->roughF0[tracker->curIndex] = argmaxF0[band];
    }
  }
  assert(tracker->roughF0[tracker->curIndex] >= 0);
  maxSum = -1;
  for (F0 = 0; F0 < Highest_F0; F0++) {
    if (sumHarF0[F0] > maxSum) {
      maxSum = sumHarF0[F0];
      tracker->maxSumHarmn = sumHarF0_ori[F0];
    }
  }
  delete []wave;
  //-------------------------------------------
  delete []sumHarF0;
  delete []sumHarF0_ori;
  delete []fftSum;
  delete []argmaxF0;
}
void  PitchTrack::GetVoicedDegreeByNCCF(PitchTrackGroup *tracker) {
  int   i, lag;
  float energy1, energy2;
  float cor;
  int   signal1;
  int   signal2;

  //  use the rough F0 to get suitable lag to calculate the voiced degree by the normalize cross correlation function(NCCF)
  assert(tracker->roughF0[tracker->curIndex] != 0);
  lag = nSampleRate / tracker->roughF0[tracker->curIndex];

  if (nNumCorrSamples + lag > tracker->frameLen) {
    printf("Abort: Because nNumCorrSamples + lag > tracker->frameLen!!!\n");
    printf("You should decrease the constant nNumCorrSamples in PitchTracker.h\n");
    exit(1);
  }

  cor = 0;
  energy1 = energy2 = 0;
  for (i = 0; i < nNumCorrSamples; i++) {
    signal1 = (int)tracker->signal[i];
    signal2 = (int)tracker->signal[i + lag];
    cor = cor + (int)signal1 * (int)signal2;
    energy1 = energy1 + signal1 * signal1;
    energy2 = energy2 + signal2 * signal2;
  }

  //  Basic_Energy_Square is used to decrease the scores which are in periodical noise section
  tracker->voicedDegree[tracker->curIndex] = cor / (float)sqrt(BasicEnergySquare + energy1 * energy2);
}

int   PitchTrack::GetCurrentScoreAndBackTrace(PitchTrackGroup *tracker) {
  int   curIndex, lastIndex;
  int   localBand, lastBand;

  curIndex = tracker->curIndex;
  lastIndex = (curIndex - 1 + Pitch_Buf_Size) % Pitch_Buf_Size;
  assert(lastIndex >= 0 && lastIndex < Pitch_Buf_Size);

  for (localBand = nStartBand;  localBand < nEndBand; localBand++) {
    float maxTransScore = -1;
    int   argmaxBand = -1;
    int   shiftNo;

    //  select the best band on the back path
    for (lastBand = localBand - Band_Shift_Range, shiftNo = 0; lastBand <= localBand + Band_Shift_Range; lastBand++, shiftNo++) {
      if (lastBand < nStartBand || lastBand >= nEndBand)
        continue;
      if (lastBand < 0 || lastBand >= nNumFFT / 2) {
        printf("nBandShiftRange = %d is out of FFT range!!!", Band_Shift_Range);
        return  0;
      }

      float transScore;

      transScore = tracker->score[lastIndex][lastBand];
      if (transScore > maxTransScore) {
        maxTransScore = transScore;
        argmaxBand = lastBand;
      }
    }
    if (argmaxBand == -1) {
      printf("there is something wrong1 in InitPitchPath()!\n");
      return  0;
    }

    //  if a lot of past frames are unvoiced and current frame is voiced, then do not inherit the score
    if (tracker->unvoiceCnt >= Min_Voiced_Gap && tracker->voicedDegree[curIndex] > Voiced_Threshold || tracker->frameIndex == 0)
      maxTransScore = 0;

    //  current score = local score(tracker->sumHarmonics[curIndex][localBand]) + cumulative score(maxTransScore)
    tracker->score[curIndex][localBand] = tracker->sumHarmonics[curIndex][localBand] + maxTransScore;
    tracker->backTrack[curIndex][localBand] = argmaxBand;

    if (argmaxBand == -1) {
      printf("Error in GetCurrentScoreAndBackTrace()!\n");
      return  0;
    }
  }
  return  1;
}

int   PitchTrack::TraceBackInBuffer(PitchTrackGroup *tracker) {
  int   band, index, i;
  float finalMaxScore = -1;
  int   finalArgmaxBand = -1;
  int   curIndex, lastIndex, endIndex;

  endIndex = tracker->curIndex;

  for (band = nStartBand;  band < nEndBand; band++) {
    if (tracker->score[endIndex][band] > finalMaxScore) {
      finalMaxScore = tracker->score[endIndex][band];
      finalArgmaxBand = band;
    }
  }
  if (finalArgmaxBand == -1) {
    printf("there is something wrong2 in TraceBackInBuffer!\n");
    return  0;
  }

  tracker->bandPath[endIndex] = finalArgmaxBand;
  curIndex = endIndex;

  int   path;
  for (i = 0; i < Pitch_Buf_Size - 1; i++) {
    //  if the unvoiced section are near the voiced section(Back_Voice_Gap frames distance), then it will not inherit the path
    if (i != 0 && tracker->unvoiceNo[curIndex] == Back_Voice_Gap) {
      finalArgmaxBand = -1;
      finalMaxScore = -1;
      for (band = nStartBand;  band < nEndBand; band++) {
        if (tracker->score[curIndex][band] > finalMaxScore) {
          finalMaxScore = tracker->score[curIndex][band];
          finalArgmaxBand = band;
        }
      }
      if (finalArgmaxBand == -1) {
        printf("there is something wrong3 in TraceBackInBuffer!\n");
        return  0;
      }

      tracker->bandPath[curIndex] = finalArgmaxBand;
    }

    lastIndex = (curIndex - 1 + Pitch_Buf_Size) % Pitch_Buf_Size;
    path = tracker->backTrack[curIndex][ tracker->bandPath[curIndex] ];
    //  if the same path as before, then it stops tracing back
    if (path == tracker->bandPath[lastIndex]) {
      tracker->stopBackIndex = lastIndex;
      break;
    }

    tracker->bandPath[lastIndex] = path;


#if DEBUG_PitchTracker
    if (tracker->bandPath[lastIndex] < nStartBand || tracker->bandPath[lastIndex] >= nEndBand) {
      printf("there is something wrong3 in TraceBackInBuffer!\n");
      return  0;
    }
    if (tracker->bandPath[curIndex] < nStartBand || tracker->bandPath[curIndex] >= nEndBand) {
      printf("there is something wrong4 in TraceBackInBuffer!\n");
      return  0;
    }
#endif
    curIndex = lastIndex;
  }

  //  get the optimal pitch values after DP
  for (index = 0; index < Pitch_Buf_Size; index++) {
    tracker->roughF0[index] = tracker->fullBandRoughF0[index][tracker->bandPath[index]];
  }

  return  1;
}

void  PitchTrack::GenerateHammingWindow(PitchTrackGroup *tracker) {
  //  get the hamming window for linear smooth
  int   i;
  for (i = 1; i <= Linear_Smooth_Window_Size * 2 + 1; i++) {
    tracker->hamming_window[i - 1] = (float)sin(i / (Linear_Smooth_Window_Size * 2.0F + 2) * PI);
  }

  float tmp = PI * 2.0F / (float)(nFrameLen - 1);
  for (i = 0; i < nFrameLen; i++)
    hamWin[i] = (float)(0.54 - 0.46 * cos(tmp * i));
}

void  PitchTrack::LinearSmoothArray(int *pitch, int numPitch) {
  int   *f0_bak;
  int   i;

  f0_bak = (int *) calloc(numPitch, sizeof(int));

  for (i = 0; i < numPitch; i++)
    f0_bak[i] = pitch[i];

  for (i = 2; i < numPitch - 2; i++) {
    pitch[i] = (int)((f0_bak[i - 2] + f0_bak[i - 1] * 2 + f0_bak[i + 1] * 2 + f0_bak[i + 2]) / 12.0F + f0_bak[i] / 2.0F + 0.5F);
  }
  free(f0_bak);
}

void  PitchTrack::LinearSmoothArray(float*voicedDegrees, int numPitch) {
  float   *f0_bak;
  int   i;

  f0_bak = (float *) calloc(numPitch, sizeof(float));

  for (i = 0; i < numPitch; i++)
    f0_bak[i] = voicedDegrees[i];

  for (i = 2; i < numPitch - 2; i++) {
    voicedDegrees[i] = (float)((f0_bak[i - 2] + f0_bak[i - 1] * 2 + f0_bak[i + 1] * 2 + f0_bak[i + 2]) / 12.0F + f0_bak[i] / 2.0F + 0.5F);
  }
  free(f0_bak);
}
void  PitchTrack::MedianSmoothArray1(int *pitch, int numPitch) {
  int   *f0_bak, *pitchTemp;
  int   i, j, t;
  int   index, curIndex, smoothed_f0;

  f0_bak = (int *) calloc(numPitch, sizeof(int));
  assert(f0_bak);

  pitchTemp = (int *) calloc(numPitch, sizeof(int));
  assert(pitchTemp);

  for (index = 0; index < numPitch; index++)
    f0_bak[index] = pitch[index];

  //  use the median pitch value as the final pitch value
  assert(Median_Smooth_Range % 2 != 0);
  int   radius = (Median_Smooth_Range - 1) / 2;

//  for (t = 0; t < numPitch; t++)
  for (t = numPitch - 1; t >= 0; t--) {
    curIndex = t;
    //  if curIndex equals to the stopBackIndex, this means the rest of them are the same
    //  as before, so it's not necessary to do it again for them
    if (t - radius < 0 || t + radius >= numPitch) {
      smoothed_f0 = pitch[curIndex];
    } else {
      int   temp, outerIndex, innerIndex;
      for (i = - radius; i <= radius; i++) {
        index = curIndex + i;
        pitchTemp[index] = pitch[index];
      }

      //  sort them and get the median value
      for (i = - radius; i <= 0; i++) {
        outerIndex = curIndex + i;
        for (j = i + 1; j <= radius; j++) {
          innerIndex = curIndex + j;
          if (pitchTemp[outerIndex] < pitchTemp[innerIndex]) {
            temp = pitchTemp[outerIndex];
            pitchTemp[outerIndex] = pitchTemp[innerIndex];
            pitchTemp[innerIndex] = temp;
          }
        }
      }
      smoothed_f0 = pitchTemp[curIndex];
    }
    f0_bak[curIndex] = smoothed_f0;
  }

  for (index = 0; index < numPitch; index++) {
    pitch[index] = f0_bak[index];
  }

  free(pitchTemp);
  free(f0_bak);
}

void  PitchTrack::MedianSmoothArray(float *pitch, int numPitch) {
	float   *f0_bak, *pitchTemp;
	int   i, j, t;
	int   index, curIndex;
	float smoothed_f0;

	f0_bak = (float *) calloc(numPitch, sizeof(float));
	assert(f0_bak);

	pitchTemp = (float *) calloc(numPitch, sizeof(float));
	assert(pitchTemp);

  for (index = 0; index < numPitch; index++)
    f0_bak[index] = pitch[index];

  //  use the median pitch value as the final pitch value
  assert(Median_Smooth_Range % 2 != 0);
  int   radius = (Median_Smooth_Range - 1) / 2;

  //  for (t = 0; t < numPitch; t++)
  for (t = numPitch - 1; t >= 0; t--) {
    curIndex = t;
    //  if curIndex equals to the stopBackIndex, this means the rest of them are the same
    //  as before, so it's not necessary to do it again for them
    if (t - radius < 0 || t + radius >= numPitch) {
      smoothed_f0 = pitch[curIndex];
    } else {
      int    outerIndex, innerIndex;
      float temp;
      for (i = - radius; i <= radius; i++) {
        index = curIndex + i;
        pitchTemp[index] = pitch[index];
      }

      //  sort them and get the median value
      for (i = - radius; i <= 0; i++) {
        outerIndex = curIndex + i;
        for (j = i + 1; j <= radius; j++) {
          innerIndex = curIndex + j;
          if (pitchTemp[outerIndex] < pitchTemp[innerIndex]) {
            temp = pitchTemp[outerIndex];
            pitchTemp[outerIndex] = pitchTemp[innerIndex];
            pitchTemp[innerIndex] = temp;
          }
        }
      }
      smoothed_f0 = pitchTemp[curIndex];
    }
    f0_bak[curIndex] = smoothed_f0;
  }

  for (index = 0; index < numPitch; index++) {
    pitch[index] = f0_bak[index];
  }

  free(pitchTemp);
  free(f0_bak);
}
//----------------------------------------interface--------------------------
bool PitchTrack::Init(char *config) {

////////////////////////////////////////////////
  // read input parameter
//  Config *cfg=new Config(config);
//  cfg->ReadConfig("FT_SampleRate",nSampleRate);
//  cfg->ReadConfig("FT_FrameTime",nFrameTime);
//  cfg->ReadConfig("FT_FrameShiftTime",nFrameShiftTime);
//  cfg->ReadConfig("FT_LowestF0",nLowestF0);
//  cfg->ReadConfig("FT_HighestF0",nHighestF0);
//  cfg->ReadConfig("FT_MaxHarmonics",nMaxHarmonics);
//  cfg->ReadConfig("FT_Max_HarmonicsFreq",nMaxHarmonicsFreq);
//  cfg->ReadConfig("FT_BandShiftRange",nBandShiftRange);
//  cfg->ReadConfig("FT_HarmonicsFactor",HarmonicsFactor);
//  cfg->ReadConfig("FT_VoicedThreshold",VoicedThreshold);
//  cfg->ReadConfig("FT_MedianSmoothRange",nMedianSmoothRange);
//  cfg->ReadConfig("FT_LinearSmoothWindowSize",nLinearSmoothWindowSize);
//  cfg->ReadConfig("FT_PitchBufSize",nPitchBufSize);
//  cfg->ReadConfig("FT_MinVoicedGap",nMinVoicedGap);
//  cfg->ReadConfig("FT_BackVoiceGap",nBackVoiceGap);
//  cfg->ReadConfig("FT_NUMF0",nNUMF0);
//  delete cfg;


  return true;
}


int PitchTrack::DoUtterance(short *waveData, int numFrame, int **pitch) {
//  short *waveData;
//  int   *pitches;
//  float *energy;
//  int   *sumHarmn;
//  int   numSamples;
  int   frame;
//  PitchTrackGroup *tracker;

//  tracker = new PitchTrackGroup;
  InitPitchTracker(tracker);

  //numFrame = numSamples / nFrameShit -4
//  numFrame = (numSamples - nFrameLen + nFrameShit) / nFrameShit;

  if (pitches)delete []pitches, pitches = NULL;
  pitches = new int [numFrame];
//  energy = new float [numFrame];
//  sumHarmn = new int [numFrame];

  for (frame = 0; frame < numFrame; frame++) {
    pitches[frame] = GetCurPitch(tracker, &waveData[frame * nFrameShit], nFrameLen, NULL);
//    energy[frame] = tracker->frameEnergy;
//    sumHarmn[frame] = (int)(10*(float)(log10(tracker->maxSumHarmn+1)));
  }
   //comment for online
  for (frame = 2; frame < numFrame-2; frame++)
  {
    pitches[frame] = pitches[frame+2];
  }
  // rewrite the first pitches
  for (frame = 0; frame < 3; frame++)
  {
    pitches[frame] = pitches[frame+3];
  }
  LinearSmoothArray(pitches, numFrame);
  MedianSmoothArray1(pitches, numFrame);
  

//-------test gen file-----------------------------
#ifdef GENPITCHFILE
  FILE *oFile;
  strtok(waveFileName, ".");
  strcat(waveFileName, ".pitch");
  if ((oFile = fopen((const char*)waveFileName, "w")) == NULL)
    printf("Can not open %s for write", waveFileName);
  for (frame = 0; frame < numFrame; frame++) {
    fprintf(oFile, "frame %d: F0 %d\n", frame, pitches[frame]);
    printf( "frame %d: F0 %d\n", frame, pitches[frame]);
  }
  fclose(oFile);
#endif
//--------------------------------------------------

  *pitch = pitches;
  return  numFrame;

}

int PitchTrack::DoUtterance(short *waveData, int numFrame, int **pitch, float **voicedDegree, bool initPitch) {
//  short *waveData;
//  int   *pitches;
//  float *energy;
//  int   *sumHarmn;
//  int   numSamples;
  int   frame;
//  PitchTrackGroup *tracker;

//  tracker = new PitchTrackGroup;
  if (initPitch)
  {
	  InitPitchTracker(tracker);
  }

  //numFrame = numSamples / nFrameShit -4
//  numFrame = (numSamples - nFrameLen + nFrameShit) / nFrameShit;

  if (pitches)delete []pitches;
  pitches = NULL;
  if (voicedDegrees)delete []voicedDegrees;
  voicedDegrees = NULL;
  pitches = new int [numFrame];
  voicedDegrees = new float [numFrame];
//  sumHarmn = new int [numFrame];


  for (frame = 0; frame < numFrame; frame++) {
    pitches[frame] = GetCurPitch(tracker, &waveData[frame * nFrameShit], nFrameLen, &voicedDegrees[frame]);

//    energy[frame] = tracker->frameEnergy;
//    sumHarmn[frame] = (int)(10*(float)(log10(tracker->maxSumHarmn+1)));
  }

//  for (frame = 2; frame < numFrame - 2; frame++) {
//
//    pitches[frame] = pitches[frame + 2];
//    voicedDegrees[frame] = voicedDegrees[frame + 2];
//  }
//
//
////  // rewrite the first pitches
//  for (frame = 0; frame < 3; frame++) {
//
//    pitches[frame] = pitches[frame + 3];
//    voicedDegrees[frame] = voicedDegrees[frame + 3];
//  }
//
//
//
//
//
//
//  LinearSmoothArray(pitches, numFrame);
//  MedianSmoothArray(pitches, numFrame);
//
//  LinearSmoothArray(voicedDegrees, numFrame);//qqzhang
//  MedianSmoothArray(voicedDegrees, numFrame);//qqzhang
  /*  for (frame = 0; frame < numFrame; frame++)
    {
      if(voicedDegrees[frame] > 0.70)
      {
        printf("voicedDegrees[%d]\t%f\tpitch[%d]\t%d\n",frame,voicedDegrees[frame],frame,pitches[frame]);
      }
    }
  */

//-------test gen file-----------------------------
#ifdef GENPITCHFILE
  FILE *oFile;
  strtok(waveFileName, ".");
  strcat(waveFileName, ".pitch");
  if ((oFile = fopen((const char*)waveFileName, "w")) == NULL)
    printf("Can not open %s for write", waveFileName);
  for (frame = 0; frame < numFrame; frame++) {
    fprintf(oFile, "frame %d: F0 %d\n", frame, pitches[frame]);
    printf( "frame %d: F0 %d\n", frame, pitches[frame]);
  }
  fclose(oFile);
#endif
//--------------------------------------------------

  *pitch = pitches;
  *voicedDegree = voicedDegrees;
  return  numFrame;

}
void PitchTrack::close() {
  if (pitches)
    delete []pitches, pitches = NULL;

}
