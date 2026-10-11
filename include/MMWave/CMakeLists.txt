# Sarwave.Core.MMWave: everything that talks to, decodes or records the radar. Pure C++23 plus
# Boost.Asio for the serial ports -- no Qt, so the terminal front end stays dependency free.
#
# The .cfg model (Config/ and Configured/PeopleTracking/Config/) is its own target so it can be
# built and tested on its own; nothing in the capture path needs it.

set(MMWAVE_CONFIG_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/Config
    ${CMAKE_CURRENT_SOURCE_DIR}/Configured/PeopleTracking/Config
)

file(GLOB_RECURSE MMWAVE_ALL_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp)

set(MMWAVE_CORE_SOURCES)
set(MMWAVE_CONFIG_SOURCES)
foreach(source IN LISTS MMWAVE_ALL_SOURCES)
    set(is_config FALSE)
    foreach(dir IN LISTS MMWAVE_CONFIG_DIRS)
        cmake_path(IS_PREFIX dir "${source}" NORMALIZE prefixed)
        if(prefixed)
            set(is_config TRUE)
        endif()
    endforeach()
    if(is_config)
        list(APPEND MMWAVE_CONFIG_SOURCES ${source})
    else()
        list(APPEND MMWAVE_CORE_SOURCES ${source})
    endif()
endforeach()

add_library(SarwaveCoreMMWave STATIC ${MMWAVE_CORE_SOURCES})
target_include_directories(SarwaveCoreMMWave PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(SarwaveCoreMMWave PUBLIC Boost::boost Threads::Threads)
set_target_properties(SarwaveCoreMMWave PROPERTIES AUTOMOC OFF AUTOUIC OFF AUTORCC OFF)
if(WIN32)
    target_compile_definitions(SarwaveCoreMMWave PUBLIC _WIN32_WINNT=0x0A00)
endif()

if(MMWAVE_CONFIG_SOURCES MATCHES "\\.cpp")
    add_library(SarwaveCoreMMWaveConfig STATIC ${MMWAVE_CONFIG_SOURCES})
    target_include_directories(SarwaveCoreMMWaveConfig PUBLIC ${CMAKE_SOURCE_DIR}/include)
    target_link_libraries(SarwaveCoreMMWaveConfig PUBLIC SarwaveCoreMMWave)
    set_target_properties(SarwaveCoreMMWaveConfig PROPERTIES AUTOMOC OFF AUTOUIC OFF AUTORCC OFF)
else()
    # Nothing ported yet: keep the target so dependents still configure.
    add_library(SarwaveCoreMMWaveConfig INTERFACE)
    target_link_libraries(SarwaveCoreMMWaveConfig INTERFACE SarwaveCoreMMWave)
endif()
