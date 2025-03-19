// Copyright (c) ZeroC, Inc.

#include "Operation.h"
#include "Communicator.h"
#include "Proxy.h"
#include "Types.h"
#include "Util.h"

using namespace std;
using namespace IcePHP;

extern "C"
{
    ZEND_FUNCTION(IcePHP_Operation_call);
}

namespace IcePHP
{
    class ParamInfo
    {
    public:
        TypeInfoPtr type;
        bool optional;
        int tag;
        int pos;
    };
    using ParamInfoPtr = std::shared_ptr<ParamInfo>;
    using ParamInfoList = list<ParamInfoPtr>;

    // Receives an out parameter or return value.
    class ResultCallback final : public UnmarshalCallback
    {
    public:
        ResultCallback();
        ~ResultCallback();

        void unmarshaled(zval*, zval*, void*) final;

        void unset(void);

        zval zv;
    };
    using ResultCallbackPtr = std::shared_ptr<ResultCallback>;
    using ResultCallbackList = vector<ResultCallbackPtr>;

    // Encapsulates attributes of an operation.
    class OperationI final : public Operation
    {
    public:
        OperationI(
            const char*,
            const char*,
            Ice::OperationMode,
            std::optional<Ice::FormatType>,
            zval*,
            zval*,
            zval*,
            zval*);
        ~OperationI();

        zend_function* function() final;

        string sliceName;  // On-the-wire name.
        string mappedName; // mapped PHP function name.
        Ice::OperationMode mode;
        std::optional<Ice::FormatType> format;
        ParamInfoList inParams;
        ParamInfoList optionalInParams;
        ParamInfoList outParams;
        ParamInfoList optionalOutParams;
        ParamInfoPtr returnType;
        ExceptionInfoList exceptions;
        bool sendsClasses;
        bool returnsClasses;
        int numParams;

    private:
        zend_internal_function* _zendFunction;

        static void convertParams(zval*, ParamInfoList&, bool&);
        static ParamInfoPtr convertParam(zval*, int);
        static void getArgInfo(zend_internal_arg_info&, const ParamInfoPtr&, bool);
    };
    using OperationIPtr = std::shared_ptr<OperationI>;

    // The base class for client-side invocations.
    class Invocation
    {
    public:
        virtual ~Invocation() = default;

        Invocation(Ice::ObjectPrx, CommunicatorInfoPtr);

        virtual void invoke(INTERNAL_FUNCTION_PARAMETERS) = 0;

    protected:
        Ice::ObjectPrx _prx;
        CommunicatorInfoPtr _communicator;
    };
    using InvocationPtr = std::shared_ptr<Invocation>;

    // TypedInvocation uses the information in the given operation to validate, marshal, and unmarshal parameters and
    // exceptions.
    class TypedInvocation : public Invocation
    {
    public:
        TypedInvocation(Ice::ObjectPrx, CommunicatorInfoPtr, OperationIPtr);

    protected:
        OperationIPtr _op;

        bool prepareRequest(int, zval*, Ice::OutputStream*, pair<const byte*, const byte*>&);
        void unmarshalResults(int, zval*, zval*, pair<const byte*, const byte*>);
        void unmarshalException(zval*, pair<const byte*, const byte*>);
        bool validateException(const ExceptionInfoPtr&) const;
        void checkTwowayOnly(const Ice::ObjectPrx&) const;
    };

    // A synchronous typed invocation.
    class SyncTypedInvocation final : public TypedInvocation
    {
    public:
        SyncTypedInvocation(Ice::ObjectPrx, CommunicatorInfoPtr, OperationIPtr);

        void invoke(INTERNAL_FUNCTION_PARAMETERS) final;
    };
}

// ResultCallback implementation.
IcePHP::ResultCallback::ResultCallback() { ZVAL_UNDEF(&zv); }

IcePHP::ResultCallback::~ResultCallback()
{
#ifdef NDEBUG
    // BUGFIX releasing this object triggers an assert in zend_weakrefs_notify
    // see https://github.com/zeroc-ice/ice/issues/1439
    zval_ptr_dtor(&zv);
#endif
}

void
IcePHP::ResultCallback::unmarshaled(zval* val, zval*, void*)
{
    // Copy the unmarshaled value into this result callback. This increases the refcount for refcounted values and
    // coppies non-refcound values.
    ZVAL_COPY(&zv, val);
}

void
IcePHP::ResultCallback::unset(void)
{
    assignUnset(&zv);
}

// OperationI implementation.
IcePHP::OperationI::OperationI(
    const char* name,
    const char* mapped,
    Ice::OperationMode m,
    std::optional<Ice::FormatType> f,
    zval* in,
    zval* out,
    zval* ret,
    zval* ex)
    : sliceName(name),
      mappedName(mapped),
      mode(m),
      format(f),
      _zendFunction(0)
{
    // inParams
    sendsClasses = false;
    if (in)
    {
        convertParams(in, inParams, sendsClasses);
    }

    // outParams
    returnsClasses = false;
    if (out)
    {
        convertParams(out, outParams, returnsClasses);
    }

    // returnType
    if (ret)
    {
        returnType = convertParam(ret, 0);
        if (!returnsClasses)
        {
            returnsClasses = returnType->type->usesClasses();
        }
    }

    numParams = static_cast<int>(inParams.size() + outParams.size());

    class SortFn
    {
    public:
        static bool compare(const ParamInfoPtr& lhs, const ParamInfoPtr& rhs) { return lhs->tag < rhs->tag; }

        static bool isRequired(const ParamInfoPtr& i) { return !i->optional; }
    };

    // The inParams list represents the parameters in the order of declaration. We also need a sorted list of optional
    // parameters.
    ParamInfoList l = inParams;
    copy(l.begin(), remove_if(l.begin(), l.end(), SortFn::isRequired), back_inserter(optionalInParams));
    optionalInParams.sort(SortFn::compare);

    // The outParams list represents the parameters in the order of declaration. We also need a sorted list of optional
    // parameters. If the return value is optional, we must include it in this list.
    l = outParams;
    copy(l.begin(), remove_if(l.begin(), l.end(), SortFn::isRequired), back_inserter(optionalOutParams));
    if (returnType && returnType->optional)
    {
        optionalOutParams.push_back(returnType);
    }
    optionalOutParams.sort(SortFn::compare);

    // exceptions
    if (ex)
    {
        HashTable* arr = Z_ARRVAL_P(ex);
        zval* val;

        ZEND_HASH_FOREACH_VAL(arr, val)
        {
            ExceptionInfoPtr i = Wrapper<ExceptionInfoPtr>::value(val);
            exceptions.push_back(i);
        }
        ZEND_HASH_FOREACH_END();
    }
}

IcePHP::OperationI::~OperationI()
{
    if (_zendFunction)
    {
        delete[] _zendFunction->arg_info;
        zend_string_release(_zendFunction->function_name);
        efree(_zendFunction);
    }
}

zend_function*
IcePHP::OperationI::function()
{
    if (!_zendFunction)
    {
        // Create an array that indicates how arguments are passed to the operation.
        zend_internal_arg_info* argInfo = new zend_internal_arg_info[numParams];

        int i = 0;
        for (ParamInfoList::const_iterator p = inParams.begin(); p != inParams.end(); ++p, ++i)
        {
            getArgInfo(argInfo[i], *p, false);
        }

        for (ParamInfoList::const_iterator p = outParams.begin(); p != outParams.end(); ++p, ++i)
        {
            getArgInfo(argInfo[i], *p, true);
        }

        _zendFunction = static_cast<zend_internal_function*>(ecalloc(1, sizeof(zend_internal_function)));
        _zendFunction->type = ZEND_INTERNAL_FUNCTION;
        _zendFunction->function_name =
            zend_string_init(mappedName.c_str(), static_cast<uint32_t>(mappedName.length()), 0);
        _zendFunction->scope = proxyClassEntry;
        _zendFunction->fn_flags = ZEND_ACC_PUBLIC;
        _zendFunction->prototype = 0;
        _zendFunction->num_args = static_cast<uint32_t>(numParams);
        _zendFunction->arg_info = argInfo;
        _zendFunction->required_num_args = _zendFunction->num_args;
        _zendFunction->handler = ZEND_FN(IcePHP_Operation_call);
        zend_set_function_arg_flags(reinterpret_cast<zend_function*>(_zendFunction));
    }
    return reinterpret_cast<zend_function*>(_zendFunction);
}

void
IcePHP::OperationI::convertParams(zval* p, ParamInfoList& params, bool& usesClasses)
{
    assert(Z_TYPE_P(p) == IS_ARRAY);
    HashTable* arr = Z_ARRVAL_P(p);
    zval* val;
    int32_t i = 0;
    ZEND_HASH_FOREACH_VAL(arr, val)
    {
        ParamInfoPtr param = convertParam(val, i++);
        params.push_back(param);
        if (!param->optional && !usesClasses)
        {
            usesClasses = param->type->usesClasses();
        }
    }
    ZEND_HASH_FOREACH_END();
}

ParamInfoPtr
IcePHP::OperationI::convertParam(zval* p, int pos)
{
    assert(Z_TYPE_P(p) == IS_ARRAY);
    HashTable* arr = Z_ARRVAL_P(p);

    ParamInfoPtr param = make_shared<ParamInfo>();

    param->type = Wrapper<TypeInfoPtr>::value(zend_hash_index_find(arr, 0));
    param->optional = zend_hash_num_elements(arr) > 1;

    if (param->optional)
    {
        assert(Z_TYPE_P(zend_hash_index_find(arr, 1)) == IS_LONG);
        param->tag = static_cast<int>(Z_LVAL_P(zend_hash_index_find(arr, 1)));
    }
    param->pos = pos;

    return param;
}

void
IcePHP::OperationI::getArgInfo(zend_internal_arg_info& arg, const ParamInfoPtr& info, bool out)
{
    const zend_uchar pass_by_ref = out ? 1 : 0;
    const zend_bool allow_null = 1;
    if (!info->optional &&
        (dynamic_pointer_cast<SequenceInfo>(info->type) || dynamic_pointer_cast<DictionaryInfo>(info->type)))
    {
        zend_internal_arg_info ai[] = {
            ZEND_ARG_ARRAY_INFO(pass_by_ref, static_cast<uint32_t>(0), static_cast<uint32_t>(allow_null))};
        arg = ai[0];
    }
    else
    {
        zend_internal_arg_info ai[] = {
            ZEND_ARG_CALLABLE_INFO(pass_by_ref, static_cast<uint32_t>(0), static_cast<uint32_t>(allow_null))};
        arg = ai[0];
    }
}

// Invocation
IcePHP::Invocation::Invocation(Ice::ObjectPrx prx, CommunicatorInfoPtr communicator)
    : _prx(std::move(prx)),
      _communicator(std::move(communicator))
{
}

// TypedInvocation
IcePHP::TypedInvocation::TypedInvocation(Ice::ObjectPrx prx, CommunicatorInfoPtr communicator, OperationIPtr op)
    : Invocation(std::move(prx), std::move(communicator)),
      _op(std::move(op))
{
}

bool
IcePHP::TypedInvocation::prepareRequest(
    int argc,
    zval* args,
    Ice::OutputStream* os,
    pair<const byte*, const byte*>& params)
{
    // Verify that the expected number of arguments are supplied. The context argument is optional.
    if (argc != _op->numParams && argc != _op->numParams + 1)
    {
        ostringstream os;
        os << "incorrect number of parameters (" << argc << ")";
        runtimeError(os.str());
        return false;
    }

    // The operation's configuration (zend_function) forces out parameters to be passed by reference.
#ifdef DEBUG
    for (int i = static_cast<int>(_op->inParams.size()); i < _op->numParams; ++i)
    {
        assert(Z_ISREF(args[i]));
    }
#endif

    if (!_op->inParams.empty())
    {
        try
        {
            // Marshal the in parameters.
            os->startEncapsulation(_prx->ice_getEncodingVersion(), _op->format);

            ObjectMap objectMap;
            ParamInfoList::iterator p;

            // Validate the supplied arguments.
            for (const auto& info : _op->inParams)
            {
                zval* arg = &args[info->pos];
                assert(!Z_ISREF_P(arg));

                if ((!info->optional || !isUnset(arg)) && !info->type->validate(arg, false))
                {
                    ostringstream os;
                    os << "invalid value for argument " << info->pos + 1 << " in operation '" << _op->mappedName << "'";
                    invalidArgument(os.str());
                    return false;
                }
            }

            // Marshal the required parameters.
            for (const auto& info : _op->inParams)
            {
                if (!info->optional)
                {
                    zval* arg = &args[info->pos];
                    assert(!Z_ISREF_P(arg));

                    info->type->marshal(arg, os, &objectMap, false);
                }
            }

            // Marshal the optional parameters.
            for (const auto& info : _op->optionalInParams)
            {
                zval* arg = &args[info->pos];
                assert(!Z_ISREF_P(arg));

                if (!isUnset(arg) && os->writeOptional(info->tag, info->type->optionalFormat()))
                {
                    info->type->marshal(arg, os, &objectMap, true);
                }
            }

            if (_op->sendsClasses)
            {
                os->writePendingValues();
            }

            os->endEncapsulation();
            params = os->finished();
        }
        catch (const AbortMarshaling&)
        {
            return false;
        }
        catch (...)
        {
            throwException(current_exception());
            return false;
        }
    }

    return true;
}

void
IcePHP::TypedInvocation::unmarshalResults(int argc, zval* args, zval* ret, pair<const byte*, const byte*> bytes)
{
    Ice::InputStream is(_communicator->getCommunicator(), bytes);

    // Store a pointer to a local StreamUtil object as the stream's closure. This is necessary to support object
    // unmarshaling (see ValueReader).
    StreamUtil util;
    assert(!is.getClosure());
    is.setClosure(&util);

    is.startEncapsulation();

    // These callbacks collect references (copies for unreferenced types) to the unmarshaled values. We copy them into
    // the argument list *after* any pending objects have been unmarshaled.
    ResultCallbackList outParamCallbacks;
    ResultCallbackPtr retCallback;

    outParamCallbacks.resize(_op->outParams.size());

    // Unmarshal the required out parameters.
    for (const auto& info : _op->outParams)
    {
        if (!info->optional)
        {
            auto cb = make_shared<ResultCallback>();
            outParamCallbacks[info->pos] = cb;
            info->type->unmarshal(&is, cb, _communicator, 0, 0, false);
        }
    }

    // Unmarshal the required return value, if any.
    if (_op->returnType && !_op->returnType->optional)
    {
        retCallback = make_shared<ResultCallback>();
        _op->returnType->type->unmarshal(&is, retCallback, _communicator, 0, 0, false);
    }

    // Unmarshal the optional results. This includes an optional return value.
    for (const auto& info : _op->optionalOutParams)
    {
        auto cb = make_shared<ResultCallback>();
        if (_op->returnType && info->tag == _op->returnType->tag)
        {
            retCallback = cb;
        }
        else
        {
            outParamCallbacks[info->pos] = cb;
        }

        if (is.readOptional(info->tag, info->type->optionalFormat()))
        {
            info->type->unmarshal(&is, cb, _communicator, 0, 0, true);
        }
        else
        {
            cb->unset();
        }
    }

    if (_op->returnsClasses)
    {
        is.readPendingValues();
    }

    is.endEncapsulation();

    util.updateSlicedData();

    int i = static_cast<int>(_op->inParams.size());
    for (ResultCallbackList::iterator q = outParamCallbacks.begin(); q != outParamCallbacks.end(); ++q, ++i)
    {
        assert(Z_ISREF(args[i]));
        zval* arg = Z_REFVAL_P(&args[i]);
        ZVAL_COPY(arg, &(*q)->zv);
    }

    if (_op->returnType)
    {
        ZVAL_COPY(ret, &retCallback->zv);
    }
}

void
IcePHP::TypedInvocation::unmarshalException(zval* zex, pair<const byte*, const byte*> bytes)
{
    Ice::InputStream is(_communicator->getCommunicator(), bytes);

    // Store a pointer to a local StreamUtil object as the stream's closure. This is necessary to support object
    // unmarshaling (see ValueReader).
    StreamUtil util;
    assert(!is.getClosure());
    is.setClosure(&util);

    is.startEncapsulation();

    try
    {
        is.throwException(
            [this](string_view id)
            {
                ExceptionInfoPtr info = getExceptionInfo(id);
                if (info)
                {
                    throw ExceptionReader(_communicator, info);
                }
                // It's ok for a user exception factory to not throw - it's equivalent to a null factory.
            });

        assert(false); // throwException always throws an exception
    }
    catch (const ExceptionReader& r)
    {
        is.endEncapsulation();

        zval* ex = r.getException();
        ExceptionInfoPtr info = r.getInfo();

        if (validateException(info))
        {
            ZVAL_COPY(zex, ex);
        }
        else
        {
            convertException(
                zex,
                make_exception_ptr(Ice::UnknownUserException::fromTypeId(__FILE__, __LINE__, info->id.c_str())));
        }
    }
    catch (...)
    {
        convertException(zex, current_exception());
    }
}

bool
IcePHP::TypedInvocation::validateException(const ExceptionInfoPtr& info) const
{
    for (const auto& p : _op->exceptions)
    {
        if (info->isA(p->id))
        {
            return true;
        }
    }
    return false;
}

void
IcePHP::TypedInvocation::checkTwowayOnly(const Ice::ObjectPrx& proxy) const
{
    if ((_op->returnType || !_op->outParams.empty()) && !proxy->ice_isTwoway())
    {
        throw Ice::TwowayOnlyException(__FILE__, __LINE__, _op->sliceName);
    }
}

// SyncTypedInvocation
IcePHP::SyncTypedInvocation::SyncTypedInvocation(Ice::ObjectPrx prx, CommunicatorInfoPtr communicator, OperationIPtr op)
    : TypedInvocation(std::move(prx), std::move(communicator), std::move(op))
{
}

void
IcePHP::SyncTypedInvocation::invoke(INTERNAL_FUNCTION_PARAMETERS)
{
    // Retrieve the arguments.
    zval* args = static_cast<zval*>(ecalloc(1, ZEND_NUM_ARGS() * sizeof(zval)));
    AutoEfree autoArgs(args); // Call efree on return
    if (zend_get_parameters_array_ex(ZEND_NUM_ARGS(), args) == FAILURE)
    {
        runtimeError("unable to get arguments");
        return;
    }

    Ice::OutputStream os(_prx->ice_getCommunicator());
    pair<const byte*, const byte*> params;
    if (!prepareRequest(ZEND_NUM_ARGS(), args, &os, params))
    {
        return;
    }

    bool hasCtx = false;
    Ice::Context ctx;
    if (ZEND_NUM_ARGS() == static_cast<uint32_t>(_op->numParams) + 1)
    {
        if (!extractContext(&args[ZEND_NUM_ARGS() - 1], ctx))
        {
            return;
        }
        hasCtx = true;
    }

    try
    {
        checkTwowayOnly(_prx);

        // Invoke the operation.
        vector<byte> result;
        bool status;
        {
            if (hasCtx)
            {
                status = _prx->ice_invoke(_op->sliceName, _op->mode, params, result, ctx);
            }
            else
            {
                status = _prx->ice_invoke(_op->sliceName, _op->mode, params, result);
            }
        }

        // Process the reply.
        if (_prx->ice_isTwoway())
        {
            if (!status)
            {
                // Unmarshal a user exception.
                pair<const byte*, const byte*> rb(0, 0);
                if (!result.empty())
                {
                    rb.first = &result[0];
                    rb.second = &result[0] + result.size();
                }

                zval ex;
                ZVAL_UNDEF(&ex);
                unmarshalException(&ex, rb);
                if (!Z_ISUNDEF(ex))
                {
                    zend_throw_exception_object(&ex);
                }
            }
            else if (!_op->outParams.empty() || _op->returnType)
            {
                // Unmarshal the results.
                pair<const byte*, const byte*> rb(0, 0);
                if (!result.empty())
                {
                    rb.first = &result[0];
                    rb.second = &result[0] + result.size();
                }
                unmarshalResults(ZEND_NUM_ARGS(), args, return_value, rb);
            }
        }
    }
    catch (const AbortMarshaling&)
    {
    }
    catch (...)
    {
        throwException(current_exception());
    }
}

ZEND_FUNCTION(IcePHP_defineOperation)
{
    zval* cls;
    char* sliceName;
    size_t sliceNameLen;
    char* mappedName;
    size_t mappedNameLen;
    zend_long mode;
    zend_long format;
    zval* inParams;
    zval* outParams;
    zval* returnType;
    zval* exceptions;

    if (zend_parse_parameters(
            ZEND_NUM_ARGS(),
            const_cast<char*>("oslla!a!a!a!"),
            &cls,
            &sliceName,
            &sliceNameLen,
            &mappedName,
            &mappedNameLen,
            &mode,
            &format,
            &inParams,
            &outParams,
            &returnType,
            &exceptions) == FAILURE)
    {
        return;
    }

    TypeInfoPtr type = Wrapper<TypeInfoPtr>::value(cls);
    auto c = dynamic_pointer_cast<ProxyInfo>(type);
    assert(c);
    // we encode nullopt as -1.
    std::optional<Ice::FormatType> cppFormat;
    if (format != -1)
    {
        cppFormat = static_cast<Ice::FormatType>(format);
    }

    auto op = make_shared<OperationI>(
        sliceName,
        mappedName,
        static_cast<Ice::OperationMode>(mode),
        cppFormat,
        inParams,
        outParams,
        returnType,
        exceptions);

    c->addOperation(mappedName, op);
}

ZEND_FUNCTION(IcePHP_Operation_call)
{
    optional<Ice::ObjectPrx> proxy;
    ProxyInfoPtr info;
    CommunicatorInfoPtr comm;
#ifndef NDEBUG
    bool b =
#endif
        fetchProxy(getThis(), proxy, info, comm);
    assert(b);
    assert(proxy);
    assert(info);

    OperationPtr op = info->getOperation(get_active_function_name());
    assert(op); // handleGetMethod should have already verified the operation's existence.
    auto opi = dynamic_pointer_cast<OperationI>(op);
    assert(opi);

    auto inv = make_shared<SyncTypedInvocation>(std::move(proxy).value(), comm, opi);
    inv->invoke(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
