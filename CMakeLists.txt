cmake_minimum_required(VERSION 3.16)

project(OrionSimulator VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.2 REQUIRED COMPONENTS Core Gui Widgets Network Qml Quick Multimedia)

# --- HEADER (INCLUDE) YOLLARI ---
# Kod içinde #include "ConfigManager.h" diyebilmek için bu klasörleri ekliyoruz.
include_directories(
    src/Core
    src/Network
    src/Scenes
)

# --- KAYNAK DOSYALAR ---
set(PROJECT_SOURCES
    main.cpp

    # Core
    src/Core/ConfigManager.h
    src/Core/ConfigManager.cpp

    # Network
    src/Network/NetworkService.h
    src/Network/NetworkService.cpp
    src/Network/NetworkManager.h
    src/Network/NetworkManager.cpp

    # Scenes
    src/Scenes/IScene.h
    src/Scenes/SceneManager.h
    src/Scenes/SceneManager.cpp
    src/Scenes/ReflexScene.cpp
    src/Scenes/ReflexScene.h
    src/Scenes/PolygonScene.cpp
    src/Scenes/PolygonScene.h
)

qt_add_executable(OrionSimulator
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
    src/UI/Main.qml
)

target_link_libraries(OrionSimulator PRIVATE
    Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Network Qt6::Qml Qt6::Quick Qt6::Multimedia
)

# QML Modülü
qt_add_qml_module(OrionSimulator
    URI "Orion"
    VERSION 1.0
    QML_FILES src/UI/Main.qml
    SOURCES src/Scenes/ReflexScene.h src/Scenes/ReflexScene.cpp
    SOURCES src/Scenes/PolygonScene.h src/Scenes/PolygonScene.cpp
    RESOURCES
        assets/sounds/gunshot.wav
        assets/images/target_classic.png
)

# Config dosyasını kopyala
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/config.json DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(OrionSimulator)
endif()
