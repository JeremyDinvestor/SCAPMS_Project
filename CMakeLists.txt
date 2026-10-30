cmake_minimum_required(VERSION 3.16)
project(SCAPMS LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
find_package(Qt6 REQUIRED COMPONENTS Widgets Svg)
add_executable(scapms main.cpp
    model/PatientRecord.cpp model/Patients.cpp
    controller/ClinicDatabase.cpp controller/ClinicManager.cpp
    gui/Icons.cpp gui/Theme.cpp gui/Widgets.cpp gui/Panels.cpp gui/MainWindow.cpp
    gui/Widgets.h gui/Panels.h gui/MainWindow.h)
target_include_directories(scapms PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(scapms PRIVATE Qt6::Widgets Qt6::Svg)
configure_file(data/scapms_data.txt ${CMAKE_BINARY_DIR}/scapms_data.txt COPYONLY)
