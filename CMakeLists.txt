cmake_minimum_required(VERSION 3.21)
project(propertymap VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

# Find public Qt modules
find_package(Qt6 6.11 REQUIRED COMPONENTS Core Gui Quick Qml)

# Private module targets are shipped as separate CMake packages in Qt 6
# These are intentionally used for QMetaObjectBuilder and QQmlOpenMetaObject
set(QT_NO_PRIVATE_MODULE_WARNING ON)
find_package(Qt6CorePrivate REQUIRED PATHS ${Qt6_DIR}/..)
find_package(Qt6QmlPrivate REQUIRED PATHS ${Qt6_DIR}/..)

qt_policy(SET QTP0005 NEW)

qt_add_executable(propertymap
    main.cpp
    Player.cpp
    Player.h
    QmlPropertyMap/QmlPropertyMap.cpp
    QmlPropertyMap/QmlPropertyMap.h
    QuickPropertyMap/QuickPropertyMap.cpp
    QuickPropertyMap/QuickPropertyMap.h
    StaticPropertyMap/StaticPropertyMap.cpp
    StaticPropertyMap/StaticPropertyMap.h
)

# Link to public and private Qt modules
target_link_libraries(propertymap PRIVATE
    Qt6::Core
    Qt6::CorePrivate      # for QMetaObjectBuilder
    Qt6::Gui
    Qt6::Quick
    Qt6::Qml
    Qt6::QmlPrivate        # for QQmlOpenMetaObject
)

# Add QML resources via the Qt resource system
qt_add_resources(propertymap "qml"
    PREFIX "/"
    FILES
        Scene.qml
        Tank.qml
)

# Set target properties for a nice app
set_target_properties(propertymap PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)
