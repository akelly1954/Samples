cmake_minimum_required (VERSION 3.10)

project (Video)

include (${SampleRoot_DIR}/source/cmake/Video.cmake)

file(GLOB TEST_SCRIPTS "src/main_programs/*.bash")
file(GLOB SOURCEFILES "src/*.c" "src/*.cpp")
file(GLOB HEADERS "include/*.h*")
file(GLOB LOGGER_HEADERS "${LoggerCpp_HEADERS}/LoggerCpp/*.h*")
set( SOURCES ${SOURCEFILES} ${HEADERS} "${LOGGER_HEADERS}" )

set ( LIBTYPE "${Video_LIBTYPE}" )
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")

include_directories( ${Video_HEADERS} 
                     ../Util/include
                     ../EnetUtil/include
                     ${LoggerCpp_HEADERS} )

add_library( ${Video} ${LIBTYPE} ${SOURCES} )
target_link_libraries( ${Video}
                       ${CMAKE_THREAD_LIBS_INIT} ${LINKOPTIONS}
                     )

install(TARGETS ${Video} DESTINATION lib)
install(TARGETS ${Video} DESTINATION localrun)
install(FILES ${HEADERS} DESTINATION include/Video)
install(FILES ${LOGGER_HEADERS} DESTINATION include/LoggerCpp)

install(FILES ${TEST_SCRIPTS}
        PERMISSIONS OWNER_EXECUTE OWNER_READ
                    GROUP_EXECUTE GROUP_READ
                    WORLD_EXECUTE WORLD_READ
        DESTINATION localrun)

set (Util_LIB "${SampleRoot_DIR}/build/Util/libUtil.so")
set (EnetUtil_LIB "${SampleRoot_DIR}/build/EnetUtil/libEnetUtil.so")
#
# v4l2_raw_capture main
#
set (main_v4l2_raw_capture "main_v4l2_raw_capture${DBG}")
add_executable (main_v4l2_raw_capture src/main_programs/main_v4l2_raw_capture.cpp)
target_link_libraries( main_v4l2_raw_capture 
                            ${Video_LIB}
                            ${EnetUtil_LIB}
                            ${Util_LIB}
                            ${LoggerCpp_LIB}
                            ${CMAKE_THREAD_LIBS_INIT} 
                            ${LINKOPTIONS}
                     )
install(TARGETS main_v4l2_raw_capture DESTINATION localrun)

# Dependencies
add_dependencies (main_v4l2_raw_capture ${Video} ${Util})

