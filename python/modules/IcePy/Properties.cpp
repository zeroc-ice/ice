// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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
propertiesNew(PyTypeObject* type, PyObject* /*args*/, PyObject* /*kwds*/)
{
    PropertiesObject* self = reinterpret_cast<PropertiesObject*>(type->tp_alloc(type, 0));
    if(!self)
    {
        return 0;
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
    PyObject* arglist = 0;
    PyObject* defaultsObj = 0;

    if(!PyArg_ParseTuple(args, STRCAST("|OO"), &arglist, &defaultsObj))
    {
        return -1;
    }

    Ice::StringSeq seq;
    if(arglist)
    {
        PyTypeObject* listType = &PyList_Type; // Necessary to prevent GCC's strict-alias warnings.
        if(PyObject_IsInstance(arglist, reinterpret_cast<PyObject*>(listType)))
        {
            if(!listToStringSeq(arglist, seq))
            {
                return -1;
            }
        }
        else if(arglist != Py_None)
        {
            PyErr_Format(PyExc_ValueError, STRCAST("args must be None or a list"));
            return -1;
        }
    }

    Ice::PropertiesPtr defaults;
    if(defaultsObj)
    {
        PyObject* propType = lookupType("Ice.PropertiesI");
        assert(propType);
        if(PyObject_IsInstance(defaultsObj, propType))
        {
            PyObjectHandle impl = PyObject_GetAttrString(defaultsObj, STRCAST("_impl"));
            defaults = getProperties(impl.get());
        }
        else if(defaultsObj != Py_None)
        {
            PyErr_Format(PyExc_ValueError, STRCAST("defaults must be None or a Ice.Properties"));
            return -1;
        }
    }

    Ice::PropertiesPtr props;
    try
    {
        if(defaults || (arglist && arglist != Py_None))
        {
            props = Ice::createProperties(seq, defaults);
        }
        else
        {
            props = Ice::createProperties();
        }
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return -1;
    }

    //
    // Replace the contents of the given argument list with the filtered arguments.
    //
    if(arglist && arglist != Py_None)
    {
        if(PyList_SetSlice(arglist, 0, PyList_Size(arglist), 0) < 0)
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
    Py_TYPE(self)->tp_free(reinterpret_cast<PyObject*>(self));
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
        return 0;
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

    return createString(str);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
propertiesGetProperty(PropertiesObject* self, PyObject* args)
{
    PyObject* keyObj;
    if(!PyArg_ParseTuple(args, STRCAST("O"), &keyObj))
    {
        return 0;
    }

    string key;
    if(!getStringArg(keyObj, "key", key))
    {
        return 0;
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
        return 0;
    }

    return createString(value);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
propertiesGetPropertyWithDefault(PropertiesObject* self, PyObject* args)
{
    PyObject* keyObj;
    PyObject* defObj;
    if(!PyArg_ParseTuple(args, STRCAST("OO"), &keyObj, &defObj))
    {
        return 0;
    }

    string key;
    string def;
    if(!getStringArg(keyObj, "key", key))
    {
        return 0;
    }
    if(!getStringArg(defObj, "value", def))
    {
        return 0;
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
        return 0;
    }

    return createString(value);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
propertiesGetPropertyAsInt(PropertiesObject* self, PyObject* args)
{
    PyObject* keyObj;
    if(!PyArg_ParseTuple(args, STRCAST("O"), &keyObj))
    {
        return 0;
    }

    string key;
    if(!getStringArg(keyObj, "key", key))
    {
        return 0;
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
        return 0;
    }

    return PyLong_FromLong(value);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
propertiesGetPropertyAsIntWithDefault(PropertiesObject* self, PyObject* args)
{
    PyObject* keyObj;
    int def;
    if(!PyArg_ParseTuple(args, STRCAST("Oi"), &keyObj, &def))
    {
        return 0;
    }

    string key;
    if(!getStringArg(keyObj, "key", key))
    {
        return 0;
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
        return 0;
    }

    return PyLong_FromLong(value);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
propertiesGetPropertyAsList(PropertiesObject* self, PyObject* args)
{
    PyObject* keyObj;
    if(!PyArg_ParseTuple(args, STRCAST("O"), &keyObj))
    {
        return 0;
    }

    string key;
    if(!getStringArg(keyObj, "key", key))
    {
        return 0;
    }

    assert(self->properties);
    Ice::StringSeq value;
    try
    {
        value = (*self->properties)->getPropertyAsList(key);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    PyObject* list = PyList_New(0);
    if(!list)
    {
        return 0;
    }
    if(!stringSeqToList(value, list))
    {
        return 0;
    }

    return list;
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
propertiesGetPropertyAsListWithDefault(PropertiesObject* self, PyObject* args)
{
    PyObject* keyObj;
    PyObject* defList;
    if(!PyArg_ParseTuple(args, STRCAST("OO!"), &keyObj, &PyList_Type, &defList))
    {
        return 0;
    }

    string key;
    if(!getStringArg(keyObj, "key", key))
    {
        return 0;
    }

    assert(self->properties);
    Ice::StringSeq def;
    if(!listToStringSeq(defList, def))
    {
        return 0;
    }

    Ice::StringSeq value;
    try
    {
        value = (*self->properties)->getPropertyAsListWithDefault(key, def);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    PyObject* list = PyList_New(0);
    if(!list)
    {
        return 0;
    }
    if(!stringSeqToList(value, list))
    {
        return 0;
    }

    return list;
}



#ifdef WIN32
extern "C"
#endif
static PyObject*
propertiesGetPropertiesForPrefix(PropertiesObject* self, PyObject* args)
{
    PyObject* prefixObj;
    if(!PyArg_ParseTuple(args, STRCAST("O"), &prefixObj))
    {
        return 0;
    }

    string prefix;
    if(!getStringArg(prefixObj, "prefix", prefix))
    {
        return 0;
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
        return 0;
    }

    PyObjectHandle result = PyDict_New();
    if(result.get())
    {
        for(Ice::PropertyDict::iterator p = dict.begin(); p != dict.end(); ++p)
        {
            PyObjectHandle key = createString(p->first);
            PyObjectHandle val = createString(p->second);
            if(!val.get() || PyDict_SetItem(result.get(), key.get(), val.get()) < 0)
            {
                return 0;
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
    PyObject* keyObj;
    PyObject* valueObj;
    if(!PyArg_ParseTuple(args, STRCAST("OO"), &keyObj, &valueObj))
    {
        return 0;
    }

    string key;
    string value;
    if(!getStringArg(keyObj, "key", key))
    {
        return 0;
    }
    if(!getStringArg(valueObj, "value", value))
    {
        return 0;
    }

    assert(self->properties);
    try
    {
        (*self->properties)->setProperty(key, value);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
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
        return 0;
    }

    PyObject* list = PyList_New(0);
    if(!list)
    {
        return 0;
    }
    if(!stringSeqToList(options, list))
    {
        return 0;
    }

    return list;
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
propertiesParseCommandLineOptions(PropertiesObject* self, PyObject* args)
{
    PyObject* prefixObj;
    PyObject* options;
    if(!PyArg_ParseTuple(args, STRCAST("OO!"), &prefixObj, &PyList_Type, &options))
    {
        return 0;
    }

    Ice::StringSeq seq;
    if(!listToStringSeq(options, seq))
    {
        return 0;
    }

    string prefix;
    if(!getStringArg(prefixObj, "prefix", prefix))
    {
        return 0;
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
        return 0;
    }

    PyObject* list = PyList_New(0);
    if(!list)
    {
        return 0;
    }
    if(!stringSeqToList(filteredSeq, list))
    {
        return 0;
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
    if(!PyArg_ParseTuple(args, STRCAST("O!"), &PyList_Type, &options))
    {
        return 0;
    }

    Ice::StringSeq seq;
    if(!listToStringSeq(options, seq))
    {
        return 0;
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
        return 0;
    }

    PyObject* list = PyList_New(0);
    if(!list)
    {
        return 0;
    }
    if(!stringSeqToList(filteredSeq, list))
    {
        return 0;
    }

    return list;
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
propertiesLoad(PropertiesObject* self, PyObject* args)
{
    PyObject* fileObj;
    if(!PyArg_ParseTuple(args, STRCAST("O"), &fileObj))
    {
        return 0;
    }

    string file;
    if(!getStringArg(fileObj, "file", file))
    {
        return 0;
    }

    assert(self->properties);
    try
    {
        (*self->properties)->load(file);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
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
        return 0;
    }

    return createProperties(properties);
}

static PyMethodDef PropertyMethods[] =
{
    { STRCAST("getProperty"), reinterpret_cast<PyCFunction>(propertiesGetProperty), METH_VARARGS,
        PyDoc_STR(STRCAST("getProperty(key) -> string")) },
    { STRCAST("getPropertyWithDefault"), reinterpret_cast<PyCFunction>(propertiesGetPropertyWithDefault), METH_VARARGS,
        PyDoc_STR(STRCAST("getPropertyWithDefault(key, default) -> string")) },
    { STRCAST("getPropertyAsInt"), reinterpret_cast<PyCFunction>(propertiesGetPropertyAsInt), METH_VARARGS,
        PyDoc_STR(STRCAST("getPropertyAsInt(key) -> int")) },
    { STRCAST("getPropertyAsIntWithDefault"), reinterpret_cast<PyCFunction>(propertiesGetPropertyAsIntWithDefault),
        METH_VARARGS, PyDoc_STR(STRCAST("getPropertyAsIntWithDefault(key, default) -> int")) },
    { STRCAST("getPropertyAsList"), reinterpret_cast<PyCFunction>(propertiesGetPropertyAsList), METH_VARARGS,
        PyDoc_STR(STRCAST("getPropertyAsList(key) -> list")) },
    { STRCAST("getPropertyAsListWithDefault"), reinterpret_cast<PyCFunction>(propertiesGetPropertyAsListWithDefault),
        METH_VARARGS, PyDoc_STR(STRCAST("getPropertyAsListWithDefault(key, default) -> list")) },
    { STRCAST("getPropertiesForPrefix"), reinterpret_cast<PyCFunction>(propertiesGetPropertiesForPrefix), METH_VARARGS,
        PyDoc_STR(STRCAST("getPropertiesForPrefix(prefix) -> dict")) },
    { STRCAST("setProperty"), reinterpret_cast<PyCFunction>(propertiesSetProperty), METH_VARARGS,
        PyDoc_STR(STRCAST("setProperty(key, value) -> None")) },
    { STRCAST("getCommandLineOptions"), reinterpret_cast<PyCFunction>(propertiesGetCommandLineOptions), METH_NOARGS,
        PyDoc_STR(STRCAST("getCommandLineOptions() -> list")) },
    { STRCAST("parseCommandLineOptions"), reinterpret_cast<PyCFunction>(propertiesParseCommandLineOptions),
        METH_VARARGS, PyDoc_STR(STRCAST("parseCommandLineOptions(prefix, options) -> list")) },
    { STRCAST("parseIceCommandLineOptions"), reinterpret_cast<PyCFunction>(propertiesParseIceCommandLineOptions),
        METH_VARARGS, PyDoc_STR(STRCAST("parseIceCommandLineOptions(prefix, options) -> list")) },
    { STRCAST("load"), reinterpret_cast<PyCFunction>(propertiesLoad), METH_VARARGS,
        PyDoc_STR(STRCAST("load(file) -> None")) },
    { STRCAST("clone"), reinterpret_cast<PyCFunction>(propertiesClone), METH_NOARGS,
        PyDoc_STR(STRCAST("clone() -> Ice.Properties")) },
    { 0, 0 } /* sentinel */
};

namespace IcePy
{

PyTypeObject PropertiesType =
{
    /* The ob_type field must be initialized in the module init function
     * to be portable to Windows without using C++. */
    PyVarObject_HEAD_INIT(0, 0)
    STRCAST("IcePy.Properties"),    /* tp_name */
    sizeof(PropertiesObject),       /* tp_basicsize */
    0,                              /* tp_itemsize */
    /* methods */
    reinterpret_cast<destructor>(propertiesDealloc), /* tp_dealloc */
    0,                              /* tp_print */
    0,                              /* tp_getattr */
    0,                              /* tp_setattr */
    0,                              /* tp_reserved */
    0,                              /* tp_repr */
    0,                              /* tp_as_number */
    0,                              /* tp_as_sequence */
    0,                              /* tp_as_mapping */
    0,                              /* tp_hash */
    0,                              /* tp_call */
    reinterpret_cast<reprfunc>(propertiesStr), /* tp_str */
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
    reinterpret_cast<initproc>(propertiesInit), /* tp_init */
    0,                              /* tp_alloc */
    reinterpret_cast<newfunc>(propertiesNew), /* tp_new */
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
    PyTypeObject* type = &PropertiesType; // Necessary to prevent GCC's strict-alias warnings.
    if(PyModule_AddObject(module, STRCAST("Properties"), reinterpret_cast<PyObject*>(type)) < 0)
    {
        return false;
    }

    return true;
}

PyObject*
IcePy::createProperties(const Ice::PropertiesPtr& props)
{
    PropertiesObject* obj = propertiesNew(&PropertiesType, 0, 0);
    if(obj)
    {
        obj->properties = new Ice::PropertiesPtr(props);
    }
    return reinterpret_cast<PyObject*>(obj);
}

Ice::PropertiesPtr
IcePy::getProperties(PyObject* p)
{
    PropertiesObject* obj = reinterpret_cast<PropertiesObject*>(p);
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
    PyTypeObject* type = &PropertiesType; // Necessary to prevent GCC's strict-alias warnings.
    return PyObject_Call(reinterpret_cast<PyObject*>(type), args, 0);
}
