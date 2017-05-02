// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
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
#include <Slice/PHPUtil.h>
#include <Ice/SlicedData.h>

#include <limits>

using namespace std;
using namespace IcePHP;
using namespace IceUtil;
using namespace IceUtilInternal;

ZEND_EXTERN_MODULE_GLOBALS(ice)

//
// Class entries represent the PHP class implementations we have registered.
//
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
static zend_object_value handleTypeInfoAlloc(zend_class_entry* TSRMLS_DC);
static void handleTypeInfoFreeStorage(void* TSRMLS_DC);

static zend_object_value handleExceptionInfoAlloc(zend_class_entry* TSRMLS_DC);
static void handleExceptionInfoFreeStorage(void* TSRMLS_DC);
}

typedef map<string, ProxyInfoPtr> ProxyInfoMap;
typedef map<string, ClassInfoPtr> ClassInfoMap;
typedef map<Ice::Int, ClassInfoPtr> CompactIdMap;
typedef map<string, ExceptionInfoPtr> ExceptionInfoMap;

//
// addProxyInfo()
//
static void
addProxyInfo(const ProxyInfoPtr& p TSRMLS_DC)
{
    ProxyInfoMap* m;
    if(ICE_G(proxyInfoMap))
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

//
// getProxyInfo()
//
IcePHP::ProxyInfoPtr
IcePHP::getProxyInfo(const string& id TSRMLS_DC)
{
    if(ICE_G(proxyInfoMap))
    {
        ProxyInfoMap* m = reinterpret_cast<ProxyInfoMap*>(ICE_G(proxyInfoMap));
        ProxyInfoMap::iterator p = m->find(id);
        if(p != m->end())
        {
            return p->second;
        }
    }
    return 0;
}

//
// addClassInfoById()
//
static void
addClassInfoById(const ClassInfoPtr& p TSRMLS_DC)
{
    assert(!getClassInfoById(p->id TSRMLS_CC));

    ClassInfoMap* m = reinterpret_cast<ClassInfoMap*>(ICE_G(idToClassInfoMap));
    if(!m)
    {
        m = new ClassInfoMap;
        ICE_G(idToClassInfoMap) = m;
    }
    m->insert(ClassInfoMap::value_type(p->id, p));
}

//
// addClassInfoByName()
//
static void
addClassInfoByName(const ClassInfoPtr& p TSRMLS_DC)
{
    assert(!getClassInfoByName(p->name TSRMLS_CC));
#ifdef ICEPHP_USE_NAMESPACES
    assert(name[0] == '\\');
#endif

    ClassInfoMap* m = reinterpret_cast<ClassInfoMap*>(ICE_G(nameToClassInfoMap));
    if(!m)
    {
        m = new ClassInfoMap;
        ICE_G(nameToClassInfoMap) = m;
    }
    m->insert(ClassInfoMap::value_type(p->name, p));
}

static ClassInfoPtr
getClassInfoByClass(zend_class_entry* cls, zend_class_entry* formal TSRMLS_DC)
{
    //
    // See if there's a match in our class name => ClassInfo map.
    //
    ClassInfoPtr info = getClassInfoByName(cls->name TSRMLS_CC);

    //
    // Check the base class, assuming it's compatible with our formal type (if any).
    //
    if(!info && cls->parent && (!formal || checkClass(cls->parent, formal)))
    {
        info = getClassInfoByClass(cls->parent, formal TSRMLS_CC);
    }

    //
    // Check interfaces.
    //
    if(!info)
    {
        for(zend_uint i = 0; i < cls->num_interfaces && !info; ++i)
        {
            if(!formal || checkClass(cls->interfaces[i], formal))
            {
                info = getClassInfoByClass(cls->interfaces[i], formal TSRMLS_CC);
            }
        }
    }

    return info;
}

//
// getClassInfoById()
//
IcePHP::ClassInfoPtr
IcePHP::getClassInfoById(const string& id TSRMLS_DC)
{
    if(ICE_G(idToClassInfoMap))
    {
        ClassInfoMap* m = reinterpret_cast<ClassInfoMap*>(ICE_G(idToClassInfoMap));
        ClassInfoMap::iterator p = m->find(id);
        if(p != m->end())
        {
            return p->second;
        }
    }
    return 0;
}

//
// getClassInfoByName()
//
IcePHP::ClassInfoPtr
IcePHP::getClassInfoByName(const string& name TSRMLS_DC)
{
    if(ICE_G(nameToClassInfoMap))
    {
        string s = name;

#ifdef ICEPHP_USE_NAMESPACES
        //
        // PHP's class definition (zend_class_entry) does not use a leading backslash
        // in the class name.
        //
        if(s[0] != '\\')
        {
            s.insert(0, "\\");
        }
#endif

        ClassInfoMap* m = reinterpret_cast<ClassInfoMap*>(ICE_G(nameToClassInfoMap));
        ClassInfoMap::iterator p = m->find(s);
        if(p != m->end())
        {
            return p->second;
        }
    }
    return 0;
}

//
// getExceptionInfo()
//
IcePHP::ExceptionInfoPtr
IcePHP::getExceptionInfo(const string& id TSRMLS_DC)
{
    if(ICE_G(exceptionInfoMap))
    {
        ExceptionInfoMap* m = reinterpret_cast<ExceptionInfoMap*>(ICE_G(exceptionInfoMap));
        ExceptionInfoMap::iterator p = m->find(id);
        if(p != m->end())
        {
            return p->second;
        }
    }
    return 0;
}

//
// StreamUtil implementation
//
zend_class_entry* IcePHP::StreamUtil::_slicedDataType = 0;
zend_class_entry* IcePHP::StreamUtil::_sliceInfoType = 0;

IcePHP::StreamUtil::~StreamUtil()
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
IcePHP::StreamUtil::add(const ReadObjectCallbackPtr& callback)
{
    _callbacks.push_back(callback);
}

void
IcePHP::StreamUtil::add(const ObjectReaderPtr& reader)
{
    assert(reader->getSlicedData());
    _readers.insert(reader);
}

void
IcePHP::StreamUtil::updateSlicedData(TSRMLS_D)
{
    for(set<ObjectReaderPtr>::iterator p = _readers.begin(); p != _readers.end(); ++p)
    {
        setSlicedDataMember((*p)->getObject(), (*p)->getSlicedData() TSRMLS_CC);
    }
}

void
IcePHP::StreamUtil::setSlicedDataMember(zval* obj, const Ice::SlicedDataPtr& slicedData TSRMLS_DC)
{
    //
    // Create a PHP equivalent of the SlicedData object.
    //

    assert(slicedData);

    if(!_slicedDataType)
    {
        _slicedDataType = idToClass("::Ice::SlicedData" TSRMLS_CC);
        assert(_slicedDataType);
    }
    if(!_sliceInfoType)
    {
        _sliceInfoType = idToClass("::Ice::SliceInfo" TSRMLS_CC);
        assert(_sliceInfoType);
    }

    zval* sd;
    MAKE_STD_ZVAL(sd);
    AutoDestroy sdDestroyer(sd);

    if(object_init_ex(sd, _slicedDataType) != SUCCESS)
    {
        throw AbortMarshaling();
    }

    zval* slices;
    MAKE_STD_ZVAL(slices);
    array_init(slices);
    AutoDestroy slicesDestroyer(slices);

    if(add_property_zval(sd, STRCAST("slices"), slices) != SUCCESS)
    {
        throw AbortMarshaling();
    }

    //
    // Translate each SliceInfo object into its PHP equivalent.
    //
    for(vector<Ice::SliceInfoPtr>::const_iterator p = slicedData->slices.begin(); p != slicedData->slices.end(); ++p)
    {
        zval* slice;
        MAKE_STD_ZVAL(slice);
        AutoDestroy sliceDestroyer(slice);

        if(object_init_ex(slice, _sliceInfoType) != SUCCESS)
        {
            throw AbortMarshaling();
        }

        add_next_index_zval(slices, slice); // Steals a reference.
        Z_ADDREF_P(slice);

        //
        // typeId
        //
        zval* typeId;
        MAKE_STD_ZVAL(typeId);
        AutoDestroy typeIdDestroyer(typeId);
        ZVAL_STRINGL(typeId, STRCAST((*p)->typeId.c_str()), static_cast<int>((*p)->typeId.size()), 1);
        if(add_property_zval(slice, STRCAST("typeId"), typeId) != SUCCESS)
        {
            throw AbortMarshaling();
        }

        //
        // compactId
        //
        zval* compactId;
        MAKE_STD_ZVAL(compactId);
        AutoDestroy compactIdDestroyer(compactId);
        ZVAL_LONG(compactId, (*p)->compactId);
        if(add_property_zval(slice, STRCAST("compactId"), compactId) != SUCCESS)
        {
            throw AbortMarshaling();
        }

        //
        // bytes
        //
        zval* bytes;
        MAKE_STD_ZVAL(bytes);
        array_init(bytes);
        AutoDestroy bytesDestroyer(bytes);
        for(vector<Ice::Byte>::const_iterator q = (*p)->bytes.begin(); q != (*p)->bytes.end(); ++q)
        {
            add_next_index_long(bytes, *q & 0xff);
        }
        if(add_property_zval(slice, STRCAST("bytes"), bytes) != SUCCESS)
        {
            throw AbortMarshaling();
        }

        //
        // instances
        //
        zval* instances;
        MAKE_STD_ZVAL(instances);
        array_init(instances);
        AutoDestroy instancesDestroyer(instances);
        if(add_property_zval(slice, STRCAST("instances"), instances) != SUCCESS)
        {
            throw AbortMarshaling();
        }

        for(vector<Ice::ObjectPtr>::const_iterator q = (*p)->instances.begin(); q != (*p)->instances.end(); ++q)
        {
            //
            // Each element in the instances list is an instance of ObjectReader that wraps a PHP object.
            //
            assert(*q);
            ObjectReaderPtr r = ObjectReaderPtr::dynamicCast(*q);
            assert(r);
            zval* o = r->getObject();
            assert(Z_TYPE_P(o) == IS_OBJECT); // Should be non-nil.
            add_next_index_zval(instances, o); // Steals a reference.
            Z_ADDREF_P(o);
        }

        //
        // hasOptionalMembers
        //
        zval* hasOptionalMembers;
        MAKE_STD_ZVAL(hasOptionalMembers);
        AutoDestroy hasOptionalMembersDestroyer(hasOptionalMembers);
        ZVAL_BOOL(hasOptionalMembers, (*p)->hasOptionalMembers ? 1 : 0);
        if(add_property_zval(slice, STRCAST("hasOptionalMembers"), hasOptionalMembers) != SUCCESS)
        {
            throw AbortMarshaling();
        }

        //
        // isLastSlice
        //
        zval* isLastSlice;
        MAKE_STD_ZVAL(isLastSlice);
        AutoDestroy isLastSliceDestroyer(isLastSlice);
        ZVAL_BOOL(isLastSlice, (*p)->isLastSlice ? 1 : 0);
        if(add_property_zval(slice, STRCAST("isLastSlice"), isLastSlice) != SUCCESS)
        {
            throw AbortMarshaling();
        }
    }

    if(add_property_zval(obj, STRCAST("_ice_slicedData"), sd) != SUCCESS)
    {
        throw AbortMarshaling();
    }
}

//
// Instances of preserved class and exception types may have a data member
// named _ice_slicedData which is an instance of the PHP class Ice_SlicedData.
//
Ice::SlicedDataPtr
IcePHP::StreamUtil::getSlicedDataMember(zval* obj, ObjectMap* objectMap TSRMLS_DC)
{
    Ice::SlicedDataPtr slicedData;

    string name = "_ice_slicedData";
    void* data;
    if(zend_hash_find(Z_OBJPROP_P(obj), STRCAST(name.c_str()), name.size() + 1, &data) == SUCCESS)
    {
        zval* sd = *(reinterpret_cast<zval**>(data));

        if(Z_TYPE_P(sd) != IS_NULL)
        {
#ifndef NDEBUG
            int status;
#endif

            //
            // The "slices" member is an array of Ice_SliceInfo objects.
            //
#ifndef NDEBUG
            status =
#endif
            zend_hash_find(Z_OBJPROP_P(sd), STRCAST("slices"), sizeof("slices"), &data);
            assert(status == SUCCESS);
            zval* sl = *(reinterpret_cast<zval**>(data));
            assert(Z_TYPE_P(sl) == IS_ARRAY);

            Ice::SliceInfoSeq slices;

            HashTable* arr = Z_ARRVAL_P(sl);
            assert(arr);
            HashPosition pos;
            zend_hash_internal_pointer_reset_ex(arr, &pos);

            while(zend_hash_get_current_data_ex(arr, &data, &pos) != FAILURE)
            {
                zval* s = *(reinterpret_cast<zval**>(data));
                assert(Z_OBJCE_P(s) == _sliceInfoType);

                Ice::SliceInfoPtr info = new Ice::SliceInfo;

#ifndef NDEBUG
                status =
#endif
                zend_hash_find(Z_OBJPROP_P(s), STRCAST("typeId"), sizeof("typeId"), &data);
                assert(status == SUCCESS);
                zval* typeId = *(reinterpret_cast<zval**>(data));
                assert(Z_TYPE_P(typeId) == IS_STRING);
                info->typeId = string(Z_STRVAL_P(typeId), Z_STRLEN_P(typeId));

#ifndef NDEBUG
                status =
#endif
                zend_hash_find(Z_OBJPROP_P(s), STRCAST("compactId"), sizeof("compactId"), &data);
                assert(status == SUCCESS);
                zval* compactId = *(reinterpret_cast<zval**>(data));
                assert(Z_TYPE_P(compactId) == IS_LONG);
                info->compactId = Z_LVAL_P(compactId);

#ifndef NDEBUG
                status =
#endif
                zend_hash_find(Z_OBJPROP_P(s), STRCAST("bytes"), sizeof("bytes"), &data);
                assert(status == SUCCESS);
                zval* bytes = *(reinterpret_cast<zval**>(data));
                assert(Z_TYPE_P(bytes) == IS_ARRAY);
                HashTable* barr = Z_ARRVAL_P(bytes);
                HashPosition bpos;
                zend_hash_internal_pointer_reset_ex(barr, &bpos);
                info->bytes.resize(zend_hash_num_elements(barr));

                vector<Ice::Byte>::size_type i = 0;
                while(zend_hash_get_current_data_ex(barr, &data, &bpos) != FAILURE)
                {
                    zval* e = *(reinterpret_cast<zval**>(data));
                    long l = Z_LVAL_P(e);
                    assert(l >= 0 && l <= 255);
                    info->bytes[i++] = static_cast<Ice::Byte>(l);
                    zend_hash_move_forward_ex(barr, &bpos);
                }

#ifndef NDEBUG
                status =
#endif
                zend_hash_find(Z_OBJPROP_P(s), STRCAST("instances"), sizeof("instances"), &data);
                assert(status == SUCCESS);
                zval* instances = *(reinterpret_cast<zval**>(data));
                assert(Z_TYPE_P(instances) == IS_ARRAY);
                HashTable* oarr = Z_ARRVAL_P(instances);
                HashPosition opos;
                zend_hash_internal_pointer_reset_ex(oarr, &opos);

                while(zend_hash_get_current_data_ex(oarr, &data, &opos) != FAILURE)
                {
                    zval* o = *(reinterpret_cast<zval**>(data));
                    assert(Z_TYPE_P(o) == IS_OBJECT);

                    Ice::ObjectPtr writer;

                    ObjectMap::iterator i = objectMap->find(Z_OBJ_HANDLE_P(o));
                    if(i == objectMap->end())
                    {
                        writer = new ObjectWriter(o, objectMap, 0 TSRMLS_CC);
                        objectMap->insert(ObjectMap::value_type(Z_OBJ_HANDLE_P(o), writer));
                    }
                    else
                    {
                        writer = i->second;
                    }

                    info->instances.push_back(writer);
                    zend_hash_move_forward_ex(oarr, &opos);
                }

#ifndef NDEBUG
                status =
#endif
                zend_hash_find(Z_OBJPROP_P(s), STRCAST("hasOptionalMembers"), sizeof("hasOptionalMembers"), &data);
                assert(status == SUCCESS);
                zval* hasOptionalMembers = *(reinterpret_cast<zval**>(data));
                assert(Z_TYPE_P(hasOptionalMembers) == IS_BOOL);
                info->hasOptionalMembers = Z_BVAL_P(hasOptionalMembers) ? true : false;

#ifndef NDEBUG
                status =
#endif
                zend_hash_find(Z_OBJPROP_P(s), STRCAST("isLastSlice"), sizeof("isLastSlice"), &data);
                assert(status == SUCCESS);
                zval* isLastSlice = *(reinterpret_cast<zval**>(data));
                assert(Z_TYPE_P(isLastSlice) == IS_BOOL);
                info->isLastSlice = Z_BVAL_P(isLastSlice) ? true : false;

                slices.push_back(info);
                zend_hash_move_forward_ex(arr, &pos);
            }

            slicedData = new Ice::SlicedData(slices);
        }
    }

    return slicedData;
}

//
// UnmarshalCallback implementation.
//
IcePHP::UnmarshalCallback::~UnmarshalCallback()
{
}

//
// TypeInfo implementation.
//
IcePHP::TypeInfo::TypeInfo()
{
}

bool
IcePHP::TypeInfo::usesClasses() const
{
    return false;
}

void
IcePHP::TypeInfo::unmarshaled(zval*, zval*, void* TSRMLS_DC)
{
    assert(false);
}

void
IcePHP::TypeInfo::destroy()
{
}

//
// PrimitiveInfo implementation.
//
string
IcePHP::PrimitiveInfo::getId() const
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
IcePHP::PrimitiveInfo::validate(zval* zv, bool throwException TSRMLS_DC)
{
    switch(kind)
    {
    case PrimitiveInfo::KindBool:
    {
        if(Z_TYPE_P(zv) != IS_BOOL)
        {
            if(throwException)
            {
                string s = zendTypeToString(Z_TYPE_P(zv));
                invalidArgument("expected boolean value but received %s" TSRMLS_CC, s.c_str());
            }
            return false;
        }
        break;
    }
    case PrimitiveInfo::KindByte:
    {
        if(Z_TYPE_P(zv) != IS_LONG)
        {
            string s = zendTypeToString(Z_TYPE_P(zv));
            if(throwException)
            {
                invalidArgument("expected byte value but received %s" TSRMLS_CC, s.c_str());
            }
            return false;
        }
        long val = Z_LVAL_P(zv);
        if(val < 0 || val > 255)
        {
            if(throwException)
            {
                invalidArgument("value %ld is out of range for a byte" TSRMLS_CC, val);
            }
            return false;
        }
        break;
    }
    case PrimitiveInfo::KindShort:
    {
        if(Z_TYPE_P(zv) != IS_LONG)
        {
            string s = zendTypeToString(Z_TYPE_P(zv));
            if(throwException)
            {
                invalidArgument("expected short value but received %s" TSRMLS_CC, s.c_str());
            }
            return false;
        }
        long val = Z_LVAL_P(zv);
        if(val < SHRT_MIN || val > SHRT_MAX)
        {
            if(throwException)
            {
                invalidArgument("value %ld is out of range for a short" TSRMLS_CC, val);
            }
            return false;
        }
        break;
    }
    case PrimitiveInfo::KindInt:
    {
        if(Z_TYPE_P(zv) != IS_LONG)
        {
            string s = zendTypeToString(Z_TYPE_P(zv));
            if(throwException)
            {
                invalidArgument("expected int value but received %s" TSRMLS_CC, s.c_str());
            }
            return false;
        }
        long val = Z_LVAL_P(zv);
        if(val < INT_MIN || val > INT_MAX)
        {
            if(throwException)
            {
                invalidArgument("value %ld is out of range for an int" TSRMLS_CC, val);
            }
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
        if(Z_TYPE_P(zv) != IS_LONG && Z_TYPE_P(zv) != IS_STRING)
        {
            string s = zendTypeToString(Z_TYPE_P(zv));
            if(throwException)
            {
                invalidArgument("expected long value but received %s" TSRMLS_CC, s.c_str());
            }
            return false;
        }
        Ice::Long val;
        if(Z_TYPE_P(zv) == IS_LONG)
        {
            val = Z_LVAL_P(zv);
        }
        else
        {
            string sval(Z_STRVAL_P(zv), Z_STRLEN_P(zv));
            if(!IceUtilInternal::stringToInt64(sval, val))
            {
                if(throwException)
                {
                    invalidArgument("invalid long value `%s'" TSRMLS_CC, Z_STRVAL_P(zv));
                }
                return false;
            }
        }
        break;
    }
    case PrimitiveInfo::KindFloat:
    {
        if(Z_TYPE_P(zv) != IS_DOUBLE && Z_TYPE_P(zv) != IS_LONG)
        {
            string s = zendTypeToString(Z_TYPE_P(zv));
            if(throwException)
            {
                invalidArgument("expected float value but received %s" TSRMLS_CC, s.c_str());
            }
            return false;
        }
        if(Z_TYPE_P(zv) == IS_DOUBLE)
        {
            double val = Z_DVAL_P(zv);
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
        if(Z_TYPE_P(zv) != IS_DOUBLE && Z_TYPE_P(zv) != IS_LONG)
        {
            string s = zendTypeToString(Z_TYPE_P(zv));
            if(throwException)
            {
                invalidArgument("expected double value but received %s" TSRMLS_CC, s.c_str());
            }
            return false;
        }
        break;
    }
    case PrimitiveInfo::KindString:
    {
        if(Z_TYPE_P(zv) != IS_STRING && Z_TYPE_P(zv) != IS_NULL)
        {
            string s = zendTypeToString(Z_TYPE_P(zv));
            if(throwException)
            {
                invalidArgument("expected string value but received %s" TSRMLS_CC, s.c_str());
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
IcePHP::PrimitiveInfo::optionalFormat() const
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
IcePHP::PrimitiveInfo::marshal(zval* zv, Ice::OutputStream* os, ObjectMap*, bool TSRMLS_DC)
{
    switch(kind)
    {
    case PrimitiveInfo::KindBool:
    {
        assert(Z_TYPE_P(zv) == IS_BOOL);
        os->write(Z_BVAL_P(zv) ? true : false);
        break;
    }
    case PrimitiveInfo::KindByte:
    {
        assert(Z_TYPE_P(zv) == IS_LONG);
        long val = Z_LVAL_P(zv);
        assert(val >= 0 && val <= 255); // validate() should have caught this.
        os->write(static_cast<Ice::Byte>(val));
        break;
    }
    case PrimitiveInfo::KindShort:
    {
        assert(Z_TYPE_P(zv) == IS_LONG);
        long val = Z_LVAL_P(zv);
        assert(val >= SHRT_MIN && val <= SHRT_MAX); // validate() should have caught this.
        os->write(static_cast<Ice::Short>(val));
        break;
    }
    case PrimitiveInfo::KindInt:
    {
        assert(Z_TYPE_P(zv) == IS_LONG);
        long val = Z_LVAL_P(zv);
        assert(val >= INT_MIN && val <= INT_MAX); // validate() should have caught this.
        os->write(static_cast<Ice::Int>(val));
        break;
    }
    case PrimitiveInfo::KindLong:
    {
        //
        // The platform's 'long' type may not be 64 bits, so we also accept
        // a string argument for this type.
        //
        assert(Z_TYPE_P(zv) == IS_LONG || Z_TYPE_P(zv) == IS_STRING); // validate() should have caught this.
        Ice::Long val;
        if(Z_TYPE_P(zv) == IS_LONG)
        {
            val = Z_LVAL_P(zv);
        }
        else
        {
            string sval(Z_STRVAL_P(zv), Z_STRLEN_P(zv));
            IceUtilInternal::stringToInt64(sval, val);
        }
        os->write(val);
        break;
    }
    case PrimitiveInfo::KindFloat:
    {
        Ice::Double val = 0;
        if(Z_TYPE_P(zv) == IS_DOUBLE)
        {
            val = Z_DVAL_P(zv);
        }
        else if(Z_TYPE_P(zv) == IS_LONG)
        {
            val = static_cast<double>(Z_LVAL_P(zv));
        }
        else
        {
            assert(false); // validate() should have caught this.
        }
        os->write(static_cast<Ice::Float>(val));
        break;
    }
    case PrimitiveInfo::KindDouble:
    {
        Ice::Double val = 0;
        if(Z_TYPE_P(zv) == IS_DOUBLE)
        {
            val = Z_DVAL_P(zv);
        }
        else if(Z_TYPE_P(zv) == IS_LONG)
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
        if(Z_TYPE_P(zv) == IS_STRING)
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
IcePHP::PrimitiveInfo::unmarshal(Ice::InputStream* is, const UnmarshalCallbackPtr& cb,
                                 const CommunicatorInfoPtr&, zval* target, void* closure, bool TSRMLS_DC)
{
    zval* zv;
    MAKE_STD_ZVAL(zv);
    AutoDestroy destroy(zv);

    switch(kind)
    {
    case PrimitiveInfo::KindBool:
    {
        bool val;
        is->read(val);
        ZVAL_BOOL(zv, val ? 1 : 0);
        break;
    }
    case PrimitiveInfo::KindByte:
    {
        Ice::Byte val;
        is->read(val);
        ZVAL_LONG(zv, val & 0xff);
        break;
    }
    case PrimitiveInfo::KindShort:
    {
        Ice::Short val;
        is->read(val);
        ZVAL_LONG(zv, val);
        break;
    }
    case PrimitiveInfo::KindInt:
    {
        Ice::Int val;
        is->read(val);
        ZVAL_LONG(zv, val);
        break;
    }
    case PrimitiveInfo::KindLong:
    {
        Ice::Long val;
        is->read(val);

        //
        // The platform's 'long' type may not be 64 bits, so we store 64-bit
        // values as a string.
        //
        if(sizeof(Ice::Long) > sizeof(long) && (val < LONG_MIN || val > LONG_MAX))
        {
            string str = IceUtilInternal::int64ToString(val);
            ZVAL_STRINGL(zv, STRCAST(str.c_str()), static_cast<int>(str.length()), 1);
        }
        else
        {
            ZVAL_LONG(zv, static_cast<long>(val));
        }
        break;
    }
    case PrimitiveInfo::KindFloat:
    {
        Ice::Float val;
        is->read(val);
        ZVAL_DOUBLE(zv, val);
        break;
    }
    case PrimitiveInfo::KindDouble:
    {
        Ice::Double val;
        is->read(val);
        ZVAL_DOUBLE(zv, val);
        break;
    }
    case PrimitiveInfo::KindString:
    {
        string val;
        is->read(val);
        ZVAL_STRINGL(zv, STRCAST(val.c_str()), static_cast<int>(val.length()), 1);
        break;
    }
    }

    cb->unmarshaled(zv, target, closure TSRMLS_CC);
}

void
IcePHP::PrimitiveInfo::print(zval* zv, IceUtilInternal::Output& out, PrintObjectHistory* TSRMLS_DC)
{
    if(!validate(zv, false TSRMLS_CC))
    {
        out << "<invalid value - expected " << getId() << ">";
        return;
    }
    zval tmp = *zv;
    zval_copy_ctor(&tmp);
    INIT_PZVAL(&tmp);
    convert_to_string(&tmp);
    out << Z_STRVAL(tmp);
    zval_dtor(&tmp);
}

//
// EnumInfo implementation.
//
IcePHP::EnumInfo::EnumInfo(const string& ident, zval* en TSRMLS_DC) :
    id(ident),
    maxValue(0)
{
    HashTable* arr = Z_ARRVAL_P(en);
    void* data;
    HashPosition pos;
    zend_hash_internal_pointer_reset_ex(arr, &pos);
    while(zend_hash_get_current_data_ex(arr, &data, &pos) != FAILURE)
    {
        zval** val;

        val = reinterpret_cast<zval**>(data);
        assert(Z_TYPE_PP(val) == IS_STRING);
        string name = Z_STRVAL_PP(val);
        zend_hash_move_forward_ex(arr, &pos);

        zend_hash_get_current_data_ex(arr, &data, &pos);
        val = reinterpret_cast<zval**>(data);
        assert(Z_TYPE_PP(val) == IS_LONG);
        Ice::Int value = static_cast<Ice::Int>(Z_LVAL_PP(val));
        zend_hash_move_forward_ex(arr, &pos);

        if(value > maxValue)
        {
            const_cast<int&>(maxValue) = value;
        }

        const_cast<map<Ice::Int, string>&>(enumerators)[value] = name;
    }
}

string
IcePHP::EnumInfo::getId() const
{
    return id;
}

bool
IcePHP::EnumInfo::validate(zval* zv, bool TSRMLS_DC)
{
    if(Z_TYPE_P(zv) == IS_LONG)
    {
        const Ice::Int l = static_cast<Ice::Int>(Z_LVAL_P(zv));
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
    return Ice::OptionalFormatSize;
}

void
IcePHP::EnumInfo::marshal(zval* zv, Ice::OutputStream* os, ObjectMap*, bool TSRMLS_DC)
{
    assert(Z_TYPE_P(zv) == IS_LONG); // validate() should have caught this.
    const Ice::Int val = static_cast<Ice::Int>(Z_LVAL_P(zv));
    assert(val >= 0 && enumerators.find(val) != enumerators.end()); // validate() should have caught this.

    os->writeEnum(val, maxValue);
}

void
IcePHP::EnumInfo::unmarshal(Ice::InputStream* is, const UnmarshalCallbackPtr& cb,
                            const CommunicatorInfoPtr&, zval* target, void* closure, bool TSRMLS_DC)
{
    zval* zv;
    ALLOC_INIT_ZVAL(zv);
    AutoDestroy destroy(zv);

    const Ice::Int val = is->readEnum(maxValue);

    if(enumerators.find(val) == enumerators.end())
    {
        invalidArgument("enumerator %d is out of range for enum %s" TSRMLS_CC, val, id.c_str());
        throw AbortMarshaling();
    }

    ZVAL_LONG(zv, val);
    cb->unmarshaled(zv, target, closure TSRMLS_CC);
}

void
IcePHP::EnumInfo::print(zval* zv, IceUtilInternal::Output& out, PrintObjectHistory* TSRMLS_DC)
{
    if(!validate(zv, false TSRMLS_CC))
    {
        out << "<invalid value - expected " << id << ">";
        return;
    }
    const Ice::Int val = static_cast<Ice::Int>(Z_LVAL_P(zv));
    map<Ice::Int, string>::const_iterator p = enumerators.find(val);
    assert(p != enumerators.end());
    out << p->second;
}

//
// DataMember implementation.
//
void
IcePHP::DataMember::unmarshaled(zval* zv, zval* target, void* TSRMLS_DC)
{
    setMember(target, zv TSRMLS_CC);
}

void
IcePHP::DataMember::setMember(zval* target, zval* zv TSRMLS_DC)
{
    assert(Z_TYPE_P(target) == IS_OBJECT);

    //
    // The add_property_zval function fails if the data member has protected visibility.
    // As a workaround, before calling the function we change the current scope to be that
    // of the object.
    //
    zend_class_entry *oldScope = EG(scope);
    EG(scope) = Z_OBJCE_P(target);

    //
    // add_property_zval increments the refcount of zv.
    //
    int status = add_property_zval(target, STRCAST(name.c_str()), zv);

    EG(scope) = oldScope; // Restore the previous scope.

    if(status == FAILURE)
    {
        runtimeError("unable to set member `%s'" TSRMLS_CC, name.c_str());
        throw AbortMarshaling();
    }
}

static void
convertDataMembers(zval* zv, DataMemberList& reqMembers, DataMemberList& optMembers, bool allowOptional TSRMLS_DC)
{
    list<DataMemberPtr> optList;

    assert(Z_TYPE_P(zv) == IS_ARRAY);
    HashTable* membersArray = Z_ARRVAL_P(zv);
    void* data;
    HashPosition pos;
    zend_hash_internal_pointer_reset_ex(membersArray, &pos);
    while(zend_hash_get_current_data_ex(membersArray, &data, &pos) != FAILURE)
    {
        zval** arr = reinterpret_cast<zval**>(data);

        DataMemberPtr m = new DataMember();
        zval** elem;

        assert(Z_TYPE_PP(arr) == IS_ARRAY);
        HashTable* member = Z_ARRVAL_PP(arr);
        assert(zend_hash_num_elements(member) == allowOptional ? 4 : 2);
        zend_hash_index_find(member, 0, reinterpret_cast<void**>(&elem));
        assert(Z_TYPE_PP(elem) == IS_STRING);
        m->name = Z_STRVAL_PP(elem);
        zend_hash_index_find(member, 1, reinterpret_cast<void**>(&elem));
        assert(Z_TYPE_PP(elem) == IS_OBJECT);
        m->type = Wrapper<TypeInfoPtr>::value(*elem TSRMLS_CC);

        if(allowOptional)
        {
            zend_hash_index_find(member, 2, reinterpret_cast<void**>(&elem));
            assert(Z_TYPE_PP(elem) == IS_BOOL);
            m->optional = Z_BVAL_PP(elem) ? true : false;
            zend_hash_index_find(member, 3, reinterpret_cast<void**>(&elem));
            assert(Z_TYPE_PP(elem) == IS_LONG);
            m->tag = static_cast<int>(Z_LVAL_PP(elem));
        }
        else
        {
            m->optional = false;
            m->tag = 0;
        }

        if(m->optional)
        {
            optList.push_back(m);
        }
        else
        {
            reqMembers.push_back(m);
        }

        zend_hash_move_forward_ex(membersArray, &pos);
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
IcePHP::StructInfo::StructInfo(const string& ident, const string& n, zval* m TSRMLS_DC) :
    id(ident), name(n), _nullMarshalValue(0)
{
    DataMemberList opt;
    convertDataMembers(m, const_cast<DataMemberList&>(members), opt, false TSRMLS_CC);
    assert(opt.empty());
    const_cast<zend_class_entry*&>(zce) = nameToClass(name TSRMLS_CC);
    assert(zce);

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
IcePHP::StructInfo::getId() const
{
    return id;
}

bool
IcePHP::StructInfo::validate(zval* zv, bool throwException TSRMLS_DC)
{
    if(Z_TYPE_P(zv) == IS_NULL)
    {
        return true;
    }
    else if(Z_TYPE_P(zv) != IS_OBJECT)
    {
        if(throwException)
        {
            string s = zendTypeToString(Z_TYPE_P(zv));
            invalidArgument("expected struct value of type %s but received %s" TSRMLS_CC, zce->name, s.c_str());
        }
        return false;
    }

    //
    // Compare class entries.
    //
    zend_class_entry* ce = Z_OBJCE_P(zv);
    if(ce != zce)
    {
        if(throwException)
        {
            invalidArgument("expected struct value of type %s but received %s" TSRMLS_CC, zce->name, ce->name);
        }
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
    return _variableLength ? Ice::OptionalFormatFSize : Ice::OptionalFormatVSize;
}

bool
IcePHP::StructInfo::usesClasses() const
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
IcePHP::StructInfo::marshal(zval* zv, Ice::OutputStream* os, ObjectMap* objectMap, bool optional TSRMLS_DC)
{
    assert(Z_TYPE_P(zv) == IS_NULL || (Z_TYPE_P(zv) == IS_OBJECT && Z_OBJCE_P(zv) == zce));

    if(Z_TYPE_P(zv) == IS_NULL)
    {
        if(_nullMarshalValue == 0)
        {
            MAKE_STD_ZVAL(_nullMarshalValue);
            if(object_init_ex(_nullMarshalValue, const_cast<zend_class_entry*>(zce)) != SUCCESS)
            {
                runtimeError("unable to initialize object of type %s" TSRMLS_CC, zce->name);
                throw AbortMarshaling();
            }

            if(!invokeMethod(_nullMarshalValue, ZEND_CONSTRUCTOR_FUNC_NAME TSRMLS_CC))
            {
                assert(false);
            }
        }
        zv = _nullMarshalValue;
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

        void* data;
        if(zend_hash_find(Z_OBJPROP_P(zv), STRCAST(member->name.c_str()), member->name.size() + 1, &data) == FAILURE)
        {
            runtimeError("member `%s' of %s is not defined" TSRMLS_CC, member->name.c_str(), id.c_str());
            throw AbortMarshaling();
        }

        zval** val = reinterpret_cast<zval**>(data);
        if(!member->type->validate(*val, false TSRMLS_CC))
        {
            invalidArgument("invalid value for %s member `%s'" TSRMLS_CC, id.c_str(), member->name.c_str());
            throw AbortMarshaling();
        }

        member->type->marshal(*val, os, objectMap, false TSRMLS_CC);
    }

    if(optional && _variableLength)
    {
        os->endSize(sizePos);
    }
}

void
IcePHP::StructInfo::unmarshal(Ice::InputStream* is, const UnmarshalCallbackPtr& cb,
                              const CommunicatorInfoPtr& comm, zval* target, void* closure, bool optional TSRMLS_DC)
{
    zval* zv;
    MAKE_STD_ZVAL(zv);
    AutoDestroy destroy(zv);

    if(object_init_ex(zv, const_cast<zend_class_entry*>(zce)) != SUCCESS)
    {
        runtimeError("unable to initialize object of type %s" TSRMLS_CC, zce->name);
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
        member->type->unmarshal(is, member, comm, zv, 0, false TSRMLS_CC);
    }

    cb->unmarshaled(zv, target, closure TSRMLS_CC);
}

void
IcePHP::StructInfo::print(zval* zv, IceUtilInternal::Output& out, PrintObjectHistory* history TSRMLS_DC)
{
    if(!validate(zv, false TSRMLS_CC))
    {
        out << "<invalid value - expected " << id << ">";
        return;
    }

    if(Z_TYPE_P(zv) == IS_NULL)
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
            void* data;
            if(zend_hash_find(Z_OBJPROP_P(zv), STRCAST(member->name.c_str()), member->name.size() + 1, &data) ==
               SUCCESS)
            {
                zval** val = reinterpret_cast<zval**>(data);
                member->type->print(*val, out, history TSRMLS_CC);
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
    for(DataMemberList::const_iterator p = members.begin(); p != members.end(); ++p)
    {
        (*p)->type->destroy();
    }
    const_cast<DataMemberList&>(members).clear();
    if(_nullMarshalValue != 0)
    {
        zval_ptr_dtor(&_nullMarshalValue);
        _nullMarshalValue = 0;
    }
}

//
// SequenceInfo implementation.
//
IcePHP::SequenceInfo::SequenceInfo(const string& ident, zval* e TSRMLS_DC) :
    id(ident)
{
    const_cast<TypeInfoPtr&>(elementType) = Wrapper<TypeInfoPtr>::value(e TSRMLS_CC);
}

string
IcePHP::SequenceInfo::getId() const
{
    return id;
}

bool
IcePHP::SequenceInfo::validate(zval* zv, bool TSRMLS_DC)
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
    return elementType->variableLength() ? Ice::OptionalFormatFSize : Ice::OptionalFormatVSize;
}

bool
IcePHP::SequenceInfo::usesClasses() const
{
    return elementType->usesClasses();
}

void
IcePHP::SequenceInfo::marshal(zval* zv, Ice::OutputStream* os, ObjectMap* objectMap, bool optional TSRMLS_DC)
{
    Ice::Int sz = 0;
    HashTable* arr = 0;

    if(Z_TYPE_P(zv) != IS_NULL)
    {
        assert(Z_TYPE_P(zv) == IS_ARRAY); // validate() should have caught this.
        arr = Z_ARRVAL_P(zv);
        sz = static_cast<Ice::Int>(zend_hash_num_elements(arr));
    }

    Ice::OutputStream::size_type sizePos = 0;
    if(optional)
    {
        if(elementType->variableLength())
        {
            sizePos = os->startSize();
        }
        else if(elementType->wireSize() > 1)
        {
            os->writeSize(sz == 0 ? 1 : sz * elementType->wireSize() + (sz > 254 ? 5 : 1));
        }
    }

    if(sz == 0)
    {
        os->writeSize(0);
    }
    else
    {
        PrimitiveInfoPtr pi = PrimitiveInfoPtr::dynamicCast(elementType);
        if(pi)
        {
            marshalPrimitiveSequence(pi, zv, os TSRMLS_CC);
            return;
        }

        HashPosition pos;
        zend_hash_internal_pointer_reset_ex(arr, &pos);

        os->writeSize(sz);

        void* data;
        while(zend_hash_get_current_data_ex(arr, &data, &pos) != FAILURE)
        {
            zval** val = reinterpret_cast<zval**>(data);
            if(!elementType->validate(*val, false TSRMLS_CC))
            {
                invalidArgument("invalid value for sequence element `%s'" TSRMLS_CC, id.c_str());
                throw AbortMarshaling();
            }
            elementType->marshal(*val, os, objectMap, false TSRMLS_CC);
            zend_hash_move_forward_ex(arr, &pos);
        }
    }

    if(optional && elementType->variableLength())
    {
        os->endSize(sizePos);
    }
}

void
IcePHP::SequenceInfo::unmarshal(Ice::InputStream* is, const UnmarshalCallbackPtr& cb,
                                const CommunicatorInfoPtr& comm, zval* target, void* closure, bool optional TSRMLS_DC)
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
        unmarshalPrimitiveSequence(pi, is, cb, target, closure TSRMLS_CC);
        return;
    }

    zval* zv;
    MAKE_STD_ZVAL(zv);
    array_init(zv);
    AutoDestroy destroy(zv);

    Ice::Int sz = is->readSize();
    for(Ice::Int i = 0; i < sz; ++i)
    {
        void* cl = reinterpret_cast<void*>(i);
        elementType->unmarshal(is, this, comm, zv, cl, false TSRMLS_CC);
    }

    cb->unmarshaled(zv, target, closure TSRMLS_CC);
}

void
IcePHP::SequenceInfo::print(zval* zv, IceUtilInternal::Output& out, PrintObjectHistory* history TSRMLS_DC)
{
    if(!validate(zv, false TSRMLS_CC))
    {
        out << "<invalid value - expected " << id << ">";
        return;
    }

    if(Z_TYPE_P(zv) == IS_NULL)
    {
        out << "{}";
    }
    else
    {
        assert(Z_TYPE_P(zv) == IS_ARRAY);

        HashTable* arr = Z_ARRVAL_P(zv);

        HashPosition pos;
        zend_hash_internal_pointer_reset_ex(arr, &pos);

        out.sb();

        int i = 0;
        void* data;
        while(zend_hash_get_current_data_ex(arr, &data, &pos) != FAILURE)
        {
            zval** val = reinterpret_cast<zval**>(data);
            out << nl << '[' << i << "] = ";
            elementType->print(*val, out, history TSRMLS_CC);
            zend_hash_move_forward_ex(arr, &pos);
            ++i;
        }

        out.eb();
    }
}

void
IcePHP::SequenceInfo::unmarshaled(zval* zv, zval* target, void* closure TSRMLS_DC)
{
    assert(Z_TYPE_P(target) == IS_ARRAY);
    long i = reinterpret_cast<long>(closure);
    add_index_zval(target, i, zv);
    Z_ADDREF_P(zv);
}

void
IcePHP::SequenceInfo::destroy()
{
    if(elementType)
    {
        elementType->destroy();
        const_cast<TypeInfoPtr&>(elementType) = 0;
    }
}

void
IcePHP::SequenceInfo::marshalPrimitiveSequence(const PrimitiveInfoPtr& pi, zval* zv, Ice::OutputStream* os TSRMLS_DC)
{
    HashTable* arr = Z_ARRVAL_P(zv);

    HashPosition pos;
    zend_hash_internal_pointer_reset_ex(arr, &pos);

    Ice::Int sz = static_cast<Ice::Int>(zend_hash_num_elements(arr));
    assert(sz > 0);

    switch(pi->kind)
    {
    case PrimitiveInfo::KindBool:
    {
        Ice::BoolSeq seq(sz);
        Ice::Int i = 0;
        void* data;
        while(zend_hash_get_current_data_ex(arr, &data, &pos) != FAILURE)
        {
            zval** val = reinterpret_cast<zval**>(data);
            if(!pi->validate(*val, true TSRMLS_CC))
            {
                throw AbortMarshaling();
            }
            seq[i++] = Z_BVAL_P(*val) ? true : false;
            zend_hash_move_forward_ex(arr, &pos);
        }
#if defined(_MSC_VER) && (_MSC_VER < 1300)
        os->writeBoolSeq(seq);
#else
        os->write(seq);
#endif
        break;
    }
    case PrimitiveInfo::KindByte:
    {
        Ice::ByteSeq seq(sz);
        Ice::Int i = 0;
        void* data;
        while(zend_hash_get_current_data_ex(arr, &data, &pos) != FAILURE)
        {
            zval** val = reinterpret_cast<zval**>(data);
            if(!pi->validate(*val, true TSRMLS_CC))
            {
                throw AbortMarshaling();
            }
            long l = Z_LVAL_P(*val);
            assert(l >= 0 && l <= 255);
            seq[i++] = static_cast<Ice::Byte>(l);
            zend_hash_move_forward_ex(arr, &pos);
        }
        os->write(&seq[0], &seq[0] + seq.size());
        break;
    }
    case PrimitiveInfo::KindShort:
    {
        Ice::ShortSeq seq(sz);
        Ice::Int i = 0;
        void* data;
        while(zend_hash_get_current_data_ex(arr, &data, &pos) != FAILURE)
        {
            zval** val = reinterpret_cast<zval**>(data);
            if(!pi->validate(*val, true TSRMLS_CC))
            {
                throw AbortMarshaling();
            }
            long l = Z_LVAL_P(*val);
            assert(l >= SHRT_MIN && l <= SHRT_MAX);
            seq[i++] = static_cast<Ice::Short>(l);
            zend_hash_move_forward_ex(arr, &pos);
        }
        os->write(&seq[0], &seq[0] + seq.size());
        break;
    }
    case PrimitiveInfo::KindInt:
    {
        Ice::IntSeq seq(sz);
        Ice::Int i = 0;
        void* data;
        while(zend_hash_get_current_data_ex(arr, &data, &pos) != FAILURE)
        {
            zval** val = reinterpret_cast<zval**>(data);
            if(!pi->validate(*val, true TSRMLS_CC))
            {
                throw AbortMarshaling();
            }
            long l = Z_LVAL_P(*val);
            assert(l >= INT_MIN && l <= INT_MAX);
            seq[i++] = static_cast<Ice::Int>(l);
            zend_hash_move_forward_ex(arr, &pos);
        }
        os->write(&seq[0], &seq[0] + seq.size());
        break;
    }
    case PrimitiveInfo::KindLong:
    {
        Ice::LongSeq seq(sz);
        Ice::Int i = 0;
        void* data;
        while(zend_hash_get_current_data_ex(arr, &data, &pos) != FAILURE)
        {
            zval** val = reinterpret_cast<zval**>(data);
            if(!pi->validate(*val, true TSRMLS_CC))
            {
                throw AbortMarshaling();
            }
            //
            // The platform's 'long' type may not be 64 bits, so we also accept
            // a string argument for this type.
            //
            assert(Z_TYPE_P(*val) == IS_LONG || Z_TYPE_P(*val) == IS_STRING);
            Ice::Long l;
            if(Z_TYPE_P(*val) == IS_LONG)
            {
                l = Z_LVAL_P(*val);
            }
            else
            {
                string sval(Z_STRVAL_P(*val), Z_STRLEN_P(*val));
                IceUtilInternal::stringToInt64(sval, l);
            }
            seq[i++] = l;
            zend_hash_move_forward_ex(arr, &pos);
        }
        os->write(&seq[0], &seq[0] + seq.size());
        break;
    }
    case PrimitiveInfo::KindFloat:
    {
        Ice::FloatSeq seq(sz);
        Ice::Int i = 0;
        void* data;
        while(zend_hash_get_current_data_ex(arr, &data, &pos) != FAILURE)
        {
            zval** val = reinterpret_cast<zval**>(data);
            if(!pi->validate(*val, true TSRMLS_CC))
            {
                throw AbortMarshaling();
            }
            double d = 0;
            if(Z_TYPE_P(*val) == IS_DOUBLE)
            {
                d = Z_DVAL_P(*val);
            }
            else if(Z_TYPE_P(*val) == IS_LONG)
            {
                d = static_cast<double>(Z_LVAL_P(*val));
            }
            else
            {
                assert(false); // validate() should have caught this.
            }
            seq[i++] = static_cast<Ice::Float>(d);
            zend_hash_move_forward_ex(arr, &pos);
        }
        os->write(&seq[0], &seq[0] + seq.size());
        break;
    }
    case PrimitiveInfo::KindDouble:
    {
        Ice::DoubleSeq seq(sz);
        Ice::Int i = 0;
        void* data;
        while(zend_hash_get_current_data_ex(arr, &data, &pos) != FAILURE)
        {
            zval** val = reinterpret_cast<zval**>(data);
            if(!pi->validate(*val, true TSRMLS_CC))
            {
                throw AbortMarshaling();
            }
            double d = 0;
            if(Z_TYPE_P(*val) == IS_DOUBLE)
            {
                d = Z_DVAL_P(*val);
            }
            else if(Z_TYPE_P(*val) == IS_LONG)
            {
                d = static_cast<double>(Z_LVAL_P(*val));
            }
            else
            {
                assert(false); // validate() should have caught this.
            }
            seq[i++] = d;
            zend_hash_move_forward_ex(arr, &pos);
        }
        os->write(&seq[0], &seq[0] + seq.size());
        break;
    }
    case PrimitiveInfo::KindString:
    {
        Ice::StringSeq seq(sz);
        Ice::Int i = 0;
        void* data;
        while(zend_hash_get_current_data_ex(arr, &data, &pos) != FAILURE)
        {
            zval** val = reinterpret_cast<zval**>(data);
            if(!pi->validate(*val, true TSRMLS_CC))
            {
                throw AbortMarshaling();
            }
            string s;
            if(Z_TYPE_P(*val) == IS_STRING)
            {
                s = string(Z_STRVAL_P(*val), Z_STRLEN_P(*val));
            }
            else
            {
                assert(Z_TYPE_P(*val) == IS_NULL);
            }
            seq[i++] = s;
            zend_hash_move_forward_ex(arr, &pos);
        }
        os->write(seq);
        break;
    }
    }
}

void
IcePHP::SequenceInfo::unmarshalPrimitiveSequence(const PrimitiveInfoPtr& pi, Ice::InputStream* is,
                                                 const UnmarshalCallbackPtr& cb, zval* target, void* closure TSRMLS_DC)
{
    zval* zv;
    MAKE_STD_ZVAL(zv);
    array_init(zv);
    AutoDestroy destroy(zv);

    switch(pi->kind)
    {
    case PrimitiveInfo::KindBool:
    {
        pair<const bool*, const bool*> pr;
        IceUtil::ScopedArray<bool> arr;
        is->read(pr, arr);
        for(const bool* p = pr.first; p != pr.second; ++p)
        {
            add_next_index_bool(zv, *p ? 1 : 0);
        }
        break;
    }
    case PrimitiveInfo::KindByte:
    {
        pair<const Ice::Byte*, const Ice::Byte*> pr;
        is->read(pr);
        for(const Ice::Byte* p = pr.first; p != pr.second; ++p)
        {
            add_next_index_long(zv, *p & 0xff);
        }
        break;
    }
    case PrimitiveInfo::KindShort:
    {
        pair<const Ice::Short*, const Ice::Short*> pr;
        IceUtil::ScopedArray<Ice::Short> arr;
        is->read(pr, arr);
        for(const Ice::Short* p = pr.first; p != pr.second; ++p)
        {
            add_next_index_long(zv, *p);
        }
        break;
    }
    case PrimitiveInfo::KindInt:
    {
        pair<const Ice::Int*, const Ice::Int*> pr;
        IceUtil::ScopedArray<Ice::Int> arr;
        is->read(pr, arr);
        for(const Ice::Int* p = pr.first; p != pr.second; ++p)
        {
            add_next_index_long(zv, *p);
        }
        break;
    }
    case PrimitiveInfo::KindLong:
    {
        pair<const Ice::Long*, const Ice::Long*> pr;
        IceUtil::ScopedArray<Ice::Long> arr;
        is->read(pr, arr);
        Ice::Int i = 0;
        for(const Ice::Long* p = pr.first; p != pr.second; ++p, ++i)
        {
            zval* val;
            MAKE_STD_ZVAL(val);
            //
            // The platform's 'long' type may not be 64 bits, so we store 64-bit
            // values as a string.
            //
            if(sizeof(Ice::Long) > sizeof(long) && (*p < LONG_MIN || *p > LONG_MAX))
            {
                string str = IceUtilInternal::int64ToString(*p);
                ZVAL_STRINGL(val, STRCAST(str.c_str()), static_cast<int>(str.length()), 1);
            }
            else
            {
                ZVAL_LONG(val, static_cast<long>(*p));
            }
            add_index_zval(zv, i, val);
        }
        break;
    }
    case PrimitiveInfo::KindFloat:
    {
        pair<const Ice::Float*, const Ice::Float*> pr;
        IceUtil::ScopedArray<Ice::Float> arr;
        is->read(pr, arr);
        Ice::Int i = 0;
        for(const Ice::Float* p = pr.first; p != pr.second; ++p, ++i)
        {
            zval* val;
            MAKE_STD_ZVAL(val);
            ZVAL_DOUBLE(val, *p);
            add_index_zval(zv, i, val);
        }
        break;
    }
    case PrimitiveInfo::KindDouble:
    {
        pair<const Ice::Double*, const Ice::Double*> pr;
        IceUtil::ScopedArray<Ice::Double> arr;
        is->read(pr, arr);
        Ice::Int i = 0;
        for(const Ice::Double* p = pr.first; p != pr.second; ++p, ++i)
        {
            zval* val;
            MAKE_STD_ZVAL(val);
            ZVAL_DOUBLE(val, *p);
            add_index_zval(zv, i, val);
        }
        break;
    }
    case PrimitiveInfo::KindString:
    {
        Ice::StringSeq seq;
        is->read(seq, true);
        Ice::Int i = 0;
        for(Ice::StringSeq::iterator p = seq.begin(); p != seq.end(); ++p, ++i)
        {
            zval* val;
            MAKE_STD_ZVAL(val);
            ZVAL_STRINGL(val, STRCAST(p->c_str()), static_cast<int>(p->length()), 1);
            add_index_zval(zv, i, val);
        }
        break;
    }
    }

    cb->unmarshaled(zv, target, closure TSRMLS_CC);
}

//
// DictionaryInfo implementation.
//
IcePHP::DictionaryInfo::DictionaryInfo(const string& ident, zval* k, zval* v TSRMLS_DC) :
    id(ident)
{
    const_cast<TypeInfoPtr&>(keyType) = Wrapper<TypeInfoPtr>::value(k TSRMLS_CC);
    const_cast<TypeInfoPtr&>(valueType) = Wrapper<TypeInfoPtr>::value(v TSRMLS_CC);

    _variableLength = keyType->variableLength() || valueType->variableLength();
    _wireSize = keyType->wireSize() + valueType->wireSize();
}

string
IcePHP::DictionaryInfo::getId() const
{
    return id;
}

bool
IcePHP::DictionaryInfo::validate(zval* zv, bool TSRMLS_DC)
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
    return _variableLength ? Ice::OptionalFormatFSize : Ice::OptionalFormatVSize;
}

bool
IcePHP::DictionaryInfo::usesClasses() const
{
    return valueType->usesClasses();
}

void
IcePHP::DictionaryInfo::marshal(zval* zv, Ice::OutputStream* os, ObjectMap* objectMap, bool optional TSRMLS_DC)
{
    Ice::Int sz = 0;
    HashTable* arr = 0;

    if(Z_TYPE_P(zv) != IS_NULL)
    {
        assert(Z_TYPE_P(zv) == IS_ARRAY); // validate() should have caught this.
        arr = Z_ARRVAL_P(zv);
        sz = static_cast<Ice::Int>(zend_hash_num_elements(arr));
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

    PrimitiveInfoPtr piKey = PrimitiveInfoPtr::dynamicCast(keyType);
    EnumInfoPtr enKey = EnumInfoPtr::dynamicCast(keyType);
    if(!enKey && (!piKey || piKey->kind == PrimitiveInfo::KindFloat || piKey->kind == PrimitiveInfo::KindDouble))
    {
        invalidArgument("dictionary type `%s' cannot be marshaled" TSRMLS_CC, id.c_str());
        throw AbortMarshaling();
    }

    os->writeSize(sz);

    if(sz > 0)
    {
        HashPosition pos;
        void* data;

        zend_hash_internal_pointer_reset_ex(arr, &pos);
        while(zend_hash_get_current_data_ex(arr, &data, &pos) != FAILURE)
        {
            zval** val = reinterpret_cast<zval**>(data);

            //
            // Get the key (which can be a long or a string).
            //
            char* keyStr;
            uint keyLen;
            ulong keyNum;
            int hashKeyType = zend_hash_get_current_key_ex(arr, &keyStr, &keyLen, &keyNum, 0, &pos);

            //
            // Store the key in a zval so that we can reuse the marshaling logic.
            //
            zval* zkey;
            MAKE_STD_ZVAL(zkey);
            AutoDestroy destroy(zkey);

            if(hashKeyType == HASH_KEY_IS_LONG)
            {
                ZVAL_LONG(zkey, keyNum);
            }
            else
            {
                ZVAL_STRINGL(zkey, keyStr, keyLen - 1, 1);
            }

            //
            // Convert the zval to the required type, if necessary.
            //
            if(piKey)
            {
                switch(piKey->kind)
                {
                case PrimitiveInfo::KindBool:
                {
                    convert_to_boolean(zkey);
                    break;
                }

                case PrimitiveInfo::KindByte:
                case PrimitiveInfo::KindShort:
                case PrimitiveInfo::KindInt:
                case PrimitiveInfo::KindLong:
                {
                    if(hashKeyType == HASH_KEY_IS_STRING)
                    {
                        convert_to_long(zkey);
                    }
                    break;
                }

                case PrimitiveInfo::KindString:
                {
                    if(hashKeyType == HASH_KEY_IS_LONG)
                    {
                        convert_to_string(zkey);
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
                if(hashKeyType == HASH_KEY_IS_STRING)
                {
                    convert_to_long(zkey);
                }
            }

            //
            // Marshal the key.
            //
            if(!keyType->validate(zkey, false TSRMLS_CC))
            {
                invalidArgument("invalid key in `%s' element" TSRMLS_CC, id.c_str());
                throw AbortMarshaling();
            }
            keyType->marshal(zkey, os, objectMap, false TSRMLS_CC);

            //
            // Marshal the value.
            //
            if(!valueType->validate(*val, false TSRMLS_CC))
            {
                invalidArgument("invalid value in `%s' element" TSRMLS_CC, id.c_str());
                throw AbortMarshaling();
            }
            valueType->marshal(*val, os, objectMap, false TSRMLS_CC);

            zend_hash_move_forward_ex(arr, &pos);
        }
    }

    if(optional && _variableLength)
    {
        os->endSize(sizePos);
    }
}

void
IcePHP::DictionaryInfo::unmarshal(Ice::InputStream* is, const UnmarshalCallbackPtr& cb,
                                  const CommunicatorInfoPtr& comm, zval* target, void* closure, bool optional TSRMLS_DC)
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

    PrimitiveInfoPtr piKey = PrimitiveInfoPtr::dynamicCast(keyType);
    EnumInfoPtr enKey = EnumInfoPtr::dynamicCast(keyType);
    if(!enKey && (!piKey || piKey->kind == PrimitiveInfo::KindFloat || piKey->kind == PrimitiveInfo::KindDouble))
    {
        invalidArgument("dictionary type `%s' cannot be unmarshaled" TSRMLS_CC, id.c_str());
        throw AbortMarshaling();
    }

    zval* zv;
    MAKE_STD_ZVAL(zv);
    array_init(zv);
    AutoDestroy destroy(zv);

    KeyCallbackPtr keyCB = new KeyCallback;

    Ice::Int sz = is->readSize();
    for(Ice::Int i = 0; i < sz; ++i)
    {
        //
        // A dictionary key cannot be a class (or contain one), so the key must be
        // available immediately.
        //
        keyType->unmarshal(is, keyCB, comm, 0, 0, false TSRMLS_CC);
        assert(keyCB->key);

        //
        // Allocate a callback that holds a reference to the key.
        //
        ValueCallbackPtr valueCB = new ValueCallback(keyCB->key TSRMLS_CC);

        //
        // Pass the key to the callback.
        //
        valueType->unmarshal(is, valueCB, comm, zv, 0, false TSRMLS_CC);
    }

    cb->unmarshaled(zv, target, closure TSRMLS_CC);
}

void
IcePHP::DictionaryInfo::print(zval* zv, IceUtilInternal::Output& out, PrintObjectHistory* history TSRMLS_DC)
{
    if(!validate(zv, false TSRMLS_CC))
    {
        out << "<invalid value - expected " << id << ">";
        return;
    }

    if(Z_TYPE_P(zv) == IS_NULL)
    {
        out << "{}";
    }
    else
    {
        HashTable* arr = Z_ARRVAL_P(zv);
        HashPosition pos;
        void* data;
        bool first = true;

        out.sb();

        zend_hash_internal_pointer_reset_ex(arr, &pos);
        while(zend_hash_get_current_data_ex(arr, &data, &pos) != FAILURE)
        {
            zval** val = reinterpret_cast<zval**>(data);

            //
            // Get the key (which can be a long or a string).
            //
            char* keyStr;
            uint keyLen;
            ulong keyNum;
            int hashKeyType = zend_hash_get_current_key_ex(arr, &keyStr, &keyLen, &keyNum, 0, &pos);

            if(first)
            {
                first = false;
            }
            else
            {
                out << nl;
            }
            out << nl << "key = ";
            if(hashKeyType == HASH_KEY_IS_LONG)
            {
                out << keyNum;
            }
            else
            {
                out << keyStr;
            }
            out << nl << "value = ";
            valueType->print(*val, out, history TSRMLS_CC);

            zend_hash_move_forward_ex(arr, &pos);
        }

        out.eb();
    }
}

IcePHP::DictionaryInfo::KeyCallback::KeyCallback() :
    key(0)
{
}

IcePHP::DictionaryInfo::KeyCallback::~KeyCallback()
{
    if(key)
    {
        zval_ptr_dtor(&key);
    }
}

void
IcePHP::DictionaryInfo::KeyCallback::unmarshaled(zval* zv, zval*, void* TSRMLS_DC)
{
    if(key)
    {
        zval_ptr_dtor(&key);
    }

    key = zv;
    Z_ADDREF_P(key);
}

IcePHP::DictionaryInfo::ValueCallback::ValueCallback(zval* k TSRMLS_DC) :
    key(k)
{
    Z_ADDREF_P(key);
}

IcePHP::DictionaryInfo::ValueCallback::~ValueCallback()
{
    zval_ptr_dtor(&key);
}

void
IcePHP::DictionaryInfo::ValueCallback::unmarshaled(zval* zv, zval* target, void* TSRMLS_DC)
{
    assert(Z_TYPE_P(target) == IS_ARRAY);

    switch(Z_TYPE_P(key))
    {
    case IS_LONG:
        add_index_zval(target, Z_LVAL_P(key), zv);
        break;
    case IS_BOOL:
        add_index_zval(target, Z_BVAL_P(key) ? 1 : 0, zv);
        break;
    case IS_STRING:
        add_assoc_zval_ex(target, Z_STRVAL_P(key), Z_STRLEN_P(key) + 1, zv);
        break;
    default:
        assert(false);
        return;
    }
    Z_ADDREF_P(zv);
}

void
IcePHP::DictionaryInfo::destroy()
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
IcePHP::ClassInfo::ClassInfo(const string& ident TSRMLS_DC) :
    id(ident), compactId(-1), preserve(false), interface(false), zce(0), defined(false)
{
}

void
IcePHP::ClassInfo::define(const string& n, Ice::Int compact, bool pres, bool intf, zval* b, zval* m TSRMLS_DC)
{
    const_cast<string&>(name) = n;
    const_cast<Ice::Int&>(compactId) = static_cast<Ice::Int>(compact);
    const_cast<bool&>(preserve) = pres;
    const_cast<bool&>(interface) = intf;

    if(b)
    {
        TypeInfoPtr p = Wrapper<TypeInfoPtr>::value(b TSRMLS_CC);
        const_cast<ClassInfoPtr&>(base) = ClassInfoPtr::dynamicCast(p);
        assert(base);
    }

    if(m)
    {
        convertDataMembers(m, const_cast<DataMemberList&>(members), const_cast<DataMemberList&>(optionalMembers),
                           true TSRMLS_CC);
    }

    const_cast<bool&>(defined) = true;
#ifdef ICEPHP_USE_NAMESPACES
    const string valueClass = "Ice\\Value";
#else
    const string valueClass = "Ice_Value";
#endif
    const_cast<zend_class_entry*&>(zce) = nameToClass(interface ? valueClass : name TSRMLS_CC);
    // LocalObject and interfaces does not have a native PHP equivalent.
    assert(zce || id == "::Ice::LocalObject" || interface);
}

string
IcePHP::ClassInfo::getId() const
{
    return id;
}

bool
IcePHP::ClassInfo::validate(zval* val, bool TSRMLS_DC)
{
    if(Z_TYPE_P(val) == IS_OBJECT)
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
    return Ice::OptionalFormatClass;
}

bool
IcePHP::ClassInfo::usesClasses() const
{
    return true;
}

void
IcePHP::ClassInfo::marshal(zval* zv, Ice::OutputStream* os, ObjectMap* objectMap, bool TSRMLS_DC)
{
    if(!defined)
    {
        runtimeError("class or interface %s is declared but not defined" TSRMLS_CC, id.c_str());
        throw AbortMarshaling();
    }

    if(Z_TYPE_P(zv) == IS_NULL)
    {
        Ice::ObjectPtr nil;
        os->write(nil);
        return;
    }

    assert(Z_TYPE_P(zv) == IS_OBJECT); // validate() should have caught this.
    assert(checkClass(Z_OBJCE_P(zv), const_cast<zend_class_entry*>(zce))); // validate() should have caught this.

    //
    // Ice::ObjectWriter is a subclass of Ice::Object that wraps a PHP object for marshaling.
    // It is possible that this PHP object has already been marshaled, therefore we first must
    // check the object map to see if this object is present. If so, we use the existing ObjectWriter,
    // otherwise we create a new one. The key of the map is the object's handle.
    //
    Ice::ObjectPtr writer;
    assert(objectMap);
    ObjectMap::iterator q = objectMap->find(Z_OBJ_HANDLE_P(zv));
    if(q == objectMap->end())
    {
        writer = new ObjectWriter(zv, objectMap, this TSRMLS_CC);
        objectMap->insert(ObjectMap::value_type(Z_OBJ_HANDLE_P(zv), writer));
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
IcePHP::ClassInfo::unmarshal(Ice::InputStream* is, const UnmarshalCallbackPtr& cb,
                             const CommunicatorInfoPtr& comm, zval* target, void* closure, bool TSRMLS_DC)
{
    if(!defined)
    {
        runtimeError("class or interface %s is declared but not defined" TSRMLS_CC, id.c_str());
        throw AbortMarshaling();
    }

    //
    // This callback is notified when the Slice value is actually read. The StreamUtil object
    // attached to the stream keeps a reference to the callback object to ensure it lives
    // long enough.
    //
    ReadObjectCallbackPtr rocb = new ReadObjectCallback(this, cb, target, closure TSRMLS_CC);
    StreamUtil* util = reinterpret_cast<StreamUtil*>(is->getClosure());
    assert(util);
    util->add(rocb);
    is->read(patchObject, rocb.get());
}

void
IcePHP::ClassInfo::print(zval* zv, IceUtilInternal::Output& out, PrintObjectHistory* history TSRMLS_DC)
{
    if(!validate(zv, false TSRMLS_CC))
    {
        out << "<invalid value - expected " << id << ">";
        return;
    }

    if(Z_TYPE_P(zv) == IS_NULL)
    {
        out << "<nil>";
    }
    else
    {
        map<unsigned int, int>::iterator q = history->objects.find(Z_OBJ_HANDLE_P(zv));
        if(q != history->objects.end())
        {
            out << "<object #" << q->second << ">";
        }
        else
        {
            out << "object #" << history->index << " (" << id << ')';
            history->objects.insert(map<unsigned int, int>::value_type(Z_OBJ_HANDLE_P(zv), history->index));
            ++history->index;
            out.sb();
            printMembers(zv, out, history TSRMLS_CC);
            out.eb();
        }
    }
}

void
IcePHP::ClassInfo::destroy()
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
IcePHP::ClassInfo::printMembers(zval* zv, IceUtilInternal::Output& out, PrintObjectHistory* history TSRMLS_DC)
{
    if(base)
    {
        base->printMembers(zv, out, history TSRMLS_CC);
    }

    DataMemberList::const_iterator q;

    for(q = members.begin(); q != members.end(); ++q)
    {
        DataMemberPtr member = *q;

        out << nl << member->name << " = ";
        void* data;
        if(zend_hash_find(Z_OBJPROP_P(zv), STRCAST(member->name.c_str()), member->name.size() + 1, &data) == SUCCESS)
        {
            zval** val = reinterpret_cast<zval**>(data);
            member->type->print(*val, out, history TSRMLS_CC);
        }
        else
        {
            out << "<not defined>";
        }
    }

    for(q = optionalMembers.begin(); q != optionalMembers.end(); ++q)
    {
        DataMemberPtr member = *q;

        out << nl << member->name << " = ";
        void* data;
        if(zend_hash_find(Z_OBJPROP_P(zv), STRCAST(member->name.c_str()), member->name.size() + 1, &data) == SUCCESS)
        {
            zval** val = reinterpret_cast<zval**>(data);
            if(isUnset(*val TSRMLS_CC))
            {
                out << "<unset>";
            }
            else
            {
                member->type->print(*val, out, history TSRMLS_CC);
            }
        }
        else
        {
            out << "<not defined>";
        }
    }
}

bool
IcePHP::ClassInfo::isA(const string& typeId) const
{
    if(id == typeId)
    {
        return true;
    }

    return base && base->isA(typeId);
}

//
// ProxyInfo implementation.
//
IcePHP::ProxyInfo::ProxyInfo(const string& ident TSRMLS_DC) :
    id(ident), defined(false)
{
}

void
IcePHP::ProxyInfo::define(zval* b, zval* i TSRMLS_DC)
{
    if(b)
    {
        TypeInfoPtr p = Wrapper<TypeInfoPtr>::value(b);
        const_cast<ProxyInfoPtr&>(base) = ProxyInfoPtr::dynamicCast(p);
        assert(base);
    }

    if(i)
    {
        HashTable* interfacesArray = Z_ARRVAL_P(i);
        void* data;
        HashPosition pos;
        zend_hash_internal_pointer_reset_ex(interfacesArray, &pos);
        while(zend_hash_get_current_data_ex(interfacesArray, &data, &pos) != FAILURE)
        {
            zval** interfaceType = reinterpret_cast<zval**>(data);
            TypeInfoPtr t = Wrapper<TypeInfoPtr>::value(*interfaceType TSRMLS_CC);
            ProxyInfoPtr c = ProxyInfoPtr::dynamicCast(t);
            assert(c);
            const_cast<ProxyInfoList&>(interfaces).push_back(c);
            zend_hash_move_forward_ex(interfacesArray, &pos);
        }
    }

    const_cast<bool&>(defined) = true;
}

string
IcePHP::ProxyInfo::getId() const
{
    return id;
}

bool
IcePHP::ProxyInfo::validate(zval* zv, bool throwException TSRMLS_DC)
{
    if(Z_TYPE_P(zv) != IS_NULL)
    {
        if(Z_TYPE_P(zv) != IS_OBJECT || (Z_TYPE_P(zv) == IS_OBJECT && Z_OBJCE_P(zv) != proxyClassEntry))
        {
            if(throwException)
            {
                string s = zendTypeToString(Z_TYPE_P(zv));
                invalidArgument("expected proxy value or null but received %s" TSRMLS_CC, s.c_str());
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
    return Ice::OptionalFormatFSize;
}

void
IcePHP::ProxyInfo::marshal(zval* zv, Ice::OutputStream* os, ObjectMap*, bool optional TSRMLS_DC)
{
    Ice::OutputStream::size_type sizePos = 0;
    if(optional)
    {
        sizePos = os->startSize();
    }

    if(Z_TYPE_P(zv) == IS_NULL)
    {
        os->write(Ice::ObjectPrx());
    }
    else
    {
        assert(Z_TYPE_P(zv) == IS_OBJECT && Z_OBJCE_P(zv) == proxyClassEntry); // validate() should have caught this.
        Ice::ObjectPrx proxy;
        ProxyInfoPtr info;
        if(!fetchProxy(zv, proxy, info TSRMLS_CC))
        {
            throw AbortMarshaling();
        }
        if(!info->isA(id))
        {
            invalidArgument("proxy is not narrowed to %s" TSRMLS_CC, id.c_str());
            throw AbortMarshaling();
        }
        os->write(proxy);
    }

    if(optional)
    {
        os->endSize(sizePos);
    }
}

void
IcePHP::ProxyInfo::unmarshal(Ice::InputStream* is, const UnmarshalCallbackPtr& cb,
                             const CommunicatorInfoPtr& comm, zval* target, void* closure, bool optional TSRMLS_DC)
{
    zval* zv;
    MAKE_STD_ZVAL(zv);
    AutoDestroy destroy(zv);

    if(optional)
    {
        is->skip(4);
    }

    Ice::ObjectPrx proxy;
    is->read(proxy);

    if(!proxy)
    {
        ZVAL_NULL(zv);
        cb->unmarshaled(zv, target, closure TSRMLS_CC);
        return;
    }

    if(!defined)
    {
        runtimeError("proxy %s is declared but not defined" TSRMLS_CC, id.c_str());
        throw AbortMarshaling();
    }

    if(!createProxy(zv, proxy, this, comm TSRMLS_CC))
    {
        throw AbortMarshaling();
    }
    cb->unmarshaled(zv, target, closure TSRMLS_CC);
}

void
IcePHP::ProxyInfo::print(zval* zv, IceUtilInternal::Output& out, PrintObjectHistory* TSRMLS_DC)
{
    if(!validate(zv, false TSRMLS_CC))
    {
        out << "<invalid value - expected " << id << ">";
        return;
    }

    if(Z_TYPE_P(zv) == IS_NULL)
    {
        out << "<nil>";
    }
    else
    {
        Ice::ObjectPrx proxy;
        ProxyInfoPtr info;
        if(!fetchProxy(zv, proxy, info TSRMLS_CC))
        {
            return;
        }
        out << proxy->ice_toString();
    }
}

void
IcePHP::ProxyInfo::destroy()
{
    const_cast<ProxyInfoPtr&>(base) = 0;
    const_cast<ProxyInfoList&>(interfaces).clear();
}

bool
IcePHP::ProxyInfo::isA(const string& typeId) const
{
    if(id == typeId)
    {
        return true;
    }

    if(base && base->isA(typeId))
    {
        return true;
    }

    for(ProxyInfoList::const_iterator p = interfaces.begin(); p != interfaces.end(); ++p)
    {
        if((*p)->isA(typeId))
        {
            return true;
        }
    }

    return false;
}

void
IcePHP::ProxyInfo::addOperation(const string& name, const OperationPtr& op)
{
    operations.insert(OperationMap::value_type(Slice::PHP::fixIdent(name), op));
}

IcePHP::OperationPtr
IcePHP::ProxyInfo::getOperation(const string& name) const
{
    OperationPtr op;
    OperationMap::const_iterator p = operations.find(name);
    if(p != operations.end())
    {
        op = p->second;
    }
    if(!op && base)
    {
        op = base->getOperation(name);
    }
    if(!op && !interfaces.empty())
    {
        for(ProxyInfoList::const_iterator q = interfaces.begin(); q != interfaces.end() && !op; ++q)
        {
            op = (*q)->getOperation(name);
        }
    }
    return op;
}

//
// ObjectWriter implementation.
//
IcePHP::ObjectWriter::ObjectWriter(zval* object, ObjectMap* objectMap, const ClassInfoPtr& formal TSRMLS_DC) :
    _object(object), _map(objectMap), _formal(formal)
{
#ifdef ZTS
    this->TSRMLS_C = TSRMLS_C;
#endif

    Z_ADDREF_P(_object);

    if(!_formal || !_formal->interface)
    {
        //
        // For no interface types We need to determine the most-derived Slice type supported by this object.
        // This is typically a Slice class, but it can also be an interface.
        //
        // The caller may have provided a ClassInfo representing the formal type, in
        // which case we ensure that the actual type is compatible with the formal type.
        //
        _info = getClassInfoByClass(Z_OBJCE_P(object), formal ? const_cast<zend_class_entry*>(formal->zce) : 0 TSRMLS_CC);
        assert(_info);
    }
}

IcePHP::ObjectWriter::~ObjectWriter()
{
    zval_ptr_dtor(&_object);
}

void
IcePHP::ObjectWriter::ice_preMarshal()
{
    string name = "ice_premarshal"; // Must be lowercase.
    if(zend_hash_exists(&Z_OBJCE_P(_object)->function_table, STRCAST(name.c_str()), static_cast<uint>(name.size() + 1)))
    {
        if(!invokeMethod(_object, name TSRMLS_CC))
        {
            throw AbortMarshaling();
        }
    }
}

void
IcePHP::ObjectWriter::_iceWrite(Ice::OutputStream* os) const
{
    Ice::SlicedDataPtr slicedData;

    if(_info && _info->preserve)
    {
        //
        // Retrieve the SlicedData object that we stored as a hidden member of the PHP object.
        //
        slicedData = StreamUtil::getSlicedDataMember(_object, const_cast<ObjectMap*>(_map) TSRMLS_CC);
    }

    os->startValue(slicedData);

    if(_formal && _formal->interface)
    {
        //
        // For an interface by value we just marshal the Ice type id
        // of the object in its own slice.
        //
        zval* ret = 0;

        zend_try
        {
            const char* func = "ice_id";
            zval* obj = const_cast<zval*>(_object);
            zend_call_method(&obj, 0, 0, const_cast<char*>(func), static_cast<int>(strlen(func)), &ret, 0, 0,
                             0 TSRMLS_CC);
        }
        zend_catch
        {
            ret = 0;
        }
        zend_end_try();

        //
        // Bail out if an exception has already been thrown.
        //
        if(!ret || EG(exception))
        {
            throw AbortMarshaling();
        }

        AutoDestroy destroy(ret);

        if(Z_TYPE_P(ret) != IS_STRING)
        {
            throw AbortMarshaling();
        }

        string id(Z_STRVAL_P(ret), Z_STRLEN_P(ret));
        os->startSlice(id, -1, true);
        os->endSlice();
    }
    else
    {
        if(_info->id != "::Ice::UnknownSlicedValue")
        {
            ClassInfoPtr info = _info;
            while(info && info->id != Ice::Object::ice_staticId())
            {
                assert(info->base); // All classes have the Ice::Object base type.
                const bool lastSlice = info->base->id == Ice::Object::ice_staticId();
                os->startSlice(info->id, info->compactId, lastSlice);

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
IcePHP::ObjectWriter::_iceRead(Ice::InputStream*)
{
    assert(false);
}

void
IcePHP::ObjectWriter::writeMembers(Ice::OutputStream* os, const DataMemberList& members) const
{
    for(DataMemberList::const_iterator q = members.begin(); q != members.end(); ++q)
    {
        DataMemberPtr member = *q;

        void* data;
        if(zend_hash_find(Z_OBJPROP_P(_object),
                          STRCAST(member->name.c_str()), static_cast<int>(member->name.size() + 1), &data) == FAILURE)
        {
            runtimeError("member `%s' of %s is not defined" TSRMLS_CC, member->name.c_str(), _info->id.c_str());
            throw AbortMarshaling();
        }

        zval** val = reinterpret_cast<zval**>(data);

        if(member->optional && (isUnset(*val TSRMLS_CC) ||
           !os->writeOptional(member->tag, member->type->optionalFormat())))
        {
            continue;
        }

        if(!member->type->validate(*val, false TSRMLS_CC))
        {
            invalidArgument("invalid value for %s member `%s'" TSRMLS_CC, _info->id.c_str(),
                            member->name.c_str());
            throw AbortMarshaling();
        }

        member->type->marshal(*val, os, _map, member->optional TSRMLS_CC);
    }
}

//
// ObjectReader implementation.
//
IcePHP::ObjectReader::ObjectReader(zval* object, const ClassInfoPtr& info, const CommunicatorInfoPtr& comm TSRMLS_DC) :
    _object(object), _info(info), _communicator(comm)
{
#ifdef ZTS
    this->TSRMLS_C = TSRMLS_C;
#endif

    Z_ADDREF_P(_object);
}

IcePHP::ObjectReader::~ObjectReader()
{
    zval_ptr_dtor(&_object);
}

void
IcePHP::ObjectReader::ice_postUnmarshal()
{
    string name = "ice_postunmarshal"; // Must be lowercase.
    if(zend_hash_exists(&Z_OBJCE_P(_object)->function_table, STRCAST(name.c_str()), static_cast<int>(name.size() + 1)))
    {
        if(!invokeMethod(_object, name TSRMLS_CC))
        {
            throw AbortMarshaling();
        }
    }
}

void
IcePHP::ObjectReader::_iceWrite(Ice::OutputStream*) const
{
    assert(false);
}

void
IcePHP::ObjectReader::_iceRead(Ice::InputStream* is)
{
    is->startValue();

    const bool unknown = _info->id == "::Ice::UnknownSlicedValue";

    //
    // Unmarshal the slices of a user-defined class.
    //
    if(!unknown)
    {
        ClassInfoPtr info = _info;
        while(info && info->id != Ice::Object::ice_staticId())
        {
            is->startSlice();

            DataMemberList::const_iterator p;

            for(p = info->members.begin(); p != info->members.end(); ++p)
            {
                DataMemberPtr member = *p;
                member->type->unmarshal(is, member, _communicator, _object, 0, false TSRMLS_CC);
            }

            //
            // The optional members have already been sorted by tag.
            //
            for(p = info->optionalMembers.begin(); p != info->optionalMembers.end(); ++p)
            {
                DataMemberPtr member = *p;
                if(is->readOptional(member->tag, member->type->optionalFormat()))
                {
                    member->type->unmarshal(is, member, _communicator, _object, 0, true TSRMLS_CC);
                }
                else
                {
                    zval* zv;
                    MAKE_STD_ZVAL(zv);
                    AutoDestroy destroy(zv);
                    assignUnset(zv TSRMLS_CC);
                    member->setMember(_object, zv TSRMLS_CC);
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
        // Define the "unknownTypeId" member for an instance of UnknownSlicedObject.
        //
        if(unknown)
        {
            assert(!_slicedData->slices.empty());

            const string typeId = _slicedData->slices[0]->typeId;
            zval* zv;
            MAKE_STD_ZVAL(zv);
            AutoDestroy typeIdDestroyer(zv);
            ZVAL_STRINGL(zv, STRCAST(typeId.c_str()), static_cast<int>(typeId.size()), 1);
            add_property_zval(_object, STRCAST("unknownTypeId"), zv);
        }
    }
}

ClassInfoPtr
IcePHP::ObjectReader::getInfo() const
{
    return _info;
}

zval*
IcePHP::ObjectReader::getObject() const
{
    return _object;
}

Ice::SlicedDataPtr
IcePHP::ObjectReader::getSlicedData() const
{
    return _slicedData;
}

//
// ReadObjectCallback implementation.
//
IcePHP::ReadObjectCallback::ReadObjectCallback(const ClassInfoPtr& info, const UnmarshalCallbackPtr& cb,
                                               zval* target, void* closure TSRMLS_DC) :
    _info(info), _cb(cb), _target(target), _closure(closure)
{
#ifdef ZTS
    this->TSRMLS_C = TSRMLS_C;
#endif

    //
    // Keep a reference to the target to prevent it from being deallocated until the
    // Ice object is actually read.
    //
    if(_target)
    {
        Z_ADDREF_P(_target);
    }
}

IcePHP::ReadObjectCallback::~ReadObjectCallback()
{
    if(_target)
    {
        zval_ptr_dtor(&_target);
    }
}

void
IcePHP::ReadObjectCallback::invoke(const Ice::ObjectPtr& p)
{
    if(p)
    {
        ObjectReaderPtr reader = ObjectReaderPtr::dynamicCast(p);
        assert(reader);

        //
        // Verify that the unmarshaled object is compatible with the formal type.
        //
        zval* obj = reader->getObject();
        if(!_info->interface && !reader->getInfo()->isA(_info->id))
        {
            Ice::UnexpectedObjectException ex(__FILE__, __LINE__);
            ex.reason = "unmarshaled object is not an instance of " + _info->id;
            ex.type = reader->getInfo()->id;
            ex.expectedType = _info->id;
            throw ex;
        }

        _cb->unmarshaled(obj, _target, _closure TSRMLS_CC);
    }
    else
    {
        zval* zv;
        MAKE_STD_ZVAL(zv);
        AutoDestroy destroy(zv);
        ZVAL_NULL(zv);
        _cb->unmarshaled(zv, _target, _closure TSRMLS_CC);
    }
}

//
// ExceptionInfo implementation.
//
zval*
IcePHP::ExceptionInfo::unmarshal(Ice::InputStream* is, const CommunicatorInfoPtr& comm TSRMLS_DC)
{
    zval* zv;
    MAKE_STD_ZVAL(zv);
    AutoDestroy destroy(zv);

    if(object_init_ex(zv, zce) != SUCCESS)
    {
        runtimeError("unable to initialize object of type %s" TSRMLS_CC, zce->name);
        throw AbortMarshaling();
    }

    //
    // NOTE: The type id for the first slice has already been read.
    //
    ExceptionInfoPtr info = this;
    while(info)
    {
        is->startSlice();

        DataMemberList::iterator q;

        for(q = info->members.begin(); q != info->members.end(); ++q)
        {
            DataMemberPtr member = *q;
            member->type->unmarshal(is, member, comm, zv, 0, false TSRMLS_CC);
        }

        //
        // The optional members have already been sorted by tag.
        //
        for(q = info->optionalMembers.begin(); q != info->optionalMembers.end(); ++q)
        {
            DataMemberPtr member = *q;
            if(is->readOptional(member->tag, member->type->optionalFormat()))
            {
                member->type->unmarshal(is, member, comm, zv, 0, true TSRMLS_CC);
            }
            else
            {
                zval* un;
                MAKE_STD_ZVAL(un);
                AutoDestroy destroy(un);
                assignUnset(un TSRMLS_CC);
                member->setMember(zv, un TSRMLS_CC);
            }
        }

        is->endSlice();

        info = info->base;
    }

    return destroy.release();
}

void
IcePHP::ExceptionInfo::print(zval* zv, IceUtilInternal::Output& out TSRMLS_DC)
{
    out << "exception " << id;
    out.sb();

    if(Z_TYPE_P(zv) != IS_OBJECT)
    {
        string s = zendTypeToString(Z_TYPE_P(zv));
        out << nl << "expected exception value of type " << zce->name << " but received " << s;
        out.eb();
        return;
    }

    //
    // Compare class entries.
    //
    zend_class_entry* ce = Z_OBJCE_P(zv);
    if(ce != zce)
    {
        out << nl << "expected exception value of type " << zce->name << " but received " << ce->name;
        out.eb();
        return;
    }

    PrintObjectHistory history;
    history.index = 0;

    out << "exception " << id;
    out.sb();
    printMembers(zv, out, &history TSRMLS_CC);
    out.eb();
}

void
IcePHP::ExceptionInfo::printMembers(zval* zv, IceUtilInternal::Output& out, PrintObjectHistory* history TSRMLS_DC)
{
    if(base)
    {
        base->printMembers(zv, out, history TSRMLS_CC);
    }

    DataMemberList::iterator q;

    for(q = members.begin(); q != members.end(); ++q)
    {
        DataMemberPtr member = *q;

        out << nl << member->name << " = ";
        void* data;
        if(zend_hash_find(Z_OBJPROP_P(zv), STRCAST(member->name.c_str()), static_cast<int>(member->name.size() + 1),
                          &data) == SUCCESS)
        {
            zval** val = reinterpret_cast<zval**>(data);
            member->type->print(*val, out, history TSRMLS_CC);
        }
        else
        {
            out << "<not defined>";
        }
    }

    for(q = optionalMembers.begin(); q != optionalMembers.end(); ++q)
    {
        DataMemberPtr member = *q;

        out << nl << member->name << " = ";
        void* data;
        if(zend_hash_find(Z_OBJPROP_P(zv), STRCAST(member->name.c_str()), static_cast<int>(member->name.size() + 1),
                          &data) == SUCCESS)
        {
            zval** val = reinterpret_cast<zval**>(data);
            if(isUnset(*val TSRMLS_CC))
            {
                out << "<unset>";
            }
            else
            {
                member->type->print(*val, out, history TSRMLS_CC);
            }
        }
        else
        {
            out << "<not defined>";
        }
    }
}

bool
IcePHP::ExceptionInfo::isA(const string& typeId) const
{
    if(id == typeId)
    {
        return true;
    }

    if(base && base->isA(typeId))
    {
        return true;
    }

    return false;
}

//
// ExceptionReader implementation.
//
IcePHP::ExceptionReader::ExceptionReader(const CommunicatorInfoPtr& communicatorInfo, const ExceptionInfoPtr& info
                                         TSRMLS_DC) :
    _communicatorInfo(communicatorInfo), _info(info)
{
#ifdef ZTS
    this->TSRMLS_C = TSRMLS_C;
#endif
}

IcePHP::ExceptionReader::~ExceptionReader()
    throw()
{
}

string
IcePHP::ExceptionReader::ice_id() const
{
    return _info->id;
}

IcePHP::ExceptionReader*
IcePHP::ExceptionReader::ice_clone() const
{
    assert(false);
    return 0;
}

void
IcePHP::ExceptionReader::ice_throw() const
{
    throw *this;
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

    const_cast<zval*&>(_ex) = _info->unmarshal(is, _communicatorInfo TSRMLS_CC);

    const_cast<Ice::SlicedDataPtr&>(_slicedData) = is->endException(_info->preserve);
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
    return _ex;
}

Ice::SlicedDataPtr
IcePHP::ExceptionReader::getSlicedData() const
{
    return _slicedData;
}

//
// IdResolver
//
IcePHP::IdResolver::IdResolver(TSRMLS_D)
{
#ifdef ZTS
    this->TSRMLS_C = TSRMLS_C;
#endif
}

string
IcePHP::IdResolver::resolve(Ice::Int id) const
{
    CompactIdMap* m = reinterpret_cast<CompactIdMap*>(ICE_G(compactIdToClassInfoMap));
    if(m)
    {
        CompactIdMap::iterator p = m->find(id);
        if(p != m->end())
        {
            return p->second->id;
        }
    }
    return string();
}

#ifdef _WIN32
extern "C"
#endif
static zend_object_value
handleTypeInfoAlloc(zend_class_entry* ce TSRMLS_DC)
{
    zend_object_value result;

    Wrapper<TypeInfoPtr>* obj = Wrapper<TypeInfoPtr>::create(ce TSRMLS_CC);
    assert(obj);

    result.handle =
        zend_objects_store_put(obj, 0, reinterpret_cast<zend_objects_free_object_storage_t>(handleTypeInfoFreeStorage),
                               0 TSRMLS_CC);
    result.handlers = &_typeInfoHandlers;

    return result;
}

#ifdef _WIN32
extern "C"
#endif
static void
handleTypeInfoFreeStorage(void* p TSRMLS_DC)
{
    Wrapper<TypeInfoPtr>* obj = static_cast<Wrapper<TypeInfoPtr>*>(p);
    delete obj->ptr;
    zend_object_std_dtor(static_cast<zend_object*>(p) TSRMLS_CC);
    efree(p);
}

static bool
createTypeInfo(zval* zv, const TypeInfoPtr& p TSRMLS_DC)
{
    if(object_init_ex(zv, typeInfoClassEntry) != SUCCESS)
    {
        runtimeError("unable to initialize type" TSRMLS_CC);
        return false;
    }

    Wrapper<TypeInfoPtr>* ze = static_cast<Wrapper<TypeInfoPtr>*>(zend_object_store_get_object(zv TSRMLS_CC));
    assert(!ze->ptr);
    ze->ptr = new TypeInfoPtr(p);

    return true;
}

ZEND_FUNCTION(IcePHP_defineEnum)
{
    char* id;
    int idLen;
    zval* enumerators;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, const_cast<char*>("sa"), &id, &idLen, &enumerators) == FAILURE)
    {
        return;
    }

    EnumInfoPtr type = new EnumInfo(id, enumerators TSRMLS_CC);

    if(!createTypeInfo(return_value, type TSRMLS_CC))
    {
        RETURN_NULL();
    }
}

ZEND_FUNCTION(IcePHP_defineStruct)
{
    char* id;
    int idLen;
    char* name;
    int nameLen;
    zval* members;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, const_cast<char*>("ssa"), &id, &idLen, &name, &nameLen,
                             &members) == FAILURE)
    {
        return;
    }

    StructInfoPtr type = new StructInfo(id, name, members TSRMLS_CC);

    if(!createTypeInfo(return_value, type TSRMLS_CC))
    {
        RETURN_NULL();
    }
}

ZEND_FUNCTION(IcePHP_defineSequence)
{
    char* id;
    int idLen;
    zval* element;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, const_cast<char*>("so"), &id, &idLen, &element) == FAILURE)
    {
        return;
    }

    SequenceInfoPtr type = new SequenceInfo(id, element TSRMLS_CC);

    if(!createTypeInfo(return_value, type TSRMLS_CC))
    {
        RETURN_NULL();
    }
}

ZEND_FUNCTION(IcePHP_defineDictionary)
{
    char* id;
    int idLen;
    zval* key;
    zval* value;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, const_cast<char*>("soo"), &id, &idLen, &key, &value) == FAILURE)
    {
        return;
    }

    DictionaryInfoPtr type = new DictionaryInfo(id, key, value TSRMLS_CC);

    if(!createTypeInfo(return_value, type TSRMLS_CC))
    {
        RETURN_NULL();
    }
}

ZEND_FUNCTION(IcePHP_declareProxy)
{
    char* id;
    int idLen;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, const_cast<char*>("s"), &id, &idLen) == FAILURE)
    {
        return;
    }

    ProxyInfoPtr type = getProxyInfo(id TSRMLS_CC);
    if(!type)
    {
        type = new ProxyInfo(id TSRMLS_CC);
        addProxyInfo(type TSRMLS_CC);
    }

    if(!createTypeInfo(return_value, type TSRMLS_CC))
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


    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, const_cast<char*>("so!a!"), &id, &idLen, &base, &interfaces) ==
       FAILURE)
    {
        return;
    }

    ProxyInfoPtr type = getProxyInfo(id TSRMLS_CC);
    if(!type)
    {
        type = new ProxyInfo(id TSRMLS_CC);
        addProxyInfo(type TSRMLS_CC);
    }

    type->define(base, interfaces TSRMLS_CC);

    if(!createTypeInfo(return_value, type TSRMLS_CC))
    {
        RETURN_NULL();
    }
}

ZEND_FUNCTION(IcePHP_declareClass)
{
    char* id;
    int idLen;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, const_cast<char*>("s"), &id, &idLen) == FAILURE)
    {
        return;
    }

    ClassInfoPtr type = getClassInfoById(id TSRMLS_CC);
    if(!type)
    {
        type = new ClassInfo(id TSRMLS_CC);
        addClassInfoById(type TSRMLS_CC);
    }

    if(!createTypeInfo(return_value, type TSRMLS_CC))
    {
        RETURN_NULL();
    }
}

ZEND_FUNCTION(IcePHP_defineClass)
{
    char* id;
    int idLen;
    char* name;
    int nameLen;
    long compactId;
    zend_bool preserve;
    zend_bool interface;
    zval* base;
    zval* members;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, const_cast<char*>("sslbbo!a!"), &id, &idLen, &name, &nameLen,
                             &compactId, &preserve, &interface, &base, &members) == FAILURE)
    {
        return;
    }

    ClassInfoPtr type = getClassInfoById(id TSRMLS_CC);
    if(!type)
    {
        type = new ClassInfo(id TSRMLS_CC);
        addClassInfoById(type TSRMLS_CC);
    }

    type->define(name, static_cast<Ice::Int>(compactId), preserve ? true : false, interface ? true : false, base,
                 members TSRMLS_CC);
    if(!interface)
    {
        addClassInfoByName(type TSRMLS_CC);
    }

    if(type->compactId != -1)
    {
        CompactIdMap* m = reinterpret_cast<CompactIdMap*>(ICE_G(compactIdToClassInfoMap));
        if(!m)
        {
            m = new CompactIdMap;
            ICE_G(compactIdToClassInfoMap) = m;
        }
        m->insert(CompactIdMap::value_type(type->compactId, type));
    }

    if(!createTypeInfo(return_value, type TSRMLS_CC))
    {
        RETURN_NULL();
    }
}

#ifdef _WIN32
extern "C"
#endif
static zend_object_value
handleExceptionInfoAlloc(zend_class_entry* ce TSRMLS_DC)
{
    zend_object_value result;

    Wrapper<ExceptionInfoPtr>* obj = Wrapper<ExceptionInfoPtr>::create(ce TSRMLS_CC);
    assert(obj);

    result.handle =
        zend_objects_store_put(obj, 0,
            reinterpret_cast<zend_objects_free_object_storage_t>(handleExceptionInfoFreeStorage), 0 TSRMLS_CC);
    result.handlers = &_exceptionInfoHandlers;

    return result;
}

#ifdef _WIN32
extern "C"
#endif
static void
handleExceptionInfoFreeStorage(void* p TSRMLS_DC)
{
    Wrapper<ExceptionInfoPtr>* obj = static_cast<Wrapper<ExceptionInfoPtr>*>(p);
    delete obj->ptr;
    zend_object_std_dtor(static_cast<zend_object*>(p) TSRMLS_CC);
    efree(p);
}


static bool
createExceptionInfo(zval* zv, const ExceptionInfoPtr& p TSRMLS_DC)
{
    if(object_init_ex(zv, exceptionInfoClassEntry) != SUCCESS)
    {
        runtimeError("unable to initialize exception info" TSRMLS_CC);
        return false;
    }

    Wrapper<ExceptionInfoPtr>* ze = static_cast<Wrapper<ExceptionInfoPtr>*>(zend_object_store_get_object(zv TSRMLS_CC));
    assert(!ze->ptr);
    ze->ptr = new ExceptionInfoPtr(p);

    return true;
}

ZEND_FUNCTION(IcePHP_defineException)
{
    char* id;
    int idLen;
    char* name;
    int nameLen;
    zend_bool preserve;
    zval* base;
    zval* members;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, const_cast<char*>("ssbo!a!"), &id, &idLen, &name, &nameLen,
                             &preserve, &base, &members) == FAILURE)
    {
        return;
    }

    ExceptionInfoPtr ex = new ExceptionInfo();
    ex->id = id;
    ex->name = name;
    ex->preserve = preserve ? true : false;
    if(base)
    {
        ex->base = Wrapper<ExceptionInfoPtr>::value(base TSRMLS_CC);
    }
    if(members)
    {
        convertDataMembers(members, ex->members, ex->optionalMembers, true TSRMLS_CC);
    }

    ex->usesClasses = false;

    //
    // Only examine the required members to see if any use classes.
    //
    for(DataMemberList::iterator p = ex->members.begin(); p != ex->members.end(); ++p)
    {
        if(!ex->usesClasses)
        {
            ex->usesClasses = (*p)->type->usesClasses();
        }
    }

    ex->zce = nameToClass(ex->name TSRMLS_CC);

    assert(!getExceptionInfo(ex->id TSRMLS_CC));

    ExceptionInfoMap* m;
    if(ICE_G(exceptionInfoMap))
    {
        m = reinterpret_cast<ExceptionInfoMap*>(ICE_G(exceptionInfoMap));
    }
    else
    {
        m = new ExceptionInfoMap;
        ICE_G(exceptionInfoMap) = m;
    }
    m->insert(ExceptionInfoMap::value_type(ex->id, ex));

    if(!createExceptionInfo(return_value, ex TSRMLS_CC))
    {
        RETURN_NULL();
    }
}

ZEND_FUNCTION(IcePHP_stringify)
{
    if(ZEND_NUM_ARGS() != 2)
    {
        WRONG_PARAM_COUNT;
    }

    zval* v;
    zval* t;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, const_cast<char*>("zz"), &v, &t) == FAILURE)
    {
        return;
    }

    TypeInfoPtr type = Wrapper<TypeInfoPtr>::value(t TSRMLS_CC);
    assert(type);

    ostringstream ostr;
    IceUtilInternal::Output out(ostr);
    PrintObjectHistory history;
    history.index = 0;
    type->print(v, out, &history TSRMLS_CC);

    string str = ostr.str();
    RETURN_STRINGL(STRCAST(str.c_str()), static_cast<int>(str.length()), 1);
}

ZEND_FUNCTION(IcePHP_stringifyException)
{
    if(ZEND_NUM_ARGS() != 2)
    {
        WRONG_PARAM_COUNT;
    }

    zval* v;
    zval* t;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, const_cast<char*>("oo"), &v, &t) == FAILURE)
    {
        return;
    }

    ExceptionInfoPtr ex = Wrapper<ExceptionInfoPtr>::value(t TSRMLS_CC);
    assert(ex);

    ostringstream ostr;
    IceUtilInternal::Output out(ostr);
    ex->print(v, out TSRMLS_CC);

    string str = ostr.str();
    RETURN_STRINGL(STRCAST(str.c_str()), static_cast<int>(str.length()), 1);
}

//
// Necessary to suppress warnings from zend_function_entry in php-5.2.
//
#if defined(__GNUC__)
#  pragma GCC diagnostic ignored "-Wwrite-strings"
#endif

//
// Predefined methods for IcePHP_TypeInfo.
//
static zend_function_entry _typeInfoMethods[] =
{
    {0, 0, 0}
};

//
// Predefined methods for IcePHP_ExceptionInfo.
//
static zend_function_entry _exceptionInfoMethods[] =
{
    {0, 0, 0}
};

bool
IcePHP::isUnset(zval* zv TSRMLS_DC)
{
    if(Z_TYPE_P(zv) == IS_STRING)
    {
        return _unsetGUID == string(Z_STRVAL_P(zv), Z_STRLEN_P(zv));
    }
    return false;
}

void
IcePHP::assignUnset(zval* zv TSRMLS_DC)
{
    *zv = *ICE_G(unset);
    zval_copy_ctor(zv);
}

bool
IcePHP::typesInit(INIT_FUNC_ARGS)
{
    zend_class_entry ce;

    //
    // Register the IcePHP_TypeInfo class.
    //
    INIT_CLASS_ENTRY(ce, "IcePHP_TypeInfo", _typeInfoMethods);
    ce.create_object = handleTypeInfoAlloc;
    typeInfoClassEntry = zend_register_internal_class(&ce TSRMLS_CC);
    memcpy(&_typeInfoHandlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));

    //
    // Register the IcePHP_ExceptionInfo class.
    //
    INIT_CLASS_ENTRY(ce, "IcePHP_ExceptionInfo", _exceptionInfoMethods);
    ce.create_object = handleExceptionInfoAlloc;
    exceptionInfoClassEntry = zend_register_internal_class(&ce TSRMLS_CC);
    memcpy(&_exceptionInfoHandlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));

#ifdef ICEPHP_USE_NAMESPACES
    REGISTER_NS_STRING_CONSTANT("Ice", "None", const_cast<char*>(_unsetGUID.c_str()), CONST_CS|CONST_PERSISTENT);
#else
    REGISTER_STRING_CONSTANT("Ice_Unset", const_cast<char*>(_unsetGUID.c_str()), CONST_CS|CONST_PERSISTENT);
#endif

    return true;
}

//
// enable warning again
//
#if defined(__GNUC__)
#  pragma GCC diagnostic error "-Wwrite-strings"
#endif

bool
IcePHP::typesRequestInit(TSRMLS_D)
{
    //
    // Create the global variables for the primitive types.
    //
    for(int i = static_cast<int>(PrimitiveInfo::KindBool); i <= static_cast<int>(PrimitiveInfo::KindString); ++i)
    {
        PrimitiveInfoPtr type = new PrimitiveInfo();
        type->kind = static_cast<PrimitiveInfo::Kind>(i);

        zval* zv;
        MAKE_STD_ZVAL(zv);
        if(!createTypeInfo(zv, type TSRMLS_CC))
        {
            zval_ptr_dtor(&zv);
            return false;
        }
        string name = "IcePHP__t_" + type->getId();
        ZEND_SET_SYMBOL(&EG(symbol_table), const_cast<char*>(name.c_str()), zv);
    }

    ICE_G(idToClassInfoMap) = 0;
    ICE_G(nameToClassInfoMap) = 0;
    ICE_G(proxyInfoMap) = 0;
    ICE_G(exceptionInfoMap) = 0;

    zval* unset;
    MAKE_STD_ZVAL(unset);
    ZVAL_STRINGL(unset, STRCAST(_unsetGUID.c_str()), static_cast<int>(_unsetGUID.length()), 1);
    ICE_G(unset) = unset;

    return true;
}

bool
IcePHP::typesRequestShutdown(TSRMLS_D)
{
    if(ICE_G(proxyInfoMap))
    {
        ProxyInfoMap* m = static_cast<ProxyInfoMap*>(ICE_G(proxyInfoMap));
        for(ProxyInfoMap::iterator p = m->begin(); p != m->end(); ++p)
        {
            p->second->destroy();
        }
        delete m;
    }

    if(ICE_G(idToClassInfoMap))
    {
        ClassInfoMap* m = static_cast<ClassInfoMap*>(ICE_G(idToClassInfoMap));
        for(ClassInfoMap::iterator p = m->begin(); p != m->end(); ++p)
        {
            p->second->destroy();
        }
        delete m;
    }

    if(ICE_G(nameToClassInfoMap))
    {
        ClassInfoMap* m = static_cast<ClassInfoMap*>(ICE_G(nameToClassInfoMap));
        delete m;
    }

    delete static_cast<ExceptionInfoMap*>(ICE_G(exceptionInfoMap));

    zval_ptr_dtor(&ICE_G(unset));

    return true;
}
