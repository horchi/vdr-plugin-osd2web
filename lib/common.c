/*
 * common.c:
 *
 * See the README file for copyright information and how to reach the author.
 *
 */

#include <sys/stat.h>
#include <sys/time.h>

#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>

#ifdef USEUUID
#  include <uuid/uuid.h>
#endif

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <errno.h>
#include <regex.h>
#include <dirent.h>

#include <limits.h>
#include <algorithm>
#include <locale>

#ifdef USELIBARCHIVE
#  include <archive.h>
#  include <archive_entry.h>
#endif

#include "common.h"
#include "config.h"

cMyMutex logMutex;

//***************************************************************************
// Tell
//***************************************************************************

const char* getLogPrefix()
{
   return logPrefix;
}

void tell(int eloquence, const char* format, ...)
{
   enum { sizeBuffer = 100000 };
   static char buffer[sizeBuffer+100+TB] = "";

   if (cConfigBase::loglevel < eloquence)
      return ;

   logMutex.Lock();

#ifndef VDR_PLUGIN
   static int init = no;

   if (!init)
   {
      init = yes;
      openlog(cConfigBase::logName, LOG_CONS, cConfigBase::logFacility);
   }
#endif

   va_list ap;
   int max = sizeBuffer;

   *buffer = 0;

   if (cConfigBase::logwidth > 0)
      max = std::min((int)sizeBuffer, cConfigBase::logwidth);

   va_start(ap, format);

   if (getLogPrefix())
      snprintf(buffer, max, "%s", getLogPrefix());

   int bytesToWrite = max-strlen(buffer);
   int len = vsnprintf(eos(buffer), bytesToWrite, format, ap);

   if (len > bytesToWrite)
      sprintf(eos(buffer), "...");

   strReplace(buffer, '\n', '$');

   if (cConfigBase::logstdout)
   {
      char buf[50+TB];
      timeval tp;

      gettimeofday(&tp, 0);

      tm* tm = localtime(&tp.tv_sec);

      sprintf(buf,"%2.2d:%2.2d:%2.2d,%3.3ld ",
              tm->tm_hour, tm->tm_min, tm->tm_sec,
              tp.tv_usec / 1000);

      printf("%s %s\n", buf, buffer);
   }
   else
      syslog(LOG_ERR, "%s", buffer);

   logMutex.Unlock();

   va_end(ap);
}

//***************************************************************************
// Syslog Facilities
//***************************************************************************

const Syslog::Facilities Syslog::facilities[] =
{
   { "auth",     LOG_AUTH   },
   { "cron",     LOG_CRON   },
   { "daemon",   LOG_DAEMON },
   { "kern",     LOG_KERN   },
   { "lpr",      LOG_LPR    },
   { "mail",     LOG_MAIL   },
   { "news",     LOG_NEWS   },
   { "security", LOG_AUTH   },
   { "syslog",   LOG_SYSLOG },
   { "user",     LOG_USER   },
   { "uucp",     LOG_UUCP   },
   { "local0",   LOG_LOCAL0 },
   { "local1",   LOG_LOCAL1 },
   { "local2",   LOG_LOCAL2 },
   { "local3",   LOG_LOCAL3 },
   { "local4",   LOG_LOCAL4 },
   { "local5",   LOG_LOCAL5 },
   { "local6",   LOG_LOCAL6 },
   { "local7",   LOG_LOCAL7 },
   { 0, 0 }
};

//***************************************************************************
// To Name
//***************************************************************************

char* Syslog::toName(int code)
{
   for (int i = 0; facilities[i].name; i++)
      if (facilities[i].code == code)
         return (char*) facilities[i].name;                           // #83

   return 0;
}

//***************************************************************************
// To Code
//***************************************************************************

int Syslog::toCode(const char* name)
{
   for (int i = 0; facilities[i].name; i++)
      if (strcmp(facilities[i].name, name) == 0)
         return facilities[i].code;

   return na;
}

//***************************************************************************
// Save Realloc
//***************************************************************************

char* srealloc(void* ptr, size_t size)
{
   void* n = realloc(ptr, size);

   if (!n)
   {
      free(ptr);
      ptr = 0;
   }

   return (char*)n;
}

//***************************************************************************
// us now
//***************************************************************************

double usNow()
{
   struct timeval tp;

   gettimeofday(&tp, 0);

   return tp.tv_sec * 1000000.0 + tp.tv_usec;
}

//***************************************************************************
// Host ID
//***************************************************************************

unsigned int getHostId()
{
   static unsigned int id = gethostid() & 0xFFFFFFFF;
   return id;
}

//***************************************************************************
// String Operations
//***************************************************************************

void toUpper(std::string& str)
{
   const char* s = str.c_str();
   int lenSrc = str.length();

   char* dest = (char*)malloc(lenSrc+TB); *dest = 0;
   char* d = dest;

   int csSrc;  // size of character

   for (int ps = 0; ps < lenSrc; ps += csSrc)
   {
      csSrc = std::max(mblen(&s[ps], lenSrc-ps), 1);

      if (csSrc == 1)
         *d++ = toupper(s[ps]);
      else if (csSrc == 2 && s[ps] == (char)0xc3 && s[ps+1] >= (char)0xa0)
      {
         *d++ = s[ps];
         *d++ = s[ps+1] - 32;
      }
      else
      {
         for (int i = 0; i < csSrc; i++)
            *d++ = s[ps+i];
      }
   }

   *d = 0;

   str = dest;
   free(dest);
}

//***************************************************************************
// To Case (UTF-8 save)
//***************************************************************************

const char* toCase(Case cs, char* str)
{
   char* s = str;
   int lenSrc = strlen(str);

   int csSrc;  // size of character

   for (int ps = 0; ps < lenSrc; ps += csSrc)
   {
      csSrc = std::max(mblen(&s[ps], lenSrc-ps), 1);

      if (csSrc == 1)
         s[ps] = cs == cUpper ? toupper(s[ps]) : tolower(s[ps]);
      else if (csSrc == 2 && s[ps] == (char)0xc3 && s[ps+1] >= (char)0xa0)
      {
         s[ps] = s[ps];
         s[ps+1] = cs == cUpper ? toupper(s[ps+1]) : tolower(s[ps+1]);
      }
      else
      {
         for (int i = 0; i < csSrc; i++)
            s[ps+i] = s[ps+i];
      }
   }

   return str;
}

void removeChars(std::string& str, const char* ignore)
{
   const char* s = str.c_str();
   int lenSrc = str.length();
   int lenIgn = strlen(ignore);

   char* dest = (char*)malloc(lenSrc+TB); *dest = 0;
   char* d = dest;

   int csSrc;  // size of character
   int csIgn;  //

   for (int ps = 0; ps < lenSrc; ps += csSrc)
   {
      int skip = no;

      csSrc = std::max(mblen(&s[ps], lenSrc-ps), 1);

      for (int pi = 0; pi < lenIgn; pi += csIgn)
      {
         csIgn = std::max(mblen(&ignore[pi], lenIgn-pi), 1);

         if (csSrc == csIgn && strncmp(&s[ps], &ignore[pi], csSrc) == 0)
         {
            skip = yes;
            break;
         }
      }

      if (!skip)
      {
         for (int i = 0; i < csSrc; i++)
            *d++ = s[ps+i];
      }
   }

   *d = 0;

   str = dest;
   free(dest);
}

void removeCharsExcept(std::string& str, const char* except)
{
   const char* s = str.c_str();
   int lenSrc = str.length();
   int lenIgn = strlen(except);

   char* dest = (char*)malloc(lenSrc+TB); *dest = 0;
   char* d = dest;

   int csSrc;  // size of character
   int csIgn;  //

   for (int ps = 0; ps < lenSrc; ps += csSrc)
   {
      int skip = yes;

      mblen(0,0);
      csSrc = std::max(mblen(&s[ps], lenSrc-ps), 1);

      for (int pi = 0; pi < lenIgn; pi += csIgn)
      {
         csIgn = std::max(mblen(&except[pi], lenIgn-pi), 1);

         if (csSrc == csIgn && strncmp(&s[ps], &except[pi], csSrc) == 0)
         {
            skip = no;
            break;
         }
      }

      if (!skip)
      {
         for (int i = 0; i < csSrc; i++)
            *d++ = s[ps+i];
      }
   }

   *d = 0;

   str = dest;
   free(dest);
}

void removeWord(std::string& pattern, std::string word)
{
   size_t  pos;

   if ((pos = pattern.find(word)) != std::string::npos)
      pattern.swap(pattern.erase(pos, word.length()));
}

//***************************************************************************
// String Manipulation
//***************************************************************************

void prepareCompressed(std::string& pattern)
{
   // const char* ignore = " (),.;:-_+*!#?=&%$<>§/'`´@~\"[]{}";
   const char* notignore = "ABCDEFGHIJKLMNOPQRSTUVWXYZßÖÄÜöäü0123456789";

   toUpper(pattern);
   removeWord(pattern, " TEIL ");
   removeWord(pattern, " FOLGE ");
   removeCharsExcept(pattern, notignore);
}

//***************************************************************************
// Get Range Parts of String like '33-123' or '-123' or '21-'
//***************************************************************************

int rangeFrom(const char* s)
{
   return atoi(s);
}

int rangeTo(const char* s)
{
   int to = INT_MAX;

   const char* p = strchr(s, '-');

   if (p && *(p+1))
      to = atoi(p+1);

   return to;
}

//***************************************************************************
// Left Trim
//***************************************************************************

char* lTrim(char* buf)
{
   if (buf)
   {
      char *tp = buf;

      while (*tp && strchr("\n\r\t ",*tp))
         tp++;

      memmove(buf, tp, strlen(tp) +1);
   }

   return buf;
}

//*************************************************************************
// Right Trim
//*************************************************************************

char* rTrim(char* buf)
{
   if (buf)
   {
      char *tp = buf + strlen(buf);

      while (tp >= buf && strchr("\n\r\t ",*tp))
         tp--;

      *(tp+1) = 0;
   }

   return buf;
}

//*************************************************************************
// All Trim
//*************************************************************************

char* allTrim(char* buf)
{
   return lTrim(rTrim(buf));
}

std::string& allTrim(std::string& str)
{
   return lTrim(rTrim(str));
}

std::string& lTrim(std::string& str)
{
   auto it2 = std::find_if(str.begin(), str.end(), [](char ch){ return !std::isspace<char>(ch, std::locale::classic());} );
   str.erase(str.begin(), it2);
   return str;
}

std::string& rTrim(std::string& str)
{
   auto it1 = std::find_if(str.rbegin(), str.rend(), [](char ch) { return !std::isspace<char>(ch , std::locale::classic()); });
   str.erase(it1.base(), str.end());
   return str;
}

std::string strReplace(const std::string& what, const std::string& with, const std::string& subject)
{
   std::string str = subject;
   size_t pos = 0;

   while ((pos = str.find(what, pos)) != std::string::npos)
   {
      str.replace(pos, what.length(), with);
      pos += with.length();
   }

   return str;
}

std::string strReplace(const std::string& what, long with, const std::string& subject)
{
   char swith[100];

   sprintf(swith, "%ld", with);

   return strReplace(what, swith, subject);
}

std::string strReplace(const std::string& what, double with, const std::string& subject)
{
   char swith[100];

   sprintf(swith, "%.2f", with);

   return strReplace(what, swith, subject);
}

char* strReplace(char* buffer, char from, char to)
{
   char* p = buffer;

   while (*p)
   {
      if (*p == from)
         *p = to;

      p++;
   }

   return buffer;
}

//***************************************************************************
// Number to String
//***************************************************************************

std::string num2Str(int num)
{
   char txt[16];

   snprintf(txt, sizeof(txt), "%d", num);

   return std::string(txt);
}

//***************************************************************************
// Long to Pretty Time
//***************************************************************************

std::string l2pTime(time_t t, const char* format)
{
   char txt[100];
   tm* tmp = localtime(&t);

   strftime(txt, sizeof(txt), format, tmp);

   return std::string(txt);
}

std::string l2pDate(time_t t)
{
   char txt[30];
   tm* tmp = localtime(&t);

   strftime(txt, sizeof(txt), "%d.%m.%Y", tmp);

   return std::string(txt);
}

//***************************************************************************
// To HTTP Header Date Format
//***************************************************************************

std::string l2HttpTime(time_t t)
{
   char date[128+TB];
   tm now;

   static const char *const days[] =
      { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };

   static const char *const mons[] =
      { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

   gmtime_r(&t, &now);

   sprintf(date, "%3s, %02u %3s %04u %02u:%02u:%02u GMT",
           days[now.tm_wday % 7],
           (unsigned int)now.tm_mday,
           mons[now.tm_mon % 12],
           (unsigned int)(1900 + now.tm_year),
           (unsigned int)now.tm_hour,
           (unsigned int)now.tm_min,
           (unsigned int)now.tm_sec);

   return std::string(date);
}

//***************************************************************************
// Is Daylight Saving Time
//***************************************************************************

int isDST(time_t t)
{
   struct tm tm;

   if (!t)
      t = time(0);

   localtime_r(&t, &tm);
   tm.tm_isdst = -1;  // force DST auto detect
   mktime(&tm);

   return tm.tm_isdst;
}

//***************************************************************************
// Time of
//***************************************************************************

time_t timeOf(time_t t)
{
   struct tm tm;

   localtime_r(&t, &tm);

   tm.tm_year = 0;
   tm.tm_mon = 0;
   tm.tm_mday = 0;
   tm.tm_wday = 0;
   tm.tm_yday = 0;
   tm.tm_isdst = -1;  // force DST auto detect

   return mktime(&tm);
}

//***************************************************************************
// Weekday Of
//***************************************************************************

int weekdayOf(time_t t)
{
  struct tm tm_r;
  int weekday = localtime_r(&t, &tm_r)->tm_wday;

  return weekday == 0 ? 6 : weekday - 1;    //  Monday is 0
}

//***************************************************************************
// To Weekday Name
//***************************************************************************

const char* toWeekdayName(uint day)
{
   const char* dayNames[] =
   {
      "Monday",
      "Tuesday",
      "Wednesday",
      "Thursday",
      "Friday",
      "Saturday",
      "Sunday",
      0
   };

   if (day > 6)
      return "<unknown>";

   return dayNames[day];
}

//***************************************************************************
// hhmm of
//***************************************************************************

time_t hhmmOf(time_t t)
{
   struct tm tm;

   localtime_r(&t, &tm);

   tm.tm_year = 0;
   tm.tm_mon = 0;
   tm.tm_mday = 0;
   tm.tm_wday = 0;
   tm.tm_yday = 0;
   tm.tm_sec = 0;
   tm.tm_isdst = -1;  // force DST auto detect

   return mktime(&tm);
}

//***************************************************************************
// time_t to hhmm like '2015'
//***************************************************************************

int l2hhmm(time_t t)
{
   struct tm tm;

   localtime_r(&t, &tm);

   return  tm.tm_hour * 100 + tm.tm_min;
}

//***************************************************************************
// HHMM to Pretty Time
//***************************************************************************

std::string hhmm2pTime(int hhmm)
{
   char txt[100];

   sprintf(txt, "%02d:%02d", hhmm / 100, hhmm % 100);

   return std::string(txt);
}

//***************************************************************************
// Midnight of
//***************************************************************************

time_t midnightOf(time_t t)
{
   struct tm tm;

   localtime_r(&t, &tm);

   tm.tm_hour = 0;
   tm.tm_min = 0;
   tm.tm_sec = 0;
   tm.tm_isdst = -1;  // force DST auto detect

   return mktime(&tm);
}

//***************************************************************************
// MS to Duration
//***************************************************************************

std::string ms2Dur(uint64_t t)
{
   char txt[30];

   int s = t / 1000;
   int ms = t % 1000;

   if (s != 0)
      snprintf(txt, sizeof(txt), "%d.%03d seconds", s, ms);
   else
      snprintf(txt, sizeof(txt), "%d ms", ms);

   return std::string(txt);
}

//***************************************************************************
// Char to Char-String
//***************************************************************************

const char* c2s(char c, char* buf)
{
   sprintf(buf, "%c", c);

   return buf;
}

//***************************************************************************
// End Of String
//***************************************************************************

char* eos(char* s)
{
   if (!s)
      return 0;

   return s + strlen(s);
}

//***************************************************************************
// Store To File
//***************************************************************************

int storeToFile(const char* filename, const char* data, int size)
{
   FILE* fout;

   if ((fout = fopen(filename, "w+")))
   {
      fwrite(data, sizeof(char), size, fout);
      fclose(fout);
   }
   else
   {
      tell(0, "Error, can't store '%s' to filesystem '%s'", filename, strerror(errno));
      return fail;
   }

   return success;
}

//***************************************************************************
// Load From File
//***************************************************************************

int loadFromFile(const char* infile, MemoryStruct* data)
{
   FILE* fin;
   struct stat sb;

   data->clear();

   if (!fileExists(infile))
   {
      tell(0, "File '%s' not found'", infile);
      return fail;
   }

   if (stat(infile, &sb) < 0)
   {
      tell(0, "Can't get info of '%s', error was '%s'", infile, strerror(errno));
      return fail;
   }

   if ((fin = fopen(infile, "r")))
   {
      const char* sfx = suffixOf(infile);

      data->size = sb.st_size;
      data->modTime = sb.st_mtime;
      data->memory = (char*)malloc(data->size);
      fread(data->memory, sizeof(char), data->size, fin);
      fclose(fin);
      sprintf(data->tag, "%ld", (long int)data->size);

      if (strcmp(sfx, "gz") == 0)
         sprintf(data->contentEncoding, "gzip");

      if (strcmp(sfx, "js") == 0)
         sprintf(data->contentType, "application/javascript");

      else if (strcmp(sfx, "png") == 0 || strcmp(sfx, "jpg") == 0 || strcmp(sfx, "gif") == 0)
         sprintf(data->contentType, "image/%s", sfx);
      else if (strcmp(sfx, "svg") == 0)
         sprintf(data->contentType, "image/%s+xml", sfx);
      else if (strcmp(sfx, "ico") == 0)
         strcpy(data->contentType, "image/x-icon");

      else
         sprintf(data->contentType, "text/%s", sfx);
   }
   else
   {
      tell(0, "Error, can't open '%s' for reading, error was '%s'", infile, strerror(errno));
      return fail;
   }

   return success;
}

//***************************************************************************
// TOOLS
//***************************************************************************

int isEmpty(const char* str)
{
   return !str || !*str;
}

const char* notNull(const char* str, const char* def)
{
   if (!str)
      return def;

   return str;
}

int isZero(const char* str)
{
   const char* p = str;

   while (p && *p)
   {
      if (*p != '0')
         return no;

      p++;
   }

   return yes;
}

//***************************************************************************
// Is Member
//***************************************************************************

int isMember(const char** list, const char* item)
{
   const char** p;
   int i;

   for (i = 0, p = list; *p; i++, p++)
      if (strcmp(*p, item) == 0)
         return i;

   return na;
}

//***************************************************************************
// Save String Copy
//***************************************************************************

char* sstrcpy(char* dest, const char* src, int max)
{
   if (!dest || !src)
      return 0;

   strncpy(dest, src, max);
   dest[max-1] = 0;

   return dest;
}

//***************************************************************************
// Save String Duplicate
//***************************************************************************

char* sstrdup(const char* str)
{
   if (!str)
      return 0;

   return strdup(str);
}

int isLink(const char* path)
{
   struct stat sb;

   if (lstat(path, &sb) == 0)
      return S_ISLNK(sb.st_mode);

   tell(0, "Error: Detecting state for '%s' failed, error was '%s'", path, strerror(errno));

   return false;
}

const char* suffixOf(const char* path)
{
   const char* p;

   if (path && (p = strrchr(path, '.')))
      return p+1;

   return "";
}

int fileSize(const char* path)
{
   struct stat sb;

   if (lstat(path, &sb) == 0)
      return sb.st_size;

   tell(0, "Error: Detecting state for '%s' failed, error was '%s'", path, strerror(errno));

   return 0;
}

time_t fileModTime(const char* path)
{
   struct stat sb;

   if (lstat(path, &sb) == 0)
      return sb.st_mtime;

   tell(0, "Error: Detecting state for '%s' failed, error was '%s'", path, strerror(errno));

   return 0;
}

int folderExists(const char* path)
{
   struct stat fs;

   return stat(path, &fs) == 0 && S_ISDIR(fs.st_mode);
}

int fileExists(const char* path)
{
   return access(path, F_OK) == 0;
}

//***************************************************************************
// Get Directory Type
//***************************************************************************

unsigned char dirTypeOf(const char* path, dirent* d)
{
   struct stat fileStat;
   char* fullPath = 0;
   unsigned char type = DT_UNKNOWN;

   if (d->d_type != DT_UNKNOWN)
      return d->d_type;

   asprintf(&fullPath, "%s/%s", path, d->d_name);

   if (stat(fullPath, &fileStat) == 0)
   {
      // check types

      if (S_ISBLK(fileStat.st_mode))        /* block special */
         type = DT_BLK;

      else if (S_ISCHR(fileStat.st_mode))   /* char special */
         type = DT_CHR;

      else if (S_ISDIR(fileStat.st_mode))   /* directory */
         type = DT_DIR;

      else if (S_ISFIFO(fileStat.st_mode))  /* fifo */
         type = DT_FIFO;

      else if (S_ISLNK(fileStat.st_mode))   /* symbolic link */
         type = DT_LNK;

      else if (S_ISREG(fileStat.st_mode))   /* regular file */
         type = DT_REG;

      else if (S_ISSOCK(fileStat.st_mode))  /* socket */
         type = DT_SOCK;
   }

   free(fullPath);

   return type;
}

int createLink(const char* link, const char* dest, int force)
{
   if (!fileExists(link) || force)
   {
      // may be the link exists and point to a wrong or already deleted destination ...
      //   .. therefore we delete the link at first

      unlink(link);

      if (symlink(dest, link) != 0)
      {
         tell(0, "Failed to create symlink '%s', error was '%s'", link, strerror(errno));
         return fail;
      }
   }

   return success;
}

//***************************************************************************
// Remove File
//***************************************************************************

int removeFile(const char* filename)
{
   int lnk = isLink(filename);

   if (unlink(filename) != 0)
   {
      tell(0, "Can't remove file '%s', '%s'", filename, strerror(errno));

      return 1;
   }

   tell(3, "Removed %s '%s'", lnk ? "link" : "file", filename);

   return 0;
}

//***************************************************************************
// Check Dir
//***************************************************************************

int chkDir(const char* path)
{
   struct stat fs;

   if (stat(path, &fs) != 0 || !S_ISDIR(fs.st_mode))
   {
      tell(0, "Creating directory '%s'", path);

      if (mkdir(path, ACCESSPERMS) == -1)
      {
         tell(0, "Can't create directory '%s'", strerror(errno));
         return fail;
      }
   }

   return success;
}

//***************************************************************************
// Get File List of Path
//***************************************************************************

bool compareFileInfo(const FileInfo& firstFi, const FileInfo& secondFi)
{
  unsigned int i = 0;

  while ((i < firstFi.name.length()) && (i < secondFi.name.length()))
  {
    if (tolower(firstFi.name[i]) < tolower(secondFi.name[i]))
        return true;
    else if (tolower(firstFi.name[i]) > tolower(secondFi.name[i]))
        return false;

    i++;
  }
  return firstFi.name.length() < firstFi.name.length();
}

int getFileList(const char* path, int type, const char* extensions, int recursion, FileList* dirs, int& count)
{
   DIR* dir;

   if (!(dir = opendir(path)))
   {
      tell(1, "Can't open directory '%s', '%s'", path, strerror(errno));
      return fail;
   }

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
      // if 'recursion' is set scan subfolders also

      if (recursion && pEntry->d_type == DT_DIR && pEntry->d_name[0] != '.')
      {
         char* buf;
         asprintf(&buf, "%s/%s", path, pEntry->d_name);
         getFileList(buf, type, extensions, recursion, dirs, count);
         free(buf);
      }

      // filter type and ignore '.', '..' an hidden files

      if (pEntry->d_type != type || pEntry->d_name[0] == '.')
         continue;

      // filter file extensions

      if (extensions)
      {
         const char* ext;

         if ((ext = strrchr(pEntry->d_name, '.')))
            ext++;

         if (isEmpty(ext) || !strcasestr(extensions, ext))
         {
            tell(4, "Skipping file '%s' with extension '%s'", pEntry->d_name, ext);
            continue;
         }
      }

      count++;

      if (dirs)
         dirs->push_back({ path, pEntry->d_name, pEntry->d_type });
   }

   closedir(dir);

   return success;
}

#ifdef USELIBXML

//***************************************************************************
// Load XSLT
//***************************************************************************

xsltStylesheetPtr loadXSLT(const char* name, const char* path, int utf8)
{
   xsltStylesheetPtr stylesheet;
   char* xsltfile;

   asprintf(&xsltfile, "%s/%s-%s.xsl", path, name, utf8 ? "utf-8" : "iso-8859-1");

   if ((stylesheet = xsltParseStylesheetFile((const xmlChar*)xsltfile)) == 0)
      tell(0, "Error: Can't load xsltfile %s", xsltfile);
   else
      tell(0, "Info: Stylesheet '%s' loaded", xsltfile);

   free(xsltfile);
   return stylesheet;
}
#endif

#ifdef USEGUNZIP

//***************************************************************************
// Gnu Unzip
//***************************************************************************

int gunzip(MemoryStruct* zippedData, MemoryStruct* unzippedData)
{
   const int growthStep = 1024;

   z_stream stream = {0,0,0,0,0,0,0,0,0,0,0,Z_NULL,Z_NULL,Z_NULL};
   unsigned int resultSize = 0;
   int res = 0;

   unzippedData->clear();

   // determining the size in this way is taken from the sources of the gzip utility.

   memcpy(&unzippedData->size, zippedData->memory + zippedData->size -4, 4);
   unzippedData->memory = (char*)malloc(unzippedData->size);

   // zlib initialisation

   stream.avail_in  = zippedData->size;
   stream.next_in   = (Bytef*)zippedData->memory;
   stream.avail_out = unzippedData->size;
   stream.next_out  = (Bytef*)unzippedData->memory;

   // The '+ 32' tells zlib to process zlib&gzlib headers

   res = inflateInit2(&stream, MAX_WBITS + 32);

   if (res != Z_OK)
   {
      tellZipError(res, " during zlib initialisation", stream.msg);
      inflateEnd(&stream);
      return fail;
   }

   // skip the header

   res = inflate(&stream, Z_BLOCK);

   if (res != Z_OK)
   {
      tellZipError(res, " while skipping the header", stream.msg);
      inflateEnd(&stream);
      return fail;
   }

   while (res == Z_OK)
   {
      if (stream.avail_out == 0)
      {
         unzippedData->size += growthStep;
         unzippedData->memory = (char*)realloc(unzippedData->memory, unzippedData->size);

         // Set the stream pointers to the potentially changed buffer!

         stream.avail_out = resultSize - stream.total_out;
         stream.next_out  = (Bytef*)(unzippedData + stream.total_out);
      }

      res = inflate(&stream, Z_SYNC_FLUSH);
      resultSize = stream.total_out;
   }

   if (res != Z_STREAM_END)
   {
      tellZipError(res, " during inflating", stream.msg);
      inflateEnd(&stream);
      return fail;
   }

   unzippedData->size = resultSize;
   inflateEnd(&stream);

   return success;
}

//***************************************************************************
// gzip
//***************************************************************************

int gzip(Bytef* dest, uLongf* destLen, const Bytef* source, uLong sourceLen)
{
    z_stream stream;
    int res;

    stream.next_in = (Bytef *)source;
    stream.avail_in = (uInt)sourceLen;
    stream.next_out = dest;
    stream.avail_out = (uInt)*destLen;
    if ((uLong)stream.avail_out != *destLen) return Z_BUF_ERROR;

    stream.zalloc = (alloc_func)0;
    stream.zfree = (free_func)0;
    stream.opaque = (voidpf)0;

    res = deflateInit2(&stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 15 + 16, 8, Z_DEFAULT_STRATEGY);

    if (res == Z_OK)
    {
       res = deflate(&stream, Z_FINISH);

       if (res != Z_STREAM_END)
       {
          deflateEnd(&stream);
          res = res == Z_OK ? Z_BUF_ERROR : res;
       }
    }

    if (res == Z_STREAM_END)
    {
       *destLen = stream.total_out;
       res = deflateEnd(&stream);
    }

    if (res !=  Z_OK)
       tellZipError(res, " during compression", "");

    return res == Z_OK ? success : fail;
}

ulong gzipBound(ulong size)
{
   return compressBound(size);
}

//*************************************************************************
// tellZipError
//*************************************************************************

void tellZipError(int errorCode, const char* op, const char* msg)
{
   if (!op)  op  = "";
   if (!msg) msg = "None";

   switch (errorCode)
   {
      case Z_OK:           return;
      case Z_STREAM_END:   return;
      case Z_MEM_ERROR:    tell(0, "Error: Not enough memory to zip/unzip file%s!\n", op); return;
      case Z_BUF_ERROR:    tell(0, "Error: Couldn't zip/unzip data due to output buffer size problem%s!\n", op); return;
      case Z_DATA_ERROR:   tell(0, "Error: Zipped input data corrupted%s! Details: %s\n", op, msg); return;
      case Z_STREAM_ERROR: tell(0, "Error: Invalid stream structure%s. Details: %s\n", op, msg); return;
      default:             tell(0, "Error: Couldn't zip/unzip data for unknown reason (%6d)%s!\n", errorCode, op); return;
   }
}

#endif //  USEGUNZIP

//*************************************************************************
// Host Data
//*************************************************************************

#include <sys/utsname.h>
#include <netdb.h>
#include <ifaddrs.h>

static struct utsname info;

const char* getHostName()
{
   // get info from kernel

   if (uname(&info) == -1)
      return "";

   return info.nodename;
}

const char* getFirstIp(int skipLo)
{
   struct ifaddrs *ifaddr, *ifa;
   static char host[NI_MAXHOST] = "";
   static char netMask[INET6_ADDRSTRLEN] = "";

   if (getifaddrs(&ifaddr) == -1)
   {
      tell(0, "Error: getifaddrs() failed");
      return "";
   }

   // walk through linked interface list

   for (ifa = ifaddr; ifa; ifa = ifa->ifa_next)
   {
      if (!ifa->ifa_addr)
         continue;

      // For an AF_INET interfaces

      if (ifa->ifa_addr->sa_family == AF_INET) //  || ifa->ifa_addr->sa_family == AF_INET6)
      {
         int res = getnameinfo(ifa->ifa_addr,
                               (ifa->ifa_addr->sa_family == AF_INET) ? sizeof(struct sockaddr_in) :
                               sizeof(struct sockaddr_in6),
                               host, NI_MAXHOST, 0, 0, NI_NUMERICHOST);

         if (res)
         {
            tell(0, "Error: getnameinfo() for '%s' failed: %s", gai_strerror(res), ifa->ifa_name);
            continue;
         }

         // skip loopback interface

         if (skipLo && strcmp(host, "127.0.0.1") == 0)
            continue;

         if (ifa->ifa_netmask && ifa->ifa_netmask->sa_family == AF_INET)
         {
            void* p = &((struct sockaddr_in*)ifa->ifa_netmask)->sin_addr;
            inet_ntop(ifa->ifa_netmask->sa_family, p, netMask, sizeof(netMask));
         }

         tell(1, "%-8s %-15s %s; netmask '%s'", ifa->ifa_name, host,
              ifa->ifa_addr->sa_family == AF_INET   ? " (AF_INET)" :
              ifa->ifa_addr->sa_family == AF_INET6  ? " (AF_INET6)" : "",
              netMask);
      }
   }

   freeifaddrs(ifaddr);

   return host;
}

//***************************************************************************
// Get Interfaces
//***************************************************************************

const char* getInterfaces()
{
   static char buffer[1000+TB] = "";
   static char host[NI_MAXHOST] = "";
   struct ifaddrs *ifaddr, *ifa;

   *buffer = 0;

   if (getifaddrs(&ifaddr) == -1)
   {
      tell(0, "Error: getifaddrs() failed");
      return "";
   }

   // walk through linked interface list

   for (ifa = ifaddr; ifa; ifa = ifa->ifa_next)
   {
      if (!ifa->ifa_addr)
         continue;

      // For an AF_INET interfaces

      if (ifa->ifa_addr->sa_family == AF_INET) //  || ifa->ifa_addr->sa_family == AF_INET6)
      {
         int res = getnameinfo(ifa->ifa_addr,
                               (ifa->ifa_addr->sa_family == AF_INET) ? sizeof(struct sockaddr_in) :
                               sizeof(struct sockaddr_in6),
                               host, NI_MAXHOST, 0, 0, NI_NUMERICHOST);

         if (res)
         {
            tell(0, "Error: getnameinfo() failed: %s, skipping interface '%s'", gai_strerror(res), ifa->ifa_name);
            continue;
         }

         sprintf(eos(buffer), "%s:%s ", ifa->ifa_name, host);
      }
   }

   freeifaddrs(ifaddr);

   return buffer;
}

//***************************************************************************
// Get First Interface
//***************************************************************************

const char* getFirstInterface()
{
   static char buffer[1000+TB] = "";
   static char host[NI_MAXHOST] = "";
   struct ifaddrs *ifaddr, *ifa;

   *buffer = 0;

   if (getifaddrs(&ifaddr) == -1)
   {
      tell(0, "Error: getifaddrs() failed");
      return "";
   }

   // walk through linked interface list

   for (ifa = ifaddr; ifa; ifa = ifa->ifa_next)
   {
      if (!ifa->ifa_addr)
         continue;

      // For an AF_INET interfaces

      if (ifa->ifa_addr->sa_family == AF_INET) //  || ifa->ifa_addr->sa_family == AF_INET6)
      {
         int res = getnameinfo(ifa->ifa_addr,
                               (ifa->ifa_addr->sa_family == AF_INET) ? sizeof(struct sockaddr_in) :
                               sizeof(struct sockaddr_in6),
                               host, NI_MAXHOST, 0, 0, NI_NUMERICHOST);

         if (res)
         {
            tell(0, "Error: getnameinfo() failed: %s, skipping interface '%s'", gai_strerror(res), ifa->ifa_name);
            continue;
         }

         if (strcasecmp(ifa->ifa_name, "lo") != 0)
            sprintf(buffer, "%s", ifa->ifa_name);
      }
   }

   freeifaddrs(ifaddr);

   return buffer;
}

//***************************************************************************
// Get IP Of
//***************************************************************************

const char* getIpOf(const char* device)
{
   struct ifreq ifr;
   int fd;

   if (isEmpty(device))
      return getFirstIp();

   fd = socket(AF_INET, SOCK_DGRAM, 0);

   ifr.ifr_addr.sa_family = AF_INET;
   strncpy(ifr.ifr_name, device, IFNAMSIZ-1);

   ioctl(fd, SIOCGIFADDR, &ifr);
   close(fd);

   // caller has to copy the result string 'before' calling again!

   return inet_ntoa(((struct sockaddr_in*)&ifr.ifr_addr)->sin_addr);
}

//***************************************************************************
// Get Mask Of
//***************************************************************************

const char* getMaskOf(const char* device)
{
   struct ifreq ifr;
   static char netMask[INET6_ADDRSTRLEN] = "";
   int fd;

   if (isEmpty(device))
      device = getFirstInterface();

   fd = socket(AF_INET, SOCK_DGRAM, 0);

   ifr.ifr_addr.sa_family = AF_INET;
   strncpy(ifr.ifr_name, device, IFNAMSIZ-1);
   ioctl(fd, SIOCGIFNETMASK, &ifr);
   close(fd);

   if (ifr.ifr_netmask.sa_family == AF_INET)
   {
      void* p = &((struct sockaddr_in*)(&ifr.ifr_netmask))->sin_addr;
      inet_ntop(ifr.ifr_netmask.sa_family, p, netMask, sizeof(netMask));

      tell(5, "netmask for device '%s' is '%s'", device, netMask);
  }

  return netMask;
}

//***************************************************************************
// Broadcast Address Of
//***************************************************************************

const char* bcastAddressOf(const char* ipStr, const char* maskStr)
{
   struct in_addr host, mask, broadcast;
   static char bcastAddress[INET_ADDRSTRLEN] = "";

   if (isEmpty(maskStr))
      maskStr = "255.255.255.0";

   if (inet_pton(AF_INET, ipStr, &host) == 1 && inet_pton(AF_INET, maskStr, &mask) == 1)
   {
      broadcast.s_addr = host.s_addr | ~mask.s_addr;

      if (inet_ntop(AF_INET, &broadcast, bcastAddress, INET_ADDRSTRLEN))
         tell(5, "Bcast for '%s' is '%s'", ipStr, bcastAddress);
      else
         tell(0, "Error: Failed converting number to string");
   }
   else
   {
      tell(0, "Error: Failed converting strings to numbers");
   }

   return bcastAddress;
}

//***************************************************************************
// MAC Address Of
//***************************************************************************

const char* getMacOf(const char* device)
{
   enum { macTuppel = 6 };

   static char mac[20+TB];
   struct ifreq ifr;
   int s;

   s = socket(AF_INET, SOCK_DGRAM, 0);
   strcpy(ifr.ifr_name, device);
   ioctl(s, SIOCGIFHWADDR, &ifr);

   for (int i = 0; i < macTuppel; i++)
      sprintf(&mac[i*3], "%02x:", ((unsigned char*)ifr.ifr_hwaddr.sa_data)[i]);

   mac[17] = 0;

   close(s);

   return mac;
}

//***************************************************************************
// Is Alive  (fork ping therefore no root permissions needed)
//***************************************************************************

int isAlive(const char* ip)
{
   char* cmd;
   int res;

   // ping -W 1 -w 1 -c 1 -q <ip>

   asprintf(&cmd, "%s -W 1 -w 1 -c 1 -q %s > /dev/null 2>&1", "ping", ip);
   res = system(cmd);
   free(cmd);

   if (res == -1 || WEXITSTATUS(res) == 127)
   {
      tell(0, "Can't execute ping, result was (%d) '%d'", res,  WEXITSTATUS(res));
      return na;
   }
   else if (res > 0)
   {
      tell(4, "Host '%s' not alive or not known\n", ip);
      return no;
   }
   else
   {
      tell(4, "Host '%s' is alive!\n", ip);
   }

   return yes;
}

#ifdef USELIBARCHIVE

//***************************************************************************
// unzip <file> and get data of first content which name matches <filter>
//***************************************************************************

int unzip(const char* file, const char* filter, char*& buffer, int& size, char* entryName)
{
   const int step = 1024*10;

   int bufSize = 0;
   int r;
   int res;

   struct archive_entry* entry;
   struct archive* a = archive_read_new();

   *entryName = 0;
   buffer = 0;
   size = 0;

   archive_read_support_filter_all(a);
   archive_read_support_format_all(a);

   r = archive_read_open_filename(a, file, 10204);

   if (r != ARCHIVE_OK)
   {
      tell(0, "Error: Open '%s' failed - %s", file, strerror(errno));
      return 1;
   }

   while (archive_read_next_header(a, &entry) == ARCHIVE_OK)
   {
      strcpy(entryName, archive_entry_pathname(entry));

      if (strstr(entryName, filter))
      {
         bufSize = step;
         buffer = (char*)malloc(bufSize+1);

         while ((res = archive_read_data(a, buffer+size, step)) > 0)
         {
            size += res;
            bufSize += step;

            buffer = (char*)realloc(buffer, bufSize+1);
         }

         buffer[size] = 0;

         break;
      }
   }

   r = archive_read_free(a);

   if (r != ARCHIVE_OK)
   {
      size = 0;
      free(buffer);
      return fail;
   }

   return size > 0 ? success : fail;
}

#endif

//***************************************************************************
// cMyMutex
//***************************************************************************

cMyMutex::cMyMutex (void)
{
  locked = 0;
  pthread_mutexattr_t attr;
  pthread_mutexattr_init(&attr);
  pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK_NP);
  pthread_mutex_init(&mutex, &attr);
}

cMyMutex::~cMyMutex()
{
  pthread_mutex_destroy(&mutex);
}

void cMyMutex::Lock(void)
{
  pthread_mutex_lock(&mutex);
  locked++;
}

void cMyMutex::Unlock(void)
{
 if (!--locked)
    pthread_mutex_unlock(&mutex);
}

//***************************************************************************
// cMyTimeMs
//***************************************************************************

cMyTimeMs::cMyTimeMs(int Ms)
{
  if (Ms >= 0)
     Set(Ms);
  else
     begin = 0;
}

uint64_t cMyTimeMs::Now(void)
{
#define MIN_RESOLUTION 5 // ms
  static bool initialized = false;
  static bool monotonic = false;
  struct timespec tp;
  if (!initialized) {
     // check if monotonic timer is available and provides enough accurate resolution:
     if (clock_getres(CLOCK_MONOTONIC, &tp) == 0) {
        // long Resolution = tp.tv_nsec;
        // require a minimum resolution:
        if (tp.tv_sec == 0 && tp.tv_nsec <= MIN_RESOLUTION * 1000000) {
           if (clock_gettime(CLOCK_MONOTONIC, &tp) == 0) {
              monotonic = true;
              }
           else
              tell(0, "Error: cMyTimeMs: clock_gettime(CLOCK_MONOTONIC) failed");
           }
        else
           tell(0, "Info: cMyTimeMs: not using monotonic clock - resolution is too bad (%ld s %ld ns)", tp.tv_sec, tp.tv_nsec);
        }
     else
        tell(0, "Error: cMyTimeMs: clock_getres(CLOCK_MONOTONIC) failed");
     initialized = true;
     }
  if (monotonic) {
     if (clock_gettime(CLOCK_MONOTONIC, &tp) == 0)
        return (uint64_t(tp.tv_sec)) * 1000 + tp.tv_nsec / 1000000;
     tell(0, "Error: cMyTimeMs: clock_gettime(CLOCK_MONOTONIC) failed");
     monotonic = false;
     // fall back to gettimeofday()
     }
  struct timeval t;
  if (gettimeofday(&t, NULL) == 0)
     return (uint64_t(t.tv_sec)) * 1000 + t.tv_usec / 1000;
  return 0;
}

void cMyTimeMs::Set(int Ms)
{
  begin = Now() + Ms;
}

bool cMyTimeMs::TimedOut(void)
{
  return Now() >= begin;
}

uint64_t cMyTimeMs::Elapsed(void)
{
  return Now() - begin;
}

//**************************************************************************
//  Regular Expression Searching
//**************************************************************************

int rep(const char* string, const char* expression, Option options)
{
  const char* tmpA;
  const char* tmpB;

  return rep(string, expression, tmpA, tmpB, options);
}

int rep(const char* string, const char* expression, const char*& s_location,
        Option options)
{
  const char* tmpA;

  return rep(string, expression, s_location, tmpA, options);
}


int rep(const char* string, const char* expression, const char*& s_location,
        const char*& e_location, Option options)
{
   regex_t reg;
   regmatch_t rm;
   int status;
   int opt = 0;

   // Vorbereiten von reg fuer die Expressionsuche mit regexec
   // Flags:  REG_EXTENDED = Use Extended Regular Expressions
   //         REG_ICASE    = Ignore case in match.

   reg.re_nsub = 0;

   // Options umwandeln
   if (options & repUseRegularExpression)
     opt = opt | REG_EXTENDED;
   if (options & repIgnoreCase)
     opt = opt | REG_ICASE;

   if (regcomp( &reg, expression, opt) != 0)
     return fail;

   // Suchen des ersten Vorkommens von reg in string

   status = regexec(&reg, string, 1, &rm, 0);
   regfree(&reg);

   if (status != 0)
     return fail;

   // Suche erfolgreich =>
   // Setzen der ermittelten Start- und Endpositionen

   s_location = (char*)(string + rm.rm_so);
   e_location = (char*)(string + rm.rm_eo);

   return success;
}

//***************************************************************************
// Class LogDuration
//***************************************************************************

LogDuration::LogDuration(const char* aMessage, int aLogLevel)
{
   logLevel = aLogLevel;
   strcpy(message, aMessage);

   // at last !

   durationStart = cMyTimeMs::Now();
}

LogDuration::~LogDuration()
{
   tell(logLevel, "duration '%s' was (%ldms)",
        message, (long)(cMyTimeMs::Now() - durationStart));
}

void LogDuration::show(const char* label)
{
   tell(logLevel, "elapsed '%s' at '%s' was (%ldms)",
        message, label, (long)(cMyTimeMs::Now() - durationStart));
}

//***************************************************************************
// Get Unique ID
//***************************************************************************

#ifdef USEUUID
const char* getUniqueId()
{
   static char uuid[sizeUuid+TB] = "";

   uuid_t id;
   uuid_generate(id);
   uuid_unparse_upper(id, uuid);

   return uuid;
}
#endif // USEUUID

//***************************************************************************
// Create MD5
//***************************************************************************

#ifdef USEMD5

int createMd5(const char* buf, md5* md5)
{
   MD5_CTX c;
   unsigned char out[MD5_DIGEST_LENGTH];

   MD5_Init(&c);
   MD5_Update(&c, buf, strlen(buf));
   MD5_Final(out, &c);

   for (int n = 0; n < MD5_DIGEST_LENGTH; n++)
      sprintf(md5+2*n, "%02x", out[n]);

   md5[sizeMd5] = 0;

   return done;
}

int createMd5OfFile(const char* path, const char* name, md5* md5)
{
   FILE* f;
   char buffer[1000];
   int nread = 0;
   MD5_CTX c;
   unsigned char out[MD5_DIGEST_LENGTH];
   char* file = 0;

   asprintf(&file, "%s/%s", path, name);

   if (!(f = fopen(file, "r")))
   {
      tell(0, "Fatal: Cannot build MD5 of '%s'; Error was '%s'", file, strerror(errno));
      free(file);
      return fail;
   }

   free(file);

   MD5_Init(&c);

   while ((nread = fread(buffer, 1, 1000, f)) > 0)
      MD5_Update(&c, buffer, nread);

   fclose(f);

   MD5_Final(out, &c);

   for (int n = 0; n < MD5_DIGEST_LENGTH; n++)
      sprintf(md5+2*n, "%02x", out[n]);

   md5[sizeMd5] = 0;

   return success;
}

#endif // USEMD5

//***************************************************************************
// Url Unescape
//***************************************************************************
/*
 * The buffer pointed to by @dst must be at least strlen(@src) bytes.
 * Decoding stops at the first character from @src that decodes to null.

 * Path normalization will remove redundant slashes and slash+dot sequences,
 * as well as removing path components when slash+dot+dot is found. It will
 * keep the root slash (if one was present) and will stop normalization
 * at the first questionmark found (so query parameters won't be normalized).
 *
 * @param dst       destination buffer
 * @param src       source buffer
 * @param normalize perform path normalization if nonzero
 * @return          number of valid characters in @dst
 */

int urlUnescape(char* dst, const char* src, int normalize)
{
//    CURL* curl;
//    int resultSize;

//    if (curl_global_init(CURL_GLOBAL_ALL) != 0)
//    {
//       tell(0, "Error, something went wrong with curl_global_init()");

//       return fail;
//    }

//    curl = curl_easy_init();

//    if (!curl)
//    {
//       tell(0, "Error, unable to get handle from curl_easy_init()");

//       return fail;
//    }

//    dst = curl_easy_unescape(curl, src, strlen(src), &resultSize);

//    tell(0, " [%.40s]", src);

//    tell(0, "res size %d [%.40s]", resultSize, dst);
//    return resultSize;

   char* org_dst = dst;
   int slash_dot_dot = 0;
   char ch, a, b;

   a = 0;

   do {
      ch = *src++;

      if (ch == '%' && isxdigit(a = src[0]) && isxdigit(b = src[1]))
      {
         if (a < 'A')
            a -= '0';
         else if
            (a < 'a') a -= 'A' - 10;
         else
            a -= 'a' - 10;

         if (b < 'A')
            b -= '0';
         else if (b < 'a')
            b -= 'A' - 10;
         else
            b -= 'a' - 10;

         ch = 16 * a + b;
         src += 2;
      }

      if (normalize)
      {
         switch (ch)
         {
            case '/':                         // compress consecutive slashes and remove slash-dot
               if (slash_dot_dot < 3)
               {

                  dst -= slash_dot_dot;
                  slash_dot_dot = 1;
                  break;
               }
               // fall-through

            case '?':                         // at start of query, stop normalizing
               if (ch == '?')
                  normalize = 0;

               // fall-through

            case '\0':                        // remove trailing slash-dot-(dot)
               if (slash_dot_dot > 1)
               {
                  dst -= slash_dot_dot;

                  // remove parent directory if it was two dots

                  if (slash_dot_dot == 3)
                     while (dst > org_dst && *--dst != '/')
                        ; //  empty body
                  slash_dot_dot = (ch == '/') ? 1 : 0;

                  // keep the root slash if any

                  if (!slash_dot_dot && dst == org_dst && *dst == '/')
                     ++dst;

               }
               break;

            case '.':
               if (slash_dot_dot == 1 || slash_dot_dot == 2)
               {
                  ++slash_dot_dot;
                  break;
               }
               // fall-through

            default:
               slash_dot_dot = 0;
         }
      }

      *dst++ = ch;
   } while(ch);

   return (dst - org_dst) - 1;
}
