// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Properties.h>
#include <Exception.h>
#include <Util.h>
#include <Ice/Initialize.h>
#include <Ice/Properties.h>

using namespace std;
using namespace IcePy;

namespace IcePy
{

//
// An iterator allows an Ice.Properties object to be used like this:
//
// for key in myProps:
//     v = myProps.getProperty(key)
//
struct PropertiesIteratorObject
{
    PyObject_HEAD
    vector<string>* keys;
    vector<string>::size_type pos;
};

extern PyTypeObject PropertiesIteratorType; // Forward declaration.

}

#ifdef WIN32
extern "C"
#endif
static PropertiesObject*
propertiesNew(PyObject* /*arg*/)
{
    PropertiesObject* self = PyObject_New(PropertiesObject, &PropertiesType);
    if (self == NULL)
    {
        return NULL;
    }
    self->properties = 0;
    return self;
}

#ifdef WIN32
extern "C"
#endif
static int
propertiesInit(PropertiesObject* self, PyObject* /*args*/, PyObject* /*kwds*/)
{
    self->properties = new Ice::PropertiesPtr;
    *(self->properties) = Ice::createProperties();
    return 0;
}

#ifdef WIN32
extern "C"
#endif
static void
propertiesDealloc(PropertiesObject* self)
{
    delete self->properties;
    PyObject_Del(self);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
propertiesIter(PropertiesObject* self)
{
    assert(self->properties);

    Ice::PropertyDict dict;
    try
    {
        dict = (*self->properties)->getPropertiesForPrefix("");
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return NULL;
    }

    PropertiesIteratorObject* iter = PyObject_New(PropertiesIteratorObject, &PropertiesIteratorType);
    if(iter == NULL)
    {
        return NULL;
    }

    iter->keys = new vector<string>;
    iter->keys->reserve(dict.size());
    for(Ice::PropertyDict::const_iterator p = dict.begin(); p != dict.end(); ++p)
    {
        iter->keys->push_back(p->first);
    }
    iter->pos = 0;

    return (PyObject*)iter;
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
propertiesStr(PropertiesObject* self)
{
    assert(self->properties);

    Ice::PropertyDict dict;
    try
    {
        dict = (*self->properties)->getPropertiesForPrefix("");
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return NULL;
    }

    string str;
    for(Ice::PropertyDict::const_iterator p = dict.begin(); p != dict.end(); ++p)
    {
        if(p != dict.begin())
        {
            str.append("\n");
        }
        str.append(p->first + "=" + p->second);
    }

    return Py_BuildValue("s", str.c_str());
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
propertiesGetProperty(PropertiesObject* self, PyObject* args, PyObject* kwds)
{
    static char* kwlist[] = {"key", "default", NULL};

    char* key;
    char* def = 0;
    if(!PyArg_ParseTupleAndKeywords(args, kwds, "s|s", kwlist, &key, &def))
    {
        return NULL;
    }

    assert(self->properties);
    try
    {
        string defaultValue;
        if(def)
        {
            defaultValue = def;
        }
        string value = (*self->properties)->getPropertyWithDefault(key, defaultValue);
        return Py_BuildValue("s", const_cast<char*>(value.c_str()));
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
propertiesSetProperty(PropertiesObject* self, PyObject* args, PyObject* kwds)
{
    static char* kwlist[] = {"key", "value", NULL};

    char* key;
    char* value;
    if(!PyArg_ParseTupleAndKeywords(args, kwds, "ss", kwlist, &key, &value))
    {
        return NULL;
    }

    assert(self->properties);
    try
    {
        (*self->properties)->setProperty(key, value);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
propertiesGetCommandLineOptions(PropertiesObject* self)
{
    Ice::StringSeq options;
    assert(self->properties);
    try
    {
        options = (*self->properties)->getCommandLineOptions();
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return NULL;
    }

    PyObject* list = PyList_New(0);
    if(list == NULL)
    {
        return false;
    }
    if(!stringSeqToList(options, list))
    {
        return NULL;
    }

    return list;
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
propertiesParseCommandLineOptions(PropertiesObject* self, PyObject* args, PyObject* kwds)
{
    static char* kwlist[] = {"prefix", "options", NULL};

    char* prefix;
    PyObject* options;
    if(!PyArg_ParseTupleAndKeywords(args, kwds, "sO!", kwlist, &prefix, &PyList_Type, &options))
    {
        return NULL;
    }

    Ice::StringSeq seq;
    if(!listToStringSeq(options, seq))
    {
        return NULL;
    }

    assert(self->properties);
    Ice::StringSeq filteredSeq;
    try
    {
        filteredSeq = (*self->properties)->parseCommandLineOptions(prefix, seq);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return NULL;
    }

    PyObject* list = PyList_New(0);
    if(list == NULL)
    {
        return false;
    }
    if(!stringSeqToList(filteredSeq, list))
    {
        return NULL;
    }

    return list;
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
propertiesLoad(PropertiesObject* self, PyObject* args)
{
    char* file;
    if(!PyArg_ParseTuple(args, "s", &file))
    {
        return NULL;
    }

    assert(self->properties);
    try
    {
        (*self->properties)->load(file);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

static PyMethodDef PropertyMethods[] =
{
    { "getProperty", (PyCFunction)propertiesGetProperty, METH_VARARGS | METH_KEYWORDS,
        PyDoc_STR("getProperty(key[, default]) -> string") },
    { "setProperty", (PyCFunction)propertiesSetProperty, METH_VARARGS | METH_KEYWORDS,
        PyDoc_STR("setProperty(key, value) -> None") },
    { "getCommandLineOptions", (PyCFunction)propertiesGetCommandLineOptions, METH_NOARGS,
        PyDoc_STR("getCommandLineOptions() -> list") },
    { "parseCommandLineOptions", (PyCFunction)propertiesParseCommandLineOptions, METH_VARARGS | METH_KEYWORDS,
        PyDoc_STR("parseCommandLineOptions(prefix, options) -> list") },
    { "load", (PyCFunction)propertiesLoad, METH_VARARGS,
        PyDoc_STR("load(file) -> None") },
    { NULL, NULL} /* sentinel */
};

namespace IcePy
{

PyTypeObject PropertiesType =
{
    /* The ob_type field must be initialized in the module init function
     * to be portable to Windows without using C++. */
    PyObject_HEAD_INIT(NULL)
    0,                              /* ob_size */
    "IcePy.Properties",             /* tp_name */
    sizeof(PropertiesObject),       /* tp_basicsize */
    0,                              /* tp_itemsize */
    /* methods */
    (destructor)propertiesDealloc,  /* tp_dealloc */
    0,                              /* tp_print */
    0,                              /* tp_getattr */
    0,                              /* tp_setattr */
    0,                              /* tp_compare */
    0,                              /* tp_repr */
    0,                              /* tp_as_number */
    0,                              /* tp_as_sequence */
    0,                              /* tp_as_mapping */
    0,                              /* tp_hash */
    0,                              /* tp_call */
    (reprfunc)propertiesStr,        /* tp_str */
    0,                              /* tp_getattro */
    0,                              /* tp_setattro */
    0,                              /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,             /* tp_flags */
    0,                              /* tp_doc */
    0,                              /* tp_traverse */
    0,                              /* tp_clear */
    0,                              /* tp_richcompare */
    0,                              /* tp_weaklistoffset */
    (getiterfunc)propertiesIter,    /* tp_iter */
    0,                              /* tp_iternext */
    PropertyMethods,                /* tp_methods */
    0,                              /* tp_members */
    0,                              /* tp_getset */
    0,                              /* tp_base */
    0,                              /* tp_dict */
    0,                              /* tp_descr_get */
    0,                              /* tp_descr_set */
    0,                              /* tp_dictoffset */
    (initproc)propertiesInit,       /* tp_init */
    0,                              /* tp_alloc */
    (newfunc)propertiesNew,         /* tp_new */
    0,                              /* tp_free */
    0,                              /* tp_is_gc */
};

}

#ifdef WIN32
extern "C"
#endif
static void
propertiesIteratorDealloc(PropertiesIteratorObject* self)
{
    delete self->keys;
    PyObject_Del(self);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
propertiesIteratorNext(PropertiesIteratorObject* self)
{
    if(self->pos < self->keys->size())
    {
        PyObject* result = Py_BuildValue("s", (*self->keys)[self->pos].c_str());
        if(result)
        {
            ++self->pos;
        }
        return result;
    }
    return NULL;
}

namespace IcePy
{

PyTypeObject PropertiesIteratorType =
{
    /* The ob_type field must be initialized in the module init function
     * to be portable to Windows without using C++. */
    PyObject_HEAD_INIT(NULL)
    0,                                     /* ob_size */
    "IcePy.PropertiesIterator",            /* tp_name */
    sizeof(PropertiesIteratorObject),      /* tp_basicsize */
    0,                                     /* tp_itemsize */
    /* methods */
    (destructor)propertiesIteratorDealloc, /* tp_dealloc */
    0,                                     /* tp_print */
    0,                                     /* tp_getattr */
    0,                                     /* tp_setattr */
    0,                                     /* tp_compare */
    0,                                     /* tp_repr */
    0,                                     /* tp_as_number */
    0,                                     /* tp_as_sequence */
    0,                                     /* tp_as_mapping */
    0,                                     /* tp_hash */
    0,                                     /* tp_call */
    0,                                     /* tp_str */
    0,                                     /* tp_getattro */
    0,                                     /* tp_setattro */
    0,                                     /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,                    /* tp_flags */
    0,                                     /* tp_doc */
    0,                                     /* tp_traverse */
    0,                                     /* tp_clear */
    0,                                     /* tp_richcompare */
    0,                                     /* tp_weaklistoffset */
    PyObject_SelfIter,                     /* tp_iter */
    (iternextfunc)propertiesIteratorNext,  /* tp_iternext */
    0,                                     /* tp_methods */
    0,                                     /* tp_members */
    0,                                     /* tp_getset */
    0,                                     /* tp_base */
    0,                                     /* tp_dict */
    0,                                     /* tp_descr_get */
    0,                                     /* tp_descr_set */
    0,                                     /* tp_dictoffset */
    0,                                     /* tp_init */
    0,                                     /* tp_alloc */
    0,                                     /* tp_new */
    0,                                     /* tp_free */
    0,                                     /* tp_is_gc */
};

}

bool
IcePy::initProperties(PyObject* module)
{
    if(PyType_Ready(&PropertiesType) < 0)
    {
        return false;
    }
    if(PyModule_AddObject(module, "Properties", (PyObject*)&PropertiesType) < 0)
    {
        return false;
    }
    if(PyType_Ready(&PropertiesIteratorType) < 0)
    {
        return false;
    }
    if(PyModule_AddObject(module, "PropertiesIterator", (PyObject*)&PropertiesIteratorType) < 0)
    {
        return false;
    }
    return true;
}

PyObject*
IcePy::createProperties(const Ice::PropertiesPtr& props)
{
    PropertiesObject* obj = propertiesNew(NULL);
    if(obj != NULL)
    {
        obj->properties = new Ice::PropertiesPtr(props);
    }
    return (PyObject*)obj;
}

Ice::PropertiesPtr
IcePy::getProperties(PyObject* p)
{
    PropertiesObject* obj = (PropertiesObject*)p;
    if(obj->properties)
    {
        return *obj->properties;
    }
    return 0;
}

extern "C"
PyObject*
Ice_createProperties(PyObject* /*self*/)
{
    //
    // Currently the same as "p = Ice.Properties()".
    //
    return PyObject_Call((PyObject*)&PropertiesType, NULL, NULL);
}

extern "C"
PyObject*
Ice_getDefaultProperties(PyObject* /*self*/, PyObject* args)
{
    PyObject* arglist;
    if(!PyArg_ParseTuple(args, "|O!", &PyList_Type, &arglist))
    {
        return NULL;
    }

    Ice::StringSeq seq;
    if(arglist && !listToStringSeq(arglist, seq))
    {
        return NULL;
    }

    Ice::PropertiesPtr defaultProps;
    try
    {
        defaultProps = Ice::getDefaultProperties(seq);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return NULL;
    }

    //
    // Replace the contents of the given argument list with the filtered arguments.
    //
    if(arglist)
    {
        if(PyList_SetSlice(arglist, 0, PyList_Size(arglist), NULL) < 0)
        {
            return NULL;
        }
        if(!stringSeqToList(seq, arglist))
        {
            return NULL;
        }
    }

    return createProperties(defaultProps);
}
