// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Exception.h>
#include <Identity.h>
#include <Util.h>
#include <Ice/LocalException.h>

using namespace std;

PyObject*
IcePy::getPythonException(bool clear)
{
    PyObject* t;
    PyObject* val;
    PyObject* tb;

    PyErr_Fetch(&t, &val, &tb); // PyErr_Fetch clears the exception.
    PyErr_NormalizeException(&t, &val, &tb);

    if(clear)
    {
        Py_XDECREF(t);
        Py_XDECREF(tb);
    }
    else
    {
        Py_XINCREF(val);
        PyErr_Restore(t, val, tb);
    }

    return val; // New reference.
}

PyObject*
IcePy::createExceptionInstance(PyObject* type)
{
    assert(PyClass_Check(type));
    IcePy::PyObjectHandle args = PyTuple_New(0);
    if(args.get() == NULL)
    {
        return NULL;
    }
    return PyEval_CallObject(type, args.get());
}

void
IcePy::setPythonException(const Ice::Exception& ex)
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
    catch(const Ice::AlreadyRegisteredException& e)
    {
        type = lookupType("Ice.AlreadyRegisteredException");
        assert(type != NULL);
        p = createExceptionInstance(type);
        if(p.get() != NULL)
        {
            PyObjectHandle s;
            s = PyString_FromString(const_cast<char*>(e.kindOfObject.c_str()));
            PyObject_SetAttrString(p.get(), "kindOfObject", s.get());
            s = PyString_FromString(const_cast<char*>(e.id.c_str()));
            PyObject_SetAttrString(p.get(), "id", s.get());
        }
    }
    catch(const Ice::NotRegisteredException& e)
    {
        type = lookupType("Ice.NotRegisteredException");
        assert(type != NULL);
        p = createExceptionInstance(type);
        if(p.get() != NULL)
        {
            PyObjectHandle s;
            s = PyString_FromString(const_cast<char*>(e.kindOfObject.c_str()));
            PyObject_SetAttrString(p.get(), "kindOfObject", s.get());
            s = PyString_FromString(const_cast<char*>(e.id.c_str()));
            PyObject_SetAttrString(p.get(), "id", s.get());
        }
    }
    catch(const Ice::RequestFailedException& e)
    {
        type = lookupType(scopedToName(e.ice_name()));
        assert(type != NULL);
        p = createExceptionInstance(type);
        if(p.get() != NULL)
        {
            PyObjectHandle m;
            m = createIdentity(e.id);
            PyObject_SetAttrString(p.get(), "id", m.get());
            m = PyString_FromString(const_cast<char*>(e.facet.c_str()));
            PyObject_SetAttrString(p.get(), "facet", m.get());
            m = PyString_FromString(const_cast<char*>(e.operation.c_str()));
            PyObject_SetAttrString(p.get(), "operation", m.get());
        }
    }
    catch(const Ice::UnknownException& e)
    {
        type = lookupType(scopedToName(e.ice_name()));
        assert(type != NULL);
        p = createExceptionInstance(type);
        if(p.get() != NULL)
        {
            PyObjectHandle s = PyString_FromString(const_cast<char*>(e.unknown.c_str()));
            PyObject_SetAttrString(p.get(), "unknown", s.get());
        }
    }
    catch(const Ice::NoObjectFactoryException& e)
    {
        type = lookupType(scopedToName(e.ice_name()));
        assert(type != NULL);
        p = createExceptionInstance(type);
        if(p.get() != NULL)
        {
            PyObjectHandle m;
            m = PyString_FromString(const_cast<char*>(e.type.c_str()));
            PyObject_SetAttrString(p.get(), "type", m.get());
        }
    }
    catch(const Ice::LocalException& e)
    {
        type = lookupType(scopedToName(e.ice_name()));
        if(type != NULL)
        {
            p = createExceptionInstance(type);
        }
        else
        {
            type = lookupType("Ice.UnknownLocalException");
            assert(type != NULL);
            p = createExceptionInstance(type);
            if(p.get() != NULL)
            {
                PyObjectHandle s = PyString_FromString(const_cast<char*>(str.c_str()));
                PyObject_SetAttrString(p.get(), "unknown", s.get());
            }
        }
    }
    catch(const Ice::UserException& e)
    {
        type = lookupType("Ice.UnknownUserException");
        assert(type != NULL);
        p = createExceptionInstance(type);
        if(p.get() != NULL)
        {
            PyObjectHandle s = PyString_FromString(const_cast<char*>(str.c_str()));
            PyObject_SetAttrString(p.get(), "unknown", s.get());
        }
    }
    catch(const Ice::Exception& e)
    {
        type = lookupType("Ice.UnknownException");
        assert(type != NULL);
        p = createExceptionInstance(type);
        if(p.get() != NULL)
        {
            PyObjectHandle s = PyString_FromString(const_cast<char*>(str.c_str()));
            PyObject_SetAttrString(p.get(), "unknown", s.get());
        }
    }

    if(p.get() != NULL)
    {
        Py_INCREF(type);
        PyErr_Restore(type, p.release(), NULL);
    }
}

static void
throwLocalException(PyObject* ex)
{
    string typeName = ex->ob_type->tp_name;

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
        member = PyObject_GetAttrString(ex, "id");
        if(member.get() != NULL && IcePy::checkIdentity(member.get()))
        {
            IcePy::getIdentity(member.get(), e.id);
        }
        member = PyObject_GetAttrString(ex, "facet");
        if(member.get() != NULL && PyString_Check(member.get()))
        {
            e.facet = PyString_AS_STRING(member.get());
        }
        member = PyObject_GetAttrString(ex, "operation");
        if(member.get() != NULL && PyString_Check(member.get()))
        {
            e.operation = PyString_AS_STRING(member.get());
        }
        throw e;
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
        member = PyObject_GetAttrString(ex, "unknown");
        if(member.get() != NULL && PyString_Check(member.get()))
        {
            e.unknown = PyString_AS_STRING(member.get());
        }
        throw e;
    }

    Ice::UnknownLocalException e(__FILE__, __LINE__);
    e.unknown = typeName;
    throw e;
}

void
IcePy::throwPythonException(PyObject* ex)
{
    PyObjectHandle h;
    if(ex == NULL)
    {
        h = getPythonException();
        ex = h.get();
    }

    PyObject* userExceptionType = lookupType("Ice.UserException");
    PyObject* localExceptionType = lookupType("Ice.LocalException");

    if(PyObject_IsInstance(ex, userExceptionType))
    {
        PyObjectHandle id = PyObject_CallMethod(ex, "ice_id", NULL);
        PyErr_Clear();
        Ice::UnknownUserException e(__FILE__, __LINE__);
        if(id.get() == NULL)
        {
            e.unknown = ex->ob_type->tp_name;
        }
        else
        {
            e.unknown = PyString_AS_STRING(id.get());
        }
        throw e;
    }
    else if(PyObject_IsInstance(ex, localExceptionType))
    {
        throwLocalException(ex);
    }
    else
    {
        PyObjectHandle str = PyObject_Str(ex);
        assert(str.get() != NULL);
        assert(PyString_Check(str.get()));

        Ice::UnknownException e(__FILE__, __LINE__);
        e.unknown = PyString_AS_STRING(str.get());
        throw e;
    }
}

void
IcePy::handleSystemExit()
{
    PyObjectHandle ex = getPythonException();
    assert(ex.get() != NULL);

    //
    // This code is similar to handle_system_exit in pythonrun.c.
    //
    PyObjectHandle code;
    if(PyInstance_Check(ex.get()))
    {
        code = PyObject_GetAttrString(ex.get(), "code");
    }
    else
    {
        code = ex;
    }

    int status;
    if(PyInt_Check(code.get()))
    {
        status = static_cast<int>(PyInt_AsLong(code.get()));
    }
    else
    {
        PyObject_Print(code.get(), stderr, Py_PRINT_RAW);
        PySys_WriteStderr("\n");
        status = 1;
    }

    code = 0;
    ex = 0;
    Py_Exit(status);
}
