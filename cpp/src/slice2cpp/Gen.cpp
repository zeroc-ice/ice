// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Functional.h>
#include <Gen.h>
#include <GenUtil.h>
#include <limits>

//
// Stupid Visual C++ defines min and max as macros :-(
//
#ifdef min
#   undef min
#endif
#ifdef max
#   undef max
#endif

using namespace std;
using namespace Slice;

struct ToIfdef
{
    char operator()(char c)
    {
	if(!isalnum(c))
	    return '_';
	else
	    return c;
    }
};

Slice::Gen::Gen(const string& name,
		const string& base,
		const string& include,
		const vector<string>& includePaths,
		const string& dllExport)
    : base_(base),
      include_(include),
      includePaths_(includePaths),
      dllExport_(dllExport)
{
    for(vector<string>::iterator p = includePaths_.begin();
	p != includePaths_.end();
	++p)
    {
	if(p -> length() && (*p)[p -> length() - 1] != '/')
	    *p += '/';
    }

    if(dllExport_.length())
	dllExport_ = " " + dllExport_;

    string::size_type pos = base_.rfind('/');
    if(pos != string::npos)
	base_.erase(0, pos + 1);

    string fileH = base_ + ".h";
    string fileC = base_ + ".cpp";

    H.open(fileH.c_str());
    if(!H)
    {
	cerr << name << ": can't open `" << fileH << "' for writing: "
	     << strerror(errno) << endl;
	return;
    }
    
    C.open(fileC.c_str());
    if(!C)
    {
	cerr << name << ": can't open `" << fileC << "' for writing: "
	     << strerror(errno) << endl;
    }

    printHeader(H);
    printHeader(C);

    string s = fileH;
    transform(s.begin(), s.end(), s.begin(), ToIfdef());
    H << "\n#ifndef __" << s << "__";
    H << "\n#define __" << s << "__";
    H << '\n';
}

Slice::Gen::~Gen()
{
    H << "\n\n#endif\n";
    C << '\n';
}

bool
Slice::Gen::operator!() const
{
    return !H || !C;
}

void
Slice::Gen::generate(const Unit_ptr& unit)
{
    C << "\n#include <";
    if(include_.length())
	C << include_ << '/';
    C << base_ << ".h>";

    H << "\n#include <Ice/ProxyF.h>";
    H << "\n#include <Ice/ObjectF.h>";
    H << "\n#include <Ice/LocalObjectF.h>";
    H << "\n#include <Ice/Native.h>";
    if(unit -> hasProxies())
    {
	H << "\n#include <Ice/Proxy.h>";
	H << "\n#include <Ice/Object.h>";
	H << "\n#include <Ice/Outgoing.h>";
	H << "\n#include <Ice/Incoming.h>";
	H << "\n#include <Ice/LocalException.h>";
    }
    else
    {
	H << "\n#include <Ice/LocalObject.h>";
	C << "\n#include <Ice/Stream.h>";
    }

    list<string> includes = unit -> includeFiles();
    for(list<string>::iterator q = includes.begin();
	q != includes.end();
	++q)
    {
	H << "\n#include <" << changeInclude(*q) << ".h>";
    }

    ProxyDeclVisitor proxyDeclVisitor(H, C, dllExport_);
    unit -> visit(&proxyDeclVisitor);

    ObjectDeclVisitor objectDeclVisitor(H, C, dllExport_);
    unit -> visit(&objectDeclVisitor);

    IceVisitor iceVisitor(H, C, dllExport_);
    unit -> visit(&iceVisitor);

    HandleVisitor handleVisitor(H, C, dllExport_);
    unit -> visit(&handleVisitor);

    TypesVisitor typesVisitor(H, C, dllExport_);
    unit -> visit(&typesVisitor);

    ProxyVisitor proxyVisitor(H, C, dllExport_);
    unit -> visit(&proxyVisitor);

    DelegateVisitor delegateVisitor(H, C, dllExport_);
    unit -> visit(&delegateVisitor);

    DelegateMVisitor delegateMVisitor(H, C, dllExport_);
    unit -> visit(&delegateMVisitor);

    ObjectVisitor objectVisitor(H, C, dllExport_);
    unit -> visit(&objectVisitor);
}

string
Slice::Gen::changeInclude(const string& orig)
{
    string file = orig;
    for(vector<string>::iterator p = includePaths_.begin();
	p != includePaths_.end();
	++p)
    {
	if(orig.compare(0, p -> length(), *p) == 0)
	{
	    string s = orig.substr(p -> length());
	    if(s.length() < file.length())
		file = s;
	}
    }
    
    string::size_type pos = file.rfind('.');
    if(pos != string::npos)
	file.erase(pos);

    return file;
}

void
Slice::Gen::printHeader(Output& out)
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
    out << "\n// Generated from file `" << changeInclude(base_) << ".ice'";
    out << '\n';
}

Slice::Gen::TypesVisitor::TypesVisitor(Output& h, Output& c,
				       const string& dllExport)
    : H(h), C(c), dllExport_(dllExport)
{
}

void
Slice::Gen::TypesVisitor::visitModuleStart(const Module_ptr& p)
{
    if(!p -> hasOtherConstructedTypes())
	return;

    string name = p -> name();
    
    H << sp;
    H << nl << "namespace " << name << nl << '{';
}

void
Slice::Gen::TypesVisitor::visitModuleEnd(const Module_ptr& p)
{
    if(!p -> hasOtherConstructedTypes())
	return;

    H << sp;
    H << nl << '}';
}

void
Slice::Gen::TypesVisitor::visitVector(const Vector_ptr& p)
{
    string name = p -> name();
    Type_ptr subtype = p -> type();
    string s = typeToString(subtype);
    if(s[0] == ':')
	s.insert(0, " ");
    H << sp;
    H << nl << "typedef ::std::vector<" << s << "> " << name << ';';

    Builtin_ptr builtin = Builtin_ptr::dynamicCast(subtype);
    if(!builtin)
    {
	string scoped = p -> scoped();
	string scope = p -> scope();
	if(scope.length())
	    scope.erase(0, 2);

	H << sp;
	H << nl << "class __U__" << name << " { };";
	H << nl << "void" << dllExport_ << " __write(::__Ice::Stream*, const "
	  << name << "&, __U__" << name << ");";
	H << nl << "void" << dllExport_ << " __read(::__Ice::Stream*, "
	  << name << "&, __U__" << name << ");";
	C << sp;
	C << nl << "void" << nl << scope
	  << "::__write(::__Ice::Stream* __os, const " << scoped << "& v, ::"
	  << scope << "::__U__" << name << ')';
	C << sb;
	C << nl << "__os -> write(::Ice::Int(v.size()));";
	C << nl << scoped << "::const_iterator p;";
	C << nl << "for(p = v.begin(); p != v.end(); ++p)";
	C.inc();
	writeMarshalUnmarshalCode(C, subtype, "*p", true);
	C.dec();
	C << eb;
	C << sp;
	C << nl << "void" << nl << scope
	  << "::__read(::__Ice::Stream* __is, " << scoped << "& v, ::"
	  << scope << "::__U__" << name << ')';
	C << sb;
	C << nl << "::Ice::Int sz;";
	C << nl << "__is -> read(sz);";
	// Don't use v.resize(sz) or v.reserve(sz) here, as it
	// cannot be checked whether sz is a reasonable value
	C << nl << "while(sz--)";
	C << sb;
	C.zeroIndent();
	C << nl << "#ifdef WIN32"; // STLBUG
	C.restoreIndent();
	C << nl << "v.push_back(" << typeToString(subtype) << "());";
	C.zeroIndent();
	C << nl << "#else";
	C.restoreIndent();
	C << nl << "v.push_back();";
	C.zeroIndent();
	C << nl << "#endif";
	C.restoreIndent();
	writeMarshalUnmarshalCode(C, subtype, "v.back()", false);
	C << eb;
	C << eb;
    }
}

void
Slice::Gen::TypesVisitor::visitEnum(const Enum_ptr& p)
{
    string name = p -> name();
    StringList enumerators = p -> enumerators();
    H << sp;
    H << nl << "enum " << name;
    H << sb;
    StringList::iterator en = enumerators.begin();
    while(en != enumerators.end())
    {
	H << nl << *en;
	if(++en != enumerators.end())
	    H << ',';
    }
    H << eb << ';';

    string scoped = p -> scoped();
    string scope = p -> scope();
    if(scope.length())
	scope.erase(0, 2);

    int sz = enumerators.size();
    
    H << sp;
    H << nl << "void" << dllExport_ << " __write(::__Ice::Stream*, " << name
      << ");";
    H << nl << "void" << dllExport_ << " __read(::__Ice::Stream*, " << name
      << "&);";
    C << sp;
    C << nl << "void" << nl << scope
      << "::__write(::__Ice::Stream* __os, " << scoped << " v)";
    C << sb;
    if(sz <= numeric_limits<Ice::Byte>::max())
	C << nl << "__os -> write(static_cast< ::Ice::Byte>(v));";
    else if(sz <= numeric_limits<Ice::Short>::max())
	C << nl << "__os -> write(static_cast< ::Ice::Short>(v));";
    else if(sz <= numeric_limits<Ice::Int>::max())
	C << nl << "__os -> write(static_cast< ::Ice::Int>(v));";
    else
	C << nl << "__os -> write(static_cast< ::Ice::Long>(v));";
    C << eb;
    C << sp;
    C << nl << "void" << nl << scope << "::__read(::__Ice::Stream* __is, "
      << scoped << "& v)";
    C << sb;
    if(sz <= numeric_limits<Ice::Byte>::max())
    {
	C << nl << "::Ice::Byte val;";
	C << nl << "__is -> read(val);";
	C << nl << "v = static_cast< " << scoped << ">(val);";
    }
    else if(sz <= numeric_limits<Ice::Short>::max())
    {
	C << nl << "::Ice::Short val;";
	C << nl << "__is -> read(val);";
	C << nl << "v = static_cast< " << scoped << ">(val);";
    }
    else if(sz <= numeric_limits<Ice::Int>::max())
    {
	C << nl << "::Ice::Int val;";
	C << nl << "__is -> read(val);";
	C << nl << "v = static_cast< " << scoped << ">(val);";
    }
    else
    {
	C << nl << "::Ice::Long val;";
	C << nl << "__is -> read(val);";
	C << nl << "v = static_cast< " << scoped << ">(val);";
    }
    C << eb;
}

void
Slice::Gen::TypesVisitor::visitNative(const Native_ptr& p)
{
    string name = p -> name();

    H << sp;
    H << nl << "typedef ::__IceNative::" << name << ' ' << name << ';';
}

Slice::Gen::ProxyDeclVisitor::ProxyDeclVisitor(Output& h, Output& c,
					       const string& dllExport)
    : H(h), C(c), dllExport_(dllExport)
{
}

void
Slice::Gen::ProxyDeclVisitor::visitUnitStart(const Unit_ptr& p)
{
    if(!p -> hasProxies())
	return;

    H << sp;
    H << nl << "namespace __IceProxy" << nl << '{';
}

void
Slice::Gen::ProxyDeclVisitor::visitUnitEnd(const Unit_ptr& p)
{
    if(!p -> hasProxies())
	return;

    H << sp;
    H << nl << '}';
}
    
void
Slice::Gen::ProxyDeclVisitor::visitModuleStart(const Module_ptr& p)
{
    if(!p -> hasProxies())
	return;

    string name = p -> name();
    
    H << sp;
    H << nl << "namespace " << name << nl << '{';
}

void
Slice::Gen::ProxyDeclVisitor::visitModuleEnd(const Module_ptr& p)
{
    if(!p -> hasProxies())
	return;

    H << sp;
    H << nl << '}';
}

void
Slice::Gen::ProxyDeclVisitor::visitClassDecl(const ClassDecl_ptr& p)
{
    if(p -> isLocal())
	return;

    string name = p -> name();

    H << sp;
    H << nl << "class " << name << ';';
}

Slice::Gen::ProxyVisitor::ProxyVisitor(Output& h, Output& c,
				       const string& dllExport)
    : H(h), C(c), dllExport_(dllExport)
{
}

void
Slice::Gen::ProxyVisitor::visitUnitStart(const Unit_ptr& p)
{
    if(!p -> hasProxies())
	return;

    H << sp;
    H << nl << "namespace __IceProxy" << nl << '{';
}

void
Slice::Gen::ProxyVisitor::visitUnitEnd(const Unit_ptr& p)
{
    if(!p -> hasProxies())
	return;

    H << sp;
    H << nl << '}';
}
    
void
Slice::Gen::ProxyVisitor::visitModuleStart(const Module_ptr& p)
{
    if(!p -> hasProxies())
	return;

    string name = p -> name();
    
    H << sp;
    H << nl << "namespace " << name << nl << '{';
}

void
Slice::Gen::ProxyVisitor::visitModuleEnd(const Module_ptr& p)
{
    if(!p -> hasProxies())
	return;

    H << sp;
    H << nl << '}';
}

void
Slice::Gen::ProxyVisitor::visitClassDefStart(const ClassDef_ptr& p)
{
    if(p -> isLocal())
	return;

    string name = p -> name();

    ClassDef_ptr base = p -> base();
    string baseS;
    if(base)
	baseS = base -> scoped();
    else
	baseS = "::Ice::Object";
    
    H << sp;
    H << nl << "class" << dllExport_ << ' ' << name << " : "
      << "virtual public ::__IceProxy" << baseS;
    H << sb;
    H.dec();
    H << nl << "public: ";
    H.inc();
}

void
Slice::Gen::ProxyVisitor::visitClassDefEnd(const ClassDef_ptr& p)
{
    if(p -> isLocal())
	return;

    string scoped = p -> scoped();
    
    H.dec();
    H << sp;
    H << nl << "private: ";
    H.inc();
    H << sp;
    H << nl << "virtual ::__Ice::Handle< ::__IceDelegateM::Ice::Object> "
      << "__createDelegateM();";
    H << eb << ';';
    C << sp;
    C << nl << "::__Ice::Handle< ::__IceDelegateM::Ice::Object>"
      << nl << "__IceProxy" << scoped << "::__createDelegateM()" << sb;
    C << nl << "return ::__Ice::Handle< ::__IceDelegateM::Ice::Object>"
      << "(new ::__IceDelegateM" << scoped << ");";
    C << eb;
}

void
Slice::Gen::ProxyVisitor::visitOperation(const Operation_ptr& p)
{
    Container_ptr container = p -> container();
    ClassDef_ptr cl = ClassDef_ptr::dynamicCast(container);
    if(cl -> isLocal())
	return;

    string name = p -> name();
    string scoped = p -> scoped();
    string scope = p -> scope();

    Type_ptr ret = p -> returnType();
    string retS = returnTypeToString(ret);

    TypeStringList inParams = p -> inputParameters();
    TypeStringList outParams = p -> outputParameters();
    TypeStringList::iterator q;

    string params = "(";
    string paramsDecl = "("; // With declarators
    string args = "(";

    for(q = inParams.begin(); q != inParams.end(); ++q)
    {
	if(q != inParams.begin())
	{
	    params += ", ";
	    paramsDecl += ", ";
	    args += ", ";
	}

	string typeString = inputTypeToString(q -> first);
	params += typeString;
	paramsDecl += typeString;
	paramsDecl += ' ';
	paramsDecl += q -> second;
	args += q -> second;
    }

    for(q = outParams.begin(); q != outParams.end(); ++q)
    {
	if(q != outParams.begin() || !inParams.empty())
	{
	    params += ", ";
	    paramsDecl += ", ";
	    args += ", ";
	}

	string typeString = outputTypeToString(q -> first);
	params += typeString;
	paramsDecl += typeString;
	paramsDecl += ' ';
	paramsDecl += q -> second;
	args += q -> second;
    }

    params += ')';
    paramsDecl += ')';
    args += ')';
    
    TypeList throws = p -> throws();

    H << sp;
    H << nl << retS << ' ' << name << params << ';';
    C << sp;
    C << nl << retS << nl << "__IceProxy" << scoped << paramsDecl
      << sb;
    C << nl << "::__Ice::Handle< ::__IceDelegate::Ice::Object> __delBase"
      << " = __getDelegate();";
    C << nl << "::__IceDelegate" << scope
      << "* __del = dynamic_cast< ::__IceDelegate" << scope
      << "*>(__delBase.get());";
    C << nl;
    if(ret)
	C << "return ";
    C << "__del -> " << name << args << ";";
    C << eb;
}

Slice::Gen::DelegateVisitor::DelegateVisitor(Output& h, Output& c,
					     const string& dllExport)
    : H(h), C(c), dllExport_(dllExport)
{
}

void
Slice::Gen::DelegateVisitor::visitUnitStart(const Unit_ptr& p)
{
    if(!p -> hasProxies())
	return;

    H << sp;
    H << nl << "namespace __IceDelegate" << nl << '{';
}

void
Slice::Gen::DelegateVisitor::visitUnitEnd(const Unit_ptr& p)
{
    if(!p -> hasProxies())
	return;

    H << sp;
    H << nl << '}';
}
    
void
Slice::Gen::DelegateVisitor::visitModuleStart(const Module_ptr& p)
{
    if(!p -> hasProxies())
	return;

    string name = p -> name();
    
    H << sp;
    H << nl << "namespace " << name << nl << '{';
}

void
Slice::Gen::DelegateVisitor::visitModuleEnd(const Module_ptr& p)
{
    if(!p -> hasProxies())
	return;

    H << sp;
    H << nl << '}';
}

void
Slice::Gen::DelegateVisitor::visitClassDefStart(const ClassDef_ptr& p)
{
    if(p -> isLocal())
	return;

    string name = p -> name();
    ClassDef_ptr base = p -> base();
    string baseS;
    if(base)
	baseS = base -> scoped();
    else
	baseS = "::Ice::Object";
    
    H << sp;
    H << nl << "class" << dllExport_ << ' ' << name << " : "
      << "virtual public ::__IceDelegate" << baseS;
    H << sb;
    H.dec();
    H << nl << "public: ";
    H.inc();
}

void
Slice::Gen::DelegateVisitor::visitClassDefEnd(const ClassDef_ptr& p)
{
    if(p -> isLocal())
	return;

    H << eb << ';';
}

void
Slice::Gen::DelegateVisitor::visitOperation(const Operation_ptr& p)
{
    Container_ptr container = p -> container();
    ClassDef_ptr cl = ClassDef_ptr::dynamicCast(container);
    if(cl -> isLocal())
	return;

    string name = p -> name();

    Type_ptr ret = p -> returnType();
    string retS = returnTypeToString(ret);

    TypeStringList inParams = p -> inputParameters();
    TypeStringList outParams = p -> outputParameters();
    TypeStringList::iterator q;

    string params = "(";
    string args = "(";

    for(q = inParams.begin(); q != inParams.end(); ++q)
    {
	if(q != inParams.begin())
	{
	    params += ", ";
	    args += ", ";
	}

	string typeString = inputTypeToString(q -> first);
	params += typeString;
	args += q -> second;
    }

    for(q = outParams.begin(); q != outParams.end(); ++q)
    {
	if(q != outParams.begin() || !inParams.empty())
	{
	    params += ", ";
	    args += ", ";
	}

	string typeString = outputTypeToString(q -> first);
	params += typeString;
	args += q -> second;
    }

    params += ')';
    args += ')';
    
    TypeList throws = p -> throws();

    H << sp;
    H << nl << "virtual " << retS << ' ' << name << params << " = 0;";
}

Slice::Gen::DelegateMVisitor::DelegateMVisitor(Output& h, Output& c,
					       const string& dllExport)
    : H(h), C(c), dllExport_(dllExport)
{
}

void
Slice::Gen::DelegateMVisitor::visitUnitStart(const Unit_ptr& p)
{
    if(!p -> hasProxies())
	return;

    H << sp;
    H << nl << "namespace __IceDelegateM" << nl << '{';
}

void
Slice::Gen::DelegateMVisitor::visitUnitEnd(const Unit_ptr& p)
{
    if(!p -> hasProxies())
	return;

    H << sp;
    H << nl << '}';
}
    
void
Slice::Gen::DelegateMVisitor::visitModuleStart(const Module_ptr& p)
{
    if(!p -> hasProxies())
	return;

    string name = p -> name();
    
    H << sp;
    H << nl << "namespace " << name << nl << '{';
}

void
Slice::Gen::DelegateMVisitor::visitModuleEnd(const Module_ptr& p)
{
    if(!p -> hasProxies())
	return;

    H << sp;
    H << nl << '}';
}

void
Slice::Gen::DelegateMVisitor::visitClassDefStart(const ClassDef_ptr& p)
{
    if(p -> isLocal())
	return;

    string name = p -> name();
    string scoped = p -> scoped();

    ClassDef_ptr base = p -> base();
    string baseS;
    if(base)
	baseS = base -> scoped();
    else
	baseS = "::Ice::Object";
    
    H << sp;
    H << nl << "class" << dllExport_ << ' ' << name << " : ";
    H.useCurrentPosAsIndent();
    H << "virtual public ::__IceDelegate" << scoped << ',';
    H << nl << "virtual public ::__IceDelegateM" << baseS;
    H.restoreIndent();
    H << sb;
    H.dec();
    H << nl << "public: ";
    H.inc();
}

void
Slice::Gen::DelegateMVisitor::visitClassDefEnd(const ClassDef_ptr& p)
{
    if(p -> isLocal())
	return;

    H << eb << ';';
}

void
Slice::Gen::DelegateMVisitor::visitOperation(const Operation_ptr& p)
{
    Container_ptr container = p -> container();
    ClassDef_ptr cl = ClassDef_ptr::dynamicCast(container);
    if(cl -> isLocal())
	return;

    string name = p -> name();
    string scoped = p -> scoped();
    string scope = p -> scope();

    Type_ptr ret = p -> returnType();
    string retS = returnTypeToString(ret);

    TypeStringList inParams = p -> inputParameters();
    TypeStringList outParams = p -> outputParameters();
    TypeStringList::iterator q;

    string params = "(";
    string paramsDecl = "("; // With declarators
    string args = "(";

    for(q = inParams.begin(); q != inParams.end(); ++q)
    {
	if(q != inParams.begin())
	{
	    params += ", ";
	    paramsDecl += ", ";
	    args += ", ";
	}

	string typeString = inputTypeToString(q -> first);
	params += typeString;
	paramsDecl += typeString;
	paramsDecl += ' ';
	paramsDecl += q -> second;
	args += q -> second;
    }

    for(q = outParams.begin(); q != outParams.end(); ++q)
    {
	if(q != outParams.begin() || !inParams.empty())
	{
	    params += ", ";
	    paramsDecl += ", ";
	    args += ", ";
	}

	string typeString = outputTypeToString(q -> first);
	params += typeString;
	paramsDecl += typeString;
	paramsDecl += ' ';
	paramsDecl += q -> second;
	args += q -> second;
    }

    params += ')';
    paramsDecl += ')';
    args += ')';
    
    TypeList throws = p -> throws();

    H << sp;
    H << nl << "virtual " << retS << ' ' << name << params << ';';
    C << sp;
    C << nl << retS << nl << "__IceDelegateM" << scoped << paramsDecl
      << sb;
    C << nl << "::__Ice::Outgoing __out(__emitter(), __reference());";
    if(ret || !outParams.empty() || !throws.empty())
	C << nl << "::__Ice::Stream* __is = __out.is();";
    C << nl << "::__Ice::Stream* __os = __out.os();";
    C << nl << "__os -> write(\"" << name << "\");";
    writeMarshalCode(C, inParams, 0);
    C << nl << "if(!__out.invoke())";
    if(!throws.empty())
    {
	C << sb;
	C << nl << "::Ice::Int __exnum;";
	C << nl << "__is -> read(__exnum);";
	C << nl << "switch(__exnum)";
	C << sb;
	TypeList::iterator r;
	Ice::Int cnt = 0;
	for(r = throws.begin(); r != throws.end(); ++r)
	{
	    C.dec();
	    C << nl << "case " << cnt++ << ':';
	    C.sb();
	    TypeStringList li;
	    li.push_back(make_pair(*r, string("__ex")));
	    writeAllocateCode(C, li, 0);
	    writeUnmarshalCode(C, li, 0);
	    C << nl << "throw __ex;";
	    C.eb();
	    C.inc();
	}
	C << eb;
	C << nl
	  << "throw ::Ice::UnknownUserException(__FILE__, __LINE__);";
	C << eb;
    }
    else
    {
	C.inc();
	C << nl
	  << "throw ::Ice::UnknownUserException(__FILE__, __LINE__);";
	C.dec();
    }
    writeAllocateCode(C, TypeStringList(), ret);
    writeUnmarshalCode(C, outParams, ret);
    if(ret)
	C << nl << "return __ret;";
    C << eb;
}

Slice::Gen::ObjectDeclVisitor::ObjectDeclVisitor(Output& h, Output& c,
						 const string& dllExport)
    : H(h), C(c), dllExport_(dllExport)
{
}

void
Slice::Gen::ObjectDeclVisitor::visitModuleStart(const Module_ptr& p)
{
    if(!p -> hasClassDecls())
	return;

    string name = p -> name();
    
    H << sp;
    H << nl << "namespace " << name << nl << '{';
}

void
Slice::Gen::ObjectDeclVisitor::visitModuleEnd(const Module_ptr& p)
{
    if(!p -> hasClassDecls())
	return;

    H << sp;
    H << nl << '}';
}

void
Slice::Gen::ObjectDeclVisitor::visitClassDecl(const ClassDecl_ptr& p)
{
    string name = p -> name();
    
    H << sp;
    H << nl << "class " << name << ';';
}

Slice::Gen::ObjectVisitor::ObjectVisitor(Output& h, Output& c,
					 const string& dllExport)
    : H(h), C(c), dllExport_(dllExport)
{
}

void
Slice::Gen::ObjectVisitor::visitModuleStart(const Module_ptr& p)
{
    if(!p -> hasClassDefs())
	return;

    string name = p -> name();
    
    H << sp;
    H << nl << "namespace " << name << nl << '{';
}

void
Slice::Gen::ObjectVisitor::visitModuleEnd(const Module_ptr& p)
{
    if(!p -> hasClassDefs())
	return;

    H << sp;
    H << nl << '}';
}

void
Slice::Gen::ObjectVisitor::visitClassDefStart(const ClassDef_ptr& p)
{
    bool isLocal = p -> isLocal();
    string name = p -> name();
    string scoped = p -> scoped();

    vector<ClassDef_ptr> bases;
    vector<ClassDef_ptr>::const_iterator q;

    ClassDef_ptr base = p;
    while((base = base -> base()))
	bases.push_back(base);

    string baseS;
    if(!bases.empty())
    {
	base = bases[0];
	baseS = base -> scoped();
    }
    else
    {
	if(isLocal)
	    baseS = "::Ice::LocalObject";
	else
	    baseS = "::Ice::Object";
    }

    H << sp;
    H << nl << "class" << dllExport_ << ' ' << name << " : ";
    if(isLocal)
    {
        // No virtual inheritance for local objects
	H << "public " << baseS;
    }
    else
    {
	H.useCurrentPosAsIndent();
	H << "virtual public ::__IceDelegate" << scoped << ',';
 	H << nl << "virtual public " << baseS;
	H.restoreIndent();
    }
    H << sb;
    H.dec();
    H << nl << "public: ";
    H.inc();
    if(!isLocal)
    {
	H << sp;
	H << nl << "static std::string __implements["
	  << bases.size() + 2 << "];";
	H << sp;
	H << nl << "virtual bool _implements(const std::string&);";
	H << sp;
	H << nl << "virtual const std::string* __ids();";
	C << sp;
	C << nl << "std::string " << scoped.substr(2)
	  << "::__implements[" << bases.size() + 2 << "] =";
	C << sb;
	C << nl << '"' << scoped << "\",";
	for(q = bases.begin(); q != bases.end(); ++q)
	    C << nl << '"' << (*q) -> scoped() << "\",";
	C << nl << "\"::Ice::Object\"";
	C << eb << ';';
	C << sp;
	C << nl << "bool" << nl << scoped.substr(2)
	  << "::_implements(const std::string& s)";
	C << sb;
	C << nl << "std::string* b = __implements;";
	C << nl << "std::string* e = __implements + " << bases.size() + 2
	  << ';';
	C << nl << "std::string* r = std::find(b, e, s);";
	C << nl << "return(r != e);";
	C << eb;
	C << sp;
	C << nl << "const std::string*" << nl << scoped.substr(2)
	  << "::__ids()";
	C << sb;
	C << nl << "return __implements;";
	C << eb;
    }
}
    
void
Slice::Gen::ObjectVisitor::visitClassDefEnd(const ClassDef_ptr& p)
{
    string name = p -> name();
    string scoped = p -> scoped();

    list<Operation_ptr> operations = p -> operations();
    ClassDef_ptr base = p -> base();
    
    if(!p -> isLocal() && !operations.empty())
    {
	operations.sort();
	list<Operation_ptr>::iterator op;
    
	H << sp;
	H << nl << "typedef ::__Ice::DispatchStatus (" << name
	  << "::*__Op)(::__Ice::Incoming&);";
	H << nl << "static __Op __ops[" << operations.size() << "];";
	H << nl << "static std::string __names[" << operations.size()
	  << "];";
	H << nl << "virtual ::__Ice::DispatchStatus "
	  << "__dispatch(::__Ice::Incoming&, const std::string&);";
	C << sp;
	C << nl << scoped << "::__Op " << scoped.substr(2)
	  << "::__ops[" << operations.size() << "] =";
	C << sb;
	op = operations.begin();
	while(op != operations.end())
	{
	    C << nl << '&' << scoped.substr(2) << "::___"
	      << (*op) -> name();
	    if(++op != operations.end())
		C << ',';
	}
	C << eb << ';';
	C << sp;
	C << nl << "std::string " << scoped.substr(2)
	  << "::__names[" << operations.size() << "] =";
	C << sb;
	op = operations.begin();
	while(op != operations.end())
	{
	    C << nl << '"' << (*op) -> name() << '"';
	    if(++op != operations.end())
		C << ',';
	}
	C << eb << ';';
	C << sp;
	C << nl << "::__Ice::DispatchStatus" << nl << scoped.substr(2)
	  << "::__dispatch(::__Ice::Incoming& in, const std::string& s)";
	C << sb;
	C << nl << "std::string* b = __names;";
	C << nl << "std::string* e = __names + " << operations.size()
	  << ';';
	C << nl << "std::pair<std::string*, std::string*> r = "
	  << "std::equal_range(b, e, s);";
	C << nl << "if(r.first != r.second)";
	C.inc();
	C << nl << "return (this ->* __ops[r.first - b])(in);";
	C.dec();
	C << nl << "else";
	C.inc();
	C.zeroIndent();
	C << nl << "#ifdef WIN32"; // COMPILERBUG
	C.restoreIndent();
	if(base)
	    C << nl << "return " << base -> name();
	else
	    C << nl << "return Object";
	C << "::__dispatch(in, s);";
	C.zeroIndent();
	C << nl << "#else";
	C.restoreIndent();
	if(base)
	    C << nl << "return " << base -> scoped();
	else
	    C << nl << "return ::Ice::Object";
	C << "::__dispatch(in, s);";
	C.zeroIndent();
	C << nl << "#endif";
	C.restoreIndent();
	C.dec();
	C << eb;
    }
    H << sp;
    H << nl << "virtual void __write(::__Ice::Stream*);";
    H << nl << "virtual void __read(::__Ice::Stream*);";
    H << eb << ';';
    TypeStringList memberList;
    list<DataMember_ptr> dataMembers = p -> dataMembers();
    list<DataMember_ptr>::const_iterator q;
    for(q = dataMembers.begin(); q != dataMembers.end(); ++q)
	memberList.push_back(make_pair((*q) -> type(), (*q) -> name()));
    C << sp;
    C << nl << "void" << nl << scoped.substr(2)
      << "::__write(::__Ice::Stream* __os)";
    C << sb;
    C << nl << "__os -> startWriteEncaps();";
    writeMarshalCode(C, memberList, 0);
    C << nl << "__os -> endWriteEncaps();";
    if(base)
    {
	C.zeroIndent();
	C << nl << "#ifdef WIN32"; // COMPILERBUG
	C.restoreIndent();
	C << nl << base -> name() << "::__write(__os);";
	C.zeroIndent();
	C << nl << "#else";
	C.restoreIndent();
	C << nl << base -> scoped() << "::__write(__os);";
	C.zeroIndent();
	C << nl << "#endif";
	C.restoreIndent();
    }
    C << eb;
    C << sp;
    C << nl << "void" << nl << scoped.substr(2)
      << "::__read(::__Ice::Stream* __is)";
    C << sb;
    C << nl << "__is -> startReadEncaps();";
    writeUnmarshalCode(C, memberList, 0);
    C << nl << "__is -> endReadEncaps();";
    if(base)
    {
	C.zeroIndent();
	C << nl << "#ifdef WIN32"; // COMPILERBUG
	C.restoreIndent();
	C << nl << base -> name() << "::__read(__is);";
	C.zeroIndent();
	C << nl << "#else";
	C.restoreIndent();
	C << nl << base -> scoped() << "::__read(__is);";
	C.zeroIndent();
	C << nl << "#endif";
	C.restoreIndent();
    }
    C << eb;
}

void
Slice::Gen::ObjectVisitor::visitOperation(const Operation_ptr& p)
{
    Container_ptr container = p -> container();
    ClassDef_ptr cl = ClassDef_ptr::dynamicCast(container);
    string name = p -> name();
    string scoped = p -> scoped();
    string scope = p -> scope();

    Type_ptr ret = p -> returnType();
    string retS = returnTypeToString(ret);

    TypeStringList inParams = p -> inputParameters();
    TypeStringList outParams = p -> outputParameters();
    TypeStringList::iterator q;

    string params = "(";
    string paramsDecl = "("; // With declarators
    string args = "(";

    for(q = inParams.begin(); q != inParams.end(); ++q)
    {
	if(q != inParams.begin())
	{
	    params += ", ";
	    paramsDecl += ", ";
	    args += ", ";
	}

	string typeString = inputTypeToString(q -> first);
	params += typeString;
	paramsDecl += typeString;
	paramsDecl += ' ';
	paramsDecl += q -> second;
	args += q -> second;
    }

    for(q = outParams.begin(); q != outParams.end(); ++q)
    {
	if(q != outParams.begin() || !inParams.empty())
	{
	    params += ", ";
	    paramsDecl += ", ";
	    args += ", ";
	}

	string typeString = outputTypeToString(q -> first);
	params += typeString;
	paramsDecl += typeString;
	paramsDecl += ' ';
	paramsDecl += q -> second;
	args += q -> second;
    }

    params += ')';
    paramsDecl += ')';
    args += ')';
    
    TypeList throws = p -> throws();

    H << sp;
    H << nl << "virtual " << retS << ' ' << name << params << " = 0;";

    if(!cl -> isLocal())
    {
	H << nl << "::__Ice::DispatchStatus ___" << name
	  << "(::__Ice::Incoming&);";
	C << sp;
	C << nl << "::__Ice::DispatchStatus" << nl << scope.substr(2)
	  << "::___" << name << "(::__Ice::Incoming& __in)";
	C << sb;
	if(!inParams.empty())
	    C << nl << "::__Ice::Stream* __is = __in.is();";
	if(ret || !outParams.empty() || !throws.empty())
	    C << nl << "::__Ice::Stream* __os = __in.os();";
	writeAllocateCode(C, inParams, 0);
	writeUnmarshalCode(C, inParams, 0);
	writeAllocateCode(C, outParams, 0);
	if(!throws.empty())
	{
	    C << nl << "try";
	    C << sb;
	}
	C << nl;
	if(ret)
	    C << retS << " __ret = ";
	C << name << args << ';';
	if(!throws.empty())
	{
	    C << eb;
	    TypeList::iterator r;
	    Ice::Int cnt = 0;
	    for(r = throws.begin(); r != throws.end(); ++r)
	    {
		C << nl << "catch(" << inputTypeToString(*r) << " __ex)";
		C << sb;
		C << nl << "__os -> write(" << cnt++ << ");";
		writeMarshalUnmarshalCode(C, *r, "__ex", true);
		C << nl << "return ::__Ice::DispatchException;";
		C << eb;
	    }
	}
	writeMarshalCode(C, outParams, ret);
	C << nl << "return ::__Ice::DispatchOK;";
	C << eb;
    }	
}

void
Slice::Gen::ObjectVisitor::visitDataMember(const DataMember_ptr& p)
{
    string name = p -> name();
    string s = typeToString(p -> type());
    H << sp;
    H << nl << s << ' ' << name << ';';
}

Slice::Gen::IceVisitor::IceVisitor(Output& h, Output& c,
				   const string& dllExport)
    : H(h), C(c), dllExport_(dllExport)
{
}

void
Slice::Gen::IceVisitor::visitUnitStart(const Unit_ptr& p)
{
    if(!p -> hasClassDecls())
	return;

    H << sp;
    H << nl << "namespace __Ice" << nl << '{';
}

void
Slice::Gen::IceVisitor::visitUnitEnd(const Unit_ptr& p)
{
    if(!p -> hasClassDecls())
	return;

    H << sp;
    H << nl << '}';
}
    
void
Slice::Gen::IceVisitor::visitClassDecl(const ClassDecl_ptr& p)
{
    string scoped = p -> scoped();
    
    H << sp;
    H << nl << "void" << dllExport_ << " incRef(" << scoped << "*);";
    H << nl << "void" << dllExport_ << " decRef(" << scoped << "*);";
    if(!p -> isLocal())
    {
	H << sp;
	H << nl << "void" << dllExport_ << " incRef(::__IceProxy" << scoped
	  << "*);";
	H << nl << "void" << dllExport_ << " decRef(::__IceProxy" << scoped
	  << "*);";
	H << sp;
	H << nl << "void" << dllExport_
	  << " checkedCast(::__IceProxy::Ice::Object*, ::__IceProxy"
	  << scoped << "*&);";
	H << nl << "void" << dllExport_
	  << " uncheckedCast(::__IceProxy::Ice::Object*, ::__IceProxy"
	  << scoped << "*&);";
    }
}

void
Slice::Gen::IceVisitor::visitClassDefStart(const ClassDef_ptr& p)
{
    string scoped = p -> scoped();
    
    C << sp;
    C << nl << "void __Ice::incRef(" << scoped << "* p) { p -> __incRef(); }";
    C << nl << "void __Ice::decRef(" << scoped << "* p) { p -> __decRef(); }";

    if(!p -> isLocal())
    {
	C << sp;
	C << nl << "void __Ice::incRef(::__IceProxy" << scoped
	  << "* p) { p -> __incRef(); }";
	C << nl << "void __Ice::decRef(::__IceProxy" << scoped
	  << "* p) { p -> __decRef(); }";
	C << sp;
	C << nl << "void __Ice::checkedCast(::__IceProxy::Ice::Object* b, "
	  << "::__IceProxy" << scoped << "*& d)";
	C << sb;
	C << nl << "d = dynamic_cast< ::__IceProxy" << scoped
	  << "*>(b);";
	C << nl << "if(!d && b -> _implements(\"" << scoped << "\"))";
	C << sb;
	C << nl << "d = new ::__IceProxy" << scoped << ';';
	C << nl << "b -> __copyTo(d);";
	C << eb;
	C << eb;
	C << sp;
	C << nl << "void __Ice::uncheckedCast(::__IceProxy::Ice::Object* b, "
	  << "::__IceProxy" << scoped << "*& d)";
	C << sb;
	C << nl << "d = dynamic_cast< ::__IceProxy" << scoped
	  << "*>(b);";
	C << nl << "if(!d)";
	C << sb;
	C << nl << "d = new ::__IceProxy" << scoped << ';';
	C << nl << "b -> __copyTo(d);";
	C << eb;
	C << eb;
    }
}

Slice::Gen::HandleVisitor::HandleVisitor(Output& h, Output& c,
					 const string& dllExport)
    : H(h), C(c), dllExport_(dllExport)
{
}

void
Slice::Gen::HandleVisitor::visitModuleStart(const Module_ptr& p)
{
    if(!p -> hasClassDecls())
	return;

    string name = p -> name();
    
    H << sp;
    H << nl << "namespace " << name << nl << '{';
}

void
Slice::Gen::HandleVisitor::visitModuleEnd(const Module_ptr& p)
{
    if(!p -> hasClassDecls())
	return;

    H << sp;
    H << nl << '}';
}

void
Slice::Gen::HandleVisitor::visitClassDecl(const ClassDecl_ptr& p)
{
    string name = p -> name();
    string scoped = p -> scoped();
    
    H << sp;
    H << nl << "typedef ::__Ice::Handle< " << scoped << "> " << name
      << "_ptr;";
    if(!p -> isLocal())
    {
	H << nl << "typedef ::__Ice::ProxyHandle< ::__IceProxy" << scoped
	  << "> " << name << "_prx;";
	H << sp;
	H << nl << "void" << dllExport_ << " __write(::__Ice::Stream*, const "
	  << name << "_prx&);";
	H << nl << "void" << dllExport_ << " __read(::__Ice::Stream*, "
	  << name << "_prx&);";
    }
}

void
Slice::Gen::HandleVisitor::visitClassDefStart(const ClassDef_ptr& p)
{
    if(p -> isLocal())
	return;

    string scoped = p -> scoped();
    string scope = p -> scope();
    if(scope.length())
	scope.erase(0, 2);

    C << sp;
    C << nl << "void" << nl << scope
      << "::__write(::__Ice::Stream* __os, const " << scoped << "_prx& v)";
    C << sb;
    // TODO: Should be ::Ice::__write
    C << nl << "::__Ice::write(__os, ::Ice::Object_prx(v));";
    C << eb;
    C << sp;
    C << nl << "void" << nl << scope
      << "::__read(::__Ice::Stream* __is, " << scoped << "_prx& v)";
    C << sb;
    C << nl << "::Ice::Object_prx proxy;";
    // TODO: Should be ::Ice::__read
    C << nl << "::__Ice::read(__is, proxy);";
    C << nl << "v = new ::__IceProxy" << scoped << ';';
    C << nl << "proxy -> __copyTo(v.get());";
    C << eb;
}
