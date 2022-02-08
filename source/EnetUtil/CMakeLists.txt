cmake_minimum_required (VERSION 3.10)

project (EnetUtil)

include (${SampleRoot_DIR}/source/cmake/EnetUtil.cmake)

file(GLOB SOURCEFILES "src/*.cpp")
file(GLOB HEADERS "include/*.h*")
set( SOURCES ${SOURCEFILES} ${HEADERS} )

# include_directories( ${EnetUtil_HEADERS} 
#                      ${EnetUtilCommon_HEADERS} 
#                      ${Util_HEADERS} 
#                      ${UtilCommon_HEADERS} 
#                    )

include_directories( ${EnetUtil_HEADERS} 
                     ${Util_HEADERS} 
                   )

add_library( ${EnetUtil} ${LIBTYPE} ${SOURCES} )
target_link_libraries( ${EnetUtil}
                       ${CMAKE_THREAD_LIBS_INIT} ${LINKOPTIONS}
                     )

install(TARGETS ${EnetUtil} DESTINATION lib)
install(TARGETS ${EnetUtil} DESTINATION localrun)
install(FILES ${HEADERS} DESTINATION include/EnetUtil)

#
# EnetEnetUtils main
#
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lstdc++fs")
set (main_enet_util "main_enet_util${DBG}")
add_executable (main_enet_util src/main_programs/main_enet_util.cpp)
target_link_libraries( main_enet_util 
                            ${EnetUtil_LIB}
                            ${Util_LIB}
                            ${CMAKE_THREAD_LIBS_INIT} 
                            ${LINKOPTIONS}
                     )
install(TARGETS main_enet_util DESTINATION localrun)

# Dependencies
add_dependencies (main_enet_util ${EnetUtil} ${Util} )
