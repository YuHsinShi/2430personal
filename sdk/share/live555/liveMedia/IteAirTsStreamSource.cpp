
#if defined(CFG_DEMOD_ENABLE) || defined(CFG_IPTV_TX)

#include <fcntl.h>
#include <sys/ioctl.h>
#include "ite/itp.h"
#include "../../../driver/itp/itp_demod_thread.h"
#include "IteAirTsStreamSource.h"

//#include "ite/itp_dbg.h"
//=============================================================================
//				  Constant Definition
//=============================================================================


//=============================================================================
//				  Macro Definition
//=============================================================================


//=============================================================================
//				  Structure Definition
//=============================================================================


//=============================================================================
//				  Global Data Definition
//=============================================================================
static bool ts_used = false;

#ifdef __cplusplus
    extern "C" 
    {
        TSAF_HANDLE  *g_pHTsaf[MAX_TSI_SUPPORT_CNT] = {0};
    }
#endif

extern "C" char* getConfigKey(void);

//=============================================================================
//				  Private Function Definition
//=============================================================================


//=============================================================================
//				  Public Function Definition
//=============================================================================
int
IteAirTsStreamSource
::createNew_Probe(
    UsageEnvironment &env,
    int              demod_idx)
{
    int         result = -1;
    uint32_t    value = 0;
    char        serviceName[32] = {0};
    int         demodFd = -1;

    do{
        if( demod_idx < MAX_DEMOD_SUPPORT_CNT && !g_pHTsaf[demod_idx] )
        {
            printf(" demode init fail !! %s()[#%d]\n", __FUNCTION__, __LINE__);
            break;
        }

        result = 0;
    }while(0);

    return result;
}

#if defined (CFG_PURE_TS_STREAM) || defined(CFG_IPTV_TX)
IteAirTsStreamSource*
IteAirTsStreamSource
::createNew(
    UsageEnvironment &env,
    int              tsi_dev_idx,
    uint32_t         preferredSampleSize)
{
    IteAirTsStreamSource *iteAirTsSource 
        = new IteAirTsStreamSource(env, tsi_dev_idx, preferredSampleSize);
    return iteAirTsSource;
}

uint8_t convertChar2Hex(char c)
{
    if (c >= '0' && c <= '9') return (c - '0');
    else if (c >= 'a' && c <='f') return (c - 'a' + 10);
    else if (c >= 'A' && c <='F') return (c - 'A' + 10);
    else return 0;
}

IteAirTsStreamSource
::IteAirTsStreamSource(
    UsageEnvironment &env,
    int              tsi_dev_idx,
    uint32_t         preferredSampleSize)
: FramedSource(env), _pHTsaf(g_pHTsaf[tsi_dev_idx]), 
  _preferredSampleSize(preferredSampleSize), _lastPlayTime(0),
  _bLimitNumBytesToStream(False), _numBytesToStream(0)
{
	//tsaf_Open(&_pHTsaf, 0, 0);
	ts_used = true;

#ifdef CFG_IP_SECURITY_MODE
    //open DPU engine for AES encryption/decryption.
    {
        //convert char key to hex
        uint8_t key[16];
        char configKey[33];
        strncpy(configKey, getConfigKey(), sizeof(configKey) -1);

        for(int i =0; i<16; i++)
        {
            int a,b;
            a = convertChar2Hex(configKey[i*2]);
            b = convertChar2Hex(configKey[i*2+1]);

            key[i] = (a<<4) | b;
            //printf("0x%02x %d %d %c %c\n",key[i], a, b , configKey[i*2], configKey[i*2+1]);
        }

        _dpuFd = open(":dpu:aes", 0);
        if (_dpuFd < 0)
        {
            printf("DPU init is failed\n");
            while(1);
        }
        else
        {
            int keyLen = 128;
            uint8_t cipherMode = ITP_DPU_CIPHER_ECB;
            uint8_t decryMode = 0;
            memcpy(_pSessionKey, key, 16);

            ioctl(_dpuFd, ITP_IOCTL_SET_KEY_LENGTH, &keyLen); //key.len, key.ptr
            ioctl(_dpuFd, ITP_IOCTL_SET_KEY_BUFFER, _pSessionKey); //key.len, key.ptr
            ioctl(_dpuFd, ITP_IOCTL_SET_CIPHER, &cipherMode); //key.len, key.ptr
            ioctl(_dpuFd, ITP_IOCTL_SET_DESCRYPT, &decryMode); //key.len, key.ptr
            {
                int i = 0;
                printf("Tx Session Key is:................\n");
                for (i = 0; i < 16; i++)
                {
                    printf("0x%02X, ", _pSessionKey[i]);
                }
                printf("\n");
            }
        }

		_bEncryptStream = 0;
		printf("dpu Fd: %d\n", _dpuFd);
	}
#endif

    return;
}

IteAirTsStreamSource
::~IteAirTsStreamSource()
{    
    do{
        //tsaf_Close(&_pHTsaf, 0);
		ts_used = false;
    }while(0);
    
    return;
}

bool IteAirTsStreamSource
::get_ts_status(void)
{
	return ts_used;
}

#else
IteAirTsStreamSource*
IteAirTsStreamSource
::createNew(
    UsageEnvironment &env,
    int              demod_idx,
    int              service_idx,
    uint32_t         preferredSampleSize)
{
    IteAirTsStreamSource *iteAirTsSource 
        = new IteAirTsStreamSource(env, demod_idx, service_idx, preferredSampleSize);

    return iteAirTsSource;
}

IteAirTsStreamSource
::IteAirTsStreamSource(
    UsageEnvironment &env,
    int              demod_idx,
    int              service_idx,
    uint32_t         preferredSampleSize)
: FramedSource(env), _pHTsaf(g_pHTsaf[demod_idx]), 
  _demod_idex(demod_idx), _service_idex(service_idx),
  _preferredSampleSize(preferredSampleSize), _lastPlayTime(0),
  _bLimitNumBytesToStream(False), _numBytesToStream(0)
{
    //tsaf_Open(&_pHTsaf, service_idx, 0);
    return;
}

IteAirTsStreamSource
::~IteAirTsStreamSource()
{
    do{
        //tsaf_Close(&_pHTsaf, 0);
    }while(0);
    
    return;
}
#endif

#ifdef CFG_IP_SECURITY_MODE
void IteAirTsStreamSource
::setSessionKey(uint8_t* pKey)
{
}

int IteAirTsStreamSource
::isSitContent(unsigned char* pBuffer, int size, int* pIsHdcp, int* pOffset)
{
	int remainSize = 0;
	int i = 0;
	unsigned long pid = 0;
	unsigned char* pCurPtr = pBuffer;
	int startIdx = 0;
	#define SIT_PID 			0x77
	#define TABLE_START_OFFSET 	5

	for (i = 0; i < size; i++)
	{
		if (pBuffer[i] == 0x47 && pBuffer[i + 188] == 0x47)
		{
			startIdx = i;
			*pOffset = startIdx;
			break;
		}
	}
	remainSize = size - startIdx;
	pCurPtr = &pBuffer[startIdx];
	//Assume all data from JEDI is correct
	for (i = 0; i < remainSize; i += 188)
	{
		pid = ((unsigned long) (pCurPtr[i + 1] & 0x1F) << 8) | pCurPtr[i + 2];
		if (pid == SIT_PID && pCurPtr[i] == 0x47)
		{
			if (pCurPtr[i + TABLE_START_OFFSET + 8] & 0x80)
			{
				*pIsHdcp = 1;
			}
			else
			{
				*pIsHdcp = 0;
			}
			return 1;
		}
	}
	return 0;
}
#endif

extern "C" uint32_t
taskStreamMuxGetSample(
    void* pBuffer,
    uint32_t bufferSize);


void IteAirTsStreamSource
::doGetNextFrame() 
{
#define GET_TS_TIME_OUT     1000 // ms

    do{
        struct timeval startT;
        uint64_t duration_time = 0;
        uint32_t realSize = 0;
		int isHdcpContent = 0;
		int startOffset = 0;
	    uint8_t pTmpBuffer[1504] = { 0 };

        if( _preferredSampleSize > 0 && _preferredSampleSize < fMaxSize )
        {
            fMaxSize = _preferredSampleSize;
        }
		realSize = taskStreamMuxGetSample(fTo, fMaxSize);
        //tsaf_Read(&_pHTsaf, pTmpBuffer, fMaxSize, &realSize, 0);
		
        fFrameSize = (unsigned)realSize;
#ifdef CFG_IP_SECURITY_MODE
		//if (isSitContent(pTmpBuffer, realSize, &isHdcpContent, &startOffset) && tsaf_get_stream_hdcp())
		if (isSitContent(fTo, realSize, &isHdcpContent, &startOffset) && tsaf_get_stream_hdcp())
		{
			if (_bEncryptStream != isHdcpContent)
			{
				printf("New HDCP Status %d, Old HDCP Status: %d\n", isHdcpContent, _bEncryptStream);
			}
			_bEncryptStream = isHdcpContent;
		}

		// not alignment
		if (startOffset && (realSize - startOffset) > 0)
		{
			//memcpy(pTmpBuffer, &pTmpBuffer[startOffset], realSize - startOffset);
            memcpy(fTo, &fTo[startOffset], realSize - startOffset);
            //realSize = taskStreamMuxGetSample(&pTmpBuffer[realSize - startOffset], startOffset);
            realSize = taskStreamMuxGetSample(&fTo[realSize - startOffset], startOffset);
			//tsaf_Read(&_pHTsaf, &pTmpBuffer[realSize - startOffset], startOffset, &realSize, 0);
		}

		//if (_bEncryptStream && fFrameSize == 1316)
		if (tsaf_get_stream_hdcp() && fFrameSize == 1316)
		{
		    //open if CPU write back is on
            //ithFlushDCacheRange(fTo, 1312); //-h->nal_length_size);
            //ithFlushMemBuffer();

			write(_dpuFd, fTo, 1312);
	        read(_dpuFd, fTo, 1312);

            //open if CPU write back is on
            //ithInvalidateDCacheRange(fTo, 1312);
        }
#endif
		//if (fFrameSize)
		//{
		//	memcpy(fTo, pTmpBuffer, fFrameSize);
		//}
        gettimeofday(&startT, NULL);

        // Set the 'presentation time':
        if( _playTimePerFrame > 0 && _preferredSampleSize > 0 )
        {
            fPresentationTime.tv_sec = 0;
            fPresentationTime.tv_usec = 0;
            //if( fPresentationTime.tv_sec == 0 && fPresentationTime.tv_usec == 0 )
            //{
            //    // This is the first frame, so use the current time:
            //    gettimeofday(&fPresentationTime, NULL);
            //}
            //else
            //{
            //    // Increment by the play time of the previous data:
            //    unsigned uSeconds = fPresentationTime.tv_usec + _lastPlayTime;
            //    fPresentationTime.tv_sec += uSeconds / 1000000;
            //    fPresentationTime.tv_usec = uSeconds % 1000000;
            //}
            //
            //// Remember the play time of this data:
            //_lastPlayTime = (_playTimePerFrame * fFrameSize) / _preferredSampleSize;
            //fDurationInMicroseconds = _lastPlayTime;
        }
        else
        {
            // We don't know a specific play time duration for this data,
            // so just record the current time as being the 'presentation time':
            fPresentationTime.tv_sec = 0;
            fPresentationTime.tv_usec = 0;            
            //gettimeofday(&fPresentationTime, NULL);
        }
        fDurationInMicroseconds = 0;
        // Inform the downstream object that it has data:
        FramedSource::afterGetting(this);
        //printf("doGetNextFrame end\n");
    }while(0);
    
    return;
}
#endif
