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
#include <ThreadNotification.h>

using namespace std;
using namespace IcePy;

IcePy::ThreadNotificationWrapper::ThreadNotificationWrapper(PyObject* threadNotification) :
    _threadNotification(threadNotification)
{
    Py_INCREF(threadNotification);
}

void
IcePy::ThreadNotificationWrapper::start()
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    PyObjectHandle tmp = PyObject_CallMethod(_threadNotification.get(), STRCAST("start"), 0);
    if(!tmp.get())
    {
        throwPythonException();
    }
}

void
IcePy::ThreadNotificationWrapper::stop()
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    PyObjectHandle tmp = PyObject_CallMethod(_threadNotification.get(), STRCAST("stop"), 0);
    if(!tmp.get())
    {
        throwPythonException();
    }
}

PyObject*
IcePy::ThreadNotificationWrapper::getObject()
{
    return _threadNotification.get();
}
