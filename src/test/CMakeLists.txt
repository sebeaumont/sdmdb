# test programs for rtl
include_directories(..)

find_package(Boost REQUIRED)
find_package(Boost REQUIRED COMPONENTS program_options)

include_directories (${Boost_INCLUDE_DIR})

message("${CPU_FEATURES}")

add_executable (rtl_load_space rtl_load_space.cpp)
target_link_libraries(rtl_load_space sdm)
target_link_libraries(rtl_load_space ${CMAKE_EXE_LINKER_FLAGS})


# low level space test 
add_executable (mms_0 mms_0.cpp)
target_link_libraries(mms_0 ${CMAKE_EXE_LINKER_FLAGS})

# test programs
add_test(NAME mms_0 COMMAND mms_0 --log_level=all)

add_test(NAME rtl_load_space COMMAND rtl_load_space --log_level=all)


# test utility 

add_executable (sdmi sdmi.cpp)
target_link_libraries(sdmi ${Boost_LIBRARIES} sdm)

add_executable (frametrain frametrain.cpp)
target_link_libraries(frametrain ${Boost_LIBRARIES} sdm)

add_executable (capi capi.c)
target_link_libraries(capi sdm)

