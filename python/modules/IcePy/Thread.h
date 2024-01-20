//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICEPY_THREAD_H
#define ICEPY_THREAD_H

#include <Config.h>
#include <Util.h>
#include <Ice/Initialize.h>

namespace IcePy
{

//
// Release Python's Global Interpreter Lock during potentially time-consuming
// (and non-Python related) work.
//
class AllowThreads
{
public:

    AllowThreads();
    ~AllowThreads();

private:

    PyThreadState* _state;
};

//
// Ensure that the current thread is capable of calling into Python.
//
class AdoptThread
{
public:

    AdoptThread();
    ~AdoptThread();

private:

    PyGILState_STATE _state;
};

//
// ThreadHook ensures that every Ice thread is ready to invoke the Python API.
// It also acts as a wrapper for an optional ThreadNotification object.
//
class ThreadHook : public Ice::ThreadNotification
{
public:

    ThreadHook(PyObject*, PyObject*, PyObject*);

    virtual void start();
    virtual void stop();

private:

    PyObjectHandle _threadNotification;
    PyObjectHandle _threadStart;
    PyObjectHandle _threadStop;
};
typedef IceUtil::Handle<ThreadHook> ThreadHookPtr;

}

#endif
