cmake_minimum_required(VERSION 2.6)
project(stepgui)
find_package(Qt4 REQUIRED)
add_definitions(-std=c++11)
include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR})


set(stepGui_SRCS StepGui.cpp
                 main.cpp
                 TrajectoryModel.cpp
                 qcustomplot.cpp)

set(stepGui_MOC StepGui.hpp
                TrajectoryModel.hpp
                qcustomplot.h)

qt4_wrap_cpp(stepGui_MOC_OUTFILES ${stepGui_MOC})
                 
                 
qt4_automoc(${stepGui_SRCS})
add_executable(stepgui ${stepGui_SRCS} ${stepGui_MOC_OUTFILES})
target_link_libraries(stepgui ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})

install(TARGETS stepgui RUNTIME DESTINATION bin)
