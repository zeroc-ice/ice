// Copyright (c) ZeroC, Inc.

#include "Util.h"
#include "Ice/DisableWarnings.h"
#include "Thread.h"
#include "slice2py/PythonUtil.h"

#include <compile.h>
#include <cstddef>
#include <frameobject.h>

using namespace std;
using namespace Slice::Python;

namespace IcePy
{
    template<typename T> bool setVersion(PyObject* p, const T& version)
    {
        PyObjectHandle major{PyLong_FromLong(version.major)};
        PyObjectHandle minor{PyLong_FromLong(version.minor)};
        if (!major.get() || !minor.get())
        {
            return false;
        }
        if (PyObject_SetAttrString(p, "major", major.get()) < 0 || PyObject_SetAttrString(p, "minor", minor.get()) < 0)
        {
            return false;
        }
        return true;
    }

    template<typename T> bool getVersion(PyObject* p, T& v)
    {
        PyObjectHandle major{getAttr(p, "major", false)};
        PyObjectHandle minor{getAttr(p, "minor", false)};
        if (major.get())
        {
            major = PyNumber_Long(major.get());
            if (!major.get())
            {
                PyErr_Format(PyExc_ValueError, "version major must be a numeric value");
                return false;
            }
            long m = PyLong_AsLong(major.get());
            if (m < 0 || m > 255)
            {
                PyErr_Format(PyExc_ValueError, "version major must be a value between 0 and 255");
                return false;
            }
            v.major = static_cast<uint8_t>(m);
        }
        else
        {
            v.major = 0;
        }

        if (minor.get())
        {
            major = PyNumber_Long(minor.get());
            if (!minor.get())
            {
                PyErr_Format(PyExc_ValueError, "version minor must be a numeric value");
                return false;
            }
            long m = PyLong_AsLong(minor.get());
            if (m < 0 || m > 255)
            {
                PyErr_Format(PyExc_ValueError, "version minor must be a value between 0 and 255");
                return false;
            }
            v.minor = static_cast<uint8_t>(m);
        }
        else
        {
            v.minor = 0;
        }
        return true;
    }

    template<typename T> PyObject* createVersion(const T& version, const char* type)
    {
        PyObject* versionType{lookupType(type)};

        PyObjectHandle obj{PyObject_CallObject(versionType, nullptr)};
        if (!obj.get())
        {
            return nullptr;
        }

        if (!setVersion<T>(obj.get(), version))
        {
            return nullptr;
        }

        return obj.release();
    }

    char Ice_EncodingVersion[] = "Ice.EncodingVersion";
}

string
IcePy::getString(PyObject* p)
{
    assert(p == Py_None || checkString(p));

    string str;
    if (p != Py_None)
    {
        PyObjectHandle bytes{PyUnicode_AsUTF8String(p)};
        if (bytes.get())
        {
            char* s{nullptr};
            Py_ssize_t sz{};
            PyBytes_AsStringAndSize(bytes.get(), &s, &sz);
            str.assign(s, static_cast<size_t>(sz));
        }
    }
    return str;
}

bool
IcePy::getStringArg(PyObject* p, const string& arg, string& val)
{
    if (checkString(p))
    {
        val = getString(p);
    }
    else if (p != Py_None)
    {
        string funcName = getFunction();
        PyErr_Format(PyExc_ValueError, "%s expects a string for argument '%s'", funcName.c_str(), arg.c_str());
        return false;
    }
    return true;
}

PyObject*
IcePy::getAttr(PyObject* obj, const string& attrib, bool allowNone)
{
    PyObject* v{PyObject_GetAttrString(obj, attrib.c_str())};
    if (v == Py_None)
    {
        if (!allowNone)
        {
            v = nullptr;
        }
    }
    else if (!v)
    {
        PyErr_Clear(); // PyObject_GetAttrString sets an error on failure.
    }

    return v;
}

string
IcePy::getFunction()
{
    //
    // Get name of current function.
    //
    PyFrameObject* f{PyEval_GetFrame()};
    PyObjectHandle code{getAttr(reinterpret_cast<PyObject*>(f), "f_code", false)};
    assert(code.get());
    PyObjectHandle func{getAttr(code.get(), "co_name", false)};
    assert(func.get());
    return getString(func.get());
}

IcePy::PyObjectHandle::PyObjectHandle(PyObject* p) : _p(p) {}

IcePy::PyObjectHandle::PyObjectHandle(const PyObjectHandle& p) : _p(p._p) { Py_XINCREF(_p); }

IcePy::PyObjectHandle::~PyObjectHandle()
{
    // The destructor can be called from Py_Exit at which point is no longer safe to call Python code.
    // But this should only happen for objects that have been already released.
    Py_XDECREF(_p);
}

IcePy::PyObjectHandle&
IcePy::PyObjectHandle::operator=(PyObject* p)
{
    if (p != _p)
    {
        Py_XDECREF(_p);
        _p = p;
    }
    return *this;
}

IcePy::PyObjectHandle&
IcePy::PyObjectHandle::operator=(const PyObjectHandle& p)
{
    if (this != &p)
    {
        Py_XDECREF(_p);
        _p = p._p;
        Py_XINCREF(_p);
    }
    return *this;
}

PyObject*
IcePy::PyObjectHandle::get() const
{
    return _p;
}

PyObject*
IcePy::PyObjectHandle::release()
{
    PyObject* result{_p};
    _p = nullptr;
    return result;
}

IcePy::PyException::PyException() { ex = PyErr_GetRaisedException(); }

IcePy::PyException::PyException(PyObject* raisedException)
{
    assert(raisedException);
    ex = Py_NewRef(raisedException);
}

namespace
{
    string getTypeName(PyObject* ex)
    {
        PyObject* cls{reinterpret_cast<PyObject*>(ex->ob_type)};
        IcePy::PyObjectHandle name{IcePy::getAttr(cls, "__name__", false)};
        assert(name.get());
        IcePy::PyObjectHandle mod{IcePy::getAttr(cls, "__module__", false)};
        assert(mod.get());
        string result = IcePy::getString(mod.get());
        result += ".";
        result += IcePy::getString(name.get());
        return result;
    }

    string createUnknownExceptionMessage(PyObject* ex)
    {
        ostringstream ostr;
        ostr << getTypeName(ex);
        IcePy::PyObjectHandle exStr{PyObject_Str(ex)};
        if (exStr.get() && IcePy::checkString(exStr.get()))
        {
            string message = IcePy::getString(exStr.get());
            if (!message.empty())
            {
                ostr << ": " << message;
            }
        }
        return ostr.str();
    }
}

void
IcePy::PyException::raise()
{
    assert(ex.get());
    PyObject* localExceptionType{lookupType("Ice.LocalException")};
    if (PyObject_IsInstance(ex.get(), localExceptionType))
    {
        string typeName = getTypeName(ex.get());

        PyObject* requestFailedExceptionType{lookupType("Ice.RequestFailedException")};

        if (PyObject_IsInstance(ex.get(), requestFailedExceptionType))
        {
            PyObjectHandle idAttr{getAttr(ex.get(), "id", false)};
            Ice::Identity id;
            if (idAttr.get())
            {
                IcePy::getIdentity(idAttr.get(), id);
            }
            PyObjectHandle facetAttr{getAttr(ex.get(), "facet", false)};
            string facet = getString(facetAttr.get());
            PyObjectHandle operationAttr{getAttr(ex.get(), "operation", false)};
            string operation = getString(operationAttr.get());

            if (typeName == "Ice.ObjectNotExistException")
            {
                throw Ice::ObjectNotExistException{
                    __FILE__,
                    __LINE__,
                    std::move(id),
                    std::move(facet),
                    std::move(operation)};
            }
            else if (typeName == "Ice.OperationNotExistException")
            {
                throw Ice::OperationNotExistException{
                    __FILE__,
                    __LINE__,
                    std::move(id),
                    std::move(facet),
                    std::move(operation)};
            }
            else if (typeName == "Ice.FacetNotExistException")
            {
                throw Ice::FacetNotExistException{
                    __FILE__,
                    __LINE__,
                    std::move(id),
                    std::move(facet),
                    std::move(operation)};
            }
        }

        IcePy::PyObjectHandle exStr{PyObject_Str(ex.get())};
        string message;
        if (exStr.get() && checkString(exStr.get()))
        {
            message = getString(exStr.get());
        }

        if (typeName == "Ice.UnknownLocalException")
        {
            throw Ice::UnknownLocalException{__FILE__, __LINE__, std::move(message)};
        }
        else if (typeName == "Ice.UnknownUserException")
        {
            throw Ice::UnknownUserException{__FILE__, __LINE__, std::move(message)};
        }
        else if (typeName == "Ice.UnknownException")
        {
            throw Ice::UnknownException{__FILE__, __LINE__, std::move(message)};
        }

        PyObject* dispatchExceptionType{lookupType("Ice.DispatchException")};
        if (PyObject_IsInstance(ex.get(), dispatchExceptionType))
        {
            PyObjectHandle replyStatusAttr{getAttr(ex.get(), "replyStatus", false)};
            Ice::ReplyStatus replyStatus{static_cast<uint8_t>(PyLong_AsLong(replyStatusAttr.get()))};

            throw Ice::DispatchException{__FILE__, __LINE__, replyStatus, std::move(message)};
        }

        throw Ice::UnknownLocalException{__FILE__, __LINE__, createUnknownExceptionMessage(ex.get())};
    }
    else
    {
        throw Ice::UnknownException{__FILE__, __LINE__, createUnknownExceptionMessage(ex.get())};
    }
}

void
IcePy::PyException::checkSystemExit()
{
    if (PyObject_IsInstance(ex.get(), PyExc_SystemExit))
    {
        handleSystemExit(ex.get()); // Does not return.
    }
}

bool
IcePy::listToStringSeq(PyObject* l, Ice::StringSeq& seq)
{
    assert(PyList_Check(l));

    Py_ssize_t sz{PyList_GET_SIZE(l)};
    for (Py_ssize_t i = 0; i < sz; ++i)
    {
        PyObject* item{PyList_GET_ITEM(l, i)};
        if (!item)
        {
            return false;
        }
        string str;
        if (checkString(item))
        {
            str = getString(item);
        }
        else if (item != Py_None)
        {
            PyErr_Format(PyExc_ValueError, "list element must be a string");
            return false;
        }
        seq.push_back(str);
    }

    return true;
}

bool
IcePy::stringSeqToList(const Ice::StringSeq& source, PyObject* target)
{
    assert(PyList_Check(target));

    for (const auto& value : source)
    {
        PyObjectHandle str{Py_BuildValue("s", value.c_str())};
        if (!str)
        {
            return false;
        }

        int status = PyList_Append(target, str.get());
        if (status < 0)
        {
            return false;
        }
    }
    return true;
}

bool
IcePy::tupleToStringSeq(PyObject* t, Ice::StringSeq& seq)
{
    assert(PyTuple_Check(t));

    int sz = static_cast<int>(PyTuple_GET_SIZE(t));
    for (int i = 0; i < sz; ++i)
    {
        PyObject* item{PyTuple_GET_ITEM(t, i)};
        if (!item)
        {
            return false;
        }

        string str;
        if (checkString(item))
        {
            str = getString(item);
        }
        else if (item != Py_None)
        {
            PyErr_Format(PyExc_ValueError, "tuple element must be a string");
            return false;
        }
        seq.push_back(str);
    }

    return true;
}

bool
IcePy::dictionaryToContext(PyObject* dict, Ice::Context& context)
{
    assert(PyDict_Check(dict));

    Py_ssize_t pos = 0;
    PyObject* key{nullptr};
    PyObject* value{nullptr};
    while (PyDict_Next(dict, &pos, &key, &value))
    {
        string keystr;
        if (checkString(key))
        {
            keystr = getString(key);
        }
        else if (key != Py_None)
        {
            PyErr_Format(PyExc_ValueError, "context key must be a string");
            return false;
        }

        string valuestr;
        if (checkString(value))
        {
            valuestr = getString(value);
        }
        else if (value != Py_None)
        {
            PyErr_Format(PyExc_ValueError, "context value must be a string");
            return false;
        }

        context.insert(Ice::Context::value_type(keystr, valuestr));
    }

    return true;
}

bool
IcePy::contextToDictionary(const Ice::Context& ctx, PyObject* dict)
{
    assert(PyDict_Check(dict));

    for (const auto& [key, value] : ctx)
    {
        PyObjectHandle pyKey{createString(key)};
        PyObjectHandle pyValue{createString(value)};
        if (!pyKey.get() || !pyValue.get())
        {
            return false;
        }
        if (PyDict_SetItem(dict, pyKey.get(), pyValue.get()) < 0)
        {
            return false;
        }
    }

    return true;
}

PyObject*
IcePy::lookupType(const string& typeName)
{
    string::size_type dot = typeName.rfind('.');
    assert(dot != string::npos);
    string moduleName = typeName.substr(0, dot);
    string name = typeName.substr(dot + 1);

    PyObjectHandle pyModule{PyImport_ImportModule(const_cast<char*>(moduleName.c_str()))};
    if (!pyModule.get())
    {
        PyErr_Print(); // Print full Python exception and traceback
        return nullptr;
    }

    PyObject* dict = PyModule_GetDict(pyModule.get());
    assert(dict);
    return PyDict_GetItemString(dict, const_cast<char*>(name.c_str()));
}

PyObject*
IcePy::createExceptionInstance(PyObject* type)
{
    assert(PyExceptionClass_Check(type));
    return PyObject_CallObject(type, nullptr);
}

namespace
{
    // This function takes ownership of each PyObject* in args.
    template<size_t N>
    PyObject*
    createPythonException(const char* typeId, std::array<PyObject*, N> args, bool fallbackToLocalException = false)
    {
        // Convert the exception's typeId to its mapped Python type by replacing "::Ice::" with "Ice.".
        // This function should only ever be called on Ice local exceptions which don't use 'python:identifier'.
        string result = typeId;
        assert(result.find("::Ice::") == 0);
        result.replace(0, 7, "Ice.");
        assert(result.find(':') == string::npos); // Assert that there weren't any intermediate scopes.

        PyObject* type{IcePy::lookupType(result)};
        if (!type)
        {
            if (fallbackToLocalException)
            {
                type = IcePy::lookupType("Ice.LocalException");
            }
            else
            {
                for (PyObject* pArg : args)
                {
                    Py_DECREF(pArg);
                }

                ostringstream os;
                os << "unable to create Python exception class for type ID " << typeId;
                return PyObject_CallFunction(PyExc_Exception, "s", os.str().c_str());
            }
        }
        IcePy::PyObjectHandle pArgs{PyTuple_New(N)};
        for (size_t i = 0; i < N; ++i)
        {
            // PyTuple_SetItem takes ownership of the args[i] reference.
            PyTuple_SetItem(pArgs.get(), static_cast<Py_ssize_t>(i), args[i]);
        }
        return PyObject_CallObject(type, pArgs.get());
    }
}

PyObject*
IcePy::convertException(std::exception_ptr exPtr)
{
    const char* const localExceptionTypeId = "::Ice::LocalException";

    // We cannot throw a C++ exception or raise a Python exception. If an error occurs while we are converting the
    // exception, we do our best to _return_ an appropriate Python exception.
    try
    {
        rethrow_exception(exPtr);
    }
    // First handle exceptions with extra fields we want to provide to Python users.
    catch (const Ice::AlreadyRegisteredException& ex)
    {
        std::array args{
            IcePy::createString(ex.kindOfObject()),
            IcePy::createString(ex.id()),
            IcePy::createString(ex.what())};
        return createPythonException(ex.ice_id(), args);
    }
    catch (const Ice::NotRegisteredException& ex)
    {
        std::array args{
            IcePy::createString(ex.kindOfObject()),
            IcePy::createString(ex.id()),
            IcePy::createString(ex.what())};
        return createPythonException(ex.ice_id(), args);
    }
    catch (const Ice::ConnectionAbortedException& ex)
    {
        std::array args{ex.closedByApplication() ? Py_True : Py_False, IcePy::createString(ex.what())};
        return createPythonException(ex.ice_id(), args);
    }
    catch (const Ice::ConnectionClosedException& ex)
    {
        std::array args{ex.closedByApplication() ? Py_True : Py_False, IcePy::createString(ex.what())};
        return createPythonException(ex.ice_id(), args);
    }
    catch (const Ice::RequestFailedException& ex)
    {
        std::array args{
            IcePy::createIdentity(ex.id()),
            IcePy::createString(ex.facet()),
            IcePy::createString(ex.operation()),
            IcePy::createString(ex.what())};
        return createPythonException(ex.ice_id(), args);
    }
    catch (const Ice::UnknownException& ex)
    {
        // The 3 Unknown exceptions can be constructed from just a message.
        std::array args{IcePy::createString(ex.what())};
        return createPythonException(ex.ice_id(), args);
    }
    catch (const Ice::DispatchException& ex)
    {
        std::array args{PyLong_FromLong(static_cast<int>(ex.replyStatus())), IcePy::createString(ex.what())};
        return createPythonException(ex.ice_id(), args);
    }
    // Then all other exceptions.
    catch (const Ice::LocalException& ex)
    {
        std::array args{IcePy::createString(ex.what())};
        return createPythonException(ex.ice_id(), args, true);
    }
    catch (const std::exception& ex)
    {
        std::array args{IcePy::createString(ex.what())};
        return createPythonException(localExceptionTypeId, args);
    }
    catch (...)
    {
        std::array args{IcePy::createString("unknown C++ exception")};
        return createPythonException(localExceptionTypeId, args);
    }
}

void
IcePy::setPythonException(std::exception_ptr ex)
{
    PyObjectHandle p{convertException(ex)};
    if (p.get())
    {
        setPythonException(p.get());
    }
}

void
IcePy::setPythonException(PyObject* ex)
{
    // PyErr_SetRaisedException steals a reference.
    PyErr_SetRaisedException(Py_NewRef(ex));
}

void
IcePy::throwPythonException()
{
    PyException ex;
    ex.raise();
}

void
IcePy::handleSystemExit(PyObject* ex)
{
    //
    // This code is similar to handle_system_exit in pythonrun.c.
    //
    PyObjectHandle code;
    if (PyExceptionInstance_Check(ex))
    {
        code = PyObjectHandle{getAttr(ex, "code", true)};
    }
    else
    {
        code = PyObjectHandle{Py_NewRef(ex)};
    }

    int status;
    if (PyLong_Check(code.get()))
    {
        status = static_cast<int>(PyLong_AsLong(code.get()));
    }
    else
    {
        PyObject_Print(code.get(), stderr, Py_PRINT_RAW);
        PySys_WriteStderr("\n");
        status = 1;
    }

    code = nullptr;
    Py_Exit(status);
}

PyObject*
IcePy::createIdentity(const Ice::Identity& ident)
{
    PyObject* identityType{lookupType("Ice.Identity")};

    PyObjectHandle obj{PyObject_CallObject(identityType, nullptr)};
    if (!obj.get())
    {
        return nullptr;
    }

    if (!setIdentity(obj.get(), ident))
    {
        return nullptr;
    }

    return obj.release();
}

bool
IcePy::checkIdentity(PyObject* p)
{
    PyObject* identityType{lookupType("Ice.Identity")};
    return PyObject_IsInstance(p, identityType) == 1;
}

bool
IcePy::setIdentity(PyObject* p, const Ice::Identity& ident)
{
    assert(checkIdentity(p));
    PyObjectHandle name{createString(ident.name)};
    PyObjectHandle category{createString(ident.category)};
    if (!name.get() || !category.get())
    {
        return false;
    }
    if (PyObject_SetAttrString(p, "name", name.get()) < 0 || PyObject_SetAttrString(p, "category", category.get()) < 0)
    {
        return false;
    }
    return true;
}

bool
IcePy::getIdentity(PyObject* p, Ice::Identity& ident)
{
    assert(checkIdentity(p));
    PyObjectHandle name{getAttr(p, "name", true)};
    PyObjectHandle category{getAttr(p, "category", true)};
    if (name.get())
    {
        if (!checkString(name.get()))
        {
            PyErr_Format(PyExc_ValueError, "identity name must be a string");
            return false;
        }
        ident.name = getString(name.get());
    }
    if (category.get())
    {
        if (!checkString(category.get()))
        {
            PyErr_Format(PyExc_ValueError, "identity category must be a string");
            return false;
        }
        ident.category = getString(category.get());
    }
    return true;
}

PyObject*
IcePy::createEncodingVersion(const Ice::EncodingVersion& v)
{
    return createVersion<Ice::EncodingVersion>(v, Ice_EncodingVersion);
}

bool
IcePy::getEncodingVersion(PyObject* p, Ice::EncodingVersion& v)
{
    if (!getVersion<Ice::EncodingVersion>(p, v))
    {
        return false;
    }

    return true;
}

PyObject*
IcePy::callMethod(PyObject* obj, const string& name, PyObject* arg1, PyObject* arg2)
{
    PyObjectHandle method{PyObject_GetAttrString(obj, const_cast<char*>(name.c_str()))};
    if (!method.get())
    {
        return nullptr;
    }
    return callMethod(method.get(), arg1, arg2);
}

PyObject*
IcePy::callMethod(PyObject* method, PyObject* arg1, PyObject* arg2)
{
    PyObjectHandle args;
    if (arg1 && arg2)
    {
        args = PyObjectHandle{PyTuple_New(2)};
        if (!args.get())
        {
            return nullptr;
        }

        PyTuple_SET_ITEM(args.get(), 0, Py_NewRef(arg1));
        PyTuple_SET_ITEM(args.get(), 1, Py_NewRef(arg2));
    }
    else if (arg1)
    {
        args = PyObjectHandle{PyTuple_New(1)};
        if (!args.get())
        {
            return nullptr;
        }

        PyTuple_SET_ITEM(args.get(), 0, Py_NewRef(arg1));
    }
    else
    {
        assert(!arg1 && !arg2);
        args = PyTuple_New(0);
        if (!args.get())
        {
            return nullptr;
        }
    }
    return PyObject_Call(method, args.get(), nullptr);
}

extern "C" PyObject*
IcePy_stringVersion(PyObject* /*self*/, PyObject* /*args*/)
{
    string s = ICE_STRING_VERSION;
    return IcePy::createString(s);
}

extern "C" PyObject*
IcePy_intVersion(PyObject* /*self*/, PyObject* /*args*/)
{
    return PyLong_FromLong(ICE_INT_VERSION);
}
