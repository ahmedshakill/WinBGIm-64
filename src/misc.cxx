// $Id: misc.cpp,v 1.11 2003/05/07 23:41:23 schmidap Exp $
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
*   Structures
*
*****************************************************************************/
struct LinePattern
{
    int width;
    DWORD pattern[16];
};



/*****************************************************************************
*
*   Global Variables
*
*****************************************************************************/
// Solid line:  0xFFFF
// Dotted line: 0011 0011 0011 0011b    dot space
// Center line: 0001 1110 0011 1111b    dot space dash space
// Dashed line: 0001 1111 0001 1111b    dash space
// The numbers in the pattern (of the LinePattern struct) represent the width
// in pixels of the first dash, then the first space, then the next dash, etc.
// A leading space is moved to the end.
// The dash is one longer than specified; the space is one shorter.
// Creating a geometric pen using the predefined constants produces
// poor results.  Thus, the above widths have been modifed:
// Space: 3 pixels
// Dash:  8 pixels
// Dot:   4 pixels
LinePattern SOLID  = { 2, {16, 0} };        // In reality, these are (see note above)
LinePattern DOTTED = { 2, {3, 4} };         // 4, 3
LinePattern CENTER = { 4, {3, 4, 7, 4} };   // 4, 3, 8, 3
LinePattern DASHED = {2, {7, 4} };          // 8, 3

// Color format:
// High byte: 0  Color is an index, BGI color
//     -- This is necessary since these colors are defined to be 0-15 in BGI
// High byte: 3  Color is an RGB value (page 244 of Win32 book)
//     -- Note the internal COLORREF structure has RGB values with a high byte
//     of 0, but this conflicts with the BGI color notation.
// We store the value the user gave internally (be it number 4 for RED or
// our RGB encoded value).  This is then converted when needed.

// From http://www.textmodegames.com/articles/coolgame.html
// Then used BGI graphics on my system for slight modification.
COLORREF BGI__Colors[16];
// These are set in graphdefaults in winbgi.cpp
/*
= {
    RGB( 0, 0, 0 ),         // Black
    RGB( 0, 0, 168),        // Blue
    RGB( 0, 168, 0 ),       // Green
    RGB( 0, 168, 168 ),     // Cyan
    RGB( 168, 0, 0 ),       // Red
    RGB( 168, 0, 168 ),     // Magenta
    RGB( 168, 84, 0 ),      // Brown
    RGB( 168, 168, 168 ),   // Light Gray
    RGB( 84, 84, 84 ),      // Dark Gray
    RGB( 84, 84, 252 ),     // Light Blue
    RGB( 84, 252, 84 ),     // Light Green
    RGB( 84, 252, 252 ),    // Light Cyan
    RGB( 252, 84, 84 ),     // Light Red
    RGB( 252, 84, 252 ),    // Light Magenta
    RGB( 252, 252, 84 ),    // Yellow
    RGB( 252, 252, 252 )    // White
};
*/


/*****************************************************************************
*
*   Prototypes
*
*****************************************************************************/
LinePattern CreateUserStyle( );


/*****************************************************************************
*
*   Helper functions
*
*****************************************************************************/

// This function converts a given color (specified by the user) into a format
// native to windows.
//
int converttorgb( int color )
{
    // Convert from BGI color to RGB color
    if ( IS_BGI_COLOR( color ) )
        color = BGI__Colors[color];
    else
        color &= 0x0FFFFFF;

    return color;
}

#include <iostream>
// This function creates a new pen in the current drawing color and selects it
// into all the memory DC's.
//
void CreateNewPen( )
{
    WindowData* pWndData = BGI__GetWindowDataPtr( );
    int color = pWndData->drawColor;;
    LinePattern style;
    LOGBRUSH lb;
    HPEN hPen;

    // Convert from BGI color to RGB color
    color = converttorgb( color );

    // Set the color and style of the logical brush
    lb.lbColor = color;
    lb.lbStyle = BS_SOLID;

    if ( pWndData->lineInfo.linestyle == SOLID_LINE )   style = SOLID;
    if ( pWndData->lineInfo.linestyle == DOTTED_LINE )  style = DOTTED;
    if ( pWndData->lineInfo.linestyle == CENTER_LINE )  style = CENTER;
    if ( pWndData->lineInfo.linestyle == DASHED_LINE )  style = DASHED;
    // TODO: If user specifies a 0 pattern, create a NULL pen.
    if ( pWndData->lineInfo.linestyle == USERBIT_LINE ) style = CreateUserStyle( );

    // Round endcaps are default, set to square
    // Use a bevel join
    WaitForSingleObject(pWndData->hDCMutex, 5000);
    for ( int i = 0; i < MAX_PAGES; i++ )
    {
        hPen = ExtCreatePen( PS_GEOMETRIC | PS_ENDCAP_SQUARE 
                              | PS_JOIN_BEVEL | PS_USERSTYLE,   // Pen Style
                             pWndData->lineInfo.thickness,      // Pen Width
                             &lb,                               // Logical Brush
                             style.width,                       // Bytes in pattern
                             style.pattern );                   // Line Pattern
        DeletePen( (HPEN)SelectObject( pWndData->hDC[i], hPen ) );
    }
    ReleaseMutex(pWndData->hDCMutex);
}


// The user style might appear reversed from what you expect.  With the
// original Borland graphics, the least significant bit specified the first
// pixel of the line.  Thus, if you reverse the bit string, the line is
// drawn as the pixels then appear.
LinePattern CreateUserStyle( )
{
    WindowData* pWndData = BGI__GetWindowDataPtr( );
    int style = pWndData->lineInfo.upattern;
    int zeroCount = 0;          // A count of the number of leading zeros
    int i = 0, j, sum = 0;      // i is number of dwords, sum is a running count of bits used
    LinePattern userPattern;    // The pattern

    style &= 0xFFFF;            // Only lower 16 bits matter

    if ( style == 0 )
    {
        userPattern.pattern[0] = 0;
        userPattern.pattern[1] = 16;
        userPattern.width = 2;
        return userPattern;
    }

    // If the pattern starts with a zero, count how many and store until
    // later
    if ( (style & 1) == 0 )
    {
        for ( j = 0; !( style & 1 ); j++ ) style >>= 1;
        zeroCount = j;
        sum += j;
    }

    // See note above (in Global Variables) for dash being one pixel more,
    // space being one pixel less
    while( true )
    {
        // Get a count of the number of ones
        for ( j = 0; style & 1; j++ ) style >>= 1;
        userPattern.pattern[i++] = j-1;                     // Subtract one for dash
        sum += j;


        // Check if the pattern is now zero.
        if ( style == 0 )
        {
            if ( sum != 16 )
                userPattern.pattern[i++] = 16 - sum + 1;    // Add one for space
            break;
        }

        // Get a count of the number of zeros
        for ( j = 0; !( style & 1 ); j++ ) style >>= 1;
        userPattern.pattern[i++] = j + 1;                   // Add one for space
        sum += j;
    }

    // If there were leading zeros, put them at the end
    if ( zeroCount > 0 )
    {
        // If i is even, we ended on a space.  Add the leading zeros to the back
        // end count.  If i is odd, we ended on a dash.  Append the leading zeros.
        if ( (i % 2) == 0 )
            userPattern.pattern[i-1] += zeroCount;
        else
            userPattern.pattern[i++] = zeroCount;
    }
    else // If there were no leading zeros, check if we need to add a space
    {
        // If we ended on a dash and there are no more following zeros, put a
        // zero-length space.  This is necessary since the user may specify
        // a style of 0xFFFF.  In this case, a solid line is not created unless
        // there is a zero-length space at the end.
        if ( (i % 2) != 0 )
            userPattern.pattern[i++] = 0;
    }

    // Set the with to the number of array indices used
    userPattern.width = i;

    return userPattern;
}




/*****************************************************************************
*
*   The actual API calls are implemented below
*
*****************************************************************************/

// This function will pause the current thread for the specified number of
// milliseconds
//
void delay( int msec )
{
    Sleep( msec );
}


// This function returns information about the last call to arc.
//
void getarccoords( arccoordstype *arccoords )
{
    WindowData* pWndData = BGI__GetWindowDataPtr( );

    *arccoords = pWndData->arcInfo;
}


// This function returns the current background color.
//
int getbkcolor( )
{
    WindowData* pWndData = BGI__GetWindowDataPtr( );

    return pWndData->bgColor;
}


// This function returns the current drawing color.
//
int getcolor( )
{
    WindowData* pWndData = BGI__GetWindowDataPtr( );

    return pWndData->drawColor;
}


// This function returns the user-defined fill pattern in the 8-byte area
// specified by pattern.
//
void getfillpattern( char *pattern )
{
    WindowData* pWndData = BGI__GetWindowDataPtr( );

    memcpy( pattern, pWndData->uPattern, sizeof( pWndData->uPattern ) );
}


// This function returns the current fill settings.
//
void getfillsettings( fillsettingstype *fillinfo )
{
    WindowData* pWndData = BGI__GetWindowDataPtr( );
    *fillinfo = pWndData->fillInfo;
}


// This function returns the current line settings.
//
void getlinesettings( linesettingstype *lineinfo )
{
    WindowData* pWndData = BGI__GetWindowDataPtr( );
    *lineinfo = pWndData->lineInfo;
}


// This function returns the highest color possible in the current graphics
// mode.  For WinBGI, this is always WHITE (15), even though larger RGB
// colors are possible.
//
int getmaxcolor( )
{
    return WHITE;
}


// This function returns the maximum x screen coordinate.
//
int getmaxx( )
{
    WindowData* pWndData = BGI__GetWindowDataPtr( );
    return pWndData->width - 1;
}


// This function returns the maximum y screen coordinate.
int getmaxy( )
{
    WindowData* pWndData = BGI__GetWindowDataPtr( );
    return pWndData->height- 1;
}

// This function returns the maximum height of a window for the current screen
int getmaxheight( )
{
    int CaptionHeight = GetSystemMetrics( SM_CYCAPTION );   // Height of caption area
    int yBorder = GetSystemMetrics( SM_CYFIXEDFRAME );      // Height of border
    int TotalHeight = GetSystemMetrics( SM_CYSCREEN );      // Height of screen
    
    return TotalHeight - (CaptionHeight + 2*yBorder);       // Calculate max height
}

// This function returns the maximum width of a window for the current screen
int getmaxwidth( )
{
    int xBorder = GetSystemMetrics( SM_CXFIXEDFRAME );      // Width of border
    int TotalWidth = GetSystemMetrics( SM_CXSCREEN );       // Width of screen
    
    return TotalWidth - (2*xBorder);       // Calculate max width
}

// This function returns the total window height, including borders
int getwindowheight( )
{
    WindowData* pWndData = BGI__GetWindowDataPtr( );
    int CaptionHeight = GetSystemMetrics( SM_CYCAPTION );   // Height of caption area
    int yBorder = GetSystemMetrics( SM_CYFIXEDFRAME );      // Height of border

    return pWndData->height + CaptionHeight + 2*yBorder;    // Calculate total height
}

// This function returns the total window width, including borders
int getwindowwidth( )
{
    WindowData* pWndData = BGI__GetWindowDataPtr( );
    int xBorder = GetSystemMetrics( SM_CXFIXEDFRAME );      // Width of border

    return pWndData->width + 2*xBorder;                     // Calculate total width
}

// MGM: Function to convert rgb values to a color that can be
// used with any bgi functions.  Numbers 0 to WHITE are the
// original bgi colors. Other colors are 0x03rrggbb.
// This used to be a macro.
int COLOR(int r, int g, int b)
{
    COLORREF color = RGB(r,g,b);
    int i;

    for (i = 0; i <= WHITE; i++)
    {
	if ( color == BGI__Colors[i] )
	    return i;
    }

    return ( 0x03000000 | color );
}

int getdisplaycolor( int color )
{
    int save = getpixel( 0, 0 );
    int answer;
    
    putpixel( 0, 0, color );
    answer = getpixel( 0, 0 );
    putpixel( 0, 0, save );
    return answer;
}

int getpixel( int x, int y )
{
    HDC hDC = BGI__GetWinbgiDC( );
    COLORREF color = GetPixel( hDC, x, y );
    BGI__ReleaseWinbgiDC( );
    int i;

    if ( color == CLR_INVALID )
        return CLR_INVALID;

    // If the color is a BGI color, return the index rather than the RGB value.
    for ( i = 0; i <= WHITE; i++ )
    {
        if ( color == BGI__Colors[i] )
            return i;
    }

    // If we got here, the color didn't match a BGI color.  Thus, convert to 
    // our RGB format.
    color |= 0x03000000;

    return color;
}


void getviewsettings( viewporttype *viewport )
{
    WindowData* pWndData = BGI__GetWindowDataPtr( );
    *viewport = pWndData->viewportInfo;
}


// This function returns the x-cordinate of the current graphics position.
//
int getx( )
{
    HDC hDC = BGI__GetWinbgiDC( );
    POINT cp;

    GetCurrentPositionEx( hDC, &cp );
    BGI__ReleaseWinbgiDC( );
    return cp.x;
}


// This function returns the y-cordinate of the current graphics position.
//
int gety( )
{
    HDC hDC = BGI__GetWinbgiDC( );
    POINT cp;

    GetCurrentPositionEx( hDC, &cp );
    BGI__ReleaseWinbgiDC( );
    return cp.y;
}


// This function moves the current postion by dx pixels in the x direction and
// dy pixels in the y direction.
//
void moverel( int dx, int dy )
{
    HDC hDC = BGI__GetWinbgiDC( );
    POINT cp;

    // Get the current position
    GetCurrentPositionEx( hDC, &cp );
    // Move to the new posotion
    MoveToEx( hDC, cp.x + dx, cp.y + dy, NULL );
    BGI__ReleaseWinbgiDC( );
}


// This function moves the current point to position (x,y)
//
void moveto( int x, int y )
{
    HDC hDC = BGI__GetWinbgiDC( );

    MoveToEx( hDC, x, y, NULL );
    BGI__ReleaseWinbgiDC( );
}


void setbkcolor( int color )
{
    WindowData* pWndData = BGI__GetWindowDataPtr( );

    pWndData->bgColor = color;

    // Convert from BGI color to RGB color
    color = converttorgb( color );

    WaitForSingleObject(pWndData->hDCMutex, 5000);
    for ( int i = 0; i < MAX_PAGES; i++ )
        SetBkColor( pWndData->hDC[i], color );
    ReleaseMutex(pWndData->hDCMutex);
}


void setcolor( int color )
{
    WindowData* pWndData = BGI__GetWindowDataPtr( );

    // Update the color in our structure
    pWndData->drawColor = color;

    // Convert from BGI color to RGB color
    color = converttorgb( color );

    // Use that to set the text color for each page
    WaitForSingleObject(pWndData->hDCMutex, 5000);
    for ( int i = 0; i < MAX_PAGES; i++ )
        SetTextColor( pWndData->hDC[i], color );
    ReleaseMutex(pWndData->hDCMutex);

    // Create the new drawing pen
    CreateNewPen( );
}


void setlinestyle( int linestyle, unsigned upattern, int thickness )
{
    WindowData* pWndData = BGI__GetWindowDataPtr( );
    pWndData->lineInfo.linestyle = linestyle;
    pWndData->lineInfo.upattern = upattern;
    pWndData->lineInfo.thickness = thickness;

    // Create the new drawing pen
    CreateNewPen( );
}


// The user calls this function to create a brush with a pattern they create
//
void setfillpattern( char *upattern, int color )
{
    WindowData* pWndData = BGI__GetWindowDataPtr( );
    HBITMAP hBitmap;
    HBRUSH hBrush;
    unsigned short pattern[8];
    int i;

    // Copy the pattern to the storage for the window
    memcpy( pWndData->uPattern, upattern, sizeof( pWndData->uPattern ) );

    // Convert the pattern to create a brush
    for ( i = 0; i < 8; i++ )
        pattern[i] = (unsigned char)~upattern[i];       // Restrict to 8 bits

    // Set the settings for the structure
    pWndData->fillInfo.pattern = USER_FILL;
    pWndData->fillInfo.color = color;

    // Create the bitmap
    hBitmap = CreateBitmap( 8, 8, 1, 1, pattern );
    // Create a brush for each DC
    WaitForSingleObject(pWndData->hDCMutex, 5000);
    for ( int i = 0; i < MAX_PAGES; i++ )
    {
        hBrush = CreatePatternBrush( hBitmap );
        // Select the new brush into the device context and delete the old one.
        DeleteBrush( (HBRUSH)SelectBrush( pWndData->hDC[i], hBrush ) );
    }
    ReleaseMutex(pWndData->hDCMutex);
    // I'm not sure if it's safe to delete the bitmap here or not, but it
    // hasn't caused any problems.  The material I've found just says the
    // bitmap must be deleted in addition to the brush when finished.
    DeleteBitmap( hBitmap );

}


// If the USER_FILL pattern is passed, nothing is changed.
//
void setfillstyle( int pattern, int color )
{
    WindowData* pWndData = BGI__GetWindowDataPtr( );
    HDC hDC = BGI__GetWinbgiDC( );
    HBRUSH hBrush;
    // Unsigned char creates a truncation for some reason.
    unsigned short Slash[8]      = { ~0xE0, ~0xC1, ~0x83, ~0x07, ~0x0E, ~0x1C, ~0x38, ~0x70 };
    unsigned short BkSlash[8]    = { ~0x07, ~0x83, ~0xC1, ~0xE0, ~0x70, ~0x38, ~0x1C, ~0x0E };
    unsigned short Interleave[8] = { ~0xCC, ~0x33, ~0xCC, ~0x33, ~0xCC, ~0x33, ~0xCC, ~0x33 };
    unsigned short WideDot[8]    = { ~0x80, ~0x00, ~0x08, ~0x00, ~0x80, ~0x00, ~0x08, ~0x00 };
    unsigned short CloseDot[8]   = { ~0x88, ~0x00, ~0x22, ~0x00, ~0x88, ~0x00, ~0x22, ~0x00 };
    HBITMAP hBitmap;

    // Convert from BGI color to RGB color
    color = converttorgb( color );

    switch ( pattern )
    {
    case EMPTY_FILL:
        hBrush = CreateSolidBrush( converttorgb( pWndData->bgColor ));
        break;
    case SOLID_FILL:
        hBrush = CreateSolidBrush( color );
        break;
    case LINE_FILL:
        hBrush = CreateHatchBrush( HS_HORIZONTAL, color );
        break;
    case LTSLASH_FILL:
        hBrush = CreateHatchBrush( HS_BDIAGONAL, color );
        break;
    case SLASH_FILL:
        // The colors of the monochrome bitmap are drawn using the text color
        // and the current background color.
        // TODO: We may have to set the text color in every fill function
        // and then reset the text color to the user-specified text color
        // after the fill-draw routines are complete.
        hBitmap = CreateBitmap( 8, 8, 1, 1, Slash );
        hBrush = CreatePatternBrush( hBitmap );
        DeleteBitmap( hBitmap );
        break;
    case BKSLASH_FILL:
        hBitmap = CreateBitmap( 8, 8, 1, 1, BkSlash );
        hBrush = CreatePatternBrush( hBitmap );
        DeleteBitmap( hBitmap );
        break;
    case LTBKSLASH_FILL:
        hBrush = CreateHatchBrush( HS_FDIAGONAL, color );
        break;
    case HATCH_FILL:
        hBrush = CreateHatchBrush( HS_CROSS, color );
        break;
    case XHATCH_FILL:
        hBrush = CreateHatchBrush( HS_DIAGCROSS, color );
        break;
    case INTERLEAVE_FILL:
        hBitmap = CreateBitmap( 8, 8, 1, 1, Interleave );
        hBrush = CreatePatternBrush( hBitmap );
        DeleteBitmap( hBitmap );
        break;
    case WIDE_DOT_FILL:
        hBitmap = CreateBitmap( 8, 8, 1, 1, WideDot );
        hBrush = CreatePatternBrush( hBitmap );
        DeleteBitmap( hBitmap );
        break;
    case CLOSE_DOT_FILL:
        hBitmap = CreateBitmap( 8, 8, 1, 1, CloseDot );
        hBrush = CreatePatternBrush( hBitmap );
        DeleteBitmap( hBitmap );
        break;
    case USER_FILL:
        return;
        break;
    default:
        pWndData->error_code = grError;
        return;
    }

    // TODO: Modify this so the brush is created in every DC
    pWndData->fillInfo.pattern = pattern;
    pWndData->fillInfo.color = color;

    // Select the new brush into the device context and delete the old one.
    DeleteBrush( (HBRUSH)SelectBrush( hDC, hBrush ) );
    BGI__ReleaseWinbgiDC( );
}


void setviewport( int left, int top, int right, int bottom, int clip )
{
    WindowData* pWndData = BGI__GetWindowDataPtr( );
    HRGN hRGN = NULL;
    
    // Store the viewport information in the structure
    pWndData->viewportInfo.left = left;
    pWndData->viewportInfo.top = top;
    pWndData->viewportInfo.right = right;
    pWndData->viewportInfo.bottom = bottom;
    pWndData->viewportInfo.clip = clip;
    
    // If the drwaing should be clipped at the viewport boundary, create a
    // clipping region
    if ( clip != 0 )
        hRGN = CreateRectRgn( left, top, right, bottom );

    WaitForSingleObject(pWndData->hDCMutex, 5000);
    for ( int i = 0; i < MAX_PAGES; i++ )
    {
        SelectClipRgn( pWndData->hDC[i], hRGN );

        // Set the viewport origin to be the upper left corner
        SetViewportOrgEx( pWndData->hDC[i], left, top, NULL );

        // Move to the new origin
        MoveToEx( pWndData->hDC[i], 0, 0, NULL );
    }
    ReleaseMutex(pWndData->hDCMutex);
    // A copy of the region is used for the clipping region, so it is
    // safe to delete the region  (p. 369 Win32 API book)
    DeleteRgn( hRGN );
}


void setwritemode( int mode )
{
    HDC hDC = BGI__GetWinbgiDC( );

    if ( mode == COPY_PUT )
        SetROP2( hDC, R2_COPYPEN );
    if ( mode == XOR_PUT )
        SetROP2( hDC, R2_XORPEN );
    BGI__ReleaseWinbgiDC( );
}


