// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Types.h>
#include <Current.h>
#include <Proxy.h>
#include <Util.h>
#include <IceUtil/InputUtil.h>
#include <Ice/IdentityUtil.h>
#include <Ice/Initialize.h>
#include <Ice/LocalException.h>
#include <Ice/ObjectAdapter.h>
#include <Slice/PythonUtil.h>

using namespace std;
using namespace IcePy;
using namespace Slice::Python;

// TODO: Destroyer for maps?

typedef map<string, TypeInfoPtr> TypeInfoMap;
static TypeInfoMap _typeInfoMap;

typedef map<string, ExceptionInfoPtr> ExceptionInfoMap;
static ExceptionInfoMap _exceptionInfoMap;

namespace IcePy
{

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

}

//
// addTypeInfo()
//
static void
addTypeInfo(const string& id, const TypeInfoPtr& info)
{
    TypeInfoMap::iterator p = _typeInfoMap.find(id);
    assert(p == _typeInfoMap.end());
    _typeInfoMap.insert(TypeInfoMap::value_type(id, info));
}

//
// addExceptionInfo()
//
static void
addExceptionInfo(const string& id, const ExceptionInfoPtr& info)
{
    ExceptionInfoMap::iterator p = _exceptionInfoMap.find(id);
    assert(p == _exceptionInfoMap.end());
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

        if(val < 0 || val > 255)
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

        if(val < SHRT_MIN || val > SHRT_MAX)
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

        if(val < INT_MIN || val > INT_MAX)
        {
            return false;
        }
        break;
    }
    case PrimitiveInfo::KindLong:
    {
        //
        // The platform's 'long' type may not be 64 bits, so we also accept
        // a string argument for this type.
        //
        if(!PyInt_Check(p) && !PyLong_Check(p) && !PyString_Check(p))
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
        if(!PyString_Check(p))
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
            assert(false); // validate() should have caught this.
        }

        assert(val >= 0 && val <= 255); // validate() should have caught this.
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
            assert(false); // validate() should have caught this.
        }

        assert(val >= SHRT_MIN && val <= SHRT_MAX); // validate() should have caught this.
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
            assert(false); // validate() should have caught this.
        }

        assert(val >= INT_MIN && val <= INT_MAX); // validate() should have caught this.
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
            assert(false); // validate() should have caught this.
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
            assert(false); // validate() should have caught this.
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
            val = (const char*)PyString_AS_STRING(p);
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
        PyObjectHandle p;

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
            p = PyString_FromString(buf);
        }
        else
        {
            p = PyInt_FromLong(static_cast<long>(val));
        }

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
    PyObjectHandle val = PyObject_GetAttrString(p, "value");
    if(val.get() == NULL)
    {
        throw AbortMarshaling();
    }
    if(!PyInt_Check(val.get()))
    {
        PyErr_Format(PyExc_ValueError, "value for enum %s is not an int", id.c_str());
        throw AbortMarshaling();
    }
    int ival = static_cast<int>(PyInt_AsLong(val.get()));
    int count = static_cast<int>(enumerators.size());
    if(ival < 0 || ival >= count)
    {
        PyErr_Format(PyExc_ValueError, "value %d is out of range for enum %s", ival, id.c_str());
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
        PyErr_Format(PyExc_ValueError, "enumerator %d is out of range for enum %s", val, id.c_str());
        throw AbortMarshaling();
    }

    PyObject* pyval = enumerators[val].get();
    assert(pyval);
    cb->unmarshaled(pyval, target, closure);
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
            PyErr_Format(PyExc_AttributeError, "no member `%s' found in %s value", memberName,
                         const_cast<char*>(id.c_str()));
            throw AbortMarshaling();
        }
        if(!member->type->validate(attr.get()))
        {
            PyErr_Format(PyExc_AttributeError, "invalid value for %s member `%s'", const_cast<char*>(id.c_str()),
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
    return val == Py_None || PyList_Check(val) == 1 || PyTuple_Check(val) == 1;
}

void
IcePy::SequenceInfo::marshal(PyObject* p, const Ice::OutputStreamPtr& os, ObjectMap* objectMap)
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
            if(!elementType->validate(item))
            {
                PyErr_Format(PyExc_AttributeError, "invalid value for element %d of `%s'", i,
                             const_cast<char*>(id.c_str()));
                throw AbortMarshaling();
            }
            elementType->marshal(item, os, objectMap);
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
            if(!elementType->validate(item))
            {
                PyErr_Format(PyExc_AttributeError, "invalid value for element %d of `%s'", i,
                             const_cast<char*>(id.c_str()));
                throw AbortMarshaling();
            }
            elementType->marshal(item, os, objectMap);
        }
    }
    else if(p == Py_None)
    {
        os->writeSize(0);
    }
    else
    {
        assert(false); // validate() should have caught this.
    }
}

void
IcePy::SequenceInfo::unmarshal(const Ice::InputStreamPtr& is, const UnmarshalCallbackPtr& cb, PyObject* target,
                               void* closure)
{
    Ice::Int sz = is->readSize();
    PyObjectHandle result = PyList_New(sz);
    if(result.get() == NULL)
    {
        throw AbortMarshaling();
    }

    // TODO: Optimize for certain sequence types (e.g., bytes)?

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
IcePy::SequenceInfo::destroy()
{
    elementType->destroy();
    elementType = 0;
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
        if(!keyType->validate(key))
        {
            PyErr_Format(PyExc_AttributeError, "invalid key in `%s' element", const_cast<char*>(id.c_str()));
            throw AbortMarshaling();
        }
        keyType->marshal(key, os, objectMap);

        if(!valueType->validate(value))
        {
            PyErr_Format(PyExc_AttributeError, "invalid value in `%s' element", const_cast<char*>(id.c_str()));
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
IcePy::DictionaryInfo::KeyCallback::unmarshaled(PyObject* val, PyObject*, void*)
{
    key = val;
    Py_INCREF(val);
}

void
IcePy::DictionaryInfo::destroy()
{
    keyType->destroy();
    keyType = 0;
    valueType->destroy();
    valueType = 0;
}

//
// ParamInfo implementation.
//
void
IcePy::ParamInfo::unmarshaled(PyObject* val, PyObject* target, void* closure)
{
    assert(PyTuple_Check(target));
    int i = reinterpret_cast<int>(closure);
    PyTuple_SET_ITEM(target, i, val);
    Py_INCREF(val); // PyTuple_SET_ITEM steals a reference.
}

//
// OperationInfo implementation.
//
PyObject*
IcePy::OperationInfo::invoke(const Ice::ObjectPrx& proxy, const Ice::CommunicatorPtr& communicator, PyObject* args)
{
    assert(PyTuple_Check(args));

    //
    // Validate the number of arguments. There may be an extra argument for the context.
    //
    int argc = PyTuple_GET_SIZE(args);
    int paramCount = static_cast<int>(inParams.size());
    if(argc != paramCount && argc != paramCount + 1)
    {
        string fixedName = fixIdent(name);
        PyErr_Format(PyExc_RuntimeError, "%s expects %d in parameters", fixedName.c_str(), paramCount);
        return NULL;
    }

    //
    // Retrieve the context if any.
    //
    Ice::Context ctx;
    bool haveContext = false;
    if(argc == paramCount + 1)
    {
        PyObject* pyctx = PyTuple_GET_ITEM(args, argc - 1);
        if(pyctx != Py_None)
        {
            if(!PyDict_Check(pyctx))
            {
                PyErr_Format(PyExc_ValueError, "context argument must be a dictionary");
                return NULL;
            }

            if(!dictionaryToContext(pyctx, ctx))
            {
                return NULL;
            }

            haveContext = true;
        }
    }

    try
    {
        //
        // Marshal the in parameters.
        //
        Ice::ByteSeq params;
        Ice::OutputStreamPtr os = Ice::createOutputStream(communicator);

        ObjectMap objectMap;
        int i = 0;
        ParamInfoList::iterator p;
        for(p = inParams.begin(); p != inParams.end(); ++p, ++i)
        {
            PyObject* arg = PyTuple_GET_ITEM(args, i);
            if(!(*p)->type->validate(arg))
            {
                PyErr_Format(PyExc_AttributeError, "invalid value for argument %d in operation `%s'", i + 1,
                             const_cast<char*>(name.c_str()));
                return false;
            }
            (*p)->type->marshal(arg, os, &objectMap);
        }

        os->finished(params);

        //
        // Invoke the operation. Use _info->name here, not fixedName.
        //
        Ice::ByteSeq result;
        bool status;
        {
            AllowThreads allowThreads; // Release Python's global interpreter lock during remote invocations.

            if(haveContext)
            {
                status = proxy->ice_invoke(name, (Ice::OperationMode)mode, params, result, ctx);
            }
            else
            {
                status = proxy->ice_invoke(name, (Ice::OperationMode)mode, params, result);
            }
        }

        //
        // Process the reply.
        //
        if(proxy->ice_isTwoway())
        {
            if(!status)
            {
                //
                // Unmarshal and "throw" a user exception.
                //
                unmarshalException(result, communicator);
                return NULL;
            }
            else if(outParams.size() > 0 || returnType)
            {
                i = returnType ? 1 : 0;
                int numResults = static_cast<int>(outParams.size()) + i;
                PyObjectHandle results = PyTuple_New(numResults);
                if(results.get() == NULL)
                {
                    return NULL;
                }

                //
                // Unmarshal the results. If there is more than one value to be returned, then return them
                // in a tuple of the form (result, outParam1, ...). Otherwise just return the value.
                //
                Ice::InputStreamPtr is = Ice::createInputStream(communicator, result);
                for(p = outParams.begin(); p != outParams.end(); ++p, ++i)
                {
                    (*p)->type->unmarshal(is, *p, results.get(), (void*)i);
                }

                if(returnType)
                {
                    returnType->type->unmarshal(is, returnType, results.get(), 0);
                }

                is->finished();

                if(numResults > 1)
                {
                    return results.release();
                }
                else
                {
                    PyObject* ret = PyTuple_GET_ITEM(results.get(), 0);
                    Py_INCREF(ret);
                    return ret;
                }
            }
        }
    }
    catch(const AbortMarshaling&)
    {
        return NULL;
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

bool
IcePy::OperationInfo::dispatch(PyObject* servant, const std::vector<Ice::Byte>& inBytes,
                               std::vector<Ice::Byte>& outBytes, const Ice::Current& current)
{
    string fixedName = fixIdent(current.operation);
    Ice::CommunicatorPtr communicator = current.adapter->getCommunicator();

    //
    // Unmarshal the in parameters.
    //
    int count = static_cast<int>(inParams.size());
    PyObjectHandle args = PyTuple_New(count + 1); // Leave room for a trailing Ice::Current object.
    if(args.get() == NULL)
    {
        throwPythonException();
    }

    Ice::InputStreamPtr is = Ice::createInputStream(communicator, inBytes);

    try
    {
        int i = 0;
        for(ParamInfoList::iterator p = inParams.begin(); p != inParams.end(); ++p, ++i)
        {
            (*p)->type->unmarshal(is, *p, args.get(), (void*)i);
        }
        is->finished();
    }
    catch(const AbortMarshaling&)
    {
        throwPythonException();
    }

    //
    // Create an object to represent Ice::Current. We need to append this to the argument tuple.
    //
    PyObjectHandle curr = createCurrent(current);
    if(PyTuple_SET_ITEM(args.get(), PyTuple_GET_SIZE(args.get()) - 1, curr.get()) < 0)
    {
        throwPythonException();
    }
    curr.release(); // PyTuple_SET_ITEM steals a reference.

    //
    // Dispatch the operation. Use fixedName here, not current.operation.
    //
    PyObjectHandle method = PyObject_GetAttrString(servant, const_cast<char*>(fixedName.c_str()));
    if(method.get() == NULL)
    {
        ostringstream ostr;
        ostr << "servant for identity " << Ice::identityToString(current.id) << " does not define operation `"
             << fixedName << "'";
        string str = ostr.str();
        PyErr_Warn(PyExc_RuntimeWarning, const_cast<char*>(str.c_str()));
        Ice::UnknownException ex(__FILE__, __LINE__);
        ex.unknown = str;
        throw ex;
    }

    PyObjectHandle result = PyObject_Call(method.get(), args.get(), NULL);

    //
    // Check for exceptions.
    //
    if(checkDispatchException(outBytes, communicator))
    {
        return false;
    }

    //
    // Marshal the results. If there is more than one value to be returned, then they must be
    // returned in a tuple of the form (result, outParam1, ...).
    //
    Ice::OutputStreamPtr os = Ice::createOutputStream(communicator);
    try
    {
        int i = returnType ? 1 : 0;
        int numResults = static_cast<int>(outParams.size()) + i;
        if(numResults > 1)
        {
            if(!PyTuple_Check(result.get()) || PyTuple_GET_SIZE(result.get()) != numResults)
            {
                ostringstream ostr;
                ostr << "operation `" << fixIdent(name) << "' should return a tuple of length " << numResults;
                string str = ostr.str();
                PyErr_Warn(PyExc_RuntimeWarning, const_cast<char*>(str.c_str()));
                throw Ice::MarshalException(__FILE__, __LINE__);
            }
        }

        ObjectMap objectMap;

        for(ParamInfoList::iterator p = outParams.begin(); p != outParams.end(); ++p, ++i)
        {
            PyObject* arg;
            if(numResults > 1)
            {
                arg = PyTuple_GET_ITEM(result.get(), i);
            }
            else
            {
                arg = result.get();
                assert(outParams.size() == 1);
            }

            if(!(*p)->type->validate(arg))
            {
                // TODO: Provide the parameter name instead
                PyErr_Format(PyExc_AttributeError, "invalid value for out argument %d in operation `%s'", i + 1,
                             const_cast<char*>(name.c_str()));
                return false;
            }
            (*p)->type->marshal(arg, os, &objectMap);
        }

        if(returnType)
        {
            PyObject* res;
            if(numResults > 1)
            {
                res = PyTuple_GET_ITEM(result.get(), 0);
            }
            else
            {
                assert(outParams.size() == 0);
                res = result.get();
            }
            if(!returnType->type->validate(res))
            {
                PyErr_Format(PyExc_AttributeError, "invalid return value for operation `%s'",
                             const_cast<char*>(name.c_str()));
                return false;
            }
            returnType->type->marshal(res, os, &objectMap);
        }

        os->finished(outBytes);
    }
    catch(const AbortMarshaling&)
    {
        throwPythonException();
    }

    return true;
}

bool
IcePy::OperationInfo::checkDispatchException(std::vector<Ice::Byte>& bytes, const Ice::CommunicatorPtr& communicator)
{
    //
    // Check for exceptions. Return true if we marshaled a user exception, or false if no
    // exception was set. Local exceptions may be thrown.
    //
    PyObject* exType = PyErr_Occurred();
    if(exType)
    {
        //
        // A servant that calls sys.exit() will raise the SystemExit exception.
        // This is normally caught by the interpreter, causing it to exit.
        // However, we have no way to pass this exception to the interpreter,
        // so we act on it directly.
        //
        if(PyErr_GivenExceptionMatches(exType, PyExc_SystemExit))
        {
            handleSystemExit(); // Does not return.
        }

        PyObjectHandle ex = getPythonException(); // Retrieve it before another Python API call clears it.

        PyObject* userExceptionType = lookupType("Ice.UserException");

        if(PyErr_GivenExceptionMatches(exType, userExceptionType))
        {
            //
            // Get the exception's id and Verify that it is legal to be thrown from this operation.
            //
            PyObjectHandle id = PyObject_CallMethod(ex.get(), "ice_id", NULL);
            PyErr_Clear();
            if(id.get() == NULL || !validateException(ex.get()))
            {
                throwPythonException(ex.get()); // Raises UnknownUserException.
            }
            else
            {
                assert(PyString_Check(id.get()));
                char* str = PyString_AS_STRING(id.get());
                ExceptionInfoPtr info = getExceptionInfo(str);
                if(!info)
                {
                    Ice::UnknownUserException e(__FILE__, __LINE__);
                    e.unknown = str;
                    throw e;
                }

                Ice::OutputStreamPtr os = Ice::createOutputStream(communicator);
                ObjectMap objectMap;
                info->marshal(ex.get(), os, &objectMap);
                os->finished(bytes);
                return true;
            }
        }
        else
        {
            throwPythonException(ex.get());
        }
    }

    return false;
}

void
IcePy::OperationInfo::unmarshalException(const std::vector<Ice::Byte>& bytes, const Ice::CommunicatorPtr& communicator)
{
    Ice::InputStreamPtr is = Ice::createInputStream(communicator, bytes);

    is->readBool(); // usesClasses

    string id = is->readString();
    while(!id.empty())
    {
        ExceptionInfoPtr info = getExceptionInfo(id);
        if(info)
        {
            PyObjectHandle ex = info->unmarshal(is);
            is->finished();

            if(validateException(ex.get()))
            {
                //
                // Set the Python exception.
                //
                assert(PyInstance_Check(ex.get()));
                PyObject* type = (PyObject*)((PyInstanceObject*)ex.get())->in_class;
                Py_INCREF(type);
                PyErr_Restore(type, ex.release(), NULL);
            }
            else
            {
                throwPythonException(ex.get());
            }

            return;
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
    // have factory for. This means that sender and receiver disagree
    // about the Slice definitions they use.
    //
    throw Ice::UnknownUserException(__FILE__, __LINE__);
}

bool
IcePy::OperationInfo::validateException(PyObject* ex) const
{
    for(ExceptionInfoList::const_iterator p = exceptions.begin(); p != exceptions.end(); ++p)
    {
        if(PyObject_IsInstance(ex, (*p)->pythonType.get()))
        {
            return true;
        }
    }

    return false;
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

void
IcePy::ClassInfo::marshal(PyObject* p, const Ice::OutputStreamPtr& os, ObjectMap* objectMap)
{
    if(pythonType.get() == NULL)
    {
        PyErr_Format(PyExc_RuntimeError, "class %s is declared but not defined", id.c_str());
        throw AbortMarshaling();
    }

    if(p == Py_None)
    {
        os->writeObject(0);
        return;
    }

    if(!PyObject_IsInstance(p, pythonType.get()))
    {
        PyErr_Format(PyExc_ValueError, "expected value of type %s", id.c_str());
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

void
IcePy::ClassInfo::unmarshal(const Ice::InputStreamPtr& is, const UnmarshalCallbackPtr& cb, PyObject* target,
                            void* closure)
{
    if(pythonType.get() == NULL)
    {
        PyErr_Format(PyExc_RuntimeError, "class %s is declared but not defined", id.c_str());
        throw AbortMarshaling();
    }

    is->readObject(new ReadObjectCallback(this, cb, target, closure));
}

void
IcePy::ClassInfo::destroy()
{
    base = 0;
    interfaces.clear();
    for(DataMemberList::iterator p = members.begin(); p != members.end(); ++p)
    {
        (*p)->type->destroy();
    }
    members.clear();
    operations.clear();
}

IcePy::OperationInfoPtr
IcePy::ClassInfo::findOperation(const string& name) const
{
    OperationInfoMap::const_iterator p = operations.find(name);
    if(p != operations.end())
    {
        return p->second;
    }

    if(base)
    {
        OperationInfoPtr op = base->findOperation(name);
        if(op)
        {
            return op;
        }
    }

    for(ClassInfoList::const_iterator q = interfaces.begin(); q != interfaces.end(); ++q)
    {
        OperationInfoPtr op = (*q)->findOperation(name);
        if(op)
        {
            return op;
        }
    }

    return 0;
}

bool
IcePy::ClassInfo::hasOperations() const
{
    if(!operations.empty())
    {
        return true;
    }

    if(base && base->hasOperations())
    {
        return true;
    }

    for(ClassInfoList::const_iterator q = interfaces.begin(); q != interfaces.end(); ++q)
    {
        if((*q)->hasOperations())
        {
            return true;
        }
    }

    return false;
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
        PyErr_Format(PyExc_RuntimeError, "class %s is declared but not defined", id.c_str());
        return;
    }

    PyObjectHandle p = createProxy(proxy, is->communicator(), pythonType.get());
    cb->unmarshaled(p.get(), target, closure);
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
                PyErr_Format(PyExc_AttributeError, "no member `%s' found in %s value", memberName,
                             const_cast<char*>(_info->id.c_str()));
                throw AbortMarshaling();
            }

            if(!member->type->validate(val.get()))
            {
                PyErr_Format(PyExc_AttributeError, "invalid value for %s member `%s'",
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
        PyErr_Format(PyExc_ValueError, "expected exception %s", id.c_str());
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
                PyErr_Format(PyExc_AttributeError, "no member `%s' found in %s value", memberName,
                             const_cast<char*>(id.c_str()));
                throw AbortMarshaling();
            }

            if(!member->type->validate(val.get()))
            {
                PyErr_Format(PyExc_AttributeError, "invalid value for %s member `%s'", const_cast<char*>(id.c_str()),
                             memberName);
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

//
// getTypeInfo()
//
IcePy::TypeInfoPtr
IcePy::getTypeInfo(const string& id)
{
    TypeInfoMap::iterator p = _typeInfoMap.find(id);
    if(p != _typeInfoMap.end())
    {
        return p->second;
    }
    return 0;
}

//
// getExceptionInfo()
//
IcePy::ExceptionInfoPtr
IcePy::getExceptionInfo(const string& id)
{
    ExceptionInfoMap::iterator p = _exceptionInfoMap.find(id);
    if(p != _exceptionInfoMap.end())
    {
        return p->second;
    }
    return 0;
}

static PrimitiveInfoPtr _boolType;
static PrimitiveInfoPtr _byteType;
static PrimitiveInfoPtr _shortType;
static PrimitiveInfoPtr _intType;
static PrimitiveInfoPtr _longType;
static PrimitiveInfoPtr _floatType;
static PrimitiveInfoPtr _doubleType;
static PrimitiveInfoPtr _stringType;

bool
IcePy::initTypes(PyObject* module)
{
    _boolType = new PrimitiveInfo;
    _boolType->kind = PrimitiveInfo::KindBool;
    _byteType = new PrimitiveInfo;
    _byteType->kind = PrimitiveInfo::KindByte;
    _shortType = new PrimitiveInfo;
    _shortType->kind = PrimitiveInfo::KindShort;
    _intType = new PrimitiveInfo;
    _intType->kind = PrimitiveInfo::KindInt;
    _longType = new PrimitiveInfo;
    _longType->kind = PrimitiveInfo::KindLong;
    _floatType = new PrimitiveInfo;
    _floatType->kind = PrimitiveInfo::KindFloat;
    _doubleType = new PrimitiveInfo;
    _doubleType->kind = PrimitiveInfo::KindDouble;
    _stringType = new PrimitiveInfo;
    _stringType->kind = PrimitiveInfo::KindString;

    if(!PyModule_AddIntConstant(module, "T_BOOL", TYPE_BOOL) < 0)
    {
        return false;
    }
    if(!PyModule_AddIntConstant(module, "T_BYTE", TYPE_BYTE) < 0)
    {
        return false;
    }
    if(!PyModule_AddIntConstant(module, "T_SHORT", TYPE_SHORT) < 0)
    {
        return false;
    }
    if(!PyModule_AddIntConstant(module, "T_INT", TYPE_INT) < 0)
    {
        return false;
    }
    if(!PyModule_AddIntConstant(module, "T_LONG", TYPE_LONG) < 0)
    {
        return false;
    }
    if(!PyModule_AddIntConstant(module, "T_FLOAT", TYPE_FLOAT) < 0)
    {
        return false;
    }
    if(!PyModule_AddIntConstant(module, "T_DOUBLE", TYPE_DOUBLE) < 0)
    {
        return false;
    }
    if(!PyModule_AddIntConstant(module, "T_STRING", TYPE_STRING) < 0)
    {
        return false;
    }
    if(!PyModule_AddIntConstant(module, "T_OBJECT", TYPE_OBJECT) < 0)
    {
        return false;
    }
    if(!PyModule_AddIntConstant(module, "T_OBJECT_PROXY", TYPE_OBJECT_PROXY) < 0)
    {
        return false;
    }
    if(!PyModule_AddIntConstant(module, "OP_NORMAL", OP_NORMAL) < 0)
    {
        return false;
    }
    if(!PyModule_AddIntConstant(module, "OP_NONMUTATING", OP_NONMUTATING) < 0)
    {
        return false;
    }
    if(!PyModule_AddIntConstant(module, "OP_IDEMPOTENT", OP_IDEMPOTENT) < 0)
    {
        return false;
    }
    return true;
}

static IcePy::TypeInfoPtr
convertType(PyObject* obj)
{
    if(PyInt_Check(obj))
    {
        int i = static_cast<int>(PyInt_AS_LONG(obj));
        switch(i)
        {
        case TYPE_BOOL:
            return _boolType;
        case TYPE_BYTE:
            return _byteType;
        case TYPE_SHORT:
            return _shortType;
        case TYPE_INT:
            return _intType;
        case TYPE_LONG:
            return _longType;
        case TYPE_FLOAT:
            return _floatType;
        case TYPE_DOUBLE:
            return _doubleType;
        case TYPE_STRING:
            return _stringType;
        case TYPE_OBJECT:
            return getTypeInfo("::Ice::Object");
        case TYPE_OBJECT_PROXY:
            return getTypeInfo("::Ice::ObjectPrx");
        }
    }
    else
    {
        assert(PyString_Check(obj));
        char* id = PyString_AS_STRING(obj);
        return getTypeInfo(id);
    }

    assert(false);
    return 0;
}

extern "C"
PyObject*
Ice_defineEnum(PyObject*, PyObject* args)
{
    char* id;
    PyObject* type;
    PyObject* enumerators;
    if(!PyArg_ParseTuple(args, "sOO", &id, &type, &enumerators))
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

    addTypeInfo(id, info);

    Py_INCREF(Py_None);
    return Py_None;
}

extern "C"
PyObject*
Ice_defineStruct(PyObject*, PyObject* args)
{
    char* id;
    PyObject* type;
    PyObject* members;
    if(!PyArg_ParseTuple(args, "sOO", &id, &type, &members))
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
        member->type = convertType(t);
        info->members.push_back(member);
    }

    addTypeInfo(id, info);

    Py_INCREF(Py_None);
    return Py_None;
}

extern "C"
PyObject*
Ice_defineSequence(PyObject*, PyObject* args)
{
    char* id;
    PyObject* elementType;
    if(!PyArg_ParseTuple(args, "sO", &id, &elementType))
    {
        return NULL;
    }

    SequenceInfoPtr info = new SequenceInfo;
    info->id = id;
    info->elementType = convertType(elementType);

    addTypeInfo(id, info);

    Py_INCREF(Py_None);
    return Py_None;
}

extern "C"
PyObject*
Ice_defineDictionary(PyObject*, PyObject* args)
{
    char* id;
    PyObject* keyType;
    PyObject* valueType;
    if(!PyArg_ParseTuple(args, "sOO", &id, &keyType, &valueType))
    {
        return NULL;
    }

    DictionaryInfoPtr info = new DictionaryInfo;
    info->id = id;
    info->keyType = convertType(keyType);
    info->valueType = convertType(valueType);

    addTypeInfo(id, info);

    Py_INCREF(Py_None);
    return Py_None;
}

extern "C"
PyObject*
Ice_declareProxy(PyObject*, PyObject* args)
{
    char* id;
    if(!PyArg_ParseTuple(args, "s", &id))
    {
        return NULL;
    }

    string proxyId = id;
    proxyId += "Prx";

    TypeInfoPtr ti = getTypeInfo(proxyId);
    if(!ti)
    {
        ProxyInfoPtr info = new ProxyInfo;
        info->id = proxyId;
        info->_class = ClassInfoPtr::dynamicCast(getTypeInfo(id));
        assert(info->_class);
        addTypeInfo(proxyId, info);
    }
    else
    {
        assert(ProxyInfoPtr::dynamicCast(ti));
    }

    Py_INCREF(Py_None);
    return Py_None;
}

extern "C"
PyObject*
Ice_defineProxy(PyObject*, PyObject* args)
{
    char* id;
    PyObject* type;
    if(!PyArg_ParseTuple(args, "sO", &id, &type))
    {
        return NULL;
    }

    assert(PyType_Check(type));

    string proxyId = id;
    proxyId += "Prx";

    ProxyInfoPtr info = ProxyInfoPtr::dynamicCast(getTypeInfo(proxyId));
    if(!info)
    {
        info = new ProxyInfo;
        info->id = proxyId;
        addTypeInfo(proxyId, info);
    }

    info->pythonType = type;
    Py_INCREF(type);

    Py_INCREF(Py_None);
    return Py_None;
}

extern "C"
PyObject*
Ice_declareClass(PyObject*, PyObject* args)
{
    char* id;
    if(!PyArg_ParseTuple(args, "s", &id))
    {
        return NULL;
    }

    TypeInfoPtr ti = getTypeInfo(id);
    if(!ti)
    {
        ClassInfoPtr info = new ClassInfo;
        info->id = id;
        addTypeInfo(id, info);
    }
    else
    {
        assert(ClassInfoPtr::dynamicCast(ti));
    }

    Py_INCREF(Py_None);
    return Py_None;
}

extern "C"
PyObject*
Ice_defineClass(PyObject*, PyObject* args)
{
    char* id;
    PyObject* type;
    int isInterface;
    char* baseId;
    PyObject* interfaces;
    PyObject* members;
    PyObject* operations;
    if(!PyArg_ParseTuple(args, "sOisOOO", &id, &type, &isInterface, &baseId, &interfaces, &members, &operations))
    {
        return NULL;
    }

    assert(PyType_Check(type));
    assert(PyTuple_Check(interfaces));
    assert(PyTuple_Check(members));
    assert(PyDict_Check(operations));

    ClassInfoPtr info = ClassInfoPtr::dynamicCast(getTypeInfo(id));
    if(!info)
    {
        info = new ClassInfo;
        info->id = id;
        addTypeInfo(id, info);
    }

    info->isInterface = isInterface ? true : false;
    if(strlen(baseId) > 0)
    {
        TypeInfoPtr t = getTypeInfo(baseId);
        assert(t);
        info->base = ClassInfoPtr::dynamicCast(t);
        assert(info->base);
    }
    info->pythonType = type;
    Py_INCREF(type);

    int i, sz;
    sz = PyTuple_GET_SIZE(interfaces);
    for(i = 0; i < sz; ++i)
    {
        PyObject* o = PyTuple_GET_ITEM(interfaces, i);
        assert(PyString_Check(o));
        info->interfaces.push_back(ClassInfoPtr::dynamicCast(convertType(o)));
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
        member->type = convertType(t);
        info->members.push_back(member);
    }

    int pos = 0;
    PyObject* key;
    PyObject* value;
    while(PyDict_Next(operations, &pos, &key, &value))
    {
        assert(PyString_Check(key));
        assert(PyTuple_Check(value));
        assert(PyTuple_GET_SIZE(value) == 5);

        PyObject* item;

        OperationInfoPtr op = new OperationInfo;
        op->name = PyString_AS_STRING(key);

        //
        // mode
        //
        item = PyTuple_GET_ITEM(value, 0);
        assert(PyInt_Check(item));
        assert(PyInt_AS_LONG(item) >= OP_NORMAL && PyInt_AS_LONG(item) <= OP_IDEMPOTENT);
        op->mode = (Ice::OperationMode)static_cast<int>(PyInt_AS_LONG(item));

        //
        // inParams
        //
        item = PyTuple_GET_ITEM(value, 1);
        assert(PyTuple_Check(item));
        sz = PyTuple_GET_SIZE(item);
        for(i = 0; i < sz; ++i)
        {
            ParamInfoPtr param = new ParamInfo;
            param->type = convertType(PyTuple_GET_ITEM(item, i));
            assert(param->type);
            op->inParams.push_back(param);
        }

        //
        // outParams
        //
        item = PyTuple_GET_ITEM(value, 2);
        assert(PyTuple_Check(item));
        sz = PyTuple_GET_SIZE(item);
        for(i = 0; i < sz; ++i)
        {
            ParamInfoPtr param = new ParamInfo;
            param->type = convertType(PyTuple_GET_ITEM(item, i));
            assert(param->type);
            op->outParams.push_back(param);
        }

        //
        // returnType
        //
        item = PyTuple_GET_ITEM(value, 3);
        if(item != Py_None)
        {
            op->returnType = new ParamInfo;
            op->returnType->type = convertType(item);
        }

        //
        // exceptions
        //
        item = PyTuple_GET_ITEM(value, 4);
        assert(PyTuple_Check(item));
        sz = PyTuple_GET_SIZE(item);
        for(i = 0; i < sz; ++i)
        {
            PyObject* s = PyTuple_GET_ITEM(item, i);
            assert(PyString_Check(s));
            op->exceptions.push_back(getExceptionInfo(PyString_AS_STRING(s)));
        }

        info->operations.insert(OperationInfoMap::value_type(op->name, op));
    }

    Py_INCREF(Py_None);
    return Py_None;
}

extern "C"
PyObject*
Ice_defineException(PyObject*, PyObject* args)
{
    char* id;
    PyObject* type;
    char* baseId;
    PyObject* members;
    int usesClasses;
    if(!PyArg_ParseTuple(args, "sOsOi", &id, &type, &baseId, &members, &usesClasses))
    {
        return NULL;
    }

    assert(PyClass_Check(type));
    assert(PyTuple_Check(members));

    ExceptionInfoPtr info = new ExceptionInfo;
    info->id = id;
    if(strlen(baseId) > 0)
    {
        info->base = getExceptionInfo(baseId);
        assert(info->base);
    }
    info->usesClasses = usesClasses ? true : false;
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
        member->type = convertType(t);
        info->members.push_back(member);
    }

    addExceptionInfo(id, info);

    Py_INCREF(Py_None);
    return Py_None;
}
