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
Slice::typeToString(const TypePtr& type)
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

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if (builtin)
	result = "<type>" + string(builtinTable[builtin->kind()]) + "</type>";

    ProxyPtr proxy = ProxyPtr::dynamicCast(type);
    if (proxy)
	result = "<classname>" + proxy->_class()->scoped().substr(2) + "*</classname>";

    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
    if (cl)
	result = "<classname>" + cl->scoped().substr(2) + "</classname>";

    ContainedPtr contained = ContainedPtr::dynamicCast(type);
    if (contained)
    {
	if (result.empty())
	    result = "<type>" + contained->scoped().substr(2) + "</type>";

	result = "<link linkend=" + scopedToId(contained->scoped()) + ">" + result + "</link>";
    }

    if (result.empty())
       result = "???";

    return result;
}

string
Slice::addLink(const string& s, const ContainerPtr& container)
{
    TypeList types = container->lookupType(s, false);
    if (!types.empty())
    {
	string result;

	if (ClassDeclPtr::dynamicCast(types.front()))
	    result = "<classname>" + s + "</classname>";
	else
	    result = "<type>" + s + "</type>";

	ContainedPtr p = ContainedPtr::dynamicCast(types.front());
	if (p)
	    result = "<link linkend=" + scopedToId(p->scoped()) + ">" + result + "</link>";

	return result;
    }

    ContainedList contList = container->lookupContained(s, false);
    if (!contList.empty())
    {
	string result = "<link linkend=" + scopedToId(contList.front()->scoped()) + ">";

	if (ModulePtr::dynamicCast(contList.front()))
	    result += "<classname>" + s + "</classname>";
	else if (OperationPtr::dynamicCast(contList.front()))
	    result += "<function>" + s + "</function>";
	else if (DataMemberPtr::dynamicCast(contList.front()))
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
	if (c == ':')
	    return '_';
	else
	    return c;
    }
};

string
Slice::scopedToFile(const string& scoped)
{
    string result;
    if (scoped[0] == ':')
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
    static int _nextId = 0;

    string s;
    if (scoped[0] == ':')
	s = scoped.substr(2);
    else
	s = scoped;

    int id = idMap[s];
    if (id == 0)
    {
	id = ++_nextId;
	idMap[s] = id;
    }

    ostringstream result;
    result << "\"slice2docbook." << id << '"';
    return result.str();
}
