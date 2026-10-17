# Sarwave.Core.Imu: the ICM-42688-P IMU on Linux SPI -- driver, sampler, tilt fusion, motion
# gating and .swimu recording. Pure C++23 (Linux spidev; elsewhere the IMU simply counts as not
# detected). Shares only the header-only numeric and time types of include/MMWave/Common.

file(GLOB_RECURSE IMU_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp)

add_library(SarwaveCoreImu STATIC ${IMU_SOURCES})
target_include_directories(SarwaveCoreImu PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(SarwaveCoreImu PUBLIC Threads::Threads)
set_target_properties(SarwaveCoreImu PROPERTIES AUTOMOC OFF AUTOUIC OFF AUTORCC OFF)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(SarwaveCoreImu PRIVATE -Wall -Wextra)
endif()
