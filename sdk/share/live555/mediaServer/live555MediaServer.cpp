/**********
This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the
Free Software Foundation; either version 2.1 of the License, or (at your
option) any later version. (See <http://www.gnu.org/copyleft/lesser.html>.)

This library is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
**********/
// Copyright (c) 1996-2012, Live Networks, Inc.  All rights reserved
// LIVE555 Media Server
// main program

#include "liveMedia.hh"
#include <BasicUsageEnvironment.hh>
#include "DynamicRTSPServer.hh"
#include "version.hh"
#include "GroupsockHelper.hh"
#include "IteAirTsStreamSource.h"

//#define tsi_dev_index 0
#define TRANSPORT_PACKET_SIZE 188
#define TRANSPORT_PACKETS_PER_NETWORK_PACKET 7
// The product of these two numbers must be enough to fit within a network packet

//#define stream_RTP_multicast
/*RTP : rtp://@xxx.xxx.xxx.xxx:portnum*/
/*UDP : udp://@xxx.xxx.xxx.xxx:portnum*/
//#define USE_SSM

UsageEnvironment* env;
char const* inputFileName = "test.ts";
FramedSource* videoSource = NULL;
IteAirTsStreamSource* iteAirSource = NULL;

//Unicast
RTSPServer* rtspServer;

//Multicast
RTPSink* rtpvideoSink = NULL;
BasicUDPSink* udpvideoSink = NULL;
Groupsock* grtpGroupsock = NULL;

static bool gbUnicast = false;
static char gStopEventLoop = 0;

//void play(); // forward

extern "C" int live555MediaServer_main(int argc, char** argv)
{
  // Begin by setting up our usage environment:
    TaskScheduler* scheduler = BasicTaskScheduler::createNew();
    env = BasicUsageEnvironment::createNew(*scheduler);
    gStopEventLoop = 0;

    if (argc)
    {
        gbUnicast = true;
    }
    else
    {
        gbUnicast = false;
    }

    if (gbUnicast)
    {
        // Create the RTSP server.  Try first with the default port number (554),
        // and then with the alternative port number (8554):

        portNumBits rtspServerPortNum = 554;
        UserAuthenticationDatabase* authDB = NULL;
#ifdef ACCESS_CONTROL
  // To implement client access control to the RTSP server, do the following:
  authDB = new UserAuthenticationDatabase;
  authDB->addUserRecord("username1", "password1"); // replace these with real strings
  // Repeat the above with each <username>, <password> that you wish to allow
  // access to the server.
#endif

        rtspServer = DynamicRTSPServer::createNew(*env, rtspServerPortNum, authDB, 10);
        if (rtspServer == NULL) {
        *env << "Failed to create RTSP server in port: " << rtspServerPortNum << "\n";
        rtspServerPortNum = 8554;
        rtspServer = DynamicRTSPServer::createNew(*env, rtspServerPortNum, authDB, 10);
        }
        if (rtspServer == NULL) {
        *env << "Failed to create RTSP server: " << env->getResultMsg() << "\n";
        exit(1);
        }

        *env << "LIVE555 Media Server\n";
        *env << "\tversion " << MEDIA_SERVER_VERSION_STRING
           << " (LIVE555 Streaming Media library version "
           << LIVEMEDIA_LIBRARY_VERSION_STRING << ").\n";

  char* urlPrefix = rtspServer->rtspURLPrefix();
  
  *env << "Play streams from this server using the URL\n\t"
       << urlPrefix << "<filename>\nwhere <filename> is a file present in the current directory.\n";
  *env << "Each file's type is inferred from its name suffix:\n";
  *env << "\t\".264\" => a H.264 Video Elementary Stream file\n";
  *env << "\t\".aac\" => an AAC Audio (ADTS format) file\n";
  *env << "\t\".ac3\" => an AC-3 Audio file\n";
  *env << "\t\".amr\" => an AMR Audio file\n";
  *env << "\t\".dv\" => a DV Video file\n";
  *env << "\t\".m4e\" => a MPEG-4 Video Elementary Stream file\n";
  *env << "\t\".mkv\" => a Matroska audio+video+(optional)subtitles file\n";
  *env << "\t\".mp3\" => a MPEG-1 or 2 Audio file\n";
  *env << "\t\".mpg\" => a MPEG-1 or 2 Program Stream (audio+video) file\n";
  *env << "\t\".ts\" => a MPEG Transport Stream file\n";
  *env << "\t\t(a \".tsx\" index file - if present - provides server 'trick play' support)\n";
  *env << "\t\".wav\" => a WAV Audio file\n";
  *env << "\t\".webm\" => a WebM audio(Vorbis)+video(VP8) file\n";

  // *env << "See http://www.live555.com/mediaServer/ for additional documentation.\n";

  // Also, attempt to create a HTTP server for RTSP-over-HTTP tunneling.
  // Try first with the default HTTP port (80), and then with the alternative HTTP
  // port numbers (8000 and 8080).

        if (rtspServer->setUpTunnelingOverHTTP(80) || rtspServer->setUpTunnelingOverHTTP(8000) || rtspServer->setUpTunnelingOverHTTP(8080)) {
        *env << "(We use port " << rtspServer->httpServerPortNum() << " for optional RTSP-over-HTTP tunneling, or for HTTP live streaming (for indexed Transport Stream files only).)\n";
        } else {
        *env << "(RTSP-over-HTTP tunneling is not available.)\n";
        }
        env->taskScheduler().doEventLoop(&gStopEventLoop); // does not return
        *env << "End schedular...\n";

		printf("%s %d\n", __FILE__, __LINE__);
        Medium::close(rtspServer);
        env->reclaim();
        delete scheduler;
        return 0;
    }
    else
    {
        // Create 'groupsocks' for RTP and RTCP:
        char const* destinationAddressStr
#ifdef USE_SSM
        = "232.255.42.42";
#else
        = "239.255.42.42";
        //= "239.255.0.1";
        // Note: This is a multicast address.  If you wish to stream using
        // unicast instead, then replace this string with the unicast address
        // of the (single) destination.  (You may also need to make a similar
        // change to the receiver program.)
#endif
        const unsigned short rtpPortNum = 5004;
        const unsigned short rtcpPortNum = rtpPortNum+1;
        const unsigned char ttl = 30; // low, in case routers don't admin scope

        struct in_addr destinationAddress;
        destinationAddress.s_addr = our_inet_addr(destinationAddressStr);

        const Port rtpPort(rtpPortNum);
#ifdef stream_RTP_multicast
        const Port rtcpPort(rtcpPortNum);
#endif

        Groupsock rtpGroupsock(*env, destinationAddress, rtpPort, ttl);
#ifdef stream_RTP_multicast
        Groupsock rtcpGroupsock(*env, destinationAddress, rtcpPort, ttl);
#endif
#ifdef USE_SSM
        rtpGroupsock.multicastSendOnly();
        rtcpGroupsock.multicastSendOnly();
#endif
        grtpGroupsock = &rtpGroupsock;

#if 0
#ifdef stream_RTP_multicast
        // Create an appropriate 'RTP sink' from the RTP 'groupsock':
        rtpvideoSink =
          SimpleRTPSink::createNew(*env, &rtpGroupsock, 33, 90000, "video", "MP2T",
        			     1, True, False /*no 'M' bit*/);
#else
        udpvideoSink = BasicUDPSink::createNew(*env, &rtpGroupsock);  			     
#endif
#endif
        // Finally, ready to streaming:
        *env << "ready to streaming...\n";
        env->taskScheduler().doEventLoop(&gStopEventLoop); // does not return
        *env << "End schedular...\n";
        //env->reclaim();
        delete scheduler;
        return 0;
    }
}

extern "C" int live555MediaServer_Termiate()
{
    if (!gbUnicast)
    {
        if (udpvideoSink)
        {
            udpvideoSink->stopPlaying();
            usleep(10000);

            Medium::close(udpvideoSink);

            if (videoSource)
            {
                Medium::close(videoSource);
            }
            udpvideoSink = NULL;
            videoSource = NULL;
        }
        if (rtpvideoSink)
        {
            rtpvideoSink->stopPlaying();
            usleep(10000);
            Medium::close(rtpvideoSink);
            if (videoSource)
            {
                Medium::close(videoSource);
            }
            rtpvideoSink = NULL;
            videoSource = NULL;
        }
    }
    gStopEventLoop = 1;
}

void afterPlaying(void* /*clientData*/) 
{
    *env << "...done reading from live stream\n";
    Medium::close(videoSource);
}

extern "C" int stop_play(void)
{
    if (gbUnicast == false)
    {
        *env << "...stop reading from live stream\n";
        if (udpvideoSink)
        {
            udpvideoSink->stopPlaying();
            usleep(10000);
            Medium::close(udpvideoSink);
            if (videoSource)
            {
                Medium::close(videoSource);
            }
            udpvideoSink = NULL;
            videoSource = NULL;
        }
        if (rtpvideoSink)
        {
            rtpvideoSink->stopPlaying();
            usleep(10000);
            Medium::close(rtpvideoSink);
            if (videoSource)
            {
                Medium::close(videoSource);
            }
            rtpvideoSink = NULL;
            videoSource = NULL;
        }
    }
}

extern "C" int play(int tsi_dev_index,
					  bool stream_reset,	
                      bool udp_stream,
                      bool multicast,
                      char *addr,
                      uint32_t port,
                      char* pKey)
{
    unsigned const inputDataChunkSize
        = TRANSPORT_PACKETS_PER_NETWORK_PACKET*TRANSPORT_PACKET_SIZE;

    if (gbUnicast == false)
    {
        if(grtpGroupsock == NULL)
            return 1;

        if( stream_reset == true)
        {
        	// Create 'groupsocks' for RTP and RTCP:
        	const unsigned short new_rtpPortNum = port;
        	const unsigned short new_rtcpPortNum = new_rtpPortNum+1;
        	const unsigned char ttl = 30; // low, in case routers don't admin scope

        	struct in_addr newdestinationAddr;
        	newdestinationAddr.s_addr = our_inet_addr(addr);
        	const Port new_rtpPort(new_rtpPortNum);
#ifdef stream_RTP_multicast
        	const Port new_rtcpPort(new_rtcpPortNum);
#endif

            if( udpvideoSink != NULL )
        	{
                //*env << "...change groupsock and close udpvideoSink sink\n";
                udpvideoSink->stopPlaying();
                usleep(10000);
                Medium::close(udpvideoSink);
                if (videoSource)
                {
                    Medium::close(videoSource);
                }
                udpvideoSink = NULL;
                videoSource = NULL;
            }
            if( rtpvideoSink != NULL )
        	{
                //*env << "...change groupsock and close rtpvideoSink sink\n";
                rtpvideoSink->stopPlaying();
                usleep(10000);
                Medium::close(rtpvideoSink);
                if (videoSource)
                {
                    Medium::close(videoSource);
                }
                Medium::close(videoSource);
                rtpvideoSink = NULL;
                videoSource = NULL;
            }
        	grtpGroupsock->changeDestinationParameters(newdestinationAddr, new_rtpPort, ttl);

            if(udp_stream == true)
            {
                *env << "...create udp\n";
                udpvideoSink = BasicUDPSink::createNew(*env, grtpGroupsock, inputDataChunkSize);
				printf("\nURL udp://@%s:%u \n\n", addr, new_rtpPortNum);
        	}
        	else
        	{
                *env << "...create rtp\n";
                rtpvideoSink =
                SimpleRTPSink::createNew(*env, grtpGroupsock, 33, 90000, "video", "MP2T",
                	         1, True, False /*no 'M' bit*/);
				printf("\nURL rtp://@%s:%u \n\n", addr, new_rtpPortNum);
			}
        }
        else
        {
            *env << "...stop reading from live stream\n";
            if(udp_stream == true)
            {
                *env << "...stop udp\n";
                udpvideoSink->stopPlaying();
        	}
        	else
        	{
                *env << "...stop rtp\n";
                rtpvideoSink->stopPlaying();
        	}  
            usleep(10000);
        	Medium::close(videoSource);
        }
#ifdef CFG_IPTV_TX
        iteAirSource = IteAirTsStreamSource::createNew(*env, tsi_dev_index, inputDataChunkSize);
#endif
        if( iteAirSource == NULL )
            printf("\tCreate IteAirTsStreamSource fail !!\n");

        // Create a framer for the Transport Stream:
        videoSource = MPEG2TransportStreamFramer::createNew(*env, iteAirSource);

        if( videoSource == NULL )    printf(" fail !!!! %s()[#%d]\n", __FUNCTION__, __LINE__);

        // Finally, start playing:
        *env << "Beginning to read from live stream...\n";
        if(udp_stream == true)
        {
            udpvideoSink->startPlaying(*videoSource, afterPlaying, udpvideoSink);
        }
        else
        {
            rtpvideoSink->startPlaying(*videoSource, afterPlaying, rtpvideoSink);
        }
        return 0;
    }
}

extern "C" void changeStreamParameters()
{
  // Create 'groupsocks' for RTP and RTCP:
  char const* destinationAddressStr
#ifdef USE_SSM
  = "232.255.42.42";
#else
  = "239.255.42.42";
  //= "192.168.1.1";
#endif  
  //Groupsock changeDestinationParameters();
}

#ifdef CFG_IP_SECURITY_MODE
extern "C" void setSessionKey(uint8_t* pKey)
{
	iteAirSource->setSessionKey(pKey);
}
#endif
