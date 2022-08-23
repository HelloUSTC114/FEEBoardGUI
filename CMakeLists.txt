cmake_minimum_required(VERSION 3.5)

project(MuonTestControl VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
find_package(ROOT REQUIRED COMPONENTS Core)


include_directories("C:/Program\ Files/Zaber\ Motion\ Library/include" "include/")
#link_directories("C:/Program\ Files/Zaber\ Motion\ Library/lib")
#link_libraries(zml.lib)

find_library(ZMLLIB zml.lib "C:/Program\ Files/Zaber\ Motion\ Library/lib")
#Ws2_32.lib : windowssokets2 lib
link_libraries(${ZMLLIB} Ws2_32)
message(${ZMLLIB})

link_libraries(
    ROOT::Core ROOT::Imt ROOT::RIO ROOT::Net ROOT::Hist ROOT::Graf ROOT::Graf3d ROOT::Gpad ROOT::ROOTDataFrame ROOT::Tree ROOT::TreePlayer ROOT::Rint ROOT::Postscript ROOT::Matrix ROOT::Physics ROOT::MathCore ROOT::Thread ROOT::ROOTVecOps
)



set(PROJECT_SOURCES
        main.cpp

        include/FEEControlWidget.h
        src/FEEControlWidget.ui
        src/FEEControlWidget.cpp

        include/ConnectionManager.h
        src/ConnectionManager.cpp

        include/feecontrol.h
        src/feecontrol.cpp

        include/datamanager.h
        src/datamanager.cpp

        include/configfileparser.h
        src/configfileparser.cpp

        include/ROOTDraw.h
        src/ROOTDraw.cpp

        include/analyzer.h
        src/analyzer.cpp

        include/ftanalyzerwin.h 
        src/ftanalyzerwin.cpp 
        src/ftanalyzerwin.ui

        include/zabercontrolwidget.h
        src/zabercontrolwidget.cpp
        src/zabercontrolwidget.ui

        src/ROOTDraw.ui

)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(MuonTestControl
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET MuonTestControl APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(MuonTestControl SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(MuonTestControl
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(MuonTestControl PRIVATE Qt${QT_VERSION_MAJOR}::Widgets
)

set_target_properties(MuonTestControl PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(MuonTestControl)
endif()
