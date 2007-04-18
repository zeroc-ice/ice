// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifdef _WIN32
#   include <IceUtil/Config.h>
#endif
#include <Logger.h>
#include <Ice/Initialize.h>

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

    PyObjectHandle tmp = PyObject_CallMethod(_logger.get(), STRCAST("print"), STRCAST("s"), message.c_str());
    if(!tmp.get())
    {
        throwPythonException();
    }
}

void
IcePy::LoggerWrapper::trace(const string& category, const string& message)
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    PyObjectHandle tmp = PyObject_CallMethod(_logger.get(), STRCAST("trace"), STRCAST("ss"), category.c_str(),
                                             message.c_str());
    if(!tmp.get())
    {
        throwPythonException();
    }
}

void
IcePy::LoggerWrapper::warning(const string& message)
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    PyObjectHandle tmp = PyObject_CallMethod(_logger.get(), STRCAST("warning"), STRCAST("s"), message.c_str());
    if(!tmp.get())
    {
        throwPythonException();
    }
}

void
IcePy::LoggerWrapper::error(const string& message)
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    PyObjectHandle tmp = PyObject_CallMethod(_logger.get(), STRCAST("error"), STRCAST("s"), message.c_str());
    if(!tmp.get())
    {
        throwPythonException();
    }
}

PyObject*
IcePy::LoggerWrapper::getObject()
{
    return _logger.get();
}

#ifdef WIN32
extern "C"
#endif
static LoggerObject*
loggerNew(PyObject* /*arg*/)
{
    LoggerObject* self = PyObject_New(LoggerObject, &LoggerType);
    if(!self)
    {
        return 0;
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
    if(!PyArg_ParseTuple(args, STRCAST("s"), &message))
    {
        return 0;
    }

    assert(self->logger);
    try
    {
        (*self->logger)->print(message);
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
loggerTrace(LoggerObject* self, PyObject* args)
{
    char* category;
    char* message;
    if(!PyArg_ParseTuple(args, STRCAST("ss"), &category, &message))
    {
        return 0;
    }

    assert(self->logger);
    try
    {
        (*self->logger)->trace(category, message);
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
loggerWarning(LoggerObject* self, PyObject* args)
{
    char* message;
    if(!PyArg_ParseTuple(args, STRCAST("s"), &message))
    {
        return 0;
    }

    assert(self->logger);
    try
    {
        (*self->logger)->warning(message);
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
loggerError(LoggerObject* self, PyObject* args)
{
    char* message;
    if(!PyArg_ParseTuple(args, STRCAST("s"), &message))
    {
        return 0;
    }

    assert(self->logger);
    try
    {
        (*self->logger)->error(message);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

static PyMethodDef LoggerMethods[] =
{
    { STRCAST("_print"), reinterpret_cast<PyCFunction>(loggerPrint), METH_VARARGS,
        PyDoc_STR(STRCAST("_print(message) -> None")) },
    { STRCAST("trace"), reinterpret_cast<PyCFunction>(loggerTrace), METH_VARARGS,
        PyDoc_STR(STRCAST("trace(category, message) -> None")) },
    { STRCAST("warning"), reinterpret_cast<PyCFunction>(loggerWarning), METH_VARARGS,
        PyDoc_STR(STRCAST("warning(message) -> None")) },
    { STRCAST("error"), reinterpret_cast<PyCFunction>(loggerError), METH_VARARGS,
        PyDoc_STR(STRCAST("error(message) -> None")) },
    { 0, 0 } /* sentinel */
};

namespace IcePy
{

PyTypeObject LoggerType =
{
    /* The ob_type field must be initialized in the module init function
     * to be portable to Windows without using C++. */
    PyObject_HEAD_INIT(0)
    0,                              /* ob_size */
    STRCAST("IcePy.Logger"),        /* tp_name */
    sizeof(LoggerObject),           /* tp_basicsize */
    0,                              /* tp_itemsize */
    /* methods */
    reinterpret_cast<destructor>(loggerDealloc), /* tp_dealloc */
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
    reinterpret_cast<newfunc>(loggerNew), /* tp_new */
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
    PyTypeObject* type = &LoggerType; // Necessary to prevent GCC's strict-alias warnings.
    if(PyModule_AddObject(module, STRCAST("Logger"), reinterpret_cast<PyObject*>(type)) < 0)
    {
        return false;
    }

    return true;
}

PyObject*
IcePy::createLogger(const Ice::LoggerPtr& logger)
{
    LoggerObject* obj = loggerNew(0);
    if(obj)
    {
        obj->logger = new Ice::LoggerPtr(logger);
    }
    return reinterpret_cast<PyObject*>(obj);
}

extern "C"
PyObject*
IcePy_getProcessLogger(PyObject* /*self*/)
{
    Ice::LoggerPtr logger;
    try
    {
        logger = Ice::getProcessLogger();
    }
    catch(const Ice::Exception& ex)
    {
        IcePy::setPythonException(ex);
        return 0;
    }

    //
    // The process logger can either be a C++ object (such as
    // the default logger supplied by the Ice run time), or a C++
    // wrapper around a Python implementation. If the latter, we
    // return it directly. Otherwise, we create a Python object
    // that delegates to the C++ object.
    //
    LoggerWrapperPtr wrapper = LoggerWrapperPtr::dynamicCast(logger);
    if(wrapper)
    {
        PyObject* obj = wrapper->getObject();
        Py_INCREF(obj);
        return obj;
    }

    return createLogger(logger);
}

extern "C"
PyObject*
IcePy_setProcessLogger(PyObject* /*self*/, PyObject* args)
{
    PyObject* loggerType = lookupType("Ice.Logger");
    assert(loggerType);

    PyObject* logger;
    if(!PyArg_ParseTuple(args, STRCAST("O!"), loggerType, &logger))
    {
        return 0;
    }

    Ice::LoggerPtr wrapper = new LoggerWrapper(logger);
    try
    {
        Ice::setProcessLogger(wrapper);
    }
    catch(const Ice::Exception& ex)
    {
        IcePy::setPythonException(ex);
        return 0;
    }

    Py_INCREF(Py_None);
    return Py_None;
}
