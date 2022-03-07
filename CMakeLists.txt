cmake_minimum_required(VERSION 3.11)

project(Mall C CXX)

set(CMAKE_CXX_FLAGS "-std=c++17 -Wall")
set(CMAKE_CXX_FLAGS_DEBUG "-std=c++17 -g3 -O0 -pg")
set(CMAKE_CXX_FLAGS_RELEASE "-std=c++17 -O2 -s -DNDEBUG -march=native")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-std=c++17 -g3 -Og -pg")
set(CMAKE_CXX_FLAGS_MINSIZEREL "-std=c++17 -Os -s -DNDEBUG -march=native")

find_package(Bullet REQUIRED)

include_directories(
   vulkan
   GLFW
   glm
   stb
   cutlass
   "/usr/local/include/assimp" #AUR's assimp is broken
   ${BULLET_INCLUDE_DIR}
   mvecs
   "./include"
   portaudio
)

file(GLOB_RECURSE SRCS src/*.c*)

add_executable(App ${SRCS})

target_link_libraries(App
   vulkan
   glfw
   cutlass
   "/usr/local/lib/libassimp.so"
   ${BULLET_LIBRARIES}
   mvecs
   portaudio
)
