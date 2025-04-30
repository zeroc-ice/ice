// Copyright (c) ZeroC, Inc.

#include "Types.h"
#include "Ice/OutputUtil.h"
#include "Ice/SlicedData.h"
#include "Proxy.h"
#include "Util.h"

#include <limits>
#include <memory>
#include <optional>

using namespace std;
using namespace IcePHP;
using namespace Ice;
using namespace IceInternal;

ZEND_EXTERN_MODULE_GLOBALS(ice)

// Class entries represent the PHP class implementations we have registered.
namespace IcePHP
{
    zend_class_entry* typeInfoClassEntry = 0;
    zend_class_entry* exceptionInfoClassEntry = 0;
}

static zend_object_handlers _typeInfoHandlers;
static zend_object_handlers _exceptionInfoHandlers;

static string _unsetGUID = "710A52F2-A014-4CB2-AF40-348D48DBCDDD";

extern "C"
{
    static zend_object* handleTypeInfoAlloc(zend_class_entry*);
    static void handleTypeInfoFreeStorage(zend_object*);

    static zend_object* handleExceptionInfoAlloc(zend_class_entry*);
    static void handleExceptionInfoFreeStorage(zend_object*);
}

typedef map<string, ProxyInfoPtr, std::less<>> ProxyInfoMap;
typedef map<string, ClassInfoPtr, std::less<>> ClassInfoMap;

typedef map<string, ExceptionInfoPtr, std::less<>> ExceptionInfoMap;

static void
addProxyInfo(const ProxyInfoPtr& p)
{
    ProxyInfoMap* m;
    if (ICE_G(proxyInfoMap))
    {
        m = reinterpret_cast<ProxyInfoMap*>(ICE_G(proxyInfoMap));
    }
    else
    {
        m = new ProxyInfoMap;
        ICE_G(proxyInfoMap) = m;
    }
    m->insert(ProxyInfoMap::value_type(p->id, p));
}

static void
addPropertyZval(zval* arg, const char* key, zval* value)
{
    add_property_zval(arg, key, value);
}

IcePHP::ProxyInfoPtr
IcePHP::getProxyInfo(string_view id)
{
    if (ICE_G(proxyInfoMap))
    {
        ProxyInfoMap* m = reinterpret_cast<ProxyInfoMap*>(ICE_G(proxyInfoMap));
        ProxyInfoMap::iterator p = m->find(id);
        if (p != m->end())
        {
            return p->second;
        }
    }
    return nullptr;
}

static void
addClassInfoById(const ClassInfoPtr& p)
{
    assert(!getClassInfoById(p->id));

    ClassInfoMap* m = reinterpret_cast<ClassInfoMap*>(ICE_G(idToClassInfoMap));
    if (!m)
    {
        m = new ClassInfoMap;
        ICE_G(idToClassInfoMap) = m;
    }
    m->insert(ClassInfoMap::value_type(p->id, p));
}

static void
addClassInfoByCompactId(const ClassInfoPtr& p)
{
    assert(p->compactId != -1);
    ClassInfoMap* m = reinterpret_cast<ClassInfoMap*>(ICE_G(idToClassInfoMap));
    assert(m);

    // No-op if it's already in the map.
    m->insert(ClassInfoMap::value_type(std::to_string(p->compactId), p));
}

static void
addClassInfoByName(const ClassInfoPtr& p)
{
    assert(!getClassInfoByName(p->name));
    assert(p->name[0] == '\\');

    ClassInfoMap* m = reinterpret_cast<ClassInfoMap*>(ICE_G(nameToClassInfoMap));
    if (!m)
    {
        m = new ClassInfoMap;
        ICE_G(nameToClassInfoMap) = m;
    }
    m->insert(ClassInfoMap::value_type(p->name, p));
}

static ClassInfoPtr
getClassInfoByClass(zend_class_entry* cls, zend_class_entry* formal)
{
    // See if there's a match in our class name => ClassInfo map.
    ClassInfoPtr info = getClassInfoByName(cls->name->val);

    // Check the base class, assuming it's compatible with our formal type (if any).
    if (!info && cls->parent && (!formal || checkClass(cls->parent, formal)))
    {
        info = getClassInfoByClass(cls->parent, formal);
    }

    // TODO remove interfaces, and rename ClassInfo to ValueInfo
    // Check interfaces.
    if (!info)
    {
        for (zend_ulong i = 0; i < cls->num_interfaces && !info; ++i)
        {
            if (!formal || checkClass(cls->interfaces[i], formal))
            {
                info = getClassInfoByClass(cls->interfaces[i], formal);
            }
        }
    }

    return info;
}

IcePHP::ClassInfoPtr
IcePHP::getClassInfoById(string_view id)
{
    if (ICE_G(idToClassInfoMap))
    {
        ClassInfoMap* m = reinterpret_cast<ClassInfoMap*>(ICE_G(idToClassInfoMap));
        ClassInfoMap::iterator p = m->find(id);
        if (p != m->end())
        {
            return p->second;
        }
    }
    return 0;
}

IcePHP::ClassInfoPtr
IcePHP::getClassInfoByName(const string& name)
{
    if (ICE_G(nameToClassInfoMap))
    {
        string s = name;

        // PHP's class definition (zend_class_entry) does not use a leading backslash in the class name.
        if (s[0] != '\\')
        {
            s.insert(0, "\\");
        }

        ClassInfoMap* m = reinterpret_cast<ClassInfoMap*>(ICE_G(nameToClassInfoMap));
        ClassInfoMap::iterator p = m->find(s);
        if (p != m->end())
        {
            return p->second;
        }
    }
    return 0;
}

IcePHP::ExceptionInfoPtr
IcePHP::getExceptionInfo(string_view id)
{
    if (ICE_G(exceptionInfoMap))
    {
        ExceptionInfoMap* m = reinterpret_cast<ExceptionInfoMap*>(ICE_G(exceptionInfoMap));
        ExceptionInfoMap::iterator p = m->find(id);
        if (p != m->end())
        {
            return p->second;
        }
    }
    return 0;
}

// StreamUtil implementation
zend_class_entry* IcePHP::StreamUtil::_slicedDataType = 0;
zend_class_entry* IcePHP::StreamUtil::_sliceInfoType = 0;

IcePHP::StreamUtil::~StreamUtil()
{
    // Make sure we break any cycles among the ValueReaders in preserved slices.
    for (const auto& p : _readers)
    {
        for (const auto& q : p->getSlicedData()->slices)
        {
            //
            // Don't just call (*q)->instances.clear(), as releasing references
            // to the instances could have unexpected side effects. We exchange
            // the vector into a temporary and then let the temporary fall out
            // of scope.
            //
            vector<shared_ptr<Ice::Value>> tmp;
            tmp.swap(q->instances);
        }
    }
}

void
IcePHP::StreamUtil::add(const ReadObjectCallbackPtr& callback)
{
    _callbacks.push_back(callback);
}

void
IcePHP::StreamUtil::add(const shared_ptr<ValueReader>& reader)
{
    assert(reader->getSlicedData());
    _readers.insert(reader);
}

void
IcePHP::StreamUtil::updateSlicedData(void)
{
    for (set<shared_ptr<ValueReader>>::iterator p = _readers.begin(); p != _readers.end(); ++p)
    {
        setSlicedDataMember((*p)->getObject(), (*p)->getSlicedData());
    }
}

void
IcePHP::StreamUtil::setSlicedDataMember(zval* obj, const Ice::SlicedDataPtr& slicedData)
{
    // Create a PHP equivalent of the SlicedData object.
    assert(slicedData);

    if (!_slicedDataType)
    {
        _slicedDataType = nameToClass("\\Ice\\SlicedData");
        assert(_slicedDataType);
    }
    if (!_sliceInfoType)
    {
        _sliceInfoType = nameToClass("\\Ice\\SliceInfo");
        assert(_sliceInfoType);
    }

    zval sd;
    AutoDestroy sdDestroyer(&sd);

    if (object_init_ex(&sd, _slicedDataType) != SUCCESS)
    {
        throw AbortMarshaling();
    }

    zval slices;
    array_init(&slices);
    AutoDestroy slicesDestroyer(&slices);

    addPropertyZval(&sd, "slices", &slices);

    // Translate each SliceInfo object into its PHP equivalent.
    for (const auto& p : slicedData->slices)
    {
        zval slice;
        AutoDestroy sliceDestroyer(&slice);

        if (object_init_ex(&slice, _sliceInfoType) != SUCCESS)
        {
            throw AbortMarshaling();
        }
#ifdef HT_ALLOW_COW_VIOLATION
        HT_ALLOW_COW_VIOLATION(Z_ARRVAL(slices));
#endif
        add_next_index_zval(&slices, &slice); // Steals a reference.
        Z_ADDREF_P(&slice);

        zval typeId;
        AutoDestroy typeIdDestroyer(&typeId);
        ZVAL_STRINGL(&typeId, p->typeId.c_str(), static_cast<int>(p->typeId.size()));
        addPropertyZval(&slice, "typeId", &typeId);

        zval compactId;
        AutoDestroy compactIdDestroyer(&compactId);
        ZVAL_LONG(&compactId, p->compactId);
        addPropertyZval(&slice, "compactId", &compactId);

        zval bytes;
        array_init(&bytes);
        AutoDestroy bytesDestroyer(&bytes);
        for (const auto& q : p->bytes)
        {
            add_next_index_long(&bytes, std::to_integer<zend_long>(q & byte{0xff}));
        }
        addPropertyZval(&slice, "bytes", &bytes);

        zval instances;
        array_init(&instances);
#ifdef HT_ALLOW_COW_VIOLATION
        HT_ALLOW_COW_VIOLATION(Z_ARRVAL(instances)); // Allow circular references.
#endif
        AutoDestroy instancesDestroyer(&instances);
        addPropertyZval(&slice, "instances", &instances);

        for (const auto& q : p->instances)
        {
            // Each element in the instances list is an instance of ValueReader that wraps a PHP object.
            auto r = dynamic_pointer_cast<ValueReader>(q);
            assert(r);
            zval* o = r->getObject();
            assert(Z_TYPE_P(o) == IS_OBJECT);   // Should be non-nil.
            add_next_index_zval(&instances, o); // Steals a reference.
            Z_ADDREF_P(o);
        }

        zval hasOptionalMembers;
        AutoDestroy hasOptionalMembersDestroyer(&hasOptionalMembers);
        ZVAL_BOOL(&hasOptionalMembers, p->hasOptionalMembers ? 1 : 0);
        addPropertyZval(&slice, "hasOptionalMembers", &hasOptionalMembers);

        zval isLastSlice;
        AutoDestroy isLastSliceDestroyer(&isLastSlice);
        ZVAL_BOOL(&isLastSlice, p->isLastSlice ? 1 : 0);
        addPropertyZval(&slice, "isLastSlice", &isLastSlice);
    }

    addPropertyZval(obj, "_ice_slicedData", &sd);
}

// Instances of preserved class types may have a data member named _ice_slicedData which is an instance
// of the PHP class \Ice\SlicedData.
Ice::SlicedDataPtr
IcePHP::StreamUtil::getSlicedDataMember(zval* obj, ObjectMap* objectMap)
{
    Ice::SlicedDataPtr slicedData;

    string name = "_ice_slicedData";
    zval* sd = zend_hash_str_find_ind(Z_OBJPROP_P(obj), name.c_str(), name.size());
    if (sd)
    {
        if (Z_TYPE_P(sd) != IS_NULL)
        {
            // The "slices" member is an array of Ice_SliceInfo objects.
            zval* sl = zend_hash_str_find(Z_OBJPROP_P(sd), "slices", sizeof("slices") - 1);
            assert(sl);
            assert(Z_TYPE_P(sl) == IS_INDIRECT);
            sl = Z_INDIRECT_P(sl);
            assert(Z_TYPE_P(sl) == IS_ARRAY);

            Ice::SliceInfoSeq slices;

            HashTable* arr = Z_ARRVAL_P(sl);
            assert(arr);

            zval* s;

            ZEND_HASH_FOREACH_VAL(arr, s)
            {
                assert(Z_OBJCE_P(s) == _sliceInfoType);

                zval* typeId = zend_hash_str_find(Z_OBJPROP_P(s), "typeId", sizeof("typeId") - 1);
                assert(Z_TYPE_P(typeId) == IS_INDIRECT);
                typeId = Z_INDIRECT_P(typeId);
                assert(typeId && Z_TYPE_P(typeId) == IS_STRING);

                zval* compactId = zend_hash_str_find(Z_OBJPROP_P(s), "compactId", sizeof("compactId") - 1);
                assert(Z_TYPE_P(compactId) == IS_INDIRECT);
                compactId = Z_INDIRECT_P(compactId);
                assert(compactId && Z_TYPE_P(compactId) == IS_LONG);

                zval* bytes = zend_hash_str_find(Z_OBJPROP_P(s), "bytes", sizeof("bytes") - 1);
                assert(Z_TYPE_P(bytes) == IS_INDIRECT);
                bytes = Z_INDIRECT_P(bytes);
                assert(bytes && Z_TYPE_P(bytes) == IS_ARRAY);
                HashTable* barr = Z_ARRVAL_P(bytes);
                zval* e;
                vector<byte> byteVector(zend_hash_num_elements(barr));

#if defined(__clang__)
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wshadow"
#endif
                vector<byte>::size_type i = 0;
                ZEND_HASH_FOREACH_VAL(barr, e)
                {
                    long l = static_cast<long>(Z_LVAL_P(e));
                    assert(l >= 0 && l <= 255);
                    byteVector[i++] = static_cast<byte>(l);
                }
                ZEND_HASH_FOREACH_END();
#if defined(__clang__)
#    pragma clang diagnostic pop
#endif

                zval* hasOptionalMembers =
                    zend_hash_str_find(Z_OBJPROP_P(s), "hasOptionalMembers", sizeof("hasOptionalMembers") - 1);
                assert(Z_TYPE_P(hasOptionalMembers) == IS_INDIRECT);
                hasOptionalMembers = Z_INDIRECT_P(hasOptionalMembers);
                assert(
                    hasOptionalMembers &&
                    (Z_TYPE_P(hasOptionalMembers) == IS_TRUE || Z_TYPE_P(hasOptionalMembers) == IS_FALSE));

                zval* isLastSlice = zend_hash_str_find(Z_OBJPROP_P(s), "isLastSlice", sizeof("isLastSlice") - 1);
                assert(Z_TYPE_P(isLastSlice) == IS_INDIRECT);
                isLastSlice = Z_INDIRECT_P(isLastSlice);
                assert(isLastSlice && (Z_TYPE_P(isLastSlice) == IS_TRUE || Z_TYPE_P(isLastSlice) == IS_FALSE));

                auto info = make_shared<Ice::SliceInfo>(
                    string(Z_STRVAL_P(typeId), Z_STRLEN_P(typeId)),
                    static_cast<long>(Z_LVAL_P(compactId)),
                    std::move(byteVector),
                    Z_TYPE_P(hasOptionalMembers) == IS_TRUE,
                    Z_TYPE_P(isLastSlice) == IS_TRUE);

                zval* instances = zend_hash_str_find(Z_OBJPROP_P(s), "instances", sizeof("instances") - 1);
                assert(Z_TYPE_P(instances) == IS_INDIRECT);
                instances = Z_INDIRECT_P(instances);
                assert(instances && Z_TYPE_P(instances) == IS_ARRAY);
                HashTable* oarr = Z_ARRVAL_P(instances);
                zval* o;

#if defined(__clang__)
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wshadow"
#endif
                ZEND_HASH_FOREACH_VAL(oarr, o)
                {
                    assert(Z_TYPE_P(o) == IS_OBJECT);

                    shared_ptr<Ice::Value> writer;

                    ObjectMap::iterator i = objectMap->find(Z_OBJ_HANDLE_P(o));
                    if (i == objectMap->end())
                    {
                        writer = make_shared<ValueWriter>(o, objectMap, nullptr);
                        objectMap->insert(ObjectMap::value_type(Z_OBJ_HANDLE_P(o), writer));
                    }
                    else
                    {
                        writer = i->second;
                    }

                    info->instances.push_back(writer);
                }
                ZEND_HASH_FOREACH_END();
#if defined(__clang__)
#    pragma clang diagnostic pop
#endif
                slices.push_back(info);
            }
            ZEND_HASH_FOREACH_END();

            slicedData = make_shared<Ice::SlicedData>(slices);
        }
    }

    return slicedData;
}

// UnmarshalCallback implementation.
IcePHP::UnmarshalCallback::~UnmarshalCallback() = default;

// TypeInfo implementation.
IcePHP::TypeInfo::TypeInfo() = default;

bool
IcePHP::TypeInfo::usesClasses() const
{
    return false;
}

void
IcePHP::TypeInfo::unmarshaled(zval*, zval*, void*)
{
    assert(false);
}

void
IcePHP::TypeInfo::destroy()
{
}

// PrimitiveInfo implementation.
string
IcePHP::PrimitiveInfo::getId() const
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
    return string();
}

bool
IcePHP::PrimitiveInfo::validate(zval* zv, bool throwException)
{
    switch (kind)
    {
        case PrimitiveInfo::KindBool:
        {
            if (!(Z_TYPE_P(zv) == IS_TRUE || Z_TYPE_P(zv) == IS_FALSE))
            {
                string s = zendTypeToString(Z_TYPE_P(zv));
                if (throwException)
                {
                    ostringstream os;
                    os << "expected boolean value but received " << s;
                    invalidArgument(os.str());
                }
                return false;
            }
            break;
        }
        case PrimitiveInfo::KindByte:
        {
            if (Z_TYPE_P(zv) != IS_LONG)
            {
                string s = zendTypeToString(Z_TYPE_P(zv));
                if (throwException)
                {
                    ostringstream os;
                    os << "expected byte value but received " << s;
                    invalidArgument(os.str());
                }
                return false;
            }
            long val = static_cast<long>(Z_LVAL_P(zv));
            if (val < 0 || val > 255)
            {
                if (throwException)
                {
                    ostringstream os;
                    os << "value " << val << " is out of range for a byte";
                    invalidArgument(os.str());
                }
                return false;
            }
            break;
        }
        case PrimitiveInfo::KindShort:
        {
            if (Z_TYPE_P(zv) != IS_LONG)
            {
                string s = zendTypeToString(Z_TYPE_P(zv));
                if (throwException)
                {
                    ostringstream os;
                    os << "expected short value but received " << s;
                    invalidArgument(os.str());
                }
                return false;
            }
            zend_long val = Z_LVAL_P(zv);
            if (val < SHRT_MIN || val > SHRT_MAX)
            {
                if (throwException)
                {
                    ostringstream os;
                    os << "value " << val << " is out of range for a short";
                    invalidArgument(os.str());
                }
                return false;
            }
            break;
        }
        case PrimitiveInfo::KindInt:
        {
            if (Z_TYPE_P(zv) != IS_LONG)
            {
                string s = zendTypeToString(Z_TYPE_P(zv));
                if (throwException)
                {
                    ostringstream os;
                    os << "expected int value but received " << s;
                    invalidArgument(os.str());
                }
                return false;
            }
            zend_long val = Z_LVAL_P(zv);
            if (val < INT_MIN || val > INT_MAX)
            {
                if (throwException)
                {
                    ostringstream os;
                    os << "value " << val << " is out of range for an int";
                    invalidArgument(os.str());
                }
                return false;
            }
            break;
        }
        case PrimitiveInfo::KindLong:
        {
            // The platform's 'long' type may not be 64 bits, so we also accept a string argument for this type.
            if (Z_TYPE_P(zv) != IS_LONG && Z_TYPE_P(zv) != IS_STRING)
            {
                string s = zendTypeToString(Z_TYPE_P(zv));
                if (throwException)
                {
                    ostringstream os;
                    os << "expected long value but received " << s;
                    invalidArgument(os.str());
                }
                return false;
            }

            if (Z_TYPE_P(zv) != IS_LONG)
            {
                string sval(Z_STRVAL_P(zv), Z_STRLEN_P(zv));
                try
                {
                    std::stoll(sval, nullptr, 0);
                }
                catch (const std::exception&)
                {
                    if (throwException)
                    {
                        ostringstream os;
                        os << "invalid long value `" << sval << "'";
                        invalidArgument(os.str());
                    }
                    return false;
                }
            }
            break;
        }
        case PrimitiveInfo::KindFloat:
        {
            if (Z_TYPE_P(zv) != IS_DOUBLE && Z_TYPE_P(zv) != IS_LONG)
            {
                string s = zendTypeToString(Z_TYPE_P(zv));
                if (throwException)
                {
                    ostringstream os;
                    os << "expected float value but received " << s;
                    invalidArgument(os.str());
                }
                return false;
            }
            if (Z_TYPE_P(zv) == IS_DOUBLE)
            {
                double val = Z_DVAL_P(zv);
                return (val <= numeric_limits<float>::max() && val >= -numeric_limits<float>::max()) || !isfinite(val);
            }
            break;
        }
        case PrimitiveInfo::KindDouble:
        {
            if (Z_TYPE_P(zv) != IS_DOUBLE && Z_TYPE_P(zv) != IS_LONG)
            {
                string s = zendTypeToString(Z_TYPE_P(zv));
                if (throwException)
                {
                    ostringstream os;
                    os << "expected double value but received " << s;
                    invalidArgument(os.str());
                }
                return false;
            }
            break;
        }
        case PrimitiveInfo::KindString:
        {
            if (Z_TYPE_P(zv) != IS_STRING && Z_TYPE_P(zv) != IS_NULL)
            {
                string s = zendTypeToString(Z_TYPE_P(zv));
                if (throwException)
                {
                    ostringstream os;
                    os << "expected string value but received " << s;
                    invalidArgument(os.str());
                }
                return false;
            }
            break;
        }
    }

    return true;
}

bool
IcePHP::PrimitiveInfo::variableLength() const
{
    return kind == KindString;
}

int
IcePHP::PrimitiveInfo::wireSize() const
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
IcePHP::PrimitiveInfo::optionalFormat() const
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
IcePHP::PrimitiveInfo::marshal(zval* zv, Ice::OutputStream* os, ObjectMap*, bool)
{
    switch (kind)
    {
        case PrimitiveInfo::KindBool:
        {
            assert(Z_TYPE_P(zv) == IS_TRUE || Z_TYPE_P(zv) == IS_FALSE);
            os->write(Z_TYPE_P(zv) == IS_TRUE);
            break;
        }
        case PrimitiveInfo::KindByte:
        {
            assert(Z_TYPE_P(zv) == IS_LONG);
            long val = static_cast<long>(Z_LVAL_P(zv));
            assert(val >= 0 && val <= 255); // validate() should have caught this.
            os->write(static_cast<uint8_t>(val));
            break;
        }
        case PrimitiveInfo::KindShort:
        {
            assert(Z_TYPE_P(zv) == IS_LONG);
            long val = static_cast<long>(Z_LVAL_P(zv));
            assert(val >= SHRT_MIN && val <= SHRT_MAX); // validate() should have caught this.
            os->write(static_cast<int16_t>(val));
            break;
        }
        case PrimitiveInfo::KindInt:
        {
            assert(Z_TYPE_P(zv) == IS_LONG);
            long val = static_cast<long>(Z_LVAL_P(zv));
            assert(val >= INT_MIN && val <= INT_MAX); // validate() should have caught this.
            os->write(static_cast<int32_t>(val));
            break;
        }
        case PrimitiveInfo::KindLong:
        {
            // The platform's 'long' type may not be 64 bits, so we also accept a string argument for this type.
            assert(Z_TYPE_P(zv) == IS_LONG || Z_TYPE_P(zv) == IS_STRING); // validate() should have caught this.
            int64_t val = 0;
            if (Z_TYPE_P(zv) == IS_LONG)
            {
                val = static_cast<int64_t>(Z_LVAL_P(zv));
            }
            else
            {
                string sval(Z_STRVAL_P(zv), Z_STRLEN_P(zv));
                try
                {
                    val = std::stoll(sval, nullptr, 0);
                }
                catch (const std::exception&)
                {
                    assert(false); // validate() should have caught this.
                    throw;
                }
            }
            os->write(static_cast<int64_t>(val));
            break;
        }
        case PrimitiveInfo::KindFloat:
        {
            double val = 0;
            if (Z_TYPE_P(zv) == IS_DOUBLE)
            {
                val = Z_DVAL_P(zv);
            }
            else if (Z_TYPE_P(zv) == IS_LONG)
            {
                val = static_cast<double>(Z_LVAL_P(zv));
            }
            else
            {
                assert(false); // validate() should have caught this.
            }
            os->write(static_cast<float>(val));
            break;
        }
        case PrimitiveInfo::KindDouble:
        {
            double val = 0;
            if (Z_TYPE_P(zv) == IS_DOUBLE)
            {
                val = Z_DVAL_P(zv);
            }
            else if (Z_TYPE_P(zv) == IS_LONG)
            {
                val = static_cast<double>(Z_LVAL_P(zv));
            }
            else
            {
                assert(false); // validate() should have caught this.
            }
            os->write(val);
            break;
        }
        case PrimitiveInfo::KindString:
        {
            assert(Z_TYPE_P(zv) == IS_STRING || Z_TYPE_P(zv) == IS_NULL); // validate() should have caught this.
            if (Z_TYPE_P(zv) == IS_STRING)
            {
                string val(Z_STRVAL_P(zv), Z_STRLEN_P(zv));
                os->write(val);
            }
            else
            {
                os->write(string());
            }
            break;
        }
    }
}

void
IcePHP::PrimitiveInfo::unmarshal(
    Ice::InputStream* is,
    const UnmarshalCallbackPtr& cb,
    const CommunicatorInfoPtr&,
    zval* target,
    void* closure,
    bool)
{
    zval zv;
    AutoDestroy destroy(&zv);

    switch (kind)
    {
        case PrimitiveInfo::KindBool:
        {
            bool val;
            is->read(val);
            ZVAL_BOOL(&zv, val ? 1 : 0);
            break;
        }
        case PrimitiveInfo::KindByte:
        {
            uint8_t val;
            is->read(val);
            ZVAL_LONG(&zv, val & 0xff);
            break;
        }
        case PrimitiveInfo::KindShort:
        {
            int16_t val;
            is->read(val);
            ZVAL_LONG(&zv, val);
            break;
        }
        case PrimitiveInfo::KindInt:
        {
            int32_t val;
            is->read(val);
            ZVAL_LONG(&zv, val);
            break;
        }
        case PrimitiveInfo::KindLong:
        {
            int64_t val;
            is->read(val);

            // The platform's 'long' type may not be 64 bits, so we store 64-bit values as a string.
            if (sizeof(int64_t) > sizeof(long) && (val < LONG_MIN || val > LONG_MAX))
            {
                string str = std::to_string(val);
                ZVAL_STRINGL(&zv, str.c_str(), static_cast<int>(str.length()));
            }
            else
            {
                ZVAL_LONG(&zv, static_cast<long>(val));
            }
            break;
        }
        case PrimitiveInfo::KindFloat:
        {
            float val;
            is->read(val);
            ZVAL_DOUBLE(&zv, val);
            break;
        }
        case PrimitiveInfo::KindDouble:
        {
            double val;
            is->read(val);
            ZVAL_DOUBLE(&zv, val);
            break;
        }
        case PrimitiveInfo::KindString:
        {
            string val;
            is->read(val);
            ZVAL_STRINGL(&zv, val.c_str(), static_cast<int>(val.length()));
            break;
        }
    }
    cb->unmarshaled(&zv, target, closure);
}

void
IcePHP::PrimitiveInfo::print(zval* zv, IceInternal::Output& out, PrintObjectHistory*)
{
    if (!validate(zv, false))
    {
        out << "<invalid value - expected " << getId() << ">";
        return;
    }
    zval tmp;
    ZVAL_STR(&tmp, zval_get_string(zv));
    out << Z_STRVAL(tmp);
}

// EnumInfo implementation.
IcePHP::EnumInfo::EnumInfo(string ident, zval* en) : id(std::move(ident)), maxValue(0)
{
    HashTable* arr = Z_ARRVAL_P(en);
    HashPosition pos;
    zval* val;

    zend_hash_internal_pointer_reset_ex(arr, &pos);
    while ((val = zend_hash_get_current_data_ex(arr, &pos)) != 0)
    {
        assert(Z_TYPE_P(val) == IS_STRING);
        string name = Z_STRVAL_P(val);
        zend_hash_move_forward_ex(arr, &pos);

        val = zend_hash_get_current_data_ex(arr, &pos);
        assert(Z_TYPE_P(val) == IS_LONG);
        int32_t value = static_cast<int32_t>(Z_LVAL_P(val));
        zend_hash_move_forward_ex(arr, &pos);

        if (value > maxValue)
        {
            const_cast<int&>(maxValue) = value;
        }

        const_cast<map<int32_t, string>&>(enumerators)[value] = name;
    }
}

string
IcePHP::EnumInfo::getId() const
{
    return id;
}

bool
IcePHP::EnumInfo::validate(zval* zv, bool)
{
    if (Z_TYPE_P(zv) == IS_LONG)
    {
        const int32_t l = static_cast<int32_t>(Z_LVAL_P(zv));
        return l >= 0 && enumerators.find(l) != enumerators.end();
    }
    return false;
}

bool
IcePHP::EnumInfo::variableLength() const
{
    return true;
}

int
IcePHP::EnumInfo::wireSize() const
{
    return 1;
}

Ice::OptionalFormat
IcePHP::EnumInfo::optionalFormat() const
{
    return Ice::OptionalFormat::Size;
}

void
IcePHP::EnumInfo::marshal(zval* zv, Ice::OutputStream* os, ObjectMap*, bool)
{
    assert(Z_TYPE_P(zv) == IS_LONG); // validate() should have caught this.
    const int32_t val = static_cast<int32_t>(Z_LVAL_P(zv));
    assert(val >= 0 && enumerators.find(val) != enumerators.end()); // validate() should have caught this.

    os->writeEnum(val, maxValue);
}

void
IcePHP::EnumInfo::unmarshal(
    Ice::InputStream* is,
    const UnmarshalCallbackPtr& cb,
    const CommunicatorInfoPtr&,
    zval* target,
    void* closure,
    bool)
{
    zval zv;
    AutoDestroy destroy(&zv);

    const int32_t val = is->readEnum(maxValue);

    if (enumerators.find(val) == enumerators.end())
    {
        ostringstream os;
        os << "enumerator " << val << " is out of range for enum " << id;
        invalidArgument(os.str());
        throw AbortMarshaling();
    }

    ZVAL_LONG(&zv, val);
    cb->unmarshaled(&zv, target, closure);
}

void
IcePHP::EnumInfo::print(zval* zv, IceInternal::Output& out, PrintObjectHistory*)
{
    if (!validate(zv, false))
    {
        out << "<invalid value - expected " << id << ">";
        return;
    }
    const int32_t val = static_cast<int32_t>(Z_LVAL_P(zv));
    map<int32_t, string>::const_iterator p = enumerators.find(val);
    assert(p != enumerators.end());
    out << p->second;
}

// DataMember implementation.
void
IcePHP::DataMember::unmarshaled(zval* zv, zval* target, void*)
{
    setMember(target, zv);
}

void
IcePHP::DataMember::setMember(zval* target, zval* zv)
{
    assert(Z_TYPE_P(target) == IS_OBJECT);
    zend_update_property(Z_OBJCE_P(target), Z_OBJ_P(target), name.c_str(), strlen(name.c_str()), zv);
}

static void
convertDataMembers(zval* zv, DataMemberList& reqMembers, DataMemberList& optMembers, bool allowOptional)
{
    list<DataMemberPtr> optList;

    assert(Z_TYPE_P(zv) == IS_ARRAY);
    HashTable* membersArray = Z_ARRVAL_P(zv);
    zval* arr;

    ZEND_HASH_FOREACH_VAL(membersArray, arr)
    {
        DataMemberPtr m = make_shared<DataMember>();
        zval* elem;

        assert(Z_TYPE_P(arr) == IS_ARRAY);
        HashTable* member = Z_ARRVAL_P(arr);
        assert(zend_hash_num_elements(member) == static_cast<uint32_t>(allowOptional ? 4 : 2));

        elem = zend_hash_index_find(member, 0);
        assert(Z_TYPE_P(elem) == IS_STRING);
        m->name = Z_STRVAL_P(elem);

        elem = zend_hash_index_find(member, 1);
        assert(Z_TYPE_P(elem) == IS_OBJECT);
        m->type = Wrapper<TypeInfoPtr>::value(elem);

        if (allowOptional)
        {
            elem = zend_hash_index_find(member, 2);
            assert(Z_TYPE_P(elem) == IS_TRUE || Z_TYPE_P(elem) == IS_FALSE);
            m->optional = Z_TYPE_P(elem) == IS_TRUE;

            elem = zend_hash_index_find(member, 3);
            assert(Z_TYPE_P(elem) == IS_LONG);
            m->tag = static_cast<int>(Z_LVAL_P(elem));
        }
        else
        {
            m->optional = false;
            m->tag = 0;
        }

        if (m->optional)
        {
            optList.push_back(m);
        }
        else
        {
            reqMembers.push_back(m);
        }
    }
    ZEND_HASH_FOREACH_END();

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

// StructInfo implementation.
IcePHP::StructInfo::StructInfo(string ident, const string& n, zval* m) : id(std::move(ident)), name(n)
{
    // Set to undefined
    ZVAL_UNDEF(&_nullMarshalValue);

    DataMemberList opt;
    convertDataMembers(m, const_cast<DataMemberList&>(members), opt, false);
    assert(opt.empty());
    const_cast<zend_class_entry*&>(zce) = nameToClass(name);
    assert(zce);

    _variableLength = false;
    _wireSize = 0;
    for (DataMemberList::const_iterator p = members.begin(); p != members.end(); ++p)
    {
        if (!_variableLength && (*p)->type->variableLength())
        {
            _variableLength = true;
        }
        _wireSize += (*p)->type->wireSize();
    }
}

string
IcePHP::StructInfo::getId() const
{
    return id;
}

bool
IcePHP::StructInfo::validate(zval* zv, bool throwException)
{
    if (Z_TYPE_P(zv) == IS_NULL)
    {
        return true;
    }
    else if (Z_TYPE_P(zv) != IS_OBJECT)
    {
        if (throwException)
        {
            string s = zendTypeToString(Z_TYPE_P(zv));
            ostringstream os;
            os << "expected struct value of type " << zce->name->val << " but received " << s;
            invalidArgument(os.str());
        }
        return false;
    }

    // Compare class entries.
    zend_class_entry* ce = Z_OBJCE_P(zv);
    if (ce != zce)
    {
        ostringstream os;
        os << "expected struct value of type " << zce->name->val << " but received " << ce->name->val;
        invalidArgument(os.str());
        return false;
    }

    return true;
}

bool
IcePHP::StructInfo::variableLength() const
{
    return _variableLength;
}

int
IcePHP::StructInfo::wireSize() const
{
    return _wireSize;
}

Ice::OptionalFormat
IcePHP::StructInfo::optionalFormat() const
{
    return _variableLength ? Ice::OptionalFormat::FSize : Ice::OptionalFormat::VSize;
}

bool
IcePHP::StructInfo::usesClasses() const
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
IcePHP::StructInfo::marshal(zval* zv, Ice::OutputStream* os, ObjectMap* objectMap, bool optional)
{
    assert(Z_TYPE_P(zv) == IS_NULL || (Z_TYPE_P(zv) == IS_OBJECT && Z_OBJCE_P(zv) == zce));

    if (Z_TYPE_P(zv) == IS_NULL)
    {
        if (Z_ISUNDEF(_nullMarshalValue))
        {
            if (object_init_ex(&_nullMarshalValue, const_cast<zend_class_entry*>(zce)) != SUCCESS)
            {
                ostringstream os;
                os << "unable to initialize object of type " << zce->name->val;
                runtimeError(os.str());
                throw AbortMarshaling();
            }

            if (!invokeMethod(&_nullMarshalValue, ZEND_CONSTRUCTOR_FUNC_NAME))
            {
                assert(false);
            }
        }
        assert(!Z_ISUNDEF(_nullMarshalValue));
        ZVAL_COPY_VALUE(zv, &_nullMarshalValue);
    }

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

    for (const auto& member : members)
    {
        zval* val = zend_hash_str_find(Z_OBJPROP_P(zv), member->name.c_str(), member->name.size());
        if (!val)
        {
            ostringstream os;
            os << "member '" << member->name << "' of " << id << " is not defined";
            runtimeError(os.str());
            throw AbortMarshaling();
        }

        if (Z_TYPE_P(val) == IS_INDIRECT)
        {
            val = Z_INDIRECT_P(val);
        }

        if (!member->type->validate(val, false))
        {
            ostringstream os;
            os << "invalid value for " << id << " member '" << member->name << "'";
            invalidArgument(os.str());
            throw AbortMarshaling();
        }

        member->type->marshal(val, os, objectMap, false);
    }

    if (optional && _variableLength)
    {
        os->endSize(sizePos);
    }
}

void
IcePHP::StructInfo::unmarshal(
    Ice::InputStream* is,
    const UnmarshalCallbackPtr& cb,
    const CommunicatorInfoPtr& comm,
    zval* target,
    void* closure,
    bool optional)
{
    zval zv;
    AutoDestroy destroy(&zv);
    if (object_init_ex(&zv, const_cast<zend_class_entry*>(zce)) != SUCCESS)
    {
        ostringstream os;
        os << "unable to initialize object of type " << zce->name->val;
        runtimeError(os.str());
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

    for (const auto& member : members)
    {
        member->type->unmarshal(is, member, comm, &zv, 0, false);
    }
    cb->unmarshaled(&zv, target, closure);
}

void
IcePHP::StructInfo::print(zval* zv, IceInternal::Output& out, PrintObjectHistory* history)
{
    if (!validate(zv, false))
    {
        out << "<invalid value - expected " << id << ">";
        return;
    }

    if (Z_TYPE_P(zv) == IS_NULL)
    {
        out << "<nil>";
    }
    else
    {
        out.sb();
        for (const auto& member : members)
        {
            out << nl << member->name << " = ";
            zval* val = zend_hash_str_find(Z_OBJPROP_P(zv), member->name.c_str(), member->name.size());
            assert(Z_TYPE_P(val) == IS_INDIRECT);
            val = Z_INDIRECT_P(val);
            if (val)
            {
                member->type->print(val, out, history);
            }
            else
            {
                out << "<not defined>";
            }
        }
        out.eb();
    }
}

void
IcePHP::StructInfo::destroy()
{
    for (const auto& p : members)
    {
        p->type->destroy();
    }
    const_cast<DataMemberList&>(members).clear();
    if (!Z_ISUNDEF(_nullMarshalValue))
    {
        zval_ptr_dtor(&_nullMarshalValue);
    }
}

// SequenceInfo implementation.
IcePHP::SequenceInfo::SequenceInfo(string ident, zval* e) : id(std::move(ident))
{
    const_cast<TypeInfoPtr&>(elementType) = Wrapper<TypeInfoPtr>::value(e);
}

string
IcePHP::SequenceInfo::getId() const
{
    return id;
}

bool
IcePHP::SequenceInfo::validate(zval* zv, bool)
{
    return Z_TYPE_P(zv) == IS_NULL || Z_TYPE_P(zv) == IS_ARRAY;
}

bool
IcePHP::SequenceInfo::variableLength() const
{
    return true;
}

int
IcePHP::SequenceInfo::wireSize() const
{
    return 1;
}

Ice::OptionalFormat
IcePHP::SequenceInfo::optionalFormat() const
{
    return elementType->variableLength() ? Ice::OptionalFormat::FSize : Ice::OptionalFormat::VSize;
}

bool
IcePHP::SequenceInfo::usesClasses() const
{
    return elementType->usesClasses();
}

void
IcePHP::SequenceInfo::marshal(zval* zv, Ice::OutputStream* os, ObjectMap* objectMap, bool optional)
{
    int32_t sz = 0;
    HashTable* arr = 0;

    if (Z_TYPE_P(zv) != IS_NULL)
    {
        assert(Z_TYPE_P(zv) == IS_ARRAY); // validate() should have caught this.
        arr = Z_ARRVAL_P(zv);
        sz = static_cast<int32_t>(zend_hash_num_elements(arr));
    }

    Ice::OutputStream::size_type sizePos = 0;
    if (optional)
    {
        if (elementType->variableLength())
        {
            sizePos = os->startSize();
        }
        else if (elementType->wireSize() > 1)
        {
            os->writeSize(sz == 0 ? 1 : sz * elementType->wireSize() + (sz > 254 ? 5 : 1));
        }
    }

    if (sz == 0)
    {
        os->writeSize(0);
    }
    else
    {
        PrimitiveInfoPtr pi = dynamic_pointer_cast<PrimitiveInfo>(elementType);
        if (pi)
        {
            marshalPrimitiveSequence(pi, zv, os);
            return;
        }

        os->writeSize(sz);

        zval* val;
        ZEND_HASH_FOREACH_VAL(arr, val)
        {
            if (!elementType->validate(val, false))
            {
                ostringstream os;
                os << "invalid value for sequence element '" << id << "'";
                invalidArgument(os.str());
                throw AbortMarshaling();
            }
            elementType->marshal(val, os, objectMap, false);
        }
        ZEND_HASH_FOREACH_END();
    }

    if (optional && elementType->variableLength())
    {
        os->endSize(sizePos);
    }
}

void
IcePHP::SequenceInfo::unmarshal(
    Ice::InputStream* is,
    const UnmarshalCallbackPtr& cb,
    const CommunicatorInfoPtr& comm,
    zval* target,
    void* closure,
    bool optional)
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

    PrimitiveInfoPtr pi = dynamic_pointer_cast<PrimitiveInfo>(elementType);
    if (pi)
    {
        unmarshalPrimitiveSequence(pi, is, cb, target, closure);
        return;
    }

    zval zv;
    array_init(&zv);
    AutoDestroy destroy(&zv);

    int32_t sz = is->readSize();
    for (int32_t i = 0; i < sz; ++i)
    {
        void* cl = reinterpret_cast<void*>(i);

        // Add a temporary null value so that the foreach order is the same as the index order.
#ifdef HT_ALLOW_COW_VIOLATION
        HT_ALLOW_COW_VIOLATION(Z_ARRVAL(zv));
#endif
        add_index_null(&zv, i);
        elementType->unmarshal(is, shared_from_this(), comm, &zv, cl, false);
    }

    cb->unmarshaled(&zv, target, closure);
}

void
IcePHP::SequenceInfo::print(zval* zv, IceInternal::Output& out, PrintObjectHistory* history)
{
    if (!validate(zv, false))
    {
        out << "<invalid value - expected " << id << ">";
        return;
    }

    if (Z_TYPE_P(zv) == IS_NULL)
    {
        out << "{}";
    }
    else
    {
        assert(Z_TYPE_P(zv) == IS_ARRAY);

        HashTable* arr = Z_ARRVAL_P(zv);

        out.sb();

        int i = 0;
        zval* val;
        ZEND_HASH_FOREACH_VAL(arr, val)
        {
            out << nl << '[' << i << "] = ";
            elementType->print(val, out, history);
            ++i;
        }
        ZEND_HASH_FOREACH_END();

        out.eb();
    }
}

void
IcePHP::SequenceInfo::unmarshaled(zval* zv, zval* target, void* closure)
{
    assert(Z_TYPE_P(target) != IS_REFERENCE);
    assert(Z_TYPE_P(target) == IS_ARRAY);
    long i = reinterpret_cast<long>(closure);
    Z_TRY_ADDREF(*zv);
    add_index_zval(target, i, zv);
}

void
IcePHP::SequenceInfo::destroy()
{
    if (elementType)
    {
        elementType->destroy();
        const_cast<TypeInfoPtr&>(elementType) = 0;
    }
}

void
IcePHP::SequenceInfo::marshalPrimitiveSequence(const PrimitiveInfoPtr& pi, zval* zv, Ice::OutputStream* os)
{
    HashTable* arr = Z_ARRVAL_P(zv);

    int32_t sz = static_cast<int32_t>(zend_hash_num_elements(arr));
    assert(sz > 0);

    switch (pi->kind)
    {
        case PrimitiveInfo::KindBool:
        {
            Ice::BoolSeq seq(sz);
            int32_t i = 0;
            zval* val;
            ZEND_HASH_FOREACH_VAL(arr, val)
            {
                if (!pi->validate(val, true))
                {
                    throw AbortMarshaling();
                }
                seq[i++] = Z_TYPE_P(val) == IS_TRUE;
            }
            ZEND_HASH_FOREACH_END();
            os->write(seq);
            break;
        }
        case PrimitiveInfo::KindByte:
        {
            Ice::ByteSeq seq(sz);
            int32_t i = 0;
            zval* val;
            ZEND_HASH_FOREACH_VAL(arr, val)
            {
                if (!pi->validate(val, true))
                {
                    throw AbortMarshaling();
                }
                long l = static_cast<long>(Z_LVAL_P(val));
                assert(l >= 0 && l <= 255);
                seq[i++] = static_cast<byte>(l);
            }
            ZEND_HASH_FOREACH_END();

            os->write(seq);
            break;
        }
        case PrimitiveInfo::KindShort:
        {
            Ice::ShortSeq seq(sz);
            int32_t i = 0;
            zval* val;
            ZEND_HASH_FOREACH_VAL(arr, val)
            {
                if (!pi->validate(val, true))
                {
                    throw AbortMarshaling();
                }
                long l = static_cast<long>(Z_LVAL_P(val));
                assert(l >= SHRT_MIN && l <= SHRT_MAX);
                seq[i++] = static_cast<int16_t>(l);
            }
            ZEND_HASH_FOREACH_END();

            os->write(seq);
            break;
        }
        case PrimitiveInfo::KindInt:
        {
            Ice::IntSeq seq(sz);
            int32_t i = 0;
            zval* val;
            ZEND_HASH_FOREACH_VAL(arr, val)
            {
                if (!pi->validate(val, true))
                {
                    throw AbortMarshaling();
                }
                long l = static_cast<long>(Z_LVAL_P(val));
                assert(l >= INT_MIN && l <= INT_MAX);
                seq[i++] = static_cast<int32_t>(l);
            }
            ZEND_HASH_FOREACH_END();

            os->write(seq);
            break;
        }
        case PrimitiveInfo::KindLong:
        {
            Ice::LongSeq seq(sz);
            int32_t i = 0;
            zval* val;
            ZEND_HASH_FOREACH_VAL(arr, val)
            {
                if (!pi->validate(val, true))
                {
                    throw AbortMarshaling();
                }
                // The platform's 'long' type may not be 64 bits, so we also accept a string argument for this type.
                assert(Z_TYPE_P(val) == IS_LONG || Z_TYPE_P(val) == IS_STRING);
                int64_t l;
                if (Z_TYPE_P(val) == IS_LONG)
                {
                    l = static_cast<int64_t>(Z_LVAL_P(val));
                }
                else
                {
                    string sval(Z_STRVAL_P(val), Z_STRLEN_P(val));
                    try
                    {
                        l = std::stoll(sval, nullptr, 0);
                    }
                    catch (const std::exception&)
                    {
                        assert(false); // validate() should have caught this.
                        throw;
                    }
                }
                seq[i++] = l;
            }
            ZEND_HASH_FOREACH_END();

            os->write(seq);
            break;
        }
        case PrimitiveInfo::KindFloat:
        {
            Ice::FloatSeq seq(sz);
            int32_t i = 0;
            zval* val;
            ZEND_HASH_FOREACH_VAL(arr, val)
            {
                if (!pi->validate(val, true))
                {
                    throw AbortMarshaling();
                }
                double d = 0;
                if (Z_TYPE_P(val) == IS_DOUBLE)
                {
                    d = Z_DVAL_P(val);
                }
                else if (Z_TYPE_P(val) == IS_LONG)
                {
                    d = static_cast<double>(Z_LVAL_P(val));
                }
                else
                {
                    assert(false); // validate() should have caught this.
                }
                seq[i++] = static_cast<float>(d);
            }
            ZEND_HASH_FOREACH_END();

            os->write(seq);
            break;
        }
        case PrimitiveInfo::KindDouble:
        {
            Ice::DoubleSeq seq(sz);
            int32_t i = 0;
            zval* val;
            ZEND_HASH_FOREACH_VAL(arr, val)
            {
                if (!pi->validate(val, true))
                {
                    throw AbortMarshaling();
                }
                double d = 0;
                if (Z_TYPE_P(val) == IS_DOUBLE)
                {
                    d = Z_DVAL_P(val);
                }
                else if (Z_TYPE_P(val) == IS_LONG)
                {
                    d = static_cast<double>(Z_LVAL_P(val));
                }
                else
                {
                    assert(false); // validate() should have caught this.
                }
                seq[i++] = d;
            }
            ZEND_HASH_FOREACH_END();

            os->write(seq);
            break;
        }
        case PrimitiveInfo::KindString:
        {
            Ice::StringSeq seq(sz);
            int32_t i = 0;
            zval* val;
            ZEND_HASH_FOREACH_VAL(arr, val)
            {
                if (!pi->validate(val, true))
                {
                    throw AbortMarshaling();
                }
                string s;
                if (Z_TYPE_P(val) == IS_STRING)
                {
                    s = string(Z_STRVAL_P(val), Z_STRLEN_P(val));
                }
                else
                {
                    assert(Z_TYPE_P(val) == IS_NULL);
                }
                seq[i++] = s;
            }
            ZEND_HASH_FOREACH_END();

            os->write(seq);
            break;
        }
    }
}

void
IcePHP::SequenceInfo::unmarshalPrimitiveSequence(
    const PrimitiveInfoPtr& pi,
    Ice::InputStream* is,
    const UnmarshalCallbackPtr& cb,
    zval* target,
    void* closure)
{
    zval zv;
    array_init(&zv);
    AutoDestroy destroy(&zv);

    switch (pi->kind)
    {
        case PrimitiveInfo::KindBool:
        {
            pair<const bool*, const bool*> pr;
            is->read(pr);
            for (const bool* p = pr.first; p != pr.second; ++p)
            {
                add_next_index_bool(&zv, *p ? 1 : 0);
            }
            break;
        }
        case PrimitiveInfo::KindByte:
        {
            pair<const uint8_t*, const uint8_t*> pr;
            is->read(pr);
            for (const uint8_t* p = pr.first; p != pr.second; ++p)
            {
                add_next_index_long(&zv, *p & 0xff);
            }
            break;
        }
        case PrimitiveInfo::KindShort:
        {
            pair<const int16_t*, const int16_t*> pr;
            is->read(pr);
            for (const int16_t* p = pr.first; p != pr.second; ++p)
            {
                add_next_index_long(&zv, *p);
            }
            break;
        }
        case PrimitiveInfo::KindInt:
        {
            pair<const int32_t*, const int32_t*> pr;
            is->read(pr);
            for (const int32_t* p = pr.first; p != pr.second; ++p)
            {
                add_next_index_long(&zv, *p);
            }
            break;
        }
        case PrimitiveInfo::KindLong:
        {
            pair<const int64_t*, const int64_t*> pr;
            is->read(pr);
            int32_t i = 0;
            for (const int64_t* p = pr.first; p != pr.second; ++p, ++i)
            {
                zval val;
                // The platform's 'long' type may not be 64 bits, so we store 64-bit values as a string.
                if (sizeof(int64_t) > sizeof(long) && (*p < LONG_MIN || *p > LONG_MAX))
                {
                    string str = std::to_string(*p);
                    ZVAL_STRINGL(&val, str.c_str(), static_cast<int>(str.length()));
                }
                else
                {
                    ZVAL_LONG(&val, static_cast<long>(*p));
                }
                add_index_zval(&zv, i, &val);
            }
            break;
        }
        case PrimitiveInfo::KindFloat:
        {
            pair<const float*, const float*> pr;
            is->read(pr);
            int32_t i = 0;
            for (const float* p = pr.first; p != pr.second; ++p, ++i)
            {
                zval val;
                ZVAL_DOUBLE(&val, *p);
                add_index_zval(&zv, i, &val);
            }
            break;
        }
        case PrimitiveInfo::KindDouble:
        {
            pair<const double*, const double*> pr;
            is->read(pr);
            int32_t i = 0;
            for (const double* p = pr.first; p != pr.second; ++p, ++i)
            {
                zval val;
                ZVAL_DOUBLE(&val, *p);
                add_index_zval(&zv, i, &val);
            }
            break;
        }
        case PrimitiveInfo::KindString:
        {
            Ice::StringSeq seq;
            is->read(seq, true);
            int32_t i = 0;
            for (const auto& p : seq)
            {
                zval val;
                ZVAL_STRINGL(&val, p.c_str(), static_cast<int>(p.length()));
                add_index_zval(&zv, i++, &val);
            }
            break;
        }
    }

    cb->unmarshaled(&zv, target, closure);
}

// DictionaryInfo implementation.
IcePHP::DictionaryInfo::DictionaryInfo(string ident, zval* k, zval* v) : id(std::move(ident))
{
    const_cast<TypeInfoPtr&>(keyType) = Wrapper<TypeInfoPtr>::value(k);
    const_cast<TypeInfoPtr&>(valueType) = Wrapper<TypeInfoPtr>::value(v);

    _variableLength = keyType->variableLength() || valueType->variableLength();
    _wireSize = keyType->wireSize() + valueType->wireSize();
}

string
IcePHP::DictionaryInfo::getId() const
{
    return id;
}

bool
IcePHP::DictionaryInfo::validate(zval* zv, bool)
{
    return Z_TYPE_P(zv) == IS_NULL || Z_TYPE_P(zv) == IS_ARRAY;
}

bool
IcePHP::DictionaryInfo::variableLength() const
{
    return true;
}

int
IcePHP::DictionaryInfo::wireSize() const
{
    return 1;
}

Ice::OptionalFormat
IcePHP::DictionaryInfo::optionalFormat() const
{
    return _variableLength ? Ice::OptionalFormat::FSize : Ice::OptionalFormat::VSize;
}

bool
IcePHP::DictionaryInfo::usesClasses() const
{
    return valueType->usesClasses();
}

void
IcePHP::DictionaryInfo::marshal(zval* zv, Ice::OutputStream* os, ObjectMap* objectMap, bool optional)
{
    int32_t sz = 0;
    HashTable* arr = 0;

    if (Z_TYPE_P(zv) != IS_NULL)
    {
        assert(Z_TYPE_P(zv) == IS_ARRAY); // validate() should have caught this.
        arr = Z_ARRVAL_P(zv);
        sz = static_cast<int32_t>(zend_hash_num_elements(arr));
    }

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

    auto piKey = dynamic_pointer_cast<PrimitiveInfo>(keyType);
    auto enKey = dynamic_pointer_cast<EnumInfo>(keyType);
    if (!enKey && (!piKey || piKey->kind == PrimitiveInfo::KindFloat || piKey->kind == PrimitiveInfo::KindDouble))
    {
        ostringstream os;
        os << "dictionary type '" << id << "' cannot be marshaled";
        invalidArgument(os.str());
        throw AbortMarshaling();
    }

    os->writeSize(sz);

    if (sz > 0)
    {
        zend_long num_key;
        zend_string* key;
        zval* val;

        ZEND_HASH_FOREACH_KEY_VAL(arr, num_key, key, val)
        {
            // Store the key (which can be a long or a string) in a zval so that we can reuse the marshaling logic.
            zval zkey;
            AutoDestroy destroy(&zkey);

            if (key)
            {
                ZVAL_STRINGL(&zkey, key->val, key->len);
            }
            else
            {
                ZVAL_LONG(&zkey, num_key);
            }

            // Convert the zval to the required type, if necessary.
            if (piKey)
            {
                switch (piKey->kind)
                {
                    case PrimitiveInfo::KindBool:
                    {
                        convert_to_boolean(&zkey);
                        break;
                    }

                    case PrimitiveInfo::KindByte:
                    case PrimitiveInfo::KindShort:
                    case PrimitiveInfo::KindInt:
                    case PrimitiveInfo::KindLong:
                    {
                        if (key) // HASH_KEY_IS_STRING
                        {
                            convert_to_long(&zkey);
                        }
                        break;
                    }

                    case PrimitiveInfo::KindString:
                    {
                        if (!key) // HASH_KEY_IS_LONG
                        {
                            convert_to_string(&zkey);
                        }
                        break;
                    }

                    case PrimitiveInfo::KindFloat:
                    case PrimitiveInfo::KindDouble:
                        assert(false);
                }
            }
            else
            {
                if (key) // HASH_KEY_IS_STRING
                {
                    convert_to_long(&zkey);
                }
            }

            // Marshal the key.
            if (!keyType->validate(&zkey, false))
            {
                ostringstream os;
                os << "invalid key in '" << id << "' element";
                invalidArgument(os.str());
                throw AbortMarshaling();
            }
            keyType->marshal(&zkey, os, objectMap, false);

            // Marshal the value.
            if (!valueType->validate(val, false))
            {
                ostringstream os;
                os << "invalid value in '" << id << "' element";
                invalidArgument(os.str());
                throw AbortMarshaling();
            }
            valueType->marshal(val, os, objectMap, false);
        }
        ZEND_HASH_FOREACH_END();
    }

    if (optional && _variableLength)
    {
        os->endSize(sizePos);
    }
}

void
IcePHP::DictionaryInfo::unmarshal(
    Ice::InputStream* is,
    const UnmarshalCallbackPtr& cb,
    const CommunicatorInfoPtr& comm,
    zval* target,
    void* closure,
    bool optional)
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

    auto piKey = dynamic_pointer_cast<PrimitiveInfo>(keyType);
    auto enKey = dynamic_pointer_cast<EnumInfo>(keyType);
    if (!enKey && (!piKey || piKey->kind == PrimitiveInfo::KindFloat || piKey->kind == PrimitiveInfo::KindDouble))
    {
        ostringstream os;
        os << "dictionary type '" << id << "' cannot be unmarshaled";
        invalidArgument(os.str());
        throw AbortMarshaling();
    }

    zval zv;
    array_init(&zv);
#ifdef HT_ALLOW_COW_VIOLATION
    HT_ALLOW_COW_VIOLATION(Z_ARRVAL(zv)); // Allow circular references.
#endif
    AutoDestroy destroy(&zv);

    int32_t sz = is->readSize();
    for (int32_t i = 0; i < sz; ++i)
    {
        // A dictionary key cannot be a class (or contain one), so the key must be available immediately.
        KeyCallbackPtr keyCB = make_shared<KeyCallback>();
        keyType->unmarshal(is, keyCB, comm, 0, 0, false);
        assert(Z_TYPE(keyCB->key) != IS_UNDEF);

        // Allocate a callback that holds a reference to the key.
        ValueCallbackPtr valueCB = make_shared<ValueCallback>(&keyCB->key);

        // Pass the key to the callback.
        valueType->unmarshal(is, valueCB, comm, &zv, 0, false);
    }

    cb->unmarshaled(&zv, target, closure);
}

void
IcePHP::DictionaryInfo::print(zval* zv, IceInternal::Output& out, PrintObjectHistory* history)
{
    if (!validate(zv, false))
    {
        out << "<invalid value - expected " << id << ">";
        return;
    }

    if (Z_TYPE_P(zv) == IS_NULL)
    {
        out << "{}";
    }
    else
    {
        HashTable* arr = Z_ARRVAL_P(zv);
        zval* val;
        zend_long num_key;
        zend_string* key;
        bool first = true;

        out.sb();

        ZEND_HASH_FOREACH_KEY_VAL(arr, num_key, key, val)
        {
            if (first)
            {
                first = false;
            }
            else
            {
                out << nl;
            }
            out << nl << "key = ";

            if (key) // HASH_KEY_IS_STRING
            {
                out << key->val;
            }
            else // HASH_KEY_IS_LONG
            {
                out << num_key;
            }
            out << nl << "value = ";
            valueType->print(val, out, history);
        }
        ZEND_HASH_FOREACH_END();

        out.eb();
    }
}

IcePHP::DictionaryInfo::KeyCallback::KeyCallback() { ZVAL_UNDEF(&key); }

IcePHP::DictionaryInfo::KeyCallback::~KeyCallback() { zval_ptr_dtor(&key); }

void
IcePHP::DictionaryInfo::KeyCallback::unmarshaled(zval* zv, zval*, void*)
{
    zval_ptr_dtor(&key);
    ZVAL_COPY(&key, zv);
}

IcePHP::DictionaryInfo::ValueCallback::ValueCallback(zval* k) { ZVAL_COPY_VALUE(&key, k); }

IcePHP::DictionaryInfo::ValueCallback::~ValueCallback() = default;

void
IcePHP::DictionaryInfo::ValueCallback::unmarshaled(zval* zv, zval* target, void*)
{
    assert(Z_TYPE_P(target) == IS_ARRAY);

    switch (Z_TYPE(key))
    {
        case IS_LONG:
            add_index_zval(target, Z_LVAL(key), zv);
            break;
        case IS_TRUE:
            add_index_zval(target, 1, zv);
            break;
        case IS_FALSE:
            add_index_zval(target, 0, zv);
            break;
        case IS_STRING:
            add_assoc_zval_ex(target, Z_STRVAL(key), Z_STRLEN(key), zv);
            break;
        default:
            assert(false);
            return;
    }

    Z_TRY_ADDREF_P(zv);
}

void
IcePHP::DictionaryInfo::destroy()
{
    if (keyType)
    {
        keyType->destroy();
        keyType = nullptr;
    }
    if (valueType)
    {
        valueType->destroy();
        valueType = nullptr;
    }
}

// ClassInfo implementation.
IcePHP::ClassInfo::ClassInfo(string ident) : id(std::move(ident)), compactId(-1), zce(0), defined(false) {}

void
IcePHP::ClassInfo::define(const string& n, int32_t compact, zval* b, zval* m)
{
    const_cast<string&>(name) = n;
    const_cast<int32_t&>(compactId) = static_cast<int32_t>(compact);

    if (b)
    {
        TypeInfoPtr p = Wrapper<TypeInfoPtr>::value(b);
        base = dynamic_pointer_cast<ClassInfo>(p);
        assert(base);
    }

    if (m)
    {
        convertDataMembers(m, const_cast<DataMemberList&>(members), const_cast<DataMemberList&>(optionalMembers), true);
    }

    const_cast<bool&>(defined) = true;
    const_cast<zend_class_entry*&>(zce) = nameToClass(name);
    assert(zce);
}

string
IcePHP::ClassInfo::getId() const
{
    return id;
}

bool
IcePHP::ClassInfo::validate(zval* val, bool)
{
    if (Z_TYPE_P(val) == IS_OBJECT)
    {
        return checkClass(Z_OBJCE_P(val), const_cast<zend_class_entry*>(zce));
    }
    return Z_TYPE_P(val) == IS_NULL;
}

bool
IcePHP::ClassInfo::variableLength() const
{
    return true;
}

int
IcePHP::ClassInfo::wireSize() const
{
    return 1;
}

Ice::OptionalFormat
IcePHP::ClassInfo::optionalFormat() const
{
    return Ice::OptionalFormat::Class;
}

bool
IcePHP::ClassInfo::usesClasses() const
{
    return true;
}

void
IcePHP::ClassInfo::marshal(zval* zv, Ice::OutputStream* os, ObjectMap* objectMap, bool)
{
    if (!defined)
    {
        ostringstream os;
        os << "class " << id << " is declared but not defined";
        runtimeError(os.str());
        throw AbortMarshaling();
    }

    if (Z_TYPE_P(zv) == IS_NULL)
    {
        shared_ptr<Ice::Value> value; // nullptr
        os->write(value);
        return;
    }

    assert(Z_TYPE_P(zv) == IS_OBJECT);                                     // validate() should have caught this.
    assert(checkClass(Z_OBJCE_P(zv), const_cast<zend_class_entry*>(zce))); // validate() should have caught this.

    // Ice::ValueWriter is a subclass of Ice::Value that wraps a PHP object for marshaling.
    // It is possible that this PHP object has already been marshaled, therefore we first must
    // check the object map to see if this object is present. If so, we use the existing ValueWriter,
    // otherwise we create a new one. The key of the map is the object's handle.
    shared_ptr<Ice::Value> writer;
    assert(objectMap);
    ObjectMap::iterator q = objectMap->find(Z_OBJ_HANDLE_P(zv));
    if (q == objectMap->end())
    {
        writer = make_shared<ValueWriter>(zv, objectMap, dynamic_pointer_cast<ClassInfo>(shared_from_this()));
        objectMap->insert(ObjectMap::value_type(Z_OBJ_HANDLE_P(zv), writer));
    }
    else
    {
        writer = q->second;
    }

    // Give the writer to the stream. The stream will eventually call write() on it.
    assert(writer);
    os->write(writer);
}

namespace
{
    void patchObject(void* addr, const shared_ptr<Ice::Value>& v)
    {
        ReadObjectCallback* cb = static_cast<ReadObjectCallback*>(addr);
        assert(cb);
        cb->invoke(v);
    }
}

void
IcePHP::ClassInfo::unmarshal(
    Ice::InputStream* is,
    const UnmarshalCallbackPtr& cb,
    const CommunicatorInfoPtr& comm,
    zval* target,
    void* closure,
    bool)
{
    if (!defined)
    {
        ostringstream os;
        os << "class or interface " << id << " is declared but not defined";
        runtimeError(os.str());
        throw AbortMarshaling();
    }

    // This callback is notified when the Slice value is actually read. The StreamUtil object attached to the stream
    // keeps a reference to the callback object to ensure it lives long enough.
    ReadObjectCallbackPtr rocb =
        make_shared<ReadObjectCallback>(dynamic_pointer_cast<ClassInfo>(shared_from_this()), cb, target, closure);
    StreamUtil* util = reinterpret_cast<StreamUtil*>(is->getClosure());
    assert(util);
    util->add(rocb);
    is->read(patchObject, rocb.get());
}

void
IcePHP::ClassInfo::print(zval* zv, IceInternal::Output& out, PrintObjectHistory* history)
{
    if (!validate(zv, false))
    {
        out << "<invalid value - expected " << id << ">";
        return;
    }

    if (Z_TYPE_P(zv) == IS_NULL)
    {
        out << "<nil>";
    }
    else
    {
        map<unsigned int, int>::iterator q = history->objects.find(Z_OBJ_HANDLE_P(zv));
        if (q != history->objects.end())
        {
            out << "<object #" << q->second << ">";
        }
        else
        {
            out << "object #" << history->index << " (" << id << ')';
            history->objects.insert(map<unsigned int, int>::value_type(Z_OBJ_HANDLE_P(zv), history->index));
            ++history->index;
            out.sb();
            printMembers(zv, out, history);
            out.eb();
        }
    }
}

void
IcePHP::ClassInfo::destroy()
{
    const_cast<ClassInfoPtr&>(base) = 0;
    if (!members.empty())
    {
        DataMemberList ml = members;
        const_cast<DataMemberList&>(members).clear();
        for (const auto& p : ml)
        {
            p->type->destroy();
        }
    }
}

void
IcePHP::ClassInfo::printMembers(zval* zv, IceInternal::Output& out, PrintObjectHistory* history)
{
    if (base)
    {
        base->printMembers(zv, out, history);
    }

    for (const auto& member : members)
    {
        out << nl << member->name << " = ";
        zval* val = zend_hash_str_find(Z_OBJPROP_P(zv), member->name.c_str(), member->name.size());
        assert(Z_TYPE_P(val) == IS_INDIRECT);
        val = Z_INDIRECT_P(val);
        if (val)
        {
            member->type->print(val, out, history);
        }
        else
        {
            out << "<not defined>";
        }
    }

    for (const auto& member : members)
    {
        out << nl << member->name << " = ";
        zval* val = zend_hash_str_find(Z_OBJPROP_P(zv), member->name.c_str(), member->name.size());
        assert(Z_TYPE_P(val) == IS_INDIRECT);
        val = Z_INDIRECT_P(val);
        if (val)
        {
            if (isUnset(val))
            {
                out << "<unset>";
            }
            else
            {
                member->type->print(val, out, history);
            }
        }
        else
        {
            out << "<not defined>";
        }
    }
}

bool
IcePHP::ClassInfo::isA(string_view typeId) const
{
    if (id == typeId)
    {
        return true;
    }

    return base && base->isA(typeId);
}

// ProxyInfo implementation.
IcePHP::ProxyInfo::ProxyInfo(string ident) : id(std::move(ident)), defined(false) {}

void
IcePHP::ProxyInfo::define(zval* b, zval* i)
{
    if (b)
    {
        TypeInfoPtr p = Wrapper<TypeInfoPtr>::value(b);
        base = dynamic_pointer_cast<ProxyInfo>(p);
        assert(base);
    }

    if (i)
    {
        HashTable* interfacesArray = Z_ARRVAL_P(i);
        zval* interfaceType;

        ZEND_HASH_FOREACH_VAL(interfacesArray, interfaceType)
        {
            TypeInfoPtr t = Wrapper<TypeInfoPtr>::value(interfaceType);
            ProxyInfoPtr c = dynamic_pointer_cast<ProxyInfo>(t);
            assert(c);
            const_cast<ProxyInfoList&>(interfaces).push_back(c);
        }
        ZEND_HASH_FOREACH_END();
    }

    const_cast<bool&>(defined) = true;
}

string
IcePHP::ProxyInfo::getId() const
{
    return id;
}

bool
IcePHP::ProxyInfo::validate(zval* zv, bool throwException)
{
    if (Z_TYPE_P(zv) != IS_NULL)
    {
        if (Z_TYPE_P(zv) != IS_OBJECT || (Z_TYPE_P(zv) == IS_OBJECT && Z_OBJCE_P(zv) != proxyClassEntry))
        {
            if (throwException)
            {
                string s = zendTypeToString(Z_TYPE_P(zv));
                ostringstream os;
                os << "expected proxy value or null but received " << s;
                invalidArgument(os.str());
            }
            return false;
        }
    }

    return true;
}

bool
IcePHP::ProxyInfo::variableLength() const
{
    return true;
}

int
IcePHP::ProxyInfo::wireSize() const
{
    return 1;
}

Ice::OptionalFormat
IcePHP::ProxyInfo::optionalFormat() const
{
    return Ice::OptionalFormat::FSize;
}

void
IcePHP::ProxyInfo::marshal(zval* zv, Ice::OutputStream* os, ObjectMap*, bool optional)
{
    Ice::OutputStream::size_type sizePos = 0;
    if (optional)
    {
        sizePos = os->startSize();
    }

    std::optional<Ice::ObjectPrx> proxy;
    if (Z_TYPE_P(zv) == IS_NULL)
    {
        os->write(proxy);
    }
    else
    {
        assert(Z_TYPE_P(zv) == IS_OBJECT && Z_OBJCE_P(zv) == proxyClassEntry); // validate() should have caught this.
        ProxyInfoPtr info;
        if (!fetchProxy(zv, proxy, info))
        {
            throw AbortMarshaling();
        }
        if (!info->isA(id))
        {
            ostringstream os;
            os << "proxy is not narrowed to " << id;
            invalidArgument(os.str());
            throw AbortMarshaling();
        }
        os->write(proxy);
    }

    if (optional)
    {
        os->endSize(sizePos);
    }
}

void
IcePHP::ProxyInfo::unmarshal(
    Ice::InputStream* is,
    const UnmarshalCallbackPtr& cb,
    const CommunicatorInfoPtr& comm,
    zval* target,
    void* closure,
    bool optional)
{
    zval zv;
    AutoDestroy destroy(&zv);

    if (optional)
    {
        is->skip(4);
    }

    std::optional<Ice::ObjectPrx> proxy;
    is->read(proxy);

    if (!proxy)
    {
        ZVAL_NULL(&zv);
        cb->unmarshaled(&zv, target, closure);
        return;
    }

    if (!defined)
    {
        ostringstream os;
        os << "proxy " << id << " is declared but not defined";
        runtimeError(os.str());
        throw AbortMarshaling();
    }

    if (!createProxy(&zv, proxy.value(), dynamic_pointer_cast<ProxyInfo>(shared_from_this()), comm))
    {
        throw AbortMarshaling();
    }
    cb->unmarshaled(&zv, target, closure);
}

void
IcePHP::ProxyInfo::print(zval* zv, IceInternal::Output& out, PrintObjectHistory*)
{
    if (!validate(zv, false))
    {
        out << "<invalid value - expected " << id << ">";
        return;
    }

    if (Z_TYPE_P(zv) == IS_NULL)
    {
        out << "<nil>";
    }
    else
    {
        optional<Ice::ObjectPrx> proxy;
        ProxyInfoPtr info;
        if (!fetchProxy(zv, proxy, info))
        {
            return;
        }
        out << proxy->ice_toString();
    }
}

void
IcePHP::ProxyInfo::destroy()
{
    const_cast<OperationMap&>(operations).clear();

    for (const auto& p : interfaces)
    {
        p->destroy();
    }
    const_cast<ProxyInfoList&>(interfaces).clear();

    if (base)
    {
        const_cast<ProxyInfoPtr&>(base)->destroy();
        const_cast<ProxyInfoPtr&>(base) = 0;
    }
}

bool
IcePHP::ProxyInfo::isA(string_view typeId) const
{
    if (id == typeId)
    {
        return true;
    }

    if (base && base->isA(typeId))
    {
        return true;
    }

    for (const auto& p : interfaces)
    {
        if (p->isA(typeId))
        {
            return true;
        }
    }

    return false;
}

void
IcePHP::ProxyInfo::addOperation(const string& mappedName, const OperationPtr& op)
{
    operations.insert(OperationMap::value_type(mappedName, op));
}

IcePHP::OperationPtr
IcePHP::ProxyInfo::getOperation(const string& mappedName) const
{
    OperationPtr op;
    OperationMap::const_iterator p = operations.find(mappedName);
    if (p != operations.end())
    {
        op = p->second;
    }

    if (!op && base)
    {
        op = base->getOperation(mappedName);
    }

    if (!op && !interfaces.empty())
    {
        for (const auto& q : interfaces)
        {
            op = q->getOperation(mappedName);
            if (op)
            {
                break;
            }
        }
    }
    return op;
}

// ValueWriter implementation.
IcePHP::ValueWriter::ValueWriter(zval* object, ObjectMap* objectMap, ClassInfoPtr formal)
    : _map(objectMap),
      _formal(std::move(formal))
{
    // Copy zval and increase ref count
    ZVAL_COPY(&_object, object);

    // We need to determine the most-derived Slice type supported by this object. This is typically a Slice class,
    // but it can also be an interface.
    //
    // The caller may have provided a ClassInfo representing the formal type, in which case we ensure that the
    // actual type is compatible with the formal type.
    _info = getClassInfoByClass(Z_OBJCE_P(object), formal ? const_cast<zend_class_entry*>(formal->zce) : 0);
    assert(_info);
}

IcePHP::ValueWriter::~ValueWriter() { zval_ptr_dtor(&_object); }

void
IcePHP::ValueWriter::ice_preMarshal()
{
    string name = "ice_premarshal"; // Must be lowercase.
    if (zend_hash_str_exists(&Z_OBJCE_P(&_object)->function_table, name.c_str(), static_cast<uint32_t>(name.size())))
    {
        if (!invokeMethod(&_object, name))
        {
            throw AbortMarshaling();
        }
    }
}

void
IcePHP::ValueWriter::_iceWrite(Ice::OutputStream* os) const
{
    // Retrieve the SlicedData object that we stored as a hidden member of the PHP object.
    Ice::SlicedDataPtr slicedData =
        StreamUtil::getSlicedDataMember(const_cast<zval*>(&_object), const_cast<ObjectMap*>(_map));

    os->startValue(slicedData);
    if (_info->id != "::Ice::UnknownSlicedValue")
    {
        ClassInfoPtr info = _info;
        while (info && info->id != Ice::Value::ice_staticId())
        {
            assert(info->base); // All classes have the Ice::Value base type.
            const bool lastSlice = info->base->id == Ice::Value::ice_staticId();
            os->startSlice(info->id, info->compactId, lastSlice);

            writeMembers(os, info->members);
            writeMembers(os, info->optionalMembers); // The optional members have already been sorted by tag.

            os->endSlice();

            info = info->base;
        }
    }
    os->endValue();
}

void
IcePHP::ValueWriter::_iceRead(Ice::InputStream*)
{
    assert(false);
}

void
IcePHP::ValueWriter::writeMembers(Ice::OutputStream* os, const DataMemberList& members) const
{
    for (const auto& member : members)
    {
        zval* val = zend_hash_str_find(
            Z_OBJPROP_P(const_cast<zval*>(&_object)),
            member->name.c_str(),
            static_cast<int>(member->name.size()));

        if (!val)
        {
            ostringstream os;
            os << "member '" << member->name << "' of " << _info->id << " is not defined";
            runtimeError(os.str());
            throw AbortMarshaling();
        }

        assert(Z_TYPE_P(val) == IS_INDIRECT);
        val = Z_INDIRECT_P(val);

        if (Z_TYPE_P(val) == IS_REFERENCE)
        {
            val = Z_REFVAL_P(val);
        }

        if (member->optional && (isUnset(val) || !os->writeOptional(member->tag, member->type->optionalFormat())))
        {
            continue;
        }

        if (!member->type->validate(val, false))
        {
            ostringstream os;
            os << "invalid value for " << _info->id << " member '" << member->name << "'";
            invalidArgument(os.str());
            throw AbortMarshaling();
        }

        member->type->marshal(val, os, _map, member->optional);
    }
}

// ValueReader implementation.
IcePHP::ValueReader::ValueReader(zval* object, const ClassInfoPtr& info, const CommunicatorInfoPtr& comm)
    : _info(info),
      _communicator(comm)
{
    assert(Z_TYPE_P(object) == IS_OBJECT);
    ZVAL_COPY(&_object, object);
}

IcePHP::ValueReader::~ValueReader() { zval_ptr_dtor(&_object); }

void
IcePHP::ValueReader::ice_postUnmarshal()
{
    string name = "ice_postunmarshal"; // Must be lowercase.
    if (zend_hash_str_exists(&Z_OBJCE(_object)->function_table, name.c_str(), static_cast<int>(name.size())))
    {
        if (!invokeMethod(&_object, name))
        {
            throw AbortMarshaling();
        }
    }
}

void
IcePHP::ValueReader::_iceWrite(Ice::OutputStream*) const
{
    assert(false);
}

void
IcePHP::ValueReader::_iceRead(Ice::InputStream* is)
{
    is->startValue();
    const bool unknown = _info->id == "::Ice::UnknownSlicedValue";

    // Unmarshal the slices of a user-defined class.
    if (!unknown)
    {
        ClassInfoPtr info = _info;

        while (info && info->id != Ice::Value::ice_staticId())
        {
            is->startSlice();

            for (const auto& member : info->members)
            {
                member->type->unmarshal(is, member, _communicator, &_object, 0, false);
            }

            // The optional members have already been sorted by tag.
            for (const auto& member : info->optionalMembers)
            {
                if (is->readOptional(member->tag, member->type->optionalFormat()))
                {
                    member->type->unmarshal(is, member, _communicator, &_object, 0, true);
                }
                else
                {
                    zval zv;
                    AutoDestroy destroy(&zv);
                    assignUnset(&zv);
                    member->setMember(&_object, &zv);
                }
            }

            is->endSlice();

            info = info->base;
        }
    }

    _slicedData = is->endValue();

    if (_slicedData)
    {
        StreamUtil* util = reinterpret_cast<StreamUtil*>(is->getClosure());
        assert(util);
        util->add(shared_ptr<ValueReader>(shared_from_this()));

        // Define the "unknownTypeId" member for an instance of UnknownSlicedObject.
        if (unknown)
        {
            assert(!_slicedData->slices.empty());

            const string typeId = _slicedData->slices[0]->typeId;
            zval zv;
            AutoDestroy typeIdDestroyer(&zv);
            ZVAL_STRINGL(&zv, typeId.c_str(), static_cast<int>(typeId.size()));
            add_property_zval(&_object, "unknownTypeId", &zv);
        }
    }
}

ClassInfoPtr
IcePHP::ValueReader::getInfo() const
{
    return _info;
}

zval*
IcePHP::ValueReader::getObject() const
{
    return const_cast<zval*>(&_object);
}

Ice::SlicedDataPtr
IcePHP::ValueReader::getSlicedData() const
{
    return _slicedData;
}

// ReadObjectCallback implementation.
IcePHP::ReadObjectCallback::ReadObjectCallback(
    const ClassInfoPtr& info,
    const UnmarshalCallbackPtr& cb,
    zval* target,
    void* closure)
    : _info(info),
      _cb(cb),
      _closure(closure)
{
    ZVAL_NULL(&_target);

    if (target)
    {
        assert(Z_REFCOUNTED_P(target));
        ZVAL_COPY(&_target, target);
    }
}

IcePHP::ReadObjectCallback::~ReadObjectCallback() { zval_ptr_dtor(&_target); }

void
IcePHP::ReadObjectCallback::invoke(const shared_ptr<Ice::Value>& p)
{
#ifdef HT_ALLOW_COW_VIOLATION
    if (!ZVAL_IS_NULL(&_target))
    {
        HT_ALLOW_COW_VIOLATION(Z_ARRVAL(_target));
    }
#endif
    if (p)
    {
        auto reader = dynamic_pointer_cast<ValueReader>(p);
        assert(reader);

        // Verify that the unmarshaled object is compatible with the formal type.
        if (!reader->getInfo()->isA(_info->id))
        {
            throw MarshalException{
                __FILE__,
                __LINE__,
                "failed to unmarshal class with type ID '" + _info->id +
                    "': the Slice loader returned a class with type ID '" + reader->getInfo()->id + "'"};
        }
        zval* obj = reader->getObject();
        _cb->unmarshaled(obj, &_target, _closure);
    }
    else
    {
        zval zv;
        AutoDestroy destroy(&zv);
        ZVAL_NULL(&zv);
        _cb->unmarshaled(&zv, &_target, _closure);
    }
}

// ExceptionInfo implementation.
void
IcePHP::ExceptionInfo::unmarshal(Ice::InputStream* is, const CommunicatorInfoPtr& comm, zval* zv)
{
    if (object_init_ex(zv, zce) != SUCCESS)
    {
        ostringstream os;
        os << "unable to initialize object of type " << zce->name->val;
        runtimeError(os.str());
        throw AbortMarshaling();
    }

    // The type id for the first slice has already been read.
    auto info = shared_from_this();
    do
    {
        is->startSlice();

        for (const auto& member : info->members)
        {
            member->type->unmarshal(is, member, comm, zv, 0, false);
        }

        // The optional members have already been sorted by tag.
        for (const auto& member : info->optionalMembers)
        {
            if (is->readOptional(member->tag, member->type->optionalFormat()))
            {
                member->type->unmarshal(is, member, comm, zv, 0, true);
            }
            else
            {
                zval un;
                AutoDestroy destroy(&un);
                assignUnset(&un);
                member->setMember(zv, &un);
            }
        }

        is->endSlice();

        info = info->base;
    } while (info);
}

void
IcePHP::ExceptionInfo::print(zval* zv, IceInternal::Output& out)
{
    out << "exception " << id;
    out.sb();

    if (Z_TYPE_P(zv) != IS_OBJECT)
    {
        string s = zendTypeToString(Z_TYPE_P(zv));
        out << nl << "expected exception value of type " << zce->name->val << " but received " << s;
        out.eb();
        return;
    }

    // Compare class entries.
    zend_class_entry* ce = Z_OBJCE_P(zv);
    if (ce != zce)
    {
        out << nl << "expected exception value of type " << zce->name->val << " but received " << ce->name->val;
        out.eb();
        return;
    }

    PrintObjectHistory history;
    history.index = 0;

    printMembers(zv, out, &history);
    out.eb();
}

void
IcePHP::ExceptionInfo::printMembers(zval* zv, IceInternal::Output& out, PrintObjectHistory* history)
{
    if (base)
    {
        base->printMembers(zv, out, history);
    }

    for (const auto& member : members)
    {
        out << nl << member->name << " = ";
        zval* val = zend_hash_str_find(Z_OBJPROP_P(zv), member->name.c_str(), static_cast<int>(member->name.size()));
        assert(Z_TYPE_P(val) == IS_INDIRECT);
        val = Z_INDIRECT_P(val);

        if (val)
        {
            member->type->print(val, out, history);
        }
        else
        {
            out << "<not defined>";
        }
    }

    for (const auto& member : optionalMembers)
    {
        out << nl << member->name << " = ";
        zval* val = zend_hash_str_find(Z_OBJPROP_P(zv), member->name.c_str(), static_cast<int>(member->name.size()));

        assert(Z_TYPE_P(val) == IS_INDIRECT);
        val = Z_INDIRECT_P(val);

        if (val)
        {
            if (isUnset(val))
            {
                out << "<unset>";
            }
            else
            {
                member->type->print(val, out, history);
            }
        }
        else
        {
            out << "<not defined>";
        }
    }
}

bool
IcePHP::ExceptionInfo::isA(string_view typeId) const
{
    if (id == typeId)
    {
        return true;
    }

    if (base && base->isA(typeId))
    {
        return true;
    }

    return false;
}

// ExceptionReader implementation.
IcePHP::ExceptionReader::ExceptionReader(const CommunicatorInfoPtr& communicatorInfo, const ExceptionInfoPtr& info)
    : _communicatorInfo(communicatorInfo),
      _info(info)
{
    ZVAL_UNDEF(&_ex);
}

IcePHP::ExceptionReader::~ExceptionReader()
{
#ifdef NDEBUG
    // BUGFIX: releasing this object trigers an assert in PHP objects_store
    // https://github.com/php/php-src/issues/10593
    if (!Z_ISUNDEF(_ex))
    {
        zval_ptr_dtor(&_ex);
    }
#endif
}

const char*
IcePHP::ExceptionReader::ice_id() const noexcept
{
    return _info->id.c_str();
}

void
IcePHP::ExceptionReader::ice_throw() const
{
    assert(false);
}

void
IcePHP::ExceptionReader::_write(Ice::OutputStream*) const
{
    assert(false);
}

void
IcePHP::ExceptionReader::_read(Ice::InputStream* is)
{
    is->startException();

    _info->unmarshal(is, _communicatorInfo, const_cast<zval*>(&_ex));

    is->endException();
}

bool
IcePHP::ExceptionReader::_usesClasses() const
{
    return _info->usesClasses;
}

ExceptionInfoPtr
IcePHP::ExceptionReader::getInfo() const
{
    return _info;
}

zval*
IcePHP::ExceptionReader::getException() const
{
    return const_cast<zval*>(&_ex);
}

static zend_object*
handleTypeInfoAlloc(zend_class_entry* ce)
{
    Wrapper<TypeInfoPtr>* obj = Wrapper<TypeInfoPtr>::create(ce);
    assert(obj);
    obj->zobj.handlers = &_typeInfoHandlers;
    return &obj->zobj;
}

static void
handleTypeInfoFreeStorage(zend_object* object)
{
    Wrapper<TypeInfoPtr>* obj = Wrapper<TypeInfoPtr>::fetch(object);
    delete obj->ptr;
    zend_object_std_dtor(object);
}

static bool
createTypeInfo(zval* zv, shared_ptr<TypeInfo> p)
{
    assert(typeInfoClassEntry);
    if (object_init_ex(zv, typeInfoClassEntry) != SUCCESS)
    {
        runtimeError("unable to initialize type");
        return false;
    }

    Wrapper<shared_ptr<TypeInfo>>* ze = Wrapper<shared_ptr<TypeInfo>>::extract(zv);
    assert(!ze->ptr);
    ze->ptr = new shared_ptr<TypeInfo>(std::move(p));

    return true;
}

ZEND_FUNCTION(IcePHP_defineEnum)
{
    char* id;
    size_t idLen;
    zval* enumerators;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("sa"), &id, &idLen, &enumerators) == FAILURE)
    {
        return;
    }

    if (!createTypeInfo(return_value, make_shared<EnumInfo>(id, enumerators)))
    {
        RETURN_NULL();
    }
}

ZEND_FUNCTION(IcePHP_defineStruct)
{
    char* id;
    size_t idLen;
    char* name;
    size_t nameLen;
    zval* members;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("ssa"), &id, &idLen, &name, &nameLen, &members) ==
        FAILURE)
    {
        return;
    }

    if (!createTypeInfo(return_value, make_shared<StructInfo>(id, name, members)))
    {
        RETURN_NULL();
    }
}

ZEND_FUNCTION(IcePHP_defineSequence)
{
    char* id;
    size_t idLen;
    zval* element;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("so"), &id, &idLen, &element) == FAILURE)
    {
        assert(false);
        return;
    }

    if (!createTypeInfo(return_value, make_shared<SequenceInfo>(id, element)))
    {
        RETURN_NULL();
    }
}

ZEND_FUNCTION(IcePHP_defineDictionary)
{
    char* id;
    size_t idLen;
    zval* key;
    zval* value;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("soo"), &id, &idLen, &key, &value) == FAILURE)
    {
        return;
    }

    if (!createTypeInfo(return_value, make_shared<DictionaryInfo>(id, key, value)))
    {
        RETURN_NULL();
    }
}

ZEND_FUNCTION(IcePHP_declareProxy)
{
    char* id;
    size_t idLen;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("s"), &id, &idLen) == FAILURE)
    {
        return;
    }

    ProxyInfoPtr type = getProxyInfo(id);
    if (!type)
    {
        type = make_shared<ProxyInfo>(id);
        addProxyInfo(type);
    }

    if (!createTypeInfo(return_value, std::move(type)))
    {
        RETURN_NULL();
    }
}

ZEND_FUNCTION(IcePHP_defineProxy)
{
    char* id;
    size_t idLen;
    zval* base;
    zval* interfaces;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("so!a!"), &id, &idLen, &base, &interfaces) == FAILURE)
    {
        return;
    }

    ProxyInfoPtr type = getProxyInfo(id);
    if (!type)
    {
        type = make_shared<ProxyInfo>(id);
        addProxyInfo(type);
    }
    type->define(base, interfaces);

    if (!createTypeInfo(return_value, std::move(type)))
    {
        RETURN_NULL();
    }
}

ZEND_FUNCTION(IcePHP_declareClass)
{
    char* id;
    size_t idLen;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("s"), &id, &idLen) == FAILURE)
    {
        return;
    }

    ClassInfoPtr type = getClassInfoById(id);
    if (!type)
    {
        type = make_shared<ClassInfo>(id);
        addClassInfoById(type);
    }

    if (!createTypeInfo(return_value, std::move(type)))
    {
        RETURN_NULL();
    }
}

ZEND_FUNCTION(IcePHP_defineClass)
{
    char* id;
    size_t idLen;
    char* name;
    size_t nameLen;
    zend_long compactId;
    zval* base;
    zval* members;

    if (zend_parse_parameters(
            ZEND_NUM_ARGS(),
            const_cast<char*>("sslo!a!"),
            &id,
            &idLen,
            &name,
            &nameLen,
            &compactId,
            &base,
            &members) == FAILURE)
    {
        return;
    }

    ClassInfoPtr type = getClassInfoById(id);
    if (!type)
    {
        type = make_shared<ClassInfo>(id);
        addClassInfoById(type);
    }

    type->define(name, static_cast<int32_t>(compactId), base, members);

    addClassInfoByName(type);
    if (type->compactId != -1)
    {
        addClassInfoByCompactId(type);
    }

    if (!createTypeInfo(return_value, std::move(type)))
    {
        RETURN_NULL();
    }
}

static zend_object*
handleExceptionInfoAlloc(zend_class_entry* ce)
{
    Wrapper<ExceptionInfoPtr>* obj = Wrapper<ExceptionInfoPtr>::create(ce);
    assert(obj);

    obj->zobj.handlers = &_exceptionInfoHandlers;

    return &obj->zobj;
}

static void
handleExceptionInfoFreeStorage(zend_object* object)
{
    Wrapper<ExceptionInfoPtr>* obj = Wrapper<ExceptionInfoPtr>::fetch(object);
    delete obj->ptr;
    zend_object_std_dtor(object);
}

static bool
createExceptionInfo(zval* zv, const ExceptionInfoPtr& p)
{
    if (object_init_ex(zv, exceptionInfoClassEntry) != SUCCESS)
    {
        runtimeError("unable to initialize exception info");
        return false;
    }

    Wrapper<ExceptionInfoPtr>* ze = Wrapper<ExceptionInfoPtr>::extract(zv);
    assert(!ze->ptr);
    ze->ptr = new ExceptionInfoPtr(p);

    return true;
}

ZEND_FUNCTION(IcePHP_defineException)
{
    char* id;
    size_t idLen;
    char* name;
    size_t nameLen;
    zval* base;
    zval* members;

    if (zend_parse_parameters(
            ZEND_NUM_ARGS(),
            const_cast<char*>("sso!a!"),
            &id,
            &idLen,
            &name,
            &nameLen,
            &base,
            &members) == FAILURE)
    {
        return;
    }

    auto ex = make_shared<ExceptionInfo>();
    ex->id = id;
    ex->name = name;
    if (base)
    {
        ex->base = Wrapper<ExceptionInfoPtr>::value(base);
    }
    if (members)
    {
        convertDataMembers(members, ex->members, ex->optionalMembers, true);
    }

    ex->usesClasses = false;

    // Only examine the required members to see if any use classes.
    for (const auto& dm : ex->members)
    {
        if (!ex->usesClasses)
        {
            ex->usesClasses = dm->type->usesClasses();
        }
    }

    ex->zce = nameToClass(ex->name);

    assert(!getExceptionInfo(ex->id));

    ExceptionInfoMap* m;
    if (ICE_G(exceptionInfoMap))
    {
        m = reinterpret_cast<ExceptionInfoMap*>(ICE_G(exceptionInfoMap));
    }
    else
    {
        m = new ExceptionInfoMap;
        ICE_G(exceptionInfoMap) = m;
    }
    m->insert(ExceptionInfoMap::value_type(ex->id, ex));

    if (!createExceptionInfo(return_value, ex))
    {
        RETURN_NULL();
    }
}

ZEND_FUNCTION(IcePHP_stringify)
{
    if (ZEND_NUM_ARGS() != 2)
    {
        WRONG_PARAM_COUNT;
    }

    zval* v;
    zval* t;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("zz"), &v, &t) == FAILURE)
    {
        return;
    }

    TypeInfoPtr type = Wrapper<TypeInfoPtr>::value(t);
    assert(type);

    ostringstream ostr;
    IceInternal::Output out(ostr);
    PrintObjectHistory history;
    history.index = 0;
    type->print(v, out, &history);

    string str = ostr.str();
    RETURN_STRINGL(str.c_str(), static_cast<int>(str.length()));
}

ZEND_FUNCTION(IcePHP_stringifyException)
{
    if (ZEND_NUM_ARGS() != 2)
    {
        WRONG_PARAM_COUNT;
    }

    zval* v;
    zval* t;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("oo"), &v, &t) == FAILURE)
    {
        return;
    }

    ExceptionInfoPtr ex = Wrapper<ExceptionInfoPtr>::value(t);
    assert(ex);

    ostringstream ostr;
    IceInternal::Output out(ostr);
    ex->print(v, out);

    string str = ostr.str();
    RETURN_STRINGL(str.c_str(), static_cast<int>(str.length()));
}

// Predefined methods for IcePHP_TypeInfo.
static zend_function_entry _typeInfoMethods[] = {{0, 0, 0}};

// Predefined methods for IcePHP_ExceptionInfo.
static zend_function_entry _exceptionInfoMethods[] = {{0, 0, 0}};

bool
IcePHP::isUnset(zval* zv)
{
    if (Z_TYPE_P(zv) == IS_STRING)
    {
        return _unsetGUID == string(Z_STRVAL_P(zv), Z_STRLEN_P(zv));
    }
    return false;
}

void
IcePHP::assignUnset(zval* zv)
{
    ZVAL_COPY(zv, ICE_G(unset));
}

bool
IcePHP::typesInit(INIT_FUNC_ARGS)
{
    zend_class_entry ce;

    // Register the IcePHP_TypeInfo class.
    INIT_CLASS_ENTRY(ce, "IcePHP_TypeInfo", _typeInfoMethods);
    ce.create_object = handleTypeInfoAlloc;
    typeInfoClassEntry = zend_register_internal_class(&ce);
    memcpy(&_typeInfoHandlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    _typeInfoHandlers.free_obj = handleTypeInfoFreeStorage;
    _typeInfoHandlers.offset = XtOffsetOf(Wrapper<TypeInfoPtr>, zobj);

    // Register the IcePHP_ExceptionInfo class.
    INIT_CLASS_ENTRY(ce, "IcePHP_ExceptionInfo", _exceptionInfoMethods);
    ce.create_object = handleExceptionInfoAlloc;
    exceptionInfoClassEntry = zend_register_internal_class(&ce);
    memcpy(&_exceptionInfoHandlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    _exceptionInfoHandlers.free_obj = handleExceptionInfoFreeStorage;
    _exceptionInfoHandlers.offset = XtOffsetOf(Wrapper<ExceptionInfoPtr>, zobj);

    REGISTER_NS_STRING_CONSTANT("Ice", "None", const_cast<char*>(_unsetGUID.c_str()), CONST_CS | CONST_PERSISTENT);
    return true;
}

bool
IcePHP::typesRequestInit(void)
{
    // Create the global variables for the primitive types.
    for (int i = static_cast<int>(PrimitiveInfo::KindBool); i <= static_cast<int>(PrimitiveInfo::KindString); ++i)
    {
        PrimitiveInfoPtr type = make_shared<PrimitiveInfo>();
        type->kind = static_cast<PrimitiveInfo::Kind>(i);
        string name = "IcePHP__t_" + type->getId();

        zval zv;
        if (!createTypeInfo(&zv, std::move(type)))
        {
            zval_ptr_dtor(&zv);
            return false;
        }
        zend_hash_str_update(&EG(symbol_table), const_cast<char*>(name.c_str()), name.size(), &zv);
    }

    ICE_G(idToClassInfoMap) = 0;
    ICE_G(nameToClassInfoMap) = 0;
    ICE_G(proxyInfoMap) = 0;
    ICE_G(exceptionInfoMap) = 0;

    zval* unset = static_cast<zval*>(ecalloc(1, sizeof(zval)));
    ZVAL_STRINGL(unset, _unsetGUID.c_str(), static_cast<int>(_unsetGUID.length()));
    ICE_G(unset) = unset;

    return true;
}

bool
IcePHP::typesRequestShutdown(void)
{
    if (ICE_G(proxyInfoMap))
    {
        ProxyInfoMap* m = static_cast<ProxyInfoMap*>(ICE_G(proxyInfoMap));
        for (ProxyInfoMap::iterator p = m->begin(); p != m->end(); ++p)
        {
            p->second->destroy();
        }
        delete m;
    }

    if (ICE_G(idToClassInfoMap))
    {
        ClassInfoMap* m = static_cast<ClassInfoMap*>(ICE_G(idToClassInfoMap));
        for (ClassInfoMap::iterator p = m->begin(); p != m->end(); ++p)
        {
            p->second->destroy();
        }
        delete m;
    }

    if (ICE_G(nameToClassInfoMap))
    {
        ClassInfoMap* m = static_cast<ClassInfoMap*>(ICE_G(nameToClassInfoMap));
        delete m;
    }

    delete static_cast<ExceptionInfoMap*>(ICE_G(exceptionInfoMap));

    zval_dtor(ICE_G(unset));
    efree(ICE_G(unset));
    return true;
}
