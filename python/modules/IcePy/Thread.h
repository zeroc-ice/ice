// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEPY_THREAD_H
#define ICEPY_THREAD_H

#include <Config.h>
#include <Util.h>
#include <Ice/Initialize.h>
#include <IceUtil/Thread.h>
#include <IceUtil/Monitor.h>

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

    ThreadHook(PyObject*);

    virtual void start();
    virtual void stop();

    PyObject* getObject();

private:

    PyObjectHandle _threadNotification;
};
typedef IceUtil::Handle<ThreadHook> ThreadHookPtr;

//
// This class invokes a member function in a separate thread.
//
template<typename T>
class InvokeThread : public IceUtil::Thread
{
public:

    InvokeThread(const IceInternal::Handle<T>& target, void (T::*func)(void),
                 IceUtil::Monitor<IceUtil::Mutex>& monitor, bool& done) :
        _target(target), _func(func), _monitor(monitor), _done(done), _ex(0)
    {
    }

    ~InvokeThread()
    {
        delete _ex;
    }

    virtual void run()
    {
        try
        {
            (_target.get() ->* _func)();
        }
        catch(const Ice::Exception& ex)
        {
            _ex = ex.ice_clone();
        }

        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
        _done = true;
        _monitor.notify();
    }

    Ice::Exception* getException() const
    {
        return _ex;
    }

private:

    IceInternal::Handle<T> _target;
    void (T::*_func)(void);
    IceUtil::Monitor<IceUtil::Mutex>& _monitor;
    bool& _done;
    Ice::Exception* _ex;
};

}

#endif
