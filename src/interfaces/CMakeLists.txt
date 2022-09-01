add_library(FortModGenInterfaces STATIC
  CInterfaceGenerator.cc)

target_link_libraries(FortModGenInterfaces FortModGen)

target_include_directories(FortModGenInterfaces PUBLIC 
  ${CMAKE_CURRENT_SOURCE_DIR})