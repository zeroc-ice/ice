// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifdef _WIN32
#   include <IceUtil/Config.h>
#endif
#include <Util.h>
#include <IceUtil/DisableWarnings.h>
#include <Ice/LocalException.h>
#include <Ice/Protocol.h>
#include <IceUtil/UUID.h>
#include <Slice/PythonUtil.h>
#include <compile.h>
#include <frameobject.h>

using namespace std;
using namespace Slice::Python;

namespace IcePy
{

bool
checkIsInstance(PyObject* p, const char* type)
{
    PyObject* pyType = lookupType(type);
    return PyObject_IsInstance(p, pyType) == 1;
}

template<typename T> bool
setVersion(PyObject* p, const T& version, const char* type)
{
    assert(checkIsInstance(p, type));

    PyObjectHandle major = PyLong_FromLong(version.major);
    PyObjectHandle minor = PyLong_FromLong(version.minor);
    if(!major.get() || !minor.get())
    {
        return false;
    }
    if(PyObject_SetAttrString(p, STRCAST("major"), major.get()) < 0 ||
       PyObject_SetAttrString(p, STRCAST("minor"), minor.get()) < 0)
    {
        return false;
    }
    return true;
}

template<typename T> bool
getVersion(PyObject* p, T& v, const char* type)
{
    assert(checkIsInstance(p, type));
    PyObjectHandle major = PyObject_GetAttrString(p, STRCAST("major"));
    PyObjectHandle minor = PyObject_GetAttrString(p, STRCAST("minor"));
    if(major.get())
    {
        major = PyNumber_Long(major.get());
        if(!major.get())
        {
            PyErr_Format(PyExc_ValueError, STRCAST("version major must be a numeric value"));
            return false;
        }
        long m = PyLong_AsLong(major.get());
        if(m < 0 || m > 255)
        {
            PyErr_Format(PyExc_ValueError, STRCAST("version major must be a value between 0 and 255"));
            return false;
        }
        v.major = static_cast<Ice::Byte>(m);
    }
    if(minor.get())
    {
        major = PyNumber_Long(minor.get());
        if(!minor.get())
        {
            PyErr_Format(PyExc_ValueError, STRCAST("version minor must be a numeric value"));
            return false;
        }
        long m = PyLong_AsLong(minor.get());
        if(m < 0 || m > 255)
        {
            PyErr_Format(PyExc_ValueError, STRCAST("version minor must be a value between 0 and 255"));
            return false;
        }
        v.minor = static_cast<Ice::Byte>(m);
    }
    return true;
}

template<typename T> PyObject*
createVersion(const T& version, const char* type)
{
    PyObject* versionType = lookupType(type);

    PyObjectHandle obj = PyObject_CallObject(versionType, 0);
    if(!obj.get())
    {
        return 0;
    }

    if(!setVersion<T>(obj.get(), version, type))
    {
        return 0;
    }

    return obj.release();
}

template<typename T> PyObject*
versionToString(PyObject* args, const char* type)
{
    PyObject* versionType = IcePy::lookupType(type);
    PyObject* p;
    if(!PyArg_ParseTuple(args, STRCAST("O!"), versionType, &p))
    {
        return NULL;
    }

    T v;
    if(!getVersion<T>(p, v, type))
    {
        return NULL;
    }

    string s;
    try
    {
        s = IceInternal::versionToString<T>(v);
    }
    catch(const Ice::Exception& ex)
    {
        IcePy::setPythonException(ex);
        return NULL;
    }
    return createString(s);
}

template<typename T> PyObject*
stringToVersion(PyObject* args, const char* type)
{
    char* str;
    if(!PyArg_ParseTuple(args, STRCAST("s"), &str))
    {
        return NULL;
    }

    T v;
    try
    {
        v = IceInternal::stringToVersion<T>(str);
    }
    catch(const Ice::Exception& ex)
    {
        IcePy::setPythonException(ex);
        return NULL;
    }

    return createVersion<T>(v, type);
}

char Ice_ProtocolVersion[] = "Ice.ProtocolVersion";
char Ice_EncodingVersion[] = "Ice.EncodingVersion";

}

string
IcePy::getString(PyObject* p)
{
    assert(p == Py_None || checkString(p));

    string str;
    if(p != Py_None)
    {
#if PY_VERSION_HEX >= 0x03000000
        PyObjectHandle bytes = PyUnicode_AsUTF8String(p);
        if(bytes.get())
        {
            char* s;
            Py_ssize_t sz;
            PyBytes_AsStringAndSize(bytes.get(), &s, &sz);
            str.assign(s, sz);
        }
#else
        str.assign(PyString_AS_STRING(p), PyString_GET_SIZE(p));
#endif
    }
    return str;
}

bool
IcePy::getStringArg(PyObject* p, const string& arg, string& val)
{
    if(checkString(p))
    {
        val = getString(p);
    }
    else if(p != Py_None)
    {
        string funcName = getFunction();
        PyErr_Format(PyExc_ValueError, STRCAST("%s expects a string for argument '%s'"), funcName.c_str(), arg.c_str());
        return false;
    }
    return true;
}

string
IcePy::getFunction()
{
    //
    // Get name of current function.
    //
    PyFrameObject *f = PyThreadState_GET()->frame;
    PyObjectHandle code = PyObject_GetAttrString(reinterpret_cast<PyObject*>(f), STRCAST("f_code"));
    assert(code.get());
    PyObjectHandle func = PyObject_GetAttrString(code.get(), STRCAST("co_name"));
    assert(func.get());
    return getString(func.get());
}

IcePy::PyObjectHandle::PyObjectHandle(PyObject* p) :
    _p(p)
{
}

IcePy::PyObjectHandle::PyObjectHandle(const PyObjectHandle& p) :
    _p(p._p)
{
    Py_XINCREF(_p);
}

IcePy::PyObjectHandle::~PyObjectHandle()
{
    if(_p)
    {
        Py_DECREF(_p);
    }
}

void
IcePy::PyObjectHandle::operator=(PyObject* p)
{
    if(_p)
    {
        Py_DECREF(_p);
    }
    _p = p;
}

void
IcePy::PyObjectHandle::operator=(const PyObjectHandle& p)
{
    Py_XDECREF(_p);
    _p = p._p;
    Py_XINCREF(_p);
}

PyObject*
IcePy::PyObjectHandle::get() const
{
    return _p;
}

PyObject*
IcePy::PyObjectHandle::release()
{
    PyObject* result = _p;
    _p = 0;
    return result;
}

IcePy::PyException::PyException()
{
    PyObject* type;
    PyObject* e;
    PyObject* tb;

    PyErr_Fetch(&type, &e, &tb); // PyErr_Fetch clears the exception.
    PyErr_NormalizeException(&type, &e, &tb);

    _type = type;
    ex = e;
    _tb = tb;
}

IcePy::PyException::PyException(PyObject* p)
{
    ex = p;
    Py_XINCREF(p);
}

void
IcePy::PyException::raise()
{
    assert(ex.get());

    PyObject* userExceptionType = lookupType("Ice.UserException");
    PyObject* localExceptionType = lookupType("Ice.LocalException");

    if(PyObject_IsInstance(ex.get(), userExceptionType))
    {
        Ice::UnknownUserException e(__FILE__, __LINE__);
        string tb = getTraceback();
        if(!tb.empty())
        {
            e.unknown = tb;
        }
        else
        {
            PyObjectHandle name = PyObject_CallMethod(ex.get(), STRCAST("ice_name"), 0);
            PyErr_Clear();
            if(!name.get())
            {
                e.unknown = getTypeName();
            }
            else
            {
                e.unknown = getString(name.get());
            }
        }
        throw e;
    }
    else if(PyObject_IsInstance(ex.get(), localExceptionType))
    {
        raiseLocalException();
    }
    else
    {
        Ice::UnknownException e(__FILE__, __LINE__);
        string tb = getTraceback();
        if(!tb.empty())
        {
            e.unknown = tb;
        }
        else
        {
            ostringstream ostr;

            ostr << getTypeName();

            IcePy::PyObjectHandle msg = PyObject_Str(ex.get());
            if(msg.get())
            {
                string s = getString(msg.get());
                if(!s.empty())
                {
                    ostr << ": " << s;
                }
            }

            e.unknown = ostr.str();
        }
        throw e;
    }
}

void
IcePy::PyException::checkSystemExit()
{
    if(PyObject_IsInstance(ex.get(), PyExc_SystemExit))
    {
        handleSystemExit(ex.get()); // Does not return.
    }
}

void
IcePy::PyException::raiseLocalException()
{
    string typeName = getTypeName();

    try
    {
        if(typeName == "Ice.ObjectNotExistException")
        {
            throw Ice::ObjectNotExistException(__FILE__, __LINE__);
        }
        else if(typeName == "Ice.OperationNotExistException")
        {
            throw Ice::OperationNotExistException(__FILE__, __LINE__);
        }
        else if(typeName == "Ice.FacetNotExistException")
        {
            throw Ice::FacetNotExistException(__FILE__, __LINE__);
        }
        else if(typeName == "Ice.RequestFailedException")
        {
            throw Ice::RequestFailedException(__FILE__, __LINE__);
        }
    }
    catch(Ice::RequestFailedException& e)
    {
        IcePy::PyObjectHandle member;
        member = PyObject_GetAttrString(ex.get(), STRCAST("id"));
        if(member.get() && IcePy::checkIdentity(member.get()))
        {
            IcePy::getIdentity(member.get(), e.id);
        }
        member = PyObject_GetAttrString(ex.get(), STRCAST("facet"));
        if(member.get() && checkString(member.get()))
        {
            e.facet = getString(member.get());
        }
        member = PyObject_GetAttrString(ex.get(), STRCAST("operation"));
        if(member.get() && checkString(member.get()))
        {
            e.operation = getString(member.get());
        }
        throw;
    }

    try
    {
        if(typeName == "Ice.UnknownLocalException")
        {
            throw Ice::UnknownLocalException(__FILE__, __LINE__);
        }
        else if(typeName == "Ice.UnknownUserException")
        {
            throw Ice::UnknownUserException(__FILE__, __LINE__);
        }
        else if(typeName == "Ice.UnknownException")
        {
            throw Ice::UnknownException(__FILE__, __LINE__);
        }
    }
    catch(Ice::UnknownException& e)
    {
        IcePy::PyObjectHandle member;
        member = PyObject_GetAttrString(ex.get(), STRCAST("unknown"));
        if(member.get() && checkString(member.get()))
        {
            e.unknown = getString(member.get());
        }
        throw;
    }

    Ice::UnknownLocalException e(__FILE__, __LINE__);
    string tb = getTraceback();
    if(!tb.empty())
    {
        e.unknown = tb;
    }
    else
    {
        e.unknown = typeName;
    }
    throw e;
}

string
IcePy::PyException::getTraceback()
{
    if(!_tb.get())
    {
        return string();
    }

    //
    // We need the equivalent of the following Python code:
    //
    // import traceback
    // list = traceback.format_exception(type, ex, tb)
    //
    PyObjectHandle str = createString("traceback");
    PyObjectHandle mod = PyImport_Import(str.get());
    assert(mod.get()); // Unable to import traceback module - Python installation error?
    PyObject* d = PyModule_GetDict(mod.get());
    PyObject* func = PyDict_GetItemString(d, "format_exception");
    assert(func); // traceback.format_exception must be present.
    PyObjectHandle args = Py_BuildValue("(OOO)", _type.get(), ex.get(), _tb.get());
    PyObjectHandle list = PyObject_CallObject(func, args.get());

    string result;
    for(Py_ssize_t i = 0; i < PyList_GET_SIZE(list.get()); ++i)
    {
        string s = getString(PyList_GetItem(list.get(), i));
        result += s;
    }

    return result;
}

string
IcePy::PyException::getTypeName()
{
    PyObject* cls = reinterpret_cast<PyObject*>(ex.get()->ob_type);
    PyObjectHandle name = PyObject_GetAttrString(cls, "__name__");
    assert(name.get());
    PyObjectHandle mod = PyObject_GetAttrString(cls, "__module__");
    assert(mod.get());
    string result = getString(mod.get());
    result += ".";
    result += getString(name.get());
    return result;
}

bool
IcePy::listToStringSeq(PyObject* l, Ice::StringSeq& seq)
{
    assert(PyList_Check(l));

    Py_ssize_t sz = PyList_GET_SIZE(l);
    for(Py_ssize_t i = 0; i < sz; ++i)
    {
        PyObject* item = PyList_GET_ITEM(l, i);
        if(!item)
        {
            return false;
        }
        string str;
        if(checkString(item))
        {
            str = getString(item);
        }
        else if(item != Py_None)
        {
            PyErr_Format(PyExc_ValueError, STRCAST("list element must be a string"));
            return false;
        }
        seq.push_back(str);
    }

    return true;
}

bool
IcePy::stringSeqToList(const Ice::StringSeq& seq, PyObject* l)
{
    assert(PyList_Check(l));

    for(Ice::StringSeq::const_iterator p = seq.begin(); p != seq.end(); ++p)
    {
        PyObject* str = Py_BuildValue(STRCAST("s"), p->c_str());
        if(!str)
        {
            Py_DECREF(l);
            return false;
        }
        int status = PyList_Append(l, str);
        Py_DECREF(str); // Give ownership to the list.
        if(status < 0)
        {
            Py_DECREF(l);
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
    for(int i = 0; i < sz; ++i)
    {
        PyObject* item = PyTuple_GET_ITEM(t, i);
        if(!item)
        {
            return false;
        }
        string str;
        if(checkString(item))
        {
            str = getString(item);
        }
        else if(item != Py_None)
        {
            PyErr_Format(PyExc_ValueError, STRCAST("tuple element must be a string"));
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
    PyObject* key;
    PyObject* value;
    while(PyDict_Next(dict, &pos, &key, &value))
    {
        string keystr;
        if(checkString(key))
        {
            keystr = getString(key);
        }
        else if(key != Py_None)
        {
            PyErr_Format(PyExc_ValueError, STRCAST("context key must be a string"));
            return false;
        }

        string valuestr;
        if(checkString(value))
        {
            valuestr = getString(value);
        }
        else if(value != Py_None)
        {
            PyErr_Format(PyExc_ValueError, STRCAST("context value must be a string"));
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

    for(Ice::Context::const_iterator p = ctx.begin(); p != ctx.end(); ++p)
    {
        PyObjectHandle key = createString(p->first);
        PyObjectHandle value = createString(p->second);
        if(!key.get() || !value.get())
        {
            return false;
        }
        if(PyDict_SetItem(dict, key.get(), value.get()) < 0)
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

    //
    // First search for the module in sys.modules.
    //
    PyObject* sysModules = PyImport_GetModuleDict();
    assert(sysModules);

    PyObject* module = PyDict_GetItemString(sysModules, const_cast<char*>(moduleName.c_str()));
    PyObject* dict;
    if(!module)
    {
        //
        // Not found, so we need to import the module.
        //
        PyObjectHandle h = PyImport_ImportModule(const_cast<char*>(moduleName.c_str()));
        if(!h.get())
        {
            return 0;
        }

        dict = PyModule_GetDict(h.get());
    }
    else
    {
        dict = PyModule_GetDict(module);
    }

    assert(dict);
    return PyDict_GetItemString(dict, const_cast<char*>(name.c_str()));
}

PyObject*
IcePy::createExceptionInstance(PyObject* type)
{
    assert(PyExceptionClass_Check(type));
    IcePy::PyObjectHandle args = PyTuple_New(0);
    if(!args.get())
    {
        return 0;
    }
    return PyEval_CallObject(type, args.get());
}

static void
convertLocalException(const Ice::LocalException& ex, PyObject* p)
{
    //
    // Transfer data members from Ice exception to Python exception.
    //
    try
    {
        ex.ice_throw();
    }
    catch(const Ice::InitializationException& e)
    {
        IcePy::PyObjectHandle m = IcePy::createString(e.reason);
        PyObject_SetAttrString(p, STRCAST("reason"), m.get());
    }
    catch(const Ice::PluginInitializationException& e)
    {
        IcePy::PyObjectHandle m = IcePy::createString(e.reason);
        PyObject_SetAttrString(p, STRCAST("reason"), m.get());
    }
    catch(const Ice::AlreadyRegisteredException& e)
    {
        IcePy::PyObjectHandle m;
        m = IcePy::createString(e.kindOfObject);
        PyObject_SetAttrString(p, STRCAST("kindOfObject"), m.get());
        m = IcePy::createString(e.id);
        PyObject_SetAttrString(p, STRCAST("id"), m.get());
    }
    catch(const Ice::NotRegisteredException& e)
    {
        IcePy::PyObjectHandle m;
        m = IcePy::createString(e.kindOfObject);
        PyObject_SetAttrString(p, STRCAST("kindOfObject"), m.get());
        m = IcePy::createString(e.id);
        PyObject_SetAttrString(p, STRCAST("id"), m.get());
    }
    catch(const Ice::TwowayOnlyException& e)
    {
        IcePy::PyObjectHandle m = IcePy::createString(e.operation);
        PyObject_SetAttrString(p, STRCAST("operation"), m.get());
    }
    catch(const Ice::UnknownException& e)
    {
        IcePy::PyObjectHandle m = IcePy::createString(e.unknown);
        PyObject_SetAttrString(p, STRCAST("unknown"), m.get());
    }
    catch(const Ice::ObjectAdapterDeactivatedException& e)
    {
        IcePy::PyObjectHandle m = IcePy::createString(e.name);
        PyObject_SetAttrString(p, STRCAST("name"), m.get());
    }
    catch(const Ice::ObjectAdapterIdInUseException& e)
    {
        IcePy::PyObjectHandle m = IcePy::createString(e.id);
        PyObject_SetAttrString(p, STRCAST("id"), m.get());
    }
    catch(const Ice::NoEndpointException& e)
    {
        IcePy::PyObjectHandle m = IcePy::createString(e.proxy);
        PyObject_SetAttrString(p, STRCAST("proxy"), m.get());
    }
    catch(const Ice::EndpointParseException& e)
    {
        IcePy::PyObjectHandle m = IcePy::createString(e.str);
        PyObject_SetAttrString(p, STRCAST("str"), m.get());
    }
    catch(const Ice::IdentityParseException& e)
    {
        IcePy::PyObjectHandle m = IcePy::createString(e.str);
        PyObject_SetAttrString(p, STRCAST("str"), m.get());
    }
    catch(const Ice::ProxyParseException& e)
    {
        IcePy::PyObjectHandle m = IcePy::createString(e.str);
        PyObject_SetAttrString(p, STRCAST("str"), m.get());
    }
    catch(const Ice::IllegalIdentityException& e)
    {
        IcePy::PyObjectHandle m = IcePy::createIdentity(e.id);
        PyObject_SetAttrString(p, STRCAST("id"), m.get());
    }
    catch(const Ice::RequestFailedException& e)
    {
        IcePy::PyObjectHandle m;
        m = IcePy::createIdentity(e.id);
        PyObject_SetAttrString(p, STRCAST("id"), m.get());
        m = IcePy::createString(e.facet);
        PyObject_SetAttrString(p, STRCAST("facet"), m.get());
        m = IcePy::createString(e.operation);
        PyObject_SetAttrString(p, STRCAST("operation"), m.get());
    }
    catch(const Ice::FileException& e)
    {
        IcePy::PyObjectHandle m = PyLong_FromLong(e.error);
        PyObject_SetAttrString(p, STRCAST("error"), m.get());
        m = IcePy::createString(e.path);
        PyObject_SetAttrString(p, STRCAST("path"), m.get());
    }
    catch(const Ice::SyscallException& e) // This must appear after all subclasses of SyscallException.
    {
        IcePy::PyObjectHandle m = PyLong_FromLong(e.error);
        PyObject_SetAttrString(p, STRCAST("error"), m.get());
    }
    catch(const Ice::DNSException& e)
    {
        IcePy::PyObjectHandle m;
        m = PyLong_FromLong(e.error);
        PyObject_SetAttrString(p, STRCAST("error"), m.get());
        m = IcePy::createString(e.host);
        PyObject_SetAttrString(p, STRCAST("host"), m.get());
    }
    catch(const Ice::UnsupportedProtocolException& e)
    {
        IcePy::PyObjectHandle m;
        m = IcePy::createProtocolVersion(e.bad);
        PyObject_SetAttrString(p, STRCAST("bad"), m.get());
        m = IcePy::createProtocolVersion(e.supported);
        PyObject_SetAttrString(p, STRCAST("supported"), m.get());
    }
    catch(const Ice::UnsupportedEncodingException& e)
    {
        IcePy::PyObjectHandle m;
        m = IcePy::createEncodingVersion(e.bad);
        PyObject_SetAttrString(p, STRCAST("bad"), m.get());
        m = IcePy::createEncodingVersion(e.supported);
        PyObject_SetAttrString(p, STRCAST("supported"), m.get());
    }
    catch(const Ice::NoObjectFactoryException& e)
    {
        IcePy::PyObjectHandle m;
        m = IcePy::createString(e.reason);
        PyObject_SetAttrString(p, STRCAST("reason"), m.get());
        m = IcePy::createString(e.type);
        PyObject_SetAttrString(p, STRCAST("type"), m.get());
    }
    catch(const Ice::UnexpectedObjectException& e)
    {
        IcePy::PyObjectHandle m;
        m = IcePy::createString(e.reason);
        PyObject_SetAttrString(p, STRCAST("reason"), m.get());
        m = IcePy::createString(e.type);
        PyObject_SetAttrString(p, STRCAST("type"), m.get());
        m = IcePy::createString(e.expectedType);
        PyObject_SetAttrString(p, STRCAST("expectedType"), m.get());
    }
    catch(const Ice::ProtocolException& e) // This must appear after all subclasses of ProtocolException.
    {
        IcePy::PyObjectHandle m = IcePy::createString(e.reason);
        PyObject_SetAttrString(p, STRCAST("reason"), m.get());
    }
    catch(const Ice::FeatureNotSupportedException& e)
    {
        IcePy::PyObjectHandle m = IcePy::createString(e.unsupportedFeature);
        PyObject_SetAttrString(p, STRCAST("unsupportedFeature"), m.get());
    }
    catch(const Ice::SecurityException& e)
    {
        IcePy::PyObjectHandle m = IcePy::createString(e.reason);
        PyObject_SetAttrString(p, STRCAST("reason"), m.get());
    }
    catch(const Ice::IllegalServantException& e)
    {
        IcePy::PyObjectHandle m = IcePy::createString(e.reason);
        PyObject_SetAttrString(p, STRCAST("reason"), m.get());
    }
    catch(const Ice::LocalException&)
    {
        //
        // Nothing to do.
        //
    }
}

PyObject*
IcePy::convertException(const Ice::Exception& ex)
{
    PyObjectHandle p;
    PyObject* type;

    ostringstream ostr;
    ostr << ex;
    string str = ostr.str();

    try
    {
        ex.ice_throw();
    }
    catch(const Ice::LocalException& e)
    {
        type = lookupType(scopedToName(e.ice_name()));
        if(type)
        {
            p = createExceptionInstance(type);
            if(p.get())
            {
                convertLocalException(e, p.get());
            }
        }
        else
        {
            type = lookupType("Ice.UnknownLocalException");
            assert(type);
            p = createExceptionInstance(type);
            if(p.get())
            {
                PyObjectHandle s = createString(str);
                PyObject_SetAttrString(p.get(), STRCAST("unknown"), s.get());
            }
        }
    }
    catch(const Ice::UserException&)
    {
        type = lookupType("Ice.UnknownUserException");
        assert(type);
        p = createExceptionInstance(type);
        if(p.get())
        {
            PyObjectHandle s = createString(str);
            PyObject_SetAttrString(p.get(), STRCAST("unknown"), s.get());
        }
    }
    catch(const Ice::Exception&)
    {
        type = lookupType("Ice.UnknownException");
        assert(type);
        p = createExceptionInstance(type);
        if(p.get())
        {
            PyObjectHandle s = createString(str);
            PyObject_SetAttrString(p.get(), STRCAST("unknown"), s.get());
        }
    }

    return p.release();
}

void
IcePy::setPythonException(const Ice::Exception& ex)
{
    PyObjectHandle p = convertException(ex);
    if(p.get())
    {
        setPythonException(p.get());
    }
}

void
IcePy::setPythonException(PyObject* ex)
{
    //
    // PyErr_Restore steals references to the type and exception.
    //
    PyObject* type = PyObject_Type(ex);
    assert(type);
    Py_INCREF(ex);
    PyErr_Restore(type, ex, 0);
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
    if(PyExceptionInstance_Check(ex))
    {
        code = PyObject_GetAttrString(ex, STRCAST("code"));
    }
    else
    {
        code = ex;
        Py_INCREF(ex);
    }

    int status;
    if(PyLong_Check(code.get()))
    {
        status = static_cast<int>(PyLong_AsLong(code.get()));
    }
    else
    {
        PyObject_Print(code.get(), stderr, Py_PRINT_RAW);
        PySys_WriteStderr(STRCAST("\n"));
        status = 1;
    }

    code = 0;
    Py_Exit(status);
}

PyObject*
IcePy::createIdentity(const Ice::Identity& ident)
{
    PyObject* identityType = lookupType("Ice.Identity");

    PyObjectHandle obj = PyObject_CallObject(identityType, 0);
    if(!obj.get())
    {
        return 0;
    }

    if(!setIdentity(obj.get(), ident))
    {
        return 0;
    }

    return obj.release();
}

bool
IcePy::checkIdentity(PyObject* p)
{
    PyObject* identityType = lookupType("Ice.Identity");
    return PyObject_IsInstance(p, identityType) == 1;
}

bool
IcePy::setIdentity(PyObject* p, const Ice::Identity& ident)
{
    assert(checkIdentity(p));
    PyObjectHandle name = createString(ident.name);
    PyObjectHandle category = createString(ident.category);
    if(!name.get() || !category.get())
    {
        return false;
    }
    if(PyObject_SetAttrString(p, STRCAST("name"), name.get()) < 0 ||
       PyObject_SetAttrString(p, STRCAST("category"), category.get()) < 0)
    {
        return false;
    }
    return true;
}

bool
IcePy::getIdentity(PyObject* p, Ice::Identity& ident)
{
    assert(checkIdentity(p));
    PyObjectHandle name = PyObject_GetAttrString(p, STRCAST("name"));
    PyObjectHandle category = PyObject_GetAttrString(p, STRCAST("category"));
    if(name.get())
    {
        if(!checkString(name.get()))
        {
            PyErr_Format(PyExc_ValueError, STRCAST("identity name must be a string"));
            return false;
        }
        ident.name = getString(name.get());
    }
    if(category.get())
    {
        if(!checkString(category.get()))
        {
            PyErr_Format(PyExc_ValueError, STRCAST("identity category must be a string"));
            return false;
        }
        ident.category = getString(category.get());
    }
    return true;
}

PyObject*
IcePy::createProtocolVersion(const Ice::ProtocolVersion& v)
{
    return createVersion<Ice::ProtocolVersion>(v, Ice_ProtocolVersion);
}

PyObject*
IcePy::createEncodingVersion(const Ice::EncodingVersion& v)
{
    return createVersion<Ice::EncodingVersion>(v, Ice_EncodingVersion);
}

bool
IcePy::getEncodingVersion(PyObject* args, Ice::EncodingVersion& v)
{
    PyObject* versionType = IcePy::lookupType(Ice_EncodingVersion);
    PyObject* p;
    if(!PyArg_ParseTuple(args, STRCAST("O!"), versionType, &p))
    {
        return false;
    }

    if(!getVersion<Ice::EncodingVersion>(p, v, Ice_EncodingVersion))
    {
        return false;
    }

    return true;
}

extern "C"
PyObject*
IcePy_stringVersion(PyObject* /*self*/)
{
    string s = ICE_STRING_VERSION;
    return IcePy::createString(s);
}

extern "C"
PyObject*
IcePy_intVersion(PyObject* /*self*/)
{
    return PyLong_FromLong(ICE_INT_VERSION);
}

extern "C"
PyObject*
IcePy_currentProtocol(PyObject* /*self*/)
{
    return IcePy::createProtocolVersion(Ice::currentProtocol);
}

extern "C"
PyObject*
IcePy_currentProtocolEncoding(PyObject* /*self*/)
{
    return IcePy::createEncodingVersion(Ice::currentProtocolEncoding);
}

extern "C"
PyObject*
IcePy_currentEncoding(PyObject* /*self*/)
{
    return IcePy::createEncodingVersion(Ice::currentEncoding);
}

extern "C"
PyObject*
IcePy_protocolVersionToString(PyObject* /*self*/, PyObject* args)
{
    return IcePy::versionToString<Ice::ProtocolVersion>(args, IcePy::Ice_ProtocolVersion);
}

extern "C"
PyObject*
IcePy_stringToProtocolVersion(PyObject* /*self*/, PyObject* args)
{
    return IcePy::stringToVersion<Ice::ProtocolVersion>(args, IcePy::Ice_ProtocolVersion);
}

extern "C"
PyObject*
IcePy_encodingVersionToString(PyObject* /*self*/, PyObject* args)
{
    return IcePy::versionToString<Ice::EncodingVersion>(args, IcePy::Ice_EncodingVersion);
}

extern "C"
PyObject*
IcePy_stringToEncodingVersion(PyObject* /*self*/, PyObject* args)
{
    return IcePy::stringToVersion<Ice::EncodingVersion>(args, IcePy::Ice_EncodingVersion);
}

extern "C"
PyObject*
IcePy_generateUUID(PyObject* /*self*/)
{
    string uuid = IceUtil::generateUUID();
    return IcePy::createString(uuid);
}
