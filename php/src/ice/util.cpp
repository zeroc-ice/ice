// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "ice_util.h"
#include <algorithm>
#include <ctype.h>

using namespace std;

static string
lookupKwd(const string& name)
{
    string lower = ice_lowerCase(name); // PHP is case insensitive.

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

#ifdef WIN32
extern "C"
#endif
static void
dtor_wrapper(void* p)
{
    zval_ptr_dtor((zval**)p);
}

ice_object*
ice_newObject(zend_class_entry* ce TSRMLS_DC)
{
    ice_object* obj;
    zval* tmp;

    obj = static_cast<ice_object*>(emalloc(sizeof(ice_object)));
    obj->zobj.ce = ce;
    obj->zobj.in_get = 0;
    obj->zobj.in_set = 0;
    obj->ptr = 0;

    obj->zobj.properties = static_cast<HashTable*>(emalloc(sizeof(HashTable)));
    zend_hash_init(obj->zobj.properties, 0, NULL, dtor_wrapper, 0);
    zend_hash_copy(obj->zobj.properties, &ce->default_properties, (copy_ctor_func_t) zval_add_ref, &tmp,
                   sizeof(zval*));

    return obj;
}

ice_object*
ice_getObject(zval* zv TSRMLS_DC)
{
    if(!zv)
    {
        zend_error(E_ERROR, "method %s() must be invoked on an object", get_active_function_name(TSRMLS_C));
        return 0;
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(zv TSRMLS_CC));
    if(!obj)
    {
        zend_error(E_ERROR, "no object found in %s()", get_active_function_name(TSRMLS_C));
        return 0;
    }

    return obj;
}

bool
ice_splitString(const string& str, vector<string>& args)
{
    string delim = " \t\n\r";
    string::size_type beg;
    string::size_type end = 0;
    while(true)
    {
        beg = str.find_first_not_of(delim, end);
        if(beg == string::npos)
        {
            break;
        }

        //
        // Check for quoted argument.
        //
        char ch = str[beg];
        if(ch == '"' || ch == '\'')
        {
            beg++;
            end = str.find(ch, beg);
            if(end == string::npos)
            {
                zend_error(E_ERROR, "unterminated quote in `%s'", str.c_str());
                return false;
            }
            args.push_back(str.substr(beg, end - beg));
            end++; // Skip end quote.
        }
        else
        {
            end = str.find_first_of(delim + "'\"", beg);
            if(end == string::npos)
            {
                end = str.length();
            }
            args.push_back(str.substr(beg, end - beg));
        }
    }

    return true;
}

string
ice_lowerCase(const string& s)
{
    string result(s);
    transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

string
ice_flatten(const string& scoped)
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

    return ice_fixIdent(result);
}

string
ice_fixIdent(const string& ident)
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
ice_zendTypeToString(int type)
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
