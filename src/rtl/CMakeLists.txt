#########################
# build shared libraries
#########################

include_directories(..)

# this for generated: config.h
include_directories (${CMAKE_BINARY_DIR})

# header only boost
find_package(Boost REQUIRED) 
include_directories (${Boost_INCLUDE_DIR})

# header only cereal
#find_package(Cereal REQUIRED)
#include_directories (${Cereal_INCLUDE_DIRS})

# eigen
find_package(Eigen3 REQUIRED)
include_directories(${EIGEN3_INCLUDE_DIR})


# c library: cshim + database
add_library(sdm SHARED sdmlib.cpp manifold.cpp database.cpp)

set_target_properties(sdm PROPERTIES
  VERSION ${SDM_VERSION_MAJOR}.${SDM_VERSION_MINOR}
  SOVERSION ${SDM_VERSION_MAJOR})

# C++ library -- database and locally euclidean topologies
add_library(sdmdb SHARED manifold.cpp database.cpp)

set_target_properties(sdmdb PROPERTIES
  VERSION ${SDM_VERSION_MAJOR}.${SDM_VERSION_MINOR}
  SOVERSION ${SDM_VERSION_MAJOR})


# install lib and headers
install (TARGETS sdm LIBRARY DESTINATION lib)
install (TARGETS sdmdb LIBRARY DESTINATION lib)
# review this 
install (FILES sdmlib.h sdmconfig.h sdmstatus.h DESTINATION include)
