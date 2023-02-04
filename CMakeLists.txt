cmake_minimum_required(VERSION 3.5)

project(FEEBoardGUI VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# User defined library option
set(USE_VISA ON)
set(USE_ZABER ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Concurrent)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
find_package(ROOT REQUIRED COMPONENTS Core)

include_directories("include/")
file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cpp ${PROJECT_SOURCE_DIR}/src/*.ui)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.h)

# message(${VISA64} ${NIVISA64})
# link_libraries(Concurrent)
link_libraries(Ws2_32)
link_libraries(Iphlpapi)
link_libraries(
    ROOT::Core ROOT::Imt ROOT::RIO ROOT::Net ROOT::Hist ROOT::Graf ROOT::Graf3d ROOT::Gpad ROOT::ROOTDataFrame ROOT::Tree ROOT::TreePlayer ROOT::Rint ROOT::Postscript ROOT::Matrix ROOT::Physics ROOT::MathCore ROOT::Thread ROOT::ROOTVecOps
)

set(PROJECT_SOURCES
    main.cpp

    ${sources}
    ${headers}

    # include/FEEControlWidget.h
    # src/FEEControlWidget.ui
    # src/FEEControlWidget.cpp

    # include/feecontrol.h
    # src/feecontrol.cpp

    # include/datamanager.h
    # src/datamanager.cpp

    # include/configfileparser.h
    # src/configfileparser.cpp

    # include/ROOTDraw.h
    # src/ROOTDraw.cpp
    # src/ROOTDraw.ui

    # include/VDeviceController.h
    # src/VDeviceController.cpp

    # include/General.h
    # src/General.cpp
)

# User define ZABER Motion Control lib
if(USE_ZABER)
    include_directories("C:/lib/Zaber/include")
    include_directories("include/Zaber")
    find_library(ZMLLIB_d zml.lib "C:/Program\ Files/Zaber\ Motion\ Library/lib")
    find_library(ZMLLIB zml.lib "C:/lib/Zaber/lib")
    link_libraries(debug ${ZMLLIB_d}
        optimized ${ZMLLIB})

    set(PROJECT_SOURCES ${PROJECT_SOURCES}

        include/Zaber/ZaberControlWidget.h
        src/Zaber/ZaberControlWidget.cpp
        src/Zaber/ZaberControlWidget.ui

        include/Zaber/ZaberConnectionManager.h
        src/Zaber/ZaberConnectionManager.cpp
    )
    add_definitions(-DUSE_ZABER_MOTION)
endif()

# User define VISA Control lib
if(USE_VISA)
    include_directories("C:/Program\ Files/IVI Foundation/VISA/Win64/Include")
    include_directories("include/Visa")

    find_library(NIVISA64 nivisa64.lib "C:/Program\ Files/IVI\ Foundation/VISA/Win64/Lib_x64/msc")
    find_library(VISA64 visa64.lib "C:/Program\ Files/IVI\ Foundation/VISA/Win64/Lib_x64/msc")
    link_libraries(${VISA64} ${NIVISA64})

    set(PROJECT_SOURCES ${PROJECT_SOURCES}

        include/Visa/visaapi.h
        src/Visa/visaapi.cpp

        include/Visa/VisaDAQControl.h
        src/Visa/VisaDAQControl.cpp
        src/Visa/VisaDAQControl.ui
    )
    add_definitions(-DUSE_VISA_CONTROL)
endif()

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(FEEBoardGUI
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )

# Define target properties for Android with Qt 6 as:
# set_property(TARGET FEEBoardGUI APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
# ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(FEEBoardGUI SHARED
            ${PROJECT_SOURCES}
        )

    # Define properties for Android with Qt 5 after find_package() calls as:
    # set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(FEEBoardGUI
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(FEEBoardGUI PRIVATE Qt${QT_VERSION_MAJOR}::Widgets
)

set_target_properties(FEEBoardGUI PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(FEEBoardGUI)
endif()
