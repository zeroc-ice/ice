// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Marshal.h>
#include <Proxy.h>
#include <Types.h>
#include <Util.h>
#include <IceUtil/InputUtil.h>
#include <Ice/LocalException.h>

using namespace std;
using namespace IcePy;

namespace IcePy
{

//
// Marshaler subclass definitions.
//
class PrimitiveMarshaler : public Marshaler
{
public:

    PrimitiveMarshaler(const PrimitiveInfoPtr&);

    virtual void marshal(PyObject*, const Ice::OutputStreamPtr&, ObjectMap*);
    virtual PyObject* unmarshal(const Ice::CommunicatorPtr&, const Ice::InputStreamPtr&);

    virtual void destroy();

private:

    PrimitiveInfoPtr _info;
};

class SequenceMarshaler : public Marshaler
{
public:

    SequenceMarshaler(const SequenceInfoPtr&);

    virtual void marshal(PyObject*, const Ice::OutputStreamPtr&, ObjectMap*);
    virtual PyObject* unmarshal(const Ice::CommunicatorPtr&, const Ice::InputStreamPtr&);

    virtual void destroy();

private:

    class SequenceReceiver : public ObjectReceiver
    {
    public:

        SequenceReceiver(const ClassInfoPtr&, PyObject*, int);

        virtual void setObject(PyObject*);

    private:

        PyObjectHandle _seq;
        int _index;
    };

    ClassInfoPtr _elementClassInfo; // Nil unless the element type is a class.
    MarshalerPtr _elementMarshaler;
};

class ProxyMarshaler : public Marshaler
{
public:

    ProxyMarshaler(const ProxyInfoPtr&);

    virtual void marshal(PyObject*, const Ice::OutputStreamPtr&, ObjectMap*);
    virtual PyObject* unmarshal(const Ice::CommunicatorPtr&, const Ice::InputStreamPtr&);

    virtual void destroy();

private:

    ProxyInfoPtr _info;
};

class ObjectMemberReceiver : public ObjectReceiver
{
public:

    ObjectMemberReceiver(const ClassInfoPtr&, PyObject*, const string&);

    virtual void setObject(PyObject*);

private:

    PyObjectHandle _target;
    string _member;
};

class StructMarshaler : public Marshaler
{
public:

    StructMarshaler(const StructInfoPtr&);

    virtual void marshal(PyObject*, const Ice::OutputStreamPtr&, ObjectMap*);
    virtual PyObject* unmarshal(const Ice::CommunicatorPtr&, const Ice::InputStreamPtr&);

    virtual void destroy();

private:

    StructInfoPtr _info;
    vector<MarshalerPtr> _members;
    vector<ClassInfoPtr> _memberClassInfo; // For use when unmarshaling class members.
};

class EnumMarshaler : public Marshaler
{
public:

    EnumMarshaler(const EnumInfoPtr&);

    virtual void marshal(PyObject*, const Ice::OutputStreamPtr&, ObjectMap*);
    virtual PyObject* unmarshal(const Ice::CommunicatorPtr&, const Ice::InputStreamPtr&);

    virtual void destroy();

private:

    EnumInfoPtr _info;
};

class NativeDictionaryMarshaler : public Marshaler
{
public:

    NativeDictionaryMarshaler(const DictionaryInfoPtr&);

    virtual void marshal(PyObject*, const Ice::OutputStreamPtr&, ObjectMap*);
    virtual PyObject* unmarshal(const Ice::CommunicatorPtr&, const Ice::InputStreamPtr&);

    virtual void destroy();

private:

    class ValueReceiver: public ObjectReceiver
    {
    public:

        ValueReceiver(const ClassInfoPtr&, PyObject*, PyObject*);

        virtual void setObject(PyObject*);

    private:

        PyObjectHandle _dict;
        PyObjectHandle _key;
    };

    DictionaryInfoPtr _info;
    MarshalerPtr _keyMarshaler;
    MarshalerPtr _valueMarshaler;
};

class ExceptionMarshaler : public Marshaler
{
public:

    ExceptionMarshaler(const ExceptionInfoPtr&);

    virtual void marshal(PyObject*, const Ice::OutputStreamPtr&, ObjectMap*);
    virtual PyObject* unmarshal(const Ice::CommunicatorPtr&, const Ice::InputStreamPtr&);

    virtual void destroy();

private:

    ExceptionInfoPtr _info;
};

}

//
// Marshaler implementation.
//
IcePy::Marshaler::Marshaler()
{
}

IcePy::Marshaler::~Marshaler()
{
}

IcePy::MarshalerPtr
IcePy::Marshaler::createMarshaler(const TypeInfoPtr& type)
{
    assert(type);

    PrimitiveInfoPtr p = PrimitiveInfoPtr::dynamicCast(type);
    if(p)
    {
        return new PrimitiveMarshaler(p);
    }

    SequenceInfoPtr seq = SequenceInfoPtr::dynamicCast(type);
    if(seq)
    {
        return new SequenceMarshaler(seq);
    }

    ProxyInfoPtr proxy = ProxyInfoPtr::dynamicCast(type);
    if(proxy)
    {
        return new ProxyMarshaler(proxy);
    }

    StructInfoPtr st = StructInfoPtr::dynamicCast(type);
    if(st)
    {
        return new StructMarshaler(st);
    }

    EnumInfoPtr en = EnumInfoPtr::dynamicCast(type);
    if(en)
    {
        return new EnumMarshaler(en);
    }

    DictionaryInfoPtr d = DictionaryInfoPtr::dynamicCast(type);
    if(d)
    {
        return new NativeDictionaryMarshaler(d);
    }

    ClassInfoPtr cl = ClassInfoPtr::dynamicCast(type);
    if(cl)
    {
        return new ObjectMarshaler(cl);
    }

    assert(false);
    return 0;
}

IcePy::MarshalerPtr
IcePy::Marshaler::createExceptionMarshaler(const ExceptionInfoPtr& info)
{
    return new ExceptionMarshaler(info);
}

//
// PrimitiveMarshaler implementation.
//
IcePy::PrimitiveMarshaler::PrimitiveMarshaler(const PrimitiveInfoPtr& info) :
    _info(info)
{
}

void
IcePy::PrimitiveMarshaler::marshal(PyObject* p, const Ice::OutputStreamPtr& os, ObjectMap*)
{
    switch(_info->kind)
    {
    case PrimitiveInfo::KindBool:
    {
        int isTrue = PyObject_IsTrue(p);
        if(isTrue < 0)
        {
            throw AbortMarshaling();
        }
        os->writeBool(isTrue ? true : false);
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
            PyErr_Format(PyExc_ValueError, "expected byte value");
            throw AbortMarshaling();
        }

        if(val < 0 || val > 255)
        {
            PyErr_Format(PyExc_ValueError, "value %ld is out of range for a byte", val);
            throw AbortMarshaling();
        }
        os->writeByte(static_cast<Ice::Byte>(val));
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
            PyErr_Format(PyExc_ValueError, "expected short value");
            throw AbortMarshaling();
        }

        if(val < SHRT_MIN || val > SHRT_MAX)
        {
            PyErr_Format(PyExc_ValueError, "value %ld is out of range for a short", val);
            throw AbortMarshaling();
        }
        os->writeShort(static_cast<Ice::Short>(val));
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
            PyErr_Format(PyExc_ValueError, "expected int value");
            throw AbortMarshaling();
        }

        if(val < INT_MIN || val > INT_MAX)
        {
            PyErr_Format(PyExc_ValueError, "value %ld is out of range for an int", val);
            throw AbortMarshaling();
        }
        os->writeInt(static_cast<Ice::Int>(val));
        break;
    }
    case PrimitiveInfo::KindLong:
    {
        //
        // The platform's 'long' type may not be 64 bits, so we also accept
        // a string argument for this type.
        //
        Ice::Long val;
        if(PyInt_Check(p))
        {
            val = PyInt_AS_LONG(p);
        }
        else if(PyLong_Check(p))
        {
            val = PyLong_AsLong(p);
        }
        else if(PyString_Check(p))
        {
            char* sval = PyString_AS_STRING(p);
            assert(sval);
            string::size_type pos;
            if(!IceUtil::stringToInt64(sval, val, pos))
            {
                PyErr_Format(PyExc_ValueError, "invalid long value `%s'", sval);
                throw AbortMarshaling();
            }
        }
        else
        {
            PyErr_Format(PyExc_ValueError, "expected long value");
            throw AbortMarshaling();
        }

        os->writeLong(val);
        break;
    }
    case PrimitiveInfo::KindFloat:
    {
        float val;
        if(PyFloat_Check(p))
        {
            val = static_cast<float>(PyFloat_AS_DOUBLE(p));
        }
        else
        {
            PyErr_Format(PyExc_ValueError, "expected float value");
            throw AbortMarshaling();
        }

        os->writeFloat(val);
        break;
    }
    case PrimitiveInfo::KindDouble:
    {
        double val;
        if(PyFloat_Check(p))
        {
            val = PyFloat_AS_DOUBLE(p);
        }
        else
        {
            PyErr_Format(PyExc_ValueError, "expected double value");
            throw AbortMarshaling();
        }

        os->writeDouble(val);
        break;
    }
    case PrimitiveInfo::KindString:
    {
        string val;
        if(PyString_Check(p))
        {
            val = (const char*)PyString_AS_STRING(p);
        }
        else if(p != Py_None)
        {
            PyErr_Format(PyExc_ValueError, "expected string value");
            throw AbortMarshaling();
        }

        os->writeString(val);
        break;
    }
    }
}

PyObject*
IcePy::PrimitiveMarshaler::unmarshal(const Ice::CommunicatorPtr&, const Ice::InputStreamPtr& is)
{
    switch(_info->kind)
    {
    case PrimitiveInfo::KindBool:
    {
        if(is->readBool())
        {
            Py_INCREF(Py_True);
            return Py_True;
        }
        else
        {
            Py_INCREF(Py_False);
            return Py_False;
        }
    }
    case PrimitiveInfo::KindByte:
    {
        Ice::Byte val = is->readByte();
        return PyInt_FromLong(val);
    }
    case PrimitiveInfo::KindShort:
    {
        Ice::Short val = is->readShort();
        return PyInt_FromLong(val);
    }
    case PrimitiveInfo::KindInt:
    {
        Ice::Int val = is->readInt();
        return PyInt_FromLong(val);
    }
    case PrimitiveInfo::KindLong:
    {
        Ice::Long val = is->readLong();

        //
        // The platform's 'long' type may not be 64 bits, so we store 64-bit values as a string.
        //
        if(sizeof(Ice::Long) > sizeof(long) && (val < LONG_MIN || val > LONG_MAX))
        {
            char buf[64];
#ifdef WIN32
            sprintf(buf, "%I64d", val);
#else
            sprintf(buf, "%lld", val);
#endif
            return PyString_FromString(buf);
        }
        else
        {
            return PyInt_FromLong(static_cast<long>(val));
        }
    }
    case PrimitiveInfo::KindFloat:
    {
        Ice::Float val = is->readFloat();
        return PyFloat_FromDouble(val);
    }
    case PrimitiveInfo::KindDouble:
    {
        Ice::Double val = is->readDouble();
        return PyFloat_FromDouble(val);
    }
    case PrimitiveInfo::KindString:
    {
        string val = is->readString();
        return PyString_FromString(val.c_str());
    }
    }

    assert(false);
    return NULL;
}

void
IcePy::PrimitiveMarshaler::destroy()
{
}

//
// SequenceMarshaler implementation.
//
IcePy::SequenceMarshaler::SequenceMarshaler(const SequenceInfoPtr& info)
{
    _elementClassInfo = ClassInfoPtr::dynamicCast(info);
    _elementMarshaler = createMarshaler(info->elementType);
}

void
IcePy::SequenceMarshaler::marshal(PyObject* p, const Ice::OutputStreamPtr& os, ObjectMap* objectMap)
{
    if(PyList_Check(p))
    {
        int sz = PyList_GET_SIZE(p);
        os->writeSize(sz);
        for(int i = 0; i < sz; ++i)
        {
            PyObject* item = PyList_GetItem(p, i);
            if(item == NULL)
            {
                throw AbortMarshaling();
            }
            _elementMarshaler->marshal(item, os, objectMap);
        }
    }
    else if(PyTuple_Check(p))
    {
        int sz = PyTuple_GET_SIZE(p);
        os->writeSize(sz);
        for(int i = 0; i < sz; ++i)
        {
            PyObject* item = PyTuple_GetItem(p, i);
            if(item == NULL)
            {
                throw AbortMarshaling();
            }
            _elementMarshaler->marshal(item, os, objectMap);
        }
    }
    else if(p == Py_None)
    {
        os->writeSize(0);
    }
    else
    {
        PyErr_Format(PyExc_ValueError, "expected sequence value");
        throw AbortMarshaling();
    }
}

PyObject*
IcePy::SequenceMarshaler::unmarshal(const Ice::CommunicatorPtr& communicator, const Ice::InputStreamPtr& is)
{
    Ice::Int sz = is->readSize();
    PyObjectHandle result = PyList_New(sz);
    if(result.get() == NULL)
    {
        return NULL;
    }

    // TODO: Optimize for certain sequence types (e.g., bytes)?

    if(_elementClassInfo)
    {
        ObjectMarshalerPtr om = ObjectMarshalerPtr::dynamicCast(_elementMarshaler);
        assert(om);
        for(Ice::Int i = 0; i < sz; ++i)
        {
            om->unmarshalObject(communicator, is, new SequenceReceiver(_elementClassInfo, result.get(), i));
        }
    }
    else
    {
        for(Ice::Int i = 0; i < sz; ++i)
        {
            PyObject* item = _elementMarshaler->unmarshal(communicator, is);
            if(item == NULL)
            {
                return NULL;
            }
            PyList_SET_ITEM(result.get(), i, item);
        }
    }

    return result.release();
}

IcePy::SequenceMarshaler::SequenceReceiver::SequenceReceiver(const ClassInfoPtr& info, PyObject* seq, int index) :
    ObjectReceiver(info), _seq(seq), _index(index)
{
    Py_INCREF(seq);
    assert(PyList_Check(seq));
    assert(_index < PyList_GET_SIZE(seq));
}

void
IcePy::SequenceMarshaler::SequenceReceiver::setObject(PyObject* p)
{
    Py_INCREF(p); // PyList_SET_ITEM steals a reference.
    PyList_SET_ITEM(_seq.get(), _index, p);
}

void
IcePy::SequenceMarshaler::destroy()
{
    _elementMarshaler->destroy();
    _elementMarshaler = 0;
}

//
// ProxyMarshaler implementation.
//
IcePy::ProxyMarshaler::ProxyMarshaler(const ProxyInfoPtr& info) :
    _info(info)
{
}

void
IcePy::ProxyMarshaler::marshal(PyObject* p, const Ice::OutputStreamPtr& os, ObjectMap*)
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
        PyErr_Format(PyExc_ValueError, "expected proxy value");
        throw AbortMarshaling();
    }
}

PyObject*
IcePy::ProxyMarshaler::unmarshal(const Ice::CommunicatorPtr& communicator, const Ice::InputStreamPtr& is)
{
    Ice::ObjectPrx proxy = is->readProxy();

    if(!proxy)
    {
        Py_INCREF(Py_None);
        return Py_None;
    }

    if(_info->pythonType.get() == NULL)
    {
        PyErr_Format(PyExc_RuntimeError, "class %s is declared but not defined", _info->name.c_str());
        return NULL;
    }

    return createProxy(proxy, communicator, _info->pythonType.get());
}

void
IcePy::ProxyMarshaler::destroy()
{
}

//
// ObjectMemberReceiver implementation.
//
IcePy::ObjectMemberReceiver::ObjectMemberReceiver(const ClassInfoPtr& info, PyObject* target, const string& member) :
    ObjectReceiver(info), _target(target), _member(member)
{
    Py_INCREF(target);
}

void
IcePy::ObjectMemberReceiver::setObject(PyObject* p)
{
    char* memberName = const_cast<char*>(_member.c_str());
    if(PyObject_SetAttrString(_target.get(), memberName, p) < 0)
    {
        throw AbortMarshaling();
    }
}

//
// StructMarshaler implementation.
//
IcePy::StructMarshaler::StructMarshaler(const StructInfoPtr& info) :
    _info(info)
{
    for(DataMemberList::iterator p = info->members.begin(); p != info->members.end(); ++p)
    {
        MarshalerPtr marshaler = createMarshaler(p->type);
        assert(marshaler);
        _members.push_back(marshaler);
    }
}

void
IcePy::StructMarshaler::marshal(PyObject* p, const Ice::OutputStreamPtr& os, ObjectMap* objectMap)
{
    int n = PyObject_IsInstance(p, _info->pythonType.get());
    if(n < 0)
    {
        throw AbortMarshaling();
    }
    else if(n == 0)
    {
        PyErr_Format(PyExc_ValueError, "expected %s value", _info->name.c_str());
        throw AbortMarshaling();
    }

    int i = 0;
    for(DataMemberList::iterator q = _info->members.begin(); q != _info->members.end(); ++q, ++i)
    {
        char* memberName = const_cast<char*>(q->name.c_str());
        PyObjectHandle attr = PyObject_GetAttrString(p, memberName);
        if(attr.get() == NULL)
        {
            throw AbortMarshaling();
        }

        _members[i]->marshal(attr.get(), os, objectMap);
    }
}

PyObject*
IcePy::StructMarshaler::unmarshal(const Ice::CommunicatorPtr& communicator, const Ice::InputStreamPtr& is)
{
    PyObjectHandle args = PyTuple_New(0);
    PyTypeObject* type = (PyTypeObject*)_info->pythonType.get();
    PyObjectHandle p = type->tp_new(type, args.get(), NULL);
    if(p.get() == NULL)
    {
        return NULL;
    }

    if(_memberClassInfo.empty()) // Lazy initialization.
    {
        for(DataMemberList::iterator q = _info->members.begin(); q != _info->members.end(); ++q)
        {
            _memberClassInfo.push_back(ClassInfoPtr::dynamicCast(q->type));
        }
    }

    int i = 0;
    for(DataMemberList::iterator q = _info->members.begin(); q != _info->members.end(); ++q, ++i)
    {
        if(_memberClassInfo[i])
        {
            ObjectMarshalerPtr om = ObjectMarshalerPtr::dynamicCast(_members[i]);
            assert(om);
            om->unmarshalObject(communicator, is, new ObjectMemberReceiver(_memberClassInfo[i], p.get(), q->name));
        }
        else
        {
            char* memberName = const_cast<char*>(q->name.c_str());
            PyObjectHandle attr = _members[i]->unmarshal(communicator, is);
            if(attr.get() == NULL)
            {
                return NULL;
            }

            if(PyObject_SetAttrString(p.get(), memberName, attr.get()) < 0)
            {
                return NULL;
            }
        }
    }

    return p.release();
}

void
IcePy::StructMarshaler::destroy()
{
    vector<MarshalerPtr> members = _members;
    _members.clear();
    _memberClassInfo.clear();

    for(vector<MarshalerPtr>::iterator p = members.begin(); p != members.end(); ++p)
    {
        (*p)->destroy();
    }
}

//
// EnumMarshaler implementation.
//
IcePy::EnumMarshaler::EnumMarshaler(const EnumInfoPtr& info) :
    _info(info)
{
}

void
IcePy::EnumMarshaler::marshal(PyObject* p, const Ice::OutputStreamPtr& os, ObjectMap*)
{
    int n = PyObject_IsInstance(p, _info->pythonType.get());
    if(n < 0)
    {
        throw AbortMarshaling();
    }
    else if(n == 0)
    {
        PyErr_Format(PyExc_ValueError, "expected %s value", _info->name.c_str());
        throw AbortMarshaling();
    }

    //
    // Validate value.
    //
    PyObjectHandle val = PyObject_GetAttrString(p, "value");
    if(val.get() == NULL)
    {
        throw AbortMarshaling();
    }
    if(!PyInt_Check(val.get()))
    {
        PyErr_Format(PyExc_ValueError, "value for enum %s is not an int", _info->name.c_str());
        throw AbortMarshaling();
    }
    int ival = static_cast<int>(PyInt_AsLong(val.get()));
    int count = static_cast<int>(_info->enumerators.size());
    if(ival < 0 || ival >= count)
    {
        PyErr_Format(PyExc_ValueError, "value %d is out of range for enum %s", ival, _info->name.c_str());
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

PyObject*
IcePy::EnumMarshaler::unmarshal(const Ice::CommunicatorPtr&, const Ice::InputStreamPtr& is)
{
    int val;
    int count = static_cast<int>(_info->enumerators.size());
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
        PyErr_Format(PyExc_ValueError, "enumerator %d is out of range for enum %s", val, _info->name.c_str());
        return NULL;
    }

    PyObject* pyval = _info->enumerators[val].get();
    assert(pyval);
    Py_INCREF(pyval);
    return pyval;
}

void
IcePy::EnumMarshaler::destroy()
{
}

//
// NativeDictionaryMarshaler implementation.
//
IcePy::NativeDictionaryMarshaler::NativeDictionaryMarshaler(const DictionaryInfoPtr& info) :
    _info(info)
{
    _keyMarshaler = createMarshaler(info->keyType);
    _valueMarshaler = createMarshaler(info->valueType);
}

void
IcePy::NativeDictionaryMarshaler::marshal(PyObject* p, const Ice::OutputStreamPtr& os, ObjectMap* objectMap)
{
    if(p == Py_None)
    {
        os->writeSize(0);
        return;
    }
    else if(!PyDict_Check(p))
    {
        PyErr_Format(PyExc_ValueError, "expected dictionary value");
        throw AbortMarshaling();
    }

    int sz = PyDict_Size(p);
    os->writeSize(sz);

    int pos = 0;
    PyObject* key;
    PyObject* value;
    while(PyDict_Next(p, &pos, &key, &value))
    {
        _keyMarshaler->marshal(key, os, objectMap);
        _valueMarshaler->marshal(value, os, objectMap);
    }
}

PyObject*
IcePy::NativeDictionaryMarshaler::unmarshal(const Ice::CommunicatorPtr& communicator, const Ice::InputStreamPtr& is)
{
    PyObjectHandle p = PyDict_New();
    if(p.get() == NULL)
    {
        return NULL;
    }

    ClassInfoPtr valueClassInfo = ClassInfoPtr::dynamicCast(_info->valueType);
    ObjectMarshalerPtr om = ObjectMarshalerPtr::dynamicCast(_valueMarshaler);

    Ice::Int sz = is->readSize();
    for(Ice::Int i = 0; i < sz; ++i)
    {
        PyObjectHandle key = _keyMarshaler->unmarshal(communicator, is);
        if(key.get() == NULL)
        {
            return NULL;
        }

        if(valueClassInfo)
        {
            assert(om);
            om->unmarshalObject(communicator, is, new ValueReceiver(valueClassInfo, p.get(), key.get()));
        }
        else
        {
            PyObjectHandle value = _valueMarshaler->unmarshal(communicator, is);
            if(value.get() == NULL)
            {
                return NULL;
            }

            if(PyDict_SetItem(p.get(), key.get(), value.get()) < 0)
            {
                return NULL;
            }
        }
    }

    return p.release();
}

void
IcePy::NativeDictionaryMarshaler::destroy()
{
    _keyMarshaler->destroy();
    _keyMarshaler = 0;
    _valueMarshaler->destroy();
    _valueMarshaler = 0;
}

IcePy::NativeDictionaryMarshaler::ValueReceiver::ValueReceiver(const ClassInfoPtr& info, PyObject* dict,
                                                               PyObject* key) :
    ObjectReceiver(info), _dict(dict), _key(key)
{
    Py_INCREF(dict);
    Py_INCREF(key);
    assert(PyDict_Check(dict));
}

void
IcePy::NativeDictionaryMarshaler::ValueReceiver::setObject(PyObject* p)
{
    if(PyDict_SetItem(_dict.get(), _key.get(), p) < 0)
    {
        throw AbortMarshaling();
    }
}

//
// ExceptionMarshaler implementation.
//
IcePy::ExceptionMarshaler::ExceptionMarshaler(const ExceptionInfoPtr& info) :
    _info(info)
{
}

void
IcePy::ExceptionMarshaler::marshal(PyObject* p, const Ice::OutputStreamPtr& os, ObjectMap* objectMap)
{
    if(!PyInstance_Check(p) || !PyObject_IsInstance(p, _info->pythonType.get()))
    {
        PyErr_Format(PyExc_ValueError, "expected exception %s", _info->name.c_str());
        throw AbortMarshaling();
    }

    os->writeBool(_info->usesClasses);

    ExceptionInfoPtr info = _info;
    while(info)
    {
        os->writeString(info->name);

        os->startSlice();
        for(DataMemberList::iterator q = info->members.begin(); q != info->members.end(); ++q)
        {
            MarshalerPtr marshaler = createMarshaler(q->type);

            PyObjectHandle val = PyObject_GetAttrString(p, const_cast<char*>(q->name.c_str()));
            if(val.get() == NULL)
            {
                throw AbortMarshaling();
            }

            marshaler->marshal(val.get(), os, objectMap);
        }
        os->endSlice();

        info = info->base;
    }
}

PyObject*
IcePy::ExceptionMarshaler::unmarshal(const Ice::CommunicatorPtr& communicator, const Ice::InputStreamPtr& is)
{
    PyObjectHandle p = createExceptionInstance(_info->pythonType.get());

    //
    // NOTE: The type id for the first slice has already been read.
    //
    ExceptionInfoPtr info = _info;
    while(info)
    {
        is->startSlice();
        for(DataMemberList::iterator q = info->members.begin(); q != info->members.end(); ++q)
        {
            MarshalerPtr marshaler = createMarshaler(q->type);
            ObjectMarshalerPtr om = ObjectMarshalerPtr::dynamicCast(marshaler);
            if(om)
            {
                ClassInfoPtr cls = ClassInfoPtr::dynamicCast(q->type);
                assert(cls);
                om->unmarshalObject(communicator, is, new ObjectMemberReceiver(cls, p.get(), q->name));
            }
            else
            {
                PyObjectHandle m = marshaler->unmarshal(communicator, is);
                if(m.get() == NULL)
                {
                    return NULL;
                }

                if(PyObject_SetAttrString(p.get(), const_cast<char*>(q->name.c_str()), m.get()) < 0)
                {
                    return NULL;
                }
            }
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
IcePy::ExceptionMarshaler::destroy()
{
}

//
// ObjectReceiver implementation.
//
IcePy::ObjectReceiver::ObjectReceiver(const ClassInfoPtr& info) :
    _info(info)
{
}

IcePy::ObjectReceiver::~ObjectReceiver()
{
}

void
IcePy::ObjectReceiver::invoke(const Ice::ObjectPtr& p)
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
            Ice::NoObjectFactoryException ex(__FILE__, __LINE__);
            ex.type = _info->name;
            throw ex;
        }

        setObject(obj);
    }
    else
    {
        setObject(Py_None);
    }
}

//
// TupleReceiver implementation.
//
IcePy::TupleReceiver::TupleReceiver(const ClassInfoPtr& info, PyObject* tuple, int index) :
    ObjectReceiver(info), _tuple(tuple), _index(index)
{
    Py_INCREF(tuple);
    assert(PyTuple_Check(tuple));
    assert(_index < PyTuple_GET_SIZE(tuple));
}

void
IcePy::TupleReceiver::setObject(PyObject* p)
{
    Py_INCREF(p); // PyTuple_SET_ITEM steals a reference.
    PyTuple_SET_ITEM(_tuple.get(), _index, p);
}

//
// ObjectMarshaler implementation.
//
IcePy::ObjectMarshaler::ObjectMarshaler(const ClassInfoPtr& info) :
    _info(info)
{
}

IcePy::ObjectMarshaler::~ObjectMarshaler()
{
}

void
IcePy::ObjectMarshaler::marshal(PyObject* p, const Ice::OutputStreamPtr& os, ObjectMap* objectMap)
{
    if(p == Py_None)
    {
        os->writeObject(0);
        return;
    }

    if(_info->pythonType.get() == NULL)
    {
        PyErr_Format(PyExc_RuntimeError, "class %s is declared but not defined", _info->name.c_str());
        throw AbortMarshaling();
    }

    if(!PyObject_IsInstance(p, _info->pythonType.get()))
    {
        PyErr_Format(PyExc_ValueError, "expected value of type %s", _info->name.c_str());
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
        PyObjectHandle id = PyObject_CallMethod(p, "ice_id", NULL);
        if(id.get() == NULL)
        {
            throw AbortMarshaling();
        }
        assert(PyString_Check(id.get()));
        char* str = PyString_AS_STRING(id.get());
        ClassInfoPtr info = ClassInfoPtr::dynamicCast(getTypeInfo(str));
        if(!info)
        {
            PyErr_Format(PyExc_ValueError, "unknown class type %s", str);
            throw AbortMarshaling();
        }
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

PyObject*
IcePy::ObjectMarshaler::unmarshal(const Ice::CommunicatorPtr& communicator, const Ice::InputStreamPtr& is)
{
    //
    // Not implemented. Use unmarshalObject.
    //
    assert(false);
    return NULL;
}

void
IcePy::ObjectMarshaler::unmarshalObject(const Ice::CommunicatorPtr&, const Ice::InputStreamPtr& is,
                                        const ObjectReceiverPtr& receiver)
{
    if(_info->pythonType.get() == NULL)
    {
        PyErr_Format(PyExc_RuntimeError, "class %s is declared but not defined", _info->name.c_str());
        throw AbortMarshaling();
    }

    is->readObject(receiver);
}

void
IcePy::ObjectMarshaler::destroy()
{
}

IcePy::ClassInfoPtr
IcePy::ObjectMarshaler::info() const
{
    return _info;
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
IcePy::ObjectWriter::write(const Ice::OutputStreamPtr& os) const
{
    ClassInfoPtr info = _info;
    while(info)
    {
        os->writeTypeId(info->name);

        os->startSlice();
        for(DataMemberList::iterator q = info->members.begin(); q != info->members.end(); ++q)
        {
            MarshalerPtr marshaler = Marshaler::createMarshaler(q->type); // TODO: Cache this

            PyObjectHandle val = PyObject_GetAttrString(_object, const_cast<char*>(q->name.c_str()));
            if(val.get() == NULL)
            {
                throw AbortMarshaling();
            }

            marshaler->marshal(val.get(), os, _map);
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
IcePy::ObjectReader::ObjectReader(PyObject* object, const ClassInfoPtr& info,
                                  const Ice::CommunicatorPtr& communicator) :
    _object(object), _info(info), _communicator(communicator)
{
    Py_INCREF(_object);
}

IcePy::ObjectReader::~ObjectReader()
{
    Py_DECREF(_object);
}

void
IcePy::ObjectReader::read(const Ice::InputStreamPtr& is, bool rid)
{
    //
    // Unmarshal the slices of a user-defined class.
    //
    if(_info->name != Ice::Object::ice_staticId())
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
                char* memberName = const_cast<char*>(p->name.c_str());
                MarshalerPtr marshaler = Marshaler::createMarshaler(p->type); // TODO: Cache this
                assert(marshaler);
                ObjectMarshalerPtr om = ObjectMarshalerPtr::dynamicCast(marshaler);
                if(om)
                {
                    ClassInfoPtr cls = ClassInfoPtr::dynamicCast(p->type);
                    assert(cls);
                    om->unmarshalObject(_communicator, is, new ObjectMemberReceiver(cls, _object, p->name));
                }
                else
                {
                    PyObjectHandle attr = marshaler->unmarshal(_communicator, is);
                    if(attr.get() == NULL)
                    {
                        throw AbortMarshaling();
                    }

                    if(PyObject_SetAttrString(_object, memberName, attr.get()) < 0)
                    {
                        throw AbortMarshaling();
                    }
                }
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

PyObject*
IcePy::ObjectReader::getObject() const
{
    return _object;
}
