add_library(bgi src/drawing.cxx
        src/misc.cxx
        src/mouse.cxx
        src/palette.cxx
        src/text.cxx
        src/winbgi.cxx
        src/winthread.cxx
        src/dibutil.cxx
        src/file.cxx
        include/bgi/winbgim.h
        include/bgi/winbgitypes.h)

target_include_directories(bgi
        PUBLIC include
        PUBLIC src)
target_link_libraries(bgi
        PUBLIC gdiplus comdlg32 gdi32 uuid oleaut32 ole32)
