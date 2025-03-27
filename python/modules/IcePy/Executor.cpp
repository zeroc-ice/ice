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
        // Release Python's global interpreter lock during blocking calls.
        AllowThreads allowThreads;
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
    // clang-format off
    PyTypeObject ExecutorCallType = {
        .ob_base = PyVarObject_HEAD_INIT(nullptr, 0)
        .tp_name = "IcePy.ExecutorCall",
        .tp_basicsize = sizeof(ExecutorCallObject),
        .tp_dealloc = reinterpret_cast<destructor>(executorCallDealloc),
        .tp_call = reinterpret_cast<ternaryfunc>(executorCallInvoke),
        .tp_flags = Py_TPFLAGS_DEFAULT,
    };
    // clang-format on
}

bool
IcePy::initExecutor(PyObject* module)
{
    if (PyType_Ready(&ExecutorCallType) < 0)
    {
        return false;
    }

    if (PyModule_AddObject(module, "ExecutorCall", reinterpret_cast<PyObject*>(&ExecutorCallType)) < 0)
    {
        return false;
    }

    return true;
}

IcePy::Executor::Executor(PyObject* executor) : _executor(Py_NewRef(executor))
{
    if (!PyCallable_Check(executor))
    {
        throw Ice::InitializationException(__FILE__, __LINE__, "executor must be a callable");
    }
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
