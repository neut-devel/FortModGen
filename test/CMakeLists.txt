include(FortModGen)

FortModGen(MOD_DESCRIPTOR_FILE ${CMAKE_CURRENT_SOURCE_DIR}/testmod.toml
           MOD_OUTPUT_STUB testmod)

add_library(testmod STATIC cppwrite.cc cwrite.c fwrite.f90 testmod.f90)
target_include_directories(testmod PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

add_executable(ftest ftest.f90)
target_link_libraries(ftest testmod)

add_executable(cpptest cpptest.cc)
target_link_libraries(cpptest testmod)