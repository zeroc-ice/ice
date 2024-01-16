//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICEPY_THREAD_H
#define ICEPY_THREAD_H

#include <Config.h>
#include <Util.h>
#include <Ice/Initialize.h>
#include <IceUtil/Thread.h>
#include <IceUtil/Monitor.h>

#include <thread>

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

//
// This class invokes a function in a separate thread.
//
class InvokeThread
{
public:

    InvokeThread(std::function<void()> func, std::mutex* mutex, std::condition_variable* cond, bool& done) :
        _func(func),
        _mutex(mutex),
        _cond(cond),
        _done(done),
        _ex(nullptr),
        _thread([this] { run(); })
    {
    }

    void run()
    {
        try
        {
            _func();
        }
        catch(const Ice::Exception& ex)
        {
            _ex = ex.ice_clone();
        }
        std::unique_lock lock(*_mutex);
        _done = true;
        _cond->notify_one();
    }

    ~InvokeThread()
    {
        _thread.join();
        delete _ex;
    }

    void join()
    {
        _thread.join();
    }

    Ice::Exception* getException() const
    {
        return _ex;
    }

private:
    std::function<void()> _func;
    std::mutex* _mutex;
    std::condition_variable* _cond;
    bool& _done;
    Ice::Exception* _ex;
    std::thread _thread;
};

}

#endif
