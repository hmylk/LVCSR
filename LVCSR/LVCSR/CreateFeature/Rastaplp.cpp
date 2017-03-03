/******************************DOCUMENT*COMMENT***********************************
*D
*D 文件名称            : Rastaplp.cpp
*D
*D 项目名称            : HCCL-LVCSR
*D
*D 版本号              : 1.1.0002
*D
*D 文件描述            :
*D
*D
*D 文件修改记录
*D ------------------------------------------------------------------------------ 
*D 版本号       修改日期       修改人     改动内容
*D ------------------------------------------------------------------------------ 
*D 1.1.0001     2004.05.31     plu        创建文件
*D 1.1.0002     2004.06.30     plu        用PLP的C0代替归一化能量
*D*******************************************************************************/

// C++ encapsulation of rasta processing chain (interface to old C code)
// Part of feacalc project.
//
// 1997jul28 dpwe@icsi.berkeley.edu
// $Header: /n/abbott/da/drspeech/src/feacalc/RCS/Rasta.C,v 1.11 2002/03/18 21:10:38 dpwe Exp $

#include <limits.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "RastaPlp.h"
#include "cFFT.h"


RASTA_PLP::RASTA_PLP()
{
	m_ffts = m_FtrData = NULL;
	m_cepCoef=m_specCoef=m_pspecptr=NULL;	

	m_post_audptr = m_audptr = m_outptr = NULL;
	m_cbweightptr = m_autoptr = m_lpcptr=m_outptrOLD=m_outptr=NULL;

	m_hamWin=NULL;

	pitch=NULL;//add pitch info by zliu
	pitchTrack=NULL;
}

RASTA_PLP::~RASTA_PLP()
{
	if (m_pspecptr)			free_fvec(m_pspecptr);
	if (m_outptr)           free_fvec(m_outptr);
	if (m_post_audptr)      free_fvec(m_post_audptr);
	if (m_autoptr)          free_fvec(m_autoptr);
	if (m_lpcptr)           free_fvec(m_lpcptr);
	if (m_outptrOLD)        free_fvec(m_outptrOLD);
	if (m_cepCoef)          free_fvec(m_cepCoef);
	if (m_specCoef)         free_fvec(m_specCoef);
	if (m_audptr)           free_fvec(m_audptr);
	if (m_cbweightptr)      free_fvec(m_cbweightptr);
	if (m_wts)              free_fmat(m_wts);

	if(pitch)	free(pitch);//add pitch info by zliu
	if(pitchTrack) delete pitchTrack;
}

void RASTA_PLP::WriteFile(char *outFile)
{
   ASSERT2(m_FtrData,"error when WriteFile(): m_FtrData=NULL");
   FILE *fout; int i; float *ft;

   short tframeSize=(short)(m_BaseDim*sizeof(float));

   fout=fopen(outFile,"wb");
   ASSERT3(fout,"Error open %s for write.",outFile);

   fwrite(&m_FrameNum,sizeof(int),1,fout);
   fwrite(&m_baseInfo.framePeriod,sizeof(int),1,fout);
   fwrite(&tframeSize,sizeof(short),1,fout);
   fwrite(&m_rastaKind,sizeof(short),1,fout);

   for (i=0,ft=m_FtrData;i<m_FrameNum;i++,ft+=m_BaseDim) 
   {
      for (int k=0;k<m_BaseDim;k++) 
	  {
		  // 2004.11.05 plu : 增大数字量级
		  ft[k] *= 10.f;

         if (ft[k]>DYN_RANGE) ft[k]=DYN_RANGE;
         if (ft[k]<-DYN_RANGE) ft[k]= -DYN_RANGE;
      }
      fwrite(ft,sizeof(float),m_BaseDim,fout);
   }
   fclose(fout);

   /*// plu 2004.06.30_16:15:06
   fout=fopen(outFile,"wt");
   ASSERT2(fout,"Error open %s for write.",outFile);

   for (i=0,ft=m_FtrData;i<m_FrameNum;i++,ft+=m_BaseDim) 
   {
	   for (int ii=0;ii<m_BaseDim;ii++)
		fprintf(fout,"%f ",ft[ii]);
	   fprintf(fout,"\n");
   }
   fclose(fout);   
   */// plu 2004.06.30_16:15:06
}
float *RASTA_PLP::GetFeatureDate(int *frm,short *tframeSize)
{
   ASSERT2(m_FtrData,"error when WriteFile(): mfccData=NULL");

   *frm = m_FrameNum;
   *tframeSize=(short)(m_BaseDim*sizeof(float));
   float *ft;
   int i, k;
   for (i=0,ft=m_FtrData;i<m_FrameNum;i++,ft+=m_BaseDim) 
   {
      for (k=0;k<m_BaseDim;k++) {
         if (ft[k]>DYN_RANGE) ft[k]=DYN_RANGE;
         if (ft[k]<-DYN_RANGE) ft[k]= -DYN_RANGE;
      }
//      fwrite(ft,sizeof(float),BaseDim,fout);
   }
   return m_FtrData;
}

//void RASTA_PLP::SetBaseInfo(RASTAPLP_BASEINFO p_info)
void RASTA_PLP::SetBaseInfo(FEATURE_BASEINFO p_info)
{
	if (strstr(p_info.targetKind,"RASTAPLP"))
    {
		 m_rastaKind=FYT_RASTAPLP;
		if (strstr(p_info.targetKind,"_C0"))			m_rastaKind |= HASENERGY;
	}
	else
	{
		printf("Error set targetkind in RASTAPLP!\n");	exit(-1);
	}

	m_baseInfo.bDoRasta = p_info.bDoRasta;
	m_baseInfo.bgainflag = p_info.bgainflag;
	m_baseInfo.cepNum = p_info.cepNum;
//	m_baseInfo.doMain = p_info.doMain;
	m_baseInfo.bDoCep = p_info.bDoCep;
	m_baseInfo.smpPeriod = p_info.smpPeriod;
	m_baseInfo.framePeriod = p_info.framePeriod;
	m_baseInfo.stepsize = p_info.stepsize;
	m_baseInfo.lpcOrder = p_info.lpcOrder;
	m_baseInfo.winSize = p_info.winSize;
	m_baseInfo.fcepLifter = p_info.fcepLifter;
	m_baseInfo.fpole = p_info.fpole;

	/*// plu 2004.06.10_15:51:47
	m_baseInfo.bDoMel =	p_info.bDoMel;
	m_baseInfo.fclower = p_info.fclower;
	m_baseInfo.fcupper = p_info.fcupper;
	m_baseInfo.bTrapezfilter = p_info.bTrapezfilter;
	m_baseInfo.nfilts = p_info.nfilts;
	*/// plu 2004.06.10_15:51:47

    m_bBaseSet=true;

	// add by zliu
	m_baseInfo.bPitch = p_info.bPitch;
}


void RASTA_PLP::Initialize(int &BaseDim) 
{
	ASSERT2(m_bBaseSet,"pls call SetBaseInfo() firstly!");

	m_frameRate = m_baseInfo.framePeriod/m_baseInfo.smpPeriod;
	m_winSize   = m_baseInfo.winSize/m_baseInfo.smpPeriod;

	m_hamWin=new float[m_winSize];
	double a = 2*PI /(double)(m_winSize - 1);
	int i=0;
	for (i=0;i<m_winSize;i++)
		m_hamWin[i] =(float)( 0.54 - 0.46 * cos(a*(i)));

	m_nyqhz = (int)(1.0e7/m_baseInfo.smpPeriod)/2;

	m_FFTNum = 2;    
	while (m_FFTNum < m_winSize) m_FFTNum *= 2;

	m_ffts = new float[m_FFTNum];
	ASSERT2(m_ffts,"Error new m_ffts.");


	// Nyquist rate in bark specifies audspec range and default num filters 
	// Here is some magical stuff to get the Nyquist frequency in barks 
	m_nyqbar = HZ2BARK(m_nyqhz);

	m_nfilts = ceil(m_nyqbar)+1;	// 2004.06.09 plu : 

	  // compute filter step in Barks 
	float step_barks = m_nyqbar / (float)(m_nfilts - 1);

	// for a given step, must ignore the first and last few filters 
	m_first_good = (int)(1.0 / step_barks + 0.5);
	m_lastfilt = m_nfilts - m_first_good;

    m_pspecptr = alloc_fvec ( (m_FFTNum / 2) + 1 );
	m_outptr = alloc_fvec( m_nfilts );
	m_post_audptr = alloc_fvec( m_nfilts );
	m_autoptr = alloc_fvec( m_baseInfo.lpcOrder + 1 );
	m_lpcptr = alloc_fvec( m_baseInfo.lpcOrder + 1 );
	m_outptrOLD = alloc_fvec( m_baseInfo.lpcOrder + 1 );

	//if (m_baseInfo.doMain == DOMAIN_CEPSTRA)
	if (m_baseInfo.bDoCep)
		m_BaseDim = (m_rastaKind&HASENERGY)?(m_baseInfo.cepNum+1):m_baseInfo.cepNum;
	else
		m_BaseDim = (m_rastaKind&HASENERGY)?(m_nfilts+1):m_nfilts;

	//add by zliu 
	if (m_baseInfo.bPitch) {
		m_BaseDim += 1; //add pitch
	}
	BaseDim = m_BaseDim;

	float f_bark_mid,f_hz_mid,ftmp,fsq;
	for(i = m_first_good; i < m_lastfilt; i++)
	{ 
		m_fir_filt[i] = get_rasta_fir();
		m_iir_filt[i] = get_integ();

		f_bark_mid = i * step_barks;

		// get center frequency of the j-th filter in Hz 
		f_hz_mid = 300. * (exp((double)f_bark_mid / 6.)	- exp(-(double)f_bark_mid / 6.));

		// calculate the LOG 40 db equal-loudness curve at center frequency 

		fsq = (f_hz_mid * f_hz_mid) ;
		ftmp = fsq + 1.6e5;
		m_eql[i] = (fsq / ftmp) * (fsq / ftmp) * ((fsq + (float)1.44e6)/(fsq + (float)9.61e6));
	}
	
	for(i=0; i<m_outptrOLD->length; i++)
		m_outptrOLD->values[i] = TINY;		

//	if (m_baseInfo.doMain == DOMAIN_CEPSTRA)
	if (m_baseInfo.bDoCep)
		m_cepCoef = alloc_fvec(m_baseInfo.cepNum);
	else
		m_specCoef = alloc_fvec(m_nfilts);

	/*// plu 2004.06.10_15:45:08
	// build the frequency axis mapping
	set_freq_axis();		
	*/// plu 2004.06.10_15:45:08
	
	/*// plu 2004.06.10_15:45:00
	// 2004.06.10 plu : 
static float eql [23];
long npoint=257;
long nfilt;
float sf=16000.f;
	// compute spectral weights 
	adw_(&npoint, &nfilt, cb, eql, ibegen, &sf);
	*/// plu 2004.06.10_15:45:00

// 2004.06.10 plu : 
m_binbarks = alloc_fvec(m_FFTNum/2+1);
float hz_in_samp;
hz_in_samp = (float)m_nyqhz / (float)(m_FFTNum/2);

for (i=0;i<m_FFTNum/2;i++)
	m_binbarks->values[i] = HZ2BARK(i*hz_in_samp);

m_wts = alloc_fmat(m_nfilts,m_FFTNum/2+1);
for (i=0;i<m_nfilts;i++)
{
	float f_bark_mid = i* step_barks;
	float tmp1,tmp2,tmp;

	for (int j=0;j<m_FFTNum/2+1;j++)
	{
		tmp1 = m_binbarks->values[j]-f_bark_mid+0.5;
		tmp2 = -2.5*(m_binbarks->values[j]-f_bark_mid-0.5);

		tmp=min(0.0,min(tmp1,tmp2));

		m_wts->values[i][j] = (float)pow((float)10.0,tmp);
	}	
}
free_fvec(m_binbarks);

	m_audptr = alloc_fvec( m_nfilts );

	m_i_call = 0;

	//add by zliu
	if (m_baseInfo.bPitch) 
		pitchTrack = new PitchTrack(m_baseInfo.smpPeriod, m_baseInfo.winSize, m_baseInfo.framePeriod);

	m_bInitialize=true;
}

/* Allocate fmat, and return pointer to it. */
FMAT * RASTA_PLP::alloc_fmat(int nrows, int ncols)
{
        FMAT *mptr;
        int i;

        mptr = ( FMAT *)malloc(sizeof(FMAT));
        if(mptr == ( FMAT *)NULL)
        {
                fprintf(stderr,"Cant malloc an fmat\n");
                exit(-1);
        }

        mptr->values = (float **)malloc(nrows * sizeof(float *));
        if(mptr->values == (float **)NULL)
        {
                fprintf(stderr,"Cant malloc an fmat ptr array\n");
                exit(-1);
        }
	/* I need the rows of the fmat to be contiguous in memory 
	   so that I can use the phipac routines on them.  Anyway, 
	   it's neater.  So allocate space for the whole matrix at
	   once, then construct row pointers into it. */
        mptr->values[0] = (float *)malloc(ncols*nrows*sizeof(float));
        if (mptr->values[0] == NULL) {
	    fprintf(stderr, "Can't malloc an fmat array %dx%d\n", 
		    nrows, ncols);
	    exit(-1);
	}
	/* clear the new space */
	memset(mptr->values[0], 0, nrows*ncols*sizeof(float));
	/* set up pointers to subsequent rows */
        for (i=1; i<nrows; ++i) {
	    mptr->values[i] = mptr->values[0] + i*ncols;
        }

        mptr->nrows = nrows;
        mptr->ncols = ncols;

        return( mptr );
}

void RASTA_PLP::free_fmat(FMAT* fm) 
{
    int i;
    /* only the first row points to allocated space (now) */
    free(fm->values[0]);
    free(fm->values);
    free(fm);
}


void RASTA_PLP::AddWaveData(short *waveData,int smpNum,int *residue)
{
    ASSERT2(m_bInitialize,"pls call Initialize() firstly!");
    ASSERT2(waveData,"error in AddWaveData(): input wavedata=NULL!");
    ASSERT2(smpNum>0,"error in AddWaveData(): input smpNum <= 0!");
    
    m_FrameNum=(smpNum-m_winSize)/m_frameRate+1; // 只少不多
	if (residue) *residue = smpNum-(m_FrameNum-1)*m_frameRate-m_winSize;
 	if (m_baseInfo.bPitch){
		pitch=NULL;
		short *tmpwav = new short [smpNum];
		
//		for(int i=0; i < smpNum; i++){
//			tmpwav[i]=waveData[i];
//		}	
		memcpy(tmpwav,waveData,sizeof(short)*smpNum);
	//	short *tmpwav=waveData;
		pitchTrack->DoUtterance(tmpwav,m_FrameNum, &pitch);
		delete(tmpwav);
	}
	int i;
     
	/*// plu 2004.05.31_17:54:15
	// remove time-domain mean
    if (m_baseInfo.zeroGlobalMean) ZeroGlobalMean(waveData,smpNum);
	*/// plu 2004.05.31_17:54:15
  
	float *pCurFrame,energy;
	short *pCurWave;

    m_FtrData=new float[m_FrameNum*m_BaseDim];
    ASSERT2(m_FtrData,"error alloc space for m_FtrData");
	
    pCurFrame = m_FtrData;	
	pCurWave  = waveData;
    for (i=0; i<m_FrameNum; i++)
    {
		printf("-------- frame = %d :\r",i);

        energy = Process(pCurWave,pCurFrame,i);

		/*// plu 2004.06.30_17:04:50
		if (m_rastaKind&HASENERGY)  
			pCurFrame[m_BaseDim-1]= (float)(energy<=MINLARG)?LOGZERO:log(energy);
		*/// plu 2004.06.30_17:04:50

		pCurWave += m_frameRate;
		pCurFrame += m_BaseDim;
    }

    /*// plu 2004.05.31_17:54:37
    if ( (m_rastaKind&HASENERGY) && m_baseInfo.normEnergy )         
		NormEnergy();
    */// plu 2004.05.31_17:54:37

    printf("%d, %d\n", m_BaseDim,m_FrameNum);                                                                      
}



float RASTA_PLP::Process(short* samps,float *pCurFrame,int p_i) 
{
	ASSERT2(samps,"Error in Process() :  samps = NULL");

    int i;
	float energy=0.f;

	for (i=0;i<m_winSize;i++) m_ffts[i]=(float)samps[i];
	for (i=m_winSize;i<m_FFTNum;i++) m_ffts[i]=0;

	// 计算时域能量
	for (i=0;i<m_winSize;i++)	energy+=m_ffts[i]*m_ffts[i];

	// ham window
	for (i=0;i<m_winSize;i++)	m_ffts[i] *= m_hamWin[i];

	// Compute the power spectrum from the windowed input 
	powspec(NULL);

	// Compute critical band (auditory) spectrum 
	AudSpec();		// -- > m_audptr

	if (m_baseInfo.bDoRasta) 
	{		    
		// Put into some nonlinear domain : log()
		nl_audspec();
		
		// Do rasta filtering 
		rasta_filt();				// --> m_outptr

		// Take the quasi inverse on the nonlinearity : exp()
		inverse_nonlin();			// --> m_outptr

		// Do final cube-root compression and equalization 
		post_audspec(m_outptr);			// --> m_post_audptr
	}
	else
		post_audspec(m_audptr);			// --> m_post_audptr
	
	//  Code to do final smoothing; initial implementation will only permit plp-style lpc-based representation,
	//  which will consist of inverse dft followed by autocorrelation-based lpc analysis and an
	//  lpc to cepstrum recursion.
	float lpcgain;
	spec2lpc(&lpcgain);		// m_lpcptr

	// maybe go straight from lpcoefs to cepstra? 
	//   - faster and much closer numeric match to original rasta 
//	if (m_baseInfo.doMain == DOMAIN_CEPSTRA) 
	if (m_baseInfo.bDoCep)
	{
		lpc_to_cep( m_lpcptr, m_cepCoef, m_baseInfo.cepNum);		// -->m_cepCoef

		for (int j=0;j<m_baseInfo.cepNum;j++)
			pCurFrame[j] = m_cepCoef->values[j];

		// 2004.06.30 plu : add
		if (m_rastaKind&HASENERGY)
			pCurFrame[m_BaseDim-1-(m_baseInfo.bPitch?1:0)] = (float) -log((double) 1.0/lpcgain);
	}
	else 
	{
	    // Convert the LPC params to a spectrum 
	    lpc_to_spec(m_lpcptr,m_specCoef);						// --> m_specCoef

//		if (m_baseInfo.doMain == DOMAIN_LOG)
// 		if (!m_baseInfo.bDoCep)
		{
			for (i = 0; i < m_specCoef->length; ++i) 
				pCurFrame[i] = log(m_specCoef->values[i]);
		}
	}
	if (m_baseInfo.bPitch) pCurFrame[m_BaseDim-1] = pitch[p_i]/100.0;

    return energy;
}



// This routine computes the temporally filtered form of
//   the trajectories for each of the parameters included in
//   the input vector. To do this, it internally holds state
//   from the histories of each parameter.
//	
//   The initial, default form uses a "standard" rasta
//   filter, which is a bandpass filter with a delta
//   calculation followed by a single-pole integrator.
//   This form of the routine has no dynamic changing of
//   the filters, and it calls simple routines to set
//   up the integrators and differentiators to be the
//   same for each filter. The calls to these routines,
//   however, could be replaced with other routines
//   which will put in differing filters if needed. 
void RASTA_PLP::rasta_filt()
{
	char *funcname;

	bool init;  // flag which says to not filter, just accumulate history impulse responses 

	funcname = "rasta_filt";

	m_i_call++;
	if (m_i_call > MAXHIST) 
	{
		m_i_call = MAXHIST; // Saturate i_call since we only care if it is greater
						    // than the filter history length;this way it will not wrap around if
						    // we call this program a zillion times  
	}

	fvec_check( funcname, m_audptr, m_lastfilt - 1 );

	// Now for all bands except first and last few (which are
	// left out here but just copied in later on) 
	for(int i = m_first_good; i < m_lastfilt; i++)
	{
		// If we have enough history of the input, do
		//	 the filter computation. 
		if ((m_i_call > (m_fir_filt[i]->length - 1)))
			init = false;
		else
			init = true;

		// Now call the routine to do the actual filtering. 
		m_outptr->values[i] = filt(&m_history,init,m_audptr->values[i],i,
									m_fir_filt[i],m_iir_filt[i]);
	}
}


// This routine implements a bandpass ``RASTA'' style filter,
//   which ordinarily re-integrates a delta-style FIR differentiator
//   with a single pole leaky integrator. However,
//   as the fvecs for numerator and denominator are passed,
//   these can be any linear-time invariant filter with
//   maximum length of MAXHIST. Also, the filters can
//   be different for every call, either changing over
//   time or being different for each nfilt.  (Note: while
//   this potential is in this routine, the calling routines
//   and the command line reader do not currently handle this
//   case). The passed variable
//   nfilt is there so that we can maintain separate
//   histories for the different channels which we are
//   filtering. Because of this internal history, the routine
//   is not generally callable from functions in other files.
//   The routine returns the filter output value.
//   The variable ``init'' is used to say whether we should filter
//   or just build up the history. 
float RASTA_PLP::filt(FHistory *hptr,bool init,float inval,int nfilt,FVEC* numer,FVEC* denom)
{
	int j;
	float sum;

	if(nfilt >= MAXFILTS)
	{
		fprintf(stderr,"filter %d is past %d\n",
			  nfilt, MAXFILTS );
		exit(-1);
	}

	if(numer->length >= MAXHIST)
	{
		fprintf(stderr,"filter %d needs more history than %d\n",
			  numer->length, MAXHIST );
		exit(-1);
	}

	if(denom->length >= MAXHIST)
	{
		fprintf(stderr,"filter %d needs more history than %d\n",
			  denom->length, MAXHIST );
		exit(-1);
	}

	// The current initialization scheme is just to
	// ignore the input until there is enough history
	// in this routine to do the filtering. 

	hptr->filtIN[nfilt][0] = inval;
	sum = 0.0;

	if(init == false)
	{
		// Here we do the FIR filtering. In standard
		// RASTA processing, this comes from a delta
		// calculation. 
		for(j=0; j<numer->length; j++)
			sum += numer->values[j] * hptr->filtIN[nfilt][j];

		// Here we would insert any nonlinear processing
		// that we want between numerator and
		// denominator filtering. For instance,
		// for the standard case of delta coefficient
		// calculation followed by integration,
		// we can insert something like thresholding
		// or median smoothing here. 

		// Now we do the IIR filtering (denominator) 
		for(j=0; j<denom->length; j++)
			sum += denom->values[j] * hptr->filtOUT[nfilt][j];
	}

	// Now shift the data into the histories. This could
	//be done with a circular buffer, but this
	//is probably easier to understand. 
	for(j=numer->length-1; j>0; j--)
		hptr->filtIN[nfilt][j] = hptr->filtIN[nfilt][j-1];
	
	for(j=denom->length-1; j>0; j--)
		hptr->filtOUT[nfilt][j] = hptr->filtOUT[nfilt][j-1];

	hptr->filtOUT[nfilt][0] = sum;

	return sum;
}


void RASTA_PLP::set_freq_axis() 
{
	float hz_in_fsamp;

	int pspeclen = m_FFTNum/2 + 1;		// 512/2 +1 = 257

	// rasta plp 中常用的是 trapezoidal

	/*// plu 2004.06.10_15:55:00
	int axistype = ((m_baseInfo.bDoMel)?FRQAXIS_MEL:
			 (m_baseInfo.bTrapezfilter?FRQAXIS_TRPZ:FRQAXIS_TRNG));

	// what is 1 Hz in fft sampling points 
	hz_in_fsamp = (float)(pspeclen - 1) / (float)(10000000./(m_baseInfo.smpPeriod*2.));

	// Find the fft bin ranges for the critical band filters 
	if (axistype == FRQAXIS_MEL) 
		get_mel_ranges(pspeclen);
	else if (axistype == FRQAXIS_TRPZ) 
		get_trapezoidal_ranges (pspeclen,hz_in_fsamp);
	else	  // assume FRQAXIS_TRNG 
		get_triangular_ranges (pspeclen,hz_in_fsamp);
	
	m_cbweightptr = alloc_fvec(m_cbrange[m_lastfilt-1].end + 1);

	// Now compute the weightings 
	if (axistype == FRQAXIS_MEL) 
		get_mel_cbweights();
	else if (axistype == FRQAXIS_TRPZ) 
		get_trapezoidal_cbweights(hz_in_fsamp);
	else		// FRQAXIS_TRNG 
		get_triangular_cbweights(hz_in_fsamp);
	*/// plu 2004.06.10_15:55:00
}



// Setup the cbrange and cbweight structures for a mel-scaled 
// frequency division, using triangular filters (I guess) 
// ramping down to zero at the center frqs of adjacent bands. 
void RASTA_PLP::get_mel_ranges(int pspeclength)
{   
    double fmax = m_nyqhz;

    int nbins = pspeclength;

    int nchans = m_nfilts - 2*m_first_good;

    double hzPerBin = fmax/nbins;

    double melPerChan = freq2mel(fmax)/(nchans+1);

    double melMin = 0;
    double melCtr = melPerChan;
    double melTop;

    int binMin = (int)ceil(mel2freq(melMin)/hzPerBin);
    int binCtr = (int)ceil(mel2freq(melCtr)/hzPerBin);

    int binTop;
    int cbweightlen = 0;
    int i, ii;

    // first pass sets up the ranges, so we know length of cbweight 
    for (i = 0; i < nchans; ++i) 
	{
		ii = i + m_first_good;
		
		melTop = (i+2)*melPerChan;
		
		binTop = ceil(mel2freq(melTop)/hzPerBin);

		m_frange[ii].start = MAX(0, binMin + 1);
		m_frange[ii].end   = MIN(nbins-1, binTop-1);

		m_cbrange[ii].start = cbweightlen;

		cbweightlen += m_frange[ii].end - m_frange[ii].start;
		
		m_cbrange[ii].end = cbweightlen;
		
		++cbweightlen;
		
		binMin = binCtr;
		binCtr = binTop;
	}
}

// Get the start and end indices for the critical bands using
//   trapezoidal auditory filters 
void RASTA_PLP::get_trapezoidal_ranges(int pspeclength,float hz_in_samp)
{
	int i, wtindex;
	char *funcname;
	float step_barks;
	float f_bark_mid, f_hz_mid, f_hz_low, f_hz_high, 
	f_pts_low, f_pts_high;

	funcname = "get_trapezoidal_ranges";

	// compute filter step in Barks 
	step_barks = m_nyqbar / (float)(m_nfilts - 1);

	// start the critical band weighting array index where we ignore 1st and last bands,
	// as these values are just copied from their neighbors later on. 
	wtindex = 0;

	// Now store all the indices for the ranges of fft bins (powspec) that will be summed up
	// to approximate critical band filters. Similarly save the start points for the frequency 
	// band weightings that implement the filtering. 
	for(i=m_first_good; i<m_lastfilt; i++)
	{
		m_cbrange[i].start = wtindex;

		// get center frequency of the j-th filter in Bark 
		f_bark_mid = i * step_barks;

		// get center frequency of the j-th filter in Hz 
		f_hz_mid = 300. * (exp((double)f_bark_mid / 6.) - exp(-(double)f_bark_mid / 6.));

		// get low-cut frequency of j-th filter in Hz 
		f_hz_low = 300. * (exp((f_bark_mid - 2.5) / 6.) - exp(-(double)(f_bark_mid - 2.5) / 6.));

		// get high-cut frequency of j-th filter in Hz 
		f_hz_high = 300. * (exp((f_bark_mid + 1.3) / 6.) - exp(-(double)(f_bark_mid + 1.3) / 6.));

		f_pts_low = f_hz_low * hz_in_samp;

		m_frange[i].start = ourint((double)f_pts_low);
		if(m_frange[i].start < 0) 
			m_frange[i].start = 0;
		else if(m_frange[i].start > (pspeclength-1) ) 
			m_frange[i].start = pspeclength - 1;

		f_pts_high = f_hz_high * hz_in_samp;
		m_frange[i].end = ourint((double)f_pts_high) ;
		if(m_frange[i].end < 0) 
			m_frange[i].end = 0;
		else if(m_frange[i].end > (pspeclength-1) )
			m_frange[i].end = pspeclength - 1;

		// fprintf(stderr, "f_pts=%.2f..%.2f\n", f_pts_low, f_pts_high); 

		wtindex += m_frange[i].end - m_frange[i].start;
		m_cbrange[i].end = wtindex;
		wtindex++;
	}
}

// Get the start and end indices for the critical bands using
//   triangular auditory filters 
void RASTA_PLP::get_triangular_ranges(int pspeclength,float hz_in_samp)
{
	int i, wtindex;
	char *funcname;
	float step_barks;
	float f_bark_mid, f_hz_mid, f_hz_low, f_hz_high, 
	f_pts_low, f_pts_high;

	funcname = "get_triangular_ranges";

	// compute filter step in Barks 
	step_barks = m_nyqbar / (float)(m_nfilts - 1);

	// start the critical band weighting array index where we ignore 1st and last bands,
	//as these values are just copied	from their neighbors later on. 	wtindex = 0;

	// Now store all the indices for the ranges of fft bins (powspec) that will be summed up
	// to approximate critical band filters. Similarly save the start points for the
	// frequency band weightings that implement the filtering. 
	for(i=m_first_good; i<m_lastfilt; i++)
	{
		(m_cbrange+i)->start = wtindex;

		// get center frequency of the j-th filter in Bark 
		f_bark_mid = i * step_barks;

		// get center frequency of the j-th filter in Hz 
		f_hz_mid = 300. * (exp((double)f_bark_mid / 6.) - exp(-(double)f_bark_mid / 6.));

		// get low-cut frequency of j-th filter in Hz 
		f_hz_low = 300. * (exp((f_bark_mid - 0.5) / 6.) - exp(-(double)(f_bark_mid - 0.5) / 6.));

		// get high-cut frequency of j-th filter in Hz 
		f_hz_high = 300. * (exp((f_bark_mid + 0.5) / 6.) - exp(-(double)(f_bark_mid + 0.5) / 6.));

		f_pts_low = f_hz_low * hz_in_samp;

		m_frange[i].start = ourint((double)f_pts_low);
		if(m_frange[i].start < 0)
			m_frange[i].start = 0;
		
		f_pts_high = f_hz_high * hz_in_samp;
		m_frange[i].end = ourint((double)f_pts_high) ;
		if(m_frange[i].end > (pspeclength-1) )
			m_frange[i].end = pspeclength - 1;
		
		wtindex += (m_frange[i].end - m_frange[i].start);
		m_cbrange[i].end = wtindex;
		wtindex++;
	}
}


// fill in the cbweight structure for the weights 
void RASTA_PLP::get_mel_cbweights()
{   
    int nchans = m_nfilts - 2*m_first_good;

    int binMin, binCtr, binTop;
    int i, ii, j;
    int slopelen;
    double scale;

    float *cbweight = m_cbweightptr->values;

    // cheat - we know first band center is bottom of 2nd band 
    binTop = m_frange[1 + m_first_good].start - 1;

    // fill in all the weights 
    for (i = 0; i < nchans; ++i) 
	{
		ii = i + m_first_good;
		binCtr = binTop;
		binTop = m_frange[ii].end + 1;
		binMin = m_frange[ii].start - 1;
		slopelen = binCtr - binMin;

		// make all weight for a chan sum to 1 
		scale = 2.0/(slopelen * (binTop - binMin));
		for (j = 1; j < slopelen; ++j) 
			*cbweight++ = scale * j;

		slopelen = binTop - binCtr;
		scale = 2.0/(slopelen * (binTop - binMin));
		for (j = 0; j < slopelen; ++j) 
			*cbweight++ = scale * (slopelen-j);
    }
}


// Get the freq domain weights for the equivalent critical band
//   filters using trapezoidal auditory filters 
void RASTA_PLP::get_trapezoidal_cbweights(float hz_in_fsamp)
{
	int i, j, wtindex;
	float f_bark_mid, step_barks;
	double freq_hz, freq_bark, ftmp, logwt;
	char *funcname;

	wtindex = 0;

	funcname = "get_trapezoidal_cbweights";

	// compute filter step in Barks 
	step_barks = m_nyqbar / (float)(m_nfilts - 1);

	for(i=m_first_good; i<m_lastfilt; i++)
	{
		// get center frequency of the j-th filter in Bark 
		f_bark_mid = i * step_barks;

		for(j=m_frange[i].start; j<=m_frange[i].end; j++)
		{
			// get frequency of j-th spectral point in Hz 
			freq_hz = (float) j / hz_in_fsamp;

			// get frequency of j-th spectral point in Bark 
			ftmp = freq_hz / 600.;
			freq_bark = 6. * log(ftmp + sqrt(ftmp * ftmp + 1.));

			// normalize by center frequency in barks: 
			freq_bark -= f_bark_mid;

			// compute weighting 
			if (freq_bark <= -.5) 
			  logwt = (double)(freq_bark + .5);
			else if(freq_bark >= .5)
			  logwt = (-2.5)*(double)(freq_bark - .5);
			else 
			  logwt = 0.0;

			fvec_check( funcname, m_cbweightptr, wtindex );
			m_cbweightptr->values[wtindex] = (float)pow(LOG_BASE, logwt);
			wtindex++;
		}
	}
}


// Get the freq domain weights for the equivalent critical band
//   filters using triangular auditory filters 
void RASTA_PLP::get_triangular_cbweights(float hz_in_fsamp)
{
	int i, j, wtindex;
	float f_bark_mid, step_barks;
	double freq_hz, freq_bark, ftmp;
	char *funcname;

	wtindex = 0;

	funcname = "get_triangular_cbweights";

	// compute filter step in Barks 
	step_barks = m_nyqbar / (float)(m_nfilts - 1);

	for(i=m_first_good; i<m_lastfilt; i++)
	{
		// get center frequency of the j-th filter in Bark 
		f_bark_mid = i * step_barks;
		for(j=m_frange[i].start; j<=m_frange[i].end; j++)
		{
			// get frequency of j-th spectral point in Hz 
			freq_hz = (float) j / hz_in_fsamp;

			// get frequency of j-th spectral point in Bark 
			ftmp = freq_hz / 600.;
			freq_bark = 6. * log(ftmp + sqrt(ftmp * ftmp + 1.));

			// normalize by center frequency in barks: 
			freq_bark -= f_bark_mid;

			// compute weighting 
			if (freq_bark < -0.5)
				m_cbweightptr->values[wtindex] = 0.0f;
			else if (freq_bark < 0.0) 
				m_cbweightptr->values[wtindex] = (float)(2 * freq_bark + 1.0);
			else if (freq_bark < 0.5)
				m_cbweightptr->values[wtindex] = (float)(-2 * freq_bark + 1.0);
			else
				m_cbweightptr->values[wtindex] = 0.0f;

			fvec_check( funcname, m_cbweightptr, wtindex );

			wtindex++;
		}
	}
}


// Calculate modified FIR portion of rasta filter to respect
//   requested fcupper (frequency of the zero) 
FVEC * RASTA_PLP::get_rasta_fir()
{
    int length = 5;	// This is intrinsic to the definition here 
    FVEC *temp = alloc_fvec(length);

    /*// plu 2004.06.10_15:56:15
    float w = 2*M_PI * m_baseInfo.fcupper*(m_baseInfo.stepsize/1000.0);
    */// plu 2004.06.10_15:56:15
	float w = 2*M_PI * UPPER_CUTOFF_FRQ *(m_baseInfo.stepsize/1000.0);
	
    float denom = 0;
    float *array = temp->values;
    int i;

    // check that rasta.h still does what we think 
    assert(length == FIR_COEF_NUM);

	//  The rasta filter has 4 zeros:
	//   - one at dc (0Hz)
	//   - one at fnyq (half the frame rate)
	//   - a complex pair on the unit circle
	//   This last pair is defined as being at 0.5804pi in 'classic' 
	//   rasta, but you may want to move it around to hit a particular
	//   frequency for other frame rates.  Here, we place it by 
	//   frq. 
	//
	//   Thus, the rasta FIR polynomial is (z+1)(z-1)(2z^2 - 4cos(w)z + 2)
	//    == 2.z^4 - 4cos(w).z^3 + (2-2).z^2 + 4cos(w)z -2 
	//    == [2 1 0 -1 -2] when 4cos(w)==-1 => w = 0.5804 pi . 
    array[0] = 2.f;
    array[1] = -4.f * cos(w);
    array[2] = 0.f;
    array[3] = 4.f * cos(w);
    array[4] = -2.f;

    // Normalize the total mag to 1.0 
    for(i=0; i<length; i++) 
		denom += (array[i] * array[i]);
    
    // the original rasta filter forgot to sqrt this denom, so we 
    //   add a factor of sqrt(10) to make the classic values match 
    denom = sqrt(10*denom);
    for(i=0; i<length; i++) 
		array[i] /= denom;

    /*// plu 2004.06.01_14:43:09
    /* Maybe display  // Replaced by ProjectTools 2004.06.01_14:43:09
    if (pptr->debug) {
	static int flag = 0;
	if (!flag) {
	    fprintf(stderr, "rasta fir = %f %f %f %f %f\n", 
		    temp->values[0], temp->values[1], temp->values[2], 
		    temp->values[3], temp->values[4]);
	    flag = 1;
	}
    }
    */// plu 2004.06.01_14:43:09

    return temp;
}   


// Put single pole into iir coefficient array 
FVEC *RASTA_PLP::get_integ()
{
	float *array;
	FVEC *temp;

	temp = alloc_fvec( IIR_COEF_NUM );
	array = temp->values;

	// calculate the polepos from the lower cutoff frq & the framerate 
	/*// plu 2004.06.10_15:37:20
	array[0] = 1 - sin(2*M_PI*m_baseInfo.fclower*(m_baseInfo.stepsize/1000.0));
	*/// plu 2004.06.10_15:37:20

	array[0] = m_baseInfo.fpole;

    /*// plu 2004.06.01_14:45:44
    if (pptr->debug) {
	static int flag = 0;
	if (!flag) {
	    fprintf(stderr, "rasta iir polepos = %f\n", 
		    temp->values[0]);
	    flag = 1;
	}
    }
    */// plu 2004.06.01_14:45:44
	return (temp );
}


void RASTA_PLP::powspec(float *totalE)
{
	int i, fftlength, log2length,reqpts;
	char *funcname;
	float noise;

	funcname = "powspec";

    // Round up 
	log2length = ceil(log((double)(m_frameRate))/log(2.0));

	//  Not currently checking array bounds for fft;
	//	since we pass the input length, we know that
	//	we are not reading from outside of the array.
	//	The power spectral routine should not write past
	//	the fftlength/2 + 1 . 
	fft_pow( m_ffts, m_pspecptr->values,(long)m_FFTNum);
	    
	// Calculate total power 
	if (totalE != NULL) 
	{
	    double E = 0;
	    for(i=0; i<m_pspecptr->length; i++) 
			E += m_pspecptr->values[i];
	    
	    *totalE = E;
	}

	return;
}

void RASTA_PLP::AudSpec()
{
	int i, j, icb, icb_end;
	char *funcname;

	funcname = "AudSpec";

	fvec_check( funcname, m_audptr, m_lastfilt - 1); // bounds-checking for array reference 

	/*// plu 2004.06.10_15:42:00
	for(i=m_first_good; i<m_lastfilt; i++) 
	{
		m_audptr->values[i] = 0.;

		fvec_check( funcname, m_pspecptr, m_frange[i].end );

		icb_end = m_cbrange[i].start + m_frange[i].end - m_frange[i].start;

		fvec_check( funcname, m_cbweightptr, icb_end);

		for(j=m_frange[i].start; j<=m_frange[i].end;j++) 
		{
			icb = m_cbrange[i].start + j - m_frange[i].start;
			m_audptr->values[i] += m_pspecptr->values[j] * m_cbweightptr->values[icb];
		}
	}
	*/// plu 2004.06.10_15:42:00

	/*// plu 2004.06.10_15:43:54
	//     compute auditory spectrum 
	int i_1,i_2;
	i_1 = m_nfilts - 1;
	for (int jfilt = 2; jfilt <= i_1; ++jfilt) 
	{
		m_audptr->values[jfilt - 1] = (float)0.;
		i_2 = ibegen[jfilt + 22];
		for (int kk = ibegen[jfilt - 1]; kk <= i_2; ++kk) 
		{
			icb = ibegen[jfilt + 45] - ibegen[jfilt - 1] + kk;
			m_audptr->values[jfilt - 1] += m_pspecptr->values[kk - 1] * cb[icb - 1];
		}
	}
	*/// plu 2004.06.10_15:43:54

	for(i=m_first_good; i<m_lastfilt; i++) 
	{
		m_audptr->values[i] = 0.;

		for(j=0; j<(m_FFTNum/2+1);j++) 
			m_audptr->values[i] += m_pspecptr->values[j] * m_wts->values[i][j];
	}

}

/*
 *	This routine computes a nonlinearity on an FVEC array (floats).
 *	Currently defined are log(x), log(1+ jah * x), and x.
 *
 *	The first time that this program is called, we do
 *	the usual allocation.
 */
void RASTA_PLP::nl_audspec(void)
{
	int i;
	char *funcname;

	funcname = "nl_audspec";

	fvec_check( funcname, m_audptr, (m_lastfilt - 1) );  // bounds-checking for array reference 

	for(i=m_first_good; i<m_lastfilt; i++)
	{
		if(m_audptr->values[i] < TINY)
			m_audptr->values[i] = TINY;

		m_audptr->values[i] = log((m_audptr->values[i]));
	}
}


/*
 *	This routine computes a nonlinear function of
 *	an FVEC array (floats).
 *	Currently this is  just the exp, which is the inverse
 *	of the log compression used in log rasta.
 *
 *	The first time that this program is called, we do
 *	the usual allocation.
 */
void RASTA_PLP::inverse_nonlin()
{
	int i;
	char *funcname;

	funcname = "inverse_nonlin";

	// We only check on the incoming vector, as the output
	// is allocated here so we know how long it is. 
	//fvec_check(funcname, in, lastfilt - 1);
	for(i=m_first_good; i<m_lastfilt; i++)
	{
		if (m_outptr->values[i] < LOG_MAXFLOAT)
			m_outptr->values[i] = exp((m_outptr->values[i]));
		else
		{
			fprintf(stderr,"Warning (%s): saturating inverse nonlinearity to prevent overflow.\n",funcname);
			fprintf(stderr,"You should run rasta with the -M option.\n\n");
			
			m_outptr->values[i] = MAXFLOAT;
		}
	}
}

/*
 *	This is the file that would get hacked and replaced if
 *	you want to change the preliminary post-auditory band
 * 	analysis. In original form, a critical band -like
 *	analysis is augmented with an equal loudness curve
 *	and cube root compression.

 *	The first time that this program is called, 
 *	in addition to the usual allocations, we compute the
 * 	spectral weights for the equal loudness curve.
 */
void RASTA_PLP::post_audspec(FVEC *audspec)
{
	int i;
	char *funcname;
	float step_barks;
	float f_bark_mid, f_hz_mid ;
	float ftmp, fsq;
	
    // compute filter step in Barks 
    step_barks = m_nyqbar / (float)(m_nfilts - 1);

	funcname = "post_audspec";
	
	fvec_check( funcname, audspec, (m_lastfilt - 1) );

	for(i=m_first_good; i<m_lastfilt; i++)
	{
		// Apply equal-loudness curve 
		m_post_audptr->values[i] = audspec->values[i] * m_eql[i];

		// Apply equal-loudness compression 
		m_post_audptr->values[i] = pow( (double)m_post_audptr->values[i], COMPRESS_EXP);
	}

	// Since the first critical band has center frequency at 0 Hz and
	//bandwidth 1 Bark (which is about 100 Hz there) 
	//we would need negative frequencies to integrate.
	//In short the first filter is JUNK. Since all-pole model always
	//starts its spectrum perpendicular to the y-axis (its slope is
	//0 at the beginning) and the same is true at the Nyquist frequency,
	//we start the auditory spectrum (and also end it) with this slope
	//to get around this junky frequency bands. This is not to say
	//that this operation is justified by anything but it seems
	//to do no harm. - H.H. 
	//8-8-93 Morgan note: in this version, as per request from H.H.,
	//the number of critical band filters is a command-line option.
	//Therefore, if the spacing is less than one bark, more than
	//one filter on each end can be junk. Now the number of
	//copied filter band outputs depends on the number
	//of filters used. 

	for(i=m_first_good; i > 0; i--)
		m_post_audptr->values[i-1] = m_post_audptr->values[i];
	
	for(i=m_lastfilt; i < m_nfilts; i++)
		m_post_audptr->values[i] = m_post_audptr->values[i-1];		
}

// modified version of lpccep.c:lpccep() returns lpc coefficients 
//   of going straight to cepstrum 
FVEC *RASTA_PLP::spec2lpc(float * p_lpcgain)
{   	
    /*// plu 2004.06.30_16:23:32
    float lpcgain;
    */// plu 2004.06.30_16:23:32
    int i;
    char *funcname = "spec2lpc";
	    
    // Do IDFT by multiplying cosine matrix times power values,getting autocorrelations 
    band_to_auto(m_post_audptr, m_autoptr);

    // do Durbin recursion to get predictor coefficients 
    auto_to_lpc( m_autoptr, m_lpcptr, p_lpcgain );

    if(*p_lpcgain<=0) 
	{						// in case of calculation inaccuracy 
		for(i=0; i<m_outptrOLD->length; i++)
			m_lpcptr->values[i] = m_outptrOLD->values[i];
		fprintf(stderr,"Warning: inaccuracy of calculation -> using last frame\n");
    } 
	else 
	{
		if( m_baseInfo.bgainflag) 
		{
			for(int i=0; i<m_lpcptr->length; i++)
			{
				m_lpcptr->values[i] /= *p_lpcgain;
			}

		}
    }

    for(i=0; i<m_lpcptr->length; i++)
		m_outptrOLD->values[i] = m_lpcptr->values[i];
    
    return( m_lpcptr );
}


// Do IDFT by multiplying cosine matrix times power values,
//   getting autocorrelations 
void RASTA_PLP::band_to_auto(FVEC *bandptr,FVEC *autoptr)
{
	static double **wcosptr = (double **)NULL;

	double base_angle,tmp;
	int n_auto, n_freq;
	int i, j;
	char *funcname;
	
	funcname = "band_to_auto";
	
	n_auto = m_baseInfo.lpcOrder + 1;
	n_freq = m_nfilts;

	// check dimension 
	if(bandptr->length != n_freq)
	{
		fprintf(stderr,"input veclength neq number of critical bands\n");
		exit(-1);
	}
	if(autoptr->length != n_auto)
	{
		fprintf(stderr,"output veclength neq autocorrelation length\n");
		exit(-1);
	}
	
	// Builds up a matrix of cosines for IDFT, 1st time 
	if(wcosptr == (double **)NULL)
	{
		// Allocate double matrix 
		wcosptr = (double **)malloc(n_auto * sizeof(double *));
		if(wcosptr == (double **)NULL)
		{
			fprintf(stderr,"Cannot malloc double ptr array (matrix of cosines)\n");
			exit(-1);
		}
		for(i=0; i<n_auto; i++)
		{
			wcosptr[i] = (double *)malloc(n_freq * sizeof(double));
			if(wcosptr[i] == (double *)NULL)
			{
				fprintf(stderr,"Cannot malloc double array (matrix of cosines)\n");
				exit(-1);
			}
		}

		// Builds up a matrix of cosines for IDFT 
		base_angle =  M_PI / (double)(n_freq - 1);  // M_PI is PI, defined in math.h 
		for(i=0; i<n_auto; i++)
		{
			wcosptr[i][0] = 1.0;
			for(j=1; j<(n_freq-1); j++)
				wcosptr[i][j] = 2.0 * cos(base_angle * (double)i * (double)j);

			// No folding over from neg values for Nyquist freq 
			wcosptr[i][n_freq-1] = cos(base_angle * (double)i * (double)(n_freq-1));
		}
	}

	// multiplying cosine matrix times power values 
	for(i=0; i<n_auto; i++)
	{
		tmp = wcosptr[i][0] * (double)bandptr->values[0];
		for(j=1; j<n_freq; j++)
			tmp += wcosptr[i][j] * (double)bandptr->values[j];
		autoptr->values[i] = (float)(tmp / (double)(2. * (n_freq-1)));  // normalize 
	}
}


// This routine computes the solution for an autoregressive
//	model given the autocorrelation vector. 
//	This routine uses essentially the
//	same variables as were used in the original Fortran,
//	as I don't want to mess with the magic therein. 
void RASTA_PLP::auto_to_lpc(FVEC * autoptr, FVEC * lpcptr, float *lpcgain )
{
	float s;
	static float *alp = NULL;
	static float *rc;
	float alpmin, rcmct, aib, aip;
	float *a, *r;
	char *funcname;

	int idx, mct, mct2, ib, ip, i_1, i_2, mh;

	funcname = "auto_to_lpc";

	if(alp == (float *)NULL) // If first time 
	{
		alp = (float *)malloc((m_baseInfo.lpcOrder + 1) * sizeof(float));
		if(alp == (float *)NULL)
		{
			fprintf(stderr,"cant allocate alp\n");
			exit(-1);
		}
		rc = (float *)malloc((m_baseInfo.lpcOrder ) * sizeof(float));
		if(rc == (float *)NULL)
		{
			fprintf(stderr,"cant allocate rc\n");
			exit(-1);
		}
	}

	fvec_check( funcname, lpcptr, m_baseInfo.lpcOrder );
	fvec_check( funcname, autoptr, m_baseInfo.lpcOrder );

	// Move values and pointers over from nice calling
	//	names to Fortran names 
	a = lpcptr->values;
	r = autoptr->values;

	//     solution for autoregressive model 
	a[0] = 1.;
	alp[0] = r[0];
	rc[0] = -(double)r[1] / r[0];
	a[1] = rc[0];
	alp[1] = r[0] + r[1] * rc[0];
	i_2 = m_baseInfo.lpcOrder;
	
	for (mct = 2; mct <= i_2; ++mct) 
	{
        s = 0.;
        mct2 = mct + 2;
        alpmin = alp[mct - 1];
        i_1 = mct;
        for (ip = 1; ip <= i_1; ++ip) 
		{
            idx = mct2 - ip;
            s += r[idx - 1] * a[ip-1];
        }
        rcmct = -(double)s / alpmin;
        mh = mct / 2 + 1;
        i_1 = mh;
		
        for (ip = 2; ip <= i_1; ++ip) 
		{
			ib = mct2 - ip;
			aip = a[ip-1];
			aib = a[ib-1];
			a[ip-1] = aip + rcmct * aib;
			a[ib-1] = aib + rcmct * aip;
        }
        a[mct] = rcmct;
        alp[mct] = alpmin - alpmin * rcmct * rcmct;
        rc[mct-1] = rcmct;
   	}

   	*lpcgain = alp[m_baseInfo.lpcOrder];
}


// This routine computes the cepstral coefficients for an autoregressive model 
// given the prediction vector. 
// This routine uses essentially the same variables as were used in the original Fortran,
// as I don't want to mess with the magic therein. 
// I believe this is the same algorithm as described on page 442 
// of Rabiner & Schafer (1978) "Digital processsing of speech signals"
// section 8.8.2 1999jun09 dpwe@icsi.berkeley.edu 
void RASTA_PLP::lpc_to_cep(FVEC * lpcptr, FVEC * cepptr, int nout)
{
	int ii, j, l, jb;
	float sum;

	static float *c = NULL;

	float *a, *gexp;
	char *funcname;
	
	double d_1, d_2;

	funcname = "lpc_to_cep";

	if(c == (float *)NULL)
	{
		c = (float *)malloc((nout) * sizeof(float));
		if(c == (float *)NULL)
		{
			fprintf(stderr,"cant allocate c\n");
			exit(-1);
		}
	}


	fvec_check( funcname, lpcptr, m_baseInfo.lpcOrder );
                // bounds-checking for array reference 

	fvec_check( funcname, cepptr, nout - 1 );
                // bounds-checking for array reference 

	// Move values and pointers over from calling
	//	names to local array names 

	a = lpcptr->values;
	gexp = cepptr->values;

    // Function Body 
    c[0] = -log(a[0]);
    c[1] = -(double)a[1] / a[0];

    for (l = 2; l < nout; ++l) 
	{
       	if (l <= m_baseInfo.lpcOrder ) 
        	sum = l * a[l] / a[0];
		else 
			sum = 0.;

		for (j = 2; j <= l; ++j) 
		{
            jb = l - j + 2;
            if (j <= (m_baseInfo.lpcOrder + 1) ) 
				sum += a[j-1] * c[jb - 1] * (jb - 1) / a[0];
   		}

		c[l] = -(double)sum / l;
    }

	
	gexp[0] = c[0];
    for (ii = 2; ii <= nout; ++ii) 
	{
        if (m_baseInfo.fcepLifter > 0.f ) 
		{
			d_1 = (double) ((ii - 1));
			d_2 = (double) (m_baseInfo.fcepLifter);
			gexp[ii-1] = pow(d_1, d_2) * c[ii - 1];
        } 
		else 
			gexp[ii-1] = c[ii - 1];
   	}
}


// convert lpc coefficients directly to a spectrum 
void RASTA_PLP::lpc_to_spec(FVEC * lpcptr,FVEC * specptr )
{   
    int order = lpcptr->length;
    int nfpts = specptr->length ;

    // assume the lpc params are gain*[1 -a1 -a2 -a3] etc 
    int i, j;
    double re, im, sc, cf;
    
	// looking at this compared to the mapped output of lpccep, 
	//   nfpts is the right answer, if nfpts = nfilts 
    double pionk = M_PI/(nfpts - 1);	// same as for cep2spec 

    sc = lpcptr->values[0];
    for(i = 0; i < specptr->length; ++i) 
	{
		re = 1.0; im = 0;

		// figure complex val of 1 - sum(ai.z^-i) where z = expijpi/n 
		for(j = 1; j < lpcptr->length; ++j) 
		{
			cf = lpcptr->values[j]/sc;
			re += cf * cos(i*j*pionk);
			im -= cf * sin(i*j*pionk);
		}

		// specptr->values[i] = 1 / hypot(re, im); 
		// Spec is power - so take square 
		specptr->values[i] = 1 / (sc* (re*re + im*im));
	}
}


// Allocate FVEC, and return pointer to it. 
FVEC *RASTA_PLP::alloc_fvec( int veclength )
{
	FVEC *vecptr;

	vecptr = (FVEC *)malloc (sizeof(FVEC) );
	if(vecptr == (FVEC *)NULL)
	{
		fprintf(stderr,"Can't allocate %ld bytes for FVEC\n",
                                sizeof(FVEC) );
		exit(-1);
	}
	vecptr->length = veclength;
	vecptr->values = (float *)malloc ((veclength) * sizeof(float) );
	if(vecptr->values == (float *)NULL)
	{
		fprintf(stderr,"Can't allocate %ld bytes for vector\n",
                                (veclength) * sizeof(float) );
		exit(-1);
	}
	// clear it 
	memset(vecptr->values, 0, veclength*sizeof(float));

    return (vecptr);
} 


// Routine to check that it is OK to access an array element.
// Use this in accordance with your level of paranoia; if truly
// paranoid, use it before every array reference. If only moderately
// paranoid, use it once per loop with the indices set to the
// largest value they will achieve in the loop. You don't need to use this
// at all, of course. 
//
//	The routine accesses a global character array that is supposed
// to hold the name of the calling function. Of course if you
// write a new function and don't update this value, this fature won't work. 
void RASTA_PLP::fvec_check(char *funcname, const struct FVEC *vec, int index )
{
	if((index >= vec->length) || (index < 0))
	{
		fprintf(stderr,"Tried to access %dth elt, array length=%d\n",
			index + 1, vec->length);
		fprintf(stderr,"\tIn routine %s\n", funcname);
		fflush(stderr);
		abort();
	}
}



//  The following code assumes:
//    (1) that `double' has more bits of significance than `int'.
//    (2) that conversions from `int' to `double' are made without error.
//    (3) that if x and y are approximately equal double-precision values,
//        that x - y will be computed without error.
//  No rounding behavior is assumed, other than what is required by the
//  C language (K&R or ANSI).
//
//  Thanks to John Hauser for this implementation of the function.
int RASTA_PLP::ourint(double x) 
{
	int result;
	double fraction;

	if (x > ((double)INT_MAX ) - 0.5) return INT_MAX;
	if (x < ((double)INT_MIN ) + 0.5) return INT_MIN;

	result = x;
	fraction = x - result;
	if (fraction < 0) 
	{
	
		if (fraction == -0.5) return (result & ~0x01);

		if (fraction > -0.5) 
			return result;
		else 
			return (result - 1);
	}
	else 
	{
		if (fraction == 0.5)	return ((result + 1) & ~0x01);
		
		if (fraction < 0.5) 	return result;
		else 					return (result + 1);
	}
}


/*// plu 2004.06.10_15:44:08
int RASTA_PLP::adw_(long* npoint,long*  nfilt,float* cb, float* eql, long* ibegen,float* sf)
{
    // System generated locals 
    long i_1, i_2;
    float r_1, r_2;
    double d_1;


    // Local variables 
    static float freq, zdel, rsss;
    static long i, j;
    static float x, z, f0, z0, f2samp, fh, fl, fnqbar;
    static long icount;
    static float fsq;

    // Parameter adjustments 
    --cb;
    ibegen -= 24;

    // Function Body 
//     subroutine computes auditory weighting functions 
//     inputs: 
//     npoint - number of points in the fft spectrum 
//     nfilt - number of samples of auditory spectrum 
//     equally spaced on the bark scale; 
//     1st filter at dc and last at the Nyquist frequency 
//     outputs: 
//     cb - array of weighting coefficients to simulate 
//     critical band spectral resolution 
//     eql - array of weighting coefficients to simulate 
//     equal loudness sensitivity of hearing 
//     on npoint speech power spectrum 
//     ibegen - three-dimensional array which indicates where 
//     to begin and where to end integration 
//     of speech spectrum and where the given 
//     weighting function starts in array cb 
//     get Nyquist frequency in Bark 

    r_1 = *sf / (float)1200.;
    fnqbar = log((double)*sf / (double)1200. + sqrt(r_1 * r_1 + (double)1.)) * (double) 6.;
//     compute number of filters for less than 1 Bark spacing 
    *nfilt = (long ) fnqbar + 2;
//     frequency -> fft spectral sample conversion 
    f2samp = (float) (*npoint - 1) / (*sf / (float)2.);
//     compute filter step in Bark 
    zdel = fnqbar / (float) (*nfilt - 1);

//     loop over all weighting functions 

    icount = 1;
    i_1 = *nfilt - 1;
    for (j = 2; j <= i_1; ++j) 
	{
		ibegen[j + 69] = icount;

	//     get center frequency of the j-th filter in Bark 
		z0 = zdel * (float) (j - 1);

	//     get center frequency of the j-th filter in Hz 
		f0 = (exp((double)z0 / 6.) - exp(-(double)z0 / 6)) * (float)600. / (float) 2.;

	//     get low-cut frequency of j-th filter in Hz 
		fl = (exp((z0 - (double)2.5) / 6) - exp(-(double)(z0 - (float)2.5) / 6)) * (float)600. / (float)2.;

		r_1 = fl * f2samp;
		ibegen[j + 23] = (((double)r_1)+0.5) + 1;

		if (ibegen[j + 23] < 1) 
			ibegen[j + 23] = 1;

		//     get high-cut frequency of j-th filter in Hz 
		fh = (exp((z0 + (double)1.3) / 6) - exp(-(double)(z0 + (float)1.3) / 6)) * (float)600. / (float)2.;

		r_1 = fh * f2samp;
		ibegen[j + 46] = (((double)r_1)+0.5) + 1;
		if (ibegen[j + 46] > *npoint) 
			ibegen[j + 46] = *npoint;
		
		//     do-loop over the power spectrum 
		i_2 = ibegen[j + 46];
		for (i = ibegen[j + 23]; i <= i_2; ++i) 
		{
			// get frequency of j-th spectral point in Hz 
			freq = (float) (i - 1) / f2samp;

			// get frequency of j-th spectral point in Bark 
			x = freq / (float)600.;
			r_1 = x;
			z = log(x + sqrt(r_1 * r_1 + (double)1.)) * (float)6.;
			
			// normalize by center frequency in barks: 
			z -= z0;
			
			// compute weighting 
			if (z <= (float)-.5)
			{
				d_1 = (double) (z + (float).5);
				cb[icount] = pow(10., d_1);
			} 
			else if (z >= (float).5) 
			{
				d_1 = (double) ((z - (float).5) * (float)-2.5);
				cb[icount] = pow(10., d_1);
			} 
			else 
			{
				cb[icount] = (float)1.;
			}

			//     calculate the LOG 40 db equal-loudness curve 
			//     at center frequency 
			r_1 = f0;
			fsq = r_1 * r_1;
			r_1 = fsq;
			r_2 = fsq + (float)1.6e5;
			rsss = r_1 * r_1 * (fsq + (float)1.44e6) / (r_2 * r_2 * (fsq + (float)9.61e6));

			//     take log and put the equal-loundness curve into eql array 
			eql[j] = log((double)rsss);
			
			++icount;
		}
    }
    return 0;
} // adw_ 
*/// plu 2004.06.10_15:44:08

