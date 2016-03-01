// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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

//
// These macros replace Py_RETURN_FALSE and Py_RETURN TRUE. We use these
// instead of the standard ones in order to avoid GCC warnings about
// strict aliasing and type punning.
//
#define PyRETURN_FALSE return incFalse()
#define PyRETURN_TRUE return incTrue()

#define PyRETURN_BOOL(b) if(b) PyRETURN_TRUE; else PyRETURN_FALSE

namespace IcePy
{

//
// This should be used instead of Py_False to avoid GCC compiler warnings.
//
inline PyObject* getFalse()
{
#if PY_VERSION_HEX >= 0x03000000
    PyLongObject* i = &_Py_FalseStruct;
    return reinterpret_cast<PyObject*>(i);
#else
    PyIntObject* i = &_Py_ZeroStruct;
    return reinterpret_cast<PyObject*>(i);
#endif
}

//
// This should be used instead of Py_True to avoid GCC compiler warnings.
//
inline PyObject* getTrue()
{
#if PY_VERSION_HEX >= 0x03000000
    PyLongObject* i = &_Py_TrueStruct;
    return reinterpret_cast<PyObject*>(i);
#else
    PyIntObject* i = &_Py_TrueStruct;
    return reinterpret_cast<PyObject*>(i);
#endif
}

inline PyObject* incFalse()
{
    PyObject* f = getFalse();
    Py_INCREF(f);
    return f;
}

inline PyObject* incTrue()
{
    PyObject* t = getTrue();
    Py_INCREF(t);
    return t;
}

//
// Create a string object.
//
inline PyObject* createString(const std::string& str)
{
#if PY_VERSION_HEX >= 0x03000000
    //
    // PyUnicode_FromStringAndSize interprets the argument as UTF-8.
    //
    return PyUnicode_FromStringAndSize(str.c_str(), static_cast<Py_ssize_t>(str.size()));
#else
    return PyString_FromStringAndSize(str.c_str(), static_cast<Py_ssize_t>(str.size()));
#endif
}

//
// Obtain a string from a string object; None is also legal.
//
std::string getString(PyObject*);

//
// Verify that the object is a string; None is NOT legal.
//
inline bool checkString(PyObject* p)
{
#if PY_VERSION_HEX >= 0x03000000
    return PyUnicode_Check(p) ? true : false;
#else
    return PyString_Check(p) ? true : false;
#endif
}

//
// Validate and retrieve a string argument; None is also legal.
//
bool getStringArg(PyObject*, const std::string&, std::string&);

//
// Get the name of the current Python function.
//
std::string getFunction();

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

    PyObject* get() const;
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

    //
    // If the Python exception is SystemExit, act on it. May not return.
    //
    void checkSystemExit();

    PyObjectHandle ex;

private:

    void raiseLocalException();
    std::string getTraceback();
    std::string getTypeName();

    PyObjectHandle _type;
    PyObjectHandle _tb;
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
// Converts the interpreter's current exception into an Ice exception
// and throws it.
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
// Create a Python instance of Ice.ProtocolVersion.
//
PyObject* createProtocolVersion(const Ice::ProtocolVersion&);

//
// Create a Python instance of Ice.EncodingVersion.
//
PyObject* createEncodingVersion(const Ice::EncodingVersion&);

//
// Extracts the members of an encoding version.
//
bool getEncodingVersion(PyObject*, Ice::EncodingVersion&);

}

extern "C" PyObject* IcePy_stringVersion(PyObject*);
extern "C" PyObject* IcePy_intVersion(PyObject*);
extern "C" PyObject* IcePy_currentProtocol(PyObject*);
extern "C" PyObject* IcePy_currentProtocolEncoding(PyObject*);
extern "C" PyObject* IcePy_currentEncoding(PyObject*);
extern "C" PyObject* IcePy_protocolVersionToString(PyObject*, PyObject*);
extern "C" PyObject* IcePy_stringToProtocolVersion(PyObject*, PyObject*);
extern "C" PyObject* IcePy_encodingVersionToString(PyObject*, PyObject*);
extern "C" PyObject* IcePy_stringToEncodingVersion(PyObject*, PyObject*);
extern "C" PyObject* IcePy_generateUUID(PyObject*);

#endif
