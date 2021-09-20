//*******************************************************************
//
//  file.c
//
//  Source file for Device-Independent Bitmap (DIB) API.  Provides
//  the following functions:
//
//  SaveDIB()           - Saves the specified dib in a file
//  LoadDIB()           - Loads a DIB from a file
//  DestroyDIB()        - Deletes DIB when finished using it
//
// Written by Microsoft Product Support Services, Developer Support.
// Copyright (C) 1991-1996 Microsoft Corporation. All rights reserved.
//*******************************************************************

#define     STRICT      // enable strict type checking

#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <io.h>
#include <direct.h>
#include <stdlib.h>
#include "dibutil.h"
#include "dibapi.h"
#include "winbgitypes.h" // Provides showerrorbox prototype


// Dib Header Marker - used in writing DIBs to files

#define DIB_HEADER_MARKER   ((WORD) ('M' << 8) | 'B')


/*********************************************************************
 *
 * Local Function Prototypes
 *
 *********************************************************************/


HANDLE ReadDIBFile(HANDLE);
BOOL SaveDIBFile(void);
BOOL WriteDIB(LPSTR, HANDLE);

/*************************************************************************
 *
 * LoadDIB()
 *
 * Loads the specified DIB from a file, allocates memory for it,
 * and reads the disk file into the memory.
 *
 *
 * Parameters:
 *
 * LPSTR lpFileName - specifies the file to load a DIB from
 *
 * Returns: A handle to a DIB, or NULL if unsuccessful.
 *
 * NOTE: The DIB API were not written to handle OS/2 DIBs; This
 * function will reject any file that is not a Windows DIB.
 *
 *************************************************************************/

HDIB LoadDIB(const char* lpFileName)
{
    HDIB        hDIB;
    HANDLE      hFile;

    // Set the cursor to a hourglass, in case the loading operation
    // takes more than a sec, the user will know what's going on.

    SetCursor(LoadCursor(NULL, IDC_WAIT));

    if ((hFile = CreateFile(lpFileName, GENERIC_READ, FILE_SHARE_READ, NULL,
            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
            NULL)) != INVALID_HANDLE_VALUE)
    {
        hDIB = ReadDIBFile(hFile);
        CloseHandle(hFile);
        SetCursor(LoadCursor(NULL, IDC_ARROW));
        return hDIB;
    }
    else
    {
        showerrorbox("File not found");
        SetCursor(LoadCursor(NULL, IDC_ARROW));
        return NULL;
    }
}


/*************************************************************************
 *
 * SaveDIB()
 *
 * Saves the specified DIB into the specified file name on disk.  No
 * error checking is done, so if the file already exists, it will be
 * written over.
 *
 * Parameters:
 *
 * HDIB hDib - Handle to the dib to save
 *
 * LPSTR lpFileName - pointer to full pathname to save DIB under
 *
 * Return value: 0 if successful, or one of:
 *        ERR_INVALIDHANDLE
 *        ERR_OPEN
 *        ERR_LOCK
 *
 *************************************************************************/

WORD SaveDIB(HDIB hDib, const char* lpFileName)
{
    BITMAPFILEHEADER    bmfHdr;     // Header for Bitmap file
    LPBITMAPINFOHEADER  lpBI;       // Pointer to DIB info structure
    HANDLE              fh;         // file handle for opened file
    DWORD               dwDIBSize;
    DWORD               dwWritten;

    if (!hDib)
        return ERR_INVALIDHANDLE;

    fh = CreateFile(lpFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);

    if (fh == INVALID_HANDLE_VALUE)
        return ERR_OPEN;

    // Get a pointer to the DIB memory, the first of which contains
    // a BITMAPINFO structure

    lpBI = (LPBITMAPINFOHEADER)GlobalLock(hDib);
    if (!lpBI)
    {
        CloseHandle(fh);
        return ERR_LOCK;
    }

    // Check to see if we're dealing with an OS/2 DIB.  If so, don't
    // save it because our functions aren't written to deal with these
    // DIBs.

    if (lpBI->biSize != sizeof(BITMAPINFOHEADER))
    {
        GlobalUnlock(hDib);
        CloseHandle(fh);
        return ERR_NOT_DIB;
    }

    // Fill in the fields of the file header

    // Fill in file type (first 2 bytes must be "BM" for a bitmap)

    bmfHdr.bfType = DIB_HEADER_MARKER;  // "BM"

    // Calculating the size of the DIB is a bit tricky (if we want to
    // do it right).  The easiest way to do this is to call GlobalSize()
    // on our global handle, but since the size of our global memory may have
    // been padded a few bytes, we may end up writing out a few too
    // many bytes to the file (which may cause problems with some apps,
    // like HC 3.0).
    //
    // So, instead let's calculate the size manually.
    //
    // To do this, find size of header plus size of color table.  Since the
    // first DWORD in both BITMAPINFOHEADER and BITMAPCOREHEADER conains
    // the size of the structure, let's use this.

    // Partial Calculation

    dwDIBSize = *(LPDWORD)lpBI + PaletteSize((LPSTR)lpBI);  

    // Now calculate the size of the image

    // It's an RLE bitmap, we can't calculate size, so trust the biSizeImage
    // field

    if ((lpBI->biCompression == BI_RLE8) || (lpBI->biCompression == BI_RLE4))
        dwDIBSize += lpBI->biSizeImage;
    else
    {
        DWORD dwBmBitsSize;  // Size of Bitmap Bits only

        // It's not RLE, so size is Width (DWORD aligned) * Height

        dwBmBitsSize = WIDTHBYTES((lpBI->biWidth)*((DWORD)lpBI->biBitCount)) *
                lpBI->biHeight;

        dwDIBSize += dwBmBitsSize;

        // Now, since we have calculated the correct size, why don't we
        // fill in the biSizeImage field (this will fix any .BMP files which 
        // have this field incorrect).

        lpBI->biSizeImage = dwBmBitsSize;
    }


    // Calculate the file size by adding the DIB size to sizeof(BITMAPFILEHEADER)
                   
    bmfHdr.bfSize = dwDIBSize + sizeof(BITMAPFILEHEADER);
    bmfHdr.bfReserved1 = 0;
    bmfHdr.bfReserved2 = 0;

    // Now, calculate the offset the actual bitmap bits will be in
    // the file -- It's the Bitmap file header plus the DIB header,
    // plus the size of the color table.
    
    bmfHdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + lpBI->biSize +
            PaletteSize((LPSTR)lpBI);

    // Write the file header

    WriteFile(fh, (LPSTR)&bmfHdr, sizeof(BITMAPFILEHEADER), &dwWritten, NULL);

    // Write the DIB header and the bits -- use local version of
    // MyWrite, so we can write more than 32767 bytes of data
    
    WriteFile(fh, (LPSTR)lpBI, dwDIBSize, &dwWritten, NULL);

    GlobalUnlock(hDib);
    CloseHandle(fh);

    if (dwWritten == 0)
        return ERR_OPEN; // oops, something happened in the write
    else
        return 0; // Success code
}


/*************************************************************************
 *
 * DestroyDIB ()
 *
 * Purpose:  Frees memory associated with a DIB
 *
 * Returns:  Nothing
 *
 *************************************************************************/

WORD DestroyDIB(HDIB hDib)
{
    GlobalFree(hDib);
    return 0;
}


//************************************************************************
//
// Auxiliary Functions which the above procedures use
//
//************************************************************************


/*************************************************************************
 *
 * Function:  ReadDIBFile (int)
 *
 *  Purpose:  Reads in the specified DIB file into a global chunk of
 *            memory.
 *
 *  Returns:  A handle to a dib (hDIB) if successful.
 *            NULL if an error occurs.
 *
 * Comments:  BITMAPFILEHEADER is stripped off of the DIB.  Everything
 *            from the end of the BITMAPFILEHEADER structure on is
 *            returned in the global memory handle.
 *
 *
 * NOTE: The DIB API were not written to handle OS/2 DIBs, so this
 * function will reject any file that is not a Windows DIB.
 *
 *************************************************************************/

HANDLE ReadDIBFile(HANDLE hFile)
{
    BITMAPFILEHEADER    bmfHeader;
    DWORD               dwBitsSize;
    UINT                nNumColors;   // Number of colors in table
    HANDLE              hDIB;        
    HANDLE              hDIBtmp;      // Used for GlobalRealloc() //MPB
    LPBITMAPINFOHEADER  lpbi;
    DWORD               offBits;
    DWORD               dwRead;

    // get length of DIB in bytes for use when reading

    dwBitsSize = GetFileSize(hFile, NULL);

    // Allocate memory for header & color table. We'll enlarge this
    // memory as needed.

    hDIB = GlobalAlloc(GMEM_MOVEABLE, (DWORD)(sizeof(BITMAPINFOHEADER) +
            256 * sizeof(RGBQUAD)));

    if (!hDIB)
        return NULL;

    lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDIB);

    if (!lpbi) 
    {
        GlobalFree(hDIB);
        return NULL;
    }

    // read the BITMAPFILEHEADER from our file

    if (!ReadFile(hFile, (LPSTR)&bmfHeader, sizeof (BITMAPFILEHEADER),
            &dwRead, NULL))
        goto ErrExit;

    if (sizeof (BITMAPFILEHEADER) != dwRead)
        goto ErrExit;

    if (bmfHeader.bfType != 0x4d42)  // 'BM'
        goto ErrExit;

    // read the BITMAPINFOHEADER

    if (!ReadFile(hFile, (LPSTR)lpbi, sizeof(BITMAPINFOHEADER), &dwRead,
            NULL))
        goto ErrExit;

    if (sizeof(BITMAPINFOHEADER) != dwRead)
        goto ErrExit;

    // Check to see that it's a Windows DIB -- an OS/2 DIB would cause
    // strange problems with the rest of the DIB API since the fields
    // in the header are different and the color table entries are
    // smaller.
    //
    // If it's not a Windows DIB (e.g. if biSize is wrong), return NULL.

    if (lpbi->biSize == sizeof(BITMAPCOREHEADER))
        goto ErrExit;

    // Now determine the size of the color table and read it.  Since the
    // bitmap bits are offset in the file by bfOffBits, we need to do some
    // special processing here to make sure the bits directly follow
    // the color table (because that's the format we are susposed to pass
    // back)

    if (!(nNumColors = (UINT)lpbi->biClrUsed))
    {
        // no color table for 24-bit, default size otherwise

        if (lpbi->biBitCount != 24)
            nNumColors = 1 << lpbi->biBitCount; // standard size table
    }

    // fill in some default values if they are zero

    if (lpbi->biClrUsed == 0)
        lpbi->biClrUsed = nNumColors;

    if (lpbi->biSizeImage == 0)
    {
        lpbi->biSizeImage = ((((lpbi->biWidth * (DWORD)lpbi->biBitCount) +
                31) & ~31) >> 3) * lpbi->biHeight;
    }

    // get a proper-sized buffer for header, color table and bits

    GlobalUnlock(hDIB);
    hDIBtmp = GlobalReAlloc(hDIB, lpbi->biSize + nNumColors *
            sizeof(RGBQUAD) + lpbi->biSizeImage, 0);

    if (!hDIBtmp) // can't resize buffer for loading
        goto ErrExitNoUnlock; //MPB
    else
        hDIB = hDIBtmp;

    lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDIB);

    // read the color table

    ReadFile (hFile, (LPSTR)(lpbi) + lpbi->biSize,
            nNumColors * sizeof(RGBQUAD), &dwRead, NULL);

    // offset to the bits from start of DIB header

    offBits = lpbi->biSize + nNumColors * sizeof(RGBQUAD);

    // If the bfOffBits field is non-zero, then the bits might *not* be
    // directly following the color table in the file.  Use the value in
    // bfOffBits to seek the bits.

    if (bmfHeader.bfOffBits != 0L)
        SetFilePointer(hFile, bmfHeader.bfOffBits, NULL, FILE_BEGIN);

    if (ReadFile(hFile, (LPSTR)lpbi + offBits, lpbi->biSizeImage, &dwRead,
            NULL))
        goto OKExit;


ErrExit:
    GlobalUnlock(hDIB);    

ErrExitNoUnlock:    
    GlobalFree(hDIB);
    return NULL;

OKExit:
    GlobalUnlock(hDIB);
    return hDIB;
}

