// Copyright (c) ZeroC, Inc.

#ifndef ICEPY_UTIL_H
#define ICEPY_UTIL_H

#include "Config.h"
#include "Ice/Ice.h"

namespace IcePy
{
    //
    // Create a string object.
    //
    inline PyObject* createString(std::string_view str)
    {
        //
        // PyUnicode_FromStringAndSize interprets the argument as UTF-8.
        //
        return PyUnicode_FromStringAndSize(str.data(), static_cast<Py_ssize_t>(str.size()));
    }

    //
    // Obtain a string from a string object; None is also legal.
    //
    std::string getString(PyObject*);

    //
    // Verify that the object is a string; None is NOT legal.
    //
    inline bool checkString(PyObject* p) { return PyUnicode_Check(p) ? true : false; }

    //
    // Validate and retrieve a string argument; None is also legal.
    //
    bool getStringArg(PyObject*, const std::string&, std::string&);

    //
    // Get an object attribute having the given name. If allowNone is true, a value of Py_None is allowed, otherwise
    // a value of Py_None is treated as if the attribute is undefined (i.e., the function returns nil). The caller
    // must release the reference to the returned object.
    //
    PyObject* getAttr(PyObject*, const std::string&, bool allowNone);

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
        explicit PyObjectHandle(PyObject* = nullptr);
        PyObjectHandle(const PyObjectHandle&);
        ~PyObjectHandle();

        PyObjectHandle& operator=(PyObject*);
        PyObjectHandle& operator=(const PyObjectHandle&);

        operator bool() const { return _p != nullptr; }

        [[nodiscard]] PyObject* get() const;
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
        // Converts the Python exception into one of the 6 special Ice local exceptions.
        //
        void raise();

        //
        // If the Python exception is SystemExit, act on it. May not return.
        //
        void checkSystemExit();

        PyObjectHandle ex;
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
    // Converts a C++ exception into a Python exception.
    //
    PyObject* convertException(std::exception_ptr);

    //
    // Converts a C++ exception into a Python exception and sets it in the Python environment.
    //
    void setPythonException(std::exception_ptr);

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
    // Create a Python instance of Ice.EncodingVersion.
    //
    PyObject* createEncodingVersion(const Ice::EncodingVersion&);

    //
    // Extracts the members of an encoding version.
    //
    bool getEncodingVersion(PyObject*, Ice::EncodingVersion&);

    //
    // Call a Python method.
    //
    PyObject* callMethod(PyObject*, const std::string&, PyObject* = nullptr, PyObject* = nullptr);
    PyObject* callMethod(PyObject*, PyObject* = nullptr, PyObject* = nullptr);

    /// Returns true if the current thread is the main thread.
    ///
    /// @return True if the current thread is the main thread, false otherwise.
    bool isMainThread();
}

extern "C" PyObject* IcePy_stringVersion(PyObject*, PyObject*);
extern "C" PyObject* IcePy_intVersion(PyObject*, PyObject*);

#endif
