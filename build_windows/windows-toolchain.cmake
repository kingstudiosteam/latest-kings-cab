# CMake toolchain file for cross-compiling to Windows from macOS
# Using MinGW-w64

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# Specify the cross compiler
set(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)
set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)
set(CMAKE_RC_COMPILER x86_64-w64-mingw32-windres)

# Where to look for the target environment
set(CMAKE_FIND_ROOT_PATH /opt/homebrew/Cellar/mingw-w64/13.0.0/toolchain-x86_64)

# Search for programs in the build host directories
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# For libraries and headers in the target directories
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Windows-specific settings
set(CMAKE_SYSTEM_VERSION 10.0)
set(WIN32 TRUE)
set(MINGW TRUE)

# Ensure we're building 64-bit
set(CMAKE_SIZEOF_VOID_P 8)

# Windows threading
set(CMAKE_USE_WIN32_THREADS_INIT TRUE)

# Static linking to avoid DLL dependencies
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -static-libgcc -static-libstdc++")

# Windows specific definitions
add_definitions(-DWIN32 -D_WIN32 -D_WINDOWS)
add_definitions(-DUNICODE -D_UNICODE)

# JUCE specific settings for Windows
add_definitions(-DJUCE_WINDOWS=1)
add_definitions(-DJUCE_WIN32=1)
