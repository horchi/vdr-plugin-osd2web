/*
 * imgtools.c
 *
 * See the README file for copyright information and how to reach the author.
 *
 */

#ifndef __IMGTOOLS_H
#define __IMGTOOLS_H

#include <stdio.h>

#define X_DISPLAY_MISSING 1

#include <jpeglib.h>
#include <Imlib2.h>

#include "common.h"

//***************************************************************************
// Image Manipulating
//***************************************************************************

int fromJpeg(Imlib_Image& image, unsigned char* buffer, int size);
long toJpeg(Imlib_Image image, MemoryStruct* data, int quality);
int scaleImageToJpegBuffer(Imlib_Image image, MemoryStruct* data, int width = 0, int height = 0);
int scaleJpegBuffer(MemoryStruct* data, int width = 0, int height = 0);
const char* strImlibError(Imlib_Load_Error err);
   
//***************************************************************************
#endif // __IMGTOOLS_H
