cmake_minimum_required (VERSION 3.10)
project (sample)

add_subdirectory(Util)
add_subdirectory(EnetUtil)

# Basic dependencies
# add_dependencies (${Projectname1} ${dependsOnProject2} ${dependsOnProjectN})

add_dependencies ( EnetUtil Util )


