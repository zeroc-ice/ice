// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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
#include <Thread.h>
#include <Util.h>
#include <IceUtil/InputUtil.h>
#include <IceUtil/ScopedArray.h>
#include <Ice/LocalException.h>
#include <Ice/SlicedData.h>

#include <list>
#include <limits>

using namespace std;
using namespace IcePy;
using namespace IceUtil;
using namespace IceUtilInternal;

typedef map<string, ClassInfoPtr> ClassInfoMap;
static ClassInfoMap _classInfoMap;

typedef map<Ice::Int, ClassInfoPtr> CompactIdMap;
static CompactIdMap _compactIdMap;

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

bool
writeString(PyObject* p, const Ice::OutputStreamPtr& os)
{
    if(p == Py_None)
    {
        os->write(string());
    }
    else if(checkString(p))
    {
        os->write(getString(p));
    }
#if defined(Py_USING_UNICODE) && PY_VERSION_HEX < 0x03000000
    else if(PyUnicode_Check(p))
    {
        //
        // Convert a Unicode object to a UTF-8 string and write it without manipulation from
        // a C++ string converter.
        //
        PyObjectHandle h = PyUnicode_AsUTF8String(p);
        if(!h.get())
        {
            return false;
        }
        os->write(getString(h.get()), false);
    }
#endif
    else
    {
        assert(false);
    }

    return true;
}

}

#ifdef WIN32
extern "C"
#endif
static TypeInfoObject*
typeInfoNew(PyTypeObject* type, PyObject* /*args*/, PyObject* /*kwds*/)
{
    TypeInfoObject* self = reinterpret_cast<TypeInfoObject*>(type->tp_alloc(type, 0));
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
    Py_TYPE(self)->tp_free(reinterpret_cast<PyObject*>(self));
}

#ifdef WIN32
extern "C"
#endif
static ExceptionInfoObject*
exceptionInfoNew(PyTypeObject* type, PyObject* /*args*/, PyObject* /*kwds*/)
{
    ExceptionInfoObject* self = reinterpret_cast<ExceptionInfoObject*>(type->tp_alloc(type, 0));
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
    Py_TYPE(self)->tp_free(reinterpret_cast<PyObject*>(self));
}

#ifdef WIN32
extern "C"
#endif
static void
unsetDealloc(PyTypeObject* /*self*/)
{
    Py_FatalError("deallocating Unset");
}

#ifdef WIN32
extern "C"
#endif
static int
unsetNonzero(PyObject* /*v*/)
{
    //
    // We define tp_as_number->nb_nonzero so that the Unset marker value evaluates as "zero" or "false".
    //
    return 0;
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
unsetRepr(PyObject* /*v*/)
{
#if PY_VERSION_HEX >= 0x03000000
    return PyBytes_FromString("Unset");
#else
    return PyString_FromString("Unset");
#endif
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
// SlicedDataUtil implementation
//
PyObject* IcePy::SlicedDataUtil::_slicedDataType = 0;
PyObject* IcePy::SlicedDataUtil::_sliceInfoType = 0;

IcePy::SlicedDataUtil::SlicedDataUtil()
{
}

IcePy::SlicedDataUtil::~SlicedDataUtil()
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
            // Don't just call (*q)->objects.clear(), as releasing references
            // to the objects could have unexpected side effects. We exchange
            // the vector into a temporary and then let the temporary fall out
            // of scope.
            //
            vector<Ice::ObjectPtr> tmp;
            tmp.swap((*q)->objects);
        }
    }
}

void
IcePy::SlicedDataUtil::add(const ObjectReaderPtr& reader)
{
    assert(reader->getSlicedData());
    _readers.insert(reader);
}

void
IcePy::SlicedDataUtil::update()
{
    for(set<ObjectReaderPtr>::iterator p = _readers.begin(); p != _readers.end(); ++p)
    {
        setMember((*p)->getObject(), (*p)->getSlicedData());
    }
}

void
IcePy::SlicedDataUtil::setMember(PyObject* obj, const Ice::SlicedDataPtr& slicedData)
{
    //
    // Create a Python equivalent of the SlicedData object.
    //

    assert(slicedData);

    if(!_slicedDataType)
    {
        _slicedDataType = lookupType("Ice.SlicedData");
        assert(_slicedDataType);
    }
    if(!_sliceInfoType)
    {
        _sliceInfoType = lookupType("Ice.SliceInfo");
        assert(_sliceInfoType);
    }

    IcePy::PyObjectHandle args = PyTuple_New(0);
    if(!args.get())
    {
        assert(PyErr_Occurred());
        throw AbortMarshaling();
    }

    PyObjectHandle sd = PyEval_CallObject(_slicedDataType, args.get());
    if(!sd.get())
    {
        assert(PyErr_Occurred());
        throw AbortMarshaling();
    }

    PyObjectHandle slices = PyTuple_New(slicedData->slices.size());
    if(!slices.get())
    {
        assert(PyErr_Occurred());
        throw AbortMarshaling();
    }

    if(PyObject_SetAttrString(sd.get(), STRCAST("slices"), slices.get()) < 0)
    {
        assert(PyErr_Occurred());
        throw AbortMarshaling();
    }

    //
    // Translate each SliceInfo object into its Python equivalent.
    //
    int i = 0;
    for(vector<Ice::SliceInfoPtr>::const_iterator p = slicedData->slices.begin(); p != slicedData->slices.end(); ++p)
    {
        PyObjectHandle slice = PyEval_CallObject(_sliceInfoType, args.get());
        if(!slice.get())
        {
            assert(PyErr_Occurred());
            throw AbortMarshaling();
        }

        PyTuple_SET_ITEM(slices.get(), i++, slice.get());
        Py_INCREF(slice.get()); // PyTuple_SET_ITEM steals a reference.

        //
        // typeId
        //
        PyObjectHandle typeId = createString((*p)->typeId);
        if(!typeId.get() || PyObject_SetAttrString(slice.get(), STRCAST("typeId"), typeId.get()) < 0)
        {
            assert(PyErr_Occurred());
            throw AbortMarshaling();
        }

        //
        // compactId
        //
        PyObjectHandle compactId = PyLong_FromLong((*p)->compactId);
        if(!compactId.get() || PyObject_SetAttrString(slice.get(), STRCAST("compactId"), compactId.get()) < 0)
        {
            assert(PyErr_Occurred());
            throw AbortMarshaling();
        }

        //
        // bytes
        //
        PyObjectHandle bytes;
        if((*p)->bytes.size() > 0)
        {
    #if PY_VERSION_HEX >= 0x03000000
            bytes = PyBytes_FromStringAndSize(reinterpret_cast<const char*>(&(*p)->bytes[0]), (*p)->bytes.size());
    #else
            bytes = PyString_FromStringAndSize(reinterpret_cast<const char*>(&(*p)->bytes[0]), (*p)->bytes.size());
    #endif
        }
        else
        {
    #if PY_VERSION_HEX >= 0x03000000
            bytes = PyBytes_FromStringAndSize(0, 0);
    #else
            bytes = PyString_FromStringAndSize(0, 0);
    #endif
        }
        if(!bytes.get() || PyObject_SetAttrString(slice.get(), STRCAST("bytes"), bytes.get()) < 0)
        {
            assert(PyErr_Occurred());
            throw AbortMarshaling();
        }

        //
        // objects
        //
        PyObjectHandle objects = PyTuple_New((*p)->objects.size());
        if(!objects.get() || PyObject_SetAttrString(slice.get(), STRCAST("objects"), objects.get()) < 0)
        {
            assert(PyErr_Occurred());
            throw AbortMarshaling();
        }

        int j = 0;
        for(vector<Ice::ObjectPtr>::iterator q = (*p)->objects.begin(); q != (*p)->objects.end(); ++q)
        {
            //
            // Each element in the objects list is an instance of ObjectReader that wraps a Python object.
            //
            assert(*q);
            ObjectReaderPtr r = ObjectReaderPtr::dynamicCast(*q);
            assert(r);
            PyObject* obj = r->getObject();
            assert(obj != Py_None); // Should be non-nil.
            PyTuple_SET_ITEM(objects.get(), j++, obj);
            Py_INCREF(obj); // PyTuple_SET_ITEM steals a reference.
        }

        //
        // hasOptionalMembers
        //
        PyObject* hasOptionalMembers = (*p)->hasOptionalMembers ? getTrue() : getFalse();
        if(PyObject_SetAttrString(slice.get(), STRCAST("hasOptionalMembers"), hasOptionalMembers) < 0)
        {
            assert(PyErr_Occurred());
            throw AbortMarshaling();
        }

        //
        // isLastSlice
        //
        PyObject* isLastSlice = (*p)->isLastSlice ? getTrue() : getFalse();
        if(PyObject_SetAttrString(slice.get(), STRCAST("isLastSlice"), isLastSlice) < 0)
        {
            assert(PyErr_Occurred());
            throw AbortMarshaling();
        }
    }

    if(PyObject_SetAttrString(obj, STRCAST("_ice_slicedData"), sd.get()) < 0)
    {
        assert(PyErr_Occurred());
        throw AbortMarshaling();
    }
}

//
// Instances of preserved class and exception types may have a data member
// named _ice_slicedData which is an instance of the Python class Ice.SlicedData.
//
Ice::SlicedDataPtr
IcePy::SlicedDataUtil::getMember(PyObject* obj, ObjectMap* objectMap)
{
    Ice::SlicedDataPtr slicedData;

    if(PyObject_HasAttrString(obj, STRCAST("_ice_slicedData")))
    {
        PyObjectHandle sd = PyObject_GetAttrString(obj, STRCAST("_ice_slicedData"));
        assert(sd.get());

        if(sd.get() != Py_None)
        {
            //
            // The "slices" member is a tuple of Ice.SliceInfo objects.
            //
            PyObjectHandle sl = PyObject_GetAttrString(sd.get(), STRCAST("slices"));
            assert(sl.get());
            assert(PyTuple_Check(sl.get()));

            Ice::SliceInfoSeq slices;

            Py_ssize_t sz = PyTuple_GET_SIZE(sl.get());
            for(Py_ssize_t i = 0; i < sz; ++i)
            {
                PyObjectHandle s = PyTuple_GET_ITEM(sl.get(), i);
                Py_INCREF(s.get());

                Ice::SliceInfoPtr info = new Ice::SliceInfo;

                PyObjectHandle typeId = PyObject_GetAttrString(s.get(), STRCAST("typeId"));
                assert(typeId.get());
                info->typeId = getString(typeId.get());

                PyObjectHandle compactId = PyObject_GetAttrString(s.get(), STRCAST("compactId"));
                assert(compactId.get());
                info->compactId = static_cast<int>(PyLong_AsLong(compactId.get()));

                PyObjectHandle bytes = PyObject_GetAttrString(s.get(), STRCAST("bytes"));
                assert(bytes.get());
                char* str;
                Py_ssize_t strsz;
#if PY_VERSION_HEX >= 0x03000000
                assert(PyBytes_Check(bytes.get()));
                PyBytes_AsStringAndSize(bytes.get(), &str, &strsz);
#else
                assert(PyString_Check(bytes.get()));
                PyString_AsStringAndSize(bytes.get(), &str, &strsz);
#endif
                vector<Ice::Byte> vtmp(reinterpret_cast<Ice::Byte*>(str), reinterpret_cast<Ice::Byte*>(str + strsz));
                info->bytes.swap(vtmp);

                PyObjectHandle objects = PyObject_GetAttrString(s.get(), STRCAST("objects"));
                assert(objects.get());
                assert(PyTuple_Check(objects.get()));
                Py_ssize_t osz = PyTuple_GET_SIZE(objects.get());
                for(Py_ssize_t j = 0; j < osz; ++j)
                {
                    PyObject* o = PyTuple_GET_ITEM(objects.get(), j);

                    Ice::ObjectPtr writer;

                    ObjectMap::iterator i = objectMap->find(o);
                    if(i == objectMap->end())
                    {
                        writer = new ObjectWriter(o, objectMap);
                        objectMap->insert(ObjectMap::value_type(o, writer));
                    }
                    else
                    {
                        writer = i->second;
                    }

                    info->objects.push_back(writer);
                }

                PyObjectHandle hasOptionalMembers = PyObject_GetAttrString(s.get(), STRCAST("hasOptionalMembers"));
                assert(hasOptionalMembers.get());
                info->hasOptionalMembers = PyObject_IsTrue(hasOptionalMembers.get()) ? true : false;

                PyObjectHandle isLastSlice = PyObject_GetAttrString(s.get(), STRCAST("isLastSlice"));
                assert(isLastSlice.get());
                info->isLastSlice = PyObject_IsTrue(isLastSlice.get()) ? true : false;

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
IcePy::TypeInfo::usesClasses() const
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
IcePy::PrimitiveInfo::PrimitiveInfo(Kind k) :
    kind(k)
{
}

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
        long val = PyLong_AsLong(p);

        if(PyErr_Occurred() || val < 0 || val > 255)
        {
            return false;
        }
        break;
    }
    case PrimitiveInfo::KindShort:
    {
        long val = PyLong_AsLong(p);

        if(PyErr_Occurred() || val < SHRT_MIN || val > SHRT_MAX)
        {
            return false;
        }
        break;
    }
    case PrimitiveInfo::KindInt:
    {
        long val = PyLong_AsLong(p);

        if(PyErr_Occurred() || val < INT_MIN || val > INT_MAX)
        {
            return false;
        }
        break;
    }
    case PrimitiveInfo::KindLong:
    {
        PyLong_AsLongLong(p); // Just to see if it raises an error.

        if(PyErr_Occurred())
        {
            return false;
        }

        break;
    }
    case PrimitiveInfo::KindFloat:
    {
        if(!PyFloat_Check(p))
        {
            if(PyLong_Check(p))
            {
                PyLong_AsDouble(p); // Just to see if it raises an error.
                if(PyErr_Occurred())
                {
                    return false;
                }
            }
#if PY_VERSION_HEX < 0x03000000
            else if(PyInt_Check(p))
            {
                return true;
            }
#endif
            else
            {
                return false;
            }
        }
        else
        {
            // Ensure double does not exceed maximum float value before casting
            double val = PyFloat_AsDouble(p);
            return (val <= numeric_limits<float>::max() && val >= -numeric_limits<float>::max()) ||
#if defined(_MSC_VER) && (_MSC_VER <= 1700)
                !_finite(val);
#else
                !isfinite(val);
#endif
        }

        break;
    }
    case PrimitiveInfo::KindDouble:
    {
        if(!PyFloat_Check(p))
        {
            if(PyLong_Check(p))
            {
                PyLong_AsDouble(p); // Just to see if it raises an error.
                if(PyErr_Occurred())
                {
                    return false;
                }
            }
#if PY_VERSION_HEX < 0x03000000
            else if(PyInt_Check(p))
            {
                return true;
            }
#endif
            else
            {
                return false;
            }
        }

        break;
    }
    case PrimitiveInfo::KindString:
    {
#if defined(Py_USING_UNICODE) && PY_VERSION_HEX < 0x03000000
        if(p != Py_None && !checkString(p) && !PyUnicode_Check(p))
#else
        if(p != Py_None && !checkString(p))
#endif
        {
            return false;
        }
        break;
    }
    }

    return true;
}

bool
IcePy::PrimitiveInfo::variableLength() const
{
    return kind == KindString;
}

int
IcePy::PrimitiveInfo::wireSize() const
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
IcePy::PrimitiveInfo::optionalFormat() const
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
IcePy::PrimitiveInfo::marshal(PyObject* p, const Ice::OutputStreamPtr& os, ObjectMap*, bool, const Ice::StringSeq*)
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
        os->write(isTrue ? true : false);
        break;
    }
    case PrimitiveInfo::KindByte:
    {
        long val = PyLong_AsLong(p);
        assert(!PyErr_Occurred()); // validate() should have caught this.
        assert(val >= 0 && val <= 255); // validate() should have caught this.
        os->write(static_cast<Ice::Byte>(val));
        break;
    }
    case PrimitiveInfo::KindShort:
    {
        long val = PyLong_AsLong(p);
        assert(!PyErr_Occurred()); // validate() should have caught this.
        assert(val >= SHRT_MIN && val <= SHRT_MAX); // validate() should have caught this.
        os->write(static_cast<Ice::Short>(val));
        break;
    }
    case PrimitiveInfo::KindInt:
    {
        long val = PyLong_AsLong(p);
        assert(!PyErr_Occurred()); // validate() should have caught this.
        assert(val >= INT_MIN && val <= INT_MAX); // validate() should have caught this.
        os->write(static_cast<Ice::Int>(val));
        break;
    }
    case PrimitiveInfo::KindLong:
    {
        Ice::Long val = PyLong_AsLongLong(p);
        assert(!PyErr_Occurred()); // validate() should have caught this.
        os->write(val);
        break;
    }
    case PrimitiveInfo::KindFloat:
    {
        float val = static_cast<float>(PyFloat_AsDouble(p)); // Attempts to perform conversion.
        if(PyErr_Occurred())
        {
            throw AbortMarshaling();
        }

        os->write(val);
        break;
    }
    case PrimitiveInfo::KindDouble:
    {
        double val = PyFloat_AsDouble(p); // Attempts to perform conversion.
        if(PyErr_Occurred())
        {
            throw AbortMarshaling();
        }

        os->write(val);
        break;
    }
    case PrimitiveInfo::KindString:
    {
        if(!writeString(p, os))
        {
            assert(PyErr_Occurred());
            throw AbortMarshaling();
        }
        break;
    }
    }
}

void
IcePy::PrimitiveInfo::unmarshal(const Ice::InputStreamPtr& is, const UnmarshalCallbackPtr& cb, PyObject* target,
                                void* closure, bool, const Ice::StringSeq*)
{
    switch(kind)
    {
    case PrimitiveInfo::KindBool:
    {
        bool b;
        is->read(b);
        if(b)
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
        Ice::Byte val;
        is->read(val);
        PyObjectHandle p = PyLong_FromLong(val);
        cb->unmarshaled(p.get(), target, closure);
        break;
    }
    case PrimitiveInfo::KindShort:
    {
        Ice::Short val;
        is->read(val);
        PyObjectHandle p = PyLong_FromLong(val);
        cb->unmarshaled(p.get(), target, closure);
        break;
    }
    case PrimitiveInfo::KindInt:
    {
        Ice::Int val;
        is->read(val);
        PyObjectHandle p = PyLong_FromLong(val);
        cb->unmarshaled(p.get(), target, closure);
        break;
    }
    case PrimitiveInfo::KindLong:
    {
        Ice::Long val;
        is->read(val);
        PyObjectHandle p = PyLong_FromLongLong(val);
        cb->unmarshaled(p.get(), target, closure);
        break;
    }
    case PrimitiveInfo::KindFloat:
    {
        Ice::Float val;
        is->read(val);
        PyObjectHandle p = PyFloat_FromDouble(val);
        cb->unmarshaled(p.get(), target, closure);
        break;
    }
    case PrimitiveInfo::KindDouble:
    {
        Ice::Double val;
        is->read(val);
        PyObjectHandle p = PyFloat_FromDouble(val);
        cb->unmarshaled(p.get(), target, closure);
        break;
    }
    case PrimitiveInfo::KindString:
    {
        string val;
        is->read(val);
        PyObjectHandle p = createString(val);
        cb->unmarshaled(p.get(), target, closure);
        break;
    }
    }
}

void
IcePy::PrimitiveInfo::print(PyObject* value, IceUtilInternal::Output& out, PrintObjectHistory*)
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
    assert(checkString(p.get()));
    out << getString(p.get());
}

//
// EnumInfo implementation.
//
IcePy::EnumInfo::EnumInfo(const string& ident, PyObject* t, PyObject* e) :
    id(ident), pythonType(t), maxValue(0)
{
    assert(PyType_Check(t));
    assert(PyDict_Check(e));

    Py_INCREF(t);

    Py_ssize_t pos = 0;
    PyObject* key;
    PyObject* value;
    while(PyDict_Next(e, &pos, &key, &value))
    {
#if PY_VERSION_HEX >= 0x03000000
        assert(PyLong_Check(key));
#else
        assert(PyInt_Check(key));
#endif
        const Ice::Int val = static_cast<Ice::Int>(PyLong_AsLong(key));
        assert(enumerators.find(val) == enumerators.end());

        Py_INCREF(value);
        assert(PyObject_IsInstance(value, t));
        const_cast<EnumeratorMap&>(enumerators)[val] = value;

        if(val > maxValue)
        {
            const_cast<Ice::Int&>(maxValue) = val;
        }
    }
}

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

bool
IcePy::EnumInfo::variableLength() const
{
    return true;
}

int
IcePy::EnumInfo::wireSize() const
{
    return 1;
}

Ice::OptionalFormat
IcePy::EnumInfo::optionalFormat() const
{
    return Ice::OptionalFormatSize;
}

void
IcePy::EnumInfo::marshal(PyObject* p, const Ice::OutputStreamPtr& os, ObjectMap*, bool optional, const Ice::StringSeq*)
{
    //
    // Validate value.
    //
    const Ice::Int val = valueForEnumerator(p);
    if(val < 0)
    {
        assert(PyErr_Occurred());
        throw AbortMarshaling();
    }

    os->writeEnum(val, maxValue);
}

void
IcePy::EnumInfo::unmarshal(const Ice::InputStreamPtr& is, const UnmarshalCallbackPtr& cb, PyObject* target,
                           void* closure, bool, const Ice::StringSeq*)
{
    Ice::Int val = is->readEnum(maxValue);

    PyObjectHandle p = enumeratorForValue(val);
    if(!p.get())
    {
        ostringstream ostr;
        ostr << "enumerator " << val << " is out of range for enum " << id;
        setPythonException(Ice::MarshalException(__FILE__, __LINE__, ostr.str()));
        throw AbortMarshaling();
    }

    cb->unmarshaled(p.get(), target, closure);
}

void
IcePy::EnumInfo::print(PyObject* value, IceUtilInternal::Output& out, PrintObjectHistory*)
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
    assert(checkString(p.get()));
    out << getString(p.get());
}

Ice::Int
IcePy::EnumInfo::valueForEnumerator(PyObject* p) const
{
    assert(PyObject_IsInstance(p, pythonType.get()) == 1);

    PyObjectHandle v = PyObject_GetAttrString(p, STRCAST("_value"));
    if(!v.get())
    {
        assert(PyErr_Occurred());
        return -1;
    }
#if PY_VERSION_HEX >= 0x03000000
    if(!PyLong_Check(v.get()))
#else
    if(!PyInt_Check(v.get()))
#endif
    {
        PyErr_Format(PyExc_ValueError, STRCAST("value for enum %s is not an int"), id.c_str());
        return -1;
    }
    const Ice::Int val = static_cast<Ice::Int>(PyLong_AsLong(v.get()));
    if(enumerators.find(val) == enumerators.end())
    {
        PyErr_Format(PyExc_ValueError, STRCAST("illegal value %d for enum %s"), val, id.c_str());
        return -1;
    }

    return val;
}

PyObject*
IcePy::EnumInfo::enumeratorForValue(Ice::Int v) const
{
    EnumeratorMap::const_iterator p = enumerators.find(v);
    if(p == enumerators.end())
    {
        return 0;
    }
    PyObject* r = p->second.get();
    Py_INCREF(r);
    return r;
}

//
// DataMember implementation.
//
void
IcePy::DataMember::unmarshaled(PyObject* val, PyObject* target, void*)
{
    if(PyObject_SetAttrString(target, const_cast<char*>(name.c_str()), val) < 0)
    {
        assert(PyErr_Occurred());
        throw AbortMarshaling();
    }
}

static void
convertDataMembers(PyObject* members, DataMemberList& reqMembers, DataMemberList& optMembers, bool allowOptional)
{
    list<DataMemberPtr> optList;

    Py_ssize_t sz = PyTuple_GET_SIZE(members);
    for(Py_ssize_t i = 0; i < sz; ++i)
    {
        PyObject* m = PyTuple_GET_ITEM(members, i);
        assert(PyTuple_Check(m));
        assert(PyTuple_GET_SIZE(m) == allowOptional ? 5 : 3);

        PyObject* name = PyTuple_GET_ITEM(m, 0); // Member name.
        assert(checkString(name));
        PyObject* meta = PyTuple_GET_ITEM(m, 1); // Member metadata.
        assert(PyTuple_Check(meta));
        PyObject* t = PyTuple_GET_ITEM(m, 2); // Member type.

        PyObject* opt = 0;
        PyObject* tag = 0;
        if(allowOptional)
        {
            opt = PyTuple_GET_ITEM(m, 3); // Optional?
            tag = PyTuple_GET_ITEM(m, 4);
#if PY_VERSION_HEX < 0x03000000
            assert(PyInt_Check(tag));
#else
            assert(PyLong_Check(tag));
#endif
        }

        DataMemberPtr member = new DataMember;
        member->name = getString(name);
#ifndef NDEBUG
        bool b =
#endif
        tupleToStringSeq(meta, member->metaData);
        assert(b);
        member->type = getType(t);
        if(allowOptional)
        {
            member->optional = PyObject_IsTrue(opt) == 1;
            member->tag = static_cast<int>(PyLong_AsLong(tag));
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
IcePy::StructInfo::StructInfo(const string& ident, PyObject* t, PyObject* m) :
    id(ident), pythonType(t)
{
    assert(PyType_Check(t));
    assert(PyTuple_Check(m));

    Py_INCREF(t);

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
IcePy::StructInfo::getId() const
{
    return id;
}

bool
IcePy::StructInfo::validate(PyObject* val)
{
    return val == Py_None || PyObject_IsInstance(val, pythonType.get()) == 1;
}

bool
IcePy::StructInfo::variableLength() const
{
    return _variableLength;
}

int
IcePy::StructInfo::wireSize() const
{
    return _wireSize;
}

Ice::OptionalFormat
IcePy::StructInfo::optionalFormat() const
{
    return _variableLength ? Ice::OptionalFormatFSize : Ice::OptionalFormatVSize;
}

bool
IcePy::StructInfo::usesClasses() const
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
IcePy::StructInfo::marshal(PyObject* p, const Ice::OutputStreamPtr& os, ObjectMap* objectMap, bool optional,
                           const Ice::StringSeq*)
{
    assert(p == Py_None || PyObject_IsInstance(p, pythonType.get()) == 1); // validate() should have caught this.

    if(p == Py_None)
    {
        if(!_nullMarshalValue.get())
        {
            PyObjectHandle args = PyTuple_New(0);
            PyTypeObject* type = reinterpret_cast<PyTypeObject*>(pythonType.get());
            _nullMarshalValue = type->tp_new(type, args.get(), 0);
            type->tp_init(_nullMarshalValue.get(), args.get(), 0); // Initialize the struct members
        }
        p = _nullMarshalValue.get();
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
            os->writeSize(_wireSize);
        }
    }

    for(DataMemberList::const_iterator q = members.begin(); q != members.end(); ++q)
    {
        DataMemberPtr member = *q;
        char* memberName = const_cast<char*>(member->name.c_str());
        PyObjectHandle attr = PyObject_GetAttrString(p, memberName);
        if(!attr.get())
        {
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
        member->type->marshal(attr.get(), os, objectMap, false, &member->metaData);
    }

    if(optional && _variableLength)
    {
        os->endSize(sizePos);
    }
}

void
IcePy::StructInfo::unmarshal(const Ice::InputStreamPtr& is, const UnmarshalCallbackPtr& cb, PyObject* target,
                             void* closure, bool optional, const Ice::StringSeq*)
{
    PyObjectHandle p = instantiate(pythonType.get());
    if(!p.get())
    {
        assert(PyErr_Occurred());
        throw AbortMarshaling();
    }

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
        member->type->unmarshal(is, member, p.get(), 0, false, &member->metaData);
    }

    cb->unmarshaled(p.get(), target, closure);
}

void
IcePy::StructInfo::print(PyObject* value, IceUtilInternal::Output& out, PrintObjectHistory* history)
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
        out.sb();
        for(DataMemberList::const_iterator q = members.begin(); q != members.end(); ++q)
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
}

void
IcePy::StructInfo::destroy()
{
    for(DataMemberList::const_iterator p = members.begin(); p != members.end(); ++p)
    {
        (*p)->type->destroy();
    }
    const_cast<DataMemberList&>(members).clear();
    if(_nullMarshalValue.get())
    {
        _nullMarshalValue.release();
    }
}

PyObject*
IcePy::StructInfo::instantiate(PyObject* pythonType)
{
    PyObjectHandle args = PyTuple_New(0);
    PyTypeObject* type = reinterpret_cast<PyTypeObject*>(pythonType);
    return type->tp_new(type, args.get(), 0);
}

//
// SequenceInfo implementation.
//
IcePy::SequenceInfo::SequenceInfo(const string& ident, PyObject* m, PyObject* t) :
    id(ident)
{
    assert(PyTuple_Check(m));

    vector<string> metaData;
#ifndef NDEBUG
    bool b =
#endif
    tupleToStringSeq(m, metaData);
    assert(b);

    const_cast<SequenceMappingPtr&>(mapping) = new SequenceMapping(metaData);
    const_cast<TypeInfoPtr&>(elementType) = getType(t);
}

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
IcePy::SequenceInfo::variableLength() const
{
    return true;
}

int
IcePy::SequenceInfo::wireSize() const
{
    return 1;
}

Ice::OptionalFormat
IcePy::SequenceInfo::optionalFormat() const
{
    return elementType->variableLength() ? Ice::OptionalFormatFSize : Ice::OptionalFormatVSize;
}

bool
IcePy::SequenceInfo::usesClasses() const
{
    return elementType->usesClasses();
}

void
IcePy::SequenceInfo::marshal(PyObject* p, const Ice::OutputStreamPtr& os, ObjectMap* objectMap, bool optional,
                             const Ice::StringSeq* metaData)
{
    PrimitiveInfoPtr pi = PrimitiveInfoPtr::dynamicCast(elementType);

    Ice::OutputStream::size_type sizePos = 0;
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
            Py_ssize_t sz = 0;
            if(p != Py_None)
            {
                const void* buf = 0;
                if(PyObject_AsReadBuffer(p, &buf, &sz) == 0)
                {
                    if(pi->kind == PrimitiveInfo::KindString)
                    {
                        PyErr_Format(PyExc_ValueError, STRCAST("expected sequence value"));
                        throw AbortMarshaling();
                    }
                }
                else
                {
                    PyErr_Clear(); // PyObject_AsReadBuffer sets an exception on failure.

                    PyObjectHandle fs;
                    if(pi)
                    {
                        fs = getSequence(pi, p);
                    }
                    else
                    {
                        fs = PySequence_Fast(p, STRCAST("expected a sequence value"));
                    }
                    if(!fs.get())
                    {
                        assert(PyErr_Occurred());
                        return;
                    }
                    sz = PySequence_Fast_GET_SIZE(fs.get());
                }
            }

            const Ice::Int isz = static_cast<Ice::Int>(sz);
            os->writeSize(isz == 0 ? 1 : isz * elementType->wireSize() + (isz > 254 ? 5 : 1));
        }
    }

    if(p == Py_None)
    {
        os->writeSize(0);
    }
    else if(pi)
    {
        marshalPrimitiveSequence(pi, p, os);
    }
    else
    {
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
                assert(PyErr_Occurred());
                throw AbortMarshaling();
            }
            if(!elementType->validate(item))
            {
                PyErr_Format(PyExc_ValueError, STRCAST("invalid value for element %d of `%s'"), static_cast<int>(i),
                             const_cast<char*>(id.c_str()));
                throw AbortMarshaling();
            }
            elementType->marshal(item, os, objectMap, false);
        }
    }

    if(optional && elementType->variableLength())
    {
        os->endSize(sizePos);
    }
}

void
IcePy::SequenceInfo::unmarshal(const Ice::InputStreamPtr& is, const UnmarshalCallbackPtr& cb, PyObject* target,
                               void* closure, bool optional, const Ice::StringSeq* metaData)
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
        assert(PyErr_Occurred());
        throw AbortMarshaling();
    }

    for(Ice::Int i = 0; i < sz; ++i)
    {
        void* cl = reinterpret_cast<void*>(static_cast<Py_ssize_t>(i));
        elementType->unmarshal(is, sm, result.get(), cl, false);
    }

    cb->unmarshaled(result.get(), target, closure);
}

void
IcePy::SequenceInfo::print(PyObject* value, IceUtilInternal::Output& out, PrintObjectHistory* history)
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
        const_cast<TypeInfoPtr&>(elementType) = 0;
    }
}

PyObject*
IcePy::SequenceInfo::getSequence(const PrimitiveInfoPtr& pi, PyObject* p)
{
    PyObjectHandle fs;

    if(pi->kind == PrimitiveInfo::KindByte)
    {
#if PY_VERSION_HEX >= 0x03000000
        //
        // For sequence<byte>, accept a bytes object or a sequence.
        //
        if(!PyBytes_Check(p))
        {
            fs = PySequence_Fast(p, STRCAST("expected a bytes, sequence, or buffer value"));
        }
#else
        //
        // For sequence<byte>, accept a string or a sequence.
        //
        if(!checkString(p))
        {
            fs = PySequence_Fast(p, STRCAST("expected a string, sequence, or buffer value"));
        }
#endif
    }
    else
    {
        fs = PySequence_Fast(p, STRCAST("expected a sequence or buffer value"));
    }

    return fs.release();
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
            os->write(reinterpret_cast<const bool*>(b), reinterpret_cast<const bool*>(b + sz));
            break;
        }
        case PrimitiveInfo::KindByte:
        {
            os->write(reinterpret_cast<const Ice::Byte*>(b), reinterpret_cast<const Ice::Byte*>(b + sz));
            break;
        }
        case PrimitiveInfo::KindShort:
        {
            os->write(reinterpret_cast<const Ice::Short*>(b), reinterpret_cast<const Ice::Short*>(b + sz));
            break;
        }
        case PrimitiveInfo::KindInt:
        {
            os->write(reinterpret_cast<const Ice::Int*>(b), reinterpret_cast<const Ice::Int*>(b + sz));
            break;
        }
        case PrimitiveInfo::KindLong:
        {
            os->write(reinterpret_cast<const Ice::Long*>(b), reinterpret_cast<const Ice::Long*>(b + sz));
            break;
        }
        case PrimitiveInfo::KindFloat:
        {
            os->write(reinterpret_cast<const Ice::Float*>(b), reinterpret_cast<const Ice::Float*>(b + sz));
            break;
        }
        case PrimitiveInfo::KindDouble:
        {
            os->write(reinterpret_cast<const Ice::Double*>(b), reinterpret_cast<const Ice::Double*>(b + sz));
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
    else
    {
        PyErr_Clear(); // PyObject_AsReadBuffer sets an exception on failure.
    }

    PyObjectHandle fs = getSequence(pi, p);
    if(!fs.get())
    {
        assert(PyErr_Occurred());
        return;
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
                assert(PyErr_Occurred());
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
        os->write(seq);
        break;
    }
    case PrimitiveInfo::KindByte:
    {
        if(!fs.get())
        {
#if PY_VERSION_HEX >= 0x03000000
            assert(PyBytes_Check(p));
            char* str;
            PyBytes_AsStringAndSize(p, &str, &sz);
            os->write(reinterpret_cast<const Ice::Byte*>(str), reinterpret_cast<const Ice::Byte*>(str + sz));
#else
            assert(PyString_Check(p));
            char* str;
            PyString_AsStringAndSize(p, &str, &sz);
            os->write(reinterpret_cast<const Ice::Byte*>(str), reinterpret_cast<const Ice::Byte*>(str + sz));
#endif
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
                    assert(PyErr_Occurred());
                    throw AbortMarshaling();
                }

                long val = PyLong_AsLong(item);

                if(PyErr_Occurred() || val < 0 || val > 255)
                {
                    PyErr_Format(PyExc_ValueError, STRCAST("invalid value for element %d of sequence<byte>"),
                                 static_cast<int>(i));
                    throw AbortMarshaling();
                }
                seq[i] = static_cast<Ice::Byte>(val);
            }
            os->write(seq);
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
                assert(PyErr_Occurred());
                throw AbortMarshaling();
            }

            long val = PyLong_AsLong(item);

            if(PyErr_Occurred() || val < SHRT_MIN || val > SHRT_MAX)
            {
                PyErr_Format(PyExc_ValueError, STRCAST("invalid value for element %d of sequence<short>"),
                             static_cast<int>(i));
                throw AbortMarshaling();
            }
            seq[i] = static_cast<Ice::Short>(val);
        }
        os->write(seq);
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
                assert(PyErr_Occurred());
                throw AbortMarshaling();
            }

            long val = PyLong_AsLong(item);

            if(PyErr_Occurred() || val < INT_MIN || val > INT_MAX)
            {
                PyErr_Format(PyExc_ValueError, STRCAST("invalid value for element %d of sequence<int>"),
                             static_cast<int>(i));
                throw AbortMarshaling();
            }
            seq[i] = static_cast<Ice::Int>(val);
        }
        os->write(seq);
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
                assert(PyErr_Occurred());
                throw AbortMarshaling();
            }

            Ice::Long val = PyLong_AsLongLong(item);

            if(PyErr_Occurred())
            {
                PyErr_Format(PyExc_ValueError, STRCAST("invalid value for element %d of sequence<long>"),
                             static_cast<int>(i));
                throw AbortMarshaling();
            }
            seq[i] = val;
        }
        os->write(seq);
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
                assert(PyErr_Occurred());
                throw AbortMarshaling();
            }

            float val = static_cast<float>(PyFloat_AsDouble(item));
            if(PyErr_Occurred())
            {
                PyErr_Format(PyExc_ValueError, STRCAST("invalid value for element %d of sequence<float>"),
                             static_cast<int>(i));
                throw AbortMarshaling();
            }

            seq[i] = val;
        }
        os->write(seq);
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
                assert(PyErr_Occurred());
                throw AbortMarshaling();
            }

            double val = PyFloat_AsDouble(item);
            if(PyErr_Occurred())
            {
                PyErr_Format(PyExc_ValueError, STRCAST("invalid value for element %d of sequence<double>"),
                             static_cast<int>(i));
                throw AbortMarshaling();
            }

            seq[i] = val;
        }
        os->write(seq);
        break;
    }
    case PrimitiveInfo::KindString:
    {
        sz = PySequence_Fast_GET_SIZE(fs.get());
        os->writeSize(static_cast<int>(sz));
        for(Py_ssize_t i = 0; i < sz; ++i)
        {
            PyObject* item = PySequence_Fast_GET_ITEM(fs.get(), i);
            if(!item)
            {
                assert(PyErr_Occurred());
                throw AbortMarshaling();
            }

#if defined(Py_USING_UNICODE) && PY_VERSION_HEX < 0x03000000
            if(item != Py_None && !checkString(item) && !PyUnicode_Check(item))
#else
            if(item != Py_None && !checkString(item))
#endif
            {
                PyErr_Format(PyExc_ValueError, STRCAST("invalid value for element %d of sequence<string>"),
                             static_cast<int>(i));
                throw AbortMarshaling();
            }

            if(!writeString(item, os))
            {
                assert(PyErr_Occurred());
                throw AbortMarshaling();
            }
        }
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
        IceUtil::ScopedArray<bool> arr;
        is->read(p, arr);
        int sz = static_cast<int>(p.second - p.first);
        result = sm->createContainer(sz);
        if(!result.get())
        {
            assert(PyErr_Occurred());
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
        is->read(p);
        int sz = static_cast<int>(p.second - p.first);
        if(sm->type == SequenceMapping::SEQ_DEFAULT)
        {
#if PY_VERSION_HEX >= 0x03000000
            result = PyBytes_FromStringAndSize(reinterpret_cast<const char*>(p.first), sz);
#else
            result = PyString_FromStringAndSize(reinterpret_cast<const char*>(p.first), sz);
#endif
            if(!result.get())
            {
                assert(PyErr_Occurred());
                throw AbortMarshaling();
            }
        }
        else
        {
            result = sm->createContainer(sz);
            if(!result.get())
            {
                assert(PyErr_Occurred());
                throw AbortMarshaling();
            }

            for(int i = 0; i < sz; ++i)
            {
                PyObjectHandle item = PyLong_FromLong(p.first[i]);
                if(!item.get())
                {
                    assert(PyErr_Occurred());
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
        IceUtil::ScopedArray<Ice::Short> arr;
        is->read(p, arr);
        int sz = static_cast<int>(p.second - p.first);
        result = sm->createContainer(sz);
        if(!result.get())
        {
            assert(PyErr_Occurred());
            throw AbortMarshaling();
        }

        for(int i = 0; i < sz; ++i)
        {
            PyObjectHandle item = PyLong_FromLong(p.first[i]);
            if(!item.get())
            {
                assert(PyErr_Occurred());
                throw AbortMarshaling();
            }
            sm->setItem(result.get(), i, item.get());
        }
        break;
    }
    case PrimitiveInfo::KindInt:
    {
        pair<const Ice::Int*, const Ice::Int*> p;
        IceUtil::ScopedArray<Ice::Int> arr;
        is->read(p, arr);
        int sz = static_cast<int>(p.second - p.first);
        result = sm->createContainer(sz);
        if(!result.get())
        {
            assert(PyErr_Occurred());
            throw AbortMarshaling();
        }

        for(int i = 0; i < sz; ++i)
        {
            PyObjectHandle item = PyLong_FromLong(p.first[i]);
            if(!item.get())
            {
                assert(PyErr_Occurred());
                throw AbortMarshaling();
            }
            sm->setItem(result.get(), i, item.get());
        }
        break;
    }
    case PrimitiveInfo::KindLong:
    {
        pair<const Ice::Long*, const Ice::Long*> p;
        IceUtil::ScopedArray<Ice::Long> arr;
        is->read(p, arr);
        int sz = static_cast<int>(p.second - p.first);
        result = sm->createContainer(sz);
        if(!result.get())
        {
            assert(PyErr_Occurred());
            throw AbortMarshaling();
        }

        for(int i = 0; i < sz; ++i)
        {
            PyObjectHandle item = PyLong_FromLongLong(p.first[i]);
            if(!item.get())
            {
                assert(PyErr_Occurred());
                throw AbortMarshaling();
            }
            sm->setItem(result.get(), i, item.get());
        }
        break;
    }
    case PrimitiveInfo::KindFloat:
    {
        pair<const Ice::Float*, const Ice::Float*> p;
        IceUtil::ScopedArray<Ice::Float> arr;
        is->read(p, arr);
        int sz = static_cast<int>(p.second - p.first);
        result = sm->createContainer(sz);
        if(!result.get())
        {
            assert(PyErr_Occurred());
            throw AbortMarshaling();
        }

        for(int i = 0; i < sz; ++i)
        {
            PyObjectHandle item = PyFloat_FromDouble(p.first[i]);
            if(!item.get())
            {
                assert(PyErr_Occurred());
                throw AbortMarshaling();
            }
            sm->setItem(result.get(), i, item.get());
        }
        break;
    }
    case PrimitiveInfo::KindDouble:
    {
        pair<const Ice::Double*, const Ice::Double*> p;
        IceUtil::ScopedArray<Ice::Double> arr;
        is->read(p, arr);
        int sz = static_cast<int>(p.second - p.first);
        result = sm->createContainer(sz);
        if(!result.get())
        {
            assert(PyErr_Occurred());
            throw AbortMarshaling();
        }

        for(int i = 0; i < sz; ++i)
        {
            PyObjectHandle item = PyFloat_FromDouble(p.first[i]);
            if(!item.get())
            {
                assert(PyErr_Occurred());
                throw AbortMarshaling();
            }
            sm->setItem(result.get(), i, item.get());
        }
        break;
    }
    case PrimitiveInfo::KindString:
    {
        Ice::StringSeq seq;
        is->read(seq);
        int sz = static_cast<int>(seq.size());
        result = sm->createContainer(sz);
        if(!result.get())
        {
            assert(PyErr_Occurred());
            throw AbortMarshaling();
        }

        for(int i = 0; i < sz; ++i)
        {
            PyObjectHandle item = createString(seq[i]);
            if(!item.get())
            {
                assert(PyErr_Occurred());
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
    Py_ssize_t i = reinterpret_cast<Py_ssize_t>(closure);
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
// CustomInfo implementation.
//
IcePy::CustomInfo::CustomInfo(const string& ident, PyObject* t) :
    id(ident), pythonType(t)
{
    assert(PyType_Check(t));
}

string
IcePy::CustomInfo::getId() const
{
    return id;
}

bool
IcePy::CustomInfo::validate(PyObject* val)
{
    return PyObject_IsInstance(val, pythonType.get()) == 1;
}

bool
IcePy::CustomInfo::variableLength() const
{
    return true;
}

int
IcePy::CustomInfo::wireSize() const
{
    return 1;
}

Ice::OptionalFormat
IcePy::CustomInfo::optionalFormat() const
{
    return Ice::OptionalFormatVSize;
}

bool
IcePy::CustomInfo::usesClasses() const
{
    return false;
}

void
IcePy::CustomInfo::marshal(PyObject* p, const Ice::OutputStreamPtr& os, ObjectMap* objectMap, bool,
                           const Ice::StringSeq* metaData)
{
    assert(PyObject_IsInstance(p, pythonType.get()) == 1); // validate() should have caught this.

    PyObjectHandle obj = PyObject_CallMethod(p, STRCAST("IsInitialized"), 0);
    if(!obj.get())
    {
        throwPythonException();
    }
    if(!PyObject_IsTrue(obj.get()))
    {
        setPythonException(Ice::MarshalException(__FILE__, __LINE__, "type not fully initialized"));
        throw AbortMarshaling();
    }

    obj = PyObject_CallMethod(p, STRCAST("SerializeToString"), 0);
    if(!obj.get())
    {
        assert(PyErr_Occurred());
        throw AbortMarshaling();
    }

    assert(checkString(obj.get()));
    char* str;
    Py_ssize_t sz;
#if PY_VERSION_HEX >= 0x03000000
    PyBytes_AsStringAndSize(obj.get(), &str, &sz);
#else
    PyString_AsStringAndSize(obj.get(), &str, &sz);
#endif
    os->write(reinterpret_cast<const Ice::Byte*>(str), reinterpret_cast<const Ice::Byte*>(str + sz));
}

void
IcePy::CustomInfo::unmarshal(const Ice::InputStreamPtr& is, const UnmarshalCallbackPtr& cb, PyObject* target,
                             void* closure, bool, const Ice::StringSeq* metaData)
{
    //
    // Unmarshal the raw byte sequence.
    //
    pair<const Ice::Byte*, const Ice::Byte*> seq;
    is->read(seq);
    int sz = static_cast<int>(seq.second - seq.first);

    //
    // Create a new instance of the protobuf type.
    //
    PyObjectHandle args = PyTuple_New(0);
    if(!args.get())
    {
        assert(PyErr_Occurred());
        throw AbortMarshaling();
    }
    PyTypeObject* type = reinterpret_cast<PyTypeObject*>(pythonType.get());
    PyObjectHandle p = type->tp_new(type, args.get(), 0);
    if(!p.get())
    {
        assert(PyErr_Occurred());
        throw AbortMarshaling();
    }

    //
    // Initialize the object.
    //
    PyObjectHandle obj = PyObject_CallMethod(p.get(), STRCAST("__init__"), 0, 0);
    if(!obj.get())
    {
        assert(PyErr_Occurred());
        throw AbortMarshaling();
    }

    //
    // Convert the seq to a string.
    //
#if PY_VERSION_HEX >= 0x03000000
    obj = PyBytes_FromStringAndSize(reinterpret_cast<const char*>(seq.first), sz);
#else
    obj = PyString_FromStringAndSize(reinterpret_cast<const char*>(seq.first), sz);
#endif
    if(!obj.get())
    {
        assert(PyErr_Occurred());
        throw AbortMarshaling();
    }

    //
    // Parse the string.
    //
    obj = PyObject_CallMethod(p.get(), STRCAST("ParseFromString"), STRCAST("O"), obj.get(), 0);
    if(!obj.get())
    {
        assert(PyErr_Occurred());
        throw AbortMarshaling();
    }

    cb->unmarshaled(p.get(), target, closure);
}

void
IcePy::CustomInfo::print(PyObject* value, IceUtilInternal::Output& out, PrintObjectHistory* history)
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
    }
}

void
IcePy::CustomInfo::destroy()
{
}

//
// DictionaryInfo implementation.
//
IcePy::DictionaryInfo::DictionaryInfo(const string& ident, PyObject* kt, PyObject* vt) :
    id(ident)
{
    const_cast<TypeInfoPtr&>(keyType) = getType(kt);
    const_cast<TypeInfoPtr&>(valueType) = getType(vt);

    _variableLength = keyType->variableLength() || valueType->variableLength();
    _wireSize = keyType->wireSize() + valueType->wireSize();
}

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
IcePy::DictionaryInfo::variableLength() const
{
    return true;
}

int
IcePy::DictionaryInfo::wireSize() const
{
    return 1;
}

Ice::OptionalFormat
IcePy::DictionaryInfo::optionalFormat() const
{
    return _variableLength ? Ice::OptionalFormatFSize : Ice::OptionalFormatVSize;
}

bool
IcePy::DictionaryInfo::usesClasses() const
{
    return valueType->usesClasses();
}

void
IcePy::DictionaryInfo::marshal(PyObject* p, const Ice::OutputStreamPtr& os, ObjectMap* objectMap, bool optional,
                               const Ice::StringSeq*)
{
    if(p != Py_None && !PyDict_Check(p))
    {
        PyErr_Format(PyExc_ValueError, STRCAST("expected dictionary value"));
        throw AbortMarshaling();
    }

    const Ice::Int sz = p == Py_None ? 0 : static_cast<Ice::Int>(PyDict_Size(p));

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

    if(p == Py_None)
    {
        os->writeSize(0);
    }
    else
    {
        os->writeSize(sz);

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
            keyType->marshal(key, os, objectMap, false);

            if(!valueType->validate(value))
            {
                PyErr_Format(PyExc_ValueError, STRCAST("invalid value in `%s' element"), const_cast<char*>(id.c_str()));
                throw AbortMarshaling();
            }
            valueType->marshal(value, os, objectMap, false);
        }
    }

    if(optional && _variableLength)
    {
        os->endSize(sizePos);
    }
}

void
IcePy::DictionaryInfo::unmarshal(const Ice::InputStreamPtr& is, const UnmarshalCallbackPtr& cb, PyObject* target,
                                 void* closure, bool optional, const Ice::StringSeq*)
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

    PyObjectHandle p = PyDict_New();
    if(!p.get())
    {
        assert(PyErr_Occurred());
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
        keyType->unmarshal(is, keyCB, 0, 0, false);
        assert(keyCB->key.get());

        //
        // Insert the key into the dictionary with a dummy value in order to hold
        // a reference to the key. In case of an exception, we don't want to leak
        // the key.
        //
        if(PyDict_SetItem(p.get(), keyCB->key.get(), Py_None) < 0)
        {
            assert(PyErr_Occurred());
            throw AbortMarshaling();
        }

        //
        // The callback will reset the dictionary entry with the unmarshaled value,
        // so we pass it the key.
        //
        void* cl = reinterpret_cast<void*>(keyCB->key.get());
        valueType->unmarshal(is, this, p.get(), cl, false);
    }

    cb->unmarshaled(p.get(), target, closure);
}

void
IcePy::DictionaryInfo::unmarshaled(PyObject* val, PyObject* target, void* closure)
{
    PyObject* key = reinterpret_cast<PyObject*>(closure);
    if(PyDict_SetItem(target, key, val) < 0)
    {
        assert(PyErr_Occurred());
        throw AbortMarshaling();
    }
}

void
IcePy::DictionaryInfo::print(PyObject* value, IceUtilInternal::Output& out, PrintObjectHistory* history)
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
IcePy::ClassInfo::ClassInfo(const string& ident) :
    id(ident), compactId(-1), isAbstract(false), preserve(false), defined(false)
{
    const_cast<PyObjectHandle&>(typeObj) = createType(this);
}

void
IcePy::ClassInfo::define(PyObject* t, int compact, bool abstr, bool pres, PyObject* b, PyObject* i, PyObject* m)
{
    assert(PyType_Check(t));
    assert(PyTuple_Check(i));
    assert(PyTuple_Check(m));

    const_cast<int&>(compactId) = compact;
    const_cast<bool&>(isAbstract) = abstr;
    const_cast<bool&>(preserve) = pres;

    if(b != Py_None)
    {
        const_cast<ClassInfoPtr&>(base) = ClassInfoPtr::dynamicCast(getType(b));
        assert(base);
    }

    Py_ssize_t n, sz;
    sz = PyTuple_GET_SIZE(i);
    for(n = 0; n < sz; ++n)
    {
        PyObject* o = PyTuple_GET_ITEM(i, n);
        ClassInfoPtr iface = ClassInfoPtr::dynamicCast(getType(o));
        assert(iface);
        const_cast<ClassInfoList&>(interfaces).push_back(iface);
    }

    convertDataMembers(m, const_cast<DataMemberList&>(members), const_cast<DataMemberList&>(optionalMembers), true);

    const_cast<PyObjectHandle&>(pythonType) = t;
    Py_INCREF(t);

    const_cast<bool&>(defined) = true;
}

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
IcePy::ClassInfo::variableLength() const
{
    return true;
}

int
IcePy::ClassInfo::wireSize() const
{
    return 1;
}

Ice::OptionalFormat
IcePy::ClassInfo::optionalFormat() const
{
    return Ice::OptionalFormatClass;
}

bool
IcePy::ClassInfo::usesClasses() const
{
    return true;
}

void
IcePy::ClassInfo::marshal(PyObject* p, const Ice::OutputStreamPtr& os, ObjectMap* objectMap, bool,
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
        writer = new ObjectWriter(p, objectMap);
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
                            void* closure, bool, const Ice::StringSeq*)
{
    if(!pythonType.get())
    {
        PyErr_Format(PyExc_RuntimeError, STRCAST("class %s is declared but not defined"), id.c_str());
        throw AbortMarshaling();
    }

    is->readObject(new ReadObjectCallback(this, cb, target, closure));
}

void
IcePy::ClassInfo::print(PyObject* value, IceUtilInternal::Output& out, PrintObjectHistory* history)
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
            PyObjectHandle iceType = PyObject_GetAttrString(value, STRCAST("_ice_type"));
            ClassInfoPtr info;
            if(!iceType.get())
            {
                //
                // The _ice_type attribute will be missing in an instance of LocalObject
                // that does not derive from a user-defined type.
                //
                assert(id == "::Ice::LocalObject");
                info = this;
            }
            else
            {
                info = ClassInfoPtr::dynamicCast(getType(iceType.get()));
                assert(info);
            }
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
    const_cast<ClassInfoPtr&>(base) = 0;
    const_cast<ClassInfoList&>(interfaces).clear();
    if(!members.empty())
    {
        DataMemberList ml = members;
        const_cast<DataMemberList&>(members).clear();
        for(DataMemberList::iterator p = ml.begin(); p != ml.end(); ++p)
        {
            (*p)->type->destroy();
        }
    }
    const_cast<PyObjectHandle&>(typeObj) = 0; // Break circular reference.
}

void
IcePy::ClassInfo::printMembers(PyObject* value, IceUtilInternal::Output& out, PrintObjectHistory* history)
{
    if(base)
    {
        base->printMembers(value, out, history);
    }

    DataMemberList::const_iterator q;

    for(q = members.begin(); q != members.end(); ++q)
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

    for(q = optionalMembers.begin(); q != optionalMembers.end(); ++q)
    {
        DataMemberPtr member = *q;
        char* memberName = const_cast<char*>(member->name.c_str());
        PyObjectHandle attr = PyObject_GetAttrString(value, memberName);
        out << nl << member->name << " = ";
        if(!attr.get())
        {
            out << "<not defined>";
        }
        else if(attr.get() == Unset)
        {
            out << "<unset>";
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
IcePy::ProxyInfo::ProxyInfo(const string& ident) :
    id(ident)
{
    const_cast<PyObjectHandle&>(typeObj) = createType(this);
}

void
IcePy::ProxyInfo::define(PyObject* t)
{
    const_cast<PyObjectHandle&>(pythonType) = t;
    Py_INCREF(t);
}

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

bool
IcePy::ProxyInfo::variableLength() const
{
    return true;
}

int
IcePy::ProxyInfo::wireSize() const
{
    return 1;
}

Ice::OptionalFormat
IcePy::ProxyInfo::optionalFormat() const
{
    return Ice::OptionalFormatFSize;
}

void
IcePy::ProxyInfo::marshal(PyObject* p, const Ice::OutputStreamPtr& os, ObjectMap*, bool optional, const Ice::StringSeq*)
{
    Ice::OutputStream::size_type sizePos = 0;
    if(optional)
    {
        sizePos = os->startSize();
    }

    if(p == Py_None)
    {
        os->write(Ice::ObjectPrx());
    }
    else if(checkProxy(p))
    {
        os->write(getProxy(p));
    }
    else
    {
        assert(false); // validate() should have caught this.
    }

    if(optional)
    {
        os->endSize(sizePos);
    }
}

void
IcePy::ProxyInfo::unmarshal(const Ice::InputStreamPtr& is, const UnmarshalCallbackPtr& cb, PyObject* target,
                            void* closure, bool optional, const Ice::StringSeq*)
{
    if(optional)
    {
        is->skip(4);
    }

    Ice::ObjectPrx proxy;
    is->read(proxy);

    if(!proxy)
    {
        cb->unmarshaled(Py_None, target, closure);
        return;
    }

    if(!pythonType.get())
    {
        PyErr_Format(PyExc_RuntimeError, STRCAST("class %s is declared but not defined"), id.c_str());
        throw AbortMarshaling();
    }

    PyObjectHandle p = createProxy(proxy, is->communicator(), pythonType.get());
    cb->unmarshaled(p.get(), target, closure);
}

void
IcePy::ProxyInfo::print(PyObject* value, IceUtilInternal::Output& out, PrintObjectHistory*)
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
        assert(checkString(p.get()));
        out << getString(p.get());
    }
}

void
IcePy::ProxyInfo::destroy()
{
    const_cast<PyObjectHandle&>(typeObj) = 0; // Break circular reference.
}

//
// ObjectWriter implementation.
//
IcePy::ObjectWriter::ObjectWriter(PyObject* object, ObjectMap* objectMap) :
    _object(object), _map(objectMap)
{
    Py_INCREF(_object);

    PyObjectHandle iceType = PyObject_GetAttrString(object, STRCAST("_ice_type"));
    if(!iceType.get())
    {
        assert(PyErr_Occurred());
        throw AbortMarshaling();
    }
    _info = ClassInfoPtr::dynamicCast(getType(iceType.get()));
    assert(_info);
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
        if(!tmp.get())
        {
            assert(PyErr_Occurred());
            throw AbortMarshaling();
        }
    }
}

void
IcePy::ObjectWriter::write(const Ice::OutputStreamPtr& os) const
{
    Ice::SlicedDataPtr slicedData;

    if(_info->preserve)
    {
        //
        // Retrieve the SlicedData object that we stored as a hidden member of the Python object.
        //
        slicedData = SlicedDataUtil::getMember(_object, const_cast<ObjectMap*>(_map));
    }

    os->startObject(slicedData);

    if(_info->id != "::Ice::UnknownSlicedObject")
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

    os->endObject();
}

void
IcePy::ObjectWriter::writeMembers(const Ice::OutputStreamPtr& os, const DataMemberList& members) const
{
    for(DataMemberList::const_iterator q = members.begin(); q != members.end(); ++q)
    {
        DataMemberPtr member = *q;

        char* memberName = const_cast<char*>(member->name.c_str());

        PyObjectHandle val = PyObject_GetAttrString(_object, memberName);
        if(!val.get())
        {
            if(member->optional)
            {
                PyErr_Clear();
                continue;
            }
            else
            {
                PyErr_Format(PyExc_AttributeError, STRCAST("no member `%s' found in %s value"), memberName,
                             const_cast<char*>(_info->id.c_str()));
                throw AbortMarshaling();
            }
        }
        else if(member->optional &&
                (val.get() == Unset || !os->writeOptional(member->tag, member->type->optionalFormat())))
        {
            continue;
        }

        if(!member->type->validate(val.get()))
        {
            PyErr_Format(PyExc_ValueError, STRCAST("invalid value for %s member `%s'"),
                         const_cast<char*>(_info->id.c_str()), memberName);
            throw AbortMarshaling();
        }

        member->type->marshal(val.get(), os, _map, member->optional, &member->metaData);
    }
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
        if(!tmp.get())
        {
            assert(PyErr_Occurred());
            throw AbortMarshaling();
        }
    }
}

void
IcePy::ObjectReader::read(const Ice::InputStreamPtr& is)
{
    is->startObject();

    const bool unknown = _info->id == "::Ice::UnknownSlicedObject";

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
                member->type->unmarshal(is, member, _object, 0, false, &member->metaData);
            }

            //
            // The optional members have already been sorted by tag.
            //
            for(p = info->optionalMembers.begin(); p != info->optionalMembers.end(); ++p)
            {
                DataMemberPtr member = *p;
                if(is->readOptional(member->tag, member->type->optionalFormat()))
                {
                    member->type->unmarshal(is, member, _object, 0, true, &member->metaData);
                }
                else if(PyObject_SetAttrString(_object, const_cast<char*>(member->name.c_str()), Unset) < 0)
                {
                    assert(PyErr_Occurred());
                    throw AbortMarshaling();
                }
            }

            is->endSlice();

            info = info->base;
        }
    }

    _slicedData = is->endObject(_info->preserve);

    if(_slicedData)
    {
        SlicedDataUtil* util = reinterpret_cast<SlicedDataUtil*>(is->closure());
        assert(util);
        util->add(this);

        //
        // Define the "unknownTypeId" member for an instance of UnknownSlicedObject.
        //
        if(unknown)
        {
            assert(!_slicedData->slices.empty());

            PyObjectHandle typeId = createString(_slicedData->slices[0]->typeId);
            if(!typeId.get() || PyObject_SetAttrString(_object, STRCAST("unknownTypeId"), typeId.get()) < 0)
            {
                assert(PyErr_Occurred());
                throw AbortMarshaling();
            }
        }
    }
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

Ice::SlicedDataPtr
IcePy::ObjectReader::getSlicedData() const
{
    return _slicedData;
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
    _compactIdMap.clear();
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
        // Verify that the object's type is compatible with the formal type.
        //
        PyObject* obj = reader->getObject(); // Borrowed reference.
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

    Ice::SlicedDataPtr slicedData;

    if(preserve)
    {
        //
        // Retrieve the SlicedData object that we stored as a hidden member of the Python object.
        //
        slicedData = SlicedDataUtil::getMember(p, objectMap);
    }

    os->startException(slicedData);

    ExceptionInfoPtr info = this;
    while(info)
    {
        os->startSlice(info->id, -1, !info->base);

        writeMembers(p, os, info->members, objectMap);
        writeMembers(p, os, info->optionalMembers, objectMap); // The optional members have already been sorted by tag.

        os->endSlice();

        info = info->base;
    }

    os->endException();
}

void
IcePy::ExceptionInfo::writeMembers(PyObject* p, const Ice::OutputStreamPtr& os, const DataMemberList& members,
                                   ObjectMap* objectMap) const
{
    for(DataMemberList::const_iterator q = members.begin(); q != members.end(); ++q)
    {
        DataMemberPtr member = *q;

        char* memberName = const_cast<char*>(member->name.c_str());

        PyObjectHandle val = PyObject_GetAttrString(p, memberName);
        if(!val.get())
        {
            if(member->optional)
            {
                PyErr_Clear();
                continue;
            }
            else
            {
                PyErr_Format(PyExc_AttributeError, STRCAST("no member `%s' found in %s value"), memberName,
                             const_cast<char*>(id.c_str()));
                throw AbortMarshaling();
            }
        }
        else if(member->optional &&
                (val.get() == Unset || !os->writeOptional(member->tag, member->type->optionalFormat())))
        {
            continue;
        }

        if(!member->type->validate(val.get()))
        {
            PyErr_Format(PyExc_ValueError, STRCAST("invalid value for %s member `%s'"),
                         const_cast<char*>(id.c_str()), memberName);
            throw AbortMarshaling();
        }

        member->type->marshal(val.get(), os, objectMap, member->optional, &member->metaData);
    }
}

PyObject*
IcePy::ExceptionInfo::unmarshal(const Ice::InputStreamPtr& is)
{
    PyObjectHandle p = createExceptionInstance(pythonType.get());

    ExceptionInfoPtr info = this;
    while(info)
    {
        is->startSlice();

        DataMemberList::iterator q;

        for(q = info->members.begin(); q != info->members.end(); ++q)
        {
            DataMemberPtr member = *q;
            member->type->unmarshal(is, member, p.get(), 0, false, &member->metaData);
        }

        //
        // The optional members have already been sorted by tag.
        //
        for(q = info->optionalMembers.begin(); q != info->optionalMembers.end(); ++q)
        {
            DataMemberPtr member = *q;
            if(is->readOptional(member->tag, member->type->optionalFormat()))
            {
                member->type->unmarshal(is, member, p.get(), 0, true, &member->metaData);
            }
            else if(PyObject_SetAttrString(p.get(), const_cast<char*>(member->name.c_str()), Unset) < 0)
            {
                assert(PyErr_Occurred());
                throw AbortMarshaling();
            }
        }

        is->endSlice();

        info = info->base;
    }

    return p.release();
}

void
IcePy::ExceptionInfo::print(PyObject* value, IceUtilInternal::Output& out)
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
IcePy::ExceptionInfo::printMembers(PyObject* value, IceUtilInternal::Output& out, PrintObjectHistory* history)
{
    if(base)
    {
        base->printMembers(value, out, history);
    }

    DataMemberList::iterator q;

    for(q = members.begin(); q != members.end(); ++q)
    {
        DataMemberPtr member = *q;
        char* memberName = const_cast<char*>(member->name.c_str());
        PyObjectHandle attr = PyObject_GetAttrString(value, memberName);
        out << nl << member->name << " = ";
        if(!attr.get() || attr.get() == Unset)
        {
            out << "<not defined>";
        }
        else
        {
            member->type->print(attr.get(), out, history);
        }
    }

    for(q = optionalMembers.begin(); q != optionalMembers.end(); ++q)
    {
        DataMemberPtr member = *q;
        char* memberName = const_cast<char*>(member->name.c_str());
        PyObjectHandle attr = PyObject_GetAttrString(value, memberName);
        out << nl << member->name << " = ";
        if(!attr.get())
        {
            out << "<not defined>";
        }
        else if(attr.get() == Unset)
        {
            out << "<unset>";
        }
        else
        {
            member->type->print(attr.get(), out, history);
        }
    }
}

//
// ExceptionWriter implementation.
//
IcePy::ExceptionWriter::ExceptionWriter(const Ice::CommunicatorPtr& communicator, const PyObjectHandle& ex,
                                        const ExceptionInfoPtr& info) :
    Ice::UserExceptionWriter(communicator), _ex(ex), _info(info)
{
    if(!info)
    {
        PyObjectHandle iceType = PyObject_GetAttrString(ex.get(), STRCAST("_ice_type"));
        assert(iceType.get());
        _info = ExceptionInfoPtr::dynamicCast(getException(iceType.get()));
        assert(_info);
    }
}

IcePy::ExceptionWriter::~ExceptionWriter() throw()
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    _ex = 0;
}

void
IcePy::ExceptionWriter::write(const Ice::OutputStreamPtr& os) const
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    _info->marshal(_ex.get(), os, const_cast<ObjectMap*>(&_objects));
}

bool
IcePy::ExceptionWriter::usesClasses() const
{
    return _info->usesClasses;
}

string
IcePy::ExceptionWriter::ice_name() const
{
    return _info->id;
}

Ice::UserException*
IcePy::ExceptionWriter::ice_clone() const
{
    return new ExceptionWriter(*this);
}

void
IcePy::ExceptionWriter::ice_throw() const
{
    throw *this;
}

//
// ExceptionReader implementation.
//
IcePy::ExceptionReader::ExceptionReader(const Ice::CommunicatorPtr& communicator, const ExceptionInfoPtr& info) :
    Ice::UserExceptionReader(communicator), _info(info)
{
}

IcePy::ExceptionReader::~ExceptionReader() throw()
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    _ex = 0;
}

void
IcePy::ExceptionReader::read(const Ice::InputStreamPtr& is) const
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    is->startException();

    const_cast<PyObjectHandle&>(_ex) = _info->unmarshal(is);

    const_cast<Ice::SlicedDataPtr&>(_slicedData) = is->endException(_info->preserve);
}

bool
IcePy::ExceptionReader::usesClasses() const
{
    return _info->usesClasses;
}

string
IcePy::ExceptionReader::ice_name() const
{
    return _info->id;
}

Ice::UserException*
IcePy::ExceptionReader::ice_clone() const
{
    assert(false);
    return 0;
}

void
IcePy::ExceptionReader::ice_throw() const
{
    throw *this;
}

PyObject*
IcePy::ExceptionReader::getException() const
{
    return _ex.get();
}

Ice::SlicedDataPtr
IcePy::ExceptionReader::getSlicedData() const
{
    return _slicedData;
}

//
// IdResolver
//
string
IcePy::IdResolver::resolve(Ice::Int id) const
{
    CompactIdMap::iterator p = _compactIdMap.find(id);
    if(p != _compactIdMap.end())
    {
        return p->second->id;
    }
    return string();
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
    PyVarObject_HEAD_INIT(0, 0)
    STRCAST("IcePy.TypeInfo"),       /* tp_name */
    sizeof(TypeInfoObject),          /* tp_basicsize */
    0,                               /* tp_itemsize */
    /* methods */
    reinterpret_cast<destructor>(typeInfoDealloc), /* tp_dealloc */
    0,                               /* tp_print */
    0,                               /* tp_getattr */
    0,                               /* tp_setattr */
    0,                               /* tp_reserved */
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
    reinterpret_cast<newfunc>(typeInfoNew), /* tp_new */
    0,                               /* tp_free */
    0,                               /* tp_is_gc */
};

PyTypeObject ExceptionInfoType =
{
    /* The ob_type field must be initialized in the module init function
     * to be portable to Windows without using C++. */
    PyVarObject_HEAD_INIT(0, 0)
    STRCAST("IcePy.ExceptionInfo"),  /* tp_name */
    sizeof(ExceptionInfoObject),     /* tp_basicsize */
    0,                               /* tp_itemsize */
    /* methods */
    reinterpret_cast<destructor>(exceptionInfoDealloc), /* tp_dealloc */
    0,                               /* tp_print */
    0,                               /* tp_getattr */
    0,                               /* tp_setattr */
    0,                               /* tp_reserved */
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
    reinterpret_cast<newfunc>(exceptionInfoNew), /* tp_new */
    0,                               /* tp_free */
    0,                               /* tp_is_gc */
};

static PyNumberMethods UnsetAsNumber =
{
    0,                          /* nb_add */
    0,                          /* nb_subtract */
    0,                          /* nb_multiply */
#if PY_VERSION_HEX < 0x03000000
    0,                          /* nb_divide */
#endif
    0,                          /* nb_remainder */
    0,                          /* nb_divmod */
    0,                          /* nb_power */
    0,                          /* nb_negative */
    0,                          /* nb_positive */
    0,                          /* nb_absolute */
    reinterpret_cast<inquiry>(unsetNonzero), /* nb_nonzero/nb_bool */
};

PyTypeObject UnsetType =
{
    /* The ob_type field must be initialized in the module init function
     * to be portable to Windows without using C++. */
    PyVarObject_HEAD_INIT(&PyType_Type, 0)
    STRCAST("IcePy.UnsetType"),      /* tp_name */
    0,                               /* tp_basicsize */
    0,                               /* tp_itemsize */
    /* methods */
    reinterpret_cast<destructor>(unsetDealloc), /* tp_dealloc */
    0,                               /* tp_print */
    0,                               /* tp_getattr */
    0,                               /* tp_setattr */
    0,                               /* tp_reserved */
    reinterpret_cast<reprfunc>(unsetRepr), /* tp_repr */
    &UnsetAsNumber,                  /* tp_as_number */
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
    0,                               /* tp_new */
    0,                               /* tp_free */
    0,                               /* tp_is_gc */
};

//
// Unset is a singleton, similar to None.
//
PyObject UnsetValue =
{
    _PyObject_EXTRA_INIT
    1, &UnsetType
};

PyObject* Unset = &UnsetValue;

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

    PrimitiveInfoPtr boolType = new PrimitiveInfo(PrimitiveInfo::KindBool);
    PyObjectHandle boolTypeObj = createType(boolType);
    if(PyModule_AddObject(module, STRCAST("_t_bool"), boolTypeObj.get()) < 0)
    {
        return false;
    }
    boolTypeObj.release(); // PyModule_AddObject steals a reference.

    PrimitiveInfoPtr byteType = new PrimitiveInfo(PrimitiveInfo::KindByte);
    PyObjectHandle byteTypeObj = createType(byteType);
    if(PyModule_AddObject(module, STRCAST("_t_byte"), byteTypeObj.get()) < 0)
    {
        return false;
    }
    byteTypeObj.release(); // PyModule_AddObject steals a reference.

    PrimitiveInfoPtr shortType = new PrimitiveInfo(PrimitiveInfo::KindShort);
    PyObjectHandle shortTypeObj = createType(shortType);
    if(PyModule_AddObject(module, STRCAST("_t_short"), shortTypeObj.get()) < 0)
    {
        return false;
    }
    shortTypeObj.release(); // PyModule_AddObject steals a reference.

    PrimitiveInfoPtr intType = new PrimitiveInfo(PrimitiveInfo::KindInt);
    PyObjectHandle intTypeObj = createType(intType);
    if(PyModule_AddObject(module, STRCAST("_t_int"), intTypeObj.get()) < 0)
    {
        return false;
    }
    intTypeObj.release(); // PyModule_AddObject steals a reference.

    PrimitiveInfoPtr longType = new PrimitiveInfo(PrimitiveInfo::KindLong);
    PyObjectHandle longTypeObj = createType(longType);
    if(PyModule_AddObject(module, STRCAST("_t_long"), longTypeObj.get()) < 0)
    {
        return false;
    }
    longTypeObj.release(); // PyModule_AddObject steals a reference.

    PrimitiveInfoPtr floatType = new PrimitiveInfo(PrimitiveInfo::KindFloat);
    PyObjectHandle floatTypeObj = createType(floatType);
    if(PyModule_AddObject(module, STRCAST("_t_float"), floatTypeObj.get()) < 0)
    {
        return false;
    }
    floatTypeObj.release(); // PyModule_AddObject steals a reference.

    PrimitiveInfoPtr doubleType = new PrimitiveInfo(PrimitiveInfo::KindDouble);
    PyObjectHandle doubleTypeObj = createType(doubleType);
    if(PyModule_AddObject(module, STRCAST("_t_double"), doubleTypeObj.get()) < 0)
    {
        return false;
    }
    doubleTypeObj.release(); // PyModule_AddObject steals a reference.

    PrimitiveInfoPtr stringType = new PrimitiveInfo(PrimitiveInfo::KindString);
    PyObjectHandle stringTypeObj = createType(stringType);
    if(PyModule_AddObject(module, STRCAST("_t_string"), stringTypeObj.get()) < 0)
    {
        return false;
    }
    stringTypeObj.release(); // PyModule_AddObject steals a reference.

    if(PyModule_AddObject(module, STRCAST("Unset"), Unset) < 0)
    {
        return false;
    }
    Py_IncRef(Unset); // PyModule_AddObject steals a reference.

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
    TypeInfoObject* obj = typeInfoNew(&TypeInfoType, 0, 0);
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
    ExceptionInfoObject* obj = exceptionInfoNew(&ExceptionInfoType, 0, 0);
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
    PyObject* meta; // Not currently used.
    PyObject* enumerators;
    if(!PyArg_ParseTuple(args, STRCAST("sOOO"), &id, &type, &meta, &enumerators))
    {
        return 0;
    }

    assert(PyTuple_Check(meta));

    EnumInfoPtr info = new EnumInfo(id, type, enumerators);

    return createType(info);
}

extern "C"
PyObject*
IcePy_defineStruct(PyObject*, PyObject* args)
{
    char* id;
    PyObject* type;
    PyObject* meta; // Not currently used.
    PyObject* members;
    if(!PyArg_ParseTuple(args, STRCAST("sOOO"), &id, &type, &meta, &members))
    {
        return 0;
    }

    assert(PyType_Check(type));
    assert(PyTuple_Check(meta));
    assert(PyTuple_Check(members));

    StructInfoPtr info = new StructInfo(id, type, members);

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

    SequenceInfoPtr info = new SequenceInfo(id, meta, elementType);

    return createType(info);
}

extern "C"
PyObject*
IcePy_defineCustom(PyObject*, PyObject* args)
{
    char* id;
    PyObject* type;
    if(!PyArg_ParseTuple(args, STRCAST("sO"), &id, &type))
    {
        return 0;
    }

    CustomInfoPtr info = new CustomInfo(id, type);

    return createType(info);
}

extern "C"
PyObject*
IcePy_defineDictionary(PyObject*, PyObject* args)
{
    char* id;
    PyObject* meta; // Not currently used.
    PyObject* keyType;
    PyObject* valueType;
    if(!PyArg_ParseTuple(args, STRCAST("sOOO"), &id, &meta, &keyType, &valueType))
    {
        return 0;
    }

    assert(PyTuple_Check(meta));

    DictionaryInfoPtr info = new DictionaryInfo(id, keyType, valueType);

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
        info = new ProxyInfo(proxyId);
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
        info = new ProxyInfo(proxyId);
        addProxyInfo(proxyId, info);
    }

    info->define(type);

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
        info = new ClassInfo(id);
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
    int compactId;
    PyObject* meta; // Not currently used.
    int isAbstract;
    int preserve;
    PyObject* base;
    PyObject* interfaces;
    PyObject* members;
    if(!PyArg_ParseTuple(args, STRCAST("sOiOiiOOO"), &id, &type, &compactId, &meta, &isAbstract, &preserve, &base,
                         &interfaces, &members))
    {
        return 0;
    }

    assert(PyTuple_Check(meta));

    //
    // A ClassInfo object will already exist for this id if a forward declaration
    // was encountered, or if the Slice definition is being reloaded. In the latter
    // case, we act as if it hasn't been defined yet.
    //
    ClassInfoPtr info = lookupClassInfo(id);
    if(!info || info->defined)
    {
        info = new ClassInfo(id);
        addClassInfo(id, info);
    }

    info->define(type, compactId, isAbstract ? true : false, preserve ? true : false, base, interfaces, members);

    CompactIdMap::iterator q = _compactIdMap.find(info->compactId);
    if(q != _compactIdMap.end())
    {
        _compactIdMap.erase(q);
    }
    _compactIdMap.insert(CompactIdMap::value_type(info->compactId, info));

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
    int preserve;
    PyObject* base;
    PyObject* members;
    if(!PyArg_ParseTuple(args, STRCAST("sOOiOO"), &id, &type, &meta, &preserve, &base, &members))
    {
        return 0;
    }

    assert(PyExceptionClass_Check(type));
    assert(PyTuple_Check(meta));
    assert(PyTuple_Check(members));

    ExceptionInfoPtr info = new ExceptionInfo;
    info->id = id;

    info->preserve = preserve ? true : false;

    if(base != Py_None)
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
    IceUtilInternal::Output out(ostr);
    PrintObjectHistory history;
    history.index = 0;
    info->print(value, out, &history);

    string str = ostr.str();
    return createString(str);
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

    PyObjectHandle iceType = PyObject_GetAttrString(value, STRCAST("_ice_type"));
    assert(iceType.get());
    ExceptionInfoPtr info = getException(iceType.get());
    assert(info);

    ostringstream ostr;
    IceUtilInternal::Output out(ostr);
    info->print(value, out);

    string str = ostr.str();
    return createString(str);
}
