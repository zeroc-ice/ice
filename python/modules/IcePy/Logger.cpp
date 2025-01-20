// Copyright (c) ZeroC, Inc.

#include "Logger.h"
#include "Ice/Initialize.h"
#include "Thread.h"

using namespace std;
using namespace IcePy;

namespace IcePy
{
    extern PyTypeObject LoggerType;

    struct LoggerObject
    {
        PyObject_HEAD Ice::LoggerPtr* logger;
    };
}

IcePy::LoggerWrapper::LoggerWrapper(PyObject* logger) : _logger(logger) { Py_INCREF(logger); }

void
IcePy::LoggerWrapper::print(const string& message)
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    //
    // Method must be named "_print".
    //
    PyObjectHandle tmp{PyObject_CallMethod(_logger.get(), "_print", "s", message.c_str())};
    if (!tmp.get())
    {
        throwPythonException();
    }
}

void
IcePy::LoggerWrapper::trace(const string& category, const string& message)
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    PyObjectHandle tmp{PyObject_CallMethod(_logger.get(), "trace", "ss", category.c_str(), message.c_str())};
    if (!tmp.get())
    {
        throwPythonException();
    }
}

void
IcePy::LoggerWrapper::warning(const string& message)
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    PyObjectHandle tmp{PyObject_CallMethod(_logger.get(), "warning", "s", message.c_str())};
    if (!tmp.get())
    {
        throwPythonException();
    }
}

void
IcePy::LoggerWrapper::error(const string& message)
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    PyObjectHandle tmp{PyObject_CallMethod(_logger.get(), "error", "s", message.c_str())};
    if (!tmp.get())
    {
        throwPythonException();
    }
}

string
IcePy::LoggerWrapper::getPrefix()
{
    AdoptThread adoptThread;

    PyObjectHandle tmp{PyObject_CallMethod(_logger.get(), "getPrefix", 0)};
    if (!tmp.get())
    {
        throwPythonException();
    }
    return getString(tmp.get());
}

Ice::LoggerPtr
IcePy::LoggerWrapper::cloneWithPrefix(string prefix)
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    PyObjectHandle tmp{PyObject_CallMethod(_logger.get(), "cloneWithPrefix", "s", prefix.c_str())};
    if (!tmp.get())
    {
        throwPythonException();
    }

    return make_shared<LoggerWrapper>(tmp.get());
}

PyObject*
IcePy::LoggerWrapper::getObject()
{
    return _logger.get();
}

extern "C" LoggerObject*
loggerNew(PyTypeObject* type, PyObject* /*args*/, PyObject* /*kwds*/)
{
    LoggerObject* self = reinterpret_cast<LoggerObject*>(type->tp_alloc(type, 0));
    if (!self)
    {
        return nullptr;
    }
    self->logger = nullptr;
    return self;
}

extern "C" void
loggerDealloc(LoggerObject* self)
{
    delete self->logger;
    Py_TYPE(self)->tp_free(reinterpret_cast<PyObject*>(self));
}

extern "C" PyObject*
loggerPrint(LoggerObject* self, PyObject* args)
{
    PyObject* messageObj;
    if (!PyArg_ParseTuple(args, "O", &messageObj))
    {
        return nullptr;
    }

    string message;
    if (!getStringArg(messageObj, "message", message))
    {
        return nullptr;
    }

    assert(self->logger);
    try
    {
        (*self->logger)->print(message);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    return Py_None;
}

extern "C" PyObject*
loggerTrace(LoggerObject* self, PyObject* args)
{
    PyObject* categoryObj;
    PyObject* messageObj;
    if (!PyArg_ParseTuple(args, "OO", &categoryObj, &messageObj))
    {
        return nullptr;
    }

    string category;
    string message;
    if (!getStringArg(categoryObj, "category", category))
    {
        return nullptr;
    }
    if (!getStringArg(messageObj, "message", message))
    {
        return nullptr;
    }

    assert(self->logger);
    try
    {
        (*self->logger)->trace(category, message);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    return Py_None;
}

extern "C" PyObject*
loggerWarning(LoggerObject* self, PyObject* args)
{
    PyObject* messageObj;
    if (!PyArg_ParseTuple(args, "O", &messageObj))
    {
        return nullptr;
    }

    string message;
    if (!getStringArg(messageObj, "message", message))
    {
        return nullptr;
    }

    assert(self->logger);
    try
    {
        (*self->logger)->warning(message);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    return Py_None;
}

extern "C" PyObject*
loggerError(LoggerObject* self, PyObject* args)
{
    PyObject* messageObj;
    if (!PyArg_ParseTuple(args, "O", &messageObj))
    {
        return nullptr;
    }

    string message;
    if (!getStringArg(messageObj, "message", message))
    {
        return nullptr;
    }

    assert(self->logger);
    try
    {
        (*self->logger)->error(message);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    return Py_None;
}

extern "C" PyObject*
loggerGetPrefix(LoggerObject* self, PyObject* /*args*/)
{
    string prefix;

    assert(self->logger);
    try
    {
        prefix = (*self->logger)->getPrefix();
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    return createString(prefix);
}

extern "C" PyObject*
loggerCloneWithPrefix(LoggerObject* self, PyObject* args)
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

    Ice::LoggerPtr clone;

    assert(self->logger);
    try
    {
        clone = (*self->logger)->cloneWithPrefix(prefix);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    //
    // The new clone can either be a C++ object (such as
    // the default logger supplied by the Ice run time), or a C++
    // wrapper around a Python implementation. If the latter, we
    // return it directly. Otherwise, we create a Python object
    // that delegates to the C++ object.
    //
    auto wrapper = dynamic_pointer_cast<LoggerWrapper>(clone);
    if (wrapper)
    {
        PyObject* obj = wrapper->getObject();
        Py_INCREF(obj);
        return obj;
    }

    return createLogger(clone);
}

static PyMethodDef LoggerMethods[] = {
    {"_print", reinterpret_cast<PyCFunction>(loggerPrint), METH_VARARGS, PyDoc_STR("_print(message) -> None")},
    {"trace", reinterpret_cast<PyCFunction>(loggerTrace), METH_VARARGS, PyDoc_STR("trace(category, message) -> None")},
    {"warning", reinterpret_cast<PyCFunction>(loggerWarning), METH_VARARGS, PyDoc_STR("warning(message) -> None")},
    {"error", reinterpret_cast<PyCFunction>(loggerError), METH_VARARGS, PyDoc_STR("error(message) -> None")},
    {"getPrefix", reinterpret_cast<PyCFunction>(loggerGetPrefix), METH_NOARGS, PyDoc_STR("getPrefix() -> string")},
    {"cloneWithPrefix",
     reinterpret_cast<PyCFunction>(loggerCloneWithPrefix),
     METH_VARARGS,
     PyDoc_STR("cloneWithPrefix(prefix) -> Ice.Logger")},
    {0, 0} /* sentinel */
};

namespace IcePy
{
    PyTypeObject LoggerType = {
        /* The ob_type field must be initialized in the module init function
         * to be portable to Windows without using C++. */
        PyVarObject_HEAD_INIT(0, 0) "IcePy.Logger", /* tp_name */
        sizeof(LoggerObject),                       /* tp_basicsize */
        0,                                          /* tp_itemsize */
        /* methods */
        reinterpret_cast<destructor>(loggerDealloc), /* tp_dealloc */
        0,                                           /* tp_print */
        0,                                           /* tp_getattr */
        0,                                           /* tp_setattr */
        0,                                           /* tp_reserved */
        0,                                           /* tp_repr */
        0,                                           /* tp_as_number */
        0,                                           /* tp_as_sequence */
        0,                                           /* tp_as_mapping */
        0,                                           /* tp_hash */
        0,                                           /* tp_call */
        0,                                           /* tp_str */
        0,                                           /* tp_getattro */
        0,                                           /* tp_setattro */
        0,                                           /* tp_as_buffer */
        Py_TPFLAGS_DEFAULT,                          /* tp_flags */
        0,                                           /* tp_doc */
        0,                                           /* tp_traverse */
        0,                                           /* tp_clear */
        0,                                           /* tp_richcompare */
        0,                                           /* tp_weaklistoffset */
        0,                                           /* tp_iter */
        0,                                           /* tp_iternext */
        LoggerMethods,                               /* tp_methods */
        0,                                           /* tp_members */
        0,                                           /* tp_getset */
        0,                                           /* tp_base */
        0,                                           /* tp_dict */
        0,                                           /* tp_descr_get */
        0,                                           /* tp_descr_set */
        0,                                           /* tp_dictoffset */
        0,                                           /* tp_init */
        0,                                           /* tp_alloc */
        reinterpret_cast<newfunc>(loggerNew),        /* tp_new */
        0,                                           /* tp_free */
        0,                                           /* tp_is_gc */
    };
}

bool
IcePy::initLogger(PyObject* module)
{
    if (PyType_Ready(&LoggerType) < 0)
    {
        return false;
    }
    PyTypeObject* type = &LoggerType; // Necessary to prevent GCC's strict-alias warnings.
    if (PyModule_AddObject(module, "Logger", reinterpret_cast<PyObject*>(type)) < 0)
    {
        return false;
    }

    return true;
}

void
IcePy::cleanupLogger()
{
    //
    // Python is about to exit; we need to remove the wrapper around the process logger.
    //
    Ice::setProcessLogger(nullptr);
}

PyObject*
IcePy::createLogger(const Ice::LoggerPtr& logger)
{
    LoggerObject* obj = loggerNew(&LoggerType, 0, 0);
    if (obj)
    {
        obj->logger = new Ice::LoggerPtr(logger);
    }
    return reinterpret_cast<PyObject*>(obj);
}

extern "C" PyObject*
IcePy_getProcessLogger(PyObject* /*self*/, PyObject* /*args*/)
{
    Ice::LoggerPtr logger;
    try
    {
        logger = Ice::getProcessLogger();
    }
    catch (...)
    {
        IcePy::setPythonException(current_exception());
        return nullptr;
    }

    //
    // The process logger can either be a C++ object (such as
    // the default logger supplied by the Ice run time), or a C++
    // wrapper around a Python implementation. If the latter, we
    // return it directly. Otherwise, we create a Python object
    // that delegates to the C++ object.
    //
    auto wrapper = dynamic_pointer_cast<LoggerWrapper>(logger);
    if (wrapper)
    {
        PyObject* obj = wrapper->getObject();
        Py_INCREF(obj);
        return obj;
    }

    return createLogger(logger);
}

extern "C" PyObject*
IcePy_setProcessLogger(PyObject* /*self*/, PyObject* args)
{
    PyObject* loggerType = lookupType("Ice.Logger");
    assert(loggerType);

    PyObject* logger;
    if (!PyArg_ParseTuple(args, "O!", loggerType, &logger))
    {
        return nullptr;
    }

    auto wrapper = make_shared<LoggerWrapper>(logger);
    try
    {
        Ice::setProcessLogger(wrapper);
    }
    catch (...)
    {
        IcePy::setPythonException(current_exception());
        return nullptr;
    }

    return Py_None;
}
