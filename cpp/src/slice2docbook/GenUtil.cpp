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
Slice::getScopedMinimized(const ContainedPtr& contained, const ContainerPtr& container)
{
    string s = contained->scoped();
    ContainerPtr p = container;
    ContainedPtr q;

    while((q = ContainedPtr::dynamicCast(p)))
    {
	string s2 = q->scoped();
	s2 += "::";

	if (s.find(s2) == 0)
	{
	    return s.substr(s2.size());
	}

	p = q->container();
    }

    return s;
}

string
Slice::toString(const SyntaxTreeBasePtr& p, const ContainerPtr& container)
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

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(p);
    if (builtin)
    {
	return "<type>" + string(builtinTable[builtin->kind()]) + "</type>";
    }

    string tag;
    string linkend;
    string s;

    ProxyPtr proxy = ProxyPtr::dynamicCast(p);
    if (proxy)
    {
	tag = "classname";
	linkend = scopedToId(proxy->_class()->scoped());
	s = getScopedMinimized(proxy->_class(), container);
	s += "*";
    }

    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(p);
    if (cl)
    {
	tag = "classname";
	linkend = scopedToId(cl->scoped());
	s = getScopedMinimized(cl, container);
    }

    if (s.empty())
    {
	ContainedPtr contained = ContainedPtr::dynamicCast(p);
	assert(contained);
	tag = "type";
	linkend = scopedToId(contained->scoped());
	s = getScopedMinimized(contained, container);
    }

    return "<link linkend=" + linkend + "><" + tag + ">" + s + "</" + tag + "></link>";
}

string
Slice::toString(const string& str, const ContainerPtr& container)
{
    string s = str;

    TypeList types = container->lookupType(s, false);
    if (!types.empty())
    {
	return toString(types.front(), container);
    }

    ContainedList contList = container->lookupContained(s, false);
    if (!contList.empty())
    {
	return toString(contList.front(), container);
    }

    return s;
}

struct ToFile
{
    char operator()(char c)
    {
	if (c == ':')
	{
	    return '_';
	}
	else
	{
	    return c;
	}
    }
};

string
Slice::scopedToId(const string& scoped)
{
    static map<string, int> idMap;
    static int _nextId = 0;

    string s;
    if (scoped[0] == ':')
    {
	s = scoped.substr(2);
    }
    else
    {
	s = scoped;
    }

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
