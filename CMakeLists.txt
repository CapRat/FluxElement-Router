cmake_minimum_required(VERSION 3.16)

project(RaspiHost VERSION 0.1 )

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Add our cmake folder to the module path
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(Qt6 REQUIRED COMPONENTS Quick)
find_package(PipeWire REQUIRED)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(appRaspiHost
        res/resources.qrc
        src/main.cpp
        src/PipeWireManager.cpp
        src/PipeWireManager.h
        src/PipeWireUIModel.cpp
        src/PipeWireUIModel.h
        src/PipeWire/Registry.cpp
        src/PipeWire/Registry.h
        src/Utils/Event.h
        src/PipeWire/Loop.cpp
        src/PipeWire/Loop.h
        src/PipeWire/Context.cpp
        src/PipeWire/Context.h
        src/PipeWire/Core.cpp
        src/PipeWire/Core.h
        src/PipeWire/Utils.h

)
add_executable(test src/clientMain.c)
target_link_libraries(test
        PUBLIC PipeWire::PipeWire
)

qt_add_qml_module(appRaspiHost
    URI RaspiHost
    QML_FILES ui/Main.qml
        ui/PipeWireNode.qml
        ui/screens/Home.qml
        ui/screens/WelcomeScreen.qml
        ui/screens/PipewireRouting.qml
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appRaspiHost PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appRaspiHost
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appRaspiHost
    PRIVATE Qt6::Quick PipeWire::PipeWire
)

include(GNUInstallDirs)
install(TARGETS appRaspiHost
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
