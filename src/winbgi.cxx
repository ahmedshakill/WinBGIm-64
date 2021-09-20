// File: winbgi.cpp
// Written by:
//      Grant Macklem (Grant.Macklem@colorado.edu)
//      Gregory Schmelter (Gregory.Schmelter@colorado.edu)
//      Alan Schmidt (Alan.Schmidt@colorado.edu)
//      Ivan Stashak (Ivan.Stashak@colorado.edu)
// CSCI 4830/7818: API Programming
// University of Colorado at Boulder, Spring 2003
// http://www.cs.colorado.edu/~main/bgi
//

#include <windows.h>            // Provides the Win32 API
#include <windowsx.h>           // Provides message cracker macros (p. 96)
#include <stdio.h>              // Provides sprintf
#include <iostream>             // This is for debug only
#include <vector>               // MGM: Added for BGI__WindowTable
#include "winbgim.h"             // External API routines
#include "winbgitypes.h"        // Internal structures and routines

// The window message-handling function (in WindowThread.cpp)
LRESULT CALLBACK WndProc( HWND, UINT, WPARAM, LPARAM );


// This function is called whenever a process attaches itself to the DLL.
// Thus, it registers itself in the process' address space.
//
BOOL registerWindowClass( )
{
    WNDCLASSEX wcex;                        // The main window class that we create

    // Set up the properties of the Windows BGI window class and register it
    wcex.cbSize = sizeof( WNDCLASSEX );     // Size of the strucutre
    wcex.style  = CS_SAVEBITS | CS_DBLCLKS; // Use default class styles
    wcex.lpfnWndProc = WndProc;             // The message handling function
    wcex.cbClsExtra = 0;                    // No extra memory allocation
    wcex.cbWndExtra = 0;
    wcex.hInstance = BGI__hInstance;        // HANDLE for this program
    wcex.hIcon = 0;                         // Use default icon
    wcex.hIconSm = 0;                       // Default small icon
    wcex.hCursor = LoadCursor( NULL, IDC_ARROW );       // Set mouse cursor
    wcex.hbrBackground = GetStockBrush( BLACK_BRUSH );  // Background color
    wcex.lpszMenuName = 0;                  // Menu identification
    wcex.lpszClassName = _T( "BGILibrary" );// a c-string name for the window

    if ( RegisterClassEx( &wcex ) == 0 )
    {
        showerrorbox( );
        return FALSE;
    }
    return TRUE;
}


// This function unregisters the window class so that it can be registered
// again if using LoadLibrary and FreeLibrary
void unregisterWindowClass( )
{
    UnregisterClass( "BGILibrary", BGI__hInstance );
}


// This is the entry point for the DLL
// MGM: I changed it to a regular function rather than a DLL entry
// point (since I deleted the DLL).  As such, it is now called
// by initwindow.
bool DllMain_MGM( 
                          HINSTANCE hinstDll,   // Handle to DLL module
                          DWORD Reason,         // Reason for calling function
                          LPVOID Reserved       // reserved
                          )
{
    // MGM: Add a static variable so that this work is done but once.
    static bool called = false;
    if (called) return true;
    called = true;

    switch ( Reason )
    {
    case DLL_PROCESS_ATTACH:
        BGI__hInstance = hinstDll;                   // Set the global hInstance variable
        return registerWindowClass( );          // Register the window class for this process
        break;
    case DLL_PROCESS_DETACH:
        unregisterWindowClass( );
        break;
    }
    return TRUE;        // Ignore other initialization cases
}



void showerrorbox( const char* msg )
{
    LPVOID lpMsgBuf;

    if ( msg == NULL )
    {
        // This code is taken from the MSDN help for FormatMessage
        FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,          // Formatting options
            NULL,                                   // Location of message definiton
            GetLastError( ),                        // Message ID
            MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), // Default language
            (LPTSTR) &lpMsgBuf,                     // Pointer to buffer
            0,                                      // Minimum size of buffer
            NULL );

        MessageBox( NULL, (LPCTSTR)lpMsgBuf, "Error", MB_OK | MB_ICONERROR );

        // Free the buffer
        LocalFree( lpMsgBuf );
    }
    else
        MessageBox( NULL, msg, "Error", MB_OK | MB_ICONERROR );
}


/*****************************************************************************
*
*   The actual API calls are implemented below
*   MGM: Rearranged order of functions by trial-and-error until internal
*   error in g++ 3.2.3 disappeared.
*****************************************************************************/


void graphdefaults( )
{
    WindowData *pWndData = BGI__GetWindowDataPtr( );
    HPEN hPen;      // The default drawing pen
    HBRUSH hBrush;  // The default filling brush
    int bgi_color;                      // A bgi color number
    COLORREF actual_color;              // The color that's actually put on the screen
    HDC hDC;

    // TODO: Do this for each DC

    // Set viewport to the entire screen and move current position to (0,0)
    setviewport( 0, 0, pWndData->width, pWndData->height, 0 );
    pWndData->mouse.x = 0;
    pWndData->mouse.y = 0;

    // Turn on autorefreshing
    pWndData->refreshing = true;
    
    // MGM: Grant suggested the following colors, but I have changed
    // them so that they will be consistent on any 16-color VGA
    // display.  It's also important to use 255-255-255 for white
    // to match some stock tools such as the stock WHITE brush
    /*
    BGI__Colors[0] = RGB( 0, 0, 0 );         // Black
    BGI__Colors[1] = RGB( 0, 0, 168);        // Blue
    BGI__Colors[2] = RGB( 0, 168, 0 );       // Green
    BGI__Colors[3] = RGB( 0, 168, 168 );     // Cyan
    BGI__Colors[4] = RGB( 168, 0, 0 );       // Red
    BGI__Colors[5] = RGB( 168, 0, 168 );     // Magenta
    BGI__Colors[6] = RGB( 168, 84, 0 );      // Brown
    BGI__Colors[7] = RGB( 168, 168, 168 );   // Light Gray
    BGI__Colors[8] = RGB( 84, 84, 84 );      // Dark Gray
    BGI__Colors[9] = RGB( 84, 84, 252 );     // Light Blue
    BGI__Colors[10] = RGB( 84, 252, 84 );    // Light Green
    BGI__Colors[11] = RGB( 84, 252, 252 );   // Light Cyan
    BGI__Colors[12] = RGB( 252, 84, 84 );    // Light Red
    BGI__Colors[13] = RGB( 252, 84, 252 );   // Light Magenta
    BGI__Colors[14] = RGB( 252, 252, 84 );   // Yellow
    BGI__Colors[15] = RGB( 252, 252, 252 );  // White
    for (bgi_color = 0; bgi_color <= WHITE; bgi_color++)
    {
	putpixel(0, 0, bgi_color);
	actual_color = GetPixel(hDC, 0, 0);
	BGI__Colors[bgi_color] = actual_color;
    }
    */

    BGI__Colors[0] = RGB( 0, 0, 0 );         // Black
    BGI__Colors[1] = RGB( 0, 0, 128);        // Blue
    BGI__Colors[2] = RGB( 0, 128, 0 );       // Green
    BGI__Colors[3] = RGB( 0, 128, 128 );     // Cyan
    BGI__Colors[4] = RGB( 128, 0, 0 );       // Red
    BGI__Colors[5] = RGB( 128, 0, 128 );     // Magenta
    BGI__Colors[6] = RGB( 128, 128, 0 );     // Brown
    BGI__Colors[7] = RGB( 192, 192, 192 );   // Light Gray
    BGI__Colors[8] = RGB( 128, 128, 128 );   // Dark Gray
    BGI__Colors[9] = RGB( 128, 128, 255 );   // Light Blue
    BGI__Colors[10] = RGB( 128, 255, 128 );  // Light Green
    BGI__Colors[11] = RGB( 128, 255, 255 );  // Light Cyan
    BGI__Colors[12] = RGB( 255, 128, 128 );  // Light Red
    BGI__Colors[13] = RGB( 255, 128, 255 );  // Light Magenta
    BGI__Colors[14] = RGB( 255, 255, 0 );  // Yellow
    BGI__Colors[15] = RGB( 255, 255, 255 );  // White

    // Set background color to default (black)
    setbkcolor( BLACK );

    // Set drawing color to default (white)
    pWndData->drawColor = WHITE;
    // Set fill style and pattern to default (white solid)
    pWndData->fillInfo.pattern = SOLID_FILL;
    pWndData->fillInfo.color = WHITE;

    hDC = BGI__GetWinbgiDC( );
    // Reset the pen and brushes for each DC
    for ( int i = 0; i < MAX_PAGES; i++ )
    {
	// Using Stock stuff is more efficient.
        // Create the default pen for drawing in the window.
        hPen = GetStockPen( WHITE_PEN );
        // Select this pen into the DC and delete the old pen.
        DeletePen( SelectPen( pWndData->hDC[i], hPen ) );

        // Create the default brush for painting in the window.
        hBrush = GetStockBrush( WHITE_BRUSH );
        // Select this brush into the DC and delete the old brush.
        DeleteBrush( SelectBrush( pWndData->hDC[i], hBrush ) );

	// Set the default text color for each page
	SetTextColor(pWndData->hDC[i], converttorgb(WHITE));
    }
    ReleaseMutex(pWndData->hDCMutex);

    // Set text font and justification to default
    pWndData->textInfo.horiz = LEFT_TEXT;
    pWndData->textInfo.vert = TOP_TEXT;
    pWndData->textInfo.font = DEFAULT_FONT;
    pWndData->textInfo.direction = HORIZ_DIR;
    pWndData->textInfo.charsize = 1;

    // set this something that it can never be to force set_align to be called
    pWndData->alignment = 2;

    pWndData->t_scale[0] = 1; // multx
    pWndData->t_scale[1] = 1; // divx
    pWndData->t_scale[2] = 1; // multy
    pWndData->t_scale[3] = 1; // divy

    // Set the error code to Ok: There is no error
    pWndData->error_code = grOk;

    // Line style as well?
    pWndData->lineInfo.linestyle = SOLID_LINE;
    pWndData->lineInfo.thickness = NORM_WIDTH;

    // Set the default active and visual page
    if ( pWndData->DoubleBuffer )
    {
        pWndData->ActivePage = 1;
        pWndData->VisualPage = 0;
    }
    else
    {
        pWndData->ActivePage = 0;
        pWndData->VisualPage = 0;
    }

    // Set the aspect ratios.  Unless Windows is doing something funky,
    // these should not need to be changed by the user to produce geometrically
    // correct shapes (circles, squares, etc).
    pWndData->x_aspect_ratio = 10000;
    pWndData->y_aspect_ratio = 10000;
}

using namespace std;
// The initwindow function is typicaly the first function called by the
// application.  It will create a separate thread for each window created.
// This thread is responsible for creating the window and processing all the
// messages.  It returns a positive integer value which the user can then
// use whenever he needs to reference the window.
// RETURN VALUE: If the window is successfully created, a nonnegative integer
//                  uniquely identifing the window.
//               On failure, -1.
//
int initwindow
( int width, int height, const char* title, int left, int top, bool dbflag , bool closeflag)
{
    HANDLE hThread;                     // Handle to the message pump thread
    int index;                          // Index of current window in the table
    HANDLE objects[2];                  // Handle to objects (thread and event) to ensure proper creation
    int code;                           // Return code of thread wait function

    // MGM: Call the DllMain, which used to be the DLL entry point.
    if (!DllMain_MGM(
        NULL,
        DLL_PROCESS_ATTACH,
        NULL))
        return -1;

    WindowData* pWndData = new WindowData;
    // Check if new failed
    if ( pWndData == NULL )
        return -1;

    // Todo: check to make sure the events are created successfully
    pWndData->key_waiting = CreateEvent( NULL, FALSE, FALSE, NULL );
    pWndData->WindowCreated = CreateEvent( NULL, FALSE, FALSE, NULL );
    pWndData->width = width;
    pWndData->height = height;
    pWndData->initleft = left;
    pWndData->inittop = top;
    pWndData->title = title; // Converts to a string object
    
    hThread = CreateThread( NULL,                   // Security Attributes (use default)
                            0,                      // Stack size (use default)
                            BGI__ThreadInitWindow,  // Start routine
                            (LPVOID)pWndData,       // Thread specific parameter
                            0,                      // Creation flags (run immediately)
                            &pWndData->threadID );  // Thread ID

    // Check if the message pump thread was created
    if ( hThread == NULL )
    {
        showerrorbox( );
        delete pWndData;
        return -1;
    }

    // Create an array of events to wait for
    objects[0] = hThread;
    objects[1] = pWndData->WindowCreated;
    // We'll either wait to be signaled that the window was created
    // successfully or the thread will terminate and we'll have some problem.
    code = WaitForMultipleObjects( 2,           // Number of objects to wait on
                                   objects,     // Array of objects
                                   FALSE,       // Whether all objects must be signaled
                                   INFINITE );  // How long to wait before timing out

    switch( code )
    {
    case WAIT_OBJECT_0:
        // Thread terminated without creating the window
        delete pWndData;
        return -1;
        break;
    case WAIT_OBJECT_0 + 1:
        // Successfully running
        break;
    }

    // Set index to the next available position
    index = BGI__WindowCount;
    // Increment the count
    ++BGI__WindowCount;
    // Store the window in the next position of the vector
    BGI__WindowTable.push_back(pWndData->hWnd);
    // Set the current window to the newly created window
    BGI__CurrentWindow = index;

    // Set double-buffering and close behavior
    pWndData->DoubleBuffer = dbflag;
    pWndData->CloseBehavior = closeflag;

    // Set the bitmap info struct to NULL
    pWndData->pbmpInfo = NULL; 

    // Set up the defaults for the window
    graphdefaults( );

    // MGM: Draw diagonal line since otherwise BitBlt into window doesn't work.
    setcolor(BLACK);
    line( 0, 0, width-1, height-1 );
    // MGM: The black rectangle is because on my laptop, the background
    // is not automatically reset to black when the window opens.
    setfillstyle(SOLID_FILL, BLACK);
    bar( 0, 0, width-1, height-1 );
    // MGM: Reset the color and fillstyle to the default for a new window.
    setcolor(WHITE);
    setfillstyle(SOLID_FILL, WHITE);

    // Everything went well!  Return the window index to the user.
    return index;
}


void closegraph(int wid)
{
    if (wid == CURRENT_WINDOW)
	closegraph(BGI__CurrentWindow);
    else if (wid == ALL_WINDOWS)
    {
	for ( int i = 0; i < BGI__WindowCount; i++ )
	    closegraph(i);
    }
    else if (wid >= 0 && wid <= BGI__WindowCount && BGI__WindowTable[wid] != NULL)
    {
        // DestroyWindow cannot delete a window created by another thread.
        // Thus, use SendMessage to close the requested window.
	// Destroying the window causes cls_OnDestroy to be called,
	// releasing any dynamic memory that's being used by the window.
	// The WindowData structure is released at the end of BGI__ThreadInitWindow,
	// which is reached when the message loop of BGI__ThreadInitWindow get WM_QUIT.
        SendMessage( BGI__WindowTable[wid], WM_DESTROY, 0, 0 );

	// Remove the HWND from the BGI__WindowTable vector:
	BGI__WindowTable[wid] = NULL;

	// Reset the global BGI__CurrentWindow if needed:
	if (BGI__CurrentWindow == wid)
	    BGI__CurrentWindow = NO_CURRENT_WINDOW;
    }
}


// This fuction detects the graphics driver and returns the highest resolution
// mode possible.  For WinBGI, this is always VGA/VGAHI
//
void detectgraph( int *graphdriver, int *graphmode )
{
    *graphdriver = VGA;
    *graphmode = VGAHI;
}


// This function returns the aspect ratio of the current window.  Unless there
// is something weird going on with Windows resolutions, these quantities
// will be equal and correctly proportioned geometric shapes will result.
//
void getaspectratio( int *xasp, int *yasp )
{
    WindowData *pWndData = BGI__GetWindowDataPtr( );
    *xasp = pWndData->x_aspect_ratio;
    *yasp = pWndData->y_aspect_ratio;
}


// This function will return the next character waiting to be read in the
// window's keyboard buffer
//
int getch( )
{
    char c;
    WindowData *pWndData = BGI__GetWindowDataPtr( );

    // TODO: Start critical section
    // check queue empty
    // end critical section

    if ( pWndData->kbd_queue.empty( ) )
        WaitForSingleObject( pWndData->key_waiting, INFINITE );
    else
    // Make sure the event is not signaled.  It will remain signaled until a
    // thread is woken up by it.  This could cause problems if the queue was
    // not empty and we got characters from it, never having to wait.  If the
	// queue then becomes empty, the WaitForSingleObjectX will immediately
    // return since it's still signaled.  If no key was pressed, this would
    // obviously be an error.
        ResetEvent( pWndData->key_waiting );

    // TODO: Start critical section
    // access queue
    // end critical section

    c = pWndData->kbd_queue.front( );             // Obtain the next element in the queue
    pWndData->kbd_queue.pop( );                   // Remove the element from the queue

    return c;
}


// This function returns the name of the current driver in use.  For WinBGI,
// this is always "EGAVGA"
//
char *getdrivername( )
{
    return "EGAVGA";
}


// This function gets the current graphics mode.  For WinBGI, this is always
// VGAHI
//
int getgraphmode( )
{
    return VGAHI;
}


// This function returns the maximum mode the current graphics driver can
// display.  For WinBGI, this is always VGAHI.
//
int getmaxmode( )
{
    return VGAHI;
}


// This function returns a string describing the current graphics mode.  It has the format
// "width*height MODE_NAME"  For WinBGI, this is the window size followed by VGAHI
//
char *getmodename( int mode_number )
{
    static char mode[20];
    WindowData *pWndData = BGI__GetWindowDataPtr( );

    sprintf( mode, "%d*%d VGAHI", pWndData->width, pWndData->height );
    return mode;
}


// This function returns the range of possible graphics modes for the given graphics driver.
// If -1 is given for the driver, the current driver and mode is used.
//
void getmoderange( int graphdriver, int *lomode, int *himode )
{
    WindowData *pWndData = BGI__GetWindowDataPtr( );
    int graphmode;

    // Use current driver modes
    if ( graphdriver == -1 )
        detectgraph( &graphdriver, &graphmode );

    switch ( graphdriver )
    {
    case CGA:
        *lomode = CGAC0;
        *himode = CGAHI;
        break;
    case MCGA:
        *lomode = MCGAC0;
        *himode = MCGAHI;
        break;
    case EGA:
        *lomode = EGALO;
        *himode = EGAHI;
        break;
    case EGA64:
        *lomode = EGA64LO;
        *himode = EGA64HI;
        break;
    case EGAMONO:
        *lomode = *himode = EGAMONOHI;
        break;
    case HERCMONO:
        *lomode = *himode = HERCMONOHI;
        break;
    case ATT400:
        *lomode = ATT400C0;
        *himode = ATT400HI;
        break;
    case VGA:
        *lomode = VGALO;
        *himode = VGAHI;
        break;
    case PC3270:
        *lomode = *himode = PC3270HI;
        break;
    case IBM8514:
        *lomode = IBM8514LO;
        *himode = IBM8514HI;
        break;
    default:
        *lomode = *himode = -1;
        pWndData->error_code = grInvalidDriver;
        break;
    }
}


// This function returns an error string corresponding to the given error code.
// This code is returned by graphresult()
//
char *grapherrormsg( int errorcode )
{
    static char *msg[16] = { "No error", "Graphics not installed",
        "Graphics hardware not detected", "Device driver not found",
        "Invalid device driver file", "Insufficient memory to load driver",
        "Out of memory in scan fill", "Out of memory in flood fill",
        "Font file not found", "Not enough meory to load font",
        "Invalid mode for selected driver", "Graphics error",
        "Graphics I/O error", "Invalid font file",
        "Invalid font number", "Invalid device number" };

    if ( ( errorcode < -16 ) || ( errorcode > 0 ) )
        return NULL;
    else
        return msg[-errorcode];
}


// This function returns the error code from the most recent graphics operation.
// It also resets the error to grOk.
//
int graphresult( )
{
    WindowData *pWndData = BGI__GetWindowDataPtr( );
    int code;

    code = pWndData->error_code;
    pWndData->error_code = grOk;
    return code;

}


// This function uses the information in graphdriver and graphmode to select
// the appropriate size for the window to be created.
//
void initgraph( int *graphdriver, int *graphmode, char *pathtodriver )
{
    WindowData *pWndData;
    int width = 0, height = 0;
    bool valid = true;
    
    if ( *graphdriver == DETECT )
        detectgraph( graphdriver, graphmode );

    switch ( *graphdriver )
    {
    case MCGA:
        switch ( *graphmode )
        {
        case MCGAC0:
        case MCGAC1:
        case MCGAC2:
        case MCGAC3:
            width = 320;
            height = 200;
            break;
        case MCGAMED:
            width = 640;
            height = 200;
            break;
        case MCGAHI:
            width = 640;
            height = 480;
            break;
        }
        break;
    case EGA:
        switch ( *graphmode )
        {
        case EGALO:
            width = 640;
            height = 200;
            break;
        case EGAHI:
            width = 640;
            height = 350;
            break;
        }
        break;
    case EGA64:
        switch ( *graphmode )
        {
        case EGALO:
            width = 640;
            height = 200;
            break;
        case EGAHI:
            width = 640;
            height = 350;
            break;
        }
        break;
    case EGAMONO:
        width = 640;
        height = 350;
        break;
    case HERCMONO:
        width = 720;
        width = 348;
        break;
    case ATT400:
        switch ( *graphmode )
        {
        case ATT400C0:
        case ATT400C1:
        case ATT400C2:
        case ATT400C3:
            width = 320;
            height = 200;
            break;
        case ATT400MED:
            width = 640;
            height = 200;
            break;
        case ATT400HI:
            width = 640;
            height = 400;
            break;
        }
        break;
    case VGA:
        switch ( *graphmode )
        {
        case VGALO:
            width = 640;
            height = 200;
            break;
        case VGAMED:
            width = 640;
            height = 350;
            break;
        case VGAHI:
            width = 640;
            height = 480;
            break;
        }
        break;
    case PC3270:
        width = 720;
        height = 350;
        break;
    case IBM8514:
        switch ( *graphmode )
        {
        case IBM8514LO:
            width = 640;
            height = 480;
            break;
        case IBM8514HI:
            width = 1024;
            height = 768;
            break;
        }
        break;
    default:
	valid = false;
    case CGA:
        switch ( *graphmode )
        {
        case CGAC0:
        case CGAC1:
        case CGAC2:
        case CGAC3:
            width = 320;
            height = 200;
            break;
        case CGAHI:
            width = 640;
            height = 200;
            break;
        }
        break;
    }

    // Create the window with with the specified dimensions
    initwindow( width, height );
    if (!valid)
    {
        pWndData = BGI__GetWindowDataPtr( );
        pWndData->error_code = grInvalidDriver;

    }
}


// This function does not do any work in WinBGI since the graphics and text
// windows are always both open.
//
void restorecrtmode( )
{ }


// This function returns true if there is a character waiting to be read
// from the window's keyboard buffer.
//
int kbhit( )
{
    // TODO: start critical section
    // check queue empty
    // end critical section
    WindowData *pWndData = BGI__GetWindowDataPtr( );

    return !pWndData->kbd_queue.empty( );
}


// This function sets the aspect ratio of the current window.
//
void setaspectratio( int xasp, int yasp )
{
    WindowData *pWndData = BGI__GetWindowDataPtr( );

    pWndData->x_aspect_ratio = xasp;
    pWndData->y_aspect_ratio = yasp;
}


void setgraphmode( int mode )
{
    // Reset graphics stuff to default
    graphdefaults( );
    // Clear the screen
    cleardevice( );
}




/*****************************************************************************
*
*   User-Controlled Window Functions
*
*****************************************************************************/

// This function returns the current window index to the user.  The user can
// use this return value to refer to the window at a later time.
//
int getcurrentwindow( )
{
    return BGI__CurrentWindow;
}


// This function sets the current window to the value specified by the user.
// All future drawing activity will be sent to this window.  If the window
// index is invalid, the current window is unchanged
//
void setcurrentwindow( int window )
{
    if ( (window < 0) || (window >= BGI__WindowCount) || BGI__WindowTable[window] == NULL)
        return;

    BGI__CurrentWindow = window;
}





/*****************************************************************************
*
*   Double buffering support
*
*****************************************************************************/

// This function returns the current active page for the current window.
//
int getactivepage( )
{
    WindowData *pWndData = BGI__GetWindowDataPtr( );
    return pWndData->ActivePage;
}


// This function returns the current visual page for the current window.
//
int getvisualpage( )
{
    WindowData *pWndData = BGI__GetWindowDataPtr( );
    return pWndData->VisualPage;
}


// This function changes the active page of the current window to the page
// specified by page.  If page refers to an invalid number, the current
// active page is unchanged.
//
void setactivepage( int page )
{
    WindowData *pWndData = BGI__GetWindowDataPtr( );

    if ( (page < 0) || (page > MAX_PAGES) )
        return;

    pWndData->ActivePage = page;
}


// This function changes the visual page of the current window to the page
// specified by page.  If page refers to an invalid number, the current
// visual page is unchanged.  The graphics window is then redrawn with the
// new page.
//
void setvisualpage( int page )
{
    WindowData *pWndData = BGI__GetWindowDataPtr( );

    if ( (page < 0) || (page > MAX_PAGES) )
        return;

    pWndData->VisualPage = page;

    // Redraw the entire current window.  No need to erase the background as
    // the new image is simply copied over the old one.
    InvalidateRect( pWndData->hWnd, NULL, FALSE );
}


// This function will swap the buffers if you have created a double-buffered
// window.  That is, by having the dbflag true when initwindow was called.
//
void swapbuffers( )
{
    WindowData *pWndData = BGI__GetWindowDataPtr( );
    
    if ( pWndData->ActivePage == 0 )
    {
        pWndData->VisualPage = 0;
        pWndData->ActivePage = 1;
    }
    else    // Active page is 1
    {
        pWndData->VisualPage = 1;
        pWndData->ActivePage = 0;
    }
    // Redraw the entire current window.  No need to erase the background as
    // the new image is simply copied over the old one.
    InvalidateRect( pWndData->hWnd, NULL, FALSE );
}

