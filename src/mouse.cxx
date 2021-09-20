// $Id: mouse.cpp,v 1.1 2003/05/06 03:32:35 bush Exp $
// Written by:
//      Grant Macklem (Grant.Macklem@colorado.edu)
//      Gregory Schmelter (Gregory.Schmelter@colorado.edu)
//      Alan Schmidt (Alan.Schmidt@colorado.edu)
//      Ivan Stashak (Ivan.Stashak@colorado.edu)
// CSCI 4830/7818: API Programming
// University of Colorado at Boulder, Spring 2003
// http://www.cs.colorado.edu/~main/bgi
//


#include <windows.h>        // Provides Win32 API
#include <windowsx.h>       // Provides GDI helper macros
#include "winbgim.h"         // API routines
#include "winbgitypes.h"    // Internal structure data

/*****************************************************************************
*
*   Helper functions
*
*****************************************************************************/
// This function tests whether a given kind of mouse event is in range
// MGM: Added static to prevent linker conflicts
static bool MouseKindInRange( int kind )
{
    return ( (kind >= WM_MOUSEFIRST) && (kind <= WM_MOUSELAST) );
}


/*****************************************************************************
*
*   The actual API calls are implemented below
*   MGM: Moved ismouseclick function to top to stop g++ 3.2.3 internal error.
*****************************************************************************/
bool ismouseclick( int kind )
{
    WindowData *pWndData = BGI__GetWindowDataPtr( );
    return ( MouseKindInRange( kind ) && pWndData->clicks[kind - WM_MOUSEFIRST].size( ) );
}

void clearmouseclick( int kind )
{
    WindowData *pWndData = BGI__GetWindowDataPtr( );

    // Clear the mouse event
    if ( MouseKindInRange( kind ) && pWndData->clicks[kind - WM_MOUSEFIRST].size( ) )
        pWndData->clicks[kind - WM_MOUSEFIRST].pop( );
}

void getmouseclick( int kind, int& x, int& y )
{
    WindowData *pWndData = BGI__GetWindowDataPtr( );
    POINTS where; // POINT (short) to tell where mouse event happened.
    
    // Check if mouse event is in range
    if ( !MouseKindInRange( kind ) )
        return;

    // Set position variables to mouse location, or to NO_CLICK if no event occured
    if ( MouseKindInRange( kind ) && pWndData->clicks[kind - WM_MOUSEFIRST].size( ) )
    {
	where = pWndData->clicks[kind - WM_MOUSEFIRST].front( );
        pWndData->clicks[kind - WM_MOUSEFIRST].pop( );
        x = where.x;
        y = where.y;
    }
    else
    {
        x = y = NO_CLICK;
    }
}

void setmousequeuestatus( int kind, bool status )
{
    if ( MouseKindInRange( kind ) )
	BGI__GetWindowDataPtr( )->mouse_queuing[kind - WM_MOUSEFIRST] = status;
}

// TODO: This may be viewport relative.  The documentation specifies with will range from 0 to getmaxx()
int mousex( )
{
    WindowData *pWndData = BGI__GetWindowDataPtr( );
    return pWndData->mouse.x;
}


// TODO: This may be viewport relative.  The documentation specifies with will range from 0 to getmaxy()
int mousey( )
{
    WindowData *pWndData = BGI__GetWindowDataPtr( );
    return pWndData->mouse.y;
}


void registermousehandler( int kind, void h( int, int ) )
{
    WindowData *pWndData = BGI__GetWindowDataPtr( );
    if ( MouseKindInRange( kind ) )
        pWndData->mouse_handlers[kind - WM_MOUSEFIRST] = h;
}

