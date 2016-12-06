# This file is configured to use a recent MinGW-w64 version with
# dwarf2 exception handling a win32 threading support
# e.g. \\lx03\shared3\programmi\mingw\i686-w64-mingw32-gcc-dw2-4.8.0-linux64_rubenvb.tar.xz,
# downloaded from
# http://sourceforge.net/projects/mingw-w64/files/Toolchains%20targetting%20Win32/Personal%20Builds/rubenvb/gcc-4.8-dw2-release/i686-w64-mingw32-gcc-dw2-4.8.0-linux64_rubenvb.tar.xz/download

# The toolchain is expected to stay in /opt/mingw32-dw2
SET(CMAKE_SYSTEM_NAME Windows)
SET(CMAKE_C_COMPILER /opt/mingw32-dw2/bin/i686-w64-mingw32-gcc)
SET(CMAKE_GENERATOR "MinGW Makefiles")
SET(CMAKE_RC_COMPILER /opt/mingw32-dw2/bin/i686-w64-mingw32-windres)
SET(CMAKE_FIND_ROOT_PATH /opt/mingw32-dw2 /opt/qt/qt_5.4.1_mingw32-dw2_lin64_win32)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_SYSTEM_PROCESSOR i686)
set(USE_PTHREAD OFF CACHE BOOL "")
add_definitions(-DWIN32)
