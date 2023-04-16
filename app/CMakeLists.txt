cmake_minimum_required(VERSION 3.4.1)

# Specifies a library name, specifies whether the library is STATIC or
# SHARED, and provides relative paths to the source code. You can
# define multiple libraries by adding multiple add_library() commands,
# and CMake builds them for you. When you build your app, Gradle
# automatically packages shared libraries with your APK.

add_library( # Specifies the name of the library.
        azartBluetooth

        # Sets the library as a shared library.
        SHARED
        # Provides a relative path to your source file(s).
        native-lib.cpp
        azart_proto_cl.cpp
        include/azart_proto_cl.h)

include_directories(src/main/cpp/include/)

find_library( # Sets the name of the path variable.
        log-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log )

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
        azartBluetooth

        # Links the target library to the log library
        # included in the NDK.
        ${log-lib} )
