// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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
#include <Ice/IdentityUtil.h>
#include <Ice/LocalException.h>
#include <IceUtil/UUID.h>
#include <Slice/PythonUtil.h>

using namespace std;
using namespace Slice::Python;

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
IcePy::PyObjectHandle::get()
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
                e.unknown = PyString_AS_STRING(name.get());
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
            if(msg.get() && strlen(PyString_AsString(msg.get())) > 0)
            {
                ostr << ": " << PyString_AsString(msg.get());
            }

            e.unknown = ostr.str();
        }
        throw e;
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
        if(member.get() && PyString_Check(member.get()))
        {
            e.facet = PyString_AS_STRING(member.get());
        }
        member = PyObject_GetAttrString(ex.get(), STRCAST("operation"));
        if(member.get() && PyString_Check(member.get()))
        {
            e.operation = PyString_AS_STRING(member.get());
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
        if(member.get() && PyString_Check(member.get()) && strlen(PyString_AS_STRING(member.get())) > 0)
        {
            e.unknown = PyString_AS_STRING(member.get());
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
    PyObjectHandle str = PyString_FromString("traceback");
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
        result += PyString_AsString(PyList_GetItem(list.get(), i));
    }

    return result;
}

string
IcePy::PyException::getTypeName()
{
#ifdef ICEPY_OLD_EXCEPTIONS
    PyObject* cls = reinterpret_cast<PyObject*>(reinterpret_cast<PyInstanceObject*>(ex.get())->in_class);
#else
    PyObject* cls = reinterpret_cast<PyObject*>(ex.get()->ob_type);
#endif
    PyObjectHandle name = PyObject_GetAttrString(cls, "__name__");
    assert(name.get());
    PyObjectHandle mod = PyObject_GetAttrString(cls, "__module__");
    assert(mod.get());
    string result = PyString_AsString(mod.get());
    result += ".";
    result += PyString_AsString(name.get());
    return result;
}

IcePy::AllowThreads::AllowThreads()
{
    _state = PyEval_SaveThread();
}

IcePy::AllowThreads::~AllowThreads()
{
    PyEval_RestoreThread(_state);
}

IcePy::AdoptThread::AdoptThread()
{
    _state = PyGILState_Ensure();
}

IcePy::AdoptThread::~AdoptThread()
{
    PyGILState_Release(_state);
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
        if(!PyString_Check(item))
        {
            PyErr_Format(PyExc_ValueError, STRCAST("list element must be a string"));
            return false;
        }
        seq.push_back(string(PyString_AS_STRING(item), PyString_GET_SIZE(item)));
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
        if(!PyString_Check(item))
        {
            PyErr_Format(PyExc_ValueError, STRCAST("tuple element must be a string"));
            return false;
        }
        seq.push_back(string(PyString_AS_STRING(item), PyString_GET_SIZE(item)));
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
        char* keystr = PyString_AsString(key);
        if(!keystr)
        {
            PyErr_Format(PyExc_ValueError, STRCAST("context key must be a string"));
            return false;
        }
        char* valuestr = PyString_AsString(value);
        if(!valuestr)
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
        PyObjectHandle key = PyString_FromString(const_cast<char*>(p->first.c_str()));
        PyObjectHandle value = PyString_FromString(const_cast<char*>(p->second.c_str()));
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
#ifdef ICEPY_OLD_EXCEPTIONS
    assert(PyClass_Check(type));
#else
    assert(PyExceptionClass_Check(type));
#endif
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
        IcePy::PyObjectHandle m = PyString_FromString(const_cast<char*>(e.reason.c_str()));
        PyObject_SetAttrString(p, STRCAST("reason"), m.get());
    }
    catch(const Ice::PluginInitializationException& e)
    {
        IcePy::PyObjectHandle m = PyString_FromString(const_cast<char*>(e.reason.c_str()));
        PyObject_SetAttrString(p, STRCAST("reason"), m.get());
    }
    catch(const Ice::AlreadyRegisteredException& e)
    {
        IcePy::PyObjectHandle m;
        m = PyString_FromString(const_cast<char*>(e.kindOfObject.c_str()));
        PyObject_SetAttrString(p, STRCAST("kindOfObject"), m.get());
        m = PyString_FromString(const_cast<char*>(e.id.c_str()));
        PyObject_SetAttrString(p, STRCAST("id"), m.get());
    }
    catch(const Ice::NotRegisteredException& e)
    {
        IcePy::PyObjectHandle m;
        m = PyString_FromString(const_cast<char*>(e.kindOfObject.c_str()));
        PyObject_SetAttrString(p, STRCAST("kindOfObject"), m.get());
        m = PyString_FromString(const_cast<char*>(e.id.c_str()));
        PyObject_SetAttrString(p, STRCAST("id"), m.get());
    }
    catch(const Ice::TwowayOnlyException& e)
    {
        IcePy::PyObjectHandle m = PyString_FromString(const_cast<char*>(e.operation.c_str()));
        PyObject_SetAttrString(p, STRCAST("operation"), m.get());
    }
    catch(const Ice::UnknownException& e)
    {
        IcePy::PyObjectHandle m = PyString_FromString(const_cast<char*>(e.unknown.c_str()));
        PyObject_SetAttrString(p, STRCAST("unknown"), m.get());
    }
    catch(const Ice::ObjectAdapterDeactivatedException& e)
    {
        IcePy::PyObjectHandle m = PyString_FromString(const_cast<char*>(e.name.c_str()));
        PyObject_SetAttrString(p, STRCAST("name"), m.get());
    }
    catch(const Ice::ObjectAdapterIdInUseException& e)
    {
        IcePy::PyObjectHandle m = PyString_FromString(const_cast<char*>(e.id.c_str()));
        PyObject_SetAttrString(p, STRCAST("id"), m.get());
    }
    catch(const Ice::NoEndpointException& e)
    {
        IcePy::PyObjectHandle m = PyString_FromString(const_cast<char*>(e.proxy.c_str()));
        PyObject_SetAttrString(p, STRCAST("proxy"), m.get());
    }
    catch(const Ice::EndpointParseException& e)
    {
        IcePy::PyObjectHandle m = PyString_FromString(const_cast<char*>(e.str.c_str()));
        PyObject_SetAttrString(p, STRCAST("str"), m.get());
    }
    catch(const Ice::IdentityParseException& e)
    {
        IcePy::PyObjectHandle m = PyString_FromString(const_cast<char*>(e.str.c_str()));
        PyObject_SetAttrString(p, STRCAST("str"), m.get());
    }
    catch(const Ice::ProxyParseException& e)
    {
        IcePy::PyObjectHandle m = PyString_FromString(const_cast<char*>(e.str.c_str()));
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
        m = PyString_FromString(const_cast<char*>(e.facet.c_str()));
        PyObject_SetAttrString(p, STRCAST("facet"), m.get());
        m = PyString_FromString(const_cast<char*>(e.operation.c_str()));
        PyObject_SetAttrString(p, STRCAST("operation"), m.get());
    }
    catch(const Ice::FileException& e)
    {
        IcePy::PyObjectHandle m = PyInt_FromLong(e.error);
        PyObject_SetAttrString(p, STRCAST("error"), m.get());
        m = PyString_FromString(const_cast<char*>(e.path.c_str()));
        PyObject_SetAttrString(p, STRCAST("path"), m.get());
    }
    catch(const Ice::SyscallException& e) // This must appear after all subclasses of SyscallException.
    {
        IcePy::PyObjectHandle m = PyInt_FromLong(e.error);
        PyObject_SetAttrString(p, STRCAST("error"), m.get());
    }
    catch(const Ice::DNSException& e)
    {
        IcePy::PyObjectHandle m;
        m = PyInt_FromLong(e.error);
        PyObject_SetAttrString(p, STRCAST("error"), m.get());
        m = PyString_FromString(const_cast<char*>(e.host.c_str()));
        PyObject_SetAttrString(p, STRCAST("host"), m.get());
    }
    catch(const Ice::UnsupportedProtocolException& e)
    {
        IcePy::PyObjectHandle m;
        m = PyInt_FromLong(e.badMajor);
        PyObject_SetAttrString(p, STRCAST("badMajor"), m.get());
        m = PyInt_FromLong(e.badMinor);
        PyObject_SetAttrString(p, STRCAST("badMinor"), m.get());
        m = PyInt_FromLong(e.major);
        PyObject_SetAttrString(p, STRCAST("major"), m.get());
        m = PyInt_FromLong(e.minor);
        PyObject_SetAttrString(p, STRCAST("minor"), m.get());
    }
    catch(const Ice::UnsupportedEncodingException& e)
    {
        IcePy::PyObjectHandle m;
        m = PyInt_FromLong(e.badMajor);
        PyObject_SetAttrString(p, STRCAST("badMajor"), m.get());
        m = PyInt_FromLong(e.badMinor);
        PyObject_SetAttrString(p, STRCAST("badMinor"), m.get());
        m = PyInt_FromLong(e.major);
        PyObject_SetAttrString(p, STRCAST("major"), m.get());
        m = PyInt_FromLong(e.minor);
        PyObject_SetAttrString(p, STRCAST("minor"), m.get());
    }
    catch(const Ice::NoObjectFactoryException& e)
    {
        IcePy::PyObjectHandle m;
        m = PyString_FromString(const_cast<char*>(e.reason.c_str()));
        PyObject_SetAttrString(p, STRCAST("reason"), m.get());
        m = PyString_FromString(const_cast<char*>(e.type.c_str()));
        PyObject_SetAttrString(p, STRCAST("type"), m.get());
    }
    catch(const Ice::UnexpectedObjectException& e)
    {
        IcePy::PyObjectHandle m;
        m = PyString_FromString(const_cast<char*>(e.reason.c_str()));
        PyObject_SetAttrString(p, STRCAST("reason"), m.get());
        m = PyString_FromString(const_cast<char*>(e.type.c_str()));
        PyObject_SetAttrString(p, STRCAST("type"), m.get());
        m = PyString_FromString(const_cast<char*>(e.expectedType.c_str()));
        PyObject_SetAttrString(p, STRCAST("expectedType"), m.get());
    }
    catch(const Ice::ProtocolException& e) // This must appear after all subclasses of ProtocolException.
    {
        IcePy::PyObjectHandle m = PyString_FromString(const_cast<char*>(e.reason.c_str()));
        PyObject_SetAttrString(p, STRCAST("reason"), m.get());
    }
    catch(const Ice::FeatureNotSupportedException& e)
    {
        IcePy::PyObjectHandle m = PyString_FromString(const_cast<char*>(e.unsupportedFeature.c_str()));
        PyObject_SetAttrString(p, STRCAST("unsupportedFeature"), m.get());
    }
    catch(const Ice::SecurityException& e)
    {
        IcePy::PyObjectHandle m = PyString_FromString(const_cast<char*>(e.reason.c_str()));
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
                PyObjectHandle s = PyString_FromString(const_cast<char*>(str.c_str()));
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
            PyObjectHandle s = PyString_FromString(const_cast<char*>(str.c_str()));
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
            PyObjectHandle s = PyString_FromString(const_cast<char*>(str.c_str()));
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
#ifdef ICEPY_OLD_EXCEPTIONS
    PyObject* type = reinterpret_cast<PyObject*>(reinterpret_cast<PyInstanceObject*>(ex)->in_class);
    Py_INCREF(type);
#else
    PyObject* type = PyObject_Type(ex);
    assert(type);
#endif
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
#ifdef ICEPY_OLD_EXCEPTIONS
    if(PyInstance_Check(ex))
#else
    if(PyExceptionInstance_Check(ex))
#endif
    {
        code = PyObject_GetAttrString(ex, STRCAST("code"));
    }
    else
    {
        code = ex;
        Py_INCREF(ex);
    }

    int status;
    if(PyInt_Check(code.get()))
    {
        status = static_cast<int>(PyInt_AsLong(code.get()));
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
    PyObjectHandle name = PyString_FromString(const_cast<char*>(ident.name.c_str()));
    PyObjectHandle category = PyString_FromString(const_cast<char*>(ident.category.c_str()));
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
        char* s = PyString_AsString(name.get());
        if(!s)
        {
            PyErr_Format(PyExc_ValueError, STRCAST("identity name must be a string"));
            return false;
        }
        ident.name = s;
    }
    if(category.get())
    {
        char* s = PyString_AsString(category.get());
        if(!s)
        {
            PyErr_Format(PyExc_ValueError, STRCAST("identity category must be a string"));
            return false;
        }
        ident.category = s;
    }
    return true;
}

extern "C"
PyObject*
IcePy_identityToString(PyObject* /*self*/, PyObject* args)
{
    PyObject* identityType = IcePy::lookupType("Ice.Identity");
    PyObject* p;
    if(!PyArg_ParseTuple(args, STRCAST("O!"), identityType, &p))
    {
        return 0;
    }

    Ice::Identity id;
    if(!IcePy::getIdentity(p, id))
    {
        return 0;
    }

    string s;
    try
    {
        s = Ice::identityToString(id);
    }
    catch(const Ice::Exception& ex)
    {
        IcePy::setPythonException(ex);
        return 0;
    }
    return PyString_FromString(const_cast<char*>(s.c_str()));
}

extern "C"
PyObject*
IcePy_stringToIdentity(PyObject* /*self*/, PyObject* args)
{
    char* str;
    if(!PyArg_ParseTuple(args, STRCAST("s"), &str))
    {
        return 0;
    }

    Ice::Identity id;
    try
    {
        id = Ice::stringToIdentity(str);
    }
    catch(const Ice::Exception& ex)
    {
        IcePy::setPythonException(ex);
        return 0;
    }

    return IcePy::createIdentity(id);
}

extern "C"
PyObject*
IcePy_generateUUID(PyObject* /*self*/)
{
    string uuid = IceUtil::generateUUID();
    return PyString_FromString(const_cast<char*>(uuid.c_str()));
}
