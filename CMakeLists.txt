cmake_minimum_required(VERSION 3.20)
# 项目信息
project(openALLearn)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_SOURCE_DIR ./src)
set(${PROJECT_NAME}_VERSION_MAJOR 1)
set(${PROJECT_NAME}_VERSION_MINOR 0)

include_directories(/Users/panfeng/coder/myProject/LUINeedLib/include)
link_directories(/Users/panfeng/coder/myProject/LUINeedLib)

add_executable(LUI
        main.cpp
)

target_link_libraries(LUI
        openal.1
)