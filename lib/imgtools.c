/*
 * imgtools.c
 *
 * See the README file for copyright information and how to reach the author.
 *
 */

#include "imgtools.h"

//***************************************************************************
// Image converting stuff
//***************************************************************************

int fromJpeg(Imlib_Image& image, unsigned char* buffer, int size)
{
   struct jpeg_decompress_struct cinfo;
   struct jpeg_error_mgr jerr;
   int w, h;
   DATA8 *ptr, *line[16], *data;
   DATA32 *ptr2, *dest;
   int x, y;

   cinfo.err = jpeg_std_error(&jerr);
   
   jpeg_create_decompress(&cinfo);
   jpeg_mem_src(&cinfo, buffer, size);
   jpeg_read_header(&cinfo, TRUE);
   cinfo.do_fancy_upsampling = FALSE;
   cinfo.do_block_smoothing = FALSE;
   
   jpeg_start_decompress(&cinfo);

   w = cinfo.output_width;
   h = cinfo.output_height;

   image = imlib_create_image(w, h);
   imlib_context_set_image(image);

   dest = ptr2 = imlib_image_get_data();
   data = (DATA8*)malloc(w * 16 * cinfo.output_components);

   for (int i = 0; i < cinfo.rec_outbuf_height; i++)
      line[i] = data + (i * w * cinfo.output_components);
   
   for (int l = 0; l < h; l += cinfo.rec_outbuf_height)
   {
      jpeg_read_scanlines(&cinfo, line, cinfo.rec_outbuf_height);
      int scans = cinfo.rec_outbuf_height;
      
      if (h - l < scans)
         scans = h - l;
      
      ptr = data;
      
      for (y = 0; y < scans; y++)
      {
         for (x = 0; x < w; x++)
         {
            *ptr2 = (0xff000000) | ((ptr[0]) << 16) | ((ptr[1]) << 8) | (ptr[2]);
            ptr += cinfo.output_components;
            ptr2++;
         }
      }
   }

   free(data);

   imlib_image_put_back_data(dest);

   jpeg_finish_decompress(&cinfo);
   jpeg_destroy_decompress(&cinfo);

   return success;
}

long toJpeg(Imlib_Image image, MemoryStruct* data, int quality)
{
   struct jpeg_compress_struct cinfo;
   struct jpeg_error_mgr jerr;
   DATA32* ptr;
   DATA8* buf;

   imlib_context_set_image(image);

   data->clear();
   
   cinfo.err = jpeg_std_error(&jerr);

   jpeg_create_compress(&cinfo);
   jpeg_mem_dest(&cinfo, (unsigned char**)(&data->memory), &data->size);

   cinfo.image_width = imlib_image_get_width();
   cinfo.image_height = imlib_image_get_height();
   cinfo.input_components = 3;
   cinfo.in_color_space = JCS_RGB;

   jpeg_set_defaults(&cinfo);
   jpeg_set_quality(&cinfo, quality, TRUE);
   jpeg_start_compress(&cinfo, TRUE);

   // get data pointer

   if (!(ptr = imlib_image_get_data_for_reading_only()))
      return 0;
   
   // allocate a small buffer to convert image data */

   buf = (DATA8*)malloc(imlib_image_get_width() * 3 * sizeof(DATA8));

   while (cinfo.next_scanline < cinfo.image_height) 
   {
      // convert scanline from ARGB to RGB packed

      for (int j = 0, i = 0; i < imlib_image_get_width(); i++)
      {
         buf[j++] = ((*ptr) >> 16) & 0xff;
         buf[j++] = ((*ptr) >>  8) & 0xff;
         buf[j++] = ((*ptr))       & 0xff;

         ptr++;
      }

      // write scanline

      jpeg_write_scanlines(&cinfo, (JSAMPROW*)(&buf), 1);
   }
   
   free(buf);
   jpeg_finish_compress(&cinfo);
   jpeg_destroy_compress(&cinfo);
   
   return data->size;
}

int scaleImageToJpegBuffer(Imlib_Image image, MemoryStruct* data, int width, int height)
{
   if (width && height)
   {
      Imlib_Image scaledImage;

      imlib_context_set_image(image);

      int imgWidth = imlib_image_get_width();
      int imgHeight = imlib_image_get_height();
      double ratio = (double)imgWidth / (double)imgHeight;
      
      if ((double)width/(double)imgWidth < (double)height/(double)imgHeight)
         height = (int)((double)width / ratio);
      else
         width = (int)((double)height * ratio);

      scaledImage = imlib_create_image(width, height);
      imlib_context_set_image(scaledImage);

      imlib_context_set_color(240, 240, 240, 255);
      imlib_image_fill_rectangle(0, 0, width, height);

      imlib_blend_image_onto_image(image, 0, 0, 0, 
                                   imgWidth, imgHeight, 0, 0, 
                                   width, height);
      
      toJpeg(scaledImage, data, 70);

      imlib_context_set_image(scaledImage);
      imlib_free_image();

      tell(2, "Scaled image to %d/%d, now %d bytes", width, height, (int)data->size);
   }
   else
   {
      toJpeg(image, data, 70);
   }

   return success;
}

int scaleJpegBuffer(MemoryStruct* data, int width, int height)
{
   Imlib_Image image;

   fromJpeg(image, (unsigned char*)data->memory, data->size);

   scaleImageToJpegBuffer(image, data, width, height);
   
   imlib_context_set_image(image);
   imlib_free_image();

   return success;
}

//***************************************************************************
// Str Imlib Error
//***************************************************************************

const char* strImlibError(Imlib_Load_Error err)
{
   switch (err)
   {
      case IMLIB_LOAD_ERROR_NONE:
      case IMLIB_LOAD_ERROR_FILE_DOES_NOT_EXIST:                return "File does not exist";
      case IMLIB_LOAD_ERROR_FILE_IS_DIRECTORY:                  return "File is directory";
      case IMLIB_LOAD_ERROR_PERMISSION_DENIED_TO_READ:          return "Permission denied to read";
      case IMLIB_LOAD_ERROR_NO_LOADER_FOR_FILE_FORMAT:          return "No loader for file format";
      case IMLIB_LOAD_ERROR_PATH_TOO_LONG:                      return "Path too long";
      case IMLIB_LOAD_ERROR_PATH_COMPONENT_NON_EXISTANT:        return "Path component non existant";
      case IMLIB_LOAD_ERROR_PATH_COMPONENT_NOT_DIRECTORY:       return "Path component not directory";
      case IMLIB_LOAD_ERROR_PATH_POINTS_OUTSIDE_ADDRESS_SPACE:  return "Path points outside address space";
      case IMLIB_LOAD_ERROR_TOO_MANY_SYMBOLIC_LINKS:            return "Too many symbolic links";
      case IMLIB_LOAD_ERROR_OUT_OF_MEMORY:                      return "Out of memory";
      case IMLIB_LOAD_ERROR_OUT_OF_FILE_DESCRIPTORS:            return "Out of file descriptors"; 
      case IMLIB_LOAD_ERROR_PERMISSION_DENIED_TO_WRITE:         return "Permission denied to write";
      case IMLIB_LOAD_ERROR_OUT_OF_DISK_SPACE:                  return "Out of disk space";
      case IMLIB_LOAD_ERROR_UNKNOWN:                            return "Unknown format";
   }
   
   return "";
}
