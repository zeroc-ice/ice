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
	"::Ice::Byte",
	"bool",
	"::Ice::Short",
	"::Ice::Int",
	"::Ice::Long",
	"::Ice::Float",
	"::Ice::Double",
	"::std::string",
	"::std::wstring",
	"::Ice::Object_ptr",
	"::Ice::Object_prx",
	"::Ice::LocalObject_ptr"
    };

    Builtin_ptr builtin = Builtin_ptr::dynamicCast(type);
    if(builtin)
	return builtinTable[builtin -> kind()];

    ClassDecl_ptr cl = ClassDecl_ptr::dynamicCast(type);
    if(cl)
	return cl -> scoped() + "_ptr";
	    
    Proxy_ptr proxy = Proxy_ptr::dynamicCast(type);
    if(proxy)
	return proxy -> _class() -> scoped() + "_prx";
	    
    Contained_ptr contained = Contained_ptr::dynamicCast(type);
    if(contained)
	return contained -> scoped();
	    
    return "???";
}

string
Slice::returnTypeToString(const Type_ptr& type)
{
    if(!type)
	return "void";

    return typeToString(type);
}

string
Slice::inputTypeToString(const Type_ptr& type)
{
    static const char* inputBuiltinTable[] =
    {
	"::Ice::Byte",
	"bool",
	"::Ice::Short",
	"::Ice::Int",
	"::Ice::Long",
	"::Ice::Float",
	"::Ice::Double",
	"const ::std::string&",
	"const ::std::wstring&",
	"const ::Ice::Object_ptr&",
	"const ::Ice::Object_prx&",
	"const ::Ice::LocalObject_ptr&"
    };

    Builtin_ptr builtin = Builtin_ptr::dynamicCast(type);
    if(builtin)
	return inputBuiltinTable[builtin -> kind()];

    ClassDecl_ptr cl = ClassDecl_ptr::dynamicCast(type);
    if(cl)
	return "const " + cl -> scoped() + "_ptr&";
	    
    Proxy_ptr proxy = Proxy_ptr::dynamicCast(type);
    if(proxy)
	return "const " + proxy -> _class() -> scoped() + "_prx&";
	    
    Enum_ptr en = Enum_ptr::dynamicCast(type);
    if(en)
	return en -> scoped();
	    
    Native_ptr native = Native_ptr::dynamicCast(type);
    if(native)
	return native -> scoped();
	    
    Contained_ptr contained = Contained_ptr::dynamicCast(type);
    if(contained)
	return "const " + contained -> scoped() + "&";

    return "???";
}

string
Slice::outputTypeToString(const Type_ptr& type)
{
    static const char* outputBuiltinTable[] =
    {
	"::Ice::Byte&",
	"bool&",
	"::Ice::Short&",
	"::Ice::Int&",
	"::Ice::Long&",
	"::Ice::Float&",
	"::Ice::Double&",
	"::std::string&",
	"::std::wstring&",
	"::Ice::Object_ptr",
	"::Ice::Object_prx",
	"::Ice::LocalObject_ptr"
    };
    
    Builtin_ptr builtin = Builtin_ptr::dynamicCast(type);
    if(builtin)
	return outputBuiltinTable[builtin -> kind()];

    ClassDecl_ptr cl = ClassDecl_ptr::dynamicCast(type);
    if(cl)
	return cl -> scoped() + "_ptr&";
	    
    Proxy_ptr proxy = Proxy_ptr::dynamicCast(type);
    if(proxy)
	return proxy -> _class() -> scoped() + "_prx&";
	    
    Native_ptr native = Native_ptr::dynamicCast(type);
    if(native)
	return native -> scoped();
	    
    Contained_ptr contained = Contained_ptr::dynamicCast(type);
    if(contained)
	return contained -> scoped() + "&";

    return "???";
}

void
Slice::writeMarshalUnmarshalCode(Output& out, const Type_ptr& type,
				 const string& param, bool marshal)
{
    const char* func = marshal ? "write(" : "read(";
    const char* stream = marshal ? "__os" : "__is";

    if(Builtin_ptr::dynamicCast(type))
    {
	out << nl << stream << " -> " << func << param << ");";
	return;
    }
    
    ClassDecl_ptr cl = ClassDecl_ptr::dynamicCast(type);
    if(cl)
    {
	out << sb;
	if(marshal)
	{
	    out << nl << "::Ice::Object_ptr __obj = " << param << ';';
	    out << nl << stream << " -> write(__obj);";
	}
	else
	{
	    out << nl << "::Ice::Object_ptr __obj;";
	    out << nl << stream << " -> read(__obj, " << cl -> scoped()
		<< "::__implements[0]);";
	    out << nl << "if(!__obj)";
	    ClassDef_ptr def = cl -> definition();
	    if(def && !def -> abstract())
	    {
		out << sb;
		out << nl << "__obj = new " << cl -> scoped() << ";";
		out << nl << "__obj -> __read(__is);";
		out << eb;
	    }
	    else
	    {
		out.inc();
		out << nl
		    << "throw ::Ice::NoFactoryException(__FILE__, __LINE__);";
		out.dec();
	    }
	    out << nl << param << " = " << cl -> scoped()
		<< "_ptr::dynamicCast(__obj);";
	    out << nl << "if(!" << param << ')';
	    out.inc();
	    out << nl
		<< "throw ::Ice::ValueUnmarshalException(__FILE__, __LINE__);";
	    out.dec();
	}
	out << eb;

	return;
    }
    
    Vector_ptr vec = Vector_ptr::dynamicCast(type);
    if(vec)
    {
	if(Builtin_ptr::dynamicCast(vec -> type()))
	    out << nl << stream << " -> " << func << param << ");";
	else
	    out << nl << vec -> scope() << "::__" << func << stream\
		<< ", "	<< param << ", " << vec -> scope()
		<< "::__U__" << vec -> name() << "());";
	return;
    }
    
    Native_ptr native = Native_ptr::dynamicCast(type);
    assert(!native); // TODO
    
    Constructed_ptr constructed = Constructed_ptr::dynamicCast(type);
    if(!constructed)
    {
	Proxy_ptr proxy = Proxy_ptr::dynamicCast(type);
	assert(proxy);
	constructed = proxy -> _class();
    }

    out << nl << constructed -> scope() << "::__" << func << stream << ", "
	<< param << ");";
}

void
Slice::writeMarshalCode(Output& out,
			const list<pair<Type_ptr, string> >& params,
			const Type_ptr& ret)
{
    list<pair<Type_ptr, string> >::const_iterator p;
    for(p = params.begin(); p != params.end(); ++p)
	writeMarshalUnmarshalCode(out, p -> first, p -> second, true);
    if(ret)
	writeMarshalUnmarshalCode(out, ret, "__ret", true);
}

void
Slice::writeUnmarshalCode(Output& out,
			    const list<pair<Type_ptr, string> >& params,
			    const Type_ptr& ret)
{
    list<pair<Type_ptr, string> >::const_iterator p;
    for(p = params.begin(); p != params.end(); ++p)
	writeMarshalUnmarshalCode(out, p -> first, p -> second, false);
    if(ret)
	writeMarshalUnmarshalCode(out, ret, "__ret", false);
}

void
Slice::writeAllocateCode(Output& out,
			 const list<pair<Type_ptr, string> >& params,
			 const Type_ptr& ret)
{
    list<pair<Type_ptr, string> > ps = params;
    if(ret)
	ps.push_back(make_pair(ret, string("__ret")));

    list<pair<Type_ptr, string> >::const_iterator p;
    for(p = ps.begin(); p != ps.end(); ++p)
	out << nl << typeToString(p -> first) << ' ' << p -> second << ';';
}
