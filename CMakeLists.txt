cmake_minimum_required(VERSION 3.17)
project(WinBGIm)

set(CMAKE_CXX_STANDARD 11)

SET(GCC_COVERAGE_COMPILE_FLAGS "-g -fno-rtti -fno-exceptions -Wno-narrowing -fpermissive")
SET(GCC_COVERAGE_LINK_FLAGS    "-lgdi32 -lcomdlg32 -luuid -loleaut32 -lole32")

SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")
SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}")

add_executable(WinBGIm main.cpp
        src/drawing.cxx
        src/misc.cxx
        src/mouse.cxx
        src/palette.cxx
        src/text.cxx
        src/winbgi.cxx
        src/winthread.cxx
        src/dibutil.cxx
        src/file.cxx
        src/winbgim.h
        src/winbgitypes.h
        )
