SET(SRC_FILES
        Catalog.cc
        Monitor.cc
)

add_library(srces ${SRC_FILES})

target_link_libraries(srces bpftools)
install(TARGETS srces DESTINATION lib)