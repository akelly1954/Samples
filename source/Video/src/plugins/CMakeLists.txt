cmake_minimum_required (VERSION 3.10)

project (VideoPlugin_V4L2)

include (${SampleRoot_DIR}/source/cmake/VideoPlugin.cmake)

file(GLOB SOURCEFILES                     "common/*.c*" )
file(GLOB PLUGIN_V4L2_SOuRCEFILES         "v4l2/*.c*" )
##### file(GLOB PLUGIN_OPENCV_SOuRCEFILES "src/plugins/opencv/*.c*" )

file(GLOB HEADERS "${VideoPlugin_V4L2_HEADERS}" )   ##########  /plugins/*.h*" )

file(GLOB LOGGER_HEADERS "${LoggerCpp_HEADERS}/LoggerCpp/*.h*")
file(GLOB JSONCPP_HEADERS "${JsonCpp_HEADERS}/JsonCpp/*.h*")
set( SOURCES ${SOURCEFILES} ${PLUGIN_V4L2_SOuRCEFILES} ${HEADERS} "${LOGGER_HEADERS}" ${JSONCPP_HEADERS} )

set ( VideoPlugin_V4L2  "VideoPlugin_V4L2${DBG}" )
set ( LIBTYPE           "${VideoPlugin_LIBTYPE}" )

include_directories( ${Video_HEADERS} 
                     ${VideoPlugin_V4L2_HEADERS}
                     ${Util_HEADERS}
                     ${EnetUtil_HEADERS}
                     ${LoggerCpp_HEADERS}
                     ${JsonCpp_HEADERS} )

add_library( ${VideoPlugin_V4L2} ${LIBTYPE} ${SOURCES} )
target_link_libraries( ${VideoPlugin_V4L2}
                       ${EnetUtil_LIB}
                       ${Util_LIB}
                       ${LoggerCpp_LIB}
                       ${JsonCpp_LIB}
##########                       ${OpenCV_LIBS}
                       ${CMAKE_THREAD_LIBS_INIT} ${LINKOPTIONS}
                     )

# install(FILES ${VideoPlugin_V4L2_HEADERS} DESTINATION include/Video/src/plugins)
# install(FILES ${LOGGER_HEADERS} DESTINATION include/LoggerCpp)
install(FILES ${JSONCPP_HEADERS} DESTINATION include/JsonCpp)

install(TARGETS ${VideoPlugin_V4L2} DESTINATION lib)
install(TARGETS ${VideoPlugin_V4L2} DESTINATION localrun)

set (Util_LIB "${SampleRoot_DIR}/build/Util/libUtil.so")
set (EnetUtil_LIB "${SampleRoot_DIR}/build/EnetUtil/libEnetUtil.so")

