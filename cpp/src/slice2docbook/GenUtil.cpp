// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <GenUtil.h>

using namespace std;
using namespace Slice;

string
Slice::typeToString(const Type_ptr& type)
{
    static const char* builtinTable[] =
    {
	"byte",
	"bool",
	"short",
	"int",
	"long",
	"float",
	"double",
	"string",
	"wstring",
	"Object",
	"Object*",
	"LocalObject"
    };

    Builtin_ptr builtin = Builtin_ptr::dynamicCast(type);
    if(builtin)
	return builtinTable[builtin -> kind()];

    ClassDecl_ptr cl = ClassDecl_ptr::dynamicCast(type);
    if(cl)
	return cl -> scoped().substr(2);
	    
    Proxy_ptr proxy = Proxy_ptr::dynamicCast(type);
    if(proxy)
	return proxy -> _class() -> scoped().substr(2) + "*";
	    
    Contained_ptr contained = Contained_ptr::dynamicCast(type);
    if(contained)
	return contained -> scoped().substr(2);
	    
    return "???";
}


struct ToFile
{
    char operator()(char c)
    {
	if(c == ':')
	    return '_';
	else
	    return c;
    }
};

string
Slice::scopedToFile(const string& scoped)
{
    string result;
    if(scoped[0] == ':')
	result = scoped.substr(2);
    else
	result = scoped;
    transform(result.begin(), result.end(), result.begin(), ToFile());
    result += ".sgml";
    return result;    
}
