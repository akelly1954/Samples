cmake_minimum_required (VERSION 3.10)

project (VideoPlugins)

include (${SampleRoot_DIR}/source/cmake/Video.cmake)

file(GLOB SOURCEFILES "common/*.c*" )

##### file(GLOB PLUGIN_V4L2_SOuRCEFILES "src/plugins/v4l2/*.c*" )
##### file(GLOB PLUGIN_OPENCV_SOuRCEFILES "src/plugins/opencv/*.c*" )

file(GLOB HEADERS "../../include/plugins/*.h*")
file(GLOB LOGGER_HEADERS "${LoggerCpp_HEADERS}/LoggerCpp/*.h*")
file(GLOB JSONCPP_HEADERS "${JsonCpp_HEADERS}/JsonCpp/*.h*")
set( SOURCES ${SOURCEFILES} ${HEADERS} "${LOGGER_HEADERS}" ${JSONCPP_HEADERS} )

set ( VideoPlugin_V4L2 "VideoPlugin_V4L2${DBG}" )
set ( LIBTYPE "${Video_LIBTYPE}" )

include_directories( ${HEADERS} 
                     ${Video_HEADERS} 
                     ../../../Util/include
                     ../../../EnetUtil/include
                     ${LoggerCpp_HEADERS}
                     ${JsonCpp_HEADERS}} )

add_library( ${VideoPlugin_V4L2} ${LIBTYPE} ${SOURCES} )
target_link_libraries( ${VideoPlugin_V4L2}
                       ${EnetUtil_LIB}
                       ${Util_LIB}
                       ${LoggerCpp_LIB}
                       ${JsonCpp_LIB}
                       ${OpenCV_LIBS}
                       ${CMAKE_THREAD_LIBS_INIT} ${LINKOPTIONS}
                     )

install(TARGETS ${VideoPlugin_V4L2} DESTINATION lib)
install(TARGETS ${VideoPlugin_V4L2} DESTINATION localrun)
install(FILES ${HEADERS} DESTINATION include/Video/plugins)
install(FILES ${LOGGER_HEADERS} DESTINATION include/LoggerCpp)
install(FILES ${JSONCPP_HEADERS} DESTINATION include/JsonCpp)

set (Util_LIB "${SampleRoot_DIR}/build/Util/libUtil.so")
set (EnetUtil_LIB "${SampleRoot_DIR}/build/EnetUtil/libEnetUtil.so")

