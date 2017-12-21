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

#include "xml.h"

//***************************************************************************
// ctor
//***************************************************************************

cXml::cXml()
   : printer(0, yes)
{
   root = 0;
}

int cXml::create(const char* rootName)
{
   root = doc.NewElement(rootName);
   doc.InsertFirstChild(root);

   return success;
}

int cXml::set(const char* data)
{
   if (doc.Parse(data) != XML_SUCCESS)
      return fail;

   root = doc.FirstChild();

   return root ? success : fail;
}

//***************************************************************************
// Append Element
//***************************************************************************

XMLNode* cXml::appendElement(const char* name, const char* value, XMLNode* node)
{
   XMLElement* element = doc.NewElement(name);

   element->SetText(value);
   node = node ? node : root;
   node->InsertEndChild(element);

   return element;
}

XMLNode* cXml::appendElement(const char* name, int value, XMLNode* node)
{
   XMLElement* element = doc.NewElement(name);

   element->SetText(value);
   node = node ? node : root;
   node->InsertEndChild(element);

   return element;
}

//***************************************************************************
// To Text
//***************************************************************************

const char* cXml::toText()
{
   doc.Accept(&printer);

   return printer.CStr();
}

XMLElement* cXml::getFirst(XMLNode* node)
{
   node = node ? node : root;
   return node->FirstChildElement();
}

XMLElement* cXml::getNext(XMLNode* node)
{
   return node->NextSiblingElement();
}


XMLElement* cXml::getElementByName(const char* name, XMLElement* element)
{
   if (element)
      return element->FirstChildElement(name);

   return root->FirstChildElement(name);
}
