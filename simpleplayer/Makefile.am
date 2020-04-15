sbin_PROGRAMS=simpleplayer

simpleplayer_SOURCES=main.cpp simpleplayer.cpp sdlwrapper.cpp

av_sdk_dir=../../av_sdk
av_sdk_inc_dir=${av_sdk_dir}/include
av_sdk_lib_dir=${av_sdk_dir}/lib

simpleplayer_CPPFLAGS= -std=c++11 -I${av_sdk_inc_dir} $(SDL2_CFLAGS)
simpleplayer_LDADD= ${av_sdk_lib_dir}/libavformat.a ${av_sdk_lib_dir}/libavcodec.a ${av_sdk_lib_dir}/libavutil.a ${av_sdk_lib_dir}/libavfilter.a ${av_sdk_lib_dir}/libfdk-aac.a ${av_sdk_lib_dir}/libx264.a ${av_sdk_lib_dir}/libswscale.a ${av_sdk_lib_dir}/libswresample.a -lz -lws2_32 -lole32 -lSecur32 -lBcrypt ${SDL2_LIBS} -liconv
#simpleplayer_LDFLAGS=-lz -lole32

