cmake_minimum_required(VERSION 3.9)
project(hypertool)

set(CMAKE_CXX_STANDARD 17)

#GDAL
find_package(GDAL REQUIRED)
include_directories(${GDAL_INCLUDE_DIR})

#Boost
find_package(Boost REQUIRED COMPONENTS filesystem)

add_executable(transpose_hyperimage transpose_hyperimage.cpp )
target_link_libraries(transpose_hyperimage ${Boost_LIBRARIES} ${GDAL_LIBRARY})

add_executable(remove_dark_reference remove_dark_reference.cpp CommonFunc.h)
target_link_libraries(remove_dark_reference ${GDAL_LIBRARY})