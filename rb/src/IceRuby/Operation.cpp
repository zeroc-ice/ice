// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Operation.h>
#include <Proxy.h>
#include <Types.h>
#include <Util.h>
#include <Ice/Initialize.h>
#include <Ice/LocalException.h>
#include <Ice/Proxy.h>
#include <Slice/RubyUtil.h>

using namespace std;
using namespace IceRuby;
using namespace Slice::Ruby;

static VALUE _operationClass;

namespace IceRuby
{

class ParamInfo : public UnmarshalCallback
{
public:

    virtual void unmarshaled(VALUE, VALUE, void*);

    TypeInfoPtr type;
};
typedef IceUtil::Handle<ParamInfo> ParamInfoPtr;
typedef vector<ParamInfoPtr> ParamInfoList;

class OperationI : public Operation
{
public:

    OperationI(VALUE, VALUE, VALUE, VALUE, VALUE, VALUE, VALUE, VALUE);

    virtual VALUE invoke(const Ice::ObjectPrx&, VALUE, VALUE);
    virtual void deprecate(const string&);

private:

    string _name;
    Ice::OperationMode _mode;
    Ice::OperationMode _sendMode;
    bool _amd;
    ParamInfoList _inParams;
    ParamInfoList _outParams;
    ParamInfoPtr _returnType;
    ExceptionInfoList _exceptions;
    string _dispatchName;
    bool _sendsClasses;
    bool _returnsClasses;
    string _deprecateMessage;

    void prepareRequest(const Ice::CommunicatorPtr&, VALUE, bool, vector<Ice::Byte>&);
    VALUE unmarshalResults(const vector<Ice::Byte>&, const Ice::CommunicatorPtr&);
    VALUE unmarshalException(const vector<Ice::Byte>&, const Ice::CommunicatorPtr&);
    bool validateException(VALUE) const;
    void checkTwowayOnly(const Ice::ObjectPrx&) const;
};
typedef IceUtil::Handle<OperationI> OperationIPtr;

}

extern "C"
void
IceRuby_Operation_free(OperationPtr* p)
{
    delete p;
}

extern "C"
VALUE
IceRuby_defineOperation(VALUE /*self*/, VALUE name, VALUE mode, VALUE sendMode, VALUE amd, VALUE inParams,
                        VALUE outParams, VALUE returnType, VALUE exceptions)
{
    ICE_RUBY_TRY
    {
        OperationIPtr op = new OperationI(name, mode, sendMode, amd, inParams, outParams, returnType, exceptions);
        return Data_Wrap_Struct(_operationClass, 0, IceRuby_Operation_free, new OperationPtr(op));
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_Operation_invoke(VALUE self, VALUE proxy, VALUE opArgs, VALUE ctx)
{
    ICE_RUBY_TRY
    {
        assert(TYPE(opArgs) == T_ARRAY);

        OperationPtr op = getOperation(self);
        assert(op);
        return op->invoke(getProxy(proxy), opArgs, ctx);
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_Operation_deprecate(VALUE self, VALUE msg)
{
    ICE_RUBY_TRY
    {
        OperationPtr op = getOperation(self);
        assert(op);
        op->deprecate(getString(msg));
    }
    ICE_RUBY_CATCH
    return Qnil;
}

//
// Operation implementation.
//
IceRuby::Operation::~Operation()
{
}

//
// ParamInfo implementation.
//
void
IceRuby::ParamInfo::unmarshaled(VALUE val, VALUE target, void* closure)
{
    assert(TYPE(target) == T_ARRAY);
    long i = reinterpret_cast<long>(closure);
    RARRAY(target)->ptr[i] = val;
}

//
// OperationI implementation.
//
IceRuby::OperationI::OperationI(VALUE name, VALUE mode, VALUE sendMode, VALUE amd, VALUE inParams, VALUE outParams,
                                VALUE returnType, VALUE exceptions)
{
    _name = getString(name);
    _amd = amd == Qtrue;
    if(_amd)
    {
        _dispatchName = fixIdent(_name, IdentNormal) + "_async";
    }
    else
    {
        _dispatchName = fixIdent(_name, IdentNormal);
    }

    //
    // mode
    //
    volatile VALUE modeValue = callRuby(rb_funcall, mode, rb_intern("to_i"), 0);
    assert(TYPE(modeValue) == T_FIXNUM);
    _mode = static_cast<Ice::OperationMode>(FIX2LONG(modeValue));

    //
    // sendMode
    //
    volatile VALUE sendModeValue = callRuby(rb_funcall, sendMode, rb_intern("to_i"), 0);
    assert(TYPE(sendModeValue) == T_FIXNUM);
    _sendMode = static_cast<Ice::OperationMode>(FIX2LONG(sendModeValue));

    long i;

    //
    // inParams
    //
    _sendsClasses = false;
    for(i = 0; i < RARRAY(inParams)->len; ++i)
    {
        ParamInfoPtr param = new ParamInfo;
        param->type = getType(RARRAY(inParams)->ptr[i]);
        _inParams.push_back(param);
        if(!_sendsClasses)
        {
            _sendsClasses = param->type->usesClasses();
        }
    }

    //
    // outParams
    //
    _returnsClasses = false;
    for(i = 0; i < RARRAY(outParams)->len; ++i)
    {
        ParamInfoPtr param = new ParamInfo;
        param->type = getType(RARRAY(outParams)->ptr[i]);
        _outParams.push_back(param);
        if(!_returnsClasses)
        {
            _returnsClasses = param->type->usesClasses();
        }
    }

    //
    // returnType
    //
    if(!NIL_P(returnType))
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
    for(i = 0; i < RARRAY(exceptions)->len; ++i)
    {
        _exceptions.push_back(getException(RARRAY(exceptions)->ptr[i]));
    }
}

VALUE
IceRuby::OperationI::invoke(const Ice::ObjectPrx& proxy, VALUE args, VALUE hctx)
{
    Ice::CommunicatorPtr communicator = proxy->ice_getCommunicator();

    //
    // Marshal the input parameters to a byte sequence.
    //
    Ice::ByteSeq params;
    prepareRequest(communicator, args, false, params);

    if(!_deprecateMessage.empty())
    {
        rb_warning(_deprecateMessage.c_str());
        _deprecateMessage.clear(); // Only show the warning once.
    }

    checkTwowayOnly(proxy);

    //
    // Invoke the operation.
    //
    Ice::ByteSeq result;
    bool status;

    if(!NIL_P(hctx))
    {
        Ice::Context ctx;
        if(!hashToContext(hctx, ctx))
        {
            throw RubyException(rb_eArgError, "context argument must be nil or a hash");
        }

        status = proxy->ice_invoke(_name, _sendMode, params, result, ctx);
    }
    else
    {
        status = proxy->ice_invoke(_name, _sendMode, params, result);
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
            volatile VALUE ex = unmarshalException(result, communicator);
            throw RubyException(ex);
        }
        else if(_outParams.size() > 0 || _returnType)
        {
            //
            // Unmarshal the results. If there is more than one value to be returned, then return them
            // in an array of the form [result, outParam1, ...]. Otherwise just return the value.
            //
            volatile VALUE results = unmarshalResults(result, communicator);

            if(RARRAY(results)->len > 1)
            {
                return results;
            }
            else
            {
                return RARRAY(results)->ptr[0];
            }
        }
    }

    return Qnil;
}

void
IceRuby::OperationI::deprecate(const string& msg)
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

void
IceRuby::OperationI::prepareRequest(const Ice::CommunicatorPtr& communicator, VALUE args, bool async,
                                    vector<Ice::Byte>& bytes)
{
    //
    // Validate the number of arguments.
    //
    long argc = RARRAY(args)->len;
    long paramCount = static_cast<long>(_inParams.size());
    if(argc != paramCount)
    {
        string fixedName = fixIdent(_name, IdentNormal);
        throw RubyException(rb_eArgError, "%s expects %ld in parameters", fixedName.c_str(), paramCount);
    }

    if(!_inParams.empty())
    {
        //
        // Marshal the in parameters.
        //
        Ice::OutputStreamPtr os = Ice::createOutputStream(communicator);

        ObjectMap objectMap;
        long i = 0;
        for(ParamInfoList::iterator p = _inParams.begin(); p != _inParams.end(); ++p, ++i)
        {
            volatile VALUE arg = RARRAY(args)->ptr[i];
            if(!(*p)->type->validate(arg))
            {
                string opName;
                if(async)
                {
                    opName = fixIdent(_name, IdentNormal) + "_async";
                }
                else
                {
                    opName = fixIdent(_name, IdentNormal);
                }
                throw RubyException(rb_eTypeError, "invalid value for argument %ld in operation `%s'",
                                    async ? i + 2 : i + 1, opName.c_str());
            }
            (*p)->type->marshal(arg, os, &objectMap);
        }

        if(_sendsClasses)
        {
            os->writePendingObjects();
        }

        os->finished(bytes);
    }
}

VALUE
IceRuby::OperationI::unmarshalResults(const vector<Ice::Byte>& bytes, const Ice::CommunicatorPtr& communicator)
{
    int i = _returnType ? 1 : 0;
    int numResults = static_cast<int>(_outParams.size()) + i;
    assert(numResults > 0);

    volatile VALUE results = createArray(numResults);

    //
    // Unmarshal the results. If there is more than one value to be returned, then return them
    // in a tuple of the form (result, outParam1, ...). Otherwise just return the value.
    //
    Ice::InputStreamPtr is = Ice::createInputStream(communicator, bytes);
    for(ParamInfoList::iterator p = _outParams.begin(); p != _outParams.end(); ++p, ++i)
    {
        void* closure = reinterpret_cast<void*>(i);
        (*p)->type->unmarshal(is, *p, results, closure);
        RARRAY(results)->len++; // Increment len for each new element to prevent premature GC.
    }

    if(_returnType)
    {
        _returnType->type->unmarshal(is, _returnType, results, 0);
        RARRAY(results)->len++; // Increment len for each new element to prevent premature GC.
    }

    if(_returnsClasses)
    {
        is->readPendingObjects();
    }

    return results;
}

VALUE
IceRuby::OperationI::unmarshalException(const vector<Ice::Byte>& bytes, const Ice::CommunicatorPtr& communicator)
{
    Ice::InputStreamPtr is = Ice::createInputStream(communicator, bytes);

    is->readBool(); // usesClasses

    string id = is->readString();
    while(!id.empty())
    {
        ExceptionInfoPtr info = lookupExceptionInfo(id);
        if(info)
        {
            volatile VALUE ex = info->unmarshal(is);
            if(info->usesClasses)
            {
                is->readPendingObjects();
            }

            if(validateException(ex))
            {
                return ex;
            }
            else
            {
                volatile VALUE cls = CLASS_OF(ex);
                volatile VALUE path = callRuby(rb_class_path, cls);
                assert(TYPE(path) == T_STRING);
                Ice::UnknownUserException e(__FILE__, __LINE__);
                e.unknown = RSTRING(path)->ptr;
                throw e;
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
    // have a factory for. This means that sender and receiver disagree
    // about the Slice definitions they use.
    //
    throw Ice::UnknownUserException(__FILE__, __LINE__);
}

bool
IceRuby::OperationI::validateException(VALUE ex) const
{
    for(ExceptionInfoList::const_iterator p = _exceptions.begin(); p != _exceptions.end(); ++p)
    {
        if(callRuby(rb_obj_is_kind_of, ex, (*p)->rubyClass))
        {
            return true;
        }
    }

    return false;
}

void
IceRuby::OperationI::checkTwowayOnly(const Ice::ObjectPrx& proxy) const
{
    if((_returnType != 0 || !_outParams.empty()) && !proxy->ice_isTwoway())
    {
        Ice::TwowayOnlyException ex(__FILE__, __LINE__);
        ex.operation = _name;
        throw ex;
    }
}

bool
IceRuby::initOperation(VALUE iceModule)
{
    rb_define_module_function(iceModule, "__defineOperation", CAST_METHOD(IceRuby_defineOperation), 8);

    //
    // Define a class to represent an operation.
    //
    _operationClass = rb_define_class_under(iceModule, "IceRuby_Operation", rb_cObject);

    rb_define_method(_operationClass, "invoke", CAST_METHOD(IceRuby_Operation_invoke), 3);
    rb_define_method(_operationClass, "deprecate", CAST_METHOD(IceRuby_Operation_deprecate), 1);

    return true;
}

IceRuby::OperationPtr
IceRuby::getOperation(VALUE obj)
{
    assert(TYPE(obj) == T_DATA);
    assert(rb_obj_is_instance_of(obj, _operationClass) == Qtrue);
    OperationPtr* p = reinterpret_cast<OperationPtr*>(DATA_PTR(obj));
    return *p;
}
