// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEPY_UTIL_H
#define ICEPY_UTIL_H

#include <Config.h>
#include <Ice/BuiltinSequences.h>
#include <Ice/Current.h>
#include <Ice/Exception.h>
#include <IceUtil/Thread.h>
#include <IceUtil/Monitor.h>
#include <IceUtil/Mutex.h>

namespace IcePy
{

//
// Invokes Py_DECREF on a Python object.
//
class PyObjectHandle
{
public:

    PyObjectHandle(PyObject* = NULL);
    PyObjectHandle(const PyObjectHandle&);
    ~PyObjectHandle();

    void operator=(PyObject*);
    void operator=(const PyObjectHandle&);

    PyObject* get();
    PyObject* release();

private:

    PyObject* _p;
};

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
// Convert Ice::StringSeq to and from a Python list.
//
bool listToStringSeq(PyObject*, Ice::StringSeq&);
bool stringSeqToList(const Ice::StringSeq&, PyObject*);

//
// Convert Ice::Context to and from a Python dictionary.
//
bool dictionaryToContext(PyObject*, Ice::Context&);
bool contextToDictionary(const Ice::Context&, PyObject*);

//
// Returns a borrowed reference to the Python type object corresponding
// to the given Python type name.
//
PyObject* lookupType(const std::string&);

//
// Returns the current Python exception.
//
PyObject* getPythonException(bool = true);

//
// Creates an exception instance of the given type.
//
PyObject* createExceptionInstance(PyObject*);

//
// Converts an Ice exception into a Python exception.
//
PyObject* convertException(const Ice::Exception&);

//
// Converts an Ice exception into a Python exception and sets it in the Python environment.
//
void setPythonException(const Ice::Exception&);

//
// Converts a Python exception into an Ice exception and throws it.
//
void throwPythonException(PyObject* = NULL);

//
// Handle the SystemExit exception.
//
void handleSystemExit(PyObject*);

//
// Create a Python instance of Ice.Identity.
//
PyObject* createIdentity(const Ice::Identity&);

//
// Verify that the object is Ice.Identity.
//
bool checkIdentity(PyObject*);

//
// Assign values to members of an instance of Ice.Identity.
//
bool setIdentity(PyObject*, const Ice::Identity&);

//
// Extract the members of Ice.Identity.
//
bool getIdentity(PyObject*, Ice::Identity&);

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

extern "C" PyObject* IcePy_identityToString(PyObject*, PyObject*);
extern "C" PyObject* IcePy_stringToIdentity(PyObject*, PyObject*);
extern "C" PyObject* IcePy_generateUUID(PyObject*);

#endif
