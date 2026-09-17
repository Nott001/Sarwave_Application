# Include directory for MMWave headers
add_library(MMWave INTERFACE
        ThreadedStreaming.hpp)
target_include_directories(MMWave INTERFACE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(MMWave INTERFACE Boost::boost)
