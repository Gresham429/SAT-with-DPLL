set(CMAKE_BUILD_TYPE Release)

cmake_minimum_required(VERSION 3.10)

project(HanidokuSolver)

# 设置 C 和 C++ 编译器为 MinGW 的路径
set(CMAKE_C_COMPILER "D:/mingw64/bin/gcc.exe")
set(CMAKE_CXX_COMPILER "D:/mingw64/bin/g++.exe")

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 设置异常处理相关的链接选项
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")


# 源文件
set(SOURCES
    src/main.cpp
    src/display.cpp
    src/hanidoku_solver.cpp
    src/cnf_parser.cpp
    src/utils.cpp
)

# 头文件
set(HEADERS
    include/display.h
    include/hanidoku_solver.h
    include/cnf_parser.h
    include/dpll_solver.h
    include/utils.h
    include/list.h
)

# 添加可执行文件
add_executable(HanidokuSolver ${SOURCES} ${HEADERS})

# 添加头文件搜索路径
target_include_directories(HanidokuSolver PRIVATE include)

# 添加线程库
find_package(Threads REQUIRED)
target_link_libraries(HanidokuSolver PRIVATE Threads::Threads)