// **********************************************************************
//
// Copyright (c) 2003-2010 ZeroC, Inc. All rights reserved.
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

//
// Receives an out parameter or return value.
//
class ResultCallback : public UnmarshalCallback
{
public:

    ResultCallback();
    ~ResultCallback();

    virtual void unmarshaled(zval*, zval*, void* TSRMLS_DC);

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

    OperationI(const char*, Ice::OperationMode, Ice::OperationMode, zval*, zval*, zval*, zval* TSRMLS_DC);
    ~OperationI();

    virtual zend_function* function();

    string name; // On-the-wire name.
    Ice::OperationMode mode;
    Ice::OperationMode sendMode;
    TypeInfoList inParams;
    TypeInfoList outParams;
    TypeInfoPtr returnType;
    ExceptionInfoList exceptions;
    bool sendsClasses;
    bool returnsClasses;
    int numParams;

private:

    zend_internal_function* _zendFunction;

    static void convertParams(zval*, TypeInfoList&, bool& TSRMLS_DC);
    static void getArgInfo(zend_arg_info&, const TypeInfoPtr&, bool);
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

    bool prepareRequest(int, zval**, Ice::ByteSeq& TSRMLS_DC);
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

//
// OperationI implementation.
//
IcePHP::OperationI::OperationI(const char* n, Ice::OperationMode m, Ice::OperationMode sm, zval* in, zval* out,
                               zval* ret, zval* ex TSRMLS_DC) :
    name(n), mode(m), sendMode(sm), sendsClasses(false), returnsClasses(false), _zendFunction(0)
{
    //
    // inParams
    //
    if(in)
    {
        convertParams(in, inParams, sendsClasses TSRMLS_CC);
    }

    //
    // outParams
    //
    if(out)
    {
        convertParams(out, outParams, returnsClasses TSRMLS_CC);
    }

    numParams = static_cast<int>(inParams.size() + outParams.size());

    //
    // returnType
    //
    if(ret)
    {
        returnType = Wrapper<TypeInfoPtr>::value(ret TSRMLS_CC);
        if(!returnsClasses)
        {
            returnsClasses = returnType->usesClasses();
        }
    }

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
        efree(_zendFunction->function_name);
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
        TypeInfoList::const_iterator p;
        for(p = inParams.begin(); p != inParams.end(); ++p, ++i)
        {
            getArgInfo(argInfo[i], *p, false);
            argInfo[i].required_num_args = static_cast<zend_uint>(numParams);
        }
        for(p = outParams.begin(); p != outParams.end(); ++p, ++i)
        {
            getArgInfo(argInfo[i], *p, true);
            argInfo[i].required_num_args = static_cast<zend_uint>(numParams);
        }

        string fixed = fixIdent(name);
        _zendFunction = static_cast<zend_internal_function*>(emalloc(sizeof(zend_internal_function)));
        _zendFunction->type = ZEND_INTERNAL_FUNCTION;
        _zendFunction->function_name = estrndup(STRCAST(fixed.c_str()), fixed.length());
        _zendFunction->scope = proxyClassEntry;
        _zendFunction->fn_flags = ZEND_ACC_PUBLIC;
        _zendFunction->prototype = 0;
        _zendFunction->num_args = static_cast<zend_uint>(numParams);
        _zendFunction->arg_info = argInfo;
        _zendFunction->pass_rest_by_reference = 0;
        _zendFunction->required_num_args = _zendFunction->num_args;
        _zendFunction->return_reference = 0;
        _zendFunction->handler = ZEND_FN(IcePHP_Operation_call);
    }

    return reinterpret_cast<zend_function*>(_zendFunction);
}

void
IcePHP::OperationI::convertParams(zval* p, TypeInfoList& params, bool& usesClasses TSRMLS_DC)
{
    usesClasses = false;

    assert(Z_TYPE_P(p) == IS_ARRAY);
    HashTable* arr = Z_ARRVAL_P(p);
    HashPosition pos;
    zend_hash_internal_pointer_reset_ex(arr, &pos);
    void* data;
    while(zend_hash_get_current_data_ex(arr, &data, &pos) != FAILURE)
    {
        zval** val = reinterpret_cast<zval**>(data);
        TypeInfoPtr type = Wrapper<TypeInfoPtr>::value(*val TSRMLS_CC);
        params.push_back(type);
        if(!usesClasses)
        {
            usesClasses = type->usesClasses();
        }
        zend_hash_move_forward_ex(arr, &pos);
    }
}

void
IcePHP::OperationI::getArgInfo(zend_arg_info& arg, const TypeInfoPtr& info, bool out)
{
    arg.name = 0;
    arg.class_name = 0;
    arg.allow_null = 1;
    if(SequenceInfoPtr::dynamicCast(info) || DictionaryInfoPtr::dynamicCast(info))
    {
        arg.array_type_hint = 1;
    }
    else
    {
        arg.array_type_hint = 0;
    }
    arg.return_reference = 0;
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
IcePHP::TypedInvocation::prepareRequest(int argc, zval** args, Ice::ByteSeq& bytes TSRMLS_DC)
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
            Ice::OutputStreamPtr os = Ice::createOutputStream(_communicator->getCommunicator());

            ObjectMap objectMap;
            int i = 0;
            for(TypeInfoList::iterator p = _op->inParams.begin(); p != _op->inParams.end(); ++p, ++i)
            {
                zval* arg = args[i];
                if(!(*p)->validate(arg TSRMLS_CC))
                {
                    invalidArgument("invalid value for argument %d in operation `%s'" TSRMLS_CC, i, _op->name.c_str());
                    return false;
                }
                (*p)->marshal(arg, os, &objectMap TSRMLS_CC);
            }

            if(_op->sendsClasses)
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
    Ice::InputStreamPtr is = Ice::createInputStream(_communicator->getCommunicator(), bytes);

    //
    // These callbacks collect references to the unmarshaled values. We copy them into
    // the argument list *after* any pending objects have been unmarshaled.
    //
    ResultCallbackList outParamCallbacks;
    ResultCallbackPtr retCallback;

    //
    // Unmarshal the out parameters.
    //
    for(TypeInfoList::iterator p = _op->outParams.begin(); p != _op->outParams.end(); ++p)
    {
        ResultCallbackPtr cb = new ResultCallback;
        outParamCallbacks.push_back(cb);
        (*p)->unmarshal(is, cb, _communicator, 0, 0 TSRMLS_CC);
    }

    //
    // Unmarshal the return value.
    //
    if(_op->returnType)
    {
        retCallback = new ResultCallback;
        _op->returnType->unmarshal(is, retCallback, _communicator, 0, 0 TSRMLS_CC);
    }

    if(_op->returnsClasses)
    {
        is->readPendingObjects();
    }

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
    int traceSlicing = -1;

    Ice::InputStreamPtr is = Ice::createInputStream(_communicator->getCommunicator(), bytes);

    bool usesClasses;
    is->read(usesClasses);

    string id;
    is->read(id);
    const string origId = id;

    while(!id.empty())
    {
        ExceptionInfoPtr info = getExceptionInfo(id TSRMLS_CC);
        if(info)
        {
            zval* ex = info->unmarshal(is, _communicator TSRMLS_CC);
            if(ex)
            {
                if(info->usesClasses)
                {
                    is->readPendingObjects();
                }

                if(validateException(info TSRMLS_CC))
                {
                    return ex;
                }
                else
                {
                    zval_ptr_dtor(&ex);
                    Ice::UnknownUserException uue(__FILE__, __LINE__,
                                                  "operation raised undeclared exception `" + id + "'");
                    return convertException(uue TSRMLS_CC);
                }
            }
        }
        else
        {
            if(traceSlicing == -1)
            {
                traceSlicing =
                    _communicator->getCommunicator()->getProperties()->getPropertyAsInt("Ice.Trace.Slicing") > 0;
            }

            if(traceSlicing > 0)
            {
                _communicator->getCommunicator()->getLogger()->trace("Slicing", "unknown exception type `" + id + "'");
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
                return convertException(ex TSRMLS_CC);
            }
        }
    }

    //
    // Getting here should be impossible: we can get here only if the
    // sender has marshaled a sequence of type IDs, none of which we
    // have a factory for. This means that sender and receiver disagree
    // about the Slice definitions they use.
    //
    Ice::UnknownUserException uue(__FILE__, __LINE__, "unknown exception type `" + origId + "'");
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

    Ice::ByteSeq params;
    if(!prepareRequest(ZEND_NUM_ARGS(), *args, params TSRMLS_CC))
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
    zval* inParams;
    zval* outParams;
    zval* returnType;
    zval* exceptions;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, const_cast<char*>("oslla!a!o!a!"), &cls, &name, &nameLen, &mode,
                             &sendMode, &inParams, &outParams, &returnType, &exceptions) == FAILURE)
    {
        return;
    }

    TypeInfoPtr type = Wrapper<TypeInfoPtr>::value(cls TSRMLS_CC);
    ClassInfoPtr c = ClassInfoPtr::dynamicCast(type);
    assert(c);

    OperationIPtr op = new OperationI(name, static_cast<Ice::OperationMode>(mode),
                                      static_cast<Ice::OperationMode>(sendMode), inParams, outParams, returnType,
                                      exceptions TSRMLS_CC);

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
