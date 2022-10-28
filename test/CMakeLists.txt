include(FortModGen)

FortModGen(MOD_DESCRIPTOR_FILE ${CMAKE_CURRENT_SOURCE_DIR}/testmod.toml
           MOD_OUTPUT_STUB testmod)

add_library(testmod STATIC cppwrite.cc fwrite.f90 testmod.f90)
target_include_directories(testmod PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

add_executable(ftest ftest.f90)
target_link_libraries(ftest testmod)

add_executable(cpptest cpptest.cc)
target_link_libraries(cpptest testmod fmt::fmt)

add_executable(full_precision_parameter_test full_precision_parameter_test.cc)
target_link_libraries(full_precision_parameter_test testmod fmt::fmt)

add_test(NAME ftest COMMAND ftest)
add_test(NAME cpptest COMMAND cpptest)
add_test(NAME full_precision_parameter_test COMMAND full_precision_parameter_test)