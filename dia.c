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

int cUpdate::initDia(const char* path)
{
   diaImages.clear();
   scanDiaDir(path);

   tell(0, "Info: Added (%ld) images of path '%s'", diaImages.size(), path);
   itCurrentDiaImage = diaImages.end();

   return success;
}

//***************************************************************************
// Scan Directory
//***************************************************************************

int cUpdate::scanDiaDir(const char* path, int level)
{
   const char* extensions = "jpeg:jpg";
   const char* ext;
   DIR* dir;
   int count = 0;

   // open directory

   if (!(dir = opendir(path)))
   {
      tell(1, "Can't open directory '%s', '%s'", path, strerror(errno));
      return done;
   }

   // iterate ..

   tell(1, "Info: Scanning %sdirectory '%s' for dia-show images", level ? "sub-" : "", path);

#ifndef HAVE_READDIR_R
   dirent* pEntry;

   while ((pEntry = readdir(dir)))
#else
   dirent entry;
   dirent* pEntry = &entry;
   dirent* res;

   // deprecated but the only reentrant with old libc!

   while (readdir_r(dir, pEntry, &res) == 0 && res)
#endif
   {
      ImageFile f;

      if (pEntry->d_type == DT_DIR)
      {
         char* subPath;

         if (strcmp(pEntry->d_name, ".") == 0 || strcmp(pEntry->d_name, "..") == 0)
            continue;

         asprintf(&subPath, "%s/%s", path, pEntry->d_name);
         scanDiaDir(subPath, level+1);
         free(subPath);
      }

      // check extension

      if ((ext = strrchr(pEntry->d_name, '.')))
         ext++;

      if (isEmpty(ext))
      {
         tell(4, "skipping file '%s' without extension", pEntry->d_name);
         continue;
      }

      if (!strcasestr(extensions, ext))
      {
         tell(4, "skipping file '%s' with extension '%s'", pEntry->d_name, ext);
         continue;
      }

      // fill image infos

      f.path = path + std::string("/") + pEntry->d_name;
      f.initialized = no;
      f.orientation = 1;                   // 1 => 'normal'
      f.landscape = yes;
      f.width = 0;
      f.height = 0;

      diaImages.push_back(f);
      count++;

      tell(3, "Info: Added '%s'", f.path.c_str());
   }

   tell(0, "Info: Added (%d) images of %sdirectory '%s' for dia-show", count, level ? "sub-" : "", path);
   closedir(dir);

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
