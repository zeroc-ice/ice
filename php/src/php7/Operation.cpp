// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Operation.h>
#include <Communicator.h>
#include <Proxy.h>
#include <Types.h>
#include <Util.h>
#include <Slice/PHPUtil.h>

using namespace std;
using namespace IcePHP;
using namespace Slice::PHP;

extern "C"
{
ZEND_FUNCTION(IcePHP_Operation_call);
}

namespace IcePHP
{

class ParamInfo : public IceUtil::Shared
{
public:

    TypeInfoPtr type;
    bool optional;
    int tag;
    int pos;
};
typedef IceUtil::Handle<ParamInfo> ParamInfoPtr;
typedef list<ParamInfoPtr> ParamInfoList;

//
// Receives an out parameter or return value.
//
class ResultCallback : public UnmarshalCallback
{
public:

    ResultCallback();
    ~ResultCallback();

    virtual void unmarshaled(zval*, zval*, void*);

    void unset(void);

    zval zv;
};
typedef IceUtil::Handle<ResultCallback> ResultCallbackPtr;
typedef vector<ResultCallbackPtr> ResultCallbackList;

//
// Encapsulates attributes of an operation.
//
class OperationI : public Operation
{
public:

    OperationI(const char*, Ice::OperationMode, Ice::OperationMode, Ice::FormatType, zval*, zval*, zval*, zval*);
    ~OperationI();

    virtual zend_function* function();

    string name; // On-the-wire name.
    Ice::OperationMode mode;
    Ice::OperationMode sendMode;
    Ice::FormatType format;
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
typedef IceUtil::Handle<OperationI> OperationIPtr;

//
// The base class for client-side invocations.
//
class Invocation : public virtual IceUtil::Shared
{
public:

    Invocation(const Ice::ObjectPrx&, const CommunicatorInfoPtr&);

    virtual void invoke(INTERNAL_FUNCTION_PARAMETERS) = 0;

protected:

    Ice::ObjectPrx _prx;
    CommunicatorInfoPtr _communicator;
};
typedef IceUtil::Handle<Invocation> InvocationPtr;

//
// TypedInvocation uses the information in the given operation to validate, marshal, and unmarshal
// parameters and exceptions.
//
class TypedInvocation : public virtual Invocation
{
public:

    TypedInvocation(const Ice::ObjectPrx&, const CommunicatorInfoPtr&, const OperationIPtr&);

protected:

    OperationIPtr _op;

    bool prepareRequest(int, zval*, Ice::OutputStream*, pair<const Ice::Byte*, const Ice::Byte*>&);
    void unmarshalResults(int, zval*, zval*, const pair<const Ice::Byte*, const Ice::Byte*>&);
    void unmarshalException(zval*, const pair<const Ice::Byte*, const Ice::Byte*>&);
    bool validateException(const ExceptionInfoPtr&) const;
    void checkTwowayOnly(const Ice::ObjectPrx&) const;
};

//
// A synchronous typed invocation.
//
class SyncTypedInvocation : public virtual TypedInvocation
{
public:

    SyncTypedInvocation(const Ice::ObjectPrx&, const CommunicatorInfoPtr&, const OperationIPtr&);

    virtual void invoke(INTERNAL_FUNCTION_PARAMETERS);
};

class UserExceptionFactory : public Ice::UserExceptionFactory
{
public:

    UserExceptionFactory(const CommunicatorInfoPtr& communicator) :
        _communicator(communicator)
    {
    }

    virtual void createAndThrow(const string& id)
    {
        ExceptionInfoPtr info = getExceptionInfo(id);
        if(info)
        {
            throw ExceptionReader(_communicator, info);
        }
    }

private:

    const CommunicatorInfoPtr _communicator;
};

}

//
// ResultCallback implementation.
//
IcePHP::ResultCallback::ResultCallback()
{
    ZVAL_UNDEF(&zv);
}

IcePHP::ResultCallback::~ResultCallback()
{
    zval_ptr_dtor(&zv);
}

void
IcePHP::ResultCallback::unmarshaled(zval* val, zval*, void*)
{
    //
    // Copy the unmarshaled value into this result callback
    // This increases the refcount for refcounted values and
    // coppies non-refcound values
    //
    ZVAL_COPY(&zv, val);
}

void
IcePHP::ResultCallback::unset(void)
{
    assignUnset(&zv);
}

//
// OperationI implementation.
//
IcePHP::OperationI::OperationI(const char* n, Ice::OperationMode m, Ice::OperationMode sm, Ice::FormatType f, zval* in,
                               zval* out, zval* ret, zval* ex) :
    name(n), mode(m), sendMode(sm), format(f), _zendFunction(0)
{
    //
    // inParams
    //
    sendsClasses = false;
    if(in)
    {
        convertParams(in, inParams, sendsClasses);
    }

    //
    // outParams
    //
    returnsClasses = false;
    if(out)
    {
        convertParams(out, outParams, returnsClasses);
    }

    //
    // returnType
    //
    if(ret)
    {
        returnType = convertParam(ret, 0);
        if(!returnsClasses)
        {
            returnsClasses = returnType->type->usesClasses();
        }
    }

    numParams = static_cast<int>(inParams.size() + outParams.size());

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
    if(ex)
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
    if(_zendFunction)
    {
        delete []_zendFunction->arg_info;
        zend_string_release(_zendFunction->function_name);
        efree(_zendFunction);
    }
}

zend_function*
IcePHP::OperationI::function()
{
    if(!_zendFunction)
    {
        //
        // Create an array that indicates how arguments are passed to the operation.
        //
        zend_internal_arg_info* argInfo = new zend_internal_arg_info[numParams];

        int i = 0;
        ParamInfoList::const_iterator p;
        for(p = inParams.begin(); p != inParams.end(); ++p, ++i)
        {
            getArgInfo(argInfo[i], *p, false);
        }
        for(p = outParams.begin(); p != outParams.end(); ++p, ++i)
        {
            getArgInfo(argInfo[i], *p, true);
        }

        string fixed = fixIdent(name);

        _zendFunction = static_cast<zend_internal_function*>(emalloc(sizeof(zend_internal_function)));
        _zendFunction->type = ZEND_INTERNAL_FUNCTION;
        _zendFunction->function_name = zend_string_init(STRCAST(fixed.c_str()), static_cast<uint32_t>(fixed.length()), 0);
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
    Ice::Int i = 0;
    ZEND_HASH_FOREACH_VAL(arr, val)
    {
        ParamInfoPtr param = convertParam(val, i++);
        params.push_back(param);
        if(!param->optional && !usesClasses)
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

    ParamInfoPtr param = new ParamInfo;

    param->type = Wrapper<TypeInfoPtr>::value(zend_hash_index_find(arr, 0));
    param->optional = zend_hash_num_elements(arr) > 1;

    if(param->optional)
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
    if(!info->optional && (SequenceInfoPtr::dynamicCast(info->type) || DictionaryInfoPtr::dynamicCast(info->type)))
    {
        zend_internal_arg_info ai[] =
        {
            ZEND_ARG_ARRAY_INFO(pass_by_ref, 0, allow_null)
        };
        arg = ai[0];
    }
    else
    {
        zend_internal_arg_info ai[] =
        {
            ZEND_ARG_CALLABLE_INFO(pass_by_ref, 0, allow_null)
        };
        arg = ai[0];
    }
}

//
// Invocation
//
IcePHP::Invocation::Invocation(const Ice::ObjectPrx& prx, const CommunicatorInfoPtr& communicator) :
    _prx(prx), _communicator(communicator)
{
}

//
// TypedInvocation
//
IcePHP::TypedInvocation::TypedInvocation(const Ice::ObjectPrx& prx, const CommunicatorInfoPtr& communicator,
                                         const OperationIPtr& op) :
    Invocation(prx, communicator), _op(op)
{
}

bool
IcePHP::TypedInvocation::prepareRequest(int argc, zval* args, Ice::OutputStream* os,
                                        pair<const Ice::Byte*, const Ice::Byte*>& params)
{
    //
    // Verify that the expected number of arguments are supplied. The context argument is optional.
    //
    if(argc != _op->numParams && argc != _op->numParams + 1)
    {
        runtimeError("incorrect number of parameters (%d)", argc);
        return false;
    }

    //
    // The operation's configuration (zend_function) forces out parameters
    // to be passed by reference.
    //
    for(int i = static_cast<int>(_op->inParams.size()); i < _op->numParams; ++i)
    {
        assert(Z_ISREF(args[i]));
    }

    if(!_op->inParams.empty())
    {
        try
        {
            //
            // Marshal the in parameters.
            //
            os->startEncapsulation(_prx->ice_getEncodingVersion(), _op->format);

            ObjectMap objectMap;
            ParamInfoList::iterator p;

            //
            // Validate the supplied arguments.
            //
            for(p = _op->inParams.begin(); p != _op->inParams.end(); ++p)
            {
                ParamInfoPtr info = *p;
                zval* arg = &args[info->pos];
                assert(!Z_ISREF_P(arg));

                if((!info->optional || !isUnset(arg)) && !info->type->validate(arg, false))
                {
                    invalidArgument("invalid value for argument %d in operation `%s'", info->pos + 1,
                                    _op->name.c_str());
                    return false;
                }
            }

            //
            // Marshal the required parameters.
            //
            for(p = _op->inParams.begin(); p != _op->inParams.end(); ++p)
            {
                ParamInfoPtr info = *p;
                if(!info->optional)
                {
                    zval* arg = &args[info->pos];
                    assert(!Z_ISREF_P(arg));

                    info->type->marshal(arg, os, &objectMap, false);
                }
            }

            //
            // Marshal the optional parameters.
            //
            for(p = _op->optionalInParams.begin(); p != _op->optionalInParams.end(); ++p)
            {
                ParamInfoPtr info = *p;
                zval* arg = &args[info->pos];
                assert(!Z_ISREF_P(arg));

                if(!isUnset(arg) && os->writeOptional(info->tag, info->type->optionalFormat()))
                {
                    info->type->marshal(arg, os, &objectMap, true);
                }
            }

            if(_op->sendsClasses)
            {
                os->writePendingValues();
            }

            os->endEncapsulation();
            params = os->finished();
        }
        catch(const AbortMarshaling&)
        {
            return false;
        }
        catch(const Ice::Exception& ex)
        {
            throwException(ex);
            return false;
        }
    }

    return true;
}

void
IcePHP::TypedInvocation::unmarshalResults(int argc, zval* args, zval* ret,
                                          const pair<const Ice::Byte*, const Ice::Byte*>& bytes)
{
    Ice::InputStream is(_communicator->getCommunicator(), bytes);

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
    // These callbacks collect references (copies for unreferenced types) to the unmarshaled values.
    // We copy them into the argument list *after* any pending objects have been unmarshaled.
    //
    ResultCallbackList outParamCallbacks;
    ResultCallbackPtr retCallback;

    outParamCallbacks.resize(_op->outParams.size());

    //
    // Unmarshal the required out parameters.
    //
    for(p = _op->outParams.begin(); p != _op->outParams.end(); ++p)
    {
        ParamInfoPtr info = *p;
        if(!info->optional)
        {
            ResultCallbackPtr cb = new ResultCallback;
            outParamCallbacks[info->pos] = cb;
            info->type->unmarshal(&is, cb, _communicator, 0, 0, false);
        }
    }

    //
    // Unmarshal the required return value, if any.
    //
    if(_op->returnType && !_op->returnType->optional)
    {
        retCallback = new ResultCallback;
        _op->returnType->type->unmarshal(&is, retCallback, _communicator, 0, 0, false);
    }

    //
    // Unmarshal the optional results. This includes an optional return value.
    //
    for(p = _op->optionalOutParams.begin(); p != _op->optionalOutParams.end(); ++p)
    {
        ParamInfoPtr info = *p;

        ResultCallbackPtr cb = new ResultCallback;
        if(_op->returnType && info->tag == _op->returnType->tag)
        {
            retCallback = cb;
        }
        else
        {
            outParamCallbacks[info->pos] = cb;
        }

        if(is.readOptional(info->tag, info->type->optionalFormat()))
        {
            info->type->unmarshal(&is, cb, _communicator, 0, 0, true);
        }
        else
        {
            cb->unset();
        }
    }

    if(_op->returnsClasses)
    {
        is.readPendingValues();
    }

    is.endEncapsulation();

    util.updateSlicedData();

    int i = static_cast<int>(_op->inParams.size());
    for(ResultCallbackList::iterator q = outParamCallbacks.begin(); q != outParamCallbacks.end(); ++q, ++i)
    {
        assert(Z_ISREF(args[i]));

        //
        // We must explicitly destroy the existing contents of all zvals passed
        // as out parameters, otherwise leaks occur.
        //
        zval* arg = Z_REFVAL_P(&args[i]);
        zval_ptr_dtor(arg);
        ZVAL_DUP(arg, &(*q)->zv);
    }

    if(_op->returnType)
    {
        ZVAL_DUP(ret, &retCallback->zv);
    }
}

void
IcePHP::TypedInvocation::unmarshalException(zval* zex, const pair<const Ice::Byte*, const Ice::Byte*>& bytes)
{
    Ice::InputStream is(_communicator->getCommunicator(), bytes);

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
        Ice::UserExceptionFactoryPtr factory = new UserExceptionFactory(_communicator);
        is.throwException(factory);
    }
    catch(const ExceptionReader& r)
    {
        is.endEncapsulation();

        zval* ex = r.getException();
        ExceptionInfoPtr info = r.getInfo();

        if(validateException(info))
        {
            util.updateSlicedData();

            Ice::SlicedDataPtr slicedData = r.getSlicedData();
            if(slicedData)
            {
                StreamUtil::setSlicedDataMember(ex, slicedData);
            }
            ZVAL_DUP(zex, ex);
            return;
        }
        else
        {
            zval_ptr_dtor(ex);
            Ice::UnknownUserException uue(__FILE__, __LINE__,
                                          "operation raised undeclared exception `" + info->id + "'");
            convertException(zex, uue);
            return;
        }
    }

    //
    // Getting here should be impossible: we can get here only if the
    // sender has marshaled a sequence of type IDs, none of which we
    // have a factory for. This means that sender and receiver disagree
    // about the Slice definitions they use.
    //
    Ice::UnknownUserException uue(__FILE__, __LINE__, "unknown exception");
    convertException(zex, uue);
}

bool
IcePHP::TypedInvocation::validateException(const ExceptionInfoPtr& info) const
{
    for(ExceptionInfoList::const_iterator p = _op->exceptions.begin(); p != _op->exceptions.end(); ++p)
    {
        if(info->isA((*p)->id))
        {
            return true;
        }
    }

    return false;
}

void
IcePHP::TypedInvocation::checkTwowayOnly(const Ice::ObjectPrx& proxy) const
{
    if((_op->returnType || !_op->outParams.empty()) && !proxy->ice_isTwoway())
    {
        Ice::TwowayOnlyException ex(__FILE__, __LINE__);
        ex.operation = _op->name;
        throw ex;
    }
}

//
// SyncTypedInvocation
//
IcePHP::SyncTypedInvocation::SyncTypedInvocation(const Ice::ObjectPrx& prx, const CommunicatorInfoPtr& communicator,
                                                 const OperationIPtr& op) :
    Invocation(prx, communicator), TypedInvocation(prx, communicator, op)
{
}

void
IcePHP::SyncTypedInvocation::invoke(INTERNAL_FUNCTION_PARAMETERS)
{
    //
    // Retrieve the arguments.
    //
    zval* args = static_cast<zval*>(emalloc(ZEND_NUM_ARGS() * sizeof(zval)));
    AutoEfree autoArgs(args); // Call efree on return
    if(zend_get_parameters_array_ex(ZEND_NUM_ARGS(), args) == FAILURE)
    {
        runtimeError("unable to get arguments");
        return;
    }

    Ice::OutputStream os(_prx->ice_getCommunicator());
    pair<const Ice::Byte*, const Ice::Byte*> params;
    if(!prepareRequest(ZEND_NUM_ARGS(), args, &os, params))
    {
        return;
    }

    bool hasCtx = false;
    Ice::Context ctx;
    if(ZEND_NUM_ARGS() == static_cast<uint32_t>(_op->numParams) + 1)
    {
        if(!extractStringMap(&args[ZEND_NUM_ARGS() - 1], ctx))
        {
            return;
        }
        hasCtx = true;
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
            if(hasCtx)
            {
                status = _prx->ice_invoke(_op->name, _op->sendMode, params, result, ctx);
            }
            else
            {
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
                pair<const Ice::Byte*, const Ice::Byte*> rb(0, 0);
                if(!result.empty())
                {
                    rb.first = &result[0];
                    rb.second = &result[0] + result.size();
                }

                zval ex;
                unmarshalException(&ex, rb);
                if(!Z_ISUNDEF(ex))
                {
                    zend_throw_exception_object(&ex);
                }
            }
            else if(!_op->outParams.empty() || _op->returnType)
            {
                //
                // Unmarshal the results.
                //
                pair<const Ice::Byte*, const Ice::Byte*> rb(0, 0);
                if(!result.empty())
                {
                    rb.first = &result[0];
                    rb.second = &result[0] + result.size();
                }
                unmarshalResults(ZEND_NUM_ARGS(), args, return_value, rb);
            }
        }
    }
    catch(const AbortMarshaling&)
    {
    }
    catch(const Ice::Exception& ex)
    {
        throwException(ex);
    }
}

ZEND_FUNCTION(IcePHP_defineOperation)
{
    zval* cls;
    char* name;
    size_t nameLen;
    zend_long mode;
    zend_long sendMode;
    zend_long format;
    zval* inParams;
    zval* outParams;
    zval* returnType;
    zval* exceptions;

    if(zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("osllla!a!a!a!"), &cls, &name, &nameLen,
                             &mode, &sendMode, &format, &inParams, &outParams, &returnType, &exceptions) == FAILURE)
    {
        return;
    }

    TypeInfoPtr type = Wrapper<TypeInfoPtr>::value(cls);
    ProxyInfoPtr c = ProxyInfoPtr::dynamicCast(type);
    assert(c);

    OperationIPtr op = new OperationI(name,
                                      static_cast<Ice::OperationMode>(mode),
                                      static_cast<Ice::OperationMode>(sendMode),
                                      static_cast<Ice::FormatType>(format),
                                      inParams, outParams, returnType, exceptions);

    c->addOperation(name, op);
}

ZEND_FUNCTION(IcePHP_Operation_call)
{
    Ice::ObjectPrx proxy;
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
    OperationIPtr opi = OperationIPtr::dynamicCast(op);
    assert(opi);

    InvocationPtr inv = new SyncTypedInvocation(proxy, comm, opi);
    inv->invoke(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
