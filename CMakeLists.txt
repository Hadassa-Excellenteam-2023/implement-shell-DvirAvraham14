cmake_minimum_required(VERSION 3.25)
project(implement-shell-DvirAvraham14)

set(CMAKE_CXX_STANDARD 17)

add_executable(${PROJECT_NAME} main.cpp src/main.cpp include/CommandExecutor.h include/BackgroundProcessManager.h include/CommandHistoryManager.h include/Shell.h src/BackgroundProcessManager.cpp src/CommandExecutor.cpp src/CommandHistoryManager.cpp src/Shell.cpp)

add_subdirectory(include)
add_subdirectory(src)