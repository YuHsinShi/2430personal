#include <math.h>
#include "fft.h"
 
extern complex_of_N_FFT data_of_N_FFT[N_FFT];
extern ElemType SIN_TABLE_of_N_FFT[Npart4_of_N_FFT+1];
 

void CREATE_SIN_TABLE(void)
{
	int i=0; 
	for(i=0; i<=Npart4_of_N_FFT; i++)
	{
		SIN_TABLE_of_N_FFT[i] = sin(PI*i/Npart2_of_N_FFT);//SIN_TABLE[i] = sin(PI2*i/N);
	}
}
 
ElemType Sin_find(ElemType x)
{
    int i = (int)(N_FFT*x);
    i = i>>1;// i = i / 2;
    if(i>Npart4_of_N_FFT)
    {
        i = Npart2_of_N_FFT - i;//i = i - 2*(i-Npart4);
    } 
    return SIN_TABLE_of_N_FFT[i];
}
ElemType Cos_find(ElemType x)
{
    int i = (int)(N_FFT*x);
    i = i>>1;
    if(i < Npart4_of_N_FFT ) 
    { 
        //i = Npart4 - i;
        return SIN_TABLE_of_N_FFT[Npart4_of_N_FFT - i];
    } 
    else //i > Npart4 && i < N/2 
    {
        //i = i - Npart4;
        return -SIN_TABLE_of_N_FFT[i - Npart4_of_N_FFT];
    }
}
 

void ChangeSeat(complex_of_N_FFT *DataInput)
{
	int nextValue,nextM,i,k,j=0;
    complex_of_N_FFT temp;
	
    nextValue=N_FFT/2;
    nextM=N_FFT-1;
    for (i=0;i<nextM;i++)
    {
        if (i<j)
        {
            temp=DataInput[j];
            DataInput[j]=DataInput[i];
            DataInput[i]=temp;
        }
        k=nextValue;
        while (k<=j)
        {
            j=j-k;
            k=k/2;
        }
        j=j+k;					
    }                                       
}                                           


void FFT(void)
{
	int L=0,B=0,J=0,K=0;
	int step=0, KB=0;
	//ElemType P=0;
	ElemType angle;
	complex_of_N_FFT W,Temp_XX;
	
	ChangeSeat(data_of_N_FFT);
	//CREATE_SIN_TABLE();
	for(L=1; L<=M_of_N_FFT; L++)
	{
		step = 1<<L;//2^L
		B = step>>1;//B=2^(L-1)
		for(J=0; J<B; J++)
		{
			//P = (1<<(M-L))*J;//P=2^(M-L) *J 
			angle = (double)J/B;
			W.imag =  -Sin_find(angle);
			W.real =   Cos_find(angle);	
			//W.real =  cos(angle*PI);
			//W.imag = -sin(angle*PI);
			for(K=J; K<N_FFT; K=K+step)
			{
				KB = K + B;

				Temp_XX.real = data_of_N_FFT[KB].real * W.real-data_of_N_FFT[KB].imag*W.imag;
				Temp_XX.imag = W.imag*data_of_N_FFT[KB].real + data_of_N_FFT[KB].imag*W.real;
				
				data_of_N_FFT[KB].real = data_of_N_FFT[K].real - Temp_XX.real;
				data_of_N_FFT[KB].imag = data_of_N_FFT[K].imag - Temp_XX.imag;
				
				data_of_N_FFT[K].real = data_of_N_FFT[K].real + Temp_XX.real;
				data_of_N_FFT[K].imag = data_of_N_FFT[K].imag + Temp_XX.imag;
			}
		}
	}
}
 

void IFFT(void)
{
	int L=0,B=0,J=0,K=0;
	int step=0, KB=0;
	//ElemType P=0;
	ElemType angle;
	complex_of_N_FFT W,Temp_XX;
	
	ChangeSeat(data_of_N_FFT);
	//CREATE_SIN_TABLE();
	for(L=1; L<=M_of_N_FFT; L++)
	{
		step = 1<<L;//2^L
		B = step>>1;//B=2^(L-1)
		for(J=0; J<B; J++)
		{
			//P = (1<<(M-L))*J;//P=2^(M-L) *J 
			angle = (double)J/B;
			
			W.imag =   Sin_find(angle);	
			W.real =   Cos_find(angle);		
			//W.real =  cos(angle*PI);
			//W.imag = -sin(angle*PI);
			for(K=J; K<N_FFT; K=K+step)
			{
				KB = K + B;

				Temp_XX.real = data_of_N_FFT[KB].real * W.real-data_of_N_FFT[KB].imag*W.imag;
				Temp_XX.imag = W.imag*data_of_N_FFT[KB].real + data_of_N_FFT[KB].imag*W.real;
				
				data_of_N_FFT[KB].real = data_of_N_FFT[K].real - Temp_XX.real;
				data_of_N_FFT[KB].imag = data_of_N_FFT[K].imag - Temp_XX.imag;
				
				data_of_N_FFT[K].real = data_of_N_FFT[K].real + Temp_XX.real;
				data_of_N_FFT[K].imag = data_of_N_FFT[K].imag + Temp_XX.imag;
			}
		}
	}
}
 

void Init_FFT()
{
	CREATE_SIN_TABLE();			
}
 

void Close_FFT(void)
{
	
}