// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <IceUtil/Functional.h>
#include <Slice/CPlusPlusUtil.h>
#include <Gen.h>
#include <limits>
#include <iterator>

#include <sys/stat.h>

using namespace std;
using namespace Slice;
using namespace IceUtil;

Slice::Gen::Gen(const string& name, const string& base,	const string& include, const vector<string>& includePaths,
		const string& dllExport, const string& dir, bool imp) :
    _base(base),
    _include(include),
    _includePaths(includePaths),
    _dllExport(dllExport),
    _impl(imp)
{
    for(vector<string>::iterator p = _includePaths.begin(); p != _includePaths.end(); ++p)
    {
	if(p->length() && (*p)[p->length() - 1] != '/')
	{
	    *p += '/';
	}
    }

    string::size_type pos = _base.rfind('/');
    if(pos != string::npos)
    {
	_base.erase(0, pos + 1);
    }

    if(_impl)
    {
        string fileImplH = _base + "I.h";
        string fileImplC = _base + "I.cpp";
        if(!dir.empty())
        {
            fileImplH = dir + '/' + fileImplH;
            fileImplC = dir + '/' + fileImplC;
        }

        struct stat st;
        if(stat(fileImplH.c_str(), &st) == 0)
        {
            cerr << name << ": `" << fileImplH
                 << "' already exists - will not overwrite" << endl;
            return;
        }
        if(stat(fileImplC.c_str(), &st) == 0)
        {
            cerr << name << ": `" << fileImplC
                 << "' already exists - will not overwrite" << endl;
            return;
        }

        implH.open(fileImplH.c_str());
        if(!implH)
        {
            cerr << name << ": can't open `" << fileImplH << "' for writing: "
                 << strerror(errno) << endl;
            return;
        }
        
        implC.open(fileImplC.c_str());
        if(!implC)
        {
            cerr << name << ": can't open `" << fileImplC << "' for writing: "
                 << strerror(errno) << endl;
            return;
        }

        string s = fileImplH;
        if(_include.size())
        {
            s = _include + '/' + s;
        }
        transform(s.begin(), s.end(), s.begin(), ToIfdef());
        implH << "#ifndef __" << s << "__";
        implH << "\n#define __" << s << "__";
        implH << '\n';
    }

    string fileH = _base + ".h";
    string fileC = _base + ".cpp";
    if(!dir.empty())
    {
	fileH = dir + '/' + fileH;
	fileC = dir + '/' + fileC;
    }

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
    H << "\n// Generated from file `" << changeInclude(_base, _includePaths) << ".ice'\n";
    C << "\n// Generated from file `" << changeInclude(_base, _includePaths) << ".ice'\n";

    string s = fileH;
    if(_include.size())
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

    if(_impl)
    {
        implH << "\n\n#endif\n";
        implC << '\n';
    }
}

bool
Slice::Gen::operator!() const
{
    if(!H || !C)
    {
        return true;
    }
    if(_impl && (!implH || !implC))
    {
        return true;
    }
    return false;
}

void
Slice::Gen::generate(const UnitPtr& unit)
{
    C << "\n#include <";
    if(_include.size())
    {
	C << _include << '/';
    }
    C << _base << ".h>";

    H << "\n#include <Ice/LocalObjectF.h>";
    H << "\n#include <Ice/ProxyF.h>";
    H << "\n#include <Ice/ObjectF.h>";

    if(unit->usesProxies())
    {
	H << "\n#include <Ice/Exception.h>";
	H << "\n#include <Ice/LocalObject.h>";
	H << "\n#include <Ice/Proxy.h>";
	H << "\n#include <Ice/Object.h>";
	H << "\n#include <Ice/Outgoing.h>";
	H << "\n#include <Ice/Incoming.h>";
	H << "\n#include <Ice/Direct.h>";

	C << "\n#include <Ice/LocalException.h>";
	C << "\n#include <Ice/ObjectFactory.h>";
	C << "\n#include <Ice/BasicStream.h>";
	C << "\n#include <Ice/Stream.h>";
    }
    else if(unit->usesNonLocals())
    {
	H << "\n#include <Ice/Exception.h>";
	H << "\n#include <Ice/LocalObject.h>";

	C << "\n#include <Ice/BasicStream.h>";
	C << "\n#include <Ice/Stream.h>";
    }
    else
    {
	H << "\n#include <Ice/Exception.h>";
	H << "\n#include <Ice/LocalObject.h>";
    }

    if(unit->usesConsts())
    {
	H << "\n#include <Ice/Const.h>";
    }

    StringList includes = unit->includeFiles();
    for(StringList::const_iterator q = includes.begin(); q != includes.end(); ++q)
    {
	H << "\n#include <" << changeInclude(*q, _includePaths) << ".h>";
    }

    printVersionCheck(H);
    printVersionCheck(C);

    printDllExportStuff(H, _dllExport);
    if(_dllExport.size())
    {
	_dllExport += " ";
    }

    ProxyDeclVisitor proxyDeclVisitor(H, C, _dllExport);
    unit->visit(&proxyDeclVisitor);

    ObjectDeclVisitor objectDeclVisitor(H, C, _dllExport);
    unit->visit(&objectDeclVisitor);

    IceInternalVisitor iceInternalVisitor(H, C, _dllExport);
    unit->visit(&iceInternalVisitor);

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

    if(_impl)
    {
        implH << "\n#include <";
        if(_include.size())
        {
            implH << _include << '/';
        }
        implH << _base << ".h>";

        implC << "#include <";
        if(_include.size())
        {
            implC << _include << '/';
        }
        implC << _base << "I.h>";

        ImplVisitor implVisitor(implH, implC, _dllExport);
        unit->visit(&implVisitor);
    }
}

Slice::Gen::TypesVisitor::TypesVisitor(Output& h, Output& c, const string& dllExport) :
    H(h), C(c), _dllExport(dllExport)
{
}

bool
Slice::Gen::TypesVisitor::visitModuleStart(const ModulePtr& p)
{
    if(!p->hasOtherConstructedOrExceptions())
    {
	return false;
    }

    string name = fixKwd(p->name());
    
    H << sp << nl << "namespace " << name << nl << '{';

    return true;
}

void
Slice::Gen::TypesVisitor::visitModuleEnd(const ModulePtr& p)
{
    H << sp << nl << '}';
}

bool
Slice::Gen::TypesVisitor::visitClassDefStart(const ClassDefPtr&)
{
    return false;
}

bool
Slice::Gen::TypesVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    string name = fixKwd(p->name());
    string scoped = fixKwd(p->scoped());
    ExceptionPtr base = p->base();

    H << sp << nl << "class " << name << " : ";
    H.useCurrentPosAsIndent();
    if(!base)
    {
	if(p->isLocal())
	{
	    H << "public ::Ice::LocalException";
	}
	else
	{
	    H << "public ::Ice::UserException";
	}
    }
    else
    {
	H << "public " << fixKwd(base->scoped());
    }
    H.restoreIndent();
    H << sb;

    H.dec();
    H << nl << "public:";
    H.inc();

    H << sp;
    if(p->isLocal())
    {
	H << nl << _dllExport << name << "(const char*, int);";
	C << sp << nl << scoped.substr(2) << "::" << name << "(const char* file, int line) : ";
	C.inc();
	if(!base)
	{
	    C << nl << "::Ice::LocalException(file, line)";
	}
	else
	{
	    C << nl << fixKwd(base->scoped()) << "(file, line)";
	}
	C.dec();
	C << sb;
	C << eb;
    }

    H << nl << _dllExport << "virtual ::std::string ice_name() const;";
    C << sp << nl << "::std::string" << nl << scoped.substr(2) << "::ice_name() const";
    C << sb;
    C << nl << "return \"" << p->scoped().substr(2) << "\";";
    C << eb;
    
    if(p->isLocal())
    {
	H << nl << _dllExport << "virtual void ice_print(::std::ostream&) const;";
    }

    H << nl << _dllExport << "virtual ::Ice::Exception* ice_clone() const;";
    C << sp << nl << "::Ice::Exception*" << nl << scoped.substr(2) << "::ice_clone() const";
    C << sb;
    C << nl << "return new " << name << "(*this);";
    C << eb;

    H << nl << _dllExport << "virtual void ice_throw() const;";
    C << sp << nl << "void" << nl << scoped.substr(2) << "::ice_throw() const";
    C << sb;
    C << nl << "throw *this;";
    C << eb;
    
    if(!p->isLocal())
    {
	ExceptionList allBases = p->allBases();
	StringList exceptionIds;
	transform(allBases.begin(), allBases.end(), back_inserter(exceptionIds),
		  ::IceUtil::constMemFun(&Exception::scoped));
	exceptionIds.push_front(p->scoped());
	exceptionIds.push_back("::Ice::UserException");
	
	StringList::const_iterator q;
	
	H << sp;
	H << nl << _dllExport << "static const ::std::string __exceptionIds[" << exceptionIds.size() << "];";
	H << nl << _dllExport << "virtual const ::std::string* __getExceptionIds() const;";
	C << sp << nl << "const ::std::string " << scoped.substr(2) << "::__exceptionIds[" << exceptionIds.size()
	  << "] =";
	C << sb;
	q = exceptionIds.begin();
	while(q != exceptionIds.end())
	{
	    C << nl << '"' << *q << '"';
	    if(++q != exceptionIds.end())
	    {
		C << ',';
	    }
	}
	C << eb << ';';
	C << sp << nl << "const ::std::string*" << nl << scoped.substr(2) << "::__getExceptionIds() const";
	C << sb;
	C << nl << "return __exceptionIds;";
	C << eb;
    }

    return true;
}

void
Slice::Gen::TypesVisitor::visitExceptionEnd(const ExceptionPtr& p)
{
    if(!p->isLocal())
    {
	string name = fixKwd(p->name());
	string scoped = fixKwd(p->scoped());
	
	ExceptionPtr base = p->base();
    
	H << sp << nl << _dllExport << "virtual void __write(::IceInternal::BasicStream*) const;";
	H << nl << _dllExport << "virtual void __read(::IceInternal::BasicStream*);";

	H << sp << nl << _dllExport << "virtual void __marshal(const ::Ice::StreamPtr&) const;";
	H << nl << _dllExport << "virtual void __unmarshal(const ::Ice::StreamPtr&);";
	H << nl << _dllExport << "void ice_unmarshal(const ::std::string&, const ::Ice::StreamPtr&);";

	TypeStringList memberList;
	DataMemberList dataMembers = p->dataMembers();
	for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
	{
	    memberList.push_back(make_pair((*q)->type(), (*q)->name()));
	}
	C << sp << nl << "void" << nl << scoped.substr(2) << "::__write(::IceInternal::BasicStream* __os) const";
	C << sb;
	writeMarshalCode(C, memberList, 0);
	emitExceptionBase(base, "__write(__os)");
	C << eb;

	C << sp << nl << "void" << nl << scoped.substr(2) << "::__read(::IceInternal::BasicStream* __is)";
	C << sb;
	writeUnmarshalCode(C, memberList, 0);
	emitExceptionBase(base, "__read(__is)");
	C << eb;

	C << sp << nl << "void" << nl << scoped.substr(2)
	  << "::__marshal(const ::Ice::StreamPtr& __os) const";
	C << sb;
	emitExceptionBase(base, "__marshal(__os)"); // Base must come first (due to schema rules).
	writeGenericMarshalCode(C, memberList, 0);
	C << eb;
	C << sp << nl << "void" << nl << scoped.substr(2) << "::__unmarshal(const ::Ice::StreamPtr& __is)";
	C << sb;
	emitExceptionBase(base, "__unmarshal(__is)"); // Base must come first (due to schema rules).
	writeGenericUnmarshalCode(C, memberList, 0);
	C << eb;

	C << sp << nl << "void" << nl << scoped.substr(2) << "::ice_unmarshal(const ::std::string& __name, "
	  << "const ::Ice::StreamPtr& __is)";
	C << sb;
	C << nl << "__is->startReadException(__name);";
	C << nl << "__unmarshal(__is);";
	C << nl << "__is->endReadException();";
	C << eb;
    }


    H << eb << ';';
}

bool
Slice::Gen::TypesVisitor::visitStructStart(const StructPtr& p)
{
    string name = fixKwd(p->name());

    H << sp << nl << "struct " << name;
    H << sb;

    return true;
}

void
Slice::Gen::TypesVisitor::visitStructEnd(const StructPtr& p)
{
    string name = fixKwd(p->name());
    string scoped = fixKwd(p->scoped());

    H << sp;
    H << nl << _dllExport << "bool operator==(const " << name << "&) const;";
    H << nl << _dllExport << "bool operator!=(const " << name << "&) const;";
    H << nl << _dllExport << "bool operator<(const " << name << "&) const;";
    
    DataMemberList dataMembers = p->dataMembers();
    DataMemberList::const_iterator q;
    C << sp << nl << "bool" << nl << scoped.substr(2) << "::operator==(const " << name << "& __rhs) const";
    C << sb;
    C << nl << "return !operator!=(__rhs);";
    C << eb;
    C << sp << nl << "bool" << nl << scoped.substr(2) << "::operator!=(const " << name << "& __rhs) const";
    C << sb;
    C << nl << "if(this == &__rhs)";
    C << sb;
    C << nl << "return false;";
    C << eb;
    for(q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
	C << nl << "if(" << fixKwd((*q)->name()) << " != __rhs." << fixKwd((*q)->name()) << ')';
	C << sb;
	C << nl << "return true;";
	C << eb;
    }
    C << nl << "return false;";
    C << eb;
    C << sp << nl << "bool" << nl << scoped.substr(2) << "::operator<(const " << name << "& __rhs) const";
    C << sb;
    C << nl << "if(this == &__rhs)";
    C << sb;
    C << nl << "return false;";
    C << eb;
    for(q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
	C << nl << "if(" << fixKwd((*q)->name()) << " < __rhs." << fixKwd((*q)->name()) << ')';
	C << sb;
	C << nl << "return true;";
	C << eb;
	C << nl << "else if(__rhs." << fixKwd((*q)->name()) << " < " << fixKwd((*q)->name()) << ')';
	C << sb;
	C << nl << "return false;";
	C << eb;
    }
    C << nl << "return false;";
    C << eb;
    
    if(!p->isLocal())
    {
	//
	// Neither of these four member functions are virtual!
	//
	H << sp << nl << _dllExport << "void __write(::IceInternal::BasicStream*) const;";
	H << nl << _dllExport << "void __read(::IceInternal::BasicStream*);";

	H << sp << nl << _dllExport << "void ice_marshal(const ::std::string&, const ::Ice::StreamPtr&) const;";
	H << nl << _dllExport << "void ice_unmarshal(const ::std::string&, const ::Ice::StreamPtr&);";
	
	TypeStringList memberList;
	for(q = dataMembers.begin(); q != dataMembers.end(); ++q)
	{
	    memberList.push_back(make_pair((*q)->type(), (*q)->name()));
	}
	C << sp << nl << "void" << nl << scoped.substr(2) << "::__write(::IceInternal::BasicStream* __os) const";
	C << sb;
	writeMarshalCode(C, memberList, 0);
	C << eb;

	C << sp << nl << "void" << nl << scoped.substr(2) << "::__read(::IceInternal::BasicStream* __is)";
	C << sb;
	writeUnmarshalCode(C, memberList, 0);
	C << eb;

	C << sp << nl << "void" << nl << scoped.substr(2)
          << "::ice_marshal(const ::std::string& __name, const ::Ice::StreamPtr& __os) const";
	C << sb;
	C << nl << "__os->startWriteStruct(__name);";
	writeGenericMarshalCode(C, memberList, 0);
	C << nl << "__os->endWriteStruct();";
	C << eb;

	C << sp << nl << "void" << nl << scoped.substr(2)
	  << "::ice_unmarshal(const ::std::string& __name, const ::Ice::StreamPtr& __is)";
	C << sb;
	C << nl << "__is->startReadStruct(__name);";
	writeGenericUnmarshalCode(C, memberList, 0);
	C << nl << "__is->endReadStruct();";
	C << eb;
    }

    H << eb << ';';
}

void
Slice::Gen::TypesVisitor::visitDataMember(const DataMemberPtr& p)
{
    string name = fixKwd(p->name());
    string s = typeToString(p->type());
    H << sp << nl << s << ' ' << name << ';';
}

void
Slice::Gen::TypesVisitor::visitSequence(const SequencePtr& p)
{
    string name = fixKwd(p->name());
    TypePtr type = p->type();
    string s = typeToString(type);
    if(s[0] == ':')
    {
	s.insert(0, " ");
    }
    H << sp << nl << "typedef ::std::vector<" << s << "> " << name << ';';

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if(!p->isLocal() &&
	(!builtin || builtin->kind() == Builtin::KindObject || builtin->kind() == Builtin::KindObjectProxy))
    {
	string scoped = fixKwd(p->scoped());
	string scope = fixKwd(p->scope());

	//
	// TODO: ice_marshal/ice_unmarshal, __write/__read can be
	// moved into the helper.
	//
	H << sp << nl << "class __U__" << name << " { };";
	H << nl << _dllExport << "void __write(::IceInternal::BasicStream*, const " << name << "&, __U__" << name
	  << ");";
	H << nl << _dllExport << "void __read(::IceInternal::BasicStream*, " << name << "&, __U__" << name << ");";

	H << sp << nl << _dllExport << "void ice_marshal(const ::std::string&, const ::Ice::StreamPtr&, const "
          << name << "&, __U__" << name
	  << ");";
	H << nl << _dllExport << "void ice_unmarshal(const ::std::string&, const ::Ice::StreamPtr&, "
	  << name << "&, __U__" << name << ");";

	H << sp << nl << "class " << _dllExport << name << "Helper";
	H << sb;
	H.zeroIndent();
	H << nl << "public:";
	H.restoreIndent();
	H << sp << nl << "static void ice_marshal(const ::std::string&, const ::Ice::StreamPtr&, const "
          << name << "&);";
	H << nl << "static void ice_unmarshal(const ::std::string&, const ::Ice::StreamPtr&, "
	  << name << "&);";
	H << eb << ";";
	
	C << sp << nl << "void" << nl << scope.substr(2) << "__write(::IceInternal::BasicStream* __os, const "
	  << scoped << "& v, " << scope << "__U__" << name << ")";
	C << sb;
	C << nl << "__os->writeSize(::Ice::Int(v.size()));";
	C << nl << scoped << "::const_iterator p;";
	C << nl << "for(p = v.begin(); p != v.end(); ++p)";
	C << sb;
	writeMarshalUnmarshalCode(C, type, "(*p)", true);
	C << eb;
	C << eb;

	C << sp << nl << "void" << nl << scope.substr(2) << "__read(::IceInternal::BasicStream* __is, " << scoped
	  << "& v, " << scope << "__U__" << name << ')';
	C << sb;
	C << nl << "::Ice::Int sz;";
	C << nl << "__is->readSize(sz);";
	//
	// Don't use v.resize(sz) or v.reserve(sz) here, as it cannot
	// be checked whether sz is a reasonable value.
	//
	C << nl << "while(sz--)";
	C << sb;
	C << nl << "v.resize(v.size() + 1);";
	writeMarshalUnmarshalCode(C, type, "v.back()", false);
	C << eb;
	C << eb;

	C << sp << nl << "void" << nl << scope.substr(2)
          << "ice_marshal(const ::std::string& __name, const ::Ice::StreamPtr& __os, const "
	  << scoped << "& v, " << scope << "__U__" << name << ")";
	C << sb;
	C << nl << "__os->startWriteSequence(__name, ::Ice::Int(v.size()));";
	C << nl << scoped << "::const_iterator p;";
	C << nl << "for(p = v.begin(); p != v.end(); ++p)";
	C << sb;
	writeGenericMarshalUnmarshalCode(C, type, "(*p)", true, "\"e\"");
	C << eb;
	C << nl << "__os->endWriteSequence();";
	C << eb;

	C << sp << nl << "void" << nl << scope.substr(2) << "ice_unmarshal(const ::std::string& __name, "
	  << "const ::Ice::StreamPtr& __is, " << scoped << "& v, " << scope << "__U__" << name << ')';
	C << sb;
	C << nl << "::Ice::Int sz = __is->startReadSequence(__name);";
	//
	// Don't use v.resize(sz) or v.reserve(sz) here, as it cannot
	// be checked whether sz is a reasonable value.
	//
	C << nl << "while(sz--)";
	C << sb;
	C << nl << "v.resize(v.size() + 1);";
	writeGenericMarshalUnmarshalCode(C, type, "v.back()", false, "\"e\"");
	C << eb;
	C << nl << "__is->endReadSequence();";
	C << eb;

	C << sp << nl << "void" << nl << scope.substr(2) << name << "Helper::"
	  << "ice_marshal(const ::std::string& __name, const ::Ice::StreamPtr& __os, const "
          << name << "& v)";
        C << sb;
	C << nl << scope << "ice_marshal(__name, __os, v, " << scope << "__U__" << name << "());";
	C << eb;

	C << sp << nl << "void" << nl << scope.substr(2) << name << "Helper::"
	  << "ice_unmarshal(const ::std::string& __name, const ::Ice::StreamPtr& __is, "
	  << name << "& v)";
	C << sb;
	C << nl << scope << "ice_unmarshal(__name, __is, v, " << scope << "__U__" << name << "());";
	C << eb;
    }
}

void
Slice::Gen::TypesVisitor::visitDictionary(const DictionaryPtr& p)
{
    string name = fixKwd(p->name());
    TypePtr keyType = p->keyType();
    TypePtr valueType = p->valueType();
    string ks = typeToString(keyType);
    if(ks[0] == ':')
    {
	ks.insert(0, " ");
    }
    string vs = typeToString(valueType);
    H << sp << nl << "typedef ::std::map<" << ks << ", " << vs << "> " << name << ';';

    if(!p->isLocal())
    {
	string scoped = fixKwd(p->scoped());
	string scope = fixKwd(p->scope());

	//
	// TODO: ice_marshal/ice_unmarshal, __write/__read can be
	// moved into the helper.
	//
	H << sp << nl << "class __U__" << name << " { };";
	H << nl << _dllExport << "void __write(::IceInternal::BasicStream*, const " << name
	  << "&, __U__" << name << ");";
	H << nl << _dllExport << "void __read(::IceInternal::BasicStream*, " << name
	  << "&, __U__" << name << ");";

	H << sp << nl << _dllExport << "void ice_marshal(const ::std::string&, const ::Ice::StreamPtr&, const "
	  << name << "&, __U__" << name << ");";
	H << nl << _dllExport << "void ice_unmarshal(const ::std::string&, const ::Ice::StreamPtr&, "
	  << name << "&, __U__" << name << ");";

	H << sp << nl << "class " << _dllExport << name << "Helper";
	H << sb;
	H.zeroIndent();
	H << nl << "public:";
	H.restoreIndent();
	H << sp << nl << "static void ice_marshal(const ::std::string&, const ::Ice::StreamPtr&, const "
	  << name << "&);";
	H << nl << "static void ice_unmarshal(const ::std::string&, const ::Ice::StreamPtr&, "
	  << name << "&);";
	H << eb << ";";

	C << sp << nl << "void" << nl << scope.substr(2) << "__write(::IceInternal::BasicStream* __os, const "
	  << scoped << "& v, " << scope << "__U__" << name << ")";
	C << sb;
	C << nl << "__os->writeSize(::Ice::Int(v.size()));";
	C << nl << scoped << "::const_iterator p;";
	C << nl << "for(p = v.begin(); p != v.end(); ++p)";
	C << sb;
	writeMarshalUnmarshalCode(C, keyType, "p->first", true);
	writeMarshalUnmarshalCode(C, valueType, "p->second", true);
	C << eb;
	C << eb;

	C << sp << nl << "void" << nl << scope.substr(2) << "__read(::IceInternal::BasicStream* __is, " << scoped
	  << "& v, " << scope << "__U__" << name << ')';
	C << sb;
	C << nl << "::Ice::Int sz;";
	C << nl << "__is->readSize(sz);";
	C << nl << "while(sz--)";
	C << sb;
	C << nl << "::std::pair<" << ks << ", " << vs << "> pair;";
	writeMarshalUnmarshalCode(C, keyType, "pair.first", false);
	writeMarshalUnmarshalCode(C, valueType, "pair.second", false);
	C << nl << "v.insert(v.end(), pair);";
	C << eb;
	C << eb;

	C << sp << nl << "void" << nl << scope.substr(2)
          << "ice_marshal(const ::std::string& __name, const ::Ice::StreamPtr& __os, const "
	  << scoped << "& v, " << scope << "__U__" << name << ")";
	C << sb;
	C << nl << "__os->startWriteDictionary(__name, ::Ice::Int(v.size()));";
	C << nl << scoped << "::const_iterator p;";
	C << nl << "for(p = v.begin(); p != v.end(); ++p)";
	C << sb;
	C << nl << "__os->startWriteDictionaryElement();";
	writeGenericMarshalUnmarshalCode(C, keyType, "p->first", true, "\"key\"");
	writeGenericMarshalUnmarshalCode(C, valueType, "p->second", true, "\"value\"");
	C << nl << "__os->endWriteDictionaryElement();";
	C << eb;
	C << nl << "__os->endWriteDictionary();";
	C << eb;

	C << sp << nl << "void" << nl << scope.substr(2) << "ice_unmarshal(const ::std::string& __name, "
	  << "const ::Ice::StreamPtr& __is, " << scoped << "& v, " << scope << "__U__" << name << ')';
	C << sb;
	C << nl << "::Ice::Int sz = __is->startReadDictionary(__name);";
	C << nl << "while(sz--)";
	C << sb;
	C << nl << "::std::pair<" << ks << ", " << vs << "> pair;";
	C << nl << "__is->startReadDictionaryElement();";
	writeGenericMarshalUnmarshalCode(C, keyType, "pair.first", false, "\"key\"");
	writeGenericMarshalUnmarshalCode(C, valueType, "pair.second", false, "\"value\"");
	C << nl << "__is->endReadDictionaryElement();";
	C << nl << "v.insert(v.end(), pair);";
	C << eb;
	C << nl << "__is->endReadDictionary();";
	C << eb;

	C << sp << nl << "void" << nl << scope.substr(2) << name << "Helper::"
	  << "ice_marshal(const ::std::string& __name, const ::Ice::StreamPtr& __os, const "
          << name << "& v)";
        C << sb;
	C << nl << scope << "ice_marshal(__name, __os, v, " << scope << "__U__" << name << "());";
	C << eb;

	C << sp << nl << "void" << nl << scope.substr(2) << name << "Helper::"
	  << "ice_unmarshal(const ::std::string& __name, const ::Ice::StreamPtr& __is, "
	  << name << "& v)";
	C << sb;
	C << nl << scope << "ice_unmarshal(__name, __is, v, " << scope << "__U__" << name << "());";
	C << eb;
    }
}

void
Slice::Gen::TypesVisitor::visitEnum(const EnumPtr& p)
{
    string name = fixKwd(p->name());
    EnumeratorList enumerators = p->getEnumerators();
    H << sp << nl << "enum " << name;
    H << sb;
    EnumeratorList::const_iterator en = enumerators.begin();
    while(en != enumerators.end())
    {
	H << nl << fixKwd((*en)->name());
	if(++en != enumerators.end())
	{
	    H << ',';
	}
    }
    H << eb << ';';

    if(!p->isLocal())
    {
	string scoped = fixKwd(p->scoped());
	string scope = fixKwd(p->scope());
	
	int sz = enumerators.size();
	assert(sz <= 0x7fffffff); // 64-bit enums are not supported
	
	H << sp << nl << _dllExport << "void __write(::IceInternal::BasicStream*, " << name << ");";
	H << nl << _dllExport << "void __read(::IceInternal::BasicStream*, " << name << "&);";

	H << sp << nl << _dllExport << "void ice_marshal(const ::std::string&, const ::Ice::StreamPtr&, "
	  << name << ");";
	H << nl << _dllExport << "void ice_unmarshal(const ::std::string&, const ::Ice::StreamPtr&, "
	  << name << "&);";

	C << sp << nl << "void" << nl << scope.substr(2) << "__write(::IceInternal::BasicStream* __os, " << scoped
	  << " v)";
	C << sb;
	if(sz <= 0x7f)
	{
	    C << nl << "__os->write(static_cast< ::Ice::Byte>(v));";
	}
	else if(sz <= 0x7fff)
	{
	    C << nl << "__os->write(static_cast< ::Ice::Short>(v));";
	}
	else
	{
	    C << nl << "__os->write(static_cast< ::Ice::Int>(v));";
	}
	C << eb;

	C << sp << nl << "void" << nl << scope.substr(2) << "__read(::IceInternal::BasicStream* __is, " << scoped
	  << "& v)";
	C << sb;
	if(sz <= 0x7f)
	{
	    C << nl << "::Ice::Byte val;";
	    C << nl << "__is->read(val);";
	    C << nl << "v = static_cast< " << scoped << ">(val);";
	}
	else if(sz <= 0x7fff)
	{
	    C << nl << "::Ice::Short val;";
	    C << nl << "__is->read(val);";
	    C << nl << "v = static_cast< " << scoped << ">(val);";
	}
	else
	{
	    C << nl << "::Ice::Int val;";
	    C << nl << "__is->read(val);";
	    C << nl << "v = static_cast< " << scoped << ">(val);";
	}
	C << eb;

	string tableName = "__T__";
	tableName += name;

	C << sp;
	C << nl << "static const ::std::string " << "__RT__" << name << "[] =";
	C << sb;
	en = enumerators.begin();
	while(en != enumerators.end())
	{
	    C << nl << "\"" << (*en)->name() << "\"";
	    if(++en != enumerators.end())
	    {
		C << ',';
	    }
	}
	C << eb << ";";
	C << nl << "static const ::Ice::StringSeq __T__" << name << "(&__RT__" << name << "[0], "
	  << "&__RT__" << name << "[" << enumerators.size() << "]);";
	
	C << sp << nl << "void" << nl << scope.substr(2) << "ice_marshal(const ::std::string& __name, "
	  << "const ::Ice::StreamPtr& __os, " << scoped << " v)";
	C << sb;
	C << nl << "__os->writeEnum(__name, " << "__T__" << name << ", v);";
	C << eb;

	C << sp << nl << "void" << nl << scope.substr(2) << "ice_unmarshal(const ::std::string& __name, "
	  << "const ::Ice::StreamPtr& __is, " << scoped << "& v)";
	C << sb;
	C << nl << "::Ice::Int ordinal = __is->readEnum(__name, __T__" << name << ");";
        C << nl << "v = static_cast< " << scoped << ">(ordinal);";
	C << eb;
    }
}

void
Slice::Gen::TypesVisitor::visitConstDef(const ConstDefPtr& p)
{
    H << sp;
    H << nl << "const " << typeToString(p->type()) << " " << fixKwd(p->name()) << " = ";

    BuiltinPtr bp = BuiltinPtr::dynamicCast(p->type());
    if(bp && bp->kind() == Builtin::KindString)
    {
	//
	// Expand strings into the basic source character set. We can't use isalpha() and the like
	// here because they are sensitive to the current locale.
	//
	static const string basicSourceChars = "abcdefghijklmnopqrstuvwxyz"
					       "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
					       "0123456789"
					       "_{}[]#()<>%:;,?*+=/^&|~!=,\\\"' \t";
    	static const set<char> charSet(basicSourceChars.begin(), basicSourceChars.end());

	H << "\"";					// Opening "

	ios_base::fmtflags originalFlags = H.flags();	// Save stream state
	streamsize originalWidth = H.width();
	ostream::char_type originalFill = H.fill();

	const string val = p->value();
	for(string::const_iterator c = val.begin(); c != val.end(); ++c)
	{
	    if(charSet.find(*c) == charSet.end())
	    {
		unsigned char uc = *c;			// char may be signed, so make it positive
		H << "\\";    				// Print as octal if not in basic source character set
		H.flags(ios_base::oct);
		H.width(3);
		H.fill('0');
		H << static_cast<unsigned>(uc);
	    }
	    else
	    {
		H << *c;				// Print normally if in basic source character set
	    }
	}

	H.fill(originalFill);				// Restore stream state
	H.width(originalWidth);
	H.flags(originalFlags);

	H << "\"";					// Closing "
    }
    else if(bp && bp->kind() == Builtin::KindLong)
    {
	H << "ICE_INT64_LITERAL(" << p->value() << ")";
    }
    else
    {
	EnumPtr ep = EnumPtr::dynamicCast(p->type());
	if(ep)
	{
	    H << fixKwd(p->value());
	}
	else
	{
	    H << p->value();
	}
    }

    H << ";";
}

void
Slice::Gen::TypesVisitor::emitExceptionBase(const ExceptionPtr& base, const std::string& call)
{
    if(base)
    {
	C.zeroIndent();
	C << nl << "#ifdef _WIN32"; // COMPILERBUG
	C.restoreIndent();
	C << nl << fixKwd(base->name()) << "::" << call << ";";
	C.zeroIndent();
	C << nl << "#else";
	C.restoreIndent();
	C << nl << fixKwd(base->scoped()) << "::" << call << ";";
	C.zeroIndent();
	C << nl << "#endif";
	C.restoreIndent();
    }
}

Slice::Gen::ProxyDeclVisitor::ProxyDeclVisitor(Output& h, Output& c, const string& dllExport) :
    H(h), C(c), _dllExport(dllExport)
{
}

bool
Slice::Gen::ProxyDeclVisitor::visitUnitStart(const UnitPtr& p)
{
    if(!p->hasNonLocalClassDecls())
    {
	return false;
    }

    H << sp << nl << "namespace IceProxy" << nl << '{';

    return true;
}

void
Slice::Gen::ProxyDeclVisitor::visitUnitEnd(const UnitPtr& p)
{
    H << sp << nl << '}';
}
    
bool
Slice::Gen::ProxyDeclVisitor::visitModuleStart(const ModulePtr& p)
{
    if(!p->hasNonLocalClassDecls())
    {
	return false;
    }

    string name = fixKwd(p->name());
    
    H << sp << nl << "namespace " << name << nl << '{';

    return true;
}

void
Slice::Gen::ProxyDeclVisitor::visitModuleEnd(const ModulePtr& p)
{
    H << sp << nl << '}';
}

void
Slice::Gen::ProxyDeclVisitor::visitClassDecl(const ClassDeclPtr& p)
{
    if(p->isLocal())
    {
	return;
    }

    string name = fixKwd(p->name());

    H << sp << nl << "class " << name << ';';
    H << nl << _dllExport << "bool operator==(const " << name << "&, const " << name << "&);";
    H << nl << _dllExport << "bool operator!=(const " << name << "&, const " << name << "&);";
    H << nl << _dllExport << "bool operator<(const " << name << "&, const " << name << "&);";
}

Slice::Gen::ProxyVisitor::ProxyVisitor(Output& h, Output& c, const string& dllExport) :
    H(h), C(c), _dllExport(dllExport)
{
}

bool
Slice::Gen::ProxyVisitor::visitUnitStart(const UnitPtr& p)
{
    if(!p->hasNonLocalClassDecls())
    {
	return false;
    }

    H << sp << nl << "namespace IceProxy" << nl << '{';

    return true;
}

void
Slice::Gen::ProxyVisitor::visitUnitEnd(const UnitPtr& p)
{
    H << sp << nl << '}';
}
    
bool
Slice::Gen::ProxyVisitor::visitModuleStart(const ModulePtr& p)
{
    if(!p->hasNonLocalClassDecls())
    {
	return false;
    }

    string name = fixKwd(p->name());
    
    H << sp << nl << "namespace " << name << nl << '{';

    return true;
}

void
Slice::Gen::ProxyVisitor::visitModuleEnd(const ModulePtr& p)
{
    H << sp << nl << '}';
}

bool
Slice::Gen::ProxyVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    if(p->isLocal())
    {
	return false;
    }

    string name = fixKwd(p->name());
    string scoped = fixKwd(p->scoped());
    ClassList bases = p->bases();

    H << sp << nl << "class " << _dllExport << name << " : ";
    if(bases.empty())
    {
	H << "virtual public ::IceProxy::Ice::Object";
    }
    else
    {
	H.useCurrentPosAsIndent();
	ClassList::const_iterator q = bases.begin();
	while(q != bases.end())
	{
	    H << "virtual public ::IceProxy" << fixKwd((*q)->scoped());
	    if(++q != bases.end())
	    {
		H << ',' << nl;
	    }
	}
	H.restoreIndent();
    }

    H << sb;
    H.dec();
    H << nl << "public:";
    H.inc();

    return true;
}

void
Slice::Gen::ProxyVisitor::visitClassDefEnd(const ClassDefPtr& p)
{
    string scoped = fixKwd(p->scoped());
    string scope = fixKwd(p->scope());
    
    H.dec();
    H << sp << nl << "private: ";
    H.inc();
    H << sp << nl << "virtual ::IceInternal::Handle< ::IceDelegateM::Ice::Object> __createDelegateM();";
    H << nl << "virtual ::IceInternal::Handle< ::IceDelegateD::Ice::Object> __createDelegateD();";
    H << eb << ';';
    C << sp << nl << "::IceInternal::Handle< ::IceDelegateM::Ice::Object>";
    C << nl << "IceProxy" << scoped << "::__createDelegateM()";
    C << sb;
    C << nl << "return ::IceInternal::Handle< ::IceDelegateM::Ice::Object>(new ::IceDelegateM" << scoped << ");";
    C << eb;
    C << sp << nl << "::IceInternal::Handle< ::IceDelegateD::Ice::Object>";
    C << nl << "IceProxy" << scoped << "::__createDelegateD()";
    C << sb;
    C << nl << "return ::IceInternal::Handle< ::IceDelegateD::Ice::Object>(new ::IceDelegateD" << scoped << ");";
    C << eb;

    C << sp;
    C << nl << "bool" << nl << "IceProxy" << scope << "operator==(const ::IceProxy" << scoped
      << "& l, const ::IceProxy" << scoped << "& r)";
    C << sb;
    C << nl << "return static_cast<const ::IceProxy::Ice::Object&>(l) == "
      << "static_cast<const ::IceProxy::Ice::Object&>(r);";
    C << eb;
    C << sp;
    C << nl << "bool" << nl << "IceProxy" << scope << "operator!=(const ::IceProxy" << scoped
      << "& l, const ::IceProxy" << scoped << "& r)";
    C << sb;
    C << nl << "return static_cast<const ::IceProxy::Ice::Object&>(l) != "
      << "static_cast<const ::IceProxy::Ice::Object&>(r);";
    C << eb;
    C << sp;
    C << nl << "bool" << nl << "IceProxy" << scope << "operator<(const ::IceProxy" << scoped
      << "& l, const ::IceProxy" << scoped << "& r)";
    C << sb;
    C << nl << "return static_cast<const ::IceProxy::Ice::Object&>(l) < "
      << "static_cast<const ::IceProxy::Ice::Object&>(r);";
    C << eb;
}

void
Slice::Gen::ProxyVisitor::visitOperation(const OperationPtr& p)
{
    string name = fixKwd(p->name());
    string scoped = fixKwd(p->scoped());
    string scope = fixKwd(p->scope());

    TypePtr ret = p->returnType();
    string retS = returnTypeToString(ret);

    string params = "(";
    string paramsDecl = "("; // With declarators
    string args = "(";

    ParamDeclList paramList = p->parameters();
    for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
	string typeString = (*q)->isOutParam() ? outputTypeToString((*q)->type()) : inputTypeToString((*q)->type());
	params += typeString;
	paramsDecl += typeString;
	paramsDecl += ' ';
	string name = (*q)->name();
	paramsDecl += fixKwd(name);
	args += fixKwd(name);
	params += ", ";
	paramsDecl += ", ";
	args += ", ";
    }

    params += "const ::Ice::Context& = ::Ice::Context())";
    paramsDecl += "const ::Ice::Context& __context)";
    args += "__context)";

    H << sp << nl << retS << ' ' << name << params << ';';
    C << sp << nl << retS << nl << "IceProxy" << scoped << paramsDecl;
    string thisPointer = fixKwd(scope.substr(0, scope.size() - 2)) + "*";
    C << sb;
    C << nl << "int __cnt = 0;";
    C << nl << "while(true)";
    C << sb;
    C << nl << "try";
    C << sb;
    C << nl << "::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase = __getDelegate();";
    C << nl << "::IceDelegate" << thisPointer << " __del = dynamic_cast< ::IceDelegate"
      << thisPointer << ">(__delBase.get());";
    C << nl;
    if(ret)
    {
	C << "return ";
    }
    C << "__del->" << name << args << ";";
    if(!ret)
    {
	C << nl << "return;";
    }
    C << eb;
    C << nl << "catch(const ::Ice::LocationForward& __ex)";
    C << sb;
    C << nl << "__locationForward(__ex);";
    C << eb;
    C << nl << "catch(const ::IceInternal::NonRepeatable& __ex)";
    C << sb;
    if(p->mode() == Operation::Idempotent || p->mode() == Operation::Nonmutating)
    {
	C << nl << "__handleException(*__ex.get(), __cnt);";
    }
    else
    {
	C << nl << "__rethrowException(*__ex.get());";
    }
    C << eb;
    C << nl << "catch(const ::Ice::LocalException& __ex)";
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
    if(!p->hasNonLocalClassDecls())
    {
	return false;
    }

    H << sp << nl << "namespace IceDelegate" << nl << '{';

    return true;
}

void
Slice::Gen::DelegateVisitor::visitUnitEnd(const UnitPtr& p)
{
    H << sp << nl << '}';
}
    
bool
Slice::Gen::DelegateVisitor::visitModuleStart(const ModulePtr& p)
{
    if(!p->hasNonLocalClassDecls())
    {
	return false;
    }

    string name = fixKwd(p->name());
    
    H << sp << nl << "namespace " << name << nl << '{';

    return true;
}

void
Slice::Gen::DelegateVisitor::visitModuleEnd(const ModulePtr& p)
{
    H << sp << nl << '}';
}

bool
Slice::Gen::DelegateVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    if(p->isLocal())
    {
	return false;
    }

    string name = fixKwd(p->name());
    ClassList bases = p->bases();

    H << sp << nl << "class " << _dllExport << name << " : ";
    if(bases.empty())
    {
	H << "virtual public ::IceDelegate::Ice::Object";
    }
    else
    {
	H.useCurrentPosAsIndent();
	ClassList::const_iterator q = bases.begin();
	while(q != bases.end())
	{
	    H << "virtual public ::IceDelegate" << fixKwd((*q)->scoped());
	    if(++q != bases.end())
	    {
		H << ',' << nl;
	    }
	}
	H.restoreIndent();
    }
    H << sb;
    H.dec();
    H << nl << "public:";
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
    string name = fixKwd(p->name());

    TypePtr ret = p->returnType();
    string retS = returnTypeToString(ret);

    string params = "(";

    ParamDeclList paramList = p->parameters();
    for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
	string typeString = (*q)->isOutParam() ? outputTypeToString((*q)->type()) : inputTypeToString((*q)->type());
	params += typeString;
	params += ", ";
    }

    params += "const ::Ice::Context&)";
    
    H << sp << nl << "virtual " << retS << ' ' << name << params << " = 0;";
}

Slice::Gen::DelegateMVisitor::DelegateMVisitor(Output& h, Output& c, const string& dllExport) :
    H(h), C(c), _dllExport(dllExport)
{
}

bool
Slice::Gen::DelegateMVisitor::visitUnitStart(const UnitPtr& p)
{
    if(!p->hasNonLocalClassDecls())
    {
	return false;
    }

    H << sp << nl << "namespace IceDelegateM" << nl << '{';

    return true;
}

void
Slice::Gen::DelegateMVisitor::visitUnitEnd(const UnitPtr& p)
{
    H << sp << nl << '}';
}
    
bool
Slice::Gen::DelegateMVisitor::visitModuleStart(const ModulePtr& p)
{
    if(!p->hasNonLocalClassDecls())
    {
	return false;
    }

    string name = fixKwd(p->name());
    
    H << sp << nl << "namespace " << name << nl << '{';

    return true;
}

void
Slice::Gen::DelegateMVisitor::visitModuleEnd(const ModulePtr& p)
{
    H << sp << nl << '}';
}

bool
Slice::Gen::DelegateMVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    if(p->isLocal())
    {
	return false;
    }

    string name = fixKwd(p->name());
    string scoped = fixKwd(p->scoped());
    ClassList bases = p->bases();

    H << sp << nl << "class " << _dllExport << name << " : ";
    H.useCurrentPosAsIndent();
    H << "virtual public ::IceDelegate" << scoped << ',';
    if(bases.empty())
    {
	H << nl << "virtual public ::IceDelegateM::Ice::Object";
    }
    else
    {
	ClassList::const_iterator q = bases.begin();
	while(q != bases.end())
	{
	    H << nl << "virtual public ::IceDelegateM" << fixKwd((*q)->scoped());
	    if(++q != bases.end())
	    {
		H << ',';
	    }
	}
    }
    H.restoreIndent();
    H << sb;
    H.dec();
    H << nl << "public:";
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
    string name = fixKwd(p->name());
    string scoped = fixKwd(p->scoped());

    TypePtr ret = p->returnType();
    string retS = returnTypeToString(ret);

    string params = "(";
    string paramsDecl = "("; // With declarators

    TypeStringList inParams;
    TypeStringList outParams;
    ParamDeclList paramList = p->parameters();
    for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
	string name = (*q)->name();
	TypePtr type = (*q)->type();
	bool isOutParam = (*q)->isOutParam();
	string typeString;
	if(isOutParam)
	{
	    outParams.push_back(make_pair(type, name));
	    typeString = outputTypeToString(type);
	}
	else
	{
	    inParams.push_back(make_pair(type, name));
	    typeString = inputTypeToString(type);
	}
	params += typeString;
	paramsDecl += typeString;
	paramsDecl += ' ';
	paramsDecl += fixKwd(name);
	params += ", ";
	paramsDecl += ", ";
    }

    params += "const ::Ice::Context&)";
    paramsDecl += "const ::Ice::Context& __context)";
    
    ExceptionList throws = p->throws();
    throws.sort();
    throws.unique();

    H << sp << nl << "virtual " << retS << ' ' << name << params << ';';
    C << sp << nl << retS << nl << "IceDelegateM" << scoped << paramsDecl;
    C << sb;
    C << nl << "static const ::std::string __operation(\"" << p->name() << "\");";
    C << nl << "::IceInternal::Outgoing __out(__connection, __reference, __operation, "
      << "static_cast< ::Ice::OperationMode>(" << p->mode() << "), __context);";
    if(ret || !outParams.empty() || !throws.empty())
    {
	C << nl << "::IceInternal::BasicStream* __is = __out.is();";
    }
    if(!inParams.empty())
    {
	C << nl << "::IceInternal::BasicStream* __os = __out.os();";
    }
    writeMarshalCode(C, inParams, 0);
    C << nl << "if(!__out.invoke())";
    C << sb;
    if(!throws.empty())
    {
	ExceptionList::const_iterator r;
	C << nl << "static ::std::string __throws[] =";
	C << sb;
	for(r = throws.begin(); r != throws.end(); ++r)
	{
	    C << nl << '"' << (*r)->scoped() << '"';
	    if(r != throws.end())
	    {
		C << ',';
	    }
	}
	C << eb << ';';
	C << nl << "switch(__is->throwException(__throws, __throws + " << throws.size() << "))";
	C << sb;
	int cnt = 0;
	for(r = throws.begin(); r != throws.end(); ++r)
	{
	    C << nl << "case " << cnt++ << ':';
	    C << sb;
	    C << nl << fixKwd((*r)->scoped()) << " __ex;";
	    C << nl << "__ex.__read(__is);";
	    C << nl << "throw __ex;";
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
    if(ret)
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
    if(!p->hasNonLocalClassDecls())
    {
	return false;
    }

    H << sp << nl << "namespace IceDelegateD" << nl << '{';

    return true;
}

void
Slice::Gen::DelegateDVisitor::visitUnitEnd(const UnitPtr& p)
{
    H << sp << nl << '}';
}
    
bool
Slice::Gen::DelegateDVisitor::visitModuleStart(const ModulePtr& p)
{
    if(!p->hasNonLocalClassDecls())
    {
	return false;
    }

    string name = fixKwd(p->name());
    
    H << sp << nl << "namespace " << name << nl << '{';

    return true;
}

void
Slice::Gen::DelegateDVisitor::visitModuleEnd(const ModulePtr& p)
{
    H << sp << nl << '}';
}

bool
Slice::Gen::DelegateDVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    if(p->isLocal())
    {
	return false;
    }

    string name = fixKwd(p->name());
    string scoped = fixKwd(p->scoped());
    ClassList bases = p->bases();

    H << sp << nl << "class " << _dllExport << name << " : ";
    H.useCurrentPosAsIndent();
    H << "virtual public ::IceDelegate" << scoped << ',';
    if(bases.empty())
    {
	H << nl << "virtual public ::IceDelegateD::Ice::Object";
    }
    else
    {
	ClassList::const_iterator q = bases.begin();
	while(q != bases.end())
	{
	    H << nl << "virtual public ::IceDelegateD" << fixKwd((*q)->scoped());
	    if(++q != bases.end())
	    {
		H << ',';
	    }
	}
    }
    H.restoreIndent();
    H << sb;
    H.dec();
    H << nl << "public:";
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
    
    string name = fixKwd(p->name());
    string scoped = fixKwd(p->scoped());

    TypePtr ret = p->returnType();
    string retS = returnTypeToString(ret);

    string params = "(";
    string paramsDecl = "("; // With declarators
    string args = "(";

    ParamDeclList paramList = p->parameters();
    for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
	string typeString = (*q)->isOutParam() ? outputTypeToString((*q)->type()) : inputTypeToString((*q)->type());
	params += typeString;
	paramsDecl += typeString;
	paramsDecl += ' ';
	string name = (*q)->name();
	paramsDecl += fixKwd(name);
	args += fixKwd(name);
	params += ", ";
	paramsDecl += ", ";
	args += ", ";
    }

    params += "const ::Ice::Context& = ::Ice::Context())";
    paramsDecl += "const ::Ice::Context& __context)";
    args += "__current)";
    
    H << sp << nl << "virtual " << retS << ' ' << name << params << ';';
    C << sp << nl << retS << nl << "IceDelegateD" << scoped << paramsDecl;
    C << sb;
    C << nl << "::Ice::Current __current;";
    C << nl << "__initCurrent(__current, \"" << p->name()
      << "\", static_cast< ::Ice::OperationMode>(" << p->mode() << "), __context);";
    C << nl << "while(true)";
    C << sb;
    C << nl << "::IceInternal::Direct __direct(__current);";
    string thisPointer = fixKwd(cl->scoped()) + "*";
    C << nl << thisPointer << " __servant = dynamic_cast< " << thisPointer << ">(__direct.facetServant().get());";
    C << nl << "if(!__servant)";
    C << sb;
    C << nl << "::Ice::OperationNotExistException __opEx(__FILE__, __LINE__);";
    C << nl << "__opEx.id = __current.id;";
    C << nl << "__opEx.facet = __current.facet;";
    C << nl << "__opEx.operation = __current.operation;";
    C << nl << "throw __opEx;";
    C << eb;
    C << nl;
    if(ret)
    {
	C << "return ";
    }
    C << "__servant->" << name << args << ';';
    if(!ret)
    {
	C << nl << "return;";
    }
    C << eb;
    C << eb;
}

Slice::Gen::ObjectDeclVisitor::ObjectDeclVisitor(Output& h, Output& c, const string& dllExport) :
    H(h), C(c), _dllExport(dllExport)
{
}

bool
Slice::Gen::ObjectDeclVisitor::visitModuleStart(const ModulePtr& p)
{
    if(!p->hasClassDecls())
    {
	return false;
    }

    string name = fixKwd(p->name());
    
    H << sp << nl << "namespace " << fixKwd(name) << nl << '{';

    return true;
}

void
Slice::Gen::ObjectDeclVisitor::visitModuleEnd(const ModulePtr& p)
{
    H << sp << nl << '}';
}

void
Slice::Gen::ObjectDeclVisitor::visitClassDecl(const ClassDeclPtr& p)
{
    string name = fixKwd(p->name());
    
    H << sp << nl << "class " << name << ';';
    H << nl << _dllExport << "bool operator==(const " << name << "&, const " << name << "&);";
    H << nl << _dllExport << "bool operator!=(const " << name << "&, const " << name << "&);";
    H << nl << _dllExport << "bool operator<(const " << name << "&, const " << name << "&);";
}

Slice::Gen::ObjectVisitor::ObjectVisitor(Output& h, Output& c, const string& dllExport) :
    H(h), C(c), _dllExport(dllExport)
{
}

bool
Slice::Gen::ObjectVisitor::visitModuleStart(const ModulePtr& p)
{
    if(!p->hasClassDefs())
    {
	return false;
    }

    string name = fixKwd(p->name());
    
    H << sp << nl << "namespace " << name << nl << '{';

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
    string name = fixKwd(p->name());
    string scoped = fixKwd(p->scoped());
    ClassList bases = p->bases();
    
    string exp1;
    string exp2;
    if(_dllExport.size())
    {
	if(p->hasDataMembers())
	{
	    exp2 = _dllExport;
	}
	else
	{
	    exp1 = _dllExport;
	}
    }

    H << sp;
    H << nl << "class " << exp1 << name << " : ";
    H.useCurrentPosAsIndent();
    if(bases.empty())
    {
	if(p->isLocal())
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
	while(q != bases.end())
	{
	    H << "virtual public " << fixKwd((*q)->scoped());
	    if(++q != bases.end())
	    {
		H << ',' << nl;
	    }
	}
    }
    H.restoreIndent();
    H << sb;
    H.dec();
    H << nl << "public:";
    H.inc();

    if(!p->isLocal())
    {
	ClassList allBases = p->allBases();
	StringList ids;
	transform(allBases.begin(), allBases.end(), back_inserter(ids),
		  ::IceUtil::constMemFun(&ClassDef::scoped));
	StringList other;
	other.push_back(p->scoped());
	other.push_back("::Ice::Object");
	other.sort();
	ids.merge(other);
	ids.unique();
        StringList::const_iterator firstIter = ids.begin();
        StringList::const_iterator scopedIter = find(ids.begin(), ids.end(), p->scoped());
        assert(scopedIter != ids.end());
        int scopedPos = distance(firstIter, scopedIter);

	StringList::const_iterator q;

	H << sp;
	H << nl << exp2 << "static const ::std::string __ids[" << ids.size() << "];";
	H << nl << exp2 << "virtual bool ice_isA(const ::std::string&, const ::Ice::Current& = ::Ice::Current());";
	H << nl << exp2 << "virtual ::std::vector< ::std::string> ice_ids(const ::Ice::Current& = ::Ice::Current());";
	H << nl << exp2 << "virtual const ::std::string& ice_id(const ::Ice::Current& = ::Ice::Current());";
	H << nl << exp2 << "static const ::std::string& ice_staticId();";

	if(!p->isAbstract())
	{
	    H.dec();
	    H << sp << nl << "private:";
	    H.inc();
	    H << sp << nl << exp2 << "static ::Ice::ObjectFactoryPtr _factory;";
	    H.dec();
	    H << sp << nl << "public:";
	    H.inc();
	    H << sp << nl << exp2 << "static const ::Ice::ObjectFactoryPtr& ice_factory();";
	}
	C << sp;
	C << nl << "const ::std::string " << scoped.substr(2) << "::__ids[" << ids.size() << "] =";
	C << sb;
	q = ids.begin();
	while(q != ids.end())
	{
	    C << nl << '"' << *q << '"';
	    if(++q != ids.end())
	    {
		C << ',';
	    }
	}
	C << eb << ';';

	C << sp;
	C << nl << "bool" << nl << scoped.substr(2) << "::ice_isA(const ::std::string& s, const ::Ice::Current&)";
	C << sb;
	C << nl << "return ::std::binary_search(__ids, __ids + " << ids.size() << ", s);";
	C << eb;

	C << sp;
	C << nl << "::std::vector< ::std::string>" << nl << scoped.substr(2) << "::ice_ids(const ::Ice::Current&)";
	C << sb;
	C << nl << "return ::std::vector< ::std::string>(&__ids[0], &__ids[" << ids.size() << "]);";
	C << eb;

	C << sp;
	C << nl << "const ::std::string&" << nl << scoped.substr(2) << "::ice_id(const ::Ice::Current&)";
	C << sb;
	C << nl << "return __ids[" << scopedPos << "];";
	C << eb;

	C << sp;
	C << nl << "const ::std::string&" << nl << scoped.substr(2) << "::ice_staticId()";
	C << sb;
	C << nl << "return __ids[" << scopedPos << "];";
	C << eb;
    }

    return true;
}

void
Slice::Gen::ObjectVisitor::visitClassDefEnd(const ClassDefPtr& p)
{
    string scoped = fixKwd(p->scoped());
    string scope = fixKwd(p->scope());

    if(!p->isLocal())
    {	
	string exp2;
	if(_dllExport.size())
	{
	    if(p->hasDataMembers())
	    {
		exp2 = _dllExport;
	    }
	}

	ClassList bases = p->bases();
	ClassDefPtr base;
	if(!bases.empty() && !bases.front()->isInterface())
	{
	    base = bases.front();
	}
    
	OperationList allOps = p->allOperations();
	if(!allOps.empty())
	{
	    StringList allOpNames;
	    transform(allOps.begin(), allOps.end(), back_inserter(allOpNames),
		      ::IceUtil::constMemFun(&Operation::name));
	    allOpNames.push_back("ice_facets");
	    allOpNames.push_back("ice_id");
	    allOpNames.push_back("ice_ids");
	    allOpNames.push_back("ice_isA");
	    allOpNames.push_back("ice_ping");
	    allOpNames.sort();
	    allOpNames.unique();

	    StringList::const_iterator q;
	    
	    H << sp;
	    H << nl << exp2 << "static ::std::string __all[" << allOpNames.size() << "];";
	    H << nl << exp2
	      << "virtual ::IceInternal::DispatchStatus __dispatch(::IceInternal::Incoming&, const ::Ice::Current&);";
	    C << sp;
	    C << nl << "::std::string " << scoped.substr(2) << "::__all[] =";
	    C << sb;
	    q = allOpNames.begin();
	    while(q != allOpNames.end())
	    {
		C << nl << '"' << *q << '"';
		if(++q != allOpNames.end())
		{
		    C << ',';
		}
	    }
	    C << eb << ';';
	    C << sp;
	    C << nl << "::IceInternal::DispatchStatus" << nl << scoped.substr(2)
	      << "::__dispatch(::IceInternal::Incoming& in, const ::Ice::Current& current)";
	    C << sb;
	    C << nl << "::std::pair<const ::std::string*, const ::std::string*> r = "
	      << "::std::equal_range(__all, __all + " << allOpNames.size() << ", current.operation);";
	    C << nl << "if(r.first == r.second)";
	    C << sb;
	    C << nl << "return ::IceInternal::DispatchOperationNotExist;";
	    C << eb;
	    C << sp;
	    C << nl << "switch(r.first - __all)";
	    C << sb;
	    int i = 0;
	    for(q = allOpNames.begin(); q != allOpNames.end(); ++q)
	    {
		C << nl << "case " << i++ << ':';
		C << sb;
		C << nl << "return ___" << fixKwd(*q) << "(in, current);";
		C << eb;
	    }
	    C << eb;
	    C << sp;
	    C << nl << "assert(false);";
	    C << nl << "return ::IceInternal::DispatchOperationNotExist;";
	    C << eb;
	}
	
	H << sp;
	H << nl << exp2 << "virtual void __write(::IceInternal::BasicStream*) const;";
	H << nl << exp2 << "virtual void __read(::IceInternal::BasicStream*);";

	H << sp;
	H << nl << exp2 << "virtual void __marshal(const ::Ice::StreamPtr&) const;";
	H << nl << exp2 << "virtual void __unmarshal(const ::Ice::StreamPtr&);";
	
	H << sp;
	H << nl << exp2 << "static void ice_unmarshal(const ::std::string&, const ::Ice::StreamPtr&, "
	  << scoped << "Ptr&);";

	TypeStringList memberList;
	DataMemberList dataMembers = p->dataMembers();
	for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
	{
	    memberList.push_back(make_pair((*q)->type(), (*q)->name()));
	}
	C << sp;
	C << nl << "void" << nl << scoped.substr(2) << "::__write(::IceInternal::BasicStream* __os) const";
	C << sb;
	writeMarshalCode(C, memberList, 0);
	emitClassBase(base, "__write(__os)");
	C << eb;
	C << sp;
	C << nl << "void" << nl << scoped.substr(2) << "::__read(::IceInternal::BasicStream* __is)";
	C << sb;
	writeUnmarshalCode(C, memberList, 0);
	emitClassBase(base, "__read(__is)");
	C << eb;

	C << sp;
	C << nl << "void" << nl << scoped.substr(2)
          << "::__marshal(const ::Ice::StreamPtr& __os) const";
	C << sb;
	emitClassBase(base, "__marshal(__os)"); // Base must come first (due to schema rules).
	writeGenericMarshalCode(C, memberList, 0);
	C << eb;
	
	C << sp;
	C << nl << "void" << nl << scoped.substr(2) << "::__unmarshal(const ::Ice::StreamPtr& __is)";
	C << sb;
	emitClassBase(base, "__unmarshal(__is)"); // Base must come first (due to schema rules).
	writeGenericUnmarshalCode(C, memberList, 0);
	C << eb;

	C << sp;
	C << nl << "void" << nl << scoped.substr(2) << "::ice_unmarshal(const ::std::string& __name, "
	  << "const ::Ice::StreamPtr& __is, " << scoped << "Ptr& value)";
	C << sb;
	writeGenericMarshalUnmarshalCode(C, p->declaration(), "value", false, "__name");
	C << eb;

	if(!p->isAbstract())
	{
	    string name = fixKwd(p->name());
	    string factoryName = "__F__";
	    factoryName += name;
	    C << sp;
	    C << nl << "class " << factoryName << " : public ::Ice::ObjectFactory";
	    C << sb;
	    C.dec();
	    C << nl << "public:";
	    C.inc();
	    C << sp << nl << "virtual ::Ice::ObjectPtr" << nl << "create(const ::std::string& type)";
	    C << sb;
	    C << nl << "assert(type == " << scoped << "::ice_staticId());";
	    C << nl << "return new " << scoped << ";";
	    C << eb;
	    C << sp << nl << "virtual void" << nl << "destroy()";
	    C << sb;
	    C << eb;
	    C << eb << ";";
	    C << sp;
	    C << nl << "::Ice::ObjectFactoryPtr " << scoped.substr(2) << "::_factory = new "
	      << "__F__" << fixKwd(p->name()) << ";";
	    C << sp << nl << "const ::Ice::ObjectFactoryPtr&" << nl << scoped.substr(2) << "::ice_factory()";
	    C << sb;
	    C << nl << "return _factory;";
	    C << eb;
	}
    }

    
    H << eb << ';';

    if(p->isLocal())
    {
	C << sp;
	C << nl << "bool" << nl << scope.substr(2) << "operator==(const " << scoped
	  << "& l, const " << scoped << "& r)";
	C << sb;
	C << nl << "return static_cast<const ::Ice::LocalObject&>(l) == static_cast<const ::Ice::LocalObject&>(r);";
	C << eb;
	C << sp;
	C << nl << "bool" << nl << scope.substr(2) << "operator!=(const " << scoped
	  << "& l, const " << scoped << "& r)";
	C << sb;
	C << nl << "return static_cast<const ::Ice::LocalObject&>(l) != static_cast<const ::Ice::LocalObject&>(r);";
	C << eb;
	C << sp;
	C << nl << "bool" << nl << scope.substr(2) << "operator<(const " << scoped
	  << "& l, const " << scoped << "& r)";
	C << sb;
	C << nl << "return static_cast<const ::Ice::LocalObject&>(l) < static_cast<const ::Ice::LocalObject&>(r);";
	C << eb;
    }
    else
    {
	C << sp;
	C << nl << "bool" << nl << scope.substr(2) << "operator==(const " << scoped
	  << "& l, const " << scoped << "& r)";
	C << sb;
	C << nl << "return static_cast<const ::Ice::Object&>(l) == static_cast<const ::Ice::Object&>(r);";
	C << eb;
	C << sp;
	C << nl << "bool" << nl << scope.substr(2) << "operator!=(const " << scoped
	  << "& l, const " << scoped << "& r)";
	C << sb;
	C << nl << "return static_cast<const ::Ice::Object&>(l) != static_cast<const ::Ice::Object&>(r);";
	C << eb;
	C << sp;
	C << nl << "bool" << nl << scope.substr(2) << "operator<(const " << scoped
	  << "& l, const " << scoped << "& r)";
	C << sb;
	C << nl << "return static_cast<const ::Ice::Object&>(l) < static_cast<const ::Ice::Object&>(r);";
	C << eb;
    }
}

bool
Slice::Gen::ObjectVisitor::visitExceptionStart(const ExceptionPtr&)
{
    return false;
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
    string name = fixKwd(p->name());
    string scoped = fixKwd(p->scoped());
    string scope = fixKwd(p->scope());

    TypePtr ret = p->returnType();
    string retS = returnTypeToString(ret);

    string params = "(";
    string paramsDecl = "("; // With declarators
    string args = "(";

    TypeStringList inParams;
    TypeStringList outParams;
    ParamDeclList paramList = p->parameters();
    for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
	if(q != paramList.begin())
	{
	    params += ", ";
	    paramsDecl += ", ";
	    args += ", ";
	}

	string name = (*q)->name();
	TypePtr type = (*q)->type();
	bool isOutParam = (*q)->isOutParam();
	string typeString;
	if(isOutParam)
	{
	    outParams.push_back(make_pair(type, name));
	    typeString = outputTypeToString(type);
	}
	else
	{
	    inParams.push_back(make_pair(type, name));
	    typeString = inputTypeToString((*q)->type());
	}
	params += typeString;
	paramsDecl += typeString;
	paramsDecl += ' ';
	paramsDecl += fixKwd(name);
	args += fixKwd(name);
    }

    if(!cl->isLocal())
    {
	if(!inParams.empty() || !outParams.empty())
	{
	    params += ", ";
	    paramsDecl += ", ";
	    args += ", ";
	}
	params += "const ::Ice::Current& = ::Ice::Current())";
	paramsDecl += "const ::Ice::Current& __current)";
	args += "__current)";
    }
    else
    {
	params += ')';
	paramsDecl += ')';
	args += ')';
    }
    
    string exp2;
    if(_dllExport.size())
    {
	if(cl->hasDataMembers())
	{
	    exp2 = _dllExport;
	}
    }

    bool nonmutating = p->mode() == Operation::Nonmutating;

    H << sp;
    H << nl << exp2 << "virtual " << retS << ' ' << name << params << (nonmutating ? " const" : "") << " = 0;";

    if(!cl->isLocal())
    {
	ExceptionList throws = p->throws();
	throws.sort();
	throws.unique();

	H << nl << exp2 << "::IceInternal::DispatchStatus ___" << name
	  << "(::IceInternal::Incoming&, const ::Ice::Current&)" << (nonmutating ? " const" : "") << ";";
	C << sp;
	C << nl << "::IceInternal::DispatchStatus" << nl << scope.substr(2) << "___" << name
	  << "(::IceInternal::Incoming& __in, const ::Ice::Current& __current)" << (nonmutating ? " const" : "");
	C << sb;
	if(!inParams.empty())
	{
	    C << nl << "::IceInternal::BasicStream* __is = __in.is();";
	}
	if(ret || !outParams.empty() || !throws.empty())
	{
	    C << nl << "::IceInternal::BasicStream* __os = __in.os();";
	}
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
	{
	    C << retS << " __ret = ";
	}
	C << name << args << ';';
	writeMarshalCode(C, outParams, ret);
	if(!throws.empty())
	{
	    C << eb;
	    ExceptionList::const_iterator r;
	    for(r = throws.begin(); r != throws.end(); ++r)
	    {
		C << nl << "catch(const " << fixKwd((*r)->scoped()) << "& __ex)";
		C << sb;
		C << nl << "__os->write(__ex);";
		C << nl << "return ::IceInternal::DispatchUserException;";
		C << eb;
	    }
	}
	C << nl << "return ::IceInternal::DispatchOK;";
	C << eb;
    }	
}

void
Slice::Gen::ObjectVisitor::visitDataMember(const DataMemberPtr& p)
{
    string name = fixKwd(p->name());
    string s = typeToString(p->type());
    H << sp;
    H << nl << s << ' ' << name << ';';
}

void
Slice::Gen::ObjectVisitor::emitClassBase(const ClassDefPtr& base, const std::string& call)
{
    if(base)
    {
	C.zeroIndent();
	C << nl << "#ifdef _WIN32"; // COMPILERBUG
	C.restoreIndent();
	C << nl << fixKwd(base->name()) << "::" << call << ";";
	C.zeroIndent();
	C << nl << "#else";
	C.restoreIndent();
	C << nl << fixKwd(base->scoped()) << "::" << call << ";";
	C.zeroIndent();
	C << nl << "#endif";
	C.restoreIndent();
    }
    else
    {
	C.zeroIndent();
	C << nl << "#ifdef _WIN32"; // COMPILERBUG
	C.restoreIndent();
	C << nl << "Object::" << call << ";";
	C.zeroIndent();
	C << nl << "#else";
	C.restoreIndent();
	C << nl << "::Ice::Object::" << call << ";";
	C.zeroIndent();
	C << nl << "#endif";
	C.restoreIndent();
    }
}


Slice::Gen::IceInternalVisitor::IceInternalVisitor(Output& h, Output& c, const string& dllExport) :
    H(h), C(c), _dllExport(dllExport)
{
}

bool
Slice::Gen::IceInternalVisitor::visitUnitStart(const UnitPtr& p)
{
    if(!p->hasClassDecls())
    {
	return false;
    }

    H << sp;
    H << nl << "namespace IceInternal" << nl << '{';

    return true;
}

void
Slice::Gen::IceInternalVisitor::visitUnitEnd(const UnitPtr& p)
{
    H << sp;
    H << nl << '}';
}
    
void
Slice::Gen::IceInternalVisitor::visitClassDecl(const ClassDeclPtr& p)
{
    string scoped = fixKwd(p->scoped());
    
    H << sp;
    H << nl << _dllExport << "void incRef(" << scoped << "*);";
    H << nl << _dllExport << "void decRef(" << scoped << "*);";
    if(!p->isLocal())
    {
	H << sp;
	H << nl << _dllExport << "void incRef(::IceProxy" << scoped << "*);";
	H << nl << _dllExport << "void decRef(::IceProxy" << scoped << "*);";

	H << sp;
	H << nl << _dllExport << "void checkedCast(const ::Ice::ObjectPrx&, "
	  << "ProxyHandle< ::IceProxy" << scoped << ">&);";
	H << nl << _dllExport << "void checkedCast(const ::Ice::ObjectPrx&, const ::std::string&, "
	  << "ProxyHandle< ::IceProxy" << scoped << ">&);";
	H << nl << _dllExport << "void uncheckedCast(const ::Ice::ObjectPrx&, "
	  << "ProxyHandle< ::IceProxy" << scoped << ">&);";
	H << nl << _dllExport << "void uncheckedCast(const ::Ice::ObjectPrx&, const ::std::string&, "
	  << "ProxyHandle< ::IceProxy" << scoped << ">&);";
    }
}

bool
Slice::Gen::IceInternalVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    string scoped = fixKwd(p->scoped());
    string scope = fixKwd(p->scope());
    
    C << sp;
    C << nl << "void" << nl << "IceInternal::incRef(" << scoped << "* p)";
    C << sb;
    C << nl << "p->__incRef();";
    C << eb;

    C << sp;
    C << nl << "void" << nl << "IceInternal::decRef(" << scoped << "* p)";
    C << sb;
    C << nl << "p->__decRef();";
    C << eb;

    if(!p->isLocal())
    {
	C << sp;
	C << nl << "void" << nl << "IceInternal::incRef(::IceProxy" << scoped << "* p)";
	C << sb;
	C << nl << "p->__incRef();";
	C << eb;

	C << sp;
	C << nl << "void" << nl << "IceInternal::decRef(::IceProxy" << scoped << "* p)";
	C << sb;
	C << nl << "p->__decRef();";
	C << eb;

	C << sp;
	C << nl << "void" << nl << "IceInternal::checkedCast(const ::Ice::ObjectPrx& b, "
	  << scoped << "Prx& d)";
	C << sb;
	C << nl << "d = 0;";
	C << nl << "if(b)";
	C << sb;
	C << nl << "d = dynamic_cast< ::IceProxy" << scoped << "*>(b.get());";
	C << nl << "if(!d && b->ice_isA(\"" << p->scoped() << "\"))";
	C << sb;
	C << nl << "d = new ::IceProxy" << scoped << ";";
	C << nl << "d->__copyFrom(b);";
	C << eb;
	C << eb;
	C << eb;

	C << sp;
	C << nl << "void" << nl << "IceInternal::checkedCast(const ::Ice::ObjectPrx& b, const ::std::string& f, "
	  << scoped << "Prx& d)";
	C << sb;
	C << nl << "d = 0;";
	C << nl << "if(b)";
	C << sb;
	C << nl << "::Ice::ObjectPrx bb = b->ice_appendFacet(f);";
	C << nl << "try";
	C << sb;
	C << nl << "if(bb->ice_isA(\"" << p->scoped() << "\"))";
	C << sb;
	C << nl << "d = new ::IceProxy" << scoped << ";";
	C << nl << "d->__copyFrom(bb);";
	C << eb;
	C << eb;
	C << nl << "catch(const ::Ice::FacetNotExistException&)";
	C << sb;
	C << eb;
	C << eb;
	C << eb;

	C << sp;
	C << nl << "void" << nl << "IceInternal::uncheckedCast(const ::Ice::ObjectPrx& b, "
	  << scoped << "Prx& d)";
	C << sb;
	C << nl << "d = 0;";
	C << nl << "if(b)";
	C << sb;
	C << nl << "d = dynamic_cast< ::IceProxy" << scoped << "*>(b.get());";
	C << nl << "if(!d)";
	C << sb;
	C << nl << "d = new ::IceProxy" << scoped << ";";
	C << nl << "d->__copyFrom(b);";
	C << eb;
	C << eb;
	C << eb;

	C << sp;
	C << nl << "void" << nl << "IceInternal::uncheckedCast(const ::Ice::ObjectPrx& b, const ::std::string& f, "
	  << scoped << "Prx& d)";
	C << sb;
	C << nl << "d = 0;";
	C << nl << "if(b)";
	C << sb;
	C << nl << "::Ice::ObjectPrx bb = b->ice_appendFacet(f);";
	C << nl << "d = new ::IceProxy" << scoped << ";";
	C << nl << "d->__copyFrom(bb);";
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
    if(!p->hasClassDecls())
    {
	return false;
    }

    string name = fixKwd(p->name());
    
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
    string name = fixKwd(p->name());
    string scoped = fixKwd(p->scoped());
    
    H << sp;
    H << nl << "typedef ::IceInternal::Handle< " << scoped << "> " << name << "Ptr;";
    if(!p->isLocal())
    {
	H << nl << "typedef ::IceInternal::ProxyHandle< ::IceProxy" << scoped << "> " << name << "Prx;";

	H << sp;
	H << nl << _dllExport << "void __write(::IceInternal::BasicStream*, const " << name << "Prx&);";
	H << nl << _dllExport << "void __read(::IceInternal::BasicStream*, " << name << "Prx&);";

	H << sp;
	H << nl << _dllExport << "void ice_marshal(const ::std::string&, const ::Ice::StreamPtr&, const "
	  << name << "Prx&);";
	H << nl << _dllExport << "void ice_unmarshal(const ::std::string&, const ::Ice::StreamPtr&, "
	  << name << "Prx&);";
    }
}

bool
Slice::Gen::HandleVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    if(!p->isLocal())
    {
	string scoped = fixKwd(p->scoped());
	string scope = fixKwd(p->scope());

	C << sp;
	C << nl << "void" << nl << scope.substr(2) << "__write(::IceInternal::BasicStream* __os, const " << scoped
	  << "Prx& v)";
	C << sb;
	C << nl << "__os->write(::Ice::ObjectPrx(v));";
	C << eb;

	C << sp;
	C << nl << "void" << nl << scope.substr(2) << "__read(::IceInternal::BasicStream* __is, " << scoped
	  << "Prx& v)";
	C << sb;
	C << nl << "::Ice::ObjectPrx proxy;";
	C << nl << "__is->read(proxy);";
	C << nl << "if(!proxy)";
	C << sb;
	C << nl << "v = 0;";
	C << eb;
	C << nl << "else";
	C << sb;
	C << nl << "v = new ::IceProxy" << scoped << ';';
	C << nl << "v->__copyFrom(proxy);";
	C << eb;
	C << eb;

	C << sp;
	C << nl << "void" << nl << scope.substr(2)
          << "ice_marshal(const ::std::string& __name, const ::Ice::StreamPtr& __os, const " << scoped << "Prx& v)";
	C << sb;
	C << nl << "__os->writeProxy(__name, v);";
	C << eb;

	C << sp;
	C << nl << "void" << nl << scope.substr(2) << "ice_unmarshal(const ::std::string& __name, "
	  << "const ::Ice::StreamPtr& __is, " << scoped << "Prx& v)";
	C << sb;
	C << nl << "::Ice::ObjectPrx proxy = __is->readProxy(__name);";
	C << nl << "if(!proxy)";
	C << sb;
	C << nl << "v = 0;";
	C << eb;
	C << nl << "else";
	C << sb;
	C << nl << "v = new ::IceProxy" << scoped << ';';
	C << nl << "v->__copyFrom(proxy);";
	C << eb;
	C << eb;
    }

    return true;
}

Slice::Gen::ImplVisitor::ImplVisitor(Output& h, Output& c,
                                     const string& dllExport) :
    H(h), C(c), _dllExport(dllExport)
{
}

void
Slice::Gen::ImplVisitor::writeReturn(Output& out, const TypePtr& type)
{
    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if(builtin)
    {
        switch(builtin->kind())
        {
            case Builtin::KindBool:
            {
                out << nl << "return false;";
                break;
            }
            case Builtin::KindByte:
            case Builtin::KindShort:
            case Builtin::KindInt:
            case Builtin::KindLong:
            {
                out << nl << "return 0;";
                break;
            }
            case Builtin::KindFloat:
            case Builtin::KindDouble:
            {
                out << nl << "return 0.0;";
                break;
            }
            case Builtin::KindString:
            {
                out << nl << "return ::std::string();";
                break;
            }
            case Builtin::KindObject:
            case Builtin::KindObjectProxy:
            case Builtin::KindLocalObject:
            {
                out << nl << "return 0;";
                break;
            }
        }
        return;
    }

    ProxyPtr prx = ProxyPtr::dynamicCast(type);
    if(prx)
    {
        out << nl << "return 0;";
        return;
    }

    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
    if(cl)
    {
        out << nl << "return 0;";
        return;
    }

    StructPtr st = StructPtr::dynamicCast(type);
    if(st)
    {
        out << nl << "return " << fixKwd(st->scoped()) << "();";
        return;
    }

    EnumPtr en = EnumPtr::dynamicCast(type);
    if(en)
    {
        EnumeratorList enumerators = en->getEnumerators();
        out << nl << "return " << fixKwd(en->scope()) << fixKwd(enumerators.front()->name()) << ';';
        return;
    }

    SequencePtr seq = SequencePtr::dynamicCast(type);
    if(seq)
    {
        out << nl << "return " << fixKwd(seq->scoped()) << "();";
        return;
    }

    DictionaryPtr dict = DictionaryPtr::dynamicCast(type);
    assert(dict);
    out << nl << "return " << fixKwd(dict->scoped()) << "();";
}

bool
Slice::Gen::ImplVisitor::visitModuleStart(const ModulePtr& p)
{
    if(!p->hasClassDefs())
    {
        return false;
    }

    string name = fixKwd(p->name());
    
    H << sp << nl << "namespace " << name << nl << '{';

    return true;
}

void
Slice::Gen::ImplVisitor::visitModuleEnd(const ModulePtr& p)
{
    H << sp;
    H << nl << '}';
}

bool
Slice::Gen::ImplVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    if(!p->isAbstract())
    {
        return false;
    }

    string name = fixKwd(p->name());
    string scope = fixKwd(p->scope());
    string scoped = fixKwd(p->scoped());
    string cls = scope.substr(2) + name + "I";

    ClassList bases = p->bases();
    ClassDefPtr base;
    if(!bases.empty() && !bases.front()->isInterface())
    {
        base = bases.front();
    }

    H << sp;
    H << nl << "class " << name << "I : ";
    H.useCurrentPosAsIndent();
    H << "virtual public " << name;
    ClassList::const_iterator q;
    for(q = bases.begin(); q != bases.end(); ++q)
    {
        H << ',' << nl << "virtual public " << fixKwd((*q)->scoped()) << "I";
    }
    H.restoreIndent();

    H << sb;
    H.dec();
    H << nl << "public:";
    H.inc();

    OperationList ops = p->operations();
    OperationList::const_iterator r;

    //
    // Operations
    //
    for(r = ops.begin(); r != ops.end(); ++r)
    {
        OperationPtr op = (*r);
        string opName = fixKwd(op->name());

        TypePtr ret = op->returnType();
        string retS = returnTypeToString(ret);

        H << sp << nl << "virtual " << retS << ' ' << opName << '(';
        H.useCurrentPosAsIndent();
	ParamDeclList paramList = op->parameters();
	ParamDeclList::const_iterator q;
        for(q = paramList.begin(); q != paramList.end(); ++q)
        {
            if(q != paramList.begin())
            {
                H << ',' << nl;
            }
            string typeString = (*q)->isOutParam() ? outputTypeToString((*q)->type()) : inputTypeToString((*q)->type());
            H << typeString;
        }
        if(!p->isLocal())
        {
            if(!paramList.empty())
            {
                H << ',' << nl;
            }
            H << "const Ice::Current&";
        }
        H.restoreIndent();

	bool nonmutating = op->mode() == Operation::Nonmutating;

        H << ")" << (nonmutating ? " const" : "") << ";";

        C << sp << nl << retS << nl << scoped.substr(2) << "I::" << opName << '(';
        C.useCurrentPosAsIndent();
        for(q = paramList.begin(); q != paramList.end(); ++q)
        {
            if(q != paramList.begin())
            {
                C << ',' << nl;
            }
            string typeString = (*q)->isOutParam() ? outputTypeToString((*q)->type()) : inputTypeToString((*q)->type());
            C << typeString << ' ' << fixKwd((*q)->name());
        }
        if(!p->isLocal())
        {
            if(!paramList.empty())
            {
                C << ',' << nl;
            }
            C << "const Ice::Current& current";
        }
        C.restoreIndent();
        C << ")" << (nonmutating ? " const" : "");
        C << sb;

        //
        // Return value
        //
        if(ret)
        {
            writeReturn(C, ret);
        }

        C << eb;
    }

    H << eb << ';';

    return true;
}
