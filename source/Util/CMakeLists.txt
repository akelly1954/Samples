cmake_minimum_required (VERSION 3.10)

project (Util)

include (${SampleRoot_DIR}/source/cmake/Util.cmake)

file(GLOB SOURCEFILES "src/*.cpp")
file(GLOB HEADERS "include/*.h*")
file(GLOB LOGGER_HEADERS "${LoggerCpp_HEADERS}/LoggerCpp/*.h*")
file(GLOB JSONCPP_HEADERS "${JsonCpp_HEADERS}/json/*.h*")
set( SOURCES ${SOURCEFILES} ${HEADERS} "${LOGGER_HEADERS}" "${JSONCPP_HEADERS}")

file(GLOB JSON_FILES "src/main_programs/*.json")
file(GLOB SCRIPTS_FILES "src/scripts/*.bash")

set ( LIBTYPE "${Util_LIBTYPE}" )
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lstdc++fs")

include_directories( ${Util_HEADERS} ${LoggerCpp_HEADERS} ${JsonCpp_HEADERS} )

add_library( ${Util} ${LIBTYPE} ${SOURCES} )
target_link_libraries( ${Util}
                       ${CMAKE_THREAD_LIBS_INIT} ${LINKOPTIONS}
                     )

install(TARGETS ${Util} DESTINATION lib)
install(TARGETS ${Util} DESTINATION localrun)
install(FILES ${HEADERS} DESTINATION include/Util)
install(FILES ${LOGGER_HEADERS} DESTINATION include/LoggerCpp)
install(FILES ${JSONCPP_HEADERS} DESTINATION include/JsonCpp/json)

install(FILES ${JSON_FILES} DESTINATION Util)
install(FILES ${JSON_FILES} DESTINATION localrun)

install(FILES ${SCRIPTS_FILES} 
        PERMISSIONS OWNER_EXECUTE OWNER_READ
                    GROUP_EXECUTE GROUP_READ
                    WORLD_EXECUTE WORLD_READ
        DESTINATION Util)

install(FILES ${SCRIPTS_FILES} DESTINATION 
        PERMISSIONS OWNER_EXECUTE OWNER_READ
                    GROUP_EXECUTE GROUP_READ
                    WORLD_EXECUTE WORLD_READ
        DESTINATION localrun)

#
# Json sample config main
#
set (main_jsoncpp_samplecfg "main_jsoncpp_samplecfg${DBG}")
add_executable (main_jsoncpp_samplecfg src/main_programs/main_jsoncpp_samplecfg.cpp)
target_link_libraries( main_jsoncpp_samplecfg 
                            ${Util_LIB}
                            ${LoggerCpp_LIB}
                            ${JsonCpp_LIB}
                            ${CMAKE_THREAD_LIBS_INIT} 
                            ${LINKOPTIONS}
                     )
install(TARGETS main_jsoncpp_samplecfg DESTINATION localrun)

#
# sample UtilLogger main
#
set (main_UtilLogger_example "main_UtilLogger_example${DBG}")
add_executable (main_UtilLogger_example src/main_programs/main_UtilLogger_example.cpp)
target_link_libraries( main_UtilLogger_example 
                            ${Util_LIB}
                            ${LoggerCpp_LIB}
                            ${JsonCpp_LIB}
                            ${CMAKE_THREAD_LIBS_INIT} 
                            ${LINKOPTIONS}
                     )
install(TARGETS main_UtilLogger_example DESTINATION localrun)

#
# Circular buffer main
#
set (main_circular_buffer "main_circular_buffer${DBG}")
add_executable (main_circular_buffer src/main_programs/main_circular_buffer.cpp)
target_link_libraries( main_circular_buffer 
                            ${Util_LIB}
                            ${LoggerCpp_LIB}
                            ${JsonCpp_LIB}
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
                            ${JsonCpp_LIB}
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
                            ${JsonCpp_LIB}
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
                            ${JsonCpp_LIB}
                            ${CMAKE_THREAD_LIBS_INIT} 
                            ${LINKOPTIONS}
                     )
install(TARGETS main_condition_data DESTINATION localrun)

#
# main_util_combo_objects
#
set (main_util_combo_objects "main_util_combo_objects${DBG}")
add_executable (main_util_combo_objects src/main_programs/main_util_combo_objects.cpp)
target_link_libraries( main_util_combo_objects 
                            ${Util_LIB}
                            ${LoggerCpp_LIB}
                            ${JsonCpp_LIB}
                            ${CMAKE_THREAD_LIBS_INIT} 
                            ${LINKOPTIONS}
                     )
install(TARGETS main_util_combo_objects DESTINATION localrun)

# Dependencies
add_dependencies (main_circular_buffer ${Util})
add_dependencies (main_LoggerCpp_main_example ${Util})
add_dependencies (main_UtilLogger_example ${Util})
add_dependencies (main_jsoncpp_samplecfg ${Util})
add_dependencies (main_commandline ${Util})
add_dependencies (main_condition_data ${Util})
add_dependencies (main_util_combo_objects ${Util})
