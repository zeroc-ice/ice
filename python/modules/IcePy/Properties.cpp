// Copyright (c) ZeroC, Inc.

#include "Properties.h"
#include "Ice/Initialize.h"
#include "Ice/Properties.h"
#include "Util.h"

using namespace std;
using namespace IcePy;

namespace IcePy
{
    struct PropertiesObject
    {
        PyObject_HEAD Ice::PropertiesPtr* properties;
    };
}

extern "C" PropertiesObject*
propertiesNew(PyTypeObject* type, PyObject* /*args*/, PyObject* /*kwds*/)
{
    PropertiesObject* self = reinterpret_cast<PropertiesObject*>(type->tp_alloc(type, 0));
    if (!self)
    {
        return nullptr;
    }
    self->properties = 0;
    return self;
}

extern "C" int
propertiesInit(PropertiesObject* self, PyObject* args, PyObject* /*kwds*/)
{
    PyObject* arglist = 0;
    PyObject* defaultsObj = 0;

    if (!PyArg_ParseTuple(args, "|OO", &arglist, &defaultsObj))
    {
        return -1;
    }

    Ice::StringSeq seq;
    if (arglist)
    {
        PyTypeObject* listType = &PyList_Type; // Necessary to prevent GCC's strict-alias warnings.
        if (PyObject_IsInstance(arglist, reinterpret_cast<PyObject*>(listType)))
        {
            if (!listToStringSeq(arglist, seq))
            {
                return -1;
            }
        }
        else if (arglist != Py_None)
        {
            PyErr_Format(PyExc_ValueError, "args must be None or a list");
            return -1;
        }
    }

    Ice::PropertiesPtr defaults;
    if (defaultsObj)
    {
        PyObject* propType = lookupType("Ice.Properties");
        assert(propType);
        if (PyObject_IsInstance(defaultsObj, propType))
        {
            PyObjectHandle impl{getAttr(defaultsObj, "_impl", false)};
            assert(impl.get());
            defaults = getProperties(impl.get());
        }
        else if (defaultsObj != Py_None)
        {
            PyErr_Format(PyExc_ValueError, "defaults must be None or a Ice.Properties");
            return -1;
        }
    }

    Ice::PropertiesPtr props;
    try
    {
        if (defaults || (arglist && arglist != Py_None))
        {
            props = Ice::createProperties(seq, defaults);
        }
        else
        {
            props = Ice::createProperties();
        }
    }
    catch (...)
    {
        setPythonException(current_exception());
        return -1;
    }

    //
    // Replace the contents of the given argument list with the filtered arguments.
    //
    if (arglist && arglist != Py_None)
    {
        if (PyList_SetSlice(arglist, 0, PyList_Size(arglist), 0) < 0)
        {
            return -1;
        }
        if (!stringSeqToList(seq, arglist))
        {
            return -1;
        }
    }

    self->properties = new Ice::PropertiesPtr(props);

    return 0;
}

extern "C" void
propertiesDealloc(PropertiesObject* self)
{
    delete self->properties;
    Py_TYPE(self)->tp_free(reinterpret_cast<PyObject*>(self));
}

extern "C" PyObject*
propertiesStr(PropertiesObject* self)
{
    assert(self->properties);

    Ice::PropertyDict dict;
    try
    {
        dict = (*self->properties)->getPropertiesForPrefix("");
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    string str;
    for (Ice::PropertyDict::const_iterator p = dict.begin(); p != dict.end(); ++p)
    {
        if (p != dict.begin())
        {
            str.append("\n");
        }
        str.append(p->first + "=" + p->second);
    }

    return createString(str);
}

extern "C" PyObject*
propertiesGetProperty(PropertiesObject* self, PyObject* args)
{
    PyObject* keyObj;
    if (!PyArg_ParseTuple(args, "O", &keyObj))
    {
        return nullptr;
    }

    string key;
    if (!getStringArg(keyObj, "key", key))
    {
        return nullptr;
    }

    assert(self->properties);
    string value;
    try
    {
        value = (*self->properties)->getProperty(key);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    return createString(value);
}

extern "C" PyObject*
propertiesGetIceProperty(PropertiesObject* self, PyObject* args)
{
    PyObject* keyObj;
    if (!PyArg_ParseTuple(args, "O", &keyObj))
    {
        return nullptr;
    }

    string key;
    if (!getStringArg(keyObj, "key", key))
    {
        return nullptr;
    }

    assert(self->properties);
    string value;
    try
    {
        value = (*self->properties)->getIceProperty(key);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    return createString(value);
}

extern "C" PyObject*
propertiesGetPropertyWithDefault(PropertiesObject* self, PyObject* args)
{
    PyObject* keyObj;
    PyObject* defObj;
    if (!PyArg_ParseTuple(args, "OO", &keyObj, &defObj))
    {
        return nullptr;
    }

    string key;
    string def;
    if (!getStringArg(keyObj, "key", key))
    {
        return nullptr;
    }
    if (!getStringArg(defObj, "value", def))
    {
        return nullptr;
    }

    assert(self->properties);
    string value;
    try
    {
        value = (*self->properties)->getPropertyWithDefault(key, def);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    return createString(value);
}

extern "C" PyObject*
propertiesGetPropertyAsInt(PropertiesObject* self, PyObject* args)
{
    PyObject* keyObj;
    if (!PyArg_ParseTuple(args, "O", &keyObj))
    {
        return nullptr;
    }

    string key;
    if (!getStringArg(keyObj, "key", key))
    {
        return nullptr;
    }

    assert(self->properties);
    int32_t value;
    try
    {
        value = (*self->properties)->getPropertyAsInt(key);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    return PyLong_FromLong(value);
}

extern "C" PyObject*
propertiesGetIcePropertyAsInt(PropertiesObject* self, PyObject* args)
{
    PyObject* keyObj;
    if (!PyArg_ParseTuple(args, "O", &keyObj))
    {
        return nullptr;
    }

    string key;
    if (!getStringArg(keyObj, "key", key))
    {
        return nullptr;
    }

    assert(self->properties);
    int32_t value;
    try
    {
        value = (*self->properties)->getIcePropertyAsInt(key);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    return PyLong_FromLong(value);
}

extern "C" PyObject*
propertiesGetPropertyAsIntWithDefault(PropertiesObject* self, PyObject* args)
{
    PyObject* keyObj;
    int def;
    if (!PyArg_ParseTuple(args, "Oi", &keyObj, &def))
    {
        return nullptr;
    }

    string key;
    if (!getStringArg(keyObj, "key", key))
    {
        return nullptr;
    }

    assert(self->properties);
    int32_t value;
    try
    {
        value = (*self->properties)->getPropertyAsIntWithDefault(key, def);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    return PyLong_FromLong(value);
}

extern "C" PyObject*
propertiesGetPropertyAsList(PropertiesObject* self, PyObject* args)
{
    PyObject* keyObj;
    if (!PyArg_ParseTuple(args, "O", &keyObj))
    {
        return nullptr;
    }

    string key;
    if (!getStringArg(keyObj, "key", key))
    {
        return nullptr;
    }

    assert(self->properties);
    Ice::StringSeq value;
    try
    {
        value = (*self->properties)->getPropertyAsList(key);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    PyObject* list = PyList_New(0);
    if (!list)
    {
        return nullptr;
    }
    if (!stringSeqToList(value, list))
    {
        return nullptr;
    }

    return list;
}

extern "C" PyObject*
propertiesGetIcePropertyAsList(PropertiesObject* self, PyObject* args)
{
    PyObject* keyObj;
    if (!PyArg_ParseTuple(args, "O", &keyObj))
    {
        return nullptr;
    }

    string key;
    if (!getStringArg(keyObj, "key", key))
    {
        return nullptr;
    }

    assert(self->properties);
    Ice::StringSeq value;
    try
    {
        value = (*self->properties)->getIcePropertyAsList(key);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    PyObject* list = PyList_New(0);
    if (!list)
    {
        return nullptr;
    }
    if (!stringSeqToList(value, list))
    {
        return nullptr;
    }

    return list;
}

extern "C" PyObject*
propertiesGetPropertyAsListWithDefault(PropertiesObject* self, PyObject* args)
{
    PyObject* keyObj;
    PyObject* defList;
    if (!PyArg_ParseTuple(args, "OO!", &keyObj, &PyList_Type, &defList))
    {
        return nullptr;
    }

    string key;
    if (!getStringArg(keyObj, "key", key))
    {
        return nullptr;
    }

    assert(self->properties);
    Ice::StringSeq def;
    if (!listToStringSeq(defList, def))
    {
        return nullptr;
    }

    Ice::StringSeq value;
    try
    {
        value = (*self->properties)->getPropertyAsListWithDefault(key, def);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    PyObject* list = PyList_New(0);
    if (!list)
    {
        return nullptr;
    }
    if (!stringSeqToList(value, list))
    {
        return nullptr;
    }

    return list;
}

extern "C" PyObject*
propertiesGetPropertiesForPrefix(PropertiesObject* self, PyObject* args)
{
    PyObject* prefixObj;
    if (!PyArg_ParseTuple(args, "O", &prefixObj))
    {
        return nullptr;
    }

    string prefix;
    if (!getStringArg(prefixObj, "prefix", prefix))
    {
        return nullptr;
    }

    assert(self->properties);
    Ice::PropertyDict dict;
    try
    {
        dict = (*self->properties)->getPropertiesForPrefix(prefix);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    PyObjectHandle result{PyDict_New()};
    if (result.get())
    {
        for (Ice::PropertyDict::iterator p = dict.begin(); p != dict.end(); ++p)
        {
            PyObjectHandle key{createString(p->first)};
            PyObjectHandle val{createString(p->second)};
            if (!val.get() || PyDict_SetItem(result.get(), key.get(), val.get()) < 0)
            {
                return nullptr;
            }
        }
    }

    return result.release();
}

extern "C" PyObject*
propertiesSetProperty(PropertiesObject* self, PyObject* args)
{
    PyObject* keyObj;
    PyObject* valueObj;
    if (!PyArg_ParseTuple(args, "OO", &keyObj, &valueObj))
    {
        return nullptr;
    }

    string key;
    string value;
    if (!getStringArg(keyObj, "key", key))
    {
        return nullptr;
    }
    if (!getStringArg(valueObj, "value", value))
    {
        return nullptr;
    }

    assert(self->properties);
    try
    {
        (*self->properties)->setProperty(key, value);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    return Py_None;
}

extern "C" PyObject*
propertiesGetCommandLineOptions(PropertiesObject* self, PyObject* /*args*/)
{
    Ice::StringSeq options;
    assert(self->properties);
    try
    {
        options = (*self->properties)->getCommandLineOptions();
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    PyObject* list = PyList_New(0);
    if (!list)
    {
        return nullptr;
    }
    if (!stringSeqToList(options, list))
    {
        return nullptr;
    }

    return list;
}

extern "C" PyObject*
propertiesParseCommandLineOptions(PropertiesObject* self, PyObject* args)
{
    PyObject* prefixObj;
    PyObject* options;
    if (!PyArg_ParseTuple(args, "OO!", &prefixObj, &PyList_Type, &options))
    {
        return nullptr;
    }

    Ice::StringSeq seq;
    if (!listToStringSeq(options, seq))
    {
        return nullptr;
    }

    string prefix;
    if (!getStringArg(prefixObj, "prefix", prefix))
    {
        return nullptr;
    }

    assert(self->properties);
    Ice::StringSeq filteredSeq;
    try
    {
        filteredSeq = (*self->properties)->parseCommandLineOptions(prefix, seq);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    PyObject* list = PyList_New(0);
    if (!list)
    {
        return nullptr;
    }
    if (!stringSeqToList(filteredSeq, list))
    {
        return nullptr;
    }

    return list;
}

extern "C" PyObject*
propertiesParseIceCommandLineOptions(PropertiesObject* self, PyObject* args)
{
    PyObject* options;
    if (!PyArg_ParseTuple(args, "O!", &PyList_Type, &options))
    {
        return nullptr;
    }

    Ice::StringSeq seq;
    if (!listToStringSeq(options, seq))
    {
        return nullptr;
    }

    assert(self->properties);
    Ice::StringSeq filteredSeq;
    try
    {
        filteredSeq = (*self->properties)->parseIceCommandLineOptions(seq);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    PyObject* list = PyList_New(0);
    if (!list)
    {
        return nullptr;
    }
    if (!stringSeqToList(filteredSeq, list))
    {
        return nullptr;
    }

    return list;
}

extern "C" PyObject*
propertiesLoad(PropertiesObject* self, PyObject* args)
{
    PyObject* fileObj;
    if (!PyArg_ParseTuple(args, "O", &fileObj))
    {
        return nullptr;
    }

    string file;
    if (!getStringArg(fileObj, "file", file))
    {
        return nullptr;
    }

    assert(self->properties);
    try
    {
        (*self->properties)->load(file);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    return Py_None;
}

extern "C" PyObject*
propertiesClone(PropertiesObject* self, PyObject* /*args*/)
{
    Ice::PropertiesPtr properties;
    assert(self->properties);
    try
    {
        properties = (*self->properties)->clone();
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    return createProperties(properties);
}

static PyMethodDef PropertyMethods[] = {
    {"getProperty",
     reinterpret_cast<PyCFunction>(propertiesGetProperty),
     METH_VARARGS,
     PyDoc_STR("getProperty(key) -> string")},
    {"getIceProperty",
     reinterpret_cast<PyCFunction>(propertiesGetIceProperty),
     METH_VARARGS,
     PyDoc_STR("getIceProperty(key) -> string")},
    {"getPropertyWithDefault",
     reinterpret_cast<PyCFunction>(propertiesGetPropertyWithDefault),
     METH_VARARGS,
     PyDoc_STR("getPropertyWithDefault(key, default) -> string")},
    {"getPropertyAsInt",
     reinterpret_cast<PyCFunction>(propertiesGetPropertyAsInt),
     METH_VARARGS,
     PyDoc_STR("getPropertyAsInt(key) -> int")},
    {"getIcePropertyAsInt",
     reinterpret_cast<PyCFunction>(propertiesGetIcePropertyAsInt),
     METH_VARARGS,
     PyDoc_STR("getIcePropertyAsInt(key) -> int")},
    {"getPropertyAsIntWithDefault",
     reinterpret_cast<PyCFunction>(propertiesGetPropertyAsIntWithDefault),
     METH_VARARGS,
     PyDoc_STR("getPropertyAsIntWithDefault(key, default) -> int")},
    {"getPropertyAsList",
     reinterpret_cast<PyCFunction>(propertiesGetPropertyAsList),
     METH_VARARGS,
     PyDoc_STR("getPropertyAsList(key) -> list")},
    {"getIcePropertyAsList",
     reinterpret_cast<PyCFunction>(propertiesGetIcePropertyAsList),
     METH_VARARGS,
     PyDoc_STR("getIcePropertyAsList(key) -> list")},
    {"getPropertyAsListWithDefault",
     reinterpret_cast<PyCFunction>(propertiesGetPropertyAsListWithDefault),
     METH_VARARGS,
     PyDoc_STR("getPropertyAsListWithDefault(key, default) -> list")},
    {"getPropertiesForPrefix",
     reinterpret_cast<PyCFunction>(propertiesGetPropertiesForPrefix),
     METH_VARARGS,
     PyDoc_STR("getPropertiesForPrefix(prefix) -> dict")},
    {"setProperty",
     reinterpret_cast<PyCFunction>(propertiesSetProperty),
     METH_VARARGS,
     PyDoc_STR("setProperty(key, value) -> None")},
    {"getCommandLineOptions",
     reinterpret_cast<PyCFunction>(propertiesGetCommandLineOptions),
     METH_NOARGS,
     PyDoc_STR("getCommandLineOptions() -> list")},
    {"parseCommandLineOptions",
     reinterpret_cast<PyCFunction>(propertiesParseCommandLineOptions),
     METH_VARARGS,
     PyDoc_STR("parseCommandLineOptions(prefix, options) -> list")},
    {"parseIceCommandLineOptions",
     reinterpret_cast<PyCFunction>(propertiesParseIceCommandLineOptions),
     METH_VARARGS,
     PyDoc_STR("parseIceCommandLineOptions(prefix, options) -> list")},
    {"load", reinterpret_cast<PyCFunction>(propertiesLoad), METH_VARARGS, PyDoc_STR("load(file) -> None")},
    {"clone", reinterpret_cast<PyCFunction>(propertiesClone), METH_NOARGS, PyDoc_STR("clone() -> Ice.Properties")},
    {0, 0} /* sentinel */
};

namespace IcePy
{
    PyTypeObject PropertiesType = {
        /* The ob_type field must be initialized in the module init function
         * to be portable to Windows without using C++. */
        PyVarObject_HEAD_INIT(0, 0) "IcePy.Properties", /* tp_name */
        sizeof(PropertiesObject),                       /* tp_basicsize */
        0,                                              /* tp_itemsize */
        /* methods */
        reinterpret_cast<destructor>(propertiesDealloc), /* tp_dealloc */
        0,                                               /* tp_print */
        0,                                               /* tp_getattr */
        0,                                               /* tp_setattr */
        0,                                               /* tp_reserved */
        0,                                               /* tp_repr */
        0,                                               /* tp_as_number */
        0,                                               /* tp_as_sequence */
        0,                                               /* tp_as_mapping */
        0,                                               /* tp_hash */
        0,                                               /* tp_call */
        reinterpret_cast<reprfunc>(propertiesStr),       /* tp_str */
        0,                                               /* tp_getattro */
        0,                                               /* tp_setattro */
        0,                                               /* tp_as_buffer */
        Py_TPFLAGS_DEFAULT,                              /* tp_flags */
        0,                                               /* tp_doc */
        0,                                               /* tp_traverse */
        0,                                               /* tp_clear */
        0,                                               /* tp_richcompare */
        0,                                               /* tp_weaklistoffset */
        0,                                               /* tp_iter */
        0,                                               /* tp_iternext */
        PropertyMethods,                                 /* tp_methods */
        0,                                               /* tp_members */
        0,                                               /* tp_getset */
        0,                                               /* tp_base */
        0,                                               /* tp_dict */
        0,                                               /* tp_descr_get */
        0,                                               /* tp_descr_set */
        0,                                               /* tp_dictoffset */
        reinterpret_cast<initproc>(propertiesInit),      /* tp_init */
        0,                                               /* tp_alloc */
        reinterpret_cast<newfunc>(propertiesNew),        /* tp_new */
        0,                                               /* tp_free */
        0,                                               /* tp_is_gc */
    };
}

bool
IcePy::initProperties(PyObject* module)
{
    if (PyType_Ready(&PropertiesType) < 0)
    {
        return false;
    }
    PyTypeObject* type = &PropertiesType; // Necessary to prevent GCC's strict-alias warnings.
    if (PyModule_AddObject(module, "Properties", reinterpret_cast<PyObject*>(type)) < 0)
    {
        return false;
    }

    return true;
}

PyObject*
IcePy::createProperties(const Ice::PropertiesPtr& props)
{
    PropertiesObject* obj = propertiesNew(&PropertiesType, 0, 0);
    if (obj)
    {
        obj->properties = new Ice::PropertiesPtr(props);
    }
    return reinterpret_cast<PyObject*>(obj);
}

Ice::PropertiesPtr
IcePy::getProperties(PyObject* p)
{
    PropertiesObject* obj = reinterpret_cast<PropertiesObject*>(p);
    if (obj->properties)
    {
        return *obj->properties;
    }
    return nullptr;
}

extern "C" PyObject*
IcePy_createProperties(PyObject* /*self*/, PyObject* args)
{
    //
    // Currently the same as "p = Ice.Properties()".
    //
    PyTypeObject* type = &PropertiesType; // Necessary to prevent GCC's strict-alias warnings.
    return PyObject_Call(reinterpret_cast<PyObject*>(type), args, 0);
}
