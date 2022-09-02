function(FortModGen)

  set(oneValueArgs MOD_DESCRIPTOR_FILE MOD_OUTPUT_STUB)
  cmake_parse_arguments(OPTS 
                      "${options}" 
                      "${oneValueArgs}"
                      "${multiValueArgs}" ${ARGN})

  if(NOT DEFINED OPTS_MOD_DESCRIPTOR_FILE)
    message(FATAL_ERROR "FORTMODGEN requires MOD_DESCRIPTOR_FILE argument to be passed.")
  endif()
  if(NOT DEFINED OPTS_MOD_OUTPUT_STUB)
    message(FATAL_ERROR "FORTMODGEN requires MOD_OUTPUT_STUB argument to be passed.")
  endif()

  add_custom_command(
    OUTPUT ${OPTS_MOD_OUTPUT_STUB}.f90 ${OPTS_MOD_OUTPUT_STUB}.h
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND $<TARGET_FILE:fortmodgen>
    ARGS -i ${OPTS_MOD_DESCRIPTOR_FILE} -o ${OPTS_MOD_OUTPUT_STUB}
    DEPENDS fortmodgen ${OPTS_MOD_DESCRIPTOR_FILE})

endfunction(FortModGen)