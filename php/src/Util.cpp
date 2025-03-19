// Copyright (c) ZeroC, Inc.

#include "Util.h"
#include "Ice/UUID.h"
#include "Ice/VersionFunctions.h"

#include <algorithm>
#include <ctype.h>

using namespace std;
using namespace IcePHP;

namespace
{
    bool getMember(zval* zv, const string& name, zval* member, int type, bool required)
    {
        zval* val = zend_hash_str_find(Z_OBJPROP_P(zv), name.c_str(), name.size());
        if (!val)
        {
            if (required)
            {
                ostringstream os;
                os << "object does not contain member '" << name << "'";
                invalidArgument(os.str());
                return false;
            }
        }

        if (val)
        {
            assert(Z_TYPE_P(val) == IS_INDIRECT);
            val = Z_INDIRECT_P(val);
            if (Z_TYPE_P(val) != type)
            {
                ostringstream os;
                os << "expected value of type " << zendTypeToString(type) << " for member '" << name
                   << "' but received " << zendTypeToString(Z_TYPE_P(val));
                invalidArgument(os.str());
                return false;
            }
            ZVAL_COPY_VALUE(member, val);
        }
        return true;
    }

    void setStringMember(zval* obj, const string& name, const string& val)
    {
        zend_class_entry* cls = Z_OBJCE_P(obj);
        assert(cls);
        zend_update_property_stringl(
            cls,
            Z_OBJ_P(obj),
            const_cast<char*>(name.c_str()),
            static_cast<int>(name.size()),
            const_cast<char*>(val.c_str()),
            static_cast<int>(val.size()));
    }

    void setLongMember(zval* obj, const string& name, zend_long val)
    {
        zend_class_entry* cls = Z_OBJCE_P(obj);
        assert(cls);
        zend_update_property_long(
            cls,
            Z_OBJ_P(obj),
            const_cast<char*>(name.c_str()),
            static_cast<int>(name.size()),
            val);
    }

    template<typename T> bool getVersion(zval* zv, T& v, const char* type)
    {
        if (Z_TYPE_P(zv) != IS_OBJECT)
        {
            invalidArgument("value does not contain an object");
            return false;
        }

        zend_class_entry* cls = nameToClass(type);
        assert(cls);

        zend_class_entry* ce = Z_OBJCE_P(zv);
        if (ce != cls)
        {
            ostringstream os;
            os << "expected an instance of " << ce->name->val;
            invalidArgument(os.str());
            return false;
        }

        zval majorVal;
        if (!getMember(zv, "major", &majorVal, IS_LONG, true))
        {
            return false;
        }

        zval minorVal;
        if (!getMember(zv, "minor", &minorVal, IS_LONG, true))
        {
            return false;
        }

        long m;
        m = static_cast<long>(Z_LVAL_P(&majorVal));
        if (m < 0 || m > 255)
        {
            invalidArgument("version major must be a value between 0 and 255");
            return false;
        }
        v.major = static_cast<uint8_t>(m);

        m = static_cast<long>(Z_LVAL_P(&minorVal));
        if (m < 0 || m > 255)
        {
            invalidArgument("version minor must be a value between 0 and 255");
            return false;
        }
        v.minor = static_cast<uint8_t>(m);

        return true;
    }

    void zendUpdateProperty(zend_class_entry* scope, zval* zv, const char* name, size_t nameLength, zval* value)
    {
        zend_update_property(scope, Z_OBJ_P(zv), name, nameLength, value);
    }

    void zendUpdatePropertyLong(zend_class_entry* scope, zval* zv, const char* name, size_t nameLength, zend_long value)
    {
        zend_update_property_long(scope, Z_OBJ_P(zv), name, nameLength, value);
    }

    template<typename T> bool createVersion(zval* zv, const T& version, const char* type)
    {
        zend_class_entry* cls = nameToClass(type);
        assert(cls);

        if (object_init_ex(zv, cls) != SUCCESS)
        {
            ostringstream os;
            os << "unable to initialize " << cls->name->val;
            runtimeError(os.str());
            return false;
        }
        zendUpdatePropertyLong(cls, zv, const_cast<char*>("major"), sizeof("major") - 1, version.major);
        zendUpdatePropertyLong(cls, zv, const_cast<char*>("minor"), sizeof("minor") - 1, version.minor);

        return true;
    }

    template<typename T> bool versionToString(zval* zv, zval* s, const char* type)
    {
        T v;
        if (!getVersion<T>(zv, v, type))
        {
            return false;
        }

        try
        {
            string str = IceInternal::versionToString<T>(v);
            ZVAL_STRINGL(s, str.c_str(), static_cast<int>(str.length()));
        }
        catch (...)
        {
            throwException(current_exception());
            return false;
        }

        return true;
    }

    template<typename T> bool stringToVersion(const string& s, zval* zv, const char* type)
    {
        try
        {
            T v = IceInternal::stringToVersion<T>(s);
            return createVersion<T>(zv, v, type);
        }
        catch (...)
        {
            throwException(current_exception());
        }

        return false;
    }

    char Ice_ProtocolVersionType[] = "\\Ice\\ProtocolVersion";
    char Ice_EncodingVersionType[] = "\\Ice\\EncodingVersion";
}

void*
IcePHP::extractWrapper(zval* zv)
{
    if (!zv)
    {
        ostringstream os;
        os << "method " << get_active_function_name() << " must be invoked on an object";
        runtimeError(os.str());
        return nullptr;
    }

    zend_object* obj = Z_OBJ_P(zv);
    if (!obj)
    {
        ostringstream os;
        os << "no object found in " << get_active_function_name() << "()";
        runtimeError(os.str());
        return nullptr;
    }

    return obj;
}

zend_class_entry*
IcePHP::nameToClass(const string& name)
{
    zend_class_entry* result;
    zend_string* s = zend_string_init(name.c_str(), static_cast<int>(name.length()), 0);
    result = zend_lookup_class(s);
    zend_string_release(s);
    return result;
}

bool
IcePHP::createIdentity(zval* zv, const Ice::Identity& id)
{
    zend_class_entry* cls = nameToClass("\\Ice\\Identity");
    assert(cls);

    if (object_init_ex(zv, cls) != SUCCESS)
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
    if (Z_TYPE_P(zv) != IS_OBJECT)
    {
        invalidArgument("value does not contain an object");
        return false;
    }

    zend_class_entry* cls = nameToClass("\\Ice\\Identity");
    assert(cls);

    zend_class_entry* ce = Z_OBJCE_P(zv);
    if (ce != cls)
    {
        ostringstream os;
        os << "expected an identity but received " << ce->name->val;
        invalidArgument(os.str());
        return false;
    }

    // Category is optional, but name is required.
    zval categoryVal;
    ZVAL_UNDEF(&categoryVal);
    zval nameVal;
    ZVAL_UNDEF(&nameVal);

    bool catOk = getMember(zv, "category", &categoryVal, IS_STRING, false);
    bool nameOk = getMember(zv, "name", &nameVal, IS_STRING, true);

    if (!catOk || !nameOk)
    {
        return false;
    }

    id.name = Z_STRVAL_P(&nameVal);

    if (!Z_ISUNDEF(categoryVal))
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

    for (auto p = ctx.begin(); p != ctx.end(); ++p)
    {
        add_assoc_stringl_ex(
            zv,
            const_cast<char*>(p->first.c_str()),
            static_cast<uint32_t>(p->first.length()),
            const_cast<char*>(p->second.c_str()),
            static_cast<uint32_t>(p->second.length()));
    }

    return true;
}

// TODO: avoid duplication with code above.

bool
IcePHP::createContext(zval* zv, const Ice::Context& ctx)
{
    array_init(zv);

    for (auto p = ctx.begin(); p != ctx.end(); ++p)
    {
        add_assoc_stringl_ex(
            zv,
            const_cast<char*>(p->first.c_str()),
            static_cast<uint32_t>(p->first.length()),
            const_cast<char*>(p->second.c_str()),
            static_cast<uint32_t>(p->second.length()));
    }

    return true;
}

bool
IcePHP::extractContext(zval* zv, Ice::Context& ctx)
{
    if (Z_TYPE_P(zv) != IS_ARRAY)
    {
        ostringstream os;
        os << "expected an associative array but received " << zendTypeToString(Z_TYPE_P(zv));
        invalidArgument(os.str());
        return false;
    }

    HashTable* arr = Z_ARRVAL_P(zv);
    [[maybe_unused]] zend_ulong num_key;
    zend_string* key;
    zval* val;
    ZEND_HASH_FOREACH_KEY_VAL(arr, num_key, key, val)
    {
        if (!key)
        {
            invalidArgument("array key must be a string");
            return false;
        }

        if (Z_TYPE_P(val) != IS_STRING)
        {
            invalidArgument("array value must be a string");
            return false;
        }

        ctx[key->val] = Z_STRVAL_P(val);
    }
    ZEND_HASH_FOREACH_END();

    return true;
}

bool
IcePHP::createStringArray(zval* zv, const Ice::StringSeq& seq)
{
    array_init(zv);
    for (Ice::StringSeq::const_iterator p = seq.begin(); p != seq.end(); ++p)
    {
        if (add_next_index_stringl(zv, p->c_str(), static_cast<uint32_t>(p->length())) == FAILURE)
        {
            return false;
        }
    }

    return true;
}

bool
IcePHP::extractStringArray(zval* zv, Ice::StringSeq& seq)
{
    if (Z_TYPE_P(zv) != IS_ARRAY)
    {
        ostringstream os;
        os << "expected an array of strings but received " << zendTypeToString(Z_TYPE_P(zv));
        invalidArgument(os.str());
        return false;
    }

    HashTable* arr = Z_ARRVAL_P(zv);
    zval* val;
    ZEND_HASH_FOREACH_VAL(arr, val)
    {
        if (Z_TYPE_P(val) != IS_STRING)
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
    return createVersion<Ice::ProtocolVersion>(zv, v, Ice_ProtocolVersionType);
}

bool
IcePHP::createEncodingVersion(zval* zv, const Ice::EncodingVersion& v)
{
    return createVersion<Ice::EncodingVersion>(zv, v, Ice_EncodingVersionType);
}

bool
IcePHP::extractEncodingVersion(zval* zv, Ice::EncodingVersion& v)
{
    return getVersion<Ice::EncodingVersion>(zv, v, Ice_EncodingVersionType);
}

namespace
{
    const char* const localExceptionTypeId = "::Ice::LocalException";

    zend_class_entry* createPHPException(zval* ex, const char* typeId, bool fallbackToLocalException = false)
    {
        // Convert the exception's typeId to its mapped Python type by replacing "::Ice::" with "\Ice\".
        // This function should only ever be called on a specified list of Ice local exceptions.
        string result = typeId;
        assert(result.find("::Ice::") == 0);
        result.replace(0, 7, "\\Ice\\");
        assert(result.find(':') == string::npos); // Assert that there weren't any intermediate scopes.

        zend_class_entry* cls = nameToClass(result);
        if (!cls)
        {
            if (fallbackToLocalException)
            {
                cls = nameToClass("\\Ice\\LocalException");
                assert(cls);
            }
            else
            {
                ostringstream os;
                os << "unable to create PHP exception class for type ID " << typeId;
                runtimeError(os.str());
                return nullptr;
            }
        }

        if (object_init_ex(ex, cls) != SUCCESS)
        {
            ostringstream os;
            os << "unable to create PHP exception class " << cls->name->val;
            runtimeError(os.str());
            return nullptr;
        }
        return cls;
    }

    void createInvalidArgumentException(zval* ex, const char* msg)
    {
        zend_class_entry* cls = nameToClass("InvalidArgumentException");
        assert(cls);
        if (object_init_ex(ex, cls) != SUCCESS)
        {
            runtimeError("unable to create InvalidArgumentException");
            return;
        }
        setStringMember(ex, "message", msg);
    }
}

void
IcePHP::convertException(zval* zex, std::exception_ptr ex)
{
    try
    {
        rethrow_exception(ex);
    }
    // Write the PHP exception into zex
    catch (const Ice::AlreadyRegisteredException& e)
    {
        zend_class_entry* cls = createPHPException(zex, e.ice_id());
        if (!cls)
        {
            return;
        }
        setStringMember(zex, "kindOfObject", e.kindOfObject());
        setStringMember(zex, "id", e.id());
        setStringMember(zex, "message", e.what());
    }
    catch (const Ice::NotRegisteredException& e)
    {
        zend_class_entry* cls = createPHPException(zex, e.ice_id());
        if (!cls)
        {
            return;
        }
        setStringMember(zex, "kindOfObject", e.kindOfObject());
        setStringMember(zex, "id", e.id());
        setStringMember(zex, "message", e.what());
    }
    catch (const Ice::RequestFailedException& e)
    {
        // It would be nicer to make the properties read-only and call the constructor; however, it's not easy to do
        // with the PHP C API.

        zend_class_entry* cls = createPHPException(zex, e.ice_id());
        if (!cls)
        {
            return;
        }

        setLongMember(zex, "replyStatus", static_cast<zend_long>(e.replyStatus()));

        zval id;
        if (!createIdentity(&id, e.id()))
        {
            zval_ptr_dtor(&id);
            return;
        }

        zendUpdateProperty(cls, zex, const_cast<char*>("id"), sizeof("id") - 1, &id);
        zval_ptr_dtor(&id);
        setStringMember(zex, "facet", e.facet());
        setStringMember(zex, "operation", e.operation());
        setStringMember(zex, "message", e.what()); // message is a protected property of the base class.
    }
    catch (const Ice::DispatchException& e)
    {
        zend_class_entry* cls = createPHPException(zex, e.ice_id());
        if (!cls)
        {
            return;
        }

        setLongMember(zex, "replyStatus", static_cast<zend_long>(e.replyStatus()));
        setStringMember(zex, "message", e.what());
    }
    catch (const Ice::LocalException& e)
    {
        zend_class_entry* cls = createPHPException(zex, e.ice_id(), true);
        if (!cls)
        {
            return;
        }
        setStringMember(zex, "message", e.what());
    }
    catch (const std::invalid_argument& e)
    {
        createInvalidArgumentException(zex, e.what());
    }
    catch (const std::exception& e)
    {
        // Create a plain local exception.
        zend_class_entry* cls = createPHPException(zex, localExceptionTypeId);
        if (!cls)
        {
            return;
        }
        setStringMember(zex, "message", e.what());
    }
    catch (...)
    {
        zend_class_entry* cls = createPHPException(zex, localExceptionTypeId);
        if (!cls)
        {
            return;
        }
        setStringMember(zex, "message", "unknown C++ exception");
    }
}

void
IcePHP::throwException(std::exception_ptr ex)
{
    zval zex;
    convertException(&zex, ex);
    if (!Z_ISUNDEF(zex))
    {
        zend_throw_exception_object(&zex);
    }
}

std::string
IcePHP::zendTypeToString(int type)
{
    string result;

    switch (type)
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
    if (EG(exception))
    {
        return;
    }
    zend_class_entry* cls = nameToClass(name);
    assert(cls);
    zval ex;
    if (object_init_ex(&ex, cls) == FAILURE)
    {
        assert(false);
    }

    if (!invokeMethod(&ex, ZEND_CONSTRUCTOR_FUNC_NAME, msg))
    {
        assert(false);
    }

    zend_throw_exception_object(&ex);
}

void
IcePHP::runtimeError(const string& msg)
{
    throwError("RuntimeException", msg);
}

void
IcePHP::invalidArgument(const string& msg)
{
    throwError("InvalidArgumentException", msg);
}

static bool
invokeMethodHelper(zval* obj, const string& name, zval* param)
{
    assert(zend_hash_str_exists(&Z_OBJCE_P(obj)->function_table, name.c_str(), name.size()));
    zval ret, method;
    ZVAL_STRING(&method, name.c_str());
    uint32_t numParams = param ? 1 : 0;
    int status = 0;
    zend_try { status = call_user_function(0, obj, &method, &ret, numParams, param); }
    zend_catch { status = FAILURE; }
    zend_end_try();
    zval_dtor(&method);
    zval_dtor(&ret);
    if (status == FAILURE || EG(exception))
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
    ZVAL_STRINGL(&param, arg.c_str(), static_cast<int>(arg.size()));
    AutoDestroy destroy(&param);
    bool retval = invokeMethodHelper(obj, name, &param);
    return retval;
}

bool
IcePHP::checkClass(zend_class_entry* ce, zend_class_entry* base)
{
    while (ce)
    {
        if (ce == base)
        {
            return true;
        }

        for (zend_ulong i = 0; i < ce->num_interfaces; ++i)
        {
            if (checkClass(ce->interfaces[i], base))
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
    if (ZEND_NUM_ARGS() > 0)
    {
        WRONG_PARAM_COUNT;
    }

    RETURN_STRINGL(ICE_STRING_VERSION, static_cast<int>(strlen(ICE_STRING_VERSION)));
}

ZEND_FUNCTION(Ice_intVersion)
{
    if (ZEND_NUM_ARGS() > 0)
    {
        WRONG_PARAM_COUNT;
    }

    RETURN_LONG(ICE_INT_VERSION);
}

ZEND_FUNCTION(Ice_generateUUID)
{
    if (ZEND_NUM_ARGS() > 0)
    {
        WRONG_PARAM_COUNT;
    }

    string uuid = Ice::generateUUID();
    RETURN_STRINGL(uuid.c_str(), static_cast<int>(uuid.size()));
}

ZEND_FUNCTION(Ice_currentProtocol)
{
    if (ZEND_NUM_ARGS() > 0)
    {
        WRONG_PARAM_COUNT;
    }

    if (!createProtocolVersion(return_value, Ice::currentProtocol))
    {
        RETURN_NULL();
    }
}

ZEND_FUNCTION(Ice_currentProtocolEncoding)
{
    if (ZEND_NUM_ARGS() > 0)
    {
        WRONG_PARAM_COUNT;
    }

    if (!createEncodingVersion(return_value, Ice::currentProtocolEncoding))
    {
        RETURN_NULL();
    }
}

ZEND_FUNCTION(Ice_currentEncoding)
{
    if (ZEND_NUM_ARGS() > 0)
    {
        WRONG_PARAM_COUNT;
    }

    if (!createEncodingVersion(return_value, Ice::currentEncoding))
    {
        RETURN_NULL();
    }
}

ZEND_FUNCTION(Ice_protocolVersionToString)
{
    zend_class_entry* versionClass = nameToClass(Ice_ProtocolVersionType);
    assert(versionClass);

    zval zv;
    if (zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("O"), &zv, versionClass) != SUCCESS)
    {
        RETURN_NULL();
    }

    if (!versionToString<Ice::ProtocolVersion>(&zv, return_value, Ice_ProtocolVersionType))
    {
        RETURN_NULL();
    }
}

ZEND_FUNCTION(Ice_stringToProtocolVersion)
{
    char* str;
    size_t strLen;
    if (zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("s"), &str, &strLen) != SUCCESS)
    {
        RETURN_NULL();
    }
    string s(str, strLen);

    if (!stringToVersion<Ice::ProtocolVersion>(s, return_value, Ice_ProtocolVersionType))
    {
        RETURN_NULL();
    }
}

ZEND_FUNCTION(Ice_encodingVersionToString)
{
    zend_class_entry* versionClass = nameToClass(Ice_EncodingVersionType);
    assert(versionClass);

    zval* zv;
    if (zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("O"), &zv, versionClass) != SUCCESS)
    {
        RETURN_NULL();
    }

    if (!versionToString<Ice::EncodingVersion>(zv, return_value, Ice_EncodingVersionType))
    {
        RETURN_NULL();
    }
}

ZEND_FUNCTION(Ice_stringToEncodingVersion)
{
    char* str;
    size_t strLen;
    if (zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("s"), &str, &strLen) != SUCCESS)
    {
        RETURN_NULL();
    }
    string s(str, strLen);

    if (!stringToVersion<Ice::EncodingVersion>(s, return_value, Ice_EncodingVersionType))
    {
        RETURN_NULL();
    }
}
