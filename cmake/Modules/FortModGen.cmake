function(FortModGen)

  set(oneValueArgs MOD_DESCRIPTOR_FILE MOD_OUTPUT_STUB)
  cmake_parse_arguments(OPTS 
                      "${options}" 
                      "${oneValueArgs}"
                      "${multiValueArgs}" ${ARGN})

  if(NOT DEFINED OPTS_MOD_DESCRIPTOR_FILE)
    message(FATAL_ERROR "FortModGen requires MOD_DESCRIPTOR_FILE argument to be passed.")
  endif()
  if(NOT DEFINED OPTS_MOD_OUTPUT_STUB)
    message(FATAL_ERROR "FortModGen requires MOD_OUTPUT_STUB argument to be passed.")
  endif()

  add_custom_command(
    OUTPUT ${OPTS_MOD_OUTPUT_STUB}.f90 ${OPTS_MOD_OUTPUT_STUB}.h
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND $<TARGET_FILE:fortmodgen>
    ARGS -i ${OPTS_MOD_DESCRIPTOR_FILE} -o ${OPTS_MOD_OUTPUT_STUB}
    DEPENDS fortmodgen ${OPTS_MOD_DESCRIPTOR_FILE})

endfunction(FortModGen)

function(FortModName)
  set(oneValueArgs MOD_DESCRIPTOR_FILE OUTPUT_VARIABLE)
  cmake_parse_arguments(OPTS 
                      "${options}" 
                      "${oneValueArgs}"
                      "${multiValueArgs}" ${ARGN})

  if(NOT DEFINED OPTS_MOD_DESCRIPTOR_FILE)
    message(FATAL_ERROR "FortModName requires MOD_DESCRIPTOR_FILE argument to be passed.")
  endif()
  if(NOT DEFINED OPTS_OUTPUT_VARIABLE)
    message(FATAL_ERROR "FortModName requires OUTPUT_VARIABLE argument to be passed.")
  endif()

  execute_process(COMMAND 
    bash -c "( ( test -e ./module_name ) || (g++ -I ${toml11_SOURCE_DIR}/include ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/module_name.cc -o module_name ) ) && ./module_name ${OPTS_MOD_DESCRIPTOR_FILE}"
    OUTPUT_VARIABLE OV
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} 
    OUTPUT_STRIP_TRAILING_WHITESPACE)

  set(${OPTS_OUTPUT_VARIABLE} ${OV} PARENT_SCOPE)
endfunction(FortModName)