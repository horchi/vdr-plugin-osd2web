/*
 * scan.h: A plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *  (c) 2007-2013 Jörg Wendel
 *
 * This code is distributed under the terms and conditions of the
 * GNU GENERAL PUBLIC LICENSE. See the file COPYING for details.
 *
 */

//***************************************************************************
// Includes
//***************************************************************************

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "lib/common.h"

//***************************************************************************
// Token Scanner
//***************************************************************************

class Scan
{
   public:

      Scan(const char* buf, int np = yes)  { asprintf(&buffer, "%s", buf); reset(); allowNumPrefix = np; }
      ~Scan()                     { free(buffer); }

      void reset()                { p = buffer; isStr = no; }
      const char* all()           { return buffer; }
      const char* lastString()    { return _last; }
      const char* lastIdent()     { return _last; }
      int lastInt()               { return atoi(_last); }
      int isEmpty()               { return _last[0] == 0; }
      int isString()              { return isStr; }
      int isIdent()               { return !isStr && isIdent(*_last); }
      int isNum()                 { return !isStr && isNum(*_last); }
      int isLogical()             { return !isStr && isLogical(*_last); }
      int isOperator()            { return !isStr && isOperator(*_last); }
      int hasValue(const char* v) { return strcmp(notNull(v), notNull(_last)) == 0; }

      const char* next()          { return p; }

      int eat(int aANP = na);

   private:

      void skipWs()            { while (*p && isWhiteSpace(*p)) p++; }

      int isDelimiter(char c)  { return strchr(delimiters, c) != 0; }
      int isWhiteSpace(char c) { return strchr(whitespace, c) != 0; }
      int isIdent(char c)      { return isalpha(c); }
      int isNum(char c)        { return isdigit(c) || isNumPrefix(c); }
      int isOperator(char c)   { return strchr(operators, c) != 0; }
      int isLogical(char c)    { return strchr(logicalOps, c) != 0; }
      int isNumPrefix(char c)  { return strchr(numprefix, c) != 0; }

      // data

      int isStr;
      const char* p;
      char* buffer;
      char _last[1000+TB];
      int allowNumPrefix;

      static const char* delimiters;
      static const char* operators;
      static const char* logicalOps;
      static const char* whitespace;
      static const char* numprefix;
};
