cmake_minimum_required(VERSION 3.14 FATAL_ERROR)
project(Calo5D LANGUAGES CXX)

#Define target libraries
add_library(Calo5D SHARED ${CMAKE_SOURCE_DIR}/src/Calo5D.cc)

### Dependencies ###
find_package(ILCUTIL REQUIRED COMPONENTS ILCSOFT_CMAKE_MODULES)
find_package(Marlin REQUIRED)
find_package(MarlinUtil REQUIRED) 
find_package(CED REQUIRED)
include(ilcsoft_default_settings)

target_include_directories(Calo5D PUBLIC
${Marlin_INCLUDE_DIRS}
${MarlinUtil_INCLUDE_DIRS}
${CED_INCLUDE_DIRS}
${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(Calo5D PUBLIC
${Marlin_LIBRARIES}
${MarlinUtil_LIBRARIES}
${CED_LIBRARIES}
)

target_compile_options(Calo5D PRIVATE -Wno-effc++)

install(TARGETS Calo5D DESTINATION ${CMAKE_SOURCE_DIR}/lib)
