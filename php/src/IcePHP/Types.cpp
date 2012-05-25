// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
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

extern "C"
{
static zend_object_value handleTypeInfoAlloc(zend_class_entry* TSRMLS_DC);
static void handleTypeInfoFreeStorage(void* TSRMLS_DC);

static zend_object_value handleExceptionInfoAlloc(zend_class_entry* TSRMLS_DC);
static void handleExceptionInfoFreeStorage(void* TSRMLS_DC);
}

typedef map<string, ProxyInfoPtr> ProxyInfoMap;
typedef map<string, ClassInfoPtr> ClassInfoMap;
typedef map<string, ExceptionInfoPtr> ExceptionInfoMap;

//
// getProxyInfo()
//
static IcePHP::ProxyInfoPtr
getProxyInfo(const string& id TSRMLS_DC)
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
getMostDerived(zend_class_entry* formal, zend_class_entry* cls, const ClassInfoPtr& c TSRMLS_DC)
{
    assert(checkClass(cls, formal));
    ClassInfoPtr curr = c;

    ClassInfoPtr info = getClassInfoByName(cls->name TSRMLS_CC);
    if(info)
    {
        if(!curr || info->isA(curr->id))
        {
            curr = info;
        }
    }

    if(cls->parent && checkClass(cls->parent, formal))
    {
        curr = getMostDerived(formal, cls->parent, curr TSRMLS_CC);
    }

    //
    // Only check the interfaces if we don't have a base class.
    //
    if(!curr)
    {
        for(zend_uint i = 0; i < cls->num_interfaces && !info; ++i)
        {
            if(checkClass(cls->interfaces[i], formal))
            {
                curr = getMostDerived(formal, cls->interfaces[i], curr TSRMLS_CC);
            }
        }
    }

    return curr;
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

namespace IcePHP
{

class ReadObjectCallback : public Ice::ReadObjectCallback
{
public:

    ReadObjectCallback(const ClassInfoPtr&, const UnmarshalCallbackPtr&, zval*, void* TSRMLS_DC);
    ~ReadObjectCallback();

    virtual void invoke(const ::Ice::ObjectPtr&);

private:

    ClassInfoPtr _info;
    UnmarshalCallbackPtr _cb;
    zval* _target;
    void* _closure;
#if ZTS
    TSRMLS_D;
#endif
zend_object_handle _h;
};

}

//
// SlicedDataUtil implementation
//
zend_class_entry* IcePHP::SlicedDataUtil::_slicedDataType = 0;
zend_class_entry* IcePHP::SlicedDataUtil::_sliceInfoType = 0;

IcePHP::SlicedDataUtil::~SlicedDataUtil()
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
IcePHP::SlicedDataUtil::add(const ObjectReaderPtr& reader)
{
    assert(reader->getSlicedData());
    _readers.insert(reader);
}

void
IcePHP::SlicedDataUtil::update(TSRMLS_D)
{
    for(set<ObjectReaderPtr>::iterator p = _readers.begin(); p != _readers.end(); ++p)
    {
        setMember((*p)->getObject(), (*p)->getSlicedData() TSRMLS_CC);
    }
}

void
IcePHP::SlicedDataUtil::setMember(zval* obj, const Ice::SlicedDataPtr& slicedData TSRMLS_DC)
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
        ZVAL_STRINGL(typeId, STRCAST((*p)->typeId.c_str()), (*p)->typeId.size(), 1);
        if(add_property_zval(slice, STRCAST("typeId"), typeId) != SUCCESS)
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
        // objects
        //
        zval* objects;
        MAKE_STD_ZVAL(objects);
        array_init(objects);
        AutoDestroy objectsDestroyer(objects);
        if(add_property_zval(slice, STRCAST("objects"), objects) != SUCCESS)
        {
            throw AbortMarshaling();
        }

        for(vector<Ice::ObjectPtr>::const_iterator q = (*p)->objects.begin(); q != (*p)->objects.end(); ++q)
        {
            //
            // Each element in the objects list is an instance of ObjectReader that wraps a PHP object.
            //
            assert(*q);
            ObjectReaderPtr r = ObjectReaderPtr::dynamicCast(*q);
            assert(r);
            zval* o = r->getObject();
            assert(Z_TYPE_P(o) == IS_OBJECT); // Should be non-nil.
            add_next_index_zval(objects, o); // Steals a reference.
            Z_ADDREF_P(o);
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
IcePHP::SlicedDataUtil::getMember(zval* obj, ObjectMap* objectMap TSRMLS_DC)
{
    Ice::SlicedDataPtr slicedData;

    string name = "_ice_slicedData";
    void* data;
    if(zend_hash_find(Z_OBJPROP_P(obj), STRCAST(name.c_str()), name.size() + 1, &data) == SUCCESS)
    {
        zval* sd = *(reinterpret_cast<zval**>(data));

        if(Z_TYPE_P(sd) != IS_NULL)
        {
            int status;

            //
            // The "slices" member is an array of Ice_SliceInfo objects.
            //
            status = zend_hash_find(Z_OBJPROP_P(sd), STRCAST("slices"), sizeof("slices"), &data);
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

                status = zend_hash_find(Z_OBJPROP_P(s), STRCAST("typeId"), sizeof("typeId"), &data);
                assert(status == SUCCESS);
                zval* typeId = *(reinterpret_cast<zval**>(data));
                assert(Z_TYPE_P(typeId) == IS_STRING);
                info->typeId = string(Z_STRVAL_P(typeId), Z_STRLEN_P(typeId));

                status = zend_hash_find(Z_OBJPROP_P(s), STRCAST("bytes"), sizeof("bytes"), &data);
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

                status = zend_hash_find(Z_OBJPROP_P(s), STRCAST("objects"), sizeof("objects"), &data);
                assert(status == SUCCESS);
                zval* objects = *(reinterpret_cast<zval**>(data));
                assert(Z_TYPE_P(objects) == IS_ARRAY);
                HashTable* oarr = Z_ARRVAL_P(objects);
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

                    info->objects.push_back(writer);
                    zend_hash_move_forward_ex(oarr, &opos);
                }

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
IcePHP::TypeInfo::usesClasses()
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
IcePHP::PrimitiveInfo::validate(zval* zv TSRMLS_DC)
{
    switch(kind)
    {
    case PrimitiveInfo::KindBool:
    {
        if(Z_TYPE_P(zv) != IS_BOOL)
        {
            string s = zendTypeToString(Z_TYPE_P(zv));
            invalidArgument("expected boolean value but received %s" TSRMLS_CC, s.c_str());
            return false;
        }
        break;
    }
    case PrimitiveInfo::KindByte:
    {
        if(Z_TYPE_P(zv) != IS_LONG)
        {
            string s = zendTypeToString(Z_TYPE_P(zv));
            invalidArgument("expected byte value but received %s" TSRMLS_CC, s.c_str());
            return false;
        }
        long val = Z_LVAL_P(zv);
        if(val < 0 || val > 255)
        {
            invalidArgument("value %ld is out of range for a byte" TSRMLS_CC, val);
            return false;
        }
        break;
    }
    case PrimitiveInfo::KindShort:
    {
        if(Z_TYPE_P(zv) != IS_LONG)
        {
            string s = zendTypeToString(Z_TYPE_P(zv));
            invalidArgument("expected short value but received %s" TSRMLS_CC, s.c_str());
            return false;
        }
        long val = Z_LVAL_P(zv);
        if(val < SHRT_MIN || val > SHRT_MAX)
        {
            invalidArgument("value %ld is out of range for a short" TSRMLS_CC, val);
            return false;
        }
        break;
    }
    case PrimitiveInfo::KindInt:
    {
        if(Z_TYPE_P(zv) != IS_LONG)
        {
            string s = zendTypeToString(Z_TYPE_P(zv));
            invalidArgument("expected int value but received %s" TSRMLS_CC, s.c_str());
            return false;
        }
        long val = Z_LVAL_P(zv);
        if(val < INT_MIN || val > INT_MAX)
        {
            invalidArgument("value %ld is out of range for an int" TSRMLS_CC, val);
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
            invalidArgument("expected long value but received %s" TSRMLS_CC, s.c_str());
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
                invalidArgument("invalid long value `%s'" TSRMLS_CC, Z_STRVAL_P(zv));
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
            invalidArgument("expected float value but received %s" TSRMLS_CC, s.c_str());
            return false;
        }
        break;
    }
    case PrimitiveInfo::KindDouble:
    {
        if(Z_TYPE_P(zv) != IS_DOUBLE && Z_TYPE_P(zv) != IS_LONG)
        {
            string s = zendTypeToString(Z_TYPE_P(zv));
            invalidArgument("expected double value but received %s" TSRMLS_CC, s.c_str());
            return false;
        }
        break;
    }
    case PrimitiveInfo::KindString:
    {
        if(Z_TYPE_P(zv) != IS_STRING && Z_TYPE_P(zv) != IS_NULL)
        {
            string s = zendTypeToString(Z_TYPE_P(zv));
            invalidArgument("expected string value but received %s" TSRMLS_CC, s.c_str());
            return false;
        }
        break;
    }
    }

    return true;
}

void
IcePHP::PrimitiveInfo::marshal(zval* zv, const Ice::OutputStreamPtr& os, ObjectMap* TSRMLS_DC)
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
IcePHP::PrimitiveInfo::unmarshal(const Ice::InputStreamPtr& is, const UnmarshalCallbackPtr& cb,
                                 const CommunicatorInfoPtr&, zval* target, void* closure TSRMLS_DC)
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
            ZVAL_STRINGL(zv, STRCAST(str.c_str()), str.length(), 1);
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
        ZVAL_STRINGL(zv, STRCAST(val.c_str()), val.length(), 1);
        break;
    }
    }

    cb->unmarshaled(zv, target, closure TSRMLS_CC);
}

void
IcePHP::PrimitiveInfo::print(zval* zv, IceUtilInternal::Output& out, PrintObjectHistory* TSRMLS_DC)
{
    if(!validate(zv TSRMLS_CC))
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
string
IcePHP::EnumInfo::getId() const
{
    return id;
}

bool
IcePHP::EnumInfo::validate(zval* zv TSRMLS_DC)
{
    if(Z_TYPE_P(zv) == IS_LONG)
    {
        Ice::StringSeq::size_type sz = static_cast<Ice::StringSeq::size_type>(Z_LVAL_P(zv));
        return sz >= 0 && sz < enumerators.size();
    }
    return false;
}

void
IcePHP::EnumInfo::marshal(zval* zv, const Ice::OutputStreamPtr& os, ObjectMap* TSRMLS_DC)
{
    assert(Z_TYPE_P(zv) == IS_LONG); // validate() should have caught this.
    Ice::Int val = static_cast<Ice::Int>(Z_LVAL_P(zv));
    Ice::Int count = static_cast<Ice::Int>(enumerators.size());
    assert(val >= 0 && val < count); // validate() should have caught this.

    if(count <= 127)
    {
        os->write(static_cast<Ice::Byte>(val));
    }
    else if(count <= 32767)
    {
        os->write(static_cast<Ice::Short>(val));
    }
    else
    {
        os->write(val);
    }
}

void
IcePHP::EnumInfo::unmarshal(const Ice::InputStreamPtr& is, const UnmarshalCallbackPtr& cb,
                            const CommunicatorInfoPtr&, zval* target, void* closure TSRMLS_DC)
{
    zval* zv;
    ALLOC_INIT_ZVAL(zv);
    AutoDestroy destroy(zv);

    Ice::Int val;
    Ice::Int count = static_cast<Ice::Int>(enumerators.size());
    if(count <= 127)
    {
        Ice::Byte b;
        is->read(b);
        val = b;
    }
    else if(count <= 32767)
    {
        Ice::Short sh;
        is->read(sh);
        val = sh;
    }
    else
    {
        is->read(val);
    }

    if(val < 0 || val >= count)
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
    if(!validate(zv TSRMLS_CC))
    {
        out << "<invalid value - expected " << id << ">";
        return;
    }
    int val = static_cast<int>(Z_LVAL_P(zv));
    out << enumerators[val];
}

//
// DataMember implementation.
//
void
IcePHP::DataMember::unmarshaled(zval* zv, zval* target, void* TSRMLS_DC)
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

//
// StructInfo implementation.
//
string
IcePHP::StructInfo::getId() const
{
    return id;
}

bool
IcePHP::StructInfo::validate(zval* zv TSRMLS_DC)
{
    if(Z_TYPE_P(zv) != IS_OBJECT)
    {
        string s = zendTypeToString(Z_TYPE_P(zv));
        invalidArgument("expected struct value of type %s but received %s" TSRMLS_CC, zce->name, s.c_str());
        return false;
    }

    //
    // Compare class entries.
    //
    zend_class_entry* ce = Z_OBJCE_P(zv);
    if(ce != zce)
    {
        invalidArgument("expected struct value of type %s but received %s" TSRMLS_CC, zce->name, ce->name);
        return false;
    }

    return true;
}

bool
IcePHP::StructInfo::usesClasses()
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
IcePHP::StructInfo::marshal(zval* zv, const Ice::OutputStreamPtr& os, ObjectMap* objectMap TSRMLS_DC)
{
    assert(Z_TYPE_P(zv) == IS_OBJECT); // validate() should have caught this.
    assert(Z_OBJCE_P(zv) == zce); // validate() should have caught this.

    for(DataMemberList::iterator q = members.begin(); q != members.end(); ++q)
    {
        DataMemberPtr member = *q;

        void* data;
        if(zend_hash_find(Z_OBJPROP_P(zv), STRCAST(member->name.c_str()), member->name.size() + 1, &data) == FAILURE)
        {
            runtimeError("member `%s' of %s is not defined" TSRMLS_CC, member->name.c_str(), id.c_str());
            throw AbortMarshaling();
        }

        zval** val = reinterpret_cast<zval**>(data);
        if(!member->type->validate(*val TSRMLS_CC))
        {
            invalidArgument("invalid value for %s member `%s'" TSRMLS_CC, id.c_str(), member->name.c_str());
            throw AbortMarshaling();
        }
        member->type->marshal(*val, os, objectMap TSRMLS_CC);
    }
}

void
IcePHP::StructInfo::unmarshal(const Ice::InputStreamPtr& is, const UnmarshalCallbackPtr& cb,
                              const CommunicatorInfoPtr& comm, zval* target, void* closure TSRMLS_DC)
{
    zval* zv;
    MAKE_STD_ZVAL(zv);
    AutoDestroy destroy(zv);

    if(object_init_ex(zv, zce) != SUCCESS)
    {
        runtimeError("unable to initialize object of type %s" TSRMLS_CC, zce->name);
        throw AbortMarshaling();
    }

    for(DataMemberList::iterator q = members.begin(); q != members.end(); ++q)
    {
        DataMemberPtr member = *q;
        member->type->unmarshal(is, member, comm, zv, 0 TSRMLS_CC);
    }

    cb->unmarshaled(zv, target, closure TSRMLS_CC);
}

void
IcePHP::StructInfo::print(zval* zv, IceUtilInternal::Output& out, PrintObjectHistory* history TSRMLS_DC)
{
    if(!validate(zv TSRMLS_CC))
    {
        out << "<invalid value - expected " << id << ">";
        return;
    }
    out.sb();
    for(DataMemberList::iterator q = members.begin(); q != members.end(); ++q)
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
    out.eb();
}

void
IcePHP::StructInfo::destroy()
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
IcePHP::SequenceInfo::getId() const
{
    return id;
}

bool
IcePHP::SequenceInfo::validate(zval* zv TSRMLS_DC)
{
    return Z_TYPE_P(zv) == IS_NULL || Z_TYPE_P(zv) == IS_ARRAY;
}

bool
IcePHP::SequenceInfo::usesClasses()
{
    return elementType->usesClasses();
}

void
IcePHP::SequenceInfo::marshal(zval* zv, const Ice::OutputStreamPtr& os, ObjectMap* objectMap TSRMLS_DC)
{
    if(Z_TYPE_P(zv) == IS_NULL)
    {
        os->writeSize(0);
        return;
    }

    assert(Z_TYPE_P(zv) == IS_ARRAY); // validate() should have caught this.

    HashTable* arr = Z_ARRVAL_P(zv);

    Ice::Int sz = static_cast<Ice::Int>(zend_hash_num_elements(arr));

    if(sz == 0)
    {
        os->writeSize(0);
        return;
    }

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
        if(!elementType->validate(*val TSRMLS_CC))
        {
            invalidArgument("invalid value for sequence element `%s'" TSRMLS_CC, id.c_str());
            throw AbortMarshaling();
        }
        elementType->marshal(*val, os, objectMap TSRMLS_CC);
        zend_hash_move_forward_ex(arr, &pos);
    }
}

void
IcePHP::SequenceInfo::unmarshal(const Ice::InputStreamPtr& is, const UnmarshalCallbackPtr& cb,
                                const CommunicatorInfoPtr& comm, zval* target, void* closure TSRMLS_DC)
{
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
        elementType->unmarshal(is, this, comm, zv, cl TSRMLS_CC);
    }

    cb->unmarshaled(zv, target, closure TSRMLS_CC);
}

void
IcePHP::SequenceInfo::print(zval* zv, IceUtilInternal::Output& out, PrintObjectHistory* history TSRMLS_DC)
{
    if(!validate(zv TSRMLS_CC))
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
        elementType = 0;
    }
}

void
IcePHP::SequenceInfo::marshalPrimitiveSequence(const PrimitiveInfoPtr& pi, zval* zv, const Ice::OutputStreamPtr& os
                                               TSRMLS_DC)
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
            if(!pi->validate(*val TSRMLS_CC))
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
            if(!pi->validate(*val TSRMLS_CC))
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
            if(!pi->validate(*val TSRMLS_CC))
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
            if(!pi->validate(*val TSRMLS_CC))
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
            if(!pi->validate(*val TSRMLS_CC))
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
            if(!pi->validate(*val TSRMLS_CC))
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
            if(!pi->validate(*val TSRMLS_CC))
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
            if(!pi->validate(*val TSRMLS_CC))
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
        os->write(seq, true);
        break;
    }
    }
}

void
IcePHP::SequenceInfo::unmarshalPrimitiveSequence(const PrimitiveInfoPtr& pi, const Ice::InputStreamPtr& is,
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
                ZVAL_STRINGL(val, STRCAST(str.c_str()), str.length(), 1);
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
            ZVAL_STRINGL(val, STRCAST(p->c_str()), p->length(), 1);
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
string
IcePHP::DictionaryInfo::getId() const
{
    return id;
}

bool
IcePHP::DictionaryInfo::validate(zval* zv TSRMLS_DC)
{
    return Z_TYPE_P(zv) == IS_NULL || Z_TYPE_P(zv) == IS_ARRAY;
}

bool
IcePHP::DictionaryInfo::usesClasses()
{
    return valueType->usesClasses();
}

void
IcePHP::DictionaryInfo::marshal(zval* zv, const Ice::OutputStreamPtr& os, ObjectMap* objectMap TSRMLS_DC)
{
    if(Z_TYPE_P(zv) == IS_NULL)
    {
        os->writeSize(0);
        return;
    }

    assert(Z_TYPE_P(zv) == IS_ARRAY); // validate() should have caught this.

    PrimitiveInfoPtr piKey = PrimitiveInfoPtr::dynamicCast(keyType);
    EnumInfoPtr enKey = EnumInfoPtr::dynamicCast(keyType);
    if(!enKey && (!piKey || piKey->kind == PrimitiveInfo::KindFloat || piKey->kind == PrimitiveInfo::KindDouble))
    {
        invalidArgument("dictionary type `%s' cannot be marshaled" TSRMLS_CC, id.c_str());
        throw AbortMarshaling();
    }

    HashTable* arr = Z_ARRVAL_P(zv);

    Ice::Int sz = static_cast<Ice::Int>(zend_hash_num_elements(arr));
    os->writeSize(sz);

    if(sz == 0)
    {
        return;
    }

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
        if(!keyType->validate(zkey TSRMLS_CC))
        {
            invalidArgument("invalid key in `%s' element" TSRMLS_CC, id.c_str());
            throw AbortMarshaling();
        }
        keyType->marshal(zkey, os, objectMap TSRMLS_CC);

        //
        // Marshal the value.
        //
        if(!valueType->validate(*val TSRMLS_CC))
        {
            invalidArgument("invalid value in `%s' element" TSRMLS_CC, id.c_str());
            throw AbortMarshaling();
        }
        valueType->marshal(*val, os, objectMap TSRMLS_CC);

        zend_hash_move_forward_ex(arr, &pos);
    }
}

void
IcePHP::DictionaryInfo::unmarshal(const Ice::InputStreamPtr& is, const UnmarshalCallbackPtr& cb,
                                  const CommunicatorInfoPtr& comm, zval* target, void* closure TSRMLS_DC)
{
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
        keyType->unmarshal(is, keyCB, comm, 0, 0 TSRMLS_CC);
        assert(keyCB->key);

        //
        // Allocate a callback that holds a reference to the key.
        //
        ValueCallbackPtr valueCB = new ValueCallback(keyCB->key TSRMLS_CC);

        //
        // Pass the key to the callback.
        //
        valueType->unmarshal(is, valueCB, comm, zv, 0 TSRMLS_CC);
    }

    cb->unmarshaled(zv, target, closure TSRMLS_CC);
}

void
IcePHP::DictionaryInfo::print(zval* zv, IceUtilInternal::Output& out, PrintObjectHistory* history TSRMLS_DC)
{
    if(!validate(zv TSRMLS_CC))
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
string
IcePHP::ClassInfo::getId() const
{
    return id;
}

bool
IcePHP::ClassInfo::validate(zval* val TSRMLS_DC)
{
    if(Z_TYPE_P(val) == IS_OBJECT)
    {
        return checkClass(Z_OBJCE_P(val), zce);
    }
    return Z_TYPE_P(val) == IS_NULL;
}

bool
IcePHP::ClassInfo::usesClasses()
{
    return true;
}

void
IcePHP::ClassInfo::marshal(zval* zv, const Ice::OutputStreamPtr& os, ObjectMap* objectMap TSRMLS_DC)
{
    if(!defined)
    {
        runtimeError("class or interface %s is declared but not defined" TSRMLS_CC, id.c_str());
        throw AbortMarshaling();
    }

    if(Z_TYPE_P(zv) == IS_NULL)
    {
        os->writeObject(0);
        return;
    }

    assert(Z_TYPE_P(zv) == IS_OBJECT); // validate() should have caught this.
    assert(checkClass(Z_OBJCE_P(zv), zce)); // validate() should have caught this.

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
    os->writeObject(writer);
}

void
IcePHP::ClassInfo::unmarshal(const Ice::InputStreamPtr& is, const UnmarshalCallbackPtr& cb,
                             const CommunicatorInfoPtr& comm, zval* target, void* closure TSRMLS_DC)
{
    if(!defined)
    {
        runtimeError("class or interface %s is declared but not defined" TSRMLS_CC, id.c_str());
        throw AbortMarshaling();
    }

    is->readObject(new ReadObjectCallback(this, cb, target, closure TSRMLS_CC));
}

void
IcePHP::ClassInfo::print(zval* zv, IceUtilInternal::Output& out, PrintObjectHistory* history TSRMLS_DC)
{
    if(!validate(zv TSRMLS_CC))
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
IcePHP::ClassInfo::printMembers(zval* zv, IceUtilInternal::Output& out, PrintObjectHistory* history TSRMLS_DC)
{
    if(base)
    {
        base->printMembers(zv, out, history TSRMLS_CC);
    }

    for(DataMemberList::iterator q = members.begin(); q != members.end(); ++q)
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
}

bool
IcePHP::ClassInfo::isA(const string& typeId) const
{
    if(id == typeId)
    {
        return true;
    }

    if(base && base->isA(typeId))
    {
        return true;
    }

    for(ClassInfoList::const_iterator p = interfaces.begin(); p != interfaces.end(); ++p)
    {
        if((*p)->isA(typeId))
        {
            return true;
        }
    }

    return false;
}

void
IcePHP::ClassInfo::addOperation(const string& name, const OperationPtr& op)
{
    operations.insert(OperationMap::value_type(Slice::PHP::fixIdent(name), op));
}

IcePHP::OperationPtr
IcePHP::ClassInfo::getOperation(const string& name) const
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
        for(ClassInfoList::const_iterator q = interfaces.begin(); q != interfaces.end() && !op; ++q)
        {
            op = (*q)->getOperation(name);
        }
    }
    return op;
}

//
// ProxyInfo implementation.
//
string
IcePHP::ProxyInfo::getId() const
{
    return id;
}

bool
IcePHP::ProxyInfo::validate(zval* zv TSRMLS_DC)
{
    if(Z_TYPE_P(zv) != IS_NULL)
    {
        if(Z_TYPE_P(zv) != IS_OBJECT || (Z_TYPE_P(zv) == IS_OBJECT && Z_OBJCE_P(zv) != proxyClassEntry))
        {
            string s = zendTypeToString(Z_TYPE_P(zv));
            invalidArgument("expected proxy value or null but received %s" TSRMLS_CC, s.c_str());
            return false;
        }
    }

    return true;
}

void
IcePHP::ProxyInfo::marshal(zval* zv, const Ice::OutputStreamPtr& os, ObjectMap* TSRMLS_DC)
{
    if(Z_TYPE_P(zv) == IS_NULL)
    {
        os->write(Ice::ObjectPrx());
    }
    else
    {
        assert(Z_TYPE_P(zv) == IS_OBJECT && Z_OBJCE_P(zv) == proxyClassEntry); // validate() should have caught this.
        Ice::ObjectPrx proxy;
        ClassInfoPtr info;
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
}

void
IcePHP::ProxyInfo::unmarshal(const Ice::InputStreamPtr& is, const UnmarshalCallbackPtr& cb,
                             const CommunicatorInfoPtr& comm, zval* target, void* closure TSRMLS_DC)
{
    zval* zv;
    MAKE_STD_ZVAL(zv);
    AutoDestroy destroy(zv);

    Ice::ObjectPrx proxy;
    is->read(proxy);

    if(!proxy)
    {
        ZVAL_NULL(zv);
        cb->unmarshaled(zv, target, closure TSRMLS_CC);
        return;
    }

    if(!cls->defined)
    {
        runtimeError("class or interface %s is declared but not defined" TSRMLS_CC, id.c_str());
        throw AbortMarshaling();
    }

    if(!createProxy(zv, proxy, cls, comm TSRMLS_CC))
    {
        throw AbortMarshaling();
    }
    cb->unmarshaled(zv, target, closure TSRMLS_CC);
}

void
IcePHP::ProxyInfo::print(zval* zv, IceUtilInternal::Output& out, PrintObjectHistory* TSRMLS_DC)
{
    if(!validate(zv TSRMLS_CC))
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
        ClassInfoPtr cls;
        if(!fetchProxy(zv, proxy, cls TSRMLS_CC))
        {
            return;
        }
        out << proxy->ice_toString();
    }
}

void
IcePHP::ProxyInfo::destroy()
{
    cls = 0;
}

//
// ObjectWriter implementation.
//
IcePHP::ObjectWriter::ObjectWriter(zval* object, ObjectMap* objectMap, const ClassInfoPtr& formal TSRMLS_DC) :
    _object(object), _map(objectMap)
{
#ifdef ZTS
    this->TSRMLS_C = TSRMLS_C;
#endif

    Z_ADDREF_P(_object);

    //
    // We need to determine the most-derived Slice type supported by this object.
    // This is typically a Slice class, but it can also be an interface.
    //
    // The caller may have provided a ClassInfo representing the formal type, in
    // which case we ensure that the actual type is compatible with the formal type.
    //
    _info = getClassInfoByClass(Z_OBJCE_P(object), formal ? formal->zce : 0 TSRMLS_CC);
    assert(_info);
}

IcePHP::ObjectWriter::~ObjectWriter()
{
    zval_ptr_dtor(&_object);
}

void
IcePHP::ObjectWriter::ice_preMarshal()
{
    string name = "ice_premarshal"; // Must be lowercase.
    if(zend_hash_exists(&Z_OBJCE_P(_object)->function_table, STRCAST(name.c_str()), name.size() + 1))
    {
        if(!invokeMethod(_object, name TSRMLS_CC))
        {
            throw AbortMarshaling();
        }
    }
}

void
IcePHP::ObjectWriter::write(const Ice::OutputStreamPtr& os) const
{
    Ice::SlicedDataPtr slicedData;

    if(_info->preserve)
    {
        //
        // Retrieve the SlicedData object that we stored as a hidden member of the PHP object.
        //
        slicedData = SlicedDataUtil::getMember(_object, const_cast<ObjectMap*>(_map) TSRMLS_CC);
    }

    os->startObject(slicedData);

    ClassInfoPtr info = _info;
    while(info && info->id != Ice::Object::ice_staticId())
    {
        assert(info->base); // All classes have the Ice::Object base type.
        const bool lastSlice = info->base->id == Ice::Object::ice_staticId();
        os->startSlice(info->id, lastSlice);
        for(DataMemberList::iterator q = info->members.begin(); q != info->members.end(); ++q)
        {
            DataMemberPtr member = *q;

            void* data;
            if(zend_hash_find(Z_OBJPROP_P(_object), STRCAST(member->name.c_str()), member->name.size() + 1, &data) ==
                FAILURE)
            {
                runtimeError("member `%s' of %s is not defined" TSRMLS_CC, member->name.c_str(), _info->id.c_str());
                throw AbortMarshaling();
            }

            zval** val = reinterpret_cast<zval**>(data);
            if(!member->type->validate(*val TSRMLS_CC))
            {
                invalidArgument("invalid value for %s member `%s'" TSRMLS_CC, _info->id.c_str(), member->name.c_str());
                throw AbortMarshaling();
            }
            member->type->marshal(*val, os, _map TSRMLS_CC);
        }
        os->endSlice();

        info = info->base;
    }

    os->endObject();
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
    if(zend_hash_exists(&Z_OBJCE_P(_object)->function_table, STRCAST(name.c_str()), name.size() + 1))
    {
        if(!invokeMethod(_object, name TSRMLS_CC))
        {
            throw AbortMarshaling();
        }
    }
}

void
IcePHP::ObjectReader::read(const Ice::InputStreamPtr& is)
{
    is->startObject();

    //
    // Unmarshal the slices of a user-defined class.
    //
    ClassInfoPtr info = _info;
    while(info && info->id != Ice::Object::ice_staticId())
    {
        is->startSlice();
        for(DataMemberList::iterator p = info->members.begin(); p != info->members.end(); ++p)
        {
            DataMemberPtr member = *p;
            member->type->unmarshal(is, member, _communicator, _object, 0 TSRMLS_CC);
        }
        is->endSlice();

        info = info->base;
    }

    _slicedData = is->endObject(_info->preserve);

    if(_slicedData)
    {
        SlicedDataUtil* util = reinterpret_cast<SlicedDataUtil*>(is->closure());
        assert(util);
        util->add(this);
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
        if(!reader->getInfo()->isA(_info->id))
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
IcePHP::ExceptionInfo::unmarshal(const Ice::InputStreamPtr& is, const CommunicatorInfoPtr& comm TSRMLS_DC)
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
        for(DataMemberList::iterator q = info->members.begin(); q != info->members.end(); ++q)
        {
            DataMemberPtr member = *q;
            member->type->unmarshal(is, member, comm, zv, 0 TSRMLS_CC);
        }
        is->endSlice();

        info = info->base;
    }

    return destroy.release();
}

void
IcePHP::ExceptionInfo::print(zval* zv, IceUtilInternal::Output& out TSRMLS_DC)
{
    if(Z_TYPE_P(zv) != IS_OBJECT)
    {
        string s = zendTypeToString(Z_TYPE_P(zv));
        invalidArgument("expected exception value of type %s but received %s" TSRMLS_CC, zce->name, s.c_str());
        return;
    }

    //
    // Compare class entries.
    //
    zend_class_entry* ce = Z_OBJCE_P(zv);
    if(ce != zce)
    {
        invalidArgument("expected exception value of type %s but received %s" TSRMLS_CC, zce->name, ce->name);
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

    for(DataMemberList::iterator q = members.begin(); q != members.end(); ++q)
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
    Ice::UserExceptionReader(communicatorInfo->getCommunicator()), _communicatorInfo(communicatorInfo), _info(info)
{
#ifdef ZTS
    this->TSRMLS_C = TSRMLS_C;
#endif
}

IcePHP::ExceptionReader::~ExceptionReader()
    throw()
{
}

void
IcePHP::ExceptionReader::read(const Ice::InputStreamPtr& is) const
{
    is->startException();

    const_cast<zval*&>(_ex) = _info->unmarshal(is, _communicatorInfo TSRMLS_CC);

    const_cast<Ice::SlicedDataPtr&>(_slicedData) = is->endException(_info->preserve);
}

bool
IcePHP::ExceptionReader::usesClasses() const
{
    return _info->usesClasses;
}

void
IcePHP::ExceptionReader::usesClasses(bool)
{
}

string
IcePHP::ExceptionReader::ice_name() const
{
    return _info->id;
}

Ice::Exception*
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

    EnumInfoPtr type = new EnumInfo();
    type->id = id;
    HashTable* arr = Z_ARRVAL_P(enumerators);
    void* data;
    HashPosition pos;
    zend_hash_internal_pointer_reset_ex(arr, &pos);
    while(zend_hash_get_current_data_ex(arr, &data, &pos) != FAILURE)
    {
        zval** val = reinterpret_cast<zval**>(data);
        assert(Z_TYPE_PP(val) == IS_STRING);
        type->enumerators.push_back(Z_STRVAL_PP(val));
        zend_hash_move_forward_ex(arr, &pos);
    }

    if(!createTypeInfo(return_value, type TSRMLS_CC))
    {
        RETURN_NULL();
    }
}

static void
convertDataMembers(zval* zv, DataMemberList& l TSRMLS_DC)
{
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
        assert(zend_hash_num_elements(member) == 2);
        zend_hash_index_find(member, 0, reinterpret_cast<void**>(&elem));
        assert(Z_TYPE_PP(elem) == IS_STRING);
        m->name = Z_STRVAL_PP(elem);
        zend_hash_index_find(member, 1, reinterpret_cast<void**>(&elem));
        assert(Z_TYPE_PP(elem) == IS_OBJECT);
        m->type = Wrapper<TypeInfoPtr>::value(*elem TSRMLS_CC);
        l.push_back(m);

        zend_hash_move_forward_ex(membersArray, &pos);
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

    StructInfoPtr type = new StructInfo();
    type->id = id;
    type->name = name;
    convertDataMembers(members, type->members TSRMLS_CC);
    type->zce = nameToClass(type->name TSRMLS_CC);
    assert(type->zce);

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

    SequenceInfoPtr type = new SequenceInfo();
    type->id = id;
    type->elementType = Wrapper<TypeInfoPtr>::value(element TSRMLS_CC);

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

    DictionaryInfoPtr type = new DictionaryInfo();
    type->id = id;
    type->keyType = Wrapper<TypeInfoPtr>::value(key TSRMLS_CC);
    type->valueType = Wrapper<TypeInfoPtr>::value(value TSRMLS_CC);

    if(!createTypeInfo(return_value, type TSRMLS_CC))
    {
        RETURN_NULL();
    }
}

ZEND_FUNCTION(IcePHP_defineProxy)
{
    zval* cls;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, const_cast<char*>("o"), &cls) == FAILURE)
    {
        return;
    }

    TypeInfoPtr p = Wrapper<TypeInfoPtr>::value(cls TSRMLS_CC);
    ClassInfoPtr c = ClassInfoPtr::dynamicCast(p);
    assert(c);

    ProxyInfoPtr type = getProxyInfo(c->id TSRMLS_CC);
    if(!type)
    {
        type = new ProxyInfo();
        type->id = c->id;

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
        m->insert(ProxyInfoMap::value_type(type->id, type));
    }

    type->cls = c;

    if(!createTypeInfo(return_value, type TSRMLS_CC))
    {
        RETURN_NULL();
    }
}

ZEND_FUNCTION(IcePHP_declareClass)
{
    if(ZEND_NUM_ARGS() != 1)
    {
        WRONG_PARAM_COUNT;
    }

    char* id;
    int idLen;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, const_cast<char*>("s"), &id, &idLen) == FAILURE)
    {
        return;
    }

    ClassInfoPtr type = new ClassInfo();
    type->id = id;
    type->defined = false;

    addClassInfoById(type TSRMLS_CC);

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
    zend_bool isAbstract;
    zend_bool preserve;
    zval* base;
    zval* interfaces;
    zval* members;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, const_cast<char*>("ssbbo!a!a!"), &id, &idLen, &name, &nameLen,
                             &isAbstract, &preserve, &base, &interfaces, &members) == FAILURE)
    {
        return;
    }

    ClassInfoPtr type = getClassInfoById(id TSRMLS_CC);
    if(!type)
    {
        type = new ClassInfo();
        type->id = id;
        addClassInfoById(type TSRMLS_CC);
    }

    type->name = name;
    addClassInfoByName(type TSRMLS_CC);

    type->isAbstract = isAbstract ? true : false;
    type->preserve = preserve ? true : false;
    if(base)
    {
        TypeInfoPtr p = Wrapper<TypeInfoPtr>::value(base TSRMLS_CC);
        type->base = ClassInfoPtr::dynamicCast(p);
        assert(type->base);
    }

    if(interfaces)
    {
        HashTable* interfacesArray = Z_ARRVAL_P(interfaces);
        void* data;
        HashPosition pos;
        zend_hash_internal_pointer_reset_ex(interfacesArray, &pos);
        while(zend_hash_get_current_data_ex(interfacesArray, &data, &pos) != FAILURE)
        {
            zval** interfaceType = reinterpret_cast<zval**>(data);
            TypeInfoPtr t = Wrapper<TypeInfoPtr>::value(*interfaceType TSRMLS_CC);
            ClassInfoPtr c = ClassInfoPtr::dynamicCast(t);
            assert(c);
            type->interfaces.push_back(c);
            zend_hash_move_forward_ex(interfacesArray, &pos);
        }
    }

    if(members)
    {
        convertDataMembers(members, type->members TSRMLS_CC);
    }

    type->defined = true;
    type->zce = nameToClass(type->name TSRMLS_CC);
    assert(type->zce || type->id == "::Ice::LocalObject"); // LocalObject does not have a native PHP equivalent.

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
        convertDataMembers(members, ex->members TSRMLS_CC);
    }

    ex->usesClasses = false;
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
    RETURN_STRINGL(STRCAST(str.c_str()), str.length(), 1);
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
    RETURN_STRINGL(STRCAST(str.c_str()), str.length(), 1);
}

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
IcePHP::typesInit(TSRMLS_D)
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

    return true;
}

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

    return true;
}
