cmake_minimum_required(VERSION 3.16)

project(Calculator VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Quick)

qt_standard_project_setup(REQUIRES 6.5)

qt_add_executable(appCalculator
    main.cpp
)

qt_add_qml_module(appCalculator
    URI Calculator

    QML_FILES Main.qml
    SOURCES CalculatorBackend.h CalculatorBackend.cpp

    SOURCES FixedPointFloatMath/FixedPointFloatMath.h
            FixedPointFloatMath/FixedPointFloatMath.cpp

    QML_FILES CalculatorView/QML/ControlPanel.qml
              CalculatorView/QML/CalculatorTextInput.qml
              CalculatorView/QML/CalculatorButton.qml
              CalculatorView/QML/Calculator.qml
    SOURCES CalculatorView/CalculatorInputValidator.h
            CalculatorView/CalculatorInputValidator.cpp
            CalculatorView/ICalculatorBackend.h
            CalculatorView/ICalculatorBackend.cpp
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appCalculator PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appCalculator
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appCalculator
    PRIVATE Qt6::Quick
)

include(GNUInstallDirs)
install(TARGETS appCalculator
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
