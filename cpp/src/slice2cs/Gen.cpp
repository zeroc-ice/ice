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

#include <IceUtil/Functional.h>
#include <Gen.h>
#include <limits>
#include <IceUtil/Algorithm.h>
#include <IceUtil/Iterator.h>

using namespace std;
using namespace Slice;

//
// Don't use "using namespace IceUtil", or stupid VC++ 6.0 complains
// about ambigious symbols for constructs like
// "IceUtil::constMemFun(&Slice::Exception::isLocal)".
//
using IceUtil::Output;
using IceUtil::nl;
using IceUtil::sp;
using IceUtil::sb;
using IceUtil::eb;

static string // Should be an anonymous namespace, but VC++ 6 can't handle that.
sliceModeToIceMode(const OperationPtr& op)
{
    string mode;
    switch(op->mode())
    {
	case Operation::Normal:
	{
	    mode = "Ice.OperationMode.Normal";
	    break;
	}
	case Operation::Nonmutating:
	{
	    mode = "Ice.OperationMode.Nonmutating";
	    break;
	}
	case Operation::Idempotent:
	{
	    mode = "Ice.OperationMode.Idempotent";
	    break;
	}
	default:
	{
	    assert(false);
	    break;
	}
    }
    return mode;
}

Slice::CsVisitor::CsVisitor(Output& out) : _out(out)
{
}

Slice::CsVisitor::~CsVisitor()
{
}

void
Slice::CsVisitor::writeInheritedOperations(const ClassDefPtr& p)
{
    ClassList bases = p->bases();
    if(!bases.empty() && !bases.front()->isInterface())
    {
	bases.pop_front();
    }
    if(!bases.empty())
    {
	_out << sp << nl << "#region Inherited Slice operations";

	OperationList allOps;
	for(ClassList::const_iterator q = bases.begin(); q != bases.end(); ++q)
	{
	    OperationList tmp = (*q)->allOperations();
	    allOps.splice(allOps.end(), tmp);
	}
	allOps.sort();
	allOps.unique();
	for(OperationList::const_iterator op = allOps.begin(); op != allOps.end(); ++op)
	{
	    string name = fixId((*op)->name());

	    _out << sp << nl << "public abstract " << typeToString((*op)->returnType()) << " " << name << "(";
	    string params = getParams(*op);
	    if(!p->isLocal())
	    {
		if(!params.empty())
		{
		    _out << ", ";
		}
		_out << "Ice.Current __current";
	    }
	    _out << ");";
	}

	_out << sp << nl << "#endregion"; // Inherited Slice operations
    }
}

void
Slice::CsVisitor::writeDispatch(const ClassDefPtr& p)
{
    _out << sp << nl << "#region Slice type-related members";

    _out << sp << nl << "public static new string[] __ids = ";
    _out << sb;

    string scoped = fixId(p->name());
    ClassList allBases = p->allBases();
    StringList ids;
    transform(allBases.begin(), allBases.end(), back_inserter(ids), ::IceUtil::constMemFun(&Contained::scoped));
    StringList other;
    other.push_back(scoped);
    other.push_back("::Ice::Object");
    other.sort();
    ids.merge(other);
    ids.unique();
    StringList::const_iterator firstIter = ids.begin();
    StringList::const_iterator scopedIter = find(ids.begin(), ids.end(), scoped);
    assert(scopedIter != ids.end());
    StringList::difference_type scopedPos = ice_distance(firstIter, scopedIter);
    
    StringList::const_iterator q = ids.begin();
    while(q != ids.end())
    {
        _out << nl << '"' << *q << '"';
	if(++q != ids.end())
	{
	    _out << ',';
	}
    }
    _out << eb << ";";

    _out << sp << nl << "private static readonly Ice.StringSeq __idSeq = new Ice.StringSeq(__ids);";

    _out << sp << nl << "public override bool ice_isA(string s, Ice.Current __current)";
    _out << sb;
    _out << nl << "foreach(string id in __ids)";
    _out << sb;
    _out << nl << "if(id.Equals(s))";
    _out << sb;
    _out << nl << "return true;";
    _out << eb;
    _out << eb;
    _out << nl << "return false;";
    _out << eb;

    _out << sp << nl << "public override StringSeq ice_ids(Ice.Current __current)";
    _out << sb;
    _out << nl << "return __idSeq;";
    _out << eb;

    _out << sp << nl << "public static new string ice_staticId()";
    _out << sb;
    _out << nl << "return __ids[" << scopedPos << "];";
    _out << eb;

    _out << sp << nl << "#endregion;"; // Slice type-related members
}

string
Slice::CsVisitor::getParams(const OperationPtr& op)
{
    string params;
    ParamDeclList paramList = op->parameters();
    for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
	if(q != paramList.begin())
	{
	    params += ", ";
	}
	if((*q)->isOutParam())
	{
	    params += "out ";
	}
	params += typeToString((*q)->type()) + " " + fixId((*q)->name());
    }
    return params;
}

string
Slice::CsVisitor::getArgs(const OperationPtr& op)
{
    string args;
    ParamDeclList paramList = op->parameters();
    for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
	if(q != paramList.begin())
	{
	    args += ", ";
	}
	args += fixId((*q)->name());
    }
    return args;
}

Slice::Gen::Gen(const string& name, const string& base, const vector<string>& includePaths) :
    _base(base),
    _includePaths(includePaths)
{
    string file = base + ".cs";
    _out.open(file.c_str());
    if(!_out)
    {
        cerr << name << ": can't open `" << file << "' for writing: " << strerror(errno) << endl;
	return;
    }
    printHeader();

    _out << nl << "// Generated from file `" << file << "'";

    _out << sp << nl << "using _System = System;";
    _out << nl << "using _Microsoft = Microsoft;";
}

Slice::Gen::~Gen()
{
}

bool
Slice::Gen::operator!() const
{
    return !_out;
}

void
Slice::Gen::generate(const UnitPtr& p)
{
    TypesVisitor typesVisitor(_out);
    p->visit(&typesVisitor);

    ProxyVisitor proxyVisitor(_out);
    p->visit(&proxyVisitor);

    OpsVisitor opsVisitor(_out);
    p->visit(&opsVisitor);

    HelperVisitor helperVisitor(_out);
    p->visit(&helperVisitor);

    DelegateVisitor delegateVisitor(_out);
    p->visit(&delegateVisitor);

    DelegateMVisitor delegateMVisitor(_out);
    p->visit(&delegateMVisitor);

    DelegateDVisitor delegateDVisitor(_out);
    p->visit(&delegateDVisitor);

    DispatcherVisitor dispatcherVisitor(_out);
    p->visit(&dispatcherVisitor);
}

void
Slice::Gen::printHeader()
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

    _out << header;
    _out << "\n// Ice version " << ICE_STRING_VERSION;
}

Slice::Gen::OpsVisitor::OpsVisitor(IceUtil::Output& out)
    : CsVisitor(out)
{
}

Slice::Gen::TypesVisitor::TypesVisitor(IceUtil::Output& out)
    : CsVisitor(out)
{
}

bool
Slice::Gen::TypesVisitor::visitModuleStart(const ModulePtr& p)
{
    string name = fixId(p->name());
    _out << sp << nl << "namespace " << name;
    _out << sb;

    return true;
}

void
Slice::Gen::TypesVisitor::visitModuleEnd(const ModulePtr& p)
{
    _out << eb;
}

bool
Slice::Gen::TypesVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    string name = fixId(p->name());
    string scoped = fixId(p->scoped());
    ClassList bases = p->bases();

    if(p->isInterface())
    {
        _out << sp << nl << "public interface " << name << " : ";
	_out.useCurrentPosAsIndent();
	if(p->isLocal())
	{
	    _out << "Ice.LocalObject";
	}
	else
	{
	    _out << "Ice.Object";
	}
	_out << ',' << nl << name << "_Operations";
	if(!bases.empty())
	{
	    ClassList::const_iterator q = bases.begin();
	    while(q != bases.end())
	    {
	        _out << ',' << nl << fixId((*q)->scoped());
		q++;
	    }
	}
	_out.restoreIndent();
    }
    else
    {
	_out << sp << nl << "public ";
	if(p->isAbstract())
	{
	    _out << "abstract ";
	}
	_out << "class " << name << " : ";

	_out.useCurrentPosAsIndent();
	if(bases.empty() || bases.front()->isInterface())
	{
	    if(p->isLocal())
	    {
		_out << "Ice.LocalObjectImpl";
	    }
	    else
	    {
		_out << "Ice.ObjectImpl";
	    }
	}
	else
	{
	    _out << fixId(bases.front()->scoped());
	    bases.pop_front();
	}
	if(p->isAbstract())
	{
	    _out << ',' << nl << name << "_Operations";
	}
	for(ClassList::const_iterator q = bases.begin(); q != bases.end(); ++q)
	{
	    if((*q)->isAbstract())
	    {
		_out << ',' << nl << fixId((*q)->scoped()) << "_Operations";
	    }
	}
	_out.restoreIndent();
    }

    _out << sb;

    if(!p->isInterface())
    {
	_out << sp << nl << "#region Slice data members and operations";
    }

    return true;
}

void
Slice::Gen::TypesVisitor::visitClassDefEnd(const ClassDefPtr& p)
{
    string name = fixId(p->name());
    DataMemberList dataMembers = p->dataMembers();
    DataMemberList::const_iterator q;


    if(!p->isInterface())
    {
	_out << sp << nl << "#endregion"; // Slice data members and operations

	writeInheritedOperations(p);


	if(!p->isInterface() && !p->isLocal())
	{
	    writeDispatch(p);

	    DataMemberList members = p->dataMembers();
	    DataMemberList::const_iterator d;
	    int iter;

	    _out << sp << nl << "#region Marshaling support";

	    _out << sp << nl << "public static void __write(IceInternal.BasicStream __os, bool __marshalFacets)";
	    _out << sb;
	    _out << nl << "__os.writeTypeId(ice_staticId());";
	    _out << nl << "__os.startWriteSlice();";
	    iter = 0;
	    for(d = members.begin(); d != members.end(); ++d)
	    {
		StringList metaData = (*d)->getMetaData();
		// TODO: writeMarshalUnmarshalCode(_out, package, (*d)->type(), fixId((*d)->name()), true, iter, false, metaData);
	    }
	    _out << nl << "__os.endWriteSlice();";
	    _out << nl << "base.__write(__os, __marshalFacets);";
	    _out << eb;

	    DataMemberList allClassMembers = p->allClassDataMembers();
	    if(allClassMembers.size() != 0)
	    {
		_out << sp << nl << "private class Patcher implements IceInternal.Patcher";
		_out << sb;
		if(allClassMembers.size() > 1)
		{
		    _out << sp << nl << "Patcher(int member)";
		    _out << sb;
		    _out << nl << "__member = member;";
		    _out << eb;
		}

		_out << sp << nl << "public void" << nl << "patch(Ice.Object v)";
		_out << sb;
		if(allClassMembers.size() > 1)
		{
		    _out << nl << "switch(__member)";
		    _out << sb;
		}
		int memberCount = 0;
		for(d = allClassMembers.begin(); d != allClassMembers.end(); ++d)
		{
		    if(allClassMembers.size() > 1)
		    {
			_out.dec();
			_out << nl << "case " << memberCount << ":";
			_out.inc();
		    }
		    if(allClassMembers.size() > 1)
		    {
			_out << nl << "__typeId = \"" << (*q)->type()->typeId() << "\";";
		    }
		    string memberName = fixId((*q)->name());
		    string memberType = typeToString((*q)->type());
		    _out << nl << memberName << " = (" << memberType << ")v;";
		    if(allClassMembers.size() > 1)
		    {
			_out << nl << "break;";
		    }
		    memberCount++;
		}
		if(allClassMembers.size() > 1)
		{
		    _out << eb;
		}
		_out << eb;

		_out << sp << nl << "public String" << nl << "type()";
		_out << sb;
		if(allClassMembers.size() > 1)
		{
		    _out << nl << "return __typeId;";
		}
		else
		{
		    _out << nl << "return \"" << (*allClassMembers.begin())->type()->typeId() << "\";";
		}
		_out << eb;

		if(allClassMembers.size() > 1)
		{
		    _out << sp << nl << "private int __member;";
		    _out << nl << "private String __typeId;";
		}
		_out << eb;
	    }

	    _out << sp << nl << "public void" << nl << "__read(IceInternal.BasicStream __is, bool __rid)";
	    _out << sb;
	    _out << nl << "if(__rid)";
	    _out << sb;
	    _out << nl << "string myId = __is.readTypeId();";
	    _out << eb;
	    _out << nl << "__is.startReadSlice();";
	    iter = 0;
	    DataMemberList classMembers = p->classDataMembers();
	    long classMemberCount = allClassMembers.size() - classMembers.size();
	    for(d = members.begin(); d != members.end(); ++d)
	    {
		StringList metaData = (*d)->getMetaData();
		ostringstream patchParams;
		BuiltinPtr builtin = BuiltinPtr::dynamicCast((*d)->type());
		if((builtin && builtin->kind() == Builtin::KindObject) || ClassDeclPtr::dynamicCast((*d)->type()))
		{
		    if(classMembers.size() > 1 || allClassMembers.size() > 1)
		    {
			patchParams << "new Patcher(" << classMemberCount++ << ')';
		    }
		}
		// TODO: writeMarshalUnmarshalCode(_out, package, (*d)->type(), fixId((*d)->name()), false, iter, false, metaData, patchParams.str());
	    }
	    _out << nl << "__is.endReadSlice();";
	    _out << nl << "base.__read(__is, true);";
	    _out << eb;

	    _out << sp << nl << "#endregion"; // Marshalling support
	}

	_out << sp << nl << "#region IComparable members";

	_out << sp << nl << "public override int CompareTo(object __other)";
	_out << sb;
	_out << nl << "if(__other == null)";
	_out << sb;
	_out << nl << "return 1;";
	_out << eb;
	_out << nl << "if(object.ReferenceEquals(this, __other))";
	_out << sb;
	_out << nl << "return 0;";
	_out << eb;
	_out << nl << "if(!(__other is " << name << "))";
	_out << sb;
	_out << nl << "throw new _System.ArgumentException(\"expected argument of type `"
	     << name << "'\", \"__other\");";
	_out << eb;
	_out << nl << "int __ret = base.CompareTo(__other);";
	_out << nl << "if(__ret != 0)";
	_out << sb;
	_out << nl << "return __ret;";
	_out << eb;
	for(q = dataMembers.begin(); q != dataMembers.end(); ++q)
	{
	    string memberName = fixId((*q)->name());
	    if(!isValueType((*q)->type()))
	    {
		_out << nl << "if(" << memberName << " == null && ((" << name << ")__other)."
		     << memberName << " != null)";
		_out << sb;
		_out << nl << "return -1;";
		_out << eb;
	    }
	    _out << nl << "if((__ret = " << memberName << ".CompareTo(((" << name << ")__other)."
		 << memberName << ")) != 0)";
	    _out << sb;
	    _out << nl << "return __ret;";
	    _out << eb;
	}
	_out << nl << "return 0;";

	_out << eb;

	_out << sp << nl << "#endregion"; // IComparable members

	_out << sp << nl << "#region Comparison operators";

	_out << sp << nl << "public static bool operator==(" << name << " __lhs, " << name << " __rhs)";
	_out << sb;
	_out << nl << "return Equals(__lhs, __rhs);";
	_out << eb;

	_out << sp << nl << "public static bool operator!=(" << name << " __lhs, " << name << " __rhs)";
	_out << sb;
	_out << nl << "return !Equals(__lhs, __rhs);";
	_out << eb;

	_out << sp << nl << "public static bool operator<(" << name << " __lhs, " << name << " __rhs)";
	_out << sb;
	_out << nl << "return __lhs == null ? __rhs != null : __lhs.CompareTo(__rhs) < 0;";
	_out << eb;

	_out << sp << nl << "public static bool operator>(" << name << " __lhs, " << name << " __rhs)";
	_out << sb;
	_out << nl << "return __lhs == null ? false : __lhs.CompareTo(__rhs) > 0;";
	_out << eb;

	_out << sp << nl << "public static bool operator<=(" << name << " __lhs, " << name << " __rhs)";
	_out << sb;
	_out << nl << "return __lhs == null ? true : __lhs.CompareTo(__rhs) <= 0;";
	_out << eb;

	_out << sp << nl << "public static bool operator>=(" << name << " __lhs, " << name << " __rhs)";
	_out << sb;
	_out << nl << "return __lhs == null ? __rhs == null : __lhs.CompareTo(__rhs) >= 0;";
	_out << eb;

	_out << sp << nl << "#endregion"; // Comparison operators

	_out << sp << nl << "#region Object members";

	_out << sp << nl << "public override int GetHashCode()";
	_out << sb;
	_out << nl << "int __h = 0;";
	for(q = dataMembers.begin(); q != dataMembers.end(); ++q)
	{
	    string memberName = fixId((*q)->name());
	    bool isValue = isValueType((*q)->type());
	    if(!isValue)
	    {
		_out << nl << "if(" << memberName << " != null)";
		_out << sb;
	    }
	    _out << nl << "__h = 5 * __h + " << memberName << ".GetHashCode();";
	    if(!isValue)
	    {
		_out << eb;
	    }
	}
	_out << nl << "return __h;";
	_out << eb;

	_out << sp << nl << "public override bool Equals(object other)";
	_out << sb;
	_out << nl << "return CompareTo(other) == 0;";
	_out << eb;

	_out << sp << nl << "public static bool Equals(" << name << " __lhs, " << name << " __rhs)";
	_out << sb;
	_out << nl << "return __lhs == null ? __rhs == null : __lhs.CompareTo(__rhs) == 0;";
	_out << eb;

	_out << sp << nl << "#endregion"; // Object members
    }

    _out << eb;
}

void
Slice::Gen::TypesVisitor::visitOperation(const OperationPtr& p)
{
    ClassDefPtr classDef = ClassDefPtr::dynamicCast(p->container());
    if(classDef->isInterface())
    {
        return;
    }

    bool isLocal = classDef->isLocal();

    string name = fixId(p->name());

    _out << sp << nl << "public ";
    if(isLocal)
    {
        _out << "abstract ";
    }
    _out << typeToString(p->returnType()) << " " << name << "(";
    ParamDeclList paramList = p->parameters();
    ParamDeclList::const_iterator q;
    for(q = paramList.begin(); q != paramList.end(); ++q)
    {
	if(q != paramList.begin())
	{
	    _out << ", ";
	}
	if((*q)->isOutParam())
	{
	    _out << "out ";
	}
	_out << typeToString((*q)->type()) << " " << fixId((*q)->name());
    }
    _out << ")";
    if(isLocal)
    {
        _out << ";";
    }
    else
    {
	_out << sb;
	_out << nl;
	if(p->returnType())
	{
	    _out << "return ";
	}
	_out << name << "(";
	for(q = paramList.begin(); q != paramList.end(); ++q)
	{
	    if(q != paramList.begin())
	    {
	        _out << ", ";
	    }
	    _out << fixId((*q)->name());
	}
	if(!paramList.empty())
	{
	    _out << ", ";
	}
	_out << "null);";
	_out << eb;
    }

    if(!isLocal)
    {
	_out << nl << "public abstract " << typeToString(p->returnType()) << " " << name << "(";
	ParamDeclList paramList = p->parameters();
	for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
	{
	    if(q != paramList.begin())
	    {
		_out << ", ";
	    }
	    if((*q)->isOutParam())
	    {
		_out << "out ";
	    }
	    _out << typeToString((*q)->type()) << " " << fixId((*q)->name());
	}
	if(!paramList.empty())
	{
	    _out << ", ";
	}
	_out << "Ice.Current __current);";
    }
}

void
Slice::Gen::TypesVisitor::visitSequence(const SequencePtr& p)
{
    string name = fixId(p->name());
    string s = typeToString(p->type());
    bool isValue = isValueType(p->type());

    _out << sp << nl << "public class " << name
         << " : _System.Collections.CollectionBase, _System.IComparable, _System.ICloneable";
    _out << sb;

    _out << sp << nl << "#region Constructors";

    _out << sp << nl << "public " << name << "() : base()";
    _out << sb;
    _out << eb;

    _out << sp << nl << "public " << name << "(" << s << "[] __s)";
    _out << sb;
    _out << nl << "InnerList.AddRange(__s);";
    _out << eb;

    _out << sp << nl << "#endregion"; // Constructors

    _out << sp << nl << "#region Array conversions";

    _out << sp << nl << "public void CopyTo(" << s << "[] __a, int __i)";
    _out << sb;
    _out << nl << "InnerList.CopyTo(__a, __i);";
    _out << eb;

    _out << sp << nl << "public " << s << "[] ToArray()";
    _out << sb;
    if(isValue)
    {
	_out << nl << s << "[] __a = new " << s << "[InnerList.Count];";
	_out << nl << "InnerList.CopyTo(__a, 0);";
	_out << nl << "return __a;";
    }
    else
    {
        _out << nl << "return (" << s << "[])InnerList.ToArray();";
    }
    _out << eb;

    _out << sp << nl << "public void FromArray(" << s << "[] __a)";
    _out << sb;
    _out << nl << "InnerList.Clear();";
    _out << nl << "InnerList.AddRange(__a);";
    _out << eb;

    _out << sp << nl << "#endregion"; // Array conversions

    _out << sp << nl << "#region Indexer";

    _out << sp << nl << "public " << s << " this[int index]";
    _out << sb;
    _out << nl << "get";
    _out << sb;
    _out << nl << "return (" << s << ")InnerList[index];";
    _out << eb;

    _out << nl << "set";
    _out << sb;
    _out << nl << "InnerList[index] = value;";
    _out << eb;
    _out << eb;

    _out << sp << nl << "#endregion"; // Indexer

    _out << sp << nl << "#region ICollectionBase members";

    _out << sp << nl << "public int Add(" << s << " value)";
    _out << sb;
    _out << nl << "return InnerList.Add(value);";
    _out << eb;

    _out << sp << nl << "public int IndexOf(" << s << " value)";
    _out << sb;
    _out << nl << "return InnerList.IndexOf(value);";
    _out << eb;

    _out << sp << nl << "public void Insert(int index, " << s << " value)";
    _out << sb;
    _out << nl << "InnerList.Insert(index, value);";
    _out << eb;

    _out << sp << nl << "public void Remove(" << s << " value)";
    _out << sb;
    _out << nl << "InnerList.Remove(value);";
    _out << eb;

    _out << sp << nl << "public bool Contains(" << s << " value)";
    _out << sb;
    _out << nl << "return InnerList.Contains(value);";
    _out << eb;

    _out << sp << nl << "#endregion"; // ICollectionBase members

    _out << sp << nl << "#region IComparable members";

    _out << sp << nl << "public int CompareTo(object __other)";
    _out << sb;
    _out << nl << "if(__other == null)";
    _out << sb;
    _out << nl << "return 1;";
    _out << eb;
    _out << nl << "if(object.ReferenceEquals(this, __other))";
    _out << sb;
    _out << nl << "return 0;";
    _out << eb;
    _out << nl << "if(!(__other is " << name << "))";
    _out << sb;
    _out << nl << "throw new _System.ArgumentException(\"CompareTo: expected argument of type `" << name
         << "'\", \"__other\");";
    _out << eb;
    _out << nl << "int __limit = _System.Math.Min(this.Count, ((" << name << ")__other).Count);";
    _out << nl << "for(int __i = 0; __i < __limit; ++__i)";
    _out << sb;
    if(!isValue)
    {
	_out << nl << "if(this[__i] == null && ((" << name << ")__other)[__i] != null)";
	_out << sb;
	_out << nl << "return -1;";
	_out << eb;
    }
    _out << nl << "int __ret = this[__i].CompareTo(((" << name << ")__other)[__i]);";
    _out << nl << "if(__ret != 0)";
    _out << sb;
    _out << nl << "return __ret;";
    _out << eb;
    _out << eb;
    _out << nl << "return this.Count < ((" << name << ")__other).Count ? -1 : ((this.Count > (("
         << name << ")__other).Count) ? 1 : 0);";
    _out << eb;

    _out << sp << nl << "#endregion"; // IComparable members

    _out << sp << nl << "#region Comparison operators";

    _out << sp << nl << "public static bool operator==(" << name << " __lhs, " << name << " __rhs)";
    _out << sb;
    _out << nl << "return Equals(__lhs, __rhs);";
    _out << eb;

    _out << sp << nl << "public static bool operator!=(" << name << " __lhs, " << name << " __rhs)";
    _out << sb;
    _out << nl << "return !Equals(__lhs, __rhs);";
    _out << eb;

    _out << sp << nl << "public static bool operator<(" << name << " __lhs, " << name << " __rhs)";
    _out << sb;
    _out << nl << "return __lhs == null ? __rhs != null : __lhs.CompareTo(__rhs) < 0;";
    _out << eb;

    _out << sp << nl << "public static bool operator>(" << name << " __lhs, " << name << " __rhs)";
    _out << sb;
    _out << nl << "return __lhs == null ? false : __lhs.CompareTo(__rhs) > 0;";
    _out << eb;

    _out << sp << nl << "public static bool operator<=(" << name << " __lhs, " << name << " __rhs)";
    _out << sb;
    _out << nl << "return __lhs == null ? true : __lhs.CompareTo(__rhs) <= 0;";
    _out << eb;

    _out << sp << nl << "public static bool operator>=(" << name << " __lhs, " << name << " __rhs)";
    _out << sb;
    _out << nl << "return __lhs == null ? __rhs == null : __lhs.CompareTo(__rhs) >= 0;";
    _out << eb;

    _out << sp << nl << "#endregion"; // Comparison operators

    _out << sp << nl << "#region ICloneable members";

    _out << sp << nl << "public object Clone()";
    _out << sb;
    _out << nl << name << " __ret = new "<< name << "();";
    _out << nl << "__ret.InnerList.AddRange(InnerList);";
    _out << nl << "return __ret;";
    _out << eb;

    _out << sp << nl << "#endregion"; // ICloneable members

    _out << sp << nl << "#region Object members";

    _out << sp << nl << "public override int GetHashCode()";
    _out << sb;
    _out << nl << "int hash = 0;";
    _out << nl << "for(int i = 0; i < Count; ++i)";
    _out << sb;
    if(!isValue)
    {
	_out << nl << "if((object)this[i] != null)";
	_out << sb;
    }
    _out << nl << "hash = 5 * hash + this[i].GetHashCode();";
    if(!isValue)
    {
	_out << eb;
    }
    _out << eb;
    _out << nl << "return hash;";
    _out << eb;

    _out << sp << nl << "public override bool Equals(object other)";
    _out << sb;
    _out << nl << "if(Count != ((" << name << ")other).Count)";
    _out << sb;
    _out << nl << "return false;";
    _out << eb;
    _out << nl << "return CompareTo(other) == 0;";
    _out << eb;

    _out << sp << nl << "public static bool Equals(" << name << " __lhs, " << name << " __rhs)";
    _out << sb;
    _out << nl << "return __lhs == null ? __rhs == null : __lhs.Equals(__rhs);";
    _out << eb;

    _out << sp << nl << "#endregion"; // Object members

    _out << eb;
}

bool
Slice::Gen::TypesVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    string name = fixId(p->name());
    ExceptionPtr base = p->base();

    _out << sp << nl << "public class " << name << " : ";
    if(base)
    {
        _out << fixId(base->scoped());
    }
    else
    {
        _out << (p->isLocal() ? "Ice.LocalException" : "Ice.UserException");
    }
    _out << sb;

    _out << sp << nl << "#region Slice data members";

    return true;
}

void
Slice::Gen::TypesVisitor::visitExceptionEnd(const ExceptionPtr& p)
{
    string name = fixId(p->name());

    DataMemberList dataMembers = p->dataMembers();
    DataMemberList::const_iterator q;

    _out << sp << nl << "#endregion"; // Slice data members

    _out << sp << nl << "#region Constructors";

    _out << sp << nl << "public " << name << "() : base(\"" << name << "\")";
    _out << sb;
    _out << eb;

    _out << sp << nl << "public " << name << "(string __m) : base(__m)";
    _out << sb;
    _out << eb;

    _out << sp << nl << "private static readonly string _dflt = \"" << name << "\";";
    _out << nl << "public " << name << "(_System.Exception __ex) : base(_dflt, __ex)";
    _out << sb;
    _out << eb;

    _out << sp << nl << "public " << name << "(string __m, _System.Exception __ex) : base(__m, __ex)";
    _out << sb;
    _out << eb;

    _out << sp << nl << "#endregion"; // Constructors

    if(!p->isLocal())
    {
        _out << sp << nl << "#region Marshaling support";

        string scoped = p->scoped();
        ExceptionPtr base = p->base();

        int iter;

        _out << sp << nl << "public override void __write(IceInternal.BasicStream __os)";
        _out << sb;
	_out << nl << "__os.writeString(\"" << scoped << "\");";
	_out << nl << "__os.startWriteSlice();";
        iter = 0;
        for(q = dataMembers.begin(); q != dataMembers.end(); ++q)
        {
            // TODO: writeMarshalUnmarshalCode(_out, package, (*d)->type(), fixId((*d)->name()), true, iter, false, metaData);
        }
	_out << nl << "__os.endWriteSlice();";
        if(base)
        {
            _out << nl << "base.__write(__os);";
        }
        _out << eb;

	DataMemberList allClassMembers = p->allClassDataMembers();
	if(allClassMembers.size() != 0)
	{
	    _out << sp << nl << "private class Patcher implements IceInternal.Patcher";
	    _out << sb;
	    if(allClassMembers.size() > 1)
	    {
		_out << sp << nl << "Patcher(int member)";
		_out << sb;
		_out << nl << "__member = member;";
		_out << eb;
	    }

	    _out << sp << nl << "public void" << nl << "patch(Ice.Object v)";
	    _out << sb;
	    if(allClassMembers.size() > 1)
	    {
		_out << nl << "switch(__member)";
		_out << sb;
	    }
	    int memberCount = 0;
	    for(q = allClassMembers.begin(); q != allClassMembers.end(); ++q)
	    {
		if(allClassMembers.size() > 1)
		{
		    _out.dec();
		    _out << nl << "case " << memberCount << ":";
		    _out.inc();
		}
		if(allClassMembers.size() > 1)
		{
		    _out << nl << "__typeId = \"" << (*q)->type()->typeId() << "\";";
		}
		string memberName = fixId((*q)->name());
		string memberType = typeToString((*q)->type());
		_out << nl << memberName << " = (" << memberType << ")v;";
		if(allClassMembers.size() > 1)
		{
		    _out << nl << "break;";
		}
		memberCount++;
	    }
	    if(allClassMembers.size() > 1)
	    {
		_out << eb;
	    }
	    _out << eb;

	    _out << sp << nl << "public string type()";
	    _out << sb;
	    if(allClassMembers.size() > 1)
	    {
		_out << nl << "return __typeId;";
	    }
	    else
	    {
		_out << nl << "return \"" << (*allClassMembers.begin())->type()->typeId() << "\";";
	    }
	    _out << eb;
	    if(allClassMembers.size() > 1)
	    {
		_out << sp << nl << "private int __member;";
		_out << nl << "private String __typeId;";
	    }
	    _out << eb;
	}
        _out << sp << nl << "public override void __read(IceInternal.BasicStream __is, bool __rid)";
        _out << sb;
	_out << nl << "if(__rid)";
	_out << sb;
	_out << nl << "string myId = __is.readString();";
	_out << eb;
	_out << nl << "__is.startReadSlice();";
        iter = 0;
	DataMemberList classMembers = p->classDataMembers();
	long classMemberCount = allClassMembers.size() - classMembers.size();
        for(q = dataMembers.begin(); q != dataMembers.end(); ++q)
        {
	    ostringstream patchParams;
	    BuiltinPtr builtin = BuiltinPtr::dynamicCast((*q)->type());
	    if((builtin && builtin->kind() == Builtin::KindObject) || ClassDeclPtr::dynamicCast((*q)->type()))
	    {
		if(classMembers.size() > 1 || allClassMembers.size() > 1)
		{
		    patchParams << "new Patcher(" << classMemberCount++ << ')';
		}
	    }
            // TODO: writeMarshalUnmarshalCode(_out, package, (*d)->type(), fixId((*d)->name()), false, iter, false, metaData, patchParams.str());
        }
	_out << nl << "__is.endReadSlice();";
        if(base)
        {
            _out << nl << "base.__read(__is, true);";
        }
        _out << eb;

	if(!base || base && !base->usesClasses())
	{
	    _out << sp << nl << "public override bool __usesClasses()";
	    _out << sb;
	    _out << nl << "return true;";
	    _out << eb;
	}

        _out << sp << nl << "#endregion"; // Marshalling support
    }

    _out << sp << nl << "#region Comparison operators";

    _out << sp << nl << "public static bool operator==(" << name << " __lhs, " << name << " __rhs)";
    _out << sb;
    _out << nl << "return Equals(__lhs, __rhs);";
    _out << eb;

    _out << sp << nl << "public static bool operator!=(" << name << " __lhs, " << name << " __rhs)";
    _out << sb;
    _out << nl << "return !Equals(__lhs, __rhs);";
    _out << eb;

    _out << sp << nl << "#endregion"; // Comparison operators

    _out << sp << nl << "#region Object members";

    _out << sp << nl << "public override int GetHashCode()";
    _out << sb;
    _out << nl << "int __h = 0;";
    for(q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        string memberName = fixId((*q)->name());
	bool isValue = isValueType((*q)->type());
	if(!isValue)
	{
	    _out << nl << "if((object)" << memberName << " != null)";
	    _out << sb;
	}
	_out << nl << "__h = 5 * __h + " << memberName << ".GetHashCode();";
	if(!isValue)
	{
	    _out << eb;
	}
    }
    _out << nl << "return __h;";
    _out << eb;

    _out << sp << nl << "public override bool Equals(object __other)";
    _out << sb;
    _out << nl << "if(__other == null)";
    _out << sb;
    _out << nl << "return false;";
    _out << eb;
    _out << nl << "if(object.ReferenceEquals(this, __other))";
    _out << sb;
    _out << nl << "return true;";
    _out << eb;
    _out << nl << "if(!(__other is " << name << "))";
    _out << sb;
    _out << nl << "throw new _System.ArgumentException(\"expected argument of type `" << name << "'\", \"__other\");";
    _out << eb;
    for(q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        string memberName = fixId((*q)->name());
	_out << nl << "if(!" << memberName << ".Equals(((" << name << ")__other)." << memberName << "))";
	_out << sb;
	_out << nl << "return false;";
	_out << eb;
    }
    _out << nl << "return true;";
    _out << eb;

    _out << sp << nl << "public static bool Equals(" << name << " __lhs, " << name << " __rhs)";
    _out << sb;
    _out << nl << "return __lhs == null ? __rhs == null : __lhs.Equals(__rhs);";
    _out << eb;

    _out << sp << nl << "#endregion"; // Object members

    _out << eb;
}

bool
Slice::Gen::TypesVisitor::visitStructStart(const StructPtr& p)
{
    string name = fixId(p->name());

    _out << sp << nl << "public class " << name << " : _System.IComparable";
    _out << sb;

    _out << sp << nl << "#region Slice data members";

    return true;
}

void
Slice::Gen::TypesVisitor::visitStructEnd(const StructPtr& p)
{
    string name = fixId(p->name());

    DataMemberList dataMembers = p->dataMembers();
    DataMemberList::const_iterator q;
    int iter;

    _out << sp << nl << "#endregion"; // Slice data members

    if(!p->isLocal())
    {
        _out << sp << nl << "#region Marshalling support";

        _out << sp << nl << "public void __write(IceInternal.BasicStream __os)";
        _out << sb;
	// TODO
        _out << eb;

	DataMemberList classMembers = p->classDataMembers();

	if(classMembers.size() != 0)
	{
	    _out << sp << nl << "private class Patcher implements IceInternal.Patcher";
	    _out << sb;
	    if(classMembers.size() > 1)
	    {
		_out << sp << nl << "Patcher(int member)";
		_out << sb;
		_out << nl << "__member = member;";
		_out << eb;
	    }

	    _out << sp << nl << "public void" << nl << "patch(Ice.Object v)";
	    _out << sb;
	    if(classMembers.size() > 1)
	    {
		_out << nl << "switch(__member)";
		_out << sb;
	    }
	    int memberCount = 0;
	    for(q = classMembers.begin(); q != classMembers.end(); ++q)
	    {
		if(classMembers.size() > 1)
		{
		    _out.dec();
		    _out << nl << "case " << memberCount << ":";
		    _out.inc();
		}
		if(classMembers.size() > 1)
		{
		    _out << nl << "__typeId = \"" << (*q)->type()->typeId() << "\";";
		}
		string memberName = fixId((*q)->name());
		string memberType = typeToString((*q)->type());
		_out << nl << memberName << " = (" << memberType << ")v;";
		if(classMembers.size() > 1)
		{
		    _out << nl << "break;";
		}
		memberCount++;
	    }
	    if(classMembers.size() > 1)
	    {
		_out << eb;
	    }
	    _out << eb;

	    _out << sp << nl << "public string type()";
	    _out << sb;
	    if(classMembers.size() > 1)
	    {
		_out << nl << "return __typeId;";
	    }
	    else
	    {
		_out << nl << "return \"" << (*classMembers.begin())->type()->typeId() << "\";";
	    }
	    _out << eb;

	    if(classMembers.size() > 1)
	    {
		_out << sp << nl << "private int __member;";
		_out << nl << "private string __typeId;";
	    }
	    _out << eb;
	}

        _out << sp << nl << "public void __read(IceInternal.BasicStream __is)";
        _out << sb;
        iter = 0;
	int classMemberCount = 0;
        for(q = dataMembers.begin(); q != dataMembers.end(); ++q)
        {
	    ostringstream patchParams;
	    BuiltinPtr builtin = BuiltinPtr::dynamicCast((*q)->type());
	    if((builtin && builtin->kind() == Builtin::KindObject) || ClassDeclPtr::dynamicCast((*q)->type()))
	    {
		if(classMembers.size() > 1)
		{
		    patchParams << "new Patcher(" << classMemberCount++ << ')';
		}
	    }
            // TODO: writeMarshalUnmarshalCode(_out, package, (*d)->type(), fixId((*d)->name()), false, iter, false, metaData, patchParams.str());
        }
        _out << eb;

        _out << sp << nl << "#endregion"; // Marshalling support
    }

    _out << sp << nl << "#region IComparable members";

    _out << sp << nl << "public int CompareTo(object __other)";
    _out << sb;
    _out << nl << "if(__other == null)";
    _out << sb;
    _out << nl << "return 1;";
    _out << eb;
    _out << nl << "if(object.ReferenceEquals(this, __other))";
    _out << sb;
    _out << nl << "return 0;";
    _out << eb;
    _out << nl << "if(!(__other is " << name << "))";
    _out << sb;
    _out << nl << "throw new _System.ArgumentException(\"expected argument of type `" << name << "'\", \"__other\");";
    _out << eb;
    _out << nl << "int __ret;";
    for(q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        string memberName = fixId((*q)->name());
	if(!isValueType((*q)->type()))
	{
	    _out << nl << "if(" << memberName << " == null && ((" << name << ")__other)." << memberName << " != null)";
	    _out << sb;
	    _out << nl << "return -1;";
	    _out << eb;
	}
	_out << nl << "if((__ret = " << memberName << ".CompareTo(((" << name << ")__other)."
	     << memberName << ")) != 0)";
	_out << sb;
	_out << nl << "return __ret;";
	_out << eb;
    }
    _out << nl << "return 0;";
    _out << eb;

    _out << sp << nl << "#endregion"; // IComparable members

    _out << sp << nl << "#region Comparison operators";

    _out << sp << nl << "public static bool operator==(" << name << " __lhs, " << name << " __rhs)";
    _out << sb;
    _out << nl << "return Equals(__lhs, __rhs);";
    _out << eb;

    _out << sp << nl << "public static bool operator!=(" << name << " __lhs, " << name << " __rhs)";
    _out << sb;
    _out << nl << "return !Equals(__lhs, __rhs);";
    _out << eb;

    _out << sp << nl << "public static bool operator<(" << name << " __lhs, " << name << " __rhs)";
    _out << sb;
    _out << nl << "return __lhs == null ? __rhs != null : __lhs.CompareTo(__rhs) < 0;";
    _out << eb;

    _out << sp << nl << "public static bool operator>(" << name << " __lhs, " << name << " __rhs)";
    _out << sb;
    _out << nl << "return __lhs == null ? false : __lhs.CompareTo(__rhs) > 0;";
    _out << eb;

    _out << sp << nl << "public static bool operator<=(" << name << " __lhs, " << name << " __rhs)";
    _out << sb;
    _out << nl << "return __lhs == null ? true : __lhs.CompareTo(__rhs) <= 0;";
    _out << eb;

    _out << sp << nl << "public static bool operator>=(" << name << " __lhs, " << name << " __rhs)";
    _out << sb;
    _out << nl << "return __lhs == null ? __rhs == null : __lhs.CompareTo(__rhs) >= 0;";
    _out << eb;

    _out << sp << nl << "#endregion"; // Comparison operators

    _out << sp << nl << "#region Object members";

    _out << sp << nl << "public override int GetHashCode()";
    _out << sb;
    _out << nl << "int __h = 0;";
    for(q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        string memberName = fixId((*q)->name());
	bool isValue = isValueType((*q)->type());
	if(!isValue)
	{
	    _out << nl << "if(" << memberName << " != null)";
	    _out << sb;
	}
	_out << nl << "__h = 5 * __h + " << memberName << ".GetHashCode();";
	if(!isValue)
	{
	    _out << eb;
	}
    }
    _out << nl << "return __h;";
    _out << eb;

    _out << sp << nl << "public override bool Equals(object other)";
    _out << sb;
    _out << nl << "return CompareTo(other) == 0;";
    _out << eb;

    _out << sp << nl << "public static bool Equals(" << name << " __lhs, " << name << " __rhs)";
    _out << sb;
    _out << nl << "return __lhs == null ? __rhs == null : __lhs.CompareTo(__rhs) == 0;";
    _out << eb;

    _out << sp << nl << "#endregion"; // Object members

    _out << eb;
}

void
Slice::Gen::TypesVisitor::visitDictionary(const DictionaryPtr& p)
{
    string name = fixId(p->name());
    string ks = typeToString(p->keyType());
    string vs = typeToString(p->valueType());
    bool valueIsValue = isValueType(p->valueType());

    _out << sp << nl << "public class " << name
         << " : _System.Collections.DictionaryBase, _System.IComparable, _System.ICloneable";
    _out << sb;

    _out << sp << nl << "#region " << name << " properties";

    _out << sp << nl << "#region Indexer";

    _out << nl << "public " << vs << " this[" << ks << " key]";
    _out << sb;
    _out << nl << "get";
    _out << sb;
    _out << nl << "return (" << vs << ")Dictionary[key];";
    _out << eb;

    _out << nl << "set";
    _out << sb;
    _out << nl << "Dictionary[key] = value;";
    _out << eb;
    _out << eb;

    _out << sp << nl << "#endregion"; // Indexer

    _out << sp << nl << "public _System.Collections.ICollection Keys";
    _out << sb;
    _out << nl << "get";
    _out << sb;
    _out << nl << "return Dictionary.Keys;";
    _out << eb;
    _out << eb;

    _out << sp << nl << "public _System.Collections.ICollection Values";
    _out << sb;
    _out << nl << "get";
    _out << sb;
    _out << nl << "return Dictionary.Values;";
    _out << eb;
    _out << eb;

    _out << sp << nl << "#endregion"; // properties

    _out << sp << nl << "#region IDictionary members";

    _out << sp << nl << "public void Add(" << ks << " key, " << vs << " value)";
    _out << sb;
    _out << nl << "Dictionary[key] = value;";
    _out << eb;

    _out << sp << nl << "public void Remove(" << ks << " key)";
    _out << sb;
    _out << nl << "Dictionary.Remove(key);";
    _out << eb;

    _out << sp << nl << "public bool Contains(" << ks << " key)";
    _out << sb;
    _out << nl << "return Dictionary.Contains(key);";
    _out << eb;

    _out << sp << nl << "#endregion"; // IDictionary members

    _out << sp << nl << "#region IComparable members";

    _out << sp << nl << "public int CompareTo(object __other)";
    _out << sb;
    _out << nl << "if(__other == null)";
    _out << sb;
    _out << nl << "return 1;";
    _out << eb;
    _out << nl << "if(object.ReferenceEquals(this, __other))";
    _out << sb;
    _out << nl << "return 0;";
    _out << eb;
    _out << nl << "if(!(__other is " << name << "))";
    _out << sb;
    _out << nl << "throw new _System.ArgumentException(\"CompareTo: expected argument of type `" << name
         << "'\", \"__other\");";
    _out << eb;
    _out << nl << ks << "[] __klhs = new " << ks << "[Count];";
    _out << nl << "Keys.CopyTo(__klhs, 0);";
    _out << nl << "_System.Array.Sort(__klhs);";
    _out << nl << ks << "[] __krhs = new " << ks << "[((" << name << ")__other).Count];";
    _out << nl << "((" << name << ")__other).Keys.CopyTo(__krhs, 0);";
    _out << nl << "_System.Array.Sort(__krhs);";
    _out << nl << "int __limit = _System.Math.Min(__klhs.Length, __krhs.Length);";
    _out << nl << "for(int i = 0; i < __limit; ++i)";
    _out << sb;
    _out << nl << "int ret = __klhs[i].CompareTo(__krhs[i]);";
    _out << nl << "if(ret != 0)";
    _out << sb;
    _out << nl << "return ret;";
    _out << eb;
    _out << eb;
    _out << nl << vs << "[] __vlhs = new " << vs << "[Count];";
    _out << nl << "Values.CopyTo(__vlhs, 0);";
    _out << nl << "_System.Array.Sort(__vlhs);";
    _out << nl << vs << "[] __vrhs = new " << vs << "[((" << name << ")__other).Count];";
    _out << nl << "((" << name << ")__other).Values.CopyTo(__vrhs, 0);";
    _out << nl << "_System.Array.Sort(__vrhs);";
    _out << nl << "for(int i = 0; i < __limit; ++i)";
    _out << sb;
    if(!valueIsValue)
    {
	_out << nl << "if(__vlhs[i] == null && __vrhs[i] != null)";
	_out << sb;
	_out << nl << "return -1;";
	_out << eb;
    }
    _out << nl << "int ret = __vlhs[i].CompareTo(__vrhs[i]);";
    _out << nl << "if(ret != 0)";
    _out << sb;
    _out << nl << "return ret;";
    _out << eb;
    _out << eb;
    _out << nl << "return __klhs.Length < __krhs.Length ? -1 : (__klhs.Length > __krhs.Length ? 1 : 0);";
    _out << eb;

    _out << sp << nl << "#endregion"; // IComparable members

    _out << sp << nl << "#region Comparison operators";

    _out << sp << nl << "public static bool operator==(" << name << " __lhs, " << name << " __rhs)";
    _out << sb;
    _out << nl << "return Equals(__lhs, __rhs);";
    _out << eb;

    _out << sp << nl << "public static bool operator!=(" << name << " __lhs, " << name << " __rhs)";
    _out << sb;
    _out << nl << "return !Equals(__lhs, __rhs);";
    _out << eb;

    _out << sp << nl << "public static bool operator<(" << name << " __lhs, " << name << " __rhs)";
    _out << sb;
    _out << nl << "return __lhs == null ? __rhs != null : __lhs.CompareTo(__rhs) < 0;";
    _out << eb;

    _out << sp << nl << "public static bool operator>(" << name << " __lhs, " << name << " __rhs)";
    _out << sb;
    _out << nl << "return __lhs == null ? false : __lhs.CompareTo(__rhs) > 0;";
    _out << eb;

    _out << sp << nl << "public static bool operator<=(" << name << " __lhs, " << name << " __rhs)";
    _out << sb;
    _out << nl << "return __lhs == null ? true : __lhs.CompareTo(__rhs) <= 0;";
    _out << eb;

    _out << sp << nl << "public static bool operator>=(" << name << " __lhs, " << name << " __rhs)";
    _out << sb;
    _out << nl << "return __lhs == null ? __rhs == null : __lhs.CompareTo(__rhs) >= 0;";
    _out << eb;

    _out << sp << nl << "#endregion"; // Comparison operators

    _out << sp << nl << "#region ICloneable members";

    _out << sp << nl << "public object Clone()";
    _out << sb;
    _out << nl << name << " __ret = new " << name << "();";
    _out << nl << "foreach(System.Collections.DictionaryEntry i in Dictionary)";
    _out << sb;
    _out << nl << "__ret[(" << ks << ")i.Key] = (" << vs << ")i.Value;";
    _out << eb;
    _out << nl << "return __ret;";
    _out << eb;

    _out << sp << nl << "#endregion"; // ICloneable members

    _out << sp << nl << "#region Object members";

    _out << sp << nl << "public override int GetHashCode()";
    _out << sb;
    _out << nl << "int hash = 0;";
    _out << nl << "foreach(_System.Collections.DictionaryEntry e in Dictionary)";
    _out << sb;
    _out << nl << "hash = 5 * hash + e.Key.GetHashCode();";
    if(!valueIsValue)
    {
	_out << nl << "if(e.Value != null)";
	_out << sb;
    }
    _out << nl << "hash = 5 * hash + e.Value.GetHashCode();";
    if(!valueIsValue)
    {
	_out << eb;
    }
    _out << eb;
    _out << nl << "return hash;";
    _out << eb;

    _out << sp << nl << "public override bool Equals(object other)";
    _out << sb;
    _out << nl << "if(Count != ((" << name << ")other).Count)";
    _out << sb;
    _out << nl << "return false;";
    _out << eb;
    _out << nl << "return CompareTo(other) == 0;";
    _out << eb;

    _out << sp << nl << "public static bool Equals(" << name << " __lhs, " << name << " __rhs)";
    _out << sb;
    _out << nl << "return __lhs == null ? __rhs == null : __lhs.Equals(__rhs);";
    _out << eb;

    _out << sp << nl << "#endregion"; // Object members

    _out << eb;
}

void
Slice::Gen::TypesVisitor::visitEnum(const EnumPtr& p)
{
    string name = fixId(p->name());
    EnumeratorList enumerators = p->getEnumerators();
    _out << sp << nl << "public enum " << name;
    _out << sb;
    EnumeratorList::const_iterator en = enumerators.begin();
    while(en != enumerators.end())
    {
        _out << nl << fixId((*en)->name());
	if(++en != enumerators.end())
	{
	    _out << ',';
	}
    }
    _out << eb;
}

void
Slice::Gen::TypesVisitor::visitConst(const ConstPtr& p)
{
    string name = fixId(p->name());
    _out << sp << nl << "public class " << name;
    _out << sb;
    _out << sp << nl << "public const " << typeToString(p->type()) << " value = ";
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

	_out << "\"";					 // Opening "

	ios_base::fmtflags originalFlags = _out.flags(); // Save stream state
	streamsize originalWidth = _out.width();
	ostream::char_type originalFill = _out.fill();

	const string val = p->value();
	for(string::const_iterator c = val.begin(); c != val.end(); ++c)
	{
	    if(charSet.find(*c) == charSet.end())
	    {
		unsigned char uc = *c;			 // char may be signed, so make it positive
		_out << "\\u";    			 // Print as unicode if not in basic source character set
		_out.flags(ios_base::hex);
		_out.width(4);
		_out.fill('0');
		_out << static_cast<unsigned>(uc);
	    }
	    else
	    {
		_out << *c;				 // Print normally if in basic source character set
	    }
	}

	_out.fill(originalFill);			 // Restore stream state
	_out.width(originalWidth);
	_out.flags(originalFlags);

	_out << "\"";					 // Closing "
    }
    else if(bp && bp->kind() == Builtin::KindLong)
    {
	_out << p->value() << "L";
    }
    else if(bp && bp->kind() == Builtin::KindFloat)
    {
	_out << p->value() << "F";
    }
    else
    {
	EnumPtr ep = EnumPtr::dynamicCast(p->type());
	if(ep)
	{
	    _out << fixId(typeToString(p->type())) << "." << fixId(p->value());
	}
	else
	{
	    _out << p->value();
	}
    }
    _out << ";";
    _out << eb;
}

void
Slice::Gen::TypesVisitor::visitDataMember(const DataMemberPtr& p)
{
    _out << sp << nl << "public " << typeToString(p->type()) << " " << fixId(p->name()) << ";";
}

Slice::Gen::ProxyVisitor::ProxyVisitor(IceUtil::Output& out)
    : CsVisitor(out)
{
}

bool
Slice::Gen::ProxyVisitor::visitModuleStart(const ModulePtr& p)
{
    string name = fixId(p->name());
    _out << sp << nl << "namespace " << name;
    _out << sb;

    return true;
}

void
Slice::Gen::ProxyVisitor::visitModuleEnd(const ModulePtr& p)
{
    _out << eb;
}

bool
Slice::Gen::ProxyVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    if(p->isLocal())
    {
	return false;
    }

    string name = fixId(p->name());
    string scoped = fixId(p->scoped());
    ClassList bases = p->bases();

    _out << sp << nl << "public interface " << name << "Prx : ";
    if(bases.empty())
    {
        _out << "Ice.ObjectPrx";
    }
    else
    {
        _out.useCurrentPosAsIndent();
	ClassList::const_iterator q = bases.begin();
	while(q != bases.end())
	{
	    _out << fixId((*q)->scoped()) << "Prx";
	    if(++q != bases.end())
	    {
		_out << ',' << nl;
	    }
	}
	_out.restoreIndent();
    }

    _out << sb;

    return true;
}

void
Slice::Gen::ProxyVisitor::visitClassDefEnd(const ClassDefPtr& p)
{

    _out << eb;
}

void
Slice::Gen::ProxyVisitor::visitOperation(const OperationPtr& p)
{
    string name = fixId(p->name());
    ParamDeclList paramList = p->parameters();
    ParamDeclList::const_iterator q;

    _out << sp << nl << typeToString(p->returnType()) << " " << name << "(";
    for(q = paramList.begin(); q != paramList.end(); ++q)
    {
	if(q != paramList.begin())
	{
	    _out << ", ";
	}
	if((*q)->isOutParam())
	{
	    _out << "out ";
	}
	_out << typeToString((*q)->type()) << " " << fixId((*q)->name());
    }
    _out << ");";

    _out << nl << typeToString(p->returnType()) << " " << name << "(";
    for(q = paramList.begin(); q != paramList.end(); ++q)
    {
	if(q != paramList.begin())
	{
	    _out << ", ";
	}
	if((*q)->isOutParam())
	{
	    _out << "out ";
	}
	_out << typeToString((*q)->type()) << " " << fixId((*q)->name());
    }
    if(!paramList.empty())
    {
        _out << ", ";
    }
    _out << "Ice.Context __context);";
}

bool
Slice::Gen::OpsVisitor::visitModuleStart(const ModulePtr& p)
{
    string name = fixId(p->name());
    _out << sp << nl << "namespace " << name;
    _out << sb;

    return true;
}

void
Slice::Gen::OpsVisitor::visitModuleEnd(const ModulePtr& p)
{
    _out << eb;
}

bool
Slice::Gen::OpsVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    //
    // Don't generate an Operations interface for non-abstract classes.
    //
    if(!p->isAbstract())
    {
	return false;
    }

    string name = fixId(p->name());
    string scoped = fixId(p->scoped());
    ClassList bases = p->bases();

    _out << sp << nl << "public interface " << name << "_Operations";
    if((bases.size() == 1 && bases.front()->isAbstract()) || bases.size() > 1)
    {
        _out << " : ";
	_out.useCurrentPosAsIndent();
	ClassList::const_iterator q = bases.begin();
	bool first = true;
	while(q != bases.end())
	{
	    if((*q)->isAbstract())
	    {
	        if(!first)
		{
		    _out << ',' << nl;
		}
		else
		{
		    first = false;
		}
		_out << fixId((*q)->scoped()) << "_Operations";
	    }
	    ++q;
	}
	_out.restoreIndent();
    }

    _out << sb;

    return true;
}

void
Slice::Gen::OpsVisitor::visitClassDefEnd(const ClassDefPtr& p)
{
    _out << eb;
}

void
Slice::Gen::OpsVisitor::visitOperation(const OperationPtr& p)
{
    _out << sp << nl << typeToString(p->returnType()) << " " << fixId(p->name()) << "(";
    ParamDeclList paramList = p->parameters();
    for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
	if(q != paramList.begin())
	{
	    _out << ", ";
	}
	if((*q)->isOutParam())
	{
	    _out << "out ";
	}
	_out << typeToString((*q)->type()) << " " << fixId((*q)->name());
    }
    if(!ClassDefPtr::dynamicCast(p->container())->isLocal())
    {
	if(!paramList.empty())
	{
	    _out << ", ";
	}
	_out << "Ice.Current __current";
    }
    _out << ");";
}

Slice::Gen::HelperVisitor::HelperVisitor(IceUtil::Output& out)
    : CsVisitor(out)
{
}

bool
Slice::Gen::HelperVisitor::visitModuleStart(const ModulePtr& p)
{
    string name = fixId(p->name());
    _out << sp << nl << "namespace " << name;
    _out << sb;

    return true;
}

void
Slice::Gen::HelperVisitor::visitModuleEnd(const ModulePtr& p)
{
    _out << eb;
}

bool
Slice::Gen::HelperVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    if(p->isLocal())
        return false;

    string name = fixId(p->name());
    ClassList bases = p->bases();

    _out << sp << nl << "public class " << name << "PrxHelper : Ice.ObjectPrxHelper, " << name << "Prx";
    _out << sb;

    OperationList ops = p->allOperations();

    OperationList::const_iterator r;
    for(r = ops.begin(); r != ops.end(); ++r)
    {
        OperationPtr op = *r;
	string opName = fixId(op->name());
	TypePtr ret = op->returnType();
	string retS = typeToString(ret);
	string params = getParams(op);
	string args = getArgs(op);

	_out << sp; // class

	_out << nl << "public " << retS << " " << opName << '(' << params << ')';
	_out << sb;
	_out << nl;
	if(ret)
	{
	    _out << "return ";
	}
	_out << opName << '(' << args;
	if(!args.empty())
	{
	    _out << ", ";
	}
	_out << "__defaultContext());";
	_out << eb;

	_out << sp << nl << "public " << retS << " " << opName << '(' << params;
	if(!params.empty())
	{
	    _out << ", ";
	}
	_out << "Ice.Context __context)";
	_out << sb;
	_out << nl << "int __cnt = 0;";
	_out << nl << "while(true)";
	_out << sb;
	_out << nl << "try";
	_out << sb;
	if(op->returnsData())
	{
	    _out << nl << "__checkTwowayOnly(\"" << opName << "\");";
	}
	_out << nl << "Ice._ObjectDel __delBase = __getDelegate();";
	_out << nl << '_' << name << "Del __del = (_" << name << "Del)__delBase;";
	_out << nl;
	if(ret)
	{
	    _out << "return ";
	}
	_out << "__del." << opName << '(' << args;
	if(!args.empty())
	{
	    _out << ", ";
	}
	_out << "__context);";
	if(!ret)
	{
	    _out << nl << "return;";
	}
	_out << eb;
	_out << nl << "catch(IceInternal.NonRepeatable __ex)";
	_out << sb;
	if(op->mode() == Operation::Idempotent || op->mode() == Operation::Nonmutating)
	{
	    _out << nl << "__cnt = __handleException(__ex.get(), __cnt);";
	}
	else
	{
	    _out << nl << "__rethrowException(__ex.get());";
	}
	_out << eb;
	_out << nl << "catch(Ice.LocalException __ex)";
	_out << sb;
	_out << nl << "__cnt = __handleException(__ex, __cnt);";
	_out << eb;
	_out << eb;
	
	if(p->hasMetaData("ami") || op->hasMetaData("ami"))
	{
	    // TODO: generate AMI ops
	}

	_out << eb;
    }

    _out << sp << nl << "protected override Ice._ObjectDelM __createDelegateM()";
    _out << sb;
    _out << nl << "return new _" << name << "DelM();";
    _out << eb;

    _out << sp << nl << "protected override Ice._ObjectDelD __createDelegateD()";
    _out << sb;
    _out << nl << "return new _" << name << "DelD();";
    _out << eb;

    _out << sp << nl << "public static " << name << "Prx _checkedCast(Ice.ObjectPrx b)";
    _out << sb;
    _out << nl << "if(b == null)";
    _out << sb;
    _out << nl << "return null;";
    _out << eb;
    _out << nl << "if(b is " << name << "Prx)";
    _out << sb;
    _out << nl << "return (" << name << "Prx)b;";
    _out << eb;
    _out << nl << "if(b.ice_isA(\"" << p->scoped() << "\"))";
    _out << sb;
    _out << nl << name << "PrxHelper h = new " << name << "PrxHelper();";
    _out << nl << "h.__copyFrom(b);";
    _out << nl << "return h;";
    _out << eb;
    _out << nl << "return null;";
    _out << eb;

    _out << sp << nl << "public static " << name << "Prx _checkedCast(Ice.ObjectPrx b, string f)";
    _out << sb;
    _out << nl << "if(b == null)";
    _out << sb;
    _out << nl << "return null;";
    _out << eb;
    _out << nl << "Ice.ObjectPrx bb = b.ice_appendFacet(f);";
    _out << nl << "try";
    _out << sb;
    _out << nl << "if(bb.ice_isA(\"" << p->scoped() << "\"))";
    _out << sb;
    _out << nl << name << "PrxHelper h = new " << name << "PrxHelper();";
    _out << nl << "h.__copyFrom(bb);";
    _out << nl << "return h;";
    _out << eb;
    _out << eb;
    _out << nl << "catch(Ice.FacetNotExistException)";
    _out << sb;
    _out << eb;
    _out << nl << "return null;";
    _out << eb;

    _out << sp << nl << "public static " << name << "Prx _uncheckedCast(Ice.ObjectPrx b)";
    _out << sb;
    _out << nl << "if(b == null)";
    _out << sb;
    _out << nl << "return null;";
    _out << eb;
    _out << nl << name << "PrxHelper h = new " << name << "PrxHelper();";
    _out << nl << "h.__copyFrom(b);";
    _out << nl << "return h;";
    _out << eb;

    _out << sp << nl << "public static " << name << "Prx _uncheckedCast(Ice.ObjectPrx b, string f)";
    _out << sb;
    _out << nl << "if(b == null)";
    _out << sb;
    _out << nl << "return null;";
    _out << eb;
    _out << nl << "Ice.ObjectPrx bb = b.ice_appendFacet(f);";
    _out << nl << name << "PrxHelper h = new " << name << "PrxHelper();";
    _out << nl << "h.__copyFrom(bb);";
    _out << nl << "return h;";
    _out << eb;

    _out << sp << nl << "public static void __write(IceInternal.BasicStream __os, " << name << "Prx __v)";
    _out << sb;
    _out << nl << "__os.writeProxy(__v);";
    _out << eb;

    _out << sp << nl << "public static " << name << "Prx __read(IceInternal.BasicStream __is)";
    _out << sb;
    _out << nl << "Ice.ObjectPrx proxy = __is.readProxy();";
    _out << nl << "if(proxy != null)";
    _out << sb;
    _out << nl << name << "PrxHelper result = new " << name << "PrxHelper();";
    _out << nl << "result.__copyFrom(proxy);";
    _out << nl << "return result;";
    _out << eb;
    _out << nl << "return null;";
    _out << eb;

    return true;
}

void
Slice::Gen::HelperVisitor::visitClassDefEnd(const ClassDefPtr& p)
{
    _out << eb;
}

Slice::Gen::DelegateVisitor::DelegateVisitor(IceUtil::Output& out)
    : CsVisitor(out)
{
}

bool
Slice::Gen::DelegateVisitor::visitModuleStart(const ModulePtr& p)
{
    string name = fixId(p->name());
    _out << sp << nl << "namespace " << name;
    _out << sb;

    return true;
}

void
Slice::Gen::DelegateVisitor::visitModuleEnd(const ModulePtr& p)
{
    _out << eb;
}

bool
Slice::Gen::DelegateVisitor::visitClassDefStart(const ClassDefPtr& p)
{
   if(p->isLocal())
   {
       return false;
   }

   string name = fixId(p->name());
   ClassList bases = p->bases();

   _out << sp << nl << "public interface _" << name << "Del : ";
   if(bases.empty())
   {
       _out << "Ice._ObjectDel";
   }
   else
   {
       _out.useCurrentPosAsIndent();
       ClassList::const_iterator q = bases.begin();
       while(q != bases.end())
       {
           _out << fixId((*q)->name())  << "Del";
	   if(++q != bases.end())
	   {
	       _out << ',' << nl;
	   }
       }
       _out.restoreIndent();
   }

   _out << sb;

   OperationList ops = p->operations();

   OperationList::const_iterator r;
   for(r = ops.begin(); r != ops.end(); ++r)
   {
       OperationPtr op = *r;
       string opName = fixId(op->name());
       TypePtr ret = op->returnType();
       string retS = typeToString(ret);
       string params = getParams(op);

       _out << sp << nl << retS << ' ' << opName << '(' << params;
       if(!params.empty())
       {
           _out << ", ";
       }
       _out << "Ice.Context __context);";
       if(p->hasMetaData("ami") || op->hasMetaData("ami"))
       {
           // TODO: generate async ops here
       }
   }

   return true;
}

void
Slice::Gen::DelegateVisitor::visitClassDefEnd(const ClassDefPtr& p)
{
    _out << eb;
}

Slice::Gen::DelegateMVisitor::DelegateMVisitor(IceUtil::Output& out)
    : CsVisitor(out)
{
}

bool
Slice::Gen::DelegateMVisitor::visitModuleStart(const ModulePtr& p)
{
    string name = fixId(p->name());
    _out << sp << nl << "namespace " << name;
    _out << sb;

    return true;
}

void
Slice::Gen::DelegateMVisitor::visitModuleEnd(const ModulePtr& p)
{
    _out << eb;
}

bool
Slice::Gen::DelegateMVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    if(p->isLocal())
    {
       return false;
    }

    string name = fixId(p->name());
    ClassList bases = p->bases();

    _out << sp << nl << "public sealed class _" << name << "DelM : Ice._ObjectDelM, _" << name << "Del";
    _out << sb;

    OperationList ops = p->allOperations();

    OperationList::const_iterator r;
    for(r = ops.begin(); r != ops.end(); ++r)
    {
	OperationPtr op = *r;
	string opName = fixId(op->name());
	TypePtr ret = op->returnType();
	string retS = typeToString(ret);
	int iter;

	TypeStringList inParams;
	TypeStringList outParams;
	ParamDeclList paramList = op->parameters();
	for(ParamDeclList::const_iterator pli = paramList.begin(); pli != paramList.end(); ++pli)
	{
	    if((*pli)->isOutParam())
	    {
	       outParams.push_back(make_pair((*pli)->type(), (*pli)->name()));
	    }
	    else
	    {
	       inParams.push_back(make_pair((*pli)->type(), (*pli)->name()));
	    }
	}

	TypeStringList::const_iterator q;

	ExceptionList throws = op->throws();
	throws.sort();
	throws.unique();
	throws.erase(remove_if(throws.begin(), throws.end(), IceUtil::constMemFun(&Exception::isLocal)), throws.end());

	string params = getParams(op);

	_out << sp << nl << "public " << retS << ' ' << opName << '(' << params;
	if(!params.empty())
	{
	   _out << ", ";
	}
	_out << "Ice.Context __context)";
	_out << sb;

	_out << nl << "IceInternal.Outgoing __out = getOutgoing(\"" << op->name() << "\", " << sliceModeToIceMode(op)
	     << ", __context);";
	_out << nl << "try";
	_out << sb;
	if(!inParams.empty())
	{
	    _out << nl << "IceInternal.BasicStream __is = __out.istr();";
	}
	iter = 0;
	for(q = inParams.begin(); q != inParams.end(); ++q)
	{
	    // TODO: write marshal/unmarshal code here
	}
	if(op->sendsClasses())
	{
	    _out << nl << "__os.writePendingObjects();";
	}
	_out << nl << "if(!__out.invoke())";
	_out << sb;
	if(!throws.empty())
	{
            //
            // The try/catch block is necessary because throwException()
            // can raise UserException.
            //
            _out << nl << "try";
            _out << sb;
            _out << nl << "__is.throwException();";
            _out << eb;
            for(ExceptionList::const_iterator t = throws.begin(); t != throws.end(); ++t)
            {
                _out << nl << "catch(" << fixId((*t)->name()) << ')';
                _out << sb;
                _out << nl << "throw;";
                _out << eb;
            }
            _out << nl << "catch(Ice.UserException)";
            _out << sb;
            _out << eb;
	}
	_out << nl << "throw new Ice.UnknownUserException();";
	_out << eb;
	if(!outParams.empty() || ret)
	{
	    _out << nl << "try";
	    _out << sb;
	}
	for(q = outParams.begin(); q != outParams.end(); ++q)
	{
	    // TODO: write marshal/unmarshal code here
	}
	if(ret)
	{
	    BuiltinPtr builtin = BuiltinPtr::dynamicCast(ret);
	    if((builtin && builtin->kind() == Builtin::KindObject) || ClassDeclPtr::dynamicCast(ret))
	    {
		_out << nl << retS << " __ret = new " << retS << "();";
		_out << nl << "__is.readObject(__ret.getPatcher());";
	    }
	    else
	    {
		_out << nl << retS << " __ret = null;"; // TODO: remove null initialization 
		// TODO writeMarshalUnmarshalCode(_out, package, ret, "__ret", false, iter);
	    }
	}
	if(op->returnsClasses())
	{
	    _out << nl << "__is.readPendingObjects();";
	}
	if(ret)
	{
	    _out << nl << "return __ret;";
	}
        if(!outParams.empty() || ret)
        {
            _out << eb;
            _out << nl << "catch(Ice.LocalException __ex)";
            _out << sb;
            _out << nl << "throw new IceInternal.NonRepeatable(__ex);";
            _out << eb;
        }
        _out << eb;
        _out << nl << "finally";
        _out << sb;
        _out << nl << "reclaimOutgoing(__out);";
        _out << eb;
        _out << eb;

	if(p->hasMetaData("ami") || op->hasMetaData("ami"))
	{
	    // TODO: async params
	}
    }

    return true;
}

void
Slice::Gen::DelegateMVisitor::visitClassDefEnd(const ClassDefPtr& p)
{
    _out << eb;
}

Slice::Gen::DelegateDVisitor::DelegateDVisitor(IceUtil::Output& out)
    : CsVisitor(out)
{
}

bool
Slice::Gen::DelegateDVisitor::visitModuleStart(const ModulePtr& p)
{
    string name = fixId(p->name());
    _out << sp << nl << "namespace " << name;
    _out << sb;

    return true;
}

void
Slice::Gen::DelegateDVisitor::visitModuleEnd(const ModulePtr& p)
{
    _out << eb;
}

bool
Slice::Gen::DelegateDVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    if(p->isLocal())
    {
        return false;
    }

    string name = fixId(p->name());
    ClassList bases = p->bases();

    _out << sp << nl << "public sealed class _" << name << "DelD : Ice._ObjectDelD, _" << name << "Del";
    _out << sb;

    OperationList ops = p->allOperations();

    OperationList::const_iterator r;
    for(r = ops.begin(); r != ops.end(); ++r)
    {
        OperationPtr op = *r;
        string opName = fixId(op->name());
        TypePtr ret = op->returnType();
        string retS = typeToString(ret);

        ExceptionList throws = op->throws();
        throws.sort();
        throws.unique();
        throws.erase(remove_if(throws.begin(), throws.end(), IceUtil::constMemFun(&Exception::isLocal)), throws.end());

        string params = getParams(op);
        string args = getArgs(op);

	_out << sp;
        _out << nl << "public " << retS << ' ' << opName << '(' << params;
        if(!params.empty())
        {
            _out << ", ";
        }
        _out << "Ice.Context __context)";
        // TODO: writeDelegateThrowsClause(package, throws);
        _out << sb;
	if(p->hasMetaData("amd") || op->hasMetaData("amd"))
	{
	    _out << nl << "throw new Ice.CollocationOptimizationException();";
	}
	else
	{
	    _out << nl << "Ice.Current __current = new Ice.Current();";
	    _out << nl << "__initCurrent(__current, \"" << op->name() << "\", " << sliceModeToIceMode(op)
		 << ", __context);";
	    _out << nl << "while(true)";
	    _out << sb;
	    _out << nl << "IceInternal.Direct __direct = new IceInternal.Direct(__current);";
	    _out << nl << "object __servant = __direct.facetServant();";
	    _out << nl << "if(__servant is " << name << ")";
	    _out << sb;
	    _out << nl << "try";
	    _out << sb;
	    _out << nl;
	    if(ret)
	    {
		_out << "return ";
	    }
	    _out << "((" << name << ")__servant)." << opName << '(' << args;
	    if(!args.empty())
	    {
	        _out << ", ";
	    }
	    _out << "__current);";
	    if(!ret)
	    {
	        _out << nl << "return;";
	    }
	    _out << eb;
	    _out << nl << "catch(Ice.LocalException __ex)";
	    _out << sb;
	    _out << nl << "throw new IceInternal.NonRepeatable(__ex);";
	    _out << eb;
	    _out << nl << "finally";
	    _out << sb;
	    _out << nl << "__direct.destroy();";
	    _out << eb;
	    _out << eb;
	    _out << nl << "else";
	    _out << sb;
	    _out << nl << "__direct.destroy();";
	    _out << nl << "Ice.OperationNotExistException __opEx = new Ice.OperationNotExistException();";
	    _out << nl << "__opEx.id = __current.id;";
	    _out << nl << "__opEx.facet = __current.facet;";
	    _out << nl << "__opEx.operation = __current.operation;";
	    _out << nl << "throw __opEx;";
	    _out << eb;
	    _out << eb;
	}
        _out << eb;

	if(p->hasMetaData("ami") || op->hasMetaData("ami"))
	{
	    // TODO: async
	}
    }

    return true;
}

void
Slice::Gen::DelegateDVisitor::visitClassDefEnd(const ClassDefPtr& p)
{
    _out << eb;
}

Slice::Gen::DispatcherVisitor::DispatcherVisitor(::IceUtil::Output &out)
    : CsVisitor(out)
{
}

bool
Slice::Gen::DispatcherVisitor::visitModuleStart(const ModulePtr& p)
{
    string name = fixId(p->name());
    _out << sp << nl << "namespace " << name;
    _out << sb;

    return true;
}

void
Slice::Gen::DispatcherVisitor::visitModuleEnd(const ModulePtr& p)
{
    _out << eb;
}

bool
Slice::Gen::DispatcherVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    if(p->isLocal() || !p->isInterface())
    {
        return false;
    }

    string name = fixId(p->name());

    _out << sp << nl << "public abstract class " << name << "_Disp : Ice.ObjectImpl, " << name;
    _out << sb;

    OperationList ops = p->operations();
    if(!ops.empty())
    {
        _out << sp << nl << "#region Slice operations";
    }

    for(OperationList::const_iterator op = ops.begin(); op != ops.end(); ++op)
    {
	string name = fixId((*op)->name());

	_out << sp << nl << "public abstract " << typeToString((*op)->returnType()) << " " << name << "(";
	ParamDeclList paramList = (*op)->parameters();
	for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
	{
	    if(q != paramList.begin())
	    {
		_out << ", ";
	    }
	    if((*q)->isOutParam())
	    {
		_out << "out ";
	    }
	    _out << typeToString((*q)->type()) << " " << fixId((*q)->name());
	}
	if(!p->isLocal())
	{
	    if(!paramList.empty())
	    {
		_out << ", ";
	    }
	    _out << "Ice.Current __current";
	}
	_out << ");";
    }

    if(!ops.empty())
    {
        _out << sp << nl << "#endregion"; // Slice operations
    }

    writeInheritedOperations(p);

    writeDispatch(p);
    _out << eb;

    return true;
}
