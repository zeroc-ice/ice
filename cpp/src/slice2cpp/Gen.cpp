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
    : _base(base),
      _include(include),
      _includePaths(includePaths),
      _dllExport(dllExport)
{
    for(vector<string>::iterator p = _includePaths.begin();
	p != _includePaths.end();
	++p)
    {
	if(p -> length() && (*p)[p -> length() - 1] != '/')
	    *p += '/';
    }

    if(_dllExport.length())
	_dllExport = " " + _dllExport;

    string::size_type pos = _base.rfind('/');
    if(pos != string::npos)
	_base.erase(0, pos + 1);

    string fileH = _base + ".h";
    string fileC = _base + ".cpp";

    H.open(fileH.c_str());
    if(!H)
    {
	cerr << name << ": can't open `" << fileH << "' for writing: " << strerror(errno) << endl;
	return;
    }
    
    C.open(fileC.c_str());
    if(!C)
    {
	cerr << name << ": can't open `" << fileC << "' for writing: " << strerror(errno) << endl;
	return;
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
Slice::Gen::generate(const UnitPtr& unit)
{
    C << "\n#include <";
    if(_include.length())
	C << _include << '/';
    C << _base << ".h>";

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

    StringList includes = unit -> includeFiles();
    for(StringList::iterator q = includes.begin();
	q != includes.end();
	++q)
    {
	H << "\n#include <" << changeInclude(*q) << ".h>";
    }

    H << sp;
    H.zeroIndent();
    H << "\n#ifndef ICE_IGNORE_VERSION";
    H << "\n#   if ICE_INT_VERSION != 0x" << hex << ICE_INT_VERSION;
    H << "\n#       error Ice version mismatch!";
    H << "\n#   endif";
    H << "\n#endif";
    H.restoreIndent();

    C << sp;
    C.zeroIndent();
    C << "\n#ifndef ICE_IGNORE_VERSION";
    C << "\n#   if ICE_INT_VERSION != 0x" << hex << ICE_INT_VERSION;
    C << "\n#       error Ice version mismatch!";
    C << "\n#   endif";
    C << "\n#endif";
    C.restoreIndent();

    ProxyDeclVisitor proxyDeclVisitor(H, C, _dllExport);
    unit -> visit(&proxyDeclVisitor);

    ObjectDeclVisitor objectDeclVisitor(H, C, _dllExport);
    unit -> visit(&objectDeclVisitor);

    IceVisitor iceVisitor(H, C, _dllExport);
    unit -> visit(&iceVisitor);

    HandleVisitor handleVisitor(H, C, _dllExport);
    unit -> visit(&handleVisitor);

    TypesVisitor typesVisitor(H, C, _dllExport);
    unit -> visit(&typesVisitor);

    ProxyVisitor proxyVisitor(H, C, _dllExport);
    unit -> visit(&proxyVisitor);

    DelegateVisitor delegateVisitor(H, C, _dllExport);
    unit -> visit(&delegateVisitor);

    DelegateMVisitor delegateMVisitor(H, C, _dllExport);
    unit -> visit(&delegateMVisitor);

    ObjectVisitor objectVisitor(H, C, _dllExport);
    unit -> visit(&objectVisitor);
}

string
Slice::Gen::changeInclude(const string& orig)
{
    string file = orig;
    for(vector<string>::iterator p = _includePaths.begin();
	p != _includePaths.end();
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
    out << "\n// Generated from file `" << changeInclude(_base) << ".ice'";
    out << "\n// Ice version " << ICE_STRING_VERSION;
    out << '\n';
}

Slice::Gen::TypesVisitor::TypesVisitor(Output& h, Output& c, const string& dllExport)
    : H(h), C(c), _dllExport(dllExport)
{
}

void
Slice::Gen::TypesVisitor::visitModuleStart(const ModulePtr& p)
{
    if(!p -> hasOtherConstructedTypes())
	return;

    string name = p -> name();
    
    H << sp;
    H << nl << "namespace " << name << nl << '{';
}

void
Slice::Gen::TypesVisitor::visitModuleEnd(const ModulePtr& p)
{
    if(!p -> hasOtherConstructedTypes())
	return;

    H << sp;
    H << nl << '}';
}

void
Slice::Gen::TypesVisitor::visitVector(const VectorPtr& p)
{
    string name = p -> name();
    TypePtr subtype = p -> type();
    string s = typeToString(subtype);
    if(s[0] == ':')
	s.insert(0, " ");
    H << sp;
    H << nl << "typedef ::std::vector<" << s << "> " << name << ';';

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(subtype);
    if(!builtin)
    {
	string scoped = p -> scoped();
	string scope = p -> scope();
	if(scope.length())
	    scope.erase(0, 2);

	H << sp;
	H << nl << "class __U__" << name << " { };";
	H << nl << "void" << _dllExport << " __write(::IceInternal::Stream*, const " << name << "&, __U__" << name
	  << ");";
	H << nl << "void" << _dllExport << " __read(::IceInternal::Stream*, " << name << "&, __U__" << name << ");";
	C << sp;
	C << nl << "void" << nl << scope << "::__write(::IceInternal::Stream* __os, const " << scoped << "& v, ::"
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
	C << nl << "void" << nl << scope << "::__read(::IceInternal::Stream* __is, " << scoped << "& v, ::" << scope
	  << "::__U__" << name << ')';
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
Slice::Gen::TypesVisitor::visitEnum(const EnumPtr& p)
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
    H << nl << "void" << _dllExport << " __write(::IceInternal::Stream*, " << name << ");";
    H << nl << "void" << _dllExport << " __read(::IceInternal::Stream*, " << name << "&);";
    C << sp;
    C << nl << "void" << nl << scope << "::__write(::IceInternal::Stream* __os, " << scoped << " v)";
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
    C << nl << "void" << nl << scope << "::__read(::IceInternal::Stream* __is, " << scoped << "& v)";
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
Slice::Gen::TypesVisitor::visitNative(const NativePtr& p)
{
    string name = p -> name();

    H << sp;
    H << nl << "typedef ::IceNative::" << name << ' ' << name << ';';
}

Slice::Gen::ProxyDeclVisitor::ProxyDeclVisitor(Output& h, Output& c, const string& dllExport)
    : H(h), C(c), _dllExport(dllExport)
{
}

void
Slice::Gen::ProxyDeclVisitor::visitUnitStart(const UnitPtr& p)
{
    if(!p -> hasProxies())
	return;

    H << sp;
    H << nl << "namespace IceProxy" << nl << '{';
}

void
Slice::Gen::ProxyDeclVisitor::visitUnitEnd(const UnitPtr& p)
{
    if(!p -> hasProxies())
	return;

    H << sp;
    H << nl << '}';
}
    
void
Slice::Gen::ProxyDeclVisitor::visitModuleStart(const ModulePtr& p)
{
    if(!p -> hasProxies())
	return;

    string name = p -> name();
    
    H << sp;
    H << nl << "namespace " << name << nl << '{';
}

void
Slice::Gen::ProxyDeclVisitor::visitModuleEnd(const ModulePtr& p)
{
    if(!p -> hasProxies())
	return;

    H << sp;
    H << nl << '}';
}

void
Slice::Gen::ProxyDeclVisitor::visitClassDecl(const ClassDeclPtr& p)
{
    if(p -> isLocal())
	return;

    string name = p -> name();

    H << sp;
    H << nl << "class " << name << ';';
}

Slice::Gen::ProxyVisitor::ProxyVisitor(Output& h, Output& c, const string& dllExport)
    : H(h), C(c), _dllExport(dllExport)
{
}

void
Slice::Gen::ProxyVisitor::visitUnitStart(const UnitPtr& p)
{
    if(!p -> hasProxies())
	return;

    H << sp;
    H << nl << "namespace IceProxy" << nl << '{';
}

void
Slice::Gen::ProxyVisitor::visitUnitEnd(const UnitPtr& p)
{
    if(!p -> hasProxies())
	return;

    H << sp;
    H << nl << '}';
}
    
void
Slice::Gen::ProxyVisitor::visitModuleStart(const ModulePtr& p)
{
    if(!p -> hasProxies())
	return;

    string name = p -> name();
    
    H << sp;
    H << nl << "namespace " << name << nl << '{';
}

void
Slice::Gen::ProxyVisitor::visitModuleEnd(const ModulePtr& p)
{
    if(!p -> hasProxies())
	return;

    H << sp;
    H << nl << '}';
}

void
Slice::Gen::ProxyVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    if(p -> isLocal())
	return;

    string name = p -> name();
    string scoped = p -> scoped();
    ClassList bases = p -> bases();

    H << sp;
    H << nl << "class" << _dllExport << ' ' << name << " : ";
    if(bases.empty())
	H << "virtual public ::IceProxy::Ice::Object";
    else
    {
	H.useCurrentPosAsIndent();
	ClassList::iterator q = bases.begin();
	while(q != bases.end())
	{
	    H << "virtual public ::IceProxy" << (*q) -> scoped();
	    if(++q != bases.end())
		H << ',' << nl;
	}
	H.restoreIndent();
    }

    H << sb;
    H.dec();
    H << nl << "public: ";
    H.inc();

    H << sp;
    H << nl << "virtual void _throw();";
    C << sp << nl << "void" << nl << "IceProxy" << scoped << "::_throw()";
    C << sb;
    C << nl << "throw " << scoped << "PrxE(this);";
    C << eb;
}

void
Slice::Gen::ProxyVisitor::visitClassDefEnd(const ClassDefPtr& p)
{
    if(p -> isLocal())
	return;

    string scoped = p -> scoped();
    
    H.dec();
    H << sp;
    H << nl << "private: ";
    H.inc();
    H << sp;
    H << nl << "virtual ::IceInternal::Handle< ::IceDelegateM::Ice::Object> __createDelegateM();";
    H << eb << ';';
    C << sp;
    C << nl << "::IceInternal::Handle< ::IceDelegateM::Ice::Object>";
    C << nl << "IceProxy" << scoped << "::__createDelegateM()";
    C << sb;
    C << nl << "return ::IceInternal::Handle< ::IceDelegateM::Ice::Object>(new ::IceDelegateM" << scoped << ");";
    C << eb;
}

void
Slice::Gen::ProxyVisitor::visitOperation(const OperationPtr& p)
{
    ContainerPtr container = p -> container();
    ClassDefPtr cl = ClassDefPtr::dynamicCast(container);
    if(cl -> isLocal())
	return;

    string name = p -> name();
    string scoped = p -> scoped();
    string scope = p -> scope();

    TypePtr ret = p -> returnType();
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
    C << nl << retS << nl << "IceProxy" << scoped << paramsDecl;
    C << sb;
    C << nl << "::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase = __getDelegate();";
    C << nl << "::IceDelegate" << scope << "* __del = dynamic_cast< ::IceDelegate" << scope << "*>(__delBase.get());";
    C << nl;
    if(ret)
	C << "return ";
    C << "__del -> " << name << args << ";";
    C << eb;
}

Slice::Gen::DelegateVisitor::DelegateVisitor(Output& h, Output& c, const string& dllExport)
    : H(h), C(c), _dllExport(dllExport)
{
}

void
Slice::Gen::DelegateVisitor::visitUnitStart(const UnitPtr& p)
{
    if(!p -> hasProxies())
	return;

    H << sp;
    H << nl << "namespace IceDelegate" << nl << '{';
}

void
Slice::Gen::DelegateVisitor::visitUnitEnd(const UnitPtr& p)
{
    if(!p -> hasProxies())
	return;

    H << sp;
    H << nl << '}';
}
    
void
Slice::Gen::DelegateVisitor::visitModuleStart(const ModulePtr& p)
{
    if(!p -> hasProxies())
	return;

    string name = p -> name();
    
    H << sp;
    H << nl << "namespace " << name << nl << '{';
}

void
Slice::Gen::DelegateVisitor::visitModuleEnd(const ModulePtr& p)
{
    if(!p -> hasProxies())
	return;

    H << sp;
    H << nl << '}';
}

void
Slice::Gen::DelegateVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    if(p -> isLocal())
	return;

    string name = p -> name();
    ClassList bases = p -> bases();

    H << sp;
    H << nl << "class" << _dllExport << ' ' << name << " : ";
    if(bases.empty())
	H << "virtual public ::IceDelegate::Ice::Object";
    else
    {
	H.useCurrentPosAsIndent();
	ClassList::iterator q = bases.begin();
	while(q != bases.end())
	{
	    H << "virtual public ::IceDelegate" << (*q) -> scoped();
	    if(++q != bases.end())
		H << ',' << nl;
	}
	H.restoreIndent();
    }
    H << sb;
    H.dec();
    H << nl << "public: ";
    H.inc();
}

void
Slice::Gen::DelegateVisitor::visitClassDefEnd(const ClassDefPtr& p)
{
    if(p -> isLocal())
	return;

    H << eb << ';';
}

void
Slice::Gen::DelegateVisitor::visitOperation(const OperationPtr& p)
{
    ContainerPtr container = p -> container();
    ClassDefPtr cl = ClassDefPtr::dynamicCast(container);
    if(cl -> isLocal())
	return;

    string name = p -> name();

    TypePtr ret = p -> returnType();
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

Slice::Gen::DelegateMVisitor::DelegateMVisitor(Output& h, Output& c, const string& dllExport)
    : H(h), C(c), _dllExport(dllExport)
{
}

void
Slice::Gen::DelegateMVisitor::visitUnitStart(const UnitPtr& p)
{
    if(!p -> hasProxies())
	return;

    H << sp;
    H << nl << "namespace IceDelegateM" << nl << '{';
}

void
Slice::Gen::DelegateMVisitor::visitUnitEnd(const UnitPtr& p)
{
    if(!p -> hasProxies())
	return;

    H << sp;
    H << nl << '}';
}
    
void
Slice::Gen::DelegateMVisitor::visitModuleStart(const ModulePtr& p)
{
    if(!p -> hasProxies())
	return;

    string name = p -> name();
    
    H << sp;
    H << nl << "namespace " << name << nl << '{';
}

void
Slice::Gen::DelegateMVisitor::visitModuleEnd(const ModulePtr& p)
{
    if(!p -> hasProxies())
	return;

    H << sp;
    H << nl << '}';
}

void
Slice::Gen::DelegateMVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    if(p -> isLocal())
	return;

    string name = p -> name();
    string scoped = p -> scoped();
    ClassList bases = p -> bases();

    H << sp;
    H << nl << "class" << _dllExport << ' ' << name << " : ";
    H.useCurrentPosAsIndent();
    H << "virtual public ::IceDelegate" << scoped << ',';
    if(bases.empty())
	H << nl << "virtual public ::IceDelegateM::Ice::Object";
    else
    {
	ClassList::iterator q = bases.begin();
	while(q != bases.end())
	{
	    H << nl << "virtual public ::IceDelegateM" << (*q) -> scoped();
	    if(++q != bases.end())
		H << ',';
	}
    }
    H.restoreIndent();
    H << sb;
    H.dec();
    H << nl << "public: ";
    H.inc();
}

void
Slice::Gen::DelegateMVisitor::visitClassDefEnd(const ClassDefPtr& p)
{
    if(p -> isLocal())
	return;

    H << eb << ';';
}

void
Slice::Gen::DelegateMVisitor::visitOperation(const OperationPtr& p)
{
    ContainerPtr container = p -> container();
    ClassDefPtr cl = ClassDefPtr::dynamicCast(container);
    if(cl -> isLocal())
	return;

    string name = p -> name();
    string scoped = p -> scoped();
    string scope = p -> scope();

    TypePtr ret = p -> returnType();
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
    C << nl << retS << nl << "IceDelegateM" << scoped << paramsDecl;
    C << sb;
    C << nl << "::IceInternal::Outgoing __out(__emitter(), __reference());";
    if(ret || !outParams.empty() || !throws.empty())
	C << nl << "::IceInternal::Stream* __is = __out.is();";
    C << nl << "::IceInternal::Stream* __os = __out.os();";
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
	    C << nl << "case " << cnt++ << ':';
	    C << sb;
	    TypeStringList li;
	    li.push_back(make_pair(*r, string("__ex")));
	    writeAllocateCode(C, li, 0);
	    writeUnmarshalCode(C, li, 0);
	    if(ClassDeclPtr::dynamicCast(*r) || ProxyPtr::dynamicCast(*r))
		C << nl << "__ex -> _throw();";
	    else
		C << nl << "throw __ex;";
	    C << eb;
	}
	C << eb;
	C << nl << "throw ::Ice::UnknownUserException(__FILE__, __LINE__);";
	C << eb;
    }
    else
    {
	C.inc();
	C << nl << "throw ::Ice::UnknownUserException(__FILE__, __LINE__);";
	C.dec();
    }
    writeAllocateCode(C, TypeStringList(), ret);
    writeUnmarshalCode(C, outParams, ret);
    if(ret)
	C << nl << "return __ret;";
    C << eb;
}

Slice::Gen::ObjectDeclVisitor::ObjectDeclVisitor(Output& h, Output& c, const string& dllExport)
    : H(h), C(c), _dllExport(dllExport)
{
}

void
Slice::Gen::ObjectDeclVisitor::visitModuleStart(const ModulePtr& p)
{
    if(!p -> hasClassDecls())
	return;

    string name = p -> name();
    
    H << sp;
    H << nl << "namespace " << name << nl << '{';
}

void
Slice::Gen::ObjectDeclVisitor::visitModuleEnd(const ModulePtr& p)
{
    if(!p -> hasClassDecls())
	return;

    H << sp;
    H << nl << '}';
}

void
Slice::Gen::ObjectDeclVisitor::visitClassDecl(const ClassDeclPtr& p)
{
    string name = p -> name();
    
    H << sp;
    H << nl << "class " << name << ';';
}

Slice::Gen::ObjectVisitor::ObjectVisitor(Output& h, Output& c, const string& dllExport)
    : H(h), C(c), _dllExport(dllExport)
{
}

void
Slice::Gen::ObjectVisitor::visitModuleStart(const ModulePtr& p)
{
    if(!p -> hasClassDefs())
	return;

    string name = p -> name();
    
    H << sp;
    H << nl << "namespace " << name << nl << '{';
}

void
Slice::Gen::ObjectVisitor::visitModuleEnd(const ModulePtr& p)
{
    if(!p -> hasClassDefs())
	return;

    H << sp;
    H << nl << '}';
}

void
Slice::Gen::ObjectVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    string name = p -> name();
    string scoped = p -> scoped();
    ClassList bases = p -> bases();
    
    H << sp;
    H << nl << "class" << _dllExport << ' ' << name << "PtrE : ";
    H.useCurrentPosAsIndent();
    if(bases.empty())
    {
	if(p -> isLocal())
	    H << "virtual public ::Ice::LocalObjectPtrE";
	else
	    H << "virtual public ::Ice::ObjectPtrE";
    }
    else
    {
	ClassList::iterator q = bases.begin();
	while(q != bases.end())
	{
	    H << "virtual public " << (*q) -> scoped() << "PtrE";
	    if(++q != bases.end())
		H << ',' << nl;
	}
    }
    H.restoreIndent();
    H << sb;
    H.dec();
    H << nl << "public: ";
    H.inc();
    H << sp;
    H << nl << name << "PtrE() { }";
    H << nl << name << "PtrE(const " << name << "PtrE&);";
    H << nl << "explicit " << name << "PtrE(const " << name << "Ptr&);";
    H << nl << "operator " << name << "Ptr() const;";
    H << nl << name << "* operator->() const;";
    H << eb << ';';
    C << sp << nl << scoped.substr(2) << "PtrE::" << name << "PtrE(const " << name << "PtrE& p)";
    C << sb;
    C << nl << "_ptr = p._ptr;";
    C << eb;
    C << sp << nl << scoped.substr(2) << "PtrE::" << name << "PtrE(const " << scoped << "Ptr& p)";
    C << sb;
    C << nl << "_ptr = p;";
    C << eb;
    C << sp << nl << scoped.substr(2) << "PtrE::operator " << scoped << "Ptr() const";
    C << sb;
    C << nl << "return " << scoped << "Ptr(dynamic_cast< " << scoped << "*>(_ptr.get()));";
    C << eb;
    C << sp << nl << scoped << '*' << nl << scoped.substr(2) << "PtrE::operator->() const";
    C << sb;
    C << nl << "return dynamic_cast< " << scoped << "*>(_ptr.get());";
    C << eb;

    if(!p -> isLocal())
    {
	H << sp;
	H << nl << "class" << _dllExport << ' ' << name << "PrxE : ";
	H.useCurrentPosAsIndent();
	if(bases.empty())
	    H << "virtual public ::Ice::ObjectPrxE";
	else
	{
	    ClassList::iterator q = bases.begin();
	    while(q != bases.end())
	    {
		H << "virtual public " << (*q) -> scoped() << "PrxE";
		if(++q != bases.end())
		    H << ',' << nl;
	    }
	}
	H.restoreIndent();
	H << sb;
	H.dec();
	H << nl << "public: ";
	H.inc();
	H << sp;
	H << nl << name << "PrxE() { }";
	H << nl << name << "PrxE(const " << name << "PrxE&);";
	H << nl << "explicit " << name << "PrxE(const " << name << "Prx&);";
	H << nl << "operator " << name << "Prx() const;";
	H << nl << "::IceProxy" << scoped << "* operator->() const;";
	H << eb << ';';
	C << sp << nl << scoped.substr(2) << "PrxE::" << name << "PrxE(const " << name << "PrxE& p)";
	C << sb;
	C << nl << "_prx = p._prx;";
	C << eb;
	C << sp << nl << scoped.substr(2) << "PrxE::" << name << "PrxE(const " << scoped << "Prx& p)";
	C << sb;
	C << nl << "_prx = p;";
	C << eb;
	C << sp << nl << scoped.substr(2) << "PrxE::operator " << scoped << "Prx() const";
	C << sb;
	C << nl << "return " << scoped << "Prx(dynamic_cast< ::IceProxy" << scoped << "*>(_prx.get()));";
	C << eb;
	C << sp << nl << "::IceProxy" << scoped << '*';
	C << nl << scoped.substr(2) << "PrxE::operator->() const";
	C << sb;
	C << nl << "return dynamic_cast< ::IceProxy" << scoped << "*>(_prx.get());";
	C << eb;
    }

    H << sp;
    H << nl << "class" << _dllExport << ' ' << name << " : ";
    H.useCurrentPosAsIndent();
    if(bases.empty())
    {
	if(p -> isLocal())
	    H << "virtual public ::Ice::LocalObject";
	else
	    H << "virtual public ::Ice::Object";
    }
    else
    {
	ClassList::iterator q = bases.begin();
	while(q != bases.end())
	{
	    H << "virtual public " << (*q) -> scoped();
	    if(++q != bases.end())
		H << ',' << nl;
	}
    }
    if(!p -> isLocal())
 	H << ',' << nl << "virtual public ::IceDelegate" << scoped;
    H.restoreIndent();
    H << sb;
    H.dec();
    H << nl << "public: ";
    H.inc();

    H << sp;
    H << nl << "virtual void _throw();";
    C << sp << nl << "void" << nl << scoped.substr(2) << "::_throw()";
    C << sb;
    C << nl << "throw " << scoped << "PtrE(this);";
    C << eb;

    if(!p -> isLocal())
    {
	ClassList allBases = p -> allBases();
	StringList ids;
	transform(allBases.begin(), allBases.end(),
		  back_inserter(ids),
		  ::Ice::memFun(&ClassDef::scoped));
	StringList other;
	other.push_back(scoped);
	other.push_back("::Ice::Object");
	other.sort();
	ids.merge(other);
	ids.unique();
	
	ClassList allBaseClasses;
	ClassDefPtr cl;
	if(!bases.empty())
	    cl = bases.front();
	else
	    cl = 0;
	while(cl && !cl -> isInterface())
	{
	    allBaseClasses.push_back(cl);
	    ClassList baseBases = cl -> bases();
	    if(!baseBases.empty())
		cl = baseBases.front();
	    else
		cl = 0;
	}
	StringList classIds;
	transform(allBaseClasses.begin(), allBaseClasses.end(),
		  back_inserter(classIds),
		  ::Ice::memFun(&ClassDef::scoped));
	if(!p -> isInterface())
	    classIds.push_front(scoped);
	classIds.push_back("::Ice::Object");

	StringList::iterator q;

	H << sp;
	H << nl << "static std::string __ids[" << ids.size() << "];";
	H << sp;
	H << nl << "static std::string __classIds[" << classIds.size() << "];";
	H << sp;
	H << nl << "virtual bool _isA(const std::string&);";
	H << sp;
	H << nl << "virtual const std::string* _classIds();";
	C << sp;
	C << nl << "std::string " << scoped.substr(2) << "::__ids[" << ids.size() << "] =";
	C << sb;
	q = ids.begin();
	while(q != ids.end())
	{
	    C << nl << '"' << *q << '"';
	    if(++q != ids.end())
		C << ',';
	}
	C << eb << ';';
	C << sp;
	C << nl << "std::string " << scoped.substr(2) << "::__classIds[" << classIds.size() << "] =";
	C << sb;
	q = classIds.begin();
	while(q != classIds.end())
	{
	    C << nl << '"' << *q << '"';
	    if(++q != classIds.end())
		C << ',';
	}
	C << eb << ';';
	C << sp;
	C << nl << "bool" << nl << scoped.substr(2) << "::_isA(const std::string& s)";
	C << sb;
	C << nl << "std::string* b = __ids;";
	C << nl << "std::string* e = __ids + " << ids.size() << ';';
	C << nl << "std::pair<std::string*, std::string*> r = std::equal_range(b, e, s);";
	C << nl << "return r.first != r.second;";
	C << eb;
	C << sp;
	C << nl << "const std::string*" << nl << scoped.substr(2) << "::_classIds()";
	C << sb;
	C << nl << "return __classIds;";
	C << eb;
    }
}
    
void
Slice::Gen::ObjectVisitor::visitClassDefEnd(const ClassDefPtr& p)
{
    string name = p -> name();
    string scoped = p -> scoped();

    ClassList bases = p -> bases();
    ClassDefPtr base;
    if(!bases.empty() && !bases.front() -> isInterface())
	base = bases.front();
    
    if(!p -> isLocal())
    {
	OperationList allOperations = p -> allOperations();
	if(!p -> allOperations().empty())
	{
	    StringList allOpNames;
	    transform(allOperations.begin(), allOperations.end(), back_inserter(allOpNames),
		      ::Ice::memFun(&Operation::name));
	    StringList other;
	    other.push_back("_isA");
	    //other.sort();
	    allOpNames.merge(other);
	    allOpNames.unique();
	    
	    StringList::iterator q;
	    
	    H << sp;
	    H << nl << "static std::string __names[" << allOpNames.size() << "];";
	    H << nl << "virtual ::IceInternal::DispatchStatus "
	      << "__dispatch(::IceInternal::Incoming&, const std::string&);";
	    C << sp;
	    C << nl << "std::string " << scoped.substr(2) << "::__names[" << allOpNames.size() << "] =";
	    C << sb;
	    q = allOpNames.begin();
	    while(q != allOpNames.end())
	    {
		C << nl << '"' << *q << '"';
		if(++q != allOpNames.end())
		    C << ',';
	    }
	    C << eb << ';';
	    C << sp;
	    C << nl << "::IceInternal::DispatchStatus" << nl << scoped.substr(2)
	      << "::__dispatch(::IceInternal::Incoming& in, const std::string& s)";
	    C << sb;
	    C << nl << "std::string* b = __names;";
	    C << nl << "std::string* e = __names + " << allOpNames.size() << ';';
	    C << nl << "std::pair<std::string*, std::string*> r = std::equal_range(b, e, s);";
	    C << nl << "if(r.first == r.second)";
	    C.inc();
	    C << nl << "return ::IceInternal::DispatchOperationNotExist;";
	    C.dec();
	    C << sp;
	    C << nl << "switch(r.first - __names)";
	    C << sb;
	    int i = 0;
	    for(q = allOpNames.begin(); q != allOpNames.end(); ++q)
	    {
		C << nl << "case " << i++ << ':';
		C << sb;
		C << nl << "return ___" << *q << "(in);";
		C << eb;
	    }
	    C << eb;
	    C << sp;
	    C << nl << "assert(false);";
	    C << nl << "return ::IceInternal::DispatchOperationNotExist;";
	    C << eb;
	}
    }
    H << sp;
    H << nl << "virtual void __write(::IceInternal::Stream*);";
    H << nl << "virtual void __read(::IceInternal::Stream*);";
    H << eb << ';';
    TypeStringList memberList;
    DataMemberList dataMembers = p -> dataMembers();
    DataMemberList::const_iterator q;
    for(q = dataMembers.begin(); q != dataMembers.end(); ++q)
	memberList.push_back(make_pair((*q) -> type(), (*q) -> name()));
    C << sp;
    C << nl << "void" << nl << scoped.substr(2) << "::__write(::IceInternal::Stream* __os)";
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
    C << nl << "void" << nl << scoped.substr(2) << "::__read(::IceInternal::Stream* __is)";
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
Slice::Gen::ObjectVisitor::visitOperation(const OperationPtr& p)
{
    ContainerPtr container = p -> container();
    ClassDefPtr cl = ClassDefPtr::dynamicCast(container);
    string name = p -> name();
    string scoped = p -> scoped();
    string scope = p -> scope();

    TypePtr ret = p -> returnType();
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
	H << nl << "::IceInternal::DispatchStatus ___" << name << "(::IceInternal::Incoming&);";
	C << sp;
	C << nl << "::IceInternal::DispatchStatus" << nl << scope.substr(2) << "::___" << name
	  << "(::IceInternal::Incoming& __in)";
	C << sb;
	if(!inParams.empty())
	    C << nl << "::IceInternal::Stream* __is = __in.is();";
	if(ret || !outParams.empty() || !throws.empty())
	    C << nl << "::IceInternal::Stream* __os = __in.os();";
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
		C << nl << "catch(" << exceptionTypeToString(*r) << " __ex)";
		C << sb;
		C << nl << "__os -> write(" << cnt++ << ");";
		if(ClassDeclPtr::dynamicCast(*r) || ProxyPtr::dynamicCast(*r))
		{
		    string s = "static_cast< ";
		    s += inputTypeToString(*r);
		    s += ">(__ex)";
		    writeMarshalUnmarshalCode(C, *r, s, true);
		}
		else
		    writeMarshalUnmarshalCode(C, *r, "__ex", true);
		C << nl << "return ::IceInternal::DispatchException;";
		C << eb;
	    }
	}
	writeMarshalCode(C, outParams, ret);
	C << nl << "return ::IceInternal::DispatchOK;";
	C << eb;
    }	
}

void
Slice::Gen::ObjectVisitor::visitDataMember(const DataMemberPtr& p)
{
    string name = p -> name();
    string s = typeToString(p -> type());
    H << sp;
    H << nl << s << ' ' << name << ';';
}

Slice::Gen::IceVisitor::IceVisitor(Output& h, Output& c, const string& dllExport)
    : H(h), C(c), _dllExport(dllExport)
{
}

void
Slice::Gen::IceVisitor::visitUnitStart(const UnitPtr& p)
{
    if(!p -> hasClassDecls())
	return;

    H << sp;
    H << nl << "namespace IceInternal" << nl << '{';
}

void
Slice::Gen::IceVisitor::visitUnitEnd(const UnitPtr& p)
{
    if(!p -> hasClassDecls())
	return;

    H << sp;
    H << nl << '}';
}
    
void
Slice::Gen::IceVisitor::visitClassDecl(const ClassDeclPtr& p)
{
    string scoped = p -> scoped();
    
    H << sp;
    H << nl << "void" << _dllExport << " incRef(" << scoped << "*);";
    H << nl << "void" << _dllExport << " decRef(" << scoped << "*);";
    if(!p -> isLocal())
    {
	H << sp;
	H << nl << "void" << _dllExport << " incRef(::IceProxy" << scoped << "*);";
	H << nl << "void" << _dllExport << " decRef(::IceProxy" << scoped << "*);";
	H << sp;
	H << nl << "void" << _dllExport << " checkedCast(::IceProxy::Ice::Object*, ::IceProxy" << scoped << "*&);";
	H << nl << "void" << _dllExport << " uncheckedCast(::IceProxy::Ice::Object*, ::IceProxy" << scoped << "*&);";
    }
}

void
Slice::Gen::IceVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    string scoped = p -> scoped();
    
    C << sp;
    C << nl << "void IceInternal::incRef(" << scoped << "* p) { p -> __incRef(); }";
    C << nl << "void IceInternal::decRef(" << scoped << "* p) { p -> __decRef(); }";

    if(!p -> isLocal())
    {
	C << sp;
	C << nl << "void IceInternal::incRef(::IceProxy" << scoped << "* p) { p -> __incRef(); }";
	C << nl << "void IceInternal::decRef(::IceProxy" << scoped << "* p) { p -> __decRef(); }";
	C << sp;
	C << nl << "void IceInternal::checkedCast(::IceProxy::Ice::Object* b, ::IceProxy" << scoped << "*& d)";
	C << sb;
	C << nl << "d = dynamic_cast< ::IceProxy" << scoped << "*>(b);";
	C << nl << "if(!d && b -> _isA(\"" << scoped << "\"))";
	C << sb;
	C << nl << "d = new ::IceProxy" << scoped << ';';
	C << nl << "b -> __copyTo(d);";
	C << eb;
	C << eb;
	C << sp;
	C << nl << "void IceInternal::uncheckedCast(::IceProxy::Ice::Object* b, ::IceProxy" << scoped << "*& d)";
	C << sb;
	C << nl << "d = dynamic_cast< ::IceProxy" << scoped << "*>(b);";
	C << nl << "if(!d)";
	C << sb;
	C << nl << "d = new ::IceProxy" << scoped << ';';
	C << nl << "b -> __copyTo(d);";
	C << eb;
	C << eb;
    }
}

Slice::Gen::HandleVisitor::HandleVisitor(Output& h, Output& c, const string& dllExport)
    : H(h), C(c), _dllExport(dllExport)
{
}

void
Slice::Gen::HandleVisitor::visitModuleStart(const ModulePtr& p)
{
    if(!p -> hasClassDecls())
	return;

    string name = p -> name();
    
    H << sp;
    H << nl << "namespace " << name << nl << '{';
}

void
Slice::Gen::HandleVisitor::visitModuleEnd(const ModulePtr& p)
{
    if(!p -> hasClassDecls())
	return;

    H << sp;
    H << nl << '}';
}

void
Slice::Gen::HandleVisitor::visitClassDecl(const ClassDeclPtr& p)
{
    string name = p -> name();
    string scoped = p -> scoped();
    
    H << sp;
    H << nl << "typedef ::IceInternal::Handle< " << scoped << "> " << name << "Ptr;";
    if(!p -> isLocal())
    {
	H << nl << "typedef ::IceInternal::ProxyHandle< ::IceProxy" << scoped << "> " << name << "Prx;";
	H << sp;
	H << nl << "void" << _dllExport << " __write(::IceInternal::Stream*, const " << name << "Prx&);";
	H << nl << "void" << _dllExport << " __read(::IceInternal::Stream*, " << name << "Prx&);";
    }
}

void
Slice::Gen::HandleVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    if(p -> isLocal())
	return;

    string scoped = p -> scoped();
    string scope = p -> scope();
    if(scope.length())
	scope.erase(0, 2);

    C << sp;
    C << nl << "void" << nl << scope << "::__write(::IceInternal::Stream* __os, const " << scoped << "Prx& v)";
    C << sb;
    // TODO: Should be ::Ice::__write
    C << nl << "::IceInternal::write(__os, ::Ice::ObjectPrx(v));";
    C << eb;
    C << sp;
    C << nl << "void" << nl << scope << "::__read(::IceInternal::Stream* __is, " << scoped << "Prx& v)";
    C << sb;
    C << nl << "::Ice::ObjectPrx proxy;";
    // TODO: Should be ::Ice::__read
    C << nl << "::IceInternal::read(__is, proxy);";
    C << nl << "v = new ::IceProxy" << scoped << ';';
    C << nl << "proxy -> __copyTo(v.get());";
    C << eb;
}
