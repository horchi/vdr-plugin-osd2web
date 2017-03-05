/*
 * json.h
 *
 * See the README file for copyright information and how to reach the author.
 *
 */

#ifndef __JSON_H
#define __JSON_H

//***************************************************************************
// Include
//***************************************************************************

#ifdef USEJSON

#include <jansson.h>

#include "db.h"

//***************************************************************************
// JSON Helper Functions
//***************************************************************************

int json2Data(json_t* obj, MemoryStruct* data, const char* encoding = 0);

int addFieldToJson(json_t* obj, cDbTable* table, const char* fname, int ignoreEmpty = yes, const char* extName = 0);
int addFieldToJson(json_t* obj, cDbValue* value, int ignoreEmpty = yes, const char* extName = 0);
int getFieldFromJson(json_t* obj, cDbRow* row, const char* fname, const char* extName = 0);

const char* getStringFromJson(json_t* obj, const char* name, const char* def = 0);
int getIntFromJson(json_t* obj, const char* name, int def = na);

int addToJson(json_t* obj, const char* name, const char* value, const char* def = "");
int addToJson(json_t* obj, const char* name, int value);

#endif // USEJSON

#endif // __JSON_H
