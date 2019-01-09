// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#include <Thread.h>

using namespace std;
using namespace IcePy;

IcePy::AllowThreads::AllowThreads()
{
    _state = PyEval_SaveThread();
}

IcePy::AllowThreads::~AllowThreads()
{
    PyEval_RestoreThread(_state);
}

IcePy::AdoptThread::AdoptThread()
{
    _state = PyGILState_Ensure();
}

IcePy::AdoptThread::~AdoptThread()
{
    PyGILState_Release(_state);
}

IcePy::ThreadHook::ThreadHook(PyObject* threadNotification, PyObject* threadStart, PyObject* threadStop) :
    _threadNotification(threadNotification), _threadStart(threadStart), _threadStop(threadStop)
{
    if(threadNotification)
    {
        if(!PyObject_HasAttrString(threadNotification, STRCAST("start")) ||
           !PyObject_HasAttrString(threadNotification, STRCAST("stop")))
        {
            throw Ice::InitializationException(__FILE__, __LINE__,
                "threadNotification object must have 'start' and 'stop' methods");
        }

    }

    if(threadStart && !PyCallable_Check(threadStart))
    {
        throw Ice::InitializationException(__FILE__, __LINE__, "threadStart must be a callable");
    }

    if(threadStop && !PyCallable_Check(threadStop))
    {
        throw Ice::InitializationException(__FILE__, __LINE__, "threadStop must be a callable");
    }

    Py_XINCREF(threadNotification);
    Py_XINCREF(threadStart);
    Py_XINCREF(threadStop);
}

void
IcePy::ThreadHook::start()
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    if(_threadNotification.get())
    {
        PyObjectHandle tmp = PyObject_CallMethod(_threadNotification.get(), STRCAST("start"), 0);
        if(!tmp.get())
        {
            throwPythonException();
        }
    }
    if(_threadStart.get())
    {
        PyObjectHandle args = PyTuple_New(0);
        PyObjectHandle tmp = PyObject_Call(_threadStart.get(), args.get(), 0);
        if(!tmp.get())
        {
            throwPythonException();
        }
    }
}

void
IcePy::ThreadHook::stop()
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    if(_threadNotification.get())
    {
        PyObjectHandle tmp = PyObject_CallMethod(_threadNotification.get(), STRCAST("stop"), 0);
        if(!tmp.get())
        {
            throwPythonException();
        }
    }
    if(_threadStop.get())
    {
        PyObjectHandle args = PyTuple_New(0);
        PyObjectHandle tmp = PyObject_Call(_threadStop.get(), args.get(), 0);
        if(!tmp.get())
        {
            throwPythonException();
        }
    }
}
