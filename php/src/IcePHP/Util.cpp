// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Util.h>
#include <IceUtil/DisableWarnings.h>
#include <Ice/IdentityUtil.h>
#include <algorithm>
#include <ctype.h>

using namespace std;
using namespace IcePHP;

static string
lookupKwd(const string& name)
{
    string lower = lowerCase(name); // PHP is case insensitive.

    //
    // Keyword list. *Must* be kept in alphabetical order.
    //
    static const string keywordList[] = 
    {       
        "and", "array", "as", "break", "case", "cfunction", "class", "const", "continue", "declare", "default",
        "die", "do", "echo", "else", "elseif", "empty", "enddeclare", "endfor", "endforeach", "endif", "endswitch",
        "endwhile", "eval", "exit", "extends", "for", "foreach", "function", "global", "if", "include",
        "include_once", "isset", "list", "new", "old_function", "or", "print", "require", "require_once", "return",
        "static", "switch", "unset", "use", "var", "while", "xor"
    };
    bool found =  binary_search(&keywordList[0],
                                &keywordList[sizeof(keywordList) / sizeof(*keywordList)],
                                lower);
    return found ? "_" + name : name;
}

//
// Split a scoped name into its components and return the components as a list of (unscoped) identifiers.
//
static vector<string>
splitScopedName(const string& scoped)
{
    assert(scoped[0] == ':');
    vector<string> ids;
    string::size_type next = 0;
    string::size_type pos;
    while((pos = scoped.find("::", next)) != string::npos)
    {
        pos += 2;
        if(pos != scoped.size())
        {
            string::size_type endpos = scoped.find("::", pos);
            if(endpos != string::npos)
            {
                ids.push_back(scoped.substr(pos, endpos - pos));
            }
        }
        next = pos;
    }
    if(next != scoped.size())
    {
        ids.push_back(scoped.substr(next));
    }
    else
    {
        ids.push_back("");
    }

    return ids;
}

ZEND_FUNCTION(Ice_stringToIdentity)
{
    if(ZEND_NUM_ARGS() != 1)
    {
        WRONG_PARAM_COUNT;
    }

    char* str;
    int len;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &str, &len) == FAILURE)
    {
        RETURN_NULL();
    }

    try
    {
        Ice::Identity id = Ice::stringToIdentity(str);
        createIdentity(return_value, id TSRMLS_CC);
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
    }
}

ZEND_FUNCTION(Ice_identityToString)
{
    if(ZEND_NUM_ARGS() != 1)
    {
        WRONG_PARAM_COUNT;
    }

    zend_class_entry* cls = findClass("Ice_Identity" TSRMLS_CC);
    assert(cls);

    zval *zid;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zid, cls) == FAILURE)
    {
        RETURN_NULL();
    }

    Ice::Identity id;
    if(extractIdentity(zid, id TSRMLS_CC))
    {
        string s = Ice::identityToString(id);
        RETURN_STRINGL(const_cast<char*>(s.c_str()), s.length(), 1);
    }
}

bool
IcePHP::createIdentity(zval* zv, const Ice::Identity& id TSRMLS_DC)
{
    zend_class_entry* cls = findClass("Ice_Identity" TSRMLS_CC);
    assert(cls);

    if(object_init_ex(zv, cls) != SUCCESS)
    {
        php_error_docref(0 TSRMLS_CC, E_ERROR, "unable to initialize Ice::Identity");
        return false;
    }

    zend_update_property_string(cls, zv, "name", sizeof("name") - 1, const_cast<char*>(id.name.c_str()) TSRMLS_CC);
    zend_update_property_string(cls, zv, "category", sizeof("category") - 1,
                                const_cast<char*>(id.category.c_str()) TSRMLS_CC);

    return true;
}

bool
IcePHP::extractIdentity(zval* zv, Ice::Identity& id TSRMLS_DC)
{
    if(Z_TYPE_P(zv) != IS_OBJECT)
    {
        php_error_docref(0 TSRMLS_CC, E_ERROR, "value does not contain an object");
        return false;
    }

    zend_class_entry* cls = findClass("Ice_Identity" TSRMLS_CC);
    assert(cls);

    zend_class_entry* ce = Z_OBJCE_P(zv);
    if(ce != cls)
    {
        php_error_docref(0 TSRMLS_CC, E_ERROR, "expected an identity but received %s", ce->name);
        return false;
    }

    //
    // Category is optional, but name is required.
    //
    zval** categoryVal = 0;
    zval** nameVal;
    if(zend_hash_find(Z_OBJPROP_P(zv), "name", sizeof("name"), reinterpret_cast<void**>(&nameVal)) == FAILURE)
    {
        php_error_docref(0 TSRMLS_CC, E_ERROR, "identity value does not contain member `name'");
        return false;
    }
    zend_hash_find(Z_OBJPROP_P(zv), "category", sizeof("category"), reinterpret_cast<void**>(&categoryVal));

    if(Z_TYPE_PP(nameVal) != IS_STRING)
    {
        string s = zendTypeToString(Z_TYPE_PP(nameVal));
        php_error_docref(0 TSRMLS_CC, E_ERROR, "expected a string value for identity member `name' but received %s",
                         s.c_str());
        return false;
    }

    if(categoryVal && Z_TYPE_PP(categoryVal) != IS_STRING && Z_TYPE_PP(categoryVal) != IS_NULL)
    {
        string s = zendTypeToString(Z_TYPE_PP(categoryVal));
        php_error_docref(0 TSRMLS_CC, E_ERROR,
                         "expected a string value for identity member `category' but received %s", s.c_str());
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
IcePHP::createContext(zval* zv, const Ice::Context& ctx TSRMLS_DC)
{
    array_init(zv);
    for(Ice::Context::const_iterator p = ctx.begin(); p != ctx.end(); ++p)
    {
        zval* val;
        MAKE_STD_ZVAL(val);
        ZVAL_STRINGL(val, const_cast<char*>(p->second.c_str()), p->second.length(), 1);
        add_assoc_zval_ex(zv, const_cast<char*>(p->first.c_str()), p->first.length() + 1, val);
    }

    return true;
}

bool
IcePHP::extractContext(zval* zv, Ice::Context& ctx TSRMLS_DC)
{
    if(Z_TYPE_P(zv) != IS_ARRAY)
    {
        string s = zendTypeToString(Z_TYPE_P(zv));
        php_error_docref(0 TSRMLS_CC, E_ERROR, "expected an array for the context argument but received %s",
                         s.c_str());
        return false;
    }

    HashTable* arr = Z_ARRVAL_P(zv);
    HashPosition pos;
    zval** val;

    zend_hash_internal_pointer_reset_ex(arr, &pos);
    while(zend_hash_get_current_data_ex(arr, reinterpret_cast<void**>(&val), &pos) != FAILURE)
    {
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
            php_error_docref(0 TSRMLS_CC, E_ERROR, "context key must be a string");
            return false;
        }

        if(Z_TYPE_PP(val) != IS_STRING)
        {
            php_error_docref(0 TSRMLS_CC, E_ERROR, "context value must be a string");
            return false;
        }

        ctx[keyStr] = Z_STRVAL_PP(val);

        zend_hash_move_forward_ex(arr, &pos);
    }

    return true;
}

#ifdef WIN32
extern "C"
#endif
static void
dtor_wrapper(void* p)
{
    zval_ptr_dtor(static_cast<zval**>(p));
}

ice_object*
IcePHP::newObject(zend_class_entry* ce TSRMLS_DC)
{
    ice_object* obj;
    zval* tmp;

    obj = static_cast<ice_object*>(emalloc(sizeof(ice_object)));
    obj->zobj.ce = ce;
    obj->zobj.guards = 0;
    obj->ptr = 0;

    obj->zobj.properties = static_cast<HashTable*>(emalloc(sizeof(HashTable)));
    zend_hash_init(obj->zobj.properties, 0, 0, dtor_wrapper, 0);
    zend_hash_copy(obj->zobj.properties, &ce->default_properties, (copy_ctor_func_t) zval_add_ref, &tmp,
                   sizeof(zval*));

    return obj;
}

ice_object*
IcePHP::getObject(zval* zv TSRMLS_DC)
{
    if(!zv)
    {
        php_error_docref(0 TSRMLS_CC, E_ERROR, "method %s() must be invoked on an object",
                         get_active_function_name(TSRMLS_C));
        return 0;
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(zv TSRMLS_CC));
    if(!obj)
    {
        php_error_docref(0 TSRMLS_CC, E_ERROR, "no object found in %s()", get_active_function_name(TSRMLS_C));
        return 0;
    }

    return obj;
}

void
IcePHP::throwException(const IceUtil::Exception& ex TSRMLS_DC)
{
    try
    {
        try
        {
            ex.ice_throw();
        }
        catch(const Ice::TwowayOnlyException& e)
        {
            string name = e.ice_name();
            zend_class_entry* cls = findClassScoped(name TSRMLS_CC);
            if(!cls)
            {
                throw;
            }

            zval* zex;
            MAKE_STD_ZVAL(zex);
            if(object_init_ex(zex, cls) != SUCCESS)
            {
                php_error_docref(0 TSRMLS_CC, E_ERROR, "unable to create exception %s", cls->name);
                return;
            }

            //
            // Set the unknown member.
            //
            zend_update_property_string(cls, zex, "operation", sizeof("operation") - 1,
                                        const_cast<char*>(e.operation.c_str()) TSRMLS_CC);

            //
            // Throw the exception.
            //
            zend_throw_exception_object(zex TSRMLS_CC);
        }
        catch(const Ice::UnknownException& e)
        {
            string name = e.ice_name();
            zend_class_entry* cls = findClassScoped(name TSRMLS_CC);
            if(!cls)
            {
                throw;
            }

            zval* zex;
            MAKE_STD_ZVAL(zex);
            if(object_init_ex(zex, cls) != SUCCESS)
            {
                php_error_docref(0 TSRMLS_CC, E_ERROR, "unable to create exception %s", cls->name);
                return;
            }

            //
            // Set the unknown member.
            //
            zend_update_property_string(cls, zex, "unknown", sizeof("unknown") - 1,
                                        const_cast<char*>(e.unknown.c_str()) TSRMLS_CC);

            //
            // Throw the exception.
            //
            zend_throw_exception_object(zex TSRMLS_CC);
        }
        catch(const Ice::RequestFailedException& e)
        {
            string name = e.ice_name();
            zend_class_entry* cls = findClassScoped(name TSRMLS_CC);
            if(!cls)
            {
                throw;
            }

            zval* zex;
            MAKE_STD_ZVAL(zex);
            if(object_init_ex(zex, cls) != SUCCESS)
            {
                php_error_docref(0 TSRMLS_CC, E_ERROR, "unable to create exception %s", cls->name);
                return;
            }

            //
            // Set the id member.
            //
            zval* id;
            MAKE_STD_ZVAL(id);
            if(!createIdentity(id, e.id TSRMLS_CC))
            {
                return;
            }
            zend_update_property(cls, zex, "id", sizeof("id") - 1, id TSRMLS_CC);

            //
            // Set the facet member.
            //
            zval* facet;
            MAKE_STD_ZVAL(facet);
            ZVAL_STRINGL(facet, const_cast<char*>(e.facet.c_str()), e.facet.length(), 1);
            zend_update_property(cls, zex, "facet", sizeof("facet") - 1, facet TSRMLS_CC);

            //
            // Set the operation member.
            //
            zend_update_property_string(cls, zex, "operation", sizeof("operation") - 1,
                                        const_cast<char*>(e.operation.c_str()) TSRMLS_CC);

            //
            // Throw the exception.
            //
            zend_throw_exception_object(zex TSRMLS_CC);
        }
        catch(const Ice::NoObjectFactoryException& e)
        {
            string name = e.ice_name();
            zend_class_entry* cls = findClassScoped(name TSRMLS_CC);
            if(!cls)
            {
                throw;
            }

            zval* zex;
            MAKE_STD_ZVAL(zex);
            if(object_init_ex(zex, cls) != SUCCESS)
            {
                php_error_docref(0 TSRMLS_CC, E_ERROR, "unable to create exception %s", cls->name);
                return;
            }

            //
            // Set the reason member.
            //
            zend_update_property_string(cls, zex, "reason", sizeof("reason") - 1,
                                        const_cast<char*>(e.reason.c_str()) TSRMLS_CC);

            //
            // Set the type member.
            //
            zend_update_property_string(cls, zex, "type", sizeof("type") - 1, const_cast<char*>(e.type.c_str())
                                        TSRMLS_CC);

            //
            // Throw the exception.
            //
            zend_throw_exception_object(zex TSRMLS_CC);
        }
        catch(const Ice::UnexpectedObjectException& e)
        {
            string name = e.ice_name();
            zend_class_entry* cls = findClassScoped(name TSRMLS_CC);
            if(!cls)
            {
                throw;
            }

            zval* zex;
            MAKE_STD_ZVAL(zex);
            if(object_init_ex(zex, cls) != SUCCESS)
            {
                php_error_docref(0 TSRMLS_CC, E_ERROR, "unable to create exception %s", cls->name);
                return;
            }

            //
            // Set the reason member.
            //
            zend_update_property_string(cls, zex, "reason", sizeof("reason") - 1,
                                        const_cast<char*>(e.reason.c_str()) TSRMLS_CC);

            //
            // Set the type and exptected type members.
            //
            zend_update_property_string(cls, zex, "type", sizeof("type") - 1, const_cast<char*>(e.type.c_str())
                                        TSRMLS_CC);
            zend_update_property_string(cls, zex, "expectedType", sizeof("expectedType") - 1,
                                        const_cast<char*>(e.expectedType.c_str()) TSRMLS_CC);

            //
            // Throw the exception.
            //
            zend_throw_exception_object(zex TSRMLS_CC);
        }
        catch(const Ice::MarshalException& e)
        {
            string name = e.ice_name();
            zend_class_entry* cls = findClassScoped(name TSRMLS_CC);
            if(!cls)
            {
                throw;
            }

            zval* zex;
            MAKE_STD_ZVAL(zex);
            if(object_init_ex(zex, cls) != SUCCESS)
            {
                php_error_docref(0 TSRMLS_CC, E_ERROR, "unable to create exception %s", cls->name);
                return;
            }

            //
            // Set the reason member.
            //
            zend_update_property_string(cls, zex, "reason", sizeof("reason") - 1,
                                        const_cast<char*>(e.reason.c_str()) TSRMLS_CC);

            //
            // Throw the exception.
            //
            zend_throw_exception_object(zex TSRMLS_CC);
        }
    }
    catch(const Ice::LocalException& e)
    {
        zval* zex;
        MAKE_STD_ZVAL(zex);

        //
        // See if we have a PHP class for the exception, otherwise raise UnknownLocalException.
        //
        string name = e.ice_name();
        zend_class_entry* cls = findClassScoped(name TSRMLS_CC);
        if(cls)
        {
            if(object_init_ex(zex, cls) != SUCCESS)
            {
                php_error_docref(0 TSRMLS_CC, E_ERROR, "unable to create exception %s", cls->name);
                return;
            }
        }
        else
        {
            cls = findClass("Ice_UnknownLocalException" TSRMLS_CC);
            if(!cls)
            {
                php_error_docref(0 TSRMLS_CC, E_ERROR, "unable to find class Ice_UnknownLocalException");
                return;
            }

            if(object_init_ex(zex, cls) != SUCCESS)
            {
                php_error_docref(0 TSRMLS_CC, E_ERROR, "unable to create exception %s", cls->name);
                return;
            }

            //
            // Set the unknown member.
            //
            ostringstream ostr;
            e.ice_print(ostr);
            string str = ostr.str();
            zend_update_property_string(cls, zex, "unknown", sizeof("unknown") - 1,
                                        const_cast<char*>(str.c_str()) TSRMLS_CC);
        }

        //
        // Throw the exception.
        //
        zend_throw_exception_object(zex TSRMLS_CC);
    }
    catch(const Ice::UserException&)
    {
        assert(false);
    }
    catch(const IceUtil::Exception& e)
    {
        ostringstream ostr;
        e.ice_print(ostr);
        php_error_docref(0 TSRMLS_CC, E_ERROR, "exception: %s", ostr.str().c_str());
    }
}

zend_class_entry*
IcePHP::findClass(const string& flat TSRMLS_DC)
{
    zend_class_entry** result;
    string lower = lowerCase(flat);
    if(zend_lookup_class(const_cast<char*>(lower.c_str()), lower.length(), &result TSRMLS_CC) == FAILURE)
    {
        return 0;
    }
    return *result;
}

zend_class_entry*
IcePHP::findClassScoped(const string& scoped TSRMLS_DC)
{
    return findClass(flatten(scoped) TSRMLS_CC);
}

string
IcePHP::lowerCase(const string& s)
{
    string result(s);
    transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

string
IcePHP::flatten(const string& scoped)
{
    string result = scoped;
    if(result.find("::") == 0)
    {
        result.erase(0, 2);
    }

    string::size_type pos;
    while((pos = result.find("::")) != string::npos)
    {
        result.replace(pos, 2, "_");
    }

    return fixIdent(result);
}

string
IcePHP::fixIdent(const string& ident)
{
    if(ident[0] != ':')
    {
        return lookupKwd(ident);
    }
    vector<string> ids = splitScopedName(ident);
    transform(ids.begin(), ids.end(), ids.begin(), ptr_fun(lookupKwd));
    stringstream result;
    for(vector<string>::const_iterator i = ids.begin(); i != ids.end(); ++i)
    {
        result << "::" + *i;
    }
    return result.str();
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

bool
IcePHP::isNativeKey(const Slice::TypePtr& type)
{
    //
    // PHP's native associative array supports only integer and string types for the key.
    // For Slice dictionaries that meet this criteria, we use the native array type.
    //
    Slice::BuiltinPtr b = Slice::BuiltinPtr::dynamicCast(type);
    if(b)
    {
        switch(b->kind())
        {
        case Slice::Builtin::KindByte:
        case Slice::Builtin::KindBool: // We allow bool even though PHP doesn't support it directly.
        case Slice::Builtin::KindShort:
        case Slice::Builtin::KindInt:
        case Slice::Builtin::KindLong:
        case Slice::Builtin::KindString:
            return true;

        case Slice::Builtin::KindFloat:
        case Slice::Builtin::KindDouble:
        case Slice::Builtin::KindObject:
        case Slice::Builtin::KindObjectProxy:
        case Slice::Builtin::KindLocalObject:
            break;
        }
    }

    return false;
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
