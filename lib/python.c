/*
 * python.c
 *
 * See the README file for copyright information and how to reach the author.
 *
 */

#include "python.h"

cDbTable* Python::globalEventsDb = 0;
int Python::globalNamingMode = 0;
const char* Python::globalTmplExpression = "";

//***************************************************************************
// Static Interface Methods (Table events)
//***************************************************************************

PyObject* Python::eventTitle(PyObject* /*self*/, PyObject* /*args*/)
{
   if (!globalEventsDb)
      return Py_BuildValue("s", "");

   return Py_BuildValue("s", globalEventsDb->getStrValue("TITLE"));
}

PyObject* Python::eventShortText(PyObject* /*self*/, PyObject* /*args*/)
{
   if (!globalEventsDb)
      return Py_BuildValue("s", "");

   return Py_BuildValue("s", globalEventsDb->getStrValue("SHORTTEXT"));
}

PyObject* Python::eventStartTime(PyObject* /*self*/, PyObject* /*args*/)
{
   if (!globalEventsDb)
      return Py_BuildValue("i", 0);

   return Py_BuildValue("i", globalEventsDb->getIntValue("STARTTIME"));
}

PyObject* Python::eventYear(PyObject* /*self*/, PyObject* /*args*/)
{
   if (!globalEventsDb)
      return Py_BuildValue("s", "");

   return Py_BuildValue("s", globalEventsDb->getStrValue("YEAR"));
}

PyObject* Python::eventCategory(PyObject* /*self*/, PyObject* /*args*/)
{
   if (!globalEventsDb)
      return Py_BuildValue("s", "");

   return Py_BuildValue("s", globalEventsDb->getStrValue("CATEGORY"));
}

PyObject* Python::episodeName(PyObject* /*self*/, PyObject* /*args*/)
{
   if (!globalEventsDb)
      return Py_BuildValue("s", "");

   return Py_BuildValue("s", globalEventsDb->getStrValue("EPISODENAME"));
}

PyObject* Python::episodeShortName(PyObject* /*self*/, PyObject* /*args*/)
{
   if (!globalEventsDb)
      return Py_BuildValue("s", "");

   return Py_BuildValue("s", globalEventsDb->getStrValue("EPISODESHORTNAME"));
}

PyObject* Python::episodePartName(PyObject* /*self*/, PyObject* /*args*/)
{
   if (!globalEventsDb)
      return Py_BuildValue("s", "");

   return Py_BuildValue("s", globalEventsDb->getStrValue("EPISODEPARTNAME"));
}

PyObject* Python::extracol1(PyObject* /*self*/, PyObject* /*args*/)
{
   if (!globalEventsDb)
      return Py_BuildValue("s", "");

   return Py_BuildValue("s", globalEventsDb->getStrValue("EPISODEEXTRACOL1"));
}

PyObject* Python::extracol2(PyObject* /*self*/, PyObject* /*args*/)
{
   if (!globalEventsDb)
      return Py_BuildValue("s", "");

   return Py_BuildValue("s", globalEventsDb->getStrValue("EPISODEEXTRACOL2"));
}

PyObject* Python::extracol3(PyObject* /*self*/, PyObject* /*args*/)
{
   if (!globalEventsDb)
      return Py_BuildValue("s", "");

   return Py_BuildValue("s", globalEventsDb->getStrValue("EPISODEEXTRACOL3"));
}

PyObject* Python::episodeSeason(PyObject* /*self*/, PyObject* /*args*/)
{
   if (!globalEventsDb)
      return Py_BuildValue("i", na);

   return Py_BuildValue("i", globalEventsDb->getIntValue("EPISODESEASON"));
}

PyObject* Python::episodePart(PyObject* /*self*/, PyObject* /*args*/)
{
   if (!globalEventsDb)
      return Py_BuildValue("i", na);

   return Py_BuildValue("i", globalEventsDb->getIntValue("EPISODEPART"));
}

PyObject* Python::episodeNumber(PyObject* /*self*/, PyObject* /*args*/)
{
   if (!globalEventsDb)
      return Py_BuildValue("i", na);

   return Py_BuildValue("i", globalEventsDb->getIntValue("EPISODENUMBER"));
}

PyObject* Python::namingMode(PyObject* /*self*/, PyObject* /*args*/)
{
   return Py_BuildValue("i", globalNamingMode);
}

PyObject* Python::tmplExpression(PyObject* /*self*/, PyObject* /*args*/)
{
   return Py_BuildValue("s", globalTmplExpression);
}

//***************************************************************************
// The Methods
//***************************************************************************

PyMethodDef Python::eventMethods[] =
{
   {  "title",           Python::eventTitle,       METH_VARARGS,  "Return the events ..."   },
   {  "shorttext",       Python::eventShortText,   METH_VARARGS,  "Return the events ..."   },
   {  "starttime",       Python::eventStartTime,   METH_VARARGS,  "Return the events ..."   },
   {  "year",            Python::eventYear,        METH_VARARGS,  "Return the events ..."   },
   {  "category",        Python::eventCategory,    METH_VARARGS,  "Return the events ..."   },

   {  "episodname",      Python::episodeName,      METH_VARARGS,  "Return the events ..."   },
   {  "shortname",       Python::episodeShortName, METH_VARARGS,  "Return the events ..."   },
   {  "partname",        Python::episodePartName,  METH_VARARGS,  "Return the events ..."   },

   {  "season",          Python::episodeSeason,    METH_VARARGS,  "Return the events ..."   },
   {  "part",            Python::episodePart,      METH_VARARGS,  "Return the events ..."   },
   {  "number",          Python::episodeNumber,    METH_VARARGS,  "Return the events ..."   },

   {  "extracol1",       Python::extracol1,        METH_VARARGS,  "Return the events ..."   },
   {  "extracol2",       Python::extracol2,        METH_VARARGS,  "Return the events ..."   },
   {  "extracol3",       Python::extracol3,        METH_VARARGS,  "Return the events ..."   },

   {  "namingmode",      Python::namingMode,       METH_VARARGS,  "Return the ..."          },
   {  "tmplExpression",  Python::tmplExpression,   METH_VARARGS,  "Return the ..."          },

   {            0,                       0,                   0,                        0   }
};

#if PY_MAJOR_VERSION >= 3

PyModuleDef Python::moduledef =
{
   PyModuleDef_HEAD_INIT,    // m_base
   "event",                  // m_name     - name of module
   0,                        // m_doc      - module documentation, may be NULL
   -1,                       // m_size     - size of per-interpreter state of the module, or -1 if the module keeps state in global variables.
   eventMethods,             // m_methods
   0,                        // m_slots    - array of slot definitions for multi-phase initialization, terminated by a {0, NULL} entry.
                             //              when using single-phase initialization, m_slots must be NULL.
   0,                        // traverseproc m_traverse - traversal function to call during GC traversal of the module object, or NULL if not needed.
   0,                        // inquiry m_clear         - clear function to call during GC clearing of the module object, or NULL if not needed.
   0                         // freefunc m_free         - function to call during deallocation of the module object or NULL if not needed.
};

#endif

//***************************************************************************
// Object
//***************************************************************************

Python::Python(const char* aFile, const char* aFunction)
{
   pFunc = 0;
   pModule = 0;
   result = 0;
   file = strdup(aFile);
   function = strdup(aFunction);
}

Python::~Python()
{
   exit();

   free(result);
   free(file);
   free(function);
}

//***************************************************************************
// Init / Exit
//***************************************************************************

int Python::init(const char* modulePath)
{
   PyObject* pName;

   tell(0, "Initialize python script '%s/%s.py'", modulePath, file);

   // register event methods

#if PY_MAJOR_VERSION >= 3
   PyImport_AppendInittab("event", &PyInitEvent);
   Py_Initialize();                         // initialize the Python interpreter
   pName = PyUnicode_FromString(file);
#else
   Py_Initialize();                         // initialize the Python interpreter
   Py_InitModule("event", eventMethods);
   pName = PyString_FromString(file);
#endif

   // add search path for Python modules

   if (modulePath)
   {
      char* p;
      asprintf(&p, "sys.path.append(\"%s\")", modulePath);
      PyRun_SimpleString("import sys");
      PyRun_SimpleString(p);
      free(p);
   }

   // import the module

   pModule = PyImport_Import(pName);
   Py_DECREF(pName);

   if (!pModule)
   {
      showError();
      tell(0, "Failed to load '%s.py'", file);

      return fail;
   }

   pFunc = PyObject_GetAttrString(pModule, function);

   // pFunc is a new reference

   if (!pFunc || !PyCallable_Check(pFunc))
   {
      if (PyErr_Occurred())
         showError();

      tell(0, "Cannot find function '%s'", function);

      return fail;
   }

   return success;
}

int Python::exit()
{
   if (pFunc)
      Py_XDECREF(pFunc);

   if (pModule)
      Py_DECREF(pModule);

   Py_Finalize();

   return success;
}

//***************************************************************************
// Execute
//***************************************************************************

int Python::execute(cDbTable* eventsDb, int namingmode, const char* tmplExpression)
{
   PyObject* pValue;

   free(result);
   result = 0;

   globalEventsDb = eventsDb;
   globalNamingMode = namingmode;
   globalTmplExpression = tmplExpression;

   pValue = PyObject_CallObject(pFunc, 0);

   if (!pValue)
   {
      showError();
      tell(0, "Python: Call of function '%s()' failed", function);

      return fail;
   }

#if PY_MAJOR_VERSION >= 3
   // PyObject* strExc = PyObject_Repr(pValue);
   // PyObject* pyStr = PyUnicode_AsEncodedString(strExc, "utf-8", "replace");
   PyObject* pyStr = PyUnicode_AsEncodedString(pValue, "utf-8", "replace");
   result = strdup(PyBytes_AsString(pyStr));
   // Py_XDECREF(strExc);

#else
   result = strdup(PyString_AsString(pValue));
#endif

   tell(3, "Result of call: %s", result);

   Py_DECREF(pValue);

   return success;
}

//***************************************************************************
// Error
//***************************************************************************

void Python::showError()
{
   const char* error = "";
   PyObject *ptype = 0, *pvalue = 0, *ptraceback = 0;

   PyErr_Fetch(&ptype, &pvalue, &ptraceback);

#if PY_MAJOR_VERSION >= 3
   PyObject* strExc = PyObject_Repr(pvalue);    // Now a unicode object
   PyObject* pyStr = PyUnicode_AsEncodedString(strExc, "utf-8", "Error ~");
   error = PyBytes_AsString(pyStr);

   Py_XDECREF(strExc);
   Py_XDECREF(pyStr);
#else
   error =  PyString_AsString(pvalue);
#endif

   tell(0, "Python error was: %s", error);

   Py_DECREF(pvalue);
   Py_DECREF(ptype);
   Py_XDECREF(ptraceback);
}
