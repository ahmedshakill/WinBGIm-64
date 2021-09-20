// $Id: palette.cpp,v 1.1 2003/07/08 03:42:28 bush Exp $
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


/*****************************************************************************
*
*   The actual API calls are implemented below
*
*****************************************************************************/
palettetype *getdefaultpalette( )
{
    static palettetype default_palette = { 16,
                       { BLACK, BLUE, GREEN, CYAN, RED, MAGENTA, BROWN, LIGHTGRAY,
                         DARKGRAY, LIGHTBLUE, LIGHTGREEN, LIGHTCYAN, LIGHTRED,
                         LIGHTMAGENTA, YELLOW, WHITE } };

    return &default_palette;
}


void getpalette( palettetype *palette )
{

}


int getpalettesize( )
{
    return MAXCOLORS + 1;
}


void setallpalette( palettetype *palette )
{

}


void setpalette( int colornum, int color )
{

}


void setrgbpalette( int colornum, int red, int green, int blue )
{

}

