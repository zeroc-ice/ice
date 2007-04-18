// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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

//
// These macros replace Py_RETURN_FALSE and Py_RETURN TRUE. We use these
// instead of the standard ones in order to avoid GCC warnings about
// strict aliasing and type punning.
//
#define PyRETURN_FALSE return Py_INCREF(getFalse()), getFalse()
#define PyRETURN_TRUE return Py_INCREF(getTrue()), getTrue()

namespace IcePy
{

//
// This should be used instead of Py_False to avoid GCC compiler warnings.
//
inline PyObject* getFalse()
{
    PyIntObject* i = &_Py_ZeroStruct;
    return reinterpret_cast<PyObject*>(i);
}

//
// This should be used instead of Py_True to avoid GCC compiler warnings.
//
inline PyObject* getTrue()
{
    PyIntObject* i = &_Py_TrueStruct;
    return reinterpret_cast<PyObject*>(i);
}

//
// Invokes Py_DECREF on a Python object.
//
class PyObjectHandle
{
public:

    PyObjectHandle(PyObject* = 0);
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
// Manages the interpreter's exception.
//
class PyException
{
public:

    //
    // Retrieves the interpreter's current exception.
    //
    PyException();

    //
    // Uses the given exception.
    //
    PyException(PyObject*);

    //
    // Convert the Python exception to its C++ equivalent.
    //
    void raise();

    PyObjectHandle ex;

private:

    void raiseLocalException();
    std::string getTraceback();
    std::string getTypeName();

    PyObjectHandle _type;
    PyObjectHandle _tb;
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
// Convert a tuple to Ice::StringSeq.
//
bool tupleToStringSeq(PyObject*, Ice::StringSeq&);

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
// Sets an exception in the Python environment.
//
void setPythonException(PyObject*);

//
// Converts a Python exception into an Ice exception and throws it.
// If no exception is provided, the interpreter's current exception
// is obtained. The second argument is an optional traceback object.
//
void throwPythonException();

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
