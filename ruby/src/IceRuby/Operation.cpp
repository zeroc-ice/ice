// Copyright (c) ZeroC, Inc.

#include "Operation.h"
#include "Communicator.h"
#include "Ice/Communicator.h"
#include "Ice/Initialize.h"
#include "Ice/LocalExceptions.h"
#include "Ice/Logger.h"
#include "Ice/Properties.h"
#include "Ice/Proxy.h"
#include "Proxy.h"
#include "Types.h"
#include "Util.h"
#include "slice2rb/RubyUtil.h"

#include <memory>

using namespace std;
using namespace IceRuby;

static VALUE _operationClass;

namespace IceRuby
{
    class ParamInfo final : public UnmarshalCallback
    {
    public:
        void unmarshaled(VALUE, VALUE, void*) final;

        TypeInfoPtr type;
        bool optional;
        int tag;
        int pos;
    };
    using ParamInfoPtr = shared_ptr<ParamInfo>;
    using ParamInfoList = list<ParamInfoPtr>;

    class OperationI final : public Operation
    {
    public:
        OperationI(VALUE, VALUE, VALUE, VALUE, VALUE, VALUE, VALUE, VALUE);

        VALUE invoke(const Ice::ObjectPrx&, VALUE, VALUE) final;
        void deprecate(const string&) final;

    private:
        string _sliceName;
        string _mappedName;
        Ice::OperationMode _mode;
        std::optional<Ice::FormatType> _format;
        ParamInfoList _inParams;
        ParamInfoList _optionalInParams;
        ParamInfoList _outParams;
        ParamInfoList _optionalOutParams;
        ParamInfoPtr _returnType;
        ExceptionInfoList _exceptions;
        bool _sendsClasses;
        bool _returnsClasses;
        string _deprecateMessage;

        void convertParams(VALUE, ParamInfoList&, long, bool&);
        ParamInfoPtr convertParam(VALUE, long);
        void prepareRequest(const Ice::ObjectPrx&, VALUE, Ice::OutputStream*, pair<const byte*, const byte*>&);
        VALUE unmarshalResults(const vector<byte>&, const Ice::CommunicatorPtr&);
        VALUE unmarshalException(const vector<byte>&, const Ice::CommunicatorPtr&);
        bool validateException(VALUE) const;
        void checkTwowayOnly(const Ice::ObjectPrx&) const;
    };
    using OperationIPtr = shared_ptr<OperationI>;
}

extern "C" void
IceRuby_Operation_free(void* p)
{
    delete static_cast<OperationPtr*>(p);
}

static const rb_data_type_t IceRuby_OperationType = {
    .wrap_struct_name = "Ice::Operation",
    .function =
        {
            .dfree = IceRuby_Operation_free,
        },
    .flags = RUBY_TYPED_FREE_IMMEDIATELY,
};

extern "C" VALUE
IceRuby_defineOperation(
    VALUE /*self*/,
    VALUE sliceName,
    VALUE mappedName,
    VALUE mode,
    VALUE format,
    VALUE inParams,
    VALUE outParams,
    VALUE returnType,
    VALUE exceptions)
{
    ICE_RUBY_TRY
    {
        OperationIPtr op =
            make_shared<OperationI>(sliceName, mappedName, mode, format, inParams, outParams, returnType, exceptions);
        return TypedData_Wrap_Struct(_operationClass, &IceRuby_OperationType, new OperationPtr(op));
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C" VALUE
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

extern "C" VALUE
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
IceRuby::Operation::~Operation() = default;

//
// ParamInfo implementation.
//
void
IceRuby::ParamInfo::unmarshaled(VALUE val, VALUE target, void* closure)
{
    assert(TYPE(target) == T_ARRAY);
    static_assert(sizeof(long) == sizeof(void*), "long and void* must have the same size");
    long i = reinterpret_cast<long>(closure);
    RARRAY_ASET(target, i, val);
}

//
// OperationI implementation.
//
IceRuby::OperationI::OperationI(
    VALUE name,
    VALUE mapped,
    VALUE mode,
    VALUE format,
    VALUE inParams,
    VALUE outParams,
    VALUE returnType,
    VALUE exceptions)
{
    _sliceName = getString(name);
    _mappedName = getString(mapped);

    //
    // mode
    //
    volatile VALUE modeValue = callRuby(rb_funcall, mode, rb_intern("to_i"), 0);
    assert(TYPE(modeValue) == T_FIXNUM);
    _mode = static_cast<Ice::OperationMode>(FIX2LONG(modeValue));

    //
    // format
    //
    if (format == Qnil)
    {
        _format = std::nullopt;
    }
    else
    {
        volatile VALUE formatValue = callRuby(rb_funcall, format, rb_intern("to_i"), 0);
        assert(TYPE(formatValue) == T_FIXNUM);
        _format = static_cast<Ice::FormatType>(FIX2LONG(formatValue));
    }

    //
    // returnType
    //
    _returnsClasses = false;
    if (!NIL_P(returnType))
    {
        _returnType = convertParam(returnType, 0);
        _returnsClasses = _returnType->type->usesClasses();
    }

    //
    // inParams
    //
    _sendsClasses = false;
    convertParams(inParams, _inParams, 0, _sendsClasses);

    //
    // outParams
    //
    convertParams(outParams, _outParams, NIL_P(returnType) ? 0 : 1, _returnsClasses);

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
    ParamInfoList l = _inParams;
    copy(l.begin(), remove_if(l.begin(), l.end(), SortFn::isRequired), back_inserter(_optionalInParams));
    _optionalInParams.sort(SortFn::compare);

    //
    // The outParams list represents the parameters in the order of declaration.
    // We also need a sorted list of optional parameters. If the return value is
    // optional, we must include it in this list.
    //
    l = _outParams;
    copy(l.begin(), remove_if(l.begin(), l.end(), SortFn::isRequired), back_inserter(_optionalOutParams));
    if (_returnType && _returnType->optional)
    {
        _optionalOutParams.push_back(_returnType);
    }
    _optionalOutParams.sort(SortFn::compare);

    //
    // exceptions
    //
    for (long i = 0; i < RARRAY_LEN(exceptions); ++i)
    {
        _exceptions.push_back(getException(RARRAY_AREF(exceptions, i)));
    }
}

VALUE
IceRuby::OperationI::invoke(const Ice::ObjectPrx& proxy, VALUE args, VALUE hctx)
{
    Ice::CommunicatorPtr communicator = proxy->ice_getCommunicator();

    //
    // Marshal the input parameters to a byte sequence.
    //
    Ice::OutputStream os(communicator);
    pair<const byte*, const byte*> params;
    prepareRequest(proxy, args, &os, params);

    if (!_deprecateMessage.empty())
    {
        rb_warning("%s", _deprecateMessage.c_str());
        _deprecateMessage.clear(); // Only show the warning once.
    }

    checkTwowayOnly(proxy);

    //
    // Invoke the operation.
    //
    Ice::ByteSeq result;
    bool status;

    if (!NIL_P(hctx))
    {
        Ice::Context ctx;
        if (!hashToContext(hctx, ctx))
        {
            throw RubyException(rb_eArgError, "context argument must be nil or a hash");
        }

        status = proxy->ice_invoke(_sliceName, _mode, params, result, ctx);
    }
    else
    {
        status = proxy->ice_invoke(_sliceName, _mode, params, result);
    }

    //
    // Process the reply.
    //
    if (proxy->ice_isTwoway())
    {
        if (!status)
        {
            //
            // Unmarshal a user exception.
            //
            volatile VALUE ex = unmarshalException(result, communicator);
            throw RubyException(ex);
        }
        else if (_outParams.size() > 0 || _returnType)
        {
            //
            // Unmarshal the results. If there is more than one value to be returned, then return them
            // in an array of the form [result, outParam1, ...]. Otherwise just return the value.
            //
            volatile VALUE results = unmarshalResults(result, communicator);

            if (RARRAY_LEN(results) > 1)
            {
                return results;
            }
            else
            {
                return RARRAY_AREF(results, 0);
            }
        }
    }

    return Qnil;
}

void
IceRuby::OperationI::deprecate(const string& msg)
{
    if (!msg.empty())
    {
        _deprecateMessage = msg;
    }
    else
    {
        _deprecateMessage = "operation " + _mappedName + " is deprecated";
    }
}

void
IceRuby::OperationI::convertParams(VALUE v, ParamInfoList& params, long posOffset, bool& usesClasses)
{
    assert(TYPE(v) == T_ARRAY);

    for (long i = 0; i < RARRAY_LEN(v); ++i)
    {
        ParamInfoPtr param = convertParam(RARRAY_AREF(v, i), i + posOffset);
        params.push_back(param);
        if (!param->optional && !usesClasses)
        {
            usesClasses = param->type->usesClasses();
        }
    }
}

ParamInfoPtr
IceRuby::OperationI::convertParam(VALUE v, long pos)
{
    assert(TYPE(v) == T_ARRAY);
    ParamInfoPtr param = make_shared<ParamInfo>();
    param->type = getType(RARRAY_AREF(v, 0));
    param->optional = static_cast<bool>(RTEST(RARRAY_AREF(v, 1)));
    param->tag = static_cast<int>(getInteger(RARRAY_AREF(v, 2)));
    param->pos = static_cast<int>(pos);
    return param;
}

void
IceRuby::OperationI::prepareRequest(
    const Ice::ObjectPrx& proxy,
    VALUE args,
    Ice::OutputStream* os,
    pair<const byte*, const byte*>& params)
{
    params.first = params.second = static_cast<const byte*>(0);

    //
    // Validate the number of arguments.
    //
    long argc = RARRAY_LEN(args);
    long paramCount = static_cast<long>(_inParams.size());
    if (argc != paramCount)
    {
        throw RubyException(rb_eArgError, "%s expects %ld in parameters", _mappedName.c_str(), paramCount);
    }

    if (!_inParams.empty())
    {
        //
        // Marshal the in parameters.
        //
        os->startEncapsulation(proxy->ice_getEncodingVersion(), _format);

        ValueMap valueMap;
        ParamInfoList::iterator p;

        //
        // Validate the supplied arguments.
        //
        for (p = _inParams.begin(); p != _inParams.end(); ++p)
        {
            ParamInfoPtr info = *p;
            volatile VALUE arg = RARRAY_AREF(args, info->pos);
            if ((!info->optional || arg != Qnil) && !info->type->validate(arg))
            {
                throw RubyException(
                    rb_eTypeError,
                    "invalid value for argument %ld in operation `%s'",
                    info->pos + 1,
                    _mappedName.c_str());
            }
        }

        //
        // Marshal the required parameters.
        //
        for (p = _inParams.begin(); p != _inParams.end(); ++p)
        {
            ParamInfoPtr info = *p;
            if (!info->optional)
            {
                volatile VALUE arg = RARRAY_AREF(args, info->pos);
                info->type->marshal(arg, os, &valueMap, false);
            }
        }

        //
        // Marshal the optional parameters.
        //
        for (p = _optionalInParams.begin(); p != _optionalInParams.end(); ++p)
        {
            ParamInfoPtr info = *p;
            volatile VALUE arg = RARRAY_AREF(args, info->pos);
            if (arg != Qnil && os->writeOptional(info->tag, info->type->optionalFormat()))
            {
                info->type->marshal(arg, os, &valueMap, true);
            }
        }

        if (_sendsClasses)
        {
            os->writePendingValues();
        }

        os->endEncapsulation();
        params = os->finished();
    }
}

VALUE
IceRuby::OperationI::unmarshalResults(const vector<byte>& bytes, const Ice::CommunicatorPtr& communicator)
{
    int numResults = static_cast<int>(_outParams.size());
    if (_returnType)
    {
        numResults++;
    }
    assert(numResults > 0);

    volatile VALUE results = createArray(numResults);

    //
    // Unmarshal the results. If there is more than one value to be returned, then return them
    // in a tuple of the form (result, outParam1, ...). Otherwise just return the value.
    //
    Ice::InputStream is{communicator, bytes, lookupSliceLoader(communicator)};

    //
    // Store a pointer to a local StreamUtil object as the stream's closure.
    // This is necessary to support object unmarshaling (see ValueReader).
    //
    StreamUtil util;
    assert(!is.getClosure());
    is.setClosure(&util);

    is.startEncapsulation();

    ParamInfoList::iterator p;

    //
    // Unmarshal the required out parameters.
    //
    for (p = _outParams.begin(); p != _outParams.end(); ++p)
    {
        ParamInfoPtr info = *p;
        if (!info->optional)
        {
            void* closure = reinterpret_cast<void*>(info->pos);
            info->type->unmarshal(&is, info, results, closure, false);
        }
    }

    //
    // Unmarshal the required return value, if any.
    //
    if (_returnType && !_returnType->optional)
    {
        assert(_returnType->pos == 0);
        void* closure = reinterpret_cast<void*>(_returnType->pos);
        _returnType->type->unmarshal(&is, _returnType, results, closure, false);
    }

    //
    // Unmarshal the optional results. This includes an optional return value.
    //
    for (p = _optionalOutParams.begin(); p != _optionalOutParams.end(); ++p)
    {
        ParamInfoPtr info = *p;
        if (is.readOptional(info->tag, info->type->optionalFormat()))
        {
            void* closure = reinterpret_cast<void*>(info->pos);
            info->type->unmarshal(&is, info, results, closure, true);
        }
        else
        {
            RARRAY_ASET(results, info->pos, Qnil);
        }
    }

    if (_returnsClasses)
    {
        is.readPendingValues();
    }

    is.endEncapsulation();

    util.updateSlicedData();

    return results;
}

VALUE
IceRuby::OperationI::unmarshalException(const vector<byte>& bytes, const Ice::CommunicatorPtr& communicator)
{
    Ice::InputStream is{communicator, bytes, lookupSliceLoader(communicator)};

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
        assert(false); // throwException always throws an exception
    }
    catch (const ExceptionReader& r)
    {
        is.endEncapsulation();

        volatile VALUE ex = r.getException();

        if (validateException(ex))
        {
            return ex;
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

    // Never reached.
    return Qnil;
}

bool
IceRuby::OperationI::validateException(VALUE ex) const
{
    for (ExceptionInfoList::const_iterator p = _exceptions.begin(); p != _exceptions.end(); ++p)
    {
        if (callRuby(rb_obj_is_kind_of, ex, (*p)->rubyClass))
        {
            return true;
        }
    }

    return false;
}

void
IceRuby::OperationI::checkTwowayOnly(const Ice::ObjectPrx& proxy) const
{
    if ((_returnType != 0 || !_outParams.empty()) && !proxy->ice_isTwoway())
    {
        throw Ice::TwowayOnlyException{__FILE__, __LINE__, _sliceName};
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
    rb_undef_alloc_func(_operationClass);

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
