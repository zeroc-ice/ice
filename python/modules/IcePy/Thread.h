// Copyright (c) ZeroC, Inc.

#ifndef ICEPY_THREAD_H
#define ICEPY_THREAD_H

#include "Config.h"
#include "Ice/Ice.h"
#include "Util.h"

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

    // ThreadHook ensures that every Ice thread is ready to invoke the Python API. It also acts as a wrapper thread
    // notification callbacks.
    class ThreadHook final
    {
    public:
        ThreadHook(PyObject*, PyObject*);

        void start();
        void stop();

    private:
        PyObjectHandle _threadStart;
        PyObjectHandle _threadStop;
    };
}

#endif
