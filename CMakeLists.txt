cmake_minimum_required(VERSION 3.16)
project(DeliveryTrackingSystem LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compiler warnings
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

include_directories(include)

add_executable(delivery_tracker
    src/main.cpp
    src/Package.cpp
    src/DeliveryTracker.cpp
    src/UI.cpp
)
