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
    if(dllExport.size())
    {
	out << sp;
	out << "\n#ifdef _WIN32";
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
	return cl->scoped() + "Ptr";
    }
	    
    ProxyPtr proxy = ProxyPtr::dynamicCast(type);
    if(proxy)
    {
	return proxy->_class()->scoped() + "Prx";
    }
	    
    ContainedPtr contained = ContainedPtr::dynamicCast(type);
    if(contained)
    {
	return contained->scoped();
    }

    EnumPtr en = EnumPtr::dynamicCast(type);
    if(en)
    {
	return en->scoped();
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
	return "const " + cl->scoped() + "Ptr&";
    }
	    
    ProxyPtr proxy = ProxyPtr::dynamicCast(type);
    if(proxy)
    {
	return "const " + proxy->_class()->scoped() + "Prx&";
    }
	    
    EnumPtr en = EnumPtr::dynamicCast(type);
    if(en)
    {
	return en->scoped();
    }
	    
    ContainedPtr contained = ContainedPtr::dynamicCast(type);
    if(contained)
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
	return cl->scoped() + "Ptr&";
    }
	    
    ProxyPtr proxy = ProxyPtr::dynamicCast(type);
    if(proxy)
    {
	return proxy->_class()->scoped() + "Prx&";
    }
	    
    ContainedPtr contained = ContainedPtr::dynamicCast(type);
    if(contained)
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
    
    string obj;
    if(stream.find("__") == 0)
    {
	obj = "__obj";
    }
    else
    {
	obj = "obj";
    }

    string func = marshal ? "write(" : "read(";

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if(builtin)
    {
	if(builtin->kind() == Builtin::KindObject)
	{
	    if(marshal)
	    {
		out << nl << stream << deref << func << param << ");";
	    }
	    else
	    {
		out << nl << stream << deref << func << "::Ice::Object::ice_staticId(), 0, " << param << ");";
	    }
	    return;
	}
	else
	{
	    out << nl << stream << deref << func << param << ");";
	    return;
	}
    }
    
    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
    if(cl)
    {
	out << sb;
	if(marshal)
	{
	    out << nl << "::Ice::ObjectPtr " << obj << " = " << param << ';';
	    out << nl << stream << deref << func << obj << ");";
	}
	else
	{
	    out << nl << "::Ice::ObjectPtr " << obj << ';';
	    ClassDefPtr def = cl->definition();
	    string factory;
	    string type;
	    if(def && !def->isAbstract())
	    {
		factory = cl->scoped();
		factory += "::_factory";
		type = cl->scoped();
		type += "::ice_staticId()";
	    }
	    else
	    {
		factory = "0";
		type = "\"\"";
	    }
	    out << nl << stream << deref << func << type << ", " << factory << ", " << obj << ");";
	    out << nl << param << " = " << cl->scoped() << "Ptr::dynamicCast(" << obj << ");";
	}
	out << eb;

	return;
    }
    
    StructPtr st = StructPtr::dynamicCast(type);
    if(st)
    {
	out << nl << param << ".__" << func << (pointer ? "" : "&") << stream << ");";
	return;
    }
    
    SequencePtr seq = SequencePtr::dynamicCast(type);
    if(seq)
    {
	BuiltinPtr builtin = BuiltinPtr::dynamicCast(seq->type());
	if(builtin && builtin->kind() != Builtin::KindObject && builtin->kind() != Builtin::KindObjectProxy)
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
    if(dict)
    {
	out << nl << dict->scope() << "__" << func << (pointer ? "" : "&") << stream << ", " << param << ", "
	    << dict->scope() << "__U__" << dict->name() << "());";
	return;
    }
    
    ConstructedPtr constructed = ConstructedPtr::dynamicCast(type);
    if(!constructed)
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
    for(list<pair<TypePtr, string> >::const_iterator p = params.begin(); p != params.end(); ++p)
    {
	writeMarshalUnmarshalCode(out, p->first, p->second, true);
    }
    if(ret)
    {
	writeMarshalUnmarshalCode(out, ret, "__ret", true);
    }
}

void
Slice::writeUnmarshalCode(Output& out, const list<pair<TypePtr, string> >& params, const TypePtr& ret)
{
    for(list<pair<TypePtr, string> >::const_iterator p = params.begin(); p != params.end(); ++p)
    {
	writeMarshalUnmarshalCode(out, p->first, p->second, false);
    }
    if(ret)
    {
	writeMarshalUnmarshalCode(out, ret, "__ret", false);
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
	out << nl << typeToString(p->first) << ' ' << p->second << ';';
    }
}

void
Slice::writeGenericMarshalUnmarshalCode(Output& out, const TypePtr& type, const string& param, 
					bool marshal, const string& tn, const string& str, bool pointer)
{
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
    
    string obj;
    if(stream.find("__") == 0)
    {
	obj = "__obj";
    }
    else
    {
	obj = "obj";
    }

    static const char* outputBuiltinTable[] =
    {
	"Byte",
	"Bool",
	"Short",
	"Int",
	"Long",
	"Float",
	"Double",
	"String",
	"Object",
	"Proxy",
	"???"
    };
    string tagName;
    if(tn.empty())
    {
	tagName = "\"";
	tagName += param;
	tagName += "\"";
    }
    else
    {
	tagName = tn;
    }

    string streamFunc = marshal ? "write" : "read";
    string genFunc = marshal ? "ice_marshal(" : "ice_unmarshal(";

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if(builtin)
    {
	if(builtin->kind() == Builtin::KindObject)
	{
	    streamFunc += outputBuiltinTable[builtin->kind()];
	    if(marshal)
	    {
		out << nl << stream << deref << streamFunc << "(" << tagName << ", " << param << ");";
	    }
	    else
	    {
		out << nl << param << " = " << stream << deref << streamFunc << "(" << tagName
                    << ", ::Ice::Object::ice_staticId(), 0);";
	    }
	    return;
	}
	else
	{
            if(marshal)
            {
                out << nl << stream << deref << streamFunc << outputBuiltinTable[builtin->kind()]
                    << "(" << tagName << ", " << param << ");";
            }
            else
            {
                out << nl << param << " = " << stream << deref << streamFunc << outputBuiltinTable[builtin->kind()]
                    << "(" << tagName << ");";
            }
	    return;
	}
    }

    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
    if(cl)
    {
	out << sb;
	if(marshal)
	{
	    out << nl << "::Ice::ObjectPtr " << obj << " = " << param << ';';
	    out << nl << stream << deref << streamFunc << "Object(" << tagName << ", " << obj << ");";
	}
	else
	{
	    out << nl << "::Ice::ObjectPtr " << obj << ';';
	    ClassDefPtr def = cl->definition();
	    string factory;
	    string type;
	    if(def && !def->isAbstract())
	    {
		factory = cl->scoped();
		factory += "::_factory";
		type = cl->scoped();
		type += "::ice_staticId()";
	    }
	    else
	    {
		factory = "0";
		type = "\"\"";
	    }
	    out << nl << obj << " = " << stream << deref << streamFunc << "Object(" << tagName << ", "
		<< type << ", " << factory << ");";
	    out << nl << param << " = " << cl->scoped() << "Ptr::dynamicCast(" << obj << ");";
	}
	out << eb;

	return;
    }
    
    StructPtr st = StructPtr::dynamicCast(type);
    if(st)
    {
	out << nl << param << "." << genFunc << tagName << ", " << (pointer ? "" : "&") << stream << ");";
	return;
    }

    SequencePtr seq = SequencePtr::dynamicCast(type);
    if(seq)
    {
	BuiltinPtr builtin = BuiltinPtr::dynamicCast(seq->type());
	if(builtin && builtin->kind() != Builtin::KindObject && builtin->kind() != Builtin::KindObjectProxy)
	{
            if(marshal)
            {
                out << nl << stream << deref << streamFunc << outputBuiltinTable[builtin->kind()] << "Seq("
                    << tagName << ", " << param << ");";
            }
            else
            {
                out << nl << param << " = " << stream << deref << streamFunc << outputBuiltinTable[builtin->kind()]
                    << "Seq(" << tagName << ");";
            }
	}
	else
	{
	    out << nl << seq->scope() << genFunc << tagName << ", " << (pointer ? "" : "&") << stream
		<< ", " << param << ", " << seq->scope() << "__U__" << seq->name() << "());";
	}
	return;
    }

    DictionaryPtr dict = DictionaryPtr::dynamicCast(type);
    if(dict)
    {
	out << nl << dict->scope() << genFunc << tagName << ", " << (pointer ? "" : "&") << stream
	    << ", " << param << ", " << dict->scope() << "__U__" << dict->name() << "());";
	return;
    }
    
    ConstructedPtr constructed = ConstructedPtr::dynamicCast(type);
    if(!constructed)
    {
	ProxyPtr proxy = ProxyPtr::dynamicCast(type);
	assert(proxy);
	constructed = proxy->_class();
    }

    out << nl << constructed->scope() << genFunc << tagName << ", " << (pointer ? "" : "&") << stream
	<< ", " << param << ");";
}

void
Slice::writeGenericMarshalCode(Output& out, const list<pair<TypePtr, string> >& params, const TypePtr& ret)
{
    for(list<pair<TypePtr, string> >::const_iterator p = params.begin(); p != params.end(); ++p)
    {
	writeGenericMarshalUnmarshalCode(out, p->first, p->second, true);
    }
    if(ret)
    {
	writeGenericMarshalUnmarshalCode(out, ret, "__ret", true);
    }
}

void
Slice::writeGenericUnmarshalCode(Output& out, const list<pair<TypePtr, string> >& params, const TypePtr& ret)
{
    for(list<pair<TypePtr, string> >::const_iterator p = params.begin(); p != params.end(); ++p)
    {
	writeGenericMarshalUnmarshalCode(out, p->first, p->second, false);
    }
    if(ret)
    {
	writeGenericMarshalUnmarshalCode(out, ret, "__ret", false);
    }
}
