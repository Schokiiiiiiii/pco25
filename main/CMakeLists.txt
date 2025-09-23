cmake_minimum_required(VERSION 3.13)
project(counter)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Core Gui Widgets)
if (NOT Qt5_FOUND)
    find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)
endif()


add_executable(main
    main.cpp
)

target_link_libraries(main
    PRIVATE
        common
        -lpcosynchro
        -lpthread
        -lbenchmark
)