// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

//
// TODO: This is the same stuff as in ../slice2cpp/GenUtil.cpp. This
// needs to be put into a library!
//

#include <GenUtil.h>

using namespace std;
using namespace Slice;

string
Slice::typeToString(const TypePtr& type)
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
	"::Ice::ObjectPtr",
	"::Ice::ObjectPrx",
	"::Ice::LocalObjectPtr"
    };

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if (builtin)
    {
	return builtinTable[builtin->kind()];
    }

    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
    if (cl)
    {
	return cl->scoped() + "Ptr";
    }
	    
    ProxyPtr proxy = ProxyPtr::dynamicCast(type);
    if (proxy)
    {
	return proxy->_class()->scoped() + "Prx";
    }
	    
    ContainedPtr contained = ContainedPtr::dynamicCast(type);
    if (contained)
    {
	return contained->scoped();
    }
	    
    return "???";
}

string
Slice::returnTypeToString(const TypePtr& type)
{
    if (!type)
    {
	return "void";
    }

    return typeToString(type);
}

string
Slice::inputTypeToString(const TypePtr& type)
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
	"const ::Ice::ObjectPtr&",
	"const ::Ice::ObjectPrx&",
	"const ::Ice::LocalObjectPtr&"
    };

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if (builtin)
    {
	return inputBuiltinTable[builtin->kind()];
    }

    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
    if (cl)
    {
	return "const " + cl->scoped() + "Ptr&";
    }
	    
    ProxyPtr proxy = ProxyPtr::dynamicCast(type);
    if (proxy)
    {
	return "const " + proxy->_class()->scoped() + "Prx&";
    }
	    
    EnumPtr en = EnumPtr::dynamicCast(type);
    if (en)
    {
	return en->scoped();
    }
	    
    NativePtr native = NativePtr::dynamicCast(type);
    if (native)
    {
	return native->scoped();
    }
	    
    ContainedPtr contained = ContainedPtr::dynamicCast(type);
    if (contained)
    {
	return "const " + contained->scoped() + "&";
    }

    return "???";
}

void
Slice::writeMarshalUnmarshalCode(Output& out, const TypePtr& type, const string& param, bool marshal)
{
    const char* func = marshal ? "write(" : "read(";
    const char* stream = marshal ? "__os" : "__is";

    if (BuiltinPtr::dynamicCast(type))
    {
	out << nl << stream << "->" << func << param << ");";
	return;
    }
    
    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
    if (cl)
    {
	out << sb;
	if (marshal)
	{
	    out << nl << "::Ice::ObjectPtr __obj = " << param << ';';
	    out << nl << stream << "->write(__obj);";
	}
	else
	{
	    out << nl << "::Ice::ObjectPtr __obj;";
	    out << nl << stream << "->read(__obj, " << cl->scoped() << "::__classIds[0]);";
	    out << nl << "if (!__obj)";
	    out << sb;
	    ClassDefPtr def = cl->definition();
	    if (def && !def->isAbstract())
	    {
		out << nl << "__obj = new " << cl->scoped() << ";";
		out << nl << "__obj->__read(__is);";
	    }
	    else
	    {
		out << nl << "throw ::Ice::NoServantFactoryException(__FILE__, __LINE__);";
	    }
	    out << eb;
	    out << nl << param << " = " << cl->scoped() << "Ptr::dynamicCast(__obj);";
	    out << nl << "if (!" << param << ')';
	    out << sb;
	    out << nl << "throw ::Ice::ServantUnmarshalException(__FILE__, __LINE__);";
	    out << eb;
	}
	out << eb;

	return;
    }
    
    StructPtr st = StructPtr::dynamicCast(type);
    if (st)
    {
	out << nl << param << ".__" << func << stream << ");";
	return;
    }
    
    SequencePtr seq = SequencePtr::dynamicCast(type);
    if (seq)
    {
	if (BuiltinPtr::dynamicCast(seq->type()))
	{
	    out << nl << stream << "->" << func << param << ");";
	}
	else
	{
	    out << nl << seq->scope() << "__" << func << stream << ", " << param << ", " << seq->scope()
		<< "__U__" << seq->name() << "());";
	}
	return;
    }
    
    DictionaryPtr dict = DictionaryPtr::dynamicCast(type);
    if (dict)
    {
	out << nl << dict->scope() << "__" << func << stream << ", " << param << ", " << dict->scope()
	    << "__U__" << dict->name() << "());";
	return;
    }
    
    NativePtr native = NativePtr::dynamicCast(type);
    assert(!native); // TODO
    
    ConstructedPtr constructed = ConstructedPtr::dynamicCast(type);
    if (!constructed)
    {
	ProxyPtr proxy = ProxyPtr::dynamicCast(type);
	assert(proxy);
	constructed = proxy->_class();
    }

    out << nl << constructed->scope() << "__" << func << stream << ", " << param << ");";
}
