cmake_minimum_required (VERSION 3.10)

project (QtSample)

include (${SampleRoot_DIR}/source/cmake/Qt5Sample.cmake)

#####   file(GLOB TEST_SCRIPTS "src/main_programs/*.bash")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)

set(PROJECT_SOURCES
        src/main_programs/text_finder_main.cpp
        src/main_programs/main_text_finder_window.cpp
        src/main_programs/main_text_finder_window.h
        src/main_programs/main_text_finder_window.ui
)

#####             file(GLOB SOURCEFILES "src/*.c" "src/*.cpp")
#####             file(GLOB HEADERS "include/*.h*")

file(GLOB LOGGER_HEADERS "${LoggerCpp_HEADERS}/LoggerCpp/*.h*")

#####             set( SOURCES ${SOURCEFILES} ${HEADERS} "${LOGGER_HEADERS}" )

set( SOURCES ${PROJECT_SOURCES} "${LOGGER_HEADERS}" )

##### set ( LIBTYPE "${QtSample_LIBTYPE}" )
##### set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")

include_directories( ${QtSample_HEADERS} 
                     ../Util/include
                     ../EnetUtil/include
                     ../QtSample/include
                     ${LoggerCpp_HEADERS} )

####  add_library( ${QtSample} ${LIBTYPE} ${SOURCES} )
####  target_link_libraries( ${QtSample}
####                         ${CMAKE_THREAD_LIBS_INIT} ${LINKOPTIONS}
####                       )

####                  install(TARGETS ${QtSample} DESTINATION lib)
####                  install(TARGETS ${QtSample} DESTINATION localrun)
install(FILES ${HEADERS} DESTINATION include/QtSample)
install(FILES ${LOGGER_HEADERS} DESTINATION include/LoggerCpp)

install(FILES src/main_programs/input.txt
            PERMISSIONS OWNER_EXECUTE OWNER_READ
                    GROUP_EXECUTE GROUP_READ
                    WORLD_EXECUTE WORLD_READ
            DESTINATION localrun)

##### install(FILES ${TEST_SCRIPTS}
#####        PERMISSIONS OWNER_EXECUTE OWNER_READ
#####                    GROUP_EXECUTE GROUP_READ
#####                    WORLD_EXECUTE WORLD_READ
#####        DESTINATION localrun)

set (Util_LIB "${SampleRoot_DIR}/build/Util/libUtil.so")
set (EnetUtil_LIB "${SampleRoot_DIR}/build/EnetUtil/libEnetUtil.so")
set (Video_LIB "${SampleRoot_DIR}/build/Video/libVideo.so")
#
# text_finder_main
#
set (text_finder_main "text_finder_main${DBG}")


# if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
#     qt_add_executable(text_finder_main ${PROJECT_SOURCES}
#     )
# else()
    add_executable(text_finder_main ${PROJECT_SOURCES})
# endif()


###                add_executable (text_finder_main src/main_programs/text_finder_main.cpp)
target_link_libraries( text_finder_main 
                            #########         ${QtSample_LIB}
                            ${Video_LIB}
                            ${EnetUtil_LIB}
                            ${Util_LIB}
                            ${LoggerCpp_LIB}
                            ${CMAKE_THREAD_LIBS_INIT} 
                            Qt${QT_VERSION_MAJOR}::Widgets
                            ${LINKOPTIONS}
                     )
install(TARGETS text_finder_main DESTINATION localrun)


# Dependencies
add_dependencies (text_finder_main ${EnetUtil} ${Util} Qt${QT_VERSION_MAJOR}::Widgets)

