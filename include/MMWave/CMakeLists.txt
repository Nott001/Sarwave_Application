# Include directory for MMWave headers
add_library(MMWave INTERFACE
        ThreadedStreaming.hpp
        Configured/PeopleTracking/Manager.hpp
        Configured/PeopleTracking/EntryDecompiler.hpp
        Configured/PeopleTracking/TargetObject.hpp
        Configured/PeopleTracking/PointData.hpp
        Configured/PeopleTracking/TlvOutput.hpp
        Configured/OutOfBox/TlvTypes.hpp
        Configured/OutOfBox/TlvOutput.hpp)
target_include_directories(MMWave INTERFACE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(MMWave INTERFACE Boost::boost)
