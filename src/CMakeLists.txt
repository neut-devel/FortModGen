add_library(FortModGen STATIC 
  FortranModuleGenerator.cc 
  types.cc)

target_link_libraries(FortModGen toml11)

target_include_directories(FortModGen PUBLIC 
  ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(interfaces)