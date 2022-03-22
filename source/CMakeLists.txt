cmake_minimum_required (VERSION 3.10)

project( Sample )

add_subdirectory(Util)
add_subdirectory(EnetUtil)
add_subdirectory(Video)
add_subdirectory(QtSample)

# Basic dependencies
# add_dependencies (${Projectname1} ${dependsOnProject2} ${dependsOnProjectN})

add_dependencies ( EnetUtil Util )
add_dependencies ( Video EnetUtil Util )
add_dependencies ( text_finder_main Video EnetUtil Util )


