cmake_minimum_required(VERSION 3.11)
project(WinBGIm)

set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_STANDARD 11)

SET(GCC_COVERAGE_COMPILE_FLAGS "-m64 -fno-rtti -fno-exceptions -Wno-error  -Wwrite-strings -Wno-narrowing -fpermissive")
# SET(GCC_COVERAGE_COMPILE_FLAGS "-m64 -Wno-narrowing ")
SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")

add_subdirectory(libbgi)
add_subdirectory(test)

#target_link_libraries(test libbgi)

#SET(GCC_COVERAGE_LINK_FLAGS    "-lgdi32 -lcomdlg32 -luuid -loleaut32 -lole32")
#
#SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}")
#
#add_executable(bgi main.cpp
#        libbgi/src/drawing.cxx
#        libbgi/src/misc.cxx
#        libbgi/src/mouse.cxx
#        libbgi/src/palette.cxx
#        libbgi/src/text.cxx
#        libbgi/src/winbgi.cxx
#        libbgi/src/winthread.cxx
#        libbgi/src/dibutil.cxx
#        libbgi/src/file.cxx
#        libbgi/include/bgi/winbgim.h
#        libbgi/include/bgi/winbgitypes.h
#        )
