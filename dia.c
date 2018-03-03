/**
 *  osd2web plugin for the Video Disk Recorder
 *
 *  dia.c
 *
 *  (c) 2017 Jörg Wendel
 *
 * This code is distributed under the terms and conditions of the
 * GNU GENERAL PUBLIC LICENSE. See the file COPYING for details.
 *
 **/

//***************************************************************************
// Includes
//***************************************************************************

#include <libexif/exif-data.h>

#include "config.h"
#include "update.h"

//***************************************************************************
// Get Jpeg Orientation
//***************************************************************************

int getJpegOrientation(const char* file)
{
   int orientation = 1;   // 1 => 'normal'
   ExifData* exifData = exif_data_new_from_file(file);

   if (exifData)
   {
      ExifByteOrder byteOrder = exif_data_get_byte_order(exifData);
      ExifEntry* exifEntry = exif_data_get_entry(exifData, EXIF_TAG_ORIENTATION);

      if (exifEntry)
         orientation = exif_get_short(exifEntry->data, byteOrder);

      exif_data_free(exifData);
   }

   return orientation;
}

//***************************************************************************
// JPEG Dimensions
//***************************************************************************

int jpegDimensions(const char* path, unsigned int& pWidth, unsigned int& pHeight)
{
   MemoryStruct data;
   unsigned char* pData;

   pWidth = pHeight = 0;

   if (loadFromFile(path, &data) != success)
   {
      tell(0, "Error loading '%s', error was '%s'", path, strerror(errno));
      return fail;
   }

   pData = (unsigned char*)data.memory;

   if (pData[0] != 0xFF || pData[1] != 0xD8)
      return fail;

   // retrieve the block length of the first block since the first block will not contain the size of file

   for (unsigned int i = 4; i < data.size; i += 2)
   {
      unsigned short block_length = pData[i] * 256 + pData[i+1];

      i += block_length;  // next block

      if (i >= data.size || pData[i] != 0xFF)
         return fail;

      // 0xFFC0 is the "Start Of Frame" marker which contains the file size

      if (pData[i+1] == 0xC0)
      {
         pHeight = pData[i+5] * 256 + pData[i+6];
         pWidth = pData[i+7] * 256 + pData[i+8];

         return success;
      }
   }

   return fail;
}

//***************************************************************************
// Dia Stuff
//***************************************************************************

#include <algorithm>
#include <random>

int cUpdate::initDia(const char* path)
{
   diaImages.clear();
   scanDiaDir(path);

   // sorted or random?

   if (config.diaRandom)
   {
      auto rng = std::default_random_engine { };
      std::shuffle(std::begin(diaImages), std::end(diaImages), rng);
   }
   else
   {
      std::sort(diaImages.begin(), diaImages.end());
   }

   itCurrentDiaImage = diaImages.end();

   return success;
}

//***************************************************************************
// Scan Directory
//***************************************************************************

int cUpdate::scanDiaDir(const char* path) // , int level)
{
   int count = 0;
   FileList imageFileList;

   tell(1, "Info: Scanning directory '%s' for dia-show images", path);

   if (getFileList(path, DT_REG, config.diaExtensions, yes, &imageFileList, count) != success)
      return done;

   for (auto it = imageFileList.begin(); it != imageFileList.end(); it++)
   {
      ImageFile f;
      FileInfo* fileInfo = &(*it);

      // fill image infos

      f.path = fileInfo->path + "/" + fileInfo->name;
      f.name = fileInfo->name;
      f.initialized = no;
      f.orientation = 1;                   // 1 => 'normal' as default
      f.landscape = yes;
      f.width = 0;
      f.height = 0;

      diaImages.push_back(f);
      tell(4, "Info: Added '%s'", f.path.c_str());
   }

   tell(1, "Info: Added (%ld) images of path '%s'", diaImages.size(), path);

   return success;
}

//***************************************************************************
// Get Next Dia Image
//***************************************************************************

int cUpdate::getNextDia(std::vector<ImageFile>::iterator& it, ImageFile*& file)
{
   if (it != diaImages.end())
      it++;

   if (it == diaImages.end())
      it = diaImages.begin();

   if (it == diaImages.end())
      return fail;

   file = &(*it);

   if (!file->initialized)
   {
      file->orientation = getJpegOrientation(file->path.c_str());
      jpegDimensions(file->path.c_str(), file->width, file->height);

      file->landscape =
         (file->orientation < 5 && file->width < file->height) ||
         (file->orientation >= 5 && file->width > file->height) ? no : yes;
   }

   return success;
}
