# WinBGIm 64 bit

![example branch main](https://github.com/ki9gpin/WinBGIm-64/actions/workflows/cmake.yml/badge.svg)

Solution for graphics.h libbgi error

This build of libbgi.a is compatible with 64 bit compiler.
Which means you can link against it without linker error or 
where linker says it can't find libbgi.a cause 64 bit compiler 
ignores 32 bit build of the library.


Win32 system calls have been updated to reflect the changes in win32 api for 64 bit compatibility.

Link to original source http://winbgim.codecutter.org/



### To Build and Run
####  You can build and run this project directly and write your graphics program on top of it. 
###### For that
* #####        Clone the repo using 
      
      git clone https://github.com/ki9gpin/WinBGIm-64

* #####       If You are usin VS Code then open the WinBGIm-64 folder there. 

* #####      Insatll GCC (TDM64 or mingw64) . Install CMake .Make sure GCC and CMake are installed properly and accessible from command line.

* #####       VS code will suggest necessary extensions install them.  Or manually search for cmake in extensions and install the ones from microsoft. 

* #####       You will see a bar with cmake functionality down in the VS Code window or just close VS Code and reopen the folder and you will see the expected bar.

* #####         If you see the bar then to build the project click on build and if you can't see the bar then you can build manually by  pressing CTRL+SHIFT+P to get command pallete and selecting CMake Build.  

* #####       May be watching a video tutorial will help you if you are confused .It's easier than you think.
 
* #####       You can then go to *test* folder and modify *test.cpp* or write your own graphics code in the file.
