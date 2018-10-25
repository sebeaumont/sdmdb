# programs for test and utilities

include_directories(..)

include_directories (${CMAKE_BINARY_DIR})

# eigen
find_package(Eigen3 REQUIRED)
include_directories(${EIGEN3_INCLUDE_DIR})


find_package(Boost REQUIRED)
find_package(Boost REQUIRED COMPONENTS program_options)

include_directories (${Boost_INCLUDE_DIR})

message("${CPU_FEATURES}")

add_executable (rtl_load_space rtl_load_space.cpp)
target_link_libraries(rtl_load_space sdmdb)
target_link_libraries(rtl_load_space ${CMAKE_EXE_LINKER_FLAGS})


# low level space test 
add_executable (mms_0 mms_0.cpp)
target_link_libraries(mms_0 ${CMAKE_EXE_LINKER_FLAGS})

# manifold api
add_executable (rtl_manifold rtl_manifold.cpp)
target_link_libraries(rtl_manifold sdmdb)

# rtl api
add_executable (rtl_api rtl_api.cpp)
target_link_libraries(rtl_api sdmdb)


# test programs

add_test(NAME mms_0 COMMAND mms_0 --log_level=all)
add_test(NAME rtl_api COMMAND rtl_api --log_level=all)
add_test(NAME rtl_manifold COMMAND rtl_manifold --log_level=all)
add_test(NAME rtl_load_space COMMAND rtl_load_space --log_level=all)



# utilities 

add_executable (sdmi sdmi.cpp)
target_link_libraries(sdmi ${Boost_LIBRARIES})
target_link_libraries(sdmi sdmdb)

add_executable (frametrain frametrain.cpp)
target_link_libraries(frametrain ${Boost_LIBRARIES})
target_link_libraries(frametrain sdmdb)

add_executable (affinity affinity.cpp)
target_link_libraries(affinity ${Boost_LIBRARIES})
target_link_libraries(affinity sdmdb)


