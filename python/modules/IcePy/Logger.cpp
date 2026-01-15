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

IcePy::LoggerWrapper::LoggerWrapper(PyObject* logger) : _logger(Py_NewRef(logger)) {}

void
IcePy::LoggerWrapper::print(const string& message)
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    PyObjectHandle tmp{PyObject_CallMethod(_logger.get(), "print", "s", message.c_str())};
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

    PyObjectHandle tmp{PyObject_CallMethod(_logger.get(), "getPrefix", nullptr)};
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
    auto* self = reinterpret_cast<LoggerObject*>(type->tp_alloc(type, 0));
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
        return Py_NewRef(wrapper->getObject());
    }

    return createLogger(clone);
}

static PyMethodDef LoggerMethods[] = {
    {"print", reinterpret_cast<PyCFunction>(loggerPrint), METH_VARARGS, PyDoc_STR("print(message: str) -> None")},
    {"trace",
     reinterpret_cast<PyCFunction>(loggerTrace),
     METH_VARARGS,
     PyDoc_STR("trace(category: str, message: str) -> None")},
    {"warning", reinterpret_cast<PyCFunction>(loggerWarning), METH_VARARGS, PyDoc_STR("warning(message: str) -> None")},
    {"error", reinterpret_cast<PyCFunction>(loggerError), METH_VARARGS, PyDoc_STR("error(message: str) -> None")},
    {"getPrefix", reinterpret_cast<PyCFunction>(loggerGetPrefix), METH_NOARGS, PyDoc_STR("getPrefix() -> str")},
    {"cloneWithPrefix",
     reinterpret_cast<PyCFunction>(loggerCloneWithPrefix),
     METH_VARARGS,
     PyDoc_STR("cloneWithPrefix(prefix: str) -> Logger")},
    {} /* sentinel */
};

namespace IcePy
{
    // clang-format off
    PyTypeObject LoggerType = {
        .ob_base = PyVarObject_HEAD_INIT(nullptr, 0)
        .tp_name = "IcePy.Logger",
        .tp_basicsize = sizeof(LoggerObject),
        .tp_dealloc = reinterpret_cast<destructor>(loggerDealloc),
        .tp_flags = Py_TPFLAGS_DEFAULT,
        .tp_doc = PyDoc_STR("IcePy.Logger"),
        .tp_methods = LoggerMethods,
        .tp_new = reinterpret_cast<newfunc>(loggerNew),
    };
    // clang-format on
}

bool
IcePy::initLogger(PyObject* module)
{
    if (PyType_Ready(&LoggerType) < 0)
    {
        return false;
    }

    if (PyModule_AddObject(module, "Logger", reinterpret_cast<PyObject*>(&LoggerType)) < 0)
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
    LoggerObject* obj = loggerNew(&LoggerType, nullptr, nullptr);
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
        return Py_NewRef(wrapper->getObject());
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
