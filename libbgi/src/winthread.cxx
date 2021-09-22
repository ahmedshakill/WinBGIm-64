// File: WindowThread.cpp
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
#include "../include/bgi/winbgim.h"             // External API routines
#include "../include/bgi/winbgitypes.h"        // Internal structures and routines
#include <vector>               // Used in BGI__WindowTable
#include <queue>                // Provides queue<POINTS>

// This structure is how the user interacts with the window.  Upon creation,y
// the user gets the index into this array of the window he created.  We will
// use this array for any function which does not deal with the window
// directly, but relies on the current window.
// MGM: hInstance is no longer a handle to the DLL, but instead it is
// a "self" handle, returned by GetCurrentThread( ).
std::vector<HWND> BGI__WindowTable;
int BGI__WindowCount = 0;                    // Number of windows currently in use
int BGI__CurrentWindow = NO_CURRENT_WINDOW;  // Index to current window
HINSTANCE BGI__hInstance;   // Handle to the instance of the DLL (creating the window class)

#include <iostream>
using namespace std;
// ID numbers for new options that are added to the system menu:
#define BGI_PRINT_SMALL 1
#define BGI_PRINT_MEDIUM 2
#define BGI_PRINT_LARGE 3
#define BGI_SAVE_AS 4
// This is the window creation and message processing function.  Each new
// window is created in its own thread and handles all its own messages.
// It creates the window, sets the current window of the application to this
// window, and signals the main thread that the window has been created.
//
DWORD WINAPI BGI__ThreadInitWindow( LPVOID pThreadData )
{
    HWND hWindow;                       // A handle to the window
    MSG Message;                        // A windows event message
    HDC hDC;                            // The device context of the window
    HBITMAP hBitmap;                    // A compatible bitmap of the DC for the Memory DC
    HMENU hMenu;                        // Handle to the system menu
    int CaptionHeight, xBorder, yBorder;
    
    WindowData *pWndData = (WindowData*)pThreadData; // Thread creation data

    if (pWndData->title.size( ))
    {
	CaptionHeight = GetSystemMetrics( SM_CYCAPTION );   // Height of caption
    }
    else
    {
	CaptionHeight = 0;                                 // Height of caption
    }
    xBorder = GetSystemMetrics( SM_CXFIXEDFRAME );      // Width of border
    yBorder = GetSystemMetrics( SM_CYFIXEDFRAME );      // Height of border
    
    int height = pWndData->height + CaptionHeight + 2*yBorder; // Calculate total height
    int width = pWndData->width + 2*xBorder;                   // Calculate total width

    int top = pWndData->inittop;                                 // MGM: Initial top
    int left = pWndData->initleft;                               // MGM: Initial left

    hWindow = CreateWindowEx( 0,                    // Extended window styles
                              _T( "BGILibrary" ),   // What kind of window
                              pWndData->title.c_str( ),  // Title at top of the window
			      pWndData->title.size( )
			      ? (WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_DLGFRAME)
			      : (WS_POPUP|WS_DLGFRAME),
                              left, top, width, height,  // left top width height
                              NULL,                 // HANDLE to the parent window
                              NULL,                 // HANDLE to the menu
                              BGI__hInstance,       // HANDLE to this program
                              NULL );               // Address of window-creation data
    // Check if the window was created
    if ( hWindow == 0 )
    {
        showerrorbox( );
        return 0;
    }

    // Add the print options to the system menu, as shown in Chapter 10 (p 460) of Petzold
    hMenu = GetSystemMenu( hWindow, FALSE );
    AppendMenu( hMenu, MF_SEPARATOR, 0, NULL );
    AppendMenu( hMenu, MF_STRING, BGI_SAVE_AS, TEXT("Save image as...") );
    AppendMenu( hMenu, MF_STRING, BGI_PRINT_SMALL, TEXT("Print 2\" wide...") );
    AppendMenu( hMenu, MF_STRING, BGI_PRINT_MEDIUM, TEXT("Print 4.5\" wide...") );
    AppendMenu( hMenu, MF_STRING, BGI_PRINT_LARGE, TEXT("Print 7\" wide...") );
    AppendMenu( hMenu, MF_SEPARATOR, 0, NULL );

    // Store the HANDLE in the structure
    pWndData->hWnd = hWindow;

    // Store the address of the WindowData structure in the window's user data
    // MGM TODO: Change this to SetWindowLongPtr:
    //  SetWindowLong( hWindow, GWLP_USERDATA, (LONG)pWndData );
    SetWindowLongPtr( hWindow, GWLP_USERDATA, (LONG_PTR)pWndData );

    // Set the default active and visual page.  These must be set here in
    // addition to setting all the defaults in initwindow because the paint
    // method depends on using the correct page.
    pWndData->ActivePage = 0;
    pWndData->VisualPage = 0;

    // Clear the mouse handler array and turn off queuing
    memset( pWndData->mouse_handlers, 0, (WM_MOUSELAST-WM_MOUSEFIRST+1) * sizeof(Handler) );
    memset( pWndData->mouse_queuing, 0, (WM_MOUSELAST-WM_MOUSEFIRST+1) * sizeof(bool) );

    // Create a memory Device Context used for drawing.  The image is copied from here
    // to the screen in the paint method.  The DC and bitmaps are deleted
    // in cls_OnDestroy()
    hDC = GetDC( hWindow );
    pWndData->hDCMutex = CreateMutex(NULL, FALSE,	NULL);
    WaitForSingleObject(pWndData->hDCMutex, 5000);
    for ( int i = 0; i < MAX_PAGES; i++ )
    {
        pWndData->hDC[i] = CreateCompatibleDC( hDC );
        // Create a bitmap for the memory DC.  This is where the drawn image is stored.
        hBitmap = CreateCompatibleBitmap( hDC, pWndData->width, pWndData->height );
        pWndData->hOldBitmap[i] = (HBITMAP)SelectObject( pWndData->hDC[i], hBitmap );
    }
    ReleaseMutex(pWndData->hDCMutex);    
    // Release the original DC and set up the mutex for the hDC array
    ReleaseDC( hWindow, hDC );
    
    // Make the window visible
    ShowWindow( hWindow, SW_SHOWNORMAL );           // Make the window visible
    UpdateWindow( hWindow );                        // Flush output buffer
    
    // Tell the user thread that the window was created successfully
    SetEvent( pWndData->WindowCreated );

    // The message loop, which stops when a WM_QUIT message arrives
    while( GetMessage( &Message, NULL, 0, 0 ) )
    {
        TranslateMessage( &Message );
        DispatchMessage( &Message );
    }

    // Free memory used by thread structure
    delete pWndData;
    return (DWORD)Message.wParam;
}


// This function handles the WM_CHAR message.  This message is sent whenever
// the user presses a key in the window (after a WM_KEYDOWN and WM_KEYUP
// message, a WM_CHAR message is added).  It adds the key pressed to the
// keyboard queue for the window.
//
void cls_OnChar( HWND hWnd, TCHAR ch, int repeat )
{
    // This gets the address of the WindowData structure associated with the window
    WindowData *pWndData = BGI__GetWindowDataPtr( hWnd );

    pWndData->kbd_queue.push( (TCHAR)ch );// Add the key to the queue
    SetEvent( pWndData->key_waiting );    // Notify the waiting thread, if any
    FORWARD_WM_CHAR( hWnd, ch, repeat, DefWindowProc );
}

static void cls_OnClose( HWND hWnd )
{
    // This gets the address of the WindowData structure associated with the window
    WindowData *pWndData = BGI__GetWindowDataPtr( hWnd );

    exit(0);
}

// This function handles the destroy message.  It will cause the application
// to send WM_QUIT, which will then terminate the message pump thread.
//
static void cls_OnDestroy( HWND hWnd )
{
    // This gets the address of the WindowData structure associated with the window
    WindowData *pWndData = BGI__GetWindowDataPtr( hWnd );

    WaitForSingleObject(pWndData->hDCMutex, 5000);
    for ( int i = 0; i < MAX_PAGES; i++ )
    {
        // Delete the pen in the DC's
        DeletePen( SelectPen( pWndData->hDC[i], GetStockPen( WHITE_PEN ) ) );

        // Delete the brush in the DC's
        DeleteBrush( SelectBrush( pWndData->hDC[i], GetStockBrush( WHITE_BRUSH ) ) );

        // Here we clean up the memory device contexts used by the program.
        // This selects the original bitmap back into the memory DC.  The SelectObject
        // function returns the current bitmap which we then delete.
        DeleteObject( SelectObject( pWndData->hDC[i], pWndData->hOldBitmap[i] ) );
        // Finally, we delete the MemoryDC
        DeleteObject( pWndData->hDC[i] );
    }
    ReleaseMutex(pWndData->hDCMutex);
    // Clean up the bitmap memory
    DeleteBitmap( pWndData->hbitmap );

    // Delete the two events created
    CloseHandle( pWndData->key_waiting );
    CloseHandle( pWndData->WindowCreated );

    PostQuitMessage( 0 );
}


// This function handles the KEYDOWN message and will translate the virtual
// keys to the keys expected by the user
//
static void cls_OnKey( HWND hWnd, UINT vk, BOOL down, int repeat, UINT flags )
{
    // This gets the address of the WindowData structure associated with the window
    // TODO: Set event for each key
    WindowData *pWndData = BGI__GetWindowDataPtr( hWnd );

    switch ( vk )
    {
    case VK_CLEAR:
        pWndData->kbd_queue.push( (TCHAR)0 );
        pWndData->kbd_queue.push( (TCHAR)KEY_CENTER );
        break;
    case VK_PRIOR:
        pWndData->kbd_queue.push( (TCHAR)0 );
        pWndData->kbd_queue.push( (TCHAR)KEY_PGUP );
        break;
    case VK_NEXT:
        pWndData->kbd_queue.push( (TCHAR)0 );
        pWndData->kbd_queue.push( (TCHAR)KEY_PGDN );
        break;
    case VK_END:
        pWndData->kbd_queue.push( (TCHAR)0 );
        pWndData->kbd_queue.push( (TCHAR)KEY_END );
        break;
    case VK_HOME:
        pWndData->kbd_queue.push( (TCHAR)0 );
        pWndData->kbd_queue.push( (TCHAR)KEY_HOME );
        break;
    case VK_LEFT:
        pWndData->kbd_queue.push( (TCHAR)0 );
        pWndData->kbd_queue.push( (TCHAR)KEY_LEFT );
        SetEvent( pWndData->key_waiting );
        break;
    case VK_UP:
        pWndData->kbd_queue.push( (TCHAR)0 );
        pWndData->kbd_queue.push( (TCHAR)KEY_UP );
        SetEvent( pWndData->key_waiting );
        break;
    case VK_RIGHT:
        pWndData->kbd_queue.push( (TCHAR)0 );
        pWndData->kbd_queue.push( (TCHAR)KEY_RIGHT );
        SetEvent( pWndData->key_waiting );
        break;
    case VK_DOWN:
        pWndData->kbd_queue.push( (TCHAR)0 );
        pWndData->kbd_queue.push( (TCHAR)KEY_DOWN );
        SetEvent( pWndData->key_waiting );
        break;
    case VK_INSERT:
        pWndData->kbd_queue.push( (TCHAR)0 );
        pWndData->kbd_queue.push( (TCHAR)KEY_INSERT );
        break;
    case VK_DELETE:
        pWndData->kbd_queue.push( (TCHAR)0 );
        pWndData->kbd_queue.push( (TCHAR)KEY_DELETE );
        break;
    case VK_F1:
        pWndData->kbd_queue.push( (TCHAR)0 );
        pWndData->kbd_queue.push( (TCHAR)KEY_F1 );
        break;
    case VK_F2:
        pWndData->kbd_queue.push( (TCHAR)0 );
        pWndData->kbd_queue.push( (TCHAR)KEY_F2 );
        break;
    case VK_F3:
        pWndData->kbd_queue.push( (TCHAR)0 );
        pWndData->kbd_queue.push( (TCHAR)KEY_F3 );
        break;
    case VK_F4:
        pWndData->kbd_queue.push( (TCHAR)0 );
        pWndData->kbd_queue.push( (TCHAR)KEY_F4 );
        break;
    case VK_F5:
        pWndData->kbd_queue.push( (TCHAR)0 );
        pWndData->kbd_queue.push( (TCHAR)KEY_F5 );
        break;
    case VK_F6:
        pWndData->kbd_queue.push( (TCHAR)0 );
        pWndData->kbd_queue.push( (TCHAR)KEY_F6 );
        break;
    case VK_F7:
        pWndData->kbd_queue.push( (TCHAR)0 );
        pWndData->kbd_queue.push( (TCHAR)KEY_F7 );
        break;
    case VK_F8:
        pWndData->kbd_queue.push( (TCHAR)0 );
        pWndData->kbd_queue.push( (TCHAR)KEY_F8 );
        break;
    case VK_F9:
        pWndData->kbd_queue.push( (TCHAR)0 );
        pWndData->kbd_queue.push( (TCHAR)KEY_F9 );
        break;
    }

    FORWARD_WM_KEYDOWN( hWnd, vk, repeat, flags, DefWindowProc );
}

#include <iostream>
static void cls_OnPaint( HWND hWnd )
{
    PAINTSTRUCT ps;             // BeginPaint puts info about the paint request here
    HDC hSrcDC;                 // The device context to copy from
    WindowData *pWndData = BGI__GetWindowDataPtr( hWnd );
    int width, height;          // Area that needs to be redrawn
    POINT srcCorner;            // Logical coordinates of the source image upper left point
    BOOL success;               // Is the BitBlt successful?
    int i;                      // Count for how many bitblts have been tried.

    WaitForSingleObject(pWndData->hDCMutex, INFINITE);
    BeginPaint( hWnd, &ps );
    hSrcDC = pWndData->hDC[pWndData->VisualPage];   // The source (memory) DC

    // Get the dimensions of the area that needs to be redrawn.
    width = ps.rcPaint.right - ps.rcPaint.left;
    height = ps.rcPaint.bottom - ps.rcPaint.top;
    

    // The region that needs to be updated is specified in device units (pixels) for the actual DC.
    // However, if a viewport is specified, the source image is referenced in logical
    // units.  Perform the conversion.
    // MGM TODO: When is the DPtoLP call needed?
    srcCorner.x = ps.rcPaint.left;
    srcCorner.y = ps.rcPaint.top;
    DPtoLP( hSrcDC, &srcCorner, 1 );

    // MGM: Screen BitBlts are not always successful, although I don't know why.
    success = BitBlt( ps.hdc, ps.rcPaint.left, ps.rcPaint.top, width, height,
			 hSrcDC, srcCorner.x, srcCorner.y, SRCCOPY );
    EndPaint( hWnd, &ps );  // Validates the rectangle
    ReleaseMutex(pWndData->hDCMutex);
    
    if ( !success )
    {   // I would like to invalidate the rectangle again
	// since BitBlt wasn't successful, but the recursion seems
	// to hang some machines.
	// delay(100);
	// InvalidateRect( hWnd, &(ps.rcPaint), FALSE );
	// std::cout << "Failure in cls_OnPaint" << std:: endl;
    }
}

// The message-handler function for the window
//
LRESULT CALLBACK WndProc
( HWND hWnd, UINT uiMessage, WPARAM wParam, LPARAM lParam )
{
    const std::queue<POINTS> EMPTY;
    POINTS where;
    
    WindowData *pWndData = BGI__GetWindowDataPtr( hWnd );
    int type;           // Type of mouse message
    Handler handler;    // Registered mouse handler
    UINT uHitTest;
    
    // If this is a mouse message, set our internal state
    if ( pWndData && ( uiMessage >= WM_MOUSEFIRST ) && ( uiMessage <= WM_MOUSELAST ) )
    {
	type = uiMessage - WM_MOUSEFIRST;
	if (!(pWndData->mouse_queuing[type]) && pWndData->clicks[type].size( ) )
	{
	    pWndData->clicks[type] = EMPTY;
	}
        pWndData->clicks[type].push(where = MAKEPOINTS( lParam ));  // Set the current position for the event type
	pWndData->mouse = where; // Set the current mouse position

        // If the user has registered a mouse handler, call it now
        handler = pWndData->mouse_handlers[type];
        if ( handler != NULL )
	    handler( where.x, where.y );
    }

    switch ( uiMessage )
    {
    HANDLE_MSG( hWnd, WM_CHAR, cls_OnChar );
    HANDLE_MSG( hWnd, WM_DESTROY, cls_OnDestroy );
    HANDLE_MSG( hWnd, WM_KEYDOWN, cls_OnKey );
    HANDLE_MSG( hWnd, WM_PAINT, cls_OnPaint );
    case WM_LBUTTONDBLCLK:
	return TRUE;
    case WM_NCHITTEST:
	uHitTest = DefWindowProc(hWnd, WM_NCHITTEST, wParam, lParam);
	if(uHitTest != HTCLIENT && pWndData && pWndData->title.size( ) == 0)
	    return HTCAPTION;
	else
	    return uHitTest;
    case WM_CLOSE:
	if ( pWndData->CloseBehavior )
	{
	    HANDLE_WM_CLOSE( hWnd, wParam, lParam, cls_OnClose );
	}
	return TRUE;
    case WM_SYSCOMMAND:
	switch ( LOWORD(wParam) )
	{
	case BGI_SAVE_AS: writeimagefile(NULL, 0, 0, INT_MAX, INT_MAX, false, hWnd); return 0;
	case BGI_PRINT_SMALL: printimage(NULL, 2.0, 0.75, 0.75, 0, 0, INT_MAX, INT_MAX, false, hWnd); return 0;
	case BGI_PRINT_MEDIUM: printimage(NULL, 4.5, 0.75, 0.75, 0, 0, INT_MAX, INT_MAX, false, hWnd); return 0;
	case BGI_PRINT_LARGE: printimage(NULL, 7.0, 0.75, 0.75, 0, 0, INT_MAX, INT_MAX, false, hWnd); return 0;
	}
	break;
    }
    return DefWindowProc( hWnd, uiMessage, wParam, lParam );
}

