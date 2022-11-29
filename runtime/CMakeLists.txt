set(RUNTIME_SRCS
        RuntimeLogger.cc
        Cycles.cc
        Log.cc
        LogDecompressor.cc
        NanoLog.cc
        PerfHelper.cc
        TimeTrace.cc
        Util.cc
        )
# ok
add_library(Nanolog ${RUNTIME_SRCS})
add_executable(decompressor ${RUNTIME_SRCS})

target_link_libraries(Nanolog pthread rt gtest)
target_link_libraries(decompressor pthread rt gtest)

install(TARGETS Nanolog DESTINATION lib)

set_target_properties(Nanolog PROPERTIES OUTPUT_NAME "Nanolog")
