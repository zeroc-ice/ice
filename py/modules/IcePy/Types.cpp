// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
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
    if (self == NULL)
    {
        return NULL;
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
    if (self == NULL)
    {
        return NULL;
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
IcePy::PrimitiveInfo::marshal(PyObject* p, const Ice::OutputStreamPtr& os, ObjectMap*)
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
            val = PyString_AS_STRING(p);
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
                                void* closure)
{
    switch(kind)
    {
    case PrimitiveInfo::KindBool:
    {
        if(is->readBool())
        {
            cb->unmarshaled(Py_True, target, closure);
        }
        else
        {
            cb->unmarshaled(Py_False, target, closure);
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
    if(p.get() == NULL)
    {
	return;
    }
    assert(PyString_Check(p.get()));
    out << PyString_AS_STRING(p.get());
}


void
IcePy::PrimitiveInfo::marshalSequence(PyObject* p, const Ice::OutputStreamPtr& os)
{
    PyObjectHandle fs = PySequence_Fast(p, STRCAST("expected a sequence value"));
    if(fs.get() == NULL)
    {
        return;
    }

    switch(kind)
    {
    case PrimitiveInfo::KindBool:
    {
        int sz = PySequence_Fast_GET_SIZE(fs.get());
        Ice::BoolSeq seq(sz);
        for(int i = 0; i < sz; ++i)
        {
            PyObject* item = PySequence_Fast_GET_ITEM(fs.get(), i);
            if(item == NULL)
            {
                throw AbortMarshaling();
            }
            int isTrue = PyObject_IsTrue(item);
            if(isTrue < 0)
            {
                PyErr_Format(PyExc_ValueError, STRCAST("invalid value for element %d of sequence<bool>"), i);
                throw AbortMarshaling();
            }
            seq[i] = isTrue ? true : false;
        }
        os->writeBoolSeq(seq);
        break;
    }
    case PrimitiveInfo::KindByte:
    {
        int sz = PySequence_Fast_GET_SIZE(fs.get());
        Ice::ByteSeq seq(sz);
        for(int i = 0; i < sz; ++i)
        {
            PyObject* item = PySequence_Fast_GET_ITEM(fs.get(), i);
            if(item == NULL)
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
                PyErr_Format(PyExc_ValueError, STRCAST("invalid value for element %d of sequence<byte>"), i);
                throw AbortMarshaling();
            }
            seq[i] = static_cast<Ice::Byte>(val);
        }
        os->writeByteSeq(seq);
        break;
    }
    case PrimitiveInfo::KindShort:
    {
        int sz = PySequence_Fast_GET_SIZE(fs.get());
        Ice::ShortSeq seq(sz);
        for(int i = 0; i < sz; ++i)
        {
            PyObject* item = PySequence_Fast_GET_ITEM(fs.get(), i);
            if(item == NULL)
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
                PyErr_Format(PyExc_ValueError, STRCAST("invalid value for element %d of sequence<short>"), i);
                throw AbortMarshaling();
            }
            seq[i] = static_cast<Ice::Short>(val);
        }
        os->writeShortSeq(seq);
        break;
    }
    case PrimitiveInfo::KindInt:
    {
        int sz = PySequence_Fast_GET_SIZE(fs.get());
        Ice::IntSeq seq(sz);
        for(int i = 0; i < sz; ++i)
        {
            PyObject* item = PySequence_Fast_GET_ITEM(fs.get(), i);
            if(item == NULL)
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
                PyErr_Format(PyExc_ValueError, STRCAST("invalid value for element %d of sequence<int>"), i);
                throw AbortMarshaling();
            }

            if(PyErr_Occurred() || val < INT_MIN || val > INT_MAX)
            {
                PyErr_Format(PyExc_ValueError, STRCAST("invalid value for element %d of sequence<int>"), i);
                throw AbortMarshaling();
            }
            seq[i] = static_cast<Ice::Int>(val);
        }
        os->writeIntSeq(seq);
        break;
    }
    case PrimitiveInfo::KindLong:
    {
        int sz = PySequence_Fast_GET_SIZE(fs.get());
        Ice::LongSeq seq(sz);
        for(int i = 0; i < sz; ++i)
        {
            PyObject* item = PySequence_Fast_GET_ITEM(fs.get(), i);
            if(item == NULL)
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
                PyErr_Format(PyExc_ValueError, STRCAST("invalid value for element %d of sequence<int>"), i);
                throw AbortMarshaling();
            }

            if(PyErr_Occurred())
            {
                PyErr_Format(PyExc_ValueError, STRCAST("invalid value for element %d of sequence<int>"), i);
                throw AbortMarshaling();
            }
            seq[i] = val;
        }
        os->writeLongSeq(seq);
        break;
    }
    case PrimitiveInfo::KindFloat:
    {
        int sz = PySequence_Fast_GET_SIZE(fs.get());
        Ice::FloatSeq seq(sz);
        for(int i = 0; i < sz; ++i)
        {
            PyObject* item = PySequence_Fast_GET_ITEM(fs.get(), i);
            if(item == NULL)
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
                PyErr_Format(PyExc_ValueError, STRCAST("invalid value for element %d of sequence<float>"), i);
                throw AbortMarshaling();
            }

            seq[i] = val;
        }
        os->writeFloatSeq(seq);
        break;
    }
    case PrimitiveInfo::KindDouble:
    {
        int sz = PySequence_Fast_GET_SIZE(fs.get());
        Ice::DoubleSeq seq(sz);
        for(int i = 0; i < sz; ++i)
        {
            PyObject* item = PySequence_Fast_GET_ITEM(fs.get(), i);
            if(item == NULL)
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
                PyErr_Format(PyExc_ValueError, STRCAST("invalid value for element %d of sequence<double>"), i);
                throw AbortMarshaling();
            }

            seq[i] = val;
        }
        os->writeDoubleSeq(seq);
        break;
    }
    case PrimitiveInfo::KindString:
    {
        int sz = PySequence_Fast_GET_SIZE(fs.get());
        Ice::StringSeq seq(sz);
        for(int i = 0; i < sz; ++i)
        {
            PyObject* item = PySequence_Fast_GET_ITEM(fs.get(), i);
            if(item == NULL)
            {
                throw AbortMarshaling();
            }

            string val;
            if(PyString_Check(item))
            {
                val = PyString_AS_STRING(item);
            }
            else if(p != Py_None)
            {
                PyErr_Format(PyExc_ValueError, STRCAST("invalid value for element %d of sequence<string>"), i);
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
IcePy::PrimitiveInfo::unmarshalSequence(const Ice::InputStreamPtr& is, const UnmarshalCallbackPtr& cb, PyObject* target,
                                        void* closure)
{
    PyObjectHandle list;

    switch(kind)
    {
    case PrimitiveInfo::KindBool:
    {
        Ice::BoolSeq seq = is->readBoolSeq();
        int sz = static_cast<int>(seq.size());
        list = PyList_New(sz);
        if(list.get() == NULL)
        {
            throw AbortMarshaling();
        }

        for(int i = 0; i < sz; ++i)
        {
            PyObject* val = seq[i] ? Py_True : Py_False;
            Py_INCREF(val);
            PyList_SET_ITEM(list.get(), i, val); // PyList_SET_ITEM steals a reference.
        }
        break;
    }
    case PrimitiveInfo::KindByte:
    {
        Ice::ByteSeq seq = is->readByteSeq();
        int sz = static_cast<int>(seq.size());
        list = PyList_New(sz);
        if(list.get() == NULL)
        {
            throw AbortMarshaling();
        }

        for(int i = 0; i < sz; ++i)
        {
            PyObjectHandle item = PyInt_FromLong(seq[i]);
            if(item.get() == NULL)
            {
                throw AbortMarshaling();
            }
            PyList_SET_ITEM(list.get(), i, item.release()); // PyList_SET_ITEM steals a reference.
        }
        break;
    }
    case PrimitiveInfo::KindShort:
    {
        Ice::ShortSeq seq = is->readShortSeq();
        int sz = static_cast<int>(seq.size());
        list = PyList_New(sz);
        if(list.get() == NULL)
        {
            throw AbortMarshaling();
        }

        for(int i = 0; i < sz; ++i)
        {
            PyObjectHandle item = PyInt_FromLong(seq[i]);
            if(item.get() == NULL)
            {
                throw AbortMarshaling();
            }
            PyList_SET_ITEM(list.get(), i, item.release()); // PyList_SET_ITEM steals a reference.
        }
        break;
    }
    case PrimitiveInfo::KindInt:
    {
        Ice::IntSeq seq = is->readIntSeq();
        int sz = static_cast<int>(seq.size());
        list = PyList_New(sz);
        if(list.get() == NULL)
        {
            throw AbortMarshaling();
        }

        for(int i = 0; i < sz; ++i)
        {
            PyObjectHandle item = PyInt_FromLong(seq[i]);
            if(item.get() == NULL)
            {
                throw AbortMarshaling();
            }
            PyList_SET_ITEM(list.get(), i, item.release()); // PyList_SET_ITEM steals a reference.
        }
        break;
    }
    case PrimitiveInfo::KindLong:
    {
        Ice::LongSeq seq = is->readLongSeq();
        int sz = static_cast<int>(seq.size());
        list = PyList_New(sz);
        if(list.get() == NULL)
        {
            throw AbortMarshaling();
        }

        for(int i = 0; i < sz; ++i)
        {
            PyObjectHandle item = PyLong_FromLongLong(seq[i]);
            if(item.get() == NULL)
            {
                throw AbortMarshaling();
            }
            PyList_SET_ITEM(list.get(), i, item.release()); // PyList_SET_ITEM steals a reference.
        }
        break;
    }
    case PrimitiveInfo::KindFloat:
    {
        Ice::FloatSeq seq = is->readFloatSeq();
        int sz = static_cast<int>(seq.size());
        list = PyList_New(sz);
        if(list.get() == NULL)
        {
            throw AbortMarshaling();
        }

        for(int i = 0; i < sz; ++i)
        {
            PyObjectHandle item = PyFloat_FromDouble(seq[i]);
            if(item.get() == NULL)
            {
                throw AbortMarshaling();
            }
            PyList_SET_ITEM(list.get(), i, item.release()); // PyList_SET_ITEM steals a reference.
        }
        break;
    }
    case PrimitiveInfo::KindDouble:
    {
        Ice::DoubleSeq seq = is->readDoubleSeq();
        int sz = static_cast<int>(seq.size());
        list = PyList_New(sz);
        if(list.get() == NULL)
        {
            throw AbortMarshaling();
        }

        for(int i = 0; i < sz; ++i)
        {
            PyObjectHandle item = PyFloat_FromDouble(seq[i]);
            if(item.get() == NULL)
            {
                throw AbortMarshaling();
            }
            PyList_SET_ITEM(list.get(), i, item.release()); // PyList_SET_ITEM steals a reference.
        }
        break;
    }
    case PrimitiveInfo::KindString:
    {
        Ice::StringSeq seq = is->readStringSeq();
        int sz = static_cast<int>(seq.size());
        list = PyList_New(sz);
        if(list.get() == NULL)
        {
            throw AbortMarshaling();
        }

        for(int i = 0; i < sz; ++i)
        {
            PyObjectHandle item = PyString_FromString(seq[i].c_str());
            if(item.get() == NULL)
            {
                throw AbortMarshaling();
            }
            PyList_SET_ITEM(list.get(), i, item.release()); // PyList_SET_ITEM steals a reference.
        }
        break;
    }
    }
    cb->unmarshaled(list.get(), target, closure);
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
IcePy::EnumInfo::marshal(PyObject* p, const Ice::OutputStreamPtr& os, ObjectMap*)
{
    assert(PyObject_IsInstance(p, pythonType.get()) == 1); // validate() should have caught this.

    //
    // Validate value.
    //
    PyObjectHandle val = PyObject_GetAttrString(p, STRCAST("value"));
    if(val.get() == NULL)
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
                           void* closure)
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
    if(p.get() == NULL)
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
IcePy::StructInfo::marshal(PyObject* p, const Ice::OutputStreamPtr& os, ObjectMap* objectMap)
{
    assert(PyObject_IsInstance(p, pythonType.get()) == 1); // validate() should have caught this.

    int i = 0;
    for(DataMemberList::iterator q = members.begin(); q != members.end(); ++q, ++i)
    {
        DataMemberPtr member = *q;
        char* memberName = const_cast<char*>(member->name.c_str());
        PyObjectHandle attr = PyObject_GetAttrString(p, memberName);
        if(attr.get() == NULL)
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
        member->type->marshal(attr.get(), os, objectMap);
    }
}

void
IcePy::StructInfo::unmarshal(const Ice::InputStreamPtr& is, const UnmarshalCallbackPtr& cb, PyObject* target,
                             void* closure)
{
    PyObjectHandle args = PyTuple_New(0);
    PyTypeObject* type = (PyTypeObject*)pythonType.get();
    PyObjectHandle p = type->tp_new(type, args.get(), NULL);
    if(p.get() == NULL)
    {
        throw AbortMarshaling();
    }

    int i = 0;
    for(DataMemberList::iterator q = members.begin(); q != members.end(); ++q, ++i)
    {
        DataMemberPtr member = *q;
        member->type->unmarshal(is, member, p.get(), 0);
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
	if(attr.get() == NULL)
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
IcePy::SequenceInfo::marshal(PyObject* p, const Ice::OutputStreamPtr& os, ObjectMap* objectMap)
{
    if(p == Py_None)
    {
        os->writeSize(0);
        return;
    }

    PrimitiveInfoPtr pi = PrimitiveInfoPtr::dynamicCast(elementType);
    if(pi)
    {
        pi->marshalSequence(p, os);
        return;
    }

    PyObjectHandle fastSeq = PySequence_Fast(p, STRCAST("expected a sequence value"));
    if(fastSeq.get() == NULL)
    {
        return;
    }

    int sz = PySequence_Fast_GET_SIZE(fastSeq.get());
    os->writeSize(sz);
    for(int i = 0; i < sz; ++i)
    {
        PyObject* item = PySequence_Fast_GET_ITEM(fastSeq.get(), i);
        if(item == NULL)
        {
            throw AbortMarshaling();
        }
        if(!elementType->validate(item))
        {
            PyErr_Format(PyExc_ValueError, STRCAST("invalid value for element %d of `%s'"), i,
			 const_cast<char*>(id.c_str()));
            throw AbortMarshaling();
        }
        elementType->marshal(item, os, objectMap);
    }
}

void
IcePy::SequenceInfo::unmarshal(const Ice::InputStreamPtr& is, const UnmarshalCallbackPtr& cb, PyObject* target,
                               void* closure)
{
    PrimitiveInfoPtr pi = PrimitiveInfoPtr::dynamicCast(elementType);
    if(pi)
    {
        pi->unmarshalSequence(is, cb, target, closure);
        return;
    }

    Ice::Int sz = is->readSize();
    PyObjectHandle result = PyList_New(sz);
    if(result.get() == NULL)
    {
        throw AbortMarshaling();
    }

    for(Ice::Int i = 0; i < sz; ++i)
    {
        elementType->unmarshal(is, this, result.get(), (void*)i);
    }

    cb->unmarshaled(result.get(), target, closure);
}

void
IcePy::SequenceInfo::unmarshaled(PyObject* val, PyObject* target, void* closure)
{
    int i = reinterpret_cast<int>(closure);
    PyList_SET_ITEM(target, i, val);
    Py_INCREF(val); // PyList_SET_ITEM steals a reference.
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
	if(fastSeq.get() == NULL)
	{
	    return;
	}

	int sz = PySequence_Fast_GET_SIZE(fastSeq.get());

	out.sb();
	for(int i = 0; i < sz; ++i)
	{
	    PyObject* item = PySequence_Fast_GET_ITEM(fastSeq.get(), i);
	    if(item == NULL)
	    {
		break;
	    }
	    out << nl << '[' << i << "] = ";
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
IcePy::DictionaryInfo::marshal(PyObject* p, const Ice::OutputStreamPtr& os, ObjectMap* objectMap)
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

    int sz = PyDict_Size(p);
    os->writeSize(sz);

    int pos = 0;
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
                                 void* closure)
{
    PyObjectHandle p = PyDict_New();
    if(p.get() == NULL)
    {
        throw AbortMarshaling();
    }

    KeyCallbackPtr keyCB = new KeyCallback;
    keyCB->key = NULL;

    Ice::Int sz = is->readSize();
    for(Ice::Int i = 0; i < sz; ++i)
    {
        //
        // A dictionary key cannot be a class (or contain one), so the key must be
        // available immediately.
        //
        keyType->unmarshal(is, keyCB, NULL, 0);
        assert(keyCB->key.get() != NULL);

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
        valueType->unmarshal(is, this, p.get(), (void*)keyCB->key.get());
    }

    cb->unmarshaled(p.get(), target, closure);
}

void
IcePy::DictionaryInfo::unmarshaled(PyObject* val, PyObject* target, void* closure)
{
    PyObject* key = (PyObject*)closure;
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
	int pos = 0;
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
IcePy::ClassInfo::marshal(PyObject* p, const Ice::OutputStreamPtr& os, ObjectMap* objectMap)
{
    if(pythonType.get() == NULL)
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
        if(iceType.get() == NULL)
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
                            void* closure)
{
    if(pythonType.get() == NULL)
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
	    assert(iceType.get() != NULL);
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
    typeObj = NULL; // Break circular reference.
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
	if(attr.get() == NULL)
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
IcePy::ProxyInfo::marshal(PyObject* p, const Ice::OutputStreamPtr& os, ObjectMap*)
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
                            void* closure)
{
    Ice::ObjectPrx proxy = is->readProxy();

    if(!proxy)
    {
        cb->unmarshaled(Py_None, target, closure);
        return;
    }

    if(pythonType.get() == NULL)
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
	if(p.get() == NULL)
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
    typeObj = NULL; // Break circular reference.
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
        PyObjectHandle tmp = PyObject_CallMethod(_object, STRCAST("ice_preMarshal"), NULL);
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
            if(val.get() == NULL)
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

            member->type->marshal(val.get(), os, _map);
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
        PyObjectHandle tmp = PyObject_CallMethod(_object, STRCAST("ice_postUnmarshal"), NULL);
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
            Ice::NoObjectFactoryException ex(__FILE__, __LINE__);
            ex.type = _info->id;
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
    if(!PyInstance_Check(p) || !PyObject_IsInstance(p, pythonType.get()))
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
            if(val.get() == NULL)
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

            member->type->marshal(val.get(), os, objectMap);
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
            member->type->unmarshal(is, member, p.get(), 0);
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
    if(!PyInstance_Check(value) || !PyObject_IsInstance(value, pythonType.get()))
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
	if(attr.get() == NULL)
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
    PyObject_HEAD_INIT(NULL)
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
    PyObject_HEAD_INIT(NULL)
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
    if(PyModule_AddObject(module, STRCAST("TypeInfo"), (PyObject*)&TypeInfoType) < 0)
    {
        return false;
    }

    if(PyType_Ready(&ExceptionInfoType) < 0)
    {
        return false;
    }
    if(PyModule_AddObject(module, STRCAST("ExceptionInfo"), (PyObject*)&ExceptionInfoType) < 0)
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
    assert(PyObject_IsInstance(obj, (PyObject*)&TypeInfoType));
    TypeInfoObject* p = (TypeInfoObject*)obj;
    return *p->info;
}

PyObject*
IcePy::createType(const TypeInfoPtr& info)
{
    TypeInfoObject* obj = typeInfoNew(NULL);
    if(obj != NULL)
    {
        obj->info = new IcePy::TypeInfoPtr(info);
    }
    return (PyObject*)obj;
}

IcePy::ExceptionInfoPtr
IcePy::getException(PyObject* obj)
{
    assert(PyObject_IsInstance(obj, (PyObject*)&ExceptionInfoType));
    ExceptionInfoObject* p = (ExceptionInfoObject*)obj;
    return *p->info;
}

PyObject*
IcePy::createException(const ExceptionInfoPtr& info)
{
    ExceptionInfoObject* obj = exceptionInfoNew(NULL);
    if(obj != NULL)
    {
        obj->info = new IcePy::ExceptionInfoPtr(info);
    }
    return (PyObject*)obj;
}

extern "C"
PyObject*
IcePy_defineEnum(PyObject*, PyObject* args)
{
    char* id;
    PyObject* type;
    PyObject* enumerators;
    if(!PyArg_ParseTuple(args, STRCAST("sOO"), &id, &type, &enumerators))
    {
        return NULL;
    }

    assert(PyType_Check(type));
    assert(PyTuple_Check(enumerators));

    EnumInfoPtr info = new EnumInfo;
    info->id = id;
    info->pythonType = type;
    Py_INCREF(type);

    int sz = PyTuple_GET_SIZE(enumerators);
    for(int i = 0; i < sz; ++i)
    {
        PyObjectHandle e = PyTuple_GET_ITEM(enumerators, i);
        Py_INCREF(e.get());
        assert(PyObject_IsInstance(e.get(), type));
        info->enumerators.push_back(e);
    }

    return createType(info);
}

extern "C"
PyObject*
IcePy_defineStruct(PyObject*, PyObject* args)
{
    char* id;
    PyObject* type;
    PyObject* members;
    if(!PyArg_ParseTuple(args, STRCAST("sOO"), &id, &type, &members))
    {
        return NULL;
    }

    assert(PyType_Check(type));
    assert(PyTuple_Check(members));

    StructInfoPtr info = new StructInfo;
    info->id = id;
    info->pythonType = type;
    Py_INCREF(type);

    int sz = PyTuple_GET_SIZE(members);
    for(int i = 0; i < sz; ++i)
    {
        PyObject* m = PyTuple_GET_ITEM(members, i);
        assert(PyTuple_Check(m));
        assert(PyTuple_GET_SIZE(m) == 2);
        PyObject* s = PyTuple_GET_ITEM(m, 0); // Member name.
        assert(PyString_Check(s));
        PyObject* t = PyTuple_GET_ITEM(m, 1); // Member type.
        DataMemberPtr member = new DataMember;
        member->name = PyString_AS_STRING(s);
        member->type = getType(t);
        info->members.push_back(member);
    }

    return createType(info);
}

extern "C"
PyObject*
IcePy_defineSequence(PyObject*, PyObject* args)
{
    char* id;
    PyObject* elementType;
    if(!PyArg_ParseTuple(args, STRCAST("sO"), &id, &elementType))
    {
        return NULL;
    }

    SequenceInfoPtr info = new SequenceInfo;
    info->id = id;
    info->elementType = getType(elementType);

    return createType(info);
}

extern "C"
PyObject*
IcePy_defineDictionary(PyObject*, PyObject* args)
{
    char* id;
    PyObject* keyType;
    PyObject* valueType;
    if(!PyArg_ParseTuple(args, STRCAST("sOO"), &id, &keyType, &valueType))
    {
        return NULL;
    }

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
        return NULL;
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
        return NULL;
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
        return NULL;
    }

    ClassInfoPtr info = lookupClassInfo(id);
    if(!info)
    {
        info = new ClassInfo;
        info->id = id;
        info->typeObj = createType(info);
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
    int isAbstract;
    PyObject* base;
    PyObject* interfaces;
    PyObject* members;
    if(!PyArg_ParseTuple(args, STRCAST("sOiOOO"), &id, &type, &isAbstract, &base, &interfaces, &members))
    {
        return NULL;
    }

    assert(PyType_Check(type));
    assert(PyTuple_Check(interfaces));
    assert(PyTuple_Check(members));

    ClassInfoPtr info = lookupClassInfo(id);
    if(!info)
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

    int i, sz;
    sz = PyTuple_GET_SIZE(interfaces);
    for(i = 0; i < sz; ++i)
    {
        PyObject* o = PyTuple_GET_ITEM(interfaces, i);
        ClassInfoPtr iface = ClassInfoPtr::dynamicCast(getType(o));
        assert(iface);
        info->interfaces.push_back(iface);
    }

    sz = PyTuple_GET_SIZE(members);
    for(i = 0; i < sz; ++i)
    {
        PyObject* m = PyTuple_GET_ITEM(members, i);
        assert(PyTuple_Check(m));
        assert(PyTuple_GET_SIZE(m) == 2);
        PyObject* s = PyTuple_GET_ITEM(m, 0); // Member name.
        assert(PyString_Check(s));
        PyObject* t = PyTuple_GET_ITEM(m, 1); // Member type.
        DataMemberPtr member = new DataMember;
        member->name = PyString_AS_STRING(s);
        member->type = getType(t);
        info->members.push_back(member);
    }

    info->pythonType = type;
    Py_INCREF(type);

    Py_INCREF(info->typeObj.get());
    return info->typeObj.get();
}

extern "C"
PyObject*
IcePy_defineException(PyObject*, PyObject* args)
{
    char* id;
    PyObject* type;
    PyObject* base;
    PyObject* members;
    if(!PyArg_ParseTuple(args, STRCAST("sOOO"), &id, &type, &base, &members))
    {
        return NULL;
    }

    assert(PyClass_Check(type));
    assert(PyTuple_Check(members));

    ExceptionInfoPtr info = new ExceptionInfo;
    info->id = id;

    if(base != Py_None)
    {
        info->base = ExceptionInfoPtr::dynamicCast(getException(base));
        assert(info->base);
    }

    info->usesClasses = false;

    int sz = PyTuple_GET_SIZE(members);
    for(int i = 0; i < sz; ++i)
    {
        PyObject* m = PyTuple_GET_ITEM(members, i);
        assert(PyTuple_Check(m));
        assert(PyTuple_GET_SIZE(m) == 2);
        PyObject* s = PyTuple_GET_ITEM(m, 0); // Member name.
        assert(PyString_Check(s));
        PyObject* t = PyTuple_GET_ITEM(m, 1); // Member type.
        DataMemberPtr member = new DataMember;
        member->name = PyString_AS_STRING(s);
        member->type = getType(t);
        info->members.push_back(member);
        if(!info->usesClasses)
        {
            info->usesClasses = member->type->usesClasses();
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
        return NULL;
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
        return NULL;
    }

    PyObjectHandle iceType = PyObject_GetAttrString(value, STRCAST("ice_type"));
    assert(iceType.get() != NULL);
    ExceptionInfoPtr info = getException(iceType.get());
    assert(info);

    ostringstream ostr;
    IceUtil::Output out(ostr);
    info->print(value, out);

    string str = ostr.str();
    return PyString_FromString(str.c_str());
}
