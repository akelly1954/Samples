cmake_minimum_required (VERSION 3.10)

project (Video)

include (${SampleRoot_DIR}/source/cmake/Video.cmake)

file(GLOB TEST_SCRIPTS "src/main_programs/*.bash")
file(GLOB SOURCEFILES "src/*.c*" "src/common/*.c*" "src/opencv/*.c*" "src/v4l2/*.c*" )

file(GLOB HEADERS "include/*.h*")
file(GLOB LOGGER_HEADERS "${LoggerCpp_HEADERS}/LoggerCpp/*.h*")
file(GLOB JSONCPP_HEADERS "${JsonCpp_HEADERS}/JsonCpp/*.h*")
set( SOURCES ${SOURCEFILES} ${HEADERS} "${LOGGER_HEADERS}" ${JSONCPP_HEADERS} )

file(GLOB JSON_FILES "src/main_programs/*.json")

set ( LIBTYPE "${Video_LIBTYPE}" )
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")

include_directories( ${Video_HEADERS} 
                     ../Util/include
                     ../EnetUtil/include
                     ${LoggerCpp_HEADERS}
                     ${JsonCpp_HEADERS}} )

add_library( ${Video} ${LIBTYPE} ${SOURCES} )
target_link_libraries( ${Video}
                        ${EnetUtil_LIB}
                        ${Util_LIB}
                        ${LoggerCpp_LIB}
                        ${JsonCpp_LIB}
                       ${CMAKE_THREAD_LIBS_INIT} ${LINKOPTIONS}
                     )

install(TARGETS ${Video} DESTINATION lib)
install(TARGETS ${Video} DESTINATION localrun)
install(FILES ${HEADERS} DESTINATION include/Video)
install(FILES ${LOGGER_HEADERS} DESTINATION include/LoggerCpp)
install(FILES ${JSONCPP_HEADERS} DESTINATION include/JsonCpp)

install(FILES ${JSON_FILES} DESTINATION Video)
install(FILES ${JSON_FILES} DESTINATION localrun)

install(FILES ${TEST_SCRIPTS}
        PERMISSIONS OWNER_EXECUTE OWNER_READ
                    GROUP_EXECUTE GROUP_READ
                    WORLD_EXECUTE WORLD_READ
        DESTINATION localrun)

set (Util_LIB "${SampleRoot_DIR}/build/Util/libUtil.so")
set (EnetUtil_LIB "${SampleRoot_DIR}/build/EnetUtil/libEnetUtil.so")

##############################
# main_video_capture main
##############################

set (main_video_capture "main_video_capture${DBG}")
add_executable (main_video_capture src/main_programs/main_video_capture.cpp)
target_link_libraries( main_video_capture 
                            ${Video_LIB}
                            ${EnetUtil_LIB}
                            ${Util_LIB}
                            ${LoggerCpp_LIB}
                            ${JsonCpp_LIB}
                            ${CMAKE_THREAD_LIBS_INIT} 
                            ${LINKOPTIONS}
                     )
install(TARGETS main_video_capture DESTINATION localrun)

# Dependencies
add_dependencies (main_video_capture ${Video} ${Util})

