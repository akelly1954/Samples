cmake_minimum_required (VERSION 3.10)

project (EnetUtil)

include (${SampleRoot_DIR}/source/cmake/EnetUtil.cmake)

file(GLOB MAIN_HEADERS "src/main_programs/include/ntwk_basic_sock_server/*.hpp")
file(GLOB LOGGER_HEADERS "${LoggerCpp_HEADERS}/LoggerCpp/*.h*")
file(GLOB SOURCEFILES "src/*.cpp")
file(GLOB HEADERS   "include/*.h*") 
file(GLOB TEST_SCRIPTS "src/main_programs/*.bash")
    
set( SOURCES ${SOURCEFILES} ${MAIN_HEADERS} ${HEADERS} ${LOGGER_HEADERS} )

set ( LIBTYPE "${Enet_LIBTYPE}" )
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lstdc++fs")

include_directories( ${EnetUtil_HEADERS}
                     src/main_programs/include
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
install(FILES ${MAIN_HEADERS} DESTINATION include/EnetUtil/ntwk_basic_sock_server)
install(FILES ${LOGGER_HEADERS} DESTINATION include/LoggerCpp)

install(FILES ${TEST_SCRIPTS} 
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
                    GROUP_EXECUTE GROUP_WRITE GROUP_READ
                    WORLD_EXECUTE WORLD_READ
        DESTINATION localrun/tests)

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
# main_ntwk_fixed_array main
#
set (main_ntwk_fixed_array "main_ntwk_fixed_array${DBG}")
add_executable (main_ntwk_fixed_array src/main_programs/main_ntwk_fixed_array.cpp)
target_link_libraries( main_ntwk_fixed_array
                            ${EnetUtil_LIB}
                            ${Util_LIB}
                            ${LoggerCpp_LIB}
                            ${CMAKE_THREAD_LIBS_INIT}
                            ${LINKOPTIONS}
                     )
install(TARGETS main_ntwk_fixed_array DESTINATION localrun)

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
# main_ntwk_basic_sock_server main
#
set ( "main_ntwk_basic_sock_server${DBG}")
add_executable (main_ntwk_basic_sock_server src/main_programs/main_ntwk_basic_sock_server.cpp)
target_link_libraries( main_ntwk_basic_sock_server
                            ${EnetUtil_LIB}
                            ${Util_LIB}
                            ${LoggerCpp_LIB}
                            ${CMAKE_THREAD_LIBS_INIT}
                            ${LINKOPTIONS}
                     )
install(TARGETS main_ntwk_basic_sock_server DESTINATION localrun)

#
# main_client_for_basic_server main
#
set ( "main_client_for_basic_server${DBG}")
add_executable (main_client_for_basic_server src/main_programs/main_client_for_basic_server.cpp)
target_link_libraries( main_client_for_basic_server
                            ${EnetUtil_LIB}
                            ${Util_LIB}
                            ${LoggerCpp_LIB}
                            ${CMAKE_THREAD_LIBS_INIT}
                            ${LINKOPTIONS}
                     )
install(TARGETS main_client_for_basic_server DESTINATION localrun)

# Dependencies
add_dependencies (main_enet_util ${EnetUtil} ${Util} )
add_dependencies (main_ntwk_fixed_array ${EnetUtil} ${Util} )
add_dependencies (main_ntwk_util ${EnetUtil} ${Util} )
add_dependencies (main_ntwk_basic_sock_server ${EnetUtil} ${Util} )
add_dependencies (main_client_for_basic_server ${EnetUtil} ${Util} )


