set(CMAKE_BUILD_TYPE Release)

cmake_minimum_required(VERSION 3.10)

project(HanidokuSolver)

# 设置 C 和 C++ 编译器为 MinGW 的路径
set(CMAKE_C_COMPILER "D:/mingw64/bin/gcc.exe")
set(CMAKE_CXX_COMPILER "D:/mingw64/bin/g++.exe")

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 设置异常处理相关的链接选项
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")

# 源文件
set(SOURCES
    src/main.cpp
    src/display.cpp
    src/cnf_parser.cpp
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

# 添加 OpenMp 库
find_package(OpenMP REQUIRED)
target_compile_options(HanidokuSolver PRIVATE ${OpenMP_CXX_FLAGS})
target_link_libraries(HanidokuSolver PRIVATE OpenMP::OpenMP_CXX)

set(CMAKE_PREFIX_PATH "D:/SFML-2.6.0")

# 添加 SFML 图形库
set(SFML_DIR "D:/SFML-2.6.0")

# 添加SFML的包含目录
include_directories(${SFML_DIR}/include)

# 添加SFML的库目录
link_directories(${SFML_DIR}/lib)

# 查找SFML库
find_package(SFML 3.0 COMPONENTS system window graphics REQUIRED)
# 链接SFML库
target_link_libraries(HanidokuSolver PRIVATE sfml-system sfml-window sfml-graphics)