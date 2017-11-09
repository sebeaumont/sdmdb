# build c shared library
include_directories(..)

# header only boost
find_package(Boost REQUIRED) 
include_directories (${Boost_INCLUDE_DIR})

# header only cereal
find_package(Cereal REQUIRED)
include_directories (${Cereal_INCLUDE_DIRS})

# c++ and c libraries
add_library(sdm SHARED sdmlib.cpp database.cpp)

set_target_properties(sdm PROPERTIES
  VERSION ${SDM_VERSION_MAJOR}.${SDM_VERSION_MINOR}
  SOVERSION ${SDM_VERSION_MAJOR})


# install lib and headers
install (TARGETS sdm LIBRARY DESTINATION lib)
install (FILES sdmlib.h sdmconfig.h sdmstatus.h DESTINATION include)
