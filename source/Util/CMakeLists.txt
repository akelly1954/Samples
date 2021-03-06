cmake_minimum_required (VERSION 3.10)

project (Util)

include (${SampleRoot_DIR}/source/cmake/Util.cmake)

file(GLOB SOURCEFILES "src/*.cpp")
file(GLOB HEADERS "include/*.h*")
file(GLOB LOGGER_HEADERS "${LoggerCpp_HEADERS}/LoggerCpp/*.h*")
set( SOURCES ${SOURCEFILES} ${HEADERS} "${LOGGER_HEADERS}")

set ( LIBTYPE "${Util_LIBTYPE}" )
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lstdc++fs")

include_directories( ${Util_HEADERS} ${LoggerCpp_HEADERS} )

add_library( ${Util} ${LIBTYPE} ${SOURCES} )
target_link_libraries( ${Util}
                       ${CMAKE_THREAD_LIBS_INIT} ${LINKOPTIONS}
                     )

install(TARGETS ${Util} DESTINATION lib)
install(TARGETS ${Util} DESTINATION localrun)
install(FILES ${HEADERS} DESTINATION include/Util)
install(FILES ${LOGGER_HEADERS} DESTINATION include/LoggerCpp)

#
# Circular buffer main
#
set (main_circular_buffer "main_circular_buffer${DBG}")
add_executable (main_circular_buffer src/main_programs/main_circular_buffer.cpp)
target_link_libraries( main_circular_buffer 
                            ${Util_LIB}
                            ${LoggerCpp_LIB}
                            ${CMAKE_THREAD_LIBS_INIT} 
                            ${LINKOPTIONS}
                     )
install(TARGETS main_circular_buffer DESTINATION localrun)

#
# main_LoggerCpp_main_example - LoggerCpp main from Main.cpp example
#
set (main_LoggerCpp_main_example "main_LoggerCpp_main_example${DBG}")
add_executable (main_LoggerCpp_main_example src/main_programs/main_LoggerCpp_main_example.cpp)
target_link_libraries( main_LoggerCpp_main_example 
                            ${Util_LIB}
                            ${LoggerCpp_LIB}
                            ${CMAKE_THREAD_LIBS_INIT} 
                            ${LINKOPTIONS}
                     )
install(TARGETS main_LoggerCpp_main_example DESTINATION localrun)

#
# Command line main
#
set (main_commandline "main_commandline${DBG}")
add_executable (main_commandline src/main_programs/main_commandline.cpp)
target_link_libraries( main_commandline 
                            ${Util_LIB}
                            ${LoggerCpp_LIB}
                            ${CMAKE_THREAD_LIBS_INIT} 
                            ${LINKOPTIONS}
                     )
install(TARGETS main_commandline DESTINATION localrun)

#
# Condition_data main
#
set (main_condition_data "main_condition_data${DBG}")
add_executable (main_condition_data src/main_programs/main_condition_data.cpp)
target_link_libraries( main_condition_data 
                            ${Util_LIB}
                            ${LoggerCpp_LIB}
                            ${CMAKE_THREAD_LIBS_INIT} 
                            ${LINKOPTIONS}
                     )
install(TARGETS main_condition_data DESTINATION localrun)

# Dependencies
add_dependencies (main_circular_buffer ${Util})
add_dependencies (main_LoggerCpp_main_example ${Util})
add_dependencies (main_commandline ${Util})
add_dependencies (main_condition_data ${Util})
