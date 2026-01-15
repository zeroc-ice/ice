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
    PropertiesObject* self{reinterpret_cast<PropertiesObject*>(type->tp_alloc(type, 0))};
    if (!self)
    {
        return nullptr;
    }
    self->properties = nullptr;
    return self;
}

extern "C" int
propertiesInit(PropertiesObject* self, PyObject* args, PyObject* /*kwds*/)
{
    PyObject* arglist{nullptr};
    PyObject* defaultsObj{nullptr};

    if (!PyArg_ParseTuple(args, "|OO", &arglist, &defaultsObj))
    {
        return -1;
    }

    Ice::StringSeq seq;
    if (arglist)
    {
        if (PyObject_IsInstance(arglist, reinterpret_cast<PyObject*>(&PyList_Type)))
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
        if (PyList_SetSlice(arglist, 0, PyList_Size(arglist), nullptr) < 0)
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
    for (auto p = dict.begin(); p != dict.end(); ++p)
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
    PyObject* keyObj{nullptr};
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
    PyObject* keyObj{nullptr};
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
    PyObject* keyObj{nullptr};
    PyObject* defObj{nullptr};
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
    return createString((*self->properties)->getPropertyWithDefault(key, def));
}

extern "C" PyObject*
propertiesGetPropertyAsInt(PropertiesObject* self, PyObject* args)
{
    PyObject* keyObj{nullptr};
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
    PyObject* keyObj{nullptr};
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
    PyObject* keyObj{nullptr};
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
    PyObject* keyObj{nullptr};
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

    PyObjectHandle list{PyList_New(0)};
    if (!list)
    {
        return nullptr;
    }
    if (!stringSeqToList(value, list.get()))
    {
        return nullptr;
    }

    return list.release();
}

extern "C" PyObject*
propertiesGetIcePropertyAsList(PropertiesObject* self, PyObject* args)
{
    PyObject* keyObj{nullptr};
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

    PyObjectHandle list{PyList_New(0)};
    if (!list)
    {
        return nullptr;
    }
    if (!stringSeqToList(value, list.get()))
    {
        return nullptr;
    }

    return list.release();
}

extern "C" PyObject*
propertiesGetPropertyAsListWithDefault(PropertiesObject* self, PyObject* args)
{
    PyObject* keyObj{nullptr};
    PyObject* defList{nullptr};
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

    PyObjectHandle list{PyList_New(0)};
    if (!list)
    {
        return nullptr;
    }
    if (!stringSeqToList(value, list.get()))
    {
        return nullptr;
    }

    return list.release();
}

extern "C" PyObject*
propertiesGetPropertiesForPrefix(PropertiesObject* self, PyObject* args)
{
    PyObject* prefixObj{nullptr};
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
    Ice::PropertyDict dict = (*self->properties)->getPropertiesForPrefix(prefix);

    PyObjectHandle result{PyDict_New()};
    if (result.get())
    {
        for (const auto& [key, value] : dict)
        {
            PyObjectHandle pyKey{createString(key)};
            PyObjectHandle pyValue{createString(value)};
            if (!pyValue.get() || PyDict_SetItem(result.get(), pyKey.get(), pyValue.get()) < 0)
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
    PyObject* keyObj{nullptr};
    PyObject* valueObj{nullptr};
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
    assert(self->properties);
    Ice::StringSeq options;
    try
    {
        options = (*self->properties)->getCommandLineOptions();
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    PyObjectHandle list{PyList_New(0)};
    if (!list)
    {
        return nullptr;
    }
    if (!stringSeqToList(options, list.get()))
    {
        return nullptr;
    }

    return list.release();
}

extern "C" PyObject*
propertiesParseCommandLineOptions(PropertiesObject* self, PyObject* args)
{
    PyObject* prefixObj{nullptr};
    PyObject* options{nullptr};
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

    PyObjectHandle list{PyList_New(0)};
    if (!list)
    {
        return nullptr;
    }
    if (!stringSeqToList(filteredSeq, list.get()))
    {
        return nullptr;
    }

    return list.release();
}

extern "C" PyObject*
propertiesParseIceCommandLineOptions(PropertiesObject* self, PyObject* args)
{
    PyObject* options{nullptr};
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

    PyObjectHandle list{PyList_New(0)};
    if (!list)
    {
        return nullptr;
    }
    if (!stringSeqToList(filteredSeq, list.get()))
    {
        return nullptr;
    }

    return list.release();
}

extern "C" PyObject*
propertiesLoad(PropertiesObject* self, PyObject* args)
{
    PyObject* fileObj{nullptr};
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

    return IcePy::createProperties(properties);
}

static PyMethodDef PropertyMethods[] = {
    {"getProperty",
     reinterpret_cast<PyCFunction>(propertiesGetProperty),
     METH_VARARGS,
     PyDoc_STR("getProperty(key: str) -> str")},
    {"getIceProperty",
     reinterpret_cast<PyCFunction>(propertiesGetIceProperty),
     METH_VARARGS,
     PyDoc_STR("getIceProperty(key: str) -> str")},
    {"getPropertyWithDefault",
     reinterpret_cast<PyCFunction>(propertiesGetPropertyWithDefault),
     METH_VARARGS,
     PyDoc_STR("getPropertyWithDefault(key: str, value: str) -> str")},
    {"getPropertyAsInt",
     reinterpret_cast<PyCFunction>(propertiesGetPropertyAsInt),
     METH_VARARGS,
     PyDoc_STR("getPropertyAsInt(key: str) -> int")},
    {"getIcePropertyAsInt",
     reinterpret_cast<PyCFunction>(propertiesGetIcePropertyAsInt),
     METH_VARARGS,
     PyDoc_STR("getIcePropertyAsInt(key: str) -> int")},
    {"getPropertyAsIntWithDefault",
     reinterpret_cast<PyCFunction>(propertiesGetPropertyAsIntWithDefault),
     METH_VARARGS,
     PyDoc_STR("getPropertyAsIntWithDefault(key: str, value: int) -> int")},
    {"getPropertyAsList",
     reinterpret_cast<PyCFunction>(propertiesGetPropertyAsList),
     METH_VARARGS,
     PyDoc_STR("getPropertyAsList(key: str) -> list[str]")},
    {"getIcePropertyAsList",
     reinterpret_cast<PyCFunction>(propertiesGetIcePropertyAsList),
     METH_VARARGS,
     PyDoc_STR("getIcePropertyAsList(key: str) -> list[str]")},
    {"getPropertyAsListWithDefault",
     reinterpret_cast<PyCFunction>(propertiesGetPropertyAsListWithDefault),
     METH_VARARGS,
     PyDoc_STR("getPropertyAsListWithDefault(key: str, value: list[str]) -> list[str]")},
    {"getPropertiesForPrefix",
     reinterpret_cast<PyCFunction>(propertiesGetPropertiesForPrefix),
     METH_VARARGS,
     PyDoc_STR("getPropertiesForPrefix(prefix: str) -> dict[str, str]")},
    {"setProperty",
     reinterpret_cast<PyCFunction>(propertiesSetProperty),
     METH_VARARGS,
     PyDoc_STR("setProperty(key: str, value: str) -> None")},
    {"getCommandLineOptions",
     reinterpret_cast<PyCFunction>(propertiesGetCommandLineOptions),
     METH_NOARGS,
     PyDoc_STR("getCommandLineOptions() -> list[str]")},
    {"parseCommandLineOptions",
     reinterpret_cast<PyCFunction>(propertiesParseCommandLineOptions),
     METH_VARARGS,
     PyDoc_STR("parseCommandLineOptions(prefix: str, options: list[str]) -> list[str]")},
    {"parseIceCommandLineOptions",
     reinterpret_cast<PyCFunction>(propertiesParseIceCommandLineOptions),
     METH_VARARGS,
     PyDoc_STR("parseIceCommandLineOptions(options: list[str]) -> list[str]")},
    {"load", reinterpret_cast<PyCFunction>(propertiesLoad), METH_VARARGS, PyDoc_STR("load(file: str) -> None")},
    {"clone", reinterpret_cast<PyCFunction>(propertiesClone), METH_NOARGS, PyDoc_STR("clone() -> Properties")},
    {} /* sentinel */
};

namespace IcePy
{
    // clang-format off
    PyTypeObject PropertiesType = {
        .ob_base = PyVarObject_HEAD_INIT(nullptr, 0)
        .tp_name = "IcePy.Properties",
        .tp_basicsize = sizeof(PropertiesObject),
        .tp_dealloc = reinterpret_cast<destructor>(propertiesDealloc),
        .tp_str = reinterpret_cast<reprfunc>(propertiesStr),
        .tp_flags = Py_TPFLAGS_DEFAULT,
        .tp_doc = PyDoc_STR("IcePy.Properties"),
        .tp_methods = PropertyMethods,
        .tp_init = reinterpret_cast<initproc>(propertiesInit),
        .tp_new = reinterpret_cast<newfunc>(propertiesNew),
    };
    // clang-format on
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
    PropertiesObject* obj{propertiesNew(&PropertiesType, nullptr, nullptr)};
    if (obj)
    {
        obj->properties = new Ice::PropertiesPtr(props);
    }
    return reinterpret_cast<PyObject*>(obj);
}

Ice::PropertiesPtr
IcePy::getProperties(PyObject* p)
{
    PropertiesObject* obj{reinterpret_cast<PropertiesObject*>(p)};
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
    return PyObject_Call(reinterpret_cast<PyObject*>(type), args, nullptr);
}
