// Copyright (c) ZeroC, Inc.

#include "Executor.h"
#include "Connection.h"
#include "Ice/Initialize.h"
#include "Thread.h"

#include <functional>
#include <memory>

using namespace std;
using namespace IcePy;

namespace IcePy
{
    struct ExecutorCallObject
    {
        PyObject_HEAD function<void()>* call;
    };
}

extern "C" void
executorCallDealloc(ExecutorCallObject* self)
{
    delete self->call;
    Py_TYPE(self)->tp_free(reinterpret_cast<PyObject*>(self));
}

extern "C" PyObject*
executorCallInvoke(ExecutorCallObject* self, PyObject* /*args*/, PyObject* /*kwds*/)
{
    try
    {
        AllowThreads allowThreads; // Release Python's global interpreter lock during blocking calls.
        (*self->call)();
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    return Py_None;
}

namespace IcePy
{
    PyTypeObject ExecutorCallType = {
        /* The ob_type field must be initialized in the module init function
         * to be portable to Windows without using C++. */
        PyVarObject_HEAD_INIT(0, 0) "IcePy.ExecutorCall", /* tp_name */
        sizeof(ExecutorCallObject),                       /* tp_basicsize */
        0,                                                /* tp_itemsize */
        /* methods */
        reinterpret_cast<destructor>(executorCallDealloc), /* tp_dealloc */
        0,                                                 /* tp_print */
        0,                                                 /* tp_getattr */
        0,                                                 /* tp_setattr */
        0,                                                 /* tp_reserved */
        0,                                                 /* tp_repr */
        0,                                                 /* tp_as_number */
        0,                                                 /* tp_as_sequence */
        0,                                                 /* tp_as_mapping */
        0,                                                 /* tp_hash */
        reinterpret_cast<ternaryfunc>(executorCallInvoke), /* tp_call */
        0,                                                 /* tp_str */
        0,                                                 /* tp_getattro */
        0,                                                 /* tp_setattro */
        0,                                                 /* tp_as_buffer */
        Py_TPFLAGS_DEFAULT,                                /* tp_flags */
        0,                                                 /* tp_doc */
        0,                                                 /* tp_traverse */
        0,                                                 /* tp_clear */
        0,                                                 /* tp_richcompare */
        0,                                                 /* tp_weaklistoffset */
        0,                                                 /* tp_iter */
        0,                                                 /* tp_iternext */
        0,                                                 /* tp_methods */
        0,                                                 /* tp_members */
        0,                                                 /* tp_getset */
        0,                                                 /* tp_base */
        0,                                                 /* tp_dict */
        0,                                                 /* tp_descr_get */
        0,                                                 /* tp_descr_set */
        0,                                                 /* tp_dictoffset */
        0,                                                 /* tp_init */
        0,                                                 /* tp_alloc */
        0,                                                 /* tp_new */
        0,                                                 /* tp_free */
        0,                                                 /* tp_is_gc */
    };
}

bool
IcePy::initExecutor(PyObject* module)
{
    if (PyType_Ready(&ExecutorCallType) < 0)
    {
        return false;
    }
    PyTypeObject* type = &ExecutorCallType; // Necessary to prevent GCC's strict-alias warnings.
    if (PyModule_AddObject(module, "ExecutorCall", reinterpret_cast<PyObject*>(type)) < 0)
    {
        return false;
    }

    return true;
}

IcePy::Executor::Executor(PyObject* executor) : _executor(executor)
{
    if (!PyCallable_Check(executor))
    {
        throw Ice::InitializationException(__FILE__, __LINE__, "executor must be a callable");
    }

    Py_INCREF(executor);
}

void
IcePy::Executor::setCommunicator(const Ice::CommunicatorPtr& communicator)
{
    _communicator = communicator;
}

void
IcePy::Executor::execute(function<void()> call, const Ice::ConnectionPtr& con)
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    auto obj = reinterpret_cast<ExecutorCallObject*>(ExecutorCallType.tp_alloc(&ExecutorCallType, 0));
    if (!obj)
    {
        return;
    }

    obj->call = new function<void()>(std::move(call));
    PyObjectHandle c{createConnection(con, _communicator)};
    PyObjectHandle tmp{PyObject_CallFunction(_executor.get(), "OO", obj, c.get())};
    Py_DECREF(reinterpret_cast<PyObject*>(obj));
    if (!tmp.get())
    {
        throwPythonException();
    }
}
