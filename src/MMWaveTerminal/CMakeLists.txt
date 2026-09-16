add_executable(MMWave_Data_Drawer main.cpp
        ${CMAKE_SOURCE_DIR}/include/MMWave/Prompting/Terminal.hpp
        ${CMAKE_SOURCE_DIR}/include/MMWave/Prompting/General.hpp
        ${CMAKE_SOURCE_DIR}/include/MMWave/Porter.hpp
        ${CMAKE_SOURCE_DIR}/include/MMWave/Streaming.hpp
        ${CMAKE_SOURCE_DIR}/include/MMWave/Tlv.hpp
)
target_include_directories(MMWave_Data_Drawer PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(MMWave_Data_Drawer Boost::boost)
