// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <IceUtil/Functional.h>
#include <Slice/CPlusPlusUtil.h>
#include <Gen.h>
#include <limits>

using namespace std;
using namespace Slice;

Slice::Gen::Gen(const string& name, const string& base,	const string& include, const vector<string>& includePaths,
		const string& dllExport, const string& dir) :
    _base(base),
    _include(include),
    _includePaths(includePaths),
    _dllExport(dllExport)
{
    for (vector<string>::iterator p = _includePaths.begin(); p != _includePaths.end(); ++p)
    {
	if (p->length() && (*p)[p->length() - 1] != '/')
	{
	    *p += '/';
	}
    }

    string::size_type pos = _base.rfind('/');
    if (pos != string::npos)
    {
	_base.erase(0, pos + 1);
    }

    string fileH = _base + ".h";
    string fileC = _base + ".cpp";
    if (!dir.empty())
    {
	fileH = dir + '/' + fileH;
	fileC = dir + '/' + fileC;
    }

    H.open(fileH.c_str());
    if (!H)
    {
	cerr << name << ": can't open `" << fileH << "' for writing: " << strerror(errno) << endl;
	return;
    }
    
    C.open(fileC.c_str());
    if (!C)
    {
	cerr << name << ": can't open `" << fileC << "' for writing: " << strerror(errno) << endl;
	return;
    }

    printHeader(H);
    printHeader(C);
    H << "\n// Generated from file `" << changeInclude(_base, _includePaths) << ".ice'\n";
    C << "\n// Generated from file `" << changeInclude(_base, _includePaths) << ".ice'\n";

    string s = fileH;
    if (_include.size())
    {
	s = _include + '/' + s;
    }
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
    if (_include.size())
    {
	C << _include << '/';
    }
    C << _base << ".h>";

    H << "\n#include <Ice/ProxyF.h>";
    H << "\n#include <Ice/ObjectF.h>";
    H << "\n#include <Ice/LocalObjectF.h>";
    if (unit->hasProxies())
    {
	H << "\n#include <Ice/Proxy.h>";
	H << "\n#include <Ice/Object.h>";
	H << "\n#include <Ice/Outgoing.h>";
	H << "\n#include <Ice/Incoming.h>";
	H << "\n#include <Ice/Direct.h>";
	H << "\n#include <Ice/LocalException.h>";
    }
    else
    {
	H << "\n#include <Ice/LocalObject.h>";
	C << "\n#include <Ice/IntStream.h>";
    }

    StringList includes = unit->includeFiles();
    for (StringList::const_iterator q = includes.begin(); q != includes.end(); ++q)
    {
	H << "\n#include <" << changeInclude(*q, _includePaths) << ".h>";
    }

    printVersionCheck(H);
    printVersionCheck(C);

    printDllExportStuff(H, _dllExport);
    if (_dllExport.size())
    {
	_dllExport += " ";
    }

    ProxyDeclVisitor proxyDeclVisitor(H, C, _dllExport);
    unit->visit(&proxyDeclVisitor);

    ObjectDeclVisitor objectDeclVisitor(H, C, _dllExport);
    unit->visit(&objectDeclVisitor);

    IceVisitor iceVisitor(H, C, _dllExport);
    unit->visit(&iceVisitor);

    HandleVisitor handleVisitor(H, C, _dllExport);
    unit->visit(&handleVisitor);

    TypesVisitor typesVisitor(H, C, _dllExport);
    unit->visit(&typesVisitor);

    ProxyVisitor proxyVisitor(H, C, _dllExport);
    unit->visit(&proxyVisitor);

    DelegateVisitor delegateVisitor(H, C, _dllExport);
    unit->visit(&delegateVisitor);

    DelegateMVisitor delegateMVisitor(H, C, _dllExport);
    unit->visit(&delegateMVisitor);

    DelegateDVisitor delegateDVisitor(H, C, _dllExport);
    unit->visit(&delegateDVisitor);

    ObjectVisitor objectVisitor(H, C, _dllExport);
    unit->visit(&objectVisitor);
}

Slice::Gen::TypesVisitor::TypesVisitor(Output& h, Output& c, const string& dllExport) :
    H(h), C(c), _dllExport(dllExport)
{
}

bool
Slice::Gen::TypesVisitor::visitModuleStart(const ModulePtr& p)
{
    if (!p->hasOtherConstructedTypes())
    {
	return false;
    }

    string name = p->name();
    
    H << sp;
    H << nl << "namespace " << name << nl << '{';

    return true;
}

void
Slice::Gen::TypesVisitor::visitModuleEnd(const ModulePtr& p)
{
    H << sp;
    H << nl << '}';
}

bool
Slice::Gen::TypesVisitor::visitClassDefStart(const ClassDefPtr&)
{
    return false;
}

bool
Slice::Gen::TypesVisitor::visitStructStart(const StructPtr& p)
{
    string name = p->name();

    H << sp;
    H << nl << "struct " << name;
    H << sb;

    return true;
}

void
Slice::Gen::TypesVisitor::visitStructEnd(const StructPtr& p)
{
    string name = p->name();
    string scoped = p->scoped();

    H << sp;
    H << nl << _dllExport << "void __write(::IceInternal::IntStream*) const;"; // NOT virtual!
    H << nl << _dllExport << "void __read(::IceInternal::IntStream*);"; // NOT virtual!
    H << eb << ';';
    
    TypeStringList memberList;
    DataMemberList dataMembers = p->dataMembers();
    for (DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
	memberList.push_back(make_pair((*q)->type(), (*q)->name()));
    }
    C << sp;
    C << nl << "void" << nl << scoped.substr(2) << "::__write(::IceInternal::IntStream* __os) const";
    C << sb;
    writeMarshalCode(C, memberList, 0);
    C << eb;
    C << sp;
    C << nl << "void" << nl << scoped.substr(2) << "::__read(::IceInternal::IntStream* __is)";
    C << sb;
    writeUnmarshalCode(C, memberList, 0);
    C << eb;
}

void
Slice::Gen::TypesVisitor::visitDataMember(const DataMemberPtr& p)
{
    string name = p->name();
    string s = typeToString(p->type());
    H << sp;
    H << nl << s << ' ' << name << ';';
}

void
Slice::Gen::TypesVisitor::visitSequence(const SequencePtr& p)
{
    string name = p->name();
    TypePtr type = p->type();
    string s = typeToString(type);
    if (s[0] == ':')
    {
	s.insert(0, " ");
    }
    H << sp;
    H << nl << "typedef ::std::vector<" << s << "> " << name << ';';

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if (!builtin)
    {
	string scoped = p->scoped();
	string scope = p->scope();

	H << sp;
	H << nl << "class __U__" << name << " { };";
	H << nl << _dllExport << "void __write(::IceInternal::IntStream*, const " << name << "&, __U__" << name << ");";
	H << nl << _dllExport << "void __read(::IceInternal::IntStream*, " << name << "&, __U__" << name << ");";
	C << sp;
	C << nl << "void" << nl << scope.substr(2) << "__write(::IceInternal::IntStream* __os, const " << scoped
	  << "& v, " << scope << "__U__" << name << ')';
	C << sb;
	C << nl << "__os->write(::Ice::Int(v.size()));";
	C << nl << scoped << "::const_iterator p;";
	C << nl << "for (p = v.begin(); p != v.end(); ++p)";
	C << sb;
	writeMarshalUnmarshalCode(C, type, "*p", true);
	C << eb;
	C << eb;
	C << sp;
	C << nl << "void" << nl << scope.substr(2) << "__read(::IceInternal::IntStream* __is, " << scoped
	  << "& v, " << scope << "__U__" << name << ')';
	C << sb;
	C << nl << "::Ice::Int sz;";
	C << nl << "__is->read(sz);";
	// Don't use v.resize(sz) or v.reserve(sz) here, as it
	// cannot be checked whether sz is a reasonable value
	C << nl << "while (sz--)";
	C << sb;
	C.zeroIndent();
	C << nl << "#ifdef WIN32"; // STLBUG
	C.restoreIndent();
	C << nl << "v.push_back(" << typeToString(type) << "());";
	C.zeroIndent();
	C << nl << "#else";
	C.restoreIndent();
	C << nl << "v.push_back();";
	C.zeroIndent();
	C << nl << "#endif";
	C.restoreIndent();
	writeMarshalUnmarshalCode(C, type, "v.back()", false);
	C << eb;
	C << eb;
    }
}

void
Slice::Gen::TypesVisitor::visitDictionary(const DictionaryPtr& p)
{
    string name = p->name();
    TypePtr keyType = p->keyType();
    TypePtr valueType = p->valueType();
    string ks = typeToString(keyType);
    if (ks[0] == ':')
    {
	ks.insert(0, " ");
    }
    string vs = typeToString(valueType);
    H << sp;
    H << nl << "typedef ::std::map<" << ks << ", " << vs << "> " << name << ';';

    string scoped = p->scoped();
    string scope = p->scope();
    
    H << sp;
    H << nl << "class __U__" << name << " { };";
    H << nl << _dllExport << "void __write(::IceInternal::IntStream*, const " << name << "&, __U__" << name << ");";
    H << nl << _dllExport << "void __read(::IceInternal::IntStream*, " << name << "&, __U__" << name << ");";
    C << sp;
    C << nl << "void" << nl << scope.substr(2) << "__write(::IceInternal::IntStream* __os, const " << scoped
      << "& v, " << scope << "__U__" << name << ')';
    C << sb;
    C << nl << "__os->write(::Ice::Int(v.size()));";
    C << nl << scoped << "::const_iterator p;";
    C << nl << "for (p = v.begin(); p != v.end(); ++p)";
    C << sb;
    writeMarshalUnmarshalCode(C, keyType, "p->first", true);
    writeMarshalUnmarshalCode(C, valueType, "p->second", true);
    C << eb;
    C << eb;
    C << sp;
    C << nl << "void" << nl << scope.substr(2) << "__read(::IceInternal::IntStream* __is, " << scoped
      << "& v, " << scope << "__U__" << name << ')';
    C << sb;
    C << nl << "::Ice::Int sz;";
    C << nl << "__is->read(sz);";
    C << nl << "while (sz--)";
    C << sb;
    C << nl << "::std::pair<" << ks << ", " << vs << "> pair;";
    writeMarshalUnmarshalCode(C, keyType, "pair.first", false);
    writeMarshalUnmarshalCode(C, valueType, "pair.second", false);
    C << nl << "v.insert(v.end(), pair);";
    C << eb;
    C << eb;
}

void
Slice::Gen::TypesVisitor::visitEnum(const EnumPtr& p)
{
    string name = p->name();
    EnumeratorList enumerators = p->getEnumerators();
    H << sp;
    H << nl << "enum " << name;
    H << sb;
    EnumeratorList::const_iterator en = enumerators.begin();
    while (en != enumerators.end())
    {
	H << nl << (*en)->name();
	if (++en != enumerators.end())
	{
	    H << ',';
	}
    }
    H << eb << ';';

    string scoped = p->scoped();
    string scope = p->scope();

    int sz = enumerators.size();
    
    H << sp;
    H << nl << _dllExport << "void __write(::IceInternal::IntStream*, " << name << ");";
    H << nl << _dllExport << "void __read(::IceInternal::IntStream*, " << name << "&);";
    C << sp;
    C << nl << "void" << nl << scope.substr(2) << "__write(::IceInternal::IntStream* __os, " << scoped << " v)";
    C << sb;
    if (sz <= 0x7f)
    {
	C << nl << "__os->write(static_cast< ::Ice::Byte>(v));";
    }
    else if (sz <= 0x7fff)
    {
	C << nl << "__os->write(static_cast< ::Ice::Short>(v));";
    }
    else if (sz <= 0x7fffffff)
    {
	C << nl << "__os->write(static_cast< ::Ice::Int>(v));";
    }
    else
    {
	C << nl << "__os->write(static_cast< ::Ice::Long>(v));";
    }
    C << eb;
    C << sp;
    C << nl << "void" << nl << scope.substr(2) << "__read(::IceInternal::IntStream* __is, " << scoped << "& v)";
    C << sb;
    if (sz <= 0x7f)
    {
	C << nl << "::Ice::Byte val;";
	C << nl << "__is->read(val);";
	C << nl << "v = static_cast< " << scoped << ">(val);";
    }
    else if (sz <= 0x7fff)
    {
	C << nl << "::Ice::Short val;";
	C << nl << "__is->read(val);";
	C << nl << "v = static_cast< " << scoped << ">(val);";
    }
    else if (sz <= 0x7fffffff)
    {
	C << nl << "::Ice::Int val;";
	C << nl << "__is->read(val);";
	C << nl << "v = static_cast< " << scoped << ">(val);";
    }
    else
    {
	C << nl << "::Ice::Long val;";
	C << nl << "__is->read(val);";
	C << nl << "v = static_cast< " << scoped << ">(val);";
    }
    C << eb;
}

Slice::Gen::ProxyDeclVisitor::ProxyDeclVisitor(Output& h, Output& c, const string& dllExport) :
    H(h), C(c), _dllExport(dllExport)
{
}

bool
Slice::Gen::ProxyDeclVisitor::visitUnitStart(const UnitPtr& p)
{
    if (!p->hasProxies())
    {
	return false;
    }

    H << sp;
    H << nl << "namespace IceProxy" << nl << '{';

    return true;
}

void
Slice::Gen::ProxyDeclVisitor::visitUnitEnd(const UnitPtr& p)
{
    H << sp;
    H << nl << '}';
}
    
bool
Slice::Gen::ProxyDeclVisitor::visitModuleStart(const ModulePtr& p)
{
    if (!p->hasProxies())
    {
	return false;
    }

    string name = p->name();
    
    H << sp;
    H << nl << "namespace " << name << nl << '{';

    return true;
}

void
Slice::Gen::ProxyDeclVisitor::visitModuleEnd(const ModulePtr& p)
{
    H << sp;
    H << nl << '}';
}

void
Slice::Gen::ProxyDeclVisitor::visitClassDecl(const ClassDeclPtr& p)
{
    if (p->isLocal())
    {
	return;
    }

    string name = p->name();

    H << sp;
    H << nl << "class " << name << ';';
}

Slice::Gen::ProxyVisitor::ProxyVisitor(Output& h, Output& c, const string& dllExport) :
    H(h), C(c), _dllExport(dllExport)
{
}

bool
Slice::Gen::ProxyVisitor::visitUnitStart(const UnitPtr& p)
{
    if (!p->hasProxies())
    {
	return false;
    }

    H << sp;
    H << nl << "namespace IceProxy" << nl << '{';

    return true;
}

void
Slice::Gen::ProxyVisitor::visitUnitEnd(const UnitPtr& p)
{
    H << sp;
    H << nl << '}';
}
    
bool
Slice::Gen::ProxyVisitor::visitModuleStart(const ModulePtr& p)
{
    if (!p->hasProxies())
    {
	return false;
    }

    string name = p->name();
    
    H << sp;
    H << nl << "namespace " << name << nl << '{';

    return true;
}

void
Slice::Gen::ProxyVisitor::visitModuleEnd(const ModulePtr& p)
{
    H << sp;
    H << nl << '}';
}

bool
Slice::Gen::ProxyVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    if (p->isLocal())
    {
	return false;
    }

    string name = p->name();
    string scoped = p->scoped();
    ClassList bases = p->bases();

    H << sp;
    H << nl << "class " << _dllExport << name << " : ";
    if (bases.empty())
    {
	H << "virtual public ::IceProxy::Ice::Object";
    }
    else
    {
	H.useCurrentPosAsIndent();
	ClassList::const_iterator q = bases.begin();
	while (q != bases.end())
	{
	    H << "virtual public ::IceProxy" << (*q)->scoped();
	    if (++q != bases.end())
	    {
		H << ',' << nl;
	    }
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

    return true;
}

void
Slice::Gen::ProxyVisitor::visitClassDefEnd(const ClassDefPtr& p)
{
    string scoped = p->scoped();
    
    H.dec();
    H << sp;
    H << nl << "private: ";
    H.inc();
    H << sp;
    H << nl << "virtual ::IceInternal::Handle< ::IceDelegateM::Ice::Object> __createDelegateM();";
    H << nl << "virtual ::IceInternal::Handle< ::IceDelegateD::Ice::Object> __createDelegateD();";
    H << eb << ';';
    C << sp;
    C << nl << "::IceInternal::Handle< ::IceDelegateM::Ice::Object>";
    C << nl << "IceProxy" << scoped << "::__createDelegateM()";
    C << sb;
    C << nl << "return ::IceInternal::Handle< ::IceDelegateM::Ice::Object>(new ::IceDelegateM" << scoped << ");";
    C << eb;
    C << sp;
    C << nl << "::IceInternal::Handle< ::IceDelegateD::Ice::Object>";
    C << nl << "IceProxy" << scoped << "::__createDelegateD()";
    C << sb;
    C << nl << "return ::IceInternal::Handle< ::IceDelegateD::Ice::Object>(new ::IceDelegateD" << scoped << ");";
    C << eb;
}

void
Slice::Gen::ProxyVisitor::visitOperation(const OperationPtr& p)
{
    string name = p->name();
    string scoped = p->scoped();
    string scope = p->scope();

    TypePtr ret = p->returnType();
    string retS = returnTypeToString(ret);

    TypeStringList inParams = p->inputParameters();
    TypeStringList outParams = p->outputParameters();
    TypeStringList::const_iterator q;

    string params = "(";
    string paramsDecl = "("; // With declarators
    string args = "(";

    for (q = inParams.begin(); q != inParams.end(); ++q)
    {
	if (q != inParams.begin())
	{
	    params += ", ";
	    paramsDecl += ", ";
	    args += ", ";
	}

	string typeString = inputTypeToString(q->first);
	params += typeString;
	paramsDecl += typeString;
	paramsDecl += ' ';
	paramsDecl += q->second;
	args += q->second;
    }

    for (q = outParams.begin(); q != outParams.end(); ++q)
    {
	if (q != outParams.begin() || !inParams.empty())
	{
	    params += ", ";
	    paramsDecl += ", ";
	    args += ", ";
	}

	string typeString = outputTypeToString(q->first);
	params += typeString;
	paramsDecl += typeString;
	paramsDecl += ' ';
	paramsDecl += q->second;
	args += q->second;
    }

    params += ')';
    paramsDecl += ')';
    args += ')';
    
    TypeList throws = p->throws();

    H << sp;
    H << nl << retS << ' ' << name << params << ';';
    C << sp;
    C << nl << retS << nl << "IceProxy" << scoped << paramsDecl;
    C << sb;
    C << nl << "int __cnt = 0;";
    C << nl << "while (true)";
    C << sb;
    C << nl << "::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase = __getDelegate();";
    C << nl << "::IceDelegate" << scope.substr(0, scope.size() - 2) << "* __del = dynamic_cast< ::IceDelegate"
      << scope.substr(0, scope.size() - 2) << "*>(__delBase.get());";
    C << nl << "try";
    C << sb;
    C << nl;
    if (ret)
    {
	C << "return ";
    }
    C << "__del->" << name << args << ";";
    if (!ret)
    {
	C << nl << "return;";
    }
    C << eb;
    C << nl << "catch (const ::Ice::LocationForward& __ex)";
    C << sb;
    C << nl << "__locationForward(__ex);";
    C << eb;
    C << nl << "catch (const ::IceInternal::NonRepeatable& __ex)";
    C << sb;
    if (p->nonmutating())
    {
	C << nl << "__handleException(*__ex.get(), __cnt);";
    }
    else
    {
	C << nl << "__rethrowException(*__ex.get());";
    }
    C << eb;
    C << nl << "catch (const ::Ice::LocalException& __ex)";
    C << sb;
    C << nl << "__handleException(__ex, __cnt);";
    C << eb;
    C << eb;
    C << eb;
}

Slice::Gen::DelegateVisitor::DelegateVisitor(Output& h, Output& c, const string& dllExport) :
    H(h), C(c), _dllExport(dllExport)
{
}

bool
Slice::Gen::DelegateVisitor::visitUnitStart(const UnitPtr& p)
{
    if (!p->hasProxies())
    {
	return false;
    }

    H << sp;
    H << nl << "namespace IceDelegate" << nl << '{';

    return true;
}

void
Slice::Gen::DelegateVisitor::visitUnitEnd(const UnitPtr& p)
{
    H << sp;
    H << nl << '}';
}
    
bool
Slice::Gen::DelegateVisitor::visitModuleStart(const ModulePtr& p)
{
    if (!p->hasProxies())
    {
	return false;
    }

    string name = p->name();
    
    H << sp;
    H << nl << "namespace " << name << nl << '{';

    return true;
}

void
Slice::Gen::DelegateVisitor::visitModuleEnd(const ModulePtr& p)
{
    H << sp;
    H << nl << '}';
}

bool
Slice::Gen::DelegateVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    if (p->isLocal())
    {
	return false;
    }

    string name = p->name();
    ClassList bases = p->bases();

    H << sp;
    H << nl << "class " << _dllExport << name << " : ";
    if (bases.empty())
    {
	H << "virtual public ::IceDelegate::Ice::Object";
    }
    else
    {
	H.useCurrentPosAsIndent();
	ClassList::const_iterator q = bases.begin();
	while (q != bases.end())
	{
	    H << "virtual public ::IceDelegate" << (*q)->scoped();
	    if (++q != bases.end())
	    {
		H << ',' << nl;
	    }
	}
	H.restoreIndent();
    }
    H << sb;
    H.dec();
    H << nl << "public: ";
    H.inc();

    return true;
}

void
Slice::Gen::DelegateVisitor::visitClassDefEnd(const ClassDefPtr& p)
{
    H << eb << ';';
}

void
Slice::Gen::DelegateVisitor::visitOperation(const OperationPtr& p)
{
    string name = p->name();

    TypePtr ret = p->returnType();
    string retS = returnTypeToString(ret);

    TypeStringList inParams = p->inputParameters();
    TypeStringList outParams = p->outputParameters();
    TypeStringList::const_iterator q;

    string params = "(";

    for (q = inParams.begin(); q != inParams.end(); ++q)
    {
	if (q != inParams.begin())
	{
	    params += ", ";
	}

	string typeString = inputTypeToString(q->first);
	params += typeString;
    }

    for (q = outParams.begin(); q != outParams.end(); ++q)
    {
	if (q != outParams.begin() || !inParams.empty())
	{
	    params += ", ";
	}

	string typeString = outputTypeToString(q->first);
	params += typeString;
    }

    params += ')';
    
    TypeList throws = p->throws();

    H << sp;
    H << nl << "virtual " << retS << ' ' << name << params << " = 0;";
}

Slice::Gen::DelegateMVisitor::DelegateMVisitor(Output& h, Output& c, const string& dllExport) :
    H(h), C(c), _dllExport(dllExport)
{
}

bool
Slice::Gen::DelegateMVisitor::visitUnitStart(const UnitPtr& p)
{
    if (!p->hasProxies())
    {
	return false;
    }

    H << sp;
    H << nl << "namespace IceDelegateM" << nl << '{';

    return true;
}

void
Slice::Gen::DelegateMVisitor::visitUnitEnd(const UnitPtr& p)
{
    H << sp;
    H << nl << '}';
}
    
bool
Slice::Gen::DelegateMVisitor::visitModuleStart(const ModulePtr& p)
{
    if (!p->hasProxies())
    {
	return false;
    }

    string name = p->name();
    
    H << sp;
    H << nl << "namespace " << name << nl << '{';

    return true;
}

void
Slice::Gen::DelegateMVisitor::visitModuleEnd(const ModulePtr& p)
{
    H << sp;
    H << nl << '}';
}

bool
Slice::Gen::DelegateMVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    if (p->isLocal())
    {
	return false;
    }

    string name = p->name();
    string scoped = p->scoped();
    ClassList bases = p->bases();

    H << sp;
    H << nl << "class " << _dllExport << name << " : ";
    H.useCurrentPosAsIndent();
    H << "virtual public ::IceDelegate" << scoped << ',';
    if (bases.empty())
    {
	H << nl << "virtual public ::IceDelegateM::Ice::Object";
    }
    else
    {
	ClassList::const_iterator q = bases.begin();
	while (q != bases.end())
	{
	    H << nl << "virtual public ::IceDelegateM" << (*q)->scoped();
	    if (++q != bases.end())
	    {
		H << ',';
	    }
	}
    }
    H.restoreIndent();
    H << sb;
    H.dec();
    H << nl << "public: ";
    H.inc();

    return true;
}

void
Slice::Gen::DelegateMVisitor::visitClassDefEnd(const ClassDefPtr& p)
{
    H << eb << ';';
}

void
Slice::Gen::DelegateMVisitor::visitOperation(const OperationPtr& p)
{
    string name = p->name();
    string scoped = p->scoped();

    TypePtr ret = p->returnType();
    string retS = returnTypeToString(ret);

    TypeStringList inParams = p->inputParameters();
    TypeStringList outParams = p->outputParameters();
    TypeStringList::const_iterator q;

    string params = "(";
    string paramsDecl = "("; // With declarators

    for (q = inParams.begin(); q != inParams.end(); ++q)
    {
	if (q != inParams.begin())
	{
	    params += ", ";
	    paramsDecl += ", ";
	}

	string typeString = inputTypeToString(q->first);
	params += typeString;
	paramsDecl += typeString;
	paramsDecl += ' ';
	paramsDecl += q->second;
    }

    for (q = outParams.begin(); q != outParams.end(); ++q)
    {
	if (q != outParams.begin() || !inParams.empty())
	{
	    params += ", ";
	    paramsDecl += ", ";
	}

	string typeString = outputTypeToString(q->first);
	params += typeString;
	paramsDecl += typeString;
	paramsDecl += ' ';
	paramsDecl += q->second;
    }

    params += ')';
    paramsDecl += ')';
    
    TypeList throws = p->throws();

    H << sp;
    H << nl << "virtual " << retS << ' ' << name << params << ';';
    C << sp;
    C << nl << retS << nl << "IceDelegateM" << scoped << paramsDecl;
    C << sb;
    C << nl << "::IceInternal::Outgoing __out(__emitter, __reference);";
    if (ret || !outParams.empty() || !throws.empty())
    {
	C << nl << "::IceInternal::IntStream* __is = __out.is();";
    }
    C << nl << "::IceInternal::IntStream* __os = __out.os();";
    C << nl << "__os->write(\"" << name << "\");";
    writeMarshalCode(C, inParams, 0);
    C << nl << "if (!__out.invoke())";
    C << sb;
    if (!throws.empty())
    {
	C << nl << "::Ice::Int __exnum;";
	C << nl << "__is->read(__exnum);";
	C << nl << "switch (__exnum)";
	C << sb;
	TypeList::const_iterator r;
	int cnt = 0;
	for (r = throws.begin(); r != throws.end(); ++r)
	{
	    C << nl << "case " << cnt++ << ':';
	    C << sb;
	    TypeStringList li;
	    li.push_back(make_pair(*r, string("__ex")));
	    writeAllocateCode(C, li, 0);
	    writeUnmarshalCode(C, li, 0);
	    if (ClassDeclPtr::dynamicCast(*r) || ProxyPtr::dynamicCast(*r))
	    {
		C << nl << "__ex->_throw();";
	    }
	    else
	    {
		C << nl << "throw __ex;";
	    }
	    C << eb;
	}
	C << eb;
	C << nl << "throw ::Ice::UnknownUserException(__FILE__, __LINE__);";
    }
    else
    {
	C << nl << "throw ::Ice::UnknownUserException(__FILE__, __LINE__);";
    }
    C << eb;
    writeAllocateCode(C, TypeStringList(), ret);
    writeUnmarshalCode(C, outParams, ret);
    if (ret)
    {
	C << nl << "return __ret;";
    }
    C << eb;
}

Slice::Gen::DelegateDVisitor::DelegateDVisitor(Output& h, Output& c, const string& dllExport) :
    H(h), C(c), _dllExport(dllExport)
{
}

bool
Slice::Gen::DelegateDVisitor::visitUnitStart(const UnitPtr& p)
{
    if (!p->hasProxies())
    {
	return false;
    }

    H << sp;
    H << nl << "namespace IceDelegateD" << nl << '{';

    return true;
}

void
Slice::Gen::DelegateDVisitor::visitUnitEnd(const UnitPtr& p)
{
    H << sp;
    H << nl << '}';
}
    
bool
Slice::Gen::DelegateDVisitor::visitModuleStart(const ModulePtr& p)
{
    if (!p->hasProxies())
    {
	return false;
    }

    string name = p->name();
    
    H << sp;
    H << nl << "namespace " << name << nl << '{';

    return true;
}

void
Slice::Gen::DelegateDVisitor::visitModuleEnd(const ModulePtr& p)
{
    H << sp;
    H << nl << '}';
}

bool
Slice::Gen::DelegateDVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    if (p->isLocal())
    {
	return false;
    }

    string name = p->name();
    string scoped = p->scoped();
    ClassList bases = p->bases();

    H << sp;
    H << nl << "class " << _dllExport << name << " : ";
    H.useCurrentPosAsIndent();
    H << "virtual public ::IceDelegate" << scoped << ',';
    if (bases.empty())
    {
	H << nl << "virtual public ::IceDelegateD::Ice::Object";
    }
    else
    {
	ClassList::const_iterator q = bases.begin();
	while (q != bases.end())
	{
	    H << nl << "virtual public ::IceDelegateD" << (*q)->scoped();
	    if (++q != bases.end())
	    {
		H << ',';
	    }
	}
    }
    H.restoreIndent();
    H << sb;
    H.dec();
    H << nl << "public: ";
    H.inc();

    return true;
}

void
Slice::Gen::DelegateDVisitor::visitClassDefEnd(const ClassDefPtr& p)
{
    H << eb << ';';
}

void
Slice::Gen::DelegateDVisitor::visitOperation(const OperationPtr& p)
{
    ContainerPtr container = p->container();
    ClassDefPtr cl = ClassDefPtr::dynamicCast(container);
    
    string name = p->name();
    string scoped = p->scoped();

    TypePtr ret = p->returnType();
    string retS = returnTypeToString(ret);

    TypeStringList inParams = p->inputParameters();
    TypeStringList outParams = p->outputParameters();
    TypeStringList::const_iterator q;

    string params = "(";
    string paramsDecl = "("; // With declarators
    string args = "(";

    for (q = inParams.begin(); q != inParams.end(); ++q)
    {
	if (q != inParams.begin())
	{
	    params += ", ";
	    paramsDecl += ", ";
	    args += ", ";
	}

	string typeString = inputTypeToString(q->first);
	params += typeString;
	paramsDecl += typeString;
	paramsDecl += ' ';
	paramsDecl += q->second;
	args += q->second;
    }

    for (q = outParams.begin(); q != outParams.end(); ++q)
    {
	if (q != outParams.begin() || !inParams.empty())
	{
	    params += ", ";
	    paramsDecl += ", ";
	    args += ", ";
	}

	string typeString = outputTypeToString(q->first);
	params += typeString;
	paramsDecl += typeString;
	paramsDecl += ' ';
	paramsDecl += q->second;
	args += q->second;
    }

    params += ')';
    paramsDecl += ')';
    args += ')';
    
    TypeList throws = p->throws();

    H << sp;
    H << nl << "virtual " << retS << ' ' << name << params << ';';
    C << sp;
    C << nl << retS << nl << "IceDelegateD" << scoped << paramsDecl;
    C << sb;
    C << nl << "::IceInternal::Direct __direct(__adapter, __reference, \"" << name << "\");";
    C << nl << cl->scoped() << "* __servant = dynamic_cast< " << cl->scoped() << "*>(__direct.servant().get());";
    C << nl << "if (!__servant)";
    C << sb;
    C << nl << "throw ::Ice::OperationNotExistException(__FILE__, __LINE__);";
    C << eb;
    C << nl;
    if (ret)
    {
	C << "return ";
    }
    C << "__servant->" << name << args << ';';
    C << eb;
}

Slice::Gen::ObjectDeclVisitor::ObjectDeclVisitor(Output& h, Output& c, const string& dllExport) :
    H(h), C(c), _dllExport(dllExport)
{
}

bool
Slice::Gen::ObjectDeclVisitor::visitModuleStart(const ModulePtr& p)
{
    if (!p->hasClassDecls())
    {
	return false;
    }

    string name = p->name();
    
    H << sp;
    H << nl << "namespace " << name << nl << '{';

    return true;
}

void
Slice::Gen::ObjectDeclVisitor::visitModuleEnd(const ModulePtr& p)
{
    H << sp;
    H << nl << '}';
}

void
Slice::Gen::ObjectDeclVisitor::visitClassDecl(const ClassDeclPtr& p)
{
    string name = p->name();
    
    H << sp;
    H << nl << "class " << name << ';';
}

Slice::Gen::ObjectVisitor::ObjectVisitor(Output& h, Output& c, const string& dllExport) :
    H(h), C(c), _dllExport(dllExport)
{
}

bool
Slice::Gen::ObjectVisitor::visitModuleStart(const ModulePtr& p)
{
    if (!p->hasClassDefs())
    {
	return false;
    }

    string name = p->name();
    
    H << sp;
    H << nl << "namespace " << name << nl << '{';

    return true;
}

void
Slice::Gen::ObjectVisitor::visitModuleEnd(const ModulePtr& p)
{
    H << sp;
    H << nl << '}';
}

bool
Slice::Gen::ObjectVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    string name = p->name();
    string scoped = p->scoped();
    ClassList bases = p->bases();
    
    string exp1;
    string exp2;
    if (_dllExport.size())
    {
	if (p->hasDataMembers())
	{
	    exp2 = _dllExport;
	}
	else
	{
	    exp1 = _dllExport;
	}
    }
    
    H << sp;
    H << nl << "class " << _dllExport << name << "PtrE : ";
    H.useCurrentPosAsIndent();
    if (bases.empty())
    {
	if (p->isLocal())
	{
	    H << "virtual public ::Ice::LocalObjectPtrE";
	}
	else
	{
	    H << "virtual public ::Ice::ObjectPtrE";
	}
    }
    else
    {
	ClassList::const_iterator q = bases.begin();
	while (q != bases.end())
	{
	    H << "virtual public " << (*q)->scoped() << "PtrE";
	    if (++q != bases.end())
	    {
		H << ',' << nl;
	    }
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

    if (!p->isLocal())
    {
	H << sp;
	H << nl << "class " << exp1 << name << "PrxE : ";
	H.useCurrentPosAsIndent();
	if (bases.empty())
	{
	    H << "virtual public ::Ice::ObjectPrxE";
	}
	else
	{
	    ClassList::const_iterator q = bases.begin();
	    while (q != bases.end())
	    {
		H << "virtual public " << (*q)->scoped() << "PrxE";
		if (++q != bases.end())
		{
		    H << ',' << nl;
		}
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
    H << nl << "class " << exp1 << name << " : ";
    H.useCurrentPosAsIndent();
    if (bases.empty())
    {
	if (p->isLocal())
	{
	    H << "virtual public ::Ice::LocalObject";
	}
	else
	{
	    H << "virtual public ::Ice::Object";
	}
    }
    else
    {
	ClassList::const_iterator q = bases.begin();
	while (q != bases.end())
	{
	    H << "virtual public " << (*q)->scoped();
	    if (++q != bases.end())
	    {
		H << ',' << nl;
	    }
	}
    }
    H.restoreIndent();
    H << sb;
    H.dec();
    H << nl << "public: ";
    H.inc();

    H << sp;
    H << nl << exp2 << "virtual void _throw();";
    C << sp << nl << "void" << nl << scoped.substr(2) << "::_throw()";
    C << sb;
    C << nl << "throw " << scoped << "PtrE(this);";
    C << eb;

    if (!p->isLocal())
    {
	ClassList allBases = p->allBases();
	StringList ids;
	transform(allBases.begin(), allBases.end(), back_inserter(ids), ::IceUtil::memFun(&ClassDef::scoped));
	StringList other;
	other.push_back(scoped);
	other.push_back("::Ice::Object");
	other.sort();
	ids.merge(other);
	ids.unique();
	
	ClassList allBaseClasses;
	ClassDefPtr cl;
	if (!bases.empty())
	{
	    cl = bases.front();
	}
	else
	{
	    cl = 0;
	}
	while (cl && !cl->isInterface())
	{
	    allBaseClasses.push_back(cl);
	    ClassList baseBases = cl->bases();
	    if (!baseBases.empty())
	    {
		cl = baseBases.front();
	    }
	    else
	    {
		cl = 0;
	    }
	}
	StringList classIds;
	transform(allBaseClasses.begin(), allBaseClasses.end(), back_inserter(classIds),
		  ::IceUtil::memFun(&ClassDef::scoped));
	if (!p->isInterface())
	{
	    classIds.push_front(scoped);
	}
	classIds.push_back("::Ice::Object");

	StringList::const_iterator q;

	H << sp;
	H << nl << exp2 << "static ::std::string __ids[" << ids.size() << "];";
	H << sp;
	H << nl << exp2 << "static ::std::string __classIds[" << classIds.size() << "];";
	H << sp;
	H << nl << exp2 << "virtual bool _isA(const ::std::string&);";
	H << sp;
	H << nl << exp2 << "virtual const ::std::string* _classIds();";
	C << sp;
	C << nl << "::std::string " << scoped.substr(2) << "::__ids[" << ids.size() << "] =";
	C << sb;
	q = ids.begin();
	while (q != ids.end())
	{
	    C << nl << '"' << *q << '"';
	    if (++q != ids.end())
	    {
		C << ',';
	    }
	}
	C << eb << ';';
	C << sp;
	C << nl << "::std::string " << scoped.substr(2) << "::__classIds[" << classIds.size() << "] =";
	C << sb;
	q = classIds.begin();
	while (q != classIds.end())
	{
	    C << nl << '"' << *q << '"';
	    if (++q != classIds.end())
	    {
		C << ',';
	    }
	}
	C << eb << ';';
	C << sp;
	C << nl << "bool" << nl << scoped.substr(2) << "::_isA(const ::std::string& s)";
	C << sb;
	C << nl << "::std::string* b = __ids;";
	C << nl << "::std::string* e = __ids + " << ids.size() << ';';
	C << nl << "::std::pair< ::std::string*, ::std::string*> r = ::std::equal_range(b, e, s);";
	C << nl << "return r.first != r.second;";
	C << eb;
	C << sp;
	C << nl << "const ::std::string*" << nl << scoped.substr(2) << "::_classIds()";
	C << sb;
	C << nl << "return __classIds;";
	C << eb;
    }

    return true;
}
    
void
Slice::Gen::ObjectVisitor::visitClassDefEnd(const ClassDefPtr& p)
{
    string name = p->name();
    string scoped = p->scoped();

    ClassList bases = p->bases();
    ClassDefPtr base;
    if (!bases.empty() && !bases.front()->isInterface())
    {
	base = bases.front();
    }
    
    if (!p->isLocal())
    {
	string exp2;
	if (_dllExport.size())
	{
	    if (p->hasDataMembers())
	    {
		exp2 = _dllExport;
	    }
	}

	OperationList allOps = p->allOperations();
	if (!allOps.empty())
	{
	    StringList allOpNames;
	    transform(allOps.begin(), allOps.end(), back_inserter(allOpNames), ::IceUtil::memFun(&Operation::name));
	    allOpNames.push_back("_isA");
	    allOpNames.push_back("_ping");
	    allOpNames.sort();
	    allOpNames.unique();

	    OperationList allMutatingOps;
	    remove_copy_if(allOps.begin(), allOps.end(), back_inserter(allMutatingOps), 
			   ::IceUtil::memFun(&Operation::nonmutating));
	    StringList allMutatingOpNames;
	    transform(allMutatingOps.begin(), allMutatingOps.end(), back_inserter(allMutatingOpNames),
		      ::IceUtil::memFun(&Operation::name));
	    // Don't add _isA and _ping. These operations are non-mutating.
	    allMutatingOpNames.sort();
	    allMutatingOpNames.unique();
	    
	    StringList::const_iterator q;
	    
	    H << sp;
	    H << nl << exp2 << "static ::std::string __all[" << allOpNames.size() << "];";
	    if (!allMutatingOpNames.empty())
	    {
		H << nl << exp2 << "static ::std::string __mutating[" << allMutatingOpNames.size() << "];";
	    }
	    H << nl << exp2 << "virtual ::IceInternal::DispatchStatus "
	      << "__dispatch(::IceInternal::Incoming&, const ::std::string&);";
	    H << nl << exp2 << "virtual bool __isMutating(const ::std::string&);";
	    C << sp;
	    C << nl << "::std::string " << scoped.substr(2) << "::__all[] =";
	    C << sb;
	    q = allOpNames.begin();
	    while (q != allOpNames.end())
	    {
		C << nl << '"' << *q << '"';
		if (++q != allOpNames.end())
		{
		    C << ',';
		}
	    }
	    C << eb << ';';
	    if (!allMutatingOpNames.empty())
	    {
		C << sp;
		C << nl << "::std::string " << scoped.substr(2) << "::__mutating[] =";
		C << sb;
		q = allMutatingOpNames.begin();
		while (q != allMutatingOpNames.end())
		{
		    C << nl << '"' << *q << '"';
		    if (++q != allMutatingOpNames.end())
		    {
			C << ',';
		    }
		}
		C << eb << ';';
	    }
	    C << sp;
	    C << nl << "::IceInternal::DispatchStatus" << nl << scoped.substr(2)
	      << "::__dispatch(::IceInternal::Incoming& in, const ::std::string& s)";
	    C << sb;
	    C << nl << "::std::string* b = __all;";
	    C << nl << "::std::string* e = __all + " << allOpNames.size() << ';';
	    C << nl << "::std::pair< ::std::string*, ::std::string*> r = ::std::equal_range(b, e, s);";
	    C << nl << "if (r.first == r.second)";
	    C << sb;
	    C << nl << "return ::IceInternal::DispatchOperationNotExist;";
	    C << eb;
	    C << sp;
	    C << nl << "switch (r.first - __all)";
	    C << sb;
	    int i = 0;
	    for (q = allOpNames.begin(); q != allOpNames.end(); ++q)
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
	    C << sp;
	    C << nl << "bool" << nl << scoped.substr(2)
	      << "::__isMutating(const ::std::string& s)";
	    C << sb;
	    if (!allMutatingOpNames.empty())
	    {
		C << nl << "::std::string* b = __mutating;";
		C << nl << "::std::string* e = __mutating + " << allMutatingOpNames.size() << ';';
		C << nl << "return ::std::binary_search(b, e, s);";
	    }
	    else
	    {
		C << nl << "return false;";
	    }
	    C << eb;
	}
	H << sp;
	H << nl << exp2 << "virtual void __write(::IceInternal::IntStream*);";
	H << nl << exp2 << "virtual void __read(::IceInternal::IntStream*);";
	TypeStringList memberList;
	DataMemberList dataMembers = p->dataMembers();
	for (DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
	{
	    memberList.push_back(make_pair((*q)->type(), (*q)->name()));
	}
	C << sp;
	C << nl << "void" << nl << scoped.substr(2) << "::__write(::IceInternal::IntStream* __os)";
	C << sb;
	C << nl << "__os->startWriteEncaps();";
	writeMarshalCode(C, memberList, 0);
	C << nl << "__os->endWriteEncaps();";
	if (base)
	{
	    C.zeroIndent();
	    C << nl << "#ifdef WIN32"; // COMPILERBUG
	    C.restoreIndent();
	    C << nl << base->name() << "::__write(__os);";
	    C.zeroIndent();
	    C << nl << "#else";
	    C.restoreIndent();
	    C << nl << base->scoped() << "::__write(__os);";
	    C.zeroIndent();
	    C << nl << "#endif";
	    C.restoreIndent();
	}
	C << eb;
	C << sp;
	C << nl << "void" << nl << scoped.substr(2) << "::__read(::IceInternal::IntStream* __is)";
	C << sb;
	C << nl << "__is->startReadEncaps();";
	writeUnmarshalCode(C, memberList, 0);
	C << nl << "__is->endReadEncaps();";
	if (base)
	{
	    C.zeroIndent();
	    C << nl << "#ifdef WIN32"; // COMPILERBUG
	    C.restoreIndent();
	    C << nl << base->name() << "::__read(__is);";
	    C.zeroIndent();
	    C << nl << "#else";
	    C.restoreIndent();
	    C << nl << base->scoped() << "::__read(__is);";
	    C.zeroIndent();
	    C << nl << "#endif";
	    C.restoreIndent();
	}
	C << eb;
    }
    H << eb << ';';
}

bool
Slice::Gen::ObjectVisitor::visitStructStart(const StructPtr&)
{
    return false;
}

void
Slice::Gen::ObjectVisitor::visitOperation(const OperationPtr& p)
{
    ContainerPtr container = p->container();
    ClassDefPtr cl = ClassDefPtr::dynamicCast(container);
    string name = p->name();
    string scoped = p->scoped();
    string scope = p->scope();

    TypePtr ret = p->returnType();
    string retS = returnTypeToString(ret);

    TypeStringList inParams = p->inputParameters();
    TypeStringList outParams = p->outputParameters();
    TypeStringList::const_iterator q;

    string params = "(";
    string paramsDecl = "("; // With declarators
    string args = "(";

    for (q = inParams.begin(); q != inParams.end(); ++q)
    {
	if (q != inParams.begin())
	{
	    params += ", ";
	    paramsDecl += ", ";
	    args += ", ";
	}

	string typeString = inputTypeToString(q->first);
	params += typeString;
	paramsDecl += typeString;
	paramsDecl += ' ';
	paramsDecl += q->second;
	args += q->second;
    }

    for (q = outParams.begin(); q != outParams.end(); ++q)
    {
	if (q != outParams.begin() || !inParams.empty())
	{
	    params += ", ";
	    paramsDecl += ", ";
	    args += ", ";
	}

	string typeString = outputTypeToString(q->first);
	params += typeString;
	paramsDecl += typeString;
	paramsDecl += ' ';
	paramsDecl += q->second;
	args += q->second;
    }

    params += ')';
    paramsDecl += ')';
    args += ')';
    
    TypeList throws = p->throws();

    string exp2;
    if (_dllExport.size())
    {
	if (cl->hasDataMembers())
	{
	    exp2 = _dllExport;
	}
    }

    H << sp;
    H << nl << exp2 << "virtual " << retS << ' ' << name << params << " = 0;";

    if (!cl->isLocal())
    {
	H << nl << exp2 << "::IceInternal::DispatchStatus ___" << name << "(::IceInternal::Incoming&);";
	C << sp;
	C << nl << "::IceInternal::DispatchStatus" << nl << scope.substr(2) << "___" << name
	  << "(::IceInternal::Incoming& __in)";
	C << sb;
	if (!inParams.empty())
	{
	    C << nl << "::IceInternal::IntStream* __is = __in.is();";
	}
	if (ret || !outParams.empty() || !throws.empty())
	{
	    C << nl << "::IceInternal::IntStream* __os = __in.os();";
	}
	writeAllocateCode(C, inParams, 0);
	writeUnmarshalCode(C, inParams, 0);
	writeAllocateCode(C, outParams, 0);
	if (!throws.empty())
	{
	    C << nl << "try";
	    C << sb;
	}
	C << nl;
	if (ret)
	{
	    C << retS << " __ret = ";
	}
	C << name << args << ';';
	writeMarshalCode(C, outParams, ret);
	C << nl << "return ::IceInternal::DispatchOK;";
	if (!throws.empty())
	{
	    C << eb;
	    TypeList::const_iterator r;
	    int cnt = 0;
	    for (r = throws.begin(); r != throws.end(); ++r)
	    {
		C << nl << "catch(" << exceptionTypeToString(*r) << " __ex)";
		C << sb;
		C << nl << "__os->write(" << cnt++ << ");";
		if (ClassDeclPtr::dynamicCast(*r) || ProxyPtr::dynamicCast(*r))
		{
		    string s = "static_cast< ";
		    s += inputTypeToString(*r);
		    s += ">(__ex)";
		    writeMarshalUnmarshalCode(C, *r, s, true);
		}
		else
		{
		    writeMarshalUnmarshalCode(C, *r, "__ex", true);
		}
		C << nl << "return ::IceInternal::DispatchUserException;";
		C << eb;
	    }
	}
	C << eb;
    }	
}

void
Slice::Gen::ObjectVisitor::visitDataMember(const DataMemberPtr& p)
{
    string name = p->name();
    string s = typeToString(p->type());
    H << sp;
    H << nl << s << ' ' << name << ';';
}

Slice::Gen::IceVisitor::IceVisitor(Output& h, Output& c, const string& dllExport) :
    H(h), C(c), _dllExport(dllExport)
{
}

bool
Slice::Gen::IceVisitor::visitUnitStart(const UnitPtr& p)
{
    if (!p->hasClassDecls())
    {
	return false;
    }

    H << sp;
    H << nl << "namespace IceInternal" << nl << '{';

    return true;
}

void
Slice::Gen::IceVisitor::visitUnitEnd(const UnitPtr& p)
{
    H << sp;
    H << nl << '}';
}
    
void
Slice::Gen::IceVisitor::visitClassDecl(const ClassDeclPtr& p)
{
    string scoped = p->scoped();
    
    H << sp;
    H << nl << _dllExport << "void incRef(" << scoped << "*);";
    H << nl << _dllExport << "void decRef(" << scoped << "*);";
    if (!p->isLocal())
    {
	H << sp;
	H << nl << _dllExport << "void incRef(::IceProxy" << scoped << "*);";
	H << nl << _dllExport << "void decRef(::IceProxy" << scoped << "*);";
	H << sp;
	H << nl << _dllExport << "void checkedCast(::IceProxy::Ice::Object*, ::IceProxy" << scoped << "*&);";
	H << nl << _dllExport << "void uncheckedCast(::IceProxy::Ice::Object*, ::IceProxy" << scoped << "*&);";
    }
}

bool
Slice::Gen::IceVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    string scoped = p->scoped();
    
    C << sp;
    C << nl << "void" << nl << "IceInternal::incRef(" << scoped << "* p)";
    C << sb;
    C << nl << "p->__incRef();";
    C << eb;
    C << nl << "void" << nl << "IceInternal::decRef(" << scoped << "* p)";
    C << sb;
    C << nl << "p->__decRef();";
    C << eb;

    if (!p->isLocal())
    {
	C << sp;
	C << nl << "void" << nl << "IceInternal::incRef(::IceProxy" << scoped << "* p)";
	C << sb;
	C << nl << "p->__incRef();";
	C << eb;
	C << nl << "void" << nl << "IceInternal::decRef(::IceProxy" << scoped << "* p)";
	C << sb;
	C << nl << "p->__decRef();";
	C << eb;
	C << sp;
	C << nl << "void" << nl << "IceInternal::checkedCast(::IceProxy::Ice::Object* b, ::IceProxy" << scoped
	  << "*& d)";
	C << sb;
	C << nl << "d = dynamic_cast< ::IceProxy" << scoped << "*>(b);";
	C << nl << "if (!d && b->_isA(\"" << scoped << "\"))";
	C << sb;
	C << nl << "d = new ::IceProxy" << scoped << ';';
	C << nl << "b->__copyTo(d);";
	C << eb;
	C << eb;
	C << sp;
	C << nl << "void" << nl << "IceInternal::uncheckedCast(::IceProxy::Ice::Object* b, ::IceProxy" << scoped
	  << "*& d)";
	C << sb;
	C << nl << "d = dynamic_cast< ::IceProxy" << scoped << "*>(b);";
	C << nl << "if (!d)";
	C << sb;
	C << nl << "d = new ::IceProxy" << scoped << ';';
	C << nl << "b->__copyTo(d);";
	C << eb;
	C << eb;
    }

    return true;
}

Slice::Gen::HandleVisitor::HandleVisitor(Output& h, Output& c, const string& dllExport) :
    H(h), C(c), _dllExport(dllExport)
{
}

bool
Slice::Gen::HandleVisitor::visitModuleStart(const ModulePtr& p)
{
    if (!p->hasClassDecls())
    {
	return false;
    }

    string name = p->name();
    
    H << sp;
    H << nl << "namespace " << name << nl << '{';

    return true;
}

void
Slice::Gen::HandleVisitor::visitModuleEnd(const ModulePtr& p)
{
    H << sp;
    H << nl << '}';
}

void
Slice::Gen::HandleVisitor::visitClassDecl(const ClassDeclPtr& p)
{
    string name = p->name();
    string scoped = p->scoped();
    
    H << sp;
    H << nl << "typedef ::IceInternal::Handle< " << scoped << "> " << name << "Ptr;";
    if (!p->isLocal())
    {
	H << nl << "typedef ::IceInternal::ProxyHandle< ::IceProxy" << scoped << "> " << name << "Prx;";
	H << sp;
	H << nl << _dllExport << "void __write(::IceInternal::IntStream*, const " << name << "Prx&);";
	H << nl << _dllExport << "void __read(::IceInternal::IntStream*, " << name << "Prx&);";
    }
}

bool
Slice::Gen::HandleVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    if (p->isLocal())
    {
	return false;
    }

    string scoped = p->scoped();
    string scope = p->scope();

    C << sp;
    C << nl << "void" << nl << scope.substr(2) << "__write(::IceInternal::IntStream* __os, const " << scoped << "Prx& v)";
    C << sb;
    C << nl << "__os->write(::Ice::ObjectPrx(v));";
    C << eb;
    C << sp;
    C << nl << "void" << nl << scope.substr(2) << "__read(::IceInternal::IntStream* __is, " << scoped << "Prx& v)";
    C << sb;
    C << nl << "::Ice::ObjectPrx proxy;";
    C << nl << "__is->read(proxy);";
    C << nl << "if (!proxy)";
    C << sb;
    C << nl << "v = 0;";
    C << eb;
    C << nl << "else";
    C << sb;
    C << nl << "v = new ::IceProxy" << scoped << ';';
    C << nl << "proxy->__copyTo(v.get());";
    C << eb;
    C << eb;

    return true;
}
