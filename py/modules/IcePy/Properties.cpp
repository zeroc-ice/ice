// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifdef _WIN32
#   include <IceUtil/Config.h>
#endif
#include <Properties.h>
#include <Util.h>
#include <Ice/Initialize.h>
#include <Ice/Properties.h>

using namespace std;
using namespace IcePy;

namespace IcePy
{

struct PropertiesObject
{
    PyObject_HEAD
    Ice::PropertiesPtr* properties;
};

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
propertiesInit(PropertiesObject* self, PyObject* args, PyObject* /*kwds*/)
{
    PyObject* arglist = NULL;
    if(!PyArg_ParseTuple(args, "|O!", &PyList_Type, &arglist))
    {
        return -1;
    }

    Ice::StringSeq seq;
    if(arglist && !listToStringSeq(arglist, seq))
    {
        return -1;
    }

    Ice::PropertiesPtr props;
    try
    {
        props = Ice::createProperties(seq);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return -1;
    }

    //
    // Replace the contents of the given argument list with the filtered arguments.
    //
    if(arglist)
    {
        if(PyList_SetSlice(arglist, 0, PyList_Size(arglist), NULL) < 0)
        {
            return -1;
        }
        if(!stringSeqToList(seq, arglist))
        {
            return -1;
        }
    }

    self->properties = new Ice::PropertiesPtr(props);

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

    return PyString_FromString(const_cast<char*>(str.c_str()));
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
propertiesGetProperty(PropertiesObject* self, PyObject* args)
{
    char* key;
    if(!PyArg_ParseTuple(args, "s", &key))
    {
        return NULL;
    }

    assert(self->properties);
    string value;
    try
    {
        value = (*self->properties)->getProperty(key);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return NULL;
    }

    return PyString_FromString(const_cast<char*>(value.c_str()));
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
propertiesGetPropertyWithDefault(PropertiesObject* self, PyObject* args)
{
    char* key;
    char* def;
    if(!PyArg_ParseTuple(args, "ss", &key, &def))
    {
        return NULL;
    }

    assert(self->properties);
    string value;
    try
    {
        value = (*self->properties)->getPropertyWithDefault(key, def);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return NULL;
    }

    return PyString_FromString(const_cast<char*>(value.c_str()));
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
propertiesGetPropertyAsInt(PropertiesObject* self, PyObject* args)
{
    char* key;
    if(!PyArg_ParseTuple(args, "s", &key))
    {
        return NULL;
    }

    assert(self->properties);
    Ice::Int value;
    try
    {
        value = (*self->properties)->getPropertyAsInt(key);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return NULL;
    }

    return PyInt_FromLong(value);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
propertiesGetPropertyAsIntWithDefault(PropertiesObject* self, PyObject* args)
{
    char* key;
    int def;
    if(!PyArg_ParseTuple(args, "si", &key, &def))
    {
        return NULL;
    }

    assert(self->properties);
    Ice::Int value;
    try
    {
        value = (*self->properties)->getPropertyAsIntWithDefault(key, def);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return NULL;
    }

    return PyInt_FromLong(value);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
propertiesGetPropertiesForPrefix(PropertiesObject* self, PyObject* args)
{
    char* prefix;
    if(!PyArg_ParseTuple(args, "s", &prefix))
    {
        return NULL;
    }

    assert(self->properties);
    Ice::PropertyDict dict;
    try
    {
        dict = (*self->properties)->getPropertiesForPrefix(prefix);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return NULL;
    }

    PyObjectHandle result = PyDict_New();
    if(result.get() != NULL)
    {
        for(Ice::PropertyDict::iterator p = dict.begin(); p != dict.end(); ++p)
        {
            PyObjectHandle key = PyString_FromString(const_cast<char*>(p->first.c_str()));
            PyObjectHandle val = PyString_FromString(const_cast<char*>(p->second.c_str()));
            if(val.get() == NULL || PyDict_SetItem(result.get(), key.get(), val.get()) < 0)
            {
                return NULL;
            }
        }
    }

    return result.release();
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
propertiesSetProperty(PropertiesObject* self, PyObject* args)
{
    char* key;
    char* value;
    if(!PyArg_ParseTuple(args, "ss", &key, &value))
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
propertiesParseCommandLineOptions(PropertiesObject* self, PyObject* args)
{
    char* prefix;
    PyObject* options;
    if(!PyArg_ParseTuple(args, "sO!", &prefix, &PyList_Type, &options))
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
propertiesParseIceCommandLineOptions(PropertiesObject* self, PyObject* args)
{
    PyObject* options;
    if(!PyArg_ParseTuple(args, "O!", &PyList_Type, &options))
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
        filteredSeq = (*self->properties)->parseIceCommandLineOptions(seq);
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

#ifdef WIN32
extern "C"
#endif
static PyObject*
propertiesClone(PropertiesObject* self)
{
    Ice::PropertiesPtr properties;
    assert(self->properties);
    try
    {
        properties = (*self->properties)->clone();
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return NULL;
    }

    return createProperties(properties);
}

static PyMethodDef PropertyMethods[] =
{
    { "getProperty", (PyCFunction)propertiesGetProperty, METH_VARARGS,
        PyDoc_STR("getProperty(key) -> string") },
    { "getPropertyWithDefault", (PyCFunction)propertiesGetPropertyWithDefault, METH_VARARGS,
        PyDoc_STR("getPropertyWithDefault(key, default) -> string") },
    { "getPropertyAsInt", (PyCFunction)propertiesGetPropertyAsInt, METH_VARARGS,
        PyDoc_STR("getPropertyAsInt(key) -> int") },
    { "getPropertyAsIntWithDefault", (PyCFunction)propertiesGetPropertyAsIntWithDefault, METH_VARARGS,
        PyDoc_STR("getPropertyAsIntWithDefault(key, default) -> int") },
    { "getPropertiesForPrefix", (PyCFunction)propertiesGetPropertiesForPrefix, METH_VARARGS,
        PyDoc_STR("getPropertiesForPrefix(prefix) -> dict") },
    { "setProperty", (PyCFunction)propertiesSetProperty, METH_VARARGS,
        PyDoc_STR("setProperty(key, value) -> None") },
    { "getCommandLineOptions", (PyCFunction)propertiesGetCommandLineOptions, METH_NOARGS,
        PyDoc_STR("getCommandLineOptions() -> list") },
    { "parseCommandLineOptions", (PyCFunction)propertiesParseCommandLineOptions, METH_VARARGS,
        PyDoc_STR("parseCommandLineOptions(prefix, options) -> list") },
    { "parseIceCommandLineOptions", (PyCFunction)propertiesParseIceCommandLineOptions, METH_VARARGS,
        PyDoc_STR("parseIceCommandLineOptions(prefix, options) -> list") },
    { "load", (PyCFunction)propertiesLoad, METH_VARARGS,
        PyDoc_STR("load(file) -> None") },
    { "clone", (PyCFunction)propertiesClone, METH_NOARGS,
        PyDoc_STR("clone() -> Ice.Properties") },
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
    0,                              /* tp_iter */
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
IcePy_createProperties(PyObject* /*self*/, PyObject* args)
{
    //
    // Currently the same as "p = Ice.Properties()".
    //
    return PyObject_Call((PyObject*)&PropertiesType, args, NULL);
}

extern "C"
PyObject*
IcePy_getDefaultProperties(PyObject* /*self*/, PyObject* args)
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
