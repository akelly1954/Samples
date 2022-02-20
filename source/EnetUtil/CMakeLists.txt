cmake_minimum_required (VERSION 3.10)

project (EnetUtil)

include (${SampleRoot_DIR}/source/cmake/EnetUtil.cmake)

file(GLOB SOURCEFILES "src/*.cpp")
file(GLOB HEADERS "include/*.h*" "${LoggerCpp_HEADERS}/LoggerCpp/*.h*")
set( SOURCES ${SOURCEFILES} ${HEADERS} )

set ( LIBTYPE "${Enet_LIBTYPE}" )
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lstdc++fs")

include_directories( ${EnetUtil_HEADERS}
                     ../Util/include
                     ${LoggerCpp_HEADERS}
                   )

add_library( ${EnetUtil} ${LIBTYPE} ${SOURCES} )
target_link_libraries( ${EnetUtil}
                       ${CMAKE_THREAD_LIBS_INIT} ${LINKOPTIONS}
                     )

install(TARGETS ${EnetUtil} DESTINATION lib)
install(TARGETS ${EnetUtil} DESTINATION localrun)
install(FILES ${HEADERS} DESTINATION include/EnetUtil)

set (Util_LIB "${SampleRoot_DIR}/build/Util/libUtil.so")

#
# EnetEnetUtils main
#
set (main_enet_util "main_enet_util${DBG}")
add_executable (main_enet_util src/main_programs/main_enet_util.cpp)
target_link_libraries( main_enet_util
                            ${EnetUtil_LIB}
                            ${Util_LIB}
                            ${LoggerCpp_LIB}
                            ${CMAKE_THREAD_LIBS_INIT}
                            ${LINKOPTIONS}
                     )
install(TARGETS main_enet_util DESTINATION localrun)

#
# main_fixed_array main
#
set (main_fixed_array "main_fixed_array${DBG}")
add_executable (main_fixed_array src/main_programs/main_fixed_array.cpp)
target_link_libraries( main_fixed_array
                            ${EnetUtil_LIB}
                            ${Util_LIB}
                            ${LoggerCpp_LIB}
                            ${CMAKE_THREAD_LIBS_INIT}
                            ${LINKOPTIONS}
                     )
install(TARGETS main_fixed_array DESTINATION localrun)

#
# main_ntwk_util main
#
set ( "main_ntwk_util${DBG}")
add_executable (main_ntwk_util src/main_programs/main_ntwk_util.cpp)
target_link_libraries( main_ntwk_util
                            ${EnetUtil_LIB}
                            ${Util_LIB}
                            ${LoggerCpp_LIB}
                            ${CMAKE_THREAD_LIBS_INIT}
                            ${LINKOPTIONS}
                     )
install(TARGETS main_ntwk_util DESTINATION localrun)

#
# main_simple_server main
#
set ( "main_simple_server${DBG}")
add_executable (main_simple_server src/main_programs/main_simple_server.cpp)
target_link_libraries( main_simple_server
                            ${EnetUtil_LIB}
                            ${Util_LIB}
                            ${LoggerCpp_LIB}
                            ${CMAKE_THREAD_LIBS_INIT}
                            ${LINKOPTIONS}
                     )
install(TARGETS main_simple_server DESTINATION localrun)

# Dependencies
add_dependencies (main_enet_util ${EnetUtil} ${Util} )
add_dependencies (main_fixed_array ${EnetUtil} ${Util} )
add_dependencies (main_ntwk_util ${EnetUtil} ${Util} )
add_dependencies (main_simple_server ${EnetUtil} ${Util} )
