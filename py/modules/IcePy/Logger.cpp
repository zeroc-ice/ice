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
#include <Logger.h>
#include <Util.h>
#include <Ice/Logger.h>

using namespace std;
using namespace IcePy;

namespace IcePy
{

extern PyTypeObject LoggerType;

struct LoggerObject
{
    PyObject_HEAD
    Ice::LoggerPtr* logger;
};

//
// LoggerWrapper delegates Logger operations to a Python implementation.
//
class LoggerWrapper : public Ice::Logger
{
public:

    LoggerWrapper(PyObject*);

    virtual void print(const string&);
    virtual void trace(const string&, const string&);
    virtual void warning(const string&);
    virtual void error(const string&);

private:

    PyObjectHandle _logger;
};

}

IcePy::LoggerWrapper::LoggerWrapper(PyObject* logger) :
    _logger(logger)
{
    Py_INCREF(logger);
}

void
IcePy::LoggerWrapper::print(const string& message)
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    PyObjectHandle tmp = PyObject_CallMethod(_logger.get(), "print", "s", message.c_str());
    if(tmp.get() == NULL)
    {
        throwPythonException();
    }
}

void
IcePy::LoggerWrapper::trace(const string& category, const string& message)
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    PyObjectHandle tmp = PyObject_CallMethod(_logger.get(), "trace", "ss", category.c_str(), message.c_str());
    if(tmp.get() == NULL)
    {
        throwPythonException();
    }
}

void
IcePy::LoggerWrapper::warning(const string& message)
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    PyObjectHandle tmp = PyObject_CallMethod(_logger.get(), "warning", "s", message.c_str());
    if(tmp.get() == NULL)
    {
        throwPythonException();
    }
}

void
IcePy::LoggerWrapper::error(const string& message)
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    PyObjectHandle tmp = PyObject_CallMethod(_logger.get(), "error", "s", message.c_str());
    if(tmp.get() == NULL)
    {
        throwPythonException();
    }
}

#ifdef WIN32
extern "C"
#endif
static LoggerObject*
loggerNew(PyObject* /*arg*/)
{
    LoggerObject* self = PyObject_New(LoggerObject, &LoggerType);
    if (self == NULL)
    {
        return NULL;
    }
    self->logger = 0;
    return self;
}

#ifdef WIN32
extern "C"
#endif
static void
loggerDealloc(LoggerObject* self)
{
    delete self->logger;
    PyObject_Del(self);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
loggerPrint(LoggerObject* self, PyObject* args)
{
    char* message;
    if(!PyArg_ParseTuple(args, "s", &message))
    {
        return NULL;
    }

    assert(self->logger);
    try
    {
        (*self->logger)->print(message);
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
loggerTrace(LoggerObject* self, PyObject* args)
{
    char* category;
    char* message;
    if(!PyArg_ParseTuple(args, "ss", &category, &message))
    {
        return NULL;
    }

    assert(self->logger);
    try
    {
        (*self->logger)->trace(category, message);
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
loggerWarning(LoggerObject* self, PyObject* args)
{
    char* message;
    if(!PyArg_ParseTuple(args, "s", &message))
    {
        return NULL;
    }

    assert(self->logger);
    try
    {
        (*self->logger)->warning(message);
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
loggerError(LoggerObject* self, PyObject* args)
{
    char* message;
    if(!PyArg_ParseTuple(args, "s", &message))
    {
        return NULL;
    }

    assert(self->logger);
    try
    {
        (*self->logger)->error(message);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

static PyMethodDef LoggerMethods[] =
{
    { "_print", (PyCFunction)loggerPrint, METH_VARARGS,
        PyDoc_STR("_print(message) -> None") },
    { "trace", (PyCFunction)loggerTrace, METH_VARARGS,
        PyDoc_STR("trace(category, message) -> None") },
    { "warning", (PyCFunction)loggerWarning, METH_VARARGS,
        PyDoc_STR("warning(message) -> None") },
    { "error", (PyCFunction)loggerError, METH_VARARGS,
        PyDoc_STR("error(message) -> None") },
    { NULL, NULL} /* sentinel */
};

namespace IcePy
{

PyTypeObject LoggerType =
{
    /* The ob_type field must be initialized in the module init function
     * to be portable to Windows without using C++. */
    PyObject_HEAD_INIT(NULL)
    0,                              /* ob_size */
    "IcePy.Logger",                 /* tp_name */
    sizeof(LoggerObject),           /* tp_basicsize */
    0,                              /* tp_itemsize */
    /* methods */
    (destructor)loggerDealloc,      /* tp_dealloc */
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
    0,                              /* tp_str */
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
    LoggerMethods,                  /* tp_methods */
    0,                              /* tp_members */
    0,                              /* tp_getset */
    0,                              /* tp_base */
    0,                              /* tp_dict */
    0,                              /* tp_descr_get */
    0,                              /* tp_descr_set */
    0,                              /* tp_dictoffset */
    0,                              /* tp_init */
    0,                              /* tp_alloc */
    (newfunc)loggerNew,             /* tp_new */
    0,                              /* tp_free */
    0,                              /* tp_is_gc */
};

}

bool
IcePy::initLogger(PyObject* module)
{
    if(PyType_Ready(&LoggerType) < 0)
    {
        return false;
    }
    if(PyModule_AddObject(module, "Logger", (PyObject*)&LoggerType) < 0)
    {
        return false;
    }

    return true;
}

PyObject*
IcePy::createLogger(const Ice::LoggerPtr& logger)
{
    LoggerObject* obj = loggerNew(NULL);
    if(obj != NULL)
    {
        obj->logger = new Ice::LoggerPtr(logger);
    }
    return (PyObject*)obj;
}

Ice::LoggerPtr
IcePy::wrapLogger(PyObject* p)
{
    return new LoggerWrapper(p);
}
