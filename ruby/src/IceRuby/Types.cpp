// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
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
#include <Ice/InputStream.h>
#include <Ice/LocalException.h>
#include <Ice/OutputStream.h>
#include <Ice/SlicedData.h>
#include <list>
#include <limits>
#include <math.h>

//
// Required for RHASH_SIZE to work properly with Ruby 1.8.x.
// T_ZOMBIE is only defined in Ruby 1.9.
//
#ifndef T_ZOMBIE
#   include "st.h"
#endif

#ifndef RHASH_SIZE
#   define RHASH_SIZE(v) RHASH(v)->tbl->num_entries
#endif

using namespace std;
using namespace IceRuby;
using namespace IceUtil;
using namespace IceUtilInternal;

static VALUE _typeInfoClass, _exceptionInfoClass, _unsetTypeClass;

typedef map<string, ClassInfoPtr> ClassInfoMap;
static ClassInfoMap _classInfoMap;

typedef map<Ice::Int, ClassInfoPtr> CompactIdMap;
static CompactIdMap _compactIdMap;

typedef map<string, ProxyInfoPtr> ProxyInfoMap;
static ProxyInfoMap _proxyInfoMap;

typedef map<string, ExceptionInfoPtr> ExceptionInfoMap;
static ExceptionInfoMap _exceptionInfoMap;

namespace IceRuby
{

VALUE Unset;

class InfoMapDestroyer
{
public:

    ~InfoMapDestroyer();
};
static InfoMapDestroyer infoMapDestroyer;

string
escapeString(const string& str)
{
    static const string basicSourceChars = "abcdefghijklmnopqrstuvwxyz"
                                           "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                           "0123456789"
                                           "_{}[]#()<>%:;.?*+-/^&|~!=,\\\"' ";
    static const set<char> charSet(basicSourceChars.begin(), basicSourceChars.end());

    ostringstream out;

    for(string::const_iterator c = str.begin(); c != str.end(); ++c)
    {
        if(charSet.find(*c) == charSet.end())
        {
            unsigned char uc = *c;                  // char may be signed, so make it positive
            ostringstream s;
            s << "\\";                              // Print as octal if not in basic source character set
            s.width(3);
            s.fill('0');
            s << oct;
            s << static_cast<unsigned>(uc);
            out << s.str();
        }
        else
        {
            out << *c;                              // Print normally if in basic source character set
        }
    }

    return out.str();
}

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
// StreamUtil implementation
//
VALUE IceRuby::StreamUtil::_slicedDataType = Qnil;
VALUE IceRuby::StreamUtil::_sliceInfoType = Qnil;

IceRuby::StreamUtil::StreamUtil()
{
}

IceRuby::StreamUtil::~StreamUtil()
{
    //
    // Make sure we break any cycles among the ObjectReaders in preserved slices.
    //
    for(set<ObjectReaderPtr>::iterator p = _readers.begin(); p != _readers.end(); ++p)
    {
        Ice::SlicedDataPtr slicedData = (*p)->getSlicedData();
        for(Ice::SliceInfoSeq::const_iterator q = slicedData->slices.begin(); q != slicedData->slices.end(); ++q)
        {
            //
            // Don't just call (*q)->instances.clear(), as releasing references
            // to the instances could have unexpected side effects. We exchange
            // the vector into a temporary and then let the temporary fall out
            // of scope.
            //
            vector<Ice::ObjectPtr> tmp;
            tmp.swap((*q)->instances);
        }
    }
}

void
IceRuby::StreamUtil::add(const ReadObjectCallbackPtr& callback)
{
    _callbacks.push_back(callback);
}

void
IceRuby::StreamUtil::add(const ObjectReaderPtr& reader)
{
    assert(reader->getSlicedData());
    _readers.insert(reader);
}

void
IceRuby::StreamUtil::updateSlicedData()
{
    for(set<ObjectReaderPtr>::iterator p = _readers.begin(); p != _readers.end(); ++p)
    {
        setSlicedDataMember((*p)->getObject(), (*p)->getSlicedData());
    }
}

void
IceRuby::StreamUtil::setSlicedDataMember(VALUE obj, const Ice::SlicedDataPtr& slicedData)
{
    //
    // Create a Ruby equivalent of the SlicedData object.
    //

    assert(slicedData);

    if(_slicedDataType == Qnil)
    {
        _slicedDataType = callRuby(rb_path2class, "Ice::SlicedData");
        assert(!NIL_P(_slicedDataType));
    }
    if(_sliceInfoType == Qnil)
    {
        _sliceInfoType = callRuby(rb_path2class, "Ice::SliceInfo");
        assert(!NIL_P(_sliceInfoType));
    }

    volatile VALUE sd = callRuby(rb_class_new_instance, 0, static_cast<VALUE*>(0), _slicedDataType);

    Ice::Int sz = slicedData->slices.size();
    volatile VALUE slices = createArray(sz);

    callRuby(rb_iv_set, sd, "@slices", slices);

    //
    // Translate each SliceInfo object into its Ruby equivalent.
    //
    int i = 0;
    for(vector<Ice::SliceInfoPtr>::const_iterator p = slicedData->slices.begin(); p != slicedData->slices.end(); ++p)
    {
        volatile VALUE slice = callRuby(rb_class_new_instance, 0, static_cast<VALUE*>(0), _sliceInfoType);

        RARRAY_ASET(slices, i, slice);
        i++;

        //
        // typeId
        //
        volatile VALUE typeId = createString((*p)->typeId);
        callRuby(rb_iv_set, slice, "@typeId", typeId);

        //
        // compactId
        //
        volatile VALUE compactId = INT2FIX((*p)->compactId);
        callRuby(rb_iv_set, slice, "@compactId", compactId);

        //
        // bytes
        //
        volatile VALUE bytes = callRuby(rb_str_new, reinterpret_cast<const char*>(&(*p)->bytes[0]), (*p)->bytes.size());
        callRuby(rb_iv_set, slice, "@bytes", bytes);

        //
        // instances
        //
        volatile VALUE instances = createArray((*p)->instances.size());
        callRuby(rb_iv_set, slice, "@instances", instances);

        int j = 0;
        for(vector<Ice::ObjectPtr>::iterator q = (*p)->instances.begin(); q != (*p)->instances.end(); ++q)
        {
            //
            // Each element in the instances list is an instance of ObjectReader that wraps a Ruby object.
            //
            assert(*q);
            ObjectReaderPtr r = ObjectReaderPtr::dynamicCast(*q);
            assert(r);
            VALUE o = r->getObject();
            assert(o != Qnil); // Should be non-nil.
            RARRAY_ASET(instances, j, o);
            j++;
        }

        //
        // hasOptionalMembers
        //
        callRuby(rb_iv_set, slice, "@hasOptionalMembers", (*p)->hasOptionalMembers ? Qtrue : Qfalse);

        //
        // isLastSlice
        //
        callRuby(rb_iv_set, slice, "@isLastSlice", (*p)->isLastSlice ? Qtrue : Qfalse);
    }

    callRuby(rb_iv_set, obj, "@_ice_slicedData", sd);
}

//
// Instances of preserved class and exception types may have a data member
// named _ice_slicedData which is an instance of the Ruby class Ice::SlicedData.
//
Ice::SlicedDataPtr
IceRuby::StreamUtil::getSlicedDataMember(VALUE obj, ObjectMap* objectMap)
{
    Ice::SlicedDataPtr slicedData;

    if(callRuby(rb_ivar_defined, obj, rb_intern("@_ice_slicedData")) == Qtrue)
    {
        volatile VALUE sd = callRuby(rb_iv_get, obj, "@_ice_slicedData");

        if(sd != Qnil)
        {
            //
            // The "slices" member is an array of Ice::SliceInfo objects.
            //
            volatile VALUE sl = callRuby(rb_iv_get, sd, "@slices");
            assert(TYPE(sl) == T_ARRAY);

            Ice::SliceInfoSeq slices;

            long sz = RARRAY_LEN(sl);
            for(long i = 0; i < sz; ++i)
            {
                volatile VALUE s = RARRAY_AREF(sl, i);

                Ice::SliceInfoPtr info = new Ice::SliceInfo;

                volatile VALUE typeId = callRuby(rb_iv_get, s, "@typeId");
                info->typeId = getString(typeId);

                volatile VALUE compactId = callRuby(rb_iv_get, s, "@compactId");
                info->compactId = static_cast<Ice::Int>(getInteger(compactId));

                volatile VALUE bytes = callRuby(rb_iv_get, s, "@bytes");
                assert(TYPE(bytes) == T_STRING);
                const char* str = RSTRING_PTR(bytes);
                const long len = RSTRING_LEN(bytes);
                if(str != 0 && len != 0)
                {
                    vector<Ice::Byte> vtmp(reinterpret_cast<const Ice::Byte*>(str),
                                           reinterpret_cast<const Ice::Byte*>(str + len));
                    info->bytes.swap(vtmp);
                }

                volatile VALUE instances = callRuby(rb_iv_get, s, "@instances");
                assert(TYPE(instances) == T_ARRAY);
                long osz = RARRAY_LEN(instances);
                for(long j = 0; j < osz; ++j)
                {
                    VALUE o = RARRAY_AREF(instances, j);

                    Ice::ObjectPtr writer;

                    ObjectMap::iterator i = objectMap->find(o);
                    if(i == objectMap->end())
                    {
                        writer = new ObjectWriter(o, objectMap, 0);
                        objectMap->insert(ObjectMap::value_type(o, writer));
                    }
                    else
                    {
                        writer = i->second;
                    }

                    info->instances.push_back(writer);
                }

                volatile VALUE hasOptionalMembers = callRuby(rb_iv_get, s, "@hasOptionalMembers");
                info->hasOptionalMembers = hasOptionalMembers == Qtrue;

                volatile VALUE isLastSlice = callRuby(rb_iv_get, s, "@isLastSlice");
                info->isLastSlice = isLastSlice == Qtrue;

                slices.push_back(info);
            }

            slicedData = new Ice::SlicedData(slices);
        }
    }

    return slicedData;
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
IceRuby::TypeInfo::usesClasses() const
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

bool
IceRuby::PrimitiveInfo::variableLength() const
{
    return kind == KindString;
}

int
IceRuby::PrimitiveInfo::wireSize() const
{
    switch(kind)
    {
    case KindBool:
    case KindByte:
        return 1;
    case KindShort:
        return 2;
    case KindInt:
        return 4;
    case KindLong:
        return 8;
    case KindFloat:
        return 4;
    case KindDouble:
        return 8;
    case KindString:
        return 1;
    }
    assert(false);
    return 0;
}

Ice::OptionalFormat
IceRuby::PrimitiveInfo::optionalFormat() const
{
    switch(kind)
    {
    case KindBool:
    case KindByte:
        return Ice::OptionalFormatF1;
    case KindShort:
        return Ice::OptionalFormatF2;
    case KindInt:
        return Ice::OptionalFormatF4;
    case KindLong:
        return Ice::OptionalFormatF8;
    case KindFloat:
        return Ice::OptionalFormatF4;
    case KindDouble:
        return Ice::OptionalFormatF8;
    case KindString:
        return Ice::OptionalFormatVSize;
    }

    assert(false);
    return Ice::OptionalFormatF1;
}

void
IceRuby::PrimitiveInfo::marshal(VALUE p, Ice::OutputStream* os, ObjectMap*, bool)
{
    switch(kind)
    {
    case PrimitiveInfo::KindBool:
    {
        os->write(static_cast<bool>(RTEST(p)));
        break;
    }
    case PrimitiveInfo::KindByte:
    {
        long i = getInteger(p);
        if(i >= 0 && i <= 255)
        {
            os->write(static_cast<Ice::Byte>(i));
            break;
        }
        throw RubyException(rb_eTypeError, "value is out of range for a byte");
    }
    case PrimitiveInfo::KindShort:
    {
        long i = getInteger(p);
        if(i >= SHRT_MIN && i <= SHRT_MAX)
        {
            os->write(static_cast<Ice::Short>(i));
            break;
        }
        throw RubyException(rb_eTypeError, "value is out of range for a short");
    }
    case PrimitiveInfo::KindInt:
    {
        long i = getInteger(p);
        if(i >= INT_MIN && i <= INT_MAX)
        {
            os->write(static_cast<Ice::Int>(i));
            break;
        }
        throw RubyException(rb_eTypeError, "value is out of range for an int");
    }
    case PrimitiveInfo::KindLong:
    {
        os->write(getLong(p));
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
        double d = static_cast<double>(RFLOAT_VALUE(val));
        if(
#if defined(_MSC_VER) && (_MSC_VER <= 1700)
            _finite(val) &&
#else
            isfinite(d) &&
#endif
            (d > numeric_limits<float>::max() || d < -numeric_limits<float>::max()))
        {
            throw RubyException(rb_eTypeError, "value is out of range for a float");
        }
        os->write(static_cast<float>(d));
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
        os->write(static_cast<double>(RFLOAT_VALUE(val)));
        break;
    }
    case PrimitiveInfo::KindString:
    {
        string val = getString(p);
#ifdef HAVE_RUBY_ENCODING_H
        os->write(val, false); // Bypass string conversion.
#else
        os->write(val, true);
#endif
        break;
    }
    }
}

void
IceRuby::PrimitiveInfo::unmarshal(Ice::InputStream* is, const UnmarshalCallbackPtr& cb, VALUE target,
                                  void* closure, bool)
{
    volatile VALUE val = Qnil;
    switch(kind)
    {
    case PrimitiveInfo::KindBool:
    {
        bool b;
        is->read(b);
        val = b ? Qtrue : Qfalse;
        break;
    }
    case PrimitiveInfo::KindByte:
    {
        Ice::Byte b;
        is->read(b);
        val = callRuby(rb_int2inum, b);
        break;
    }
    case PrimitiveInfo::KindShort:
    {
        Ice::Short sh;
        is->read(sh);
        val = callRuby(rb_int2inum, sh);
        break;
    }
    case PrimitiveInfo::KindInt:
    {
        Ice::Int i;
        is->read(i);
        val = callRuby(rb_int2inum, i);
        break;
    }
    case PrimitiveInfo::KindLong:
    {
        Ice::Long l;
        is->read(l);
        val = callRuby(rb_ll2inum, l);
        break;
    }
    case PrimitiveInfo::KindFloat:
    {
        Ice::Float f;
        is->read(f);
        val = callRuby(rb_float_new, f);
        break;
    }
    case PrimitiveInfo::KindDouble:
    {
        Ice::Double d;
        is->read(d);
        val = callRuby(rb_float_new, d);
        break;
    }
    case PrimitiveInfo::KindString:
    {
        string str;
#ifdef HAVE_RUBY_ENCODING_H
        is->read(str, false); // Bypass string conversion.
#else
        is->read(str, true);
#endif
        val = createString(str);
        break;
    }
    }
    cb->unmarshaled(val, target, closure);
}

void
IceRuby::PrimitiveInfo::print(VALUE value, IceUtilInternal::Output& out, PrintObjectHistory*)
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
        out << IceUtilInternal::int64ToString(l);
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
    return RFLOAT_VALUE(val);
}

//
// EnumInfo implementation.
//

namespace
{
struct EnumDefinitionIterator : public IceRuby::HashIterator
{
    EnumDefinitionIterator() :
        maxValue(0)
    {
    }

    virtual void element(VALUE key, VALUE value)
    {
        const Ice::Int v = static_cast<Ice::Int>(getInteger(key));
        assert(enumerators.find(v) == enumerators.end());
        enumerators[v] = value;

        if(v > maxValue)
        {
            maxValue = v;
        }
    }

    Ice::Int maxValue;
    IceRuby::EnumeratorMap enumerators;
};
}

IceRuby::EnumInfo::EnumInfo(VALUE ident, VALUE t, VALUE e) :
    rubyClass(t), maxValue(0)
{
    const_cast<string&>(id) = getString(ident);

    EnumDefinitionIterator iter;
    hashIterate(e, iter);

    const_cast<Ice::Int&>(maxValue) = iter.maxValue;
    const_cast<EnumeratorMap&>(enumerators) = iter.enumerators;
}

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

bool
IceRuby::EnumInfo::variableLength() const
{
    return true;
}

int
IceRuby::EnumInfo::wireSize() const
{
    return 1;
}

Ice::OptionalFormat
IceRuby::EnumInfo::optionalFormat() const
{
    return Ice::OptionalFormatSize;
}

void
IceRuby::EnumInfo::marshal(VALUE p, Ice::OutputStream* os, ObjectMap*, bool)
{
    assert(callRuby(rb_obj_is_instance_of, p, rubyClass) == Qtrue); // validate() should have caught this.

    //
    // Validate value.
    //
    volatile VALUE val = callRuby(rb_iv_get, p, "@value");
    const Ice::Int ival = static_cast<Ice::Int>(getInteger(val));
    if(enumerators.find(ival) == enumerators.end())
    {
        throw RubyException(rb_eRangeError, "invalid enumerator %ld for enum %s", ival, id.c_str());
    }

    os->writeEnum(ival, maxValue);
}

void
IceRuby::EnumInfo::unmarshal(Ice::InputStream* is, const UnmarshalCallbackPtr& cb, VALUE target, void* closure,
                             bool)
{
    Ice::Int val = is->readEnum(maxValue);

    EnumeratorMap::const_iterator p = enumerators.find(val);
    if(p == enumerators.end())
    {
        ostringstream ostr;
        ostr << "invalid enumerator " << val << " for enum " << id;
        throw Ice::MarshalException(__FILE__, __LINE__, ostr.str());
    }

    cb->unmarshaled(p->second, target, closure);
}

void
IceRuby::EnumInfo::print(VALUE value, IceUtilInternal::Output& out, PrintObjectHistory*)
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

static void
convertDataMembers(VALUE members, DataMemberList& reqMembers, DataMemberList& optMembers, bool allowOptional)
{
    list<DataMemberPtr> optList;

    volatile VALUE arr = callRuby(rb_check_array_type, members);
    assert(!NIL_P(arr));
    for(long i = 0; i < RARRAY_LEN(arr); ++i)
    {
        volatile VALUE m = callRuby(rb_check_array_type, RARRAY_AREF(arr, i));
        assert(!NIL_P(m));
        assert(RARRAY_LEN(m) == (allowOptional ? 4 : 2));

        DataMemberPtr member = new DataMember;

        member->name = getString(RARRAY_AREF(m, 0));
        member->type = getType(RARRAY_AREF(m, 1));
        string s = "@" + member->name;
        member->rubyID = rb_intern(s.c_str());

        if(allowOptional)
        {
            member->optional = RTEST(RARRAY_AREF(m, 2));
            member->tag = static_cast<int>(getInteger(RARRAY_AREF(m, 3)));
        }
        else
        {
            member->optional = false;
            member->tag = 0;
        }

        if(member->optional)
        {
            optList.push_back(member);
        }
        else
        {
            reqMembers.push_back(member);
        }
    }

    if(allowOptional)
    {
        class SortFn
        {
        public:
            static bool compare(const DataMemberPtr& lhs, const DataMemberPtr& rhs)
            {
                return lhs->tag < rhs->tag;
            }
        };

        optList.sort(SortFn::compare);
        copy(optList.begin(), optList.end(), back_inserter(optMembers));
    }
}

//
// StructInfo implementation.
//
IceRuby::StructInfo::StructInfo(VALUE ident, VALUE t, VALUE m) :
    rubyClass(t), _nullMarshalValue(Qnil)
{
    const_cast<string&>(id) = getString(ident);

    DataMemberList opt;
    convertDataMembers(m, const_cast<DataMemberList&>(members), opt, false);
    assert(opt.empty());

    _variableLength = false;
    _wireSize = 0;
    for(DataMemberList::const_iterator p = members.begin(); p != members.end(); ++p)
    {
        if(!_variableLength && (*p)->type->variableLength())
        {
            _variableLength = true;
        }
        _wireSize += (*p)->type->wireSize();
    }
}

string
IceRuby::StructInfo::getId() const
{
    return id;
}

bool
IceRuby::StructInfo::validate(VALUE val)
{
    return NIL_P(val) || callRuby(rb_obj_is_kind_of, val, rubyClass) == Qtrue;
}

bool
IceRuby::StructInfo::variableLength() const
{
    return _variableLength;
}

int
IceRuby::StructInfo::wireSize() const
{
    return _wireSize;
}

Ice::OptionalFormat
IceRuby::StructInfo::optionalFormat() const
{
    return _variableLength ? Ice::OptionalFormatFSize : Ice::OptionalFormatVSize;
}

bool
IceRuby::StructInfo::usesClasses() const
{
    for(DataMemberList::const_iterator p = members.begin(); p != members.end(); ++p)
    {
        if((*p)->type->usesClasses())
        {
            return true;
        }
    }

    return false;
}

void
IceRuby::StructInfo::marshal(VALUE p, Ice::OutputStream* os, ObjectMap* objectMap, bool optional)
{
    assert(NIL_P(p) || callRuby(rb_obj_is_kind_of, p, rubyClass) == Qtrue); // validate() should have caught this.

    if(NIL_P(p))
    {
        if(NIL_P(_nullMarshalValue))
        {
            _nullMarshalValue = callRuby(rb_class_new_instance, 0, static_cast<VALUE*>(0), rubyClass);
            rb_gc_register_address(&_nullMarshalValue); // Prevent garbage collection
        }
        p = _nullMarshalValue;
    }

    Ice::OutputStream::size_type sizePos = -1;
    if(optional)
    {
        if(_variableLength)
        {
            sizePos = os->startSize();
        }
        else
        {
            os->writeSize(_wireSize);
        }
    }

    for(DataMemberList::const_iterator q = members.begin(); q != members.end(); ++q)
    {
        DataMemberPtr member = *q;
        volatile VALUE val = callRuby(rb_ivar_get, p, member->rubyID);
        if(!member->type->validate(val))
        {
            throw RubyException(rb_eTypeError, "invalid value for %s member `%s'", const_cast<char*>(id.c_str()),
                                member->name.c_str());
        }
        member->type->marshal(val, os, objectMap, false);
    }

    if(optional && _variableLength)
    {
        os->endSize(sizePos);
    }
}

void
IceRuby::StructInfo::unmarshal(Ice::InputStream* is, const UnmarshalCallbackPtr& cb, VALUE target,
                               void* closure, bool optional)
{
    volatile VALUE obj = callRuby(rb_class_new_instance, 0, static_cast<VALUE*>(0), rubyClass);

    if(optional)
    {
        if(_variableLength)
        {
            is->skip(4);
        }
        else
        {
            is->skipSize();
        }
    }

    for(DataMemberList::const_iterator q = members.begin(); q != members.end(); ++q)
    {
        DataMemberPtr member = *q;
        member->type->unmarshal(is, member, obj, 0, false);
    }

    cb->unmarshaled(obj, target, closure);
}

void
IceRuby::StructInfo::print(VALUE value, IceUtilInternal::Output& out, PrintObjectHistory* history)
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
        out.sb();
        for(DataMemberList::const_iterator q = members.begin(); q != members.end(); ++q)
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
}

void
IceRuby::StructInfo::destroy()
{
    for(DataMemberList::const_iterator p = members.begin(); p != members.end(); ++p)
    {
        (*p)->type->destroy();
    }
    const_cast<DataMemberList&>(members).clear();
    if(!NIL_P(_nullMarshalValue))
    {
        rb_gc_unregister_address(&_nullMarshalValue); // Prevent garbage collection
        _nullMarshalValue = Qnil;
    }
}

//
// SequenceInfo implementation.
//
IceRuby::SequenceInfo::SequenceInfo(VALUE ident, VALUE t)
{
    const_cast<string&>(id) = getString(ident);
    const_cast<TypeInfoPtr&>(elementType) = getType(t);
}

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
IceRuby::SequenceInfo::variableLength() const
{
    return true;
}

int
IceRuby::SequenceInfo::wireSize() const
{
    return 1;
}

Ice::OptionalFormat
IceRuby::SequenceInfo::optionalFormat() const
{
    return elementType->variableLength() ? Ice::OptionalFormatFSize : Ice::OptionalFormatVSize;
}

bool
IceRuby::SequenceInfo::usesClasses() const
{
    return elementType->usesClasses();
}

void
IceRuby::SequenceInfo::marshal(VALUE p, Ice::OutputStream* os, ObjectMap* objectMap, bool optional)
{
    PrimitiveInfoPtr pi = PrimitiveInfoPtr::dynamicCast(elementType);

    volatile VALUE arr = Qnil;

    Ice::OutputStream::size_type sizePos = -1;
    if(optional)
    {
        if(elementType->variableLength())
        {
            sizePos = os->startSize();
        }
        else if(elementType->wireSize() > 1)
        {
            //
            // Determine the sequence size.
            //
            int sz = 0;
            if(!NIL_P(p))
            {
                if(TYPE(p) == T_ARRAY)
                {
                    sz = static_cast<int>(RARRAY_LEN(p));
                }
                else
                {
                    arr = callRuby(rb_Array, p);
                    if(NIL_P(arr))
                    {
                        throw RubyException(rb_eTypeError, "unable to convert value to an array");
                    }
                    sz = static_cast<int>(RARRAY_LEN(arr));
                }
            }
            os->writeSize(sz == 0 ? 1 : sz * elementType->wireSize() + (sz > 254 ? 5 : 1));
        }
    }

    if(NIL_P(p))
    {
        os->writeSize(0);
    }
    else if(pi)
    {
        marshalPrimitiveSequence(pi, p, os);
    }
    else
    {
        if(NIL_P(arr))
        {
            arr = callRuby(rb_Array, p);
            if(NIL_P(arr))
            {
                throw RubyException(rb_eTypeError, "unable to convert value to an array");
            }
        }

        long sz = RARRAY_LEN(arr);
        os->writeSize(static_cast<Ice::Int>(sz));
        for(long i = 0; i < sz; ++i)
        {
            if(!elementType->validate(RARRAY_AREF(arr, i)))
            {
                throw RubyException(rb_eTypeError, "invalid value for element %ld of `%s'", i,
                                    const_cast<char*>(id.c_str()));
            }
            elementType->marshal(RARRAY_AREF(arr, i), os, objectMap, false);
        }
    }

    if(optional && elementType->variableLength())
    {
        os->endSize(sizePos);
    }
}

void
IceRuby::SequenceInfo::unmarshal(Ice::InputStream* is, const UnmarshalCallbackPtr& cb, VALUE target,
                                 void* closure, bool optional)
{
    if(optional)
    {
        if(elementType->variableLength())
        {
            is->skip(4);
        }
        else if(elementType->wireSize() > 1)
        {
            is->skipSize();
        }
    }

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
        elementType->unmarshal(is, this, arr, cl, false);
    }

    cb->unmarshaled(arr, target, closure);
}

void
IceRuby::SequenceInfo::unmarshaled(VALUE val, VALUE target, void* closure)
{
#ifdef ICE_64
    long i = static_cast<long>(reinterpret_cast<long long>(closure));
#else
    long i = reinterpret_cast<long>(closure);
#endif
    RARRAY_ASET(target, i, val);
}

void
IceRuby::SequenceInfo::print(VALUE value, IceUtilInternal::Output& out, PrintObjectHistory* history)
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
        if(TYPE(value) == T_STRING)
        {
            PrimitiveInfoPtr pi = PrimitiveInfoPtr::dynamicCast(elementType);
            if(pi && pi->kind == PrimitiveInfo::KindByte)
            {
                out << "'" << escapeString(getString(value)) << "'";
                return;
            }
        }

        volatile VALUE arr = callRuby(rb_Array, value);
        if(NIL_P(arr))
        {
            throw RubyException(rb_eTypeError, "unable to convert value to an array");
        }

        long sz = RARRAY_LEN(arr);

        out.sb();
        for(long i = 0; i < sz; ++i)
        {
            out << nl << '[' << i << "] = ";
            elementType->print(RARRAY_AREF(arr, i), out, history);
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
        const_cast<TypeInfoPtr&>(elementType) = 0;
    }
}

void
IceRuby::SequenceInfo::marshalPrimitiveSequence(const PrimitiveInfoPtr& pi, VALUE p, Ice::OutputStream* os)
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
        long sz = RARRAY_LEN(arr);
        Ice::BoolSeq seq(sz);
        for(long i = 0; i < sz; ++i)
        {
            seq[i] = RTEST(RARRAY_AREF(arr, i));
        }
        os->write(seq);
        break;
    }
    case PrimitiveInfo::KindByte:
    {
        if(!NIL_P(str))
        {
            const char* s = RSTRING_PTR(str);
            const long len = RSTRING_LEN(str);
            if(s == 0 || len == 0)
            {
                os->write(Ice::Int(0));
            }
            else
            {
                os->write(reinterpret_cast<const Ice::Byte*>(s), reinterpret_cast<const Ice::Byte*>(s + len));
            }
        }
        else
        {
            long sz = RARRAY_LEN(arr);
            Ice::ByteSeq seq(sz);
            for(long i = 0; i < sz; ++i)
            {
                long val = getInteger(RARRAY_AREF(arr, i));
                if(val < 0 || val > 255)
                {
                    throw RubyException(rb_eTypeError, "invalid value for element %ld of sequence<byte>", i);
                }
                seq[i] = static_cast<Ice::Byte>(val);
            }
            os->write(&seq[0], &seq[0] + seq.size());
        }
        break;
    }
    case PrimitiveInfo::KindShort:
    {
        long sz = RARRAY_LEN(arr);
        Ice::ShortSeq seq(sz);
        for(long i = 0; i < sz; ++i)
        {
            long val = getInteger(RARRAY_AREF(arr, i));
            if(val < SHRT_MIN || val > SHRT_MAX)
            {
                throw RubyException(rb_eTypeError, "invalid value for element %ld of sequence<short>", i);
            }
            seq[i] = static_cast<Ice::Short>(val);
        }
        os->write(&seq[0], &seq[0] + seq.size());
        break;
    }
    case PrimitiveInfo::KindInt:
    {
        long sz = RARRAY_LEN(arr);
        Ice::IntSeq seq(sz);
        for(long i = 0; i < sz; ++i)
        {
            long val = getInteger(RARRAY_AREF(arr, i));
            if(val < INT_MIN || val > INT_MAX)
            {
                throw RubyException(rb_eTypeError, "invalid value for element %ld of sequence<int>", i);
            }
            seq[i] = static_cast<Ice::Int>(val);
        }
        os->write(&seq[0], &seq[0] + seq.size());
        break;
    }
    case PrimitiveInfo::KindLong:
    {
        long sz = RARRAY_LEN(arr);
        Ice::LongSeq seq(sz);
        for(long i = 0; i < sz; ++i)
        {
            seq[i] = getLong(RARRAY_AREF(arr, i));
        }
        os->write(&seq[0], &seq[0] + seq.size());
        break;
    }
    case PrimitiveInfo::KindFloat:
    {
        long sz = RARRAY_LEN(arr);
        Ice::FloatSeq seq(sz);
        for(long i = 0; i < sz; ++i)
        {
            volatile VALUE v = callRuby(rb_Float, RARRAY_AREF(arr, i));
            if(NIL_P(v))
            {
                throw RubyException(rb_eTypeError, "unable to convert array element %ld to a float", i);
            }
            assert(TYPE(v) == T_FLOAT);
            seq[i] = static_cast<Ice::Float>(RFLOAT_VALUE(v));
        }
        os->write(&seq[0], &seq[0] + seq.size());
        break;
    }
    case PrimitiveInfo::KindDouble:
    {
        long sz = RARRAY_LEN(arr);
        Ice::DoubleSeq seq(sz);
        for(long i = 0; i < sz; ++i)
        {
            volatile VALUE v = callRuby(rb_Float, RARRAY_AREF(arr, i));
            if(NIL_P(v))
            {
                throw RubyException(rb_eTypeError, "unable to convert array element %ld to a double", i);
            }
            assert(TYPE(v) == T_FLOAT);
            seq[i] = RFLOAT_VALUE(v);
        }
        os->write(&seq[0], &seq[0] + seq.size());
        break;
    }
    case PrimitiveInfo::KindString:
    {
        long sz = RARRAY_LEN(arr);
        Ice::StringSeq seq(sz);
        for(long i = 0; i < sz; ++i)
        {
            seq[i] = getString(RARRAY_AREF(arr, i));
        }
#ifdef HAVE_RUBY_ENCODING_H
        os->write(&seq[0], &seq[0] + seq.size(), false); // Bypass string conversion.
#else
        os->write(&seq[0], &seq[0] + seq.size(), true);
#endif
        break;
    }
    }
}

void
IceRuby::SequenceInfo::unmarshalPrimitiveSequence(const PrimitiveInfoPtr& pi, Ice::InputStream* is,
                                                  const UnmarshalCallbackPtr& cb, VALUE target, void* closure)
{
    volatile VALUE result = Qnil;

    switch(pi->kind)
    {
    case PrimitiveInfo::KindBool:
    {
        pair<const bool*, const bool*> p;
        IceUtil::ScopedArray<bool> sa;
        is->read(p, sa);
        long sz = static_cast<long>(p.second - p.first);
        result = createArray(sz);

        if(sz > 0)
        {
            for(long i = 0; i < sz; ++i)
            {
                RARRAY_ASET(result, i, p.first[i] ? Qtrue : Qfalse);
            }
        }
        break;
    }
    case PrimitiveInfo::KindByte:
    {
        pair<const Ice::Byte*, const Ice::Byte*> p;
        is->read(p);
        result = callRuby(rb_str_new, reinterpret_cast<const char*>(p.first), static_cast<long>(p.second - p.first));
        break;
    }
    case PrimitiveInfo::KindShort:
    {
        pair<const Ice::Short*, const Ice::Short*> p;
        IceUtil::ScopedArray<Ice::Short> sa;
        is->read(p, sa);
        long sz = static_cast<long>(p.second - p.first);
        result = createArray(sz);

        if(sz > 0)
        {
            for(long i = 0; i < sz; ++i)
            {
                RARRAY_ASET(result, i, INT2FIX(p.first[i]));
            }
        }
        break;
    }
    case PrimitiveInfo::KindInt:
    {
        pair<const Ice::Int*, const Ice::Int*> p;
        IceUtil::ScopedArray<Ice::Int> sa;
        is->read(p, sa);
        long sz = static_cast<long>(p.second - p.first);
        result = createArray(sz);

        if(sz > 0)
        {
            for(long i = 0; i < sz; ++i)
            {
                RARRAY_ASET(result, i, INT2FIX(p.first[i]));
            }
        }
        break;
    }
    case PrimitiveInfo::KindLong:
    {
        pair<const Ice::Long*, const Ice::Long*> p;
        IceUtil::ScopedArray<Ice::Long> sa;
        is->read(p, sa);
        long sz = static_cast<long>(p.second - p.first);
        result = createArray(sz);

        if(sz > 0)
        {
            for(long i = 0; i < sz; ++i)
            {
                RARRAY_ASET(result, i, callRuby(rb_ll2inum, p.first[i]));
            }
        }
        break;
    }
    case PrimitiveInfo::KindFloat:
    {
        pair<const Ice::Float*, const Ice::Float*> p;
        IceUtil::ScopedArray<Ice::Float> sa;
        is->read(p, sa);
        long sz = static_cast<long>(p.second - p.first);
        result = createArray(sz);

        if(sz > 0)
        {
            for(long i = 0; i < sz; ++i)
            {
                RARRAY_ASET(result, i, callRuby(rb_float_new, p.first[i]));
            }
        }
        break;
    }
    case PrimitiveInfo::KindDouble:
    {
        pair<const Ice::Double*, const Ice::Double*> p;
        IceUtil::ScopedArray<Ice::Double> sa;
        is->read(p, sa);
        long sz = static_cast<long>(p.second - p.first);
        result = createArray(sz);

        if(sz > 0)
        {
            for(long i = 0; i < sz; ++i)
            {
                RARRAY_ASET(result, i, callRuby(rb_float_new, p.first[i]));
            }
        }
        break;
    }
    case PrimitiveInfo::KindString:
    {
        Ice::StringSeq seq;
#ifdef HAVE_RUBY_ENCODING_H
        is->read(seq, false); // Bypass string conversion.
#else
        is->read(seq, true);
#endif
        long sz = static_cast<long>(seq.size());
        result = createArray(sz);

        if(sz > 0)
        {
            for(long i = 0; i < sz; ++i)
            {
                RARRAY_ASET(result, i, createString(seq[i]));
            }
        }
        break;
    }
    }
    cb->unmarshaled(result, target, closure);
}

//
// DictionaryInfo implementation.
//
IceRuby::DictionaryInfo::DictionaryInfo(VALUE ident, VALUE kt, VALUE vt)
{
    const_cast<string&>(id) = getString(ident);
    const_cast<TypeInfoPtr&>(keyType) = getType(kt);
    const_cast<TypeInfoPtr&>(valueType) = getType(vt);

    _variableLength = keyType->variableLength() || valueType->variableLength();
    _wireSize = keyType->wireSize() + valueType->wireSize();
}

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
IceRuby::DictionaryInfo::variableLength() const
{
    return true;
}

int
IceRuby::DictionaryInfo::wireSize() const
{
    return 1;
}

Ice::OptionalFormat
IceRuby::DictionaryInfo::optionalFormat() const
{
    return _variableLength ? Ice::OptionalFormatFSize : Ice::OptionalFormatVSize;
}

bool
IceRuby::DictionaryInfo::usesClasses() const
{
    return valueType->usesClasses();
}

namespace
{
struct DictionaryMarshalIterator : public IceRuby::HashIterator
{
    DictionaryMarshalIterator(const IceRuby::DictionaryInfoPtr& d, Ice::OutputStream* o, IceRuby::ObjectMap* m)
        : dict(d), os(o), objectMap(m)
    {
    }

    virtual void element(VALUE key, VALUE value)
    {
        dict->marshalElement(key, value, os, objectMap);
    }

    IceRuby::DictionaryInfoPtr dict;
    Ice::OutputStream* os;
    IceRuby::ObjectMap* objectMap;
};
}

void
IceRuby::DictionaryInfo::marshal(VALUE p, Ice::OutputStream* os, ObjectMap* objectMap, bool optional)
{
    volatile VALUE hash = Qnil;

    if(!NIL_P(p))
    {
        hash = callRuby(rb_convert_type, p, T_HASH, "Hash", "to_hash");
        if(NIL_P(hash))
        {
            throw RubyException(rb_eTypeError, "unable to convert value to a hash");
        }
    }

    int sz = 0;
    if(!NIL_P(hash))
    {
        sz = RHASH_SIZE(hash);
    }

    Ice::OutputStream::size_type sizePos = 0;
    if(optional)
    {
        if(_variableLength)
        {
            sizePos = os->startSize();
        }
        else
        {
            os->writeSize(sz == 0 ? 1 : sz * _wireSize + (sz > 254 ? 5 : 1));
        }
    }

    if(NIL_P(hash))
    {
        os->writeSize(0);
    }
    else
    {
        os->writeSize(sz);
        if(sz > 0)
        {
            DictionaryMarshalIterator iter(this, os, objectMap);
            hashIterate(hash, iter);
        }
    }

    if(optional && _variableLength)
    {
        os->endSize(sizePos);
    }
}

void
IceRuby::DictionaryInfo::marshalElement(VALUE key, VALUE value, Ice::OutputStream* os, ObjectMap* objectMap)
{
    if(!keyType->validate(key))
    {
        throw RubyException(rb_eTypeError, "invalid key in `%s' element", const_cast<char*>(id.c_str()));
    }

    if(!valueType->validate(value))
    {
        throw RubyException(rb_eTypeError, "invalid value in `%s' element", const_cast<char*>(id.c_str()));
    }

    keyType->marshal(key, os, objectMap, false);
    valueType->marshal(value, os, objectMap, false);
}

void
IceRuby::DictionaryInfo::unmarshal(Ice::InputStream* is, const UnmarshalCallbackPtr& cb, VALUE target,
                                   void* closure, bool optional)
{
    if(optional)
    {
        if(_variableLength)
        {
            is->skip(4);
        }
        else
        {
            is->skipSize();
        }
    }

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
        keyType->unmarshal(is, keyCB, Qnil, 0, false);
        assert(!NIL_P(keyCB->key));

        //
        // The callback will set the dictionary entry with the unmarshaled value,
        // so we pass it the key.
        //
        void* cl = reinterpret_cast<void*>(keyCB->key);
        valueType->unmarshal(is, this, hash, cl, false);
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
    DictionaryPrintIterator(const DictionaryInfoPtr& d, IceUtilInternal::Output& o, PrintObjectHistory* h) :
        dict(d), out(o), history(h)
    {
    }

    virtual void element(VALUE key, VALUE value)
    {
        dict->printElement(key, value, out, history);
    }

    IceRuby::DictionaryInfoPtr dict;
    IceUtilInternal::Output& out;
    IceRuby::PrintObjectHistory* history;
};
}

void
IceRuby::DictionaryInfo::print(VALUE value, IceUtilInternal::Output& out, PrintObjectHistory* history)
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

        if(RHASH_SIZE(hash) == 0)
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
IceRuby::DictionaryInfo::printElement(VALUE key, VALUE value, IceUtilInternal::Output& out, PrintObjectHistory* history)
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
        const_cast<TypeInfoPtr&>(keyType) = 0;
    }
    if(valueType)
    {
        valueType->destroy();
        const_cast<TypeInfoPtr&>(valueType) = 0;
    }
}

//
// ClassInfo implementation.
//
IceRuby::ClassInfo::ClassInfo(VALUE ident, bool loc) :
    compactId(-1), isBase(false), isLocal(loc), preserve(false), interface(false), rubyClass(Qnil), typeObj(Qnil),
    defined(false)
{
    const_cast<string&>(id) = getString(ident);
    if(isLocal)
    {
        const_cast<bool&>(isBase) = id == "::Ice::LocalObject";
    }
    else
    {
        const_cast<bool&>(isBase) = id == Ice::Object::ice_staticId();
    }
    const_cast<VALUE&>(typeObj) = createType(this);
}

void
IceRuby::ClassInfo::define(VALUE t, VALUE compact, VALUE pres, VALUE intf, VALUE b, VALUE m)
{
    if(!NIL_P(b))
    {
        const_cast<ClassInfoPtr&>(base) = ClassInfoPtr::dynamicCast(getType(b));
        assert(base);
    }

    const_cast<Ice::Int&>(compactId) = static_cast<Ice::Int>(getInteger(compact));
    const_cast<bool&>(preserve) = RTEST(pres);
    const_cast<bool&>(interface) = RTEST(intf);
    convertDataMembers(m, const_cast<DataMemberList&>(members), const_cast<DataMemberList&>(optionalMembers), true);
    const_cast<VALUE&>(rubyClass) = t;
    const_cast<bool&>(defined) = true;
}

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
    volatile VALUE type = Qnil;
    try
    {
        type = callRuby(rb_const_get, cls, rb_intern("ICE_TYPE"));
    }
    catch(const RubyException& ex)
    {
        if(callRuby(rb_obj_is_instance_of, ex.ex, rb_eNameError) == Qtrue)
        {
            //
            // The ICE_TYPE constant will be missing from an instance of LocalObject
            // if it does not implement a user-defined type. This means the user
            // could potentially pass any kind of object; there isn't much we can do
            // since LocalObject maps to the base object type.
            //
            return id == "::Ice::LocalObject";
        }
        else
        {
            throw;
        }
    }
    assert(!NIL_P(type));
    ClassInfoPtr info = ClassInfoPtr::dynamicCast(getType(type));
    assert(info);
    return this->interface || info->isA(this);
}

bool
IceRuby::ClassInfo::variableLength() const
{
    return true;
}

int
IceRuby::ClassInfo::wireSize() const
{
    return 1;
}

Ice::OptionalFormat
IceRuby::ClassInfo::optionalFormat() const
{
    return Ice::OptionalFormatClass;
}

bool
IceRuby::ClassInfo::usesClasses() const
{
    return true;
}

void
IceRuby::ClassInfo::marshal(VALUE p, Ice::OutputStream* os, ObjectMap* objectMap, bool)
{
    if(!defined)
    {
        throw RubyException(rb_eRuntimeError, "class %s is declared but not defined", id.c_str());
    }

    if(NIL_P(p))
    {
        Ice::ObjectPtr nil;
        os->write(nil);
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
        writer = new ObjectWriter(p, objectMap, this);
        objectMap->insert(ObjectMap::value_type(p, writer));
    }
    else
    {
        writer = q->second;
    }

    //
    // Give the writer to the stream. The stream will eventually call write() on it.
    //
    os->write(writer);
}

namespace
{

void
patchObject(void* addr, const Ice::ObjectPtr& v)
{
    ReadObjectCallback* cb = static_cast<ReadObjectCallback*>(addr);
    assert(cb);
    cb->invoke(v);
}

}

void
IceRuby::ClassInfo::unmarshal(Ice::InputStream* is, const UnmarshalCallbackPtr& cb, VALUE target, void* closure, bool)
{
    if(!defined)
    {
        throw RubyException(rb_eRuntimeError, "class %s is declared but not defined", id.c_str());
    }

    //
    // This callback is notified when the Slice value is actually read. The StreamUtil object
    // attached to the stream keeps a reference to the callback object to ensure it lives
    // long enough.
    //
    ReadObjectCallbackPtr rocb = new ReadObjectCallback(this, cb, target, closure);
    StreamUtil* util = reinterpret_cast<StreamUtil*>(is->getClosure());
    assert(util);
    util->add(rocb);
    is->read(patchObject, rocb.get());
}

void
IceRuby::ClassInfo::print(VALUE value, IceUtilInternal::Output& out, PrintObjectHistory* history)
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
            volatile VALUE type = Qnil;
            ClassInfoPtr info;
            try
            {
                type = callRuby(rb_const_get, cls, rb_intern("ICE_TYPE"));
                info = ClassInfoPtr::dynamicCast(getType(type));
            }
            catch(const RubyException& ex)
            {
                if(callRuby(rb_obj_is_instance_of, ex.ex, rb_eNameError) == Qtrue)
                {
                    //
                    // The ICE_TYPE constant will be missing from an instance of LocalObject
                    // if it does not implement a user-defined type. This means the user
                    // could potentially pass any kind of object; there isn't much we can do
                    // since LocalObject maps to the base object type.
                    //
                    if(id == "::Ice::LocalObject")
                    {
                        info = this;
                    }
                    else
                    {
                        out << "<invalid value - expected " << id << ">";
                        return;
                    }
                }
                else
                {
                    throw;
                }
            }
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
    const_cast<ClassInfoPtr&>(base) = 0;
    if(!members.empty())
    {
        DataMemberList ml = members;
        const_cast<DataMemberList&>(members).clear();
        for(DataMemberList::iterator p = ml.begin(); p != ml.end(); ++p)
        {
            (*p)->type->destroy();
        }
    }
}

void
IceRuby::ClassInfo::printMembers(VALUE value, IceUtilInternal::Output& out, PrintObjectHistory* history)
{
    if(base)
    {
        base->printMembers(value, out, history);
    }

    DataMemberList::const_iterator q;

    for(q = members.begin(); q != members.end(); ++q)
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

    for(q = optionalMembers.begin(); q != optionalMembers.end(); ++q)
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
            if(val == Unset)
            {
                out << "<unset>";
            }
            else
            {
                member->type->print(val, out, history);
            }
        }
    }
}

bool
IceRuby::ClassInfo::isA(const ClassInfoPtr& info)
{
    //
    // Return true if this class has an is-a relationship with info.
    //
    if(info->isBase && isLocal == info->isLocal)
    {
        return true;
    }
    else if(this == info.get())
    {
        return true;
    }

    return base && base->isA(info);
}

//
// ProxyInfo implementation.
//
IceRuby::ProxyInfo::ProxyInfo(VALUE ident) :
    isBase(false), rubyClass(Qnil), typeObj(Qnil)
{
    const_cast<string&>(id) = getString(ident);
    const_cast<bool&>(isBase) = id == "::Ice::Object";
    const_cast<VALUE&>(typeObj) = createType(this);
}

void
IceRuby::ProxyInfo::define(VALUE t, VALUE b, VALUE i)
{
    if(!NIL_P(b))
    {
        const_cast<ProxyInfoPtr&>(base) = ProxyInfoPtr::dynamicCast(getType(b));
        assert(base);
    }

    volatile VALUE arr = callRuby(rb_check_array_type, i);
    assert(!NIL_P(arr));
    for(int n = 0; n < RARRAY_LEN(arr); ++n)
    {
        ProxyInfoPtr iface = ProxyInfoPtr::dynamicCast(getType(RARRAY_AREF(arr, n)));
        assert(iface);
        const_cast<ProxyInfoList&>(interfaces).push_back(iface);
    }

    const_cast<VALUE&>(rubyClass) = t;
}

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
        return info->isA(this);
    }
    return true;
}

bool
IceRuby::ProxyInfo::variableLength() const
{
    return true;
}

int
IceRuby::ProxyInfo::wireSize() const
{
    return 1;
}

Ice::OptionalFormat
IceRuby::ProxyInfo::optionalFormat() const
{
    return Ice::OptionalFormatFSize;
}

void
IceRuby::ProxyInfo::marshal(VALUE p, Ice::OutputStream* os, ObjectMap*, bool optional)
{
    Ice::OutputStream::size_type sizePos = -1;
    if(optional)
    {
        sizePos = os->startSize();
    }

    if(NIL_P(p))
    {
        os->write(Ice::ObjectPrx());
    }
    else
    {
        assert(checkProxy(p)); // validate() should have caught this.
        os->write(getProxy(p));
    }

    if(optional)
    {
        os->endSize(sizePos);
    }
}

void
IceRuby::ProxyInfo::unmarshal(Ice::InputStream* is, const UnmarshalCallbackPtr& cb, VALUE target,
                              void* closure, bool optional)
{
    if(optional)
    {
        is->skip(4);
    }

    Ice::ObjectPrx proxy;
    is->read(proxy);

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
IceRuby::ProxyInfo::print(VALUE value, IceUtilInternal::Output& out, PrintObjectHistory*)
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

bool
IceRuby::ProxyInfo::isA(const ProxyInfoPtr& info)
{
    //
    // Return true if this class has an is-a relationship with info.
    //
    if(info->isBase)
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
        for(ProxyInfoList::const_iterator p = interfaces.begin(); p != interfaces.end(); ++p)
        {
            if((*p)->isA(info))
            {
                return true;
            }
        }
    }

    return false;
}

void
IceRuby::ProxyInfo::destroy()
{
    const_cast<ProxyInfoPtr&>(base) = 0;
    const_cast<ProxyInfoList&>(interfaces).clear();
}

//
// ObjectWriter implementation.
//
IceRuby::ObjectWriter::ObjectWriter(VALUE object, ObjectMap* objectMap, const ClassInfoPtr& formal) :
    _object(object), _map(objectMap), _formal(formal)
{
    //
    // Mark the object as in use for the lifetime of this wrapper.
    //
    rb_gc_register_address(&_object);
    if(!_formal || !_formal->interface)
    {
        volatile VALUE cls = CLASS_OF(object);
        volatile VALUE type = callRuby(rb_const_get, cls, rb_intern("ICE_TYPE"));
        assert(!NIL_P(type));
        _info = ClassInfoPtr::dynamicCast(getType(type));
        assert(_info);
    }
}

IceRuby::ObjectWriter::~ObjectWriter()
{
    rb_gc_unregister_address(&_object);
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
IceRuby::ObjectWriter::_iceWrite(Ice::OutputStream* os) const
{
    Ice::SlicedDataPtr slicedData;

    if(_info && _info->preserve)
    {
        //
        // Retrieve the SlicedData object that we stored as a hidden member of the Ruby object.
        //
        slicedData = StreamUtil::getSlicedDataMember(_object, const_cast<ObjectMap*>(_map));
    }

    os->startValue(slicedData);
    if(_formal && _formal->interface)
    {
        ID op = rb_intern("ice_id");
        string id = getString(callRuby(rb_funcall, _object, op, 0));
        os->startSlice(id, -1, true);
        os->endSlice();
    }
    else
    {
        if(_info->id != "::Ice::UnknownSlicedValue")
        {
            ClassInfoPtr info = _info;
            while(info)
            {
                os->startSlice(info->id, info->compactId, !info->base);

                writeMembers(os, info->members);
                writeMembers(os, info->optionalMembers); // The optional members have already been sorted by tag.

                os->endSlice();

                info = info->base;
            }
        }
    }
    os->endValue();
}

void
IceRuby::ObjectWriter::_iceRead(Ice::InputStream*)
{
    assert(false);
}

void
IceRuby::ObjectWriter::writeMembers(Ice::OutputStream* os, const DataMemberList& members) const
{
    for(DataMemberList::const_iterator q = members.begin(); q != members.end(); ++q)
    {
        DataMemberPtr member = *q;

        volatile VALUE val = callRuby(rb_ivar_get, _object, member->rubyID);

        if(member->optional && (val == Unset || !os->writeOptional(member->tag, member->type->optionalFormat())))
        {
            continue;
        }

        if(!member->type->validate(val))
        {
            throw RubyException(rb_eTypeError, "invalid value for %s member `%s'", _info->id.c_str(),
                                member->name.c_str());
        }

        member->type->marshal(val, os, _map, member->optional);
    }
}

//
// ObjectReader implementation.
//
IceRuby::ObjectReader::ObjectReader(VALUE object, const ClassInfoPtr& info) :
    _object(object), _info(info)
{
    //
    // Mark the object as in use for the lifetime of this wrapper.
    //
    rb_gc_register_address(&_object);
}

IceRuby::ObjectReader::~ObjectReader()
{
    rb_gc_unregister_address(&_object);
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
IceRuby::ObjectReader::_iceWrite(Ice::OutputStream*) const
{
    assert(false);
}

void
IceRuby::ObjectReader::_iceRead(Ice::InputStream* is)
{
    is->startValue();

    const bool unknown = _info->id == "::Ice::UnknownSlicedValue";

    //
    // Unmarshal the slices of a user-defined class.
    //
    if(!unknown && _info->id != Ice::Object::ice_staticId())
    {
        ClassInfoPtr info = _info;
        while(info)
        {
            is->startSlice();

            DataMemberList::const_iterator p;

            for(p = info->members.begin(); p != info->members.end(); ++p)
            {
                DataMemberPtr member = *p;
                member->type->unmarshal(is, member, _object, 0, false);
            }

            //
            // The optional members have already been sorted by tag.
            //
            for(p = info->optionalMembers.begin(); p != info->optionalMembers.end(); ++p)
            {
                DataMemberPtr member = *p;
                if(is->readOptional(member->tag, member->type->optionalFormat()))
                {
                    member->type->unmarshal(is, member, _object, 0, true);
                }
                else
                {
                    callRuby(rb_ivar_set, _object, member->rubyID, Unset);
                }
            }

            is->endSlice();

            info = info->base;
        }
    }

    _slicedData = is->endValue(_info->preserve);

    if(_slicedData)
    {
        StreamUtil* util = reinterpret_cast<StreamUtil*>(is->getClosure());
        assert(util);
        util->add(this);

        //
        // Define the "unknownTypeId" member for an instance of UnknownSlicedValue.
        //
        if(unknown)
        {
            assert(!_slicedData->slices.empty());

            volatile VALUE typeId = createString(_slicedData->slices[0]->typeId);
            callRuby(rb_iv_set, _object, "@unknownTypeId", typeId);
        }
    }
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

Ice::SlicedDataPtr
IceRuby::ObjectReader::getSlicedData() const
{
    return _slicedData;
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
    _compactIdMap.clear();
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
        if(!_info->interface && !_info->validate(obj))
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
VALUE
IceRuby::ExceptionInfo::unmarshal(Ice::InputStream* is)
{
    volatile VALUE obj = callRuby(rb_class_new_instance, 0, static_cast<VALUE*>(0), rubyClass);

    ExceptionInfoPtr info = this;
    while(info)
    {
        is->startSlice();

        DataMemberList::iterator q;

        for(q = info->members.begin(); q != info->members.end(); ++q)
        {
            DataMemberPtr member = *q;
            member->type->unmarshal(is, member, obj, 0, false);
        }

        //
        // The optional members have already been sorted by tag.
        //
        for(q = info->optionalMembers.begin(); q != info->optionalMembers.end(); ++q)
        {
            DataMemberPtr member = *q;
            if(is->readOptional(member->tag, member->type->optionalFormat()))
            {
                member->type->unmarshal(is, member, obj, 0, true);
            }
            else
            {
                callRuby(rb_ivar_set, obj, member->rubyID, Unset);
            }
        }

        is->endSlice();

        info = info->base;
    }

    return obj;
}

void
IceRuby::ExceptionInfo::print(VALUE value, IceUtilInternal::Output& out)
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
IceRuby::ExceptionInfo::printMembers(VALUE value, IceUtilInternal::Output& out, PrintObjectHistory* history)
{
    if(base)
    {
        base->printMembers(value, out, history);
    }

    DataMemberList::const_iterator q;

    for(q = members.begin(); q != members.end(); ++q)
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

    for(q = optionalMembers.begin(); q != optionalMembers.end(); ++q)
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
            if(val == Unset)
            {
                out << "<unset>";
            }
            else
            {
                member->type->print(val, out, history);
            }
        }
    }
}

//
// ExceptionReader implementation.
//
IceRuby::ExceptionReader::ExceptionReader(const ExceptionInfoPtr& info) :
    _info(info)
{
}

IceRuby::ExceptionReader::~ExceptionReader()
    throw()
{
}

string
IceRuby::ExceptionReader::ice_id() const
{
    return _info->id;
}

#ifndef ICE_CPP11_MAPPING
Ice::UserException*
IceRuby::ExceptionReader::ice_clone() const
{
    assert(false);
    return 0;
}
#endif

void
IceRuby::ExceptionReader::ice_throw() const
{
    throw *this;
}

void
IceRuby::ExceptionReader::_write(Ice::OutputStream*) const
{
    assert(false);
}

void
IceRuby::ExceptionReader::_read(Ice::InputStream* is)
{
    is->startException();

    const_cast<VALUE&>(_ex) = _info->unmarshal(is);

    const_cast<Ice::SlicedDataPtr&>(_slicedData) = is->endException(_info->preserve);
}

bool
IceRuby::ExceptionReader::_usesClasses() const
{
    return _info->usesClasses;
}

VALUE
IceRuby::ExceptionReader::getException() const
{
    return _ex;
}

Ice::SlicedDataPtr
IceRuby::ExceptionReader::getSlicedData() const
{
    return _slicedData;
}

//
// IdResolver
//
string
IceRuby::IdResolver::resolve(Ice::Int id) const
{
    CompactIdMap::iterator p = _compactIdMap.find(id);
    if(p != _compactIdMap.end())
    {
        return p->second->id;
    }
    return string();
}

extern "C"
VALUE
IceRuby_defineEnum(VALUE /*self*/, VALUE id, VALUE type, VALUE enumerators)
{
    ICE_RUBY_TRY
    {
        EnumInfoPtr info = new EnumInfo(id, type, enumerators);
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
        StructInfoPtr info = new StructInfo(id, type, members);
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
        SequenceInfoPtr info = new SequenceInfo(id, elementType);
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
        DictionaryInfoPtr info = new DictionaryInfo(id, keyType, valueType);
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
            info = new ProxyInfo(id);
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
            info = new ClassInfo(id, false);
            addClassInfo(idstr, info);
        }

        return info->typeObj;
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_declareLocalClass(VALUE /*self*/, VALUE id)
{
    ICE_RUBY_TRY
    {
        string idstr = getString(id);
        ClassInfoPtr info = lookupClassInfo(idstr);
        if(!info)
        {
            info = new ClassInfo(id, true);
            addClassInfo(idstr, info);
        }

        return info->typeObj;
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_defineException(VALUE /*self*/, VALUE id, VALUE type, VALUE preserve, VALUE base, VALUE members)
{
    ICE_RUBY_TRY
    {
        ExceptionInfoPtr info = new ExceptionInfo;
        info->id = getString(id);

        info->preserve = preserve == Qtrue;

        if(!NIL_P(base))
        {
            info->base = ExceptionInfoPtr::dynamicCast(getException(base));
            assert(info->base);
        }

        convertDataMembers(members, info->members, info->optionalMembers, true);

        info->usesClasses = false;

        //
        // Only examine the required members to see if any use classes.
        //
        for(DataMemberList::iterator p = info->members.begin(); p != info->members.end(); ++p)
        {
            if(!info->usesClasses)
            {
                info->usesClasses = (*p)->type->usesClasses();
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
IceRuby_TypeInfo_defineProxy(VALUE self, VALUE type, VALUE base, VALUE interfaces)
{
    ICE_RUBY_TRY
    {
        ProxyInfoPtr info = ProxyInfoPtr::dynamicCast(getType(self));
        assert(info);

        info->define(type, base, interfaces);
        rb_define_const(type, "ICE_TYPE", self);
        rb_define_const(type, "ICE_ID", createString(info->id));
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_TypeInfo_defineClass(VALUE self, VALUE type, VALUE compactId, VALUE preserve, VALUE interface, VALUE base,
                             VALUE members)
{
    ICE_RUBY_TRY
    {
        ClassInfoPtr info = ClassInfoPtr::dynamicCast(getType(self));
        assert(info);

        info->define(type, compactId, preserve, interface, base, members);

        if(info->compactId != -1)
        {
            CompactIdMap::iterator q = _compactIdMap.find(info->compactId);
            if(q != _compactIdMap.end())
            {
                _compactIdMap.erase(q);
            }
            _compactIdMap.insert(CompactIdMap::value_type(info->compactId, info));
        }

        if(type != Qnil && !info->interface)
        {
            rb_define_const(type, "ICE_TYPE", self);
            rb_define_const(type, "ICE_ID", createString(info->id));
        }
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
        IceUtilInternal::Output out(ostr);
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
        IceUtilInternal::Output out(ostr);
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
    rb_define_module_function(iceModule, "__declareLocalClass", CAST_METHOD(IceRuby_declareLocalClass), 1);
    rb_define_module_function(iceModule, "__defineException", CAST_METHOD(IceRuby_defineException), 5);

    rb_define_method(_typeInfoClass, "defineClass", CAST_METHOD(IceRuby_TypeInfo_defineClass), 6);
    rb_define_method(_typeInfoClass, "defineProxy", CAST_METHOD(IceRuby_TypeInfo_defineProxy), 3);

    rb_define_module_function(iceModule, "__stringify", CAST_METHOD(IceRuby_stringify), 2);
    rb_define_module_function(iceModule, "__stringifyException", CAST_METHOD(IceRuby_stringifyException), 1);

    _unsetTypeClass = rb_define_class_under(iceModule, "Internal_UnsetType", rb_cObject);
    Unset = callRuby(rb_class_new_instance, 0, static_cast<VALUE*>(0), _unsetTypeClass);
    rb_define_const(iceModule, "Unset", Unset);

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
