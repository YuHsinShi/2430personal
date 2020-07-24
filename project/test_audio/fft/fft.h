#include <math.h>

#define FFT_RESULT(x) 	(sqrt(data_of_N_FFT[x].real*data_of_N_FFT[x].real+data_of_N_FFT[x].imag*data_of_N_FFT[x].imag))
#define IFFT_RESULT(x)	(data_of_N_FFT[x].real/N_FFT)
 
#define PI  3.14159265358979323846264338327950288419716939937510
#define PI2 6.28318530717958647692528676655900576839433879875021
#define N_FFT			1024		
#define M_of_N_FFT		10			
#define Npart2_of_N_FFT	512			
#define Npart4_of_N_FFT	256
 
typedef float ElemType;
 
typedef struct
{
	ElemType real,imag;
}complex_of_N_FFT,*ptr_complex_of_N_FFT;

complex_of_N_FFT data_of_N_FFT[N_FFT];
ElemType SIN_TABLE_of_N_FFT[Npart4_of_N_FFT+1];