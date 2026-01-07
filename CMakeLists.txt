cmake_minimum_required(VERSION 3.13)

project(PCO_Barbier)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
add_compile_options(-g)

option(PCO_USE_HOARE_MONITOR "Use Hoare monitor instead of mutex unlock/lock around interface calls" ON)

if(PCO_USE_HOARE_MONITOR)
  add_compile_definitions(PCO_USE_HOARE_MONITOR=1)
else()
  add_compile_definitions(PCO_USE_HOARE_MONITOR=0)
endif()


find_package(Qt6 COMPONENTS Core Gui Test Widgets)
if (NOT Qt6_FOUND)
    find_package(Qt5 COMPONENTS Core Gui Test Widgets)
endif()


file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.h" "src/*.ui")

add_executable(pco_barbier ${SOURCES} barbier.qrc)


if (NOT Qt6_FOUND) 
    target_link_libraries(pco_barbier PRIVATE Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Test pcosynchro)
else()
    target_link_libraries(pco_barbier PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Test pcosynchro)
endif()


