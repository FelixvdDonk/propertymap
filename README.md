# PropertyMap

PropertyMap playground source code for Qt World Summit 2019 speech "Fast C++-to-QML properties", modernized for **Qt 6.11** and **CMake**.

## Overview

This project benchmarks three different approaches for exposing dynamic C++ properties to QML:

| Implementation | Description |
|---|---|
| **`StaticPropertyMap`** | Uses compile-time `Q_PROPERTY` macros — fastest at runtime but not dynamic |
| **`QmlPropertyMap`** | Uses Qt's `QQmlOpenMetaObject` to create properties on the fly |
| **`QuickPropertyMap`** | Custom `QMetaObject` built via `QMetaObjectBuilder` — designed for maximum performance |

Switch between implementations by editing the `using PropertyMap = ...` alias in `Player.h`.

## Requirements

- **Qt 6.11+** (uses private APIs from `CorePrivate` and `QmlPrivate`)
- **CMake 3.21+**
- A C++17 compiler

## Building

```bash
# Configure (point CMAKE_PREFIX_PATH to your Qt installation)
cmake -B build -DCMAKE_PREFIX_PATH=/path/to/Qt/6.11.0/gcc_64

# Build
cmake --build build

# Run
./build/propertymap
```

## Key changes from the original (Qt 5 / qmake)

- Migrated from **qmake** (`.pro`) to **CMake**
- Replaced custom `QmlOpenMetaObject` with Qt 6's built-in **`QQmlOpenMetaObject`**
- Updated to **C++17**: `QRandomGenerator` instead of deprecated `qrand()`, `QList` instead of `QVector`, modern `QMetaType` APIs
- Modernized QML imports (unversioned `import QtQuick`)
- Fixed `Keys.onPressed` handler to use Qt 6's arrow-function syntax for the `event` parameter
