cmake_minimum_required (VERSION 3.10)

project( Sample )

add_subdirectory(Util)
add_subdirectory(EnetUtil)
add_subdirectory(Video)

# Basic dependencies
# add_dependencies (${Projectname1} ${dependsOnProject2} ${dependsOnProjectN})

add_dependencies ( EnetUtil Util )
add_dependencies ( Video EnetUtil Util )

