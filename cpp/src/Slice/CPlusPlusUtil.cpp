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

#include <Slice/CPlusPlusUtil.h>

using namespace std;
using namespace Slice;
using namespace IceUtil;

char
Slice::ToIfdef::operator()(char c)
{
    if(!isalnum(c))
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
    replace(file.begin(), file.end(), '\\', '/');
    
    for(vector<string>::const_iterator p = includePaths.begin(); p != includePaths.end(); ++p)
    {
	string includePath = *p;
	replace(includePath.begin(), includePath.end(), '\\', '/');
	
	if(orig.compare(0, includePath.length(), *p) == 0)
	{
	    string s = orig.substr(includePath.length());
	    if(s.size() < file.size())
	    {
		file = s;
	    }
	}
    }

    string::size_type pos = file.rfind('.');
    if(pos != string::npos)
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
"// Copyright (c) 2003\n"
"// ZeroC, Inc.\n"
"// Billerica, MA, USA\n"
"//\n"
"// All Rights Reserved.\n"
"//\n"
"// Ice is free software; you can redistribute it and/or modify it under\n"
"// the terms of the GNU General Public License version 2 as published by\n"
"// the Free Software Foundation.\n"
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
    out << "\n#   if ICE_INT_VERSION != " << ICE_INT_VERSION;
    out << "\n#       error Ice version mismatch!";
    out << "\n#   endif";
    out << "\n#endif";
}

void
Slice::printDllExportStuff(Output& out, const string& dllExport)
{
    if(dllExport.size())
    {
	out << sp;
	out << "\n#ifndef " << dllExport;
	out << "\n#   ifdef " << dllExport << "_EXPORTS";
	out << "\n#       define " << dllExport << " ICE_DECLSPEC_EXPORT";
	out << "\n#   else";
	out << "\n#       define " << dllExport << " ICE_DECLSPEC_IMPORT";
	out << "\n#   endif";
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
	"::Ice::ObjectPtr",
	"::Ice::ObjectPrx",
	"::Ice::LocalObjectPtr"
    };

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if(builtin)
    {
	return builtinTable[builtin->kind()];
    }

    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
    if(cl)
    {
	return fixKwd(cl->scoped()) + "Ptr";
    }
	    
    ProxyPtr proxy = ProxyPtr::dynamicCast(type);
    if(proxy)
    {
	return fixKwd(proxy->_class()->scoped()) + "Prx";
    }
	    
    ContainedPtr contained = ContainedPtr::dynamicCast(type);
    if(contained)
    {
	return fixKwd(contained->scoped());
    }

    EnumPtr en = EnumPtr::dynamicCast(type);
    if(en)
    {
	return fixKwd(en->scoped());
    }
	    
    return "???";
}

string
Slice::returnTypeToString(const TypePtr& type)
{
    if(!type)
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
	"const ::Ice::ObjectPtr&",
	"const ::Ice::ObjectPrx&",
	"const ::Ice::LocalObjectPtr&"
    };

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if(builtin)
    {
	return inputBuiltinTable[builtin->kind()];
    }

    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
    if(cl)
    {
	return "const " + fixKwd(cl->scoped()) + "Ptr&";
    }
	    
    ProxyPtr proxy = ProxyPtr::dynamicCast(type);
    if(proxy)
    {
	return "const " + fixKwd(proxy->_class()->scoped()) + "Prx&";
    }
	    
    EnumPtr en = EnumPtr::dynamicCast(type);
    if(en)
    {
	return fixKwd(en->scoped());
    }
	    
    ContainedPtr contained = ContainedPtr::dynamicCast(type);
    if(contained)
    {
	return "const " + fixKwd(contained->scoped()) + "&";
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
	"::Ice::ObjectPtr&",
	"::Ice::ObjectPrx&",
	"::Ice::LocalObjectPtr&"
    };
    
    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if(builtin)
    {
	return outputBuiltinTable[builtin->kind()];
    }

    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
    if(cl)
    {
	return fixKwd(cl->scoped()) + "Ptr&";
    }
	    
    ProxyPtr proxy = ProxyPtr::dynamicCast(type);
    if(proxy)
    {
	return fixKwd(proxy->_class()->scoped()) + "Prx&";
    }
	    
    ContainedPtr contained = ContainedPtr::dynamicCast(type);
    if(contained)
    {
	return fixKwd(contained->scoped()) + "&";
    }

    return "???";
}

//
// If the passed name is a keyword, return the name with a "_cxx_" prefix;
// otherwise, return the name unchanged.
//

static string
lookupKwd(const string& name)
{
    //
    // Keyword list. *Must* be kept in alphabetical order.
    //
    static const string keywordList[] = 
    {       
	"and", "and_eq", "asm", "auto", "bit_and", "bit_or", "bool", "break", "case", "catch", "char",
	"class", "compl", "const", "const_cast", "continue", "default", "delete", "do", "double",
	"dynamic_cast", "else", "enum", "explicit", "export", "extern", "false", "float", "for",
	"friend", "goto", "if", "inline", "int", "long", "mutable", "namespace", "new", "not", "not_eq",
	"operator", "or", "or_eq", "private", "protected", "public", "register", "reinterpret_cast",
	"return", "short", "signed", "sizeof", "static", "static_cast", "struct", "switch", "template",
	"this", "throw", "true", "try", "typedef", "typeid", "typename", "union", "unsigned", "using",
	"virtual", "void", "volatile", "wchar_t", "while", "xor", "xor_eq"
    };
    bool found =  binary_search(&keywordList[0],
	                        &keywordList[sizeof(keywordList) / sizeof(*keywordList)],
				name);
    return found ? "_cpp_" + name : name;
}

//
// Split a scoped name into its components and return the components as a list of (unscoped) identifiers.
//
static StringList
splitScopedName(const string& scoped)
{
    assert(scoped[0] == ':');
    StringList ids;
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

//
// If the passed name is a scoped name, return the identical scoped name,
// but with all components that are C++ keywords replaced by
// their "_cxx_"-prefixed version; otherwise, if the passed name is
// not scoped, but a C++ keyword, return the "_cxx_"-prefixed name;
// otherwise, return the name unchanged.
//
string
Slice::fixKwd(const string& name)
{
    if(name[0] != ':')
    {
	return lookupKwd(name);
    }
    StringList ids = splitScopedName(name);
    transform(ids.begin(), ids.end(), ids.begin(), ptr_fun(lookupKwd));
    stringstream result;
    for(StringList::const_iterator i = ids.begin(); i != ids.end(); ++i)
    {
	result << "::" + *i;
    }
    return result.str();
}

void
Slice::writeMarshalUnmarshalCode(Output& out, const TypePtr& type, const string& param, bool marshal,
				 const string& str, bool pointer)
{
    string fixedParam = fixKwd(param);

    string stream;
    if(str.empty())
    {
	stream = marshal ? "__os" : "__is";
    }
    else
    {
	stream = str;
    }
    
    string deref;
    if(pointer)
    {
	deref = "->";
    }
    else
    {
	deref = '.';
    }
    
    string func = marshal ? "write(" : "read(";

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if(builtin)
    {
	if(builtin->kind() == Builtin::KindObject)
	{
	    if(marshal)
	    {
		out << nl << stream << deref << func << fixedParam << ");";
	    }
	    else
	    {
		out << nl << stream << deref << func << "::Ice::__patch__ObjectPtr, &" << fixedParam << ");";
	    }
	    return;
	}
	else
	{
	    out << nl << stream << deref << func << fixedParam << ");";
	    return;
	}
    }
    
    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
    if(cl)
    {
	string scope = fixKwd(cl->scope());
	if(marshal)
	{
	    out << nl << scope << "__write(" << (pointer ? "" : "&") << stream << ", " << fixedParam << ");";
	}
	else
	{
	    out << nl << stream << deref << "read("
		<< scope << "__patch__" << fixKwd(cl->name()) << "Ptr, &" << fixedParam << ");";
	}

	return;
    }
    
    StructPtr st = StructPtr::dynamicCast(type);
    if(st)
    {
	out << nl << fixedParam << ".__" << func << (pointer ? "" : "&") << stream << ");";
	return;
    }
    
    SequencePtr seq = SequencePtr::dynamicCast(type);
    if(seq)
    {
	builtin = BuiltinPtr::dynamicCast(seq->type());
	if(builtin && builtin->kind() != Builtin::KindObject && builtin->kind() != Builtin::KindObjectProxy)
	{
	    out << nl << stream << deref << func << fixedParam << ");";
	}
	else
	{
	    string scope = fixKwd(seq->scope());
	    out << nl << scope << "__" << func << (pointer ? "" : "&") << stream << ", "
		<< fixedParam << ", " << scope << "__U__" << fixKwd(seq->name()) << "());";
	}
	return;
    }
    
    DictionaryPtr dict = DictionaryPtr::dynamicCast(type);
    if(dict)
    {
	string scope = fixKwd(dict->scope());
	out << nl << scope << "__" << func << (pointer ? "" : "&") << stream << ", "
	    << fixedParam << ", " << scope << "__U__" << fixKwd(dict->name()) << "());";
	return;
    }
    
    ConstructedPtr constructed = ConstructedPtr::dynamicCast(type);
    if(!constructed)
    {
	ProxyPtr proxy = ProxyPtr::dynamicCast(type);
	assert(proxy);
	constructed = proxy->_class();
    }

    out << nl << fixKwd(constructed->scope()) << "__" << func << (pointer ? "" : "&") << stream << ", "
	<< fixedParam << ");";
}

void
Slice::writeMarshalCode(Output& out, const list<pair<TypePtr, string> >& params, const TypePtr& ret)
{
    for(list<pair<TypePtr, string> >::const_iterator p = params.begin(); p != params.end(); ++p)
    {
	writeMarshalUnmarshalCode(out, p->first, p->second, true, "", true);
    }
    if(ret)
    {
	writeMarshalUnmarshalCode(out, ret, "__ret", true, "", true);
    }
}

void
Slice::writeUnmarshalCode(Output& out, const list<pair<TypePtr, string> >& params, const TypePtr& ret)
{
    for(list<pair<TypePtr, string> >::const_iterator p = params.begin(); p != params.end(); ++p)
    {
	writeMarshalUnmarshalCode(out, p->first, p->second, false, "", true);
    }
    if(ret)
    {
	writeMarshalUnmarshalCode(out, ret, "__ret", false, "", true);
    }
}

void
Slice::writeAllocateCode(Output& out, const list<pair<TypePtr, string> >& params, const TypePtr& ret)
{
    list<pair<TypePtr, string> > ps = params;
    if(ret)
    {
	ps.push_back(make_pair(ret, string("__ret")));
    }

    for(list<pair<TypePtr, string> >::const_iterator p = ps.begin(); p != ps.end(); ++p)
    {
	out << nl << typeToString(p->first) << ' ' << fixKwd(p->second) << ';';
    }
}
