add_executable(MMWave_Data_Drawer src/main.cpp
        src/MMWave/Prompting/Terminal.hpp
        src/MMWave/Prompting/General.hpp
        src/MMWave/Porter.hpp
        src/MMWave/Streaming.hpp
        src/MMWave/Tlv.hpp
)
target_link_libraries(MMWave_Data_Drawer Boost::boost)