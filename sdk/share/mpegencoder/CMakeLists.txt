INCLUDE_DIRECTORIES(
    ${PROJECT_SOURCE_DIR}/sdk/share/mpegencoder
	${PROJECT_SOURCE_DIR}/sdk/include
    ${CMAKE_CURRENT_SOURCE_DIR}
    )


ADD_LIBRARY(mpegencoder STATIC
    av_helpers.c
    av_helpers.h
    config.h
    mencoder.c
    mencoder.h
    cache/cache_file.c
    cache/cache_file.h
    libavcodec/avpacket.c
    libavformat/allformats.c
    libavformat/avc.c
    libavformat/aviobuf.c
    libavformat/isom.c
    libavformat/isom.h
    libavformat/movenc.c
    libavformat/movenc.h
    libavformat/mux.c
    libavformat/options.c
    libavformat/riff.c
    libavformat/riff.h
    libavformat/utils.c
    libavutil/mathematics.c
    libavutil/rational.c
    libavutil/rational.h
    libavutil/mem.c
    libavutil/mem.h
    libmpdemux/muxer.c
    libmpdemux/muxer.h
    #libmpdemux/muxer_avi.c
    libmpdemux/muxer_lavf.c
    libmpdemux/mp_taglists.c
    libmpdemux/mp_taglists.h
    stream/stream.c
    stream/stream.h
    stream/stream_file.c
    )

