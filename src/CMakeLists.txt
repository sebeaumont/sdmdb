################################################################
# c++ shared library and utility tools build for sdmdb library
# copyright simon beaumont 2012-2017 - see: LICENSE for details
################################################################

project(sdmlib)
cmake_minimum_required (VERSION 3.0)
enable_testing()

##############################
# versioning and cofiguration
##############################

set (HAVE_OPENMP 0)
set (HAVE_DISPATCH 0)

# versioning

file(READ ../VERSION SDM_VERSION)

string(REGEX MATCHALL "[0-9]+" VERSION_LIST ${SDM_VERSION})
list(GET VERSION_LIST 0 SDM_VERSION_MAJOR)
list(GET VERSION_LIST 1 SDM_VERSION_MINOR)

message(STATUS "Building library version: ${SDM_VERSION_MAJOR}.${SDM_VERSION_MINOR} for SDMDB release: ${SDM_VERSION}")


########################
# local modules we need
########################

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/../cmake/Modules/")

#######################
# library install paths
#######################

set (CMAKE_INSTALL_PREFIX "/usr/local/sdm")

set (CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set (CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

#########################################
# project global c/c++ compiler settings
#########################################

# common to both c & c++ flags
set(SDM_ALL_FLAGS "-Wall -march=native")

set(SDM_DEBUG "-g")
set(SDM_OPTIMIZE "-O3")

# c++ only
set(SDM_CXX_FLAGS "-std=c++14")

# c only
# for max compatability c lib
set(SDM_C_FLAGS "-std=c90")


# is future namespace wizardry allowed?
# YES: set(SDM_CXX_FLAGS "-Wc++1z-extensions")


############################################
# compiler specific project wide defintions
############################################

# this gets gcc to pass -q to the assembler which will use the clang
# assembler which groks the avx instructions - yeah don't ask there's
# history here...
#IF ((${CMAKE_SYSTEM_NAME} MATCHES "Darwin") AND (${CMAKE_CXX_COMPILER_ID} MATCHES "GNU"))
#  set(SDM_CXX_FLAGS "-Wa,-q") 
#ENDIF()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  # INSTRUMENTATION: set(SDM_CXX_FLAGS "-Rpass=.*") # have clang show optimizations
  # OLD LINK BUG: set(SDM_CXX_FLAGS "${SDM_CXX_FLAGS} -fsanitize=undefined,address")
  # XXX review these clang optimaztions e.g. we should be able the exact loop sizes   
  set(SDM_ALL_FLAGS "${SDM_ALL_FLAGS} -funroll-loops -fvectorize -fslp-vectorize")
endif()


######################################################################
# we use think different threading backends on Darwin derived systems
# we also probe what cpu features we can exploit...
######################################################################

set(CPU_FEATURES)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  # OS X/XNU specific configuration - should work for iOS?
  message(STATUS "Will compile for Blocks/GCD and other Apple sweetness...")
  set(HAVE_DISPATCH 1)
  set(SDM_CXX_FLAGS "${SDM_CXX_FLAGS} -stdlib=libc++")
  set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} "-stdlib=libc++")
  exec_program("sysctl -n machdep.cpu.features" OUTPUT_VARIABLE CPU_FEATURES)

else()

  # Use openmp on non OSX Systems (linux)
  
  find_package(OpenMP REQUIRED)
  if (OPENMP_FOUND)

    message(STATUS "OpenMP is configured")
    set(HAVE_OPENMP 1)
    set(SDM_ALL_FLAGS "${SDM_ALL_FLAGS} -fopenmp")
    set(SDM_C_FLAGS "${SDM_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(SDM_CXX_FLAGS "${SDM_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS})
    
    find_package(Threads REQUIRED)

    if (CMAKE_USE_PTHREADS_INIT)
      set(SDM_CXX_FLAGS "${SDM_CXX_FLAGS} -pthread")
    endif()

    set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} ${CMAKE_THREAD_LIBS_INIT})
    
    file(READ "/proc/cpuinfo" _cpuinfo)
    string(REGEX REPLACE ".*flags[ \t]*:[ \t]+([a-zA-Z0-9_ ]+).*" "\\1" CPU_FEATURES "${_cpuinfo}")
  endif()

endif()

# normalise cpu features before use..
string(TOUPPER ${CPU_FEATURES} CPU_FEATURES)


# allow for non-release/debug builds

if (${CMAKE_BUILD_TYPE} MATCHES "Release") 
   set(CMAKE_CXX_FLAGS "${SDM_OPTIMIZE} ${SDM_CXX_FLAGS} ${SDM_ALL_FLAGS}")
   set(CMAKE_C_FLAGS "${SDM_OPTIMIZE} ${SDM_C_FLAGS} ${SDM_ALL_FLAGS}")
else()
   set(CMAKE_CXX_FLAGS "${SDM_DEBUG} ${SDM_CXX_FLAGS} ${SDM_ALL_FLAGS}")
   set(CMAKE_C_FLAGS "${SDM_DEBUG} ${SDM_C_FLAGS} ${SDM_ALL_FLAGS}")
endif()

message(${CMAKE_CXX_FLAGS})
message(${CMAKE_C_FLAGS})

# generate build time header configuration
configure_file(sdmconfig.h.in sdmconfig.h)

##################
# runtime library
add_subdirectory(rtl)


########
# tests
add_subdirectory(test)





