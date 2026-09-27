# Include directory for MMWave headers
add_library(MMWave STATIC Configured/PeopleTracking/Debug.cpp)
target_include_directories(MMWave PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(MMWave PUBLIC Boost::boost)
