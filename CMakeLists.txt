cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

project(encoder_gen C CXX ASM)

add_executable(encoder_gen
 encoder_gen.cpp
)

# pull in common dependencies
target_link_libraries(encoder_gen PRIVATE
        cpprest
        wiringPi
        pthread
        crypto
        )



add_executable(looper
        looper.cpp
)

target_link_libraries(looper 
        gpiod
        pthread
)
       

add_executable(spi
        spi.cpp
)
