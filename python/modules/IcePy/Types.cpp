// Copyright (c) ZeroC, Inc.

#ifdef _WIN32
// Prevents windows.h from defining min/max macros.
#    define NOMINMAX
#endif

#include "Types.h"
#include "Current.h"
#include "Ice/DisableWarnings.h"
#include "Ice/InputStream.h"
#include "Ice/LocalExceptions.h"
#include "Ice/OutputStream.h"
#include "Ice/SlicedData.h"
#include "Proxy.h"
#include "Thread.h"
#include "Util.h"

#include <limits>
#include <list>

using namespace std;
using namespace IcePy;
using namespace Ice;
using namespace IceInternal;

using ValueInfoMap = map<string, ValueInfoPtr, std::less<>>;
static ValueInfoMap valueInfoMap;

using ProxyInfoMap = map<string, ProxyInfoPtr, std::less<>>;
static ProxyInfoMap proxyInfoMap;

using ExceptionInfoMap = map<string, ExceptionInfoPtr, std::less<>>;
static ExceptionInfoMap exceptionInfoMap;

namespace
{
    //
    // This exception is raised if the factory specified in a sequence metadata
    // cannot be load or is not valid
    //
    class InvalidSequenceFactoryException
    {
    };
}

namespace IcePy
{
    struct TypeInfoObject
    {
        PyObject_HEAD IcePy::TypeInfoPtr* info;
    };

    struct ExceptionInfoObject
    {
        PyObject_HEAD IcePy::ExceptionInfoPtr* info;
    };

    struct BufferObject
    {
        PyObject_HEAD IcePy::BufferPtr* buffer;
    };

    extern PyTypeObject TypeInfoType;
    extern PyTypeObject ExceptionInfoType;

    bool writeString(PyObject* p, Ice::OutputStream* os)
    {
        if (p == Py_None)
        {
            os->write(string(), false); // Bypass string conversion.
        }
        else if (checkString(p))
        {
            os->write(getString(p), false); // Bypass string conversion.
        }
        else
        {
            assert(false);
        }

        return true;
    }
}

extern "C" TypeInfoObject*
typeInfoNew(PyTypeObject* type, PyObject* /*args*/, PyObject* /*kwds*/)
{
    auto* self = reinterpret_cast<TypeInfoObject*>(type->tp_alloc(type, 0));
    if (!self)
    {
        return nullptr;
    }
    self->info = nullptr;
    return self;
}

extern "C" void
typeInfoDealloc(TypeInfoObject* self)
{
    //
    // Every TypeInfo object is assigned to a "_t_XXX" variable in its enclosing module. Python releases these
    // objects during shutdown, which gives us a chance to release resources and break cyclic references.
    //
    assert(self->info);
    (*self->info)->destroy();
    delete self->info;
    Py_TYPE(self)->tp_free(reinterpret_cast<PyObject*>(self));
}

extern "C" ExceptionInfoObject*
exceptionInfoNew(PyTypeObject* type, PyObject* /*args*/, PyObject* /*kwds*/)
{
    auto* self = reinterpret_cast<ExceptionInfoObject*>(type->tp_alloc(type, 0));
    if (!self)
    {
        return nullptr;
    }
    self->info = nullptr;
    return self;
}

extern "C" void
exceptionInfoDealloc(ExceptionInfoObject* self)
{
    delete self->info;
    Py_TYPE(self)->tp_free(reinterpret_cast<PyObject*>(self));
}

//
// addValueInfo()
//
static void
addValueInfo(string_view id, const ValueInfoPtr& info)
{
    // Do not assert. An application may load statically-translated definitions and then dynamically load duplicate
    // definitions.
    //
    // assert(_valueInfoMap.find(id) == _valueInfoMap.end());
    auto p = valueInfoMap.find(id);
    if (p != valueInfoMap.end())
    {
        valueInfoMap.erase(p);
    }
    valueInfoMap.insert(ValueInfoMap::value_type(id, info));
}

//
// addProxyInfo()
//
static void
addProxyInfo(string_view id, const ProxyInfoPtr& info)
{
    // Do not assert. An application may load statically-translated definitions and then dynamically load duplicate
    // definitions.
    //
    // assert(_valueInfoMap.find(id) == _valueInfoMap.end());
    auto p = proxyInfoMap.find(id);
    if (p != proxyInfoMap.end())
    {
        proxyInfoMap.erase(p);
    }
    proxyInfoMap.insert(ProxyInfoMap::value_type(id, info));
}

//
// lookupProxyInfo()
//
static IcePy::ProxyInfoPtr
lookupProxyInfo(string_view id)
{
    auto p = proxyInfoMap.find(id);
    if (p != proxyInfoMap.end())
    {
        return p->second;
    }
    return nullptr;
}

//
// addExceptionInfo()
//
static void
addExceptionInfo(string_view id, const ExceptionInfoPtr& info)
{
    // Do not assert. An application may load statically-translated definitions and then dynamically load duplicate
    // definitions.
    //
    // assert(_valueInfoMap.find(id) == _valueInfoMap.end());
    exceptionInfoMap.insert(ExceptionInfoMap::value_type(id, info));
}

//
// StreamUtil implementation
//
PyObject* IcePy::StreamUtil::_slicedDataType = nullptr;
PyObject* IcePy::StreamUtil::_sliceInfoType = nullptr;

IcePy::StreamUtil::StreamUtil() = default;

IcePy::StreamUtil::~StreamUtil()
{
    //
    // Make sure we break any cycles among the ValueReaders in preserved slices.
    //
    for (const auto& valueReader : _readers)
    {
        Ice::SlicedDataPtr slicedData = valueReader->getSlicedData();
        for (const auto& sliceInfo : slicedData->slices)
        {
            //
            // Don't just call sliceInfo->instances.clear(), as releasing references
            // to the instances could have unexpected side effects. We exchange
            // the vector into a temporary and then let the temporary fall out
            // of scope.
            //
            vector<std::shared_ptr<Ice::Value>> tmp;
            tmp.swap(sliceInfo->instances);
        }
    }
}

void
IcePy::StreamUtil::add(const ReadValueCallbackPtr& callback)
{
    _callbacks.push_back(callback);
}

void
IcePy::StreamUtil::add(const ValueReaderPtr& reader)
{
    assert(reader->getSlicedData());
    _readers.insert(reader);
}

void
IcePy::StreamUtil::updateSlicedData()
{
    for (const auto& valueReader : _readers)
    {
        setSlicedDataMember(valueReader->getObject(), valueReader->getSlicedData());
    }
}

void
IcePy::StreamUtil::setSlicedDataMember(PyObject* obj, const Ice::SlicedDataPtr& slicedData)
{
    //
    // Create a Python equivalent of the SlicedData object.
    //

    assert(slicedData);

    if (!_slicedDataType)
    {
        _slicedDataType = lookupType("Ice.SlicedData");
        assert(_slicedDataType);
    }
    if (!_sliceInfoType)
    {
        _sliceInfoType = lookupType("Ice.SliceInfo");
        assert(_sliceInfoType);
    }

    PyObjectHandle sd{PyObject_CallObject(_slicedDataType, nullptr)};
    if (!sd.get())
    {
        assert(PyErr_Occurred());
        throw AbortMarshaling();
    }

    PyObjectHandle slices{PyTuple_New(static_cast<Py_ssize_t>(slicedData->slices.size()))};
    if (!slices.get())
    {
        assert(PyErr_Occurred());
        throw AbortMarshaling();
    }

    if (PyObject_SetAttrString(sd.get(), "slices", slices.get()) < 0)
    {
        assert(PyErr_Occurred());
        throw AbortMarshaling();
    }

    //
    // Translate each SliceInfo object into its Python equivalent.
    //
    int i = 0;
    for (auto p = slicedData->slices.begin(); p != slicedData->slices.end(); ++p)
    {
        PyObjectHandle slice{PyObject_CallObject(_sliceInfoType, nullptr)};
        if (!slice.get())
        {
            assert(PyErr_Occurred());
            throw AbortMarshaling();
        }

        // PyTuple_SET_ITEM steals a reference.
        PyTuple_SET_ITEM(slices.get(), i++, Py_NewRef(slice.get()));

        //
        // typeId
        //
        PyObjectHandle typeId{createString((*p)->typeId)};
        if (!typeId.get() || PyObject_SetAttrString(slice.get(), "typeId", typeId.get()) < 0)
        {
            assert(PyErr_Occurred());
            throw AbortMarshaling();
        }

        //
        // compactId
        //
        PyObjectHandle compactId{PyLong_FromLong((*p)->compactId)};
        if (!compactId.get() || PyObject_SetAttrString(slice.get(), "compactId", compactId.get()) < 0)
        {
            assert(PyErr_Occurred());
            throw AbortMarshaling();
        }

        //
        // bytes
        //
        PyObjectHandle bytes;
        if ((*p)->bytes.size() > 0)
        {
            bytes = PyObjectHandle{PyBytes_FromStringAndSize(
                reinterpret_cast<const char*>(&(*p)->bytes[0]),
                static_cast<Py_ssize_t>((*p)->bytes.size()))};
        }
        else
        {
            bytes = PyObjectHandle{PyBytes_FromStringAndSize(nullptr, 0)};
        }
        if (!bytes.get() || PyObject_SetAttrString(slice.get(), "bytes", bytes.get()) < 0)
        {
            assert(PyErr_Occurred());
            throw AbortMarshaling();
        }

        //
        // instances
        //
        PyObjectHandle instances{PyTuple_New(static_cast<Py_ssize_t>((*p)->instances.size()))};
        if (!instances.get() || PyObject_SetAttrString(slice.get(), "instances", instances.get()) < 0)
        {
            assert(PyErr_Occurred());
            throw AbortMarshaling();
        }

        int j = 0;
        for (auto q = (*p)->instances.begin(); q != (*p)->instances.end(); ++q)
        {
            //
            // Each element in the instances list is an instance of ValueReader that wraps a Python object.
            //
            assert(*q);
            auto r = dynamic_pointer_cast<ValueReader>(*q);
            assert(r);
            PyObject* pobj = r->getObject();
            assert(pobj != Py_None); // Should be non-nil.
            // PyTuple_SET_ITEM steals a reference.
            PyTuple_SET_ITEM(instances.get(), j++, Py_NewRef(pobj));
        }

        //
        // hasOptionalMembers
        //
        PyObject* hasOptionalMembers = (*p)->hasOptionalMembers ? Py_True : Py_False;
        if (PyObject_SetAttrString(slice.get(), "hasOptionalMembers", hasOptionalMembers) < 0)
        {
            assert(PyErr_Occurred());
            throw AbortMarshaling();
        }

        //
        // isLastSlice
        //
        PyObject* isLastSlice = (*p)->isLastSlice ? Py_True : Py_False;
        if (PyObject_SetAttrString(slice.get(), "isLastSlice", isLastSlice) < 0)
        {
            assert(PyErr_Occurred());
            throw AbortMarshaling();
        }
    }

    if (PyObject_SetAttrString(obj, "_ice_slicedData", sd.get()) < 0)
    {
        assert(PyErr_Occurred());
        throw AbortMarshaling();
    }
}

//
// Instances of class types may have a data member named _ice_slicedData which is an instance of the Python class
// Ice.SlicedData. This data member is only set for unmarshaled instances.
//
Ice::SlicedDataPtr
IcePy::StreamUtil::getSlicedDataMember(PyObject* obj, ObjectMap* objectMap)
{
    Ice::SlicedDataPtr slicedData;

    if (PyObject_HasAttrString(obj, "_ice_slicedData"))
    {
        PyObjectHandle sd{getAttr(obj, "_ice_slicedData", false)};
        assert(sd.get());

        if (sd.get() != Py_None)
        {
            //
            // The "slices" member is a tuple of Ice.SliceInfo objects.
            //
            PyObjectHandle sl{getAttr(sd.get(), "slices", false)};
            assert(sl.get());
            assert(PyTuple_Check(sl.get()));

            Ice::SliceInfoSeq slices;

            Py_ssize_t sz = PyTuple_GET_SIZE(sl.get());
            for (Py_ssize_t i = 0; i < sz; ++i)
            {
                PyObjectHandle s{Py_NewRef(PyTuple_GET_ITEM(sl.get(), i))};

                PyObjectHandle typeId{getAttr(s.get(), "typeId", false)};
                assert(typeId.get());

                PyObjectHandle compactId{getAttr(s.get(), "compactId", false)};
                assert(compactId.get());

                PyObjectHandle bytes{getAttr(s.get(), "bytes", false)};
                assert(bytes.get());
                char* str;
                Py_ssize_t strsz;
                assert(PyBytes_Check(bytes.get()));
                PyBytes_AsStringAndSize(bytes.get(), &str, &strsz);
                vector<byte> vtmp(reinterpret_cast<byte*>(str), reinterpret_cast<byte*>(str + strsz));

                PyObjectHandle hasOptionalMembers{getAttr(s.get(), "hasOptionalMembers", false)};
                assert(hasOptionalMembers.get());

                PyObjectHandle isLastSlice{getAttr(s.get(), "isLastSlice", false)};
                assert(isLastSlice.get());

                auto info = std::make_shared<Ice::SliceInfo>(
                    getString(typeId.get()),
                    static_cast<int>(PyLong_AsLong(compactId.get())),
                    std::move(vtmp),
                    PyObject_IsTrue(hasOptionalMembers.get()) ? true : false,
                    PyObject_IsTrue(isLastSlice.get()) ? true : false);

                PyObjectHandle instances{getAttr(s.get(), "instances", false)};
                assert(instances.get());
                assert(PyTuple_Check(instances.get()));
                Py_ssize_t osz = PyTuple_GET_SIZE(instances.get());
                for (Py_ssize_t j = 0; j < osz; ++j)
                {
                    PyObject* o = PyTuple_GET_ITEM(instances.get(), j);

                    std::shared_ptr<Ice::Value> writer;

                    auto k = objectMap->find(o);
                    if (k == objectMap->end())
                    {
                        writer = make_shared<ValueWriter>(o, objectMap, nullptr);
                        objectMap->insert(ObjectMap::value_type(o, writer));
                    }
                    else
                    {
                        writer = k->second;
                    }

                    info->instances.push_back(writer);
                }

                slices.push_back(info);
            }

            slicedData = std::make_shared<Ice::SlicedData>(slices);
        }
    }

    return slicedData;
}

//
// UnmarshalCallback implementation.
//
IcePy::UnmarshalCallback::~UnmarshalCallback() = default;

//
// TypeInfo implementation.
//
IcePy::TypeInfo::TypeInfo() = default;

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
IcePy::PrimitiveInfo::PrimitiveInfo(Kind k) : kind(k) {}

string
IcePy::PrimitiveInfo::getId() const
{
    switch (kind)
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
    return {};
}

bool
IcePy::PrimitiveInfo::validate(PyObject* p)
{
    switch (kind)
    {
        case PrimitiveInfo::KindBool:
        {
            int isTrue = PyObject_IsTrue(p);
            if (isTrue < 0)
            {
                return false;
            }
            break;
        }
        case PrimitiveInfo::KindByte:
        {
            long val = PyLong_AsLong(p);

            if (PyErr_Occurred() || val < 0 || val > 255)
            {
                return false;
            }
            break;
        }
        case PrimitiveInfo::KindShort:
        {
            long val = PyLong_AsLong(p);

            if (PyErr_Occurred() || val < SHRT_MIN || val > SHRT_MAX)
            {
                return false;
            }
            break;
        }
        case PrimitiveInfo::KindInt:
        {
            long val = PyLong_AsLong(p);

            if (PyErr_Occurred() || val < INT_MIN || val > INT_MAX)
            {
                return false;
            }
            break;
        }
        case PrimitiveInfo::KindLong:
        {
            PyLong_AsLongLong(p); // Just to see if it raises an error.

            if (PyErr_Occurred())
            {
                return false;
            }

            break;
        }
        case PrimitiveInfo::KindFloat:
        {
            if (!PyFloat_Check(p))
            {
                if (PyLong_Check(p))
                {
                    PyLong_AsDouble(p); // Just to see if it raises an error.
                    if (PyErr_Occurred())
                    {
                        return false;
                    }
                }
                else
                {
                    return false;
                }
            }
            else
            {
                // Ensure double does not exceed maximum float value before casting
                double val = PyFloat_AsDouble(p);
                return (val <= numeric_limits<float>::max() && val >= -numeric_limits<float>::max()) || !isfinite(val);
            }

            break;
        }
        case PrimitiveInfo::KindDouble:
        {
            if (!PyFloat_Check(p))
            {
                if (PyLong_Check(p))
                {
                    PyLong_AsDouble(p); // Just to see if it raises an error.
                    if (PyErr_Occurred())
                    {
                        return false;
                    }
                }
                else
                {
                    return false;
                }
            }

            break;
        }
        case PrimitiveInfo::KindString:
        {
            if (p != Py_None && !checkString(p))
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
    switch (kind)
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
    switch (kind)
    {
        case KindBool:
        case KindByte:
            return Ice::OptionalFormat::F1;
        case KindShort:
            return Ice::OptionalFormat::F2;
        case KindInt:
            return Ice::OptionalFormat::F4;
        case KindLong:
            return Ice::OptionalFormat::F8;
        case KindFloat:
            return Ice::OptionalFormat::F4;
        case KindDouble:
            return Ice::OptionalFormat::F8;
        case KindString:
            return Ice::OptionalFormat::VSize;
    }

    assert(false);
    return Ice::OptionalFormat::F1;
}

void
IcePy::PrimitiveInfo::marshal(PyObject* p, Ice::OutputStream* os, ObjectMap*, bool, const Ice::StringSeq*)
{
    switch (kind)
    {
        case PrimitiveInfo::KindBool:
        {
            int isTrue = PyObject_IsTrue(p);
            if (isTrue < 0)
            {
                assert(false); // validate() should have caught this.
            }
            os->write(isTrue ? true : false);
            break;
        }
        case PrimitiveInfo::KindByte:
        {
            long val = PyLong_AsLong(p);
            assert(!PyErr_Occurred());      // validate() should have caught this.
            assert(val >= 0 && val <= 255); // validate() should have caught this.
            os->write(static_cast<uint8_t>(val));
            break;
        }
        case PrimitiveInfo::KindShort:
        {
            long val = PyLong_AsLong(p);
            assert(!PyErr_Occurred());                  // validate() should have caught this.
            assert(val >= SHRT_MIN && val <= SHRT_MAX); // validate() should have caught this.
            os->write(static_cast<int16_t>(val));
            break;
        }
        case PrimitiveInfo::KindInt:
        {
            long val = PyLong_AsLong(p);
            assert(!PyErr_Occurred());                // validate() should have caught this.
            assert(val >= INT_MIN && val <= INT_MAX); // validate() should have caught this.
            os->write(static_cast<int32_t>(val));
            break;
        }
        case PrimitiveInfo::KindLong:
        {
            int64_t val = PyLong_AsLongLong(p);
            assert(!PyErr_Occurred()); // validate() should have caught this.
            os->write(val);
            break;
        }
        case PrimitiveInfo::KindFloat:
        {
            auto val = static_cast<float>(PyFloat_AsDouble(p)); // Attempts to perform conversion.
            if (PyErr_Occurred())
            {
                throw AbortMarshaling();
            }

            os->write(val);
            break;
        }
        case PrimitiveInfo::KindDouble:
        {
            double val = PyFloat_AsDouble(p); // Attempts to perform conversion.
            if (PyErr_Occurred())
            {
                throw AbortMarshaling();
            }

            os->write(val);
            break;
        }
        case PrimitiveInfo::KindString:
        {
            if (!writeString(p, os))
            {
                assert(PyErr_Occurred());
                throw AbortMarshaling();
            }
            break;
        }
    }
}

void
IcePy::PrimitiveInfo::unmarshal(
    Ice::InputStream* is,
    const UnmarshalCallbackPtr& cb,
    PyObject* target,
    void* closure,
    bool,
    const Ice::StringSeq*)
{
    switch (kind)
    {
        case PrimitiveInfo::KindBool:
        {
            bool b;
            is->read(b);
            if (b)
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
            uint8_t val;
            is->read(val);
            PyObjectHandle p{PyLong_FromLong(val)};
            cb->unmarshaled(p.get(), target, closure);
            break;
        }
        case PrimitiveInfo::KindShort:
        {
            int16_t val;
            is->read(val);
            PyObjectHandle p{PyLong_FromLong(val)};
            cb->unmarshaled(p.get(), target, closure);
            break;
        }
        case PrimitiveInfo::KindInt:
        {
            int32_t val;
            is->read(val);
            PyObjectHandle p{PyLong_FromLong(val)};
            cb->unmarshaled(p.get(), target, closure);
            break;
        }
        case PrimitiveInfo::KindLong:
        {
            int64_t val;
            is->read(val);
            PyObjectHandle p{PyLong_FromLongLong(val)};
            cb->unmarshaled(p.get(), target, closure);
            break;
        }
        case PrimitiveInfo::KindFloat:
        {
            float val;
            is->read(val);
            PyObjectHandle p{PyFloat_FromDouble(val)};
            cb->unmarshaled(p.get(), target, closure);
            break;
        }
        case PrimitiveInfo::KindDouble:
        {
            double val;
            is->read(val);
            PyObjectHandle p{PyFloat_FromDouble(val)};
            cb->unmarshaled(p.get(), target, closure);
            break;
        }
        case PrimitiveInfo::KindString:
        {
            string val;
            is->read(val, false); // Bypass string conversion.
            PyObjectHandle p{createString(val)};
            cb->unmarshaled(p.get(), target, closure);
            break;
        }
    }
}

//
// EnumInfo implementation.
//
IcePy::EnumInfo::EnumInfo(string ident, PyObject* t, PyObject* e) : id(std::move(ident)), pythonType(t)
{
    assert(PyType_Check(t));
    assert(PyDict_Check(e));

    Py_ssize_t pos = 0;
    PyObject* key;
    PyObject* value;
    while (PyDict_Next(e, &pos, &key, &value))
    {
        assert(PyLong_Check(key));
        const auto val = static_cast<int32_t>(PyLong_AsLong(key));
        assert(enumerators.find(val) == enumerators.end());

        assert(PyObject_IsInstance(value, t));
        const_cast<EnumeratorMap&>(enumerators)[val] = Py_NewRef(value);

        if (val > maxValue)
        {
            const_cast<int32_t&>(maxValue) = val;
        }
    }

    // Enumerators cannot be empty.
    assert(!enumerators.empty());
}

string
IcePy::EnumInfo::getId() const
{
    return id;
}

bool
IcePy::EnumInfo::validate(PyObject* val)
{
    return PyObject_IsInstance(val, pythonType) == 1;
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
    return Ice::OptionalFormat::Size;
}

void
IcePy::EnumInfo::marshal(PyObject* p, Ice::OutputStream* os, ObjectMap*, bool /*optional*/, const Ice::StringSeq*)
{
    // Validate value.
    const int32_t val = valueForEnumerator(p);
    if (val < 0)
    {
        assert(PyErr_Occurred());
        throw AbortMarshaling();
    }

    os->writeEnum(val, maxValue);
}

void
IcePy::EnumInfo::unmarshal(
    Ice::InputStream* is,
    const UnmarshalCallbackPtr& cb,
    PyObject* target,
    void* closure,
    bool,
    const Ice::StringSeq*)
{
    int32_t val = is->readEnum(maxValue);

    PyObjectHandle p{enumeratorForValue(val)};
    if (!p.get())
    {
        ostringstream ostr;
        ostr << "enumerator " << val << " is out of range for enum " << id;
        setPythonException(make_exception_ptr(Ice::MarshalException{__FILE__, __LINE__, ostr.str()}));
        throw AbortMarshaling();
    }

    cb->unmarshaled(p.get(), target, closure);
}

void
IcePy::EnumInfo::destroy()
{
    const_cast<EnumeratorMap&>(enumerators).clear();
}

int32_t
IcePy::EnumInfo::valueForEnumerator(PyObject* p) const
{
    assert(PyObject_IsInstance(p, pythonType) == 1);

    PyObjectHandle v{PyObject_GetAttrString(p, "value")};
    if (!v.get())
    {
        assert(PyErr_Occurred());
        return -1;
    }
    if (!PyLong_Check(v.get()))
    {
        PyErr_Format(PyExc_ValueError, "value for enum %s is not an int", id.c_str());
        return -1;
    }
    const auto val = static_cast<int32_t>(PyLong_AsLong(v.get()));
    if (enumerators.find(val) == enumerators.end())
    {
        PyErr_Format(PyExc_ValueError, "illegal value %d for enum %s", val, id.c_str());
        return -1;
    }

    return val;
}

PyObject*
IcePy::EnumInfo::enumeratorForValue(int32_t v) const
{
    auto p = enumerators.find(v);
    if (p == enumerators.end())
    {
        return nullptr;
    }
    return Py_NewRef(p->second.get());
}

//
// DataMember implementation.
//
void
IcePy::DataMember::unmarshaled(PyObject* val, PyObject* target, void*)
{
    if (PyObject_SetAttrString(target, const_cast<char*>(name.c_str()), val) < 0)
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
    for (Py_ssize_t i = 0; i < sz; ++i)
    {
        PyObject* m{PyTuple_GET_ITEM(members, i)};
        assert(PyTuple_Check(m));
        assert(PyTuple_GET_SIZE(m) == (allowOptional ? 5 : 3));

        PyObject* name{PyTuple_GET_ITEM(m, 0)}; // Member name.
        assert(checkString(name));
        PyObject* meta{PyTuple_GET_ITEM(m, 1)}; // Member metadata.
        assert(PyTuple_Check(meta));
        PyObject* t{PyTuple_GET_ITEM(m, 2)}; // Member type.

        PyObject* opt{nullptr};
        PyObject* tag{nullptr};
        if (allowOptional)
        {
            opt = PyTuple_GET_ITEM(m, 3); // Optional?
            tag = PyTuple_GET_ITEM(m, 4);
            assert(PyLong_Check(tag));
        }

        DataMemberPtr member = make_shared<DataMember>();
        member->name = getString(name);
        [[maybe_unused]] bool b = tupleToStringSeq(meta, member->metadata);
        assert(b);
        member->type = getType(t);
        if (allowOptional)
        {
            member->optional = PyObject_IsTrue(opt) == 1;
            member->tag = static_cast<int>(PyLong_AsLong(tag));
        }
        else
        {
            member->optional = false;
            member->tag = 0;
        }

        if (member->optional)
        {
            optList.push_back(member);
        }
        else
        {
            reqMembers.push_back(member);
        }
    }

    if (allowOptional)
    {
        class SortFn
        {
        public:
            static bool compare(const DataMemberPtr& lhs, const DataMemberPtr& rhs) { return lhs->tag < rhs->tag; }
        };

        optList.sort(SortFn::compare);
        copy(optList.begin(), optList.end(), back_inserter(optMembers));
    }
}

//
// StructInfo implementation.
//
IcePy::StructInfo::StructInfo(string ident, PyObject* t, PyObject* m) : id(std::move(ident)), pythonType(t)
{
    assert(PyType_Check(t));
    assert(PyTuple_Check(m));

    DataMemberList opt;
    convertDataMembers(m, const_cast<DataMemberList&>(members), opt, false);
    assert(opt.empty());

    _variableLength = false;
    _wireSize = 0;
    for (auto p = members.begin(); p != members.end(); ++p)
    {
        if (!_variableLength && (*p)->type->variableLength())
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
    return PyObject_IsInstance(val, pythonType) == 1;
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
    return _variableLength ? Ice::OptionalFormat::FSize : Ice::OptionalFormat::VSize;
}

bool
IcePy::StructInfo::usesClasses() const
{
    for (const auto& dm : members)
    {
        if (dm->type->usesClasses())
        {
            return true;
        }
    }

    return false;
}

void
IcePy::StructInfo::marshal(
    PyObject* p,
    Ice::OutputStream* os,
    ObjectMap* objectMap,
    bool optional,
    const Ice::StringSeq*)
{
    assert(PyObject_IsInstance(p, pythonType) == 1); // validate() should have caught this.

    Ice::OutputStream::size_type sizePos = 0;
    if (optional)
    {
        if (_variableLength)
        {
            sizePos = os->startSize();
        }
        else
        {
            os->writeSize(_wireSize);
        }
    }

    for (auto q = members.begin(); q != members.end(); ++q)
    {
        const DataMemberPtr& member = *q;
        char* memberName = const_cast<char*>(member->name.c_str());
        PyObjectHandle attr{getAttr(p, member->name, true)};
        if (!attr.get())
        {
            PyErr_Format(
                PyExc_AttributeError,
                "no member '%s' found in %s value",
                memberName,
                const_cast<char*>(id.c_str()));
            throw AbortMarshaling();
        }
        if (!member->type->validate(attr.get()))
        {
            PyErr_Format(
                PyExc_ValueError,
                "invalid value for %s member '%s'",
                const_cast<char*>(id.c_str()),
                memberName);
            throw AbortMarshaling();
        }
        member->type->marshal(attr.get(), os, objectMap, false, &member->metadata);
    }

    if (optional && _variableLength)
    {
        os->endSize(sizePos);
    }
}

void
IcePy::StructInfo::unmarshal(
    Ice::InputStream* is,
    const UnmarshalCallbackPtr& cb,
    PyObject* target,
    void* closure,
    bool optional,
    const Ice::StringSeq*)
{
    PyObjectHandle p{instantiate(pythonType)};
    if (!p.get())
    {
        assert(PyErr_Occurred());
        throw AbortMarshaling();
    }

    if (optional)
    {
        if (_variableLength)
        {
            is->skip(4);
        }
        else
        {
            is->skipSize();
        }
    }

    for (auto q = members.begin(); q != members.end(); ++q)
    {
        const DataMemberPtr& member = *q;
        member->type->unmarshal(is, member, p.get(), nullptr, false, &member->metadata);
    }

    cb->unmarshaled(p.get(), target, closure);
}

void
IcePy::StructInfo::destroy()
{
    const_cast<DataMemberList&>(members).clear();
}

PyObject*
IcePy::StructInfo::instantiate(PyObject* pythonType)
{
    PyObjectHandle emptyArgs{PyTuple_New(0)};
    auto* type = reinterpret_cast<PyTypeObject*>(pythonType);
    return type->tp_new(type, emptyArgs.get(), nullptr);
}

//
// SequenceInfo implementation.
//
IcePy::SequenceInfo::SequenceInfo(string ident, PyObject* m, PyObject* t) : id(std::move(ident))
{
    assert(PyTuple_Check(m));

    vector<string> metadata;
    [[maybe_unused]] bool b = tupleToStringSeq(m, metadata);
    assert(b);

    const_cast<SequenceMappingPtr&>(mapping) = make_shared<SequenceMapping>(metadata);
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
    return elementType->variableLength() ? Ice::OptionalFormat::FSize : Ice::OptionalFormat::VSize;
}

bool
IcePy::SequenceInfo::usesClasses() const
{
    return elementType->usesClasses();
}

void
IcePy::SequenceInfo::marshal(
    PyObject* p,
    Ice::OutputStream* os,
    ObjectMap* objectMap,
    bool optional,
    const Ice::StringSeq* /*metadata*/)
{
    auto pi = dynamic_pointer_cast<PrimitiveInfo>(elementType);

    Ice::OutputStream::size_type sizePos = 0;
    if (optional)
    {
        if (elementType->variableLength())
        {
            sizePos = os->startSize();
        }
        else if (elementType->wireSize() > 1)
        {
            //
            // Determine the sequence size.
            //
            Py_ssize_t sz = 0;
            if (p != Py_None)
            {
                Py_buffer pybuf;
                if (pi && PyObject_GetBuffer(p, &pybuf, PyBUF_SIMPLE | PyBUF_FORMAT) == 0)
                {
                    // Strings are handled as variable length types above.
                    assert(pi->kind != PrimitiveInfo::KindString);
                    sz = pybuf.len;
                    PyBuffer_Release(&pybuf);
                }
                else
                {
                    PyErr_Clear(); // PyObject_GetBuffer sets an exception on failure.

                    PyObjectHandle fs;
                    if (pi)
                    {
                        fs = PyObjectHandle{getSequence(pi, p)};
                    }
                    else
                    {
                        fs = PyObjectHandle{PySequence_Fast(p, "expected a sequence value")};
                    }
                    if (!fs.get())
                    {
                        assert(PyErr_Occurred());
                        return;
                    }
                    sz = PySequence_Fast_GET_SIZE(fs.get());
                }
            }

            const auto isz = static_cast<int32_t>(sz);
            os->writeSize(isz == 0 ? 1 : isz * elementType->wireSize() + (isz > 254 ? 5 : 1));
        }
    }

    if (p == Py_None)
    {
        os->writeSize(0);
    }
    else if (pi)
    {
        marshalPrimitiveSequence(pi, p, os);
    }
    else
    {
        PyObjectHandle fastSeq{PySequence_Fast(p, "expected a sequence value")};
        if (!fastSeq.get())
        {
            return;
        }

        Py_ssize_t sz = PySequence_Fast_GET_SIZE(fastSeq.get());
        os->writeSize(static_cast<int>(sz));
        for (Py_ssize_t i = 0; i < sz; ++i)
        {
            PyObject* item = PySequence_Fast_GET_ITEM(fastSeq.get(), i);
            if (!item)
            {
                assert(PyErr_Occurred());
                throw AbortMarshaling();
            }
            if (!elementType->validate(item))
            {
                PyErr_Format(
                    PyExc_ValueError,
                    "invalid value for element %d of '%s'",
                    static_cast<int>(i),
                    const_cast<char*>(id.c_str()));
                throw AbortMarshaling();
            }
            elementType->marshal(item, os, objectMap, false);
        }
    }

    if (optional && elementType->variableLength())
    {
        os->endSize(sizePos);
    }
}

void
IcePy::SequenceInfo::unmarshal(
    Ice::InputStream* is,
    const UnmarshalCallbackPtr& cb,
    PyObject* target,
    void* closure,
    bool optional,
    const Ice::StringSeq* metadata)
{
    if (optional)
    {
        if (elementType->variableLength())
        {
            is->skip(4);
        }
        else if (elementType->wireSize() > 1)
        {
            is->skipSize();
        }
    }

    //
    // Determine the mapping to use for this sequence. Highest priority is given
    // to the metadata argument, otherwise we use the mapping of the sequence
    // definition.
    //
    SequenceMappingPtr sm;
    if (metadata)
    {
        SequenceMapping::Type type;
        if (!SequenceMapping::getType(*metadata, type) || type == mapping->type)
        {
            sm = mapping;
        }
        else
        {
            try
            {
                sm = make_shared<SequenceMapping>(type, *metadata);
            }
            catch (const InvalidSequenceFactoryException&)
            {
                throw AbortMarshaling();
            }
        }
    }
    else
    {
        sm = mapping;
    }

    auto pi = dynamic_pointer_cast<PrimitiveInfo>(elementType);
    if (pi)
    {
        unmarshalPrimitiveSequence(pi, is, cb, target, closure, sm);
        return;
    }

    int32_t sz = is->readSize();
    PyObjectHandle result{sm->createContainer(sz)};

    if (!result.get())
    {
        assert(PyErr_Occurred());
        throw AbortMarshaling();
    }

    for (int32_t i = 0; i < sz; ++i)
    {
        // TODO: rework this code. It appears the pointer is always casted back to an integer but this is not acceptable
        // code.
        void* cl = reinterpret_cast<void*>(static_cast<Py_ssize_t>(i)); // NOLINT(performance-no-int-to-ptr)
        elementType->unmarshal(is, sm, result.get(), cl, false);
    }

    cb->unmarshaled(result.get(), target, closure);
}

void
IcePy::SequenceInfo::destroy()
{
    const_cast<TypeInfoPtr&>(elementType) = nullptr;
}

PyObject*
IcePy::SequenceInfo::getSequence(const PrimitiveInfoPtr& pi, PyObject* p)
{
    PyObjectHandle fs;

    if (pi->kind == PrimitiveInfo::KindByte)
    {
        //
        // For sequence<byte>, accept a bytes object or a sequence.
        //
        if (!PyBytes_Check(p))
        {
            fs = PyObjectHandle{PySequence_Fast(p, "expected a bytes, sequence, or buffer value")};
        }
    }
    else
    {
        fs = PyObjectHandle{PySequence_Fast(p, "expected a sequence or buffer value")};
    }

    return fs.release();
}

void
IcePy::SequenceInfo::marshalPrimitiveSequence(const PrimitiveInfoPtr& pi, PyObject* p, Ice::OutputStream* os)
{
    //
    // For most types, we accept an object that implements the buffer protocol
    // (this includes the array.array type).
    //
    if (pi->kind != PrimitiveInfo::KindString)
    {
        //
        // With Python 3 and greater we marshal sequences of primitive types using the new
        // buffer protocol when possible, for older versions we use the old buffer protocol.
        //
        Py_buffer pybuf;
        if (PyObject_GetBuffer(p, &pybuf, PyBUF_SIMPLE | PyBUF_FORMAT) == 0)
        {
            static const int itemsize[] = {
                1, // KindBool,
                1, // KindByte,
                2, // KindShort,
                4, // KindInt,
                8, // KindLong,
                4, // KindFloat,
                8, // KindDouble
            };

            static const char* itemtype[] = {
                "bool",   // KindBool,
                "byte",   // KindByte,
                "short",  // KindShort,
                "int",    // KindInt,
                "long",   // KindLong,
                "float",  // KindFloat,
                "double", // KindDouble
            };

            if (pi->kind != PrimitiveInfo::KindByte)
            {
#ifdef WORDS_BIGENDIAN
                if (pybuf.format != 0 && pybuf.format[0] == '<')
                {
                    PyErr_Format(
                        PyExc_ValueError,
                        "sequence buffer byte order doesn't match the platform native byte-order "
                        "`big-endian'");
                    PyBuffer_Release(&pybuf);
                    throw AbortMarshaling();
                }
#else
                if (pybuf.format && (pybuf.format[0] == '>' || pybuf.format[0] == '!'))
                {
                    PyErr_Format(
                        PyExc_ValueError,
                        "sequence buffer byte order doesn't match the platform native byte-order "
                        "'little-endian'");
                    PyBuffer_Release(&pybuf);
                    throw AbortMarshaling();
                }
#endif
                if (pybuf.itemsize != itemsize[pi->kind])
                {
                    PyErr_Format(
                        PyExc_ValueError,
                        "sequence item size doesn't match the size of the sequence type '%s'",
                        itemtype[pi->kind]);
                    PyBuffer_Release(&pybuf);
                    throw AbortMarshaling();
                }
            }
            const auto* b = reinterpret_cast<const uint8_t*>(pybuf.buf);
            Py_ssize_t sz = pybuf.len;

            switch (pi->kind)
            {
                case PrimitiveInfo::KindBool:
                {
                    os->write(reinterpret_cast<const bool*>(b), reinterpret_cast<const bool*>(b + sz));
                    break;
                }
                case PrimitiveInfo::KindByte:
                {
                    os->write(reinterpret_cast<const uint8_t*>(b), reinterpret_cast<const uint8_t*>(b + sz));
                    break;
                }
                case PrimitiveInfo::KindShort:
                {
                    os->write(reinterpret_cast<const int16_t*>(b), reinterpret_cast<const int16_t*>(b + sz));
                    break;
                }
                case PrimitiveInfo::KindInt:
                {
                    os->write(reinterpret_cast<const int32_t*>(b), reinterpret_cast<const int32_t*>(b + sz));
                    break;
                }
                case PrimitiveInfo::KindLong:
                {
                    os->write(reinterpret_cast<const int64_t*>(b), reinterpret_cast<const int64_t*>(b + sz));
                    break;
                }
                case PrimitiveInfo::KindFloat:
                {
                    os->write(reinterpret_cast<const float*>(b), reinterpret_cast<const float*>(b + sz));
                    break;
                }
                case PrimitiveInfo::KindDouble:
                {
                    os->write(reinterpret_cast<const double*>(b), reinterpret_cast<const double*>(b + sz));
                    break;
                }
                default:
                {
                    assert(false);
                }
            }
            PyBuffer_Release(&pybuf);
            return;
        }
        else
        {
            PyErr_Clear(); // PyObject_GetBuffer sets an exception on failure.
        }
    }

    PyObjectHandle fs{getSequence(pi, p)};
    if (!fs.get())
    {
        assert(PyErr_Occurred());
        return;
    }

    Py_ssize_t sz = 0;
    switch (pi->kind)
    {
        case PrimitiveInfo::KindBool:
        {
            sz = PySequence_Fast_GET_SIZE(fs.get());
            Ice::BoolSeq seq(static_cast<size_t>(sz));
            for (Py_ssize_t i = 0; i < sz; ++i)
            {
                PyObject* item = PySequence_Fast_GET_ITEM(fs.get(), i);
                if (!item)
                {
                    assert(PyErr_Occurred());
                    throw AbortMarshaling();
                }
                int isTrue = PyObject_IsTrue(item);
                if (isTrue < 0)
                {
                    PyErr_Format(
                        PyExc_ValueError,
                        "invalid value for element %d of sequence<bool>",
                        static_cast<int>(i));
                    throw AbortMarshaling();
                }
                seq[static_cast<size_t>(i)] = isTrue ? true : false;
            }
            os->write(seq);
            break;
        }
        case PrimitiveInfo::KindByte:
        {
            if (!fs.get())
            {
                assert(PyBytes_Check(p));
                char* str;
                PyBytes_AsStringAndSize(p, &str, &sz);
                os->write(reinterpret_cast<const byte*>(str), reinterpret_cast<const byte*>(str + sz));
            }
            else
            {
                sz = PySequence_Fast_GET_SIZE(fs.get());
                Ice::ByteSeq seq(static_cast<size_t>(sz));
                for (Py_ssize_t i = 0; i < sz; ++i)
                {
                    PyObject* item = PySequence_Fast_GET_ITEM(fs.get(), i);
                    if (!item)
                    {
                        assert(PyErr_Occurred());
                        throw AbortMarshaling();
                    }

                    long val = PyLong_AsLong(item);

                    if (PyErr_Occurred() || val < 0 || val > 255)
                    {
                        PyErr_Format(
                            PyExc_ValueError,
                            "invalid value for element %d of sequence<byte>",
                            static_cast<int>(i));
                        throw AbortMarshaling();
                    }
                    seq[static_cast<size_t>(i)] = static_cast<byte>(val);
                }
                os->write(seq);
            }
            break;
        }
        case PrimitiveInfo::KindShort:
        {
            sz = PySequence_Fast_GET_SIZE(fs.get());
            Ice::ShortSeq seq(static_cast<size_t>(sz));
            for (Py_ssize_t i = 0; i < sz; ++i)
            {
                PyObject* item = PySequence_Fast_GET_ITEM(fs.get(), i);
                if (!item)
                {
                    assert(PyErr_Occurred());
                    throw AbortMarshaling();
                }

                long val = PyLong_AsLong(item);

                if (PyErr_Occurred() || val < SHRT_MIN || val > SHRT_MAX)
                {
                    PyErr_Format(
                        PyExc_ValueError,
                        "invalid value for element %d of sequence<short>",
                        static_cast<int>(i));
                    throw AbortMarshaling();
                }
                seq[static_cast<size_t>(i)] = static_cast<int16_t>(val);
            }
            os->write(seq);
            break;
        }
        case PrimitiveInfo::KindInt:
        {
            sz = PySequence_Fast_GET_SIZE(fs.get());
            Ice::IntSeq seq(static_cast<size_t>(sz));
            for (Py_ssize_t i = 0; i < sz; ++i)
            {
                PyObject* item = PySequence_Fast_GET_ITEM(fs.get(), i);
                if (!item)
                {
                    assert(PyErr_Occurred());
                    throw AbortMarshaling();
                }

                long val = PyLong_AsLong(item);

                if (PyErr_Occurred() || val < INT_MIN || val > INT_MAX)
                {
                    PyErr_Format(
                        PyExc_ValueError,
                        "invalid value for element %d of sequence<int>",
                        static_cast<int>(i));
                    throw AbortMarshaling();
                }
                seq[static_cast<size_t>(i)] = static_cast<int32_t>(val);
            }
            os->write(seq);
            break;
        }
        case PrimitiveInfo::KindLong:
        {
            sz = PySequence_Fast_GET_SIZE(fs.get());
            Ice::LongSeq seq(static_cast<size_t>(sz));
            for (Py_ssize_t i = 0; i < sz; ++i)
            {
                PyObject* item = PySequence_Fast_GET_ITEM(fs.get(), i);
                if (!item)
                {
                    assert(PyErr_Occurred());
                    throw AbortMarshaling();
                }

                int64_t val = PyLong_AsLongLong(item);

                if (PyErr_Occurred())
                {
                    PyErr_Format(
                        PyExc_ValueError,
                        "invalid value for element %d of sequence<long>",
                        static_cast<int>(i));
                    throw AbortMarshaling();
                }
                seq[static_cast<size_t>(i)] = val;
            }
            os->write(seq);
            break;
        }
        case PrimitiveInfo::KindFloat:
        {
            sz = PySequence_Fast_GET_SIZE(fs.get());
            Ice::FloatSeq seq(static_cast<size_t>(sz));
            for (Py_ssize_t i = 0; i < sz; ++i)
            {
                PyObject* item = PySequence_Fast_GET_ITEM(fs.get(), i);
                if (!item)
                {
                    assert(PyErr_Occurred());
                    throw AbortMarshaling();
                }

                auto val = static_cast<float>(PyFloat_AsDouble(item));
                if (PyErr_Occurred())
                {
                    PyErr_Format(
                        PyExc_ValueError,
                        "invalid value for element %d of sequence<float>",
                        static_cast<int>(i));
                    throw AbortMarshaling();
                }

                seq[static_cast<size_t>(i)] = val;
            }
            os->write(seq);
            break;
        }
        case PrimitiveInfo::KindDouble:
        {
            sz = PySequence_Fast_GET_SIZE(fs.get());
            Ice::DoubleSeq seq(static_cast<size_t>(sz));
            for (Py_ssize_t i = 0; i < sz; ++i)
            {
                PyObject* item = PySequence_Fast_GET_ITEM(fs.get(), i);
                if (!item)
                {
                    assert(PyErr_Occurred());
                    throw AbortMarshaling();
                }

                double val = PyFloat_AsDouble(item);
                if (PyErr_Occurred())
                {
                    PyErr_Format(
                        PyExc_ValueError,
                        "invalid value for element %d of sequence<double>",
                        static_cast<int>(i));
                    throw AbortMarshaling();
                }

                seq[static_cast<size_t>(i)] = val;
            }
            os->write(seq);
            break;
        }
        case PrimitiveInfo::KindString:
        {
            sz = PySequence_Fast_GET_SIZE(fs.get());
            os->writeSize(static_cast<int>(sz));
            for (Py_ssize_t i = 0; i < sz; ++i)
            {
                PyObject* item = PySequence_Fast_GET_ITEM(fs.get(), i);
                if (!item)
                {
                    assert(PyErr_Occurred());
                    throw AbortMarshaling();
                }

                if (item != Py_None && !checkString(item))
                {
                    PyErr_Format(
                        PyExc_ValueError,
                        "invalid value for element %d of sequence<string>",
                        static_cast<int>(i));
                    throw AbortMarshaling();
                }

                if (!writeString(item, os))
                {
                    assert(PyErr_Occurred());
                    throw AbortMarshaling();
                }
            }
            break;
        }
    }
}

PyObject*
IcePy::SequenceInfo::createSequenceFromMemory(
    const SequenceMappingPtr& sm,
    const char* buffer,
    Py_ssize_t size,
    BuiltinType type)
{
    PyObjectHandle memoryView;
    if (size > 0)
    {
        memoryView = PyObjectHandle{PyMemoryView_FromMemory(const_cast<char*>(buffer), size, PyBUF_READ)};
        if (!memoryView.get())
        {
            assert(PyErr_Occurred());
            throw AbortMarshaling();
        }
    }

    PyObjectHandle builtinType{PyLong_FromLong(static_cast<int>(type))};
    if (!builtinType.get())
    {
        assert(PyErr_Occurred());
        throw AbortMarshaling();
    }

    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    PyObjectHandle args{PyTuple_New(2)};
    PyTuple_SET_ITEM(args.get(), 0, memoryView.get() ? memoryView.release() : Py_None);
    PyTuple_SET_ITEM(args.get(), 1, builtinType.release());
    PyObjectHandle result{PyObject_Call(sm->factory, args.get(), nullptr)};

    if (!result.get())
    {
        assert(PyErr_Occurred());
        throw AbortMarshaling();
    }
    else if (result.get() == Py_None)
    {
        PyErr_Format(PyExc_ValueError, "invalid container return from factory");
        throw AbortMarshaling();
    }
    return result.release();
}

void
IcePy::SequenceInfo::unmarshalPrimitiveSequence(
    const PrimitiveInfoPtr& pi,
    Ice::InputStream* is,
    const UnmarshalCallbackPtr& cb,
    PyObject* target,
    void* closure,
    const SequenceMappingPtr& sm)
{
    PyObjectHandle result;

    switch (pi->kind)
    {
        case PrimitiveInfo::KindBool:
        {
            pair<const bool*, const bool*> p;
            is->read(p);
            int sz = static_cast<int>(p.second - p.first);
            if (sm->factory)
            {
                const char* data = reinterpret_cast<const char*>(p.first);
                result = PyObjectHandle{createSequenceFromMemory(sm, data, sz, BuiltinTypeBool)};
            }
            else
            {
                result = PyObjectHandle{sm->createContainer(sz)};
                if (!result.get())
                {
                    assert(PyErr_Occurred());
                    throw AbortMarshaling();
                }

                for (int i = 0; i < sz; ++i)
                {
                    sm->setItem(result.get(), i, p.first[i] ? Py_True : Py_False);
                }
            }
            break;
        }
        case PrimitiveInfo::KindByte:
        {
            pair<const uint8_t*, const uint8_t*> p;
            is->read(p);
            int sz = static_cast<int>(p.second - p.first);
            if (sm->factory)
            {
                result = PyObjectHandle{
                    createSequenceFromMemory(sm, reinterpret_cast<const char*>(p.first), sz, BuiltinTypeByte)};
            }
            else if (sm->type == SequenceMapping::SEQ_DEFAULT)
            {
                result = PyObjectHandle{PyBytes_FromStringAndSize(reinterpret_cast<const char*>(p.first), sz)};
                if (!result.get())
                {
                    assert(PyErr_Occurred());
                    throw AbortMarshaling();
                }
            }
            else
            {
                result = PyObjectHandle{sm->createContainer(sz)};
                if (!result.get())
                {
                    assert(PyErr_Occurred());
                    throw AbortMarshaling();
                }

                for (int i = 0; i < sz; ++i)
                {
                    PyObjectHandle item{PyLong_FromLong(p.first[i])};
                    if (!item.get())
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
            pair<const int16_t*, const int16_t*> p;
            is->read(p);
            int sz = static_cast<int>(p.second - p.first);
            if (sm->factory)
            {
                const char* data = reinterpret_cast<const char*>(p.first);
                result = PyObjectHandle{createSequenceFromMemory(sm, data, sz * 2, BuiltinTypeShort)};
            }
            else
            {
                result = PyObjectHandle{sm->createContainer(sz)};
                if (!result.get())
                {
                    assert(PyErr_Occurred());
                    throw AbortMarshaling();
                }

                for (int i = 0; i < sz; ++i)
                {
                    PyObjectHandle item{PyLong_FromLong(p.first[i])};
                    if (!item.get())
                    {
                        assert(PyErr_Occurred());
                        throw AbortMarshaling();
                    }
                    sm->setItem(result.get(), i, item.get());
                }
            }
            break;
        }
        case PrimitiveInfo::KindInt:
        {
            pair<const int32_t*, const int32_t*> p;
            is->read(p);
            int sz = static_cast<int>(p.second - p.first);
            if (sm->factory)
            {
                const char* data = reinterpret_cast<const char*>(p.first);
                result = PyObjectHandle{createSequenceFromMemory(sm, data, sz * 4, BuiltinTypeInt)};
            }
            else
            {
                result = PyObjectHandle{sm->createContainer(sz)};
                if (!result.get())
                {
                    assert(PyErr_Occurred());
                    throw AbortMarshaling();
                }
                for (int i = 0; i < sz; ++i)
                {
                    PyObjectHandle item{PyLong_FromLong(p.first[i])};
                    if (!item.get())
                    {
                        assert(PyErr_Occurred());
                        throw AbortMarshaling();
                    }
                    sm->setItem(result.get(), i, item.get());
                }
            }
            break;
        }
        case PrimitiveInfo::KindLong:
        {
            pair<const int64_t*, const int64_t*> p;
            is->read(p);
            int sz = static_cast<int>(p.second - p.first);
            if (sm->factory)
            {
                const char* data = reinterpret_cast<const char*>(p.first);
                result = PyObjectHandle{createSequenceFromMemory(sm, data, sz * 8, BuiltinTypeLong)};
            }
            else
            {
                result = PyObjectHandle{sm->createContainer(sz)};
                if (!result.get())
                {
                    assert(PyErr_Occurred());
                    throw AbortMarshaling();
                }

                for (int i = 0; i < sz; ++i)
                {
                    PyObjectHandle item{PyLong_FromLongLong(p.first[i])};
                    if (!item.get())
                    {
                        assert(PyErr_Occurred());
                        throw AbortMarshaling();
                    }
                    sm->setItem(result.get(), i, item.get());
                }
            }
            break;
        }
        case PrimitiveInfo::KindFloat:
        {
            pair<const float*, const float*> p;
            is->read(p);
            int sz = static_cast<int>(p.second - p.first);
            if (sm->factory)
            {
                const char* data = reinterpret_cast<const char*>(p.first);
                result = PyObjectHandle{createSequenceFromMemory(sm, data, sz * 4, BuiltinTypeFloat)};
            }
            else
            {
                result = PyObjectHandle{sm->createContainer(sz)};
                if (!result.get())
                {
                    assert(PyErr_Occurred());
                    throw AbortMarshaling();
                }

                for (int i = 0; i < sz; ++i)
                {
                    PyObjectHandle item{PyFloat_FromDouble(p.first[i])};
                    if (!item.get())
                    {
                        assert(PyErr_Occurred());
                        throw AbortMarshaling();
                    }
                    sm->setItem(result.get(), i, item.get());
                }
            }
            break;
        }
        case PrimitiveInfo::KindDouble:
        {
            pair<const double*, const double*> p;
            is->read(p);
            int sz = static_cast<int>(p.second - p.first);
            if (sm->factory)
            {
                const char* data = reinterpret_cast<const char*>(p.first);
                result = PyObjectHandle{createSequenceFromMemory(sm, data, sz * 8, BuiltinTypeDouble)};
            }
            else
            {
                result = PyObjectHandle{sm->createContainer(sz)};
                if (!result.get())
                {
                    assert(PyErr_Occurred());
                    throw AbortMarshaling();
                }

                for (int i = 0; i < sz; ++i)
                {
                    PyObjectHandle item{PyFloat_FromDouble(p.first[i])};
                    if (!item.get())
                    {
                        assert(PyErr_Occurred());
                        throw AbortMarshaling();
                    }
                    sm->setItem(result.get(), i, item.get());
                }
            }
            break;
        }
        case PrimitiveInfo::KindString:
        {
            Ice::StringSeq seq;
            is->read(seq, false); // Bypass string conversion.
            int sz = static_cast<int>(seq.size());
            result = PyObjectHandle{sm->createContainer(sz)};
            if (!result.get())
            {
                assert(PyErr_Occurred());
                throw AbortMarshaling();
            }

            for (int i = 0; i < sz; ++i)
            {
                PyObjectHandle item{createString(seq[static_cast<size_t>(i)])};
                if (!item.get())
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
IcePy::SequenceInfo::SequenceMapping::getType(const Ice::StringSeq& metadata, Type& t)
{
    for (const auto& name : metadata)
    {
        if (name == "python:tuple")
        {
            t = SEQ_TUPLE;
            return true;
        }
        else if (name == "python:list")
        {
            t = SEQ_LIST;
            return true;
        }
        else if (name == "python:array.array")
        {
            t = SEQ_ARRAY;
            return true;
        }
        else if (name == "python:numpy.ndarray")
        {
            t = SEQ_NUMPYARRAY;
            return true;
        }
        else if (name.find("python:memoryview:") == 0)
        {
            t = SEQ_MEMORYVIEW;
            return true;
        }
    }
    return false;
}

SequenceInfo::SequenceMapping::Type
IcePy::SequenceInfo::SequenceMapping::getTypeWithDefault(const Ice::StringSeq& metadata)
{
    Type t;
    if (!getType(metadata, t))
    {
        t = SEQ_DEFAULT;
    }
    return t;
}

IcePy::SequenceInfo::SequenceMapping::SequenceMapping(Type t, const Ice::StringSeq& meta) : type(t), factory(nullptr)
{
    if (type == SEQ_ARRAY)
    {
        factory = lookupType("Ice._ArrayUtil.createArray");
        if (!factory)
        {
            PyErr_Format(PyExc_ImportError, "factory type not found 'Ice._ArrayUtil.createArray'");
            throw InvalidSequenceFactoryException();
        }
    }
    else if (type == SEQ_NUMPYARRAY)
    {
        factory = lookupType("Ice._ArrayUtil.createNumPyArray");
        if (!factory)
        {
            PyErr_Format(PyExc_ImportError, "factory type not found 'Ice._ArrayUtil.createNumPyArray'");
            throw InvalidSequenceFactoryException();
        }
    }
    else if (type == SEQ_MEMORYVIEW)
    {
        const string prefix = "python:memoryview:";
        for (auto i = meta.begin(); i != meta.end(); ++i)
        {
            if (i->find(prefix) == 0)
            {
                string typestr = i->substr(prefix.size());
                // Strip away the trailing type hint colon if present.
                auto pos = typestr.find(':');
                if (pos != string::npos)
                {
                    typestr = typestr.substr(0, pos);
                }

                factory = lookupType(typestr);
                if (!factory)
                {
                    PyErr_Format(PyExc_ImportError, "factory type not found '%s'", typestr.c_str());
                    throw InvalidSequenceFactoryException();
                }
                if (!PyCallable_Check(factory))
                {
                    PyErr_Format(PyExc_RuntimeError, "factory type '%s' is not callable", typestr.c_str());
                    throw InvalidSequenceFactoryException();
                }
                break;
            }
        }
    }
}

IcePy::SequenceInfo::SequenceMapping::SequenceMapping(const Ice::StringSeq& meta)
    : SequenceMapping(getTypeWithDefault(meta), meta)
{
}

void
IcePy::SequenceInfo::SequenceMapping::unmarshaled(PyObject* val, PyObject* target, void* closure)
{
    auto i = reinterpret_cast<Py_ssize_t>(closure);
    if (type == SEQ_DEFAULT || type == SEQ_LIST)
    {
        // PyList_SET_ITEM steals a reference.
        PyList_SET_ITEM(target, i, Py_NewRef(val));
    }
    else
    {
        assert(type == SEQ_TUPLE);
        // PyTuple_SET_ITEM steals a reference.
        PyTuple_SET_ITEM(target, i, Py_NewRef(val));
    }
}

PyObject*
IcePy::SequenceInfo::SequenceMapping::createContainer(int sz) const
{
    if (type == SEQ_DEFAULT || type == SEQ_LIST)
    {
        return PyList_New(sz);
    }
    else if (type == SEQ_TUPLE)
    {
        return PyTuple_New(sz);
    }
    else
    {
        //
        // Must never be called for SEQ_MEMORYVIEW, as the container
        // is created by the user factory function.
        //
        assert(false);
        return nullptr;
    }
}

void
IcePy::SequenceInfo::SequenceMapping::setItem(PyObject* cont, int i, PyObject* val) const
{
    if (type == SEQ_DEFAULT || type == SEQ_LIST)
    {
        // PyList_SET_ITEM steals a reference.
        PyList_SET_ITEM(cont, i, Py_NewRef(val));
    }
    else
    {
        assert(type == SEQ_TUPLE);
        // PyTuple_SET_ITEM steals a reference.
        PyTuple_SET_ITEM(cont, i, Py_NewRef(val));
    }
}

//
// DictionaryInfo implementation.
//
IcePy::DictionaryInfo::DictionaryInfo(string ident, PyObject* kt, PyObject* vt) : id(std::move(ident))
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
    return _variableLength ? Ice::OptionalFormat::FSize : Ice::OptionalFormat::VSize;
}

bool
IcePy::DictionaryInfo::usesClasses() const
{
    return valueType->usesClasses();
}

void
IcePy::DictionaryInfo::marshal(
    PyObject* p,
    Ice::OutputStream* os,
    ObjectMap* objectMap,
    bool optional,
    const Ice::StringSeq*)
{
    if (p != Py_None && !PyDict_Check(p))
    {
        PyErr_Format(PyExc_ValueError, "expected dictionary value");
        throw AbortMarshaling();
    }

    const int32_t sz = p == Py_None ? 0 : static_cast<int32_t>(PyDict_Size(p));

    Ice::OutputStream::size_type sizePos = 0;
    if (optional)
    {
        if (_variableLength)
        {
            sizePos = os->startSize();
        }
        else
        {
            os->writeSize(sz == 0 ? 1 : sz * _wireSize + (sz > 254 ? 5 : 1));
        }
    }

    if (p == Py_None)
    {
        os->writeSize(0);
    }
    else
    {
        os->writeSize(sz);

        Py_ssize_t pos = 0;
        PyObject* key;
        PyObject* value;
        while (PyDict_Next(p, &pos, &key, &value))
        {
            if (!keyType->validate(key))
            {
                PyErr_Format(PyExc_ValueError, "invalid key in '%s' element", const_cast<char*>(id.c_str()));
                throw AbortMarshaling();
            }
            keyType->marshal(key, os, objectMap, false);

            if (!valueType->validate(value))
            {
                PyErr_Format(PyExc_ValueError, "invalid value in '%s' element", const_cast<char*>(id.c_str()));
                throw AbortMarshaling();
            }
            valueType->marshal(value, os, objectMap, false);
        }
    }

    if (optional && _variableLength)
    {
        os->endSize(sizePos);
    }
}

void
IcePy::DictionaryInfo::unmarshal(
    Ice::InputStream* is,
    const UnmarshalCallbackPtr& cb,
    PyObject* target,
    void* closure,
    bool optional,
    const Ice::StringSeq*)
{
    if (optional)
    {
        if (_variableLength)
        {
            is->skip(4);
        }
        else
        {
            is->skipSize();
        }
    }

    PyObjectHandle p{PyDict_New()};
    if (!p.get())
    {
        assert(PyErr_Occurred());
        throw AbortMarshaling();
    }

    KeyCallbackPtr keyCB = make_shared<KeyCallback>();
    keyCB->key = nullptr;

    int32_t sz = is->readSize();
    for (int32_t i = 0; i < sz; ++i)
    {
        //
        // A dictionary key cannot be a class (or contain one), so the key must be
        // available immediately.
        //
        keyType->unmarshal(is, keyCB, nullptr, nullptr, false);
        assert(keyCB->key.get());

        //
        // Insert the key into the dictionary with a dummy value in order to hold
        // a reference to the key. In case of an exception, we don't want to leak
        // the key.
        //
        if (PyDict_SetItem(p.get(), keyCB->key.get(), Py_None) < 0)
        {
            assert(PyErr_Occurred());
            throw AbortMarshaling();
        }

        //
        // The callback will reset the dictionary entry with the unmarshaled value,
        // so we pass it the key.
        //
        void* cl = reinterpret_cast<void*>(keyCB->key.get());
        valueType->unmarshal(is, shared_from_this(), p.get(), cl, false);
    }

    cb->unmarshaled(p.get(), target, closure);
}

void
IcePy::DictionaryInfo::unmarshaled(PyObject* val, PyObject* target, void* closure)
{
    auto* key = reinterpret_cast<PyObject*>(closure);
    if (PyDict_SetItem(target, key, val) < 0)
    {
        assert(PyErr_Occurred());
        throw AbortMarshaling();
    }
}

void
IcePy::DictionaryInfo::KeyCallback::unmarshaled(PyObject* val, PyObject*, void*)
{
    key = Py_NewRef(val);
}

void
IcePy::DictionaryInfo::destroy()
{
    keyType = nullptr;
    valueType = nullptr;
}

//
// ValueInfo implementation.
//
ValueInfoPtr
IcePy::ValueInfo::create(string ident)
{
    ValueInfoPtr valueInfo{new ValueInfo{std::move(ident)}};
    valueInfo->typeObj = createType(valueInfo);
    return valueInfo;
}

IcePy::ValueInfo::ValueInfo(string ident) : id(std::move(ident)) {}

void
IcePy::ValueInfo::define(PyObject* t, int compact, bool intf, PyObject* b, PyObject* m)
{
    assert(PyType_Check(t));
    assert(PyTuple_Check(m));

    const_cast<int&>(compactId) = compact;
    const_cast<bool&>(interface) = intf;

    if (b != Py_None)
    {
        const_cast<ValueInfoPtr&>(base) = dynamic_pointer_cast<ValueInfo>(getType(b));
        assert(base);
    }

    convertDataMembers(m, const_cast<DataMemberList&>(members), const_cast<DataMemberList&>(optionalMembers), true);

    pythonType = t;

    const_cast<bool&>(defined) = true;
}

string
IcePy::ValueInfo::getId() const
{
    return id;
}

bool
IcePy::ValueInfo::validate(PyObject* val)
{
    return val == Py_None || PyObject_IsInstance(val, pythonType) == 1;
}

bool
IcePy::ValueInfo::variableLength() const
{
    return true;
}

int
IcePy::ValueInfo::wireSize() const
{
    return 1;
}

Ice::OptionalFormat
IcePy::ValueInfo::optionalFormat() const
{
    return Ice::OptionalFormat::Class;
}

bool
IcePy::ValueInfo::usesClasses() const
{
    return true;
}

void
IcePy::ValueInfo::marshal(PyObject* p, Ice::OutputStream* os, ObjectMap* objectMap, bool, const Ice::StringSeq*)
{
    if (!pythonType)
    {
        PyErr_Format(PyExc_RuntimeError, "class %s is declared but not defined", id.c_str());
        throw AbortMarshaling();
    }

    if (p == Py_None)
    {
        std::shared_ptr<Ice::Value> value; // nullptr
        os->write(value);
        return;
    }

    if (!PyObject_IsInstance(p, pythonType))
    {
        PyErr_Format(PyExc_ValueError, "expected value of type %s", id.c_str());
        throw AbortMarshaling();
    }

    //
    // Ice::ValueWriter is a subclass of Ice::Value that wraps a Python object for marshaling.
    // It is possible that this Python object has already been marshaled, therefore we first must
    // check the object map to see if this object is present. If so, we use the existing ValueWriter,
    // otherwise we create a new one.
    //
    std::shared_ptr<Ice::Value> writer;
    assert(objectMap);
    auto q = objectMap->find(p);
    if (q == objectMap->end())
    {
        writer = make_shared<ValueWriter>(p, objectMap, shared_from_this());
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
    void patchObject(void* addr, const std::shared_ptr<Ice::Value>& v)
    {
        auto* cb = static_cast<ReadValueCallback*>(addr);
        assert(cb);
        cb->invoke(v);
    }
}

void
IcePy::ValueInfo::unmarshal(
    Ice::InputStream* is,
    const UnmarshalCallbackPtr& cb,
    PyObject* target,
    void* closure,
    bool,
    const Ice::StringSeq*)
{
    if (!pythonType)
    {
        PyErr_Format(PyExc_RuntimeError, "class %s is declared but not defined", id.c_str());
        throw AbortMarshaling();
    }

    //
    // This callback is notified when the Slice value is actually read. The StreamUtil object
    // attached to the stream keeps a reference to the callback object to ensure it lives
    // long enough.
    //
    ReadValueCallbackPtr rocb = make_shared<ReadValueCallback>(shared_from_this(), cb, target, closure);
    auto* util = reinterpret_cast<StreamUtil*>(is->getClosure());
    assert(util);
    util->add(rocb);
    is->read(patchObject, rocb.get());
}

void
IcePy::ValueInfo::destroy()
{
    const_cast<ValueInfoPtr&>(base) = nullptr;
    const_cast<DataMemberList&>(members).clear();
}

//
// ProxyInfo implementation.
//
ProxyInfoPtr
IcePy::ProxyInfo::create(string ident)
{
    ProxyInfoPtr proxyInfo{new ProxyInfo{std::move(ident)}};
    proxyInfo->typeObj = createType(proxyInfo);
    return proxyInfo;
}

IcePy::ProxyInfo::ProxyInfo(string ident) : id(std::move(ident)) {}

void
IcePy::ProxyInfo::define(PyObject* t)
{
    pythonType = t; // Borrowed reference.
}

string
IcePy::ProxyInfo::getId() const
{
    return id;
}

bool
IcePy::ProxyInfo::validate(PyObject* val)
{
    return val == Py_None || checkProxy(val);
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
    return Ice::OptionalFormat::FSize;
}

void
IcePy::ProxyInfo::marshal(PyObject* p, Ice::OutputStream* os, ObjectMap*, bool optional, const Ice::StringSeq*)
{
    Ice::OutputStream::size_type sizePos = 0;
    if (optional)
    {
        sizePos = os->startSize();
    }

    std::optional<Ice::ObjectPrx> proxy;

    if (p != Py_None)
    {
        if (checkProxy(p))
        {
            proxy = getProxy(p);
        }
        else
        {
            assert(false); // validate() should have caught this.
        }
    }
    os->write(proxy);

    if (optional)
    {
        os->endSize(sizePos);
    }
}

void
IcePy::ProxyInfo::unmarshal(
    Ice::InputStream* is,
    const UnmarshalCallbackPtr& cb,
    PyObject* target,
    void* closure,
    bool optional,
    const Ice::StringSeq*)
{
    if (optional)
    {
        is->skip(4);
    }

    std::optional<Ice::ObjectPrx> proxy;
    is->read(proxy);

    if (!proxy)
    {
        cb->unmarshaled(Py_None, target, closure);
        return;
    }

    if (!pythonType)
    {
        PyErr_Format(PyExc_RuntimeError, "class %s is declared but not defined", id.c_str());
        throw AbortMarshaling();
    }

    PyObjectHandle p{createProxy(proxy.value(), proxy->ice_getCommunicator(), pythonType)};
    cb->unmarshaled(p.get(), target, closure);
}

//
// ValueWriter implementation.
//
IcePy::ValueWriter::ValueWriter(PyObject* object, ObjectMap* objectMap, ValueInfoPtr formal)
    : _object(Py_NewRef(object)),
      _map(objectMap),
      _formal(std::move(formal))
{
    if (!_formal || !_formal->interface)
    {
        PyObjectHandle iceType{getAttr(object, "_ice_type", false)};
        if (!iceType.get())
        {
            assert(PyErr_Occurred());
            throw AbortMarshaling();
        }
        _info = dynamic_pointer_cast<ValueInfo>(getType(iceType.get()));
        assert(_info);
    }
}

void
IcePy::ValueWriter::ice_preMarshal()
{
    if (PyObject_HasAttrString(_object.get(), "ice_preMarshal") == 1)
    {
        PyObjectHandle tmp{PyObject_CallMethod(_object.get(), "ice_preMarshal", nullptr)};
        if (!tmp.get())
        {
            assert(PyErr_Occurred());
            throw AbortMarshaling();
        }
    }
}

void
IcePy::ValueWriter::_iceWrite(Ice::OutputStream* os) const
{
    //
    // Retrieve the SlicedData object that we stored as a hidden member of the Python object.
    //
    Ice::SlicedDataPtr slicedData = StreamUtil::getSlicedDataMember(_object.get(), const_cast<ObjectMap*>(_map));

    os->startValue(slicedData);

    if (_formal && _formal->interface)
    {
        PyObjectHandle ret{PyObject_CallMethod(_object.get(), "ice_id", nullptr)};
        if (!ret.get())
        {
            assert(PyErr_Occurred());
            throw AbortMarshaling();
        }
        string id = getString(ret.get());
        os->startSlice(id, -1, true);
        os->endSlice();
    }
    else
    {
        if (_info->id != "::Ice::UnknownSlicedValue")
        {
            ValueInfoPtr info = _info;
            while (info)
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
IcePy::ValueWriter::_iceRead(Ice::InputStream*)
{
    assert(false);
}

void
IcePy::ValueWriter::writeMembers(Ice::OutputStream* os, const DataMemberList& members) const
{
    for (auto q = members.begin(); q != members.end(); ++q)
    {
        const DataMemberPtr& member = *q;

        char* memberName = const_cast<char*>(member->name.c_str());

        PyObjectHandle val{getAttr(_object.get(), member->name, true)};
        if (!val.get())
        {
            if (member->optional)
            {
                PyErr_Clear();
                continue;
            }
            else
            {
                PyErr_Format(
                    PyExc_AttributeError,
                    "no member '%s' found in %s value",
                    memberName,
                    const_cast<char*>(_info->id.c_str()));
                throw AbortMarshaling();
            }
        }
        else if (
            member->optional &&
            (val.get() == Py_None || !os->writeOptional(member->tag, member->type->optionalFormat())))
        {
            continue;
        }

        if (!member->type->validate(val.get()))
        {
            PyErr_Format(
                PyExc_ValueError,
                "invalid value for %s member '%s'",
                const_cast<char*>(_info->id.c_str()),
                memberName);
            throw AbortMarshaling();
        }

        member->type->marshal(val.get(), os, _map, member->optional, &member->metadata);
    }
}

//
// ValueReader implementation.
//
IcePy::ValueReader::ValueReader(PyObject* object, ValueInfoPtr info)
    : _object(Py_NewRef(object)),
      _info(std::move(info))
{
}

void
IcePy::ValueReader::ice_postUnmarshal()
{
    if (PyObject_HasAttrString(_object.get(), "ice_postUnmarshal") == 1)
    {
        PyObjectHandle tmp{PyObject_CallMethod(_object.get(), "ice_postUnmarshal", nullptr)};
        if (!tmp.get())
        {
            assert(PyErr_Occurred());
            throw AbortMarshaling();
        }
    }
}

void
IcePy::ValueReader::_iceWrite(Ice::OutputStream*) const
{
    assert(false);
}

void
IcePy::ValueReader::_iceRead(Ice::InputStream* is)
{
    is->startValue();

    const bool unknown = _info->id == "::Ice::UnknownSlicedValue";

    //
    // Unmarshal the slices of a user-defined class.
    //
    if (!unknown && _info->id != Ice::Value::ice_staticId())
    {
        ValueInfoPtr info = _info;
        while (info)
        {
            is->startSlice();

            DataMemberList::const_iterator p;

            for (p = info->members.begin(); p != info->members.end(); ++p)
            {
                const DataMemberPtr& member = *p;
                member->type->unmarshal(is, member, _object.get(), nullptr, false, &member->metadata);
            }

            //
            // The optional members have already been sorted by tag.
            //
            for (p = info->optionalMembers.begin(); p != info->optionalMembers.end(); ++p)
            {
                const DataMemberPtr& member = *p;
                if (is->readOptional(member->tag, member->type->optionalFormat()))
                {
                    member->type->unmarshal(is, member, _object.get(), nullptr, true, &member->metadata);
                }
                else if (PyObject_SetAttrString(_object.get(), const_cast<char*>(member->name.c_str()), Py_None) < 0)
                {
                    assert(PyErr_Occurred());
                    throw AbortMarshaling();
                }
            }

            is->endSlice();

            info = info->base;
        }
    }

    _slicedData = is->endValue();

    if (_slicedData)
    {
        auto* util = reinterpret_cast<StreamUtil*>(is->getClosure());
        assert(util);
        util->add(shared_from_this());

        //
        // Define the "unknownTypeId" member for an instance of UnknownSlicedObject.
        //
        if (unknown)
        {
            assert(!_slicedData->slices.empty());

            PyObjectHandle typeId{createString(_slicedData->slices[0]->typeId)};
            if (!typeId.get() || PyObject_SetAttrString(_object.get(), "unknownTypeId", typeId.get()) < 0)
            {
                assert(PyErr_Occurred());
                throw AbortMarshaling();
            }
        }
    }
}

ValueInfoPtr
IcePy::ValueReader::getInfo() const
{
    return _info;
}

PyObject*
IcePy::ValueReader::getObject() const
{
    return _object.get();
}

Ice::SlicedDataPtr
IcePy::ValueReader::getSlicedData() const
{
    return _slicedData;
}

//
// ReadValueCallback implementation.
//
IcePy::ReadValueCallback::ReadValueCallback(ValueInfoPtr info, UnmarshalCallbackPtr cb, PyObject* target, void* closure)
    : _info(std::move(info)),
      _cb(std::move(cb)),
      _target(target),
      _closure(closure)
{
    Py_XINCREF(_target);
}

IcePy::ReadValueCallback::~ReadValueCallback() { Py_XDECREF(_target); }

void
IcePy::ReadValueCallback::invoke(const std::shared_ptr<Ice::Value>& p)
{
    if (p)
    {
        auto reader = dynamic_pointer_cast<ValueReader>(p);
        assert(reader);

        //
        // Verify that the value's type is compatible with the formal type.
        //
        PyObject* obj = reader->getObject(); // Borrowed reference.
        if (!PyObject_IsInstance(obj, _info->pythonType))
        {
            throw MarshalException{
                __FILE__,
                __LINE__,
                "failed to unmarshal class with type ID '" + _info->id +
                    "': the Slice loader returned a class with type ID '" + reader->getInfo()->id + "'"};
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
IcePy::ExceptionInfo::marshal(PyObject* p, Ice::OutputStream* os, ObjectMap* objectMap)
{
    if (!PyObject_IsInstance(p, pythonType))
    {
        PyErr_Format(PyExc_ValueError, "expected exception %s", id.c_str());
        throw AbortMarshaling();
    }

    os->startException();

    ExceptionInfoPtr info = shared_from_this();
    while (info)
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
IcePy::ExceptionInfo::writeMembers(
    PyObject* p,
    Ice::OutputStream* os,
    const DataMemberList& membersP,
    ObjectMap* objectMap) const
{
    for (auto q = membersP.begin(); q != membersP.end(); ++q)
    {
        const DataMemberPtr& member = *q;

        char* memberName = const_cast<char*>(member->name.c_str());

        PyObjectHandle val{getAttr(p, member->name, true)};
        if (!val.get())
        {
            if (member->optional)
            {
                PyErr_Clear();
                continue;
            }
            else
            {
                PyErr_Format(
                    PyExc_AttributeError,
                    "no member '%s' found in %s value",
                    memberName,
                    const_cast<char*>(id.c_str()));
                throw AbortMarshaling();
            }
        }
        else if (
            member->optional &&
            (val.get() == Py_None || !os->writeOptional(member->tag, member->type->optionalFormat())))
        {
            continue;
        }

        if (!member->type->validate(val.get()))
        {
            PyErr_Format(
                PyExc_ValueError,
                "invalid value for %s member '%s'",
                const_cast<char*>(id.c_str()),
                memberName);
            throw AbortMarshaling();
        }

        member->type->marshal(val.get(), os, objectMap, member->optional, &member->metadata);
    }
}

PyObject*
IcePy::ExceptionInfo::unmarshal(Ice::InputStream* is)
{
    PyObjectHandle p{createExceptionInstance(pythonType)};
    if (!p.get())
    {
        assert(PyErr_Occurred());
        throw AbortMarshaling();
    }

    ExceptionInfoPtr info = shared_from_this();
    while (info)
    {
        is->startSlice();

        DataMemberList::iterator q;

        for (q = info->members.begin(); q != info->members.end(); ++q)
        {
            const DataMemberPtr& member = *q;
            member->type->unmarshal(is, member, p.get(), nullptr, false, &member->metadata);
        }

        //
        // The optional members have already been sorted by tag.
        //
        for (q = info->optionalMembers.begin(); q != info->optionalMembers.end(); ++q)
        {
            const DataMemberPtr& member = *q;
            if (is->readOptional(member->tag, member->type->optionalFormat()))
            {
                member->type->unmarshal(is, member, p.get(), nullptr, true, &member->metadata);
            }
            else if (PyObject_SetAttrString(p.get(), const_cast<char*>(member->name.c_str()), Py_None) < 0)
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

//
// ExceptionWriter implementation.
//
IcePy::ExceptionWriter::ExceptionWriter(const PyObjectHandle& ex, const ExceptionInfoPtr& info) noexcept
    : _ex(ex),
      _info(info)
{
    if (!info)
    {
        PyObjectHandle iceType{getAttr(ex.get(), "_ice_type", false)};
        assert(iceType.get());
        _info = dynamic_pointer_cast<ExceptionInfo>(getException(iceType.get()));
        assert(_info);
    }
}

IcePy::ExceptionWriter::ExceptionWriter(const ExceptionWriter& other)
{
    // Exception writer copy constructor can be called from a C++ thread when IcePy throws an ExceptionWriter object
    // from the dispatch pipeline. Specifically with MSVC if the catch block calls current_exception the copy
    // constructor would be used to move the exception from stack to heap.

    {
        // Ensure the current thread is able to call into Python.
        AdoptThread adoptThread;
        _ex = other._ex;
    }
    _info = other._info;
    _objects = other._objects;
}

IcePy::ExceptionWriter::~ExceptionWriter() noexcept
{
    // Exception writer destructor can be called from a C++ thread when IcePy throws an ExceptionWriter object
    // from the dispatch pipeline. The ServantLocator implementation does this.

    // Ensure the current thread is able to call into Python.
    AdoptThread adoptThread;
    _ex = nullptr;
}

const char*
IcePy::ExceptionWriter::ice_id() const noexcept
{
    return _info->id.c_str();
}

void
IcePy::ExceptionWriter::ice_throw() const
{
    assert(false);
}

void
IcePy::ExceptionWriter::_write(Ice::OutputStream* os) const
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    _info->marshal(_ex.get(), os, const_cast<ObjectMap*>(&_objects));
}

void
IcePy::ExceptionWriter::_read(Ice::InputStream*)
{
    assert(false);
}

bool
IcePy::ExceptionWriter::_usesClasses() const
{
    return _info->usesClasses;
}

//
// ExceptionReader implementation.
//
IcePy::ExceptionReader::ExceptionReader(ExceptionInfoPtr info) noexcept : _info(std::move(info)) {}

const char*
IcePy::ExceptionReader::ice_id() const noexcept
{
    return _info->id.c_str();
}

void
IcePy::ExceptionReader::ice_throw() const
{
    assert(false);
}

void
IcePy::ExceptionReader::_write(Ice::OutputStream*) const
{
    assert(false);
}

void
IcePy::ExceptionReader::_read(Ice::InputStream* is)
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    is->startException();

    const_cast<PyObjectHandle&>(_ex) = _info->unmarshal(is);

    is->endException();
}

bool
IcePy::ExceptionReader::_usesClasses() const
{
    return _info->usesClasses;
}

PyObject*
IcePy::ExceptionReader::getException() const
{
    return _ex.get();
}

//
// lookupValueInfo()
//
IcePy::ValueInfoPtr
IcePy::lookupValueInfo(string_view id)
{
    auto p = valueInfoMap.find(id);
    if (p != valueInfoMap.end())
    {
        return p->second;
    }
    return nullptr;
}

//
// lookupExceptionInfo()
//
IcePy::ExceptionInfoPtr
IcePy::lookupExceptionInfo(std::string_view id)
{
    auto p = exceptionInfoMap.find(id);
    if (p != exceptionInfoMap.end())
    {
        return p->second;
    }
    return nullptr;
}

namespace IcePy
{
    // clang-format off
    PyTypeObject TypeInfoType = {
        .ob_base = PyVarObject_HEAD_INIT(nullptr, 0)
        .tp_name = "IcePy.TypeInfo",
        .tp_basicsize = sizeof(TypeInfoObject),
        .tp_dealloc = reinterpret_cast<destructor>(typeInfoDealloc),
        .tp_flags = Py_TPFLAGS_DEFAULT,
        .tp_doc = PyDoc_STR("IcePy.TypeInfo"),
        .tp_new = reinterpret_cast<newfunc>(typeInfoNew),
    };

    PyTypeObject ExceptionInfoType = {
        .ob_base = PyVarObject_HEAD_INIT(nullptr, 0)
        .tp_name = "IcePy.ExceptionInfo",
        .tp_basicsize = sizeof(ExceptionInfoObject),
        .tp_dealloc = reinterpret_cast<destructor>(exceptionInfoDealloc),
        .tp_flags = Py_TPFLAGS_DEFAULT,
        .tp_doc = PyDoc_STR("IcePy.ExceptionInfo"),
        .tp_new = reinterpret_cast<newfunc>(exceptionInfoNew),
    };
    // clang-format on
}

bool
IcePy::initTypes(PyObject* module)
{
    if (PyType_Ready(&TypeInfoType) < 0)
    {
        return false;
    }

    if (PyModule_AddObject(module, "TypeInfo", reinterpret_cast<PyObject*>(&TypeInfoType)) < 0)
    {
        return false;
    }

    if (PyType_Ready(&ExceptionInfoType) < 0)
    {
        return false;
    }

    if (PyModule_AddObject(module, "ExceptionInfo", reinterpret_cast<PyObject*>(&ExceptionInfoType)) < 0)
    {
        return false;
    }

    PrimitiveInfoPtr boolType = make_shared<PrimitiveInfo>(PrimitiveInfo::KindBool);
    PyObjectHandle boolTypeObj{createType(boolType)};
    if (PyModule_AddObject(module, "_t_bool", boolTypeObj.get()) < 0)
    {
        return false;
    }
    boolTypeObj.release(); // PyModule_AddObject steals a reference.

    PrimitiveInfoPtr byteType = make_shared<PrimitiveInfo>(PrimitiveInfo::KindByte);
    PyObjectHandle byteTypeObj{createType(byteType)};
    if (PyModule_AddObject(module, "_t_byte", byteTypeObj.get()) < 0)
    {
        return false;
    }
    byteTypeObj.release(); // PyModule_AddObject steals a reference.

    PrimitiveInfoPtr shortType = make_shared<PrimitiveInfo>(PrimitiveInfo::KindShort);
    PyObjectHandle shortTypeObj{createType(shortType)};
    if (PyModule_AddObject(module, "_t_short", shortTypeObj.get()) < 0)
    {
        return false;
    }
    shortTypeObj.release(); // PyModule_AddObject steals a reference.

    PrimitiveInfoPtr intType = make_shared<PrimitiveInfo>(PrimitiveInfo::KindInt);
    PyObjectHandle intTypeObj{createType(intType)};
    if (PyModule_AddObject(module, "_t_int", intTypeObj.get()) < 0)
    {
        return false;
    }
    intTypeObj.release(); // PyModule_AddObject steals a reference.

    PrimitiveInfoPtr longType = make_shared<PrimitiveInfo>(PrimitiveInfo::KindLong);
    PyObjectHandle longTypeObj{createType(longType)};
    if (PyModule_AddObject(module, "_t_long", longTypeObj.get()) < 0)
    {
        return false;
    }
    longTypeObj.release(); // PyModule_AddObject steals a reference.

    PrimitiveInfoPtr floatType = make_shared<PrimitiveInfo>(PrimitiveInfo::KindFloat);
    PyObjectHandle floatTypeObj{createType(floatType)};
    if (PyModule_AddObject(module, "_t_float", floatTypeObj.get()) < 0)
    {
        return false;
    }
    floatTypeObj.release(); // PyModule_AddObject steals a reference.

    PrimitiveInfoPtr doubleType = make_shared<PrimitiveInfo>(PrimitiveInfo::KindDouble);
    PyObjectHandle doubleTypeObj{createType(doubleType)};
    if (PyModule_AddObject(module, "_t_double", doubleTypeObj.get()) < 0)
    {
        return false;
    }
    doubleTypeObj.release(); // PyModule_AddObject steals a reference.

    PrimitiveInfoPtr stringType = make_shared<PrimitiveInfo>(PrimitiveInfo::KindString);
    PyObjectHandle stringTypeObj{createType(stringType)};
    if (PyModule_AddObject(module, "_t_string", stringTypeObj.get()) < 0)
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
    auto* p = reinterpret_cast<TypeInfoObject*>(obj);
    return *p->info;
}

PyObject*
IcePy::createType(const TypeInfoPtr& info)
{
    TypeInfoObject* obj = typeInfoNew(&TypeInfoType, nullptr, nullptr);
    if (obj)
    {
        obj->info = new IcePy::TypeInfoPtr(info);
    }
    return reinterpret_cast<PyObject*>(obj);
}

IcePy::ExceptionInfoPtr
IcePy::getException(PyObject* obj)
{
    assert(PyObject_IsInstance(obj, reinterpret_cast<PyObject*>(&ExceptionInfoType)));
    auto* p = reinterpret_cast<ExceptionInfoObject*>(obj);
    return *p->info;
}

PyObject*
IcePy::createException(const ExceptionInfoPtr& info)
{
    ExceptionInfoObject* obj = exceptionInfoNew(&ExceptionInfoType, nullptr, nullptr);
    if (obj)
    {
        obj->info = new IcePy::ExceptionInfoPtr(info);
    }
    return reinterpret_cast<PyObject*>(obj);
}

extern "C" PyObject*
IcePy_defineEnum(PyObject*, PyObject* args)
{
    char* id;
    PyObject* type;
    PyObject* meta; // Not currently used.
    PyObject* enumerators;
    if (!PyArg_ParseTuple(args, "sOOO", &id, &type, &meta, &enumerators))
    {
        return nullptr;
    }

    assert(PyTuple_Check(meta));

    EnumInfoPtr info = make_shared<EnumInfo>(id, type, enumerators);

    return createType(info);
}

extern "C" PyObject*
IcePy_defineStruct(PyObject*, PyObject* args)
{
    char* id;
    PyObject* type;
    PyObject* meta; // Not currently used.
    PyObject* members;
    if (!PyArg_ParseTuple(args, "sOOO", &id, &type, &meta, &members))
    {
        return nullptr;
    }

    assert(PyType_Check(type));
    assert(PyTuple_Check(meta));
    assert(PyTuple_Check(members));

    StructInfoPtr info = make_shared<StructInfo>(id, type, members);

    return createType(info);
}

extern "C" PyObject*
IcePy_defineSequence(PyObject*, PyObject* args)
{
    char* id;
    PyObject* meta;
    PyObject* elementType;
    if (!PyArg_ParseTuple(args, "sOO", &id, &meta, &elementType))
    {
        return nullptr;
    }

    try
    {
        SequenceInfoPtr info = make_shared<SequenceInfo>(id, meta, elementType);
        return createType(info);
    }
    catch (const InvalidSequenceFactoryException&)
    {
        assert(PyErr_Occurred());
        return nullptr;
    }
}

extern "C" PyObject*
IcePy_defineDictionary(PyObject*, PyObject* args)
{
    char* id;
    PyObject* meta; // Not currently used.
    PyObject* keyType;
    PyObject* valueType;
    if (!PyArg_ParseTuple(args, "sOOO", &id, &meta, &keyType, &valueType))
    {
        return nullptr;
    }

    assert(PyTuple_Check(meta));

    DictionaryInfoPtr info = make_shared<DictionaryInfo>(id, keyType, valueType);

    return createType(info);
}

extern "C" PyObject*
IcePy_declareProxy(PyObject*, PyObject* args)
{
    char* id;
    if (!PyArg_ParseTuple(args, "s", &id))
    {
        return nullptr;
    }

    string proxyId = string{id} + "Prx";
    ProxyInfoPtr info = lookupProxyInfo(proxyId);
    if (info)
    {
        return Py_NewRef(info->typeObj);
    }
    else
    {
        info = ProxyInfo::create(proxyId);
        addProxyInfo(proxyId, info);
        return info->typeObj; // Delegate ownership to the global "_t_XXX" variable.
    }
}

extern "C" PyObject*
IcePy_defineProxy(PyObject*, PyObject* args)
{
    char* id;
    PyObject* type;
    if (!PyArg_ParseTuple(args, "sO", &id, &type))
    {
        return nullptr;
    }

    assert(PyType_Check(type));

    string proxyId = string{id} + "Prx";
    ProxyInfoPtr info = lookupProxyInfo(proxyId);
    if (info)
    {
        info->define(type);
        return Py_NewRef(info->typeObj);
    }
    else
    {
        info = ProxyInfo::create(proxyId);
        addProxyInfo(proxyId, info);
        info->define(type);
        return info->typeObj; // Delegate ownership to the global "_t_XXX" variable.
    }
}

extern "C" PyObject*
IcePy_declareValue(PyObject*, PyObject* args)
{
    char* id;
    if (!PyArg_ParseTuple(args, "s", &id))
    {
        return nullptr;
    }

    ValueInfoPtr info = lookupValueInfo(id);
    if (!info)
    {
        info = ValueInfo::create(id);
        addValueInfo(id, info);
        return info->typeObj; // Delegate ownership to the global "_t_XXX" variable.
    }
    else
    {
        return Py_NewRef(info->typeObj);
    }
}

extern "C" PyObject*
IcePy_defineValue(PyObject*, PyObject* args)
{
    char* id;
    PyObject* type;
    int compactId;
    PyObject* meta; // Not currently used.
    int interface;
    PyObject* base;
    PyObject* members;
    if (!PyArg_ParseTuple(args, "sOiOiOO", &id, &type, &compactId, &meta, &interface, &base, &members))
    {
        return nullptr;
    }

    assert(PyTuple_Check(meta));

    PyObject* r;

    //
    // A ValueInfo object will already exist for this id if a forward declaration
    // was encountered, or if the Slice definition is being reloaded. In the latter
    // case, we act as if it hasn't been defined yet.
    //
    ValueInfoPtr info = lookupValueInfo(id);
    if (!info || info->defined)
    {
        info = ValueInfo::create(id);
        addValueInfo(id, info);
        r = info->typeObj; // Delegate ownership to the global "_t_XXX" variable.
    }
    else
    {
        r = Py_NewRef(info->typeObj);
    }

    info->define(type, compactId, interface ? true : false, base, members);

    if (info->compactId != -1)
    {
        // Insert a second entry for the compact ID.
        string compactIdStr = to_string(info->compactId);
        if (!lookupValueInfo(compactIdStr))
        {
            addValueInfo(compactIdStr, info);
        }
    }

    return r;
}

extern "C" PyObject*
IcePy_defineException(PyObject*, PyObject* args)
{
    char* id;
    PyObject* type;
    PyObject* meta;
    PyObject* base;
    PyObject* members;
    if (!PyArg_ParseTuple(args, "sOOOO", &id, &type, &meta, &base, &members))
    {
        return nullptr;
    }

    assert(PyExceptionClass_Check(type));
    assert(PyTuple_Check(meta));
    assert(PyTuple_Check(members));

    ExceptionInfoPtr info = make_shared<ExceptionInfo>();
    info->id = id;

    if (base != Py_None)
    {
        info->base = dynamic_pointer_cast<ExceptionInfo>(getException(base));
        assert(info->base);
    }

    convertDataMembers(members, info->members, info->optionalMembers, true);

    info->usesClasses = false;

    //
    // Only examine the required members to see if any use classes.
    //
    for (const auto& dm : info->members)
    {
        if (!info->usesClasses)
        {
            info->usesClasses = dm->type->usesClasses();
        }
    }

    info->pythonType = type;

    addExceptionInfo(id, info);

    return createException(info);
}
