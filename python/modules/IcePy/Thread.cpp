// Copyright (c) ZeroC, Inc.

#include "Thread.h"

using namespace std;
using namespace IcePy;

IcePy::AllowThreads::AllowThreads() { _state = PyEval_SaveThread(); }

IcePy::AllowThreads::~AllowThreads() { PyEval_RestoreThread(_state); }

IcePy::AdoptThread::AdoptThread() { _state = PyGILState_Ensure(); }

IcePy::AdoptThread::~AdoptThread() { PyGILState_Release(_state); }

IcePy::ThreadHook::ThreadHook(PyObject* threadStart, PyObject* threadStop)
    : _threadStart(threadStart),
      _threadStop(threadStop)
{
    if (threadStart && !PyCallable_Check(threadStart))
    {
        throw Ice::InitializationException(__FILE__, __LINE__, "threadStart must be a callable");
    }

    if (threadStop && !PyCallable_Check(threadStop))
    {
        throw Ice::InitializationException(__FILE__, __LINE__, "threadStop must be a callable");
    }

    // Increment the reference count of the Python objects to ensure they are not garbage collected.
    // The reference count will be decremented in the destructor of PyObjectHandle, which holds them.
    Py_XINCREF(threadStart);
    Py_XINCREF(threadStop);
}

void
IcePy::ThreadHook::start()
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.
    if (_threadStart.get())
    {
        PyObjectHandle args{PyTuple_New(0)};
        PyObjectHandle tmp{PyObject_Call(_threadStart.get(), args.get(), nullptr)};
        if (!tmp.get())
        {
            throwPythonException();
        }
    }
}

void
IcePy::ThreadHook::stop()
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.
    if (_threadStop.get())
    {
        PyObjectHandle args{PyTuple_New(0)};
        PyObjectHandle tmp{PyObject_Call(_threadStop.get(), args.get(), nullptr)};
        if (!tmp.get())
        {
            throwPythonException();
        }
    }
}
