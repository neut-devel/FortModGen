add_executable(fortmodgen fortmodgen.cc)
target_link_libraries(fortmodgen FortModGenInterfaces)

install(TARGETS fortmodgen 
    EXPORT FortModGen-targets
    DESTINATION bin/)
