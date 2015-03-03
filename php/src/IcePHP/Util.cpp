// **********************************************************************
//
// Copyright (c) 2003-2011 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Util.h>
#include <IceUtil/UUID.h>
#include <Slice/PHPUtil.h>
#include <algorithm>
#include <ctype.h>

using namespace std;
using namespace IcePHP;
using namespace Slice::PHP;

#ifdef _WIN32
extern "C"
#endif
static void
dtor_wrapper(void* p)
{
    zval_ptr_dtor(static_cast<zval**>(p));
}

void*
IcePHP::createWrapper(zend_class_entry* ce, size_t sz TSRMLS_DC)
{
    zend_object* obj;
    zval* tmp;

    obj = static_cast<zend_object*>(emalloc(sz));
    obj->ce = ce;
    obj->guards = 0;

    obj->properties = static_cast<HashTable*>(emalloc(sizeof(HashTable)));
    zend_hash_init(obj->properties, 0, 0, dtor_wrapper, 0);
    zend_hash_copy(obj->properties, &ce->default_properties, (copy_ctor_func_t)zval_add_ref, &tmp, sizeof(zval*));

    return obj;
}

void*
IcePHP::extractWrapper(zval* zv TSRMLS_DC)
{
    if(!zv)
    {
        runtimeError("method %s() must be invoked on an object" TSRMLS_CC, get_active_function_name(TSRMLS_C));
        return 0;
    }

    zend_object* obj = static_cast<zend_object*>(zend_object_store_get_object(zv TSRMLS_CC));
    if(!obj)
    {
        runtimeError("no object found in %s()" TSRMLS_CC, get_active_function_name(TSRMLS_C));
        return 0;
    }

    return obj;
}

zend_class_entry*
IcePHP::idToClass(const string& id TSRMLS_DC)
{
#ifdef ICEPHP_USE_NAMESPACES
    string cls = scopedToName(id, true);
#else
    string cls = scopedToName(id, false);
#endif

    return nameToClass(cls TSRMLS_CC);
}

zend_class_entry*
IcePHP::nameToClass(const string& name TSRMLS_DC)
{
    zend_class_entry** result;
    if(zend_lookup_class(STRCAST(name.c_str()), name.length(), &result TSRMLS_CC) == FAILURE)
    {
        return 0;
    }
    return *result;
}

bool
IcePHP::createIdentity(zval* zv, const Ice::Identity& id TSRMLS_DC)
{
    zend_class_entry* cls = idToClass("::Ice::Identity" TSRMLS_CC);
    assert(cls);

    if(object_init_ex(zv, cls) != SUCCESS)
    {
        runtimeError("unable to initialize Ice::Identity" TSRMLS_CC);
        return false;
    }

    zend_update_property_string(cls, zv, const_cast<char*>("name"), sizeof("name") - 1, STRCAST(id.name.c_str())
                                TSRMLS_CC);
    zend_update_property_string(cls, zv, const_cast<char*>("category"), sizeof("category") - 1,
                                STRCAST(id.category.c_str()) TSRMLS_CC);

    return true;
}

bool
IcePHP::extractIdentity(zval* zv, Ice::Identity& id TSRMLS_DC)
{
    if(Z_TYPE_P(zv) != IS_OBJECT)
    {
        invalidArgument("value does not contain an object" TSRMLS_CC);
        return false;
    }

    zend_class_entry* cls = idToClass("::Ice::Identity" TSRMLS_CC);
    assert(cls);

    zend_class_entry* ce = Z_OBJCE_P(zv);
    if(ce != cls)
    {
        invalidArgument("expected an identity but received %s" TSRMLS_CC, ce->name);
        return false;
    }

    //
    // Category is optional, but name is required.
    //
    void* categoryData = 0;
    void* nameData;
    if(zend_hash_find(Z_OBJPROP_P(zv), STRCAST("name"), sizeof("name"), &nameData) == FAILURE)
    {
        invalidArgument("identity value does not contain member `name'" TSRMLS_CC);
        return false;
    }
    zend_hash_find(Z_OBJPROP_P(zv), STRCAST("category"), sizeof("category"), &categoryData);
    zval** categoryVal = reinterpret_cast<zval**>(categoryData);
    zval** nameVal = reinterpret_cast<zval**>(nameData);

    if(Z_TYPE_PP(nameVal) != IS_STRING)
    {
        string s = zendTypeToString(Z_TYPE_PP(nameVal));
        invalidArgument("expected a string value for identity member `name' but received %s" TSRMLS_CC, s.c_str());
        return false;
    }

    if(categoryVal && Z_TYPE_PP(categoryVal) != IS_STRING && Z_TYPE_PP(categoryVal) != IS_NULL)
    {
        string s = zendTypeToString(Z_TYPE_PP(categoryVal));
        invalidArgument("expected a string value for identity member `category' but received %s" TSRMLS_CC, s.c_str());
        return false;
    }

    id.name = Z_STRVAL_PP(nameVal);
    if(categoryVal && Z_TYPE_PP(categoryVal) == IS_STRING)
    {
        id.category = Z_STRVAL_PP(categoryVal);
    }
    else
    {
        id.category = "";
    }

    return true;
}

bool
IcePHP::createStringMap(zval* zv, const map<string, string>& ctx TSRMLS_DC)
{
    array_init(zv);
    for(map<string, string>::const_iterator p = ctx.begin(); p != ctx.end(); ++p)
    {
        if(add_assoc_stringl_ex(zv, const_cast<char*>(p->first.c_str()), p->first.length() + 1,
                                const_cast<char*>(p->second.c_str()), p->second.length(), 1) == FAILURE)
        {
            return false;
        }
    }

    return true;
}

bool
IcePHP::extractStringMap(zval* zv, map<string, string>& ctx TSRMLS_DC)
{
    if(Z_TYPE_P(zv) != IS_ARRAY)
    {
        string s = zendTypeToString(Z_TYPE_P(zv));
        invalidArgument("expected an associative array but received %s" TSRMLS_CC, s.c_str());
        return false;
    }

    HashTable* arr = Z_ARRVAL_P(zv);
    void* data;
    HashPosition pos;

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
        int keyType = zend_hash_get_current_key_ex(arr, &keyStr, &keyLen, &keyNum, 0, &pos);

        //
        // Store the key in a zval, so that we can reuse the PrimitiveMarshaler logic.
        //
        if(keyType != HASH_KEY_IS_STRING)
        {
            invalidArgument("array key must be a string" TSRMLS_CC);
            return false;
        }

        if(Z_TYPE_PP(val) != IS_STRING)
        {
            invalidArgument("array value must be a string" TSRMLS_CC);
            return false;
        }

        ctx[keyStr] = Z_STRVAL_PP(val);

        zend_hash_move_forward_ex(arr, &pos);
    }

    return true;
}

bool
IcePHP::createStringArray(zval* zv, const Ice::StringSeq& seq TSRMLS_DC)
{
    array_init(zv);
    for(Ice::StringSeq::const_iterator p = seq.begin(); p != seq.end(); ++p)
    {
        if(add_next_index_stringl(zv, STRCAST(p->c_str()), p->length(), 1) == FAILURE)
        {
            return false;
        }
    }

    return true;
}

bool
IcePHP::extractStringArray(zval* zv, Ice::StringSeq& seq TSRMLS_DC)
{
    if(Z_TYPE_P(zv) != IS_ARRAY)
    {
        string s = zendTypeToString(Z_TYPE_P(zv));
        invalidArgument("expected an array of strings but received %s" TSRMLS_CC, s.c_str());
        return false;
    }

    HashTable* arr = Z_ARRVAL_P(zv);
    void* data;
    HashPosition pos;

    zend_hash_internal_pointer_reset_ex(arr, &pos);
    while(zend_hash_get_current_data_ex(arr, &data, &pos) != FAILURE)
    {
        zval** val = reinterpret_cast<zval**>(data);

        if(Z_TYPE_PP(val) != IS_STRING)
        {
            invalidArgument("array element must be a string" TSRMLS_CC);
            return false;
        }

        string s(Z_STRVAL_PP(val), Z_STRLEN_PP(val));
        seq.push_back(s);

        zend_hash_move_forward_ex(arr, &pos);
    }

    return true;
}

static void
setStringMember(zval* obj, const string& name, const string& val TSRMLS_DC)
{
    zend_class_entry* cls = Z_OBJCE_P(obj);
    assert(cls);
    zend_update_property_stringl(cls, obj, const_cast<char*>(name.c_str()), static_cast<int>(name.size()),
                                 const_cast<char*>(val.c_str()), static_cast<int>(val.size()) TSRMLS_CC);
}

static bool
convertLocalException(const Ice::LocalException& ex, zval* zex TSRMLS_DC)
{
    zend_class_entry* cls = Z_OBJCE_P(zex);
    assert(cls);

    //
    // Transfer data members from Ice exception to PHP object.
    //
    try
    {
        ex.ice_throw();
    }
    catch(const Ice::InitializationException& e)
    {
        setStringMember(zex, "reason", e.reason TSRMLS_CC);
    }
    catch(const Ice::PluginInitializationException& e)
    {
        setStringMember(zex, "reason", e.reason TSRMLS_CC);
    }
    catch(const Ice::AlreadyRegisteredException& e)
    {
        setStringMember(zex, "kindOfObject", e.kindOfObject TSRMLS_CC);
        setStringMember(zex, "id", e.id TSRMLS_CC);
    }
    catch(const Ice::NotRegisteredException& e)
    {
        setStringMember(zex, "kindOfObject", e.kindOfObject TSRMLS_CC);
        setStringMember(zex, "id", e.id TSRMLS_CC);
    }
    catch(const Ice::TwowayOnlyException& e)
    {
        setStringMember(zex, "operation", e.operation TSRMLS_CC);
    }
    catch(const Ice::UnknownException& e)
    {
        setStringMember(zex, "unknown", e.unknown TSRMLS_CC);
    }
    catch(const Ice::ObjectAdapterDeactivatedException& e)
    {
        setStringMember(zex, "name", e.name TSRMLS_CC);
    }
    catch(const Ice::ObjectAdapterIdInUseException& e)
    {
        setStringMember(zex, "id", e.id TSRMLS_CC);
    }
    catch(const Ice::NoEndpointException& e)
    {
        setStringMember(zex, "proxy", e.proxy TSRMLS_CC);
    }
    catch(const Ice::EndpointParseException& e)
    {
        setStringMember(zex, "str", e.str TSRMLS_CC);
    }
    catch(const Ice::IdentityParseException& e)
    {
        setStringMember(zex, "str", e.str TSRMLS_CC);
    }
    catch(const Ice::ProxyParseException& e)
    {
        setStringMember(zex, "str", e.str TSRMLS_CC);
    }
    catch(const Ice::IllegalIdentityException& e)
    {
        zval* id;
        MAKE_STD_ZVAL(id);
        if(!createIdentity(id, e.id TSRMLS_CC))
        {
            zval_ptr_dtor(&id);
            return false;
        }
        zend_update_property(cls, zex, const_cast<char*>("id"), sizeof("id") - 1, id TSRMLS_CC);
    }
    catch(const Ice::RequestFailedException& e)
    {
        zval* id;
        MAKE_STD_ZVAL(id);
        if(!createIdentity(id, e.id TSRMLS_CC))
        {
            zval_ptr_dtor(&id);
            return false;
        }
        zend_update_property(cls, zex, const_cast<char*>("id"), sizeof("id") - 1, id TSRMLS_CC);
        setStringMember(zex, "facet", e.facet TSRMLS_CC);
        setStringMember(zex, "operation", e.operation TSRMLS_CC);
    }
    catch(const Ice::FileException& e)
    {
        zend_update_property_long(cls, zex, const_cast<char*>("error"), sizeof("error") - 1, e.error TSRMLS_CC);
        setStringMember(zex, "path", e.path TSRMLS_CC);
    }
    catch(const Ice::SyscallException& e) // This must appear after all subclasses of SyscallException.
    {
        zend_update_property_long(cls, zex, const_cast<char*>("error"), sizeof("error") - 1, e.error TSRMLS_CC);
    }
    catch(const Ice::DNSException& e)
    {
        zend_update_property_long(cls, zex, const_cast<char*>("error"), sizeof("error") - 1, e.error TSRMLS_CC);
        setStringMember(zex, "host", e.host TSRMLS_CC);
    }
    catch(const Ice::UnsupportedProtocolException& e)
    {
        zend_update_property_long(cls, zex, const_cast<char*>("badMajor"), sizeof("badMajor") - 1, e.badMajor
                                  TSRMLS_CC);
        zend_update_property_long(cls, zex, const_cast<char*>("badMinor"), sizeof("badMinor") - 1, e.badMinor
                                  TSRMLS_CC);
        zend_update_property_long(cls, zex, const_cast<char*>("major"), sizeof("major") - 1, e.major TSRMLS_CC);
        zend_update_property_long(cls, zex, const_cast<char*>("minor"), sizeof("minor") - 1, e.minor TSRMLS_CC);
    }
    catch(const Ice::UnsupportedEncodingException& e)
    {
        zend_update_property_long(cls, zex, const_cast<char*>("badMajor"), sizeof("badMajor") - 1, e.badMajor
                                  TSRMLS_CC);
        zend_update_property_long(cls, zex, const_cast<char*>("badMinor"), sizeof("badMinor") - 1, e.badMinor
                                  TSRMLS_CC);
        zend_update_property_long(cls, zex, const_cast<char*>("major"), sizeof("major") - 1, e.major TSRMLS_CC);
        zend_update_property_long(cls, zex, const_cast<char*>("minor"), sizeof("minor") - 1, e.minor TSRMLS_CC);
    }
    catch(const Ice::NoObjectFactoryException& e)
    {
        setStringMember(zex, "reason", e.reason TSRMLS_CC);
        setStringMember(zex, "type", e.type TSRMLS_CC);
    }
    catch(const Ice::UnexpectedObjectException& e)
    {
        setStringMember(zex, "reason", e.reason TSRMLS_CC);
        setStringMember(zex, "type", e.type TSRMLS_CC);
        setStringMember(zex, "expectedType", e.expectedType TSRMLS_CC);
    }
    catch(const Ice::ProtocolException& e) // This must appear after all subclasses of ProtocolException.
    {
        setStringMember(zex, "reason", e.reason TSRMLS_CC);
    }
    catch(const Ice::FeatureNotSupportedException& e)
    {
        setStringMember(zex, "unsupportedFeature", e.unsupportedFeature TSRMLS_CC);
    }
    catch(const Ice::SecurityException& e)
    {
        setStringMember(zex, "reason", e.reason TSRMLS_CC);
    }
    catch(const Ice::LocalException&)
    {
        //
        // Nothing to do.
        //
    }

    return true;
}

zval*
IcePHP::convertException(const Ice::Exception& ex TSRMLS_DC)
{
    zval* zex;
    MAKE_STD_ZVAL(zex);
    AutoDestroy destroy(zex);

    ostringstream ostr;
    ostr << ex;
    string str = ostr.str();

    try
    {
        ex.ice_throw();
    }
    catch(const Ice::LocalException& e)
    {
        zend_class_entry* cls = idToClass(e.ice_name() TSRMLS_CC);
        if(cls)
        {
            if(object_init_ex(zex, cls) != SUCCESS)
            {
                runtimeError("unable to create exception %s" TSRMLS_CC, cls->name);
                return 0;
            }
            if(!convertLocalException(e, zex TSRMLS_CC))
            {
                return 0;
            }
        }
        else
        {
            cls = idToClass("Ice::UnknownLocalException" TSRMLS_CC);
            assert(cls);
            if(object_init_ex(zex, cls) != SUCCESS)
            {
                runtimeError("unable to create exception %s" TSRMLS_CC, cls->name);
                return 0;
            }
            setStringMember(zex, "unknown", str TSRMLS_CC);
        }
    }
    catch(const Ice::UserException&)
    {
        zend_class_entry* cls = idToClass("Ice::UnknownUserException" TSRMLS_CC);
        assert(cls);
        if(object_init_ex(zex, cls) != SUCCESS)
        {
            runtimeError("unable to create exception %s" TSRMLS_CC, cls->name);
            return 0;
        }
        setStringMember(zex, "unknown", str TSRMLS_CC);
    }
    catch(const Ice::Exception&)
    {
        zend_class_entry* cls = idToClass("Ice::UnknownException" TSRMLS_CC);
        assert(cls);
        if(object_init_ex(zex, cls) != SUCCESS)
        {
            runtimeError("unable to create exception %s" TSRMLS_CC, cls->name);
            return 0;
        }
        setStringMember(zex, "unknown", str TSRMLS_CC);
    }

    return destroy.release();
}

void
IcePHP::throwException(const Ice::Exception& ex TSRMLS_DC)
{
    zval* zex = convertException(ex TSRMLS_CC);
    if(zex)
    {
        zend_throw_exception_object(zex TSRMLS_CC);
    }
}

std::string
IcePHP::zendTypeToString(int type)
{
    string result;

    switch(type)
    {
    case IS_NULL:
        result = "null";
        break;

    case IS_LONG:
        result = "long";
        break;

    case IS_DOUBLE:
        result = "double";
        break;

    case IS_STRING:
        result = "string";
        break;

    case IS_ARRAY:
        result = "array";
        break;

    case IS_OBJECT:
        result = "object";
        break;

    case IS_BOOL:
        result = "bool";
        break;

    default:
        result = "unknown";
        break;
    }

    return result;
}

static void
throwError(const string& name, const string& msg TSRMLS_DC)
{
    zval* ex;
    MAKE_STD_ZVAL(ex);
    AutoDestroy destroy(ex);

    zend_class_entry* cls;
    {
        zend_class_entry** p;
        if(zend_lookup_class(STRCAST(name.c_str()), name.size(), &p TSRMLS_CC) == FAILURE)
        {
            assert(false);
        }
        cls = *p;
    }
    if(object_init_ex(ex, cls) == FAILURE)
    {
        assert(false);
    }

    //
    // Invoke constructor.
    //
    if(!invokeMethod(ex, ZEND_CONSTRUCTOR_FUNC_NAME, msg TSRMLS_CC))
    {
        assert(false);
    }

    zend_throw_exception_object(ex TSRMLS_CC);
    destroy.release();
}

void
IcePHP::runtimeError(const char* fmt TSRMLS_DC, ...)
{
    va_list args;
    char msg[1024];

#if ZTS
    va_start(args, TSRMLS_C);
#else
    va_start(args, fmt);
#endif

#if defined(_MSC_VER) && (_MSC_VER >= 1500)
    vsprintf_s(msg, fmt, args);
#else
    vsprintf(msg, fmt, args);
#endif

    va_end(args);

    throwError("RuntimeException", msg TSRMLS_CC);
}

void
IcePHP::invalidArgument(const char* fmt TSRMLS_DC, ...)
{
    va_list args;
    char msg[1024];

#if ZTS
    va_start(args, TSRMLS_C);
#else
    va_start(args, fmt);
#endif

#if defined(_MSC_VER) && (_MSC_VER >= 1500)
    vsprintf_s(msg, fmt, args);
#else
    vsprintf(msg, fmt, args);
#endif

    va_end(args);

    throwError("InvalidArgumentException", msg TSRMLS_CC);
}

static bool
invokeMethodHelper(zval* obj, const string& name, zval* param TSRMLS_DC)
{
    assert(zend_hash_exists(&Z_OBJCE_P(obj)->function_table, STRCAST(name.c_str()), name.size() + 1));
    zval ret, method;
    INIT_ZVAL(ret);
    INIT_ZVAL(method);
    ZVAL_STRING(&method, STRCAST(name.c_str()), 1);
    zend_uint numParams = param ? 1 : 0;
    zval** params = param ? &param : 0;
    int status = 0;
    zend_try
    {
        status = call_user_function(0, &obj, &method, &ret, numParams, params TSRMLS_CC);
    }
    zend_catch
    {
        status = FAILURE;
    }
    zend_end_try();
    zval_dtor(&method);
    zval_dtor(&ret);
    if(status == FAILURE || EG(exception))
    {
        return false;
    }
    return true;
}

bool
IcePHP::invokeMethod(zval* obj, const string& name TSRMLS_DC)
{
    return invokeMethodHelper(obj, name, 0 TSRMLS_CC);
}

bool
IcePHP::invokeMethod(zval* obj, const string& name, const string& arg TSRMLS_DC)
{
    zval* param;
    MAKE_STD_ZVAL(param);
    ZVAL_STRINGL(param, STRCAST(arg.c_str()), arg.size(), 1);
    AutoDestroy destroy(param);
    return invokeMethodHelper(obj, name, param TSRMLS_CC);
}

bool
IcePHP::checkClass(zend_class_entry* ce, zend_class_entry* base)
{
    while(ce)
    {
        if(ce == base)
        {
            return true;
        }

        for(zend_uint i = 0; i < ce->num_interfaces; ++i)
        {
            if(checkClass(ce->interfaces[i], base))
            {
                return true;
            }
        }

        ce = ce->parent;
    }

    return false;
}

ZEND_FUNCTION(Ice_stringVersion)
{
    if(ZEND_NUM_ARGS() > 0)
    {
        WRONG_PARAM_COUNT;
    }

    RETURN_STRINGL(STRCAST(ICE_STRING_VERSION), strlen(ICE_STRING_VERSION), 1);
}

ZEND_FUNCTION(Ice_intVersion)
{
    if(ZEND_NUM_ARGS() > 0)
    {
        WRONG_PARAM_COUNT;
    }

    RETURN_LONG(ICE_INT_VERSION);
}

ZEND_FUNCTION(Ice_generateUUID)
{
    if(ZEND_NUM_ARGS() > 0)
    {
        WRONG_PARAM_COUNT;
    }

    string uuid = IceUtil::generateUUID();
    RETURN_STRINGL(STRCAST(uuid.c_str()), uuid.size(), 1);
}
