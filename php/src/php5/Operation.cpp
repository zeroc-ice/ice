// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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

    virtual void unmarshaled(zval*, zval*, void* TSRMLS_DC);

    void unset(TSRMLS_D);

    zval* zv;
};
typedef IceUtil::Handle<ResultCallback> ResultCallbackPtr;
typedef vector<ResultCallbackPtr> ResultCallbackList;

//
// Encapsulates attributes of an operation.
//
class OperationI : public Operation
{
public:

    OperationI(const char*, Ice::OperationMode, Ice::OperationMode, Ice::FormatType, zval*, zval*, zval*, zval*
               TSRMLS_DC);
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

    static void convertParams(zval*, ParamInfoList&, bool& TSRMLS_DC);
    static ParamInfoPtr convertParam(zval*, int TSRMLS_DC);
    static void getArgInfo(zend_arg_info&, const ParamInfoPtr&, bool);
};
typedef IceUtil::Handle<OperationI> OperationIPtr;

//
// The base class for client-side invocations.
//
class Invocation : virtual public IceUtil::Shared
{
public:

    Invocation(const Ice::ObjectPrx&, const CommunicatorInfoPtr& TSRMLS_DC);

    virtual void invoke(INTERNAL_FUNCTION_PARAMETERS) = 0;

protected:

    Ice::ObjectPrx _prx;
    CommunicatorInfoPtr _communicator;
#ifdef ZTS
    TSRMLS_D;
#endif
};
typedef IceUtil::Handle<Invocation> InvocationPtr;

//
// TypedInvocation uses the information in the given operation to validate, marshal, and unmarshal
// parameters and exceptions.
//
class TypedInvocation : virtual public Invocation
{
public:

    TypedInvocation(const Ice::ObjectPrx&, const CommunicatorInfoPtr&, const OperationIPtr& TSRMLS_DC);

protected:

    OperationIPtr _op;

    bool prepareRequest(int, zval**, Ice::OutputStreamPtr&, pair<const Ice::Byte*, const Ice::Byte*>& TSRMLS_DC);
    void unmarshalResults(int, zval**, zval*, const pair<const Ice::Byte*, const Ice::Byte*>& TSRMLS_DC);
    zval* unmarshalException(const pair<const Ice::Byte*, const Ice::Byte*>& TSRMLS_DC);
    bool validateException(const ExceptionInfoPtr& TSRMLS_DC) const;
    void checkTwowayOnly(const Ice::ObjectPrx&) const;
};

//
// A synchronous typed invocation.
//
class SyncTypedInvocation : virtual public TypedInvocation
{
public:

    SyncTypedInvocation(const Ice::ObjectPrx&, const CommunicatorInfoPtr&, const OperationIPtr& TSRMLS_DC);

    virtual void invoke(INTERNAL_FUNCTION_PARAMETERS);
};

class UserExceptionReaderFactoryI : public Ice::UserExceptionReaderFactory
{
public:

    UserExceptionReaderFactoryI(const CommunicatorInfoPtr& communicator TSRMLS_DC) :
        _communicator(communicator)
    {
#ifdef ZTS
        this->TSRMLS_C = TSRMLS_C;
#endif
    }

    virtual void createAndThrow(const string& id) const
    {
        ExceptionInfoPtr info = getExceptionInfo(id TSRMLS_CC);
        if(info)
        {
            throw ExceptionReader(_communicator, info TSRMLS_CC);
        }
    }

private:

    const CommunicatorInfoPtr _communicator;
#if ZTS
    TSRMLS_D;
#endif
};

}

//
// ResultCallback implementation.
//
IcePHP::ResultCallback::ResultCallback() :
    zv(0)
{
}

IcePHP::ResultCallback::~ResultCallback()
{
    if(zv)
    {
        zval_ptr_dtor(&zv);
    }
}

void
IcePHP::ResultCallback::unmarshaled(zval* val, zval*, void* TSRMLS_DC)
{
    //
    // Keep a reference to the unmarshaled value.
    //
    zv = val;
    Z_ADDREF_P(zv);
}

void
IcePHP::ResultCallback::unset(TSRMLS_D)
{
    MAKE_STD_ZVAL(zv);
    assignUnset(zv TSRMLS_CC);
}

//
// OperationI implementation.
//
IcePHP::OperationI::OperationI(const char* n, Ice::OperationMode m, Ice::OperationMode sm, Ice::FormatType f, zval* in,
                               zval* out, zval* ret, zval* ex TSRMLS_DC) :
    name(n), mode(m), sendMode(sm), format(f), _zendFunction(0)
{
    //
    // inParams
    //
    sendsClasses = false;
    if(in)
    {
        convertParams(in, inParams, sendsClasses TSRMLS_CC);
    }

    //
    // outParams
    //
    returnsClasses = false;
    if(out)
    {
        convertParams(out, outParams, returnsClasses TSRMLS_CC);
    }

    //
    // returnType
    //
    if(ret)
    {
        returnType = convertParam(ret, 0 TSRMLS_CC);
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
        HashPosition pos;
        zend_hash_internal_pointer_reset_ex(arr, &pos);
        void* data;
        while(zend_hash_get_current_data_ex(arr, &data, &pos) != FAILURE)
        {
            zval** val = reinterpret_cast<zval**>(data);
            ExceptionInfoPtr i = Wrapper<ExceptionInfoPtr>::value(*val TSRMLS_CC);
            exceptions.push_back(i);
            zend_hash_move_forward_ex(arr, &pos);
        }
    }
}

IcePHP::OperationI::~OperationI()
{
    if(_zendFunction)
    {
        delete []_zendFunction->arg_info;
        efree(const_cast<char*>(_zendFunction->function_name));
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
        zend_arg_info* argInfo = new zend_arg_info[numParams];

        int i = 0;
        ParamInfoList::const_iterator p;
        for(p = inParams.begin(); p != inParams.end(); ++p, ++i)
        {
            getArgInfo(argInfo[i], *p, false);
#if PHP_VERSION_ID < 50400
            argInfo[i].required_num_args = static_cast<zend_uint>(numParams);
#endif
        }
        for(p = outParams.begin(); p != outParams.end(); ++p, ++i)
        {
            getArgInfo(argInfo[i], *p, true);
#if PHP_VERSION_ID < 50400
            argInfo[i].required_num_args = static_cast<zend_uint>(numParams);
#endif
        }

        string fixed = fixIdent(name);
        _zendFunction = static_cast<zend_internal_function*>(emalloc(sizeof(zend_internal_function)));
        _zendFunction->type = ZEND_INTERNAL_FUNCTION;
        _zendFunction->function_name = estrndup(STRCAST(fixed.c_str()), static_cast<zend_uint>(fixed.length()));
        _zendFunction->scope = proxyClassEntry;
        _zendFunction->fn_flags = ZEND_ACC_PUBLIC;
        _zendFunction->prototype = 0;
        _zendFunction->num_args = static_cast<zend_uint>(numParams);
        _zendFunction->arg_info = argInfo;
        _zendFunction->required_num_args = _zendFunction->num_args;
#if PHP_VERSION_ID < 50400
        _zendFunction->pass_rest_by_reference = 0;
        _zendFunction->return_reference = 0;
#endif
        _zendFunction->handler = ZEND_FN(IcePHP_Operation_call);
    }

    return reinterpret_cast<zend_function*>(_zendFunction);
}

void
IcePHP::OperationI::convertParams(zval* p, ParamInfoList& params, bool& usesClasses TSRMLS_DC)
{
    assert(Z_TYPE_P(p) == IS_ARRAY);
    HashTable* arr = Z_ARRVAL_P(p);
    HashPosition pos;
    zend_hash_internal_pointer_reset_ex(arr, &pos);
    void* data;
    int i = 0;
    while(zend_hash_get_current_data_ex(arr, &data, &pos) != FAILURE)
    {
        zval** val = reinterpret_cast<zval**>(data);
        ParamInfoPtr param = convertParam(*val, i TSRMLS_CC);
        params.push_back(param);
        if(!param->optional && !usesClasses)
        {
            usesClasses = param->type->usesClasses();
        }
        zend_hash_move_forward_ex(arr, &pos);
        ++i;
    }
}

ParamInfoPtr
IcePHP::OperationI::convertParam(zval* p, int pos TSRMLS_DC)
{
    assert(Z_TYPE_P(p) == IS_ARRAY);
    HashTable* arr = Z_ARRVAL_P(p);
    assert(zend_hash_num_elements(arr) == 3);

    ParamInfoPtr param = new ParamInfo;
    zval** m;

    zend_hash_index_find(arr, 0, reinterpret_cast<void**>(&m));
    param->type = Wrapper<TypeInfoPtr>::value(*m TSRMLS_CC);
    zend_hash_index_find(arr, 1, reinterpret_cast<void**>(&m));
    assert(Z_TYPE_PP(m) == IS_BOOL);
    param->optional = Z_BVAL_PP(m) ? true : false;
    zend_hash_index_find(arr, 2, reinterpret_cast<void**>(&m));
    assert(Z_TYPE_PP(m) == IS_LONG);
    param->tag = Z_LVAL_PP(m);
    param->pos = pos;

    return param;
}

void
IcePHP::OperationI::getArgInfo(zend_arg_info& arg, const ParamInfoPtr& info, bool out)
{
    arg.name = 0;
    arg.class_name = 0;
    arg.allow_null = 1;

    if(!info->optional)
    {
        const bool isArray = SequenceInfoPtr::dynamicCast(info->type) || DictionaryInfoPtr::dynamicCast(info->type);

#if PHP_VERSION_ID < 50400
        arg.array_type_hint = isArray ? 1 : 0;
        arg.return_reference = 0;
#else
        arg.type_hint = isArray ? IS_ARRAY : 0;
#endif
    }
    else
    {
#if PHP_VERSION_ID < 50400
        arg.array_type_hint = 0;
        arg.return_reference = 0;
#else
        arg.type_hint = 0;
#endif
    }

    arg.pass_by_reference = out ? 1 : 0;
}

//
// Invocation
//
IcePHP::Invocation::Invocation(const Ice::ObjectPrx& prx, const CommunicatorInfoPtr& communicator TSRMLS_DC) :
    _prx(prx), _communicator(communicator)
{
#ifdef ZTS
    this->TSRMLS_C = TSRMLS_C;
#endif
}

//
// TypedInvocation
//
IcePHP::TypedInvocation::TypedInvocation(const Ice::ObjectPrx& prx, const CommunicatorInfoPtr& communicator,
                                         const OperationIPtr& op TSRMLS_DC) :
    Invocation(prx, communicator TSRMLS_CC), _op(op)
{
}

bool
IcePHP::TypedInvocation::prepareRequest(int argc, zval** args, Ice::OutputStreamPtr& os, 
                                        pair<const Ice::Byte*, const Ice::Byte*>& params TSRMLS_DC)
{
    //
    // Verify that the expected number of arguments are supplied. The context argument is optional.
    //
    if(argc != _op->numParams && argc != _op->numParams + 1)
    {
        runtimeError("incorrect number of parameters (%d)" TSRMLS_CC, argc);
        return false;
    }

    //
    // The operation's configuration (zend_function) forces out parameters
    // to be passed by reference.
    //
    for(int i = static_cast<int>(_op->inParams.size()); i < _op->numParams; ++i)
    {
        assert(PZVAL_IS_REF(args[i]));
    }

    if(!_op->inParams.empty())
    {
        try
        {
            //
            // Marshal the in parameters.
            //
            os = Ice::createOutputStream(_communicator->getCommunicator());
            os->startEncapsulation(_prx->ice_getEncodingVersion(), _op->format);

            ObjectMap objectMap;
            ParamInfoList::iterator p;

            //
            // Validate the supplied arguments.
            //
            for(p = _op->inParams.begin(); p != _op->inParams.end(); ++p)
            {
                ParamInfoPtr info = *p;
                zval* arg = args[info->pos];
                if((!info->optional || !isUnset(arg TSRMLS_CC)) && !info->type->validate(arg TSRMLS_CC))
                {
                    invalidArgument("invalid value for argument %d in operation `%s'" TSRMLS_CC, info->pos + 1,
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
                    zval* arg = args[info->pos];
                    info->type->marshal(arg, os, &objectMap, false TSRMLS_CC);
                }
            }

            //
            // Marshal the optional parameters.
            //
            for(p = _op->optionalInParams.begin(); p != _op->optionalInParams.end(); ++p)
            {
                ParamInfoPtr info = *p;
                zval* arg = args[info->pos];
                if(!isUnset(arg TSRMLS_CC) && os->writeOptional(info->tag, info->type->optionalFormat()))
                {
                    info->type->marshal(arg, os, &objectMap, true TSRMLS_CC);
                }
            }

            if(_op->sendsClasses)
            {
                os->writePendingObjects();
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
            throwException(ex TSRMLS_CC);
            return false;
        }
    }

    return true;
}

void
IcePHP::TypedInvocation::unmarshalResults(int argc, zval** args, zval* ret,
                                          const pair<const Ice::Byte*, const Ice::Byte*>& bytes TSRMLS_DC)
{
    Ice::InputStreamPtr is = Ice::wrapInputStream(_communicator->getCommunicator(), bytes);

    //
    // Store a pointer to a local SlicedDataUtil object as the stream's closure.
    // This is necessary to support object unmarshaling (see ObjectReader).
    //
    SlicedDataUtil util;
    assert(!is->closure());
    is->closure(&util);

    is->startEncapsulation();

    ParamInfoList::iterator p;

    //
    // These callbacks collect references to the unmarshaled values. We copy them into
    // the argument list *after* any pending objects have been unmarshaled.
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
            info->type->unmarshal(is, cb, _communicator, 0, 0, false TSRMLS_CC);
        }
    }

    //
    // Unmarshal the required return value, if any.
    //
    if(_op->returnType && !_op->returnType->optional)
    {
        retCallback = new ResultCallback;
        _op->returnType->type->unmarshal(is, retCallback, _communicator, 0, 0, false TSRMLS_CC);
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

        if(is->readOptional(info->tag, info->type->optionalFormat()))
        {
            info->type->unmarshal(is, cb, _communicator, 0, 0, true TSRMLS_CC);
        }
        else
        {
            cb->unset(TSRMLS_C);
        }
    }

    if(_op->returnsClasses)
    {
        is->readPendingObjects();
    }

    is->endEncapsulation();

    util.update(TSRMLS_C);

    int i = static_cast<int>(_op->inParams.size());
    for(ResultCallbackList::iterator q = outParamCallbacks.begin(); q != outParamCallbacks.end(); ++q, ++i)
    {
        //
        // We must explicitly destroy the existing contents of all zvals passed
        // as out parameters, otherwise leaks occur.
        //
        zval* val = (*q)->zv;
        zval_dtor(args[i]);
        args[i]->value = val->value;
        Z_TYPE_P(args[i]) = Z_TYPE_P(val);
        zval_copy_ctor(args[i]);
    }

    if(_op->returnType)
    {
        ret->value = retCallback->zv->value;
        Z_TYPE_P(ret) = Z_TYPE_P(retCallback->zv);
        zval_copy_ctor(ret);
    }
}

zval*
IcePHP::TypedInvocation::unmarshalException(const pair<const Ice::Byte*, const Ice::Byte*>& bytes TSRMLS_DC)
{
    Ice::InputStreamPtr is = Ice::wrapInputStream(_communicator->getCommunicator(), bytes);

    //
    // Store a pointer to a local SlicedDataUtil object as the stream's closure.
    // This is necessary to support object unmarshaling (see ObjectReader).
    //
    SlicedDataUtil util;
    assert(!is->closure());
    is->closure(&util);

    is->startEncapsulation();

    try
    {
        Ice::UserExceptionReaderFactoryPtr factory = new UserExceptionReaderFactoryI(_communicator TSRMLS_CC);
        is->throwException(factory);
    }
    catch(const ExceptionReader& r)
    {
        is->endEncapsulation();

        zval* ex = r.getException();
        ExceptionInfoPtr info = r.getInfo();

        if(validateException(info TSRMLS_CC))
        {
            util.update(TSRMLS_C);

            Ice::SlicedDataPtr slicedData = r.getSlicedData();
            if(slicedData)
            {
                SlicedDataUtil::setMember(ex, slicedData TSRMLS_CC);
            }

            return ex;
        }
        else
        {
            zval_ptr_dtor(&ex);
            Ice::UnknownUserException uue(__FILE__, __LINE__,
                                          "operation raised undeclared exception `" + info->id + "'");
            return convertException(uue TSRMLS_CC);
        }
    }

    //
    // Getting here should be impossible: we can get here only if the
    // sender has marshaled a sequence of type IDs, none of which we
    // have a factory for. This means that sender and receiver disagree
    // about the Slice definitions they use.
    //
    Ice::UnknownUserException uue(__FILE__, __LINE__, "unknown exception");
    return convertException(uue TSRMLS_CC);
}

bool
IcePHP::TypedInvocation::validateException(const ExceptionInfoPtr& info TSRMLS_DC) const
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
                                                 const OperationIPtr& op TSRMLS_DC) :
    Invocation(prx, communicator TSRMLS_CC), TypedInvocation(prx, communicator, op TSRMLS_CC)
{
}

void
IcePHP::SyncTypedInvocation::invoke(INTERNAL_FUNCTION_PARAMETERS)
{
    //
    // Retrieve the arguments.
    //
    zval*** args = static_cast<zval***>(emalloc(ZEND_NUM_ARGS() * sizeof(zval**)));
    AutoEfree autoArgs(args); // Call efree on return
    if(zend_get_parameters_array_ex(ZEND_NUM_ARGS(), args) == FAILURE)
    {
        runtimeError("unable to get arguments" TSRMLS_CC);
        return;
    }
    
    Ice::OutputStreamPtr os;
    pair<const Ice::Byte*, const Ice::Byte*> params;
    if(!prepareRequest(ZEND_NUM_ARGS(), *args, os, params TSRMLS_CC))
    {
        return;
    }

    bool hasCtx = false;
    Ice::Context ctx;
    if(ZEND_NUM_ARGS() == _op->numParams + 1)
    {
        if(!extractStringMap(*args[ZEND_NUM_ARGS() - 1], ctx TSRMLS_CC))
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

                zval* ex = unmarshalException(rb TSRMLS_CC);
                if(ex)
                {
                    zend_throw_exception_object(ex TSRMLS_CC);
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
                unmarshalResults(ZEND_NUM_ARGS(), *args, return_value, rb TSRMLS_CC);
            }
        }
    }
    catch(const AbortMarshaling&)
    {
    }
    catch(const Ice::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
    }
}

ZEND_FUNCTION(IcePHP_defineOperation)
{
    zval* cls;
    char* name;
    int nameLen;
    long mode;
    long sendMode;
    long format;
    zval* inParams;
    zval* outParams;
    zval* returnType;
    zval* exceptions;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, const_cast<char*>("osllla!a!a!a!"), &cls, &name, &nameLen,
                             &mode, &sendMode, &format, &inParams, &outParams, &returnType, &exceptions) == FAILURE)
    {
        return;
    }

    TypeInfoPtr type = Wrapper<TypeInfoPtr>::value(cls TSRMLS_CC);
    ClassInfoPtr c = ClassInfoPtr::dynamicCast(type);
    assert(c);

    OperationIPtr op = new OperationI(name, static_cast<Ice::OperationMode>(mode),
                                      static_cast<Ice::OperationMode>(sendMode), static_cast<Ice::FormatType>(format),
                                      inParams, outParams, returnType, exceptions TSRMLS_CC);

    c->addOperation(name, op);
}

ZEND_FUNCTION(IcePHP_Operation_call)
{
    Ice::ObjectPrx proxy;
    ClassInfoPtr cls;
    CommunicatorInfoPtr comm;
#ifndef NDEBUG
    bool b =
#endif
    fetchProxy(getThis(), proxy, cls, comm TSRMLS_CC);
    assert(b);
    assert(proxy);
    assert(cls);

    OperationPtr op = cls->getOperation(get_active_function_name(TSRMLS_C));
    assert(op); // handleGetMethod should have already verified the operation's existence.
    OperationIPtr opi = OperationIPtr::dynamicCast(op);
    assert(opi);

    InvocationPtr inv = new SyncTypedInvocation(proxy, comm, opi TSRMLS_CC);
    inv->invoke(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
