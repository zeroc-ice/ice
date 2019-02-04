//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Util.h>
#include <Ice/UUID.h>
#include <Slice/PHPUtil.h>
#include <algorithm>
#include <ctype.h>

using namespace std;
using namespace IcePHP;
using namespace Slice::PHP;

namespace
{

bool
getMember(zval* zv, const string& name, zval* member, int type, bool required)
{
    zval* val = zend_hash_str_find(Z_OBJPROP_P(zv), STRCAST(name.c_str()), name.size());
    if(!val)
    {
        if(required)
        {
            invalidArgument("object does not contain member `%s'", name.c_str());
            return false;
        }
    }

    if(val)
    {
        assert(Z_TYPE_P(val) == IS_INDIRECT);
        val = Z_INDIRECT_P(val);
        if(Z_TYPE_P(val) != type)
        {
            string expected = zendTypeToString(type);
            string actual = zendTypeToString(Z_TYPE_P(val));
            invalidArgument("expected value of type %s for member `%s' but received %s", expected.c_str(),
                            name.c_str(), actual.c_str());
            return false;
        }
        ZVAL_COPY_VALUE(member, val);
    }
    return true;
}

void
setStringMember(zval* obj, const string& name, const string& val)
{
    zend_class_entry* cls = Z_OBJCE_P(obj);
    assert(cls);
    zend_update_property_stringl(cls,
                                 obj,
                                 const_cast<char*>(name.c_str()),
                                 static_cast<int>(name.size()),
                                 const_cast<char*>(val.c_str()),
                                 static_cast<int>(val.size())
                                );
}

template<typename T>
bool
getVersion(zval* zv, T& v, const char* type)
{
    if(Z_TYPE_P(zv) != IS_OBJECT)
    {
        invalidArgument("value does not contain an object");
        return false;
    }

    zend_class_entry* cls = idToClass(type);
    assert(cls);

    zend_class_entry* ce = Z_OBJCE_P(zv);
    if(ce != cls)
    {
        invalidArgument("expected an instance of %s", ce->name->val);
        return false;
    }

    zval majorVal;
    if(!getMember(zv, "major", &majorVal, IS_LONG, true))
    {
        return false;
    }

    zval minorVal;
    if(!getMember(zv, "minor", &minorVal, IS_LONG, true))
    {
        return false;
    }

    long m;
    m = static_cast<long>(Z_LVAL_P(&majorVal));
    if(m < 0 || m > 255)
    {
        invalidArgument("version major must be a value between 0 and 255");
        return false;
    }
    v.major = static_cast<Ice::Byte>(m);

    m = static_cast<long>(Z_LVAL_P(&minorVal));
    if(m < 0 || m > 255)
    {
        invalidArgument("version minor must be a value between 0 and 255");
        return false;
    }
    v.minor = static_cast<Ice::Byte>(m);

    return true;
}

template<typename T>
bool
createVersion(zval* zv, const T& version, const char* type)
{
    zend_class_entry* cls = idToClass(type);
    assert(cls);

    if(object_init_ex(zv, cls) != SUCCESS)
    {
        runtimeError("unable to initialize %s", cls->name->val);
        return false;
    }

    zend_update_property_long(cls, zv, const_cast<char*>("major"), sizeof("major") - 1, version.major);
    zend_update_property_long(cls, zv, const_cast<char*>("minor"), sizeof("minor") - 1, version.minor);

    return true;
}

template<typename T>
bool
versionToString(zval* zv, zval* s, const char* type)
{
    T v;
    if(!getVersion<T>(zv, v, type))
    {
        return false;
    }

    try
    {
        string str = IceInternal::versionToString<T>(v);
        ZVAL_STRINGL(s, STRCAST(str.c_str()), static_cast<int>(str.length()));
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex);
        return false;
    }

    return true;
}

template<typename T>
bool
stringToVersion(const string& s, zval* zv, const char* type)
{
    try
    {
        T v = IceInternal::stringToVersion<T>(s);
        return createVersion<T>(zv, v, type);
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex);
    }

    return false;
}

char Ice_ProtocolVersion[] = "::Ice::ProtocolVersion";
char Ice_EncodingVersion[] = "::Ice::EncodingVersion";

}

void*
IcePHP::extractWrapper(zval* zv)
{
    if(!zv)
    {
        runtimeError("method %s() must be invoked on an object", get_active_function_name());
        return 0;
    }

    zend_object* obj = Z_OBJ_P(zv);
    if(!obj)
    {
        runtimeError("no object found in %s()", get_active_function_name());
        return 0;
    }

    return obj;
}

zend_class_entry*
IcePHP::idToClass(const string& id)
{
#ifdef ICEPHP_USE_NAMESPACES
    string cls = scopedToName(id, true);
#else
    string cls = scopedToName(id, false);
#endif

    return nameToClass(cls);
}

zend_class_entry*
IcePHP::nameToClass(const string& name)
{
    zend_class_entry* result;
    zend_string* s = zend_string_init(STRCAST(name.c_str()), static_cast<int>(name.length()), 0);
    result = zend_lookup_class(s);
    zend_string_release(s);
    return result;
}

bool
IcePHP::createIdentity(zval* zv, const Ice::Identity& id)
{
    zend_class_entry* cls = idToClass("::Ice::Identity");
    assert(cls);

    if(object_init_ex(zv, cls) != SUCCESS)
    {
        runtimeError("unable to initialize Ice::Identity");
        return false;
    }

    setStringMember(zv, "name", id.name);
    setStringMember(zv, "category", id.category);

    return true;
}

bool
IcePHP::extractIdentity(zval* zv, Ice::Identity& id)
{
    if(Z_TYPE_P(zv) != IS_OBJECT)
    {
        invalidArgument("value does not contain an object");
        return false;
    }

    zend_class_entry* cls = idToClass("::Ice::Identity");
    assert(cls);

    zend_class_entry* ce = Z_OBJCE_P(zv);
    if(ce != cls)
    {
        invalidArgument("expected an identity but received %s", ce->name->val);
        return false;
    }

    //
    // Category is optional, but name is required.
    //
    zval categoryVal;
    ZVAL_UNDEF(&categoryVal);
    zval nameVal;
    ZVAL_UNDEF(&nameVal);

    bool catOk = getMember(zv, "category", &categoryVal, IS_STRING, false);
    bool nameOk = getMember(zv, "name", &nameVal, IS_STRING, true);

    if(!catOk || !nameOk)
    {
        return false;
    }

    id.name = Z_STRVAL_P(&nameVal);

    if(!Z_ISUNDEF(categoryVal))
    {
        id.category = Z_STRVAL_P(&categoryVal);
    }
    else
    {
        id.category = "";
    }

    return true;
}

bool
IcePHP::createStringMap(zval* zv, const map<string, string>& ctx)
{
    array_init(zv);

    for(map<string, string>::const_iterator p = ctx.begin(); p != ctx.end(); ++p)
    {
        if(add_assoc_stringl_ex(zv,
                                const_cast<char*>(p->first.c_str()),
                                static_cast<uint>(p->first.length()),
                                const_cast<char*>(p->second.c_str()),
                                static_cast<uint>(p->second.length())) == FAILURE)
        {
            return false;
        }
    }

    return true;
}

bool
IcePHP::extractStringMap(zval* zv, map<string, string>& ctx)
{
    if(Z_TYPE_P(zv) != IS_ARRAY)
    {
        string s = zendTypeToString(Z_TYPE_P(zv));
        invalidArgument("expected an associative array but received %s", s.c_str());
        return false;
    }

    HashTable* arr = Z_ARRVAL_P(zv);
    zend_ulong num_key;
    zend_string* key;
    zval* val;
    ZEND_HASH_FOREACH_KEY_VAL(arr, num_key, key, val)
    {
        if(!key)
        {
            invalidArgument("array key must be a string");
            return false;
        }

        if(Z_TYPE_P(val) != IS_STRING)
        {
            invalidArgument("array value must be a string");
            return false;
        }

        ctx[key->val] = Z_STRVAL_P(val);
        (void)num_key; // Avoids error from older versions of GCC about unused variable num_key.
    }
    ZEND_HASH_FOREACH_END();

    return true;
}

bool
IcePHP::createStringArray(zval* zv, const Ice::StringSeq& seq)
{
    array_init(zv);
    for(Ice::StringSeq::const_iterator p = seq.begin(); p != seq.end(); ++p)
    {
        if(add_next_index_stringl(zv, STRCAST(p->c_str()), static_cast<uint>(p->length())) == FAILURE)
        {
            return false;
        }
    }

    return true;
}

bool
IcePHP::extractStringArray(zval* zv, Ice::StringSeq& seq)
{
    if(Z_TYPE_P(zv) != IS_ARRAY)
    {
        string s = zendTypeToString(Z_TYPE_P(zv));
        invalidArgument("expected an array of strings but received %s", s.c_str());
        return false;
    }

    HashTable* arr = Z_ARRVAL_P(zv);
    zval* val;
    ZEND_HASH_FOREACH_VAL(arr, val)
    {
        if(Z_TYPE_P(val) != IS_STRING)
        {
            invalidArgument("array element must be a string");
            return false;
        }

        string s(Z_STRVAL_P(val), Z_STRLEN_P(val));
        seq.push_back(s);
    }
    ZEND_HASH_FOREACH_END();

    return true;
}

bool
IcePHP::createProtocolVersion(zval* zv, const Ice::ProtocolVersion& v)
{
    return createVersion<Ice::ProtocolVersion>(zv, v, Ice_ProtocolVersion);
}

bool
IcePHP::createEncodingVersion(zval* zv, const Ice::EncodingVersion& v)
{
    return createVersion<Ice::EncodingVersion>(zv, v, Ice_EncodingVersion);
}

bool
IcePHP::extractEncodingVersion(zval* zv, Ice::EncodingVersion& v)
{
    return getVersion<Ice::EncodingVersion>(zv, v, Ice_EncodingVersion);
}

static bool
convertLocalException(const Ice::LocalException& ex, zval* zex)
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
        setStringMember(zex, "reason", e.reason);
    }
    catch(const Ice::PluginInitializationException& e)
    {
        setStringMember(zex, "reason", e.reason);
    }
    catch(const Ice::AlreadyRegisteredException& e)
    {
        setStringMember(zex, "kindOfObject", e.kindOfObject);
        setStringMember(zex, "id", e.id);
    }
    catch(const Ice::NotRegisteredException& e)
    {
        setStringMember(zex, "kindOfObject", e.kindOfObject);
        setStringMember(zex, "id", e.id);
    }
    catch(const Ice::TwowayOnlyException& e)
    {
        setStringMember(zex, "operation", e.operation);
    }
    catch(const Ice::UnknownException& e)
    {
        setStringMember(zex, "unknown", e.unknown);
    }
    catch(const Ice::ObjectAdapterDeactivatedException& e)
    {
        setStringMember(zex, "name", e.name);
    }
    catch(const Ice::ObjectAdapterIdInUseException& e)
    {
        setStringMember(zex, "id", e.id);
    }
    catch(const Ice::NoEndpointException& e)
    {
        setStringMember(zex, "proxy", e.proxy);
    }
    catch(const Ice::EndpointParseException& e)
    {
        setStringMember(zex, "str", e.str);
    }
    catch(const Ice::IdentityParseException& e)
    {
        setStringMember(zex, "str", e.str);
    }
    catch(const Ice::ProxyParseException& e)
    {
        setStringMember(zex, "str", e.str);
    }
    catch(const Ice::IllegalIdentityException& e)
    {
        zval id;
        if(!createIdentity(&id, e.id))
        {
            zval_ptr_dtor(&id);
            return false;
        }
        zend_update_property(cls, zex, const_cast<char*>("id"), sizeof("id") - 1, &id);
        zval_ptr_dtor(&id);
    }
    catch(const Ice::RequestFailedException& e)
    {
        zval id;
        if(!createIdentity(&id, e.id))
        {
            zval_ptr_dtor(&id);
            return false;
        }
        zend_update_property(cls, zex, const_cast<char*>("id"), sizeof("id") - 1, &id);
        zval_ptr_dtor(&id);
        setStringMember(zex, "facet", e.facet);
        setStringMember(zex, "operation", e.operation);
    }
    catch(const Ice::FileException& e)
    {
        zend_update_property_long(cls, zex, const_cast<char*>("error"), sizeof("error") - 1, e.error);
        setStringMember(zex, "path", e.path);
    }
    catch(const Ice::SyscallException& e) // This must appear after all subclasses of SyscallException.
    {
        zend_update_property_long(cls, zex, const_cast<char*>("error"), sizeof("error") - 1, e.error);
    }
    catch(const Ice::DNSException& e)
    {
        zend_update_property_long(cls, zex, const_cast<char*>("error"), sizeof("error") - 1, e.error);
        setStringMember(zex, "host", e.host);
    }
    catch(const Ice::UnsupportedProtocolException& e)
    {
        zval v;
        if(!createProtocolVersion(&v, e.bad))
        {
            zval_ptr_dtor(&v);
            return false;
        }
        zend_update_property(cls, zex, const_cast<char*>("bad"), sizeof("bad") - 1, &v);
        zval_ptr_dtor(&v);

        if(!createProtocolVersion(&v, e.supported))
        {
            zval_ptr_dtor(&v);
            return false;
        }
        zend_update_property(cls, zex, const_cast<char*>("supported"), sizeof("supported") - 1, &v);
        zval_ptr_dtor(&v);
    }
    catch(const Ice::UnsupportedEncodingException& e)
    {
        zval v;
        if(!createEncodingVersion(&v, e.bad))
        {
            zval_ptr_dtor(&v);
            return false;
        }
        zend_update_property(cls, zex, const_cast<char*>("bad"), sizeof("bad") - 1, &v);
        zval_ptr_dtor(&v);

        if(!createEncodingVersion(&v, e.supported))
        {
            zval_ptr_dtor(&v);
            return false;
        }
        zend_update_property(cls, zex, const_cast<char*>("supported"), sizeof("supported") - 1, &v);
        zval_ptr_dtor(&v);
    }
    catch(const Ice::NoValueFactoryException& e)
    {
        setStringMember(zex, "reason", e.reason);
        setStringMember(zex, "type", e.type);
    }
    catch(const Ice::UnexpectedObjectException& e)
    {
        setStringMember(zex, "reason", e.reason);
        setStringMember(zex, "type", e.type);
        setStringMember(zex, "expectedType", e.expectedType);
    }
    catch(const Ice::ProtocolException& e) // This must appear after all subclasses of ProtocolException.
    {
        setStringMember(zex, "reason", e.reason);
    }
    catch(const Ice::FeatureNotSupportedException& e)
    {
        setStringMember(zex, "unsupportedFeature", e.unsupportedFeature);
    }
    catch(const Ice::SecurityException& e)
    {
        setStringMember(zex, "reason", e.reason);
    }
    catch(const Ice::ConnectionManuallyClosedException& e)
    {
        add_property_bool(zex, "graceful", e.graceful ? 1 : 0);
    }
    catch(const Ice::LocalException&)
    {
        //
        // Nothing to do.
        //
    }

    return true;
}

void
IcePHP::convertException(zval* zex, const Ice::Exception& ex)
{
    ZVAL_UNDEF(zex);

    ostringstream ostr;
    ostr << ex;
    string str = ostr.str();

    try
    {
        ex.ice_throw();
    }
    catch(const Ice::LocalException& e)
    {
        zend_class_entry* cls = idToClass(e.ice_id());
        if(cls)
        {
            if(object_init_ex(zex, cls) != SUCCESS)
            {
                runtimeError("unable to create exception %s", cls->name->val);
                return;
            }
            if(!convertLocalException(e, zex))
            {
                return;
            }
        }
        else
        {
            cls = idToClass("Ice::UnknownLocalException");
            assert(cls);
            if(object_init_ex(zex, cls) != SUCCESS)
            {
                runtimeError("unable to create exception %s", cls->name->val);
                return;
            }
            setStringMember(zex, "unknown", str);
        }
    }
    catch(const Ice::UserException&)
    {
        zend_class_entry* cls = idToClass("Ice::UnknownUserException");
        assert(cls);
        if(object_init_ex(zex, cls) != SUCCESS)
        {
            runtimeError("unable to create exception %s", cls->name->val);
            return;
        }
        setStringMember(zex, "unknown", str);
    }
    catch(const Ice::Exception&)
    {
        zend_class_entry* cls = idToClass("Ice::UnknownException");
        assert(cls);
        if(object_init_ex(zex, cls) != SUCCESS)
        {
            runtimeError("unable to create exception %s", cls->name->val);
            return;
        }
        setStringMember(zex, "unknown", str);
    }

    return;
}

void
IcePHP::throwException(const Ice::Exception& ex)
{
    zval zex;
    convertException(&zex, ex);
    if(!Z_ISUNDEF(zex))
    {
        zend_throw_exception_object(&zex);
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

    case IS_TRUE:
    case IS_FALSE:
        result = "bool";
        break;

    default:
        result = "unknown";
        break;
    }

    return result;
}

static void
throwError(const string& name, const string& msg)
{
    if(EG(exception))
    {
        return;
    }
    zval ex;
    // AutoDestroy destroy(&ex);

    zend_class_entry* cls;
    {
        zend_class_entry* p;
        zend_string* s = zend_string_init(STRCAST(name.c_str()), static_cast<int>(name.size()), 0);
        p = zend_lookup_class(s);
        zend_string_release(s);
        assert(p);
        cls = p;
    }
    if(object_init_ex(&ex, cls) == FAILURE)
    {
        assert(false);
    }

    //
    // Invoke constructor.
    //
    if(!invokeMethod(&ex, ZEND_CONSTRUCTOR_FUNC_NAME, msg))
    {
        assert(false);
    }

    zend_throw_exception_object(&ex);
    // destroy.release();
}

void
IcePHP::runtimeError(const char* fmt, ...)
{
    va_list args;
    char msg[1024];

    va_start(args, fmt);

#if defined(_MSC_VER)
    vsprintf_s(msg, fmt, args);
#else
    vsprintf(msg, fmt, args);
#endif

    va_end(args);

    throwError("RuntimeException", msg);
}

void
IcePHP::invalidArgument(const char* fmt, ...)
{
    va_list args;
    char msg[1024];

    va_start(args, fmt);

#if defined(_MSC_VER)
    vsprintf_s(msg, fmt, args);
#else
    vsprintf(msg, fmt, args);
#endif

    va_end(args);

    throwError("InvalidArgumentException", msg);
}

static bool
invokeMethodHelper(zval* obj, const string& name, zval* param)
{
    assert(zend_hash_str_exists(&Z_OBJCE_P(obj)->function_table, STRCAST(name.c_str()), name.size()));
    zval ret, method;
    ZVAL_STRING(&method, STRCAST(name.c_str()));
    uint32_t numParams = param ? 1 : 0;
    // zval** params = param ? &param : 0;
    int status = 0;
    zend_try
    {
        status = call_user_function(0, obj, &method, &ret, numParams, param);
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
IcePHP::invokeMethod(zval* obj, const string& name)
{
    return invokeMethodHelper(obj, name, 0);
}

bool
IcePHP::invokeMethod(zval* obj, const string& name, const string& arg)
{
    zval param;
    ZVAL_STRINGL(&param, STRCAST(arg.c_str()), static_cast<int>(arg.size()));
    return invokeMethodHelper(obj, name, &param);
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

        for(zend_ulong i = 0; i < ce->num_interfaces; ++i)
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

    RETURN_STRINGL(STRCAST(ICE_STRING_VERSION), static_cast<int>(strlen(ICE_STRING_VERSION)));
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

    string uuid = Ice::generateUUID();
    RETURN_STRINGL(STRCAST(uuid.c_str()), static_cast<int>(uuid.size()));
}

ZEND_FUNCTION(Ice_currentProtocol)
{
    if(ZEND_NUM_ARGS() > 0)
    {
        WRONG_PARAM_COUNT;
    }

    if(!createProtocolVersion(return_value, Ice::currentProtocol))
    {
        RETURN_NULL();
    }
}

ZEND_FUNCTION(Ice_currentProtocolEncoding)
{
    if(ZEND_NUM_ARGS() > 0)
    {
        WRONG_PARAM_COUNT;
    }

    if(!createEncodingVersion(return_value, Ice::currentProtocolEncoding))
    {
        RETURN_NULL();
    }
}

ZEND_FUNCTION(Ice_currentEncoding)
{
    if(ZEND_NUM_ARGS() > 0)
    {
        WRONG_PARAM_COUNT;
    }

    if(!createEncodingVersion(return_value, Ice::currentEncoding))
    {
        RETURN_NULL();
    }
}

ZEND_FUNCTION(Ice_protocolVersionToString)
{
    zend_class_entry* versionClass = idToClass(Ice_ProtocolVersion);
    assert(versionClass);

    zval zv;
    if(zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("O"), &zv, versionClass) != SUCCESS)
    {
        RETURN_NULL();
    }

    if(!versionToString<Ice::ProtocolVersion>(&zv, return_value, Ice_ProtocolVersion))
    {
        RETURN_NULL();
    }
}

ZEND_FUNCTION(Ice_stringToProtocolVersion)
{
    char* str;
    size_t strLen;
    if(zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("s"), &str, &strLen) != SUCCESS)
    {
        RETURN_NULL();
    }
    string s(str, strLen);

    if(!stringToVersion<Ice::ProtocolVersion>(s, return_value, Ice_ProtocolVersion))
    {
        RETURN_NULL();
    }
}

ZEND_FUNCTION(Ice_encodingVersionToString)
{
    zend_class_entry* versionClass = idToClass(Ice_EncodingVersion);
    assert(versionClass);

    zval* zv;
    if(zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("O"), &zv, versionClass) != SUCCESS)
    {
        RETURN_NULL();
    }

    if(!versionToString<Ice::EncodingVersion>(zv, return_value, Ice_EncodingVersion))
    {
        RETURN_NULL();
    }
}

ZEND_FUNCTION(Ice_stringToEncodingVersion)
{
    char* str;
    size_t strLen;
    if(zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("s"), &str, &strLen) != SUCCESS)
    {
        RETURN_NULL();
    }
    string s(str, strLen);

    if(!stringToVersion<Ice::EncodingVersion>(s, return_value, Ice_EncodingVersion))
    {
        RETURN_NULL();
    }
}
