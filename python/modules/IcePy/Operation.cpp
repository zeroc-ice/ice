// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifdef _WIN32
#   include <IceUtil/Config.h>
#endif
#include <Operation.h>
#include <Communicator.h>
#include <Current.h>
#include <Proxy.h>
#include <Thread.h>
#include <Types.h>
#include <Connection.h>
#include <Util.h>
#include <Ice/Communicator.h>
#include <Ice/IncomingAsync.h>
#include <Ice/Initialize.h>
#include <Ice/LocalException.h>
#include <Ice/Logger.h>
#include <Ice/ObjectAdapter.h>
#include <Ice/AsyncResult.h>
#include <Ice/Properties.h>
#include <Ice/Proxy.h>
#include <IceUtil/Time.h>
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
    bool optional;
    int tag;
    int pos;
};
typedef IceUtil::Handle<ParamInfo> ParamInfoPtr;
typedef list<ParamInfoPtr> ParamInfoList;

//
// Encapsulates attributes of an operation.
//
class Operation : public IceUtil::Shared
{
public:

    Operation(const char*, PyObject*, PyObject*, int, PyObject*, PyObject*, PyObject*, PyObject*, PyObject*, PyObject*);

    void marshalResult(Ice::OutputStream&, PyObject*);

    void deprecate(const string&);

    string name;
    Ice::OperationMode mode;
    Ice::OperationMode sendMode;
    bool amd;
    Ice::FormatType format;
    Ice::StringSeq metaData;
    ParamInfoList inParams;
    ParamInfoList optionalInParams;
    ParamInfoList outParams;
    ParamInfoList optionalOutParams;
    ParamInfoPtr returnType;
    ExceptionInfoList exceptions;
    string dispatchName;
    bool sendsClasses;
    bool returnsClasses;
    bool pseudoOp;

private:

    string _deprecateMessage;

    static void convertParams(PyObject*, ParamInfoList&, int, bool&);
    static ParamInfoPtr convertParam(PyObject*, int);
};
typedef IceUtil::Handle<Operation> OperationPtr;

//
// The base class for client-side invocations.
//
class Invocation : public virtual IceUtil::Shared
{
public:

    Invocation(const Ice::ObjectPrx&);

    virtual PyObject* invoke(PyObject*, PyObject* = 0) = 0;

protected:

    //
    // Helpers for typed invocations.
    //

    enum MappingType { SyncMapping, AsyncMapping, NewAsyncMapping };

    bool prepareRequest(const OperationPtr&, PyObject*, MappingType, Ice::OutputStream*,
                        pair<const Ice::Byte*, const Ice::Byte*>&);
    PyObject* unmarshalResults(const OperationPtr&, const pair<const Ice::Byte*, const Ice::Byte*>&);
    PyObject* unmarshalException(const OperationPtr&, const pair<const Ice::Byte*, const Ice::Byte*>&);
    bool validateException(const OperationPtr&, PyObject*) const;
    void checkTwowayOnly(const OperationPtr&, const Ice::ObjectPrx&) const;

    Ice::ObjectPrx _prx;
    Ice::CommunicatorPtr _communicator;
};
typedef IceUtil::Handle<Invocation> InvocationPtr;

//
// Synchronous typed invocation.
//
class SyncTypedInvocation : public Invocation
{
public:

    SyncTypedInvocation(const Ice::ObjectPrx&, const OperationPtr&);

    virtual PyObject* invoke(PyObject*, PyObject* = 0);

private:

    OperationPtr _op;
};

//
// Asynchronous typed invocation.
//
class AsyncTypedInvocation : public Invocation
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

private:

    void checkAsyncTwowayOnly(const Ice::ObjectPrx&) const;

    OperationPtr _op;
    PyObject* _pyProxy;
    PyObject* _response;
    PyObject* _ex;
    PyObject* _sent;
};
typedef IceUtil::Handle<AsyncTypedInvocation> AsyncTypedInvocationPtr;

//
// Asynchronous invocation with futures.
//
class NewAsyncInvocation : public Invocation
{
public:

    NewAsyncInvocation(const Ice::ObjectPrx&, PyObject*, const string&);
    ~NewAsyncInvocation();

    virtual PyObject* invoke(PyObject*, PyObject* = 0);

    void response(bool, const pair<const Ice::Byte*, const Ice::Byte*>&);
    void exception(const Ice::Exception&);
    void sent(bool);

protected:

    virtual Ice::AsyncResultPtr handleInvoke(PyObject*, PyObject*) = 0;
    virtual void handleResponse(PyObject*, bool, const pair<const Ice::Byte*, const Ice::Byte*>&) = 0;

    PyObject* _pyProxy;
    string _operation;
    bool _twoway;
    bool _sent;
    bool _sentSynchronously;
    bool _done;
    PyObject* _future;
    bool _ok;
    vector<Ice::Byte> _results;
    PyObject* _exception;
};
typedef IceUtil::Handle<NewAsyncInvocation> NewAsyncInvocationPtr;

//
// New-style asynchronous typed invocation.
//
class NewAsyncTypedInvocation : public NewAsyncInvocation
{
public:

    NewAsyncTypedInvocation(const Ice::ObjectPrx&, PyObject*, const OperationPtr&);

protected:

    virtual Ice::AsyncResultPtr handleInvoke(PyObject*, PyObject*);
    virtual void handleResponse(PyObject*, bool, const pair<const Ice::Byte*, const Ice::Byte*>&);

private:

    OperationPtr _op;
};

//
// Synchronous blobject invocation.
//
class SyncBlobjectInvocation : public Invocation
{
public:

    SyncBlobjectInvocation(const Ice::ObjectPrx&);

    virtual PyObject* invoke(PyObject*, PyObject* = 0);
};

//
// Asynchronous blobject invocation.
//
class AsyncBlobjectInvocation : public Invocation
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
// New-style asynchronous blobject invocation.
//
class NewAsyncBlobjectInvocation : public NewAsyncInvocation
{
public:

    NewAsyncBlobjectInvocation(const Ice::ObjectPrx&, PyObject*);

protected:

    virtual Ice::AsyncResultPtr handleInvoke(PyObject*, PyObject*);
    virtual void handleResponse(PyObject*, bool, const pair<const Ice::Byte*, const Ice::Byte*>&);

    string _op;
};

//
// The base class for server-side upcalls.
//
class Upcall : public IceUtil::Shared
{
public:

    virtual void dispatch(PyObject*, const pair<const Ice::Byte*, const Ice::Byte*>&, const Ice::Current&) = 0;
    virtual void response(PyObject*) = 0;
    virtual void exception(PyException&) = 0;
    virtual void exception(const Ice::Exception&) = 0;

protected:

    void dispatchImpl(PyObject*, const string&, PyObject*, const Ice::Current&);
};
typedef IceUtil::Handle<Upcall> UpcallPtr;

//
// TypedUpcall uses the information in the given Operation to validate, marshal, and unmarshal
// parameters and exceptions.
//
class TypedUpcall;
typedef IceUtil::Handle<TypedUpcall> TypedUpcallPtr;

class TypedUpcall : public Upcall
{
public:

    TypedUpcall(const OperationPtr&, const Ice::AMD_Object_ice_invokePtr&, const Ice::CommunicatorPtr&);

    virtual void dispatch(PyObject*, const pair<const Ice::Byte*, const Ice::Byte*>&, const Ice::Current&);
    virtual void response(PyObject*);
    virtual void exception(PyException&);
    virtual void exception(const Ice::Exception&);

private:

    OperationPtr _op;
    Ice::AMD_Object_ice_invokePtr _callback;
    Ice::CommunicatorPtr _communicator;
    Ice::EncodingVersion _encoding;
};

//
// Upcall for blobject servants.
//
class BlobjectUpcall : public Upcall
{
public:

    BlobjectUpcall(const Ice::AMD_Object_ice_invokePtr&);

    virtual void dispatch(PyObject*, const pair<const Ice::Byte*, const Ice::Byte*>&, const Ice::Current&);
    virtual void response(PyObject*);
    virtual void exception(PyException&);
    virtual void exception(const Ice::Exception&);

private:

    Ice::AMD_Object_ice_invokePtr _callback;
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

    BlobjectServantWrapper(PyObject*);

    virtual void ice_invoke_async(const Ice::AMD_Object_ice_invokePtr&,
                                  const pair<const Ice::Byte*, const Ice::Byte*>&,
                                  const Ice::Current&);
};

struct OperationObject
{
    PyObject_HEAD
    OperationPtr* op;
};

struct DoneCallbackObject
{
    PyObject_HEAD
    UpcallPtr* upcall;
#if PY_VERSION_HEX >= 0x03050000
    PyObject* coroutine;
#endif
};

struct DispatchCallbackObject
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

struct MarshaledResultObject
{
    PyObject_HEAD
    Ice::OutputStream* out;
};

extern PyTypeObject MarshaledResultType;

extern PyTypeObject OperationType;

class UserExceptionFactory : public Ice::UserExceptionFactory
{
public:

    virtual void createAndThrow(const string& id)
    {
        ExceptionInfoPtr info = lookupExceptionInfo(id);
        if(info)
        {
            throw ExceptionReader(info);
        }
    }
};

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
    PyObjectHandle tmp = callMethod(method, ex);
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
callSent(PyObject* method, bool sentSynchronously, bool passArg)
{
    PyObject* arg = 0;
    if(passArg)
    {
        arg = sentSynchronously ? getTrue() : getFalse();
    }
    PyObjectHandle tmp = callMethod(method, arg);
    if(PyErr_Occurred())
    {
        handleException(); // Callback raised an exception.
    }
}

}

#ifdef WIN32
extern "C"
#endif
static OperationObject*
operationNew(PyTypeObject* type, PyObject* /*args*/, PyObject* /*kwds*/)
{
    OperationObject* self = reinterpret_cast<OperationObject*>(type->tp_alloc(type, 0));
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
    PyObject* format;
    PyObject* metaData;
    PyObject* inParams;
    PyObject* outParams;
    PyObject* returnType;
    PyObject* exceptions;
    if(!PyArg_ParseTuple(args, STRCAST("sO!O!iOO!O!O!OO!"), &name, modeType, &mode, modeType, &sendMode, &amd,
                         &format, &PyTuple_Type, &metaData, &PyTuple_Type, &inParams, &PyTuple_Type, &outParams,
                         &returnType, &PyTuple_Type, &exceptions))
    {
        return -1;
    }

    OperationPtr op = new Operation(name, mode, sendMode, amd, format, metaData, inParams, outParams, returnType,
                                    exceptions);
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
    Py_TYPE(self)->tp_free(reinterpret_cast<PyObject*>(self));
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
    PyObject* proxy;
    PyObject* opArgs;
    if(!PyArg_ParseTuple(args, STRCAST("O!O!"), &ProxyType, &proxy, &PyTuple_Type, &opArgs))
    {
        return 0;
    }

    Ice::ObjectPrx p = getProxy(proxy);
    InvocationPtr i = new NewAsyncTypedInvocation(p, proxy, *self->op);
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

    return incRef(Py_None);
}

//
// DoneCallback operations
//

#ifdef WIN32
extern "C"
#endif
static DoneCallbackObject*
doneCallbackNew(PyTypeObject* type, PyObject* /*args*/, PyObject* /*kwds*/)
{
    DoneCallbackObject* self = reinterpret_cast<DoneCallbackObject*>(type->tp_alloc(type, 0));
    if(!self)
    {
        return 0;
    }
    self->upcall = 0;
#if PY_VERSION_HEX >= 0x03050000
    self->coroutine = 0;
#endif
    return self;
}

#ifdef WIN32
extern "C"
#endif
static void
doneCallbackDealloc(DoneCallbackObject* self)
{
    delete self->upcall;
#if PY_VERSION_HEX >= 0x03050000
    Py_XDECREF(self->coroutine);
#endif
    Py_TYPE(self)->tp_free(reinterpret_cast<PyObject*>(self));
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
doneCallbackInvoke(DoneCallbackObject* self, PyObject* args)
{
    PyObject* future = 0;
    if(!PyArg_ParseTuple(args, STRCAST("O"), &future))
    {
        return 0;
    }

    try
    {
        assert(self->upcall);

        PyObjectHandle resultMethod = getAttr(future, "result", false);
        assert(resultMethod.get());
        PyObjectHandle empty = PyTuple_New(0);
        PyObjectHandle result = PyObject_Call(resultMethod.get(), empty.get(), 0);

        if(PyErr_Occurred())
        {
            PyException ex;
            (*self->upcall)->exception(ex);
        }
        else
        {
            (*self->upcall)->response(result.get());
        }
    }
    catch(...)
    {
        //
        // No exceptions should propagate to Python.
        //
        assert(false);
    }

    return incRef(Py_None);
}

//
// DispatchCallbackObject operations
//

#ifdef WIN32
extern "C"
#endif
static DispatchCallbackObject*
dispatchCallbackNew(PyTypeObject* type, PyObject* /*args*/, PyObject* /*kwds*/)
{
    DispatchCallbackObject* self = reinterpret_cast<DispatchCallbackObject*>(type->tp_alloc(type, 0));
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
dispatchCallbackDealloc(DispatchCallbackObject* self)
{
    delete self->upcall;
    Py_TYPE(self)->tp_free(reinterpret_cast<PyObject*>(self));
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
dispatchCallbackResponse(DispatchCallbackObject* self, PyObject* args)
{
    PyObject* result = 0;
    if(!PyArg_ParseTuple(args, STRCAST("O"), &result))
    {
        return 0;
    }

    try
    {
        assert(self->upcall);
        (*self->upcall)->response(result);
    }
    catch(...)
    {
        //
        // No exceptions should propagate to Python.
        //
        assert(false);
    }

    return incRef(Py_None);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
dispatchCallbackException(DispatchCallbackObject* self, PyObject* args)
{
    PyObject* ex = 0;
    if(!PyArg_ParseTuple(args, STRCAST("O"), &ex))
    {
        return 0;
    }

    try
    {
        assert(self->upcall);
        PyException pyex(ex);
        (*self->upcall)->exception(pyex);
    }
    catch(...)
    {
        //
        // No exceptions should propagate to Python.
        //
        assert(false);
    }

    return incRef(Py_None);
}

//
// AsyncResult operations
//

#ifdef WIN32
extern "C"
#endif
static AsyncResultObject*
asyncResultNew(PyTypeObject* type, PyObject* /*args*/, PyObject* /*kwds*/)
{
    AsyncResultObject* self = reinterpret_cast<AsyncResultObject*>(type->tp_alloc(type, 0));
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
    Py_TYPE(self)->tp_free(reinterpret_cast<PyObject*>(self));
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
asyncResultGetCommunicator(AsyncResultObject* self)
{
    if(self->communicator)
    {
        return incRef(self->communicator);
    }

    return incRef(Py_None);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
asyncResultCancel(AsyncResultObject* self)
{
    try
    {
        (*self->result)->cancel();
    }
    catch(...)
    {
        assert(false);
    }

    return incRef(Py_None);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
asyncResultGetConnection(AsyncResultObject* self)
{
    if(self->connection)
    {
        return incRef(self->connection);
    }

    return incRef(Py_None);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
asyncResultGetProxy(AsyncResultObject* self)
{
    if(self->proxy)
    {
        return incRef(self->proxy);
    }

    return incRef(Py_None);
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

    return incRef(Py_None);
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

    return incRef(Py_None);
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

    return incRef(Py_None);
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

#ifdef WIN32
extern "C"
#endif
static PyObject*
asyncResultCallLater(AsyncResultObject* self, PyObject* args)
{
    PyObject* callback;
    if(!PyArg_ParseTuple(args, STRCAST("O"), &callback))
    {
        return 0;
    }

    if(!PyCallable_Check(callback))
    {
        PyErr_Format(PyExc_ValueError, STRCAST("invalid argument passed to callLater"));
        return 0;
    }

    class CallbackI : public Ice::AsyncResult::Callback
    {
    public:

        CallbackI(PyObject* callback) :
            _callback(incRef(callback))
        {
        }

        ~CallbackI()
        {
            AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

            Py_DECREF(_callback);
        }

        virtual void run()
        {
            AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

            PyObjectHandle args = PyTuple_New(0);
            assert(args.get());
            PyObjectHandle tmp = PyObject_Call(_callback, args.get(), 0);
            PyErr_Clear();
        }

    private:

        PyObject* _callback;
    };

    try
    {
        (*self->result)->_scheduleCallback(new CallbackI(callback));
    }
    catch(const Ice::CommunicatorDestroyedException& ex)
    {
        setPythonException(ex);
        return 0;
    }
    catch(...)
    {
        assert(false);
    }

    return incRef(Py_None);
}

//
// MarshaledResult operations
//

#ifdef WIN32
extern "C"
#endif
static MarshaledResultObject*
marshaledResultNew(PyTypeObject* type, PyObject* /*args*/, PyObject* /*kwds*/)
{
    MarshaledResultObject* self = reinterpret_cast<MarshaledResultObject*>(type->tp_alloc(type, 0));
    if(!self)
    {
        return 0;
    }
    self->out = 0;
    return self;
}

#ifdef WIN32
extern "C"
#endif
static int
marshaledResultInit(MarshaledResultObject* self, PyObject* args, PyObject* /*kwds*/)
{
    PyObject* result;
    OperationObject* opObj;
    PyObject* communicatorObj;
    PyObject* encodingObj;
    if(!PyArg_ParseTuple(args, STRCAST("OOOO"), &result, &opObj, &communicatorObj, &encodingObj))
    {
        return -1;
    }

    Ice::CommunicatorPtr communicator = getCommunicator(communicatorObj);
    Ice::EncodingVersion encoding;
    if(!getEncodingVersion(encodingObj, encoding))
    {
        return -1;
    }

    self->out = new Ice::OutputStream(communicator);

    OperationPtr op = *opObj->op;
    self->out->startEncapsulation(encoding, op->format);

    try
    {
        op->marshalResult(*self->out, result);
    }
    catch(const AbortMarshaling&)
    {
        assert(PyErr_Occurred());
        return -1;
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return -1;
    }

    self->out->endEncapsulation();

    return 0;
}

#ifdef WIN32
extern "C"
#endif
static void
marshaledResultDealloc(MarshaledResultObject* self)
{
    delete self->out;
    Py_TYPE(self)->tp_free(reinterpret_cast<PyObject*>(self));
}

//
// ParamInfo implementation.
//
void
IcePy::ParamInfo::unmarshaled(PyObject* val, PyObject* target, void* closure)
{
    assert(PyTuple_Check(target));
    Py_ssize_t i = reinterpret_cast<Py_ssize_t>(closure);
    PyTuple_SET_ITEM(target, i, incRef(val)); // PyTuple_SET_ITEM steals a reference.
}

//
// Operation implementation.
//
IcePy::Operation::Operation(const char* n, PyObject* m, PyObject* sm, int amdFlag, PyObject* fmt, PyObject* meta,
                            PyObject* in, PyObject* out, PyObject* ret, PyObject* ex)
{
    name = n;

    //
    // mode
    //
    PyObjectHandle modeValue = getAttr(m, "value", true);
    mode = (Ice::OperationMode)static_cast<int>(PyLong_AsLong(modeValue.get()));
    assert(!PyErr_Occurred());

    //
    // sendMode
    //
    PyObjectHandle sendModeValue = getAttr(sm, "value", true);
    sendMode = (Ice::OperationMode)static_cast<int>(PyLong_AsLong(sendModeValue.get()));
    assert(!PyErr_Occurred());

    //
    // amd
    //
    amd = amdFlag ? true : false;
    dispatchName = fixIdent(name); // Use the same dispatch name regardless of AMD.

    //
    // format
    //
    if(fmt == Py_None)
    {
        format = Ice::DefaultFormat;
    }
    else
    {
        PyObjectHandle formatValue = getAttr(fmt, "value", true);
        format = (Ice::FormatType)static_cast<int>(PyLong_AsLong(formatValue.get()));
        assert(!PyErr_Occurred());
    }

    //
    // metaData
    //
    assert(PyTuple_Check(meta));
#ifndef NDEBUG
    bool b =
#endif
    tupleToStringSeq(meta, metaData);
    assert(b);

    //
    // returnType
    //
    returnsClasses = false;
    if(ret != Py_None)
    {
        returnType = convertParam(ret, 0);
        if(!returnType->optional)
        {
            returnsClasses = returnType->type->usesClasses();
        }
    }

    //
    // inParams
    //
    sendsClasses = false;
    convertParams(in, inParams, 0, sendsClasses);

    //
    // outParams
    //
    convertParams(out, outParams, returnType ? 1 : 0, returnsClasses);

    class SortFn
    {
    public:
        static bool compare(const ParamInfoPtr& lhs, const ParamInfoPtr& rhs)
        {
            return lhs->tag < rhs->tag;
        }

        static bool isRequired(const ParamInfoPtr& i)
        {
            return !i->optional;
        }
    };

    //
    // The inParams list represents the parameters in the order of declaration.
    // We also need a sorted list of optional parameters.
    //
    ParamInfoList l = inParams;
    copy(l.begin(), remove_if(l.begin(), l.end(), SortFn::isRequired), back_inserter(optionalInParams));
    optionalInParams.sort(SortFn::compare);

    //
    // The outParams list represents the parameters in the order of declaration.
    // We also need a sorted list of optional parameters. If the return value is
    // optional, we must include it in this list.
    //
    l = outParams;
    copy(l.begin(), remove_if(l.begin(), l.end(), SortFn::isRequired), back_inserter(optionalOutParams));
    if(returnType && returnType->optional)
    {
        optionalOutParams.push_back(returnType);
    }
    optionalOutParams.sort(SortFn::compare);

    //
    // exceptions
    //
    Py_ssize_t sz = PyTuple_GET_SIZE(ex);
    for(Py_ssize_t i = 0; i < sz; ++i)
    {
        exceptions.push_back(getException(PyTuple_GET_ITEM(ex, i)));
    }

    //
    // Does the operation name start with "ice_"?
    //
    pseudoOp = name.find("ice_") == 0;
}

void
Operation::marshalResult(Ice::OutputStream& os, PyObject* result)
{
    //
    // Marshal the results. If there is more than one value to be returned, then they must be
    // returned in a tuple of the form (result, outParam1, ...).
    //

    Py_ssize_t numResults = static_cast<Py_ssize_t>(outParams.size());
    if(returnType)
    {
        numResults++;
    }

    if(numResults > 1 && (!PyTuple_Check(result) || PyTuple_GET_SIZE(result) != numResults))
    {
        ostringstream ostr;
        ostr << "operation `" << fixIdent(name) << "' should return a tuple of length " << numResults;
        string str = ostr.str();
        PyErr_WarnEx(PyExc_RuntimeWarning, const_cast<char*>(str.c_str()), 1);
        throw Ice::MarshalException(__FILE__, __LINE__);
    }

    //
    // Normalize the result value. When there are multiple result values, result is already a tuple.
    // Otherwise, we create a tuple to make the code a little simpler.
    //
    PyObjectHandle t;
    if(numResults > 1)
    {
        t = incRef(result);
    }
    else
    {
        t = PyTuple_New(1);
        if(!t.get())
        {
            throw AbortMarshaling();
        }
        PyTuple_SET_ITEM(t.get(), 0, incRef(result));
    }

    ObjectMap objectMap;
    ParamInfoList::iterator p;

    //
    // Validate the results.
    //
    for(p = outParams.begin(); p != outParams.end(); ++p)
    {
        ParamInfoPtr info = *p;
        PyObject* arg = PyTuple_GET_ITEM(t.get(), info->pos);
        if((!info->optional || arg != Unset) && !info->type->validate(arg))
        {
            // TODO: Provide the parameter name instead?
            ostringstream ostr;
            ostr << "invalid value for out argument " << (info->pos + 1) << " in operation `" << dispatchName << "'";
            string str = ostr.str();
            PyErr_WarnEx(PyExc_RuntimeWarning, const_cast<char*>(str.c_str()), 1);
            throw Ice::MarshalException(__FILE__, __LINE__);
        }
    }
    if(returnType)
    {
        PyObject* res = PyTuple_GET_ITEM(t.get(), 0);
        if((!returnType->optional || res != Unset) && !returnType->type->validate(res))
        {
            ostringstream ostr;
            ostr << "invalid return value for operation `" << dispatchName << "'";
            string str = ostr.str();
            PyErr_WarnEx(PyExc_RuntimeWarning, const_cast<char*>(str.c_str()), 1);
            throw Ice::MarshalException(__FILE__, __LINE__);
        }
    }

    //
    // Marshal the required out parameters.
    //
    for(p = outParams.begin(); p != outParams.end(); ++p)
    {
        ParamInfoPtr info = *p;
        if(!info->optional)
        {
            PyObject* arg = PyTuple_GET_ITEM(t.get(), info->pos);
            info->type->marshal(arg, &os, &objectMap, false, &info->metaData);
        }
    }

    //
    // Marshal the required return value, if any.
    //
    if(returnType && !returnType->optional)
    {
        PyObject* res = PyTuple_GET_ITEM(t.get(), 0);
        returnType->type->marshal(res, &os, &objectMap, false, &metaData);
    }

    //
    // Marshal the optional results.
    //
    for(p = optionalOutParams.begin(); p != optionalOutParams.end(); ++p)
    {
        ParamInfoPtr info = *p;
        PyObject* arg = PyTuple_GET_ITEM(t.get(), info->pos);
        if(arg != Unset && os.writeOptional(info->tag, info->type->optionalFormat()))
        {
            info->type->marshal(arg, &os, &objectMap, true, &info->metaData);
        }
    }

    if(returnsClasses)
    {
        os.writePendingValues();
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
IcePy::Operation::convertParams(PyObject* p, ParamInfoList& params, int posOffset, bool& usesClasses)
{
    int sz = static_cast<int>(PyTuple_GET_SIZE(p));
    for(int i = 0; i < sz; ++i)
    {
        PyObject* item = PyTuple_GET_ITEM(p, i);
        ParamInfoPtr param = convertParam(item, i + posOffset);
        params.push_back(param);
        if(!param->optional && !usesClasses)
        {
            usesClasses = param->type->usesClasses();
        }
    }
}

ParamInfoPtr
IcePy::Operation::convertParam(PyObject* p, int pos)
{
    assert(PyTuple_Check(p));
    assert(PyTuple_GET_SIZE(p) == 4);

    ParamInfoPtr param = new ParamInfo;

    //
    // metaData
    //
    PyObject* meta = PyTuple_GET_ITEM(p, 0);
    assert(PyTuple_Check(meta));
#ifndef NDEBUG
    bool b =
#endif
    tupleToStringSeq(meta, param->metaData);
    assert(b);

    //
    // type
    //
    PyObject* type = PyTuple_GET_ITEM(p, 1);
    if(type != Py_None)
    {
        param->type = getType(type);
    }

    //
    // optional
    //
    param->optional = PyObject_IsTrue(PyTuple_GET_ITEM(p, 2)) == 1;

    //
    // tag
    //
    param->tag = static_cast<int>(PyLong_AsLong(PyTuple_GET_ITEM(p, 3)));

    //
    // position
    //
    param->pos = pos;

    return param;
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

static PyMethodDef DoneCallbackMethods[] =
{
    { STRCAST("invoke"), reinterpret_cast<PyCFunction>(doneCallbackInvoke), METH_VARARGS,
      PyDoc_STR(STRCAST("internal function")) },
    { 0, 0 } /* sentinel */
};

static PyMethodDef DispatchCallbackMethods[] =
{
    { STRCAST("response"), reinterpret_cast<PyCFunction>(dispatchCallbackResponse), METH_VARARGS,
      PyDoc_STR(STRCAST("internal function")) },
    { STRCAST("exception"), reinterpret_cast<PyCFunction>(dispatchCallbackException), METH_VARARGS,
      PyDoc_STR(STRCAST("internal function")) },
    { 0, 0 } /* sentinel */
};

static PyMethodDef AsyncResultMethods[] =
{
    { STRCAST("cancel"), reinterpret_cast<PyCFunction>(asyncResultCancel), METH_NOARGS,
      PyDoc_STR(STRCAST("cancels the invocation")) },
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
    { STRCAST("callLater"), reinterpret_cast<PyCFunction>(asyncResultCallLater), METH_VARARGS,
      PyDoc_STR(STRCAST("internal function")) },
    { 0, 0 } /* sentinel */
};

namespace IcePy
{

PyTypeObject OperationType =
{
    /* The ob_type field must be initialized in the module init function
     * to be portable to Windows without using C++. */
    PyVarObject_HEAD_INIT(0, 0)
    STRCAST("IcePy.Operation"),      /* tp_name */
    sizeof(OperationObject),         /* tp_basicsize */
    0,                               /* tp_itemsize */
    /* methods */
    reinterpret_cast<destructor>(operationDealloc), /* tp_dealloc */
    0,                               /* tp_print */
    0,                               /* tp_getattr */
    0,                               /* tp_setattr */
    0,                               /* tp_reserved */
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

static PyTypeObject DoneCallbackType =
{
    /* The ob_type field must be initialized in the module init function
     * to be portable to Windows without using C++. */
    PyVarObject_HEAD_INIT(0, 0)
    STRCAST("IcePy.DoneCallback"),   /* tp_name */
    sizeof(DoneCallbackObject),      /* tp_basicsize */
    0,                               /* tp_itemsize */
    /* methods */
    reinterpret_cast<destructor>(doneCallbackDealloc), /* tp_dealloc */
    0,                               /* tp_print */
    0,                               /* tp_getattr */
    0,                               /* tp_setattr */
    0,                               /* tp_reserved */
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
    DoneCallbackMethods,             /* tp_methods */
    0,                               /* tp_members */
    0,                               /* tp_getset */
    0,                               /* tp_base */
    0,                               /* tp_dict */
    0,                               /* tp_descr_get */
    0,                               /* tp_descr_set */
    0,                               /* tp_dictoffset */
    0,                               /* tp_init */
    0,                               /* tp_alloc */
    reinterpret_cast<newfunc>(doneCallbackNew), /* tp_new */
    0,                               /* tp_free */
    0,                               /* tp_is_gc */
};

static PyTypeObject DispatchCallbackType =
{
    /* The ob_type field must be initialized in the module init function
     * to be portable to Windows without using C++. */
    PyVarObject_HEAD_INIT(0, 0)
    STRCAST("IcePy.Dispatch"),       /* tp_name */
    sizeof(DispatchCallbackObject),  /* tp_basicsize */
    0,                               /* tp_itemsize */
    /* methods */
    reinterpret_cast<destructor>(dispatchCallbackDealloc), /* tp_dealloc */
    0,                               /* tp_print */
    0,                               /* tp_getattr */
    0,                               /* tp_setattr */
    0,                               /* tp_reserved */
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
    DispatchCallbackMethods,         /* tp_methods */
    0,                               /* tp_members */
    0,                               /* tp_getset */
    0,                               /* tp_base */
    0,                               /* tp_dict */
    0,                               /* tp_descr_get */
    0,                               /* tp_descr_set */
    0,                               /* tp_dictoffset */
    0,                               /* tp_init */
    0,                               /* tp_alloc */
    reinterpret_cast<newfunc>(dispatchCallbackNew), /* tp_new */
    0,                               /* tp_free */
    0,                               /* tp_is_gc */
};

PyTypeObject AsyncResultType =
{
    /* The ob_type field must be initialized in the module init function
     * to be portable to Windows without using C++. */
    PyVarObject_HEAD_INIT(0, 0)
    STRCAST("IcePy.AsyncResult"),    /* tp_name */
    sizeof(AsyncResultObject),       /* tp_basicsize */
    0,                               /* tp_itemsize */
    /* methods */
    reinterpret_cast<destructor>(asyncResultDealloc), /* tp_dealloc */
    0,                               /* tp_print */
    0,                               /* tp_getattr */
    0,                               /* tp_setattr */
    0,                               /* tp_reserved */
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

PyTypeObject MarshaledResultType =
{
    /* The ob_type field must be initialized in the module init function
     * to be portable to Windows without using C++. */
    PyVarObject_HEAD_INIT(0, 0)
    STRCAST("IcePy.MarshaledResult"),/* tp_name */
    sizeof(MarshaledResultObject),   /* tp_basicsize */
    0,                               /* tp_itemsize */
    /* methods */
    reinterpret_cast<destructor>(marshaledResultDealloc), /* tp_dealloc */
    0,                               /* tp_print */
    0,                               /* tp_getattr */
    0,                               /* tp_setattr */
    0,                               /* tp_reserved */
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
    0,                               /* tp_methods */
    0,                               /* tp_members */
    0,                               /* tp_getset */
    0,                               /* tp_base */
    0,                               /* tp_dict */
    0,                               /* tp_descr_get */
    0,                               /* tp_descr_set */
    0,                               /* tp_dictoffset */
    reinterpret_cast<initproc>(marshaledResultInit), /* tp_init */
    0,                               /* tp_alloc */
    reinterpret_cast<newfunc>(marshaledResultNew), /* tp_new */
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

    if(PyType_Ready(&DoneCallbackType) < 0)
    {
        return false;
    }
    PyTypeObject* cbType = &DoneCallbackType; // Necessary to prevent GCC's strict-alias warnings.
    if(PyModule_AddObject(module, STRCAST("DoneCallback"), reinterpret_cast<PyObject*>(cbType)) < 0)
    {
        return false;
    }

    if(PyType_Ready(&DispatchCallbackType) < 0)
    {
        return false;
    }
    PyTypeObject* dispatchType = &DispatchCallbackType; // Necessary to prevent GCC's strict-alias warnings.
    if(PyModule_AddObject(module, STRCAST("DispatchCallback"), reinterpret_cast<PyObject*>(dispatchType)) < 0)
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

    if(PyType_Ready(&MarshaledResultType) < 0)
    {
        return false;
    }
    PyTypeObject* mrType = &MarshaledResultType; // Necessary to prevent GCC's strict-alias warnings.
    if(PyModule_AddObject(module, STRCAST("MarshaledResult"), reinterpret_cast<PyObject*>(mrType)) < 0)
    {
        return false;
    }

    return true;
}

//
// Invocation
//
IcePy::Invocation::Invocation(const Ice::ObjectPrx& prx) :
    _prx(prx), _communicator(prx->ice_getCommunicator())
{
}

bool
IcePy::Invocation::prepareRequest(const OperationPtr& op, PyObject* args, MappingType mapping, Ice::OutputStream* os,
                                  pair<const Ice::Byte*, const Ice::Byte*>& params)
{
    assert(PyTuple_Check(args));
    params.first = params.second = static_cast<const Ice::Byte*>(0);

    //
    // Validate the number of arguments.
    //
    Py_ssize_t argc = PyTuple_GET_SIZE(args);
    Py_ssize_t paramCount = static_cast<Py_ssize_t>(op->inParams.size());
    if(argc != paramCount)
    {
        string opName;
        if(mapping == NewAsyncMapping)
        {
            opName = op->name + "Async";
        }
        else if(mapping == AsyncMapping)
        {
            opName = "begin_" + op->name;
        }
        else
        {
            opName = fixIdent(op->name);
        }
        PyErr_Format(PyExc_RuntimeError, STRCAST("%s expects %d in parameters"), opName.c_str(),
                     static_cast<int>(paramCount));
        return false;
    }

    if(!op->inParams.empty())
    {
        try
        {
            //
            // Marshal the in parameters.
            //
            os->startEncapsulation(_prx->ice_getEncodingVersion(), op->format);

            ObjectMap objectMap;
            ParamInfoList::iterator p;

            //
            // Validate the supplied arguments.
            //
            for(p = op->inParams.begin(); p != op->inParams.end(); ++p)
            {
                ParamInfoPtr info = *p;
                PyObject* arg = PyTuple_GET_ITEM(args, info->pos);
                if((!info->optional || arg != Unset) && !info->type->validate(arg))
                {
                    string name;
                    if(mapping == NewAsyncMapping)
                    {
                        name = op->name + "Async";
                    }
                    else if(mapping == AsyncMapping)
                    {
                        name = "begin_" + op->name;
                    }
                    else
                    {
                        name = fixIdent(op->name);
                    }
                    PyErr_Format(PyExc_ValueError, STRCAST("invalid value for argument %d in operation `%s'"),
                                 info->pos + 1, const_cast<char*>(name.c_str()));
                    return false;
                }
            }

            //
            // Marshal the required parameters.
            //
            for(p = op->inParams.begin(); p != op->inParams.end(); ++p)
            {
                ParamInfoPtr info = *p;
                if(!info->optional)
                {
                    PyObject* arg = PyTuple_GET_ITEM(args, info->pos);
                    info->type->marshal(arg, os, &objectMap, false, &info->metaData);
                }
            }

            //
            // Marshal the optional parameters.
            //
            for(p = op->optionalInParams.begin(); p != op->optionalInParams.end(); ++p)
            {
                ParamInfoPtr info = *p;
                PyObject* arg = PyTuple_GET_ITEM(args, info->pos);
                if(arg != Unset && os->writeOptional(info->tag, info->type->optionalFormat()))
                {
                    info->type->marshal(arg, os, &objectMap, true, &info->metaData);
                }
            }

            if(op->sendsClasses)
            {
                os->writePendingValues();
            }

            os->endEncapsulation();
            params = os->finished();
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
IcePy::Invocation::unmarshalResults(const OperationPtr& op, const pair<const Ice::Byte*, const Ice::Byte*>& bytes)
{
    Py_ssize_t numResults = static_cast<Py_ssize_t>(op->outParams.size());
    if(op->returnType)
    {
        numResults++;
    }

    PyObjectHandle results = PyTuple_New(numResults);
    if(results.get() && numResults > 0)
    {
        Ice::InputStream is(_communicator, bytes);

        //
        // Store a pointer to a local StreamUtil object as the stream's closure.
        // This is necessary to support object unmarshaling (see ObjectReader).
        //
        StreamUtil util;
        assert(!is.getClosure());
        is.setClosure(&util);

        is.startEncapsulation();

        ParamInfoList::iterator p;

        //
        // Unmarshal the required out parameters.
        //
        for(p = op->outParams.begin(); p != op->outParams.end(); ++p)
        {
            ParamInfoPtr info = *p;
            if(!info->optional)
            {
                void* closure = reinterpret_cast<void*>(static_cast<Py_ssize_t>(info->pos));
                info->type->unmarshal(&is, info, results.get(), closure, false, &info->metaData);
            }
        }

        //
        // Unmarshal the required return value, if any.
        //
        if(op->returnType && !op->returnType->optional)
        {
            assert(op->returnType->pos == 0);
            void* closure = reinterpret_cast<void*>(static_cast<Py_ssize_t>(op->returnType->pos));
            op->returnType->type->unmarshal(&is, op->returnType, results.get(), closure, false, &op->metaData);
        }

        //
        // Unmarshal the optional results. This includes an optional return value.
        //
        for(p = op->optionalOutParams.begin(); p != op->optionalOutParams.end(); ++p)
        {
            ParamInfoPtr info = *p;
            if(is.readOptional(info->tag, info->type->optionalFormat()))
            {
                void* closure = reinterpret_cast<void*>(static_cast<Py_ssize_t>(info->pos));
                info->type->unmarshal(&is, info, results.get(), closure, true, &info->metaData);
            }
            else
            {
                PyTuple_SET_ITEM(results.get(), info->pos, incRef(Unset)); // PyTuple_SET_ITEM steals a reference.
            }
        }

        if(op->returnsClasses)
        {
            is.readPendingValues();
        }

        is.endEncapsulation();

        util.updateSlicedData();
    }

    return results.release();
}

PyObject*
IcePy::Invocation::unmarshalException(const OperationPtr& op, const pair<const Ice::Byte*, const Ice::Byte*>& bytes)
{
    Ice::InputStream is(_communicator, bytes);

    //
    // Store a pointer to a local StreamUtil object as the stream's closure.
    // This is necessary to support object unmarshaling (see ObjectReader).
    //
    StreamUtil util;
    assert(!is.getClosure());
    is.setClosure(&util);

    is.startEncapsulation();

    try
    {
        Ice::UserExceptionFactoryPtr factory = new UserExceptionFactory;
        is.throwException(factory);
    }
    catch(const ExceptionReader& r)
    {
        is.endEncapsulation();

        PyObject* ex = r.getException();

        if(validateException(op, ex))
        {
            util.updateSlicedData();

            Ice::SlicedDataPtr slicedData = r.getSlicedData();
            if(slicedData)
            {
                StreamUtil::setSlicedDataMember(ex, slicedData);
            }

            return incRef(ex);
        }
        else
        {
            try
            {
                PyException pye(ex); // No traceback information available.
                pye.raise();
            }
            catch(const Ice::UnknownUserException& uue)
            {
                return convertException(uue);
            }
        }
    }

    //
    // Getting here should be impossible: we can get here only if the
    // sender has marshaled a sequence of type IDs, none of which we
    // have a factory for. This means that sender and receiver disagree
    // about the Slice definitions they use.
    //
    Ice::UnknownUserException uue(__FILE__, __LINE__, "unknown exception");
    return convertException(uue);
#ifdef __SUNPRO_CC
    return 0;
#endif
}

bool
IcePy::Invocation::validateException(const OperationPtr& op, PyObject* ex) const
{
    for(ExceptionInfoList::const_iterator p = op->exceptions.begin(); p != op->exceptions.end(); ++p)
    {
        if(PyObject_IsInstance(ex, (*p)->pythonType.get()))
        {
            return true;
        }
    }

    return false;
}

void
IcePy::Invocation::checkTwowayOnly(const OperationPtr& op, const Ice::ObjectPrx& proxy) const
{
    if((op->returnType != 0 || !op->outParams.empty() || !op->exceptions.empty()) && !proxy->ice_isTwoway())
    {
        Ice::TwowayOnlyException ex(__FILE__, __LINE__);
        ex.operation = op->name;
        throw ex;
    }
}

//
// SyncTypedInvocation
//
IcePy::SyncTypedInvocation::SyncTypedInvocation(const Ice::ObjectPrx& prx, const OperationPtr& op) :
    Invocation(prx), _op(op)
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
    Ice::OutputStream os(_communicator);
    pair<const Ice::Byte*, const Ice::Byte*> params;
    if(!prepareRequest(_op, pyparams, SyncMapping, &os, params))
    {
        return 0;
    }

    try
    {
        checkTwowayOnly(_op, _prx);

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
                pair<const Ice::Byte*, const Ice::Byte*> rb(static_cast<const Ice::Byte*>(0),
                                                            static_cast<const Ice::Byte*>(0));
                if(!result.empty())
                {
                    rb.first = &result[0];
                    rb.second = &result[0] + result.size();
                }
                PyObjectHandle ex = unmarshalException(_op, rb);

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
                pair<const Ice::Byte*, const Ice::Byte*> rb(static_cast<const Ice::Byte*>(0),
                                                            static_cast<const Ice::Byte*>(0));
                if(!result.empty())
                {
                    rb.first = &result[0];
                    rb.second = &result[0] + result.size();
                }
                PyObjectHandle results = unmarshalResults(_op, rb);
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
                        return incRef(ret);
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

    return incRef(Py_None);
}

//
// AsyncTypedInvocation
//
IcePy::AsyncTypedInvocation::AsyncTypedInvocation(const Ice::ObjectPrx& prx, PyObject* pyProxy,
                                                  const OperationPtr& op) :
    Invocation(prx), _op(op), _pyProxy(pyProxy), _response(0), _ex(0), _sent(0)
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
        _response = incRef(callable);
    }
    else if(callable != Py_None)
    {
        PyErr_Format(PyExc_RuntimeError, STRCAST("response callback must be a callable object or None"));
        return 0;
    }

    callable = PyTuple_GET_ITEM(args, 2);
    if(PyCallable_Check(callable))
    {
        _ex = incRef(callable);
    }
    else if(callable != Py_None)
    {
        PyErr_Format(PyExc_RuntimeError, STRCAST("exception callback must be a callable object or None"));
        return 0;
    }

    callable = PyTuple_GET_ITEM(args, 3);
    if(PyCallable_Check(callable))
    {
        _sent = incRef(callable);
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
    Ice::OutputStream os(_communicator);
    pair<const Ice::Byte*, const Ice::Byte*> params;
    if(!prepareRequest(_op, pyparams, AsyncMapping, &os, params))
    {
        return 0;
    }

    Ice::AsyncResultPtr result;
    try
    {
        checkAsyncTwowayOnly(_prx);

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

            if(cb)
            {
                result = _prx->begin_ice_invoke(_op->name, _op->sendMode, params, ctx, cb);
            }
            else
            {
                result = _prx->begin_ice_invoke(_op->name, _op->sendMode, params, ctx);
            }
        }
        else
        {
            if(cb)
            {
                result = _prx->begin_ice_invoke(_op->name, _op->sendMode, params, cb);
            }
            else
            {
                result = _prx->begin_ice_invoke(_op->name, _op->sendMode, params);
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
        PyErr_Format(PyExc_RuntimeError, "%s", STRCAST(ex.reason().c_str()));
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
    AsyncResultObject* obj = asyncResultNew(&AsyncResultType, 0, 0);
    if(!obj)
    {
        return 0;
    }
    obj->result = new Ice::AsyncResultPtr(result);
    obj->invocation = new InvocationPtr(this);
    obj->proxy = incRef(_pyProxy);
    obj->communicator = getCommunicatorWrapper(_communicator);
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
            ok = proxy->_iceI_end_ice_invoke(results, r);
        }

        if(ok)
        {
            //
            // Unmarshal the results.
            //
            PyObjectHandle args = unmarshalResults(_op, results);
            if(args.get())
            {
                //
                // If there are no results, return None. If there's only one element
                // in the tuple, return the element. Otherwise, return the tuple.
                //
                assert(PyTuple_Check(args.get()));
                if(PyTuple_GET_SIZE(args.get()) == 0)
                {
                    return incRef(Py_None);
                }
                else if(PyTuple_GET_SIZE(args.get()) == 1)
                {
                    PyObject* res = PyTuple_GET_ITEM(args.get(), 0);
                    return incRef(res);
                }
                else
                {
                    return args.release();
                }
            }
        }
        else
        {
            PyObjectHandle ex = unmarshalException(_op, results);
            setPythonException(ex.get());
        }
    }
    catch(const AbortMarshaling&)
    {
        // Nothing to do.
    }
    catch(const IceUtil::IllegalArgumentException& ex)
    {
        PyErr_Format(PyExc_RuntimeError, "%s", STRCAST(ex.reason().c_str()));
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
                    args = unmarshalResults(_op, results);
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
            PyObjectHandle ex = unmarshalException(_op, results);
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
// NewAsyncInvocation
//
IcePy::NewAsyncInvocation::NewAsyncInvocation(const Ice::ObjectPrx& prx, PyObject* pyProxy, const string& operation)
    : Invocation(prx), _pyProxy(pyProxy), _operation(operation), _twoway(prx->ice_isTwoway()), _sent(false),
      _sentSynchronously(false), _done(false), _future(0), _ok(false), _exception(0)
{
    Py_INCREF(_pyProxy);
}

IcePy::NewAsyncInvocation::~NewAsyncInvocation()
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    Py_DECREF(_pyProxy);
    Py_XDECREF(_future);
    Py_XDECREF(_exception);
}

PyObject*
IcePy::NewAsyncInvocation::invoke(PyObject* args, PyObject* kwds)
{
    //
    // Called from Python code, so the GIL is already acquired.
    //

    Ice::AsyncResultPtr result;

    try
    {
        result = handleInvoke(args, kwds);
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
        // TwowayOnlyException can propagate directly.
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

    if(PyErr_Occurred())
    {
        return 0;
    }

    assert(result);

    //
    // NOTE: Any time we call into interpreted Python code there's a chance that another thread will be
    // allowed to run!
    //

    PyObjectHandle communicatorObj = getCommunicatorWrapper(_communicator);

    PyObjectHandle asyncResultObj = createAsyncResult(result, _pyProxy, 0, communicatorObj.get());
    if(!asyncResultObj.get())
    {
        return 0;
    }

    PyObjectHandle future = createFuture(_operation, asyncResultObj.get()); // Calls into Python code.
    if(!future.get())
    {
        return 0;
    }

    //
    // Check if any callbacks have been invoked already.
    //
    if(!_prx->ice_isBatchOneway() && !_prx->ice_isBatchDatagram())
    {
        if(_sent)
        {
            PyObjectHandle tmp = callMethod(future.get(), "set_sent", _sentSynchronously ? getTrue() : getFalse());
            if(PyErr_Occurred())
            {
                return 0;
            }

            if(!_twoway)
            {
                //
                // For a oneway/datagram invocation, we consider it complete when sent.
                //
                tmp = callMethod(future.get(), "set_result", Py_None);
                if(PyErr_Occurred())
                {
                    return 0;
                }
            }
        }

        if(_done)
        {
            if(_exception)
            {
                PyObjectHandle tmp = callMethod(future.get(), "set_exception", _exception);
                if(PyErr_Occurred())
                {
                    return 0;
                }
            }
            else
            {
                //
                // Delegate to the subclass.
                //
                pair<const Ice::Byte*, const Ice::Byte*> p(&_results[0], &_results[0] + _results.size());
                handleResponse(future.get(), _ok, p);
                if(PyErr_Occurred())
                {
                    return 0;
                }
            }
        }
        _future = future.release();
        return incRef(_future);
    }
    else
    {
        PyObjectHandle tmp = callMethod(future.get(), "set_result", Py_None);
        if(PyErr_Occurred())
        {
            return 0;
        }
        return future.release();
    }
}

void
IcePy::NewAsyncInvocation::response(bool ok, const pair<const Ice::Byte*, const Ice::Byte*>& results)
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    if(!_future)
    {
        //
        // The future hasn't been created yet, which means invoke() is still running. Save the results for later.
        //
        _ok = ok;
        vector<Ice::Byte> v(results.first, results.second);
        _results.swap(v);
        _done = true;
        return;
    }

    PyObjectHandle future = _future; // Steals a reference.

    if(_sent)
    {
        _future = 0; // Break cyclic dependency.
    }
    else
    {
        assert(!_done);

        //
        // The sent callback will release our reference.
        //
        Py_INCREF(_future);
    }

    _done = true;

    //
    // Delegate to the subclass.
    //
    handleResponse(future.get(), ok, results);
    if(PyErr_Occurred())
    {
        handleException();
    }
}

void
IcePy::NewAsyncInvocation::exception(const Ice::Exception& ex)
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    PyObjectHandle exh = convertException(ex); // NOTE: This can release the GIL

    if(!_future)
    {
        //
        // The future hasn't been created yet, which means invoke() is still running. Save the exception for later.
        //
        _exception = exh.release();
        _done = true;
        return;
    }

    PyObjectHandle future = _future; // Steals a reference.
    _future = 0; // Break cyclic dependency.
    _done = true;

    assert(exh.get());
    PyObjectHandle tmp = callMethod(future.get(), "set_exception", exh.get());
    if(PyErr_Occurred())
    {
        handleException();
    }
}

void
IcePy::NewAsyncInvocation::sent(bool sentSynchronously)
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    if(!_future)
    {
        //
        // The future hasn't been created yet, which means invoke() is still running.
        //
        _sent = true;
        _sentSynchronously = sentSynchronously;
        return;
    }

    PyObjectHandle future = _future;

    if(_done || !_twoway)
    {
        _future = 0; // Break cyclic dependency.
    }
    else
    {
        _sent = true;

        //
        // The reference to _future will be released in response() or exception().
        //
        Py_INCREF(_future);
    }

    PyObjectHandle tmp = callMethod(future.get(), "set_sent", sentSynchronously ? getTrue() : getFalse());
    if(PyErr_Occurred())
    {
        handleException();
    }

    if(!_twoway)
    {
        //
        // For a oneway/datagram invocation, we consider it complete when sent.
        //
        tmp = callMethod(future.get(), "set_result", Py_None);
        if(PyErr_Occurred())
        {
            handleException();
        }
    }
}

//
// NewAsyncTypedInvocation
//
IcePy::NewAsyncTypedInvocation::NewAsyncTypedInvocation(const Ice::ObjectPrx& prx, PyObject* pyProxy,
                                                        const OperationPtr& op)
    : NewAsyncInvocation(prx, pyProxy, op->name), _op(op)
{
}

Ice::AsyncResultPtr
IcePy::NewAsyncTypedInvocation::handleInvoke(PyObject* args, PyObject* /* kwds */)
{
    //
    // Called from Python code, so the GIL is already acquired.
    //

    assert(PyTuple_Check(args));
    assert(PyTuple_GET_SIZE(args) == 2); // Format is ((params...), context|None)
    PyObject* pyparams = PyTuple_GET_ITEM(args, 0);
    assert(PyTuple_Check(pyparams));
    PyObject* pyctx = PyTuple_GET_ITEM(args, 1);

    //
    // Marshal the input parameters to a byte sequence.
    //
    Ice::OutputStream os(_communicator);
    pair<const Ice::Byte*, const Ice::Byte*> params;
    if(!prepareRequest(_op, pyparams, NewAsyncMapping, &os, params))
    {
        return 0;
    }

    checkTwowayOnly(_op, _prx);

    NewAsyncInvocationPtr self = this;
    Ice::Callback_Object_ice_invokePtr cb;
    if(!_prx->ice_isBatchOneway() && !_prx->ice_isBatchDatagram())
    {
        cb = Ice::newCallback_Object_ice_invoke(self,
                                                &NewAsyncInvocation::response,
                                                &NewAsyncInvocation::exception,
                                                &NewAsyncInvocation::sent);
    }

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

        if(cb)
        {
            return _prx->begin_ice_invoke(_op->name, _op->sendMode, params, ctx, cb);
        }
        else
        {
            return _prx->begin_ice_invoke(_op->name, _op->sendMode, params, ctx);
        }
    }
    else
    {
        if(cb)
        {
            return _prx->begin_ice_invoke(_op->name, _op->sendMode, params, cb);
        }
        else
        {
            return _prx->begin_ice_invoke(_op->name, _op->sendMode, params);
        }
    }
}

void
IcePy::NewAsyncTypedInvocation::handleResponse(PyObject* future, bool ok,
                                               const pair<const Ice::Byte*, const Ice::Byte*>& results)
{
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
                args = unmarshalResults(_op, results);
                if(!args.get())
                {
                    assert(PyErr_Occurred());
                    return;
                }
            }
            catch(const Ice::Exception& ex)
            {
                PyObjectHandle exh = convertException(ex);
                assert(exh.get());
                PyObjectHandle tmp = callMethod(future, "set_exception", exh.get());
                PyErr_Clear();
                return;
            }

            //
            // The future's result is always one value:
            //
            // - If the operation has no out parameters, the result is None
            // - If the operation returns one value, the result is the value
            // - If the operation returns multiple values, the result is a tuple containing the values
            //
            PyObjectHandle r;
            if(PyTuple_GET_SIZE(args.get()) == 0)
            {
                r = incRef(Py_None);
            }
            else if(PyTuple_GET_SIZE(args.get()) == 1)
            {
                r = incRef(PyTuple_GET_ITEM(args.get(), 0)); // PyTuple_GET_ITEM steals a reference.
            }
            else
            {
                r = args;
            }

            PyObjectHandle tmp = callMethod(future, "set_result", r.get());
            PyErr_Clear();
        }
        else
        {
            PyObjectHandle ex = unmarshalException(_op, results);
            PyObjectHandle tmp = callMethod(future, "set_exception", ex.get());
            PyErr_Clear();
        }
    }
    catch(const AbortMarshaling&)
    {
        assert(PyErr_Occurred());
    }
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
#if PY_VERSION_HEX >= 0x03000000
    if(!PyArg_ParseTuple(args, STRCAST("sO!O!|O"), &operation, operationModeType, &mode, &PyBytes_Type, &inParams,
                         &ctx))
    {
        return 0;
    }
#else
    if(!PyArg_ParseTuple(args, STRCAST("sO!O!|O"), &operation, operationModeType, &mode, &PyBuffer_Type, &inParams,
                         &ctx))
    {
        return 0;
    }
#endif

    PyObjectHandle modeValue = getAttr(mode, "value", true);
    Ice::OperationMode sendMode = (Ice::OperationMode)static_cast<int>(PyLong_AsLong(modeValue.get()));
    assert(!PyErr_Occurred());

#if PY_VERSION_HEX >= 0x03000000
    Py_ssize_t sz = PyBytes_GET_SIZE(inParams);
    pair<const ::Ice::Byte*, const ::Ice::Byte*> in(static_cast<const Ice::Byte*>(0),
                                                    static_cast<const Ice::Byte*>(0));
    if(sz > 0)
    {
        in.first = reinterpret_cast<Ice::Byte*>(PyBytes_AS_STRING(inParams));
        in.second = in.first + sz;
    }
#else
    //
    // Use the array API to avoid copying the data.
    //
    char* charBuf = 0;
    Py_ssize_t sz = inParams->ob_type->tp_as_buffer->bf_getcharbuffer(inParams, 0, &charBuf);
    const Ice::Byte* mem = reinterpret_cast<const Ice::Byte*>(charBuf);
    pair<const ::Ice::Byte*, const ::Ice::Byte*> in(static_cast<const Ice::Byte*>(0),
                                                    static_cast<const Ice::Byte*>(0));
    if(sz > 0)
    {
        in.first = mem;
        in.second = mem + sz;
    }
#endif

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

        PyTuple_SET_ITEM(result.get(), 0, ok ? incTrue() : incFalse());

#if PY_VERSION_HEX >= 0x03000000
        PyObjectHandle op;
        if(out.empty())
        {
            op = PyBytes_FromString("");
        }
        else
        {
            op = PyBytes_FromStringAndSize(reinterpret_cast<const char*>(&out[0]), out.size());
        }
        if(!op.get())
        {
            throwPythonException();
        }
#else
        //
        // Create the output buffer and copy in the outParams.
        //
        PyObjectHandle op = PyBuffer_New(out.size());
        if(!op.get())
        {
            throwPythonException();
        }
        if(!out.empty())
        {
            void* buf;
            Py_ssize_t sz;
            if(PyObject_AsWriteBuffer(op.get(), &buf, &sz))
            {
                throwPythonException();
            }
            memcpy(buf, &out[0], sz);
        }
#endif

        PyTuple_SET_ITEM(result.get(), 1, op.release()); // PyTuple_SET_ITEM steals a reference.

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
        const_cast<char*>("context"),
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
#if PY_VERSION_HEX >= 0x03000000
    if(!PyArg_ParseTupleAndKeywords(args, kwds, STRCAST("sO!O!|OOOO"), argNames, &operation, operationModeType, &mode,
                                    &PyBytes_Type, &inParams, &response, &ex, &sent, &pyctx))
    {
        return 0;
    }
#else
    if(!PyArg_ParseTupleAndKeywords(args, kwds, STRCAST("sO!O!|OOOO"), argNames, &operation, operationModeType, &mode,
                                    &PyBuffer_Type, &inParams, &response, &ex, &sent, &pyctx))
    {
        return 0;
    }
#endif

    _op = operation;

    PyObjectHandle modeValue = getAttr(mode, "value", true);
    Ice::OperationMode sendMode = (Ice::OperationMode)static_cast<int>(PyLong_AsLong(modeValue.get()));
    assert(!PyErr_Occurred());

    if(PyCallable_Check(response))
    {
        _response = incRef(response);
    }
    else if(response != Py_None)
    {
        PyErr_Format(PyExc_RuntimeError, STRCAST("response callback must be a callable object or None"));
        return 0;
    }

    if(PyCallable_Check(ex))
    {
        _ex = incRef(ex);
    }
    else if(ex != Py_None)
    {
        PyErr_Format(PyExc_RuntimeError, STRCAST("exception callback must be a callable object or None"));
        return 0;
    }

    if(PyCallable_Check(sent))
    {
        _sent = incRef(sent);
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

#if PY_VERSION_HEX >= 0x03000000
    Py_ssize_t sz = PyBytes_GET_SIZE(inParams);
    pair<const ::Ice::Byte*, const ::Ice::Byte*> in(static_cast<const Ice::Byte*>(0),
                                                    static_cast<const Ice::Byte*>(0));
    if(sz > 0)
    {
        in.first = reinterpret_cast<Ice::Byte*>(PyBytes_AS_STRING(inParams));
        in.second = in.first + sz;
    }
#else
    //
    // Use the array API to avoid copying the data.
    //
    char* charBuf = 0;
    Py_ssize_t sz = inParams->ob_type->tp_as_buffer->bf_getcharbuffer(inParams, 0, &charBuf);
    const Ice::Byte* mem = reinterpret_cast<const Ice::Byte*>(charBuf);
    pair<const ::Ice::Byte*, const ::Ice::Byte*> in(static_cast<const Ice::Byte*>(0),
                                                    static_cast<const Ice::Byte*>(0));
    if(sz > 0)
    {
        in.first = mem;
        in.second = mem + sz;
    }
#endif

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
    AsyncResultObject* obj = asyncResultNew(&AsyncResultType, 0, 0);
    if(!obj)
    {
        return 0;
    }
    obj->result = new Ice::AsyncResultPtr(result);
    obj->invocation = new InvocationPtr(this);
    obj->proxy = incRef(_pyProxy);
    obj->communicator = getCommunicatorWrapper(_prx->ice_getCommunicator());
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
            ok = proxy->_iceI_end_ice_invoke(results, r);
        }

        //
        // Prepare the results as a tuple of the bool and out param buffer.
        //
        PyObjectHandle args = PyTuple_New(2);
        if(!args.get())
        {
            return 0;
        }

        PyTuple_SET_ITEM(args.get(), 0, ok ? incTrue() : incFalse());

#if PY_VERSION_HEX >= 0x03000000
        Py_ssize_t sz = results.second - results.first;
        PyObjectHandle op;
        if(sz == 0)
        {
            op = PyBytes_FromString("");
        }
        else
        {
            op = PyBytes_FromStringAndSize(reinterpret_cast<const char*>(results.first), sz);
        }
        if(!op.get())
        {
            return 0;
        }
#else
        //
        // Create the output buffer and copy in the outParams.
        //
        PyObjectHandle op = PyBuffer_New(results.second - results.first);
        if(!op.get())
        {
            return 0;
        }

        void* buf;
        Py_ssize_t sz;
        if(PyObject_AsWriteBuffer(op.get(), &buf, &sz))
        {
            return 0;
        }
        assert(sz == results.second - results.first);
        memcpy(buf, results.first, sz);
#endif

        PyTuple_SET_ITEM(args.get(), 1, op.release()); // PyTuple_SET_ITEM steals a reference.

        return args.release();
    }
    catch(const AbortMarshaling&)
    {
        // Nothing to do.
    }
    catch(const IceUtil::IllegalArgumentException& ex)
    {
        PyErr_Format(PyExc_RuntimeError, "%s", STRCAST(ex.reason().c_str()));
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

        PyTuple_SET_ITEM(args.get(), 0, ok ? incTrue() : incFalse());

#if PY_VERSION_HEX >= 0x03000000
        Py_ssize_t sz = results.second - results.first;
        PyObjectHandle op;
        if(sz == 0)
        {
            op = PyBytes_FromString("");
        }
        else
        {
            op = PyBytes_FromStringAndSize(reinterpret_cast<const char*>(results.first), sz);
        }
        if(!op.get())
        {
            assert(PyErr_Occurred());
            PyErr_Print();
            return;
        }
#else
        //
        // Create the output buffer and copy in the outParams.
        //
        PyObjectHandle op = PyBuffer_New(results.second - results.first);
        if(!op.get())
        {
            assert(PyErr_Occurred());
            PyErr_Print();
            return;
        }

        void* buf;
        Py_ssize_t sz;
        if(PyObject_AsWriteBuffer(op.get(), &buf, &sz))
        {
            assert(PyErr_Occurred());
            PyErr_Print();
            return;
        }
        assert(sz == results.second - results.first);
        memcpy(buf, results.first, sz);
#endif

        PyTuple_SET_ITEM(args.get(), 1, op.release()); // PyTuple_SET_ITEM steals a reference.

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
// NewAsyncBlobjectInvocation
//
IcePy::NewAsyncBlobjectInvocation::NewAsyncBlobjectInvocation(const Ice::ObjectPrx& prx, PyObject* pyProxy) :
    NewAsyncInvocation(prx, pyProxy, "ice_invoke")
{
}

Ice::AsyncResultPtr
IcePy::NewAsyncBlobjectInvocation::handleInvoke(PyObject* args, PyObject* /* kwds */)
{
    char* operation;
    PyObject* mode;
    PyObject* inParams;
    PyObject* operationModeType = lookupType("Ice.OperationMode");
    PyObject* ctx = 0;
#if PY_VERSION_HEX >= 0x03000000
    if(!PyArg_ParseTuple(args, STRCAST("sO!O!|O"), &operation, operationModeType, &mode,
                         &PyBytes_Type, &inParams, &ctx))
    {
        return 0;
    }
#else
    if(!PyArg_ParseTuple(args, STRCAST("sO!O!|O"), &operation, operationModeType, &mode,
                         &PyBuffer_Type, &inParams, &ctx))
    {
        return 0;
    }
#endif

    _op = operation;

    PyObjectHandle modeValue = getAttr(mode, "value", true);
    Ice::OperationMode sendMode = (Ice::OperationMode)static_cast<int>(PyLong_AsLong(modeValue.get()));
    assert(!PyErr_Occurred());

#if PY_VERSION_HEX >= 0x03000000
    Py_ssize_t sz = PyBytes_GET_SIZE(inParams);
    pair<const ::Ice::Byte*, const ::Ice::Byte*> in(static_cast<const Ice::Byte*>(0),
                                                    static_cast<const Ice::Byte*>(0));
    if(sz > 0)
    {
        in.first = reinterpret_cast<Ice::Byte*>(PyBytes_AS_STRING(inParams));
        in.second = in.first + sz;
    }
#else
    //
    // Use the array API to avoid copying the data.
    //
    char* charBuf = 0;
    Py_ssize_t sz = inParams->ob_type->tp_as_buffer->bf_getcharbuffer(inParams, 0, &charBuf);
    const Ice::Byte* mem = reinterpret_cast<const Ice::Byte*>(charBuf);
    pair<const ::Ice::Byte*, const ::Ice::Byte*> in(static_cast<const Ice::Byte*>(0),
                                                    static_cast<const Ice::Byte*>(0));
    if(sz > 0)
    {
        in.first = mem;
        in.second = mem + sz;
    }
#endif

    NewAsyncInvocationPtr self = this;
    Ice::Callback_Object_ice_invokePtr cb;
    if(!_prx->ice_isBatchOneway() && !_prx->ice_isBatchDatagram())
    {
        cb = Ice::newCallback_Object_ice_invoke(self,
                                                &NewAsyncInvocation::response,
                                                &NewAsyncInvocation::exception,
                                                &NewAsyncInvocation::sent);
    }

    if(ctx == 0 || ctx == Py_None)
    {
        if(cb)
        {
            return _prx->begin_ice_invoke(operation, sendMode, in, cb);
        }
        else
        {
            return _prx->begin_ice_invoke(operation, sendMode, in);
        }
    }
    else
    {
        Ice::Context context;
        if(!dictionaryToContext(ctx, context))
        {
            return 0;
        }

        if(cb)
        {
            return _prx->begin_ice_invoke(operation, sendMode, in, context, cb);
        }
        else
        {
            return _prx->begin_ice_invoke(operation, sendMode, in, context);
        }
    }
}

void
IcePy::NewAsyncBlobjectInvocation::handleResponse(PyObject* future, bool ok,
                                                  const pair<const Ice::Byte*, const Ice::Byte*>& results)
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

    PyTuple_SET_ITEM(args.get(), 0, ok ? incTrue() : incFalse());

#if PY_VERSION_HEX >= 0x03000000
    Py_ssize_t sz = results.second - results.first;
    PyObjectHandle op;
    if(sz == 0)
    {
        op = PyBytes_FromString("");
    }
    else
    {
        op = PyBytes_FromStringAndSize(reinterpret_cast<const char*>(results.first), sz);
    }
    if(!op.get())
    {
        assert(PyErr_Occurred());
        PyErr_Print();
        return;
    }
#else
    //
    // Create the output buffer and copy in the outParams.
    //
    PyObjectHandle op = PyBuffer_New(results.second - results.first);
    if(!op.get())
    {
        assert(PyErr_Occurred());
        PyErr_Print();
        return;
    }

    void* buf;
    Py_ssize_t sz;
    if(PyObject_AsWriteBuffer(op.get(), &buf, &sz))
    {
        assert(PyErr_Occurred());
        PyErr_Print();
        return;
    }
    assert(sz == results.second - results.first);
    memcpy(buf, results.first, sz);
#endif

    PyTuple_SET_ITEM(args.get(), 1, op.release()); // PyTuple_SET_ITEM steals a reference.

    PyObjectHandle tmp = callMethod(future, "set_result", args.get());
    PyErr_Clear();
}

//
// Upcall
//
void
Upcall::dispatchImpl(PyObject* servant, const string& dispatchName, PyObject* args, const Ice::Current& current)
{
    Ice::CommunicatorPtr communicator = current.adapter->getCommunicator();

    //
    // Find the servant method for the operation. Use dispatchName here, not current.operation.
    //
    PyObjectHandle servantMethod = getAttr(servant, dispatchName, false);
    if(!servantMethod.get())
    {
        ostringstream ostr;
        ostr << "servant for identity " << communicator->identityToString(current.id)
             << " does not define operation `" << dispatchName << "'";
        string str = ostr.str();
        PyErr_WarnEx(PyExc_RuntimeWarning, const_cast<char*>(str.c_str()), 1);
        Ice::UnknownException ex(__FILE__, __LINE__);
        ex.unknown = str;
        throw ex;
    }

    //
    // Get the _iceDispatch method. The _iceDispatch method will invoke the servant method and pass it the arguments.
    //
    PyObjectHandle dispatchMethod = getAttr(servant, "_iceDispatch", false);
    if(!dispatchMethod.get())
    {
        ostringstream ostr;
        ostr << "_iceDispatch method not found for identity " << communicator->identityToString(current.id)
             << " and operation `" << dispatchName << "'";
        string str = ostr.str();
        PyErr_WarnEx(PyExc_RuntimeWarning, const_cast<char*>(str.c_str()), 1);
        Ice::UnknownException ex(__FILE__, __LINE__);
        ex.unknown = str;
        throw ex;
    }

    PyObjectHandle dispatchArgs = PyTuple_New(3);
    if(!dispatchArgs.get())
    {
        throwPythonException();
    }

    DispatchCallbackObject* callback = dispatchCallbackNew(&DispatchCallbackType, 0, 0);
    if(!callback)
    {
        throwPythonException();
    }
    callback->upcall = new UpcallPtr(this);
    PyTuple_SET_ITEM(dispatchArgs.get(), 0, reinterpret_cast<PyObject*>(callback)); // Steals a reference.
    PyTuple_SET_ITEM(dispatchArgs.get(), 1, servantMethod.release()); // Steals a reference.
    PyTuple_SET_ITEM(dispatchArgs.get(), 2, incRef(args)); // Steals a reference.

    //
    // Ignore the return value of _iceDispatch -- it will use the dispatch callback.
    //
    PyObjectHandle ignore = PyObject_Call(dispatchMethod.get(), dispatchArgs.get(), 0);

    //
    // Check for exceptions.
    //
    if(PyErr_Occurred())
    {
        PyException ex; // Retrieve it before another Python API call clears it.
        exception(ex);
    }
}

//
// TypedUpcall
//
IcePy::TypedUpcall::TypedUpcall(const OperationPtr& op, const Ice::AMD_Object_ice_invokePtr& callback,
                                const Ice::CommunicatorPtr& communicator) :
    _op(op), _callback(callback), _communicator(communicator)
{
}

void
IcePy::TypedUpcall::dispatch(PyObject* servant, const pair<const Ice::Byte*, const Ice::Byte*>& inBytes,
                             const Ice::Current& current)
{
    _encoding = current.encoding;

    //
    // Unmarshal the in parameters. We have to leave room in the arguments for a trailing
    // Ice::Current object.
    //
    Py_ssize_t count = static_cast<Py_ssize_t>(_op->inParams.size()) + 1;

    PyObjectHandle args = PyTuple_New(count);
    if(!args.get())
    {
        throwPythonException();
    }

    if(!_op->inParams.empty())
    {
        Ice::InputStream is(_communicator, inBytes);

        //
        // Store a pointer to a local StreamUtil object as the stream's closure.
        // This is necessary to support object unmarshaling (see ObjectReader).
        //
        StreamUtil util;
        assert(!is.getClosure());
        is.setClosure(&util);

        try
        {
            is.startEncapsulation();

            ParamInfoList::iterator p;

            //
            // Unmarshal the required parameters.
            //
            for(p = _op->inParams.begin(); p != _op->inParams.end(); ++p)
            {
                ParamInfoPtr info = *p;
                if(!info->optional)
                {
                    void* closure = reinterpret_cast<void*>(static_cast<Py_ssize_t>(info->pos));
                    info->type->unmarshal(&is, info, args.get(), closure, false, &info->metaData);
                }
            }

            //
            // Unmarshal the optional parameters.
            //
            for(p = _op->optionalInParams.begin(); p != _op->optionalInParams.end(); ++p)
            {
                ParamInfoPtr info = *p;
                if(is.readOptional(info->tag, info->type->optionalFormat()))
                {
                    void* closure = reinterpret_cast<void*>(static_cast<Py_ssize_t>(info->pos));
                    info->type->unmarshal(&is, info, args.get(), closure, true, &info->metaData);
                }
                else
                {
                    PyTuple_SET_ITEM(args.get(), info->pos, incRef(Unset)); // PyTuple_SET_ITEM steals a reference.
                }
            }

            if(_op->sendsClasses)
            {
                is.readPendingValues();
            }

            is.endEncapsulation();

            util.updateSlicedData();
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
    PyTuple_SET_ITEM(args.get(), PyTuple_GET_SIZE(args.get()) - 1,
                     curr.release()); // PyTuple_SET_ITEM steals a reference.

    dispatchImpl(servant, _op->dispatchName, args.get(), current);
}

void
IcePy::TypedUpcall::response(PyObject* result)
{
    try
    {
        if(PyObject_IsInstance(result, reinterpret_cast<PyObject*>(&MarshaledResultType)))
        {
            MarshaledResultObject* mro = reinterpret_cast<MarshaledResultObject*>(result);
            _callback->ice_response(true, mro->out->finished());
        }
        else
        {
            try
            {
                Ice::OutputStream os(_communicator);
                os.startEncapsulation(_encoding, _op->format);

                _op->marshalResult(os, result);

                os.endEncapsulation();

                _callback->ice_response(true, os.finished());
            }
            catch(const AbortMarshaling&)
            {
                try
                {
                    throwPythonException();
                }
                catch(const Ice::Exception& ex)
                {
                    _callback->ice_exception(ex);
                }
            }
        }
    }
    catch(const Ice::Exception& ex)
    {
        _callback->ice_exception(ex);
    }
}

void
IcePy::TypedUpcall::exception(PyException& ex)
{
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
                // Get the exception's type.
                //
                PyObjectHandle iceType = getAttr(ex.ex.get(), "_ice_type", false);
                assert(iceType.get());
                ExceptionInfoPtr info = ExceptionInfoPtr::dynamicCast(getException(iceType.get()));
                assert(info);

                Ice::OutputStream os(_communicator);
                os.startEncapsulation(_encoding, _op->format);

                ExceptionWriter writer(ex.ex, info);
                os.writeException(writer);

                os.endEncapsulation();

                _callback->ice_response(false, os.finished());
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
        exception(ex);
    }
}

void
IcePy::TypedUpcall::exception(const Ice::Exception& ex)
{
    _callback->ice_exception(ex);
}

//
// BlobjectUpcall
//
IcePy::BlobjectUpcall::BlobjectUpcall(const Ice::AMD_Object_ice_invokePtr& callback) :
    _callback(callback)
{
}

void
IcePy::BlobjectUpcall::dispatch(PyObject* servant, const pair<const Ice::Byte*, const Ice::Byte*>& inBytes,
                                const Ice::Current& current)
{
    Ice::CommunicatorPtr communicator = current.adapter->getCommunicator();

    Py_ssize_t count = 2; // First is the inParams, second is the Ice::Current object.

    Py_ssize_t start = 0;

    PyObjectHandle args = PyTuple_New(count);
    if(!args.get())
    {
        throwPythonException();
    }

    PyObjectHandle ip;

#if PY_VERSION_HEX >= 0x03000000
    if(inBytes.second == inBytes.first)
    {
        ip = PyBytes_FromString("");
    }
    else
    {
        ip = PyBytes_FromStringAndSize(reinterpret_cast<const char*>(inBytes.first), inBytes.second - inBytes.first);
    }
#else
    //
    // Make a copy of the bytes since the bytes may be accessed after this method is over.
    //
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
#endif

    PyTuple_SET_ITEM(args.get(), start, ip.release()); // PyTuple_SET_ITEM steals a reference.
    ++start;

    //
    // Create an object to represent Ice::Current. We need to append
    // this to the argument tuple.
    //
    PyObjectHandle curr = createCurrent(current);
    PyTuple_SET_ITEM(args.get(), start, curr.release()); // PyTuple_SET_ITEM steals a reference.

    dispatchImpl(servant, "ice_invoke", args.get(), current);
}

void
IcePy::BlobjectUpcall::response(PyObject* result)
{
    try
    {
        //
        // The result is a tuple of (bool, results).
        //
        if(!PyTuple_Check(result) || PyTuple_GET_SIZE(result) != 2)
        {
            string str = "operation `ice_invoke' should return a tuple of length 2";
            PyErr_WarnEx(PyExc_RuntimeWarning, const_cast<char*>(str.c_str()), 1);
            throw Ice::MarshalException(__FILE__, __LINE__);
        }

        PyObject* arg = PyTuple_GET_ITEM(result, 0);
        bool isTrue = PyObject_IsTrue(arg) == 1;

        arg = PyTuple_GET_ITEM(result, 1);

#if PY_VERSION_HEX >= 0x03000000
        if(!PyBytes_Check(arg))
        {
            ostringstream ostr;
            ostr << "invalid return value for operation `ice_invoke'";
            string str = ostr.str();
            PyErr_WarnEx(PyExc_RuntimeWarning, const_cast<char*>(str.c_str()), 1);
            throw Ice::MarshalException(__FILE__, __LINE__);
        }

        Py_ssize_t sz = PyBytes_GET_SIZE(arg);
        pair<const ::Ice::Byte*, const ::Ice::Byte*> r(static_cast<const Ice::Byte*>(0),
                                                       static_cast<const Ice::Byte*>(0));
        if(sz > 0)
        {
            r.first = reinterpret_cast<Ice::Byte*>(PyBytes_AS_STRING(arg));
            r.second = r.first + sz;
        }
#else
        if(!PyBuffer_Check(arg))
        {
            ostringstream ostr;
            ostr << "invalid return value for operation `ice_invoke'";
            string str = ostr.str();
            PyErr_WarnEx(PyExc_RuntimeWarning, const_cast<char*>(str.c_str()), 1);
            throw Ice::MarshalException(__FILE__, __LINE__);
        }

        char* charBuf = 0;
        Py_ssize_t sz = arg->ob_type->tp_as_buffer->bf_getcharbuffer(arg, 0, &charBuf);
        const Ice::Byte* mem = reinterpret_cast<const Ice::Byte*>(charBuf);
        const pair<const ::Ice::Byte*, const ::Ice::Byte*> r(mem, mem + sz);
#endif

        _callback->ice_response(isTrue, r);
    }
    catch(const AbortMarshaling&)
    {
        try
        {
            throwPythonException();
        }
        catch(const Ice::Exception& ex)
        {
            exception(ex);
        }
    }
    catch(const Ice::Exception& ex)
    {
        exception(ex);
    }
}

void
IcePy::BlobjectUpcall::exception(PyException& ex)
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

        ex.raise();
    }
    catch(const Ice::Exception& ex)
    {
        exception(ex);
    }
}

void
IcePy::BlobjectUpcall::exception(const Ice::Exception& ex)
{
    _callback->ice_exception(ex);
}

PyObject*
IcePy::invokeBuiltin(PyObject* proxy, const string& builtin, PyObject* args)
{
    string name = "_op_" + builtin;
    PyObject* objectType = lookupType("Ice.Object");
    assert(objectType);
    PyObjectHandle obj = getAttr(objectType, name, false);
    assert(obj.get());

    OperationPtr op = getOperation(obj.get());
    assert(op);

    Ice::ObjectPrx p = getProxy(proxy);
    InvocationPtr i = new SyncTypedInvocation(p, op);
    return i->invoke(args);
}

PyObject*
IcePy::invokeBuiltinAsync(PyObject* proxy, const string& builtin, PyObject* args)
{
    string name = "_op_" + builtin;
    PyObject* objectType = lookupType("Ice.Object");
    assert(objectType);
    PyObjectHandle obj = getAttr(objectType, name, false);
    assert(obj.get());

    OperationPtr op = getOperation(obj.get());
    assert(op);

    Ice::ObjectPrx p = getProxy(proxy);
    InvocationPtr i = new NewAsyncTypedInvocation(p, proxy, op);
    return i->invoke(args);
}

PyObject*
IcePy::beginBuiltin(PyObject* proxy, const string& builtin, PyObject* args)
{
    string name = "_op_" + builtin;
    PyObject* objectType = lookupType("Ice.Object");
    assert(objectType);
    PyObjectHandle obj = getAttr(objectType, name, false);
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
    PyObjectHandle obj = getAttr(objectType, name, false);
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
    InvocationPtr i = new NewAsyncBlobjectInvocation(p, proxy);
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
    AsyncResultObject* obj = asyncResultNew(&AsyncResultType, 0, 0);
    if(!obj)
    {
        return 0;
    }
    obj->result = new Ice::AsyncResultPtr(r);
    obj->proxy = incRef(proxy);
    obj->connection = incRef(connection);
    obj->communicator = incRef(communicator);
    return reinterpret_cast<PyObject*>(obj);
}

Ice::AsyncResultPtr
IcePy::getAsyncResult(PyObject* p)
{
    assert(PyObject_IsInstance(p, reinterpret_cast<PyObject*>(&AsyncResultType)) == 1);
    AsyncResultObject* obj = reinterpret_cast<AsyncResultObject*>(p);
    return *obj->result;
}

//
// FlushCallback
//
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

IcePy::FlushAsyncCallback::FlushAsyncCallback(const string& op) :
    _op(op), _future(0), _sent(false), _sentSynchronously(false), _exception(0)
{
}

IcePy::FlushAsyncCallback::~FlushAsyncCallback()
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    Py_XDECREF(_future);
    Py_XDECREF(_exception);
}

void
IcePy::FlushAsyncCallback::setFuture(PyObject* future)
{
    //
    // Called with the GIL locked.
    //

    //
    // Check if any callbacks have been invoked already.
    //
    if(_exception)
    {
        PyObjectHandle tmp = callMethod(future, "set_exception", _exception);
        PyErr_Clear();
    }
    else if(_sent)
    {
        PyObjectHandle tmp = callMethod(future, "set_sent", _sentSynchronously ? getTrue() : getFalse());
        PyErr_Clear();
        //
        // We consider the invocation complete when sent.
        //
        tmp = callMethod(future, "set_result", Py_None);
        PyErr_Clear();
    }
    else
    {
        _future = incRef(future);
    }
}

void
IcePy::FlushAsyncCallback::exception(const Ice::Exception& ex)
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    if(!_future)
    {
        //
        // The future hasn't been set yet, which means the request is still being invoked. Save the results for later.
        //
        _exception = convertException(ex);
        return;
    }

    PyObjectHandle exh = convertException(ex);
    assert(exh.get());
    PyObjectHandle tmp = callMethod(_future, "set_exception", exh.get());
    PyErr_Clear();

    Py_DECREF(_future); // Break cyclic dependency.
    _future = 0;
}

void
IcePy::FlushAsyncCallback::sent(bool sentSynchronously)
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    if(!_future)
    {
        //
        // The future hasn't been set yet, which means the request is still being invoked. Save the results for later.
        //
        _sent = true;
        _sentSynchronously = sentSynchronously;
        return;
    }

    PyObjectHandle tmp = callMethod(_future, "set_sent", _sentSynchronously ? getTrue() : getFalse());
    PyErr_Clear();
    //
    // We consider the invocation complete when sent.
    //
    tmp = callMethod(_future, "set_result", Py_None);
    PyErr_Clear();

    Py_DECREF(_future); // Break cyclic dependency.
    _future = 0;
}

IcePy::GetConnectionCallback::GetConnectionCallback(const Ice::CommunicatorPtr& communicator,
                                                    PyObject* response, PyObject* ex, const string& op) :
    _communicator(communicator), _response(response), _ex(ex), _op(op)
{
    assert(_response);
    Py_INCREF(_response);
    Py_XINCREF(_ex);
}

IcePy::GetConnectionCallback::~GetConnectionCallback()
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    Py_DECREF(_response);
    Py_XDECREF(_ex);
}

void
IcePy::GetConnectionCallback::response(const Ice::ConnectionPtr& conn)
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    PyObjectHandle pyConn = createConnection(conn, _communicator);
    PyObjectHandle args = Py_BuildValue(STRCAST("(O)"), pyConn.get());
    PyObjectHandle tmp = PyObject_Call(_response, args.get(), 0);
    if(PyErr_Occurred())
    {
        handleException(); // Callback raised an exception.
    }
}

void
IcePy::GetConnectionCallback::exception(const Ice::Exception& ex)
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    callException(_ex, ex);
}

IcePy::GetConnectionAsyncCallback::GetConnectionAsyncCallback(const Ice::CommunicatorPtr& communicator,
                                                              const string& op) :
    _communicator(communicator), _op(op), _future(0), _exception(0)
{
}

IcePy::GetConnectionAsyncCallback::~GetConnectionAsyncCallback()
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    Py_XDECREF(_future);
    Py_XDECREF(_exception);
}

void
IcePy::GetConnectionAsyncCallback::setFuture(PyObject* future)
{
    //
    // Called with the GIL locked.
    //

    //
    // Check if any callbacks have been invoked already.
    //
    if(_connection)
    {
        PyObjectHandle pyConn = createConnection(_connection, _communicator);
        assert(pyConn.get());
        PyObjectHandle tmp = callMethod(future, "set_result", pyConn.get());
        PyErr_Clear();
    }
    else if(_exception)
    {
        PyObjectHandle tmp = callMethod(future, "set_exception", _exception);
        PyErr_Clear();
    }
    else
    {
        _future = incRef(future);
    }
}

void
IcePy::GetConnectionAsyncCallback::response(const Ice::ConnectionPtr& conn)
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    if(!_future)
    {
        //
        // The future hasn't been set yet, which means the request is still being invoked. Save the results for later.
        //
        _connection = conn;
        return;
    }

    PyObjectHandle pyConn = createConnection(conn, _communicator);
    PyObjectHandle tmp = callMethod(_future, "set_result", pyConn.get());
    PyErr_Clear();

    Py_DECREF(_future); // Break cyclic dependency.
    _future = 0;
}

void
IcePy::GetConnectionAsyncCallback::exception(const Ice::Exception& ex)
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    if(!_future)
    {
        //
        // The future hasn't been set yet, which means the request is still being invoked. Save the results for later.
        //
        _exception = convertException(ex);
        return;
    }

    PyObjectHandle exh = convertException(ex);
    PyObjectHandle tmp = callMethod(_future, "set_exception", exh.get());
    PyErr_Clear();

    Py_DECREF(_future); // Break cyclic dependency.
    _future = 0;
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
                PyObjectHandle h = getAttr((PyObject*)_servant->ob_type, attrName, false);
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

        //
        // See bug 4976.
        //
        if(!op->pseudoOp)
        {
            _iceCheckMode(op->mode, current.mode);
        }

        UpcallPtr up = new TypedUpcall(op, cb, current.adapter->getCommunicator());
        up->dispatch(_servant, inParams, current);
    }
    catch(const Ice::Exception& ex)
    {
        cb->ice_exception(ex);
    }
}

//
// BlobjectServantWrapper implementation.
//
IcePy::BlobjectServantWrapper::BlobjectServantWrapper(PyObject* servant) :
    ServantWrapper(servant)
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
        UpcallPtr up = new BlobjectUpcall(cb);
        up->dispatch(_servant, inParams, current);
    }
    catch(const Ice::Exception& ex)
    {
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
        return new BlobjectServantWrapper(servant);
    }
    else if(PyObject_IsInstance(servant, blobjectAsyncType))
    {
        return new BlobjectServantWrapper(servant);
    }

    return new TypedServantWrapper(servant);
}

PyObject*
IcePy::createFuture()
{
    PyObject* futureType = lookupType("Ice.Future");
    assert(futureType);
    PyObjectHandle args = PyTuple_New(0);
    if(!args.get())
    {
        return 0;
    }
    PyTypeObject* type = reinterpret_cast<PyTypeObject*>(futureType);
    PyObject* future = type->tp_new(type, args.get(), 0);
    if(!future)
    {
        return 0;
    }
    type->tp_init(future, args.get(), 0); // Call the constructor
    return future;
}

PyObject*
IcePy::createFuture(const string& operation, PyObject* asyncResult)
{
    if(!asyncResult) // Can be nil for batch invocations.
    {
        asyncResult = Py_None;
    }

    PyObject* futureType = lookupType("Ice.InvocationFuture");
    assert(futureType);
    PyObjectHandle args = PyTuple_New(2);
    if(!args.get())
    {
        return 0;
    }
    PyTuple_SET_ITEM(args.get(), 0, createString(operation));
    PyTuple_SET_ITEM(args.get(), 1, incRef(asyncResult));
    PyTypeObject* type = reinterpret_cast<PyTypeObject*>(futureType);
    PyObject* future = type->tp_new(type, args.get(), 0);
    if(!future)
    {
        return 0;
    }
    type->tp_init(future, args.get(), 0); // Call the constructor
    return future;
}
