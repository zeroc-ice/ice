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
#include <sstream>

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

    string result;

    Builtin_ptr builtin = Builtin_ptr::dynamicCast(type);
    if(builtin)
	result = "<type>" + string(builtinTable[builtin -> kind()])
	    + "</type>";

    Proxy_ptr proxy = Proxy_ptr::dynamicCast(type);
    if(proxy)
	result = "<classname>" + proxy -> _class() -> scoped().substr(2) +
	    "*</classname>";

    ClassDecl_ptr cl = ClassDecl_ptr::dynamicCast(type);
    if(cl)
	result = "<classname>" + cl -> scoped().substr(2) + "</classname>";

    Contained_ptr contained = Contained_ptr::dynamicCast(type);
    if(contained)
    {
	if(result.empty())
	    result = "<type>" + contained -> scoped().substr(2) + "</type>";

	result = "<link linkend=" + scopedToId(contained -> scoped()) + ">"
	    + result + "</link>";
    }

    if(result.empty())
       result = "???";

    return result;
}

string
Slice::addLink(const string& s, const Container_ptr& container)
{
    TypeList types = container -> lookupType(s, false);
    if(!types.empty())
    {
	string result;

	if(ClassDecl_ptr::dynamicCast(types.front()))
	    result = "<classname>" + s + "</classname>";
	else
	    result = "<type>" + s + "</type>";

	Contained_ptr p = Contained_ptr::dynamicCast(types.front());
	if(p)
	    result = "<link linkend=" + scopedToId(p -> scoped()) + ">"
		+ result + "</link>";

	return result;
    }

    ContainedList contList = container -> lookupContained(s, false);
    if(!contList.empty())
    {
	string result = "<link linkend=" +
	    scopedToId(contList.front() -> scoped()) + ">";

	if(Module_ptr::dynamicCast(contList.front()))
	    result += "<classname>" + s + "</classname>";
	else if(Operation_ptr::dynamicCast(contList.front()))
	    result += "<function>" + s + "</function>";
	else if(DataMember_ptr::dynamicCast(contList.front()))
	    result += "<structfield>" + s + "</structfield>";
	else
	    assert(false);

	result += "</link>";
	return result;
    }

    return s;
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

string
Slice::scopedToId(const string& scoped)
{
    static map<string, int> idMap;
    static int nextId_ = 0;

    string s;
    if(scoped[0] == ':')
	s = scoped.substr(2);
    else
	s = scoped;

    int id = idMap[s];
    if(id == 0)
    {
	id = ++nextId_;
	idMap[s] = id;
    }

    ostringstream result;
    result << "\"slice2docbook." << id << '"';
    return result.str();
}
