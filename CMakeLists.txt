cmake_minimum_required(VERSION 3.23.1)

project(Kyub)

add_executable(${PROJECT_NAME} kyub.cpp)

add_subdirectory(src)

find_package(glm REQUIRED)

target_link_libraries(${PROJECT_NAME} agm)
target_link_libraries(${PROJECT_NAME} Camera)
target_link_libraries(${PROJECT_NAME} Lights)
target_link_libraries(${PROJECT_NAME} Mesh)
target_link_libraries(${PROJECT_NAME} Screen)
target_link_libraries(${PROJECT_NAME} glm::glm)