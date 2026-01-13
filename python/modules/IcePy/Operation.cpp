// Copyright (c) ZeroC, Inc.

#include "Operation.h"

#include "Communicator.h"
#include "Connection.h"
#include "Current.h"
#include "Future.h"
#include "Ice/Communicator.h"
#include "Ice/Initialize.h"
#include "Ice/LocalExceptions.h"
#include "Ice/Logger.h"
#include "Ice/ObjectAdapter.h"
#include "Ice/Properties.h"
#include "Ice/Proxy.h"
#include "Proxy.h"
#include "Thread.h"
#include "Types.h"
#include "Util.h"
#include "slice2py/PythonUtil.h"

#include <algorithm>

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
        void unmarshaled(PyObject*, PyObject*, void*) override;

        Ice::StringSeq metadata;
        TypeInfoPtr type;
        bool optional;
        int tag;
        Py_ssize_t pos;
    };
    using ParamInfoPtr = shared_ptr<ParamInfo>;
    using ParamInfoList = list<ParamInfoPtr>;

    // Encapsulates attributes of an operation.
    class Operation
    {
    public:
        Operation(
            const char*,
            const char*,
            PyObject*,
            PyObject*,
            PyObject*,
            PyObject*,
            PyObject*,
            PyObject*,
            PyObject*);

        void marshalResult(Ice::OutputStream&, PyObject*);

        void deprecate(const string&);

        string sliceName;
        string mappedName;
        Ice::OperationMode mode;
        std::optional<Ice::FormatType> format;
        Ice::StringSeq metadata;
        ParamInfoList inParams;
        ParamInfoList optionalInParams;
        ParamInfoList outParams;
        ParamInfoList optionalOutParams;
        ParamInfoPtr returnType;
        ExceptionInfoList exceptions;
        bool sendsClasses;
        bool returnsClasses;

    private:
        string _deprecateMessage;

        static void convertParams(PyObject*, ParamInfoList&, Py_ssize_t, bool&);
        static ParamInfoPtr convertParam(PyObject*, Py_ssize_t);
    };
    using OperationPtr = shared_ptr<Operation>;

    // The base class for client-side invocations.
    class Invocation
    {
    public:
        Invocation(const Ice::ObjectPrx&);

        virtual PyObject* invoke(PyObject*, PyObject* = nullptr) = 0;

    protected:
        // Helpers for typed invocations.

        enum MappingType
        {
            SyncMapping,
            AsyncMapping
        };

        bool prepareRequest(
            const OperationPtr&,
            PyObject*,
            MappingType,
            Ice::OutputStream*,
            pair<const byte*, const byte*>&);
        PyObject* unmarshalResults(const OperationPtr&, pair<const byte*, const byte*>);
        PyObject* unmarshalException(const OperationPtr&, pair<const byte*, const byte*>);
        bool validateException(const OperationPtr&, PyObject*) const;
        void checkTwowayOnly(const OperationPtr&, const Ice::ObjectPrx&) const;

        Ice::ObjectPrx _prx;
        Ice::CommunicatorPtr _communicator;
    };
    using InvocationPtr = shared_ptr<Invocation>;

    class SyncTypedInvocation final : public Invocation
    {
    public:
        SyncTypedInvocation(const Ice::ObjectPrx&, OperationPtr);

        PyObject* invoke(PyObject*, PyObject* = nullptr) final;

    private:
        OperationPtr _op;
    };

    class AsyncInvocation : public Invocation
    {
    public:
        AsyncInvocation(const Ice::ObjectPrx&, PyObject*);
        ~AsyncInvocation();

        PyObject* invoke(PyObject*, PyObject* = nullptr) final;

        void response(bool, pair<const byte*, const byte*>);
        void exception(std::exception_ptr);
        void sent(bool);

    protected:
        virtual function<void()> handleInvoke(PyObject*, PyObject*) = 0;
        virtual void handleResponse(PyObject*, bool, pair<const byte*, const byte*>) = 0;

        PyObject* _pyProxy;
        bool _twoway;
        bool _sent{false};
        bool _sentSynchronously{false};
        bool _done{false};
        PyObject* _future{nullptr};
        bool _ok{false};
        vector<byte> _results;
        PyObject* _exception{nullptr};
    };
    using AsyncInvocationPtr = shared_ptr<AsyncInvocation>;

    class AsyncTypedInvocation final : public AsyncInvocation, public enable_shared_from_this<AsyncTypedInvocation>
    {
    public:
        AsyncTypedInvocation(const Ice::ObjectPrx&, PyObject*, OperationPtr);

    protected:
        function<void()> handleInvoke(PyObject*, PyObject*) final;
        void handleResponse(PyObject*, bool, pair<const byte*, const byte*>) final;

    private:
        OperationPtr _op;
    };

    class SyncBlobjectInvocation final : public Invocation
    {
    public:
        SyncBlobjectInvocation(const Ice::ObjectPrx&);

        PyObject* invoke(PyObject*, PyObject* = nullptr) final;
    };

    class AsyncBlobjectInvocation final : public AsyncInvocation,
                                          public enable_shared_from_this<AsyncBlobjectInvocation>
    {
    public:
        AsyncBlobjectInvocation(const Ice::ObjectPrx&, PyObject*);

    protected:
        function<void()> handleInvoke(PyObject*, PyObject*) final;
        void handleResponse(PyObject*, bool, pair<const byte*, const byte*>) final;

        string _op;
    };

    // The base class for server-side upcalls.
    class Upcall : public enable_shared_from_this<Upcall>
    {
    public:
        virtual void dispatch(PyObject*, pair<const byte*, const byte*>, const Ice::Current&) = 0;
        virtual void response(PyObject*) = 0;
        virtual void exception(PyException&) = 0;

    protected:
        void dispatchImpl(PyObject*, const string&, PyObject*, const Ice::Current&);
    };
    using UpcallPtr = shared_ptr<Upcall>;

    // TypedUpcall uses the information in the given Operation to validate, marshal, and unmarshal parameters and
    // exceptions.
    class TypedUpcall final : public Upcall
    {
    public:
        TypedUpcall(
            OperationPtr,
            function<void(bool, pair<const byte*, const byte*>)>,
            function<void(std::exception_ptr)>,
            Ice::CommunicatorPtr);

        void dispatch(PyObject*, pair<const byte*, const byte*>, const Ice::Current&) final;
        void response(PyObject*) final;
        void exception(PyException&) final;

    private:
        OperationPtr _op;

        function<void(bool, pair<const byte*, const byte*>)> _response;
        function<void(std::exception_ptr)> _error;

        Ice::CommunicatorPtr _communicator;
        Ice::EncodingVersion _encoding;
    };
    using TypedUpcallPtr = shared_ptr<TypedUpcall>;

    //
    // Upcall for blobject servants.
    //
    class BlobjectUpcall final : public Upcall
    {
    public:
        BlobjectUpcall(function<void(bool, pair<const byte*, const byte*>)>, function<void(std::exception_ptr)>);

        void dispatch(PyObject*, pair<const byte*, const byte*>, const Ice::Current&) final;
        void response(PyObject*) final;
        void exception(PyException&) final;

    private:
        function<void(bool, pair<const byte*, const byte*>)> _response;
        function<void(std::exception_ptr)> _error;
    };

    // TypedServantWrapper uses the information in Operation to validate, marshal, and unmarshal parameters and
    // exceptions.
    class TypedServantWrapper final : public ServantWrapper
    {
    public:
        TypedServantWrapper(PyObject*);

        void ice_invokeAsync(
            pair<const byte*, const byte*> inEncaps,
            function<void(bool, pair<const byte*, const byte*>)> response,
            function<void(std::exception_ptr)> error,
            const Ice::Current& current) final;

    private:
        using OperationMap = map<string, OperationPtr>;
        OperationMap _operationMap;
        OperationMap::iterator _lastOp;
    };

    // Encapsulates a blobject servant.
    class BlobjectServantWrapper final : public ServantWrapper
    {
    public:
        BlobjectServantWrapper(PyObject*);

        void ice_invokeAsync(
            pair<const byte*, const byte*> inEncaps,
            function<void(bool, pair<const byte*, const byte*>)> response,
            function<void(std::exception_ptr)> error,
            const Ice::Current& current) final;
    };

    struct OperationObject
    {
        PyObject_HEAD OperationPtr* op;
    };

    struct DispatchCallbackObject
    {
        PyObject_HEAD UpcallPtr* upcall;
    };

    struct AsyncInvocationContextObject
    {
        PyObject_HEAD function<void()>* cancel;
        Ice::CommunicatorPtr* communicator;
    };

    extern PyTypeObject OperationType;
}

namespace
{
    void handleException()
    {
        assert(PyErr_Occurred());

        PyException ex; // Retrieve it before another Python API call clears it.

        // A callback that calls sys.exit() will raise the SystemExit exception.
        //
        // This is normally caught by the interpreter, causing it to exit. However, we have no way to pass this
        // exception to the interpreter, so we act on it directly.
        ex.checkSystemExit();

        ex.raise();
    }
}

extern "C" OperationObject*
operationNew(PyTypeObject* type, PyObject* /*args*/, PyObject* /*kwds*/)
{
    auto* self = reinterpret_cast<OperationObject*>(type->tp_alloc(type, 0));
    if (!self)
    {
        return nullptr;
    }
    self->op = nullptr;
    return self;
}

extern "C" int
operationInit(OperationObject* self, PyObject* args, PyObject* /*kwds*/)
{
    char* sliceName;
    char* mappedName;
    // We use `Ice.OperationMode.OperationMode` to import directly from Ice/OperationMode.py and
    // avoid circular dependencies.
    PyObject* modeType = lookupType("Ice.OperationMode.OperationMode");
    assert(modeType);
    PyObject* mode;
    PyObject* format;
    PyObject* metadata;
    PyObject* inParams;
    PyObject* outParams;
    PyObject* returnType;
    PyObject* exceptions;
    if (!PyArg_ParseTuple(
            args,
            "ssO!OO!O!O!OO!",
            &sliceName,
            &mappedName,
            modeType,
            &mode,
            &format,
            &PyTuple_Type,
            &metadata,
            &PyTuple_Type,
            &inParams,
            &PyTuple_Type,
            &outParams,
            &returnType,
            &PyTuple_Type,
            &exceptions))
    {
        return -1;
    }

    self->op = new OperationPtr(
        make_shared<
            Operation>(sliceName, mappedName, mode, format, metadata, inParams, outParams, returnType, exceptions));
    return 0;
}

extern "C" void
operationDealloc(OperationObject* self)
{
    delete self->op;
    Py_TYPE(self)->tp_free(reinterpret_cast<PyObject*>(self));
}

extern "C" PyObject*
operationInvoke(OperationObject* self, PyObject* args)
{
    PyObject* pyProxy{nullptr};
    PyObject* opArgs{nullptr};
    if (!PyArg_ParseTuple(args, "O!O!", &ProxyType, &pyProxy, &PyTuple_Type, &opArgs))
    {
        return nullptr;
    }

    Ice::ObjectPrx prx{getProxy(pyProxy)};
    assert(self->op);

    auto i = make_shared<SyncTypedInvocation>(prx, *self->op);
    return i->invoke(opArgs);
}

extern "C" PyObject*
operationInvokeAsync(OperationObject* self, PyObject* args)
{
    PyObject* proxy{nullptr};
    PyObject* opArgs{nullptr};
    if (!PyArg_ParseTuple(args, "O!O!", &ProxyType, &proxy, &PyTuple_Type, &opArgs))
    {
        return nullptr;
    }

    Ice::ObjectPrx prx{getProxy(proxy)};
    InvocationPtr i = make_shared<AsyncTypedInvocation>(prx, proxy, *self->op);
    return i->invoke(opArgs);
}

extern "C" PyObject*
operationDeprecate(OperationObject* self, PyObject* args)
{
    char* msg{nullptr};
    if (!PyArg_ParseTuple(args, "s", &msg))
    {
        return nullptr;
    }

    assert(self->op);
    (*self->op)->deprecate(msg);

    return Py_None;
}

//
// DispatchCallbackObject operations
//

extern "C" DispatchCallbackObject*
dispatchCallbackNew(PyTypeObject* type, PyObject* /*args*/, PyObject* /*kwds*/)
{
    auto* self = reinterpret_cast<DispatchCallbackObject*>(type->tp_alloc(type, 0));
    if (!self)
    {
        return nullptr;
    }
    self->upcall = nullptr;
    return self;
}

extern "C" void
dispatchCallbackDealloc(DispatchCallbackObject* self)
{
    delete self->upcall;
    Py_TYPE(self)->tp_free(reinterpret_cast<PyObject*>(self));
}

extern "C" PyObject*
dispatchCallbackResponse(DispatchCallbackObject* self, PyObject* args)
{
    PyObject* result{nullptr};
    if (!PyArg_ParseTuple(args, "O", &result))
    {
        return nullptr;
    }

    try
    {
        assert(self->upcall);
        (*self->upcall)->response(result);
    }
    catch (...)
    {
        //
        // No exceptions should propagate to Python.
        //
        assert(false);
    }

    return Py_None;
}

extern "C" PyObject*
dispatchCallbackException(DispatchCallbackObject* self, PyObject* args)
{
    PyObject* ex{nullptr};
    if (!PyArg_ParseTuple(args, "O", &ex))
    {
        return nullptr;
    }

    try
    {
        assert(self->upcall);
        PyException pyex(ex);
        (*self->upcall)->exception(pyex);
    }
    catch (...)
    {
        //
        // No exceptions should propagate to Python.
        //
        assert(false);
    }

    return Py_None;
}

//
// AsyncInvocationContext operations
//

extern "C" AsyncInvocationContextObject*
asyncInvocationContextNew(PyTypeObject* type, PyObject* /*args*/, PyObject* /*kwds*/)
{
    auto self = reinterpret_cast<AsyncInvocationContextObject*>(type->tp_alloc(type, 0));
    if (!self)
    {
        return nullptr;
    }
    self->cancel = nullptr;
    self->communicator = nullptr;
    return self;
}

extern "C" void
asyncInvocationContextDealloc(AsyncInvocationContextObject* self)
{
    delete self->cancel;
    delete self->communicator;
    Py_TYPE(self)->tp_free(reinterpret_cast<PyObject*>(self));
}

extern "C" PyObject*
asyncInvocationContextCancel(AsyncInvocationContextObject* self, PyObject* /*args*/)
{
    try
    {
        (*self->cancel)();
    }
    catch (...)
    {
        assert(false);
    }

    return Py_None;
}

//
// ParamInfo implementation.
//
void
IcePy::ParamInfo::unmarshaled(PyObject* val, PyObject* target, void* closure)
{
    assert(PyTuple_Check(target));
    auto i = reinterpret_cast<Py_ssize_t>(closure);
    PyTuple_SET_ITEM(target, i, Py_NewRef(val)); // PyTuple_SET_ITEM steals a reference.
}

//
// Operation implementation.
//
IcePy::Operation::Operation(
    const char* name,
    const char* mapped,
    PyObject* m,
    PyObject* fmt,
    PyObject* meta,
    PyObject* in,
    PyObject* out,
    PyObject* ret,
    PyObject* ex)
{
    sliceName = name;
    mappedName = mapped;

    //
    // mode
    //
    PyObjectHandle modeValue{getAttr(m, "value", true)};
    mode = static_cast<Ice::OperationMode>(PyLong_AsLong(modeValue.get()));
    assert(!PyErr_Occurred());

    //
    // format
    //
    if (fmt == Py_None)
    {
        format = std::nullopt;
    }
    else
    {
        PyObjectHandle formatValue{getAttr(fmt, "value", true)};
        format = (Ice::FormatType) static_cast<int>(PyLong_AsLong(formatValue.get()));
        assert(!PyErr_Occurred());
    }

    //
    // metadata
    //
    assert(PyTuple_Check(meta));
    [[maybe_unused]] bool b = tupleToStringSeq(meta, metadata);
    assert(b);

    //
    // returnType
    //
    returnsClasses = false;
    if (ret != Py_None)
    {
        returnType = convertParam(ret, 0);
        returnsClasses = returnType->type->usesClasses();
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
        static bool compare(const ParamInfoPtr& lhs, const ParamInfoPtr& rhs) { return lhs->tag < rhs->tag; }

        static bool isRequired(const ParamInfoPtr& i) { return !i->optional; }
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
    if (returnType && returnType->optional)
    {
        optionalOutParams.push_back(returnType);
    }
    optionalOutParams.sort(SortFn::compare);

    //
    // exceptions
    //
    Py_ssize_t sz = PyTuple_GET_SIZE(ex);
    for (Py_ssize_t i = 0; i < sz; ++i)
    {
        exceptions.push_back(getException(PyTuple_GET_ITEM(ex, i)));
    }
}

void
Operation::marshalResult(Ice::OutputStream& os, PyObject* result)
{
    // Marshal the results. If there is more than one value to be returned, then they must be returned in a tuple of
    // the form (result, outParam1, ...).

    auto numResults = static_cast<Py_ssize_t>(outParams.size());
    if (returnType)
    {
        numResults++;
    }

    if (numResults > 1 && (!PyTuple_Check(result) || PyTuple_GET_SIZE(result) != numResults))
    {
        ostringstream ostr;
        ostr << "cannot marshal result: operation '" << mappedName << "' should return a tuple of length "
             << numResults;
        throw Ice::MarshalException(__FILE__, __LINE__, ostr.str());
    }

    // Normalize the result value. When there are multiple result values, result is already a tuple.
    // Otherwise, we create a tuple to make the code a little simpler.
    PyObjectHandle resultTuple;
    if (numResults > 1)
    {
        resultTuple = PyObjectHandle{Py_NewRef(result)};
    }
    else
    {
        resultTuple = PyObjectHandle{PyTuple_New(1)};
        if (!resultTuple.get())
        {
            throw AbortMarshaling();
        }
        PyTuple_SET_ITEM(resultTuple.get(), 0, Py_NewRef(result));
    }

    ObjectMap objectMap;
    ParamInfoList::iterator p;

    //
    // Validate the results.
    //
    for (const auto& info : outParams)
    {
        PyObject* arg = PyTuple_GET_ITEM(resultTuple.get(), info->pos);
        if ((!info->optional || arg != Py_None) && !info->type->validate(arg))
        {
            // TODO: Provide the parameter name instead?
            ostringstream ostr;
            ostr << "cannot marshal result: invalid value for out argument " << (info->pos + 1) << " in operation '"
                 << mappedName << "'";
            if (PyErr_Occurred())
            {
                try
                {
                    throwPythonException();
                }
                catch (const Ice::LocalException& ex)
                {
                    ostr << "':\n" << ex.what();
                }
            }
            throw Ice::MarshalException(__FILE__, __LINE__, ostr.str());
        }
    }

    if (returnType)
    {
        PyObject* res = PyTuple_GET_ITEM(resultTuple.get(), 0);
        if ((!returnType->optional || res != Py_None) && !returnType->type->validate(res))
        {
            ostringstream ostr;
            ostr << "cannot marshal result: invalid return value for operation '" << mappedName << "'";
            if (PyErr_Occurred())
            {
                try
                {
                    throwPythonException();
                }
                catch (const Ice::LocalException& ex)
                {
                    ostr << ":\n" << ex.what();
                }
            }
            throw Ice::MarshalException(__FILE__, __LINE__, ostr.str());
        }
    }

    // Marshal the required out parameters.
    for (const auto& info : outParams)
    {
        if (!info->optional)
        {
            PyObject* arg = PyTuple_GET_ITEM(resultTuple.get(), info->pos);
            info->type->marshal(arg, &os, &objectMap, false, &info->metadata);
        }
    }

    // Marshal the required return value, if any.
    if (returnType && !returnType->optional)
    {
        PyObject* res = PyTuple_GET_ITEM(resultTuple.get(), 0);
        returnType->type->marshal(res, &os, &objectMap, false, &metadata);
    }

    // Marshal the optional results.
    for (const auto& info : optionalOutParams)
    {
        PyObject* arg = PyTuple_GET_ITEM(resultTuple.get(), info->pos);
        if (arg != Py_None && os.writeOptional(info->tag, info->type->optionalFormat()))
        {
            info->type->marshal(arg, &os, &objectMap, true, &info->metadata);
        }
    }

    if (returnsClasses)
    {
        os.writePendingValues();
    }
}

void
IcePy::Operation::deprecate(const string& msg)
{
    if (!msg.empty())
    {
        _deprecateMessage = msg;
    }
    else
    {
        _deprecateMessage = "operation " + mappedName + " is deprecated";
    }
}

void
IcePy::Operation::convertParams(PyObject* p, ParamInfoList& params, Py_ssize_t posOffset, bool& usesClasses)
{
    int sz = static_cast<int>(PyTuple_GET_SIZE(p));
    for (Py_ssize_t i = 0; i < sz; ++i)
    {
        PyObject* item{PyTuple_GET_ITEM(p, i)};
        ParamInfoPtr param = convertParam(item, i + posOffset);
        params.push_back(param);
        if (!param->optional && !usesClasses)
        {
            usesClasses = param->type->usesClasses();
        }
    }
}

ParamInfoPtr
IcePy::Operation::convertParam(PyObject* p, Py_ssize_t pos)
{
    assert(PyTuple_Check(p));
    assert(PyTuple_GET_SIZE(p) == 4);

    auto param = make_shared<ParamInfo>();

    //
    // metadata
    //
    PyObject* meta = PyTuple_GET_ITEM(p, 0);
    assert(PyTuple_Check(meta));
    [[maybe_unused]] bool b = tupleToStringSeq(meta, param->metadata);
    assert(b);

    //
    // type
    //
    PyObject* type = PyTuple_GET_ITEM(p, 1);
    if (type != Py_None)
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

static PyMethodDef OperationMethods[] = {
    {"invoke",
     reinterpret_cast<PyCFunction>(operationInvoke),
     METH_VARARGS,
     PyDoc_STR("invoke(proxy: ObjectPrx, args: tuple) -> Any")},
    {"invokeAsync",
     reinterpret_cast<PyCFunction>(operationInvokeAsync),
     METH_VARARGS,
     PyDoc_STR("invokeAsync(proxy: ObjectPrx, args: tuple) -> Awaitable[Any]")},
    {"deprecate", reinterpret_cast<PyCFunction>(operationDeprecate), METH_VARARGS, PyDoc_STR("deprecate(reason: str)")},
    {} /* sentinel */
};

static PyMethodDef DispatchCallbackMethods[] = {
    {"response",
     reinterpret_cast<PyCFunction>(dispatchCallbackResponse),
     METH_VARARGS,
     PyDoc_STR("response(*args: tuple) -> None")},
    {"exception",
     reinterpret_cast<PyCFunction>(dispatchCallbackException),
     METH_VARARGS,
     PyDoc_STR("exception(exception: BaseException) -> None")},
    {} /* sentinel */
};

static PyMethodDef AsyncInvocationContextMethods[] = {
    {"cancel", reinterpret_cast<PyCFunction>(asyncInvocationContextCancel), METH_NOARGS, PyDoc_STR("cancel() -> None")},
    {} /* sentinel */
};

namespace IcePy
{
    // clang-format off
    PyTypeObject OperationType = {
        .ob_base = PyVarObject_HEAD_INIT(nullptr, 0)
        .tp_name = "IcePy.Operation",
        .tp_basicsize = sizeof(OperationObject),
        .tp_dealloc = reinterpret_cast<destructor>(operationDealloc),
        .tp_flags = Py_TPFLAGS_DEFAULT,
        .tp_doc = PyDoc_STR("IcePy.Operation"),
        .tp_methods = OperationMethods,
        .tp_init = reinterpret_cast<initproc>(operationInit),
        .tp_new = reinterpret_cast<newfunc>(operationNew),
    };

    PyTypeObject DispatchCallbackType = {
        .ob_base = PyVarObject_HEAD_INIT(nullptr, 0)
        .tp_name = "IcePy.DispatchCallback",
        .tp_basicsize = sizeof(DispatchCallbackObject),
        .tp_dealloc = reinterpret_cast<destructor>(dispatchCallbackDealloc),
        .tp_flags = Py_TPFLAGS_DEFAULT,
        .tp_doc = PyDoc_STR("IcePy.DispatchCallback"),
        .tp_methods = DispatchCallbackMethods,
        .tp_new = reinterpret_cast<newfunc>(dispatchCallbackNew),
    };

    PyTypeObject AsyncInvocationContextType = {
        .ob_base = PyVarObject_HEAD_INIT(nullptr, 0)
        .tp_name = "IcePy.AsyncInvocationContext",
        .tp_basicsize = sizeof(AsyncInvocationContextObject),
        .tp_dealloc = reinterpret_cast<destructor>(asyncInvocationContextDealloc),
        .tp_flags = Py_TPFLAGS_DEFAULT,
        .tp_doc = PyDoc_STR("IcePy.AsyncInvocationContext"),
        .tp_methods = AsyncInvocationContextMethods,
        .tp_new = reinterpret_cast<newfunc>(asyncInvocationContextNew),
    };
    // clang-format on
}

bool
IcePy::initOperation(PyObject* module)
{
    if (PyType_Ready(&OperationType) < 0)
    {
        return false;
    }

    if (PyModule_AddObject(module, "Operation", reinterpret_cast<PyObject*>(&OperationType)) < 0)
    {
        return false;
    }

    if (PyType_Ready(&DispatchCallbackType) < 0)
    {
        return false;
    }

    if (PyModule_AddObject(module, "DispatchCallback", reinterpret_cast<PyObject*>(&DispatchCallbackType)) < 0)
    {
        return false;
    }

    if (PyType_Ready(&AsyncInvocationContextType) < 0)
    {
        return false;
    }

    if (PyModule_AddObject(module, "AsyncInvocationContext", reinterpret_cast<PyObject*>(&AsyncInvocationContextType)) <
        0)
    {
        return false;
    }

    return true;
}

//
// Invocation
//
IcePy::Invocation::Invocation(const Ice::ObjectPrx& prx) : _prx(prx), _communicator(prx->ice_getCommunicator()) {}

bool
IcePy::Invocation::prepareRequest(
    const OperationPtr& op,
    PyObject* args,
    MappingType mapping,
    Ice::OutputStream* os,
    pair<const byte*, const byte*>& params)
{
    assert(PyTuple_Check(args));
    params.first = params.second = static_cast<const byte*>(nullptr);

    //
    // Validate the number of arguments.
    //
    Py_ssize_t argc{PyTuple_GET_SIZE(args)};
    Py_ssize_t paramCount{static_cast<Py_ssize_t>(op->inParams.size())};
    if (argc != paramCount)
    {
        string opName = op->mappedName;
        if (mapping == AsyncMapping)
        {
            opName += "Async";
        }
        PyErr_Format(PyExc_RuntimeError, "%s expects %d in parameters", opName.c_str(), static_cast<int>(paramCount));
        return false;
    }

    if (!op->inParams.empty())
    {
        try
        {
            //
            // Marshal the in parameters.
            //
            os->startEncapsulation(_prx->ice_getEncodingVersion(), op->format);

            ObjectMap objectMap;
            //
            // Validate the supplied arguments.
            //
            for (const auto& info : op->inParams)
            {
                PyObject* arg = PyTuple_GET_ITEM(args, info->pos);
                if ((!info->optional || arg != Py_None) && !info->type->validate(arg))
                {
                    string opName = op->mappedName;
                    if (mapping == AsyncMapping)
                    {
                        opName += "Async";
                    }
                    PyErr_Format(
                        PyExc_ValueError,
                        "invalid value for argument %" PY_FORMAT_SIZE_T "d in operation '%s'",
                        info->pos + 1,
                        const_cast<char*>(opName.c_str()));
                    return false;
                }
            }

            //
            // Marshal the required parameters.
            //
            for (const auto& info : op->inParams)
            {
                if (!info->optional)
                {
                    PyObject* arg = PyTuple_GET_ITEM(args, info->pos);
                    info->type->marshal(arg, os, &objectMap, false, &info->metadata);
                }
            }

            //
            // Marshal the optional parameters.
            //
            for (const auto& info : op->optionalInParams)
            {
                PyObject* arg = PyTuple_GET_ITEM(args, info->pos);
                if (arg != Py_None && os->writeOptional(info->tag, info->type->optionalFormat()))
                {
                    info->type->marshal(arg, os, &objectMap, true, &info->metadata);
                }
            }

            if (op->sendsClasses)
            {
                os->writePendingValues();
            }

            os->endEncapsulation();
            params = os->finished();
        }
        catch (const AbortMarshaling&)
        {
            assert(PyErr_Occurred());
            return false;
        }
        catch (...)
        {
            setPythonException(current_exception());
            return false;
        }
    }

    return true;
}

PyObject*
IcePy::Invocation::unmarshalResults(const OperationPtr& op, pair<const byte*, const byte*> bytes)
{
    Py_ssize_t numResults{static_cast<Py_ssize_t>(op->outParams.size())};
    if (op->returnType)
    {
        numResults++;
    }

    PyObjectHandle results{PyTuple_New(numResults)};
    if (results.get() && numResults > 0)
    {
        Ice::InputStream is{_communicator, bytes, getSliceLoader(_communicator)};

        //
        // Store a pointer to a local StreamUtil object as the stream's closure.
        // This is necessary to support object unmarshaling (see ValueReader).
        //
        StreamUtil util;
        assert(!is.getClosure());
        is.setClosure(&util);

        is.startEncapsulation();

        //
        // Unmarshal the required out parameters.
        //
        for (const auto& info : op->outParams)
        {
            if (!info->optional)
            {
                void* closure = reinterpret_cast<void*>(info->pos); // NOLINT(performance-no-int-to-ptr)
                info->type->unmarshal(&is, info, results.get(), closure, false, &info->metadata);
            }
        }

        //
        // Unmarshal the required return value, if any.
        //
        if (op->returnType && !op->returnType->optional)
        {
            assert(op->returnType->pos == 0);
            void* closure = reinterpret_cast<void*>(op->returnType->pos); // NOLINT(performance-no-int-to-ptr)
            op->returnType->type->unmarshal(&is, op->returnType, results.get(), closure, false, &op->metadata);
        }

        //
        // Unmarshal the optional results. This includes an optional return value.
        //
        for (const auto& info : op->optionalOutParams)
        {
            if (is.readOptional(info->tag, info->type->optionalFormat()))
            {
                void* closure = reinterpret_cast<void*>(info->pos); // NOLINT(performance-no-int-to-ptr)
                info->type->unmarshal(&is, info, results.get(), closure, true, &info->metadata);
            }
            else
            {
                PyTuple_SET_ITEM(results.get(), info->pos, Py_None);
            }
        }

        if (op->returnsClasses)
        {
            is.readPendingValues();
        }

        is.endEncapsulation();

        util.updateSlicedData();
    }

    return results.release();
}

PyObject*
IcePy::Invocation::unmarshalException(const OperationPtr& op, pair<const byte*, const byte*> bytes)
{
    Ice::InputStream is{_communicator, bytes, getSliceLoader(_communicator)};

    //
    // Store a pointer to a local StreamUtil object as the stream's closure.
    // This is necessary to support object unmarshaling (see ValueReader).
    //
    StreamUtil util;
    assert(!is.getClosure());
    is.setClosure(&util);

    is.startEncapsulation();

    try
    {
        is.throwException();
    }
    catch (const ExceptionReader& r)
    {
        is.endEncapsulation();

        PyObject* ex{r.getException()}; // Borrowed reference.

        if (validateException(op, ex))
        {
            return Py_NewRef(ex);
        }
        else
        {
            return convertException(
                make_exception_ptr(Ice::UnknownUserException::fromTypeId(__FILE__, __LINE__, r.ice_id())));
        }
    }
    catch (...)
    {
        return convertException(std::current_exception());
    }
    assert(false);
    // Never reached.
    return Py_None;
}

bool
IcePy::Invocation::validateException(const OperationPtr& op, PyObject* ex) const
{
    for (const auto& info : op->exceptions)
    {
        if (PyObject_IsInstance(ex, info->pythonType))
        {
            return true;
        }
    }

    return false;
}

void
IcePy::Invocation::checkTwowayOnly(const OperationPtr& op, const Ice::ObjectPrx& proxy) const
{
    if ((op->returnType || !op->outParams.empty() || !op->exceptions.empty()) && !proxy->ice_isTwoway())
    {
        throw Ice::TwowayOnlyException{__FILE__, __LINE__, op->sliceName};
    }
}

//
// SyncTypedInvocation
//
IcePy::SyncTypedInvocation::SyncTypedInvocation(const Ice::ObjectPrx& prx, OperationPtr op)
    : Invocation(prx),
      _op(std::move(op))
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
    pair<const byte*, const byte*> params;
    if (!prepareRequest(_op, pyparams, SyncMapping, &os, params))
    {
        return nullptr;
    }

    try
    {
        checkTwowayOnly(_op, _prx);

        //
        // Invoke the operation.
        //
        vector<byte> result;
        bool status;
        Ice::Context ctx;
        if (pyctx != Py_None)
        {
            if (!PyDict_Check(pyctx))
            {
                PyErr_Format(PyExc_ValueError, "context argument must be None or a dictionary");
                return nullptr;
            }

            if (!dictionaryToContext(pyctx, ctx))
            {
                return nullptr;
            }
        }

        {
            AllowThreads allowThreads; // Release Python's global interpreter lock during remote invocations.
            status = _prx->ice_invoke(
                _op->sliceName,
                _op->mode,
                params,
                result,
                pyctx == Py_None ? Ice::noExplicitContext : ctx);
        }

        //
        // Process the reply.
        //
        if (_prx->ice_isTwoway())
        {
            pair<const byte*, const byte*> rb{0, 0};
            if (!result.empty())
            {
                rb.first = &result[0];
                rb.second = &result[0] + result.size();
            }

            if (!status)
            {
                //
                // Unmarshal a user exception.
                //
                PyObjectHandle ex{unmarshalException(_op, rb)};

                //
                // Set the Python exception.
                //
                setPythonException(ex.get());
                return nullptr;
            }
            else if (_op->outParams.size() > 0 || _op->returnType)
            {
                //
                // Unmarshal the results. If there is more than one value to be returned, then return them
                // in a tuple of the form (result, outParam1, ...). Otherwise just return the value.
                //
                PyObjectHandle results{unmarshalResults(_op, rb)};
                if (!results.get())
                {
                    return nullptr;
                }

                if (PyTuple_GET_SIZE(results.get()) > 1)
                {
                    return results.release();
                }
                else
                {
                    PyObject* ret = PyTuple_GET_ITEM(results.get(), 0);
                    if (!ret)
                    {
                        return nullptr;
                    }
                    else
                    {
                        return Py_NewRef(ret);
                    }
                }
            }
        }
    }
    catch (const AbortMarshaling&)
    {
        assert(PyErr_Occurred());
        return nullptr;
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    return Py_None;
}

//
// AsyncInvocation
//
IcePy::AsyncInvocation::AsyncInvocation(const Ice::ObjectPrx& prx, PyObject* pyProxy)
    : Invocation(prx),
      _pyProxy(Py_NewRef(pyProxy)),
      _twoway(prx->ice_isTwoway())
{
}

IcePy::AsyncInvocation::~AsyncInvocation()
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    Py_DECREF(_pyProxy);
    Py_XDECREF(_future);
    Py_XDECREF(_exception);
}

PyObject*
IcePy::AsyncInvocation::invoke(PyObject* args, PyObject* kwds)
{
    // Called from Python code, so the GIL is already acquired.

    function<void()> cancel;

    try
    {
        cancel = handleInvoke(args, kwds);
    }
    catch (const Ice::CommunicatorDestroyedException&)
    {
        //
        // CommunicatorDestroyedException can propagate directly.
        //
        setPythonException(current_exception());
        return nullptr;
    }
    catch (const Ice::TwowayOnlyException&)
    {
        //
        // TwowayOnlyException can propagate directly.
        //
        setPythonException(current_exception());
        return nullptr;
    }
    catch (...)
    {
        //
        // No other exceptions should be raised by invoke.
        //
        assert(false);
    }

    if (PyErr_Occurred())
    {
        return nullptr;
    }
    assert(cancel);

    // Any time we call into interpreted Python code there's a chance that another thread will be allowed to run!

    PyObjectHandle asyncInvocationContextObj{createAsyncInvocationContext(std::move(cancel), _communicator)};
    if (!asyncInvocationContextObj.get())
    {
        return nullptr;
    }

    PyObjectHandle future{createFuture(asyncInvocationContextObj.get())}; // Calls into Python code.
    if (!future.get())
    {
        return nullptr;
    }

    //
    // Check if any callbacks have been invoked already.
    //
    if (!_prx->ice_isBatchOneway() && !_prx->ice_isBatchDatagram())
    {
        if (_sent)
        {
            PyObjectHandle tmp{callMethod(future.get(), "set_sent", _sentSynchronously ? Py_True : Py_False)};
            if (PyErr_Occurred())
            {
                return nullptr;
            }

            if (!_twoway)
            {
                //
                // For a oneway/datagram invocation, we consider it complete when sent.
                //
                tmp = PyObjectHandle{callMethod(future.get(), "set_result", Py_None)};
                if (PyErr_Occurred())
                {
                    return nullptr;
                }
            }
        }

        if (_done)
        {
            if (_exception)
            {
                PyObjectHandle tmp{callMethod(future.get(), "set_exception", _exception)};
                if (PyErr_Occurred())
                {
                    return nullptr;
                }
            }
            else
            {
                //
                // Delegate to the subclass.
                //
                pair<const byte*, const byte*> p(&_results[0], &_results[0] + _results.size());
                handleResponse(future.get(), _ok, p);
                if (PyErr_Occurred())
                {
                    return nullptr;
                }
            }
        }
        _future = Py_NewRef(future.release());
        return wrapFuture(_communicator, _future);
    }
    else
    {
        PyObjectHandle tmp{callMethod(future.get(), "set_result", Py_None)};
        if (PyErr_Occurred())
        {
            return nullptr;
        }
        return wrapFuture(_communicator, future.get());
    }
}

void
IcePy::AsyncInvocation::response(bool ok, pair<const byte*, const byte*> results)
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    if (!_future)
    {
        //
        // The future hasn't been created yet, which means invoke() is still running. Save the results for later.
        //
        _ok = ok;
        vector<byte> v(results.first, results.second);
        _results.swap(v);
        _done = true;
        return;
    }

    PyObjectHandle future{_future}; // Steals a reference.

    if (_sent)
    {
        _future = nullptr; // Break cyclic dependency.
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
    if (PyErr_Occurred())
    {
        handleException();
    }
}

void
IcePy::AsyncInvocation::exception(std::exception_ptr ex)
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    PyObjectHandle exh{convertException(ex)}; // NOTE: This can release the GIL
    if (!_future)
    {
        //
        // The future hasn't been created yet, which means invoke() is still running. Save the exception for later.
        //
        _exception = exh.release();
        _done = true;
        return;
    }

    PyObjectHandle future{_future}; // Steals a reference.

    _future = nullptr; // Break cyclic dependency.
    _done = true;

    assert(exh.get());
    PyObjectHandle tmp{callMethod(future.get(), "set_exception", exh.get())};
    if (PyErr_Occurred())
    {
        handleException();
    }
}

void
IcePy::AsyncInvocation::sent(bool sentSynchronously)
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    if (!_future)
    {
        //
        // The future hasn't been created yet, which means invoke() is still running.
        //
        _sent = true;
        _sentSynchronously = sentSynchronously;
        return;
    }

    PyObjectHandle future{_future}; // Steals a reference.

    if (_done || !_twoway)
    {
        _future = nullptr; // Break cyclic dependency.
    }
    else
    {
        _sent = true;

        //
        // The reference to _future will be released in response() or exception().
        //
        Py_INCREF(_future);
    }

    PyObjectHandle tmp{callMethod(future.get(), "set_sent", sentSynchronously ? Py_True : Py_False)};
    if (PyErr_Occurred())
    {
        handleException();
    }

    if (!_twoway)
    {
        //
        // For a oneway/datagram invocation, we consider it complete when sent.
        //
        tmp = PyObjectHandle{callMethod(future.get(), "set_result", Py_None)};
        if (PyErr_Occurred())
        {
            handleException();
        }
    }
}

IcePy::AsyncTypedInvocation::AsyncTypedInvocation(const Ice::ObjectPrx& prx, PyObject* pyProxy, OperationPtr op)
    : AsyncInvocation(prx, pyProxy),
      _op(std::move(op))
{
}

function<void()>
IcePy::AsyncTypedInvocation::handleInvoke(PyObject* args, PyObject* /* kwds */)
{
    // Called from Python code, so the GIL is already acquired.

    assert(PyTuple_Check(args));
    assert(PyTuple_GET_SIZE(args) == 2); // Format is ((params...), context|None)
    PyObject* pyparams = PyTuple_GET_ITEM(args, 0);
    assert(PyTuple_Check(pyparams));
    PyObject* pyctx = PyTuple_GET_ITEM(args, 1);

    // Marshal the input parameters to a byte sequence.
    Ice::OutputStream os(_communicator);
    pair<const byte*, const byte*> params;
    if (!prepareRequest(_op, pyparams, AsyncMapping, &os, params))
    {
        return nullptr;
    }

    checkTwowayOnly(_op, _prx);

    // Invoke the operation asynchronously.
    Ice::Context context;
    if (pyctx != Py_None)
    {
        if (!PyDict_Check(pyctx))
        {
            PyErr_Format(PyExc_ValueError, "context argument must be None or a dictionary");
            return nullptr;
        }

        if (!dictionaryToContext(pyctx, context))
        {
            return nullptr;
        }
    }

    auto self = shared_from_this();
    return _prx->ice_invokeAsync(
        _op->sliceName,
        _op->mode,
        params,
        [self](bool ok, pair<const byte*, const byte*> results) { self->response(ok, results); },
        [self](exception_ptr ex) { self->exception(ex); },
        [self](bool sentSynchronously) { self->sent(sentSynchronously); },
        pyctx == Py_None ? Ice::noExplicitContext : context);
}

void
IcePy::AsyncTypedInvocation::handleResponse(PyObject* future, bool ok, pair<const byte*, const byte*> results)
{
    try
    {
        if (ok)
        {
            // Unmarshal the results.
            PyObjectHandle args;

            try
            {
                args = PyObjectHandle{unmarshalResults(_op, results)};
                if (!args.get())
                {
                    assert(PyErr_Occurred());
                    return;
                }
            }
            catch (...)
            {
                PyObjectHandle exh{convertException(current_exception())};
                assert(exh.get());
                PyObjectHandle tmp{callMethod(future, "set_exception", exh.get())};
                PyErr_Clear();
                return;
            }

            // The future's result is always one value:
            //
            // - If the operation has no out parameters, the result is None
            // - If the operation returns one value, the result is the value
            // - If the operation returns multiple values, the result is a tuple containing the values
            PyObjectHandle r;
            if (PyTuple_GET_SIZE(args.get()) == 0)
            {
                r = PyObjectHandle{Py_None};
            }
            else if (PyTuple_GET_SIZE(args.get()) == 1)
            {
                // PyTuple_GET_ITEM steals a reference.
                PyObject* obj = PyTuple_GET_ITEM(args.get(), 0);
                assert(obj);
                r = PyObjectHandle{Py_NewRef(obj)};
            }
            else
            {
                r = PyObjectHandle{args};
            }

            PyObjectHandle tmp{callMethod(future, "set_result", r.get())};
            PyErr_Clear();
        }
        else
        {
            PyObjectHandle ex{unmarshalException(_op, results)};
            PyObjectHandle tmp{callMethod(future, "set_exception", ex.get())};
            PyErr_Clear();
        }
    }
    catch (const AbortMarshaling&)
    {
        assert(PyErr_Occurred());
    }
}

IcePy::SyncBlobjectInvocation::SyncBlobjectInvocation(const Ice::ObjectPrx& prx) : Invocation(prx) {}

PyObject*
IcePy::SyncBlobjectInvocation::invoke(PyObject* args, PyObject* /* kwds */)
{
    char* operation;
    PyObject* mode{nullptr};
    PyObject* inParams{nullptr};
    PyObject* operationModeType{lookupType("Ice.OperationMode")};
    PyObject* ctx{nullptr};
    if (!PyArg_ParseTuple(args, "sO!O!|O", &operation, operationModeType, &mode, &PyBytes_Type, &inParams, &ctx))
    {
        return nullptr;
    }

    PyObjectHandle modeValue{getAttr(mode, "value", true)};
    auto opMode = (Ice::OperationMode) static_cast<int>(PyLong_AsLong(modeValue.get()));
    assert(!PyErr_Occurred());

    Py_ssize_t sz{PyBytes_GET_SIZE(inParams)};
    pair<const ::byte*, const ::byte*> in(static_cast<const byte*>(nullptr), static_cast<const byte*>(nullptr));
    if (sz > 0)
    {
        in.first = reinterpret_cast<byte*>(PyBytes_AS_STRING(inParams));
        in.second = in.first + sz;
    }

    try
    {
        vector<byte> out;

        bool ok;
        Ice::Context context;
        if (ctx != nullptr && ctx != Py_None)
        {
            if (!dictionaryToContext(ctx, context))
            {
                return nullptr;
            }
        }

        {
            AllowThreads allowThreads; // Release Python's global interpreter lock during remote invocations.
            ok =
                _prx->ice_invoke(operation, opMode, in, out, !ctx || ctx == Py_None ? Ice::noExplicitContext : context);
        }

        //
        // Prepare the result as a tuple of the bool and out param buffer.
        //
        PyObjectHandle result{PyTuple_New(2)};
        if (!result.get())
        {
            throwPythonException();
        }

        PyTuple_SET_ITEM(result.get(), 0, ok ? Py_True : Py_False);

        PyObjectHandle op;
        if (out.empty())
        {
            op = PyObjectHandle{PyBytes_FromString("")};
        }
        else
        {
            op = PyObjectHandle{
                PyBytes_FromStringAndSize(reinterpret_cast<const char*>(&out[0]), static_cast<Py_ssize_t>(out.size()))};
        }
        if (!op.get())
        {
            throwPythonException();
        }

        PyTuple_SET_ITEM(result.get(), 1, op.release()); // PyTuple_SET_ITEM steals a reference.

        return result.release();
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }
}

IcePy::AsyncBlobjectInvocation::AsyncBlobjectInvocation(const Ice::ObjectPrx& prx, PyObject* pyProxy)
    : AsyncInvocation(prx, pyProxy)
{
}

function<void()>
IcePy::AsyncBlobjectInvocation::handleInvoke(PyObject* args, PyObject* /* kwds */)
{
    char* operation{nullptr};
    PyObject* mode{nullptr};
    PyObject* inParams{nullptr};
    PyObject* operationModeType{lookupType("Ice.OperationMode")};
    PyObject* ctx{nullptr};
    if (!PyArg_ParseTuple(args, "sO!O!|O", &operation, operationModeType, &mode, &PyBytes_Type, &inParams, &ctx))
    {
        return nullptr;
    }

    _op = operation;

    PyObjectHandle modeValue{getAttr(mode, "value", true)};
    auto opMode = (Ice::OperationMode) static_cast<int>(PyLong_AsLong(modeValue.get()));
    assert(!PyErr_Occurred());

    Py_ssize_t sz{PyBytes_GET_SIZE(inParams)};
    pair<const ::byte*, const ::byte*> params{0, 0};
    if (sz > 0)
    {
        params.first = reinterpret_cast<byte*>(PyBytes_AS_STRING(inParams));
        params.second = params.first + sz;
    }

    Ice::Context context;
    if (!ctx || ctx != Py_None)
    {
        if (!dictionaryToContext(ctx, context))
        {
            return nullptr;
        }
    }

    auto self = shared_from_this();
    return _prx->ice_invokeAsync(
        operation,
        opMode,
        params,
        [self](bool ok, pair<const byte*, const byte*> results) { self->response(ok, results); },
        [self](exception_ptr ex) { self->exception(ex); },
        [self](bool sentSynchronously) { self->sent(sentSynchronously); },
        (!ctx || ctx == Py_None) ? Ice::noExplicitContext : context);
}

void
IcePy::AsyncBlobjectInvocation::handleResponse(PyObject* future, bool ok, pair<const byte*, const byte*> results)
{
    // Prepare the args as a tuple of the bool and out param buffer.
    PyObjectHandle args{PyTuple_New(2)};
    if (!args.get())
    {
        assert(PyErr_Occurred());
        PyErr_Print();
        return;
    }

    PyTuple_SET_ITEM(args.get(), 0, ok ? Py_True : Py_False);

    Py_ssize_t sz = results.second - results.first;
    PyObjectHandle op;
    if (sz == 0)
    {
        op = PyObjectHandle{PyBytes_FromString("")};
    }
    else
    {
        op = PyObjectHandle{PyBytes_FromStringAndSize(reinterpret_cast<const char*>(results.first), sz)};
    }
    if (!op.get())
    {
        assert(PyErr_Occurred());
        PyErr_Print();
        return;
    }

    PyTuple_SET_ITEM(args.get(), 1, op.release()); // PyTuple_SET_ITEM steals a reference.

    PyObjectHandle tmp{callMethod(future, "set_result", args.get())};
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
    PyObjectHandle servantMethod{getAttr(servant, dispatchName, false)};
    if (!servantMethod.get())
    {
        ostringstream ostr;
        ostr << "servant for identity " << communicator->identityToString(current.id) << " does not define operation '"
             << dispatchName << "'";
        throw Ice::UnknownException{__FILE__, __LINE__, ostr.str()};
    }

    // Get the dispatch function from Ice.Dispatch module.
    // lookupType() returns a borrowed reference.
    PyObject* dispatchMethod{lookupType("Ice.Dispatch.dispatch")};
    assert(dispatchMethod);
    // Ensure we release the reference when this method exist.
    PyObjectHandle dispatchMethodHandle{Py_NewRef(dispatchMethod)};

    PyObjectHandle dispatchArgs{PyTuple_New(3)};
    if (!dispatchArgs.get())
    {
        throwPythonException();
    }

    DispatchCallbackObject* callback = dispatchCallbackNew(&DispatchCallbackType, nullptr, nullptr);
    if (!callback)
    {
        throwPythonException();
    }
    callback->upcall = new UpcallPtr(shared_from_this());
    PyTuple_SET_ITEM(dispatchArgs.get(), 0, reinterpret_cast<PyObject*>(callback)); // Steals a reference.
    PyTuple_SET_ITEM(dispatchArgs.get(), 1, servantMethod.release());               // Steals a reference.
    PyTuple_SET_ITEM(dispatchArgs.get(), 2, Py_NewRef(args));                       // Steals a reference.

    //
    // Ignore the return value of dispatch -- it will use the dispatch callback.
    //
    PyObjectHandle ignored{PyObject_Call(dispatchMethod, dispatchArgs.get(), nullptr)};

    //
    // Check for exceptions.
    //
    if (PyErr_Occurred())
    {
        PyException ex; // Retrieve it before another Python API call clears it.
        exception(ex);
    }
}

//
// TypedUpcall
//
IcePy::TypedUpcall::TypedUpcall(
    OperationPtr op,
    function<void(bool, pair<const byte*, const byte*>)> response,
    function<void(std::exception_ptr)> error,
    Ice::CommunicatorPtr communicator)
    : _op(std::move(op)),
      _response(std::move(response)),
      _error(std::move(error)),
      _communicator(std::move(communicator))
{
}

void
IcePy::TypedUpcall::dispatch(PyObject* servant, pair<const byte*, const byte*> inBytes, const Ice::Current& current)
{
    _encoding = current.encoding;

    //
    // Unmarshal the in parameters. We have to leave room in the arguments for a trailing
    // Ice::Current object.
    //
    Py_ssize_t count = static_cast<Py_ssize_t>(_op->inParams.size()) + 1;

    PyObjectHandle args{PyTuple_New(count)};
    if (!args.get())
    {
        throwPythonException();
    }

    if (!_op->inParams.empty())
    {
        Ice::InputStream is{_communicator, inBytes, getSliceLoader(_communicator)};

        //
        // Store a pointer to a local StreamUtil object as the stream's closure.
        // This is necessary to support object unmarshaling (see ValueReader).
        //
        StreamUtil util;
        assert(!is.getClosure());
        is.setClosure(&util);

        try
        {
            is.startEncapsulation();

            //
            // Unmarshal the required parameters.
            //
            for (const auto& info : _op->inParams)
            {
                if (!info->optional)
                {
                    void* closure = reinterpret_cast<void*>(info->pos); // NOLINT(performance-no-int-to-ptr)
                    info->type->unmarshal(&is, info, args.get(), closure, false, &info->metadata);
                }
            }

            //
            // Unmarshal the optional parameters.
            //
            for (const auto& info : _op->optionalInParams)
            {
                if (is.readOptional(info->tag, info->type->optionalFormat()))
                {
                    void* closure = reinterpret_cast<void*>(info->pos); // NOLINT(performance-no-int-to-ptr)
                    info->type->unmarshal(&is, info, args.get(), closure, true, &info->metadata);
                }
                else
                {
                    PyTuple_SET_ITEM(args.get(), info->pos, Py_None);
                }
            }

            if (_op->sendsClasses)
            {
                is.readPendingValues();
            }

            is.endEncapsulation();

            util.updateSlicedData();
        }
        catch (const AbortMarshaling&)
        {
            throwPythonException();
        }
    }

    //
    // Create an object to represent Ice::Current. We need to append this to the argument tuple.
    //
    PyObjectHandle curr{createCurrent(current)};
    PyTuple_SET_ITEM(
        args.get(),
        PyTuple_GET_SIZE(args.get()) - 1,
        curr.release()); // PyTuple_SET_ITEM steals a reference.

    dispatchImpl(servant, _op->mappedName, args.get(), current);
}

void
IcePy::TypedUpcall::response(PyObject* result)
{
    try
    {
        try
        {
            Ice::OutputStream os(_communicator);
            os.startEncapsulation(_encoding, _op->format);
            _op->marshalResult(os, result);
            os.endEncapsulation();
            _response(true, os.finished());
        }
        catch (const AbortMarshaling&)
        {
            try
            {
                throwPythonException();
            }
            catch (...)
            {
                _error(current_exception());
            }
        }
    }
    catch (...)
    {
        _error(current_exception());
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

            PyObject* userExceptionType{lookupType("Ice.UserException")};

            if (PyObject_IsInstance(ex.ex.get(), userExceptionType))
            {
                //
                // Get the exception's type.
                //
                PyObjectHandle iceType{getAttr(ex.ex.get(), "_ice_type", false)};
                assert(iceType.get());
                ExceptionInfoPtr info = dynamic_pointer_cast<ExceptionInfo>(getException(iceType.get()));
                assert(info);

                Ice::OutputStream os(_communicator);
                os.startEncapsulation(_encoding, _op->format);

                ExceptionWriter writer(ex.ex, info);
                os.writeException(writer);
                os.endEncapsulation();
                _response(false, os.finished());
            }
            else
            {
                ex.raise();
            }
        }
        catch (const AbortMarshaling&)
        {
            throwPythonException();
        }
    }
    catch (...)
    {
        _error(current_exception());
    }
}

//
// BlobjectUpcall
//
IcePy::BlobjectUpcall::BlobjectUpcall(
    function<void(bool, pair<const byte*, const byte*>)> response,
    function<void(std::exception_ptr)> error)
    : _response(std::move(response)),
      _error(std::move(error))
{
}

void
IcePy::BlobjectUpcall::dispatch(PyObject* servant, pair<const byte*, const byte*> inBytes, const Ice::Current& current)
{
    Ice::CommunicatorPtr communicator = current.adapter->getCommunicator();

    Py_ssize_t count = 2; // First is the inParams, second is the Ice::Current object.

    Py_ssize_t start = 0;

    PyObjectHandle args{PyTuple_New(count)};
    if (!args.get())
    {
        throwPythonException();
    }

    PyObjectHandle ip;

    if (inBytes.second == inBytes.first)
    {
        ip = PyObjectHandle{PyBytes_FromString("")};
    }
    else
    {
        ip = PyObjectHandle{
            PyBytes_FromStringAndSize(reinterpret_cast<const char*>(inBytes.first), inBytes.second - inBytes.first)};
    }

    PyTuple_SET_ITEM(args.get(), start, ip.release()); // PyTuple_SET_ITEM steals a reference.
    ++start;

    //
    // Create an object to represent Ice::Current. We need to append
    // this to the argument tuple.
    //
    PyObjectHandle curr{createCurrent(current)};
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
        if (!PyTuple_Check(result) || PyTuple_GET_SIZE(result) != 2)
        {
            throw Ice::MarshalException(__FILE__, __LINE__, "operation 'ice_invoke' should return a tuple of length 2");
        }

        PyObject* arg{PyTuple_GET_ITEM(result, 0)};
        bool isTrue = PyObject_IsTrue(arg) == 1;

        arg = PyTuple_GET_ITEM(result, 1);

        if (!PyBytes_Check(arg))
        {
            throw Ice::MarshalException(__FILE__, __LINE__, "invalid return value for operation 'ice_invoke'");
        }

        Py_ssize_t sz = PyBytes_GET_SIZE(arg);
        pair<const ::byte*, const ::byte*> r{0, 0};
        if (sz > 0)
        {
            r.first = reinterpret_cast<byte*>(PyBytes_AS_STRING(arg));
            r.second = r.first + sz;
        }

        _response(isTrue, r);
    }
    catch (const AbortMarshaling&)
    {
        try
        {
            throwPythonException();
        }
        catch (...)
        {
            _error(current_exception());
        }
    }
    catch (...)
    {
        _error(current_exception());
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
    catch (...)
    {
        _error(current_exception());
    }
}

PyObject*
IcePy::iceInvoke(PyObject* proxy, PyObject* args)
{
    Ice::ObjectPrx prx = getProxy(proxy);
    InvocationPtr i = make_shared<SyncBlobjectInvocation>(prx);
    return i->invoke(args);
}

PyObject*
IcePy::iceInvokeAsync(PyObject* proxy, PyObject* args)
{
    Ice::ObjectPrx prx = getProxy(proxy);
    InvocationPtr i = make_shared<AsyncBlobjectInvocation>(prx, proxy);
    return i->invoke(args);
}

PyObject*
IcePy::createAsyncInvocationContext(function<void()> cancel, Ice::CommunicatorPtr communicator)
{
    AsyncInvocationContextObject* obj{asyncInvocationContextNew(&AsyncInvocationContextType, nullptr, nullptr)};
    if (!obj)
    {
        return nullptr;
    }
    obj->cancel = new function<void()>(std::move(cancel));
    obj->communicator = new Ice::CommunicatorPtr(std::move(communicator));
    return reinterpret_cast<PyObject*>(obj);
}

IcePy::FlushAsyncCallback::FlushAsyncCallback(string op) : _op(std::move(op)) {}

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
    if (_exception)
    {
        PyObjectHandle tmp{callMethod(future, "set_exception", _exception)};
        PyErr_Clear();
    }
    else if (_sent)
    {
        PyObjectHandle tmp{callMethod(future, "set_sent", _sentSynchronously ? Py_True : Py_False)};
        PyErr_Clear();
        //
        // We consider the invocation complete when sent.
        //
        tmp = callMethod(future, "set_result", Py_None);
        PyErr_Clear();
    }
    else
    {
        _future = Py_NewRef(future);
    }
}

void
IcePy::FlushAsyncCallback::exception(std::exception_ptr ex)
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    if (!_future)
    {
        //
        // The future hasn't been set yet, which means the request is still being invoked. Save the results for later.
        //
        _exception = convertException(ex);
        return;
    }

    PyObjectHandle exh{convertException(ex)};
    assert(exh.get());
    PyObjectHandle tmp{callMethod(_future, "set_exception", exh.get())};
    PyErr_Clear();

    Py_DECREF(_future); // Break cyclic dependency.
    _future = nullptr;
}

void
IcePy::FlushAsyncCallback::sent(bool sentSynchronously)
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    if (!_future)
    {
        //
        // The future hasn't been set yet, which means the request is still being invoked. Save the results for later.
        //
        _sent = true;
        _sentSynchronously = sentSynchronously;
        return;
    }

    PyObjectHandle tmp{callMethod(_future, "set_sent", _sentSynchronously ? Py_True : Py_False)};
    PyErr_Clear();
    //
    // We consider the invocation complete when sent.
    //
    tmp = callMethod(_future, "set_result", Py_None);
    PyErr_Clear();

    Py_DECREF(_future); // Break cyclic dependency.
    _future = nullptr;
}

IcePy::GetConnectionAsyncCallback::GetConnectionAsyncCallback(Ice::CommunicatorPtr communicator, string op)
    : _communicator(std::move(communicator)),
      _op(std::move(op))

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
    if (_connection)
    {
        PyObjectHandle pyConn{createConnection(_connection, _communicator)};
        assert(pyConn.get());
        PyObjectHandle tmp{callMethod(future, "set_result", pyConn.get())};
        PyErr_Clear();
    }
    else if (_exception)
    {
        PyObjectHandle tmp{callMethod(future, "set_exception", _exception)};
        PyErr_Clear();
    }
    else
    {
        _future = Py_NewRef(future);
    }
}

void
IcePy::GetConnectionAsyncCallback::response(const Ice::ConnectionPtr& conn)
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    if (!_future)
    {
        //
        // The future hasn't been set yet, which means the request is still being invoked. Save the results for later.
        //
        _connection = conn;
        return;
    }

    PyObjectHandle pyConn{createConnection(conn, _communicator)};
    PyObjectHandle tmp{callMethod(_future, "set_result", pyConn.get())};
    PyErr_Clear();

    Py_DECREF(_future); // Break cyclic dependency.
    _future = nullptr;
}

void
IcePy::GetConnectionAsyncCallback::exception(std::exception_ptr ex)
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    if (!_future)
    {
        //
        // The future hasn't been set yet, which means the request is still being invoked. Save the results for later.
        //
        _exception = convertException(ex);
        return;
    }

    PyObjectHandle exh{convertException(ex)};
    PyObjectHandle tmp{callMethod(_future, "set_exception", exh.get())};
    PyErr_Clear();

    Py_DECREF(_future); // Break cyclic dependency.
    _future = nullptr;
}

//
// ServantWrapper implementation.
//
IcePy::ServantWrapper::ServantWrapper(PyObject* servant) : _servant(Py_NewRef(servant)) {}

IcePy::ServantWrapper::~ServantWrapper()
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    Py_DECREF(_servant);
}

PyObject*
IcePy::ServantWrapper::getObject()
{
    return Py_NewRef(_servant);
}

//
// TypedServantWrapper implementation.
//
IcePy::TypedServantWrapper::TypedServantWrapper(PyObject* servant)
    : ServantWrapper(servant),
      _lastOp(_operationMap.end())
{
}

void
IcePy::TypedServantWrapper::ice_invokeAsync(
    pair<const byte*, const byte*> inParams,
    function<void(bool, pair<const byte*, const byte*>)> response,
    function<void(std::exception_ptr)> error,
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
        if (_lastOp != _operationMap.end() && _lastOp->first == current.operation)
        {
            op = _lastOp->second;
        }
        else
        {
            //
            // Next check our cache of operations.
            //
            _lastOp = _operationMap.find(current.operation);
            if (_lastOp == _operationMap.end())
            {
                //
                // Look for the Operation object in the servant's type.
                //
                string attrName = "_op_" + current.operation;
                PyObjectHandle h{getAttr((PyObject*)_servant->ob_type, attrName, false)};
                if (!h.get())
                {
                    PyErr_Clear();
                    throw Ice::OperationNotExistException{__FILE__, __LINE__};
                }

                assert(PyObject_IsInstance(h.get(), reinterpret_cast<PyObject*>(&OperationType)) == 1);
                auto* obj = reinterpret_cast<OperationObject*>(h.get());
                op = *obj->op;
                _lastOp = _operationMap.insert(OperationMap::value_type(current.operation, op)).first;
            }
            else
            {
                op = _lastOp->second;
            }
        }

        if (op->mode == Ice::OperationMode::Normal)
        {
            checkNonIdempotent(current);
        }

        UpcallPtr up = make_shared<TypedUpcall>(op, std::move(response), error, current.adapter->getCommunicator());
        up->dispatch(_servant, inParams, current);
    }
    catch (...)
    {
        error(current_exception());
    }
}

//
// BlobjectServantWrapper implementation.
//
IcePy::BlobjectServantWrapper::BlobjectServantWrapper(PyObject* servant) : ServantWrapper(servant) {}

void
IcePy::BlobjectServantWrapper::ice_invokeAsync(
    pair<const byte*, const byte*> inParams,
    function<void(bool, pair<const byte*, const byte*>)> response,
    function<void(exception_ptr)> error,
    const Ice::Current& current)
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.
    try
    {
        UpcallPtr up = make_shared<BlobjectUpcall>(std::move(response), error);
        up->dispatch(_servant, inParams, current);
    }
    catch (...)
    {
        error(current_exception());
    }
}

IcePy::ServantWrapperPtr
IcePy::createServantWrapper(PyObject* servant)
{
    PyObject* blobjectType{lookupType("Ice.Blobject")};
    if (PyObject_IsInstance(servant, blobjectType))
    {
        return make_shared<BlobjectServantWrapper>(servant);
    }
    else
    {
        return make_shared<TypedServantWrapper>(servant);
    }
}

PyObject*
IcePy::createFuture(PyObject* asyncInvocationContext)
{
    auto type = reinterpret_cast<PyTypeObject*>(lookupType("Ice.InvocationFuture"));
    assert(type);

    // Create the Ice.InvocationFuture object.
    PyObjectHandle newArgs{PyTuple_New(0)};
    PyObjectHandle future{type->tp_new(type, newArgs.get(), nullptr)};
    if (!future.get())
    {
        return nullptr;
    }

    // Prepare the arguments for the Ice.InvocationFuture.__init__ method.
    PyObjectHandle initArgs{PyTuple_New(1)};
    if (!initArgs.get())
    {
        return nullptr;
    }
    PyTuple_SET_ITEM(initArgs.get(), 0, Py_NewRef(asyncInvocationContext));

    // Call the Ice.InvocationFuture.__init__ method.
    type->tp_init(future.get(), initArgs.get(), nullptr);

    // Release the reference to the future object and let the caller take ownership.
    return future.release();
}
