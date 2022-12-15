SET(SRC_FILES
        Catalog.cc
        Monitor.cc
        Server.cc
)

add_library(backend ${SRC_FILES})

target_link_libraries(backend bpftools)
install(TARGETS backend DESTINATION lib)