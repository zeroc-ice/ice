// **********************************************************************
//
// Copyright (c) 2003-2010 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifdef _WIN32
#   include <IceUtil/Config.h>
#endif
#include <Operation.h>
#include <Current.h>
#include <Proxy.h>
#include <Types.h>
#include <Util.h>
#include <Ice/Communicator.h>
#include <Ice/IncomingAsync.h>
#include <Ice/Initialize.h>
#include <Ice/LocalException.h>
#include <Ice/Logger.h>
#include <Ice/ObjectAdapter.h>
#include <Ice/OutgoingAsync.h>
#include <Ice/Properties.h>
#include <Ice/Proxy.h>
#include <Slice/PythonUtil.h>

using namespace std;
using namespace IcePy;
using namespace Slice::Python;

namespace IcePy
{

//
// Information about an operation's parameter.
//
class ParamInfo : public UnmarshalCallback
{
public:

    virtual void unmarshaled(PyObject*, PyObject*, void*);

    Ice::StringSeq metaData;
    TypeInfoPtr type;
};
typedef IceUtil::Handle<ParamInfo> ParamInfoPtr;
typedef vector<ParamInfoPtr> ParamInfoList;

//
// Encapsulates attributes of an operation.
//
class Operation : public IceUtil::Shared
{
public:

    Operation(const char*, PyObject*, PyObject*, int, PyObject*, PyObject*, PyObject*, PyObject*, PyObject*);

    void deprecate(const string&);

    string name;
    Ice::OperationMode mode;
    Ice::OperationMode sendMode;
    bool amd;
    Ice::StringSeq metaData;
    ParamInfoList inParams;
    ParamInfoList outParams;
    ParamInfoPtr returnType;
    ExceptionInfoList exceptions;
    string dispatchName;
    bool sendsClasses;
    bool returnsClasses;

private:

    string _deprecateMessage;

    static void convertParams(PyObject*, ParamInfoList&, bool&);
};
typedef IceUtil::Handle<Operation> OperationPtr;

//
// The base class for client-side invocations.
//
class Invocation : virtual public IceUtil::Shared
{
public:

    Invocation(const Ice::ObjectPrx&);

    virtual PyObject* invoke(PyObject*, PyObject* = 0) = 0;

protected:

    Ice::ObjectPrx _prx;
};
typedef IceUtil::Handle<Invocation> InvocationPtr;

//
// TypedInvocation uses the information in the given Operation to validate, marshal, and unmarshal
// parameters and exceptions.
//
class TypedInvocation : virtual public Invocation
{
public:

    TypedInvocation(const Ice::ObjectPrx&, const OperationPtr&);

protected:

    OperationPtr _op;
    Ice::CommunicatorPtr _communicator;

    enum MappingType { SyncMapping, AsyncMapping, OldAsyncMapping };

    bool prepareRequest(PyObject*, MappingType, vector<Ice::Byte>&);
    PyObject* unmarshalResults(const pair<const Ice::Byte*, const Ice::Byte*>&);
    PyObject* unmarshalException(const pair<const Ice::Byte*, const Ice::Byte*>&);
    bool validateException(PyObject*) const;
    void checkTwowayOnly(const Ice::ObjectPrx&) const;
};

//
// Synchronous typed invocation.
//
class SyncTypedInvocation : virtual public TypedInvocation
{
public:

    SyncTypedInvocation(const Ice::ObjectPrx&, const OperationPtr&);

    virtual PyObject* invoke(PyObject*, PyObject* = 0);
};

//
// Asynchronous typed invocation.
//
class AsyncTypedInvocation : virtual public TypedInvocation
{
public:

    AsyncTypedInvocation(const Ice::ObjectPrx&, PyObject*, const OperationPtr&);
    ~AsyncTypedInvocation();

    virtual PyObject* invoke(PyObject*, PyObject* = 0);
    PyObject* end(const Ice::ObjectPrx&, const OperationPtr&, const Ice::AsyncResultPtr&);

    string opName() const;

    void response(bool, const pair<const Ice::Byte*, const Ice::Byte*>&);
    void exception(const Ice::Exception&);
    void sent(bool);

protected:

    void checkAsyncTwowayOnly(const Ice::ObjectPrx&) const;

    PyObject* _pyProxy;
    PyObject* _response;
    PyObject* _ex;
    PyObject* _sent;
};
typedef IceUtil::Handle<AsyncTypedInvocation> AsyncTypedInvocationPtr;

//
// Old-style asynchronous typed invocation.
//
class OldAsyncTypedInvocation : virtual public TypedInvocation
{
public:

    OldAsyncTypedInvocation(const Ice::ObjectPrx&, const OperationPtr&);
    ~OldAsyncTypedInvocation();

    virtual PyObject* invoke(PyObject*, PyObject* = 0);

    void response(bool, const pair<const Ice::Byte*, const Ice::Byte*>&);
    void exception(const Ice::Exception&);
    void sent(bool);

protected:

    PyObject* _callback;
};

//
// Synchronous blobject invocation.
//
class SyncBlobjectInvocation : virtual public Invocation
{
public:

    SyncBlobjectInvocation(const Ice::ObjectPrx&);

    virtual PyObject* invoke(PyObject*, PyObject* = 0);
};

//
// Asynchronous blobject invocation.
//
class AsyncBlobjectInvocation : virtual public Invocation
{
public:

    AsyncBlobjectInvocation(const Ice::ObjectPrx&, PyObject*);
    ~AsyncBlobjectInvocation();

    virtual PyObject* invoke(PyObject*, PyObject* = 0);
    PyObject* end(const Ice::ObjectPrx&, const Ice::AsyncResultPtr&);

    void response(bool, const pair<const Ice::Byte*, const Ice::Byte*>&);
    void exception(const Ice::Exception&);
    void sent(bool);

protected:

    PyObject* _pyProxy;
    string _op;
    PyObject* _response;
    PyObject* _ex;
    PyObject* _sent;
};
typedef IceUtil::Handle<AsyncBlobjectInvocation> AsyncBlobjectInvocationPtr;

//
// Old-style asynchronous blobject invocation.
//
class OldAsyncBlobjectInvocation : virtual public Invocation
{
public:

    OldAsyncBlobjectInvocation(const Ice::ObjectPrx&);
    ~OldAsyncBlobjectInvocation();

    virtual PyObject* invoke(PyObject*, PyObject* = 0);

    void response(bool, const pair<const Ice::Byte*, const Ice::Byte*>&);
    void exception(const Ice::Exception&);
    void sent(bool);

protected:

    string _op;
    PyObject* _callback;
};

//
// The base class for server-side upcalls.
//
class Upcall : virtual public IceUtil::Shared
{
public:

    virtual void dispatch(PyObject*, const pair<const Ice::Byte*, const Ice::Byte*>&, const Ice::Current&) = 0;
    virtual void response(PyObject*) = 0;
    virtual void exception(PyException&) = 0;
};
typedef IceUtil::Handle<Upcall> UpcallPtr;

//
// TypedInvocation uses the information in the given Operation to validate, marshal, and unmarshal
// parameters and exceptions.
//
class TypedUpcall : virtual public Upcall
{
public:

    TypedUpcall(const OperationPtr&, const Ice::AMD_Object_ice_invokePtr&, const Ice::CommunicatorPtr&);

    virtual void dispatch(PyObject*, const pair<const Ice::Byte*, const Ice::Byte*>&, const Ice::Current&);
    virtual void response(PyObject*);
    virtual void exception(PyException&);

private:

    bool validateException(PyObject*) const;

    OperationPtr _op;
    Ice::AMD_Object_ice_invokePtr _callback;
    Ice::CommunicatorPtr _communicator;
    bool _finished;
};

//
// Upcall for blobject servants.
//
class BlobjectUpcall : virtual public Upcall
{
public:

    BlobjectUpcall(bool, const Ice::AMD_Object_ice_invokePtr&);

    virtual void dispatch(PyObject*, const pair<const Ice::Byte*, const Ice::Byte*>&, const Ice::Current&);
    virtual void response(PyObject*);
    virtual void exception(PyException&);

private:

    bool _amd;
    Ice::AMD_Object_ice_invokePtr _callback;
    bool _finished;
};

//
// TypedServantWrapper uses the information in Operation to validate, marshal, and unmarshal
// parameters and exceptions.
//
class TypedServantWrapper : public ServantWrapper
{
public:

    TypedServantWrapper(PyObject*);

    virtual void ice_invoke_async(const Ice::AMD_Object_ice_invokePtr&,
                                  const pair<const Ice::Byte*, const Ice::Byte*>&,
                                  const Ice::Current&);

private:

    typedef map<string, OperationPtr> OperationMap;
    OperationMap _operationMap;
    OperationMap::iterator _lastOp;
};

//
// Encapsulates a blobject servant.
//
class BlobjectServantWrapper : public ServantWrapper
{
public:

    BlobjectServantWrapper(PyObject*, bool);

    virtual void ice_invoke_async(const Ice::AMD_Object_ice_invokePtr&,
                                  const pair<const Ice::Byte*, const Ice::Byte*>&,
                                  const Ice::Current&);

private:

    bool _amd;
};

struct OperationObject
{
    PyObject_HEAD
    OperationPtr* op;
};

struct AMDCallbackObject
{
    PyObject_HEAD
    UpcallPtr* upcall;
};

struct AsyncResultObject
{
    PyObject_HEAD
    Ice::AsyncResultPtr* result;
    InvocationPtr* invocation;
    PyObject* proxy;
    PyObject* connection;
    PyObject* communicator;
};

extern PyTypeObject OperationType;
extern PyTypeObject AMDCallbackType;

}

namespace
{

OperationPtr
getOperation(PyObject* p)
{
    assert(PyObject_IsInstance(p, reinterpret_cast<PyObject*>(&OperationType)) == 1);
    OperationObject* obj = reinterpret_cast<OperationObject*>(p);
    return *obj->op;
}

void
handleException()
{
    assert(PyErr_Occurred());

    PyException ex; // Retrieve it before another Python API call clears it.

    //
    // A callback that calls sys.exit() will raise the SystemExit exception.
    // This is normally caught by the interpreter, causing it to exit.
    // However, we have no way to pass this exception to the interpreter,
    // so we act on it directly.
    //
    ex.checkSystemExit();

    ex.raise();
}

void
callException(PyObject* method, PyObject* ex)
{
    PyObjectHandle args = Py_BuildValue(STRCAST("(O)"), ex);
    PyObjectHandle tmp = PyObject_Call(method, args.get(), 0);
    if(PyErr_Occurred())
    {
        handleException(); // Callback raised an exception.
    }
}

void
callException(PyObject* method, const Ice::Exception& ex)
{
    PyObjectHandle exh = convertException(ex);
    assert(exh.get());
    callException(method, exh.get());
}

void
callException(PyObject* callback, const string& op, const string& method, PyObject* ex)
{
    if(!PyObject_HasAttrString(callback, STRCAST(method.c_str())))
    {
        ostringstream ostr;
        ostr << "AMI callback object for operation `" << op << "' does not define " << method << "()";
        string str = ostr.str();
        PyErr_Warn(PyExc_RuntimeWarning, const_cast<char*>(str.c_str()));
    }
    else
    {
        PyObjectHandle m = PyObject_GetAttrString(callback, STRCAST(method.c_str()));
        assert(m.get());
        callException(m.get(), ex);
    }
}

void
callException(PyObject* callback, const string& op, const string& method, const Ice::Exception& ex)
{
    PyObjectHandle exh = convertException(ex);
    assert(exh.get());
    callException(callback, op, method, exh.get());
}

void
callSent(PyObject* method, bool sentSynchronously, bool passArg)
{
    PyObjectHandle args;
    if(passArg)
    {
        args = Py_BuildValue(STRCAST("(O)"), sentSynchronously ? getTrue() : getFalse());
    }
    else
    {
        args = PyTuple_New(0);
    }
    PyObjectHandle tmp = PyObject_Call(method, args.get(), 0);
    if(PyErr_Occurred())
    {
        handleException(); // Callback raised an exception.
    }
}

void
callSent(PyObject* callback, const string& method, bool sentSynchronously, bool passArg)
{
    if(PyObject_HasAttrString(callback, STRCAST(method.c_str())))
    {
        PyObjectHandle m = PyObject_GetAttrString(callback, STRCAST(method.c_str()));
        assert(m.get());
        callSent(m.get(), sentSynchronously, passArg);
    }
}

}

#ifdef WIN32
extern "C"
#endif
static OperationObject*
operationNew(PyObject* /*arg*/)
{
    OperationObject* self = PyObject_New(OperationObject, &OperationType);
    if(!self)
    {
        return 0;
    }
    self->op = 0;
    return self;
}

#ifdef WIN32
extern "C"
#endif
static int
operationInit(OperationObject* self, PyObject* args, PyObject* /*kwds*/)
{
    char* name;
    PyObject* modeType = lookupType("Ice.OperationMode");
    assert(modeType);
    PyObject* mode;
    PyObject* sendMode;
    int amd;
    PyObject* meta;
    PyObject* inParams;
    PyObject* outParams;
    PyObject* returnType;
    PyObject* exceptions;
    if(!PyArg_ParseTuple(args, STRCAST("sO!O!iO!O!O!OO!"), &name, modeType, &mode, modeType, &sendMode, &amd,
                         &PyTuple_Type, &meta, &PyTuple_Type, &inParams, &PyTuple_Type, &outParams, &returnType,
                         &PyTuple_Type, &exceptions))
    {
        return -1;
    }

    OperationPtr op = new Operation(name, mode, sendMode, amd, meta, inParams, outParams, returnType, exceptions);
    self->op = new OperationPtr(op);

    return 0;
}

#ifdef WIN32
extern "C"
#endif
static void
operationDealloc(OperationObject* self)
{
    delete self->op;
    PyObject_Del(self);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
operationInvoke(OperationObject* self, PyObject* args)
{
    PyObject* pyProxy;
    PyObject* opArgs;
    if(!PyArg_ParseTuple(args, STRCAST("O!O!"), &ProxyType, &pyProxy, &PyTuple_Type, &opArgs))
    {
        return 0;
    }

    Ice::ObjectPrx prx = getProxy(pyProxy);
    assert(self->op);

    InvocationPtr i = new SyncTypedInvocation(prx, *self->op);
    return i->invoke(opArgs);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
operationInvokeAsync(OperationObject* self, PyObject* args)
{
    PyObject* pyProxy;
    PyObject* opArgs;
    if(!PyArg_ParseTuple(args, STRCAST("O!O!"), &ProxyType, &pyProxy, &PyTuple_Type, &opArgs))
    {
        return 0;
    }

    Ice::ObjectPrx prx = getProxy(pyProxy);
    assert(self->op);

    InvocationPtr i = new OldAsyncTypedInvocation(prx, *self->op);
    return i->invoke(opArgs);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
operationBegin(OperationObject* self, PyObject* args)
{
    PyObject* proxy;
    PyObject* opArgs;
    if(!PyArg_ParseTuple(args, STRCAST("O!O!"), &ProxyType, &proxy, &PyTuple_Type, &opArgs))
    {
        return 0;
    }

    Ice::ObjectPrx p = getProxy(proxy);
    InvocationPtr i = new AsyncTypedInvocation(p, proxy, *self->op);
    return i->invoke(opArgs);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
operationEnd(OperationObject* self, PyObject* args)
{
    PyObject* proxy;
    PyObject* result;
    if(!PyArg_ParseTuple(args, STRCAST("O!O!"), &ProxyType, &proxy, &AsyncResultType, &result))
    {
        return 0;
    }

    AsyncResultObject* ar = reinterpret_cast<AsyncResultObject*>(result);
    assert(ar);
    AsyncTypedInvocationPtr i = AsyncTypedInvocationPtr::dynamicCast(*ar->invocation);
    if(!i)
    {
        PyErr_Format(PyExc_ValueError, STRCAST("invalid AsyncResult object passed to end_%s"),
                     (*self->op)->name.c_str());
        return 0;
    }
    Ice::ObjectPrx p = getProxy(proxy);
    return i->end(p, *self->op, *ar->result);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
operationDeprecate(OperationObject* self, PyObject* args)
{
    char* msg;
    if(!PyArg_ParseTuple(args, STRCAST("s"), &msg))
    {
        return 0;
    }

    assert(self->op);
    (*self->op)->deprecate(msg);

    Py_INCREF(Py_None);
    return Py_None;
}

//
// AMDCallback operations
//

#ifdef WIN32
extern "C"
#endif
static AMDCallbackObject*
amdCallbackNew(PyObject* /*arg*/)
{
    AMDCallbackObject* self = PyObject_New(AMDCallbackObject, &AMDCallbackType);
    if(!self)
    {
        return 0;
    }
    self->upcall = 0;
    return self;
}

#ifdef WIN32
extern "C"
#endif
static void
amdCallbackDealloc(AMDCallbackObject* self)
{
    delete self->upcall;
    PyObject_Del(self);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
amdCallbackIceResponse(AMDCallbackObject* self, PyObject* args)
{
    try
    {
        assert(self->upcall);
        (*self->upcall)->response(args);
    }
    catch(...)
    {
        //
        // No exceptions should propagate to Python.
        //
        assert(false);
    }

    Py_INCREF(Py_None);
    return Py_None;
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
amdCallbackIceException(AMDCallbackObject* self, PyObject* args)
{
    PyObject* ex;
    if(!PyArg_ParseTuple(args, STRCAST("O"), &ex))
    {
        return 0;
    }

    try
    {
        assert(self->upcall);
        PyException pye(ex); // No traceback information available.
        (*self->upcall)->exception(pye);
    }
    catch(...)
    {
        //
        // No exceptions should propagate to Python.
        //
        assert(false);
    }

    Py_INCREF(Py_None);
    return Py_None;
}

//
// AsyncResult operations
//

#ifdef WIN32
extern "C"
#endif
static AsyncResultObject*
asyncResultNew(PyObject* /*arg*/)
{
    AsyncResultObject* self = PyObject_New(AsyncResultObject, &AsyncResultType);
    if(!self)
    {
        return 0;
    }
    self->result = 0;
    self->invocation = 0;
    self->proxy = 0;
    self->connection = 0;
    self->communicator = 0;
    return self;
}

#ifdef WIN32
extern "C"
#endif
static void
asyncResultDealloc(AsyncResultObject* self)
{
    delete self->result;
    delete self->invocation;
    Py_XDECREF(self->proxy);
    Py_XDECREF(self->connection);
    Py_XDECREF(self->communicator);
    PyObject_Del(self);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
asyncResultGetCommunicator(AsyncResultObject* self)
{
    if(self->communicator)
    {
        Py_INCREF(self->communicator);
        return self->communicator;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
asyncResultGetConnection(AsyncResultObject* self)
{
    if(self->connection)
    {
        Py_INCREF(self->connection);
        return self->connection;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
asyncResultGetProxy(AsyncResultObject* self)
{
    if(self->proxy)
    {
        Py_INCREF(self->proxy);
        return self->proxy;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
asyncResultIsCompleted(AsyncResultObject* self)
{
    bool b = false;

    try
    {
        assert(self->result);
        b = (*self->result)->isCompleted();
    }
    catch(...)
    {
        assert(false);
    }

    PyRETURN_BOOL(b);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
asyncResultWaitForCompleted(AsyncResultObject* self)
{
    AllowThreads allowThreads; // Release Python's global interpreter lock during remote invocations.
    try
    {
        assert(self->result);
        (*self->result)->waitForCompleted();
    }
    catch(...)
    {
        assert(false);
    }

    Py_INCREF(Py_None);
    return Py_None;
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
asyncResultIsSent(AsyncResultObject* self)
{
    bool b = false;

    try
    {
        assert(self->result);
        b = (*self->result)->isSent();
    }
    catch(...)
    {
        assert(false);
    }

    PyRETURN_BOOL(b);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
asyncResultWaitForSent(AsyncResultObject* self)
{
    AllowThreads allowThreads; // Release Python's global interpreter lock during remote invocations.
    try
    {
        assert(self->result);
        (*self->result)->waitForSent();
    }
    catch(...)
    {
        assert(false);
    }

    Py_INCREF(Py_None);
    return Py_None;
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
asyncResultThrowLocalException(AsyncResultObject* self)
{
    try
    {
        assert(self->result);
        (*self->result)->throwLocalException();
    }
    catch(const Ice::LocalException& ex)
    {
        setPythonException(ex);
        return 0;
    }
    catch(...)
    {
        assert(false);
    }

    Py_INCREF(Py_None);
    return Py_None;
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
asyncResultSentSynchronously(AsyncResultObject* self)
{
    bool b = false;

    try
    {
        assert(self->result);
        b = (*self->result)->sentSynchronously();
    }
    catch(...)
    {
        assert(false);
    }

    PyRETURN_BOOL(b);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
asyncResultGetOperation(AsyncResultObject* self)
{
    string op;

    try
    {
        //
        // Since the extension uses the Blobject API, calling (*self->result)->getOperation()
        // always returns "ice_invoke" as the operation name. If the caller used a regular
        // (typed) proxy method, we obtain the actual operation name from the invocation.
        //
        if(self->invocation)
        {
            AsyncTypedInvocationPtr i = AsyncTypedInvocationPtr::dynamicCast(*self->invocation);
            if(i)
            {
                op = i->opName();
            }
        }
        if(op.empty())
        {
            assert(self->result);
            op = (*self->result)->getOperation();
        }
    }
    catch(...)
    {
        assert(false);
    }

    return createString(op);
}

//
// ParamInfo implementation.
//
void
IcePy::ParamInfo::unmarshaled(PyObject* val, PyObject* target, void* closure)
{
    assert(PyTuple_Check(target));
    long i = reinterpret_cast<long>(closure);
    PyTuple_SET_ITEM(target, i, val);
    Py_INCREF(val); // PyTuple_SET_ITEM steals a reference.
}

//
// Operation implementation.
//
IcePy::Operation::Operation(const char* n, PyObject* m, PyObject* sm, int amdFlag, PyObject* meta,
                            PyObject* in, PyObject* out, PyObject* ret, PyObject* ex)
{
    name = n;

    //
    // mode
    //
    PyObjectHandle modeValue = PyObject_GetAttrString(m, STRCAST("value"));
    assert(PyInt_Check(modeValue.get()));
    mode = (Ice::OperationMode)static_cast<int>(PyInt_AS_LONG(modeValue.get()));

    //
    // sendMode
    //
    PyObjectHandle sendModeValue = PyObject_GetAttrString(sm, STRCAST("value"));
    assert(PyInt_Check(sendModeValue.get()));
    sendMode = (Ice::OperationMode)static_cast<int>(PyInt_AS_LONG(sendModeValue.get()));

    //
    // amd
    //
    amd = amdFlag ? true : false;
    if(amd)
    {
        dispatchName = fixIdent(name) + "_async";
    }
    else
    {
        dispatchName = fixIdent(name);
    }

    //
    // metaData
    //
#ifndef NDEBUG
    bool b =
#endif
    tupleToStringSeq(meta, metaData);
    assert(b);

    Py_ssize_t i, sz;

    //
    // inParams
    //
    convertParams(in, inParams, sendsClasses);

    //
    // outParams
    //
    convertParams(out, outParams, returnsClasses);

    //
    // returnType
    //
    if(ret != Py_None)
    {
        returnType = new ParamInfo;
        returnType->type = getType(ret);
        if(!returnsClasses)
        {
            returnsClasses = returnType->type->usesClasses();
        }
    }

    //
    // exceptions
    //
    sz = PyTuple_GET_SIZE(ex);
    for(i = 0; i < sz; ++i)
    {
        exceptions.push_back(getException(PyTuple_GET_ITEM(ex, i)));
    }
}

void
IcePy::Operation::deprecate(const string& msg)
{
    if(!msg.empty())
    {
        _deprecateMessage = msg;
    }
    else
    {
        _deprecateMessage = "operation " + name + " is deprecated";
    }
}

void
IcePy::Operation::convertParams(PyObject* p, ParamInfoList& params, bool& usesClasses)
{
    usesClasses = false;
    int sz = static_cast<int>(PyTuple_GET_SIZE(p));
    for(int i = 0; i < sz; ++i)
    {
        PyObject* item = PyTuple_GET_ITEM(p, i);
        assert(PyTuple_Check(item));
        assert(PyTuple_GET_SIZE(item) == 2);

        ParamInfoPtr param = new ParamInfo;

        //
        // metaData
        //
        PyObject* meta = PyTuple_GET_ITEM(item, 0);
        assert(PyTuple_Check(meta));
#ifndef NDEBUG
        bool b =
#endif
        tupleToStringSeq(meta, param->metaData);
        assert(b);

        //
        // type
        //
        param->type = getType(PyTuple_GET_ITEM(item, 1));
        params.push_back(param);
        if(!usesClasses)
        {
            usesClasses = param->type->usesClasses();
        }
    }
}

static PyMethodDef OperationMethods[] =
{
    { STRCAST("invoke"), reinterpret_cast<PyCFunction>(operationInvoke), METH_VARARGS,
      PyDoc_STR(STRCAST("internal function")) },
    { STRCAST("invokeAsync"), reinterpret_cast<PyCFunction>(operationInvokeAsync), METH_VARARGS,
      PyDoc_STR(STRCAST("internal function")) },
    { STRCAST("begin"), reinterpret_cast<PyCFunction>(operationBegin), METH_VARARGS,
      PyDoc_STR(STRCAST("internal function")) },
    { STRCAST("end"), reinterpret_cast<PyCFunction>(operationEnd), METH_VARARGS,
      PyDoc_STR(STRCAST("internal function")) },
    { STRCAST("deprecate"), reinterpret_cast<PyCFunction>(operationDeprecate), METH_VARARGS,
      PyDoc_STR(STRCAST("internal function")) },
    { 0, 0 } /* sentinel */
};

static PyMethodDef AMDCallbackMethods[] =
{
    { STRCAST("ice_response"), reinterpret_cast<PyCFunction>(amdCallbackIceResponse), METH_VARARGS,
      PyDoc_STR(STRCAST("internal function")) },
    { STRCAST("ice_exception"), reinterpret_cast<PyCFunction>(amdCallbackIceException), METH_VARARGS,
      PyDoc_STR(STRCAST("internal function")) },
    { 0, 0 } /* sentinel */
};

static PyMethodDef AsyncResultMethods[] =
{
    { STRCAST("getCommunicator"), reinterpret_cast<PyCFunction>(asyncResultGetCommunicator), METH_NOARGS,
      PyDoc_STR(STRCAST("returns the communicator for the invocation")) },
    { STRCAST("getConnection"), reinterpret_cast<PyCFunction>(asyncResultGetConnection), METH_NOARGS,
      PyDoc_STR(STRCAST("returns the connection for the invocation")) },
    { STRCAST("getProxy"), reinterpret_cast<PyCFunction>(asyncResultGetProxy), METH_NOARGS,
      PyDoc_STR(STRCAST("returns the proxy for the invocation")) },
    { STRCAST("isCompleted"), reinterpret_cast<PyCFunction>(asyncResultIsCompleted), METH_NOARGS,
      PyDoc_STR(STRCAST("returns true if the request is complete")) },
    { STRCAST("waitForCompleted"), reinterpret_cast<PyCFunction>(asyncResultWaitForCompleted), METH_NOARGS,
      PyDoc_STR(STRCAST("blocks until the request is complete")) },
    { STRCAST("isSent"), reinterpret_cast<PyCFunction>(asyncResultIsSent), METH_NOARGS,
      PyDoc_STR(STRCAST("returns true if the request is sent")) },
    { STRCAST("waitForSent"), reinterpret_cast<PyCFunction>(asyncResultWaitForSent), METH_NOARGS,
      PyDoc_STR(STRCAST("blocks until the request is sent")) },
    { STRCAST("throwLocalException"), reinterpret_cast<PyCFunction>(asyncResultThrowLocalException), METH_NOARGS,
      PyDoc_STR(STRCAST("throw location exception if the request failed with a local exception")) },
    { STRCAST("sentSynchronously"), reinterpret_cast<PyCFunction>(asyncResultSentSynchronously), METH_NOARGS,
      PyDoc_STR(STRCAST("returns true if the request was sent synchronously")) },
    { STRCAST("getOperation"), reinterpret_cast<PyCFunction>(asyncResultGetOperation), METH_NOARGS,
      PyDoc_STR(STRCAST("returns the name of the operation")) },
    { 0, 0 } /* sentinel */
};

namespace IcePy
{

PyTypeObject OperationType =
{
    /* The ob_type field must be initialized in the module init function
     * to be portable to Windows without using C++. */
    PyObject_HEAD_INIT(0)
    0,                               /* ob_size */
    STRCAST("IcePy.Operation"),      /* tp_name */
    sizeof(OperationObject),         /* tp_basicsize */
    0,                               /* tp_itemsize */
    /* methods */
    reinterpret_cast<destructor>(operationDealloc), /* tp_dealloc */
    0,                               /* tp_print */
    0,                               /* tp_getattr */
    0,                               /* tp_setattr */
    0,                               /* tp_compare */
    0,                               /* tp_repr */
    0,                               /* tp_as_number */
    0,                               /* tp_as_sequence */
    0,                               /* tp_as_mapping */
    0,                               /* tp_hash */
    0,                               /* tp_call */
    0,                               /* tp_str */
    0,                               /* tp_getattro */
    0,                               /* tp_setattro */
    0,                               /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,              /* tp_flags */
    0,                               /* tp_doc */
    0,                               /* tp_traverse */
    0,                               /* tp_clear */
    0,                               /* tp_richcompare */
    0,                               /* tp_weaklistoffset */
    0,                               /* tp_iter */
    0,                               /* tp_iternext */
    OperationMethods,                /* tp_methods */
    0,                               /* tp_members */
    0,                               /* tp_getset */
    0,                               /* tp_base */
    0,                               /* tp_dict */
    0,                               /* tp_descr_get */
    0,                               /* tp_descr_set */
    0,                               /* tp_dictoffset */
    reinterpret_cast<initproc>(operationInit), /* tp_init */
    0,                               /* tp_alloc */
    reinterpret_cast<newfunc>(operationNew), /* tp_new */
    0,                               /* tp_free */
    0,                               /* tp_is_gc */
};

PyTypeObject AMDCallbackType =
{
    /* The ob_type field must be initialized in the module init function
     * to be portable to Windows without using C++. */
    PyObject_HEAD_INIT(0)
    0,                               /* ob_size */
    STRCAST("IcePy.AMDCallback"),    /* tp_name */
    sizeof(AMDCallbackObject),       /* tp_basicsize */
    0,                               /* tp_itemsize */
    /* methods */
    reinterpret_cast<destructor>(amdCallbackDealloc), /* tp_dealloc */
    0,                               /* tp_print */
    0,                               /* tp_getattr */
    0,                               /* tp_setattr */
    0,                               /* tp_compare */
    0,                               /* tp_repr */
    0,                               /* tp_as_number */
    0,                               /* tp_as_sequence */
    0,                               /* tp_as_mapping */
    0,                               /* tp_hash */
    0,                               /* tp_call */
    0,                               /* tp_str */
    0,                               /* tp_getattro */
    0,                               /* tp_setattro */
    0,                               /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,              /* tp_flags */
    0,                               /* tp_doc */
    0,                               /* tp_traverse */
    0,                               /* tp_clear */
    0,                               /* tp_richcompare */
    0,                               /* tp_weaklistoffset */
    0,                               /* tp_iter */
    0,                               /* tp_iternext */
    AMDCallbackMethods,              /* tp_methods */
    0,                               /* tp_members */
    0,                               /* tp_getset */
    0,                               /* tp_base */
    0,                               /* tp_dict */
    0,                               /* tp_descr_get */
    0,                               /* tp_descr_set */
    0,                               /* tp_dictoffset */
    0,                               /* tp_init */
    0,                               /* tp_alloc */
    reinterpret_cast<newfunc>(amdCallbackNew), /* tp_new */
    0,                               /* tp_free */
    0,                               /* tp_is_gc */
};

PyTypeObject AsyncResultType =
{
    /* The ob_type field must be initialized in the module init function
     * to be portable to Windows without using C++. */
    PyObject_HEAD_INIT(0)
    0,                               /* ob_size */
    STRCAST("IcePy.AsyncResult"),    /* tp_name */
    sizeof(AsyncResultObject),       /* tp_basicsize */
    0,                               /* tp_itemsize */
    /* methods */
    reinterpret_cast<destructor>(asyncResultDealloc), /* tp_dealloc */
    0,                               /* tp_print */
    0,                               /* tp_getattr */
    0,                               /* tp_setattr */
    0,                               /* tp_compare */
    0,                               /* tp_repr */
    0,                               /* tp_as_number */
    0,                               /* tp_as_sequence */
    0,                               /* tp_as_mapping */
    0,                               /* tp_hash */
    0,                               /* tp_call */
    0,                               /* tp_str */
    0,                               /* tp_getattro */
    0,                               /* tp_setattro */
    0,                               /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,              /* tp_flags */
    0,                               /* tp_doc */
    0,                               /* tp_traverse */
    0,                               /* tp_clear */
    0,                               /* tp_richcompare */
    0,                               /* tp_weaklistoffset */
    0,                               /* tp_iter */
    0,                               /* tp_iternext */
    AsyncResultMethods,              /* tp_methods */
    0,                               /* tp_members */
    0,                               /* tp_getset */
    0,                               /* tp_base */
    0,                               /* tp_dict */
    0,                               /* tp_descr_get */
    0,                               /* tp_descr_set */
    0,                               /* tp_dictoffset */
    0,                               /* tp_init */
    0,                               /* tp_alloc */
    reinterpret_cast<newfunc>(asyncResultNew), /* tp_new */
    0,                               /* tp_free */
    0,                               /* tp_is_gc */
};

}

bool
IcePy::initOperation(PyObject* module)
{
    if(PyType_Ready(&OperationType) < 0)
    {
        return false;
    }
    PyTypeObject* opType = &OperationType; // Necessary to prevent GCC's strict-alias warnings.
    if(PyModule_AddObject(module, STRCAST("Operation"), reinterpret_cast<PyObject*>(opType)) < 0)
    {
        return false;
    }

    if(PyType_Ready(&AMDCallbackType) < 0)
    {
        return false;
    }
    PyTypeObject* cbType = &AMDCallbackType; // Necessary to prevent GCC's strict-alias warnings.
    if(PyModule_AddObject(module, STRCAST("AMDCallback"), reinterpret_cast<PyObject*>(cbType)) < 0)
    {
        return false;
    }

    if(PyType_Ready(&AsyncResultType) < 0)
    {
        return false;
    }
    PyTypeObject* arType = &AsyncResultType; // Necessary to prevent GCC's strict-alias warnings.
    if(PyModule_AddObject(module, STRCAST("AsyncResult"), reinterpret_cast<PyObject*>(arType)) < 0)
    {
        return false;
    }

    return true;
}

//
// Invocation
//
IcePy::Invocation::Invocation(const Ice::ObjectPrx& prx) :
    _prx(prx)
{
}

//
// TypedInvocation
//
IcePy::TypedInvocation::TypedInvocation(const Ice::ObjectPrx& prx, const OperationPtr& op) :
    Invocation(prx), _op(op), _communicator(prx->ice_getCommunicator())
{
}

bool
IcePy::TypedInvocation::prepareRequest(PyObject* args, MappingType mapping, vector<Ice::Byte>& bytes)
{
    assert(PyTuple_Check(args));

    //
    // Validate the number of arguments.
    //
    Py_ssize_t argc = PyTuple_GET_SIZE(args);
    Py_ssize_t paramCount = static_cast<Py_ssize_t>(_op->inParams.size());
    if(argc != paramCount)
    {
        string opName;
        if(mapping == OldAsyncMapping)
        {
            opName = _op->name + "_async";
        }
        else if(mapping == AsyncMapping)
        {
            opName = "begin_" + _op->name;
        }
        else
        {
            opName = fixIdent(_op->name);
        }
        PyErr_Format(PyExc_RuntimeError, STRCAST("%s expects %d in parameters"), opName.c_str(),
                     static_cast<int>(paramCount));
        return false;
    }

    if(!_op->inParams.empty())
    {
        try
        {
            //
            // Marshal the in parameters.
            //
            Ice::OutputStreamPtr os = Ice::createOutputStream(_communicator);

            ObjectMap objectMap;
            int i = 0;
            for(ParamInfoList::iterator p = _op->inParams.begin(); p != _op->inParams.end(); ++p, ++i)
            {
                PyObject* arg = PyTuple_GET_ITEM(args, i);
                if(!(*p)->type->validate(arg))
                {
                    string opName;
                    if(mapping == OldAsyncMapping)
                    {
                        opName = _op->name + "_async";
                    }
                    else if(mapping == AsyncMapping)
                    {
                        opName = "begin_" + _op->name;
                    }
                    else
                    {
                        opName = fixIdent(_op->name);
                    }
                    PyErr_Format(PyExc_ValueError, STRCAST("invalid value for argument %d in operation `%s'"),
                                 mapping == OldAsyncMapping ? i + 2 : i + 1, const_cast<char*>(opName.c_str()));
                    return false;
                }
                (*p)->type->marshal(arg, os, &objectMap, &(*p)->metaData);
            }

            if(_op->sendsClasses)
            {
                os->writePendingObjects();
            }

            os->finished(bytes);
        }
        catch(const AbortMarshaling&)
        {
            assert(PyErr_Occurred());
            return false;
        }
        catch(const Ice::Exception& ex)
        {
            setPythonException(ex);
            return false;
        }
    }

    return true;
}

PyObject*
IcePy::TypedInvocation::unmarshalResults(const pair<const Ice::Byte*, const Ice::Byte*>& bytes)
{
    Py_ssize_t i = _op->returnType ? 1 : 0;
    Py_ssize_t numResults = static_cast<Py_ssize_t>(_op->outParams.size()) + i;

    PyObjectHandle results = PyTuple_New(numResults);
    if(results.get() && numResults > 0)
    {
        Ice::InputStreamPtr is = Ice::createInputStream(_communicator, bytes);
        for(ParamInfoList::iterator p = _op->outParams.begin(); p != _op->outParams.end(); ++p, ++i)
        {
            void* closure = reinterpret_cast<void*>(i);
            (*p)->type->unmarshal(is, *p, results.get(), closure, &(*p)->metaData);
        }

        if(_op->returnType)
        {
            _op->returnType->type->unmarshal(is, _op->returnType, results.get(), 0, &_op->metaData);
        }

        if(_op->returnsClasses)
        {
            is->readPendingObjects();
        }
    }

    return results.release();
}

PyObject*
IcePy::TypedInvocation::unmarshalException(const pair<const Ice::Byte*, const Ice::Byte*>& bytes)
{
    int traceSlicing = -1;

    Ice::InputStreamPtr is = Ice::createInputStream(_communicator, bytes);

    bool usesClasses;
    is->read(usesClasses);

    string id;
    is->read(id);
    const string origId = id;

    while(!id.empty())
    {
        ExceptionInfoPtr info = lookupExceptionInfo(id);
        if(info)
        {
            PyObjectHandle ex = info->unmarshal(is);
            if(info->usesClasses)
            {
                is->readPendingObjects();
            }

            if(validateException(ex.get()))
            {
                return ex.release();
            }
            else
            {
                PyException pye(ex.get()); // No traceback information available.
                pye.raise();
            }
        }
        else
        {
            if(traceSlicing == -1)
            {
                traceSlicing = _communicator->getProperties()->getPropertyAsInt("Ice.Trace.Slicing") > 0;
            }

            if(traceSlicing > 0)
            {
                _communicator->getLogger()->trace("Slicing", "unknown exception type `" + id + "'");
            }

            is->skipSlice(); // Slice off what we don't understand.

            try
            {
                is->read(id); // Read type id for next slice.
            }
            catch(Ice::UnmarshalOutOfBoundsException& ex)
            {
                //
                // When readString raises this exception it means we've seen the last slice,
                // so we set the reason member to a more helpful message.
                //
                ex.reason = "unknown exception type `" + origId + "'";
                throw;
            }
        }
    }

    //
    // Getting here should be impossible: we can get here only if the
    // sender has marshaled a sequence of type IDs, none of which we
    // have a factory for. This means that sender and receiver disagree
    // about the Slice definitions they use.
    //
    throw Ice::UnknownUserException(__FILE__, __LINE__, "unknown exception type `" + origId + "'");
}

bool
IcePy::TypedInvocation::validateException(PyObject* ex) const
{
    for(ExceptionInfoList::const_iterator p = _op->exceptions.begin(); p != _op->exceptions.end(); ++p)
    {
        if(PyObject_IsInstance(ex, (*p)->pythonType.get()))
        {
            return true;
        }
    }

    return false;
}

void
IcePy::TypedInvocation::checkTwowayOnly(const Ice::ObjectPrx& proxy) const
{
    if((_op->returnType != 0 || !_op->outParams.empty() || !_op->exceptions.empty()) && !proxy->ice_isTwoway())
    {
        Ice::TwowayOnlyException ex(__FILE__, __LINE__);
        ex.operation = _op->name;
        throw ex;
    }
}

//
// SyncTypedInvocation
//
IcePy::SyncTypedInvocation::SyncTypedInvocation(const Ice::ObjectPrx& prx, const OperationPtr& op) :
    Invocation(prx), TypedInvocation(prx, op)
{
}

PyObject*
IcePy::SyncTypedInvocation::invoke(PyObject* args, PyObject* /* kwds */)
{
    assert(PyTuple_Check(args));
    assert(PyTuple_GET_SIZE(args) == 2); // Format is ((params...), context|None)
    PyObject* pyparams = PyTuple_GET_ITEM(args, 0);
    assert(PyTuple_Check(pyparams));
    PyObject* pyctx = PyTuple_GET_ITEM(args, 1);

    //
    // Marshal the input parameters to a byte sequence.
    //
    Ice::ByteSeq params;
    if(!prepareRequest(pyparams, SyncMapping, params))
    {
        return 0;
    }

    try
    {
        checkTwowayOnly(_prx);

        //
        // Invoke the operation.
        //
        vector<Ice::Byte> result;
        bool status;
        {
            if(pyctx != Py_None)
            {
                Ice::Context ctx;

                if(!PyDict_Check(pyctx))
                {
                    PyErr_Format(PyExc_ValueError, STRCAST("context argument must be None or a dictionary"));
                    return 0;
                }

                if(!dictionaryToContext(pyctx, ctx))
                {
                    return 0;
                }

                AllowThreads allowThreads; // Release Python's global interpreter lock during remote invocations.
                status = _prx->ice_invoke(_op->name, _op->sendMode, params, result, ctx);
            }
            else
            {
                AllowThreads allowThreads; // Release Python's global interpreter lock during remote invocations.
                status = _prx->ice_invoke(_op->name, _op->sendMode, params, result);
            }
        }

        //
        // Process the reply.
        //
        if(_prx->ice_isTwoway())
        {
            if(!status)
            {
                //
                // Unmarshal a user exception.
                //
                pair<const Ice::Byte*, const Ice::Byte*> rb(static_cast<const Ice::Byte*>(0),static_cast<const Ice::Byte*>(0));
                if(!result.empty())
                {
                    rb.first = &result[0];
                    rb.second = &result[0] + result.size();
                }
                PyObjectHandle ex = unmarshalException(rb);

                //
                // Set the Python exception.
                //
                setPythonException(ex.get());
                return 0;
            }
            else if(_op->outParams.size() > 0 || _op->returnType)
            {
                //
                // Unmarshal the results. If there is more than one value to be returned, then return them
                // in a tuple of the form (result, outParam1, ...). Otherwise just return the value.
                //
                pair<const Ice::Byte*, const Ice::Byte*> rb(static_cast<const Ice::Byte*>(0),static_cast<const Ice::Byte*>(0));
                if(!result.empty())
                {
                    rb.first = &result[0];
                    rb.second = &result[0] + result.size();
                }
                PyObjectHandle results = unmarshalResults(rb);
                if(!results.get())
                {
                    return 0;
                }

                if(PyTuple_GET_SIZE(results.get()) > 1)
                {
                    return results.release();
                }
                else
                {
                    PyObject* ret = PyTuple_GET_ITEM(results.get(), 0);
                    if(!ret)
                    {
                        return 0;
                    }
                    else
                    {
                        Py_INCREF(ret);
                        return ret;
                    }
                }
            }
        }
    }
    catch(const AbortMarshaling&)
    {
        assert(PyErr_Occurred());
        return 0;
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

//
// AsyncTypedInvocation
//
IcePy::AsyncTypedInvocation::AsyncTypedInvocation(const Ice::ObjectPrx& prx, PyObject* pyProxy,
                                                  const OperationPtr& op) :
    Invocation(prx), TypedInvocation(prx, op), _pyProxy(pyProxy), _response(0), _ex(0), _sent(0)
{
    Py_INCREF(_pyProxy);
}

IcePy::AsyncTypedInvocation::~AsyncTypedInvocation()
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    Py_DECREF(_pyProxy);
    Py_XDECREF(_response);
    Py_XDECREF(_ex);
    Py_XDECREF(_sent);
}

PyObject*
IcePy::AsyncTypedInvocation::invoke(PyObject* args, PyObject* /* kwds */)
{
    assert(PyTuple_Check(args));
    assert(PyTuple_GET_SIZE(args) == 5); // Format is ((params...), response|None, exception|None, sent|None, ctx|None)
    PyObject* pyparams = PyTuple_GET_ITEM(args, 0);
    assert(PyTuple_Check(pyparams));

    PyObject* callable;

    callable = PyTuple_GET_ITEM(args, 1);
    if(PyCallable_Check(callable))
    {
        _response = callable;
        Py_INCREF(_response);
    }
    else if(callable != Py_None)
    {
        PyErr_Format(PyExc_RuntimeError, STRCAST("response callback must be a callable object or None"));
        return 0;
    }

    callable = PyTuple_GET_ITEM(args, 2);
    if(PyCallable_Check(callable))
    {
        _ex = callable;
        Py_INCREF(_ex);
    }
    else if(callable != Py_None)
    {
        PyErr_Format(PyExc_RuntimeError, STRCAST("exception callback must be a callable object or None"));
        return 0;
    }

    callable = PyTuple_GET_ITEM(args, 3);
    if(PyCallable_Check(callable))
    {
        _sent = callable;
        Py_INCREF(_sent);
    }
    else if(callable != Py_None)
    {
        PyErr_Format(PyExc_RuntimeError, STRCAST("sent callback must be a callable object or None"));
        return 0;
    }

    if(!_ex && (_response || _sent))
    {
        PyErr_Format(PyExc_RuntimeError,
            STRCAST("exception callback must also be provided when response or sent callbacks are used"));
        return 0;
    }

    PyObject* pyctx = PyTuple_GET_ITEM(args, 4);
    if(pyctx != Py_None && !PyDict_Check(pyctx))
    {
        PyErr_Format(PyExc_RuntimeError, STRCAST("context must be a dictionary or None"));
        return 0;
    }

    //
    // Marshal the input parameters to a byte sequence.
    //
    Ice::ByteSeq params;
    if(!prepareRequest(pyparams, AsyncMapping, params))
    {
        return 0;
    }

    Ice::AsyncResultPtr result;
    try
    {
        checkAsyncTwowayOnly(_prx);
        pair<const Ice::Byte*, const Ice::Byte*> pparams(static_cast<const Ice::Byte*>(0),static_cast<const Ice::Byte*>(0));
        if(!params.empty())
        {
            pparams.first = &params[0];
            pparams.second = &params[0] + params.size();
        }

        Ice::Callback_Object_ice_invokePtr cb;
        if(_response || _ex || _sent)
        {
            cb = Ice::newCallback_Object_ice_invoke(this, &AsyncTypedInvocation::response,
                                                    &AsyncTypedInvocation::exception, &AsyncTypedInvocation::sent);
        }

        //
        // Invoke the operation asynchronously.
        //
        if(pyctx != Py_None)
        {
            Ice::Context ctx;
            if(!dictionaryToContext(pyctx, ctx))
            {
                return 0;
            }

            AllowThreads allowThreads; // Release Python's global interpreter lock during remote invocations.
            if(cb)
            {
                result = _prx->begin_ice_invoke(_op->name, _op->sendMode, pparams, ctx, cb);
            }
            else
            {
                result = _prx->begin_ice_invoke(_op->name, _op->sendMode, pparams, ctx);
            }
        }
        else
        {
            AllowThreads allowThreads; // Release Python's global interpreter lock during remote invocations.
            if(cb)
            {
                result = _prx->begin_ice_invoke(_op->name, _op->sendMode, pparams, cb);
            }
            else
            {
                result = _prx->begin_ice_invoke(_op->name, _op->sendMode, pparams);
            }
        }
    }
    catch(const Ice::CommunicatorDestroyedException& ex)
    {
        //
        // CommunicatorDestroyedException can propagate directly.
        //
        setPythonException(ex);
        return 0;
    }
    catch(const IceUtil::IllegalArgumentException& ex)
    {
        //
        // IllegalArgumentException can propagate directly.
        // (Raised by checkAsyncTwowayOnly)
        //
        PyErr_Format(PyExc_RuntimeError, STRCAST(ex.reason().c_str()));
        return 0;
    }
    catch(const Ice::Exception&)
    {
        //
        // No other exceptions should be raised by begin_ice_invoke.
        //
        assert(false);
    }

    assert(result);
    AsyncResultObject* obj = asyncResultNew(0);
    if(!obj)
    {
        return 0;
    }
    obj->result = new Ice::AsyncResultPtr(result);
    obj->invocation = new InvocationPtr(this);
    obj->proxy = _pyProxy;
    Py_INCREF(obj->proxy);
    return reinterpret_cast<PyObject*>(obj);
}

PyObject*
IcePy::AsyncTypedInvocation::end(const Ice::ObjectPrx& proxy, const OperationPtr& op, const Ice::AsyncResultPtr& r)
{
    try
    {
        if(op.get() != _op.get())
        {
            throw IceUtil::IllegalArgumentException(__FILE__, __LINE__, "end_" + op->name +
                                                    " called with AsyncResult object from begin_" + _op->name);
        }

        pair<const Ice::Byte*, const Ice::Byte*> results;
        bool ok;

        {
            AllowThreads allowThreads; // Release Python's global interpreter lock during blocking operations.
            ok = proxy->___end_ice_invoke(results, r);
        }

        if(ok)
        {
            //
            // Unmarshal the results.
            //
            PyObjectHandle args = unmarshalResults(results);
            if(args.get())
            {
                //
                // If there are no results, return None. If there's only one element
                // in the tuple, return the element. Otherwise, return the tuple.
                //
                assert(PyTuple_Check(args.get()));
                if(PyTuple_GET_SIZE(args.get()) == 0)
                {
                    Py_INCREF(Py_None);
                    return Py_None;
                }
                else if(PyTuple_GET_SIZE(args.get()) == 1)
                {
                    PyObject* res = PyTuple_GET_ITEM(args.get(), 0);
                    Py_INCREF(res);
                    return res;
                }
                else
                {
                    return args.release();
                }
            }
        }
        else
        {
            PyObjectHandle ex = unmarshalException(results);
            setPythonException(ex.get());
        }
    }
    catch(const AbortMarshaling&)
    {
        // Nothing to do.
    }
    catch(const IceUtil::IllegalArgumentException& ex)
    {
        PyErr_Format(PyExc_RuntimeError, STRCAST(ex.reason().c_str()));
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
    }

    assert(PyErr_Occurred());
    return 0;
}

string
IcePy::AsyncTypedInvocation::opName() const
{
    return _op->name;
}

void
IcePy::AsyncTypedInvocation::response(bool ok, const pair<const Ice::Byte*, const Ice::Byte*>& results)
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    try
    {
        if(ok)
        {
            if(_response)
            {
                //
                // Unmarshal the results.
                //
                PyObjectHandle args;
                try
                {
                    args = unmarshalResults(results);
                    if(!args.get())
                    {
                        assert(PyErr_Occurred());
                        PyErr_Print();
                        return;
                    }
                }
                catch(const Ice::Exception& ex)
                {
                    assert(_ex);
                    callException(_ex, ex);
                    return;
                }

                PyObjectHandle tmp = PyObject_Call(_response, args.get(), 0);
                if(PyErr_Occurred())
                {
                    handleException(); // Callback raised an exception.
                }
            }
        }
        else
        {
            assert(_ex);
            PyObjectHandle ex = unmarshalException(results);
            callException(_ex, ex.get());
        }
    }
    catch(const AbortMarshaling&)
    {
        assert(PyErr_Occurred());
        PyErr_Print();
    }
}

void
IcePy::AsyncTypedInvocation::exception(const Ice::Exception& ex)
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.
    assert(_ex);
    callException(_ex, ex);
}

void
IcePy::AsyncTypedInvocation::sent(bool sentSynchronously)
{
    if(_sent)
    {
        AdoptThread adoptThread; // Ensure the current thread is able to call into Python.
        callSent(_sent, sentSynchronously, true);
    }
}

void
IcePy::AsyncTypedInvocation::checkAsyncTwowayOnly(const Ice::ObjectPrx& proxy) const
{
    if((_op->returnType != 0 || !_op->outParams.empty() || !_op->exceptions.empty()) && !proxy->ice_isTwoway())
    {
        throw IceUtil::IllegalArgumentException(__FILE__, __LINE__,
                                                "`" + _op->name + "' can only be called with a twoway proxy");
    }

    if((_op->returnType != 0 || !_op->outParams.empty()) && (!_response && (_ex || _sent)))
    {
        throw IceUtil::IllegalArgumentException(__FILE__, __LINE__, "`" + _op->name + "' requires a response callback");
    }
}

//
// OldAsyncTypedInvocation
//
IcePy::OldAsyncTypedInvocation::OldAsyncTypedInvocation(const Ice::ObjectPrx& prx, const OperationPtr& op)
    : Invocation(prx), TypedInvocation(prx, op), _callback(0)
{
}

IcePy::OldAsyncTypedInvocation::~OldAsyncTypedInvocation()
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    Py_XDECREF(_callback);
}

PyObject*
IcePy::OldAsyncTypedInvocation::invoke(PyObject* args, PyObject* /* kwds */)
{
    assert(PyTuple_Check(args));
    assert(PyTuple_GET_SIZE(args) == 3); // Format is (callback, (params...), context|None)
    _callback = PyTuple_GET_ITEM(args, 0);
    Py_INCREF(_callback);
    PyObject* pyparams = PyTuple_GET_ITEM(args, 1);
    assert(PyTuple_Check(pyparams));
    PyObject* pyctx = PyTuple_GET_ITEM(args, 2);

    //
    // Marshal the input parameters to a byte sequence.
    //
    Ice::ByteSeq params;
    if(!prepareRequest(pyparams, OldAsyncMapping, params))
    {
        return 0;
    }

    bool sentSynchronously = false;
    try
    {
        checkTwowayOnly(_prx);
        pair<const Ice::Byte*, const Ice::Byte*> pparams(static_cast<const Ice::Byte*>(0),static_cast<const Ice::Byte*>(0));
        if(!params.empty())
        {
            pparams.first = &params[0];
            pparams.second = &params[0] + params.size();
        }

        Ice::Callback_Object_ice_invokePtr cb =
            Ice::newCallback_Object_ice_invoke(this, &OldAsyncTypedInvocation::response,
                                               &OldAsyncTypedInvocation::exception, &OldAsyncTypedInvocation::sent);

        Ice::AsyncResultPtr result;

        //
        // Invoke the operation asynchronously.
        //
        if(pyctx != Py_None)
        {
            Ice::Context ctx;

            if(!PyDict_Check(pyctx))
            {
                PyErr_Format(PyExc_ValueError, STRCAST("context argument must be None or a dictionary"));
                return 0;
            }

            if(!dictionaryToContext(pyctx, ctx))
            {
                return 0;
            }

            AllowThreads allowThreads; // Release Python's global interpreter lock during remote invocations.
            result = _prx->begin_ice_invoke(_op->name, _op->sendMode, pparams, ctx, cb);
        }
        else
        {
            AllowThreads allowThreads; // Release Python's global interpreter lock during remote invocations.
            result = _prx->begin_ice_invoke(_op->name, _op->sendMode, pparams, cb);
        }

        sentSynchronously = result->sentSynchronously();
    }
    catch(const Ice::CommunicatorDestroyedException& ex)
    {
        //
        // CommunicatorDestroyedException can propagate directly.
        //
        setPythonException(ex);
        return 0;
    }
    catch(const Ice::TwowayOnlyException& ex)
    {
        //
        // Raised by checkTwowayOnly.
        //
        callException(_callback, _op->name, "ice_exception", ex);
    }
    catch(const Ice::Exception&)
    {
        //
        // No other exceptions should be raised by begin_ice_invoke.
        //
        assert(false);
    }

    PyRETURN_BOOL(sentSynchronously);
}

void
IcePy::OldAsyncTypedInvocation::response(bool ok, const pair<const Ice::Byte*, const Ice::Byte*>& results)
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    assert(_callback);

    try
    {
        if(ok)
        {
            //
            // Unmarshal the results.
            //
            PyObjectHandle args;
            try
            {
                args = unmarshalResults(results);
                if(!args.get())
                {
                    assert(PyErr_Occurred());
                    PyErr_Print();
                    return;
                }
            }
            catch(const Ice::Exception& ex)
            {
                callException(_callback, _op->name, "ice_exception", ex);
                return;
            }

            const string methodName = "ice_response";
            if(!PyObject_HasAttrString(_callback, STRCAST(methodName.c_str())))
            {
                ostringstream ostr;
                ostr << "AMI callback object for operation `" << _op->name << "' does not define " << methodName
                     << "()";
                string str = ostr.str();
                PyErr_Warn(PyExc_RuntimeWarning, const_cast<char*>(str.c_str()));
            }
            else
            {
                PyObjectHandle method = PyObject_GetAttrString(_callback, STRCAST(methodName.c_str()));
                assert(method.get());
                PyObjectHandle tmp = PyObject_Call(method.get(), args.get(), 0);
                if(PyErr_Occurred())
                {
                    handleException(); // Callback raised an exception.
                }
            }
        }
        else
        {
            PyObjectHandle ex = unmarshalException(results);
            callException(_callback, _op->name, "ice_exception", ex.get());
        }
    }
    catch(const AbortMarshaling&)
    {
        assert(PyErr_Occurred());
        PyErr_Print();
    }
}

void
IcePy::OldAsyncTypedInvocation::exception(const Ice::Exception& ex)
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    callException(_callback, _op->name, "ice_exception", ex);
}

void
IcePy::OldAsyncTypedInvocation::sent(bool)
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    callSent(_callback, "ice_sent", false, false);
}

//
// SyncBlobjectInvocation
//
IcePy::SyncBlobjectInvocation::SyncBlobjectInvocation(const Ice::ObjectPrx& prx)
    : Invocation(prx)
{
}

PyObject*
IcePy::SyncBlobjectInvocation::invoke(PyObject* args, PyObject* /* kwds */)
{
    char* operation;
    PyObject* mode;
    PyObject* inParams;
    PyObject* operationModeType = lookupType("Ice.OperationMode");
    PyObject* ctx = 0;
    if(!PyArg_ParseTuple(args, STRCAST("sO!O!|O"), &operation, operationModeType, &mode, &PyBuffer_Type, &inParams,
                         &ctx))
    {
        return 0;
    }

    PyObjectHandle modeValue = PyObject_GetAttrString(mode, STRCAST("value"));
    Ice::OperationMode sendMode = (Ice::OperationMode)static_cast<int>(PyInt_AS_LONG(modeValue.get()));

    //
    // Use the array API to avoid copying the data.
    //
#if PY_VERSION_HEX < 0x02050000
    const char* charBuf = 0;
#else
    char* charBuf = 0;
#endif
    Py_ssize_t sz = inParams->ob_type->tp_as_buffer->bf_getcharbuffer(inParams, 0, &charBuf);
    const Ice::Byte* mem = reinterpret_cast<const Ice::Byte*>(charBuf);
    pair<const ::Ice::Byte*, const ::Ice::Byte*> in(static_cast<const Ice::Byte*>(0),static_cast<const Ice::Byte*>(0));
    if(sz > 0)
    {
        in.first = mem;
        in.second = mem + sz;
    }

    try
    {
        vector<Ice::Byte> out;

        bool ok;
        if(ctx == 0 || ctx == Py_None)
        {
            AllowThreads allowThreads; // Release Python's global interpreter lock during remote invocations.
            ok = _prx->ice_invoke(operation, sendMode, in, out);
        }
        else
        {
            Ice::Context context;
            if(!dictionaryToContext(ctx, context))
            {
                return 0;
            }

            AllowThreads allowThreads; // Release Python's global interpreter lock during remote invocations.
            ok = _prx->ice_invoke(operation, sendMode, in, out, context);
        }

        //
        // Prepare the result as a tuple of the bool and out param buffer.
        //
        PyObjectHandle result = PyTuple_New(2);
        if(!result.get())
        {
            throwPythonException();
        }

        if(PyTuple_SET_ITEM(result.get(), 0, ok ? getTrue() : getFalse()) < 0)
        {
            throwPythonException();
        }

        //
        // Create the output buffer and copy in the outParams.
        //
        PyObjectHandle ip = PyBuffer_New(out.size());
        if(!ip.get())
        {
            throwPythonException();
        }
        if(!out.empty())
        {
            void* buf;
            Py_ssize_t sz;
            if(PyObject_AsWriteBuffer(ip.get(), &buf, &sz))
            {
                throwPythonException();
            }
            memcpy(buf, &out[0], sz);
        }
        
        if(PyTuple_SET_ITEM(result.get(), 1, ip.get()) < 0)
        {
            throwPythonException();
        }
        ip.release(); // PyTuple_SET_ITEM steals a reference.

        return result.release();
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }
}

//
// AsyncBlobjectInvocation
//
IcePy::AsyncBlobjectInvocation::AsyncBlobjectInvocation(const Ice::ObjectPrx& prx, PyObject* pyProxy) :
    Invocation(prx), _pyProxy(pyProxy), _response(0), _ex(0), _sent(0)
{
    Py_INCREF(_pyProxy);
}

IcePy::AsyncBlobjectInvocation::~AsyncBlobjectInvocation()
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    Py_DECREF(_pyProxy);
    Py_XDECREF(_response);
    Py_XDECREF(_ex);
    Py_XDECREF(_sent);
}

PyObject*
IcePy::AsyncBlobjectInvocation::invoke(PyObject* args, PyObject* kwds)
{
    static char* argNames[] =
    {
        const_cast<char*>("op"),
        const_cast<char*>("mode"),
        const_cast<char*>("inParams"),
        const_cast<char*>("_response"),
        const_cast<char*>("_ex"),
        const_cast<char*>("_sent"),
        const_cast<char*>("_ctx"),
        0
    };
    char* operation;
    PyObject* mode;
    PyObject* inParams;
    PyObject* operationModeType = lookupType("Ice.OperationMode");
    PyObject* response = Py_None;
    PyObject* ex = Py_None;
    PyObject* sent = Py_None;
    PyObject* pyctx = Py_None;
    if(!PyArg_ParseTupleAndKeywords(args, kwds, STRCAST("sO!O!|OOOO"), argNames, &operation, operationModeType, &mode,
                                    &PyBuffer_Type, &inParams, &response, &ex, &sent, &pyctx))
    {
        return 0;
    }

    _op = operation;

    PyObjectHandle modeValue = PyObject_GetAttrString(mode, STRCAST("value"));
    Ice::OperationMode sendMode = (Ice::OperationMode)static_cast<int>(PyInt_AS_LONG(modeValue.get()));

    if(PyCallable_Check(response))
    {
        _response = response;
        Py_INCREF(_response);
    }
    else if(response != Py_None)
    {
        PyErr_Format(PyExc_RuntimeError, STRCAST("response callback must be a callable object or None"));
        return 0;
    }

    if(PyCallable_Check(ex))
    {
        _ex = ex;
        Py_INCREF(_ex);
    }
    else if(ex != Py_None)
    {
        PyErr_Format(PyExc_RuntimeError, STRCAST("exception callback must be a callable object or None"));
        return 0;
    }

    if(PyCallable_Check(sent))
    {
        _sent = sent;
        Py_INCREF(_sent);
    }
    else if(sent != Py_None)
    {
        PyErr_Format(PyExc_RuntimeError, STRCAST("sent callback must be a callable object or None"));
        return 0;
    }

    if(!_ex && (_response || _sent))
    {
        PyErr_Format(PyExc_RuntimeError,
            STRCAST("exception callback must also be provided when response or sent callbacks are used"));
        return 0;
    }

    if(pyctx != Py_None && !PyDict_Check(pyctx))
    {
        PyErr_Format(PyExc_RuntimeError, STRCAST("context must be a dictionary or None"));
        return 0;
    }

    //
    // Use the array API to avoid copying the data.
    //
#if PY_VERSION_HEX < 0x02050000
    const char* charBuf = 0;
#else
    char* charBuf = 0;
#endif
    Py_ssize_t sz = inParams->ob_type->tp_as_buffer->bf_getcharbuffer(inParams, 0, &charBuf);
    const Ice::Byte* mem = reinterpret_cast<const Ice::Byte*>(charBuf);
    pair<const ::Ice::Byte*, const ::Ice::Byte*> in(static_cast<const Ice::Byte*>(0),static_cast<const Ice::Byte*>(0));
    if(sz > 0)
    {
        in.first = mem;
        in.second = mem + sz;
    }

    Ice::AsyncResultPtr result;
    try
    {
        Ice::Callback_Object_ice_invokePtr cb;
        if(_response || _ex || _sent)
        {
            cb = Ice::newCallback_Object_ice_invoke(this, &AsyncBlobjectInvocation::response,
                                                    &AsyncBlobjectInvocation::exception,
                                                    &AsyncBlobjectInvocation::sent);
        }

        if(pyctx == Py_None)
        {
            AllowThreads allowThreads; // Release Python's global interpreter lock during remote invocations.
            if(cb)
            {
                result = _prx->begin_ice_invoke(operation, sendMode, in, cb);
            }
            else
            {
                result = _prx->begin_ice_invoke(operation, sendMode, in);
            }
        }
        else
        {
            Ice::Context context;
            if(!dictionaryToContext(pyctx, context))
            {
                return 0;
            }

            AllowThreads allowThreads; // Release Python's global interpreter lock during remote invocations.
            if(cb)
            {
                result = _prx->begin_ice_invoke(operation, sendMode, in, context, cb);
            }
            else
            {
                result = _prx->begin_ice_invoke(operation, sendMode, in, context);
            }
        }
    }
    catch(const Ice::CommunicatorDestroyedException& ex)
    {
        //
        // CommunicatorDestroyedException is the only exception that can propagate directly.
        //
        setPythonException(ex);
        return 0;
    }
    catch(const Ice::Exception&)
    {
        //
        // No other exceptions should be raised by begin_ice_invoke.
        //
        assert(false);
    }

    assert(result);
    AsyncResultObject* obj = asyncResultNew(0);
    if(!obj)
    {
        return 0;
    }
    obj->result = new Ice::AsyncResultPtr(result);
    obj->invocation = new InvocationPtr(this);
    obj->proxy = _pyProxy;
    Py_INCREF(obj->proxy);
    return reinterpret_cast<PyObject*>(obj);
}

PyObject*
IcePy::AsyncBlobjectInvocation::end(const Ice::ObjectPrx& proxy, const Ice::AsyncResultPtr& r)
{
    try
    {
        pair<const Ice::Byte*, const Ice::Byte*> results;
        bool ok;

        {
            AllowThreads allowThreads; // Release Python's global interpreter lock during blocking operations.
            ok = proxy->___end_ice_invoke(results, r);
        }

        //
        // Prepare the results as a tuple of the bool and out param buffer.
        //
        PyObjectHandle args = PyTuple_New(2);
        if(!args.get())
        {
            return 0;
        }

        if(PyTuple_SET_ITEM(args.get(), 0, ok ? getTrue() : getFalse()) < 0)
        {
            return 0;
        }

        //
        // Create the output buffer and copy in the outParams.
        //
        PyObjectHandle ip = PyBuffer_New(results.second - results.first);
        if(!ip.get())
        {
            return 0;
        }

        void* buf;
        Py_ssize_t sz;
        if(PyObject_AsWriteBuffer(ip.get(), &buf, &sz))
        {
            return 0;
        }
        assert(sz == results.second - results.first);
        memcpy(buf, results.first, sz);

        if(PyTuple_SET_ITEM(args.get(), 1, ip.get()) < 0)
        {
            return 0;
        }
        ip.release(); // PyTuple_SET_ITEM steals a reference.

        return args.release();
    }
    catch(const AbortMarshaling&)
    {
        // Nothing to do.
    }
    catch(const IceUtil::IllegalArgumentException& ex)
    {
        PyErr_Format(PyExc_RuntimeError, STRCAST(ex.reason().c_str()));
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
    }

    assert(PyErr_Occurred());
    return 0;
}

void
IcePy::AsyncBlobjectInvocation::response(bool ok, const pair<const Ice::Byte*, const Ice::Byte*>& results)
{
    if(_response)
    {
        AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

        //
        // Prepare the args as a tuple of the bool and out param buffer.
        //
        PyObjectHandle args = PyTuple_New(2);
        if(!args.get())
        {
            assert(PyErr_Occurred());
            PyErr_Print();
            return;
        }

        if(PyTuple_SET_ITEM(args.get(), 0, ok ? getTrue() : getFalse()) < 0)
        {
            assert(PyErr_Occurred());
            PyErr_Print();
            return;
        }

        //
        // Create the output buffer and copy in the outParams.
        //
        PyObjectHandle ip = PyBuffer_New(results.second - results.first);
        if(!ip.get())
        {
            assert(PyErr_Occurred());
            PyErr_Print();
            return;
        }

        void* buf;
        Py_ssize_t sz;
        if(PyObject_AsWriteBuffer(ip.get(), &buf, &sz))
        {
            assert(PyErr_Occurred());
            PyErr_Print();
            return;
        }
        assert(sz == results.second - results.first);
        memcpy(buf, results.first, sz);

        if(PyTuple_SET_ITEM(args.get(), 1, ip.get()) < 0)
        {
            assert(PyErr_Occurred());
            PyErr_Print();
            return;
        }
        ip.release(); // PyTuple_SET_ITEM steals a reference.

        PyObjectHandle tmp = PyObject_Call(_response, args.get(), 0);
        if(PyErr_Occurred())
        {
            handleException(); // Callback raised an exception.
        }
    }
}

void
IcePy::AsyncBlobjectInvocation::exception(const Ice::Exception& ex)
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.
    assert(_ex);
    callException(_ex, ex);
}

void
IcePy::AsyncBlobjectInvocation::sent(bool sentSynchronously)
{
    if(_sent)
    {
        AdoptThread adoptThread; // Ensure the current thread is able to call into Python.
        callSent(_sent, sentSynchronously, true);
    }
}

//
// OldAsyncBlobjectInvocation
//
IcePy::OldAsyncBlobjectInvocation::OldAsyncBlobjectInvocation(const Ice::ObjectPrx& prx) :
    Invocation(prx), _callback(0)
{
}

IcePy::OldAsyncBlobjectInvocation::~OldAsyncBlobjectInvocation()
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    Py_XDECREF(_callback);
}

PyObject*
IcePy::OldAsyncBlobjectInvocation::invoke(PyObject* args, PyObject* /* kwds */)
{
    char* operation;
    PyObject* mode;
    PyObject* inParams;
    PyObject* operationModeType = lookupType("Ice.OperationMode");
    PyObject* ctx = 0;
    if(!PyArg_ParseTuple(args, STRCAST("OsO!O!|O"), &_callback, &operation, operationModeType, &mode,
                         &PyBuffer_Type, &inParams, &ctx))
    {
        return 0;
    }

    Py_INCREF(_callback);
    _op = operation;

    PyObjectHandle modeValue = PyObject_GetAttrString(mode, STRCAST("value"));
    Ice::OperationMode sendMode = (Ice::OperationMode)static_cast<int>(PyInt_AS_LONG(modeValue.get()));

    //
    // Use the array API to avoid copying the data.
    //
#if PY_VERSION_HEX < 0x02050000
    const char* charBuf = 0;
#else
    char* charBuf = 0;
#endif
    Py_ssize_t sz = inParams->ob_type->tp_as_buffer->bf_getcharbuffer(inParams, 0, &charBuf);
    const Ice::Byte* mem = reinterpret_cast<const Ice::Byte*>(charBuf);
    pair<const ::Ice::Byte*, const ::Ice::Byte*> in(static_cast<const Ice::Byte*>(0), static_cast<const Ice::Byte*>(0));
    if(sz > 0)
    {
        in.first = mem;
        in.second = mem + sz;
    }

    bool sentSynchronously = false;
    try
    {
        Ice::AsyncResultPtr result;
        Ice::Callback_Object_ice_invokePtr cb =
            Ice::newCallback_Object_ice_invoke(this, &OldAsyncBlobjectInvocation::response,
                                               &OldAsyncBlobjectInvocation::exception,
                                               &OldAsyncBlobjectInvocation::sent);

        if(ctx == 0 || ctx == Py_None)
        {
            AllowThreads allowThreads; // Release Python's global interpreter lock during remote invocations.
            result = _prx->begin_ice_invoke(operation, sendMode, in, cb);
        }
        else
        {
            Ice::Context context;
            if(!dictionaryToContext(ctx, context))
            {
                return 0;
            }

            AllowThreads allowThreads; // Release Python's global interpreter lock during remote invocations.
            result = _prx->begin_ice_invoke(operation, sendMode, in, context, cb);
        }

        sentSynchronously = result->sentSynchronously();
    }
    catch(const Ice::CommunicatorDestroyedException& ex)
    {
        //
        // CommunicatorDestroyedException is the only exception that can propagate directly.
        //
        setPythonException(ex);
        return 0;
    }
    catch(const Ice::Exception&)
    {
        //
        // No other exceptions should be raised by begin_ice_invoke.
        //
        assert(false);
    }

    PyRETURN_BOOL(sentSynchronously);
}

void
IcePy::OldAsyncBlobjectInvocation::response(bool ok, const pair<const Ice::Byte*, const Ice::Byte*>& results)
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    try
    {
        //
        // Prepare the args as a tuple of the bool and out param buffer.
        //
        PyObjectHandle args = PyTuple_New(2);
        if(!args.get())
        {
            assert(PyErr_Occurred());
            PyErr_Print();
            return;
        }

        if(PyTuple_SET_ITEM(args.get(), 0, ok ? getTrue() : getFalse()) < 0)
        {
            assert(PyErr_Occurred());
            PyErr_Print();
            return;
        }

        //
        // Create the output buffer and copy in the outParams.
        //
        PyObjectHandle ip = PyBuffer_New(results.second - results.first);
        if(!ip.get())
        {
            assert(PyErr_Occurred());
            PyErr_Print();
            return;
        }

        void* buf;
        Py_ssize_t sz;
        if(PyObject_AsWriteBuffer(ip.get(), &buf, &sz))
        {
            assert(PyErr_Occurred());
            PyErr_Print();
            return;
        }
        assert(sz == results.second - results.first);
        memcpy(buf, results.first, sz);

        if(PyTuple_SET_ITEM(args.get(), 1, ip.get()) < 0)
        {
            assert(PyErr_Occurred());
            PyErr_Print();
            return;
        }
        ip.release(); // PyTuple_SET_ITEM steals a reference.

        const string methodName = "ice_response";
        if(!PyObject_HasAttrString(_callback, STRCAST(methodName.c_str())))
        {
            ostringstream ostr;
            ostr << "AMI callback object for operation `ice_invoke_async' does not define " << methodName << "()";
            string str = ostr.str();
            PyErr_Warn(PyExc_RuntimeWarning, const_cast<char*>(str.c_str()));
        }
        else
        {
            PyObjectHandle method = PyObject_GetAttrString(_callback, STRCAST(methodName.c_str()));
            assert(method.get());
            PyObjectHandle tmp = PyObject_Call(method.get(), args.get(), 0);
            if(PyErr_Occurred())
            {
                handleException(); // Callback raised an exception.
            }
        }
    }
    catch(const Ice::Exception& ex)
    {
        ostringstream ostr;
        ostr << "Exception raised by AMI callback for operation `ice_invoke_async':" << ex;
        string str = ostr.str();
        PyErr_Warn(PyExc_RuntimeWarning, const_cast<char*>(str.c_str()));
    }
}

void
IcePy::OldAsyncBlobjectInvocation::exception(const Ice::Exception& ex)
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    callException(_callback, "ice_invoke", "ice_exception", ex);
}

void
IcePy::OldAsyncBlobjectInvocation::sent(bool)
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    callSent(_callback, "ice_sent", false, false);
}

//
// TypedUpcall
//
IcePy::TypedUpcall::TypedUpcall(const OperationPtr& op, const Ice::AMD_Object_ice_invokePtr& callback,
                                const Ice::CommunicatorPtr& communicator) :
    _op(op), _callback(callback), _communicator(communicator), _finished(false)
{
}

void
IcePy::TypedUpcall::dispatch(PyObject* servant, const pair<const Ice::Byte*, const Ice::Byte*>& inBytes,
                             const Ice::Current& current)
{
    //
    // Unmarshal the in parameters. We have to leave room in the arguments for a trailing
    // Ice::Current object.
    //
    Py_ssize_t count = static_cast<Py_ssize_t>(_op->inParams.size()) + 1;

    Py_ssize_t start = 0;
    if(_op->amd)
    {
        ++count; // Leave room for a leading AMD callback argument.
        start = 1;
    }

    PyObjectHandle args = PyTuple_New(count);
    if(!args.get())
    {
        throwPythonException();
    }

    if(!_op->inParams.empty())
    {
        Ice::InputStreamPtr is = Ice::createInputStream(_communicator, inBytes);
        try
        {
            Py_ssize_t i = start;
            for(ParamInfoList::iterator p = _op->inParams.begin(); p != _op->inParams.end(); ++p, ++i)
            {
                void* closure = reinterpret_cast<void*>(i);
                (*p)->type->unmarshal(is, *p, args.get(), closure, &(*p)->metaData);
            }
            if(_op->sendsClasses)
            {
                is->readPendingObjects();
            }
        }
        catch(const AbortMarshaling&)
        {
            throwPythonException();
        }
    }

    //
    // Create an object to represent Ice::Current. We need to append this to the argument tuple.
    //
    PyObjectHandle curr = createCurrent(current);
    if(PyTuple_SET_ITEM(args.get(), PyTuple_GET_SIZE(args.get()) - 1, curr.get()) < 0)
    {
        throwPythonException();
    }
    curr.release(); // PyTuple_SET_ITEM steals a reference.

    if(_op->amd)
    {
        //
        // Create the callback object and pass it as the first argument.
        //
        AMDCallbackObject* obj = amdCallbackNew(0);
        if(!obj)
        {
            throwPythonException();
        }
        obj->upcall = new UpcallPtr(this);
        if(PyTuple_SET_ITEM(args.get(), 0, (PyObject*)obj) < 0) // PyTuple_SET_ITEM steals a reference.
        {
            Py_DECREF(obj);
            throwPythonException();
        }
    }

    //
    // Dispatch the operation. Use _dispatchName here, not current.operation.
    //
    PyObjectHandle method = PyObject_GetAttrString(servant, const_cast<char*>(_op->dispatchName.c_str()));
    if(!method.get())
    {
        ostringstream ostr;
        ostr << "servant for identity " << _communicator->identityToString(current.id)
             << " does not define operation `" << _op->dispatchName << "'";
        string str = ostr.str();
        PyErr_Warn(PyExc_RuntimeWarning, const_cast<char*>(str.c_str()));
        Ice::UnknownException ex(__FILE__, __LINE__);
        ex.unknown = str;
        throw ex;
    }

    PyObjectHandle result = PyObject_Call(method.get(), args.get(), 0);

    //
    // Check for exceptions.
    //
    if(PyErr_Occurred())
    {
        PyException ex; // Retrieve it before another Python API call clears it.
        exception(ex);
        return;
    }

    if(!_op->amd)
    {
        response(result.get());
    }
}

void
IcePy::TypedUpcall::response(PyObject* args)
{
    if(_finished)
    {
        //
        // This method could be called more than once if the application calls
        // ice_response multiple times. We ignore subsequent calls.
        //
        return;
    }

    _finished = true;

    try
    {
        //
        // Marshal the results. If there is more than one value to be returned, then they must be
        // returned in a tuple of the form (result, outParam1, ...).
        //
        Ice::OutputStreamPtr os = Ice::createOutputStream(_communicator);
        try
        {
            Py_ssize_t i = _op->returnType ? 1 : 0;
            Py_ssize_t numResults = static_cast<Py_ssize_t>(_op->outParams.size()) + i;
            if(numResults > 1)
            {
                if(!PyTuple_Check(args) || PyTuple_GET_SIZE(args) != numResults)
                {
                    ostringstream ostr;
                    ostr << "operation `" << fixIdent(_op->name) << "' should return a tuple of length " << numResults;
                    string str = ostr.str();
                    PyErr_Warn(PyExc_RuntimeWarning, const_cast<char*>(str.c_str()));
                    throw Ice::MarshalException(__FILE__, __LINE__);
                }
            }

            ObjectMap objectMap;

            for(ParamInfoList::iterator p = _op->outParams.begin(); p != _op->outParams.end(); ++p, ++i)
            {
                PyObject* arg;
                if(_op->amd || numResults > 1)
                {
                    arg = PyTuple_GET_ITEM(args, i);
                }
                else
                {
                    arg = args;
                    assert(_op->outParams.size() == 1);
                }

                if(!(*p)->type->validate(arg))
                {
                    // TODO: Provide the parameter name instead?
                    ostringstream ostr;
                    ostr << "invalid value for out argument " << (i + 1) << " in operation `" << fixIdent(_op->name)
                         << (_op->amd ? "_async" : "") << "'";
                    string str = ostr.str();
                    PyErr_Warn(PyExc_RuntimeWarning, const_cast<char*>(str.c_str()));
                    throw Ice::MarshalException(__FILE__, __LINE__);
                }
                (*p)->type->marshal(arg, os, &objectMap, &(*p)->metaData);
            }

            if(_op->returnType)
            {
                PyObject* res;
                if(_op->amd || numResults > 1)
                {
                    res = PyTuple_GET_ITEM(args, 0);
                }
                else
                {
                    assert(_op->outParams.size() == 0);
                    res = args;
                }
                if(!_op->returnType->type->validate(res))
                {
                    ostringstream ostr;
                    ostr << "invalid return value for operation `" << fixIdent(_op->name) << "'";
                    string str = ostr.str();
                    PyErr_Warn(PyExc_RuntimeWarning, const_cast<char*>(str.c_str()));
                    throw Ice::MarshalException(__FILE__, __LINE__);
                }
                _op->returnType->type->marshal(res, os, &objectMap, &_op->metaData);
            }

            if(_op->returnsClasses)
            {
                os->writePendingObjects();
            }

            Ice::ByteSeq bytes;
            os->finished(bytes);
            pair<const Ice::Byte*, const Ice::Byte*> ob(static_cast<const Ice::Byte*>(0), static_cast<const Ice::Byte*>(0));
            if(!bytes.empty())
            {
                ob.first = &bytes[0];
                ob.second = &bytes[0] + bytes.size();
            }

            AllowThreads allowThreads; // Release Python's global interpreter lock during blocking calls.
            _callback->ice_response(true, ob);
        }
        catch(const AbortMarshaling&)
        {
            throwPythonException();
        }
    }
    catch(const Ice::Exception& ex)
    {
        AllowThreads allowThreads; // Release Python's global interpreter lock during blocking calls.
        _callback->ice_exception(ex);
    }
}

void
IcePy::TypedUpcall::exception(PyException& ex)
{
    if(_finished)
    {
        //
        // An asynchronous response or exception has already been sent. We just
        // raise an exception and let the C++ run time handle it.
        //
        ex.raise();
    }

    _finished = true;

    try
    {
        try
        {
            //
            // A servant that calls sys.exit() will raise the SystemExit exception.
            // This is normally caught by the interpreter, causing it to exit.
            // However, we have no way to pass this exception to the interpreter,
            // so we act on it directly.
            //
            ex.checkSystemExit();

            PyObject* userExceptionType = lookupType("Ice.UserException");

            if(PyObject_IsInstance(ex.ex.get(), userExceptionType))
            {
                //
                // Get the exception's type and verify that it is legal to be thrown from this operation.
                //
                PyObjectHandle iceType = PyObject_GetAttrString(ex.ex.get(), STRCAST("_ice_type"));
                assert(iceType.get());
                ExceptionInfoPtr info = ExceptionInfoPtr::dynamicCast(getException(iceType.get()));
                assert(info);
                if(!validateException(ex.ex.get()))
                {
                    ex.raise(); // Raises UnknownUserException.
                }
                else
                {
                    Ice::OutputStreamPtr os = Ice::createOutputStream(_communicator);

                    os->write(info->usesClasses);

                    ObjectMap objectMap;
                    info->marshal(ex.ex.get(), os, &objectMap);

                    if(info->usesClasses)
                    {
                        os->writePendingObjects();
                    }

                    Ice::ByteSeq bytes;
                    os->finished(bytes);
                    pair<const Ice::Byte*, const Ice::Byte*> ob(static_cast<const Ice::Byte*>(0),static_cast<const Ice::Byte*>(0));
                    if(!bytes.empty())
                    {
                        ob.first = &bytes[0];
                        ob.second = &bytes[0] + bytes.size();
                    }

                    AllowThreads allowThreads; // Release Python's global interpreter lock during blocking calls.
                    _callback->ice_response(false, ob);
                }
            }
            else
            {
                ex.raise();
            }
        }
        catch(const AbortMarshaling&)
        {
            throwPythonException();
        }
    }
    catch(const Ice::Exception& ex)
    {
        AllowThreads allowThreads; // Release Python's global interpreter lock during blocking calls.
        _callback->ice_exception(ex);
    }
}

bool
IcePy::TypedUpcall::validateException(PyObject* ex) const
{
    for(ExceptionInfoList::const_iterator p = _op->exceptions.begin(); p != _op->exceptions.end(); ++p)
    {
        if(PyObject_IsInstance(ex, (*p)->pythonType.get()))
        {
            return true;
        }
    }

    return false;
}

//
// BlobjectUpcall
//
IcePy::BlobjectUpcall::BlobjectUpcall(bool amd, const Ice::AMD_Object_ice_invokePtr& callback) :
    _amd(amd), _callback(callback), _finished(false)
{
}

void
IcePy::BlobjectUpcall::dispatch(PyObject* servant, const pair<const Ice::Byte*, const Ice::Byte*>& inBytes,
                                const Ice::Current& current)
{
    Ice::CommunicatorPtr communicator = current.adapter->getCommunicator();

    Py_ssize_t count = 2; // First is the inParams, second is the Ice::Current object.

    Py_ssize_t start = 0;
    if(_amd)
    {
        ++count; // Leave room for a leading AMD callback argument.
        start = 1;
    }

    PyObjectHandle args = PyTuple_New(count);
    if(!args.get())
    {
        throwPythonException();
    }
 
    //
    // If using AMD we need to copy the bytes since the bytes may be
    // accessed after this method is over, otherwise
    // PyBuffer_FromMemory can be used which doesn't do a copy.
    //
    PyObjectHandle ip;
    if(!_amd)
    {
        ip = PyBuffer_FromMemory((void*)inBytes.first, inBytes.second - inBytes.first);
        if(!ip.get())
        {
            throwPythonException();
        }
    }
    else
    {
        ip = PyBuffer_New(inBytes.second - inBytes.first);
        if(!ip.get())
        {
            throwPythonException();
        }
        void* buf;
        Py_ssize_t sz;
        if(PyObject_AsWriteBuffer(ip.get(), &buf, &sz))
        {
            throwPythonException();
        }
        assert(sz == inBytes.second - inBytes.first);
        memcpy(buf, inBytes.first, sz);
    }

    if(PyTuple_SET_ITEM(args.get(), start, ip.get()) < 0)
    {
        throwPythonException();
    }
    ++start;
    ip.release(); // PyTuple_SET_ITEM steals a reference.

    //
    // Create an object to represent Ice::Current. We need to append
    // this to the argument tuple.
    //
    PyObjectHandle curr = createCurrent(current);
    if(PyTuple_SET_ITEM(args.get(), start, curr.get()) < 0)
    {
        throwPythonException();
    }
    curr.release(); // PyTuple_SET_ITEM steals a reference.

    string dispatchName = "ice_invoke";
    if(_amd)
    {
        dispatchName += "_async";
        //
        // Create the callback object and pass it as the first argument.
        //
        AMDCallbackObject* obj = amdCallbackNew(0);
        if(!obj)
        {
            throwPythonException();
        }
        obj->upcall = new UpcallPtr(this);
        if(PyTuple_SET_ITEM(args.get(), 0, (PyObject*)obj) < 0) // PyTuple_SET_ITEM steals a reference.
        {
            Py_DECREF(obj);
            throwPythonException();
        }
    }

    //
    // Dispatch the operation.
    //
    PyObjectHandle method = PyObject_GetAttrString(servant, const_cast<char*>(dispatchName.c_str()));
    if(!method.get())
    {
        ostringstream ostr;
        ostr << "servant for identity " << communicator->identityToString(current.id)
             << " does not define operation `" << dispatchName << "'";
        string str = ostr.str();
        PyErr_Warn(PyExc_RuntimeWarning, const_cast<char*>(str.c_str()));
        Ice::UnknownException ex(__FILE__, __LINE__);
        ex.unknown = str;
        throw ex;
    }

    PyObjectHandle result = PyObject_Call(method.get(), args.get(), 0);

    //
    // Check for exceptions.
    //
    if(PyErr_Occurred())
    {
        PyException ex; // Retrieve it before another Python API call clears it.
        exception(ex);
        return;
    }

    if(!_amd)
    {
        response(result.get());
    }
}

void
IcePy::BlobjectUpcall::response(PyObject* args)
{
    if(_finished)
    {
        //
        // This method could be called more than once if the application calls
        // ice_response multiple times. We ignore subsequent calls.
        //
        return;
    }

    _finished = true;

    //
    // The return value is a tuple of (bool, PyBuffer).
    //
    if(!PyTuple_Check(args) || PyTuple_GET_SIZE(args) != 2)
    {
        ostringstream ostr;
        string name = "ice_invoke";
        if(_amd)
        {
            name += "_async";
        }
        ostr << "operation `" << name << "' should return a tuple of length 2";
        string str = ostr.str();
        PyErr_Warn(PyExc_RuntimeWarning, const_cast<char*>(str.c_str()));
        throw Ice::MarshalException(__FILE__, __LINE__);
    }

    PyObject* arg = PyTuple_GET_ITEM(args, 0);
    int isTrue = PyObject_IsTrue(arg);

    arg = PyTuple_GET_ITEM(args, 1);
    if(!PyBuffer_Check(arg))
    {
        ostringstream ostr;
        ostr << "invalid return value for operation `ice_invoke'";
        string str = ostr.str();
        PyErr_Warn(PyExc_RuntimeWarning, const_cast<char*>(str.c_str()));
        throw Ice::MarshalException(__FILE__, __LINE__);
    }

#if PY_VERSION_HEX < 0x02050000
    const char* charBuf = 0;
#else
    char* charBuf = 0;
#endif
    Py_ssize_t sz = arg->ob_type->tp_as_buffer->bf_getcharbuffer(arg, 0, &charBuf);
    const Ice::Byte* mem = reinterpret_cast<const Ice::Byte*>(charBuf);
    const pair<const ::Ice::Byte*, const ::Ice::Byte*> bytes(mem, mem + sz);

    AllowThreads allowThreads; // Release Python's global interpreter lock during blocking calls.
    _callback->ice_response(isTrue, bytes);
}

void
IcePy::BlobjectUpcall::exception(PyException& ex)
{
    if(_finished)
    {
        //
        // An asynchronous response or exception has already been sent. We just
        // raise an exception and let the C++ run time handle it.
        //
        ex.raise();
    }

    _finished = true;

    try
    {
        //
        // A servant that calls sys.exit() will raise the SystemExit exception.
        // This is normally caught by the interpreter, causing it to exit.
        // However, we have no way to pass this exception to the interpreter,
        // so we act on it directly.
        //
        ex.checkSystemExit();

        ex.raise();
    }
    catch(const Ice::Exception& ex)
    {
        AllowThreads allowThreads; // Release Python's global interpreter lock during blocking calls.
        _callback->ice_exception(ex);
    }
}

PyObject*
IcePy::invokeBuiltin(PyObject* proxy, const string& builtin, PyObject* args)
{
    string name = "_op_" + builtin;
    PyObject* objectType = lookupType("Ice.Object");
    assert(objectType);
    PyObjectHandle obj = PyObject_GetAttrString(objectType, STRCAST(name.c_str()));
    assert(obj.get());

    OperationPtr op = getOperation(obj.get());
    assert(op);

    Ice::ObjectPrx p = getProxy(proxy);
    InvocationPtr i = new SyncTypedInvocation(p, op);
    return i->invoke(args);
}

PyObject*
IcePy::beginBuiltin(PyObject* proxy, const string& builtin, PyObject* args)
{
    string name = "_op_" + builtin;
    PyObject* objectType = lookupType("Ice.Object");
    assert(objectType);
    PyObjectHandle obj = PyObject_GetAttrString(objectType, STRCAST(name.c_str()));
    assert(obj.get());

    OperationPtr op = getOperation(obj.get());
    assert(op);

    Ice::ObjectPrx p = getProxy(proxy);
    InvocationPtr i = new AsyncTypedInvocation(p, proxy, op);
    return i->invoke(args);
}

PyObject*
IcePy::endBuiltin(PyObject* proxy, const string& builtin, PyObject* args)
{
    PyObject* result;
    if(!PyArg_ParseTuple(args, STRCAST("O!"), &AsyncResultType, &result))
    {
        return 0;
    }

    string name = "_op_" + builtin;
    PyObject* objectType = lookupType("Ice.Object");
    assert(objectType);
    PyObjectHandle obj = PyObject_GetAttrString(objectType, STRCAST(name.c_str()));
    assert(obj.get());

    OperationPtr op = getOperation(obj.get());
    assert(op);

    AsyncResultObject* ar = reinterpret_cast<AsyncResultObject*>(result);
    assert(ar);
    AsyncTypedInvocationPtr i = AsyncTypedInvocationPtr::dynamicCast(*ar->invocation);
    if(!i)
    {
        PyErr_Format(PyExc_ValueError, STRCAST("invalid AsyncResult object passed to end_%s"), op->name.c_str());
        return 0;
    }
    Ice::ObjectPrx p = getProxy(proxy);
    return i->end(p, op, *ar->result);
}

PyObject*
IcePy::iceInvoke(PyObject* proxy, PyObject* args)
{
    Ice::ObjectPrx p = getProxy(proxy);
    InvocationPtr i = new SyncBlobjectInvocation(p);
    return i->invoke(args);
}

PyObject*
IcePy::iceInvokeAsync(PyObject* proxy, PyObject* args)
{
    Ice::ObjectPrx p = getProxy(proxy);
    InvocationPtr i = new OldAsyncBlobjectInvocation(p);
    return i->invoke(args);
}

PyObject*
IcePy::beginIceInvoke(PyObject* proxy, PyObject* args, PyObject* kwds)
{
    Ice::ObjectPrx p = getProxy(proxy);
    InvocationPtr i = new AsyncBlobjectInvocation(p, proxy);
    return i->invoke(args, kwds);
}

PyObject*
IcePy::endIceInvoke(PyObject* proxy, PyObject* args)
{
    PyObject* result;
    if(!PyArg_ParseTuple(args, STRCAST("O!"), &AsyncResultType, &result))
    {
        return 0;
    }

    AsyncResultObject* ar = reinterpret_cast<AsyncResultObject*>(result);
    assert(ar);
    AsyncBlobjectInvocationPtr i = AsyncBlobjectInvocationPtr::dynamicCast(*ar->invocation);
    if(!i)
    {
        PyErr_Format(PyExc_ValueError, STRCAST("invalid AsyncResult object passed to end_ice_invoke"));
        return 0;
    }
    Ice::ObjectPrx p = getProxy(proxy);
    return i->end(p, *ar->result);
}

PyObject*
IcePy::createAsyncResult(const Ice::AsyncResultPtr& r, PyObject* proxy, PyObject* connection, PyObject* communicator)
{
    AsyncResultObject* obj = asyncResultNew(0);
    if(!obj)
    {
        return 0;
    }
    obj->result = new Ice::AsyncResultPtr(r);
    obj->proxy = proxy;
    Py_XINCREF(obj->proxy);
    obj->connection = connection;
    Py_XINCREF(obj->connection);
    obj->communicator = communicator;
    Py_XINCREF(obj->communicator);
    return reinterpret_cast<PyObject*>(obj);
}

Ice::AsyncResultPtr
IcePy::getAsyncResult(PyObject* p)
{
    assert(PyObject_IsInstance(p, reinterpret_cast<PyObject*>(&AsyncResultType)) == 1);
    AsyncResultObject* obj = reinterpret_cast<AsyncResultObject*>(p);
    return *obj->result;
}

IcePy::FlushCallback::FlushCallback(PyObject* ex, PyObject* sent, const string& op) :
    _ex(ex), _sent(sent), _op(op)
{
    assert(_ex);
    Py_INCREF(_ex);
    Py_XINCREF(_sent);
}

IcePy::FlushCallback::~FlushCallback()
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    Py_DECREF(_ex);
    Py_XDECREF(_sent);
}

void
IcePy::FlushCallback::exception(const Ice::Exception& ex)
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    callException(_ex, ex);
}

void
IcePy::FlushCallback::sent(bool sentSynchronously)
{
    if(_sent)
    {
        AdoptThread adoptThread; // Ensure the current thread is able to call into Python.
        callSent(_sent, sentSynchronously, true);
    }
}

//
// ServantWrapper implementation.
//
IcePy::ServantWrapper::ServantWrapper(PyObject* servant) :
    _servant(servant)
{
    Py_INCREF(_servant);
}

IcePy::ServantWrapper::~ServantWrapper()
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    Py_DECREF(_servant);
}

PyObject*
IcePy::ServantWrapper::getObject()
{
    Py_INCREF(_servant);
    return _servant;
}

//
// TypedServantWrapper implementation.
//
IcePy::TypedServantWrapper::TypedServantWrapper(PyObject* servant) :
    ServantWrapper(servant), _lastOp(_operationMap.end())
{
}

void
IcePy::TypedServantWrapper::ice_invoke_async(const Ice::AMD_Object_ice_invokePtr& cb,
                                             const pair<const Ice::Byte*, const Ice::Byte*>& inParams,
                                             const Ice::Current& current)
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    try
    {
        //
        // Locate the Operation object. As an optimization we keep a reference
        // to the most recent operation we've dispatched, so check that first.
        //
        OperationPtr op;
        if(_lastOp != _operationMap.end() && _lastOp->first == current.operation)
        {
            op = _lastOp->second;
        }
        else
        {
            //
            // Next check our cache of operations.
            //
            _lastOp = _operationMap.find(current.operation);
            if(_lastOp == _operationMap.end())
            {
                //
                // Look for the Operation object in the servant's type.
                //
                string attrName = "_op_" + current.operation;
                PyObjectHandle h = PyObject_GetAttrString((PyObject*)_servant->ob_type,
                                                          const_cast<char*>(attrName.c_str()));
                if(!h.get())
                {
                    PyErr_Clear();
                    Ice::OperationNotExistException ex(__FILE__, __LINE__);
                    ex.id = current.id;
                    ex.facet = current.facet;
                    ex.operation = current.operation;
                    throw ex;
                }

                assert(PyObject_IsInstance(h.get(), reinterpret_cast<PyObject*>(&OperationType)) == 1);
                OperationObject* obj = reinterpret_cast<OperationObject*>(h.get());
                op = *obj->op;
                _lastOp = _operationMap.insert(OperationMap::value_type(current.operation, op)).first;
            }
            else
            {
                op = _lastOp->second;
            }
        }

        __checkMode(op->mode, current.mode);

        UpcallPtr up = new TypedUpcall(op, cb, current.adapter->getCommunicator());
        up->dispatch(_servant, inParams, current);
    }
    catch(const Ice::Exception& ex)
    {
        AllowThreads allowThreads; // Release Python's global interpreter lock during blocking calls.
        cb->ice_exception(ex);
    }
}

//
// BlobjectServantWrapper implementation.
//
IcePy::BlobjectServantWrapper::BlobjectServantWrapper(PyObject* servant, bool amd) :
    ServantWrapper(servant), _amd(amd)
{
}

void
IcePy::BlobjectServantWrapper::ice_invoke_async(const Ice::AMD_Object_ice_invokePtr& cb,
                                                const pair<const Ice::Byte*, const Ice::Byte*>& inParams,
                                                const Ice::Current& current)
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    try
    {
        UpcallPtr up = new BlobjectUpcall(_amd, cb);
        up->dispatch(_servant, inParams, current);
    }
    catch(const Ice::Exception& ex)
    {
        AllowThreads allowThreads; // Release Python's global interpreter lock during blocking calls.
        cb->ice_exception(ex);
    }
}

IcePy::ServantWrapperPtr
IcePy::createServantWrapper(PyObject* servant)
{
    ServantWrapperPtr wrapper;
    PyObject* blobjectType = lookupType("Ice.Blobject");
    PyObject* blobjectAsyncType = lookupType("Ice.BlobjectAsync");
    if(PyObject_IsInstance(servant, blobjectType))
    {
        return new BlobjectServantWrapper(servant, false);
    }
    else if(PyObject_IsInstance(servant, blobjectAsyncType))
    {
        return new BlobjectServantWrapper(servant, true);
    }

    return new TypedServantWrapper(servant);
}
