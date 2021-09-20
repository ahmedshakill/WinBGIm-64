// $Id: winbgitypes.h,v 1.13 2003/07/21 23:14:45 bush Exp $
// Written by:
//      Grant Macklem (Grant.Macklem@colorado.edu)
//      Gregory Schmelter (Gregory.Schmelter@colorado.edu)
//      Alan Schmidt (Alan.Schmidt@colorado.edu)
//      Ivan Stashak (Ivan.Stashak@colorado.edu)
// CSCI 4830/7818: API Programming
// University of Colorado at Boulder, Spring 2003
// http://www.cs.colorado.edu/~main/bgi

#ifndef WINBGITYPES_H
#define WINBGITYPES_H

#include <windows.h>            // Provides the Win32 API
#include <tchar.h>              // Provides the _T macro
#include <queue>                // Provides STL queue class
#include <string>               // Provides STL string class
#include "winbgim.h"            // Provides other structures

// Define maximum pages used for drawing.
#define MAX_PAGES 4
typedef void (*Handler)(int, int);

// ---------------------------------------------------------------------------
//                              Structures
// ---------------------------------------------------------------------------
// This structure gives all necessary information to the ThreadInitWindow
// function which creates a new window and processes its messages
struct WindowData
{
    int width;                  // Width of window to create
    int height;                 // Height of window to create
    int initleft, inittop;      // Initial top and left coordinates on screen
    std::string title;          // Title at the top of the window
    std::queue<TCHAR> kbd_queue;// Queue of keyboard characters
    arccoordstype arcInfo;      // Information about the last arc drawn
    fillsettingstype fillInfo;  // Information about the fill style
    char uPattern[8];           // A user-defined fill style
    linesettingstype lineInfo;  // Information about the line style
    textsettingstype textInfo;  // Information about the text style
    viewporttype viewportInfo;  // Information about the viewport
    HWND hWnd;                  // Handle to the window created
    HDC hDC[MAX_PAGES];         // Device contexts used for double buffering
    HBITMAP hOldBitmap[MAX_PAGES]; // The bitmaps generated with CreateCompatibleBitmap
    int VisualPage;             // The current device context used for painting the window
    int ActivePage;             // The current device context used for drawing
    bool DoubleBuffer;          // Whether the user wants a double buffered window (DOUBLE_BUFFER in initwindow)
    bool CloseBehavior;         // false (do nothing); true (exit program)
    int drawColor;              // The current drawing color (That the user gave us)
    int bgColor;                // The current background color (That the user gave us)
    // TODO: Maybe cahnge bgColor to always be the 0 index into the palette
    HANDLE key_waiting;         // Event signaled when a key is pressed
    HANDLE WindowCreated;       // Running event
    DWORD threadID;             // ID of thread
    int error_code;             // Error code used by graphresult (usually grOk)
    int x_aspect_ratio;         // Horizontal Aspect Ratio
    int y_aspect_ratio;         // Vertical Aspect Ratio
    HBITMAP hbitmap;            // The bitmap of the image the user loaded
    POINT ipBitmap;             // Location to draw the image
    PBITMAPINFO pbmpInfo;       // Bitmap header info
    int t_scale[4];		// scaling factor for fonts multx, divx, multy, divy
    UINT alignment;		// current alignment
    POINTS mouse;               // Current location of the mouse
    std::queue<POINTS> clicks[WM_MOUSELAST - WM_MOUSEFIRST + 1];   // Array to hold the coordinates of the clicks
    bool mouse_queuing[WM_MOUSELAST - WM_MOUSEFIRST + 1]; // Array to tell whether mouse events should be queued
    Handler mouse_handlers[WM_MOUSELAST - WM_MOUSEFIRST + 1];   // Array of mouse event handlers
    bool refreshing;            // True if autorefershing should be done after each drawing event
    HANDLE hDCMutex;            // A mutex so that only one thread at a time can access the hDC array.
};
// maybe need current position for lines, text, etc.
// palette settings
// graph error result
// lock on window






// ---------------------------------------------------------------------------
//                              Prototypes
// ---------------------------------------------------------------------------
// The entry point for each new window thread (WindowThread.cpp)
DWORD WINAPI BGI__ThreadInitWindow( LPVOID pThreadData );

// Returns a DC for the window specified by hWnd.  If hWnd is NULL, the
// current window us used (drawing.cpp)
HDC BGI__GetWinbgiDC( HWND hWnd = NULL );
void BGI__ReleaseWinbgiDC( HWND hWnd = NULL );

// Returns a pointer to the window data structure associated with hWnd.
// If hWnd is NULL, the current window is used (drawing.cpp)
WindowData* BGI__GetWindowDataPtr( HWND hWnd = NULL );

// Refreshes an area of the window:
void RefreshWindow( RECT* rect );

// ---------------------------------------------------------------------------
//                            Global Variables
// ---------------------------------------------------------------------------
#include <vector>                      // MGM: Added for WindowTable
extern std::vector<HWND> BGI__WindowTable;  // WindowThread.cpp
extern int BGI__WindowCount;         // Number of windows currently in use, WindowThread.cpp
extern int BGI__CurrentWindow;       // Index to current window, WindowThread.cpp
extern COLORREF BGI__Colors[16];  // The RGB values for the Borland 16 colors, misc.cpp
extern HINSTANCE BGI__hInstance;     // Handle to the instance of the DLL
                                // (creating the window class) WindowThread.cpp


#endif  // WINBGITYPES_H

