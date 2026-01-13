// Copyright (c) ZeroC, Inc.

#include "ImplicitContext.h"
#include "Ice/ImplicitContext.h"
#include "ObjectAdapter.h"
#include "Proxy.h"
#include "Util.h"

using namespace std;
using namespace IcePy;

namespace IcePy
{
    extern PyTypeObject ImplicitContextType;

    struct ImplicitContextObject
    {
        PyObject_HEAD Ice::ImplicitContextPtr* implicitContext;
    };
}

extern "C" ImplicitContextObject*
implicitContextNew(PyTypeObject* type, PyObject* /*args*/, PyObject* /*kwds*/)
{
    auto* self = reinterpret_cast<ImplicitContextObject*>(type->tp_alloc(type, 0));
    if (!self)
    {
        return nullptr;
    }
    self->implicitContext = nullptr;
    return self;
}

extern "C" void
implicitContextDealloc(ImplicitContextObject* self)
{
    delete self->implicitContext;
    Py_TYPE(self)->tp_free(reinterpret_cast<PyObject*>(self));
}

extern "C" PyObject*
implicitContextCompare(ImplicitContextObject* c1, PyObject* other, int op)
{
    bool result{false};

    if (PyObject_TypeCheck(other, &ImplicitContextType))
    {
        auto* c2 = reinterpret_cast<ImplicitContextObject*>(other);

        switch (op)
        {
            case Py_EQ:
                result = *c1->implicitContext == *c2->implicitContext;
                break;
            case Py_NE:
                result = *c1->implicitContext != *c2->implicitContext;
                break;
            case Py_LE:
                result = *c1->implicitContext <= *c2->implicitContext;
                break;
            case Py_GE:
                result = *c1->implicitContext >= *c2->implicitContext;
                break;
            case Py_LT:
                result = *c1->implicitContext < *c2->implicitContext;
                break;
            case Py_GT:
                result = *c1->implicitContext > *c2->implicitContext;
                break;
        }
    }
    else
    {
        if (op == Py_EQ)
        {
            result = false;
        }
        else if (op == Py_NE)
        {
            result = true;
        }
        else
        {
            PyErr_Format(PyExc_TypeError, "can't compare %s to %s", Py_TYPE(c1)->tp_name, Py_TYPE(other)->tp_name);
            return nullptr;
        }
    }

    return result ? Py_True : Py_False;
}

extern "C" PyObject*
implicitContextGetContext(ImplicitContextObject* self, PyObject* /*args*/)
{
    Ice::Context ctx = (*self->implicitContext)->getContext();

    PyObjectHandle dict{PyDict_New()};
    if (!dict.get())
    {
        return nullptr;
    }

    if (!contextToDictionary(ctx, dict.get()))
    {
        return nullptr;
    }

    return dict.release();
}

extern "C" PyObject*
implicitContextSetContext(ImplicitContextObject* self, PyObject* args)
{
    PyObject* dict;
    if (!PyArg_ParseTuple(args, "O!", &PyDict_Type, &dict))
    {
        return nullptr;
    }

    Ice::Context ctx;
    if (!dictionaryToContext(dict, ctx))
    {
        return nullptr;
    }

    (*self->implicitContext)->setContext(ctx);

    return Py_None;
}

extern "C" PyObject*
implicitContextContainsKey(ImplicitContextObject* self, PyObject* args)
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

    try
    {
        return (*self->implicitContext)->containsKey(key) ? Py_True : Py_False;
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }
}

extern "C" PyObject*
implicitContextGet(ImplicitContextObject* self, PyObject* args)
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

    string val;
    try
    {
        val = (*self->implicitContext)->get(key);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }
    return createString(val);
}

extern "C" PyObject*
implicitContextPut(ImplicitContextObject* self, PyObject* args)
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

    string oldVal;
    try
    {
        oldVal = (*self->implicitContext)->put(key, value);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }
    return createString(oldVal);
}

extern "C" PyObject*
implicitContextRemove(ImplicitContextObject* self, PyObject* args)
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

    string val;
    try
    {
        val = (*self->implicitContext)->remove(key);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }
    return createString(val);
}

static PyMethodDef ImplicitContextMethods[] = {
    {"getContext",
     reinterpret_cast<PyCFunction>(implicitContextGetContext),
     METH_NOARGS,
     PyDoc_STR("getContext() -> dict[str, str]")},
    {"setContext",
     reinterpret_cast<PyCFunction>(implicitContextSetContext),
     METH_VARARGS,
     PyDoc_STR("setContext(newContext: dict[str, str]) -> None")},
    {"containsKey",
     reinterpret_cast<PyCFunction>(implicitContextContainsKey),
     METH_VARARGS,
     PyDoc_STR("containsKey(key: str) -> bool")},
    {"get", reinterpret_cast<PyCFunction>(implicitContextGet), METH_VARARGS, PyDoc_STR("get(key: str) -> str")},
    {"put",
     reinterpret_cast<PyCFunction>(implicitContextPut),
     METH_VARARGS,
     PyDoc_STR("put(key: str, value: str) -> str | None")},
    {"remove",
     reinterpret_cast<PyCFunction>(implicitContextRemove),
     METH_VARARGS,
     PyDoc_STR("remove(key: str) -> str | None")},
    {} /* sentinel */
};

namespace IcePy
{
    // clang-format off
    PyTypeObject ImplicitContextType = {
        .ob_base = PyVarObject_HEAD_INIT(nullptr, 0)
        .tp_name = "IcePy.ImplicitContext",
        .tp_basicsize = sizeof(ImplicitContextObject),
        .tp_dealloc = reinterpret_cast<destructor>(implicitContextDealloc),
        .tp_flags = Py_TPFLAGS_DEFAULT,
        .tp_doc = PyDoc_STR("IcePy.ImplicitContext"),
        .tp_richcompare = reinterpret_cast<richcmpfunc>(implicitContextCompare),
        .tp_methods = ImplicitContextMethods,
        .tp_new = reinterpret_cast<newfunc>(implicitContextNew),
    };
    // clang-format on
}

bool
IcePy::initImplicitContext(PyObject* module)
{
    if (PyType_Ready(&ImplicitContextType) < 0)
    {
        return false;
    }

    if (PyModule_AddObject(module, "ImplicitContext", reinterpret_cast<PyObject*>(&ImplicitContextType)) < 0)
    {
        return false;
    }

    return true;
}

PyObject*
IcePy::createImplicitContext(const Ice::ImplicitContextPtr& implicitContext)
{
    ImplicitContextObject* obj{implicitContextNew(&ImplicitContextType, nullptr, nullptr)};
    if (obj)
    {
        obj->implicitContext = new Ice::ImplicitContextPtr(implicitContext);
    }
    return reinterpret_cast<PyObject*>(obj);
}
