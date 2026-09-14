add_executable(MMWave_Data_Drawer main.cpp
        ../include/MMWaveDataReader/Prompting/Terminal.hpp
        ../include/MMWaveDataReader/Prompting/General.hpp
        ../include/MMWaveDataReader/Porter.hpp
        ../include/MMWaveDataReader/Streaming.hpp
        ../include/MMWaveDataReader/Tlv.hpp
)
target_include_directories(MMWave_Data_Drawer PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(MMWave_Data_Drawer Boost::boost)
