
#ifndef __ITE_AIR_TS_STREAM_H_Y1QLUWI5_O2O6_UENM_RKQ2_Y7FZTXPMO18C__
#define __ITE_AIR_TS_STREAM_H_Y1QLUWI5_O2O6_UENM_RKQ2_Y7FZTXPMO18C__

//#if defined(CFG_DEMOD_ENABLE) || defined(CFG_IPTV_TX)
#if 1

#ifndef _FRAMED_SOURCE_HH
    #include "FramedSource.hh"
#endif

#include "ts_airfile/ite_ts_airfile.h"
#define CFG_IP_SECURITY_MODE 1

//=============================================================================
//                  Class Definition
//=============================================================================
class IteAirTsStreamSource: public FramedSource 
{
    public:
#ifdef CFG_IPTV_TX
        static int
            createNew_Probe(UsageEnvironment &env, int tsi_dev_idx);

        static IteAirTsStreamSource* 
            createNew(UsageEnvironment& env, int tsi_dev_idx, uint32_t preferredSampleSize); 
#else
        static int 
            createNew_Probe(UsageEnvironment &env, int demod_idx);
            
        static IteAirTsStreamSource* 
            createNew(UsageEnvironment& env, int demod_idx, int service_idx, uint32_t preferredSampleSize); 
#endif
#ifdef CFG_IP_SECURITY_MODE
        void setSessionKey(uint8_t* pKey);
#endif
		static bool 
			get_ts_status(void);

    protected:
#ifdef CFG_IPTV_TX
        // called only by createNew()
        IteAirTsStreamSource(UsageEnvironment& env, 
                             int        tsi_dev_idx,
                             uint32_t   preferredSampleSize);

        virtual ~IteAirTsStreamSource();
#else
        // called only by createNew()
        IteAirTsStreamSource(UsageEnvironment& env, 
                             int        demod_idx,
                             int        service_idx,
                             uint32_t   preferredSampleSize);
        
        virtual ~IteAirTsStreamSource();
#endif
        uint32_t        _preferredSampleSize;
#ifdef CFG_IP_SECURITY_MODE
        int             _dpuFd;
        uint8_t         _pSessionKey[16];
        int             _bEncryptStream;
#endif
    private:
        //===================================================
        // Private functions
        //===================================================
#ifdef CFG_IP_SECURITY_MODE  
        int isSitContent(unsigned char* pBuffer, int size, int* pIsHdcp, int* pOffset);
#endif
        //===================================================
        // Redefined virtual functions
        //===================================================
        virtual void doGetNextFrame();

        //===================================================
        // Private Parameters
        //===================================================
        int             _fid; // posix file hanlde
        int             _demod_idex;
        int             _service_idex;
        TSAF_HANDLE     *_pHTsaf;
        
        unsigned        _preferredFrameSize;
        unsigned        _playTimePerFrame;
        unsigned        _lastPlayTime;
        Boolean         _bLimitNumBytesToStream;
        u_int64_t       _numBytesToStream; // used iff "fLimitNumBytesToStream" is True
};

#endif

#endif
