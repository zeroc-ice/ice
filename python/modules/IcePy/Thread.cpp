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

IcePy::ThreadHook::ThreadHook(PyObject* threadNotification) :
    _threadNotification(threadNotification)
{
    Py_INCREF(threadNotification);
}

void
IcePy::ThreadHook::start()
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    PyObjectHandle tmp = PyObject_CallMethod(_threadNotification.get(), STRCAST("start"), 0);
    if(!tmp.get())
    {
        throwPythonException();
    }
}

void
IcePy::ThreadHook::stop()
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    PyObjectHandle tmp = PyObject_CallMethod(_threadNotification.get(), STRCAST("stop"), 0);
    if(!tmp.get())
    {
        throwPythonException();
    }
}

PyObject*
IcePy::ThreadHook::getObject()
{
    return _threadNotification.get();
}
