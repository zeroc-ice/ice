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
#include <ImplicitContext.h>
#include <ObjectAdapter.h>
#include <Proxy.h>
#include <Util.h>
#include <Ice/ImplicitContext.h>

using namespace std;
using namespace IcePy;

namespace IcePy
{

extern PyTypeObject ImplicitContextType;

struct ImplicitContextObject
{
    PyObject_HEAD
    Ice::ImplicitContextPtr* implicitContext;
};

}

#ifdef WIN32
extern "C"
#endif
static ImplicitContextObject*
implicitContextNew(PyTypeObject* type, PyObject* /*args*/, PyObject* /*kwds*/)
{
    ImplicitContextObject* self = reinterpret_cast<ImplicitContextObject*>(type->tp_alloc(type, 0));
    if(!self)
    {
        return 0;
    }
    self->implicitContext = 0;
    return self;
}

#ifdef WIN32
extern "C"
#endif
static void
implicitContextDealloc(ImplicitContextObject* self)
{
    delete self->implicitContext;
    Py_TYPE(self)->tp_free(reinterpret_cast<PyObject*>(self));
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
implicitContextCompare(ImplicitContextObject* c1, PyObject* other, int op)
{
    bool result = false;

    if(PyObject_TypeCheck(other, &ImplicitContextType))
    {
        ImplicitContextObject* c2 = reinterpret_cast<ImplicitContextObject*>(other);

        switch(op)
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
        if(op == Py_EQ)
        {
            result = false;
        }
        else if(op == Py_NE)
        {
            result = true;
        }
        else
        {
            PyErr_Format(PyExc_TypeError, "can't compare %s to %s", Py_TYPE(c1)->tp_name, Py_TYPE(other)->tp_name);
            return 0;
        }
    }

    PyObject* r = result ? getTrue() : getFalse();
    Py_INCREF(r);
    return r;
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
implicitContextGetContext(ImplicitContextObject* self)
{
    Ice::Context ctx = (*self->implicitContext)->getContext();

    PyObjectHandle dict = PyDict_New();
    if(!dict.get())
    {
        return 0;
    }

    if(!contextToDictionary(ctx, dict.get()))
    {
        return 0;
    }

    return dict.release();
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
implicitContextSetContext(ImplicitContextObject* self, PyObject* args)
{
    PyObject* dict;
    if(!PyArg_ParseTuple(args, STRCAST("O!"), &PyDict_Type, &dict))
    {
        return 0;
    }

    Ice::Context ctx;
    if(!dictionaryToContext(dict, ctx))
    {
        return 0;
    }

    (*self->implicitContext)->setContext(ctx);

    Py_INCREF(Py_None);
    return Py_None;
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
implicitContextContainsKey(ImplicitContextObject* self, PyObject* args)
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

    bool containsKey;
    try
    {
        containsKey = (*self->implicitContext)->containsKey(key);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    PyRETURN_BOOL(containsKey);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
implicitContextGet(ImplicitContextObject* self, PyObject* args)
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

    string val;
    try
    {
        val = (*self->implicitContext)->get(key);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }
    return createString(val);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
implicitContextPut(ImplicitContextObject* self, PyObject* args)
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

    string oldVal;
    try
    {
        oldVal = (*self->implicitContext)->put(key, value);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }
    return createString(oldVal);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
implicitContextRemove(ImplicitContextObject* self, PyObject* args)
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

    string val;
    try
    {
        val = (*self->implicitContext)->remove(key);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }
    return createString(val);
}

static PyMethodDef ImplicitContextMethods[] =
{
    { STRCAST("getContext"), reinterpret_cast<PyCFunction>(implicitContextGetContext), METH_VARARGS,
      PyDoc_STR(STRCAST("getContext() -> Ice.Context")) },
    { STRCAST("setContext"), reinterpret_cast<PyCFunction>(implicitContextSetContext), METH_VARARGS,
      PyDoc_STR(STRCAST("setContext(ctx) -> string")) },
    { STRCAST("containsKey"), reinterpret_cast<PyCFunction>(implicitContextContainsKey), METH_VARARGS,
      PyDoc_STR(STRCAST("containsKey(key) -> bool")) },
    { STRCAST("get"), reinterpret_cast<PyCFunction>(implicitContextGet), METH_VARARGS,
      PyDoc_STR(STRCAST("get(key) -> string")) },
    { STRCAST("put"), reinterpret_cast<PyCFunction>(implicitContextPut), METH_VARARGS,
      PyDoc_STR(STRCAST("put(key, value) -> string")) },
    { STRCAST("remove"), reinterpret_cast<PyCFunction>(implicitContextRemove), METH_VARARGS,
      PyDoc_STR(STRCAST("remove(key) -> string")) },
    { 0, 0 } /* sentinel */
};

namespace IcePy
{

PyTypeObject ImplicitContextType =
{
    /* The ob_type field must be initialized in the module init function
     * to be portable to Windows without using C++. */
    PyVarObject_HEAD_INIT(0, 0)
    STRCAST("IcePy.ImplicitContext"),    /* tp_name */
    sizeof(ImplicitContextObject),       /* tp_basicsize */
    0,                              /* tp_itemsize */
    /* methods */
    reinterpret_cast<destructor>(implicitContextDealloc), /* tp_dealloc */
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
    0,                              /* tp_str */
    0,                              /* tp_getattro */
    0,                              /* tp_setattro */
    0,                              /* tp_as_buffer */
#if PY_VERSION_HEX >= 0x03000000
    Py_TPFLAGS_DEFAULT,             /* tp_flags */
#else
    Py_TPFLAGS_DEFAULT |
    Py_TPFLAGS_HAVE_RICHCOMPARE,    /* tp_flags */
#endif
    0,                              /* tp_doc */
    0,                              /* tp_traverse */
    0,                              /* tp_clear */
    reinterpret_cast<richcmpfunc>(implicitContextCompare), /* tp_richcompare */
    0,                              /* tp_weaklistoffset */
    0,                              /* tp_iter */
    0,                              /* tp_iternext */
    ImplicitContextMethods,         /* tp_methods */
    0,                              /* tp_members */
    0,                              /* tp_getset */
    0,                              /* tp_base */
    0,                              /* tp_dict */
    0,                              /* tp_descr_get */
    0,                              /* tp_descr_set */
    0,                              /* tp_dictoffset */
    0,                              /* tp_init */
    0,                              /* tp_alloc */
    reinterpret_cast<newfunc>(implicitContextNew), /* tp_new */
    0,                              /* tp_free */
    0,                              /* tp_is_gc */
};

}

bool
IcePy::initImplicitContext(PyObject* module)
{
    if(PyType_Ready(&ImplicitContextType) < 0)
    {
        return false;
    }
    PyTypeObject* type = &ImplicitContextType; // Necessary to prevent GCC's strict-alias warnings.
    if(PyModule_AddObject(module, STRCAST("ImplicitContext"), reinterpret_cast<PyObject*>(type)) < 0)
    {
        return false;
    }

    return true;
}

PyObject*
IcePy::createImplicitContext(const Ice::ImplicitContextPtr& implicitContext)
{
    ImplicitContextObject* obj = implicitContextNew(&ImplicitContextType, 0, 0);
    if(obj)
    {
        obj->implicitContext = new Ice::ImplicitContextPtr(implicitContext);
    }
    return reinterpret_cast<PyObject*>(obj);
}
