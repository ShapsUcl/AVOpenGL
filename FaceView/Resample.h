// Resample - block-level sampling-rate changing
//
// Mark Huckvale - University College London
//
// version 1 - June 2015
//

#pragma once
#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef PI
#define PI (3.14159265358979232846)
#endif

#ifndef PI2
#define PI2 (6.28318530717958465692)
#endif

#define D2R (0.01745329348)          /* (2*pi)/360 */
#define R2D (57.29577951)            /* 360/(2*pi) */

#ifndef MAX
#define MAX(x,y) ((x)>(y) ?(x):(y))
#endif
#ifndef MIN
#define MIN(x,y) ((x)<(y) ?(x):(y))
#endif

#ifndef ABS
#define ABS(x)   ((x)<0   ?(-(x)):(x))
#endif

#ifndef SGN
#define SGN(x)   ((x)<0   ?(-1):((x)==0?(0):(1)))
#endif

/* Accuracy */

#define Npc 4096

typedef struct resample_t {
   float  *Imp;
   float  *ImpD;
   float   LpScl;
   unsigned int   Nmult;
   unsigned int   Nwing;
   double  minFactor;
   double  maxFactor;
   unsigned int   XSize;
   float  *X;
   unsigned int   Xp; /* Current "now"-sample pointer for input */
   unsigned int   Xread; /* Position to put new samples */
   unsigned int   Xoff;
   unsigned int   YSize;
   float  *Y;
   unsigned int  Yp;
   double  Time;
} rsdata;

class CResample
{
private:
	rsdata	*hp;
	BOOL interpFilt; /* TRUE means interpolate filter coeffs */
public:
	CResample(void);
	~CResample(void);
	// initialise to specific sampling rate change factor (highfactor=1, min=max=factor);
	int init(double factor);
	// initialise to range of sampling rate change factors
	int open(int highQuality, double minFactor, double maxFactor);
	// close and free memory
	int close(void);
	// process a buffer with a given factor
	int process(double  factor,float *inBuffer,int inBufferLen,int lastFlag,int *inBufferUsed,float *outBuffer,int outBufferLen);
	// process a buffer with factor given at initialisation
	int process(float *inBuffer,int inBufferLen,int lastFlag,int *inBufferUsed,float *outBuffer,int outBufferLen);
	// get the size of the filter being used
	int get_filter_width() { return hp->Xoff; }
	// get number of currently buffered samples left over
	int GetHeldOverCount(void);

protected:
	int lrsSrcUp(float X[],float Y[],double factor,double *TimePtr,unsigned int Nx,unsigned int Nwing,float LpScl,float Imp[],float ImpD[],BOOL Interp);
	int lrsSrcUD(float X[],float Y[],double factor,double *TimePtr,unsigned int Nx,unsigned int Nwing,float LpScl,float Imp[],float ImpD[],BOOL Interp);
	void lrsLpFilter(double c[], int Nf, double frq, double Beta, int Num);
	float lrsFilterUp(float Imp[],float ImpD[],unsigned int Nwing,BOOL Interp,float *Xp,double Ph,int Inc);
	float lrsFilterUD(float Imp[],float ImpD[],unsigned int Nwing,BOOL Interp,float *Xp,double Ph,int Inc,double dhb);

};

