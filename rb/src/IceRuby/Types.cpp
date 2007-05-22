// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Types.h>
#include <Proxy.h>
#include <Util.h>
#include <IceUtil/InputUtil.h>
#include <IceUtil/OutputUtil.h>
#include <IceUtil/ScopedArray.h>
#include <Ice/LocalException.h>

#include <st.h>

using namespace std;
using namespace IceRuby;
using namespace IceUtil;

static VALUE _typeInfoClass, _exceptionInfoClass;

typedef map<string, ClassInfoPtr> ClassInfoMap;
static ClassInfoMap _classInfoMap;

typedef map<string, ProxyInfoPtr> ProxyInfoMap;
static ProxyInfoMap _proxyInfoMap;

typedef map<string, ExceptionInfoPtr> ExceptionInfoMap;
static ExceptionInfoMap _exceptionInfoMap;

namespace IceRuby
{

class InfoMapDestroyer
{
public:

    ~InfoMapDestroyer();
};
static InfoMapDestroyer infoMapDestroyer;

class ReadObjectCallback : public Ice::ReadObjectCallback
{
public:

    ReadObjectCallback(const ClassInfoPtr&, const UnmarshalCallbackPtr&, VALUE, void*);

    virtual void invoke(const Ice::ObjectPtr&);

private:

    ClassInfoPtr _info;
    UnmarshalCallbackPtr _cb;
    VALUE _target;
    void* _closure;
};

}

//
// addClassInfo()
//
static void
addClassInfo(const string& id, const ClassInfoPtr& info)
{
    //
    // Do not assert. An application may load statically-
    // translated definitions and then dynamically load
    // duplicate definitions.
    //
//    assert(_classInfoMap.find(id) == _classInfoMap.end());
    ClassInfoMap::iterator p = _classInfoMap.find(id);
    if(p != _classInfoMap.end())
    {
        _classInfoMap.erase(p);
    }
    _classInfoMap.insert(ClassInfoMap::value_type(id, info));
}

//
// addProxyInfo()
//
static void
addProxyInfo(const string& id, const ProxyInfoPtr& info)
{
    //
    // Do not assert. An application may load statically-
    // translated definitions and then dynamically load
    // duplicate definitions.
    //
//    assert(_proxyInfoMap.find(id) == _proxyInfoMap.end());
    ProxyInfoMap::iterator p = _proxyInfoMap.find(id);
    if(p != _proxyInfoMap.end())
    {
        _proxyInfoMap.erase(p);
    }
    _proxyInfoMap.insert(ProxyInfoMap::value_type(id, info));
}

//
// lookupProxyInfo()
//
static IceRuby::ProxyInfoPtr
lookupProxyInfo(const string& id)
{
    ProxyInfoMap::iterator p = _proxyInfoMap.find(id);
    if(p != _proxyInfoMap.end())
    {
        return p->second;
    }
    return 0;
}

//
// addExceptionInfo()
//
static void
addExceptionInfo(const string& id, const ExceptionInfoPtr& info)
{
    //
    // Do not assert. An application may load statically-
    // translated definitions and then dynamically load
    // duplicate definitions.
    //
//    assert(_exceptionInfoMap.find(id) == _exceptionInfoMap.end());
    _exceptionInfoMap.insert(ExceptionInfoMap::value_type(id, info));
}

//
// UnmarshalCallback implementation.
//
IceRuby::UnmarshalCallback::~UnmarshalCallback()
{
}

//
// TypeInfo implementation.
//
IceRuby::TypeInfo::TypeInfo()
{
}

bool
IceRuby::TypeInfo::usesClasses()
{
    return false;
}

void
IceRuby::TypeInfo::unmarshaled(VALUE, VALUE, void*)
{
    assert(false);
}

void
IceRuby::TypeInfo::destroy()
{
}

//
// PrimitiveInfo implementation.
//
IceRuby::PrimitiveInfo::PrimitiveInfo()
{
}

IceRuby::PrimitiveInfo::PrimitiveInfo(Kind k) : kind(k)
{
}

string
IceRuby::PrimitiveInfo::getId() const
{
    switch(kind)
    {
    case KindBool:
        return "bool";
    case KindByte:
        return "byte";
    case KindShort:
        return "short";
    case KindInt:
        return "int";
    case KindLong:
        return "long";
    case KindFloat:
        return "float";
    case KindDouble:
        return "double";
    case KindString:
        return "string";
    }
    assert(false);
    return string();
}

bool
IceRuby::PrimitiveInfo::validate(VALUE)
{
    //
    // Ruby supports type coercion, such that it's technically possible for any
    // value to be coerced to a primitive type. It would be expensive to perform
    // this coercion twice, once to validate and again to marshal, so we skip
    // the validation here.
    //
    return true;
}

void
IceRuby::PrimitiveInfo::marshal(VALUE p, const Ice::OutputStreamPtr& os, ObjectMap*)
{
    switch(kind)
    {
    case PrimitiveInfo::KindBool:
    {
        os->writeBool(RTEST(p));
        break;
    }
    case PrimitiveInfo::KindByte:
    {
        long i = getInteger(p);
        if(i >= 0 && i <= 255)
        {
            os->writeByte(static_cast<Ice::Byte>(i));
            break;
        }
        throw RubyException(rb_eTypeError, "value is out of range for a byte");
    }
    case PrimitiveInfo::KindShort:
    {
        long i = getInteger(p);
        if(i >= SHRT_MIN && i <= SHRT_MAX)
        {
            os->writeShort(static_cast<Ice::Short>(i));
            break;
        }
        throw RubyException(rb_eTypeError, "value is out of range for a short");
    }
    case PrimitiveInfo::KindInt:
    {
        long i = getInteger(p);
        if(i >= INT_MIN && i <= INT_MAX)
        {
            os->writeInt(static_cast<Ice::Int>(i));
            break;
        }
        throw RubyException(rb_eTypeError, "value is out of range for an int");
    }
    case PrimitiveInfo::KindLong:
    {
        os->writeLong(getLong(p));
        break;
    }
    case PrimitiveInfo::KindFloat:
    {
        volatile VALUE val = callRuby(rb_Float, p);
        if(NIL_P(val))
        {
            throw RubyException(rb_eTypeError, "unable to convert value to a float");
        }
        assert(TYPE(val) == T_FLOAT);
        os->writeFloat(static_cast<float>(RFLOAT(val)->value));
        break;
    }
    case PrimitiveInfo::KindDouble:
    {
        volatile VALUE val = callRuby(rb_Float, p);
        if(NIL_P(val))
        {
            throw RubyException(rb_eTypeError, "unable to convert value to a double");
        }
        assert(TYPE(val) == T_FLOAT);
        os->writeDouble(RFLOAT(val)->value);
        break;
    }
    case PrimitiveInfo::KindString:
    {
        string val = getString(p);
        os->writeString(val);
        break;
    }
    }
}

void
IceRuby::PrimitiveInfo::unmarshal(const Ice::InputStreamPtr& is, const UnmarshalCallbackPtr& cb, VALUE target,
                                  void* closure)
{
    volatile VALUE val = Qnil;
    switch(kind)
    {
    case PrimitiveInfo::KindBool:
    {
        val = is->readBool() ? Qtrue : Qfalse;
        break;
    }
    case PrimitiveInfo::KindByte:
    {
        long l = is->readByte();
        val = callRuby(rb_int2inum, l);
        break;
    }
    case PrimitiveInfo::KindShort:
    {
        long l = is->readShort();
        val = callRuby(rb_int2inum, l);
        break;
    }
    case PrimitiveInfo::KindInt:
    {
        long l = is->readInt();
        val = callRuby(rb_int2inum, l);
        break;
    }
    case PrimitiveInfo::KindLong:
    {
        Ice::Long l = is->readLong();
        val = callRuby(rb_ll2inum, l);
        break;
    }
    case PrimitiveInfo::KindFloat:
    {
        Ice::Float f = is->readFloat();
        val = callRuby(rb_float_new, f);
        break;
    }
    case PrimitiveInfo::KindDouble:
    {
        Ice::Double d = is->readDouble();
        val = callRuby(rb_float_new, d);
        break;
    }
    case PrimitiveInfo::KindString:
    {
        string str = is->readString();
        val = createString(str);
        break;
    }
    }
    cb->unmarshaled(val, target, closure);
}

void
IceRuby::PrimitiveInfo::print(VALUE value, IceUtil::Output& out, PrintObjectHistory*)
{
    switch(kind)
    {
    case PrimitiveInfo::KindBool:
    {
        out << (RTEST(value) ? "true" : "false");
        break;
    }
    case PrimitiveInfo::KindByte:
    case PrimitiveInfo::KindShort:
    case PrimitiveInfo::KindInt:
    {
        out << getInteger(value);
        break;
    }
    case PrimitiveInfo::KindLong:
    {
        Ice::Long l = getLong(value);
        out << IceUtil::int64ToString(l);
        break;
    }
    case PrimitiveInfo::KindFloat:
    case PrimitiveInfo::KindDouble:
    {
        double d = toDouble(value);
        out << d;
        break;
    }
    case PrimitiveInfo::KindString:
    {
        out << "'" << getString(value) << "'";
        break;
    }
    }
}

double
IceRuby::PrimitiveInfo::toDouble(VALUE v)
{
    volatile VALUE val = callRuby(rb_Float, v);
    if(NIL_P(val))
    {
        throw RubyException(rb_eTypeError, "unable to convert value to a double");
    }
    assert(TYPE(val) == T_FLOAT);
    return RFLOAT(val)->value;
}

//
// EnumInfo implementation.
//
string
IceRuby::EnumInfo::getId() const
{
    return id;
}

bool
IceRuby::EnumInfo::validate(VALUE val)
{
    return callRuby(rb_obj_is_instance_of, val, rubyClass) == Qtrue;
}

void
IceRuby::EnumInfo::marshal(VALUE p, const Ice::OutputStreamPtr& os, ObjectMap*)
{
    assert(callRuby(rb_obj_is_instance_of, p, rubyClass) == Qtrue); // validate() should have caught this.

    //
    // Validate value.
    //
    volatile VALUE val = callRuby(rb_iv_get, p, "@val");
    assert(FIXNUM_P(val));
    long ival = FIX2LONG(val);
    long count = static_cast<long>(enumerators.size());
    if(ival < 0 || ival >= count)
    {
        throw RubyException(rb_eRangeError, "value %ld is out of range for enum %s", ival, id.c_str());
    }

    if(count <= 127)
    {
        os->writeByte(static_cast<Ice::Byte>(ival));
    }
    else if(count <= 32767)
    {
        os->writeShort(static_cast<Ice::Short>(ival));
    }
    else
    {
        os->writeInt(ival);
    }
}

void
IceRuby::EnumInfo::unmarshal(const Ice::InputStreamPtr& is, const UnmarshalCallbackPtr& cb, VALUE target, void* closure)
{
    int val;
    int count = static_cast<int>(enumerators.size());
    if(count <= 127)
    {
        val = is->readByte();
    }
    else if(count <= 32767)
    {
        val = is->readShort();
    }
    else
    {
        val = is->readInt();
    }

    if(val < 0 || val >= count)
    {
        throw RubyException(rb_eRangeError, "enumerator %ld is out of range for enum %s", val, id.c_str());
    }

    cb->unmarshaled(enumerators[val], target, closure);
}

void
IceRuby::EnumInfo::print(VALUE value, IceUtil::Output& out, PrintObjectHistory*)
{
    if(!validate(value))
    {
        out << "<invalid value - expected " << id << ">";
        return;
    }
    volatile VALUE str = callRuby(rb_funcall, value, rb_intern("inspect"), 0);
    out << getString(str);
}

//
// DataMember implementation.
//
void
IceRuby::DataMember::unmarshaled(VALUE val, VALUE target, void*)
{
    callRuby(rb_ivar_set, target, rubyID, val);
}

//
// StructInfo implementation.
//
string
IceRuby::StructInfo::getId() const
{
    return id;
}

bool
IceRuby::StructInfo::validate(VALUE val)
{
    return callRuby(rb_obj_is_kind_of, val, rubyClass) == Qtrue;
}

bool
IceRuby::StructInfo::usesClasses()
{
    for(DataMemberList::iterator q = members.begin(); q != members.end(); ++q)
    {
        if((*q)->type->usesClasses())
        {
            return true;
        }
    }

    return false;
}

void
IceRuby::StructInfo::marshal(VALUE p, const Ice::OutputStreamPtr& os, ObjectMap* objectMap)
{
    assert(callRuby(rb_obj_is_kind_of, p, rubyClass) == Qtrue); // validate() should have caught this.

    for(DataMemberList::iterator q = members.begin(); q != members.end(); ++q)
    {
        DataMemberPtr member = *q;
        volatile VALUE val = callRuby(rb_ivar_get, p, member->rubyID);
        if(!member->type->validate(val))
        {
            throw RubyException(rb_eTypeError, "invalid value for %s member `%s'", const_cast<char*>(id.c_str()),
                                member->name.c_str());
        }
        member->type->marshal(val, os, objectMap);
    }
}

void
IceRuby::StructInfo::unmarshal(const Ice::InputStreamPtr& is, const UnmarshalCallbackPtr& cb, VALUE target,
                               void* closure)
{
    volatile VALUE obj = callRuby(rb_class_new_instance, 0, static_cast<VALUE*>(0), rubyClass);

    for(DataMemberList::iterator q = members.begin(); q != members.end(); ++q)
    {
        DataMemberPtr member = *q;
        member->type->unmarshal(is, member, obj, 0);
    }

    cb->unmarshaled(obj, target, closure);
}

void
IceRuby::StructInfo::print(VALUE value, IceUtil::Output& out, PrintObjectHistory* history)
{
    if(!validate(value))
    {
        out << "<invalid value - expected " << id << ">";
        return;
    }
    out.sb();
    for(DataMemberList::iterator q = members.begin(); q != members.end(); ++q)
    {
        DataMemberPtr member = *q;
        out << nl << member->name << " = ";
        if(callRuby(rb_ivar_defined, value, member->rubyID) == Qfalse)
        {
            out << "<not defined>";
        }
        else
        {
            volatile VALUE val = callRuby(rb_ivar_get, value, member->rubyID);
            member->type->print(val, out, history);
        }
    }
    out.eb();
}

void
IceRuby::StructInfo::destroy()
{
    for(DataMemberList::iterator p = members.begin(); p != members.end(); ++p)
    {
        (*p)->type->destroy();
    }
    members.clear();
}

//
// SequenceInfo implementation.
//
string
IceRuby::SequenceInfo::getId() const
{
    return id;
}

bool
IceRuby::SequenceInfo::validate(VALUE val)
{
    //
    // Accept nil, an array, a string (for sequence<byte>), or any object that responds to to_ary.
    //
    if(NIL_P(val) || TYPE(val) == T_ARRAY)
    {
        return true;
    }
    if(TYPE(val) == T_STRING)
    {
        PrimitiveInfoPtr pi = PrimitiveInfoPtr::dynamicCast(elementType);
        if(pi && pi->kind == PrimitiveInfo::KindByte)
        {
            return true;
        }
    }
    ID id = rb_intern("to_ary");
    return callRuby(rb_respond_to, val, id) != 0;
}

bool
IceRuby::SequenceInfo::usesClasses()
{
    return elementType->usesClasses();
}

void
IceRuby::SequenceInfo::marshal(VALUE p, const Ice::OutputStreamPtr& os, ObjectMap* objectMap)
{
    if(NIL_P(p))
    {
        os->writeSize(0);
        return;
    }

    PrimitiveInfoPtr pi = PrimitiveInfoPtr::dynamicCast(elementType);
    if(pi)
    {
        marshalPrimitiveSequence(pi, p, os);
        return;
    }

    volatile VALUE arr = callRuby(rb_Array, p);
    if(NIL_P(arr))
    {
        throw RubyException(rb_eTypeError, "unable to convert value to an array");
    }

    long sz = RARRAY(arr)->len;
    os->writeSize(static_cast<Ice::Int>(sz));
    for(long i = 0; i < sz; ++i)
    {
        if(!elementType->validate(RARRAY(arr)->ptr[i]))
        {
            throw RubyException(rb_eTypeError, "invalid value for element %ld of `%s'", i,
                                const_cast<char*>(id.c_str()));
        }
        elementType->marshal(RARRAY(arr)->ptr[i], os, objectMap);
    }
}

void
IceRuby::SequenceInfo::unmarshal(const Ice::InputStreamPtr& is, const UnmarshalCallbackPtr& cb, VALUE target,
                                 void* closure)
{
    PrimitiveInfoPtr pi = PrimitiveInfoPtr::dynamicCast(elementType);
    if(pi)
    {
        unmarshalPrimitiveSequence(pi, is, cb, target, closure);
        return;
    }

    Ice::Int sz = is->readSize();
    volatile VALUE arr = createArray(sz);

    for(Ice::Int i = 0; i < sz; ++i)
    {
        void* cl = reinterpret_cast<void*>(i);
        elementType->unmarshal(is, this, arr, cl);
        RARRAY(arr)->len++; // Increment len for each new element to prevent premature GC.
    }

    cb->unmarshaled(arr, target, closure);
}

void
IceRuby::SequenceInfo::unmarshaled(VALUE val, VALUE target, void* closure)
{
    long i = reinterpret_cast<long>(closure);
    RARRAY(target)->ptr[i] = val;
}

void
IceRuby::SequenceInfo::print(VALUE value, IceUtil::Output& out, PrintObjectHistory* history)
{
    if(!validate(value))
    {
        out << "<invalid value - expected " << id << ">";
        return;
    }

    if(NIL_P(value))
    {
        out << "{}";
    }
    else
    {
        volatile VALUE arr = callRuby(rb_Array, value);
        if(NIL_P(arr))
        {
            throw RubyException(rb_eTypeError, "unable to convert value to an array");
        }

        long sz = RARRAY(arr)->len;

        out.sb();
        for(long i = 0; i < sz; ++i)
        {
            out << nl << '[' << i << "] = ";
            elementType->print(RARRAY(arr)->ptr[i], out, history);
        }
        out.eb();
    }
}

void
IceRuby::SequenceInfo::destroy()
{
    if(elementType)
    {
        elementType->destroy();
        elementType = 0;
    }
}

void
IceRuby::SequenceInfo::marshalPrimitiveSequence(const PrimitiveInfoPtr& pi, VALUE p, const Ice::OutputStreamPtr& os)
{
    volatile VALUE arr = Qnil;
    volatile VALUE str = Qnil;

    //
    // Accept a string or an array for sequence<byte>.
    //
    if(pi->kind == PrimitiveInfo::KindByte)
    {
        if(TYPE(p) == T_STRING)
        {
            str = p;
        }
        else
        {
            arr = callRuby(rb_Array, p);
            if(NIL_P(arr))
            {
                throw RubyException(rb_eTypeError, "argument is not a string or an array");
            }
        }
    }
    else
    {
        arr = callRuby(rb_Array, p);
        if(NIL_P(arr))
        {
            throw RubyException(rb_eTypeError, "unable to convert value to an array");
        }
    }

    switch(pi->kind)
    {
    case PrimitiveInfo::KindBool:
    {
        long sz = RARRAY(arr)->len;
        Ice::BoolSeq seq(sz);
        for(long i = 0; i < sz; ++i)
        {
            seq[i] = RTEST(RARRAY(arr)->ptr[i]);
        }
        os->writeBoolSeq(seq);
        break;
    }
    case PrimitiveInfo::KindByte:
    {
        if(!NIL_P(str))
        {
            const char* s = RSTRING(str)->ptr;
            const long len = RSTRING(str)->len;
            if(s == 0 || len == 0)
            {
                os->writeSize(0);
            }
            else
            {
                os->writeByteSeq(reinterpret_cast<const Ice::Byte*>(s), reinterpret_cast<const Ice::Byte*>(s + len));
            }
        }
        else
        {
            long sz = RARRAY(arr)->len;
            Ice::ByteSeq seq(sz);
            for(long i = 0; i < sz; ++i)
            {
                long val = getInteger(RARRAY(arr)->ptr[i]);
                if(val < 0 || val > 255)
                {
                    throw RubyException(rb_eTypeError, "invalid value for element %ld of sequence<byte>", i);
                }
                seq[i] = static_cast<Ice::Byte>(val);
            }
            os->writeByteSeq(seq);
        }
        break;
    }
    case PrimitiveInfo::KindShort:
    {
        long sz = RARRAY(arr)->len;
        Ice::ShortSeq seq(sz);
        for(long i = 0; i < sz; ++i)
        {
            long val = getInteger(RARRAY(arr)->ptr[i]);
            if(val < SHRT_MIN || val > SHRT_MAX)
            {
                throw RubyException(rb_eTypeError, "invalid value for element %ld of sequence<short>", i);
            }
            seq[i] = static_cast<Ice::Short>(val);
        }
        os->writeShortSeq(seq);
        break;
    }
    case PrimitiveInfo::KindInt:
    {
        long sz = RARRAY(arr)->len;
        Ice::IntSeq seq(sz);
        for(long i = 0; i < sz; ++i)
        {
            long val = getInteger(RARRAY(arr)->ptr[i]);
            if(val < INT_MIN || val > INT_MAX)
            {
                throw RubyException(rb_eTypeError, "invalid value for element %ld of sequence<int>", i);
            }
            seq[i] = static_cast<Ice::Int>(val);
        }
        os->writeIntSeq(seq);
        break;
    }
    case PrimitiveInfo::KindLong:
    {
        long sz = RARRAY(arr)->len;
        Ice::LongSeq seq(sz);
        for(long i = 0; i < sz; ++i)
        {
            seq[i] = getLong(RARRAY(arr)->ptr[i]);
        }
        os->writeLongSeq(seq);
        break;
    }
    case PrimitiveInfo::KindFloat:
    {
        long sz = RARRAY(arr)->len;
        Ice::FloatSeq seq(sz);
        for(long i = 0; i < sz; ++i)
        {
            volatile VALUE v = callRuby(rb_Float, RARRAY(arr)->ptr[i]);
            if(NIL_P(v))
            {
                throw RubyException(rb_eTypeError, "unable to convert array element %ld to a float", i);
            }
            assert(TYPE(v) == T_FLOAT);
            seq[i] = static_cast<Ice::Float>(RFLOAT(v)->value);
        }
        os->writeFloatSeq(seq);
        break;
    }
    case PrimitiveInfo::KindDouble:
    {
        long sz = RARRAY(arr)->len;
        Ice::DoubleSeq seq(sz);
        for(long i = 0; i < sz; ++i)
        {
            volatile VALUE v = callRuby(rb_Float, RARRAY(arr)->ptr[i]);
            if(NIL_P(v))
            {
                throw RubyException(rb_eTypeError, "unable to convert array element %ld to a double", i);
            }
            assert(TYPE(v) == T_FLOAT);
            seq[i] = RFLOAT(v)->value;
        }
        os->writeDoubleSeq(seq);
        break;
    }
    case PrimitiveInfo::KindString:
    {
        long sz = RARRAY(arr)->len;
        Ice::StringSeq seq(sz);
        for(long i = 0; i < sz; ++i)
        {
            seq[i] = getString(RARRAY(arr)->ptr[i]);
        }
        os->writeStringSeq(seq);
        break;
    }
    }
}

void
IceRuby::SequenceInfo::unmarshalPrimitiveSequence(const PrimitiveInfoPtr& pi, const Ice::InputStreamPtr& is,
                                                  const UnmarshalCallbackPtr& cb, VALUE target, void* closure)
{
    volatile VALUE result = Qnil;

    switch(pi->kind)
    {
    case PrimitiveInfo::KindBool:
    {
        pair<const bool*, const bool*> p;
        IceUtil::ScopedArray<bool> sa(is->readBoolSeq(p));
        long sz = static_cast<long>(p.second - p.first);
        result = createArray(sz);

        for(long i = 0; i < sz; ++i)
        {
            RARRAY(result)->ptr[i] = p.first[i] ? Qtrue : Qfalse;
            RARRAY(result)->len++; // Increment len for each new element to prevent premature GC.
        }
        break;
    }
    case PrimitiveInfo::KindByte:
    {
        pair<const Ice::Byte*, const Ice::Byte*> p;
        is->readByteSeq(p);
        result = callRuby(rb_str_new, reinterpret_cast<const char*>(p.first), static_cast<long>(p.second - p.first));
        break;
    }
    case PrimitiveInfo::KindShort:
    {
        pair<const Ice::Short*, const Ice::Short*> p;
        IceUtil::ScopedArray<Ice::Short> sa(is->readShortSeq(p));
        long sz = static_cast<long>(p.second - p.first);
        result = createArray(sz);

        for(long i = 0; i < sz; ++i)
        {
            RARRAY(result)->ptr[i] = INT2FIX(p.first[i]);
            RARRAY(result)->len++; // Increment len for each new element to prevent premature GC.
        }
        break;
    }
    case PrimitiveInfo::KindInt:
    {
        pair<const Ice::Int*, const Ice::Int*> p;
        IceUtil::ScopedArray<Ice::Int> sa(is->readIntSeq(p));
        long sz = static_cast<long>(p.second - p.first);
        result = createArray(sz);

        for(long i = 0; i < sz; ++i)
        {
            RARRAY(result)->ptr[i] = INT2FIX(p.first[i]);
            RARRAY(result)->len++; // Increment len for each new element to prevent premature GC.
        }
        break;
    }
    case PrimitiveInfo::KindLong:
    {
        pair<const Ice::Long*, const Ice::Long*> p;
        IceUtil::ScopedArray<Ice::Long> sa(is->readLongSeq(p));
        long sz = static_cast<long>(p.second - p.first);
        result = createArray(sz);

        for(long i = 0; i < sz; ++i)
        {
            RARRAY(result)->ptr[i] = callRuby(rb_ll2inum, p.first[i]);
            RARRAY(result)->len++; // Increment len for each new element to prevent premature GC.
        }
        break;
    }
    case PrimitiveInfo::KindFloat:
    {
        pair<const Ice::Float*, const Ice::Float*> p;
        IceUtil::ScopedArray<Ice::Float> sa(is->readFloatSeq(p));
        long sz = static_cast<long>(p.second - p.first);
        result = createArray(sz);

        for(long i = 0; i < sz; ++i)
        {
            RARRAY(result)->ptr[i] = callRuby(rb_float_new, p.first[i]);
            RARRAY(result)->len++; // Increment len for each new element to prevent premature GC.
        }
        break;
    }
    case PrimitiveInfo::KindDouble:
    {
        pair<const Ice::Double*, const Ice::Double*> p;
        IceUtil::ScopedArray<Ice::Double> sa(is->readDoubleSeq(p));
        long sz = static_cast<long>(p.second - p.first);
        result = createArray(sz);

        for(long i = 0; i < sz; ++i)
        {
            RARRAY(result)->ptr[i] = callRuby(rb_float_new, p.first[i]);
            RARRAY(result)->len++; // Increment len for each new element to prevent premature GC.
        }
        break;
    }
    case PrimitiveInfo::KindString:
    {
        Ice::StringSeq seq = is->readStringSeq();
        long sz = static_cast<long>(seq.size());
        result = createArray(sz);

        for(long i = 0; i < sz; ++i)
        {
            RARRAY(result)->ptr[i] = createString(seq[i]);
            RARRAY(result)->len++; // Increment len for each new element to prevent premature GC.
        }
        break;
    }
    }
    cb->unmarshaled(result, target, closure);
}

//
// DictionaryInfo implementation.
//
string
IceRuby::DictionaryInfo::getId() const
{
    return id;
}

bool
IceRuby::DictionaryInfo::validate(VALUE val)
{
    //
    // Accept nil, a hash, or any object that responds to to_hash.
    //
    if(NIL_P(val) || TYPE(val) == T_HASH)
    {
        return true;
    }
    ID id = rb_intern("to_hash");
    return callRuby(rb_respond_to, val, id) != 0;
}

bool
IceRuby::DictionaryInfo::usesClasses()
{
    return valueType->usesClasses();
}

namespace
{
struct DictionaryMarshalIterator : public IceRuby::HashIterator
{
    DictionaryMarshalIterator(const IceRuby::DictionaryInfoPtr& d, const Ice::OutputStreamPtr o, IceRuby::ObjectMap* m)
        : dict(d), os(o), objectMap(m)
    {
    }

    virtual void element(VALUE key, VALUE value)
    {
        dict->marshalElement(key, value, os, objectMap);
    }

    IceRuby::DictionaryInfoPtr dict;
    Ice::OutputStreamPtr os;
    IceRuby::ObjectMap* objectMap;
};
}

void
IceRuby::DictionaryInfo::marshal(VALUE p, const Ice::OutputStreamPtr& os, ObjectMap* objectMap)
{
    if(NIL_P(p))
    {
        os->writeSize(0);
        return;
    }

    volatile VALUE hash = callRuby(rb_convert_type, p, T_HASH, "Hash", "to_hash");
    if(NIL_P(hash))
    {
        throw RubyException(rb_eTypeError, "unable to convert value to a hash");
    }

    int sz = RHASH(hash)->tbl->num_entries;
    os->writeSize(sz);

    DictionaryMarshalIterator iter(this, os, objectMap);
    hashIterate(hash, iter);
}

void
IceRuby::DictionaryInfo::marshalElement(VALUE key, VALUE value, const Ice::OutputStreamPtr& os, ObjectMap* objectMap)
{
    if(!keyType->validate(key))
    {
        throw RubyException(rb_eTypeError, "invalid key in `%s' element", const_cast<char*>(id.c_str()));
    }

    if(!valueType->validate(value))
    {
        throw RubyException(rb_eTypeError, "invalid value in `%s' element", const_cast<char*>(id.c_str()));
    }

    keyType->marshal(key, os, objectMap);
    valueType->marshal(value, os, objectMap);
}

void
IceRuby::DictionaryInfo::unmarshal(const Ice::InputStreamPtr& is, const UnmarshalCallbackPtr& cb, VALUE target,
                                   void* closure)
{
    volatile VALUE hash = callRuby(rb_hash_new);

    KeyCallbackPtr keyCB = new KeyCallback;
    keyCB->key = Qnil;

    Ice::Int sz = is->readSize();
    for(Ice::Int i = 0; i < sz; ++i)
    {
        //
        // A dictionary key cannot be a class (or contain one), so the key must be
        // available immediately.
        //
        keyType->unmarshal(is, keyCB, Qnil, 0);
        assert(!NIL_P(keyCB->key));

        //
        // The callback will set the dictionary entry with the unmarshaled value,
        // so we pass it the key.
        //
        void* cl = reinterpret_cast<void*>(keyCB->key);
        valueType->unmarshal(is, this, hash, cl);
    }

    cb->unmarshaled(hash, target, closure);
}

void
IceRuby::DictionaryInfo::unmarshaled(VALUE val, VALUE target, void* closure)
{
    volatile VALUE key = reinterpret_cast<VALUE>(closure);
    callRuby(rb_hash_aset, target, key, val);
}

namespace
{
struct DictionaryPrintIterator : public IceRuby::HashIterator
{
    DictionaryPrintIterator(const DictionaryInfoPtr& d, IceUtil::Output& o, PrintObjectHistory* h) :
        dict(d), out(o), history(h)
    {
    }

    virtual void element(VALUE key, VALUE value)
    {
        dict->printElement(key, value, out, history);
    }

    IceRuby::DictionaryInfoPtr dict;
    IceUtil::Output& out;
    IceRuby::PrintObjectHistory* history;
};
}

void
IceRuby::DictionaryInfo::print(VALUE value, IceUtil::Output& out, PrintObjectHistory* history)
{
    if(!validate(value))
    {
        out << "<invalid value - expected " << id << ">";
        return;
    }

    if(NIL_P(value))
    {
        out << "{}";
    }
    else
    {
        volatile VALUE hash = callRuby(rb_convert_type, value, T_HASH, "Hash", "to_hash");
        if(NIL_P(hash))
        {
            throw RubyException(rb_eTypeError, "unable to convert value to a hash");
        }

        if(RHASH(hash)->tbl->num_entries == 0)
        {
            out << "{}";
            return;
        }

        out.sb();
        DictionaryPrintIterator iter(this, out, history);
        hashIterate(hash, iter);
        out.eb();
    }
}

void
IceRuby::DictionaryInfo::printElement(VALUE key, VALUE value, IceUtil::Output& out, PrintObjectHistory* history)
{
    out << nl << "key = ";
    keyType->print(key, out, history);
    out << nl << "value = ";
    valueType->print(value, out, history);
}

void
IceRuby::DictionaryInfo::KeyCallback::unmarshaled(VALUE val, VALUE, void*)
{
    key = val;
}

void
IceRuby::DictionaryInfo::destroy()
{
    if(keyType)
    {
        keyType->destroy();
        keyType = 0;
    }
    if(valueType)
    {
        valueType->destroy();
        valueType = 0;
    }
}

//
// ClassInfo implementation.
//
string
IceRuby::ClassInfo::getId() const
{
    return id;
}

bool
IceRuby::ClassInfo::validate(VALUE val)
{
    if(NIL_P(val))
    {
        return true;
    }

    //
    // We consider an object to be an instance of this class if its class contains
    // an ICE_TYPE constant that refers to this class, or a subclass of this class.
    //
    volatile VALUE cls = CLASS_OF(val);
    volatile VALUE type = callRuby(rb_const_get, cls, rb_intern("ICE_TYPE"));
    if(NIL_P(type))
    {
        return false;
    }
    ClassInfoPtr info = ClassInfoPtr::dynamicCast(getType(type));
    assert(info);
    return info->isA(this);
}

bool
IceRuby::ClassInfo::usesClasses()
{
    return true;
}

void
IceRuby::ClassInfo::marshal(VALUE p, const Ice::OutputStreamPtr& os, ObjectMap* objectMap)
{
    if(!defined)
    {
        throw RubyException(rb_eRuntimeError, "class %s is declared but not defined", id.c_str());
    }

    if(NIL_P(p))
    {
        os->writeObject(0);
        return;
    }

    //
    // Ice::ObjectWriter is a subclass of Ice::Object that wraps a Ruby object for marshaling.
    // It is possible that this Ruby object has already been marshaled, therefore we first must
    // check the object map to see if this object is present. If so, we use the existing ObjectWriter,
    // otherwise we create a new one.
    //
    Ice::ObjectPtr writer;
    assert(objectMap);
    ObjectMap::iterator q = objectMap->find(p);
    if(q == objectMap->end())
    {
        volatile VALUE cls = CLASS_OF(p);
        volatile VALUE type = callRuby(rb_const_get, cls, rb_intern("ICE_TYPE"));
        assert(!NIL_P(type)); // Should have been caught by validate().
        ClassInfoPtr info = ClassInfoPtr::dynamicCast(getType(type));
        assert(info);
        writer = new ObjectWriter(info, p, objectMap);
        objectMap->insert(ObjectMap::value_type(p, writer));
    }
    else
    {
        writer = q->second;
    }

    //
    // Give the writer to the stream. The stream will eventually call write() on it.
    //
    os->writeObject(writer);
}

void
IceRuby::ClassInfo::unmarshal(const Ice::InputStreamPtr& is, const UnmarshalCallbackPtr& cb, VALUE target,
                              void* closure)
{
    if(!defined)
    {
        throw RubyException(rb_eRuntimeError, "class %s is declared but not defined", id.c_str());
    }

    is->readObject(new ReadObjectCallback(this, cb, target, closure));
}

void
IceRuby::ClassInfo::print(VALUE value, IceUtil::Output& out, PrintObjectHistory* history)
{
    if(!validate(value))
    {
        out << "<invalid value - expected " << id << ">";
        return;
    }

    if(NIL_P(value))
    {
        out << "<nil>";
    }
    else
    {
        map<VALUE, int>::iterator q = history->objects.find(value);
        if(q != history->objects.end())
        {
            out << "<object #" << q->second << ">";
        }
        else
        {
            volatile VALUE cls = CLASS_OF(value);
            volatile VALUE type = callRuby(rb_const_get, cls, rb_intern("ICE_TYPE"));
            ClassInfoPtr info = ClassInfoPtr::dynamicCast(getType(type));
            assert(info);
            out << "object #" << history->index << " (" << info->id << ')';
            history->objects.insert(map<VALUE, int>::value_type(value, history->index));
            ++history->index;
            out.sb();
            info->printMembers(value, out, history);
            out.eb();
        }
    }
}

void
IceRuby::ClassInfo::destroy()
{
    base = 0;
    interfaces.clear();
    if(!members.empty())
    {
        DataMemberList ml = members;
        members.clear();
        for(DataMemberList::iterator p = ml.begin(); p != ml.end(); ++p)
        {
            (*p)->type->destroy();
        }
    }
}

void
IceRuby::ClassInfo::printMembers(VALUE value, IceUtil::Output& out, PrintObjectHistory* history)
{
    if(base)
    {
        base->printMembers(value, out, history);
    }

    for(DataMemberList::iterator q = members.begin(); q != members.end(); ++q)
    {
        DataMemberPtr member = *q;
        out << nl << member->name << " = ";
        if(callRuby(rb_ivar_defined, value, member->rubyID) == Qfalse)
        {
            out << "<not defined>";
        }
        else
        {
            volatile VALUE val = callRuby(rb_ivar_get, value, member->rubyID);
            member->type->print(val, out, history);
        }
    }
}

bool
IceRuby::ClassInfo::isA(const ClassInfoPtr& info)
{
    //
    // Return true if this class has an is-a relationship with info.
    //
    if(info->isIceObject)
    {
        return true;
    }
    else if(this == info.get())
    {
        return true;
    }
    else if(base && base->isA(info))
    {
        return true;
    }
    else if(!interfaces.empty())
    {
        for(ClassInfoList::iterator p = interfaces.begin(); p != interfaces.end(); ++p)
        {
            if((*p)->isA(info))
            {
                return true;
            }
        }
    }

    return false;
}

//
// ProxyInfo implementation.
//
string
IceRuby::ProxyInfo::getId() const
{
    return id;
}

bool
IceRuby::ProxyInfo::validate(VALUE val)
{
    if(!NIL_P(val))
    {
        if(!checkProxy(val))
        {
            return false;
        }
        volatile VALUE cls = CLASS_OF(val);
        volatile VALUE type = callRuby(rb_const_get, cls, rb_intern("ICE_TYPE"));
        assert(!NIL_P(type));
        ProxyInfoPtr info = ProxyInfoPtr::dynamicCast(getType(type));
        assert(info);
        return info->classInfo->isA(classInfo);
    }
    return true;
}

void
IceRuby::ProxyInfo::marshal(VALUE p, const Ice::OutputStreamPtr& os, ObjectMap*)
{
    if(NIL_P(p))
    {
        os->writeProxy(0);
    }
    else
    {
        assert(checkProxy(p)); // validate() should have caught this.
        os->writeProxy(getProxy(p));
    }
}

void
IceRuby::ProxyInfo::unmarshal(const Ice::InputStreamPtr& is, const UnmarshalCallbackPtr& cb, VALUE target,
                              void* closure)
{
    Ice::ObjectPrx proxy = is->readProxy();

    if(!proxy)
    {
        cb->unmarshaled(Qnil, target, closure);
        return;
    }

    if(NIL_P(rubyClass))
    {
        throw RubyException(rb_eRuntimeError, "class %s is declared but not defined", id.c_str());
    }

    volatile VALUE p = createProxy(proxy, rubyClass);
    cb->unmarshaled(p, target, closure);
}

void
IceRuby::ProxyInfo::print(VALUE value, IceUtil::Output& out, PrintObjectHistory*)
{
    if(!validate(value))
    {
        out << "<invalid value - expected " << getId() << ">";
        return;
    }

    if(NIL_P(value))
    {
        out << "<nil>";
    }
    else
    {
        out << getString(value);
    }
}

void
IceRuby::ProxyInfo::destroy()
{
    classInfo = 0;
}

//
// ObjectWriter implementation.
//
IceRuby::ObjectWriter::ObjectWriter(const ClassInfoPtr& info, VALUE object, ObjectMap* objectMap) :
    _info(info), _object(object), _map(objectMap)
{
}

void
IceRuby::ObjectWriter::ice_preMarshal()
{
    ID id = rb_intern("ice_preMarshal");
    if(callRuby(rb_respond_to, _object, id))
    {
        callRuby(rb_funcall, _object, id, 0);
    }
}

void
IceRuby::ObjectWriter::write(const Ice::OutputStreamPtr& os) const
{
    ClassInfoPtr info = _info;
    while(info)
    {
        os->writeTypeId(info->id);

        os->startSlice();
        for(DataMemberList::iterator q = info->members.begin(); q != info->members.end(); ++q)
        {
            DataMemberPtr member = *q;
            volatile VALUE val = callRuby(rb_ivar_get, _object, member->rubyID);
            if(!member->type->validate(val))
            {
                throw RubyException(rb_eTypeError, "invalid value for %s member `%s'", _info->id.c_str(),
                                    member->name.c_str());
            }

            member->type->marshal(val, os, _map);
        }
        os->endSlice();

        info = info->base;
    }

    //
    // Marshal the Ice::Object slice.
    //
    os->writeTypeId(Ice::Object::ice_staticId());
    os->startSlice();
    os->writeSize(0); // For compatibility with the old AFM.
    os->endSlice();
}

//
// ObjectReader implementation.
//
IceRuby::ObjectReader::ObjectReader(VALUE object, const ClassInfoPtr& info) :
    _object(object), _info(info)
{
}

void
IceRuby::ObjectReader::ice_postUnmarshal()
{
    ID id = rb_intern("ice_postUnmarshal");
    if(callRuby(rb_respond_to, _object, id))
    {
        callRuby(rb_funcall, _object, id, 0);
    }
}

void
IceRuby::ObjectReader::read(const Ice::InputStreamPtr& is, bool rid)
{
    //
    // Unmarshal the slices of a user-defined class.
    //
    if(_info->id != Ice::Object::ice_staticId())
    {
        ClassInfoPtr info = _info;
        while(info)
        {
            if(rid)
            {
                is->readTypeId();
            }

            is->startSlice();
            for(DataMemberList::iterator p = info->members.begin(); p != info->members.end(); ++p)
            {
                DataMemberPtr member = *p;
                member->type->unmarshal(is, member, _object, 0);
            }
            is->endSlice();

            rid = true;

            info = info->base;
        }
    }

    //
    // Unmarshal the Ice::Object slice.
    //
    if(rid)
    {
        is->readTypeId();
    }

    is->startSlice();
    // For compatibility with the old AFM.
    Ice::Int sz = is->readSize();
    if(sz != 0)
    {
        throw Ice::MarshalException(__FILE__, __LINE__);
    }
    is->endSlice();
}

ClassInfoPtr
IceRuby::ObjectReader::getInfo() const
{
    return _info;
}

VALUE
IceRuby::ObjectReader::getObject() const
{
    return _object;
}

//
// InfoMapDestroyer implementation.
//
IceRuby::InfoMapDestroyer::~InfoMapDestroyer()
{
    {
        for(ProxyInfoMap::iterator p = _proxyInfoMap.begin(); p != _proxyInfoMap.end(); ++p)
        {
            p->second->destroy();
        }
    }
    {
        for(ClassInfoMap::iterator p = _classInfoMap.begin(); p != _classInfoMap.end(); ++p)
        {
            p->second->destroy();
        }
    }
    _exceptionInfoMap.clear();
}

//
// ReadObjectCallback implementation.
//
IceRuby::ReadObjectCallback::ReadObjectCallback(const ClassInfoPtr& info, const UnmarshalCallbackPtr& cb,
                                                VALUE target, void* closure) :
    _info(info), _cb(cb), _target(target), _closure(closure)
{
}

void
IceRuby::ReadObjectCallback::invoke(const Ice::ObjectPtr& p)
{
    if(p)
    {
        ObjectReaderPtr reader = ObjectReaderPtr::dynamicCast(p);
        assert(reader);

        //
        // Verify that the unmarshaled object is compatible with the formal type.
        //
        volatile VALUE obj = reader->getObject();
        if(!_info->validate(obj))
        {
            Ice::UnexpectedObjectException ex(__FILE__, __LINE__);
            ex.reason = "unmarshaled object is not an instance of " + _info->id;
            ex.type = reader->getInfo()->getId();
            ex.expectedType = _info->id;
            throw ex;
        }

        _cb->unmarshaled(obj, _target, _closure);
    }
    else
    {
        _cb->unmarshaled(Qnil, _target, _closure);
    }
}

//
// ExceptionInfo implementation.
//
void
IceRuby::ExceptionInfo::marshal(VALUE p, const Ice::OutputStreamPtr& os, ObjectMap* objectMap)
{
    if(callRuby(rb_obj_is_kind_of, p, rubyClass) == Qfalse)
    {
        throw RubyException(rb_eTypeError, "expected exception %s", id.c_str());
    }

    os->writeBool(usesClasses);

    ExceptionInfoPtr info = this;
    while(info)
    {
        os->writeString(info->id);

        os->startSlice();
        for(DataMemberList::iterator q = info->members.begin(); q != info->members.end(); ++q)
        {
            DataMemberPtr member = *q;
            volatile VALUE val = callRuby(rb_ivar_get, p, member->rubyID);
            if(!member->type->validate(val))
            {
                throw RubyException(rb_eTypeError, "invalid value for %s member `%s'", id.c_str(),
                                    member->name.c_str());
            }

            member->type->marshal(val, os, objectMap);
        }
        os->endSlice();

        info = info->base;
    }
}

VALUE
IceRuby::ExceptionInfo::unmarshal(const Ice::InputStreamPtr& is)
{
    volatile VALUE obj = callRuby(rb_class_new_instance, 0, static_cast<VALUE*>(0), rubyClass);

    //
    // NOTE: The type id for the first slice has already been read.
    //
    ExceptionInfoPtr info = this;
    while(info)
    {
        is->startSlice();
        for(DataMemberList::iterator q = info->members.begin(); q != info->members.end(); ++q)
        {
            DataMemberPtr member = *q;
            member->type->unmarshal(is, member, obj, 0);
        }
        is->endSlice();

        info = info->base;
        if(info)
        {
            is->readString(); // Read the ID of the next slice.
        }
    }

    return obj;
}

void
IceRuby::ExceptionInfo::print(VALUE value, IceUtil::Output& out)
{
    if(callRuby(rb_obj_is_kind_of, value, rubyClass) == Qfalse)
    {
        out << "<invalid value - expected " << id << ">";
        return;
    }

    PrintObjectHistory history;
    history.index = 0;

    out << "exception " << id;
    out.sb();
    printMembers(value, out, &history);
    out.eb();
}

void
IceRuby::ExceptionInfo::printMembers(VALUE value, IceUtil::Output& out, PrintObjectHistory* history)
{
    if(base)
    {
        base->printMembers(value, out, history);
    }

    for(DataMemberList::iterator q = members.begin(); q != members.end(); ++q)
    {
        DataMemberPtr member = *q;
        out << nl << member->name << " = ";
        if(callRuby(rb_ivar_defined, value, member->rubyID) == Qfalse)
        {
            out << "<not defined>";
        }
        else
        {
            volatile VALUE val = callRuby(rb_ivar_get, value, member->rubyID);
            member->type->print(val, out, history);
        }
    }
}

extern "C"
VALUE
IceRuby_defineEnum(VALUE /*self*/, VALUE id, VALUE type, VALUE enumerators)
{
    ICE_RUBY_TRY
    {
        EnumInfoPtr info = new EnumInfo;
        info->id = getString(id);
        info->rubyClass = type;

        volatile VALUE arr = callRuby(rb_check_array_type, enumerators);
        assert(!NIL_P(arr));
        for(long i = 0; i < RARRAY(arr)->len; ++i)
        {
            info->enumerators.push_back(RARRAY(arr)->ptr[i]);
        }

        return createType(info);
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_defineStruct(VALUE /*self*/, VALUE id, VALUE type, VALUE members)
{
    ICE_RUBY_TRY
    {
        StructInfoPtr info = new StructInfo;
        info->id = getString(id);
        info->rubyClass = type;

        volatile VALUE arr = callRuby(rb_check_array_type, members);
        assert(!NIL_P(arr));
        for(long i = 0; i < RARRAY(arr)->len; ++i)
        {
            volatile VALUE m = callRuby(rb_check_array_type, RARRAY(arr)->ptr[i]);
            assert(!NIL_P(m));
            assert(RARRAY(m)->len == 2);
            DataMemberPtr member = new DataMember;
            member->name = getString(RARRAY(m)->ptr[0]);
            member->type = getType(RARRAY(m)->ptr[1]);
            string s = "@" + member->name;
            member->rubyID = rb_intern(s.c_str());
            info->members.push_back(member);
        }

        return createType(info);
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_defineSequence(VALUE /*self*/, VALUE id, VALUE elementType)
{
    ICE_RUBY_TRY
    {
        SequenceInfoPtr info = new SequenceInfo;
        info->id = getString(id);
        info->elementType = getType(elementType);

        return createType(info);
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_defineDictionary(VALUE /*self*/, VALUE id, VALUE keyType, VALUE valueType)
{
    ICE_RUBY_TRY
    {
        DictionaryInfoPtr info = new DictionaryInfo;
        info->id = getString(id);
        info->keyType = getType(keyType);
        info->valueType = getType(valueType);

        return createType(info);
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_declareProxy(VALUE /*self*/, VALUE id)
{
    ICE_RUBY_TRY
    {
        string proxyId = getString(id);
        proxyId += "Prx";

        ProxyInfoPtr info = lookupProxyInfo(proxyId);
        if(!info)
        {
            info = new ProxyInfo;
            info->id = proxyId;
            info->rubyClass = Qnil;
            info->typeObj = createType(info);
            addProxyInfo(proxyId, info);
        }

        return info->typeObj;
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_declareClass(VALUE /*self*/, VALUE id)
{
    ICE_RUBY_TRY
    {
        string idstr = getString(id);
        ClassInfoPtr info = lookupClassInfo(idstr);
        if(!info)
        {
            info = new ClassInfo;
            info->id = idstr;
            info->isIceObject = idstr == "::Ice::Object";
            info->rubyClass = Qnil;
            info->typeObj = createType(info);
            info->defined = false;
            addClassInfo(idstr, info);
        }

        return info->typeObj;
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_defineException(VALUE /*self*/, VALUE id, VALUE type, VALUE base, VALUE members)
{
    ICE_RUBY_TRY
    {
        ExceptionInfoPtr info = new ExceptionInfo;
        info->id = getString(id);

        if(!NIL_P(base))
        {
            info->base = ExceptionInfoPtr::dynamicCast(getException(base));
            assert(info->base);
        }

        info->usesClasses = false;

        volatile VALUE arr = callRuby(rb_check_array_type, members);
        assert(!NIL_P(arr));
        for(long i = 0; i < RARRAY(arr)->len; ++i)
        {
            volatile VALUE m = callRuby(rb_check_array_type, RARRAY(arr)->ptr[i]);
            assert(!NIL_P(m));
            assert(RARRAY(m)->len == 2);
            DataMemberPtr member = new DataMember;
            member->name = getString(RARRAY(m)->ptr[0]);
            member->type = getType(RARRAY(m)->ptr[1]);
            string s = "@" + member->name;
            member->rubyID = rb_intern(s.c_str());
            info->members.push_back(member);
            if(!info->usesClasses)
            {
                info->usesClasses = member->type->usesClasses();
            }
        }

        info->rubyClass = type;

        addExceptionInfo(info->id, info);

        return createException(info);
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_TypeInfo_defineProxy(VALUE self, VALUE type, VALUE classInfo)
{
    ICE_RUBY_TRY
    {
        ProxyInfoPtr info = ProxyInfoPtr::dynamicCast(getType(self));
        assert(info);

        info->rubyClass = type;
        info->classInfo = ClassInfoPtr::dynamicCast(getType(classInfo));
        assert(info->classInfo);
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_TypeInfo_defineClass(VALUE self, VALUE type, VALUE isAbstract, VALUE base, VALUE interfaces, VALUE members)
{
    ICE_RUBY_TRY
    {
        ClassInfoPtr info = ClassInfoPtr::dynamicCast(getType(self));
        assert(info);

        info->isAbstract = isAbstract == Qtrue;

        if(!NIL_P(base))
        {
            info->base = ClassInfoPtr::dynamicCast(getType(base));
            assert(info->base);
        }

        long i;
        volatile VALUE arr;

        arr = callRuby(rb_check_array_type, interfaces);
        assert(!NIL_P(arr));
        for(i = 0; i < RARRAY(arr)->len; ++i)
        {
            ClassInfoPtr iface = ClassInfoPtr::dynamicCast(getType(RARRAY(arr)->ptr[i]));
            assert(iface);
            info->interfaces.push_back(iface);
        }

        arr = callRuby(rb_check_array_type, members);
        assert(!NIL_P(arr));
        for(i = 0; i < RARRAY(arr)->len; ++i)
        {
            volatile VALUE m = callRuby(rb_check_array_type, RARRAY(arr)->ptr[i]);
            assert(!NIL_P(m));
            assert(RARRAY(m)->len == 2);
            DataMemberPtr member = new DataMember;
            member->name = getString(RARRAY(m)->ptr[0]);
            member->type = getType(RARRAY(m)->ptr[1]);
            string s = "@" + member->name;
            member->rubyID = rb_intern(s.c_str());
            info->members.push_back(member);
        }

        info->rubyClass = type;
        info->defined = true;
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_stringify(VALUE /*self*/, VALUE obj, VALUE type)
{
    ICE_RUBY_TRY
    {
        TypeInfoPtr info = getType(type);

        ostringstream ostr;
        IceUtil::Output out(ostr);
        PrintObjectHistory history;
        history.index = 0;
        info->print(obj, out, &history);

        string str = ostr.str();
        return createString(str);
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_stringifyException(VALUE /*self*/, VALUE ex)
{
    ICE_RUBY_TRY
    {
        volatile VALUE cls = CLASS_OF(ex);
        volatile VALUE type = callRuby(rb_const_get, cls, rb_intern("ICE_TYPE"));
        ExceptionInfoPtr info = getException(type);

        ostringstream ostr;
        IceUtil::Output out(ostr);
        info->print(ex, out);

        string str = ostr.str();
        return createString(str);
    }
    ICE_RUBY_CATCH
    return Qnil;
}

//
// lookupClassInfo()
//
IceRuby::ClassInfoPtr
IceRuby::lookupClassInfo(const string& id)
{
    ClassInfoMap::iterator p = _classInfoMap.find(id);
    if(p != _classInfoMap.end())
    {
        return p->second;
    }
    return 0;
}

//
// lookupExceptionInfo()
//
IceRuby::ExceptionInfoPtr
IceRuby::lookupExceptionInfo(const string& id)
{
    ExceptionInfoMap::iterator p = _exceptionInfoMap.find(id);
    if(p != _exceptionInfoMap.end())
    {
        return p->second;
    }
    return 0;
}

bool
IceRuby::initTypes(VALUE iceModule)
{
    //
    // Define a class to represent TypeInfo, and another to represent ExceptionInfo.
    //
    _typeInfoClass = rb_define_class_under(iceModule, "Internal_TypeInfo", rb_cObject);
    _exceptionInfoClass = rb_define_class_under(iceModule, "Internal_ExceptionInfo", rb_cObject);

    rb_define_const(iceModule, "T_bool", createType(new PrimitiveInfo(PrimitiveInfo::KindBool)));
    rb_define_const(iceModule, "T_byte", createType(new PrimitiveInfo(PrimitiveInfo::KindByte)));
    rb_define_const(iceModule, "T_short", createType(new PrimitiveInfo(PrimitiveInfo::KindShort)));
    rb_define_const(iceModule, "T_int", createType(new PrimitiveInfo(PrimitiveInfo::KindInt)));
    rb_define_const(iceModule, "T_long", createType(new PrimitiveInfo(PrimitiveInfo::KindLong)));
    rb_define_const(iceModule, "T_float", createType(new PrimitiveInfo(PrimitiveInfo::KindFloat)));
    rb_define_const(iceModule, "T_double", createType(new PrimitiveInfo(PrimitiveInfo::KindDouble)));
    rb_define_const(iceModule, "T_string", createType(new PrimitiveInfo(PrimitiveInfo::KindString)));

    rb_define_module_function(iceModule, "__defineEnum", CAST_METHOD(IceRuby_defineEnum), 3);
    rb_define_module_function(iceModule, "__defineStruct", CAST_METHOD(IceRuby_defineStruct), 3);
    rb_define_module_function(iceModule, "__defineSequence", CAST_METHOD(IceRuby_defineSequence), 2);
    rb_define_module_function(iceModule, "__defineDictionary", CAST_METHOD(IceRuby_defineDictionary), 3);
    rb_define_module_function(iceModule, "__declareProxy", CAST_METHOD(IceRuby_declareProxy), 1);
    rb_define_module_function(iceModule, "__declareClass", CAST_METHOD(IceRuby_declareClass), 1);
    rb_define_module_function(iceModule, "__defineException", CAST_METHOD(IceRuby_defineException), 4);

    rb_define_method(_typeInfoClass, "defineClass", CAST_METHOD(IceRuby_TypeInfo_defineClass), 5);
    rb_define_method(_typeInfoClass, "defineProxy", CAST_METHOD(IceRuby_TypeInfo_defineProxy), 2);

    rb_define_module_function(iceModule, "__stringify", CAST_METHOD(IceRuby_stringify), 2);
    rb_define_module_function(iceModule, "__stringifyException", CAST_METHOD(IceRuby_stringifyException), 1);

    return true;
}

IceRuby::TypeInfoPtr
IceRuby::getType(VALUE obj)
{
    assert(TYPE(obj) == T_DATA);
    assert(rb_obj_is_instance_of(obj, _typeInfoClass) == Qtrue);
    TypeInfoPtr* p = reinterpret_cast<TypeInfoPtr*>(DATA_PTR(obj));
    return *p;
}

extern "C"
void
IceRuby_TypeInfo_free(TypeInfoPtr* p)
{
    delete p;
}

VALUE
IceRuby::createType(const TypeInfoPtr& info)
{
    return Data_Wrap_Struct(_typeInfoClass, 0, IceRuby_TypeInfo_free, new TypeInfoPtr(info));
}

IceRuby::ExceptionInfoPtr
IceRuby::getException(VALUE obj)
{
    assert(TYPE(obj) == T_DATA);
    assert(rb_obj_is_instance_of(obj, _exceptionInfoClass) == Qtrue);
    ExceptionInfoPtr* p = reinterpret_cast<ExceptionInfoPtr*>(DATA_PTR(obj));
    return *p;
}

extern "C"
void
IceRuby_ExceptionInfo_free(ExceptionInfoPtr* p)
{
    delete p;
}

VALUE
IceRuby::createException(const ExceptionInfoPtr& info)
{
    return Data_Wrap_Struct(_exceptionInfoClass, 0, IceRuby_ExceptionInfo_free, new ExceptionInfoPtr(info));
}
