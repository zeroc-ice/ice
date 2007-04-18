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
#include <Types.h>
#include <Current.h>
#include <Proxy.h>
#include <Util.h>
#include <IceUtil/InputUtil.h>
#include <IceUtil/ScopedArray.h>
#include <Ice/LocalException.h>

using namespace std;
using namespace IcePy;
using namespace IceUtil;

typedef map<string, ClassInfoPtr> ClassInfoMap;
static ClassInfoMap _classInfoMap;

typedef map<string, ProxyInfoPtr> ProxyInfoMap;
static ProxyInfoMap _proxyInfoMap;

typedef map<string, ExceptionInfoPtr> ExceptionInfoMap;
static ExceptionInfoMap _exceptionInfoMap;

namespace IcePy
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

    ReadObjectCallback(const ClassInfoPtr&, const UnmarshalCallbackPtr&, PyObject*, void*);
    ~ReadObjectCallback();

    virtual void invoke(const ::Ice::ObjectPtr&);

private:

    ClassInfoPtr _info;
    UnmarshalCallbackPtr _cb;
    PyObject* _target;
    void* _closure;
};

struct TypeInfoObject
{
    PyObject_HEAD
    IcePy::TypeInfoPtr* info;
};

struct ExceptionInfoObject
{
    PyObject_HEAD
    IcePy::ExceptionInfoPtr* info;
};

extern PyTypeObject TypeInfoType;
extern PyTypeObject ExceptionInfoType;

}

#ifdef WIN32
extern "C"
#endif
static TypeInfoObject*
typeInfoNew(PyObject* /*arg*/)
{
    TypeInfoObject* self = PyObject_New(TypeInfoObject, &TypeInfoType);
    if(!self)
    {
        return 0;
    }
    self->info = 0;
    return self;
}

#ifdef WIN32
extern "C"
#endif
static void
typeInfoDealloc(TypeInfoObject* self)
{
    delete self->info;
    PyObject_Del(self);
}

#ifdef WIN32
extern "C"
#endif
static ExceptionInfoObject*
exceptionInfoNew(PyObject* /*arg*/)
{
    ExceptionInfoObject* self = PyObject_New(ExceptionInfoObject, &ExceptionInfoType);
    if(!self)
    {
        return 0;
    }
    self->info = 0;
    return self;
}

#ifdef WIN32
extern "C"
#endif
static void
exceptionInfoDealloc(ExceptionInfoObject* self)
{
    delete self->info;
    PyObject_Del(self);
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
static IcePy::ProxyInfoPtr
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
IcePy::UnmarshalCallback::~UnmarshalCallback()
{
}

//
// TypeInfo implementation.
//
IcePy::TypeInfo::TypeInfo()
{
}

bool
IcePy::TypeInfo::usesClasses()
{
    return false;
}

void
IcePy::TypeInfo::unmarshaled(PyObject*, PyObject*, void*)
{
    assert(false);
}

void
IcePy::TypeInfo::destroy()
{
}

//
// PrimitiveInfo implementation.
//
string
IcePy::PrimitiveInfo::getId() const
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
IcePy::PrimitiveInfo::validate(PyObject* p)
{
    switch(kind)
    {
    case PrimitiveInfo::KindBool:
    {
        int isTrue = PyObject_IsTrue(p);
        if(isTrue < 0)
        {
            return false;
        }
        break;
    }
    case PrimitiveInfo::KindByte:
    {
        long val;
        if(PyInt_Check(p))
        {
            val = PyInt_AS_LONG(p);
        }
        else if(PyLong_Check(p))
        {
            val = PyLong_AsLong(p);
        }
        else
        {
            return false;
        }

        if(PyErr_Occurred() || val < 0 || val > 255)
        {
            return false;
        }
        break;
    }
    case PrimitiveInfo::KindShort:
    {
        long val;
        if(PyInt_Check(p))
        {
            val = PyInt_AS_LONG(p);
        }
        else if(PyLong_Check(p))
        {
            val = PyLong_AsLong(p);
        }
        else
        {
            return false;
        }

        if(PyErr_Occurred() || val < SHRT_MIN || val > SHRT_MAX)
        {
            return false;
        }
        break;
    }
    case PrimitiveInfo::KindInt:
    {
        long val;
        if(PyInt_Check(p))
        {
            val = PyInt_AS_LONG(p);
        }
        else if(PyLong_Check(p))
        {
            val = PyLong_AsLong(p);
        }
        else
        {
            return false;
        }

        if(PyErr_Occurred() || val < INT_MIN || val > INT_MAX)
        {
            return false;
        }
        break;
    }
    case PrimitiveInfo::KindLong:
    {
        if(!PyInt_Check(p) && !PyLong_Check(p))
        {
            return false;
        }

        PyLong_AsLongLong(p);
        if(PyErr_Occurred())
        {
            return false;
        }

        break;
    }
    case PrimitiveInfo::KindFloat:
    case PrimitiveInfo::KindDouble:
    {
        if(!PyFloat_Check(p))
        {
            return false;
        }
        break;
    }
    case PrimitiveInfo::KindString:
    {
        if(p != Py_None && !PyString_Check(p))
        {
            return false;
        }
        break;
    }
    }

    return true;
}

void
IcePy::PrimitiveInfo::marshal(PyObject* p, const Ice::OutputStreamPtr& os, ObjectMap*, const Ice::StringSeq*)
{
    switch(kind)
    {
    case PrimitiveInfo::KindBool:
    {
        int isTrue = PyObject_IsTrue(p);
        if(isTrue < 0)
        {
            assert(false); // validate() should have caught this.
        }
        os->writeBool(isTrue ? true : false);
        break;
    }
    case PrimitiveInfo::KindByte:
    {
        long val = 0;
        if(PyInt_Check(p))
        {
            val = PyInt_AS_LONG(p);
        }
        else if(PyLong_Check(p))
        {
            val = PyLong_AsLong(p);
        }
        else
        {
            assert(false); // validate() should have caught this.
        }

        assert(!PyErr_Occurred()); // validate() should have caught this.
        assert(val >= 0 && val <= 255); // validate() should have caught this.
        os->writeByte(static_cast<Ice::Byte>(val));
        break;
    }
    case PrimitiveInfo::KindShort:
    {
        long val = 0;
        if(PyInt_Check(p))
        {
            val = PyInt_AS_LONG(p);
        }
        else if(PyLong_Check(p))
        {
            val = PyLong_AsLong(p);
        }
        else
        {
            assert(false); // validate() should have caught this.
        }

        assert(!PyErr_Occurred()); // validate() should have caught this.
        assert(val >= SHRT_MIN && val <= SHRT_MAX); // validate() should have caught this.
        os->writeShort(static_cast<Ice::Short>(val));
        break;
    }
    case PrimitiveInfo::KindInt:
    {
        long val = 0;
        if(PyInt_Check(p))
        {
            val = PyInt_AS_LONG(p);
        }
        else if(PyLong_Check(p))
        {
            val = PyLong_AsLong(p);
        }
        else
        {
            assert(false); // validate() should have caught this.
        }

        assert(!PyErr_Occurred()); // validate() should have caught this.
        assert(val >= INT_MIN && val <= INT_MAX); // validate() should have caught this.
        os->writeInt(static_cast<Ice::Int>(val));
        break;
    }
    case PrimitiveInfo::KindLong:
    {
        Ice::Long val = 0;
        if(PyInt_Check(p))
        {
            val = PyInt_AS_LONG(p);
        }
        else if(PyLong_Check(p))
        {
            val = PyLong_AsLongLong(p);
        }
        else
        {
            assert(false); // validate() should have caught this.
        }

        assert(!PyErr_Occurred()); // validate() should have caught this.
        os->writeLong(val);
        break;
    }
    case PrimitiveInfo::KindFloat:
    {
        float val = 0;
        if(PyFloat_Check(p))
        {
            val = static_cast<float>(PyFloat_AS_DOUBLE(p));
        }
        else
        {
            assert(false); // validate() should have caught this.
        }

        os->writeFloat(val);
        break;
    }
    case PrimitiveInfo::KindDouble:
    {
        double val = 0;
        if(PyFloat_Check(p))
        {
            val = PyFloat_AS_DOUBLE(p);
        }
        else
        {
            assert(false);
        }

        os->writeDouble(val);
        break;
    }
    case PrimitiveInfo::KindString:
    {
        string val;
        if(PyString_Check(p))
        {
            val = string(PyString_AS_STRING(p), PyString_GET_SIZE(p));
        }
        else if(p != Py_None)
        {
            assert(false); // validate() should have caught this.
        }

        os->writeString(val);
        break;
    }
    }
}

void
IcePy::PrimitiveInfo::unmarshal(const Ice::InputStreamPtr& is, const UnmarshalCallbackPtr& cb, PyObject* target,
                                void* closure, const Ice::StringSeq*)
{
    switch(kind)
    {
    case PrimitiveInfo::KindBool:
    {
        if(is->readBool())
        {
            cb->unmarshaled(getTrue(), target, closure);
        }
        else
        {
            cb->unmarshaled(getFalse(), target, closure);
        }
        break;
    }
    case PrimitiveInfo::KindByte:
    {
        Ice::Byte val = is->readByte();
        PyObjectHandle p = PyInt_FromLong(val);
        cb->unmarshaled(p.get(), target, closure);
        break;
    }
    case PrimitiveInfo::KindShort:
    {
        Ice::Short val = is->readShort();
        PyObjectHandle p = PyInt_FromLong(val);
        cb->unmarshaled(p.get(), target, closure);
        break;
    }
    case PrimitiveInfo::KindInt:
    {
        Ice::Int val = is->readInt();
        PyObjectHandle p = PyInt_FromLong(val);
        cb->unmarshaled(p.get(), target, closure);
        break;
    }
    case PrimitiveInfo::KindLong:
    {
        Ice::Long val = is->readLong();
        PyObjectHandle p = PyLong_FromLongLong(val);
        cb->unmarshaled(p.get(), target, closure);
        break;
    }
    case PrimitiveInfo::KindFloat:
    {
        Ice::Float val = is->readFloat();
        PyObjectHandle p = PyFloat_FromDouble(val);
        cb->unmarshaled(p.get(), target, closure);
        break;
    }
    case PrimitiveInfo::KindDouble:
    {
        Ice::Double val = is->readDouble();
        PyObjectHandle p = PyFloat_FromDouble(val);
        cb->unmarshaled(p.get(), target, closure);
        break;
    }
    case PrimitiveInfo::KindString:
    {
        string val = is->readString();
        PyObjectHandle p = PyString_FromString(val.c_str());
        cb->unmarshaled(p.get(), target, closure);
        break;
    }
    }
}

void
IcePy::PrimitiveInfo::print(PyObject* value, IceUtil::Output& out, PrintObjectHistory*)
{
    if(!validate(value))
    {
        out << "<invalid value - expected " << getId() << ">";
        return;
    }
    PyObjectHandle p = PyObject_Str(value);
    if(!p.get())
    {
        return;
    }
    assert(PyString_Check(p.get()));
    out << PyString_AS_STRING(p.get());
}

//
// EnumInfo implementation.
//
string
IcePy::EnumInfo::getId() const
{
    return id;
}

bool
IcePy::EnumInfo::validate(PyObject* val)
{
    return PyObject_IsInstance(val, pythonType.get()) == 1;
}

void
IcePy::EnumInfo::marshal(PyObject* p, const Ice::OutputStreamPtr& os, ObjectMap*, const Ice::StringSeq*)
{
    assert(PyObject_IsInstance(p, pythonType.get()) == 1); // validate() should have caught this.

    //
    // Validate value.
    //
    PyObjectHandle val = PyObject_GetAttrString(p, STRCAST("value"));
    if(!val.get())
    {
        throw AbortMarshaling();
    }
    if(!PyInt_Check(val.get()))
    {
        PyErr_Format(PyExc_ValueError, STRCAST("value for enum %s is not an int"), id.c_str());
        throw AbortMarshaling();
    }
    int ival = static_cast<int>(PyInt_AsLong(val.get()));
    int count = static_cast<int>(enumerators.size());
    if(ival < 0 || ival >= count)
    {
        PyErr_Format(PyExc_ValueError, STRCAST("value %d is out of range for enum %s"), ival, id.c_str());
        throw AbortMarshaling();
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
IcePy::EnumInfo::unmarshal(const Ice::InputStreamPtr& is, const UnmarshalCallbackPtr& cb, PyObject* target,
                           void* closure, const Ice::StringSeq*)
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
        PyErr_Format(PyExc_ValueError, STRCAST("enumerator %d is out of range for enum %s"), val, id.c_str());
        throw AbortMarshaling();
    }

    PyObject* pyval = enumerators[val].get();
    assert(pyval);
    cb->unmarshaled(pyval, target, closure);
}

void
IcePy::EnumInfo::print(PyObject* value, IceUtil::Output& out, PrintObjectHistory*)
{
    if(!validate(value))
    {
        out << "<invalid value - expected " << id << ">";
        return;
    }
    PyObjectHandle p = PyObject_Str(value);
    if(!p.get())
    {
        return;
    }
    assert(PyString_Check(p.get()));
    out << PyString_AS_STRING(p.get());
}

//
// DataMember implementation.
//
void
IcePy::DataMember::unmarshaled(PyObject* val, PyObject* target, void*)
{
    if(PyObject_SetAttrString(target, const_cast<char*>(name.c_str()), val) < 0)
    {
        throw AbortMarshaling();
    }
}

//
// StructInfo implementation.
//
string
IcePy::StructInfo::getId() const
{
    return id;
}

bool
IcePy::StructInfo::validate(PyObject* val)
{
    return PyObject_IsInstance(val, pythonType.get()) == 1;
}

bool
IcePy::StructInfo::usesClasses()
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
IcePy::StructInfo::marshal(PyObject* p, const Ice::OutputStreamPtr& os, ObjectMap* objectMap, const Ice::StringSeq*)
{
    assert(PyObject_IsInstance(p, pythonType.get()) == 1); // validate() should have caught this.

    for(DataMemberList::iterator q = members.begin(); q != members.end(); ++q)
    {
        DataMemberPtr member = *q;
        char* memberName = const_cast<char*>(member->name.c_str());
        PyObjectHandle attr = PyObject_GetAttrString(p, memberName);
        if(!attr.get())
        {
            PyErr_Clear();
            PyErr_Format(PyExc_AttributeError, STRCAST("no member `%s' found in %s value"), memberName,
                         const_cast<char*>(id.c_str()));
            throw AbortMarshaling();
        }
        if(!member->type->validate(attr.get()))
        {
            PyErr_Format(PyExc_ValueError, STRCAST("invalid value for %s member `%s'"), const_cast<char*>(id.c_str()),
                         memberName);
            throw AbortMarshaling();
        }
        member->type->marshal(attr.get(), os, objectMap, &member->metaData);
    }
}

void
IcePy::StructInfo::unmarshal(const Ice::InputStreamPtr& is, const UnmarshalCallbackPtr& cb, PyObject* target,
                             void* closure, const Ice::StringSeq*)
{
    PyObjectHandle args = PyTuple_New(0);
    PyTypeObject* type = reinterpret_cast<PyTypeObject*>(pythonType.get());
    PyObjectHandle p = type->tp_new(type, args.get(), 0);
    if(!p.get())
    {
        throw AbortMarshaling();
    }

    for(DataMemberList::iterator q = members.begin(); q != members.end(); ++q)
    {
        DataMemberPtr member = *q;
        member->type->unmarshal(is, member, p.get(), 0, &member->metaData);
    }

    cb->unmarshaled(p.get(), target, closure);
}

void
IcePy::StructInfo::print(PyObject* value, IceUtil::Output& out, PrintObjectHistory* history)
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
        char* memberName = const_cast<char*>(member->name.c_str());
        PyObjectHandle attr = PyObject_GetAttrString(value, memberName);
        out << nl << member->name << " = ";
        if(!attr.get())
        {
            out << "<not defined>";
        }
        else
        {
            member->type->print(attr.get(), out, history);
        }
    }
    out.eb();
}

void
IcePy::StructInfo::destroy()
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
IcePy::SequenceInfo::getId() const
{
    return id;
}

bool
IcePy::SequenceInfo::validate(PyObject* val)
{
    return val == Py_None || PySequence_Check(val) == 1;
}

bool
IcePy::SequenceInfo::usesClasses()
{
    return elementType->usesClasses();
}

void
IcePy::SequenceInfo::marshal(PyObject* p, const Ice::OutputStreamPtr& os, ObjectMap* objectMap,
                             const Ice::StringSeq* metaData)
{
    if(p == Py_None)
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

    PyObjectHandle fastSeq = PySequence_Fast(p, STRCAST("expected a sequence value"));
    if(!fastSeq.get())
    {
        return;
    }

    Py_ssize_t sz = PySequence_Fast_GET_SIZE(fastSeq.get());
    os->writeSize(static_cast<int>(sz));
    for(Py_ssize_t i = 0; i < sz; ++i)
    {
        PyObject* item = PySequence_Fast_GET_ITEM(fastSeq.get(), i);
        if(!item)
        {
            throw AbortMarshaling();
        }
        if(!elementType->validate(item))
        {
            PyErr_Format(PyExc_ValueError, STRCAST("invalid value for element %d of `%s'"), static_cast<int>(i),
                         const_cast<char*>(id.c_str()));
            throw AbortMarshaling();
        }
        elementType->marshal(item, os, objectMap);
    }
}

void
IcePy::SequenceInfo::unmarshal(const Ice::InputStreamPtr& is, const UnmarshalCallbackPtr& cb, PyObject* target,
                               void* closure, const Ice::StringSeq* metaData)
{
    //
    // Determine the mapping to use for this sequence. Highest priority is given
    // to the metaData argument, otherwise we use the mapping of the sequence
    // definition.
    //
    SequenceMappingPtr sm;
    if(metaData)
    {
        SequenceMapping::Type type;
        if(!SequenceMapping::getType(*metaData, type) || type == mapping->type)
        {
            sm = mapping;
        }
        else
        {
            sm = new SequenceMapping(type);
        }
    }
    else
    {
        sm = mapping;
    }

    PrimitiveInfoPtr pi = PrimitiveInfoPtr::dynamicCast(elementType);
    if(pi)
    {
        unmarshalPrimitiveSequence(pi, is, cb, target, closure, sm);
        return;
    }

    Ice::Int sz = is->readSize();
    PyObjectHandle result = sm->createContainer(sz);

    if(!result.get())
    {
        throw AbortMarshaling();
    }

    for(Ice::Int i = 0; i < sz; ++i)
    {
        void* cl = reinterpret_cast<void*>(i);
        elementType->unmarshal(is, sm, result.get(), cl);
    }

    cb->unmarshaled(result.get(), target, closure);
}

void
IcePy::SequenceInfo::print(PyObject* value, IceUtil::Output& out, PrintObjectHistory* history)
{
    if(!validate(value))
    {
        out << "<invalid value - expected " << id << ">";
        return;
    }

    if(value == Py_None)
    {
        out << "{}";
    }
    else
    {
        PyObjectHandle fastSeq = PySequence_Fast(value, STRCAST("expected a sequence value"));
        if(!fastSeq.get())
        {
            return;
        }

        Py_ssize_t sz = PySequence_Fast_GET_SIZE(fastSeq.get());

        out.sb();
        for(Py_ssize_t i = 0; i < sz; ++i)
        {
            PyObject* item = PySequence_Fast_GET_ITEM(fastSeq.get(), i);
            if(!item)
            {
                break;
            }
            out << nl << '[' << static_cast<int>(i) << "] = ";
            elementType->print(item, out, history);
        }
        out.eb();
    }
}

void
IcePy::SequenceInfo::destroy()
{
    if(elementType)
    {
        elementType->destroy();
        elementType = 0;
    }
}

void
IcePy::SequenceInfo::marshalPrimitiveSequence(const PrimitiveInfoPtr& pi, PyObject* p, const Ice::OutputStreamPtr& os)
{
    //
    // For most types, we accept an object that implements the buffer protocol
    // (this includes the array.array type).
    //
    const void* buf = 0;
    Py_ssize_t sz;
    if(PyObject_AsReadBuffer(p, &buf, &sz) == 0)
    {
        const Ice::Byte* b = reinterpret_cast<const Ice::Byte*>(buf);
        switch(pi->kind)
        {
        case PrimitiveInfo::KindBool:
        {
            os->writeBoolSeq(reinterpret_cast<const bool*>(b), reinterpret_cast<const bool*>(b + sz));
            break;
        }
        case PrimitiveInfo::KindByte:
        {
            os->writeByteSeq(reinterpret_cast<const Ice::Byte*>(b), reinterpret_cast<const Ice::Byte*>(b + sz));
            break;
        }
        case PrimitiveInfo::KindShort:
        {
            os->writeShortSeq(reinterpret_cast<const Ice::Short*>(b), reinterpret_cast<const Ice::Short*>(b + sz));
            break;
        }
        case PrimitiveInfo::KindInt:
        {
            os->writeIntSeq(reinterpret_cast<const Ice::Int*>(b), reinterpret_cast<const Ice::Int*>(b + sz));
            break;
        }
        case PrimitiveInfo::KindLong:
        {
            PyErr_Format(PyExc_ValueError, STRCAST("expected sequence value"));
            throw AbortMarshaling();
        }
        case PrimitiveInfo::KindFloat:
        {
            os->writeFloatSeq(reinterpret_cast<const Ice::Float*>(b), reinterpret_cast<const Ice::Float*>(b + sz));
            break;
        }
        case PrimitiveInfo::KindDouble:
        {
            os->writeDoubleSeq(reinterpret_cast<const Ice::Double*>(b),
                               reinterpret_cast<const Ice::Double*>(b + sz));
            break;
        }
        case PrimitiveInfo::KindString:
        {
            PyErr_Format(PyExc_ValueError, STRCAST("expected sequence value"));
            throw AbortMarshaling();
        }
        }
        return;
    }

    PyErr_Clear(); // PyObject_AsReadBuffer sets an exception on failure.

    PyObjectHandle fs;
    if(!buf)
    {
        if(pi->kind == PrimitiveInfo::KindByte)
        {
            //
            // Accept a string or a sequence for sequence<byte>.
            //
            if(!PyString_Check(p))
            {
                fs = PySequence_Fast(p, STRCAST("expected a string, sequence, or buffer value"));
                if(!fs.get())
                {
                    return;
                }
            }
        }
        else
        {
            fs = PySequence_Fast(p, STRCAST("expected a sequence or buffer value"));
            if(!fs.get())
            {
                return;
            }
        }
    }

    switch(pi->kind)
    {
    case PrimitiveInfo::KindBool:
    {
        sz = PySequence_Fast_GET_SIZE(fs.get());
        Ice::BoolSeq seq(sz);
        for(Py_ssize_t i = 0; i < sz; ++i)
        {
            PyObject* item = PySequence_Fast_GET_ITEM(fs.get(), i);
            if(!item)
            {
                throw AbortMarshaling();
            }
            int isTrue = PyObject_IsTrue(item);
            if(isTrue < 0)
            {
                PyErr_Format(PyExc_ValueError, STRCAST("invalid value for element %d of sequence<bool>"),
                             static_cast<int>(i));
                throw AbortMarshaling();
            }
            seq[i] = isTrue ? true : false;
        }
        os->writeBoolSeq(seq);
        break;
    }
    case PrimitiveInfo::KindByte:
    {
        if(!fs.get())
        {
            assert(PyString_Check(p));
            const char* str = PyString_AS_STRING(p);
            sz = PyString_GET_SIZE(p);
            os->writeByteSeq(reinterpret_cast<const Ice::Byte*>(str), reinterpret_cast<const Ice::Byte*>(str + sz));
        }
        else
        {
            sz = PySequence_Fast_GET_SIZE(fs.get());
            Ice::ByteSeq seq(sz);
            for(Py_ssize_t i = 0; i < sz; ++i)
            {
                PyObject* item = PySequence_Fast_GET_ITEM(fs.get(), i);
                if(!item)
                {
                    throw AbortMarshaling();
                }

                long val = -1;
                if(PyInt_Check(item))
                {
                    val = PyInt_AS_LONG(item);
                }
                else if(PyLong_Check(item))
                {
                    val = PyLong_AsLong(item);
                }

                if(PyErr_Occurred() || val < 0 || val > 255)
                {
                    PyErr_Format(PyExc_ValueError, STRCAST("invalid value for element %d of sequence<byte>"),
                                 static_cast<int>(i));
                    throw AbortMarshaling();
                }
                seq[i] = static_cast<Ice::Byte>(val);
            }
            os->writeByteSeq(seq);
        }
        break;
    }
    case PrimitiveInfo::KindShort:
    {
        sz = PySequence_Fast_GET_SIZE(fs.get());
        Ice::ShortSeq seq(sz);
        for(Py_ssize_t i = 0; i < sz; ++i)
        {
            PyObject* item = PySequence_Fast_GET_ITEM(fs.get(), i);
            if(!item)
            {
                throw AbortMarshaling();
            }

            long val = SHRT_MIN - 1;
            if(PyInt_Check(item))
            {
                val = PyInt_AS_LONG(item);
            }
            else if(PyLong_Check(item))
            {
                val = PyLong_AsLong(item);
            }

            if(PyErr_Occurred() || val < SHRT_MIN || val > SHRT_MAX)
            {
                PyErr_Format(PyExc_ValueError, STRCAST("invalid value for element %d of sequence<short>"),
                             static_cast<int>(i));
                throw AbortMarshaling();
            }
            seq[i] = static_cast<Ice::Short>(val);
        }
        os->writeShortSeq(seq);
        break;
    }
    case PrimitiveInfo::KindInt:
    {
        sz = PySequence_Fast_GET_SIZE(fs.get());
        Ice::IntSeq seq(sz);
        for(Py_ssize_t i = 0; i < sz; ++i)
        {
            PyObject* item = PySequence_Fast_GET_ITEM(fs.get(), i);
            if(!item)
            {
                throw AbortMarshaling();
            }

            long val;
            if(PyInt_Check(item))
            {
                val = PyInt_AS_LONG(item);
            }
            else if(PyLong_Check(item))
            {
                val = PyLong_AsLong(item);
            }
            else
            {
                PyErr_Format(PyExc_ValueError, STRCAST("invalid value for element %d of sequence<int>"),
                             static_cast<int>(i));
                throw AbortMarshaling();
            }

            if(PyErr_Occurred() || val < INT_MIN || val > INT_MAX)
            {
                PyErr_Format(PyExc_ValueError, STRCAST("invalid value for element %d of sequence<int>"),
                             static_cast<int>(i));
                throw AbortMarshaling();
            }
            seq[i] = static_cast<Ice::Int>(val);
        }
        os->writeIntSeq(seq);
        break;
    }
    case PrimitiveInfo::KindLong:
    {
        sz = PySequence_Fast_GET_SIZE(fs.get());
        Ice::LongSeq seq(sz);
        for(Py_ssize_t i = 0; i < sz; ++i)
        {
            PyObject* item = PySequence_Fast_GET_ITEM(fs.get(), i);
            if(!item)
            {
                throw AbortMarshaling();
            }

            Ice::Long val;
            if(PyInt_Check(item))
            {
                val = PyInt_AS_LONG(item);
            }
            else if(PyLong_Check(item))
            {
                val = PyLong_AsLongLong(item);
            }
            else
            {
                PyErr_Format(PyExc_ValueError, STRCAST("invalid value for element %d of sequence<long>"),
                             static_cast<int>(i));
                throw AbortMarshaling();
            }

            if(PyErr_Occurred())
            {
                PyErr_Format(PyExc_ValueError, STRCAST("invalid value for element %d of sequence<long>"),
                             static_cast<int>(i));
                throw AbortMarshaling();
            }
            seq[i] = val;
        }
        os->writeLongSeq(seq);
        break;
    }
    case PrimitiveInfo::KindFloat:
    {
        sz = PySequence_Fast_GET_SIZE(fs.get());
        Ice::FloatSeq seq(sz);
        for(Py_ssize_t i = 0; i < sz; ++i)
        {
            PyObject* item = PySequence_Fast_GET_ITEM(fs.get(), i);
            if(!item)
            {
                throw AbortMarshaling();
            }

            float val;
            if(PyFloat_Check(item))
            {
                val = static_cast<float>(PyFloat_AS_DOUBLE(item));
            }
            else
            {
                PyErr_Format(PyExc_ValueError, STRCAST("invalid value for element %d of sequence<float>"),
                             static_cast<int>(i));
                throw AbortMarshaling();
            }

            seq[i] = val;
        }
        os->writeFloatSeq(seq);
        break;
    }
    case PrimitiveInfo::KindDouble:
    {
        sz = PySequence_Fast_GET_SIZE(fs.get());
        Ice::DoubleSeq seq(sz);
        for(Py_ssize_t i = 0; i < sz; ++i)
        {
            PyObject* item = PySequence_Fast_GET_ITEM(fs.get(), i);
            if(!item)
            {
                throw AbortMarshaling();
            }

            double val;
            if(PyFloat_Check(item))
            {
                val = PyFloat_AS_DOUBLE(item);
            }
            else
            {
                PyErr_Format(PyExc_ValueError, STRCAST("invalid value for element %d of sequence<double>"),
                             static_cast<int>(i));
                throw AbortMarshaling();
            }

            seq[i] = val;
        }
        os->writeDoubleSeq(seq);
        break;
    }
    case PrimitiveInfo::KindString:
    {
        sz = PySequence_Fast_GET_SIZE(fs.get());
        Ice::StringSeq seq(sz);
        for(Py_ssize_t i = 0; i < sz; ++i)
        {
            PyObject* item = PySequence_Fast_GET_ITEM(fs.get(), i);
            if(!item)
            {
                throw AbortMarshaling();
            }

            string val;
            if(PyString_Check(item))
            {
                val = string(PyString_AS_STRING(item), PyString_GET_SIZE(item));
            }
            else if(p != Py_None)
            {
                PyErr_Format(PyExc_ValueError, STRCAST("invalid value for element %d of sequence<string>"),
                             static_cast<int>(i));
                throw AbortMarshaling();
            }

            seq[i] = val;
        }
        os->writeStringSeq(seq);
        break;
    }
    }
}

void
IcePy::SequenceInfo::unmarshalPrimitiveSequence(const PrimitiveInfoPtr& pi, const Ice::InputStreamPtr& is,
                                                const UnmarshalCallbackPtr& cb, PyObject* target, void* closure,
                                                const SequenceMappingPtr& sm)
{
    PyObjectHandle result;

    switch(pi->kind)
    {
    case PrimitiveInfo::KindBool:
    {
        pair<const bool*, const bool*> p;
        IceUtil::ScopedArray<bool> arr(is->readBoolSeq(p));
        int sz = static_cast<int>(p.second - p.first);
        result = sm->createContainer(sz);
        if(!result.get())
        {
            throw AbortMarshaling();
        }

        for(int i = 0; i < sz; ++i)
        {
            sm->setItem(result.get(), i, p.first[i] ? getTrue() : getFalse());
        }
        break;
    }
    case PrimitiveInfo::KindByte:
    {
        pair<const Ice::Byte*, const Ice::Byte*> p;
        is->readByteSeq(p);
        int sz = static_cast<int>(p.second - p.first);
        if(sm->type == SequenceMapping::SEQ_DEFAULT)
        {
            result = PyString_FromStringAndSize(reinterpret_cast<const char*>(p.first), sz);
            if(!result.get())
            {
                throw AbortMarshaling();
            }
        }
        else
        {
            result = sm->createContainer(sz);
            if(!result.get())
            {
                throw AbortMarshaling();
            }

            for(int i = 0; i < sz; ++i)
            {
                PyObjectHandle item = PyInt_FromLong(p.first[i]);
                if(!item.get())
                {
                    throw AbortMarshaling();
                }
                sm->setItem(result.get(), i, item.get());
            }
        }
        break;
    }
    case PrimitiveInfo::KindShort:
    {
        pair<const Ice::Short*, const Ice::Short*> p;
        IceUtil::ScopedArray<Ice::Short> arr(is->readShortSeq(p));
        int sz = static_cast<int>(p.second - p.first);
        result = sm->createContainer(sz);
        if(!result.get())
        {
            throw AbortMarshaling();
        }

        for(int i = 0; i < sz; ++i)
        {
            PyObjectHandle item = PyInt_FromLong(p.first[i]);
            if(!item.get())
            {
                throw AbortMarshaling();
            }
            sm->setItem(result.get(), i, item.get());
        }
        break;
    }
    case PrimitiveInfo::KindInt:
    {
        pair<const Ice::Int*, const Ice::Int*> p;
        IceUtil::ScopedArray<Ice::Int> arr(is->readIntSeq(p));
        int sz = static_cast<int>(p.second - p.first);
        result = sm->createContainer(sz);
        if(!result.get())
        {
            throw AbortMarshaling();
        }

        for(int i = 0; i < sz; ++i)
        {
            PyObjectHandle item = PyInt_FromLong(p.first[i]);
            if(!item.get())
            {
                throw AbortMarshaling();
            }
            sm->setItem(result.get(), i, item.get());
        }
        break;
    }
    case PrimitiveInfo::KindLong:
    {
        pair<const Ice::Long*, const Ice::Long*> p;
        IceUtil::ScopedArray<Ice::Long> arr(is->readLongSeq(p));
        int sz = static_cast<int>(p.second - p.first);
        result = sm->createContainer(sz);
        if(!result.get())
        {
            throw AbortMarshaling();
        }

        for(int i = 0; i < sz; ++i)
        {
            PyObjectHandle item = PyLong_FromLongLong(p.first[i]);
            if(!item.get())
            {
                throw AbortMarshaling();
            }
            sm->setItem(result.get(), i, item.get());
        }
        break;
    }
    case PrimitiveInfo::KindFloat:
    {
        pair<const Ice::Float*, const Ice::Float*> p;
        IceUtil::ScopedArray<Ice::Float> arr(is->readFloatSeq(p));
        int sz = static_cast<int>(p.second - p.first);
        result = sm->createContainer(sz);
        if(!result.get())
        {
            throw AbortMarshaling();
        }

        for(int i = 0; i < sz; ++i)
        {
            PyObjectHandle item = PyFloat_FromDouble(p.first[i]);
            if(!item.get())
            {
                throw AbortMarshaling();
            }
            sm->setItem(result.get(), i, item.get());
        }
        break;
    }
    case PrimitiveInfo::KindDouble:
    {
        pair<const Ice::Double*, const Ice::Double*> p;
        IceUtil::ScopedArray<Ice::Double> arr(is->readDoubleSeq(p));
        int sz = static_cast<int>(p.second - p.first);
        result = sm->createContainer(sz);
        if(!result.get())
        {
            throw AbortMarshaling();
        }

        for(int i = 0; i < sz; ++i)
        {
            PyObjectHandle item = PyFloat_FromDouble(p.first[i]);
            if(!item.get())
            {
                throw AbortMarshaling();
            }
            sm->setItem(result.get(), i, item.get());
        }
        break;
    }
    case PrimitiveInfo::KindString:
    {
        Ice::StringSeq seq = is->readStringSeq();
        int sz = static_cast<int>(seq.size());
        result = sm->createContainer(sz);
        if(!result.get())
        {
            throw AbortMarshaling();
        }

        for(int i = 0; i < sz; ++i)
        {
            PyObjectHandle item = PyString_FromString(seq[i].c_str());
            if(!item.get())
            {
                throw AbortMarshaling();
            }
            sm->setItem(result.get(), i, item.get());
        }
        break;
    }
    }
    cb->unmarshaled(result.get(), target, closure);
}

bool
IcePy::SequenceInfo::SequenceMapping::getType(const Ice::StringSeq& metaData, Type& t)
{
    if(!metaData.empty())
    {
        for(Ice::StringSeq::const_iterator p = metaData.begin(); p != metaData.end(); ++p)
        {
            if((*p) == "python:seq:default")
            {
                t = SEQ_DEFAULT;
                return true;
            }
            else if((*p) == "python:seq:tuple")
            {
                t = SEQ_TUPLE;
                return true;
            }
            else if((*p) == "python:seq:list")
            {
                t = SEQ_LIST;
                return true;
            }
        }
    }

    return false;
}

IcePy::SequenceInfo::SequenceMapping::SequenceMapping(Type t) :
    type(t)
{
}

IcePy::SequenceInfo::SequenceMapping::SequenceMapping(const Ice::StringSeq& meta)
{
    if(!getType(meta, type))
    {
        type = SEQ_DEFAULT;
    }
}

void
IcePy::SequenceInfo::SequenceMapping::unmarshaled(PyObject* val, PyObject* target, void* closure)
{
    long i = reinterpret_cast<long>(closure);
    if(type == SEQ_DEFAULT || type == SEQ_LIST)
    {
        PyList_SET_ITEM(target, i, val);
        Py_INCREF(val); // PyList_SET_ITEM steals a reference.
    }
    else
    {
        assert(type == SEQ_TUPLE);
        PyTuple_SET_ITEM(target, i, val);
        Py_INCREF(val); // PyTuple_SET_ITEM steals a reference.
    }
}

PyObject*
IcePy::SequenceInfo::SequenceMapping::createContainer(int sz) const
{
    if(type == SEQ_DEFAULT || type == SEQ_LIST)
    {
        return PyList_New(sz);
    }
    else
    {
        assert(type == SEQ_TUPLE);
        return PyTuple_New(sz);
    }
}

void
IcePy::SequenceInfo::SequenceMapping::setItem(PyObject* cont, int i, PyObject* val) const
{
    if(type == SEQ_DEFAULT || type == SEQ_LIST)
    {
        Py_INCREF(val);
        PyList_SET_ITEM(cont, i, val); // PyList_SET_ITEM steals a reference.
    }
    else
    {
        assert(type == SEQ_TUPLE);
        Py_INCREF(val);
        PyTuple_SET_ITEM(cont, i, val); // PyTuple_SET_ITEM steals a reference.
    }
}

//
// DictionaryInfo implementation.
//
string
IcePy::DictionaryInfo::getId() const
{
    return id;
}

bool
IcePy::DictionaryInfo::validate(PyObject* val)
{
    return val == Py_None || PyDict_Check(val) == 1;
}

bool
IcePy::DictionaryInfo::usesClasses()
{
    return valueType->usesClasses();
}

void
IcePy::DictionaryInfo::marshal(PyObject* p, const Ice::OutputStreamPtr& os, ObjectMap* objectMap,
                               const Ice::StringSeq*)
{
    if(p == Py_None)
    {
        os->writeSize(0);
        return;
    }
    else if(!PyDict_Check(p))
    {
        PyErr_Format(PyExc_ValueError, STRCAST("expected dictionary value"));
        throw AbortMarshaling();
    }

    Py_ssize_t sz = PyDict_Size(p);
    os->writeSize(static_cast<int>(sz));

    Py_ssize_t pos = 0;
    PyObject* key;
    PyObject* value;
    while(PyDict_Next(p, &pos, &key, &value))
    {
        if(!keyType->validate(key))
        {
            PyErr_Format(PyExc_ValueError, STRCAST("invalid key in `%s' element"), const_cast<char*>(id.c_str()));
            throw AbortMarshaling();
        }
        keyType->marshal(key, os, objectMap);

        if(!valueType->validate(value))
        {
            PyErr_Format(PyExc_ValueError, STRCAST("invalid value in `%s' element"), const_cast<char*>(id.c_str()));
            throw AbortMarshaling();
        }
        valueType->marshal(value, os, objectMap);
    }
}

void
IcePy::DictionaryInfo::unmarshal(const Ice::InputStreamPtr& is, const UnmarshalCallbackPtr& cb, PyObject* target,
                                 void* closure, const Ice::StringSeq*)
{
    PyObjectHandle p = PyDict_New();
    if(!p.get())
    {
        throw AbortMarshaling();
    }

    KeyCallbackPtr keyCB = new KeyCallback;
    keyCB->key = 0;

    Ice::Int sz = is->readSize();
    for(Ice::Int i = 0; i < sz; ++i)
    {
        //
        // A dictionary key cannot be a class (or contain one), so the key must be
        // available immediately.
        //
        keyType->unmarshal(is, keyCB, 0, 0);
        assert(keyCB->key.get());

        //
        // Insert the key into the dictionary with a dummy value in order to hold
        // a reference to the key. In case of an exception, we don't want to leak
        // the key.
        //
        if(PyDict_SetItem(p.get(), keyCB->key.get(), Py_None) < 0)
        {
            throw AbortMarshaling();
        }

        //
        // The callback will reset the dictionary entry with the unmarshaled value,
        // so we pass it the key.
        //
        void* cl = reinterpret_cast<void*>(keyCB->key.get());
        valueType->unmarshal(is, this, p.get(), cl);
    }

    cb->unmarshaled(p.get(), target, closure);
}

void
IcePy::DictionaryInfo::unmarshaled(PyObject* val, PyObject* target, void* closure)
{
    PyObject* key = reinterpret_cast<PyObject*>(closure);
    if(PyDict_SetItem(target, key, val) < 0)
    {
        throw AbortMarshaling();
    }
}

void
IcePy::DictionaryInfo::print(PyObject* value, IceUtil::Output& out, PrintObjectHistory* history)
{
    if(!validate(value))
    {
        out << "<invalid value - expected " << id << ">";
        return;
    }

    if(value == Py_None)
    {
        out << "{}";
    }
    else
    {
        Py_ssize_t pos = 0;
        PyObject* elemKey;
        PyObject* elemValue;
        out.sb();
        bool first = true;
        while(PyDict_Next(value, &pos, &elemKey, &elemValue))
        {
            if(first)
            {
                first = false;
            }
            else
            {
                out << nl;
            }
            out << nl << "key = ";
            keyType->print(elemKey, out, history);
            out << nl << "value = ";
            valueType->print(elemValue, out, history);
        }
        out.eb();
    }
}

void
IcePy::DictionaryInfo::KeyCallback::unmarshaled(PyObject* val, PyObject*, void*)
{
    key = val;
    Py_INCREF(val);
}

void
IcePy::DictionaryInfo::destroy()
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
IcePy::ClassInfo::getId() const
{
    return id;
}

bool
IcePy::ClassInfo::validate(PyObject* val)
{
    return val == Py_None || PyObject_IsInstance(val, pythonType.get()) == 1;
}

bool
IcePy::ClassInfo::usesClasses()
{
    return true;
}

void
IcePy::ClassInfo::marshal(PyObject* p, const Ice::OutputStreamPtr& os, ObjectMap* objectMap,
                          const Ice::StringSeq*)
{
    if(!pythonType.get())
    {
        PyErr_Format(PyExc_RuntimeError, STRCAST("class %s is declared but not defined"), id.c_str());
        throw AbortMarshaling();
    }

    if(p == Py_None)
    {
        os->writeObject(0);
        return;
    }

    if(!PyObject_IsInstance(p, pythonType.get()))
    {
        PyErr_Format(PyExc_ValueError, STRCAST("expected value of type %s"), id.c_str());
        throw AbortMarshaling();
    }

    //
    // Ice::ObjectWriter is a subclass of Ice::Object that wraps a Python object for marshaling.
    // It is possible that this Python object has already been marshaled, therefore we first must
    // check the object map to see if this object is present. If so, we use the existing ObjectWriter,
    // otherwise we create a new one.
    //
    Ice::ObjectPtr writer;
    assert(objectMap);
    ObjectMap::iterator q = objectMap->find(p);
    if(q == objectMap->end())
    {
        PyObjectHandle iceType = PyObject_GetAttrString(p, STRCAST("ice_type"));
        if(!iceType.get())
        {
            throw AbortMarshaling();
        }
        ClassInfoPtr info = ClassInfoPtr::dynamicCast(getType(iceType.get()));
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
IcePy::ClassInfo::unmarshal(const Ice::InputStreamPtr& is, const UnmarshalCallbackPtr& cb, PyObject* target,
                            void* closure, const Ice::StringSeq*)
{
    if(!pythonType.get())
    {
        PyErr_Format(PyExc_RuntimeError, STRCAST("class %s is declared but not defined"), id.c_str());
        throw AbortMarshaling();
    }

    is->readObject(new ReadObjectCallback(this, cb, target, closure));
}

void
IcePy::ClassInfo::print(PyObject* value, IceUtil::Output& out, PrintObjectHistory* history)
{
    if(!validate(value))
    {
        out << "<invalid value - expected " << id << ">";
        return;
    }

    if(value == Py_None)
    {
        out << "<nil>";
    }
    else
    {
        map<PyObject*, int>::iterator q = history->objects.find(value);
        if(q != history->objects.end())
        {
            out << "<object #" << q->second << ">";
        }
        else
        {
            PyObjectHandle iceType = PyObject_GetAttrString(value, STRCAST("ice_type"));
            assert(iceType.get());
            ClassInfoPtr info = ClassInfoPtr::dynamicCast(getType(iceType.get()));
            assert(info);
            out << "object #" << history->index << " (" << info->id << ')';
            history->objects.insert(map<PyObject*, int>::value_type(value, history->index));
            ++history->index;
            out.sb();
            info->printMembers(value, out, history);
            out.eb();
        }
    }
}

void
IcePy::ClassInfo::destroy()
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
    typeObj = 0; // Break circular reference.
}

void
IcePy::ClassInfo::printMembers(PyObject* value, IceUtil::Output& out, PrintObjectHistory* history)
{
    if(base)
    {
        base->printMembers(value, out, history);
    }

    for(DataMemberList::iterator q = members.begin(); q != members.end(); ++q)
    {
        DataMemberPtr member = *q;
        char* memberName = const_cast<char*>(member->name.c_str());
        PyObjectHandle attr = PyObject_GetAttrString(value, memberName);
        out << nl << member->name << " = ";
        if(!attr.get())
        {
            out << "<not defined>";
        }
        else
        {
            member->type->print(attr.get(), out, history);
        }
    }
}

//
// ProxyInfo implementation.
//
string
IcePy::ProxyInfo::getId() const
{
    return id;
}

bool
IcePy::ProxyInfo::validate(PyObject* val)
{
    return val == Py_None || PyObject_IsInstance(val, pythonType.get()) == 1;
}

void
IcePy::ProxyInfo::marshal(PyObject* p, const Ice::OutputStreamPtr& os, ObjectMap*, const Ice::StringSeq*)
{
    if(p == Py_None)
    {
        os->writeProxy(0);
    }
    else if(checkProxy(p))
    {
        os->writeProxy(getProxy(p));
    }
    else
    {
        assert(false); // validate() should have caught this.
    }
}

void
IcePy::ProxyInfo::unmarshal(const Ice::InputStreamPtr& is, const UnmarshalCallbackPtr& cb, PyObject* target,
                            void* closure, const Ice::StringSeq*)
{
    Ice::ObjectPrx proxy = is->readProxy();

    if(!proxy)
    {
        cb->unmarshaled(Py_None, target, closure);
        return;
    }

    if(!pythonType.get())
    {
        PyErr_Format(PyExc_RuntimeError, STRCAST("class %s is declared but not defined"), id.c_str());
        return;
    }

    PyObjectHandle p = createProxy(proxy, is->communicator(), pythonType.get());
    cb->unmarshaled(p.get(), target, closure);
}

void
IcePy::ProxyInfo::print(PyObject* value, IceUtil::Output& out, PrintObjectHistory*)
{
    if(!validate(value))
    {
        out << "<invalid value - expected " << getId() << ">";
        return;
    }

    if(value == Py_None)
    {
        out << "<nil>";
    }
    else
    {
        PyObjectHandle p = PyObject_Str(value);
        if(!p.get())
        {
            return;
        }
        assert(PyString_Check(p.get()));
        out << PyString_AS_STRING(p.get());
    }
}

void
IcePy::ProxyInfo::destroy()
{
    typeObj = 0; // Break circular reference.
}

//
// ObjectWriter implementation.
//
IcePy::ObjectWriter::ObjectWriter(const ClassInfoPtr& info, PyObject* object, ObjectMap* objectMap) :
    _info(info), _object(object), _map(objectMap)
{
    Py_INCREF(_object);
}

IcePy::ObjectWriter::~ObjectWriter()
{
    Py_DECREF(_object);
}

void
IcePy::ObjectWriter::ice_preMarshal()
{
    if(PyObject_HasAttrString(_object, STRCAST("ice_preMarshal")) == 1)
    {
        PyObjectHandle tmp = PyObject_CallMethod(_object, STRCAST("ice_preMarshal"), 0);
        if(PyErr_Occurred())
        {
            throw AbortMarshaling();
        }
    }
}

void
IcePy::ObjectWriter::write(const Ice::OutputStreamPtr& os) const
{
    ClassInfoPtr info = _info;
    while(info)
    {
        os->writeTypeId(info->id);

        os->startSlice();
        for(DataMemberList::iterator q = info->members.begin(); q != info->members.end(); ++q)
        {
            DataMemberPtr member = *q;
            char* memberName = const_cast<char*>(member->name.c_str());

            PyObjectHandle val = PyObject_GetAttrString(_object, memberName);
            if(!val.get())
            {
                PyErr_Clear();
                PyErr_Format(PyExc_AttributeError, STRCAST("no member `%s' found in %s value"), memberName,
                             const_cast<char*>(_info->id.c_str()));
                throw AbortMarshaling();
            }

            if(!member->type->validate(val.get()))
            {
                PyErr_Format(PyExc_ValueError, STRCAST("invalid value for %s member `%s'"),
                             const_cast<char*>(_info->id.c_str()), memberName);
                throw AbortMarshaling();
            }

            member->type->marshal(val.get(), os, _map, &member->metaData);
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
IcePy::ObjectReader::ObjectReader(PyObject* object, const ClassInfoPtr& info) :
    _object(object), _info(info)
{
    Py_INCREF(_object);
}

IcePy::ObjectReader::~ObjectReader()
{
    Py_DECREF(_object);
}

void
IcePy::ObjectReader::ice_postUnmarshal()
{
    if(PyObject_HasAttrString(_object, STRCAST("ice_postUnmarshal")) == 1)
    {
        PyObjectHandle tmp = PyObject_CallMethod(_object, STRCAST("ice_postUnmarshal"), 0);
        if(PyErr_Occurred())
        {
            throw AbortMarshaling();
        }
    }
}

void
IcePy::ObjectReader::read(const Ice::InputStreamPtr& is, bool rid)
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
                member->type->unmarshal(is, member, _object, 0, &member->metaData);
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
IcePy::ObjectReader::getInfo() const
{
    return _info;
}

PyObject*
IcePy::ObjectReader::getObject() const
{
    return _object;
}

//
// InfoMapDestroyer implementation.
//
IcePy::InfoMapDestroyer::~InfoMapDestroyer()
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
IcePy::ReadObjectCallback::ReadObjectCallback(const ClassInfoPtr& info, const UnmarshalCallbackPtr& cb,
                                              PyObject* target, void* closure) :
    _info(info), _cb(cb), _target(target), _closure(closure)
{
    Py_XINCREF(_target);
}

IcePy::ReadObjectCallback::~ReadObjectCallback()
{
    Py_XDECREF(_target);
}

void
IcePy::ReadObjectCallback::invoke(const Ice::ObjectPtr& p)
{
    if(p)
    {
        ObjectReaderPtr reader = ObjectReaderPtr::dynamicCast(p);
        assert(reader);

        //
        // Verify that the unmarshaled object is compatible with the formal type.
        //
        PyObject* obj = reader->getObject();
        if(!PyObject_IsInstance(obj, _info->pythonType.get()))
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
        _cb->unmarshaled(Py_None, _target, _closure);
    }
}

//
// ExceptionInfo implementation.
//
void
IcePy::ExceptionInfo::marshal(PyObject* p, const Ice::OutputStreamPtr& os, ObjectMap* objectMap)
{
    if(!PyObject_IsInstance(p, pythonType.get()))
    {
        PyErr_Format(PyExc_ValueError, STRCAST("expected exception %s"), id.c_str());
        throw AbortMarshaling();
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
            char* memberName = const_cast<char*>(member->name.c_str());

            PyObjectHandle val = PyObject_GetAttrString(p, memberName);
            if(!val.get())
            {
                PyErr_Clear();
                PyErr_Format(PyExc_AttributeError, STRCAST("no member `%s' found in %s value"), memberName,
                             const_cast<char*>(id.c_str()));
                throw AbortMarshaling();
            }

            if(!member->type->validate(val.get()))
            {
                PyErr_Format(PyExc_ValueError, STRCAST("invalid value for %s member `%s'"),
                             const_cast<char*>(id.c_str()), memberName);
                throw AbortMarshaling();
            }

            member->type->marshal(val.get(), os, objectMap, &member->metaData);
        }
        os->endSlice();

        info = info->base;
    }
}

PyObject*
IcePy::ExceptionInfo::unmarshal(const Ice::InputStreamPtr& is)
{
    PyObjectHandle p = createExceptionInstance(pythonType.get());

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
            member->type->unmarshal(is, member, p.get(), 0, &member->metaData);
        }
        is->endSlice();

        info = info->base;
        if(info)
        {
            is->readString(); // Read the ID of the next slice.
        }
    }

    return p.release();
}

void
IcePy::ExceptionInfo::print(PyObject* value, IceUtil::Output& out)
{
    if(!PyObject_IsInstance(value, pythonType.get()))
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
IcePy::ExceptionInfo::printMembers(PyObject* value, IceUtil::Output& out, PrintObjectHistory* history)
{
    if(base)
    {
        base->printMembers(value, out, history);
    }

    for(DataMemberList::iterator q = members.begin(); q != members.end(); ++q)
    {
        DataMemberPtr member = *q;
        char* memberName = const_cast<char*>(member->name.c_str());
        PyObjectHandle attr = PyObject_GetAttrString(value, memberName);
        out << nl << member->name << " = ";
        if(!attr.get())
        {
            out << "<not defined>";
        }
        else
        {
            member->type->print(attr.get(), out, history);
        }
    }
}

//
// lookupClassInfo()
//
IcePy::ClassInfoPtr
IcePy::lookupClassInfo(const string& id)
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
IcePy::ExceptionInfoPtr
IcePy::lookupExceptionInfo(const string& id)
{
    ExceptionInfoMap::iterator p = _exceptionInfoMap.find(id);
    if(p != _exceptionInfoMap.end())
    {
        return p->second;
    }
    return 0;
}

namespace IcePy
{

PyTypeObject TypeInfoType =
{
    /* The ob_type field must be initialized in the module init function
     * to be portable to Windows without using C++. */
    PyObject_HEAD_INIT(0)
    0,                               /* ob_size */
    STRCAST("IcePy.TypeInfo"),       /* tp_name */
    sizeof(TypeInfoObject),          /* tp_basicsize */
    0,                               /* tp_itemsize */
    /* methods */
    (destructor)typeInfoDealloc,     /* tp_dealloc */
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
    0,                               /* tp_methods */
    0,                               /* tp_members */
    0,                               /* tp_getset */
    0,                               /* tp_base */
    0,                               /* tp_dict */
    0,                               /* tp_descr_get */
    0,                               /* tp_descr_set */
    0,                               /* tp_dictoffset */
    0,                               /* tp_init */
    0,                               /* tp_alloc */
    (newfunc)typeInfoNew,            /* tp_new */
    0,                               /* tp_free */
    0,                               /* tp_is_gc */
};

PyTypeObject ExceptionInfoType =
{
    /* The ob_type field must be initialized in the module init function
     * to be portable to Windows without using C++. */
    PyObject_HEAD_INIT(0)
    0,                               /* ob_size */
    STRCAST("IcePy.ExceptionInfo"),  /* tp_name */
    sizeof(ExceptionInfoObject),     /* tp_basicsize */
    0,                               /* tp_itemsize */
    /* methods */
    (destructor)exceptionInfoDealloc,/* tp_dealloc */
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
    0,                               /* tp_methods */
    0,                               /* tp_members */
    0,                               /* tp_getset */
    0,                               /* tp_base */
    0,                               /* tp_dict */
    0,                               /* tp_descr_get */
    0,                               /* tp_descr_set */
    0,                               /* tp_dictoffset */
    0,                               /* tp_init */
    0,                               /* tp_alloc */
    (newfunc)exceptionInfoNew,       /* tp_new */
    0,                               /* tp_free */
    0,                               /* tp_is_gc */
};

}

bool
IcePy::initTypes(PyObject* module)
{
    if(PyType_Ready(&TypeInfoType) < 0)
    {
        return false;
    }
    PyTypeObject* typeInfoType = &TypeInfoType; // Necessary to prevent GCC's strict-alias warnings.
    if(PyModule_AddObject(module, STRCAST("TypeInfo"), reinterpret_cast<PyObject*>(typeInfoType)) < 0)
    {
        return false;
    }

    if(PyType_Ready(&ExceptionInfoType) < 0)
    {
        return false;
    }
    PyTypeObject* exceptionInfoType = &ExceptionInfoType; // Necessary to prevent GCC's strict-alias warnings.
    if(PyModule_AddObject(module, STRCAST("ExceptionInfo"), reinterpret_cast<PyObject*>(exceptionInfoType)) < 0)
    {
        return false;
    }

    PrimitiveInfoPtr boolType = new PrimitiveInfo;
    boolType->kind = PrimitiveInfo::KindBool;
    PyObjectHandle boolTypeObj = createType(boolType);
    if(PyModule_AddObject(module, STRCAST("_t_bool"), boolTypeObj.get()) < 0)
    {
        return false;
    }
    boolTypeObj.release(); // PyModule_AddObject steals a reference.

    PrimitiveInfoPtr byteType = new PrimitiveInfo;
    byteType->kind = PrimitiveInfo::KindByte;
    PyObjectHandle byteTypeObj = createType(byteType);
    if(PyModule_AddObject(module, STRCAST("_t_byte"), byteTypeObj.get()) < 0)
    {
        return false;
    }
    byteTypeObj.release(); // PyModule_AddObject steals a reference.

    PrimitiveInfoPtr shortType = new PrimitiveInfo;
    shortType->kind = PrimitiveInfo::KindShort;
    PyObjectHandle shortTypeObj = createType(shortType);
    if(PyModule_AddObject(module, STRCAST("_t_short"), shortTypeObj.get()) < 0)
    {
        return false;
    }
    shortTypeObj.release(); // PyModule_AddObject steals a reference.

    PrimitiveInfoPtr intType = new PrimitiveInfo;
    intType->kind = PrimitiveInfo::KindInt;
    PyObjectHandle intTypeObj = createType(intType);
    if(PyModule_AddObject(module, STRCAST("_t_int"), intTypeObj.get()) < 0)
    {
        return false;
    }
    intTypeObj.release(); // PyModule_AddObject steals a reference.

    PrimitiveInfoPtr longType = new PrimitiveInfo;
    longType->kind = PrimitiveInfo::KindLong;
    PyObjectHandle longTypeObj = createType(longType);
    if(PyModule_AddObject(module, STRCAST("_t_long"), longTypeObj.get()) < 0)
    {
        return false;
    }
    longTypeObj.release(); // PyModule_AddObject steals a reference.

    PrimitiveInfoPtr floatType = new PrimitiveInfo;
    floatType->kind = PrimitiveInfo::KindFloat;
    PyObjectHandle floatTypeObj = createType(floatType);
    if(PyModule_AddObject(module, STRCAST("_t_float"), floatTypeObj.get()) < 0)
    {
        return false;
    }
    floatTypeObj.release(); // PyModule_AddObject steals a reference.

    PrimitiveInfoPtr doubleType = new PrimitiveInfo;
    doubleType->kind = PrimitiveInfo::KindDouble;
    PyObjectHandle doubleTypeObj = createType(doubleType);
    if(PyModule_AddObject(module, STRCAST("_t_double"), doubleTypeObj.get()) < 0)
    {
        return false;
    }
    doubleTypeObj.release(); // PyModule_AddObject steals a reference.

    PrimitiveInfoPtr stringType = new PrimitiveInfo;
    stringType->kind = PrimitiveInfo::KindString;
    PyObjectHandle stringTypeObj = createType(stringType);
    if(PyModule_AddObject(module, STRCAST("_t_string"), stringTypeObj.get()) < 0)
    {
        return false;
    }
    stringTypeObj.release(); // PyModule_AddObject steals a reference.

    return true;
}

IcePy::TypeInfoPtr
IcePy::getType(PyObject* obj)
{
    assert(PyObject_IsInstance(obj, reinterpret_cast<PyObject*>(&TypeInfoType)));
    TypeInfoObject* p = reinterpret_cast<TypeInfoObject*>(obj);
    return *p->info;
}

PyObject*
IcePy::createType(const TypeInfoPtr& info)
{
    TypeInfoObject* obj = typeInfoNew(0);
    if(obj)
    {
        obj->info = new IcePy::TypeInfoPtr(info);
    }
    return reinterpret_cast<PyObject*>(obj);
}

IcePy::ExceptionInfoPtr
IcePy::getException(PyObject* obj)
{
    assert(PyObject_IsInstance(obj, reinterpret_cast<PyObject*>(&ExceptionInfoType)));
    ExceptionInfoObject* p = reinterpret_cast<ExceptionInfoObject*>(obj);
    return *p->info;
}

PyObject*
IcePy::createException(const ExceptionInfoPtr& info)
{
    ExceptionInfoObject* obj = exceptionInfoNew(0);
    if(obj)
    {
        obj->info = new IcePy::ExceptionInfoPtr(info);
    }
    return reinterpret_cast<PyObject*>(obj);
}

extern "C"
PyObject*
IcePy_defineEnum(PyObject*, PyObject* args)
{
    char* id;
    PyObject* type;
    PyObject* meta;
    PyObject* enumerators;
    if(!PyArg_ParseTuple(args, STRCAST("sOOO"), &id, &type, &meta, &enumerators))
    {
        return 0;
    }

    assert(PyType_Check(type));
    assert(PyTuple_Check(meta));
    assert(PyTuple_Check(enumerators));

    EnumInfoPtr info = new EnumInfo;
    info->id = id;
    info->pythonType = type;
    Py_INCREF(type);

    Py_ssize_t sz = PyTuple_GET_SIZE(enumerators);
    for(Py_ssize_t i = 0; i < sz; ++i)
    {
        PyObjectHandle e = PyTuple_GET_ITEM(enumerators, i);
        Py_INCREF(e.get());
        assert(PyObject_IsInstance(e.get(), type));
        info->enumerators.push_back(e);
    }

    return createType(info);
}

static void
convertDataMembers(PyObject* members, DataMemberList& l)
{
    Py_ssize_t sz = PyTuple_GET_SIZE(members);
    for(Py_ssize_t i = 0; i < sz; ++i)
    {
        PyObject* m = PyTuple_GET_ITEM(members, i);
        assert(PyTuple_Check(m));
        assert(PyTuple_GET_SIZE(m) == 3);

        PyObject* name = PyTuple_GET_ITEM(m, 0); // Member name.
        assert(PyString_Check(name));
        PyObject* meta = PyTuple_GET_ITEM(m, 1); // Member metadata.
        assert(PyTuple_Check(meta));
        PyObject* t = PyTuple_GET_ITEM(m, 2); // Member type.

        DataMemberPtr member = new DataMember;
        member->name = string(PyString_AS_STRING(name), PyString_GET_SIZE(name));
#ifndef NDEBUG
        bool b =
#endif
        tupleToStringSeq(meta, member->metaData);
        assert(b);
        member->type = getType(t);
        l.push_back(member);
    }
}

extern "C"
PyObject*
IcePy_defineStruct(PyObject*, PyObject* args)
{
    char* id;
    PyObject* type;
    PyObject* meta;
    PyObject* members;
    if(!PyArg_ParseTuple(args, STRCAST("sOOO"), &id, &type, &meta, &members))
    {
        return 0;
    }

    assert(PyType_Check(type));
    assert(PyTuple_Check(meta));
    assert(PyTuple_Check(members));

    StructInfoPtr info = new StructInfo;
    info->id = id;
    info->pythonType = type;
    Py_INCREF(type);

    convertDataMembers(members, info->members);

    return createType(info);
}

extern "C"
PyObject*
IcePy_defineSequence(PyObject*, PyObject* args)
{
    char* id;
    PyObject* meta;
    PyObject* elementType;
    if(!PyArg_ParseTuple(args, STRCAST("sOO"), &id, &meta, &elementType))
    {
        return 0;
    }

    assert(PyTuple_Check(meta));

    vector<string> metaData;
#ifndef NDEBUG
    bool b =
#endif
    tupleToStringSeq(meta, metaData);
    assert(b);

    SequenceInfoPtr info = new SequenceInfo;
    info->id = id;
    info->mapping = new SequenceInfo::SequenceMapping(metaData);
    info->elementType = getType(elementType);

    return createType(info);
}

extern "C"
PyObject*
IcePy_defineDictionary(PyObject*, PyObject* args)
{
    char* id;
    PyObject* meta;
    PyObject* keyType;
    PyObject* valueType;
    if(!PyArg_ParseTuple(args, STRCAST("sOOO"), &id, &meta, &keyType, &valueType))
    {
        return 0;
    }

    assert(PyTuple_Check(meta));

    DictionaryInfoPtr info = new DictionaryInfo;
    info->id = id;
    info->keyType = getType(keyType);
    info->valueType = getType(valueType);

    return createType(info);
}

extern "C"
PyObject*
IcePy_declareProxy(PyObject*, PyObject* args)
{
    char* id;
    if(!PyArg_ParseTuple(args, STRCAST("s"), &id))
    {
        return 0;
    }

    string proxyId = id;
    proxyId += "Prx";

    ProxyInfoPtr info = lookupProxyInfo(proxyId);
    if(!info)
    {
        info = new ProxyInfo;
        info->id = proxyId;
        info->typeObj = createType(info);
        addProxyInfo(proxyId, info);
    }

    Py_INCREF(info->typeObj.get());
    return info->typeObj.get();
}

extern "C"
PyObject*
IcePy_defineProxy(PyObject*, PyObject* args)
{
    char* id;
    PyObject* type;
    if(!PyArg_ParseTuple(args, STRCAST("sO"), &id, &type))
    {
        return 0;
    }

    assert(PyType_Check(type));

    string proxyId = id;
    proxyId += "Prx";

    ProxyInfoPtr info = lookupProxyInfo(proxyId);
    if(!info)
    {
        info = new ProxyInfo;
        info->id = proxyId;
        info->typeObj = createType(info);
        addProxyInfo(proxyId, info);
    }

    info->pythonType = type;
    Py_INCREF(type);

    Py_INCREF(info->typeObj.get());
    return info->typeObj.get();
}

extern "C"
PyObject*
IcePy_declareClass(PyObject*, PyObject* args)
{
    char* id;
    if(!PyArg_ParseTuple(args, STRCAST("s"), &id))
    {
        return 0;
    }

    ClassInfoPtr info = lookupClassInfo(id);
    if(!info)
    {
        info = new ClassInfo;
        info->id = id;
        info->typeObj = createType(info);
        info->defined = false;
        addClassInfo(id, info);
    }

    Py_INCREF(info->typeObj.get());
    return info->typeObj.get();
}

extern "C"
PyObject*
IcePy_defineClass(PyObject*, PyObject* args)
{
    char* id;
    PyObject* type;
    PyObject* meta;
    int isAbstract;
    PyObject* base;
    PyObject* interfaces;
    PyObject* members;
    if(!PyArg_ParseTuple(args, STRCAST("sOOiOOO"), &id, &type, &meta, &isAbstract, &base, &interfaces, &members))
    {
        return 0;
    }

    assert(PyType_Check(type));
    assert(PyTuple_Check(meta));
    assert(PyTuple_Check(interfaces));
    assert(PyTuple_Check(members));

    //
    // A ClassInfo object will already exist for this id if a forward declaration
    // was encountered, or if the Slice definition is being reloaded. In the latter
    // case, we act as if it hasn't been defined yet.
    //
    ClassInfoPtr info = lookupClassInfo(id);
    if(!info || info->defined)
    {
        info = new ClassInfo;
        info->id = id;
        info->typeObj = createType(info);
        addClassInfo(id, info);
    }

    info->isAbstract = isAbstract ? true : false;

    if(base != Py_None)
    {
        info->base = ClassInfoPtr::dynamicCast(getType(base));
        assert(info->base);
    }

    Py_ssize_t i, sz;
    sz = PyTuple_GET_SIZE(interfaces);
    for(i = 0; i < sz; ++i)
    {
        PyObject* o = PyTuple_GET_ITEM(interfaces, i);
        ClassInfoPtr iface = ClassInfoPtr::dynamicCast(getType(o));
        assert(iface);
        info->interfaces.push_back(iface);
    }

    convertDataMembers(members, info->members);

    info->pythonType = type;
    Py_INCREF(type);

    info->defined = true;

    Py_INCREF(info->typeObj.get());
    return info->typeObj.get();
}

extern "C"
PyObject*
IcePy_defineException(PyObject*, PyObject* args)
{
    char* id;
    PyObject* type;
    PyObject* meta;
    PyObject* base;
    PyObject* members;
    if(!PyArg_ParseTuple(args, STRCAST("sOOOO"), &id, &type, &meta, &base, &members))
    {
        return 0;
    }

#ifdef ICEPY_OLD_EXCEPTIONS
    assert(PyClass_Check(type));
#else
    assert(PyExceptionClass_Check(type));
#endif
    assert(PyTuple_Check(meta));
    assert(PyTuple_Check(members));

    ExceptionInfoPtr info = new ExceptionInfo;
    info->id = id;

    if(base != Py_None)
    {
        info->base = ExceptionInfoPtr::dynamicCast(getException(base));
        assert(info->base);
    }

    info->usesClasses = false;

    convertDataMembers(members, info->members);

    for(DataMemberList::iterator p = info->members.begin(); p != info->members.end(); ++p)
    {
        if(!info->usesClasses)
        {
            info->usesClasses = (*p)->type->usesClasses();
        }
    }

    info->pythonType = type;
    Py_INCREF(type);

    addExceptionInfo(id, info);

    return createException(info);
}

extern "C"
PyObject*
IcePy_stringify(PyObject*, PyObject* args)
{
    PyObject* value;
    PyObject* type;
    if(!PyArg_ParseTuple(args, STRCAST("OO"), &value, &type))
    {
        return 0;
    }

    TypeInfoPtr info = getType(type);
    assert(info);

    ostringstream ostr;
    IceUtil::Output out(ostr);
    PrintObjectHistory history;
    history.index = 0;
    info->print(value, out, &history);

    string str = ostr.str();
    return PyString_FromString(str.c_str());
}

extern "C"
PyObject*
IcePy_stringifyException(PyObject*, PyObject* args)
{
    PyObject* value;
    if(!PyArg_ParseTuple(args, STRCAST("O"), &value))
    {
        return 0;
    }

    PyObjectHandle iceType = PyObject_GetAttrString(value, STRCAST("ice_type"));
    assert(iceType.get());
    ExceptionInfoPtr info = getException(iceType.get());
    assert(info);

    ostringstream ostr;
    IceUtil::Output out(ostr);
    info->print(value, out);

    string str = ostr.str();
    return PyString_FromString(str.c_str());
}
