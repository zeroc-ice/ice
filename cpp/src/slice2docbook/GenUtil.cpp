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

    string result = "<type>";
    
    Builtin_ptr builtin = Builtin_ptr::dynamicCast(type);
    if(builtin)
	result += builtinTable[builtin -> kind()];
    else
    {
	ClassDecl_ptr cl = ClassDecl_ptr::dynamicCast(type);
	if(cl)
	    result += cl -> scoped().substr(2);
	else
	{	    
	    Proxy_ptr proxy = Proxy_ptr::dynamicCast(type);
	    if(proxy)
		result += proxy -> _class() -> scoped().substr(2) + "*";
	    else
	    {
		Contained_ptr contained = Contained_ptr::dynamicCast(type);
		if(contained)
		    result += contained -> scoped().substr(2);
		else
		    result += "???";
	    }
	}
    }

    result += "</type>";
    return result;
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
