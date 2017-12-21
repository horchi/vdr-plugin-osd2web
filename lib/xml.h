/*
 *
 * xml.h
 *
 * (c) 2017 Jörg Wendel
 *
 * This code is distributed under the terms and conditions of the
 * GNU GENERAL PUBLIC LICENSE. See the file COPYING for details.
 */

//***************************************************************************
// Include
//***************************************************************************

#include <tinyxml2.h>

#include "common.h"

using namespace tinyxml2;

//***************************************************************************
// Class cXml
//***************************************************************************

class cXml
{
   public:

      cXml();

      int create(const char* rootName);
      int set(const char* data);

      XMLNode* appendElement(const char* name, const char* value, XMLNode* node = 0);
      XMLNode* appendElement(const char* name, int value, XMLNode* node = 0);

      XMLNode* getRoot()     { return root; }
      XMLElement* getFirst(XMLNode* node = 0);
      XMLElement* getNext(XMLNode* node);

      XMLElement* getElementByName(const char* name, XMLElement* element = 0);

      const char* toText();

   private:

      XMLDocument doc;
      XMLNode* root;
      XMLPrinter printer;
};
