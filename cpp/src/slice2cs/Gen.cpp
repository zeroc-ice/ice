// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Functional.h>
#include <Gen.h>
#include <limits>
#include <sys/stat.h>
#ifndef _MSC_VER
#include <unistd.h>
#else
#include <direct.h>
#endif
#include <IceUtil/Algorithm.h>
#include <IceUtil/Iterator.h>
#include <IceUtil/UUID.h>
#include <Slice/Checksum.h>
#include <Slice/DotNetNames.h>

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
using IceUtil::spar;
using IceUtil::epar;

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
	    ClassDefPtr containingClass = ClassDefPtr::dynamicCast((*op)->container());
	    bool amd = containingClass->hasMetaData("amd") || (*op)->hasMetaData("amd");
	    string name = fixId((*op)->name(), DotNet::ICloneable, true);
	    if(!amd)
	    {
		vector<string> params = getParams(*op);
		vector<string> args = getArgs(*op);
		string retS = typeToString((*op)->returnType());

		_out << sp << nl << "public " << retS << ' ' << name << spar << params << epar;
		_out << sb;
		_out << nl;
		if((*op)->returnType())
		{
		    _out << "return ";
		}
		_out << name << spar << args << "Ice.ObjectImpl.defaultCurrent" << epar << ';';
		_out << eb;

		_out << sp << nl << "public abstract " << retS << ' ' << name << spar << params;
		if(!containingClass->isLocal())
		{
		    _out << "Ice.Current __current";
		}
		_out << epar << ';';
	    }
	    else
	    {
		vector<string> params = getParamsAsync(*op, true);
		vector<string> args = getArgsAsync(*op);

		_out << sp << nl << "public void " << name << "_async" << spar << params << epar;
		_out << sb;
		_out << nl << name << "_async" << spar << args << epar << ';';
		_out << eb;

		_out << sp << nl << "public abstract void " << name << "_async"
		     << spar << params << "Ice.Current __current" << epar << ';';
	    }
	}

	_out << sp << nl << "#endregion"; // Inherited Slice operations
    }
}

void
Slice::CsVisitor::writeDispatch(const ClassDefPtr& p)
{
    string name = fixId(p->name());
    string scoped = p->scoped();
    ClassList allBases = p->allBases();
    StringList ids;

#if defined(__IBMCPP__) && defined(NDEBUG)
    //
    // VisualAge C++ 6.0 does not see that ClassDef is a Contained,
    // when inlining is on. The code below issues a warning: better
    // than an error!
    //
    transform(allBases.begin(), allBases.end(), back_inserter(ids), ::IceUtil::constMemFun<string,ClassDef>(&Contained::scoped));
#else
    transform(allBases.begin(), allBases.end(), back_inserter(ids), ::IceUtil::constMemFun(&Contained::scoped));
#endif

    StringList other;
    other.push_back(p->scoped());
    other.push_back("::Ice::Object");
    other.sort();
    ids.merge(other);
    ids.unique();

    //
    // We sort again to keep the order in the same order as the one
    // expected by System.Collections.Comparer.DefaultInvariant.
    //
#if defined(__SUNPRO_CC)
    ids.sort(cICompare);
#else
    ids.sort(Slice::CICompare());
#endif

    StringList::const_iterator firstIter = ids.begin();
    StringList::const_iterator scopedIter = find(ids.begin(), ids.end(), scoped);
    assert(scopedIter != ids.end());
    StringList::difference_type scopedPos = ice_distance(firstIter, scopedIter);
    
    _out << sp << nl << "#region Slice type-related members";

    _out << sp << nl << "public static new string[] __ids = ";
    _out << sb;

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

    _out << sp << nl << "public override bool ice_isA(string s)";
    _out << sb;
    _out << nl << "if(IceInternal.AssemblyUtil._runtime == IceInternal.AssemblyUtil.Runtime.Mono)"
                  " // Bug in Mono 1.0 DefaultInvariant";
    _out << sb;
    _out << nl << "return _System.Array.BinarySearch(__ids, s) >= 0;";
    _out << eb;
    _out << nl << "else";
    _out << sb;
    _out << nl << "return _System.Array.BinarySearch(__ids, s, _System.Collections.Comparer.DefaultInvariant) >= 0;";
    _out << eb;
    _out << eb;

    _out << sp << nl << "public override bool ice_isA(string s, Ice.Current __current)";
    _out << sb;
    _out << nl << "if(_System.Type.GetType(\"Mono.Runtime\", false) != null) // Bug in Mono 1.0 DefaultInvariant";
    _out << sb;
    _out << nl << "return _System.Array.BinarySearch(__ids, s) >= 0;";
    _out << eb;
    _out << nl << "else";
    _out << sb;
    _out << nl << "return _System.Array.BinarySearch(__ids, s, _System.Collections.Comparer.DefaultInvariant) >= 0;";
    _out << eb;
    _out << eb;

    _out << sp << nl << "public override string[] ice_ids()";
    _out << sb;
    _out << nl << "return __ids;";
    _out << eb;

    _out << sp << nl << "public override string[] ice_ids(Ice.Current __current)";
    _out << sb;
    _out << nl << "return __ids;";
    _out << eb;

    _out << sp << nl << "public override string ice_id()";
    _out << sb;
    _out << nl << "return __ids[" << scopedPos << "];";
    _out << eb;

    _out << sp << nl << "public override string ice_id(Ice.Current __current)";
    _out << sb;
    _out << nl << "return __ids[" << scopedPos << "];";
    _out << eb;

    _out << sp << nl << "public static new string ice_staticId()";
    _out << sb;
    _out << nl << "return __ids[" << scopedPos << "];";
    _out << eb;

    _out << sp << nl << "#endregion"; // Slice type-related members

    OperationList ops = p->operations();
    if(!p->isInterface() || ops.size() != 0)
    {
	_out << sp << nl << "#region Operation dispatch";
    }

    OperationList::const_iterator r;
    for(r = ops.begin(); r != ops.end(); ++r)
    {
        OperationPtr op = *r;
	ContainerPtr container = op->container();
        ClassDefPtr cl = ClassDefPtr::dynamicCast(container);
        assert(cl);

        string opName = op->name();
        _out << sp << nl << "public static IceInternal.DispatchStatus ___" << opName << '(' << name
	     << " __obj, IceInternal.Incoming __in, Ice.Current __current)";
        _out << sb;

	bool amd = p->hasMetaData("amd") || op->hasMetaData("amd");
	if(!amd)
	{
	    TypePtr ret = op->returnType();
	    
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
	    
	    ExceptionList throws = op->throws();
	    throws.sort();
	    throws.unique();

	    //
	    // Arrange exceptions into most-derived to least-derived order. If we don't
	    // do this, a base exception handler can appear before a derived exception
	    // handler, causing compiler warnings and resulting in the base exception
	    // being marshaled instead of the derived exception.
	    //
#if defined(__SUNPRO_CC)
	    throws.sort(Slice::derivedToBaseCompare);
#else
	    throws.sort(Slice::DerivedToBaseCompare());
#endif

	    TypeStringList::const_iterator q;
	    
	    if(!inParams.empty())
	    {
		_out << nl << "IceInternal.BasicStream __is = __in.istr();";
	    }
	    if(!outParams.empty() || ret || !throws.empty())
	    {
		_out << nl << "IceInternal.BasicStream __os = __in.ostr();";
	    }
	    
	    //
	    // Unmarshal 'in' parameters.
	    //
	    for(q = inParams.begin(); q != inParams.end(); ++q)
	    {
		string param = fixId(q->second);
		string typeS = typeToString(q->first);
		BuiltinPtr builtin = BuiltinPtr::dynamicCast(q->first);
		bool isClass = (builtin && builtin->kind() == Builtin::KindObject)
		               || ClassDeclPtr::dynamicCast(q->first);
		if(!isClass)
		{
		    _out << nl << typeS << ' ' << param << ';';
		}
		writeMarshalUnmarshalCode(_out, q->first, param, false, false, true);
	    }
	    if(op->sendsClasses())
	    {
		_out << nl << "__is.readPendingObjects();";
	    }
	    for(q = inParams.begin(); q != inParams.end(); ++q)
	    {
		StructPtr st = StructPtr::dynamicCast(q->first);
		bool patchStruct = st && !st->hasMetaData("cs:class") && st->classDataMembers().size() != 0;
		if(patchStruct)
		{
		    _out << nl << fixId(q->second) << ".__patch();";
		}
	    }
	    
	    for(q = outParams.begin(); q != outParams.end(); ++q)
	    {
		string typeS = typeToString(q->first);
		_out << nl << typeS << ' ' << fixId(q->second) << ";";
	    }
	    
	    //
	    // Call on the servant.
	    //
	    if(!throws.empty())
	    {
		_out << nl << "try";
		_out << sb;
	    }
	    _out << nl;
	    if(ret)
	    {
		string retS = typeToString(ret);
		_out << retS << " __ret = ";
	    }
	    _out << "__obj." << fixId(opName, DotNet::ICloneable, true) << spar;
	    for(q = inParams.begin(); q != inParams.end(); ++q)
	    {
		BuiltinPtr builtin = BuiltinPtr::dynamicCast(q->first);
		bool isClass = (builtin && builtin->kind() == Builtin::KindObject)
		               || ClassDeclPtr::dynamicCast(q->first);
		
		string arg;
		if(isClass)
		{
		    arg += "(" + typeToString(q->first) + ")";
		}
		arg += fixId(q->second);
		if(isClass)
		{
		    arg += "_PP.value";
		}
		_out << arg;
	    }
	    for(q = outParams.begin(); q != outParams.end(); ++q)
	    {
		_out << "out " + fixId(q->second);
	    }
	    _out << "__current" << epar << ';';
	    
	    //
	    // Marshal 'out' parameters and return value.
	    //
	    for(q = outParams.begin(); q != outParams.end(); ++q)
	    {
		writeMarshalUnmarshalCode(_out, q->first, fixId(q->second), true, false, true, "");
	    }
	    if(ret)
	    {
		writeMarshalUnmarshalCode(_out, ret, "__ret", true, false, true, "");
	    }
	    if(op->returnsClasses())
	    {
		_out << nl << "__os.writePendingObjects();";
	    }
	    _out << nl << "return IceInternal.DispatchStatus.DispatchOK;";
	    
	    //
	    // Handle user exceptions.
	    //
	    if(!throws.empty())
	    {
		_out << eb;
		ExceptionList::const_iterator t;
		for(t = throws.begin(); t != throws.end(); ++t)
		{
		    string exS = fixId((*t)->scoped());
		    _out << nl << "catch(" << exS << " ex)";
		    _out << sb;
		    _out << nl << "__os.writeUserException(ex);";
		    _out << nl << "return IceInternal.DispatchStatus.DispatchUserException;";
		    _out << eb;
		}
	    }

	    _out << eb;
	}
	else
	{
	    TypeStringList inParams;
	    ParamDeclList paramList = op->parameters();
	    for(ParamDeclList::const_iterator pli = paramList.begin(); pli != paramList.end(); ++pli)
	    {
		if(!(*pli)->isOutParam())
		{
		    inParams.push_back(make_pair((*pli)->type(), (*pli)->name()));
		}
	    }
	    
	    TypeStringList::const_iterator q;
	    
	    if(!inParams.empty())
	    {
		_out << nl << "IceInternal.BasicStream __is = __in.istr();";
	    }
	    
	    //
	    // Unmarshal 'in' parameters.
	    //
	    for(q = inParams.begin(); q != inParams.end(); ++q)
	    {
		BuiltinPtr builtin = BuiltinPtr::dynamicCast(q->first);
		bool isClass = (builtin && builtin->kind() == Builtin::KindObject)
		               || ClassDeclPtr::dynamicCast(q->first);
		if(!isClass)
		{
		    _out << nl << typeToString(q->first) << ' ' << fixId(q->second) << ';';
		}
		writeMarshalUnmarshalCode(_out, q->first, fixId(q->second), false, false, true);
	    }
	    if(op->sendsClasses())
	    {
		_out << nl << "__is.readPendingObjects();";
	    }
	    for(q = inParams.begin(); q != inParams.end(); ++q)
	    {
		StructPtr st = StructPtr::dynamicCast(q->first);
		bool patchStruct = st && !st->hasMetaData("cs:class") && st->classDataMembers().size() != 0;
		if(patchStruct)
		{
		    _out << nl << fixId(q->second) << ".__patch();";
		}
	    }
	    
	    //
	    // Call on the servant.
	    //
	    string classNameAMD = "AMD_" + p->name();
	    _out << nl << classNameAMD << '_' << opName << " __cb = new _" << classNameAMD << '_' << opName
		 << "(__in);";
            _out << nl << "try";
            _out << sb;
	    _out << nl << "__obj.";
	    if(amd)
	    {
		_out << opName << "_async";
	    }
	    else
	    {
	        _out << fixId(opName, DotNet::ICloneable, true);
	    }
	    _out << spar;
	    if(amd)
	    {
	        _out << "__cb";
	    }
	    for(q = inParams.begin(); q != inParams.end(); ++q)
	    {
		BuiltinPtr builtin = BuiltinPtr::dynamicCast(q->first);
		bool isClass = (builtin && builtin->kind() == Builtin::KindObject)
		               || ClassDeclPtr::dynamicCast(q->first);
		string arg;
		if(isClass)
		{
		    arg += "(" + typeToString(q->first) + ")";
		}
		arg += fixId(q->second);
		if(isClass)
		{
		    arg += "_PP.value";
		}
		_out << arg;
	    }
	    _out << "__current" << epar << ';';
	    _out << eb;
	    _out << nl << "catch(_System.Exception ex)";
	    _out << sb;
	    _out << nl << "__cb.ice_exception(ex);";
	    _out << eb;
	    _out << nl << "return IceInternal.DispatchStatus.DispatchAsync;";

	    _out << eb;
	}
    }

    OperationList allOps = p->allOperations();
    if(!allOps.empty())
    {
	StringList allOpNames;
#if defined(__IBMCPP__) && defined(NDEBUG)
 	//
 	// See comment for transform above
 	//
 	transform(allOps.begin(), allOps.end(), back_inserter(allOpNames), ::IceUtil::constMemFun<string,Operation>(&Contained::name));
#else
	transform(allOps.begin(), allOps.end(), back_inserter(allOpNames), ::IceUtil::constMemFun(&Contained::name));
#endif
	allOpNames.push_back("ice_id");
	allOpNames.push_back("ice_ids");
	allOpNames.push_back("ice_isA");
	allOpNames.push_back("ice_ping");

	//
	// We sort into case-insensitive order here because, at run time,
	// the sort order must match the sort order used by System.Array.Sort().
	// (C# has no notion of the default ASCII ordering.)
	//
#if defined(__SUNPRO_CC)
	allOpNames.sort(Slice::cICompare);
#else
	allOpNames.sort(Slice::CICompare());
#endif
	allOpNames.unique();

	StringList::const_iterator q;

	_out << sp << nl << "private static string[] __all =";
	_out << sb;
	q = allOpNames.begin();
	while(q != allOpNames.end())
	{
	    _out << nl << '"' << *q << '"';
	    if(++q != allOpNames.end())
	    {
		_out << ',';
	    }
	}
	_out << eb << ';';

	_out << sp << nl << "public override IceInternal.DispatchStatus "
	     << "__dispatch(IceInternal.Incoming __in, Ice.Current __current)";
	_out << sb;
	_out << nl << "int pos;";
	_out << nl << "if(_System.Type.GetType(\"Mono.Runtime\", false) != null) // Bug in Mono 1.0 DefaultInvariant";
	_out << sb;
	_out << nl << "pos = _System.Array.BinarySearch(__all, __current.operation);";
	_out << eb;
	_out << nl << "else";
	_out << sb;
	_out << nl << "pos = _System.Array.BinarySearch(__all, __current.operation, "
	     << "_System.Collections.Comparer.DefaultInvariant);";
	_out << eb;
	_out << nl << "if(pos < 0)";
	_out << sb;
	_out << nl << "return IceInternal.DispatchStatus.DispatchOperationNotExist;";
	_out << eb;
	_out << sp << nl << "switch(pos)";
	_out << sb;
	int i = 0;
	for(q = allOpNames.begin(); q != allOpNames.end(); ++q)
	{
	    string opName = *q;

	    _out << nl << "case " << i++ << ':';
	    _out << sb;
	    if(opName == "ice_id")
	    {
		_out << nl << "return ___ice_id(this, __in, __current);";
	    }
	    else if(opName == "ice_ids")
	    {
		_out << nl << "return ___ice_ids(this, __in, __current);";
	    }
	    else if(opName == "ice_isA")
	    {
		_out << nl << "return ___ice_isA(this, __in, __current);";
	    }
	    else if(opName == "ice_ping")
	    {
		_out << nl << "return ___ice_ping(this, __in, __current);";
	    }
	    else
	    {
		//
		// There's probably a better way to do this
		//
		for(OperationList::const_iterator t = allOps.begin(); t != allOps.end(); ++t)
		{
		    if((*t)->name() == (*q))
		    {
			ContainerPtr container = (*t)->container();
			ClassDefPtr cl = ClassDefPtr::dynamicCast(container);
			assert(cl);
			if(cl->scoped() == p->scoped())
			{
			    _out << nl << "return ___" << opName << "(this, __in, __current);";
			}
			else
			{
			    string base = cl->scoped();
			    if(cl->isInterface())
			    {
			        string::size_type pos = base.rfind("::");
				assert(pos != string::npos);
				base = base.insert(pos + 2, "_");
				base += "Disp";
			    }
			    _out << nl << "return " << fixId(base) << ".___" << opName << "(this, __in, __current);";
			}
			break;
		    }
		}
	    }
	    _out << eb;
	}
	_out << eb;
	_out << sp << nl << "_System.Diagnostics.Debug.Assert(false);";
	_out << nl << "return IceInternal.DispatchStatus.DispatchOperationNotExist;";
	_out << eb;
    }

    if(!p->isInterface() || ops.size() != 0)
    {
	_out << sp << nl << "#endregion"; // Operation dispatch
    }
}

vector<string>
Slice::CsVisitor::getParams(const OperationPtr& op)
{
    vector<string> params;
    ParamDeclList paramList = op->parameters();
    for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
	string param = typeToString((*q)->type()) + " " + fixId((*q)->name());
	if((*q)->isOutParam())
	{
	    param = "out " + param;
	}
	params.push_back(param);
    }
    return params;
}

vector<string>
Slice::CsVisitor::getParamsAsync(const OperationPtr& op, bool amd)
{
    vector<string> params;

    string name = fixId(op->name());
    ContainerPtr container = op->container();
    ClassDefPtr cl = ClassDefPtr::dynamicCast(container); // Get the class containing the op.
    string scope = fixId(cl->scope());
    params.push_back(scope + (amd ? "AMD_" : "AMI_") + cl->name() + '_' + op->name() + " __cb");

    ParamDeclList paramList = op->parameters();
    for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
	if(!(*q)->isOutParam())
	{
	    params.push_back(typeToString((*q)->type()) + " " + fixId((*q)->name()));
	}
    }
    return params;
}

vector<string>
Slice::CsVisitor::getParamsAsyncCB(const OperationPtr& op)
{
    vector<string> params;

    TypePtr ret = op->returnType();
    if(ret)
    {
	params.push_back(typeToString(ret) + " __ret");
    }

    ParamDeclList paramList = op->parameters();
    for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
	if((*q)->isOutParam())
	{
	    params.push_back(typeToString((*q)->type()) + ' ' + fixId((*q)->name()));
	}
    }

    return params;
}

vector<string>
Slice::CsVisitor::getArgs(const OperationPtr& op)
{
    vector<string> args;
    ParamDeclList paramList = op->parameters();
    for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
	string arg = fixId((*q)->name());
	if((*q)->isOutParam())
	{
	    arg = "out " + arg;
	}
        args.push_back(arg);
    }
    return args;
}

vector<string>
Slice::CsVisitor::getArgsAsync(const OperationPtr& op)
{
    vector<string> args;

    args.push_back("__cb");

    ParamDeclList paramList = op->parameters();
    for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
        if(!(*q)->isOutParam())
	{
	    args.push_back(fixId((*q)->name()));
	}
    }
    return args;
}

vector<string>
Slice::CsVisitor::getArgsAsyncCB(const OperationPtr& op)
{
    vector<string> args;

    TypePtr ret = op->returnType();
    if(ret)
    {
        args.push_back("__ret");
    }

    ParamDeclList paramList = op->parameters();
    for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
        if((*q)->isOutParam())
	{
	    args.push_back(fixId((*q)->name()));
	}
    }

    return args;
}

Slice::Gen::Gen(const string& name, const string& base, const vector<string>& includePaths, const string& dir,
                bool impl, bool implTie, bool stream)
    : _base(base),
      _includePaths(includePaths),
      _stream(stream)
{
    string file = base + ".cs";
    string fileImpl = base + "I.cs";

    if(!dir.empty())
    {
	//
	// Get the working directory and look at the returned path
	// to find out whether we need to use a forward or backward slash
	// as a path separator. (This seems to be one of the very few
	// portable ways to get the correct separator.)
	//
	char* p;
#if defined(_MSC_VER)
	p = _getcwd(0, 0);
#else
	p = getcwd(0, 0);
#endif
	if(p == 0)
	{
	    cerr << name << ": cannot get working directory: " << strerror(errno) << endl;
	    return;
	}
	string cwd(p);
	string slash = cwd.find('/') == string::npos ? "\\" : "/";
	free(p);

	string::size_type pos = base.rfind('/');
	if(pos == string::npos)
	{
	    pos = base.rfind('\\');
	}
	if(pos != string::npos)
	{
	    string fileBase(base, pos + 1);
	    file = dir + slash + fileBase + ".cs";
	    fileImpl = dir + slash + fileBase + "I.cs";
	}
	else
	{
	    file = dir + slash + file;
	    fileImpl = dir + slash + fileImpl;
	}
    }
    _out.open(file.c_str());
    if(!_out)
    {
        cerr << name << ": can't open `" << file << "' for writing" << endl;
	return;
    }
    printHeader();

    _out << nl << "// Generated from file `" << base << ".ice'";

    _out << sp << nl << "using _System = System;";
    _out << nl << "using _Microsoft = Microsoft;";

    if(impl || implTie)
    {
        struct stat st;
	if(stat(fileImpl.c_str(), &st) == 0)
	{
	    cerr << name << ": `" << fileImpl << "' already exists - will not overwrite" << endl;
	    return;
	}
	_impl.open(fileImpl.c_str());
	if(!_impl)
	{
	    cerr << name << ": can't open `" << fileImpl << "' for writing" << endl;
	    return;
	}
    }
}

Slice::Gen::~Gen()
{
    if(_out.isOpen())
    {
        _out << '\n';
    }
    if(_impl.isOpen())
    {
        _impl << '\n';
    }
}

bool
Slice::Gen::operator!() const
{
    return !_out;
}

void
Slice::Gen::generate(const UnitPtr& p)
{

    CsGenerator::validateMetaData(p);

    TypesVisitor typesVisitor(_out, _stream);
    p->visit(&typesVisitor, false);

    ProxyVisitor proxyVisitor(_out);
    p->visit(&proxyVisitor, false);

    OpsVisitor opsVisitor(_out);
    p->visit(&opsVisitor, false);

    HelperVisitor helperVisitor(_out, _stream);
    p->visit(&helperVisitor, false);

    DelegateVisitor delegateVisitor(_out);
    p->visit(&delegateVisitor, false);

    DelegateMVisitor delegateMVisitor(_out);
    p->visit(&delegateMVisitor, false);

    DelegateDVisitor delegateDVisitor(_out);
    p->visit(&delegateDVisitor, false);

    DispatcherVisitor dispatcherVisitor(_out);
    p->visit(&dispatcherVisitor, false);

    AsyncVisitor asyncVisitor(_out);
    p->visit(&asyncVisitor, false);
}

void
Slice::Gen::generateTie(const UnitPtr& p)
{
    TieVisitor tieVisitor(_out);
    p->visit(&tieVisitor, false);
}

void
Slice::Gen::generateImpl(const UnitPtr& p)
{
    ImplVisitor implVisitor(_impl);
    p->visit(&implVisitor, false);
}

void
Slice::Gen::generateImplTie(const UnitPtr& p)
{
    ImplTieVisitor implTieVisitor(_impl);
    p->visit(&implTieVisitor, false);
}

void
Slice::Gen::generateChecksums(const UnitPtr& p)
{
    ChecksumMap map = createChecksums(p);
    if(!map.empty())
    {
        string className = "_" + IceUtil::generateUUID();
        for(string::size_type pos = 1; pos < className.size(); ++pos)
        {
            if(!isalnum(className[pos]))
            {
                className[pos] = '_';
            }
        }

        _out << sp << nl << "namespace IceInternal";
        _out << sb;
        _out << nl << "namespace SliceChecksums";
        _out << sb;
        _out << nl << "public sealed class " << className;
        _out << sb;
        _out << nl << "public readonly static System.Collections.Hashtable map = new System.Collections.Hashtable();";
        _out << sp << nl << "static " << className << "()";
        _out << sb;
        for(ChecksumMap::const_iterator p = map.begin(); p != map.end(); ++p)
        {
            _out << nl << "map.Add(\"" << p->first << "\", \"";
            ostringstream str;
            str.flags(ios_base::hex);
            str.fill('0');
            for(vector<unsigned char>::const_iterator q = p->second.begin(); q != p->second.end(); ++q)
            {
                str << (int)(*q);
            }
            _out << str.str() << "\");";
        }
        _out << eb;
        _out << eb << ';';
        _out << eb;
        _out << eb;
    }
}

void
Slice::Gen::printHeader()
{
    static const char* header =
"// **********************************************************************\n"
"//\n"
"// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.\n"
"//\n"
"// This copy of Ice is licensed to you under the terms described in the\n"
"// ICE_LICENSE file included in this distribution.\n"
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

Slice::Gen::TypesVisitor::TypesVisitor(IceUtil::Output& out, bool stream)
    : CsVisitor(out), _stream(stream)
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
Slice::Gen::TypesVisitor::visitModuleEnd(const ModulePtr&)
{
    _out << eb;
}

bool
Slice::Gen::TypesVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    string name = p->name();
    string scoped = fixId(p->scoped());
    ClassList bases = p->bases();

    if(!p->isLocal() && _stream)
    {
        _out << sp << nl << "public sealed class " << name << "Helper";
	_out << sb;

	_out << sp << nl << "public " << name << "Helper(Ice.InputStream __in)";
	_out << sb;
	_out << nl << "_in = __in;";
	_out << nl << "_pp = new IceInternal.ParamPatcher(typeof(" << scoped << "));";
	_out << eb;

	_out << sp << nl << "public static void write(Ice.OutputStream __out, " << fixId(name) << " __v)";
	_out << sb;
	_out << nl << "__out.writeObject(__v);";
	_out << eb;

	_out << sp << nl << "public void read()";
	_out << sb;
	_out << nl << "_in.readObject(_pp);";
	_out << eb;

	_out << sp << nl << "public " << scoped << " value";
	_out << sb;
	_out << nl << "get";
	_out << sb;
	_out << nl << "return (" << scoped << ")_pp.value;";
	_out << eb;
	_out << eb;

	_out << sp << nl << "private Ice.InputStream _in;";
	_out << nl << "private IceInternal.ParamPatcher _pp;";

	_out << eb;
    }

    if(p->isInterface())
    {
        _out << sp << nl << "public interface " << fixId(name) << " : ";
	if(p->isLocal())
	{
	    _out << "Ice.LocalObject";
	}
	else
	{
	    _out << "Ice.Object";
	}
	_out << ", _" << name;
	if(!p->isLocal())
	{
	    _out << "Operations, _" << name;
	}
	_out << "OperationsNC";
	if(!bases.empty())
	{
	    ClassList::const_iterator q = bases.begin();
	    while(q != bases.end())
	    {
	        _out << ", " << fixId((*q)->scoped());
		q++;
	    }
	}
    }
    else
    {
	_out << sp << nl << "public ";
	if(p->isAbstract())
	{
	    _out << "abstract ";
	}
	_out << "class " << fixId(name) << " : ";

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
	    if(!p->isLocal())
	    {
	        _out << ", _" << name << "Operations";
	    }
	    _out << ", _" << name << "OperationsNC";
	}
	for(ClassList::const_iterator q = bases.begin(); q != bases.end(); ++q)
	{
	    if((*q)->isAbstract())
	    {
		_out << ", " << fixId((*q)->scoped());
	    }
	}
    }

    _out << sb;

    if(!p->isInterface())
    {
        if(p->hasDataMembers() && !p->hasOperations())
	{
	    _out << sp << nl << "#region Slice data members";
	}
	else if(p->hasDataMembers())
	{
	    _out << sp << nl << "#region Slice data members and operations";
	}
	else if(p->hasOperations())
	{
	    _out << sp << nl << "#region Slice operations";
	}
    }

    return true;
}

void
Slice::Gen::TypesVisitor::visitClassDefEnd(const ClassDefPtr& p)
{
    string name = fixId(p->name());
    DataMemberList dataMembers = p->dataMembers();

    if(!p->isInterface())
    {
        if(p->hasDataMembers() && !p->hasOperations())
	{
	    _out << sp << nl << "#endregion // Slice data members";
	}
	else if(p->hasDataMembers())
	{
	    _out << sp << nl << "#endregion // Slice data members and operations";
	}
	else if(p->hasOperations())
	{
	    _out << sp << nl << "#endregion // Slice operations";
	}
	writeInheritedOperations(p);
    }

    if(!p->isInterface() && !p->isLocal())
    {
	writeDispatch(p);

	DataMemberList members = p->dataMembers();
	DataMemberList::const_iterator d;

	_out << sp << nl << "#region Marshaling support";

	_out << sp << nl << "public override void __write(IceInternal.BasicStream __os)";
	_out << sb;
	_out << nl << "__os.writeTypeId(ice_staticId());";
	_out << nl << "__os.startWriteSlice();";
	for(d = members.begin(); d != members.end(); ++d)
	{
	    StringList metaData = (*d)->getMetaData();
	    writeMarshalUnmarshalCode(_out, (*d)->type(),
	                              fixId((*d)->name(), DotNet::ICloneable, true),
				      true, false, false);
	}
	_out << nl << "__os.endWriteSlice();";
	_out << nl << "base.__write(__os);";
	_out << eb;

	DataMemberList allClassMembers = p->allClassDataMembers();
	if(allClassMembers.size() != 0)
	{
	    _out << sp << nl << "public sealed ";
	    ClassList bases = p->bases();
	    if(!bases.empty() && !bases.front()->isInterface() && bases.front()->declaration()->usesClasses())
	    {
	    	_out << "new ";
	    }
	    _out << "class __Patcher : IceInternal.Patcher";
	    _out << sb;
	    _out << sp << nl << "internal __Patcher(Ice.ObjectImpl instance";
	    if(allClassMembers.size() > 1)
	    {
	    	_out << ", int member";
	    }
	    _out << ")";
	    _out << sb;
	    _out << nl << "_instance = (" << name << ")instance;";
	    if(allClassMembers.size() > 1)
	    {
		_out << nl << "_member = member;";
	    }
	    _out << eb;

	    _out << sp << nl << "public override void patch(Ice.Object v)";
	    _out << sb;
	    if(allClassMembers.size() > 1)
	    {
		_out << nl << "switch(_member)";
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
		string memberName = fixId((*d)->name(), DotNet::ICloneable, true);
		string memberType = typeToString((*d)->type());
		_out << nl << "_type = typeof(" << memberType << ");";
		_out << nl << "_instance." << memberName << " = (" << memberType << ")v;";
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

	    _out << sp << nl << "private " << name << " _instance;";
	    if(allClassMembers.size() > 1)
	    {
		_out << nl << "private int _member;";
	    }
	    _out << eb;
	}

	_out << sp << nl << "public override void __read(IceInternal.BasicStream __is, bool __rid)";
	_out << sb;
	_out << nl << "if(__rid)";
	_out << sb;
	_out << nl << "string myId = __is.readTypeId();";
	_out << eb;
	_out << nl << "__is.startReadSlice();";
	DataMemberList classMembers = p->classDataMembers();
	int classMemberCount = static_cast<int>(allClassMembers.size() - classMembers.size());
	for(d = members.begin(); d != members.end(); ++d)
	{
	    StringList metaData = (*d)->getMetaData();
	    ostringstream patchParams;
	    patchParams << "this";
	    BuiltinPtr builtin = BuiltinPtr::dynamicCast((*d)->type());
	    if((builtin && builtin->kind() == Builtin::KindObject) || ClassDeclPtr::dynamicCast((*d)->type()))
	    {
		if(classMembers.size() > 1 || allClassMembers.size() > 1)
		{
		    patchParams << ", " << classMemberCount++;
		}
	    }
	    writeMarshalUnmarshalCode(_out, (*d)->type(),
	                              fixId((*d)->name(), DotNet::ICloneable, true),
				      false, false, false, patchParams.str());
	}
	_out << nl << "__is.endReadSlice();";
	_out << nl << "base.__read(__is, true);";
	_out << eb;

	//
	// Write streaming API.
	//
	if(_stream)
	{
	    _out << sp << nl << "public override void __write(Ice.OutputStream __out)";
	    _out << sb;
	    _out << nl << "__out.writeTypeId(ice_staticId());";
	    _out << nl << "__out.startSlice();";
	    for(d = members.begin(); d != members.end(); ++d)
	    {
		StringList metaData = (*d)->getMetaData();
		writeMarshalUnmarshalCode(_out, (*d)->type(),
					  fixId((*d)->name(), DotNet::ICloneable, true),
					  true, true, false);
	    }
	    _out << nl << "__out.endSlice();";
	    _out << nl << "base.__write(__out);";
	    _out << eb;

	    _out << sp << nl << "public override void __read(Ice.InputStream __in, bool __rid)";
	    _out << sb;
	    _out << nl << "if(__rid)";
	    _out << sb;
	    _out << nl << "string myId = __in.readTypeId();";
	    _out << eb;
	    _out << nl << "__in.startSlice();";
	    for(d = members.begin(); d != members.end(); ++d)
	    {
		StringList metaData = (*d)->getMetaData();
		ostringstream patchParams;
		patchParams << "this";
		BuiltinPtr builtin = BuiltinPtr::dynamicCast((*d)->type());
		if((builtin && builtin->kind() == Builtin::KindObject) || ClassDeclPtr::dynamicCast((*d)->type()))
		{
		    if(classMembers.size() > 1 || allClassMembers.size() > 1)
		    {
			patchParams << ", " << classMemberCount++;
		    }
		}
		writeMarshalUnmarshalCode(_out, (*d)->type(),
					  fixId((*d)->name(), DotNet::ICloneable, true),
					  false, true, false, patchParams.str());
	    }
	    _out << nl << "__in.endSlice();";
	    _out << nl << "base.__read(__in, true);";
	    _out << eb;
	}
	else
	{
	    //
	    // Emit placeholder functions to catch errors.
	    //
            string scoped = p->scoped();
	    _out << sp << nl << "public override void __write(Ice.OutputStream __out)";
	    _out << sb;
	    _out << nl << "Ice.MarshalException ex = new Ice.MarshalException();";
	    _out << nl << "ex.reason = \"type " << scoped.substr(2) << " was not generated with stream support\";";
	    _out << nl << "throw ex;";
	    _out << eb;

	    _out << sp << nl << "public override void __read(Ice.InputStream __in, bool __rid)";
	    _out << sb;
	    _out << nl << "Ice.MarshalException ex = new Ice.MarshalException();";
	    _out << nl << "ex.reason = \"type " << scoped.substr(2) << " was not generated with stream support\";";
	    _out << nl << "throw ex;";
	    _out << eb;
	}

	_out << sp << nl << "#endregion"; // Marshalling support
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
    bool amd = !isLocal && (classDef->hasMetaData("amd") || p->hasMetaData("amd"));

    string name = p->name();
    ParamDeclList paramList = p->parameters();
    vector<string> params;
    vector<string> args;
    string retS;

    if(!amd)
    {
	params = getParams(p);
	args = getArgs(p);
	name = fixId(name, DotNet::ICloneable, true);
	retS = typeToString(p->returnType());
    }
    else
    {
	params = getParamsAsync(p, true);
	args = getArgsAsync(p);
	retS = "void";
	name = name + "_async";
    }

    _out << sp << nl << "public ";
    if(isLocal)
    {
        _out << "abstract ";
    }
    _out << retS << " " << name << spar << params << epar;
    if(isLocal)
    {
        _out << ";";
    }
    else
    {
	_out << sb;
	_out << nl;
	if(!amd && p->returnType())
	{
	    _out << "return ";
	}
	_out << name << spar << args << "Ice.ObjectImpl.defaultCurrent" << epar << ';';
	_out << eb;
    }

    if(!isLocal)
    {
	_out << nl << "public abstract " << retS << " " << name
	     << spar << params << "Ice.Current __current" << epar << ';';
    }
}

void
Slice::Gen::TypesVisitor::visitSequence(const SequencePtr& p)
{
    //
    // No need to generate anything if the sequence is mapped as an array.
    //
    if(!p->hasMetaData("cs:collection"))
    {
        return;
    }

    string name = fixId(p->name());
    string s = typeToString(p->type());
    bool isValue = isValueType(p->type());

    _out << sp << nl << "public class " << name
         << " : _System.Collections.CollectionBase, _System.ICloneable";
    _out << sb;

    _out << sp << nl << "#region Constructors";

    _out << sp << nl << "public " << name << "()";
    _out << sb;
    _out << eb;

    _out << sp << nl << "public " << name << "(int capacity)";
    _out << sb;
    _out << nl << "InnerList.Capacity = capacity;";
    _out << eb;

    _out << sp << nl << "public " << name << "(" << s << "[] __a)";
    _out << sb;
    _out << nl << "InnerList.AddRange(__a);";
    _out << eb;

    _out << sp << nl << "#endregion"; // Constructors

    _out << sp << nl << "#region Array copy and conversion";

    _out << sp << nl << "public void CopyTo(" << s << "[] __a)";
    _out << sb;
    _out << nl << "InnerList.CopyTo(__a);";
    _out << eb;

    _out << sp << nl << "public void CopyTo(" << s << "[] __a, int __i)";
    _out << sb;
    _out << nl << "InnerList.CopyTo(__a, __i);";
    _out << eb;

    _out << sp << nl << "public void CopyTo(int __i, " << s << "[] __a, int __ai, int __c)";
    _out << sb;
    _out << nl << "InnerList.CopyTo(__i, __a, __ai, __c);";
    _out << eb;

    _out << sp << nl << "public " << s << "[] ToArray()";
    _out << sb;
    _out << nl << s << "[] __a = new " << toArrayAlloc(s + "[]", "InnerList.Count") << ';';
    _out << nl << "InnerList.CopyTo(__a, 0);";
    _out << nl << "return __a;";
    _out << eb;

    _out << sp << nl << "#endregion"; // Array copy and conversion

    _out << sp << nl << "#region AddRange members";

    _out << sp << nl << "public void AddRange(" << name << " __s)";
    _out << sb;
    _out << nl << "InnerList.AddRange(__s);";
    _out << eb;

    _out << sp << nl << "public void AddRange(" << s << "[] __a)";
    _out << sb;
    _out << nl << "InnerList.AddRange(__a);";
    _out << eb;

    _out << sp << nl << "#endregion"; // AddRange members

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

    _out << sp << nl << "#region ICollection members";

    _out << sp << nl << "public bool IsSynchronized";
    _out << sb;
    _out << nl << "get";
    _out << sb;
    _out << nl << "return false;";
    _out << eb;
    _out << eb;

    _out << sp << nl << "public object SyncRoot";
    _out << sb;
    _out << nl << "get";
    _out << sb;
    _out << nl << "return this;";
    _out << eb;
    _out << eb;

    _out << sp << nl << "#endregion"; // ICollection members

    _out << sp << nl << "#region IList members";

    _out << sp << nl << "public bool IsFixedSize";
    _out << sb;
    _out << nl << "get";
    _out << sb;
    _out << nl << "return false;";
    _out << eb;
    _out << eb;

    _out << sp << nl << "public bool IsReadOnly";
    _out << sb;
    _out << nl << "get";
    _out << sb;
    _out << nl << "return false;";
    _out << eb;
    _out << eb;

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

    _out << sp << nl << "#endregion"; // IList members

    _out << sp << nl << "#region ICloneable members";

    _out << sp << nl << "public object Clone()";
    _out << sb;
    _out << nl << "return MemberwiseClone();";
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
	_out << nl << "if((object)InnerList[i] != null)";
	_out << sb;
    }
    _out << nl << "hash = 5 * hash + InnerList[i].GetHashCode();";
    if(!isValue)
    {
	_out << eb;
    }
    _out << eb;
    _out << nl << "return hash;";
    _out << eb;

    _out << sp << nl << "public override bool Equals(object other)";
    _out << sb;
    _out << nl << "if(object.ReferenceEquals(this, other))";
    _out << sb;
    _out << nl << "return true;";
    _out << eb;
    _out << nl << "if(!(other is " << name << "))";
    _out << sb;
    _out << nl << "return false;";
    _out << eb;
    _out << nl << "if(Count != ((" << name << ")other).Count)";
    _out << sb;
    _out << nl << "return false;";
    _out << eb;
    _out << nl << "for(int __i = 0; __i < Count; ++__i)";
    _out << sb;
    if(!isValue)
    {
	_out << nl << "if(InnerList[__i] == null && ((" << name << ")other)[__i] != null)";
	_out << sb;
	_out << nl << "return false;";
	_out << eb;
    }
    _out << nl << "if(!((" << s << ")(InnerList[__i])).Equals(((" << name << ")other)[__i]))";
    _out << sb;
    _out << nl << "return false;";
    _out << eb;
    _out << eb;
    _out << nl << "return true;";
    _out << eb;

    _out << sp << nl << "#endregion"; // Object members

    _out << sp << nl << "#region Comparison members";

    _out << sp << nl << "public static bool Equals(" << name << " __lhs, " << name << " __rhs)";
    _out << sb;
    _out << nl << "return object.ReferenceEquals(__lhs, null)";
    _out << nl << "           ? object.ReferenceEquals(__rhs, null)";
    _out << nl << "           : __lhs.Equals(__rhs);";
    _out << eb;

    _out << sp << nl << "public static bool operator==(" << name << " __lhs, " << name << " __rhs)";
    _out << sb;
    _out << nl << "return Equals(__lhs, __rhs);";
    _out << eb;

    _out << sp << nl << "public static bool operator!=(" << name << " __lhs, " << name << " __rhs)";
    _out << sb;
    _out << nl << "return !Equals(__lhs, __rhs);";
    _out << eb;

    _out << sp << nl << "#endregion"; // Comparison members

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

    if(!p->dataMembers().empty())
    {
	_out << sp << nl << "#region Slice data members";
    }

    return true;
}

void
Slice::Gen::TypesVisitor::visitExceptionEnd(const ExceptionPtr& p)
{
    string name = fixId(p->name());

    DataMemberList dataMembers = p->dataMembers();
    DataMemberList::const_iterator q;

    if(!dataMembers.empty())
    {
	_out << sp << nl << "#endregion"; // Slice data members
    }

    _out << sp << nl << "#region Constructors";

    _out << sp << nl << "private static readonly string _dflt = \"" << name << "\";";

    _out << sp << nl << "public " << name << "() : base(_dflt)";
    _out << sb;
    _out << eb;

    _out << sp << nl << "public " << name << "(string __m) : base(__m)";
    _out << sb;
    _out << eb;

    _out << sp << nl << "public " << name << "(_System.Exception __ex) : base(_dflt, __ex)";
    _out << sb;
    _out << eb;

    _out << sp << nl << "public " << name << "(string __m, _System.Exception __ex) : base(__m, __ex)";
    _out << sb;
    _out << eb;

    _out << sp << nl << "#endregion"; // Constructors

    _out << sp << nl << "#region Object members";

    _out << sp << nl << "public override int GetHashCode()";
    _out << sb;
    _out << nl << "int __h = 0;";
    for(q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        string memberName = fixId((*q)->name(), DotNet::ApplicationException);
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
        string memberName = fixId((*q)->name(), DotNet::ApplicationException);
	_out << nl << "if(!" << memberName << ".Equals(((" << name << ")__other)." << memberName << "))";
	_out << sb;
	_out << nl << "return false;";
	_out << eb;
    }
    _out << nl << "return true;";
    _out << eb;

    _out << sp << nl << "#endregion"; // Object members

    _out << sp << nl << "#region Comparison members";

    _out << sp << nl << "public static bool Equals(" << name << " __lhs, " << name << " __rhs)";
    _out << sb;
    _out << nl << "return object.ReferenceEquals(__lhs, null)";
    _out << nl << "           ? object.ReferenceEquals(__rhs, null)";
    _out << nl << "           : __lhs.Equals(__rhs);";
    _out << eb;

    _out << sp << nl << "public static bool operator==(" << name << " __lhs, " << name << " __rhs)";
    _out << sb;
    _out << nl << "return Equals(__lhs, __rhs);";
    _out << eb;

    _out << sp << nl << "public static bool operator!=(" << name << " __lhs, " << name << " __rhs)";
    _out << sb;
    _out << nl << "return !Equals(__lhs, __rhs);";
    _out << eb;

    _out << sp << nl << "#endregion"; // Comparison members

    if(!p->isLocal())
    {
        _out << sp << nl << "#region Marshaling support";

        string scoped = p->scoped();
        ExceptionPtr base = p->base();


        _out << sp << nl << "public override void __write(IceInternal.BasicStream __os)";
        _out << sb;
	_out << nl << "__os.writeString(\"" << scoped << "\");";
	_out << nl << "__os.startWriteSlice();";
        for(q = dataMembers.begin(); q != dataMembers.end(); ++q)
        {
            writeMarshalUnmarshalCode(_out, (*q)->type(),
	                              fixId((*q)->name(), DotNet::ApplicationException),
				      true, false, false);
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
	    _out << sp << nl << "public sealed ";
	    if(base && base->usesClasses())
	    {
	    	_out << "new ";
	    }
	    _out << "class __Patcher : IceInternal.Patcher";
	    _out << sb;
	    _out << sp << nl << "internal __Patcher(Ice.Exception instance";
	    if(allClassMembers.size() > 1)
	    {
	        _out << ", int member";
	    }
	    _out << ")";
	    _out << sb;
	    _out << nl << "_instance = (" << name << ")instance;";
	    if(allClassMembers.size() > 1)
	    {
		_out << nl << "_member = member;";
	    }
	    _out << eb;

	    _out << sp << nl << "public override void patch(Ice.Object v)";
	    _out << sb;
	    if(allClassMembers.size() > 1)
	    {
		_out << nl << "switch(_member)";
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
		string memberName = fixId((*q)->name(), DotNet::ApplicationException);
		string memberType = typeToString((*q)->type());
		_out << nl << "_type = typeof(" << memberType << ");";
		_out << nl << "_instance." << memberName << " = (" << memberType << ")v;";
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

	    _out << sp << nl << "private " << name << " _instance;";
	    if(allClassMembers.size() > 1)
	    {
		_out << nl << "private int _member;";
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
	DataMemberList classMembers = p->classDataMembers();
	int classMemberCount = static_cast<int>(allClassMembers.size() - classMembers.size());
        for(q = dataMembers.begin(); q != dataMembers.end(); ++q)
        {
	    ostringstream patchParams;
	    patchParams << "this";
	    BuiltinPtr builtin = BuiltinPtr::dynamicCast((*q)->type());
	    if((builtin && builtin->kind() == Builtin::KindObject) || ClassDeclPtr::dynamicCast((*q)->type()))
	    {
		if(classMembers.size() > 1 || allClassMembers.size() > 1)
		{
		    patchParams << ", " << classMemberCount++;
		}
	    }
            writeMarshalUnmarshalCode(_out, (*q)->type(),
	                              fixId((*q)->name(), DotNet::ApplicationException),
				      false, false, false, patchParams.str());
        }
	_out << nl << "__is.endReadSlice();";
        if(base)
        {
            _out << nl << "base.__read(__is, true);";
        }
        _out << eb;

	if(_stream)
	{
	    _out << sp << nl << "public override void __write(Ice.OutputStream __out)";
	    _out << sb;
	    _out << nl << "__out.writeString(\"" << scoped << "\");";
	    _out << nl << "__out.startSlice();";
	    for(q = dataMembers.begin(); q != dataMembers.end(); ++q)
	    {
		writeMarshalUnmarshalCode(_out, (*q)->type(),
					  fixId((*q)->name(), DotNet::ApplicationException),
					  true, true, false);
	    }
	    _out << nl << "__out.endSlice();";
	    if(base)
	    {
		_out << nl << "base.__write(__out);";
	    }
	    _out << eb;

	    _out << sp << nl << "public override void __read(Ice.InputStream __in, bool __rid)";
	    _out << sb;
	    _out << nl << "if(__rid)";
	    _out << sb;
	    _out << nl << "string myId = __in.readString();";
	    _out << eb;
	    _out << nl << "__in.startSlice();";
	    classMemberCount = static_cast<int>(allClassMembers.size() - classMembers.size());
	    for(q = dataMembers.begin(); q != dataMembers.end(); ++q)
	    {
		ostringstream patchParams;
		patchParams << "this";
		BuiltinPtr builtin = BuiltinPtr::dynamicCast((*q)->type());
		if((builtin && builtin->kind() == Builtin::KindObject) || ClassDeclPtr::dynamicCast((*q)->type()))
		{
		    if(classMembers.size() > 1 || allClassMembers.size() > 1)
		    {
			patchParams << ", " << classMemberCount++;
		    }
		}
		writeMarshalUnmarshalCode(_out, (*q)->type(),
					  fixId((*q)->name(), DotNet::ApplicationException),
					  false, true, false, patchParams.str());
	    }
	    _out << nl << "__in.endSlice();";
	    if(base)
	    {
		_out << nl << "base.__read(__in, true);";
	    }
	    _out << eb;
	}
	else
	{
	    //
	    // Emit placeholder functions to catch errors.
	    //
            string scoped = p->scoped();
	    _out << sp << nl << "public override void __write(Ice.OutputStream __out)";
	    _out << sb;
	    _out << nl << "Ice.MarshalException ex = new Ice.MarshalException();";
	    _out << nl << "ex.reason = \"exception " << scoped.substr(2) << " was not generated with stream support\";";
	    _out << nl << "throw ex;";
	    _out << eb;

	    _out << sp << nl << "public override void __read(Ice.InputStream __in, bool __rid)";
	    _out << sb;
	    _out << nl << "Ice.MarshalException ex = new Ice.MarshalException();";
	    _out << nl << "ex.reason = \"exception " << scoped.substr(2) << " was not generated with stream support\";";
	    _out << nl << "throw ex;";
	    _out << eb;
	}

	if(!base || base && !base->usesClasses())
	{
	    _out << sp << nl << "public override bool __usesClasses()";
	    _out << sb;
	    _out << nl << "return true;";
	    _out << eb;
	}

        _out << sp << nl << "#endregion"; // Marshalling support
    }

    _out << eb;
}

bool
Slice::Gen::TypesVisitor::visitStructStart(const StructPtr& p)
{
    string name = fixId(p->name());

    if(!p->isLocal() && _stream)
    {
        _out << sp << nl << "public sealed class " << p->name() << "Helper";
	_out << sb;

	_out << sp << nl << "public static void write(Ice.OutputStream __out, " << name << " __v)";
	_out << sb;
	_out << nl << "__v.__write(__out);";
	_out << eb;

	_out << sp << nl << "public static " << name << " read(Ice.InputStream __in)";
	_out << sb;
	_out << nl << name << " __v = new " << name << "();";
	_out << nl << "__v.__read(__in);";
	_out << nl << "return __v;";
	_out << eb;

	_out << eb;
    }

    if(p->hasMetaData("cs:class"))
    {
	_out << sp << nl << "public class " << name << " : _System.ICloneable";
    }
    else
    {
	_out << sp << nl << "public struct " << name;
    }
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

    _out << sp << nl << "#endregion"; // Slice data members

    bool isClass = p->hasMetaData("cs:class");
    if(isClass)
    {
	_out << sp << nl << "#region ICloneable members";

	_out << sp << nl << "public object Clone()";
	_out << sb;
	_out << nl << "return MemberwiseClone();";
	_out << eb;

	_out << sp << nl << "#endregion"; // ICloneable members
    }

    _out << sp << nl << "#region Object members";

    _out << sp << nl << "public override int GetHashCode()";
    _out << sb;
    _out << nl << "int __h = 0;";
    for(q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        string memberName = fixId((*q)->name(), isClass ? DotNet::ICloneable : 0);
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

    _out << sp << nl << "public override bool Equals(object __other)";
    _out << sb;
    _out << nl << "if(object.ReferenceEquals(this, __other))";
    _out << sb;
    _out << nl << "return true;";
    _out << eb;
    _out << nl << "if(!(__other is " << name << "))";
    _out << sb;
    _out << nl << "return false;";
    _out << eb;
    for(q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        string memberName = fixId((*q)->name(), isClass ? DotNet::ICloneable : 0);
	if(!isValueType((*q)->type()))
	{
	    _out << nl << "if(" << memberName << " == null)";
	    _out << sb;
	    _out << nl << "if(((" << name << ")__other)." << memberName << " != null)";
	    _out << sb;
	    _out << nl << "return false;";
	    _out << eb;
	    _out << eb;
	    _out << nl << "else";
	    _out << sb;
	    _out << nl << "if(!(" << memberName << ".Equals(((" << name << ")__other)." << memberName << ")))";
	    _out << sb;
	    _out << nl << "return false;";
	    _out << eb;
	    _out << eb;
	}
	else
	{
	    _out << nl << "if(!(" << memberName << ".Equals(((" << name << ")__other)." << memberName << ")))";
	    _out << sb;
	    _out << nl << "return false;";
	    _out << eb;
	}
    }
    _out << nl << "return true;";
    _out << eb;

    _out << sp << nl << "#endregion"; // Object members

    _out << sp << nl << "#region Comparison members";

    _out << sp << nl << "public static bool Equals(" << name << " __lhs, " << name << " __rhs)";
    _out << sb;
    _out << nl << "return object.ReferenceEquals(__lhs, null)";
    _out << nl << "           ? object.ReferenceEquals(__rhs, null)";
    _out << nl << "           : __lhs.Equals(__rhs);";
    _out << eb;

    _out << sp << nl << "public static bool operator==(" << name << " __lhs, " << name << " __rhs)";
    _out << sb;
    _out << nl << "return Equals(__lhs, __rhs);";
    _out << eb;

    _out << sp << nl << "public static bool operator!=(" << name << " __lhs, " << name << " __rhs)";
    _out << sb;
    _out << nl << "return !Equals(__lhs, __rhs);";
    _out << eb;

    _out << sp << nl << "#endregion"; // Comparison members

    if(!p->isLocal())
    {
        _out << sp << nl << "#region Marshalling support";

        _out << sp << nl << "public void __write(IceInternal.BasicStream __os)";
        _out << sb;
	for(q = dataMembers.begin(); q != dataMembers.end(); ++q)
	{
	    writeMarshalUnmarshalCode(_out, (*q)->type(),
	                              fixId((*q)->name(), isClass ? DotNet::ICloneable : 0),
				      true, false, false);
	}
        _out << eb;

	DataMemberList classMembers = p->classDataMembers();

	bool patchStruct = !isClass && classMembers.size() != 0;

	if(classMembers.size() != 0)
	{
	    _out << sp << nl << "public sealed class __Patcher : IceInternal.Patcher";
	    _out << sb;
	    _out << sp << nl << "internal __Patcher(" << name;
	    if(patchStruct)
	    {
	        _out << ".__PatchMembers";
	    }
	    _out << " instance";
	    if(classMembers.size() > 1)
	    {
	    	_out << ", int member";
	    }
	    _out << ")";
	    _out << sb;
	    _out << nl << "_instance = instance;";
	    if(classMembers.size() > 1)
	    {
		_out << nl << "_member = member;";
	    }
	    _out << eb;

	    _out << sp << nl << "public override void patch(Ice.Object v)";
	    _out << sb;
	    if(classMembers.size() > 1)
	    {
		_out << nl << "switch(_member)";
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
		string memberType = typeToString((*q)->type());
		string memberName = fixId((*q)->name(), isClass ? DotNet::ICloneable : 0);
		_out << nl << "_type = typeof(" << memberType << ");";
		_out << nl << "_instance." << memberName << " = (" << memberType << ")v;";
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

	    _out << sp << nl << "private " << name;
	    if(patchStruct)
	    {
	        _out << ".__PatchMembers";
	    }
	    _out << " _instance;";
	    if(classMembers.size() > 1)
	    {
		_out << nl << "private int _member;";
	    }
	    _out << eb;

	    if(patchStruct)
	    {
		_out << sp << nl << "internal class __PatchMembers";
		_out << sb;
		for(q = classMembers.begin(); q != classMembers.end(); ++q)
		{
		    string memberType = typeToString((*q)->type());
		    string memberName = fixId((*q)->name(), isClass ? DotNet::ICloneable : 0);
		    _out << nl << "internal " << memberType << ' ' << memberName << ';';
		}
		_out << eb;

		_out << sp << nl << "private __PatchMembers _pm;";

		_out << sp << nl << "public void __patch()";
		_out << sb;
		for(q = classMembers.begin(); q != classMembers.end(); ++q)
		{
		    string memberName = fixId((*q)->name(), isClass ? DotNet::ICloneable : 0);
		    _out << nl << memberName << " = _pm." << memberName << ';';
		}
		_out << eb;
	    }
	}

        _out << sp << nl << "public void __read(IceInternal.BasicStream __is)";
        _out << sb;
	if(patchStruct)
	{
	    _out << nl << "if(_pm == null)";
	    _out << sb;
	    _out << nl << "_pm = new __PatchMembers();";
	    _out << eb;
	}
	int classMemberCount = 0;
        for(q = dataMembers.begin(); q != dataMembers.end(); ++q)
        {
	    ostringstream patchParams;
	    patchParams << (patchStruct ? "_pm" : "this");
	    BuiltinPtr builtin = BuiltinPtr::dynamicCast((*q)->type());
	    if((builtin && builtin->kind() == Builtin::KindObject) || ClassDeclPtr::dynamicCast((*q)->type()))
	    {
		if(classMembers.size() > 1)
		{
		    patchParams << ", " << classMemberCount++;
		}
	    }
            writeMarshalUnmarshalCode(_out, (*q)->type(),
	    			      fixId((*q)->name(), isClass ? DotNet::ICloneable : 0 ),
	                              false, false, false, patchParams.str());
        }
        _out << eb;

	if(_stream)
	{
	    _out << sp << nl << "public void __write(Ice.OutputStream __out)";
	    _out << sb;
	    for(q = dataMembers.begin(); q != dataMembers.end(); ++q)
	    {
		writeMarshalUnmarshalCode(_out, (*q)->type(),
					  fixId((*q)->name(), isClass ? DotNet::ICloneable : 0),
					  true, true, false);
	    }
	    _out << eb;

	    _out << sp << nl << "public void __read(Ice.InputStream __in)";
	    _out << sb;
	    if(patchStruct)
	    {
		_out << nl << "if(_pm == null)";
		_out << sb;
		_out << nl << "_pm = new __PatchMembers();";
		_out << eb;
	    }
	    classMemberCount = 0;
	    for(q = dataMembers.begin(); q != dataMembers.end(); ++q)
	    {
		ostringstream patchParams;
		patchParams << (patchStruct ? "_pm" : "this");
		BuiltinPtr builtin = BuiltinPtr::dynamicCast((*q)->type());
		if((builtin && builtin->kind() == Builtin::KindObject) || ClassDeclPtr::dynamicCast((*q)->type()))
		{
		    if(classMembers.size() > 1)
		    {
			patchParams << ", " << classMemberCount++;
		    }
		}
		writeMarshalUnmarshalCode(_out, (*q)->type(),
					  fixId((*q)->name(), isClass ? DotNet::ICloneable : 0 ),
					  false, true, false, patchParams.str());
	    }
	    _out << eb;
	}

        _out << sp << nl << "#endregion"; // Marshalling support
    }

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
         << " : _System.Collections.DictionaryBase, _System.ICloneable";
    _out << sb;

    _out << sp << nl << "#region " << name << " members";

    _out << sp << nl << "public void AddRange(" << name << " __d)";
    _out << sb;
    _out << nl << "foreach(_System.Collections.DictionaryEntry e in __d)";
    _out << sb;
    _out << nl << "try";
    _out << sb;
    _out << nl << "InnerHashtable.Add(e.Key, e.Value);";
    _out << eb;
    _out << nl << "catch(_System.ArgumentException)";
    _out << sb;
    _out << nl << "// ignore";
    _out << eb;
    _out << eb;
    _out << eb;

    _out << sp << nl << "#endregion"; // <name> members

    _out << sp << nl << "#region IDictionary members";

    _out << sp << nl << "public bool IsFixedSize";
    _out << sb;
    _out << nl << "get";
    _out << sb;
    _out << nl << "return false;";
    _out << eb;
    _out << eb;

    _out << sp << nl << "public bool IsReadOnly";
    _out << sb;
    _out << nl << "get";
    _out << sb;
    _out << nl << "return false;";
    _out << eb;
    _out << eb;

    _out << sp << nl << "public _System.Collections.ICollection Keys";
    _out << sb;
    _out << nl << "get";
    _out << sb;
    _out << nl << "return InnerHashtable.Keys;";
    _out << eb;
    _out << eb;

    _out << sp << nl << "public _System.Collections.ICollection Values";
    _out << sb;
    _out << nl << "get";
    _out << sb;
    _out << nl << "return InnerHashtable.Values;";
    _out << eb;
    _out << eb;

    _out << sp << nl << "#region Indexer";

    _out << sp << nl << "public " << vs << " this[" << ks << " key]";
    _out << sb;
    _out << nl << "get";
    _out << sb;
    _out << nl << "return (" << vs << ")InnerHashtable[key];";
    _out << eb;

    _out << nl << "set";
    _out << sb;
    _out << nl << "InnerHashtable[key] = value;";
    _out << eb;
    _out << eb;

    _out << sp << nl << "#endregion"; // Indexer

    _out << sp << nl << "public void Add(" << ks << " key, " << vs << " value)";
    _out << sb;
    _out << nl << "InnerHashtable.Add(key, value);";
    _out << eb;

    _out << sp << nl << "public void Remove(" << ks << " key)";
    _out << sb;
    _out << nl << "InnerHashtable.Remove(key);";
    _out << eb;

    _out << sp << nl << "public bool Contains(" << ks << " key)";
    _out << sb;
    _out << nl << "return InnerHashtable.Contains(key);";
    _out << eb;

    _out << sp << nl << "#endregion"; // IDictionary members

    _out << sp << nl << "#region ICollection members";

    _out << sp << nl << "public bool IsSynchronized";
    _out << sb;
    _out << nl << "get";
    _out << sb;
    _out << nl << "return false;";
    _out << eb;
    _out << eb;

    _out << sp << nl << "public object SyncRoot";
    _out << sb;
    _out << nl << "get";
    _out << sb;
    _out << nl << "return this;";
    _out << eb;
    _out << eb;

    _out << sp << nl << "#endregion"; // ICollection members

    _out << sp << nl << "#region ICloneable members";

    _out << sp << nl << "public object Clone()";
    _out << sb;
    _out << nl << "return MemberwiseClone();";
    _out << eb;

    _out << sp << nl << "#endregion"; // ICloneable members

    _out << sp << nl << "#region Object members";

    _out << sp << nl << "public override int GetHashCode()";
    _out << sb;
    _out << nl << "int hash = 0;";
    _out << nl << "foreach(_System.Collections.DictionaryEntry e in InnerHashtable)";
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
    _out << nl << "if(object.ReferenceEquals(this, other))";
    _out << sb;
    _out << nl << "return true;";
    _out << eb;
    _out << nl << "if(!(other is " << name << "))";
    _out << sb;
    _out << nl << "return false;";
    _out << eb;
    _out << nl << "if(Count != ((" << name << ")other).Count)";
    _out << sb;
    _out << nl << "return false;";
    _out << eb;
    _out << nl << ks << "[] __klhs = new " << ks << "[Count];";
    _out << nl << "Keys.CopyTo(__klhs, 0);";
    _out << nl << "_System.Array.Sort(__klhs);";
    _out << nl << ks << "[] __krhs = new " << ks << "[((" << name << ")other).Count];";
    _out << nl << "((" << name << ")other).Keys.CopyTo(__krhs, 0);";
    _out << nl << "_System.Array.Sort(__krhs);";
    _out << nl << "for(int i = 0; i < Count; ++i)";
    _out << sb;
    _out << nl << "if(!__klhs[i].Equals(__krhs[i]))";
    _out << sb;
    _out << nl << "return false;";
    _out << eb;
    _out << eb;
    SequencePtr seq = SequencePtr::dynamicCast(p->valueType());
    bool valueIsArray = seq && !seq->hasMetaData("cs:collection");
    if(valueIsArray)
    {
	_out << nl << vs << "[] __vlhs = new " << toArrayAlloc(vs + "[]", "Count") << ';';
    }
    else
    {
	_out << nl << vs << "[] __vlhs = new " << vs << "[Count];";
    }
    _out << nl << "Values.CopyTo(__vlhs, 0);";
    _out << nl << "_System.Array.Sort(__vlhs);";
    string vrhsCount = "((" + name + ")other).Count";
    if(valueIsArray)
    {
	_out << nl << vs << "[] __vrhs = new " << toArrayAlloc(vs + "[]", vrhsCount) << ';';
    }
    else
    {
	_out << nl << vs << "[] __vrhs = new " << vs << '[' << vrhsCount << "];";
    }
    _out << nl << "((" << name << ")other).Values.CopyTo(__vrhs, 0);";
    _out << nl << "_System.Array.Sort(__vrhs);";
    _out << nl << "for(int i = 0; i < Count; ++i)";
    _out << sb;
    if(!valueIsValue)
    {
	_out << nl << "if(__vlhs[i] == null && __vrhs[i] != null)";
	_out << sb;
	_out << nl << "return false;";
	_out << eb;
    }
    _out << nl << "if(!__vlhs[i].Equals(__vrhs[i]))";
    _out << sb;
    _out << nl << "return false;";
    _out << eb;
    _out << eb;
    _out << nl << "return true;";
    _out << eb;

    _out << sp << nl << "#endregion"; // Object members

    _out << sp << nl << "#region Comparison members";

    _out << sp << nl << "public static bool Equals(" << name << " __lhs, " << name << " __rhs)";
    _out << sb;
    _out << nl << "return object.ReferenceEquals(__lhs, null)";
    _out << nl << "           ? object.ReferenceEquals(__rhs, null)";
    _out << nl << "           : __lhs.Equals(__rhs);";
    _out << eb;

    _out << sp << nl << "public static bool operator==(" << name << " __lhs, " << name << " __rhs)";
    _out << sb;
    _out << nl << "return Equals(__lhs, __rhs);";
    _out << eb;

    _out << sp << nl << "public static bool operator!=(" << name << " __lhs, " << name << " __rhs)";
    _out << sb;
    _out << nl << "return !Equals(__lhs, __rhs);";
    _out << eb;

    _out << sp << nl << "#endregion"; // Comparison members

    _out << eb;
}

void
Slice::Gen::TypesVisitor::visitEnum(const EnumPtr& p)
{
    string name = fixId(p->name());
    string scoped = fixId(p->scoped());
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

    if(_stream)
    {
	_out << sp << nl << "public sealed class " << name << "Helper";
	_out << sb;

	_out << sp << nl << "public static void write(Ice.OutputStream __out, " << scoped << " __v)";
	_out << sb;
	writeMarshalUnmarshalCode(_out, p, "__v", true, true, false);
	_out << eb;

	_out << sp << nl << "public static " << scoped << " read(Ice.InputStream __in)";
	_out << sb;
	_out << nl << scoped << " __v;";
	writeMarshalUnmarshalCode(_out, p, "__v", false, true, false);
	_out << nl << "return __v;";
	_out << eb;

        _out << eb;
    }
}

void
Slice::Gen::TypesVisitor::visitConst(const ConstPtr& p)
{
    string name = fixId(p->name());
    _out << sp << nl << "public abstract class " << name;
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
	    _out << typeToString(p->type()) << "." << fixId(p->value());
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
    int baseTypes = 0;
    bool isClass = false;
    ContainedPtr cont = ContainedPtr::dynamicCast(p->container());
    assert(cont);
    if(StructPtr::dynamicCast(cont) && cont->hasMetaData("cs:class"))
    {
        baseTypes = DotNet::ICloneable;
    }
    else if(ExceptionPtr::dynamicCast(cont))
    {
        baseTypes = DotNet::ApplicationException;
    }
    else if(ClassDefPtr::dynamicCast(cont))
    {
	baseTypes = DotNet::ICloneable;
	isClass = true;
    }
    _out << sp << nl << "public " << typeToString(p->type()) << " " << fixId(p->name(), baseTypes, isClass) << ";";
}

Slice::Gen::ProxyVisitor::ProxyVisitor(IceUtil::Output& out)
    : CsVisitor(out)
{
}

bool
Slice::Gen::ProxyVisitor::visitModuleStart(const ModulePtr& p)
{
    if(!p->hasNonLocalClassDecls())
    {
	return false;
    }

    _out << sp << nl << "namespace " << fixId(p->name());
    _out << sb;
    return true;
}

void
Slice::Gen::ProxyVisitor::visitModuleEnd(const ModulePtr&)
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

    string name = p->name();
    ClassList bases = p->bases();

    _out << sp << nl << "public interface " << name << "Prx : ";
    if(bases.empty())
    {
        _out << "Ice.ObjectPrx";
    }
    else
    {
	ClassList::const_iterator q = bases.begin();
	while(q != bases.end())
	{
	    _out << fixId((*q)->scoped() + "Prx");
	    if(++q != bases.end())
	    {
		_out << ", ";
	    }
	}
    }

    _out << sb;

    return true;
}

void
Slice::Gen::ProxyVisitor::visitClassDefEnd(const ClassDefPtr&)
{
    _out << eb;
}

void
Slice::Gen::ProxyVisitor::visitOperation(const OperationPtr& p)
{
    ClassDefPtr cl = ClassDefPtr::dynamicCast(p->container());
    string name = fixId(p->name(), DotNet::ICloneable, true);
    vector<string> params = getParams(p);

    _out << sp << nl << typeToString(p->returnType()) << " " << name << spar << params << epar << ';';

    _out << nl << typeToString(p->returnType()) << " " << name
         << spar << params << "Ice.Context __context" << epar << ';'; 

    if(cl->hasMetaData("ami") || p->hasMetaData("ami"))
    {
        vector<string> paramsAMI = getParamsAsync(p, false);

	//
	// Write two versions of the operation - with and without a
	// context parameter.
	//
	_out << sp;
	_out << nl << "void " << p->name() << "_async" << spar << paramsAMI << epar << ';';
	_out << nl << "void " << p->name() << "_async" << spar << paramsAMI << "Ice.Context __ctx" << epar << ';';
    }
}

bool
Slice::Gen::OpsVisitor::visitModuleStart(const ModulePtr& p)
{
    if(!p->hasAbstractClassDefs())
    {
        return false;
    }

    _out << sp << nl << "namespace " << fixId(p->name());
    _out << sb;
    return true;
}

void
Slice::Gen::OpsVisitor::visitModuleEnd(const ModulePtr&)
{
    _out << eb;
}

bool
Slice::Gen::OpsVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    //
    // Don't generate Operations interfaces for non-abstract classes.
    //
    if(!p->isAbstract())
    {
	return false;
    }

    if(!p->isLocal())
    {
	writeOperations(p, false);
    }
    writeOperations(p, true);

    return false;
}

void
Slice::Gen::OpsVisitor::writeOperations(const ClassDefPtr& p, bool noCurrent)
{
    string name = p->name();
    string scoped = fixId(p->scoped());
    ClassList bases = p->bases();
    string opIntfName = "Operations";
    if(noCurrent || p->isLocal())
    {
        opIntfName += "NC";
    }

    _out << sp << nl << "public interface _" << name << opIntfName;
    if((bases.size() == 1 && bases.front()->isAbstract()) || bases.size() > 1)
    {
        _out << " : ";
	ClassList::const_iterator q = bases.begin();
	bool first = true;
	while(q != bases.end())
	{
	    if((*q)->isAbstract())
	    {
	        if(!first)
		{
		    _out << ", ";
		}
		else
		{
		    first = false;
		}
		string s = (*q)->scoped();
		string::size_type pos = s.rfind("::");
		assert(pos != string::npos);
		s.insert(pos + 2, "_");
		s += "Operations";
		if(noCurrent)
		{
		    s += "NC";
		}
		_out << fixId(s);
	    }
	    ++q;
	}
    }
    _out << sb;

    OperationList ops = p->operations();
    OperationList::const_iterator r;
    for(r = ops.begin(); r != ops.end(); ++r)
    {
	OperationPtr op = *r;
	bool amd = !p->isLocal() && (p->hasMetaData("amd") || op->hasMetaData("amd"));
	string name = amd ? (op->name() + "_async") : fixId(op->name(), DotNet::ICloneable, true);

	TypePtr ret;
	vector<string> params;


	if(amd)
	{
	    params = getParamsAsync(op, true);
	}
	else
	{
	    params = getParams(op);
	    ret = op->returnType();
	}

	string retS = typeToString(ret);

	_out << sp << nl << retS << ' ' << name << spar << params;
	if(!noCurrent && !p->isLocal())
	{ 
	    _out << "Ice.Current __current";
	}
	_out << epar << ';';
    }

    _out << eb;
}

Slice::Gen::HelperVisitor::HelperVisitor(IceUtil::Output& out, bool stream)
    : CsVisitor(out), _stream(stream)
{
}

bool
Slice::Gen::HelperVisitor::visitModuleStart(const ModulePtr& p)
{
    if(!p->hasNonLocalClassDecls() && !p->hasNonLocalSequences() && !p->hasNonLocalDictionaries())
    {
        return false;
    }

    _out << sp << nl << "namespace " << fixId(p->name());
    _out << sb;
    return true;
}

void
Slice::Gen::HelperVisitor::visitModuleEnd(const ModulePtr&)
{
    _out << eb;
}

bool
Slice::Gen::HelperVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    if(p->isLocal())
        return false;

    string name = p->name();
    ClassList bases = p->bases();

    _out << sp << nl << "public sealed class " << name << "PrxHelper : Ice.ObjectPrxHelperBase, " << name << "Prx";
    _out << sb;

    OperationList ops = p->allOperations();

    if(!ops.empty())
    {
	_out << sp << nl << "#region Synchronous operations";
    }

    OperationList::const_iterator r;
    for(r = ops.begin(); r != ops.end(); ++r)
    {
        OperationPtr op = *r;
	string opName = fixId(op->name(), DotNet::ICloneable, true);
	TypePtr ret = op->returnType();
	string retS = typeToString(ret);

	vector<string> params = getParams(op);
	vector<string> args = getArgs(op);

	_out << sp << nl << "public " << retS << " " << opName << spar << params << epar;
	_out << sb;
	_out << nl;
	if(ret)
	{
	    _out << "return ";
	}
	_out << opName << spar << args << "__defaultContext()" << epar << ';';
	_out << eb;

	_out << sp << nl << "public " << retS << " " << opName << spar << params << "Ice.Context __context" << epar;
	_out << sb;

	_out << nl << "int __cnt = 0;";
	_out << nl << "while(true)";
	_out << sb;
	_out << nl << "try";
	_out << sb;
	if(op->returnsData())
	{
	    _out << nl << "__checkTwowayOnly(\"" << op->name() << "\");";
	}
	_out << nl << "Ice._ObjectDel __delBase = __getDelegate();";
	_out << nl << '_' << name << "Del __del = (_" << name << "Del)__delBase;";
	_out << nl;
	if(ret)
	{
	    _out << "return ";
	}
	_out << "__del." << opName << spar << args << "__context" << epar << ';';
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

	_out << eb;
    }

    if(!ops.empty())
    {
	_out << sp << nl << "#endregion"; // Synchronous operations
    }

    bool hasAsyncOps = false;

    for(r = ops.begin(); r != ops.end(); ++r)
    {
	OperationPtr op = *r;
	
	ClassDefPtr containingClass = ClassDefPtr::dynamicCast(op->container());
	if(containingClass->hasMetaData("ami") || op->hasMetaData("ami"))
	{
	    if(!hasAsyncOps)
	    {
		_out << sp << nl << "#region Asynchronous operations";
		hasAsyncOps = true;
	    }
	    vector<string> paramsAMI = getParamsAsync(op, false);
	    vector<string> argsAMI = getArgsAsync(op);

	    string opName = op->name();

	    //
	    // Write two versions of the operation - with and without a
	    // context parameter
	    //
	    _out << sp;
	    _out << nl << "public void " << opName << "_async" << spar << paramsAMI << epar;
	    _out << sb;
	    _out << nl << opName << "_async" << spar << argsAMI << "__defaultContext()" << epar << ';';
	    _out << eb;

	    _out << nl << "public void " << opName << "_async" << spar << paramsAMI << "Ice.Context __ctx" << epar;
	    _out << sb;
	    _out << nl << "__checkTwowayOnly(\"" << p->name() << "\");";
	    _out << nl << "__cb.__invoke" << spar << "this" << argsAMI << "__ctx" << epar << ';';
	    _out << eb;
	}
    }

    if(hasAsyncOps)
    {
	_out << sp << nl << "#endregion"; // Asynchronous operations
    }

    _out << sp << nl << "#region Checked and unchecked cast operations";

    _out << sp << nl << "public static " << name << "Prx checkedCast(Ice.ObjectPrx b)";
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

    _out << sp << nl << "public static " << name << "Prx checkedCast(Ice.ObjectPrx b, Ice.Context ctx)";
    _out << sb;
    _out << nl << "if(b == null)";
    _out << sb;
    _out << nl << "return null;";
    _out << eb;
    _out << nl << "if(b is " << name << "Prx)";
    _out << sb;
    _out << nl << "return (" << name << "Prx)b;";
    _out << eb;
    _out << nl << "if(b.ice_isA(\"" << p->scoped() << "\", ctx))";
    _out << sb;
    _out << nl << name << "PrxHelper h = new " << name << "PrxHelper();";
    _out << nl << "h.__copyFrom(b);";
    _out << nl << "return h;";
    _out << eb;
    _out << nl << "return null;";
    _out << eb;

    _out << sp << nl << "public static " << name << "Prx checkedCast(Ice.ObjectPrx b, string f)";
    _out << sb;
    _out << nl << "if(b == null)";
    _out << sb;
    _out << nl << "return null;";
    _out << eb;
    _out << nl << "Ice.ObjectPrx bb = b.ice_newFacet(f);";
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

    _out << sp << nl << "public static " << name << "Prx checkedCast(Ice.ObjectPrx b, string f, Ice.Context ctx)";
    _out << sb;
    _out << nl << "if(b == null)";
    _out << sb;
    _out << nl << "return null;";
    _out << eb;
    _out << nl << "Ice.ObjectPrx bb = b.ice_newFacet(f);";
    _out << nl << "try";
    _out << sb;
    _out << nl << "if(bb.ice_isA(\"" << p->scoped() << "\", ctx))";
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

    _out << sp << nl << "public static " << name << "Prx uncheckedCast(Ice.ObjectPrx b)";
    _out << sb;
    _out << nl << "if(b == null)";
    _out << sb;
    _out << nl << "return null;";
    _out << eb;
    _out << nl << name << "PrxHelper h = new " << name << "PrxHelper();";
    _out << nl << "h.__copyFrom(b);";
    _out << nl << "return h;";
    _out << eb;

    _out << sp << nl << "public static " << name << "Prx uncheckedCast(Ice.ObjectPrx b, string f)";
    _out << sb;
    _out << nl << "if(b == null)";
    _out << sb;
    _out << nl << "return null;";
    _out << eb;
    _out << nl << "Ice.ObjectPrx bb = b.ice_newFacet(f);";
    _out << nl << name << "PrxHelper h = new " << name << "PrxHelper();";
    _out << nl << "h.__copyFrom(bb);";
    _out << nl << "return h;";
    _out << eb;

    _out << sp << nl << "#endregion"; // Checked and unchecked cast operations

    _out << sp << nl << "#region Marshaling support";

    _out << sp << nl << "protected override Ice._ObjectDelM __createDelegateM()";
    _out << sb;
    _out << nl << "return new _" << name << "DelM();";
    _out << eb;

    _out << sp << nl << "protected override Ice._ObjectDelD __createDelegateD()";
    _out << sb;
    _out << nl << "return new _" << name << "DelD();";
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

    if(_stream)
    {
	_out << sp << nl << "public static void write(Ice.OutputStream __out, " << name << "Prx __v)";
	_out << sb;
	_out << nl << "__out.writeProxy(__v);";
	_out << eb;

	_out << sp << nl << "public static " << name << "Prx read(Ice.InputStream __in)";
	_out << sb;
	_out << nl << "Ice.ObjectPrx proxy = __in.readProxy();";
	_out << nl << "if(proxy != null)";
	_out << sb;
	_out << nl << name << "PrxHelper result = new " << name << "PrxHelper();";
	_out << nl << "result.__copyFrom(proxy);";
	_out << nl << "return result;";
	_out << eb;
	_out << nl << "return null;";
	_out << eb;
    }

    _out << sp << nl << "#endregion"; // Marshaling support

    return true;
}

void
Slice::Gen::HelperVisitor::visitClassDefEnd(const ClassDefPtr&)
{
    _out << eb;
}

void
Slice::Gen::HelperVisitor::visitSequence(const SequencePtr& p)
{
    //
    // Don't generate helper for a sequence of a local type
    //
    if(p->isLocal())
    {
        return;
    }

    string name = fixId(p->name());
    string typeS = typeToString(p);

    _out << sp << nl << "public sealed class " << name << "Helper";
    _out << sb;

    _out << nl << "public static void write(IceInternal.BasicStream __os, " << typeS << " __v)";
    _out << sb;
    writeSequenceMarshalUnmarshalCode(_out, p, "__v", true, false);
    _out << eb;

    _out << sp << nl << "public static " << typeS << " read(IceInternal.BasicStream __is)";
    _out << sb;
    _out << nl << typeS << " __v;";
    writeSequenceMarshalUnmarshalCode(_out, p, "__v", false, false);
    _out << nl << "return __v;";
    _out << eb;

    if(_stream)
    {
	_out << sp << nl << "public static void write(Ice.OutputStream __out, " << typeS << " __v)";
	_out << sb;
	writeSequenceMarshalUnmarshalCode(_out, p, "__v", true, true);
	_out << eb;

	_out << sp << nl << "public static " << typeS << " read(Ice.InputStream __in)";
	_out << sb;
	_out << nl << typeS << " __v;";
	writeSequenceMarshalUnmarshalCode(_out, p, "__v", false, true);
	_out << nl << "return __v;";
	_out << eb;
    }

    _out << eb;
}

void
Slice::Gen::HelperVisitor::visitDictionary(const DictionaryPtr& p)
{
    //
    // Don't generate helper for a dictionary containing a local type
    //
    if(p->isLocal())
    {
        return;
    }

    TypePtr key = p->keyType();
    TypePtr value = p->valueType();

    string name = fixId(p->name());
    string keyS = typeToString(key);
    string valueS = typeToString(value);

    _out << sp << nl << "public sealed class " << name << "Helper";
    _out << sb;

    _out << nl << "public static void write(IceInternal.BasicStream __os, " << name << " __v)";
    _out << sb;
    _out << nl << "if(__v == null)";
    _out << sb;
    _out << nl << "__os.writeSize(0);";
    _out << eb;
    _out << nl << "else";
    _out << sb;
    _out << nl << "__os.writeSize(__v.Count);";
    _out << nl << "foreach(_System.Collections.DictionaryEntry __e in __v)";
    _out << sb;
    string keyArg = "((" + keyS + ")__e.Key)";
    writeMarshalUnmarshalCode(_out, key, keyArg, true, false, false);
    string valueArg = "((" + valueS + ")__e.Value)";
    writeMarshalUnmarshalCode(_out, value, valueArg, true, false, false);
    _out << eb;
    _out << eb;
    _out << eb;

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(value);
    bool hasClassValue = (builtin && builtin->kind() == Builtin::KindObject) || ClassDeclPtr::dynamicCast(value);
    if(hasClassValue)
    {
	_out << sp << nl << "public sealed class __Patcher : IceInternal.Patcher";
	_out << sb;
	_out << sp << nl << "internal __Patcher(" << name << " m, " << keyS << " key)";
	_out << sb;
	_out << nl << "_m = m;";
	_out << nl << "_key = key;";
	_out << eb;

	_out << sp << nl << "public override void" << nl << "patch(Ice.Object v)";
	_out << sb;
	_out << nl << "_type = typeof(" << typeToString(p->valueType()) << ");";
	_out << nl << "_m[_key] = (" << valueS << ")v;";
	_out << eb;

	_out << sp << nl << "private " << name << " _m;";
	_out << nl << "private " << keyS << " _key;";
	_out << eb;
    }

    _out << sp << nl << "public static " << name << " read(IceInternal.BasicStream __is)";
    _out << sb;
    _out << nl << "int __sz = __is.readSize();";
    _out << nl << name << " __r = new " << name << "();";
    _out << nl << "for(int __i = 0; __i < __sz; ++__i)";
    _out << sb;
    _out << nl << keyS << " __k;";
    writeMarshalUnmarshalCode(_out, key, "__k", false, false, false);
    if(!hasClassValue)
    {
	_out << nl << valueS << " __v;";
    }
    writeMarshalUnmarshalCode(_out, value, "__v", false, false, false, "__r, __k");
    if(!hasClassValue)
    {
	_out << nl << "__r[__k] = __v;";
    }
    _out << eb;
    _out << nl << "return __r;";
    _out << eb;

    if(_stream)
    {
	_out << sp << nl << "public static void write(Ice.OutputStream __out, " << name << " __v)";
	_out << sb;
	_out << nl << "if(__v == null)";
	_out << sb;
	_out << nl << "__out.writeSize(0);";
	_out << eb;
	_out << nl << "else";
	_out << sb;
	_out << nl << "__out.writeSize(__v.Count);";
	_out << nl << "foreach(_System.Collections.DictionaryEntry __e in __v)";
	_out << sb;
	writeMarshalUnmarshalCode(_out, key, keyArg, true, true, false);
	writeMarshalUnmarshalCode(_out, value, valueArg, true, true, false);
	_out << eb;
	_out << eb;
	_out << eb;

	_out << sp << nl << "public static " << name << " read(Ice.InputStream __in)";
	_out << sb;
	_out << nl << "int __sz = __in.readSize();";
	_out << nl << name << " __r = new " << name << "();";
	_out << nl << "for(int __i = 0; __i < __sz; ++__i)";
	_out << sb;
	_out << nl << keyS << " __k;";
	writeMarshalUnmarshalCode(_out, key, "__k", false, true, false);
	if(!hasClassValue)
	{
	    _out << nl << valueS << " __v;";
	}
	writeMarshalUnmarshalCode(_out, value, "__v", false, true, false, "__r, __k");
	if(!hasClassValue)
	{
	    _out << nl << "__r[__k] = __v;";
	}
	_out << eb;
	_out << nl << "return __r;";
	_out << eb;
    }

    _out << eb;
}

Slice::Gen::DelegateVisitor::DelegateVisitor(IceUtil::Output& out)
    : CsVisitor(out)
{
}

bool
Slice::Gen::DelegateVisitor::visitModuleStart(const ModulePtr& p)
{
    if(!p->hasNonLocalClassDecls())
    {
        return false;
    }

    _out << sp << nl << "namespace " << fixId(p->name());
    _out << sb;
    return true;
}

void
Slice::Gen::DelegateVisitor::visitModuleEnd(const ModulePtr&)
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

    string name = p->name();
    ClassList bases = p->bases();

    _out << sp << nl << "public interface _" << name << "Del : ";
    if(bases.empty())
    {
	_out << "Ice._ObjectDel";
    }
    else
    {
	ClassList::const_iterator q = bases.begin();
	while(q != bases.end())
	{
	    string s = (*q)->scoped();
	    string::size_type pos = s.rfind("::");
	    assert(pos != string::npos);
	    s = s.insert(pos + 2, "_");
	    s += "Del";
	    _out << fixId(s);
	    if(++q != bases.end())
	    {
		_out << ", ";
	    }
	}
    }

    _out << sb;

    OperationList ops = p->operations();

    OperationList::const_iterator r;
    for(r = ops.begin(); r != ops.end(); ++r)
    {
	OperationPtr op = *r;
	string opName = fixId(op->name(), DotNet::ICloneable, true);
	TypePtr ret = op->returnType();
	string retS = typeToString(ret);
	vector<string> params = getParams(op);

	_out << sp << nl << retS << ' ' << opName << spar << params << "Ice.Context __context" << epar << ';';
    }

    return true;
}

void
Slice::Gen::DelegateVisitor::visitClassDefEnd(const ClassDefPtr&)
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
    if(!p->hasNonLocalClassDecls())
    {
        return false;
    }

    _out << sp << nl << "namespace " << fixId(p->name());
    _out << sb;
    return true;
}

void
Slice::Gen::DelegateMVisitor::visitModuleEnd(const ModulePtr&)
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

    string name = p->name();
    ClassList bases = p->bases();

    _out << sp << nl << "public sealed class _" << name << "DelM : Ice._ObjectDelM, _" << name << "Del";
    _out << sb;

    OperationList ops = p->allOperations();

    OperationList::const_iterator r;
    for(r = ops.begin(); r != ops.end(); ++r)
    {
	OperationPtr op = *r;
	string opName = fixId(op->name(), DotNet::ICloneable, true);
	TypePtr ret = op->returnType();
	string retS = typeToString(ret);

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

	//
	// Arrange exceptions into most-derived to least-derived order. If we don't
	// do this, a base exception handler can appear before a derived exception
	// handler, causing compiler warnings and resulting in the base exception
	// being marshaled instead of the derived exception.
	//
#if defined(__SUNPRO_CC)
	throws.sort(Slice::derivedToBaseCompare);
#else
	throws.sort(Slice::DerivedToBaseCompare());
#endif

	vector<string> params = getParams(op);

	_out << sp << nl << "public " << retS << ' ' << opName << spar << params << "Ice.Context __context" << epar;
	_out << sb;

	_out << nl << "IceInternal.Outgoing __out = getOutgoing(\"" << op->name() << "\", " << sliceModeToIceMode(op)
	     << ", __context, __compress);";
	_out << nl << "try";
	_out << sb;
	if(!inParams.empty())
	{
	    _out << nl << "try";
	    _out << sb;
	    _out << nl << "IceInternal.BasicStream __os = __out.ostr();";
	    for(q = inParams.begin(); q != inParams.end(); ++q)
	    {
		writeMarshalUnmarshalCode(_out, q->first, fixId(q->second), true, false, false);
	    }
	    if(op->sendsClasses())
	    {
		_out << nl << "__os.writePendingObjects();";
	    }
	    _out << eb;
	    _out << nl << "catch(Ice.LocalException __ex)";
	    _out << sb;
	    _out << nl << "__out.abort(__ex);";
	    _out << eb;
	}
	if(!outParams.empty() || ret || !throws.empty())
	{
	    _out << nl << "IceInternal.BasicStream __is = __out.istr();";
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
	    writeMarshalUnmarshalCode(_out, q->first, fixId(q->second), false, false, true, "");
	}
	if(ret)
	{
	    BuiltinPtr builtin = BuiltinPtr::dynamicCast(ret);
	    if((builtin && builtin->kind() == Builtin::KindObject) || ClassDeclPtr::dynamicCast(ret))
	    {
		_out << nl << retS << " __ret;";
		_out << nl << "IceInternal.ParamPatcher __ret_PP = new IceInternal.ParamPatcher(typeof("
		     << retS << "));";
		_out << nl << "__is.readObject(__ret_PP);";
	    }
	    else
	    {
		_out << nl << retS << " __ret;";
		writeMarshalUnmarshalCode(_out, ret, "__ret", false, false, true, "");
	    }
	}
	if(op->returnsClasses())
	{
	    _out << nl << "__is.readPendingObjects();";
	    for(q = outParams.begin(); q != outParams.end(); ++q)
	    {
		string param = fixId(q->second);
	        BuiltinPtr builtin = BuiltinPtr::dynamicCast(q->first);
		if((builtin && builtin->kind() == Builtin::KindObject) || ClassDeclPtr::dynamicCast(q->first))
		{	    
		    string type = typeToString(q->first);
		    _out << nl << param << " = (" << type << ")" << param << "_PP.value;";
		}
		else
		{
		    StructPtr st = StructPtr::dynamicCast(q->first);
		    bool patchStruct = st && !st->hasMetaData("cs:class") && st->classDataMembers().size() != 0;
		    if(patchStruct)
		    {
			_out << nl << param << ".__patch();";
		    }
		}
	    }
	}
	if(ret)
	{
	    BuiltinPtr builtin = BuiltinPtr::dynamicCast(ret);
	    if((builtin && builtin->kind() == Builtin::KindObject) || ClassDeclPtr::dynamicCast(ret))
	    {
		_out << nl << "__ret = (" << retS << ")__ret_PP.value;";
	    }
	    else
	    {
		StructPtr st = StructPtr::dynamicCast(ret);
		bool patchStruct = st && !st->hasMetaData("cs:class") && st->classDataMembers().size() != 0;
		if(patchStruct)
		{
		    _out << nl << "__ret.__patch();";
		}
	    }
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
    }

    return true;
}

void
Slice::Gen::DelegateMVisitor::visitClassDefEnd(const ClassDefPtr&)
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
    if(!p->hasNonLocalClassDecls())
    {
        return false;
    }

    _out << sp << nl << "namespace " << fixId(p->name());
    _out << sb;
    return true;
}

void
Slice::Gen::DelegateDVisitor::visitModuleEnd(const ModulePtr&)
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

    string name = p->name();
    ClassList bases = p->bases();

    _out << sp << nl << "public sealed class _" << name << "DelD : Ice._ObjectDelD, _" << name << "Del";
    _out << sb;

    OperationList ops = p->allOperations();

    OperationList::const_iterator r;
    for(r = ops.begin(); r != ops.end(); ++r)
    {
        OperationPtr op = *r;
        string opName = fixId(op->name(), DotNet::ICloneable, true);
        TypePtr ret = op->returnType();
        string retS = typeToString(ret);
	ClassDefPtr containingClass = ClassDefPtr::dynamicCast(op->container());

        vector<string> params = getParams(op);
        vector<string> args = getArgs(op);

	_out << sp;
        _out << nl << "public " << retS << ' ' << opName << spar << params << "Ice.Context __context" << epar;
        _out << sb;
	if(containingClass->hasMetaData("amd") || op->hasMetaData("amd"))
	{
	    _out << nl << "throw new Ice.CollocationOptimizationException();";
	}
	else
	{
	    _out << nl << "Ice.Current __current = new Ice.Current();";
	    _out << nl << "__initCurrent(ref __current, \"" << op->name() << "\", " << sliceModeToIceMode(op)
		 << ", __context);";
	    _out << nl << "while(true)";
	    _out << sb;
	    _out << nl << "IceInternal.Direct __direct = new IceInternal.Direct(__current);";
	    _out << nl << "object __servant = __direct.servant();";
	    _out << nl << "if(__servant is " << fixId(name) << ")";
	    _out << sb;
	    _out << nl << "try";
	    _out << sb;
	    _out << nl;
	    if(ret)
	    {
		_out << "return ";
	    }
	    _out << "((" << fixId(containingClass->scoped()) << ")__servant)."
	         << opName << spar << args << "__current" << epar << ';';
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
    }

    return true;
}

void
Slice::Gen::DelegateDVisitor::visitClassDefEnd(const ClassDefPtr&)
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
    if(!p->hasNonLocalClassDecls())
    {
        return false;
    }

    _out << sp << nl << "namespace " << fixId(p->name());
    _out << sb;
    return true;
}

void
Slice::Gen::DispatcherVisitor::visitModuleEnd(const ModulePtr&)
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

    string name = p->name();

    _out << sp << nl << "public abstract class _" << name << "Disp : Ice.ObjectImpl, " << fixId(name);
    _out << sb;

    OperationList ops = p->operations();
    if(!ops.empty())
    {
        _out << sp << nl << "#region Slice operations";
    }

    for(OperationList::const_iterator op = ops.begin(); op != ops.end(); ++op)
    {
	bool amd = p->hasMetaData("amd") || (*op)->hasMetaData("amd");

	string name = (*op)->name();
	vector<string> params;
	vector<string> args;
	TypePtr ret;

	if(amd)
	{
	    name = name + "_async";
	    params = getParamsAsync(*op, true);
	    args = getArgsAsync(*op);
	}
	else
	{
	    name = fixId(name, DotNet::ICloneable, true);
	    params = getParams(*op);
	    ret = (*op)->returnType();
	    args = getArgs(*op);
	}

	_out << sp << nl << "public " << typeToString(ret) << " " << name << spar << params << epar;
	_out << sb << nl;
	if(ret)
	{
	    _out << "return ";
	}
	_out << name << spar << args << "Ice.ObjectImpl.defaultCurrent" << epar << ';';
	_out << eb;

	_out << sp << nl << "public abstract " << typeToString(ret) << " " << name << spar << params;
	if(!p->isLocal())
	{
	    _out << "Ice.Current __current";
	}
	_out << epar << ';';
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

Slice::Gen::AsyncVisitor::AsyncVisitor(::IceUtil::Output &out)
    : CsVisitor(out)
{
}

bool
Slice::Gen::AsyncVisitor::visitModuleStart(const ModulePtr& p)
{
    if(!p->hasAsyncOps())
    {
        return false;
    }

    _out << sp << nl << "namespace " << fixId(p->name());
    _out << sb;
    return true;
}

void
Slice::Gen::AsyncVisitor::visitModuleEnd(const ModulePtr&)
{
    _out << eb;
}

bool
Slice::Gen::AsyncVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    return true;
}

void
Slice::Gen::AsyncVisitor::visitClassDefEnd(const ClassDefPtr&)
{
}

void
Slice::Gen::AsyncVisitor::visitOperation(const OperationPtr& p)
{
    ContainerPtr container = p->container();
    ClassDefPtr cl = ClassDefPtr::dynamicCast(container);
    
    if(cl->isLocal())
    {
	return;
    }

    string name = p->name();
    
    if(cl->hasMetaData("ami") || p->hasMetaData("ami"))
    {

        TypePtr ret = p->returnType();
        string retS = typeToString(ret);

	TypeStringList inParams;
        TypeStringList outParams;
	ParamDeclList paramList = p->parameters();
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

        ExceptionList throws = p->throws();
        throws.sort();
        throws.unique();

	//
	// Arrange exceptions into most-derived to least-derived order. If we don't
	// do this, a base exception handler can appear before a derived exception
	// handler, causing compiler warnings and resulting in the base exception
	// being marshaled instead of the derived exception.
	//
#if defined(__SUNPRO_CC)
	throws.sort(Slice::derivedToBaseCompare);
#else
	throws.sort(Slice::DerivedToBaseCompare());
#endif

        TypeStringList::const_iterator q;

	vector<string> params = getParamsAsyncCB(p);
	vector<string> args = getArgsAsyncCB(p);

	vector<string> paramsInvoke = getParamsAsync(p, false);

	_out << sp << nl << "public abstract class AMI_" << cl->name() << '_'
	     << name << " : IceInternal.OutgoingAsync";
	_out << sb;
	_out << sp;
	_out << nl << "public abstract void ice_response" << spar << params << epar << ';';
	
	_out << sp << nl << "public void __invoke" << spar << "Ice.ObjectPrx __prx"
	    << paramsInvoke << "Ice.Context __ctx" << epar;
	_out << sb;
	_out << nl << "try";
	_out << sb;
	_out << nl << "__prepare(__prx, \"" << name << "\", " << sliceModeToIceMode(p) << ", __ctx);";
	for(q = inParams.begin(); q != inParams.end(); ++q)
	{
	    string typeS = typeToString(q->first);
	    writeMarshalUnmarshalCode(_out, q->first, fixId(q->second), true, false, false);
	}
	if(p->sendsClasses())
	{
	    _out << nl << "__os.writePendingObjects();";
	}
	_out << nl << "__os.endWriteEncaps();";
	_out << eb;
	_out << nl << "catch(Ice.LocalException __ex)";
	_out << sb;
	_out << nl << "__finished(__ex);";
	_out << nl << "return;";
	_out << eb;
	_out << nl << "__send();";
	_out << eb;

	_out << sp << nl << "protected override void __response(bool __ok)";
	_out << sb;
        for(q = outParams.begin(); q != outParams.end(); ++q)
        {
	    _out << nl << typeToString(q->first) << ' ' << fixId(q->second) << ';';
        }
        if(ret)
        {
	    _out << nl << retS << " __ret;";
        }
	_out << nl << "try";
	_out << sb;
	_out << nl << "if(!__ok)";
        _out << sb;
	_out << nl << "try";
	_out << sb;
	_out << nl << "__is.throwException();";
	_out << eb;
	for(ExceptionList::const_iterator r = throws.begin(); r != throws.end(); ++r)
	{
	    _out << nl << "catch(" << fixId((*r)->scoped()) << " __ex)";
	    _out << sb;
	    _out << nl << "throw __ex;";
	    _out << eb;
	}
	_out << nl << "catch(Ice.UserException)";
	_out << sb;
        _out << nl << "throw new Ice.UnknownUserException();";
	_out << eb;
        _out << eb;
        for(q = outParams.begin(); q != outParams.end(); ++q)
        {
	    writeMarshalUnmarshalCode(_out, q->first, fixId(q->second), false, false, true);
        }
        if(ret)
        {
	    writeMarshalUnmarshalCode(_out, ret, "__ret", false, false, true);
        }
	if(p->returnsClasses())
	{
	    _out << nl << "__is.readPendingObjects();";
	}
	for(q = outParams.begin(); q != outParams.end(); ++q)
	{
	    string param = fixId(q->second);
	    BuiltinPtr builtin = BuiltinPtr::dynamicCast(q->first);
	    if((builtin && builtin->kind() == Builtin::KindObject) || ClassDeclPtr::dynamicCast(q->first))
	    {
		string type = typeToString(q->first);
		_out << nl << param << " = (" << type << ")" << param << "_PP.value;";
	    }
	    else
	    {
		StructPtr st = StructPtr::dynamicCast(q->first);
		bool patchStruct = st && !st->hasMetaData("cs:class") && st->classDataMembers().size() != 0;
		if(patchStruct)
		{
		    _out << nl << param << ".__patch();";
		}
	    }
	}
	if(ret)
	{
	    BuiltinPtr builtin = BuiltinPtr::dynamicCast(ret);
	    if((builtin && builtin->kind() == Builtin::KindObject) || ClassDeclPtr::dynamicCast(ret))
	    {
		string type = typeToString(ret);
		_out << nl << "__ret = (" << retS << ")__ret_PP.value;";
	    }
	    else
	    {
		StructPtr st = StructPtr::dynamicCast(ret);
		bool patchStruct = st && !st->hasMetaData("cs:class") && st->classDataMembers().size() != 0;
		if(patchStruct)
		{
		    _out << nl << "__ret.__patch();";
		}
	    }
	}
   	_out << eb;
	_out << nl << "catch(Ice.LocalException __ex)";
	_out << sb;
	_out << nl << "__finished(__ex);";
	_out << nl << "return;";
	_out << eb;
	if(!throws.empty())
	{
	    _out << nl << "catch(Ice.UserException __ex)";
	    _out << sb;
	    _out << nl << "ice_exception(__ex);";
	    _out << nl << "return;";
	    _out << eb;
	}
	_out << nl << "ice_response" << spar << args << epar << ';';
	_out << eb;
	_out << eb;
    }

    if(cl->hasMetaData("amd") || p->hasMetaData("amd"))
    {
	string classNameAMD = "AMD_" + cl->name();
	string classNameAMDI = "_AMD_" + cl->name();

	vector<string> paramsAMD = getParamsAsyncCB(p);

	_out << sp << nl << "public interface " << classNameAMD << '_' << name;
	_out << sb;
	_out << sp << nl << "void ice_response" << spar << paramsAMD << epar << ';';
	_out << sp << nl << "void ice_exception(_System.Exception ex);";
	_out << eb;
    
	TypePtr ret = p->returnType();
	
	TypeStringList outParams;
	ParamDeclList paramList = p->parameters();
	for(ParamDeclList::const_iterator pli = paramList.begin(); pli != paramList.end(); ++pli)
	{
	    if((*pli)->isOutParam())
	    {
		outParams.push_back(make_pair((*pli)->type(), (*pli)->name()));
	    }
	}
	
	ExceptionList throws = p->throws();
	throws.sort();
	throws.unique();

	//
	// Arrange exceptions into most-derived to least-derived order. If we don't
	// do this, a base exception handler can appear before a derived exception
	// handler, causing compiler warnings and resulting in the base exception
	// being marshaled instead of the derived exception.
	//
#if defined(__SUNPRO_CC)
	throws.sort(Slice::derivedToBaseCompare);
#else
	throws.sort(Slice::DerivedToBaseCompare());
#endif

	TypeStringList::const_iterator q;
	_out << sp << nl << "class " << classNameAMDI << '_' << name
	    << " : IceInternal.IncomingAsync, " << classNameAMD << '_' << name;
	_out << sb;

	_out << sp << nl << "public " << classNameAMDI << '_' << name << "(IceInternal.Incoming inc) : base(inc)";
	_out << sb;
	_out << eb;

	_out << sp << nl << "public void ice_response" << spar << paramsAMD << epar;
	_out << sb;
	if(ret || !outParams.empty())
	{
	    _out << nl << "try";
	    _out << sb;
	    _out << nl << "IceInternal.BasicStream __os = this.__os();";
	    for(q = outParams.begin(); q != outParams.end(); ++q)
	    {
		string typeS = typeToString(q->first);
		writeMarshalUnmarshalCode(_out, q->first, fixId(q->second), true, false, false);
	    }
	    if(ret)
	    {
		string retS = typeToString(ret);
		writeMarshalUnmarshalCode(_out, ret, "__ret", true, false, false);
	    }
	    if(p->returnsClasses())
	    {
		_out << nl << "__os.writePendingObjects();";
	    }
	    _out << eb;
	    _out << nl << "catch(Ice.LocalException __ex)";
	    _out << sb;
	    _out << nl << "ice_exception(__ex);";
	    _out << eb;
	}
	_out << nl << "__response(true);";
	_out << eb;

	_out << sp << nl << "public void ice_exception(_System.Exception ex)";
	_out << sb;
	if(throws.empty())
	{
	    _out << nl << "__exception(ex);";
	}
	else
	{
	    _out << nl << "try";
	    _out << sb;
	    _out << nl << "throw ex;";
	    _out << eb;
	    ExceptionList::const_iterator r;
	    for(r = throws.begin(); r != throws.end(); ++r)
	    {
		string exS = fixId((*r)->scoped());
		_out << nl << "catch(" << exS << " __ex)";
		_out << sb;
		_out << nl << "__os().writeUserException(__ex);";
		_out << nl << "__response(false);";
		_out << eb;
	    }
	    _out << nl << "catch(_System.Exception __ex)";
	    _out << sb;
	    _out << nl << "__exception(__ex);";
	    _out << eb;
	}
	_out << eb;

	_out << eb;
    }
}

Slice::Gen::TieVisitor::TieVisitor(IceUtil::Output& out)
    : CsVisitor(out)
{
}

bool
Slice::Gen::TieVisitor::visitModuleStart(const ModulePtr& p)
{
    if(!p->hasClassDefs())
    {
        return false;
    }

    _out << sp << nl << "namespace " << fixId(p->name());
    _out << sb;

    return true;
}

void
Slice::Gen::TieVisitor::visitModuleEnd(const ModulePtr&)
{
    _out << eb;
}

bool
Slice::Gen::TieVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    if(!p->isAbstract())
    {
        return false;
    }
    
    string name = p->name();
    string opIntfName = "Operations";
    if(p->isLocal())
    {
        opIntfName += "NC";
    }

    _out << sp << nl << "public class _" << name << "Tie : ";
    if(p->isInterface())
    {
	if(p->isLocal())
	{
	    _out << fixId(name) << ", Ice.TieBase";
	}
	else
	{
	    _out << '_' << name << "Disp, Ice.TieBase";
	}
    }
    else
    {
        _out << fixId(name) << ", Ice.TieBase";
    }
    _out << sb;

    _out << sp << nl << "public _" << name << "Tie()";
    _out << sb;
    _out << eb;

    _out << sp << nl << "public _" << name << "Tie(_" << name << opIntfName << " del)";
    _out << sb;
    _out << nl << "_ice_delegate = del;";
    _out << eb;

    _out << sp << nl << "public object ice_delegate()";
    _out << sb;
    _out << nl << "return _ice_delegate;";
    _out << eb;

    _out << sp << nl << "public void ice_delegate(object del)";
    _out << sb;
    _out << nl << "_ice_delegate = (_" << name << opIntfName << ")del;";
    _out << eb;

    _out << sp << nl << "public ";
    if(!p->isInterface() || !p->isLocal())
    {
        _out << "override ";
    }
    _out << "int ice_hash()";

    _out << sb;
    _out << nl << "return GetHashCode();";
    _out << eb;

    _out << sp << nl << "public override int GetHashCode()";
    _out << sb;
    _out << nl << "return _ice_delegate == null ? 0 : _ice_delegate.GetHashCode();";
    _out << eb;

    _out << sp << nl << "public override bool Equals(object rhs)";
    _out << sb;
    _out << nl << "if(object.ReferenceEquals(this, rhs))";
    _out << sb;
    _out << nl << "return true;";
    _out << eb;
    _out << nl << "if(!(rhs is _" << name << "Tie))";
    _out << sb;
    _out << nl << "return false;";
    _out << eb;
    _out << nl << "if(_ice_delegate == null)";
    _out << sb;
    _out << nl << "return ((_" << name << "Tie)rhs)._ice_delegate == null;";
    _out << eb;
    _out << nl << "return _ice_delegate.Equals(((_" << name << "Tie)rhs)._ice_delegate);";
    _out << eb;

    OperationList ops = p->operations();
    OperationList::const_iterator r;
    for(r = ops.begin(); r != ops.end(); ++r)
    {
	bool hasAMD = p->hasMetaData("amd") || (*r)->hasMetaData("amd");
        string opName = hasAMD ? (*r)->name() + "_async" : fixId((*r)->name(), DotNet::ICloneable, true);

        TypePtr ret = (*r)->returnType();
        string retS = typeToString(ret);

        vector<string> params;
	vector<string> args;
        if(hasAMD)
        {
            params = getParamsAsync((*r), true);
            args = getArgsAsync(*r);
        }
        else
        {
            params = getParams((*r));
            args = getArgs(*r);
        }

        _out << sp << nl << "public ";
	if(!p->isInterface() || !p->isLocal())
	{
	    _out << "override ";
	}
	_out << (hasAMD ? "void" : retS) << ' ' << opName << spar << params;
        if(!p->isLocal())
        {
            _out << "Ice.Current __current";
        }
        _out << epar;
        _out << sb;
        _out << nl;
        if(ret && !hasAMD)
        {
            _out << "return ";
        }
        _out << "_ice_delegate." << opName << spar << args;
        if(!p->isLocal())
        {
            _out << "__current";
        }
        _out << epar << ';';
        _out << eb;
    }

    NameSet opNames;
    ClassList bases = p->bases();
    for(ClassList::const_iterator i = bases.begin(); i != bases.end(); ++i)
    {
        writeInheritedOperationsWithOpNames(*i, opNames);
    }

    _out << sp << nl << "private _" << name << opIntfName << " _ice_delegate;";

    return true;
}

void
Slice::Gen::TieVisitor::visitClassDefEnd(const ClassDefPtr&)
{
    _out << eb;
}

void
Slice::Gen::TieVisitor::writeInheritedOperationsWithOpNames(const ClassDefPtr& p, NameSet& opNames)
{
    OperationList ops = p->operations();
    OperationList::const_iterator r;
    for(r = ops.begin(); r != ops.end(); ++r)
    {
	bool hasAMD = p->hasMetaData("amd") || (*r)->hasMetaData("amd");
        string opName = hasAMD ? (*r)->name() + "_async" : fixId((*r)->name(), DotNet::ICloneable, true);
	if(opNames.find(opName) != opNames.end())
	{
	    continue;
	}
	opNames.insert(opName);

        TypePtr ret = (*r)->returnType();
        string retS = typeToString(ret);

        vector<string> params;
	vector<string> args;
        if(hasAMD)
        {
            params = getParamsAsync((*r), true);
            args = getArgsAsync(*r);
        }
        else
        {
            params = getParams((*r));
            args = getArgs(*r);
        }

        _out << sp << nl << "public ";
	if(!p->isInterface() || !p->isLocal())
	{
	    _out << "override ";
	}
	_out << (hasAMD ? "void" : retS) << ' ' << opName << spar << params;
        if(!p->isLocal())
        {
            _out << "Ice.Current __current";
        }
        _out << epar;
        _out << sb;
        _out << nl;
        if(ret && !hasAMD)
        {
            _out << "return ";
        }
        _out << "_ice_delegate." << opName << spar << args;
        if(!p->isLocal())
        {
            _out << "__current";
        }
        _out << epar << ';';
        _out << eb;
    }

    ClassList bases = p->bases();
    for(ClassList::const_iterator i = bases.begin(); i != bases.end(); ++i)
    {
        writeInheritedOperationsWithOpNames(*i, opNames);
    }
}

Slice::Gen::BaseImplVisitor::BaseImplVisitor(IceUtil::Output& out)
    : CsVisitor(out)
{
}

void
Slice::Gen::BaseImplVisitor::writeOperation(const OperationPtr& op, bool comment, bool forTie)
{
    ClassDefPtr cl = ClassDefPtr::dynamicCast(op->container());
    string opName = op->name();
    TypePtr ret = op->returnType();
    string retS = typeToString(ret);
    ParamDeclList params = op->parameters();
    ParamDeclList::const_iterator i;

    if(comment)
    {
	_out << nl << "// ";
    }
    else
    {
	_out << sp << nl;
    }

    if(!cl->isLocal() && (cl->hasMetaData("amd") || op->hasMetaData("amd")))
    {
        vector<string> pDecl = getParamsAsync(op, true);

	_out << "public ";
	if(!forTie)
	{
	    _out << "override ";
	}
	_out << "void " << opName << "_async" << spar << pDecl << "Ice.Current __current" << epar;

	if(comment)
	{
	    _out << ';';
	    return;
	}

	_out << sb;
	if(ret)
	{
	    _out << nl << typeToString(ret) << " __ret = " << writeValue(ret) << ';';
	}
	for(i = params.begin(); i != params.end(); ++i)
	{
	    if((*i)->isOutParam())
	    {
		string name = fixId((*i)->name());
		TypePtr type = (*i)->type();
		_out << nl << typeToString(type) << ' ' << name << " = " << writeValue(type) << ';';
	    }
	}
	_out << nl << "__cb.ice_response" << spar;
	if(ret)
	{
	    _out << "__ret";
	}
	for(i = params.begin(); i != params.end(); ++i)
	{
	    if((*i)->isOutParam())
	    {
		_out << fixId((*i)->name());
	    }
	}
	_out << epar << ';';
	_out << eb;
    }
    else
    {
	vector<string> pDecls = getParams(op);

	_out << "public ";
	if(!forTie && !cl->isLocal())
	{
	    _out << "override ";
	}
	_out << retS << ' ' << fixId(opName, DotNet::ICloneable, true) << spar << pDecls;
	if(!cl->isLocal())
	{
	    _out << "Ice.Current __current";
	}
	_out << epar;
	if(comment)
	{
	    _out << ';';
	    return;
	}
	_out << sb;
	for(ParamDeclList::const_iterator i = params.begin(); i != params.end(); ++i)
	{
	    if((*i)->isOutParam())
	    {
		string name = fixId((*i)->name());
		TypePtr type = (*i)->type();
		_out << nl << name << " = " << writeValue(type) << ';';
	    }
	}
	if(ret)
	{
	    _out << nl << "return " << writeValue(ret) << ';';
	}
	_out << eb;
    }
}

string
Slice::Gen::BaseImplVisitor::writeValue(const TypePtr& type)
{
    assert(type);

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if(builtin)
    {
	switch(builtin->kind())
	{
	    case Builtin::KindBool:
	    {
	        return "false";
		break;
	    }
	    case Builtin::KindByte:
	    case Builtin::KindShort:
	    case Builtin::KindInt:
	    case Builtin::KindLong:
	    {
	        return "0";
		break;
	    }
	    case Builtin::KindFloat:
	    {
	        return "0.0f";
		break;
	    }
	    case Builtin::KindDouble:
	    {
	        return "0.0";
		break;
	    }
	    default:
	    {
	        return "null";
		break;
	    }
	}
    }

    EnumPtr en = EnumPtr::dynamicCast(type);
    if(en)
    {
	return fixId(en->scoped()) + "." + fixId((*en->getEnumerators().begin())->name());
    }

    StructPtr st = StructPtr::dynamicCast(type);
    if(st)
    {
        return st->hasMetaData("cs:class") ? "null" : "new " + fixId(st->scoped()) + "()";
    }

    return "null";
}

Slice::Gen::ImplVisitor::ImplVisitor(IceUtil::Output& out)
    : BaseImplVisitor(out)
{
}

bool
Slice::Gen::ImplVisitor::visitModuleStart(const ModulePtr& p)
{
    if(!p->hasClassDefs())
    {
        return false;
    }

    _out << sp << nl << "namespace " << fixId(p->name());
    _out << sb;

    return true;
}

void
Slice::Gen::ImplVisitor::visitModuleEnd(const ModulePtr&)
{
    _out << eb;
}

bool
Slice::Gen::ImplVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    if(!p->isAbstract())
    {
        return false;
    }

    string name = p->name();

    _out << sp << nl << "public sealed class " << name << 'I';
    if(p->isInterface())
    {
        if(p->isLocal())
	{
	    _out << " : Ice.LocalObjectImpl, " << fixId(name);
	}
	else
	{
	    _out << " : _" << name << "Disp";
	}
    }
    else
    {
        _out << " : " << fixId(name);
    }
    _out << sb;

    OperationList ops = p->allOperations();
    for(OperationList::const_iterator r = ops.begin(); r != ops.end(); ++r)
    {
        writeOperation(*r, false, false);
    }

    return true;
}

void
Slice::Gen::ImplVisitor::visitClassDefEnd(const ClassDefPtr&)
{
    _out << eb;
}

Slice::Gen::ImplTieVisitor::ImplTieVisitor(IceUtil::Output& out)
    : BaseImplVisitor(out)
{
}

bool
Slice::Gen::ImplTieVisitor::visitModuleStart(const ModulePtr& p)
{
    if(!p->hasClassDefs())
    {
        return false;
    }

    _out << sp << nl << "namespace " << fixId(p->name());
    _out << sb;

    return true;
}

void
Slice::Gen::ImplTieVisitor::visitModuleEnd(const ModulePtr&)
{
    _out << eb;
}

bool
Slice::Gen::ImplTieVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    if(!p->isAbstract())
    {
        return false;
    }

    string name = p->name();
    ClassList bases = p->bases();

    //
    // Use implementation inheritance in the following situations:
    //
    // * if a class extends another class
    // * if a class implements a single interface
    // * if an interface extends only one interface
    //
    bool inheritImpl = (!p->isInterface() && !bases.empty() && !bases.front()->isInterface()) || (bases.size() == 1);

    _out << sp << nl << "public class " << name << "I : ";
    if(inheritImpl)
    {
	if(bases.front()->isAbstract())
	{
	    _out << bases.front()->name() << 'I';
	}
	else
	{
	    _out << fixId(bases.front()->name());
	}
	_out << ", ";
    }
    _out << '_' << name << "Operations";
    if(p->isLocal())
    {
        _out << "NC";
    }
    _out << sb;

    _out << nl << "public " << name << "I()";
    _out << sb;
    _out << eb;

    OperationList ops = p->allOperations();
    ops.sort();

    OperationList baseOps;
    if(inheritImpl)
    {
        baseOps = bases.front()->allOperations();
        baseOps.sort();
    }

    OperationList::const_iterator r;
    for(r = ops.begin(); r != ops.end(); ++r)
    {
        if(inheritImpl && binary_search(baseOps.begin(), baseOps.end(), *r))
        {
            _out << sp;
            _out << nl << "// ";
            _out << nl << "// Implemented by " << bases.front()->name() << 'I';
            _out << nl << "//";
            writeOperation(*r, true, true);
        }
        else
        {
            writeOperation(*r, false, true);
        }
    }

    _out << eb;

    return true;
}
