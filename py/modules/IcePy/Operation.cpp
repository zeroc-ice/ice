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
#include <Operation.h>
#include <Current.h>
#include <Proxy.h>
#include <Types.h>
#include <Util.h>
#include <Ice/Communicator.h>
#include <Ice/IdentityUtil.h>
#include <Ice/IncomingAsync.h>
#include <Ice/Initialize.h>
#include <Ice/LocalException.h>
#include <Ice/ObjectAdapter.h>
#include <Ice/OutgoingAsync.h>
#include <Ice/Proxy.h>
#include <Slice/PythonUtil.h>

using namespace std;
using namespace IcePy;
using namespace Slice::Python;

namespace IcePy
{

class ParamInfo : public UnmarshalCallback
{
public:

    virtual void unmarshaled(PyObject*, PyObject*, void*);

    Ice::StringSeq metaData;
    TypeInfoPtr type;
};
typedef IceUtil::Handle<ParamInfo> ParamInfoPtr;
typedef vector<ParamInfoPtr> ParamInfoList;

class OperationI : public Operation
{
public:

    OperationI(const char*, PyObject*, PyObject*, int, PyObject*, PyObject*, PyObject*, PyObject*, PyObject*);

    virtual PyObject* invoke(const Ice::ObjectPrx&, PyObject*, PyObject*);
    virtual PyObject* invokeAsync(const Ice::ObjectPrx&, PyObject*, PyObject*, PyObject*);
    virtual void deprecate(const string&);
    virtual Ice::OperationMode mode() const;

    virtual void dispatch(PyObject*, const Ice::AMD_Object_ice_invokePtr&, const vector<Ice::Byte>&,
                          const Ice::Current&);

    void responseAsync(PyObject*, bool, const vector<Ice::Byte>&, const Ice::CommunicatorPtr&);
    void responseAsyncException(PyObject*, PyObject*);

    void sendResponse(const Ice::AMD_Object_ice_invokePtr&, PyObject*, const Ice::CommunicatorPtr&);
    void sendException(const Ice::AMD_Object_ice_invokePtr&, PyException&, const Ice::CommunicatorPtr&);

private:

    string _name;
    Ice::OperationMode _mode;
    Ice::OperationMode _sendMode;
    bool _amd;
    Ice::StringSeq _metaData;
    ParamInfoList _inParams;
    ParamInfoList _outParams;
    ParamInfoPtr _returnType;
    ExceptionInfoList _exceptions;
    string _dispatchName;
    bool _sendsClasses;
    bool _returnsClasses;
    string _deprecateMessage;

    bool prepareRequest(const Ice::CommunicatorPtr&, PyObject*, bool, vector<Ice::Byte>&);
    PyObject* unmarshalResults(const vector<Ice::Byte>&, const Ice::CommunicatorPtr&);
    PyObject* unmarshalException(const vector<Ice::Byte>&, const Ice::CommunicatorPtr&);
    bool validateException(PyObject*) const;
    void checkTwowayOnly(const Ice::ObjectPrx&) const;
    static void convertParams(PyObject*, ParamInfoList&, bool&);
};
typedef IceUtil::Handle<OperationI> OperationIPtr;

class AMICallback : public Ice::AMI_Object_ice_invoke
{
public:

    AMICallback(const OperationIPtr&, const Ice::CommunicatorPtr&, PyObject*);
    ~AMICallback();

    virtual void ice_response(bool, const vector<Ice::Byte>&);
    virtual void ice_exception(const Ice::Exception&);

private:

    OperationIPtr _op;
    Ice::CommunicatorPtr _communicator;
    PyObject* _callback;
};

struct OperationObject
{
    PyObject_HEAD
    OperationPtr* op;
};

struct AMDCallbackObject
{
    PyObject_HEAD
    OperationIPtr* op;
    Ice::CommunicatorPtr* communicator;
    Ice::AMD_Object_ice_invokePtr* cb;
};

extern PyTypeObject OperationType;
extern PyTypeObject AMDCallbackType;

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

    OperationIPtr op = new OperationI(name, mode, sendMode, amd, meta, inParams, outParams, returnType, exceptions);
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
    PyObject* ctx;
    if(!PyArg_ParseTuple(args, STRCAST("O!O!O"), &ProxyType, &pyProxy, &PyTuple_Type, &opArgs, &ctx))
    {
        return 0;
    }

    if(ctx != Py_None && !PyDict_Check(ctx))
    {
        PyErr_Format(PyExc_ValueError, STRCAST("context argument must be None or a dictionary"));
        return 0;
    }

    Ice::ObjectPrx prx = getProxy(pyProxy);

    assert(self->op);
    return (*self->op)->invoke(prx, opArgs, ctx);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
operationInvokeAsync(OperationObject* self, PyObject* args)
{
    PyObject* pyProxy;
    PyObject* cb;
    PyObject* opArgs;
    PyObject* ctx;
    if(!PyArg_ParseTuple(args, STRCAST("O!OO!O"), &ProxyType, &pyProxy, &cb, &PyTuple_Type, &opArgs, &ctx))
    {
        return 0;
    }

    if(ctx != Py_None && !PyDict_Check(ctx))
    {
        PyErr_Format(PyExc_ValueError, STRCAST("context argument must be None or a dictionary"));
        return 0;
    }

    Ice::ObjectPrx prx = getProxy(pyProxy);

    assert(self->op);
    return (*self->op)->invokeAsync(prx, cb, opArgs, ctx);
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
    self->op = 0;
    self->communicator = 0;
    self->cb = 0;
    return self;
}

#ifdef WIN32
extern "C"
#endif
static void
amdCallbackDealloc(AMDCallbackObject* self)
{
    delete self->op;
    delete self->communicator;
    delete self->cb;
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
        assert(self->op);
        (*self->op)->sendResponse(*self->cb, args, *self->communicator);
    }
    catch(const Ice::Exception& ex)
    {
        (*self->cb)->ice_exception(ex);
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
        assert(self->op);
        PyException pye(ex); // No traceback information available.
        (*self->op)->sendException(*self->cb, pye, *self->communicator);
    }
    catch(const Ice::Exception& ex)
    {
        (*self->cb)->ice_exception(ex);
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
// Operation implementation.
//
IcePy::Operation::~Operation()
{
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
// AMICallback implementation.
//
IcePy::AMICallback::AMICallback(const OperationIPtr& op, const Ice::CommunicatorPtr& communicator,
                                PyObject* callback) :
    _op(op), _communicator(communicator), _callback(callback)
{
    Py_INCREF(_callback);
}

IcePy::AMICallback::~AMICallback()
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    Py_DECREF(_callback);
}

void
IcePy::AMICallback::ice_response(bool ok, const vector<Ice::Byte>& result)
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    _op->responseAsync(_callback, ok, result, _communicator);
}

void
IcePy::AMICallback::ice_exception(const Ice::Exception& ex)
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    PyObjectHandle exh = convertException(ex);
    assert(exh.get());
    _op->responseAsyncException(_callback, exh.get());
}

//
// OperationI implementation.
//
IcePy::OperationI::OperationI(const char* name, PyObject* mode, PyObject* sendMode, int amd, PyObject* meta,
                              PyObject* inParams, PyObject* outParams, PyObject* returnType, PyObject* exceptions)
{
    _name = name;

    //
    // mode
    //
    PyObjectHandle modeValue = PyObject_GetAttrString(mode, STRCAST("value"));
    assert(PyInt_Check(modeValue.get()));
    _mode = (Ice::OperationMode)static_cast<int>(PyInt_AS_LONG(modeValue.get()));

    //
    // sendMode
    //
    PyObjectHandle sendModeValue = PyObject_GetAttrString(sendMode, STRCAST("value"));
    assert(PyInt_Check(sendModeValue.get()));
    _sendMode = (Ice::OperationMode)static_cast<int>(PyInt_AS_LONG(sendModeValue.get()));

    //
    // amd
    //
    _amd = amd ? true : false;
    if(_amd)
    {
        _dispatchName = fixIdent(_name) + "_async";
    }
    else
    {
        _dispatchName = fixIdent(_name);
    }

    //
    // metaData
    //
#ifndef NDEBUG
    bool b =
#endif
    tupleToStringSeq(meta, _metaData);
    assert(b);

    Py_ssize_t i, sz;

    //
    // inParams
    //
    convertParams(inParams, _inParams, _sendsClasses);

    //
    // outParams
    //
    convertParams(outParams, _outParams, _returnsClasses);

    //
    // returnType
    //
    if(returnType != Py_None)
    {
        _returnType = new ParamInfo;
        _returnType->type = getType(returnType);
        if(!_returnsClasses)
        {
            _returnsClasses = _returnType->type->usesClasses();
        }
    }

    //
    // exceptions
    //
    sz = PyTuple_GET_SIZE(exceptions);
    for(i = 0; i < sz; ++i)
    {
        _exceptions.push_back(getException(PyTuple_GET_ITEM(exceptions, i)));
    }
}

PyObject*
IcePy::OperationI::invoke(const Ice::ObjectPrx& proxy, PyObject* args, PyObject* pyctx)
{
    Ice::CommunicatorPtr communicator = proxy->ice_getCommunicator();

    //
    // Marshal the input parameters to a byte sequence.
    //
    Ice::ByteSeq params;
    if(!prepareRequest(communicator, args, false, params))
    {
        return 0;
    }

    if(!_deprecateMessage.empty())
    {
        PyErr_Warn(PyExc_DeprecationWarning, const_cast<char*>(_deprecateMessage.c_str()));
        _deprecateMessage.clear(); // Only show the warning once.
    }

    try
    {
        checkTwowayOnly(proxy);

        //
        // Invoke the operation.
        //
        Ice::ByteSeq result;
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
                status = proxy->ice_invoke(_name, _sendMode, params, result, ctx);
            }
            else
            {
                AllowThreads allowThreads; // Release Python's global interpreter lock during remote invocations.
                status = proxy->ice_invoke(_name, _sendMode, params, result);
            }
        }

        //
        // Process the reply.
        //
        if(proxy->ice_isTwoway())
        {
            if(!status)
            {
                //
                // Unmarshal a user exception.
                //
                PyObjectHandle ex = unmarshalException(result, communicator);

                //
                // Set the Python exception.
                //
                setPythonException(ex.get());
                return 0;
            }
            else if(_outParams.size() > 0 || _returnType)
            {
                //
                // Unmarshal the results. If there is more than one value to be returned, then return them
                // in a tuple of the form (result, outParam1, ...). Otherwise just return the value.
                //
                PyObjectHandle results = unmarshalResults(result, communicator);
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
                    Py_INCREF(ret);
                    return ret;
                }
            }
        }
    }
    catch(const AbortMarshaling&)
    {
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

PyObject*
IcePy::OperationI::invokeAsync(const Ice::ObjectPrx& proxy, PyObject* callback, PyObject* args, PyObject* pyctx)
{
    Ice::CommunicatorPtr communicator = proxy->ice_getCommunicator();

    //
    // Marshal the input parameters to a byte sequence.
    //
    Ice::ByteSeq params;
    if(!prepareRequest(communicator, args, true, params))
    {
        return 0;
    }

    if(!_deprecateMessage.empty())
    {
        PyErr_Warn(PyExc_DeprecationWarning, const_cast<char*>(_deprecateMessage.c_str()));
        _deprecateMessage.clear(); // Only show the warning once.
    }

    Ice::AMI_Object_ice_invokePtr cb = new AMICallback(this, communicator, callback);
    try
    {
        checkTwowayOnly(proxy);

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
            proxy->ice_invoke_async(cb, _name, _sendMode, params, ctx);
        }
        else
        {
            AllowThreads allowThreads; // Release Python's global interpreter lock during remote invocations.
            proxy->ice_invoke_async(cb, _name, _sendMode, params);
        }
    }
    catch(const Ice::Exception& ex)
    {
        cb->ice_exception(ex);
    }

    Py_INCREF(Py_None);
    return Py_None;
}

void
IcePy::OperationI::deprecate(const string& msg)
{
    if(!msg.empty())
    {
        _deprecateMessage = msg;
    }
    else
    {
        _deprecateMessage = "operation " + _name + " is deprecated";
    }
}

Ice::OperationMode
IcePy::OperationI::mode() const
{
    return _mode;
}

void
IcePy::OperationI::dispatch(PyObject* servant, const Ice::AMD_Object_ice_invokePtr& cb,
                            const vector<Ice::Byte>& inBytes, const Ice::Current& current)
{
    Ice::CommunicatorPtr communicator = current.adapter->getCommunicator();

    //
    // Unmarshal the in parameters.
    //
    Py_ssize_t count = static_cast<Py_ssize_t>(_inParams.size()) + 1; // Leave room for a trailing Ice::Current object.

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

    if(!_inParams.empty())
    {
        Ice::InputStreamPtr is = Ice::createInputStream(communicator, inBytes);
        try
        {
            Py_ssize_t i = start;
            for(ParamInfoList::iterator p = _inParams.begin(); p != _inParams.end(); ++p, ++i)
            {
                void* closure = reinterpret_cast<void*>(i);
                (*p)->type->unmarshal(is, *p, args.get(), closure, &(*p)->metaData);
            }
            if(_sendsClasses)
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

    if(_amd)
    {
        //
        // Create the callback object and pass it as the first argument.
        //
        AMDCallbackObject* obj = amdCallbackNew(0);
        if(!obj)
        {
            throwPythonException();
        }
        obj->op = new OperationIPtr(this);
        obj->communicator = new Ice::CommunicatorPtr(communicator);
        obj->cb = new Ice::AMD_Object_ice_invokePtr(cb);
        if(PyTuple_SET_ITEM(args.get(), 0, (PyObject*)obj) < 0) // PyTuple_SET_ITEM steals a reference.
        {
            Py_DECREF(obj);
            throwPythonException();
        }
    }

    //
    // Dispatch the operation. Use _dispatchName here, not current.operation.
    //
    PyObjectHandle method = PyObject_GetAttrString(servant, const_cast<char*>(_dispatchName.c_str()));
    if(!method.get())
    {
        ostringstream ostr;
        ostr << "servant for identity " << communicator->identityToString(current.id) << " does not define operation `"
             << _dispatchName << "'";
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
        sendException(cb, ex, communicator);
        return;
    }

    if(!_amd)
    {
        sendResponse(cb, result.get(), communicator);
    }
}

void
IcePy::OperationI::responseAsync(PyObject* callback, bool ok, const vector<Ice::Byte>& results,
                                 const Ice::CommunicatorPtr& communicator)
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
                args = unmarshalResults(results, communicator);
                if(!args.get())
                {
                    assert(PyErr_Occurred());
                    PyErr_Print();
                    return;
                }
            }
            catch(const Ice::Exception& ex)
            {
                PyObjectHandle h = convertException(ex);
                responseAsyncException(callback, h.get());
                return;
            }

            PyObjectHandle method = PyObject_GetAttrString(callback, STRCAST("ice_response"));
            if(!method.get())
            {
                ostringstream ostr;
                ostr << "AMI callback object for operation `" << _name << "' does not define ice_response()";
                string str = ostr.str();
                PyErr_Warn(PyExc_RuntimeWarning, const_cast<char*>(str.c_str()));
            }
            else
            {
                PyObjectHandle tmp = PyObject_Call(method.get(), args.get(), 0);
                if(PyErr_Occurred())
                {
                    PyErr_Print();
                }
            }
        }
        else
        {
            PyObjectHandle ex = unmarshalException(results, communicator);
            responseAsyncException(callback, ex.get());
        }
    }
    catch(const AbortMarshaling&)
    {
        assert(PyErr_Occurred());
        PyErr_Print();
    }
    catch(const Ice::Exception& ex)
    {
        ostringstream ostr;
        ostr << "Exception raised by AMI callback for operation `" << _name << "':" << ex;
        string str = ostr.str();
        PyErr_Warn(PyExc_RuntimeWarning, const_cast<char*>(str.c_str()));
    }
}

void
IcePy::OperationI::responseAsyncException(PyObject* callback, PyObject* ex)
{
    PyObjectHandle method = PyObject_GetAttrString(callback, STRCAST("ice_exception"));
    if(!method.get())
    {
        ostringstream ostr;
        ostr << "AMI callback object for operation `" << _name << "' does not define ice_exception()";
        string str = ostr.str();
        PyErr_Warn(PyExc_RuntimeWarning, const_cast<char*>(str.c_str()));
    }
    else
    {
        PyObjectHandle args = Py_BuildValue(STRCAST("(O)"), ex);
        PyObjectHandle tmp = PyObject_Call(method.get(), args.get(), 0);
        if(PyErr_Occurred())
        {
            PyErr_Print();
        }
    }
}

void
IcePy::OperationI::sendResponse(const Ice::AMD_Object_ice_invokePtr& cb, PyObject* args,
                                const Ice::CommunicatorPtr& communicator)
{
    //
    // Marshal the results. If there is more than one value to be returned, then they must be
    // returned in a tuple of the form (result, outParam1, ...).
    //
    Ice::OutputStreamPtr os = Ice::createOutputStream(communicator);
    try
    {
        Py_ssize_t i = _returnType ? 1 : 0;
        Py_ssize_t numResults = static_cast<Py_ssize_t>(_outParams.size()) + i;
        if(numResults > 1)
        {
            if(!PyTuple_Check(args) || PyTuple_GET_SIZE(args) != numResults)
            {
                ostringstream ostr;
                ostr << "operation `" << fixIdent(_name) << "' should return a tuple of length " << numResults;
                string str = ostr.str();
                PyErr_Warn(PyExc_RuntimeWarning, const_cast<char*>(str.c_str()));
                throw Ice::MarshalException(__FILE__, __LINE__);
            }
        }

        ObjectMap objectMap;

        for(ParamInfoList::iterator p = _outParams.begin(); p != _outParams.end(); ++p, ++i)
        {
            PyObject* arg;
            if(_amd || numResults > 1)
            {
                arg = PyTuple_GET_ITEM(args, i);
            }
            else
            {
                arg = args;
                assert(_outParams.size() == 1);
            }

            if(!(*p)->type->validate(arg))
            {
                // TODO: Provide the parameter name instead?
                ostringstream ostr;
                ostr << "invalid value for out argument " << (i + 1) << " in operation `" << fixIdent(_name)
                     << (_amd ? "_async" : "") << "'";
                string str = ostr.str();
                PyErr_Warn(PyExc_RuntimeWarning, const_cast<char*>(str.c_str()));
                throw Ice::MarshalException(__FILE__, __LINE__);
            }
            (*p)->type->marshal(arg, os, &objectMap, &(*p)->metaData);
        }

        if(_returnType)
        {
            PyObject* res;
            if(_amd || numResults > 1)
            {
                res = PyTuple_GET_ITEM(args, 0);
            }
            else
            {
                assert(_outParams.size() == 0);
                res = args;
            }
            if(!_returnType->type->validate(res))
            {
                ostringstream ostr;
                ostr << "invalid return value for operation `" << fixIdent(_name) << "'";
                string str = ostr.str();
                PyErr_Warn(PyExc_RuntimeWarning, const_cast<char*>(str.c_str()));
                throw Ice::MarshalException(__FILE__, __LINE__);
            }
            _returnType->type->marshal(res, os, &objectMap, &_metaData);
        }

        if(_returnsClasses)
        {
            os->writePendingObjects();
        }

        Ice::ByteSeq outBytes;
        os->finished(outBytes);
        cb->ice_response(true, outBytes);
    }
    catch(const AbortMarshaling&)
    {
        throwPythonException();
    }
}

void
IcePy::OperationI::sendException(const Ice::AMD_Object_ice_invokePtr& cb, PyException& ex,
                                 const Ice::CommunicatorPtr& communicator)
{
    try
    {
        //
        // A servant that calls sys.exit() will raise the SystemExit exception.
        // This is normally caught by the interpreter, causing it to exit.
        // However, we have no way to pass this exception to the interpreter,
        // so we act on it directly.
        //
        if(PyObject_IsInstance(ex.ex.get(), PyExc_SystemExit))
        {
            handleSystemExit(ex.ex.get()); // Does not return.
        }

        PyObject* userExceptionType = lookupType("Ice.UserException");

        if(PyObject_IsInstance(ex.ex.get(), userExceptionType))
        {
            //
            // Get the exception's type and verify that it is legal to be thrown from this operation.
            //
            PyObjectHandle iceType = PyObject_GetAttrString(ex.ex.get(), STRCAST("ice_type"));
            assert(iceType.get());
            ExceptionInfoPtr info = ExceptionInfoPtr::dynamicCast(getException(iceType.get()));
            assert(info);
            if(!validateException(ex.ex.get()))
            {
                ex.raise(); // Raises UnknownUserException.
            }
            else
            {
                Ice::OutputStreamPtr os = Ice::createOutputStream(communicator);
                ObjectMap objectMap;
                info->marshal(ex.ex.get(), os, &objectMap);

                if(info->usesClasses)
                {
                    os->writePendingObjects();
                }

                Ice::ByteSeq bytes;
                os->finished(bytes);
                cb->ice_response(false, bytes);
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

bool
IcePy::OperationI::prepareRequest(const Ice::CommunicatorPtr& communicator, PyObject* args, bool async,
                                  vector<Ice::Byte>& bytes)
{
    assert(PyTuple_Check(args));

    //
    // Validate the number of arguments.
    //
    Py_ssize_t argc = PyTuple_GET_SIZE(args);
    Py_ssize_t paramCount = static_cast<Py_ssize_t>(_inParams.size());
    if(argc != paramCount)
    {
        string fixedName = fixIdent(_name);
        PyErr_Format(PyExc_RuntimeError, STRCAST("%s expects %d in parameters"), fixedName.c_str(),
                     static_cast<int>(paramCount));
        return false;
    }

    if(!_inParams.empty())
    {
        try
        {
            //
            // Marshal the in parameters.
            //
            Ice::OutputStreamPtr os = Ice::createOutputStream(communicator);

            ObjectMap objectMap;
            int i = 0;
            for(ParamInfoList::iterator p = _inParams.begin(); p != _inParams.end(); ++p, ++i)
            {
                PyObject* arg = PyTuple_GET_ITEM(args, i);
                if(!(*p)->type->validate(arg))
                {
                    string opName;
                    if(async)
                    {
                        opName = fixIdent(_name) + "_async";
                    }
                    else
                    {
                        opName = fixIdent(_name);
                    }
                    PyErr_Format(PyExc_ValueError, STRCAST("invalid value for argument %d in operation `%s'"),
                                 async ? i + 2 : i + 1, const_cast<char*>(opName.c_str()));
                    return false;
                }
                (*p)->type->marshal(arg, os, &objectMap, &(*p)->metaData);
            }

            if(_sendsClasses)
            {
                os->writePendingObjects();
            }

            os->finished(bytes);
        }
        catch(const AbortMarshaling&)
        {
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
IcePy::OperationI::unmarshalResults(const vector<Ice::Byte>& bytes, const Ice::CommunicatorPtr& communicator)
{
    Py_ssize_t i = _returnType ? 1 : 0;
    Py_ssize_t numResults = static_cast<Py_ssize_t>(_outParams.size()) + i;

    PyObjectHandle results = PyTuple_New(numResults);
    if(results.get() && numResults > 0)
    {
        //
        // Unmarshal the results. If there is more than one value to be returned, then return them
        // in a tuple of the form (result, outParam1, ...). Otherwise just return the value.
        //
        Ice::InputStreamPtr is = Ice::createInputStream(communicator, bytes);
        for(ParamInfoList::iterator p = _outParams.begin(); p != _outParams.end(); ++p, ++i)
        {
            void* closure = reinterpret_cast<void*>(i);
            (*p)->type->unmarshal(is, *p, results.get(), closure, &(*p)->metaData);
        }

        if(_returnType)
        {
            _returnType->type->unmarshal(is, _returnType, results.get(), 0, &_metaData);
        }

        if(_returnsClasses)
        {
            is->readPendingObjects();
        }
    }

    return results.release();
}

PyObject*
IcePy::OperationI::unmarshalException(const vector<Ice::Byte>& bytes, const Ice::CommunicatorPtr& communicator)
{
    Ice::InputStreamPtr is = Ice::createInputStream(communicator, bytes);

    is->readBool(); // usesClasses

    string id = is->readString();
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
            is->skipSlice();
            id = is->readString();
        }
    }

    //
    // Getting here should be impossible: we can get here only if the
    // sender has marshaled a sequence of type IDs, none of which we
    // have factory for. This means that sender and receiver disagree
    // about the Slice definitions they use.
    //
    throw Ice::UnknownUserException(__FILE__, __LINE__);
}

bool
IcePy::OperationI::validateException(PyObject* ex) const
{
    for(ExceptionInfoList::const_iterator p = _exceptions.begin(); p != _exceptions.end(); ++p)
    {
        if(PyObject_IsInstance(ex, (*p)->pythonType.get()))
        {
            return true;
        }
    }

    return false;
}

void
IcePy::OperationI::checkTwowayOnly(const Ice::ObjectPrx& proxy) const
{
    if((_returnType != 0 || !_outParams.empty()) && !proxy->ice_isTwoway())
    {
        Ice::TwowayOnlyException ex(__FILE__, __LINE__);
        ex.operation = _name;
        throw ex;
    }
}

void
IcePy::OperationI::convertParams(PyObject* p, ParamInfoList& params, bool& usesClasses)
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

    return true;
}

IcePy::OperationPtr
IcePy::getOperation(PyObject* p)
{
    assert(PyObject_IsInstance(p, reinterpret_cast<PyObject*>(&OperationType)) == 1);
    OperationObject* obj = reinterpret_cast<OperationObject*>(p);
    return *obj->op;
}
