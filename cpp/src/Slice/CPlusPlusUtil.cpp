// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Slice/CPlusPlusUtil.h>

using namespace std;
using namespace Slice;

char
Slice::ToIfdef::operator()(char c)
{
    if (!isalnum(c))
    {
	return '_';
    }
    else
    {
	return c;
    }
}

string
Slice::changeInclude(const string& orig, const vector<string>& includePaths)
{
    string file = orig;

    for (vector<string>::const_iterator p = includePaths.begin(); p != includePaths.end(); ++p)
    {
	if (orig.compare(0, p->length(), *p) == 0)
	{
	    string s = orig.substr(p->length());
	    if (s.size() < file.size())
	    {
		file = s;
	    }
	}
    }

    string::size_type pos = file.rfind('.');
    if (pos != string::npos)
    {
	file.erase(pos);
    }

    return file;
}

void
Slice::printHeader(Output& out)
{
    static const char* header = 
"// **********************************************************************\n"
"//\n"
"// Copyright (c) 2001\n"
"// MutableRealms, Inc.\n"
"// Huntsville, AL, USA\n"
"//\n"
"// All Rights Reserved\n"
"//\n"
"// **********************************************************************\n"
	;

    out << header;
    out << "\n// Ice version " << ICE_STRING_VERSION;
}

void
Slice::printVersionCheck(Output& out)
{
    out << "\n";
    out << "\n#ifndef ICE_IGNORE_VERSION";
    out << "\n#   if ICE_INT_VERSION != 0x" << hex << ICE_INT_VERSION;
    out << "\n#       error Ice version mismatch!";
    out << "\n#   endif";
    out << "\n#endif";
}

void
Slice::printDllExportStuff(Output& out, const string& dllExport)
{
    if (dllExport.size())
    {
	out << sp;
	out << "\n#ifdef WIN32";
	out << "\n#   ifdef " << dllExport << "_EXPORTS";
	out << "\n#       define " << dllExport << " __declspec(dllexport)";
	out << "\n#   else";
	out << "\n#       define " << dllExport << " __declspec(dllimport)";
	out << "\n#   endif";
	out << "\n#else";
	out << "\n#   define " << dllExport << " /**/";
	out << "\n#endif";
    }
}

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
	    
    ContainedPtr contained = ContainedPtr::dynamicCast(type);
    if (contained)
    {
	return "const " + contained->scoped() + "&";
    }

    return "???";
}

string
Slice::outputTypeToString(const TypePtr& type)
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
	"::Ice::ObjectPtr&",
	"::Ice::ObjectPrx&",
	"::Ice::LocalObjectPtr&"
    };
    
    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if (builtin)
    {
	return outputBuiltinTable[builtin->kind()];
    }

    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
    if (cl)
    {
	return cl->scoped() + "Ptr&";
    }
	    
    ProxyPtr proxy = ProxyPtr::dynamicCast(type);
    if (proxy)
    {
	return proxy->_class()->scoped() + "Prx&";
    }
	    
    ContainedPtr contained = ContainedPtr::dynamicCast(type);
    if (contained)
    {
	return contained->scoped() + "&";
    }

    return "???";
}

void
Slice::writeMarshalUnmarshalCode(Output& out, const TypePtr& type, const string& param, bool marshal,
				 const string& str, bool pointer)
{
    string stream;
    if (str.empty())
    {
	stream = marshal ? "__os" : "__is";
    }
    else
    {
	stream = str;
    }
    
    string deref;
    if (pointer)
    {
	deref = "->";
    }
    else
    {
	deref = '.';
    }
    
    string obj;
    if (stream.find("__") == 0)
    {
	obj = "__obj";
    }
    else
    {
	obj = "obj;";
    }

    string func = marshal ? "write(" : "read(";

    if (BuiltinPtr::dynamicCast(type))
    {
	out << nl << stream << deref << func << param << ");";
	return;
    }
    
    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
    if (cl)
    {
	out << sb;
	if (marshal)
	{
	    out << nl << "::Ice::ObjectPtr " << obj << " = " << param << ';';
	    out << nl << stream << deref << "write(" << obj << ");";
	}
	else
	{
	    out << nl << "::Ice::ObjectPtr " << obj << ';';
	    out << nl << stream << deref << "read(" << obj << ", " << cl->scoped() << "::__classIds[0]);";
	    out << nl << "if (!" << obj << ')';
	    out << sb;
	    ClassDefPtr def = cl->definition();
	    if (def && !def->isAbstract())
	    {
		out << nl << obj << " = new " << cl->scoped() << ';';
		out << nl << obj << "->__read(" << (pointer ? "" : "&") << stream << ");";
	    }
	    else
	    {
		out << nl << "throw ::Ice::NoServantFactoryException(__FILE__, __LINE__);";
	    }
	    out << eb;
	    out << nl << param << " = " << cl->scoped() << "Ptr::dynamicCast(" << obj << ");";
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
	out << nl << param << ".__" << func << (pointer ? "" : "&") << stream << ");";
	return;
    }
    
    SequencePtr seq = SequencePtr::dynamicCast(type);
    if (seq)
    {
	if (BuiltinPtr::dynamicCast(seq->type()))
	{
	    out << nl << stream << deref << func << param << ");";
	}
	else
	{
	    out << nl << seq->scope() << "__" << func << (pointer ? "" : "&") << stream << ", " << param << ", "
		<< seq->scope() << "__U__" << seq->name() << "());";
	}
	return;
    }
    
    DictionaryPtr dict = DictionaryPtr::dynamicCast(type);
    if (dict)
    {
	out << nl << dict->scope() << "__" << func << (pointer ? "" : "&") << stream << ", " << param << ", "
	    << dict->scope() << "__U__" << dict->name() << "());";
	return;
    }
    
    ConstructedPtr constructed = ConstructedPtr::dynamicCast(type);
    if (!constructed)
    {
	ProxyPtr proxy = ProxyPtr::dynamicCast(type);
	assert(proxy);
	constructed = proxy->_class();
    }

    out << nl << constructed->scope() << "__" << func << (pointer ? "" : "&") << stream << ", " << param << ");";
}

void
Slice::writeMarshalCode(Output& out, const list<pair<TypePtr, string> >& params, const TypePtr& ret)
{
    for (list<pair<TypePtr, string> >::const_iterator p = params.begin(); p != params.end(); ++p)
    {
	writeMarshalUnmarshalCode(out, p->first, p->second, true);
    }
    if (ret)
    {
	writeMarshalUnmarshalCode(out, ret, "__ret", true);
    }
}

void
Slice::writeUnmarshalCode(Output& out, const list<pair<TypePtr, string> >& params, const TypePtr& ret)
{
    for (list<pair<TypePtr, string> >::const_iterator p = params.begin(); p != params.end(); ++p)
    {
	writeMarshalUnmarshalCode(out, p->first, p->second, false);
    }
    if (ret)
    {
	writeMarshalUnmarshalCode(out, ret, "__ret", false);
    }
}

void
Slice::writeAllocateCode(Output& out, const list<pair<TypePtr, string> >& params, const TypePtr& ret)
{
    list<pair<TypePtr, string> > ps = params;
    if (ret)
    {
	ps.push_back(make_pair(ret, string("__ret")));
    }

    for (list<pair<TypePtr, string> >::const_iterator p = ps.begin(); p != ps.end(); ++p)
    {
	out << nl << typeToString(p->first) << ' ' << p->second << ';';
    }
}
