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

Slice::VbVisitor::VbVisitor(Output& out) : _out(out)
{
}

Slice::VbVisitor::~VbVisitor()
{
}

void
Slice::VbVisitor::writeInheritedOperations(const ClassDefPtr& p)
{
    ClassList bases = p->bases();
    if(!bases.empty() && !bases.front()->isInterface())
    {
	bases.pop_front();
    }
    if(!bases.empty())
    {
	_out.zeroIndent();
	_out << sp << nl << "#Region \"Inherited Slice operations\"";
	_out.restoreIndent();

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
	    string name = (*op)->name();
	    TypePtr ret = (*op)->returnType();

	    if(!amd)
	    {
		vector<string> params = getParams(*op);
		vector<string> args = getArgs(*op);
		string retS = typeToString((*op)->returnType());

		string vbOp = ret ? "Function" : "Sub";
		_out << sp << nl << "Public " << vbOp << ' ' << fixId(name, DotNet::ICloneable, true)
		     << spar << params << epar;
		if(ret)
		{
		    _out << " As " << retS;
		}
		_out << " Implements " << fixId(containingClass->scope()) << "_" << containingClass->name()
		     << "OperationsNC." << fixId(name, DotNet::ICloneable, true);
		_out.inc();
		_out << nl;
		if((*op)->returnType())
		{
		    _out << "Return ";
		}
		_out << fixId(name, DotNet::ICloneable, true)
		     << spar << args << "Ice.ObjectImpl.defaultCurrent" << epar;
		_out.dec();
		_out << nl << "End " << vbOp;

		_out << sp << nl << "Public MustOverride " << vbOp << ' ' << fixId(name, DotNet::ICloneable, true)
		     << spar << params;
		if(!containingClass->isLocal())
		{
		    _out << "ByVal __current As Ice.Current";
		}
		_out << epar;
		if(ret)
		{
		    _out << " As " << retS;
		}
		_out << " Implements " << fixId(containingClass->scope()) << "_" << containingClass->name()
		     << "Operations." << fixId(name, DotNet::ICloneable, true);
	    }
	    else
	    {
		vector<string> params = getParamsAsync(*op, true);
		vector<string> args = getArgsAsync(*op);

		_out << sp << nl << "Public Sub " << ' ' << name << "_async" << spar << params << epar
		     << " Implements " << fixId(containingClass->scope()) << "_" << containingClass->name()
		     << "OperationsNC." << name << "_async";
		_out.inc();
		_out << nl << name << "_async" << spar << args << epar;
		_out.dec();
		_out << nl << "End Sub";

		_out << sp << nl << "Public MustOverride Sub " << name << "_async"
		     << spar << params << "ByVal __current As Ice.Current" << epar
		     << " Implements " << fixId(containingClass->scope()) << "_" << containingClass->name()
		     << "Operations." << name << "_async";
	    }
	}

	_out.zeroIndent();
	_out << sp << nl << "#End Region"; // Inherited Slice operations
	_out.restoreIndent();
    }
}

void
Slice::VbVisitor::writeDispatch(const ClassDefPtr& p)
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
    
    _out.zeroIndent();
    _out << sp << nl << "#Region \"Slice type-related members\"";
    _out.restoreIndent();

    _out << sp << nl << "Public Shared Shadows __ids As String() = New String() _";
    _out.inc();
    _out << nl << "{ _";
    _out.inc();

    StringList::const_iterator q = ids.begin();
    while(q != ids.end())
    {
        _out << nl << '"' << *q << '"';
	if(++q != ids.end())
	{
	    _out << ',';
	}
	_out << " _";
    }
    _out.dec();
    _out << nl << '}';
    _out.dec();

    _out << sp << nl << "Public Overloads Overrides Function ice_isA(ByVal s As String) As Boolean";
    _out.inc();
    _out << nl << "Return _System.Array.BinarySearch(__ids, s, _System.Collections.Comparer.DefaultInvariant) >= 0";
    _out.dec();
    _out << nl << "End Function";

    _out << sp << nl << "Public Overloads Overrides Function ice_isA(ByVal s As String, Byval __current As Ice.Current)"
                        " As Boolean";
    _out.inc();
    _out << nl << "Return _System.Array.BinarySearch(__ids, s, _System.Collections.Comparer.DefaultInvariant) >= 0";
    _out.dec();
    _out << nl << "End Function";

    _out << sp << nl << "Public Overloads Overrides Function ice_ids() As String()";
    _out.inc();
    _out << nl << "Return __ids";
    _out.dec();
    _out << nl << "End Function";

    _out << sp << nl << "Public Overloads Overrides Function ice_ids(ByVal __current As Ice.Current) As String()";
    _out.inc();
    _out << nl << "Return __ids";
    _out.dec();
    _out << nl << "End Function";

    _out << sp << nl << "Public Overloads Overrides Function ice_id() As String";
    _out.inc();
    _out << nl << "Return __ids(" << scopedPos << ")";
    _out.dec();
    _out << nl << "End Function";

    _out << sp << nl << "Public Overloads Overrides Function ice_id(ByVal __current As Ice.Current) As String";
    _out.inc();
    _out << nl << "Return __ids(" << scopedPos << ")";
    _out.dec();
    _out << nl << "End Function";

    _out << sp << nl << "Public Overloads Shared Function ice_staticId() As String";
    _out.inc();
    _out << nl << "Return __ids(" << scopedPos << ")";
    _out.dec();
    _out << nl << "End Function";

    _out.zeroIndent();
    _out << sp << nl << "#End Region"; // Slice type-related members
    _out.restoreIndent();

    OperationList ops = p->operations();
    if(!p->isInterface() || ops.size() != 0)
    {
	_out.zeroIndent();
	_out << sp << nl << "#Region \"Operation dispatch\"";
	_out.restoreIndent();
    }

    OperationList::const_iterator r;
    for(r = ops.begin(); r != ops.end(); ++r)
    {
        OperationPtr op = *r;
	ContainerPtr container = op->container();
        ClassDefPtr cl = ClassDefPtr::dynamicCast(container);
        assert(cl);

        string opName = op->name();
        _out << sp << nl << "Public Shared Function ___" << opName << "( _";
	_out.inc();
	_out.inc();
	_out << nl << "ByVal __obj As _" << p->name() << "Operations, _";
	_out << nl << "ByVal __inS As IceInternal.Incoming, _";
	_out << nl << "ByVal __current As Ice.Current) As IceInternal.DispatchStatus";
        _out.dec();

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
		_out << nl << "Dim __is As IceInternal.BasicStream = __inS.istr()";
	    }
	    if(!outParams.empty() || ret || !throws.empty())
	    {
		_out << nl << "Dim __os As IceInternal.BasicStream = __inS.ostr()";
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
		    _out << nl << "Dim " << param << " As " << typeS;
		}
		writeMarshalUnmarshalCode(_out, q->first, param, false, false, true);
	    }
	    if(op->sendsClasses())
	    {
		_out << nl << "__is.readPendingObjects()";
	    }
	    for(q = inParams.begin(); q != inParams.end(); ++q)
	    {
		StructPtr st = StructPtr::dynamicCast(q->first);
		bool patchStruct = st && !st->hasMetaData("vb:class") && st->classDataMembers().size() != 0;
		if(patchStruct)
		{
		    _out << nl << fixId(q->second) << ".__patch()";
		}
	    }
	    
	    for(q = outParams.begin(); q != outParams.end(); ++q)
	    {
		string typeS = typeToString(q->first);
		_out << nl << "Dim " << fixId(q->second) << " As " << typeS;
	    }
	    
	    //
	    // Call on the servant.
	    //
	    if(!throws.empty())
	    {
		_out << nl << "Try";
		_out.inc();
	    }
	    _out << nl;
	    if(ret)
	    {
		string retS = typeToString(ret);
		_out << "Dim __ret As " << retS << " = ";
	    }
	    _out << "__obj." << fixId(opName, DotNet::ICloneable, true) << spar;
	    for(q = inParams.begin(); q != inParams.end(); ++q)
	    {
		BuiltinPtr builtin = BuiltinPtr::dynamicCast(q->first);
		bool isClass = (builtin && builtin->kind() == Builtin::KindObject)
		               || ClassDeclPtr::dynamicCast(q->first);
		
		if(isClass)
		{
		    _out << "CType(" + fixId(q->second) + "_PP.value, " + typeToString(q->first) + ")";
		}
		else
		{
		    _out << fixId(q->second);
		}
	    }
	    for(q = outParams.begin(); q != outParams.end(); ++q)
	    {
		_out << fixId(q->second);
	    }
	    _out << "__current" << epar;
	    
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
		_out << nl << "__os.writePendingObjects()";
	    }
	    _out << nl << "Return IceInternal.DispatchStatus.DispatchOK";
	    
	    //
	    // Handle user exceptions.
	    //
	    if(!throws.empty())
	    {
		_out.dec();
		ExceptionList::const_iterator t;
		for(t = throws.begin(); t != throws.end(); ++t)
		{
		    string exS = fixId((*t)->scoped());
		    _out << nl << "Catch ex As " << exS;
		    _out.inc();
		    _out << nl << "__os.writeUserException(ex)";
		    _out << nl << "Return IceInternal.DispatchStatus.DispatchUserException";
		    _out.dec();
		    _out << nl << "End Try";
		}
	    }

	    _out.dec();
	    _out << nl << "End Function";
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
		_out << nl << "Dim __is As IceInternal.BasicStream = __inS.istr()";
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
		    _out << nl << "Dim " << fixId(q->second) << " As " << typeToString(q->first);
		}
		writeMarshalUnmarshalCode(_out, q->first, fixId(q->second), false, false, true);
	    }
	    if(op->sendsClasses())
	    {
		_out << nl << "__is.readPendingObjects()";
	    }
	    for(q = inParams.begin(); q != inParams.end(); ++q)
	    {
		StructPtr st = StructPtr::dynamicCast(q->first);
		bool patchStruct = st && !st->hasMetaData("vb:class") && st->classDataMembers().size() != 0;
		if(patchStruct)
		{
		    _out << nl << fixId(q->second) << ".__patch()";
		}
	    }
	    
	    //
	    // Call on the servant.
	    //
	    string classNameAMD = "AMD_" + p->name();
	    _out << nl << "Dim __cb As " << classNameAMD << '_' << op->name() << " = new _"
	         << classNameAMD << '_' << op->name() << "(__inS)";
            _out << nl << "Try";
            _out.inc();
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
		if(isClass)
		{
		    _out << "CType(" + q->second + "__PP.value, " + typeToString(q->first);
		}
		else
		{
		    _out << fixId(q->second);
		}
	    }
	    _out << "__current" << epar;
	    _out.dec();
	    _out << nl << "Catch ex As _System.Exception";
	    _out.inc();
	    _out << nl << "__cb.ice_exception(ex)";
	    _out.dec();
	    _out << nl << "End Try";
	    _out << nl << "Return IceInternal.DispatchStatus.DispatchAsync";

	    _out.dec();
	    _out << nl << "End Function";
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

	_out << sp << nl << "Public Shared Shadows __all As String() = New String() _";
	_out.inc();
	_out << nl << "{ _";
	_out.inc();

	q = allOpNames.begin();
	while(q != allOpNames.end())
	{
	    _out << nl << '"' << *q << '"';
	    if(++q != allOpNames.end())
	    {
		_out << ',';
	    }
	    _out << " _";
	}
	_out.dec();
	_out << nl << '}';
	_out.dec();

	_out << sp << nl << "Public Overloads Overrides Function __dispatch( _";
	_out.inc();
	_out.inc();
	_out << nl << "ByVal __inS As IceInternal.Incoming, _";
	_out << nl << "ByVal __current As Ice.Current) As IceInternal.DispatchStatus";
	_out.dec();
	_out << nl << "Dim pos As Integer";
	_out << nl << "pos = _System.Array.BinarySearch(__all, __current.operation, "
	     << "_System.Collections.Comparer.DefaultInvariant)";
	_out << nl << "If pos < 0 Then";
	_out.inc();
	_out << nl << "Return IceInternal.DispatchStatus.DispatchOperationNotExist";
	_out.dec();
	_out << nl << "End If";
	_out << nl << "Select Case pos";
	_out.inc();
	int i = 0;
	for(q = allOpNames.begin(); q != allOpNames.end(); ++q)
	{
	    string opName = *q;

	    _out << nl << "Case " << i++;
	    _out.inc();
	    if(opName == "ice_id")
	    {
		_out << nl << "Return ___ice_id(Me, __inS, __current)";
	    }
	    else if(opName == "ice_ids")
	    {
		_out << nl << "Return ___ice_ids(Me, __inS, __current)";
	    }
	    else if(opName == "ice_isA")
	    {
		_out << nl << "Return ___ice_isA(Me, __inS, __current)";
	    }
	    else if(opName == "ice_ping")
	    {
		_out << nl << "Return ___ice_ping(Me, __inS, __current)";
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
			    _out << nl << "Return ___" << opName << "(Me, __inS, __current)";
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
			    _out << nl << "Return " << fixId(base) << ".___" << opName << "(Me, __inS, __current)";
			}
			break;
		    }
		}
	    }
	    _out.dec();
	}
	_out.dec();
	_out << nl << "End Select";
	_out << sp << nl << "' _System.Diagnostics.Debug.Assert(false) ' Bug in VB 7.1: Diagnostics.Debug is not found";
	_out << nl << "Return IceInternal.DispatchStatus.DispatchOperationNotExist";
	_out.dec();
	_out << nl << "End Function";
    }

    if(!p->isInterface() || ops.size() != 0)
    {
	_out.zeroIndent();
	_out << sp << nl << "#End Region"; // Operation dispatch
	_out.restoreIndent();
    }
}

vector<string>
Slice::VbVisitor::getParams(const OperationPtr& op)
{
    vector<string> params;
    ParamDeclList paramList = op->parameters();
    for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
	string param = ((*q)->isOutParam() ? "<_System.Runtime.InteropServices.Out()> ByRef " : "ByVal ")
	               + fixId((*q)->name()) + " As " + typeToString((*q)->type());
	params.push_back(param);
    }
    return params;
}

vector<string>
Slice::VbVisitor::getParamsAsync(const OperationPtr& op, bool amd)
{
    vector<string> params;

    string name = op->name();
    ContainerPtr container = op->container();
    ClassDefPtr cl = ClassDefPtr::dynamicCast(container); // Get the class containing the op.
    string param = "ByVal __cb As ";
    string scope = fixId(cl->scope());
    param += scope + (amd ? "AMD_" : "AMI_") + cl->name() + "_" + op->name();
    params.push_back(param);

    ParamDeclList paramList = op->parameters();
    for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
	if(!(*q)->isOutParam())
	{
	    params.push_back("ByVal " + fixId((*q)->name()) + " As " + typeToString((*q)->type()));
	}
    }
    return params;
}

vector<string>
Slice::VbVisitor::getParamsAsyncCB(const OperationPtr& op)
{
    vector<string> params;

    TypePtr ret = op->returnType();
    if(ret)
    {
	params.push_back("ByVal __ret As " + typeToString(ret));
    }

    ParamDeclList paramList = op->parameters();
    for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
	if((*q)->isOutParam())
	{
	    params.push_back("ByVal " + fixId((*q)->name()) + " As " + typeToString((*q)->type()));
	}
    }

    return params;
}

vector<string>
Slice::VbVisitor::getArgs(const OperationPtr& op)
{
    vector<string> args;
    ParamDeclList paramList = op->parameters();
    for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
	string arg = fixId((*q)->name());
        args.push_back(arg);
    }
    return args;
}

vector<string>
Slice::VbVisitor::getArgsAsync(const OperationPtr& op)
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
Slice::VbVisitor::getArgsAsyncCB(const OperationPtr& op)
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
    string file = base + ".vb";
    string fileImpl = base + "I.vb";

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
	    file = dir + slash + fileBase + ".vb";
	    fileImpl = dir + slash + fileBase + "I.vb";
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

    _out << nl << "' Generated from file `" << base << ".ice'";

    _out << sp << nl << "Imports _System = System";
    _out << nl << "Imports _Microsoft = Microsoft";

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

    VbGenerator::validateMetaData(p);

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

        _out << sp << nl << "Namespace IceInternal";
        _out.inc();
        _out << sp << nl << "Namespace SliceChecksums";
        _out.inc();
        _out << sp << nl << "Public NotInheritable Class " << className;
        _out.inc();
        _out << nl << "Public Shared Readonly map As _System.Collections.Hashtable"
	     << " = new _System.Collections.Hashtable()";
        _out << sp << nl << "Shared Sub New()";
        _out.inc();
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
            _out << str.str() << "\")";
        }
        _out.dec();
	_out << nl << "End Sub";
        _out.dec();
	_out << sp << nl << "End Class";
        _out.dec();
	_out << sp << nl << "End Namespace";
        _out.dec();
	_out << sp << nl << "End Namespace";
    }
}

void
Slice::Gen::printHeader()
{
    static const char* header =
"' **********************************************************************\n"
"'\n"
"' Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.\n"
"'\n"
"' This copy of Ice is licensed to you under the terms described in the\n"
"' ICE_LICENSE file included in this distribution.\n"
"'\n"
"' **********************************************************************\n"
        ;

    _out << header;
    _out << "\n' Ice version " << ICE_STRING_VERSION;
}

Slice::Gen::OpsVisitor::OpsVisitor(IceUtil::Output& out)
    : VbVisitor(out)
{
}

Slice::Gen::TypesVisitor::TypesVisitor(IceUtil::Output& out, bool stream)
    : VbVisitor(out),
      _stream(stream)
{
}

bool
Slice::Gen::TypesVisitor::visitModuleStart(const ModulePtr& p)
{
    string name = fixId(p->name());
    _out << sp << nl << "Namespace " << name;

    _out.inc();

    return true;
}

void
Slice::Gen::TypesVisitor::visitModuleEnd(const ModulePtr&)
{
    _out.dec();
    _out << sp << nl << "End Namespace";
}

bool
Slice::Gen::TypesVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    string name = p->name();
    string scoped = fixId(p->scoped());
    ClassList bases = p->bases();
    bool hasBaseClass = !bases.empty() && !bases.front()->isInterface();

    if(!p->isLocal() && _stream)
    {
        _out << sp << nl << "Public NotInheritable Class " << p->name() << "Helper";
	_out.inc();

	_out << sp << nl << "Public Sub New(ByVal __inS As Ice.InputStream)";
	_out.inc();
	_out << nl << "_in = __inS";
	_out << nl << "_pp = New IceInternal.ParamPatcher(GetType(" << scoped << "))";
	_out.dec();
	_out << nl << "End Sub";

	_out << sp << nl << "Public Shared Sub write(ByVal __outS As Ice.OutputStream, ByVal __v As "
	     << fixId(name) << ')';
	_out.inc();
	_out << nl << "__outS.writeObject(__v)";
	_out.dec();
	_out << nl << "End Sub";

	_out << sp << nl << "Public Sub read()";
	_out.inc();
	_out << nl << "_in.readObject(_pp)";
	_out.dec();
	_out << nl << "End Sub";

	_out << sp << nl << "Public ReadOnly Property value() As " << scoped;
	_out.inc();
	_out << nl << "Get";
	_out.inc();
	_out << nl << "Return CType(_pp.value, " << scoped << ')';
	_out.dec();
	_out << nl << "End Get";
	_out.dec();
	_out << nl << "End Property";

	_out << sp << nl << "Private _in As Ice.InputStream";
	_out << nl << "Private _pp As IceInternal.ParamPatcher";

	_out.dec();
	_out << nl << "End Class";
    }
    if(p->isInterface())
    {
        _out << sp << nl << "Public Interface " << name;
	_out.inc();
	if(p->isLocal())
	{
	    _out << nl << "Inherits Ice.LocalObject";
	}
	else
	{
	    _out << nl << "Inherits Ice.Object";
	}
	_out << ", _" << p->name();
	if(!p->isLocal())
	{
	    _out << "Operations, _" << p->name();
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
	_out << sp << nl << "Public ";
	if(p->isAbstract())
	{
	    _out << "MustInherit ";
	}
	_out << "Class " << name;
	_out.inc();

	if(!hasBaseClass)
	{
	    if(p->isLocal())
	    {
		_out << nl << "Inherits Ice.LocalObjectImpl";
	    }
	    else
	    {
		_out << nl << "Inherits Ice.ObjectImpl";
	    }
	}
	else
	{
	    _out << nl << "Inherits " << fixId(bases.front()->scoped());
	    bases.pop_front();
	}
	if(p->isAbstract())
	{
	    _out << nl << "Implements _" << p->name();
	    if(!p->isLocal())
	    {
	        _out << "Operations, _" << p->name();
	    }
	    _out << "OperationsNC";
	}
	for(ClassList::const_iterator q = bases.begin(); q != bases.end(); ++q)
	{
	    if((*q)->isAbstract())
	    {
		_out << ", " << fixId((*q)->scoped());
	    }
	}
    }

    if(!p->isInterface())
    {
	_out.zeroIndent();
        if(p->hasDataMembers() && !p->hasOperations())
	{
	    _out << sp << nl << "#Region \"Slice data members\"";
	}
	else if(p->hasDataMembers())
	{
	    _out << sp << nl << "#Region \"Slice data members and operations\"";
	}
	else if(p->hasOperations())
	{
	    _out << sp << nl << "#Region \"Slice operations\"";
	}
	_out.restoreIndent();
    }

    return true;
}

void
Slice::Gen::TypesVisitor::visitClassDefEnd(const ClassDefPtr& p)
{
    string name = fixId(p->name());
    DataMemberList dataMembers = p->dataMembers();
    DataMemberList allDataMembers = p->allDataMembers();
    ClassList bases = p->bases();
    bool hasBaseClass = !bases.empty() && !bases.front()->isInterface();
    DataMemberList::const_iterator d;

    if(!p->isInterface())
    {
        _out.zeroIndent();
        if(p->hasDataMembers() && !p->hasOperations())
	{
	    _out << sp << nl << "#End Region"; // Slice data members
	}
	else if(p->hasDataMembers())
	{
	    _out << sp << nl << "#End Region"; // Slice data members and operations
	}
	else if(p->hasOperations())
	{
	    _out << sp << nl << "#End Region"; // Slice operations
	}
	_out.restoreIndent();

	if(!allDataMembers.empty())
	{
	    _out.zeroIndent();
	    _out << sp << nl << "#Region \"Constructors\"";
	    _out.restoreIndent();

	    _out << sp << nl << "Public Sub New" << spar << epar;
	    _out.inc();
	    if(hasBaseClass)
	    {
	        _out << nl << "MyBase.New()";
	    }
	    _out.dec();
	    _out << nl << "End Sub";

	    _out << sp << nl << "Public Sub New" << spar;
	    vector<string> paramDecl;
	    for(d = allDataMembers.begin(); d != allDataMembers.end(); ++d)
	    {
	        string memberName = fixId((*d)->name());
		string memberType = typeToString((*d)->type());
		paramDecl.push_back("ByVal " + memberName + " As " + memberType);
	    }
	    _out << paramDecl << epar;
	    _out.inc();
	    if(hasBaseClass && allDataMembers.size() != dataMembers.size())
	    {
		_out << nl << "MyBase.New" << spar;
		vector<string> baseParamNames;
		DataMemberList baseDataMembers = bases.front()->allDataMembers();
		for(d = baseDataMembers.begin(); d != baseDataMembers.end(); ++d)
		{
		    baseParamNames.push_back(fixId((*d)->name()));
		}
		_out << baseParamNames << epar;
	    }
	    vector<string> paramNames;
	    for(d = dataMembers.begin(); d != dataMembers.end(); ++d)
	    {
	        paramNames.push_back(fixId((*d)->name()));
	    }
	    for(vector<string>::const_iterator i = paramNames.begin(); i != paramNames.end(); ++i)
	    {
	        _out << nl << "Me." << *i << " = " << *i;
	    }
	    _out.dec();
	    _out << nl << "End Sub";

	    _out.zeroIndent();
	    _out << sp << nl << "#End Region"; // Constructors
	    _out.restoreIndent();
	}

	writeInheritedOperations(p);
    }

    if(!p->isInterface() && !p->isLocal())
    {
	writeDispatch(p);

	DataMemberList members = p->dataMembers();
	DataMemberList::const_iterator d;

	_out.zeroIndent();
	_out << sp << nl << "#Region \"Marshaling support\"";
	_out.restoreIndent();

	_out << sp << nl << "Public Overloads Overrides Sub __write(ByVal __os As IceInternal.BasicStream)";
	_out.inc();
	_out << nl << "__os.writeTypeId(ice_staticId())";
	_out << nl << "__os.startWriteSlice()";
	for(d = members.begin(); d != members.end(); ++d)
	{
	    StringList metaData = (*d)->getMetaData();
	    writeMarshalUnmarshalCode(_out, (*d)->type(), fixId((*d)->name(), DotNet::ICloneable, true), true, false, false);
	}
	_out << nl << "__os.endWriteSlice()";
	_out << nl << "MyBase.__write(__os)";
	_out.dec();
	_out << nl << "End Sub";

	DataMemberList allClassMembers = p->allClassDataMembers();
	if(allClassMembers.size() != 0)
	{
	    _out << sp << nl << "Public NotInheritable Class __Patcher";
	    _out.inc();
	    _out << nl << "Inherits IceInternal.Patcher";
	    _out << sp << nl << "Friend Sub New(ByVal instance As Ice.ObjectImpl";
	    if(allClassMembers.size() > 1)
	    {
	    	_out << ", ByVal member As Integer";
	    }
	    _out << ')';
	    _out.inc();
	    _out << nl << "_instance = CType(instance, " << name << ')';
	    if(allClassMembers.size() > 1)
	    {
		_out << nl << "_member = member";
	    }
	    _out.dec();
	    _out << nl << "End Sub";

	    _out << sp << nl << "Public Overrides Sub patch(ByVal v As Ice.Object)";
	    _out.inc();
	    if(allClassMembers.size() > 1)
	    {
		_out << nl << "Select Case _member";
		_out.inc();
	    }
	    int memberCount = 0;
	    for(d = allClassMembers.begin(); d != allClassMembers.end(); ++d)
	    {
		if(allClassMembers.size() > 1)
		{
		    _out << nl << "Case " << memberCount;
		    _out.inc();
		}
		string memberName = fixId((*d)->name(), DotNet::ICloneable, true);
		string memberType = typeToString((*d)->type());
		_out << nl << "_type = GetType(" << memberType << ')';
		_out << nl << "_instance." << memberName << " = CType(v, " << memberType << ')';
		if(allClassMembers.size() > 1)
		{
		    _out.dec();
		}
		memberCount++;
	    }
	    if(allClassMembers.size() > 1)
	    {
		_out << '}';
		_out << nl << "End Select";
	    }
	    _out.dec();
	    _out << nl << "End Sub";

	    _out << sp << nl << "Private _instance As " << name;
	    if(allClassMembers.size() > 1)
	    {
		_out << nl << "Private _member As Integer";
	    }
	    _out.dec();
	    _out << nl << "End Class";
	}

	_out << sp << nl << "Public Overloads Overrides Sub __read(ByVal __is As IceInternal.BasicStream, "
	                    "ByVal __rid As Boolean)";
	_out.inc();
	_out << nl << "If __rid Then";
	_out.inc();
	_out << nl << "Dim myId As String = __is.readTypeId()";
	_out.dec();
	_out << nl << "End If";
	_out << nl << "__is.startReadSlice()";
	DataMemberList classMembers = p->classDataMembers();
	int classMemberCount = static_cast<int>(allClassMembers.size() - classMembers.size());
	for(d = members.begin(); d != members.end(); ++d)
	{
	    StringList metaData = (*d)->getMetaData();
	    ostringstream patchParams;
	    patchParams << "Me";
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
	_out << nl << "__is.endReadSlice()";
	_out << nl << "MyBase.__read(__is, true)";
	_out.dec();
	_out << nl << "End Sub";

	//
	// Write streaming API.
	//
	if(_stream)
	{
	    _out << sp << nl << "Public Overloads Overrides Sub __write(ByVal __outS As Ice.OutputStream)";
	    _out.inc();
	    _out << nl << "__outS.writeTypeId(ice_staticId())";
	    _out << nl << "__outS.startSlice()";
	    for(d = members.begin(); d != members.end(); ++d)
	    {
		StringList metaData = (*d)->getMetaData();
		writeMarshalUnmarshalCode(_out, (*d)->type(),
					  fixId((*d)->name(), DotNet::ICloneable, true),
					  true, true, false);
	    }
	    _out << nl << "__outS.endSlice()";
	    _out << nl << "MyBase.__write(__outS)";
	    _out.dec();
	    _out << nl << "End Sub";

	    _out << sp << nl << "Public Overloads Overrides Sub __read"
	         << "(ByVal __inS As Ice.InputStream, ByVal __rid As Boolean)";
	    _out.inc();
	    _out << nl << "If __rid Then";
	    _out.inc();
	    _out << nl << "Dim myId As String = __inS.readTypeId()";
	    _out.dec();
	    _out << nl << "End If";
	    _out << nl << "__inS.startSlice()";
	    for(d = members.begin(); d != members.end(); ++d)
	    {
		StringList metaData = (*d)->getMetaData();
		ostringstream patchParams;
		patchParams << "Me";
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
	    _out << nl << "__inS.endSlice()";
	    _out << nl << "MyBase.__read(__inS, True)";
	    _out.dec();
	    _out << nl << "End Sub";
	}
	else
	{
	    //
	    // Emit placeholder functions to catch errors.
	    //
            string scoped = p->scoped();
	    _out << sp << nl << "Public Overloads Overrides Sub __write(ByVal __outS As Ice.OutputStream)";
	    _out.inc();
	    _out << nl << "Dim ex As Ice.MarshalException = New Ice.MarshalException";
	    _out << nl << "ex.reason = \"type " << scoped.substr(2) << " was not generated with stream support\"";
	    _out << nl << "Throw ex";
	    _out.dec();
	    _out << nl << "End Sub";

	    _out << sp << nl << "Public Overloads Overrides Sub __read"
	         << "(ByVal __inS As Ice.InputStream, ByVal __rid As Boolean)";
	    _out.inc();
	    _out << nl << "Dim ex As Ice.MarshalException = New Ice.MarshalException";
	    _out << nl << "ex.reason = \"type " << scoped.substr(2) << " was not generated with stream support\"";
	    _out << nl << "Throw ex";
	    _out.dec();
	    _out << nl << "End Sub";
	}

	_out.zeroIndent();
	_out << sp << nl << "#End Region"; // Marshalling support
	_out.restoreIndent();
    }

    _out.dec();
    _out << nl << "End " << (p->isInterface() ? "Interface" : "Class");
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
    TypePtr ret;
    string retS;

    if(!amd)
    {
	params = getParams(p);
	args = getArgs(p);
	name = fixId(name, DotNet::ICloneable, true);
	ret = p->returnType();
	retS = typeToString(ret);
    }
    else
    {
	params = getParamsAsync(p, true);
	args = getArgsAsync(p);
	name = name + "_async";
    }

    string vbOp = ret ? "Function" : "Sub";

    _out << sp << nl << "Public ";
    if(isLocal)
    {
        _out << "MustOverride ";
    }
    _out << vbOp << ' ' << name << spar << params << epar;
    if(ret)
    {
        _out << " As " << retS;
    }
    _out << " Implements _" << classDef->name() << "OperationsNC." << name;
    if(!isLocal)
    {
	_out.inc();
	_out << nl;
	if(!amd && p->returnType())
	{
	    _out << "Return ";
	}
	_out << name << spar << args << "Ice.ObjectImpl.defaultCurrent" << epar;
	_out.dec();
	_out << nl << "End " << vbOp;
    }

    if(!isLocal)
    {
	_out << sp << nl << "Public MustOverride " << vbOp << ' ' << name
	     << spar << params << "ByVal __current As Ice.Current" << epar;
	if(ret)
	{
	    _out << " As " << retS;
	}
	_out << " Implements _" << classDef->name() << "Operations." << name;
    }
}

void
Slice::Gen::TypesVisitor::visitSequence(const SequencePtr& p)
{
    //
    // No need to generate anything if the sequence is mapped as an array.
    //
    if(!p->hasMetaData("vb:collection"))
    {
        return;
    }

    string name = fixId(p->name());
    string s = typeToString(p->type());
    bool isValue = isValueType(p->type());

    _out << sp << nl << "Public Class " << name;
    _out.inc();
    _out << nl << "Inherits _System.Collections.CollectionBase";
    _out << nl << "Implements _System.ICloneable";

    _out.zeroIndent();
    _out << sp << nl << "#Region \"Constructors\"";
    _out.restoreIndent();

    _out << sp << nl << "Public Sub New()";
    _out << nl << "End Sub";

    _out << sp << nl << "Public Sub New(ByVal capacity As Integer)";
    _out.inc();
    _out << nl << "InnerList.Capacity = capacity";
    _out.dec();
    _out << nl << "End Sub";

    _out << sp << nl << "Public Sub New(ByVal __a As " << s << "())";
    _out.inc();
    _out << nl << "InnerList.AddRange(__a)";
    _out.dec();
    _out << nl << "End Sub";

    _out.zeroIndent();
    _out << sp << nl << "#End Region"; // Constructors

    _out << sp << nl << "#Region \"Array copy and conversion\"";
    _out.restoreIndent();

    _out << sp << nl << "Public Sub CopyTo(ByVal __a As " << s << "())";
    _out.inc();
    _out << nl << "InnerList.CopyTo(__a)";
    _out.dec();
    _out << nl << "End Sub";

    _out << sp << nl << "Public Sub CopyTo(ByVal __a As " << s << "(), ByVal __i As Integer)";
    _out.inc();
    _out << nl << "InnerList.CopyTo(__a, __i)";
    _out.dec();
    _out << nl << "End Sub";

    _out << sp << nl << "Public Sub CopyTo(ByVal __i As Integer, ByVal __a As " << s << "(), "
                        "ByVal __ai As Integer, ByVal __c As Integer)";
    _out.inc();
    _out << nl << "InnerList.CopyTo(__i, __a, __ai, __c)";
    _out.dec();
    _out << nl << "End Sub";

    _out << sp << nl << "Public Function ToArray() As " << s << "()";
    _out.inc();
    _out << nl << "Dim __a As " << s << "() = New " << toArrayAlloc(s + "()", "InnerList.Count - 1") << " {}";
    _out << nl << "InnerList.CopyTo(__a)";
    _out << nl << "Return __a";
    _out.dec();
    _out << nl << "End Function";

    _out.zeroIndent();
    _out << sp << nl << "#End Region"; // Array copy and conversion

    _out << sp << nl << "#Region \"AddRange members\"";
    _out.restoreIndent();

    _out << sp << nl << "Public Sub AddRange(ByVal __s As " << name << ')';
    _out.inc();
    _out << nl << "InnerList.AddRange(__s)";
    _out.dec();
    _out << nl << "End Sub";

    _out << sp << nl << "Public Sub AddRange(ByVal __a As " << s << "())";
    _out.inc();
    _out << nl << "InnerList.AddRange(__a)";
    _out.dec();
    _out << nl << "End Sub";

    _out.zeroIndent();
    _out << sp << nl << "#End Region"; // AddRange members

    _out << sp << nl << "#Region \"ICollectionBase members\"";
    _out.restoreIndent();

    _out << sp << nl << "Public Function Add(ByVal value As " << s << ") As Integer";
    _out.inc();
    _out << nl << "Return InnerList.Add(value)";
    _out.dec();
    _out << nl << "End Function";

    _out << sp << nl << "Public Function IndexOf(ByVal value As " << s << ") As Integer";
    _out.inc();
    _out << nl << "Return InnerList.IndexOf(value)";
    _out.dec();
    _out << nl << "End Function";

    _out << sp << nl << "Public Sub Insert(ByVal index As Integer, ByVal value As " << s << ')';
    _out.inc();
    _out << nl << "InnerList.Insert(index, value)";
    _out.dec();
    _out << nl << "End Sub";

    _out << sp << nl << "Public Sub Remove(ByVal value As " << s << ')';
    _out.inc();
    _out << nl << "InnerList.Remove(value)";
    _out.dec();
    _out << nl << "End Sub";

    _out << sp << nl << "Public Function Contains(ByVal value As " << s << ") As Boolean";
    _out.inc();
    _out << nl << "Return InnerList.Contains(value)";
    _out.dec();
    _out << nl << "End Function";

    _out.zeroIndent();
    _out << sp << nl << "#End Region"; // ICollectionBase members

    _out << sp << nl << "#Region \"ICollection members\"";
    _out.restoreIndent();

    _out << sp << nl << "Readonly Property IsSynchronized As Boolean";
    _out.inc();
    _out << nl << "Get";
    _out.inc();
    _out << nl << "Return False";
    _out.dec();
    _out << nl << "End Get";
    _out.dec();
    _out << nl << "End Property";

    _out << sp << nl << "Readonly Property SyncRoot As Object";
    _out.inc();
    _out << nl << "Get";
    _out.inc();
    _out << nl << "Return Me";
    _out.dec();
    _out << nl << "End Get";
    _out.dec();
    _out << nl << "End Property";

    _out.zeroIndent();
    _out << sp << nl << "#End Region"; // ICollection members

    _out << sp << nl << "#Region \"IList members\"";
    _out.restoreIndent();

    _out << sp << nl << "ReadOnly Property IsFixedSize As Boolean";
    _out.inc();
    _out << nl << "Get";
    _out.inc();
    _out << nl << "Return False";
    _out.dec();
    _out << nl << "End Get";
    _out.dec();
    _out << nl << "End Property";

    _out << sp << nl << "ReadOnly Property IsReadOnly As Boolean";
    _out.inc();
    _out << nl << "Get";
    _out.inc();
    _out << nl << "Return False";
    _out.dec();
    _out << nl << "End Get";
    _out.dec();
    _out << nl << "End Property";

    _out.zeroIndent();
    _out << sp << nl << "#Region \"Indexer\"";
    _out.restoreIndent();

    _out << sp << nl << "Default Property Item(ByVal index As Integer) As " << s;
    _out.inc();
    _out << nl << "Get";
    _out.inc();
    _out << nl << "Return CType(InnerList(index), " << s << ')';
    _out.dec();
    _out << nl << "End Get";
    _out.dec();

    _out.inc();
    _out << nl << "Set";
    _out.inc();
    _out << nl << "InnerList(index) = value";
    _out.dec();
    _out << nl << "End Set";
    _out.dec();
    _out << nl << "End Property";

    _out.zeroIndent();
    _out << sp << nl << "#End Region"; // Indexer

    _out << sp << nl << "#End Region"; // IList members

    _out << sp << nl << "#Region \"ICloneable members\"";
    _out.restoreIndent();

    _out << sp << nl << "Function Clone() As Object Implements _System.ICloneable.Clone";
    _out.inc();
    _out << nl << "Return MemberwiseClone()";
    _out.dec();
    _out << nl << "End Function";

    _out.zeroIndent();
    _out << sp << nl << "#End Region"; // ICloneable members

    _out << sp << nl << "#Region \"Object members\"";
    _out.restoreIndent();

    _out << sp << nl << "Public Overrides Function GetHashCode() As Integer";
    _out.inc();
    _out << nl << "Dim hash As Integer = 0";
    _out << nl << "For i As Integer = 0 To Count - 1";
    _out.inc();
    if(!isValue)
    {
	_out << nl << "If Not InnerList(i) Is Nothing Then";
	_out.inc();
    }
    _out << nl << "hash = 5 * hash + InnerList(i).GetHashCode()";
    if(!isValue)
    {
	_out.dec();
	_out << nl << "End If";
    }
    _out.dec();
    _out << nl << "Next";
    _out << nl << "Return hash";
    _out.dec();
    _out << nl << "End Function";

    _out << sp << nl << "Public Overloads Overrides Function Equals(ByVal other As Object) As Boolean";
    _out.inc();
    _out << nl << "If Object.ReferenceEquals(Me, other) Then";
    _out.inc();
    _out << nl << "Return True";
    _out.dec();
    _out << nl << "End If";
    _out << nl << "If Not TypeOf other Is " << name << " Then";
    _out.inc();
    _out << nl << "Return False";
    _out.dec();
    _out << nl << "End If";
    _out << nl << "If Count <> CType(other, " << name << ").Count Then";
    _out.inc();
    _out << nl << "Return False";
    _out.dec();
    _out << nl << "End If";
    _out << nl << "For __i As Integer = 0 To Count - 1";
    _out.inc();
    if(!isValue)
    {
	_out << nl << "If InnerList(__i) Is Nothing And Not CType(other, " << name << ")(__i) Is Nothing Then";
	_out.inc();
	_out << nl << "Return False";
	_out.dec();
	_out << nl << "End If";
    }
    _out << nl << "If Not CType(InnerList(__i), Integer).Equals(CType(other, " << name << ")(__i)) Then";
    _out.inc();
    _out << nl << "Return False";
    _out.dec();
    _out << nl << "End If";
    _out.dec();
    _out << nl << "Next";
    _out << nl << "Return True";
    _out.dec();
    _out << nl << "End Function";

    _out.zeroIndent();
    _out << sp << nl << "#End Region"; // Object members

    _out << sp << nl << "#Region \"Shared Equals\"";
    _out.restoreIndent();

    _out << sp << nl << "Public Overloads Shared Function Equals(ByVal __lhs As " << name
         << ", ByVal __rhs As " << name << ") As Boolean";
    _out.inc();
    _out << nl << "If Object.ReferenceEquals(__rhs, Nothing) Then";
    _out.inc();
    _out << nl << "Return Object.ReferenceEquals(__rhs, Nothing)";
    _out.dec();
    _out << nl << "Else";
    _out.inc();
    _out << nl << "Return __lhs.Equals(__rhs)";
    _out.dec();
    _out << nl << "End If";
    _out.dec();
    _out << nl << "End Function";

    _out.zeroIndent();
    _out << sp << nl << "#End Region"; // Shared Equals
    _out.restoreIndent();

    _out.dec();
    _out << sp << nl << "End Class";
}

bool
Slice::Gen::TypesVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    string name = fixId(p->name());
    ExceptionPtr base = p->base();

    _out << sp << nl << "Public Class " << name;
    _out.inc();
    if(base)
    {
        _out << nl << "Inherits " << fixId(base->scoped());
    }
    else
    {
        _out << nl << "Inherits " << (p->isLocal() ? "Ice.LocalException" : "Ice.UserException");
    }

    if(!p->dataMembers().empty())
    {
	_out.zeroIndent();
	_out << sp << nl << "#Region \"Slice data members\"";
	_out.restoreIndent();
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
	_out.zeroIndent();
	_out << sp << nl << "#End Region"; // Slice data members
	_out.restoreIndent();
    }

    _out.zeroIndent();
    _out << sp << nl << "#Region \"Constructors\"";
    _out.restoreIndent();

    _out << sp << nl << "Private Shared ReadOnly _dflt As String = \"" << p->name() << "\"";

    _out << sp << nl << "Public Sub New()";
    _out.inc();
    _out << nl << "MyBase.New()";
    _out.dec();
    _out << nl << "End Sub";

    _out << sp << nl << "Public Sub New(ByVal __m As String)";
    _out.inc();
    _out << nl << "MyBase.New(__m)";
    _out.dec();
    _out << nl << "End Sub";

    _out << sp << nl << "Public Sub New(ByVal __ex As _System.Exception)";
    _out.inc();
    _out << nl << "MyBase.New(_dflt, __ex)";
    _out.dec();
    _out << nl << "End Sub";

    _out << sp << nl << "Public Sub New(ByVal __m As String, ByVal __ex As _System.Exception)";
    _out.inc();
    _out << nl << "MyBase.New(__m, __ex)";
    _out.dec();
    _out << nl << "End Sub";

    _out.zeroIndent();
    _out << sp << nl << "#End Region"; // Constructors

    _out << sp << nl << "#Region \"Object members\"";
    _out.restoreIndent();

    _out << sp << nl << "Public Overrides Function GetHashCode() As Integer";
    _out.inc();
    _out << nl << "Dim __h As Integer = 0";
    for(q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        string memberName = fixId((*q)->name(), DotNet::ApplicationException);
	bool isValue = isValueType((*q)->type());
	if(!isValue)
	{
	    _out << nl << "If Not " << memberName << " Is Nothing Then";
	    _out.inc();
	}
	ClassDeclPtr cl = ClassDeclPtr::dynamicCast((*q)->type());
	if(cl && cl->isInterface()) // Bug in VB 7.1: cast should not be necessary.
	{
	    _out << nl << "' Bug in VB 7.1: cast to Object should not be necessary.";
	    _out << nl << "__h = 5 * __h + CType(" << memberName << ", Object).GetHashCode()";
	}
	else if(ProxyPtr::dynamicCast((*q)->type()))
	{
	    _out << nl << "' Bug in VB 7.1: cast should not be necessary.";
	    _out << nl << "__h = 5 * __h + CType(" << memberName << ", "
		 << typeToString((*q)->type()) << "Helper).GetHashCode()";
	}
	else
	{
	    _out << nl << "__h = 5 * __h + " << memberName << ".GetHashCode()";
	}
	if(!isValue)
	{
	    _out.dec();
	    _out << nl << "End If";
	}
    }
    _out << nl << "Return __h";
    _out.dec();
    _out << nl << "End Function";

    _out << sp << nl << "Public Overloads Overrides Function Equals(ByVal __other As Object) As Boolean";
    _out.inc();
    _out << nl << "If __other Is Nothing Then";
    _out.inc();
    _out << nl << "Return False";
    _out.dec();
    _out << nl << "End If";
    _out << nl << "If Object.ReferenceEquals(Me, __other) Then";
    _out.inc();
    _out << nl << "Return True";
    _out.dec();
    _out << nl << "End If";
    _out << nl << "If Not TypeOf __other Is " << name << " Then";
    _out.inc();
    _out << nl << "Throw New _System.ArgumentException(\"expected argument of type `" << p->name()
         << "'\", \"__other\")";
    _out.dec();
    _out << nl << "End If";
    for(q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        string memberName = fixId((*q)->name(), DotNet::ApplicationException);

	ClassDeclPtr cl = ClassDeclPtr::dynamicCast((*q)->type());
	if(cl && cl->isInterface()) // Bug in VB 7.1: cast should not be necessary.
	{
	    _out << nl << "' Bug in VB 7.1: cast to Object should not be necessary.";
	    _out << nl << "If Not CType(" << memberName << ", Object).Equals(CType(__other, "
	         << name << ")." << memberName << ") Then";
	}
	else if(ProxyPtr::dynamicCast((*q)->type()))
	{
	    _out << nl << "' Bug in VB 7.1: cast should not be necessary.";
	    _out << nl << "If Not CType(" << memberName << ", " << typeToString((*q)->type())
	         << "Helper).Equals(CType(__other, " << name << ")." << memberName << ") Then";
	}
	else
	{
	    _out << nl << "If Not " << memberName << ".Equals(CType(__other, " << name << ")."
	         << memberName << ") Then";
	}
	_out.inc();
	_out << nl << "Return False";
	_out.dec();
	_out << nl << "End If";
    }
    _out << nl << "Return True";
    _out.dec();
    _out << nl << "End Function";

    _out.zeroIndent();
    _out << sp << nl << "#End Region"; // Object members

    _out << sp << nl << "#Region \"Shared Equals\"";
    _out.restoreIndent();

    _out << sp << nl << "Public Overloads Shared Function Equals(ByVal __lhs As " << name
         << ", ByVal __rhs As " << name << ") As Boolean";

    _out.inc();
    _out << nl << "If Object.ReferenceEquals(__rhs, Nothing) Then";
    _out.inc();
    _out << nl << "Return Object.ReferenceEquals(__rhs, Nothing)";
    _out.dec();
    _out << nl << "Else";
    _out.inc();
    _out << nl << "Return __lhs.Equals(__rhs)";
    _out.dec();
    _out << nl << "End If";
    _out.dec();
    _out << nl << "End Function";

    _out.zeroIndent();
    _out << sp << nl << "#End Region"; // Comparison members
    _out.restoreIndent();

    if(!p->isLocal())
    {
	_out.zeroIndent();
        _out << sp << nl << "#Region \"Marshaling support\"";
	_out.restoreIndent();

        string scoped = p->scoped();
        ExceptionPtr base = p->base();


        _out << sp << nl << "Public Overloads Overrides Sub __write(ByVal __os As IceInternal.BasicStream)";
        _out.inc();
	_out << nl << "__os.writeString(\"" << scoped << "\")";
	_out << nl << "__os.startWriteSlice()";
        for(q = dataMembers.begin(); q != dataMembers.end(); ++q)
        {
            writeMarshalUnmarshalCode(_out, (*q)->type(),
	                              fixId((*q)->name(), DotNet::ApplicationException),
				      true, false, false);
        }
	_out << nl << "__os.endWriteSlice()";
        if(base)
        {
            _out << nl << "MyBase.__write(__os)";
        }
	_out.dec();
	_out << nl << "End Sub";

	DataMemberList allClassMembers = p->allClassDataMembers();
	if(allClassMembers.size() != 0)
	{
	    _out << sp << nl << "Public NotInheritable Class __Patcher";
	    _out.inc();
	    _out << nl << "Inherits IceInternal.Patcher";
	    _out << sp << nl << "Friend Sub New(ByVal instance As Ice.Exception";
	    if(allClassMembers.size() > 1)
	    {
	        _out << ", ByVal member As Integer";
	    }
	    _out << ')';
	    _out.inc();
	    _out << nl << "_instance = CType(instance, " << name << ')';
	    if(allClassMembers.size() > 1)
	    {
		_out << nl << "_member = member";
	    }
	    _out.dec();
	    _out << nl << "End Sub";

	    _out << sp << nl << "Public Overrides Sub patch(ByVal v As Ice.Object)";
	    _out.inc();
	    if(allClassMembers.size() > 1)
	    {
		_out << nl << "Select Case _member";
		_out.inc();
	    }
	    int memberCount = 0;
	    for(q = allClassMembers.begin(); q != allClassMembers.end(); ++q)
	    {
		if(allClassMembers.size() > 1)
		{
		    _out << nl << "Case " << memberCount;
		    _out.inc();
		}
		string memberName = fixId((*q)->name(), DotNet::ApplicationException);
		string memberType = typeToString((*q)->type());
		_out << nl << "_type = GetType(" << memberType << ')';
		_out << nl << "_instance." << memberName << " = CType(v, " << memberType << ')';
		if(allClassMembers.size() > 1)
		{
		    _out.dec();
		}
		memberCount++;
	    }
	    if(allClassMembers.size() > 1)
	    {
		_out.dec();
		_out << nl << "End Select";
	    }
	    _out.dec();
	    _out << nl << "End Sub";

	    _out << sp << nl << "Private _instance As " << name;
	    if(allClassMembers.size() > 1)
	    {
		_out << nl << "Private _member As Integer";
	    }
	    _out.dec();
	    _out << nl << "End Class";
	}
        _out << sp << nl << "Public Overloads Overrides Sub __read(ByVal __is As IceInternal.BasicStream, "
	                    "ByVal __rid As Boolean)";
        _out.inc();
	_out << nl << "If __rid Then";
	_out.inc();
	_out << nl << "Dim myId As String = __is.readString()";
	_out.dec();
	_out << nl << "End If";
	_out << nl << "__is.startReadSlice()";
	DataMemberList classMembers = p->classDataMembers();
	int classMemberCount = static_cast<int>(allClassMembers.size() - classMembers.size());
        for(q = dataMembers.begin(); q != dataMembers.end(); ++q)
        {
	    ostringstream patchParams;
	    patchParams << "Me";
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
	_out << nl << "__is.endReadSlice()";
        if(base)
        {
            _out << nl << "MyBase.__read(__is, true)";
        }
        _out.dec();
	_out << nl << "End Sub";

	if(_stream)
	{
	    _out << sp << nl << "Public Overloads Overrides Sub __write(ByVal __outS As Ice.OutputStream)";
	    _out.inc();
	    _out << nl << "__outS.writeString(\"" << scoped << "\")";
	    _out << nl << "__outS.startSlice()";
	    for(q = dataMembers.begin(); q != dataMembers.end(); ++q)
	    {
		writeMarshalUnmarshalCode(_out, (*q)->type(),
					  fixId((*q)->name(), DotNet::ApplicationException),
					  true, true, false);
	    }
	    _out << nl << "__outS.endSlice()";
	    if(base)
	    {
		_out << nl << "MyBase.__write(__outS)";
	    }
	    _out.dec();
	    _out << nl << "End Sub";

	    _out << sp << nl << "Public Overloads Overrides Sub __read(ByVal __inS As Ice.InputStream, "
		 << "ByVal __rid As Boolean)";
	    _out.inc();
	    _out << nl << "If __rid Then";
	    _out.inc();
	    _out << nl << "Dim myId As String = __inS.readString()";
	    _out.dec();
	    _out << nl << "End If";
	    _out << nl << "__inS.startSlice()";
	    classMemberCount = static_cast<int>(allClassMembers.size() - classMembers.size());
	    for(q = dataMembers.begin(); q != dataMembers.end(); ++q)
	    {
		ostringstream patchParams;
		patchParams << "Me";
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
	    _out << nl << "__inS.endSlice()";
	    if(base)
	    {
		_out << nl << "MyBase.__read(__inS, true)";
	    }
	    _out.dec();
	    _out << nl << "End Sub";
	}
	else
	{
	    //
	    // Emit placeholder functions to catch errors.
	    //
            string scoped = p->scoped();
	    _out << sp << nl << "Public Overloads Overrides Sub __write(ByVal __outS As Ice.OutputStream)";
	    _out.inc();
	    _out << nl << "Dim ex As Ice.MarshalException = New Ice.MarshalException";
	    _out << nl << "ex.reason = \"type " << scoped.substr(2) << " was not generated with stream support\"";
	    _out << nl << "Throw ex";
	    _out.dec();
	    _out << nl << "End Sub";

	    _out << sp << nl << "Public Overloads Overrides Sub __read"
	         << "(ByVal __inS As Ice.InputStream, ByVal __rid As Boolean)";
	    _out.inc();
	    _out << nl << "Dim ex As Ice.MarshalException = New Ice.MarshalException";
	    _out << nl << "ex.reason = \"type " << scoped.substr(2) << " was not generated with stream support\"";
	    _out << nl << "Throw ex";
	    _out.dec();
	    _out << nl << "End Sub";
	}

	if(!base || base && !base->usesClasses())
	{
	    _out << sp << nl << "Public Overrides Function __usesClasses() As Boolean";
	    _out.inc();
	    _out << nl << "Return True";
	    _out.dec();
	    _out << nl << "End Function";
	}

	_out.zeroIndent();
        _out << sp << nl << "#End Region"; // Marshalling support
	_out.restoreIndent();
    }

    _out.dec();
    _out << sp << nl << "End Class";
}

bool
Slice::Gen::TypesVisitor::visitStructStart(const StructPtr& p)
{
    string name = fixId(p->name());

    if(!p->isLocal() && _stream)
    {
        _out << sp << nl << "Public NotInheritable Class " << p->name() << "Helper";
	_out.inc();

	_out << sp << nl << "Public Shared Sub write(ByVal __outS As Ice.OutputStream, ByVal __v As " << name << ')';
	_out.inc();
	_out << nl << "__v.__write(__outS)";
	_out.dec();
	_out << nl << "End Sub";

	_out << sp << nl << "Public Shared Function read(ByVal __inS As Ice.InputStream) As " << name;
	_out.inc();
	_out << nl << "Dim __v As " << name << " = New " << name;
	_out << nl << "__v.__read(__inS)";
	_out << nl << "Return __v";
	_out.dec();
	_out << nl << "End Function";

	_out.dec();
	_out << nl << "End Class";
    }

    if(p->hasMetaData("vb:class"))
    {
	_out << sp << nl << "Public Class " << name << " Implements _System.ICloneable";
    }
    else
    {
	_out << sp << nl << "Public Structure " << name;
    }
    _out.inc();

    _out.zeroIndent();
    _out << sp << nl << "#Region \"Slice data members\"";
    _out.restoreIndent();

    return true;
}

void
Slice::Gen::TypesVisitor::visitStructEnd(const StructPtr& p)
{
    string name = fixId(p->name());

    DataMemberList classMembers = p->classDataMembers();
    DataMemberList dataMembers = p->dataMembers();
    DataMemberList::const_iterator q;

    _out.zeroIndent();
    _out << sp << nl << "#End Region"; // Slice data members

    bool isClass = p->hasMetaData("vb:class");

    _out << sp << nl << "#Region \"Constructor";
    if(isClass)
    {
        _out << "s";
    }
    _out << "\"";
    _out.restoreIndent();

    if(isClass)
    {
        _out << sp << nl << "Public Sub New()";
	_out << nl << "End Sub";
    }

    _out << sp << nl << "Public Sub New" << spar;
    vector<string> paramDecl;
    vector<string> paramNames;
    for(q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        string memberName = fixId((*q)->name());
	string memberType = typeToString((*q)->type());
	paramDecl.push_back("ByVal " + memberName + " As " + memberType);
	paramNames.push_back(memberName);
    }
    _out << paramDecl << epar;
    _out.inc();
    for(vector<string>::const_iterator i = paramNames.begin(); i != paramNames.end(); ++i)
    {
        _out << nl << "Me." << *i << " = " << *i;
    }
    bool patchStruct = !isClass && classMembers.size() != 0;
    if(!p->isLocal() && patchStruct)
    {
        _out << nl << "_pm = Nothing";
    }
    _out.dec();
    _out << nl << "End Sub";

    _out.zeroIndent();
    _out << sp << nl << "#End Region"; // Constructor(s)
    _out.restoreIndent();

    if(isClass)
    {
	_out.zeroIndent();
	_out << sp << nl << "#Region \"ICloneable members\"";
	_out.restoreIndent();

	_out << sp << nl << "Function Clone() As Object Implements _System.ICloneable.Clone";
	_out.inc();
	_out << nl << "Return MemberwiseClone()";
	_out.dec();
	_out << nl << "End Function";

	_out.zeroIndent();
	_out << sp << nl << "#End Region"; // ICloneable members
	_out.restoreIndent();
    }

    _out.zeroIndent();
    _out << sp << nl << "#Region \"Object members\"";
    _out.restoreIndent();

    _out << sp << nl << "Public Overrides Function GetHashCode() As Integer";
    _out.inc();
    _out << nl << "Dim __h As Integer = 0";
    for(q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        string memberName = fixId((*q)->name(), isClass ? DotNet::ICloneable : 0);
	bool isValue = isValueType((*q)->type());
	if(!isValue)
	{
	    _out << nl << "If Not " << memberName << " Is Nothing Then";
	    _out.inc();
	}
	ClassDeclPtr cl = ClassDeclPtr::dynamicCast((*q)->type());
	if(cl && cl->isInterface()) // Bug in VB 7.1: cast should not be necessary.
	{
	    _out << nl << "' Bug in VB 7.1: cast to Object should not be necessary.";
	    _out << nl << "__h = 5 * __h + CType(" << memberName << ", Object).GetHashCode()";
	}
	else if(ProxyPtr::dynamicCast((*q)->type()))
	{
	    _out << nl << "' Bug in VB 7.1: cast should not be necessary.";
	    _out << nl << "__h = 5 * __h + CType(" << memberName << ", "
		 << typeToString((*q)->type()) << "Helper).GetHashCode()";
	}
	else
	{
	    _out << nl << "__h = 5 * __h + " << memberName << ".GetHashCode()";
	}
	if(!isValue)
	{
	    _out.dec();
	    _out << nl << "End If";
	}
    }
    _out << nl << "Return __h";
    _out.dec();
    _out << nl << "End Function";

    _out << sp << nl << "Public Overloads Overrides Function Equals(ByVal __other As Object) As Boolean";
    _out.inc();
    _out << nl << "If Object.ReferenceEquals(Me, __other) Then";
    _out.inc();
    _out << nl << "Return True";
    _out.dec();
    _out << nl << "End If";
    _out << nl << "If Not TypeOf __other Is " << name << " Then";
    _out.inc();
    _out << nl << "Return False";
    _out.dec();
    _out << nl << "End If";
    for(q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        string memberName = fixId((*q)->name(), isClass ? DotNet::ICloneable : 0);
	if(!isValueType((*q)->type()))
	{
	    _out << nl << "If " << memberName << " Is Nothing Then";
	    _out.inc();
	    _out << nl << "If Not CType(__other, " << name << ")." << memberName << " Is Nothing Then";
	    _out.inc();
	    _out << nl << "Return False";
	    _out.dec();
	    _out << nl << "End If";
	    _out.dec();
	    _out << nl << "Else";
	    _out.inc();
	    ClassDeclPtr cl = ClassDeclPtr::dynamicCast((*q)->type());
	    if(cl && cl->isInterface()) // Bug in VB 7.1: cast should not be necessary.
	    {
		_out << nl << "' Bug in VB 7.1: cast to Object should not be necessary.";
		_out << nl << "If Not CType(" << memberName << ", Object).Equals(CType(__other, "
		     << name << ")." << memberName << ") Then";
	    }
	    else if(ProxyPtr::dynamicCast((*q)->type()))
	    {
		_out << nl << "' Bug in VB 7.1: cast should not be necessary.";
		_out << nl << "If Not CType(" << memberName << ", " << typeToString((*q)->type())
		     << "Helper).Equals(CType(__other, " << name << ")." << memberName << ") Then";
	    }
	    else
	    {
		_out << nl << "If Not " << memberName << ".Equals(CType(__other, " << name << ")."
		     << memberName << ") Then";
	    }
	    _out.inc();
	    _out << nl << "Return False";
	    _out.dec();
	    _out << nl << "End If";
	    _out.dec();
	    _out << nl << "End If";
	}
	else
	{
	    _out << nl << "If Not " << memberName << ".Equals(CType(__other, " << name << ")."
	         << memberName << ") Then";
	    _out.inc();
	    _out << nl << "Return False";
	    _out.dec();
	    _out << nl << "End If";
	}
    }
    _out << nl << "Return True";
    _out.dec();
    _out << nl << "End Function";

    _out.zeroIndent();
    _out << sp << nl << "#End Region"; // Object members

    _out << sp << nl << "#Region \"Shared Equals\"";
    _out.restoreIndent();

    _out << sp << nl << "Public Overloads Shared Function Equals(ByVal __lhs As " << name
         << ", ByVal __rhs As " << name << ") As Boolean";

    _out.inc();
    _out << nl << "If Object.ReferenceEquals(__rhs, Nothing) Then";
    _out.inc();
    _out << nl << "Return Object.ReferenceEquals(__rhs, Nothing)";
    _out.dec();
    _out << nl << "Else";
    _out.inc();
    _out << nl << "Return __lhs.Equals(__rhs)";
    _out.dec();
    _out << nl << "End If";
    _out.dec();
    _out << nl << "End Function";

    _out.zeroIndent();
    _out << sp << nl << "#End Region"; // Shared Equals
    _out.restoreIndent();

    if(!p->isLocal())
    {
	_out.zeroIndent();
        _out << sp << nl << "#Region \"Marshalling support\"";
	_out.restoreIndent();

        _out << sp << nl << "Public Sub __write(ByVal __os As IceInternal.BasicStream)";
        _out.inc();
	for(q = dataMembers.begin(); q != dataMembers.end(); ++q)
	{
	    writeMarshalUnmarshalCode(_out, (*q)->type(),
	                              fixId((*q)->name(), isClass ? DotNet::ICloneable : 0),
				      true, false, false);
	}
        _out.dec();
	_out << nl << "End Sub";


	if(classMembers.size() != 0)
	{
	    _out << sp << nl << "Public NotInheritable Class __Patcher";
	    _out.inc();
	    _out << nl << "Inherits IceInternal.Patcher";
	    _out << sp << nl << "Friend Sub New(ByVal instance As " << name;
	    if(patchStruct)
	    {
	        _out << ".__PatchMembers";
	    }
	    if(classMembers.size() > 1)
	    {
	    	_out << ", ByVal member As Integer";
	    }
	    _out << ')';
	    _out.inc();
	    _out << nl << "_instance = instance";
	    if(classMembers.size() > 1)
	    {
		_out << nl << "_member = member";
	    }
	    _out.dec();
	    _out << nl << "End Sub";

	    _out << sp << nl << "Public Overrides Sub patch(ByVal v As Ice.Object)";
	    _out.inc();
	    if(classMembers.size() > 1)
	    {
		_out << nl << "Select Case _member";
		_out.inc();
	    }
	    int memberCount = 0;
	    for(q = classMembers.begin(); q != classMembers.end(); ++q)
	    {
		if(classMembers.size() > 1)
		{
		    _out << nl << "Case " << memberCount;
		    _out.inc();
		}
		string memberType = typeToString((*q)->type());
		string memberName = fixId((*q)->name(), isClass ? DotNet::ICloneable : 0);
		_out << nl << "_type = GetType(" << memberType << ')';
		_out << nl << "_instance." << memberName << " = CType(v, " << memberType << ')';
		if(classMembers.size() > 1)
		{
		    _out.dec();
		}
		memberCount++;
	    }
	    if(classMembers.size() > 1)
	    {
		_out.dec();
		_out << nl << "End Select";
	    }
	    _out.dec();
	    _out << nl << "End Sub";

	    _out << sp << nl << "Private _instance As " << name;
	    if(patchStruct)
	    {
	        _out << ".__PatchMembers";
	    }
	    if(classMembers.size() > 1)
	    {
		_out << nl << "Private _member As Integer";
	    }
	    _out.dec();
	    _out << nl << "End Class";

	    if(patchStruct)
	    {
		_out << sp << nl << "Friend Class __PatchMembers";
		_out.inc();
		for(q = classMembers.begin(); q != classMembers.end(); ++q)
		{
		    string memberType = typeToString((*q)->type());
		    string memberName = fixId((*q)->name(), isClass ? DotNet::ICloneable : 0);
		    _out << nl << "Friend " << memberName << " As " << memberType;
		}
		_out.dec();
		_out << nl << "End Class";

		_out << sp << nl << "Private _pm As __PatchMembers";

		_out << sp << nl << "Public Sub __patch()";
		_out.inc();
		for(q = classMembers.begin(); q != classMembers.end(); ++q)
		{
		    string memberName = fixId((*q)->name(), isClass ? DotNet::ICloneable : 0);
		    _out << nl << memberName << " = _pm." << memberName;
		}
		_out.dec();
		_out << nl << "End Sub";
	    }
	}

        _out << sp << nl << "Public Sub __read(ByVal __is As IceInternal.BasicStream)";
        _out.inc();
	if(patchStruct)
	{
	    _out << nl << "If _pm Is Nothing Then";
	    _out.inc();
	    _out << nl << "_pm = new __PatchMembers()";
	    _out.dec();
	    _out << nl << "End If";
	}
	int classMemberCount = 0;
        for(q = dataMembers.begin(); q != dataMembers.end(); ++q)
        {
	    ostringstream patchParams;
	    patchParams << (patchStruct ? "_pm" : "Me");
	    BuiltinPtr builtin = BuiltinPtr::dynamicCast((*q)->type());
	    if((builtin && builtin->kind() == Builtin::KindObject) || ClassDeclPtr::dynamicCast((*q)->type()))
	    {
		if(classMembers.size() > 1)
		{
		    patchParams << ", " << classMemberCount++;
		}
	    }
            writeMarshalUnmarshalCode(_out, (*q)->type(),
	                              fixId((*q)->name(), isClass ? DotNet::ICloneable : 0),
				      false, false, false, patchParams.str());
        }
        _out.dec();
	_out << nl << "End Sub";

	if(_stream)
	{
	    _out << sp << nl << "Public Sub __write(ByVal __outS As Ice.OutputStream)";
	    _out.inc();
	    for(q = dataMembers.begin(); q != dataMembers.end(); ++q)
	    {
		writeMarshalUnmarshalCode(_out, (*q)->type(),
					  fixId((*q)->name(), isClass ? DotNet::ICloneable : 0),
					  true, true, false);
	    }
	    _out.dec();
	    _out << nl << "End Sub";

	    _out << sp << nl << "Public Sub __read(ByVal __inS As Ice.InputStream)";
	    _out.inc();
	    if(patchStruct)
	    {
		_out << nl << "If _pm Is Nothing";
		_out.inc();
		_out << nl << "_pm = New __PatchMembers";
		_out.dec();
		_out << nl << "End If";
	    }
	    classMemberCount = 0;
	    for(q = dataMembers.begin(); q != dataMembers.end(); ++q)
	    {
		ostringstream patchParams;
		patchParams << (patchStruct ? "_pm" : "Me");
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
	    _out.dec();
	    _out << nl << "End Sub";
	}

	_out.zeroIndent();
        _out << sp << nl << "#End Region"; // Marshalling support
	_out.restoreIndent();
    }

    _out.dec();
    _out << sp << nl << "End Structure";
}

void
Slice::Gen::TypesVisitor::visitDictionary(const DictionaryPtr& p)
{
    string name = fixId(p->name());
    string ks = typeToString(p->keyType());
    string vs = typeToString(p->valueType());
    bool valueIsValue = isValueType(p->valueType());

    _out << sp << nl << "Public Class " << name;
    _out.inc();
    _out << nl << "Inherits _System.Collections.DictionaryBase";
    _out << nl << "Implements _System.ICloneable";

    _out.zeroIndent();
    _out << sp << nl << "#Region \"" << name << " members\"";
    _out.restoreIndent();

    _out << sp << nl << "Public Sub AddRange(ByVal __d As " << name << ')';
    _out.inc();
    _out << nl << "For Each e As _System.Collections.DictionaryEntry in __d";
    _out.inc();
    _out << nl << "Try";
    _out.inc();
    _out << nl << "InnerHashtable.Add(e.Key, e.Value)";
    _out.dec();
    _out << nl << "Catch ex As _System.ArgumentException";
    _out.inc();
    _out << nl << "' ignore";
    _out.dec();
    _out << nl << "End Try";
    _out.dec();
    _out << nl << "Next";
    _out.dec();
    _out << nl << "End Sub";

    _out.zeroIndent();
    _out << sp << nl << "#End Region"; // <name> members

    _out << sp << nl << "#Region \"IDictionary members\"";
    _out.restoreIndent();

    _out << sp << nl << "ReadOnly Property IsFixedSize As Boolean";
    _out.inc();
    _out << nl << "Get";
    _out.inc();
    _out << nl << "Return False";
    _out.dec();
    _out << nl << "End Get";
    _out.dec();
    _out << nl << "End Property";

    _out << sp << nl << "ReadOnly Property IsReadOnly As Boolean";
    _out.inc();
    _out << nl << "Get";
    _out.inc();
    _out << nl << "Return False";
    _out.dec();
    _out << nl << "End Get";
    _out.dec();
    _out << nl << "End Property";

    _out << sp << nl << "ReadOnly Property Keys As _System.Collections.ICollection";
    _out.inc();
    _out << nl << "Get";
    _out.inc();
    _out << nl << "Return InnerHashtable.Keys";
    _out.dec();
    _out << nl << "End Get";
    _out.dec();
    _out << nl << "End Property";

    _out << sp << nl << "ReadOnly Property Values As _System.Collections.ICollection";
    _out.inc();
    _out << nl << "Get";
    _out.inc();
    _out << nl << "Return InnerHashtable.Values";
    _out.dec();
    _out << nl << "End Get";
    _out.dec();
    _out << nl << "End Property";

    _out.zeroIndent();
    _out << sp << nl << "#Region \"Indexer\"";
    _out.restoreIndent();

    _out << sp << nl << "Default Property Item(ByVal key As " << ks << ") As " << vs;
    _out.inc();
    _out << nl << "Get";
    _out.inc();
    _out << nl << "return CType(InnerHashtable(key), " << vs << ')';
    _out.dec();
    _out << nl << "End Get";

    _out << nl << "Set";
    _out.inc();
    _out << nl << "InnerHashtable(key) = value";
    _out.dec();
    _out << nl << "End Set";
    _out.dec();
    _out << nl << "End Property";

    _out.zeroIndent();
    _out << sp << nl << "#End Region"; // Indexer
    _out.restoreIndent();

    _out << sp << nl << "Public Sub Add(ByVal key As " << ks << ", ByVal value As " << vs << ')';
    _out.inc();
    _out << nl << "InnerHashtable.Add(key, value)";
    _out.dec();
    _out << nl << "End Sub";

    _out << sp << nl << "Public Sub Remove(ByVal key As " << ks << ')';
    _out.inc();
    _out << nl << "InnerHashtable.Remove(key)";
    _out.dec();
    _out << nl << "End Sub";

    _out << sp << nl << "Public Function Contains(ByVal key As " << ks << ')';
    _out.inc();
    _out << nl << "return InnerHashtable.Contains(key)";
    _out.dec();
    _out << nl << "End Function";

    _out.zeroIndent();
    _out << sp << nl << "#End Region"; // IDictionary members

    _out << sp << nl << "#Region \"ICollection members\"";
    _out.restoreIndent();

    _out << sp << nl << "Readonly Property IsSynchronized As Boolean";
    _out.inc();
    _out << nl << "Get";
    _out.inc();
    _out << nl << "Return False";
    _out.dec();
    _out << nl << "End Get";
    _out.dec();
    _out << nl << "End Property";

    _out << sp << nl << "Readonly Property SyncRoot As Object";
    _out.inc();
    _out << nl << "Get";
    _out.inc();
    _out << nl << "Return Me";
    _out.dec();
    _out << nl << "End Get";
    _out.dec();
    _out << nl << "End Property";

    _out.zeroIndent();
    _out << sp << nl << "#End Region"; // ICollection members

    _out << sp << nl << "#Region \"ICloneable members\"";
    _out.restoreIndent();

    _out << sp << nl << "Function Clone() As Object Implements _System.ICloneable.Clone";
    _out.inc();
    _out << nl << "Return MemberwiseClone()";
    _out.dec();
    _out << nl << "End Function";

    _out.zeroIndent();
    _out << sp << nl << "#End Region"; // ICloneable members

    _out << sp << nl << "#Region \"Object members\"";
    _out.restoreIndent();

    _out << sp << nl << "Public Overrides Function GetHashCode() As Integer";
    _out.inc();
    _out << nl << "Dim hash As Integer = 0";
    _out << nl << "For Each e As _System.Collections.DictionaryEntry in InnerHashtable";
    _out.inc();
    _out << nl << "hash = 5 * hash + e.Key.GetHashCode()";
    if(!valueIsValue)
    {
	_out << nl << "If Not e.Value Is Nothing";
	_out.inc();
    }
    _out << nl << "hash = 5 * hash + e.Value.GetHashCode()";
    if(!valueIsValue)
    {
	_out.dec();
	_out << nl << "End If";
    }
    _out.dec();
    _out << nl << "Next";
    _out << nl << "Return hash";
    _out.dec();
    _out << nl << "End Function";

    _out << sp << nl << "Public Overloads Overrides Function Equals(ByVal other As Object) As Boolean";
    _out.inc();
    _out << nl << "If Object.ReferenceEquals(Me, other)";
    _out.inc();
    _out << nl << "Return True";
    _out.dec();
    _out << nl << "End If";
    _out << nl << "If Not TypeOf other Is " << name << " Then";
    _out.inc();
    _out << nl << "Return False";
    _out.dec();
    _out << nl << "End If";
    _out << nl << "If Count <> CType(other, " << name << ").Count";
    _out.inc();
    _out << nl << "Return False";
    _out.dec();
    _out << nl << "End If";
    _out << nl << "Dim __klhs() As " << ks << " = New " << ks << "(Count - 1) {}";
    _out << nl << "Keys.CopyTo(__klhs, 0)";
    _out << nl << "_System.Array.Sort(__klhs)";
    _out << nl << "Dim __krhs() As " << ks << " = New " << ks << "(CType(other, " << name << ").Count - 1) {}";
    _out << nl << "CType(other, " << name << ").Keys.CopyTo(__krhs, 0)";
    _out << nl << "_System.Array.Sort(__krhs)";
    _out << nl << "For i As Integer = 0 To Count - 1";
    _out.inc();
    _out << nl << "If Not __klhs(i).Equals(__krhs(i))";
    _out.inc();
    _out << nl << "Return False";
    _out.dec();
    _out << nl << "End If";
    _out.dec();
    _out << nl << "Next";
    SequencePtr seq = SequencePtr::dynamicCast(p->valueType());
    bool valueIsArray = seq && !seq->hasMetaData("vb:collection");
    if(valueIsArray)
    {
	_out << nl << "Dim __vlhs As " << vs << "() = New " << toArrayAlloc(vs + "()", "Count - 1") << " {}";
    }
    else
    {
	_out << nl << "Dim __vlhs As " << vs << "() = New " << vs << "(Count - 1) {}";
    }
    _out << nl << "Values.CopyTo(__vlhs, 0)";
    _out << nl << "_System.Array.Sort(__vlhs)";
    string vrhsCount = "Ctype(other, " + name + ").Count - 1";
    if(valueIsArray)
    {
	_out << nl << "Dim __vrhs As " << vs << "() = New " << toArrayAlloc(vs + "()", vrhsCount) << " {}";
    }
    else
    {
	_out << nl << "Dim __vrhs As " << vs << "() = New " << vs << '(' << vrhsCount << ") {}";
    }
    _out << nl << "CType(other, " << name << ").Values.CopyTo(__vrhs, 0)";
    _out << nl << "_System.Array.Sort(__vrhs)";
    _out << nl << "For i As Integer = 0 To Count - 1";
    _out.inc();
    if(!valueIsValue)
    {
	_out << nl << "If __vlhs(i) Is Nothing And Not __vrhs(i) Is Nothing";
	_out.inc();
	_out << nl << "Return False";
	_out.dec();
	_out << nl << "End If";
    }
    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(p->valueType());
    if(cl && cl->isInterface()) // Bug in VB 7.1: cast should not be necessary.
    {
	_out << nl << "' Bug in VB 7.1: cast to Object should not be necessary.";
        _out << nl << "If Not CType(__vlhs(i), Object).Equals(__vrhs(i))";
    }
    else if(ProxyPtr::dynamicCast(p->valueType()))
    {
	_out << nl << "' Bug in VB 7.1: cast should not be necessary.";
	_out << nl << "If Not CType(__vlhs(i), " << typeToString(p->valueType()) << "Helper).Equals(__vrhs(i))";
    }
    else
    {
	_out << nl << "If Not __vlhs(i).Equals(__vrhs(i))";
    }
    _out.inc();
    _out << nl << "Return False";
    _out.dec();
    _out << nl << "End If";
    _out.dec();
    _out << nl << "Next";
    _out << nl << "Return True";
    _out.dec();
    _out << nl << "End Function";

    _out.zeroIndent();
    _out << sp << nl << "#End Region"; // Object members

    _out << sp << nl << "#Region \"Shared Equals\"";
    _out.restoreIndent();

    _out << sp << nl << "Public Overloads Shared Function Equals(ByVal __lhs As " << name
         << ", ByVal __rhs As " << name << ") As Boolean";
    _out.inc();
    _out << nl << "If Object.ReferenceEquals(__rhs, Nothing) Then";
    _out.inc();
    _out << nl << "Return Object.ReferenceEquals(__rhs, Nothing)";
    _out.dec();
    _out << nl << "Else";
    _out.inc();
    _out << nl << "Return __lhs.Equals(__rhs)";
    _out.dec();
    _out << nl << "End If";
    _out.dec();
    _out << nl << "End Function";

    _out.zeroIndent();
    _out << sp << nl << "#End Region"; // Shared Equals
    _out.restoreIndent();

    _out.dec();
    _out << sp << nl << "End Class";
}

void
Slice::Gen::TypesVisitor::visitEnum(const EnumPtr& p)
{
    string name = fixId(p->name());
    string scoped = fixId(p->scoped());
    _out << sp << nl << "Public Enum " << name;
    _out.inc();
    EnumeratorList enumerators = p->getEnumerators();
    for(EnumeratorList::const_iterator en = enumerators.begin(); en != enumerators.end(); ++en)
    {
        _out << nl << fixId((*en)->name());
    }
    _out.dec();
    _out << nl << "End Enum";

    if(_stream)
    {
	_out << sp << nl << "Public NotInheritable Class " << name << "Helper";
	_out.inc();

	_out << sp << nl << "Public Shared Sub write(ByVal __outS As Ice.OutputStream, ByVal __v As " << scoped << ')';
	_out.inc();
	writeMarshalUnmarshalCode(_out, p, "__v", true, true, false);
	_out.dec();
	_out << nl << "End Sub";

	_out << sp << nl << "Public Shared Function read(ByVal __inS As Ice.InputStream) As " << scoped;
	_out.inc();
	_out << nl << "Dim __v As " << scoped;
	writeMarshalUnmarshalCode(_out, p, "__v", false, true, false);
	_out << nl << "Return __v";
	_out.dec();
	_out << nl << "End Function";

	_out.dec();
	_out << nl << "End Class";
    }
}

void
Slice::Gen::TypesVisitor::visitConst(const ConstPtr& p)
{
    string name = fixId(p->name());
    _out << sp << nl << "Public NotInheritable Class " << name;
    _out.inc();
    _out << nl << "Public Const value As " << typeToString(p->type()) << " = ";
    BuiltinPtr bp = BuiltinPtr::dynamicCast(p->type());
    if(bp && bp->kind() == Builtin::KindString)
    {
	static const string basicStringChars = "abcdefghijklmnopqrstuvwxyz"
					       "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
					       "0123456789"
					       "_{}[]#()<>%:;.?*+-/^&|~!=,\\\"' ";
    	static const set<char> charSet(basicStringChars.begin(), basicStringChars.end());

	enum Position { Beginning, InString, NotInString };
	Position pos = Beginning;

	const string val = p->value();
	for(string::const_iterator c = val.begin(); c != val.end(); ++c)
	{
	    if(charSet.find(*c) == charSet.end())
	    {
	        //
		// The character is something non-printable, such as a line feed,
		// so get it into the string literal using the VB Chr() function.
		//
		if(pos == InString)
		{
		    _out << '"'; // Close previous string literal.
		    pos = NotInString;
		}
		if(pos == NotInString)
		{
		    _out << " + ";
		}

		//
		// char may be signed, so make it positive.
		//
		unsigned char uc = *c;
		_out << " + Chr(" << static_cast<unsigned>(uc) << ')';

		pos = NotInString;
	    }
	    else
	    {
	        if(pos == NotInString)
		{
		    _out << " + ";
		    pos = Beginning;
		}
		if(pos == Beginning)
		{
		    _out << '"';
		}

		if(*c == '"')
		{
		    _out << "\"\"";
		}
		else
		{
		    _out << *c;
		}

		pos = InString;
	    }
	}
	if(pos == InString)
	{
	    _out << '"'; // Terminate final string literal.
	}
    }
    else if(bp && bp->kind() == Builtin::KindBool)
    {
	string val = p->value();
	val[0] = toupper(val[0]);
	_out << val;
    }
    else if(bp && bp->kind() == Builtin::KindShort)
    {
	_out << p->value() << 'S';
    }
    else if(bp && bp->kind() == Builtin::KindInt)
    {
	_out << p->value() << 'I';
    }
    else if(bp && bp->kind() == Builtin::KindLong)
    {
	_out << p->value() << 'L';
    }
    else if(bp && bp->kind() == Builtin::KindFloat)
    {
	_out << p->value() << 'F';
    }
    else if(bp && bp->kind() == Builtin::KindDouble)
    {
	_out << p->value() << 'R';
    }
    else
    {
	EnumPtr ep = EnumPtr::dynamicCast(p->type());
	if(ep)
	{
	    _out << fixId(typeToString(p->type())) << '.' << fixId(p->value());
	}
	else
	{
	    _out << p->value(); // byte
	}
    }
    _out.dec();
    _out << nl << "End Class";
}

void
Slice::Gen::TypesVisitor::visitDataMember(const DataMemberPtr& p)
{
    int baseTypes = 0;
    bool isClass = false;
    ContainedPtr cont = ContainedPtr::dynamicCast(p->container());
    assert(cont);
    if(StructPtr::dynamicCast(cont) && cont->hasMetaData("vb:class"))
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
    _out << sp << nl << "Public " << fixId(p->name(), baseTypes, isClass) << " As " << typeToString(p->type());
}

Slice::Gen::ProxyVisitor::ProxyVisitor(IceUtil::Output& out)
    : VbVisitor(out)
{
}

bool
Slice::Gen::ProxyVisitor::visitModuleStart(const ModulePtr& p)
{
    if(!p->hasNonLocalClassDecls())
    {
	return false;
    }

    _out << sp << nl << "Namespace " << fixId(p->name());
    _out.inc();
    return true;
}

void
Slice::Gen::ProxyVisitor::visitModuleEnd(const ModulePtr&)
{
    _out.dec();
    _out << sp << nl << "End Namespace";
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

    _out << sp << nl << "Public Interface " << name << "Prx";
    _out.inc();
    _out << nl << "Inherits ";
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

    return true;
}

void
Slice::Gen::ProxyVisitor::visitClassDefEnd(const ClassDefPtr&)
{
    _out.dec();
    _out << sp << nl << "End Interface";
}

void
Slice::Gen::ProxyVisitor::visitOperation(const OperationPtr& p)
{
    string name = fixId(p->name(), DotNet::ICloneable, true);
    vector<string> params = getParams(p);

    TypePtr ret = p->returnType();
    string retS = typeToString(ret);
    string vbOp = ret ? "Function" : "Sub";

    _out << sp << nl << vbOp << ' ' << name << spar << params << epar;
    if(ret)
    {
        _out << " As " << retS;
    }

    _out << nl << vbOp << ' ' << name << spar << params << "ByVal __context As Ice.Context" << epar; 
    if(ret)
    {
        _out << " As " << retS;
    }

    ClassDefPtr cl = ClassDefPtr::dynamicCast(p->container());
    if(cl->hasMetaData("ami") || p->hasMetaData("ami"))
    {
        vector<string> paramsAMI = getParamsAsync(p, false);

	//
	// Write two versions of the operation - with and without a
	// context parameter.
	//
	_out << sp;
	_out << nl << "Sub " << p->name() << "_async" << spar << paramsAMI << epar;
	_out << nl << "Sub " << p->name() << "_async" << spar << paramsAMI << "ByVal __ctx As Ice.Context" << epar;
    }
}

bool
Slice::Gen::OpsVisitor::visitModuleStart(const ModulePtr& p)
{
    if(!p->hasAbstractClassDefs())
    {
        return false;
    }

    _out << sp << nl << "Namespace " << fixId(p->name());
    _out.inc();
    return true;
}

void
Slice::Gen::OpsVisitor::visitModuleEnd(const ModulePtr&)
{
    _out.dec();
    _out << sp << nl << "End Namespace";
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

    _out << sp << nl << "Public Interface _" << name << opIntfName;
    _out.inc();
    if((bases.size() == 1 && bases.front()->isAbstract()) || bases.size() > 1)
    {
        _out << nl << "Inherits ";
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

	string vbOp = ret ? "Function" : "Sub";

	_out << sp << nl << vbOp << ' ' << name << spar << params;
	if(!noCurrent && !p->isLocal())
	{
	    _out << "ByVal __current As Ice.Current";
	}
	_out << epar;
	if(ret)
	{
	    _out << " As " << typeToString(ret);
	}
    }

    _out.dec();
    _out << sp << nl << "End Interface";
}

Slice::Gen::HelperVisitor::HelperVisitor(IceUtil::Output& out, bool stream)
    : VbVisitor(out),
      _stream(stream)
{
}

bool
Slice::Gen::HelperVisitor::visitModuleStart(const ModulePtr& p)
{
    if(!p->hasNonLocalClassDecls() && !p->hasNonLocalSequences() && !p->hasNonLocalDictionaries())
    {
        return false;
    }

    _out << sp << nl << "Namespace " << fixId(p->name());
    _out.inc();
    return true;
}

void
Slice::Gen::HelperVisitor::visitModuleEnd(const ModulePtr&)
{
    _out.dec();
    _out << sp << nl << "End Namespace";
}

bool
Slice::Gen::HelperVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    if(p->isLocal())
        return false;

    string name = p->name();
    ClassList bases = p->bases();

    _out << sp << nl << "Public NotInheritable Class " << name << "PrxHelper";
    _out.inc();
    _out << nl << "Inherits Ice.ObjectPrxHelperBase";
    _out << nl << "Implements " << name << "Prx";

    OperationList ops = p->allOperations();

    if(!ops.empty())
    {
	_out.zeroIndent();
	_out << sp << nl << "#Region \"Synchronous operations\"";
	_out.restoreIndent();
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

        string vbOp = ret ? "Function" : "Sub";
	_out << sp << nl << "Public " << vbOp << ' ' << opName << spar << params << epar;
	if(ret)
	{
	    _out << " As " << retS;
	}
	_out << " Implements " << name << "Prx." << opName;
	_out.inc();
	_out << nl;
	if(ret)
	{
	    _out << "Return ";
	}
	_out << opName << spar << args << "__defaultContext()" << epar;
	_out.dec();
	_out << nl << "End " << vbOp;

	_out << sp << nl << "Public " << vbOp << ' ' << opName << spar << params
	     << "ByVal __context As Ice.Context" << epar;
	if(ret)
	{
	    _out << " As " << retS;
	}
	_out << " Implements " << name << "Prx." << opName; // TODO: should be containing class?
	_out.inc();
	_out << nl << "Dim __cnt As Integer = 0";
	_out << nl << "While True";
	_out.inc();
	_out << nl << "Try";
	_out.inc();
	if(op->returnsData())
	{
	    _out << nl << "__checkTwowayOnly(\"" << op->name() << "\")";
	}
	_out << nl << "Dim __delBase As Ice._ObjectDel = __getDelegate()";
	_out << nl << "Dim __del As _" << name << "Del = CType(__delBase, _" << name << "Del)";
	_out << nl;
	if(ret)
	{
	    _out << "Return ";
	}
	_out << "__del." << opName << spar << args << "__context" << epar;
	if(!ret)
	{
	    _out << nl << "Return";
	}
	_out.dec();
	_out << nl << "Catch __ex As IceInternal.NonRepeatable";
	_out.inc();
	if(op->mode() == Operation::Idempotent || op->mode() == Operation::Nonmutating)
	{
	    _out << nl << "__cnt = __handleException(__ex.get(), __cnt)";
	}
	else
	{
	    _out << nl << "__rethrowException(__ex.get())";
	}
	_out.dec();
	_out << nl << "Catch __ex As Ice.LocalException";
	_out.inc();
	_out << nl << "__cnt = __handleException(__ex, __cnt)";
	_out.dec();
	_out << nl << "End Try";
	_out.dec();
	_out << nl << "End While";

	_out.dec();
	_out << nl << "End " << vbOp;
    }

    if(!ops.empty())
    {
	_out.zeroIndent();
	_out << sp << nl << "#End Region"; // Synchronous operations
	_out.restoreIndent();
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
		_out.zeroIndent();
		_out << sp << nl << "#Region \"Asynchronous operations\"";
		_out.restoreIndent();
		hasAsyncOps = true;
	    }
	    vector<string> paramsAMI = getParamsAsync(op, false);
	    vector<string> argsAMI = getArgsAsync(op);

	    string opName = op->name();

	    //
	    // Write two versions of the operation - with and without a
	    // context parameter
	    //
	    _out << sp << nl << "Public Sub " << opName << "_async" << spar << paramsAMI << epar
	         << " Implements " << name << "Prx." << opName << "_async"; // TODO: should be containing class?
	    _out.inc();
	    _out << nl << opName << "_async" << spar << argsAMI << "__defaultContext()" << epar;
	    _out.dec();
	    _out << nl << "End Sub";

	    _out << sp << nl << "Public Sub " << opName << "_async" << spar << paramsAMI
	         << "ByVal __ctx As Ice.Context" << epar
	         << " Implements " << name << "Prx." << opName << "_async"; // TODO: should be containing class?
	    _out.inc();
	    _out << nl << "__checkTwowayOnly(\"" << p->name() << "\")";
	    _out << nl << "__cb.__invoke" << spar << "Me" << argsAMI << "__ctx" << epar;
	    _out.dec();
	    _out << nl << "End Sub";
	}
    }

    if(hasAsyncOps)
    {
	_out.zeroIndent();
	_out << sp << nl << "#End Region"; // Asynchronous operations
	_out.restoreIndent();
    }

    _out.zeroIndent();
    _out << sp << nl << "#Region \"Checked and unchecked cast operations\"";
    _out.restoreIndent();

    _out << sp << nl << "Public Shared Function checkedCast(ByVal b As Ice.ObjectPrx) As " << name << "Prx";
    _out.inc();
    _out << nl << "If b Is Nothing Then";
    _out.inc();
    _out << nl << "Return Nothing";
    _out.dec();
    _out << nl << "End If";
    _out << nl << "If TypeOf b Is " << name << "Prx Then";
    _out.inc();
    _out << nl << "Return CType(b, " << name << "Prx)";
    _out.dec();
    _out << nl << "End If";
    _out << nl << "If b.ice_isA(\"" << p->scoped() << "\") Then";
    _out.inc();
    _out << nl << "Dim h As " << name << "PrxHelper = New " << name << "PrxHelper";
    _out << nl << "h.__copyFrom(b)";
    _out << nl << "Return h";
    _out.dec();
    _out << nl << "End If";
    _out << nl << "Return Nothing";
    _out.dec();
    _out << nl << "End Function";

    _out << sp << nl << "Public Shared Function checkedCast(ByVal b As Ice.ObjectPrx, ByVal ctx As Ice.Context) As "
         << name << "Prx";
    _out.inc();
    _out << nl << "If b Is Nothing Then";
    _out.inc();
    _out << nl << "Return Nothing";
    _out.dec();
    _out << nl << "End If";
    _out << nl << "If TypeOf b Is " << name << "Prx Then";
    _out.inc();
    _out << nl << "Return CType(b, " << name << "Prx)";
    _out.dec();
    _out << nl << "End If";
    _out << nl << "If b.ice_isA(\"" << p->scoped() << "\", ctx) Then";
    _out.inc();
    _out << nl << "Dim h As " << name << "PrxHelper = New " << name << "PrxHelper";
    _out << nl << "h.__copyFrom(b)";
    _out << nl << "Return h";
    _out.dec();
    _out << nl << "End If";
    _out << nl << "Return Nothing";
    _out.dec();
    _out << nl << "End Function";


    _out << sp << nl << "Public Shared Function checkedCast(ByVal b As Ice.ObjectPrx, ByVal f As String) As "
         << name << "Prx";
    _out.inc();
    _out << nl << "If b Is Nothing Then";
    _out.inc();
    _out << nl << "Return Nothing";
    _out.dec();
    _out << nl << "End If";
    _out << nl << "Dim bb As Ice.ObjectPrx = b.ice_newFacet(f)";
    _out << nl << "Try";
    _out.inc();
    _out << nl << "If bb.ice_isA(\"" << p->scoped() << "\") Then";
    _out.inc();
    _out << nl << "Dim h As " << name << "PrxHelper = new " << name << "PrxHelper()";
    _out << nl << "h.__copyFrom(bb)";
    _out << nl << "Return h";
    _out.dec();
    _out << nl << "End If";
    _out.dec();
    _out << nl << "Catch __ex As Ice.FacetNotExistException";
    _out << nl << "End Try";
    _out << nl << "Return Nothing";
    _out.dec();
    _out << nl << "End Function";

    _out << sp << nl << "Public Shared Function checkedCast(ByVal b As Ice.ObjectPrx, ByVal f As String, "
         << "ByVal ctx As Ice.Context) As " << name << "Prx";
    _out.inc();
    _out << nl << "If b Is Nothing Then";
    _out.inc();
    _out << nl << "Return Nothing";
    _out.dec();
    _out << nl << "End If";
    _out << nl << "Dim bb As Ice.ObjectPrx = b.ice_newFacet(f)";
    _out << nl << "Try";
    _out.inc();
    _out << nl << "If bb.ice_isA(\"" << p->scoped() << "\", ctx) Then";
    _out.inc();
    _out << nl << "Dim h As " << name << "PrxHelper = new " << name << "PrxHelper()";
    _out << nl << "h.__copyFrom(bb)";
    _out << nl << "Return h";
    _out.dec();
    _out << nl << "End If";
    _out.dec();
    _out << nl << "Catch __ex As Ice.FacetNotExistException";
    _out << nl << "End Try";
    _out << nl << "Return Nothing";
    _out.dec();
    _out << nl << "End Function";

    _out << sp << nl << "Public Shared Function uncheckedCast(ByVal b As Ice.ObjectPrx) As " << name << "Prx";
    _out.inc();
    _out << nl << "If b Is Nothing Then";
    _out.inc();
    _out << nl << "Return Nothing";
    _out.dec();
    _out << nl << "End If";
    _out << nl << "Dim h As " << name << "PrxHelper = new " << name << "PrxHelper()";
    _out << nl << "h.__copyFrom(b)";
    _out << nl << "Return h";
    _out.dec();
    _out << nl << "End Function";

    _out << sp << nl << "Public Shared Function uncheckedCast(Byval b As Ice.ObjectPrx, ByVal f As string) As "
         << name << "Prx";
    _out.inc();
    _out << nl << "If b Is Nothing Then";
    _out.inc();
    _out << nl << "Return Nothing";
    _out.dec();
    _out << nl << "End If";
    _out << nl << "Dim bb As Ice.ObjectPrx = b.ice_newFacet(f)";
    _out << nl << "Dim h As " << name << "PrxHelper = new " << name << "PrxHelper()";
    _out << nl << "h.__copyFrom(bb)";
    _out << nl << "Return h";
    _out.dec();
    _out << nl << "End Function";

    _out.zeroIndent();
    _out << sp << nl << "#End Region"; // Checked and unchecked cast operations

    _out << sp << nl << "#Region \"Marshaling support\"";
    _out.restoreIndent();

    _out << sp << nl << "Protected Overrides Function __createDelegateM() As Ice._ObjectDelM";
    _out.inc();
    _out << nl << "Return New _" << name << "DelM()";
    _out.dec();
    _out << nl << "End Function";

    _out << sp << nl << "Protected Overrides Function __createDelegateD() As Ice._ObjectDelD";
    _out.inc();
    _out << nl << "Return New _" << name << "DelD()";
    _out.dec();
    _out << nl << "End Function";

    _out << sp << nl << "Public Shared Sub __write(ByVal __os As IceInternal.BasicStream, ByVal __v As "
         << name << "Prx)";
    _out.inc();
    _out << nl << "__os.writeProxy(__v)";
    _out.dec();
    _out << nl << "End Sub";

    _out << sp << nl << "Public Shared Function __read(ByVal __is As IceInternal.BasicStream) As " << name << "Prx";
    _out.inc();
    _out << nl << "Dim proxy As Ice.ObjectPrx = __is.readProxy()";
    _out << nl << "If Not proxy Is Nothing Then";
    _out.inc();
    _out << nl << "Dim result As " << name << "PrxHelper = New " << name << "PrxHelper";
    _out << nl << "result.__copyFrom(proxy)";
    _out << nl << "Return result";
    _out.dec();
    _out << nl << "End If";
    _out << nl << "Return Nothing";
    _out.dec();
    _out << nl << "End Function";

    if(_stream)
    {
	_out << sp << nl << "Public Shared Sub write(ByVal __outS As Ice.OutputStream, ByVal __v As " << name << "Prx)";
	_out.inc();
	_out << nl << "__outS.writeProxy(__v)";
	_out.dec();
	_out << nl << "End Sub";

	_out << sp << nl << "Public Shared Function read(ByVal __inS As Ice.InputStream) As " << name << "Prx";
	_out.inc();
	_out << nl << "Dim proxy As Ice.ObjectPrx = __inS.readProxy()";
	_out << nl << "If Not proxy Is Nothing";
	_out.inc();
	_out << nl << "Dim result As " << name << "PrxHelper = New " << name << "PrxHelper";
	_out << nl << "result.__copyFrom(proxy)";
	_out << nl << "Return result";
	_out.dec();
	_out << nl << "End If";
	_out << nl << "Return Nothing";
	_out.dec();
	_out << nl << "End Function";
    }

    _out.zeroIndent();
    _out << sp << nl << "#End Region"; // Marshaling support
    _out.restoreIndent();

    return true;
}

void
Slice::Gen::HelperVisitor::visitClassDefEnd(const ClassDefPtr&)
{
    _out.dec();
    _out << sp << nl << "End Class";
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

    _out << sp << nl << "Public NotInheritable Class " << p->name() << "Helper";
    _out.inc();

    _out << sp << nl << "Public Shared Sub write(ByVal __os As IceInternal.BasicStream, ByVal __v As " << typeS << ')';
    _out.inc();
    writeSequenceMarshalUnmarshalCode(_out, p, "__v", true, false);
    _out.dec();
    _out << nl << "End Sub";

    _out << sp << nl << "Public Shared Function read(ByVal __is As IceInternal.BasicStream) As " << typeS;
    _out.inc();
    _out << nl << "Dim __v As " << typeS;
    writeSequenceMarshalUnmarshalCode(_out, p, "__v", false, false);
    _out << nl << "Return __v";
    _out.dec();
    _out << nl << "End Function";

    if(_stream)
    {
	_out << sp << nl << "Public Shared Sub write(ByVal __outS As Ice.OutputStream, ByVal __v As " << typeS << ')';
	_out.inc();
	writeSequenceMarshalUnmarshalCode(_out, p, "__v", true, true);
	_out.dec();
	_out << nl << "End Sub";

	_out << sp << nl << "Public Shared Function read(ByVal __inS As Ice.InputStream) As " << typeS;
	_out.inc();
	_out << nl << "Dim __v As " << typeS;
	writeSequenceMarshalUnmarshalCode(_out, p, "__v", false, true);
	_out << nl << "Return __v";
	_out.dec();
	_out << nl << "End Function";
    }

    _out.dec();
    _out << sp << nl << "End Class";
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

    _out << sp << nl << "Public NotInheritable Class " << p->name() << "Helper";
    _out.inc();

    _out << sp << nl << "Public Shared Sub write(ByVal __os As IceInternal.BasicStream, ByVal __v As "
         << name << ')';
    _out.inc();
    _out << nl << "If __v Is Nothing Then";
    _out.inc();
    _out << nl << "__os.writeSize(0)";
    _out.dec();
    _out << nl << "Else";
    _out.inc();
    _out << nl << "__os.writeSize(__v.Count)";
    _out << nl << "For Each __e As _System.Collections.DictionaryEntry In __v";
    _out.inc();
    string keyArg = "CType(__e.Key, " + keyS + ")";
    writeMarshalUnmarshalCode(_out, key, keyArg, true, false, false);
    string valueArg = "__e.Value";
    writeMarshalUnmarshalCode(_out, value, valueArg, true, false, false);
    _out.dec();
    _out << nl << "Next";
    _out.dec();
    _out << nl << "End If";
    _out.dec();
    _out << nl << "End Sub";

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(value);
    bool hasClassValue = (builtin && builtin->kind() == Builtin::KindObject) || ClassDeclPtr::dynamicCast(value);
    if(hasClassValue)
    {
	_out << sp << nl << "Public NotInheritable Class __Patcher";
	_out.inc();
	_out << nl << "Inherits IceInternal.Patcher";
	_out << sp << nl << "Friend Sub New(ByVal m As " << name << ", ByVal key As " << keyS << ')';
	_out.inc();
	_out << nl << "_m = m";
	_out << nl << "_key = key";
	_out.dec();
	_out << nl << "End Sub";

	_out << sp << nl << "Public Overrides Sub patch(ByVal v As Ice.Object)";
	_out.inc();
	_out << nl << "_type = GetType(" << typeToString(p->valueType()) << ')';
	_out << nl << "_m(_key) = CType(v, " << valueS << ')';
	_out.dec();
	_out << nl << "End Sub";

	_out << sp << nl << "Private _m As " << name;
	_out << nl << "Private _key As " << keyS;
	_out.dec();
	_out << sp << nl << "End Class";
    }

    _out << sp << nl << "Public Shared Function " << " read(ByVal __is As IceInternal.BasicStream) As " << name;
    _out.inc();

    _out << nl << "Dim __sz As Integer = __is.readSize()";
    _out << nl << "Dim __r As " << name << " = New " << name;
    _out << nl << "For __i As Integer = 0 To __sz - 1";
    _out.inc();
    _out << nl << "Dim __k As " << keyS;
    writeMarshalUnmarshalCode(_out, key, "__k", false, false, false);

    if(!hasClassValue)
    {
	_out << nl << "Dim __v As " << valueS;
    }
    writeMarshalUnmarshalCode(_out, value, "__v", false, false, false, "__r, __k");
    if(!hasClassValue)
    {
	_out << nl << "__r(__k) = __v";
    }
    _out.dec();
    _out << nl << "Next";
    _out << nl << "Return __r";

    _out.dec();
    _out << nl << "End Function";

    if(_stream)
    {
	_out << nl << "Public Shared Sub write(ByVal __outS As Ice.OutputStream, ByVal __v As " << name << ')';
	_out.inc();
	_out << nl << "If __v Is Nothing Then";
	_out.inc();
	_out << nl << "__outS.writeSize(0)";
	_out.dec();
	_out << nl << "Else";
	_out.inc();
	_out << nl << "__outS.writeSize(__v.Count)";
	_out << nl << "For Each __e As _System.Collections.DictionaryEntry In __v";
	_out.inc();
	writeMarshalUnmarshalCode(_out, key, keyArg, true, true, false);
	writeMarshalUnmarshalCode(_out, value, valueArg, true, true, false);
	_out.dec();
	_out << nl << "Next";
	_out.dec();
	_out << nl << "End If";
	_out.dec();
	_out << nl << "End Sub";

	_out << sp << nl << "Public Shared Function read(ByVal __inS As Ice.InputStream) As " << name;
	_out.inc();
	_out << nl << "Dim __sz As Integer = __inS.readSize()";
	_out << nl << "Dim __r As " << name << " = New " << name;
	_out << nl << "For __i As Integer = 0 To __sz - 1";
	_out.inc();
	_out << nl << "Dim __k As " << keyS;
	writeMarshalUnmarshalCode(_out, key, "__k", false, true, false);
	if(!hasClassValue)
	{
	    _out << nl << "Dim __v As " << valueS;
	}
	writeMarshalUnmarshalCode(_out, value, "__v", false, true, false, "__r, __k");
	if(!hasClassValue)
	{
	    _out << nl << "__r(__k) = __v";
	}
	_out.dec();
	_out << nl << "Next";
	_out << nl << "Return __r";
	_out.dec();
	_out << nl << "End Function";
    }

    _out.dec();
    _out << sp << nl << "End Class";
}

Slice::Gen::DelegateVisitor::DelegateVisitor(IceUtil::Output& out)
    : VbVisitor(out)
{
}

bool
Slice::Gen::DelegateVisitor::visitModuleStart(const ModulePtr& p)
{
    if(!p->hasNonLocalClassDecls())
    {
        return false;
    }

    _out << sp << nl << "Namespace " << fixId(p->name());
    _out.inc();
    return true;
}

void
Slice::Gen::DelegateVisitor::visitModuleEnd(const ModulePtr&)
{
    _out.dec();
    _out << sp << nl << "End Namespace";
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

    _out << sp << nl << "Public Interface _" << name << "Del";
    _out.inc();
    _out << nl << "Inherits ";
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

    OperationList ops = p->operations();

    OperationList::const_iterator r;
    for(r = ops.begin(); r != ops.end(); ++r)
    {
	OperationPtr op = *r;
	string opName = fixId(op->name(), DotNet::ICloneable, true);
	TypePtr ret = op->returnType();
	string retS = typeToString(ret);
	vector<string> params = getParams(op);
	string vbOp = ret ? "Function" : "Sub";

	_out << sp << nl << vbOp << ' ' << opName << spar << params << "ByVal __context As Ice.Context" << epar;
	if(ret)
	{
	    _out << " As " << retS;
	}
    }

    return true;
}

void
Slice::Gen::DelegateVisitor::visitClassDefEnd(const ClassDefPtr&)
{
    _out.dec();
    _out << sp << nl << "End Interface";
}

Slice::Gen::DelegateMVisitor::DelegateMVisitor(IceUtil::Output& out)
    : VbVisitor(out)
{
}

bool
Slice::Gen::DelegateMVisitor::visitModuleStart(const ModulePtr& p)
{
    if(!p->hasNonLocalClassDecls())
    {
        return false;
    }

    _out << sp << nl << "Namespace " << fixId(p->name());
    _out.inc();
    return true;
}

void
Slice::Gen::DelegateMVisitor::visitModuleEnd(const ModulePtr&)
{
    _out.dec();
    _out << sp << nl << "End Namespace";
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

    _out << sp << nl << "Public NotInheritable class _" << name << "DelM";
    _out.inc();
    _out << nl << "Inherits Ice._ObjectDelM";
    _out << nl << "Implements _" << name << "Del";

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

	string vbOp = ret ? "Function" : "Sub";

	_out << sp << nl << "Public " << vbOp << ' ' << opName << spar << params
	     << "ByVal __context As Ice.Context" << epar;
	if(ret)
	{
	    _out << " As " << retS;
	}
	_out << " Implements _" << name << "Del." << opName; // TODO: should be containing class?
	_out.inc();

	_out << nl << "Dim __og As IceInternal.Outgoing = getOutgoing(\""
	     << op->name() << "\", " << sliceModeToIceMode(op) << ", __context)";
	_out << nl << "Try";
	_out.inc();
	if(!inParams.empty())
	{
	    _out << nl << "Try";
	    _out.inc();
	    _out << nl << "Dim __os As IceInternal.BasicStream = __og.ostr()";
	    for(q = inParams.begin(); q != inParams.end(); ++q)
	    {
		writeMarshalUnmarshalCode(_out, q->first, fixId(q->second), true, false, false);
	    }
	    if(op->sendsClasses())
	    {
		_out << nl << "__os.writePendingObjects()";
	    }
	    _out.dec();
	    _out << nl << "Catch __ex As Ice.LocalException";
	    _out.inc();
	    _out << nl << "__og.abort(__ex)";
	    _out.dec();
	    _out << nl << "End Try";
	}
	_out << nl << "Dim __ok As Boolean = __og.invoke()";
	_out << nl << "Try";
	_out.inc();
	_out << nl << "Dim __is As IceInternal.BasicStream = __og.istr()";
	_out << nl << "If Not __ok Then";
	_out.inc();
	//
	// The try/catch block is necessary because throwException()
	// can raise UserException.
	//
	_out << nl << "Try";
	_out.inc();
	_out << nl << "__is.throwException()";
	_out.dec();
	for(ExceptionList::const_iterator t = throws.begin(); t != throws.end(); ++t)
	{
	    _out << nl << "Catch __ex As " << fixId((*t)->scoped());
	    _out.inc();
	    _out << nl << "Throw";
	    _out.dec();
	}
	_out << nl << "Catch __ex As Ice.UserException";
	_out.inc();
	_out << nl << "Throw New Ice.UnknownUserException()";
	_out.dec();
	_out << nl << "End Try";
	_out.dec();
	_out << nl << "End If";
	for(q = outParams.begin(); q != outParams.end(); ++q)
	{
	    writeMarshalUnmarshalCode(_out, q->first, fixId(q->second), false, false, true, "");
	}
	if(ret)
	{
	    BuiltinPtr builtin = BuiltinPtr::dynamicCast(ret);
	    if((builtin && builtin->kind() == Builtin::KindObject) || ClassDeclPtr::dynamicCast(ret))
	    {
		_out << nl << "Dim __ret As " << retS;
		_out << nl << "Dim __ret_PP As IceInternal.ParamPatcher = New IceInternal.ParamPatcher(GetType("
		     << retS << "))";
		_out << nl << "__is.readObject(__ret_PP)";
	    }
	    else
	    {
		_out << nl << "Dim __ret As " << retS;
		writeMarshalUnmarshalCode(_out, ret, "__ret", false, false, true, "");
	    }
	}
	if(op->returnsClasses())
	{
	    _out << nl << "__is.readPendingObjects()";
	    for(q = outParams.begin(); q != outParams.end(); ++q)
	    {
		string param = fixId(q->second);
	        BuiltinPtr builtin = BuiltinPtr::dynamicCast(q->first);
		if((builtin && builtin->kind() == Builtin::KindObject) || ClassDeclPtr::dynamicCast(q->first))
		{	    
		    string type = typeToString(q->first);
		    _out << nl << param << " = CType(" << param << "_PP.value, " << type << ')';
		}
		else
		{
		    StructPtr st = StructPtr::dynamicCast(q->first);
		    bool patchStruct = st && !st->hasMetaData("vb:class") && st->classDataMembers().size() != 0;
		    if(patchStruct)
		    {
			_out << nl << param << ".__patch()";
		    }
		}
	    }
	}
	if(ret)
	{
	    BuiltinPtr builtin = BuiltinPtr::dynamicCast(ret);
	    if((builtin && builtin->kind() == Builtin::KindObject) || ClassDeclPtr::dynamicCast(ret))
	    {
		_out << nl << "__ret = CType(__ret_PP.value, " << retS << ')';
	    }
	    else
	    {
		StructPtr st = StructPtr::dynamicCast(ret);
		bool patchStruct = st && !st->hasMetaData("vb:class") && st->classDataMembers().size() != 0;
		if(patchStruct)
		{
		    _out << nl << "__ret.__patch()";
		}
	    }
	    _out << nl << "Return __ret";
	}
	_out.dec();
	_out << nl << "Catch __ex As Ice.LocalException";
	_out.inc();
	_out << nl << "throw New IceInternal.NonRepeatable(__ex)";
	_out.dec();
	_out << nl << "End Try";
        _out.dec();
        _out << nl << "Finally";
        _out.inc();
        _out << nl << "reclaimOutgoing(__og)";
        _out.dec();
	_out << nl << "End Try";

        _out.dec();
	_out << nl << "End " << vbOp;
    }

    return true;
}

void
Slice::Gen::DelegateMVisitor::visitClassDefEnd(const ClassDefPtr&)
{
    _out.dec();
    _out << sp << nl << "End Class";
}

Slice::Gen::DelegateDVisitor::DelegateDVisitor(IceUtil::Output& out)
    : VbVisitor(out)
{
}

bool
Slice::Gen::DelegateDVisitor::visitModuleStart(const ModulePtr& p)
{
    if(!p->hasNonLocalClassDecls())
    {
        return false;
    }

    _out << sp << nl << "Namespace " << fixId(p->name());
    _out.inc();
    return true;
}

void
Slice::Gen::DelegateDVisitor::visitModuleEnd(const ModulePtr&)
{
    _out.dec();
    _out << sp << nl << "End Namespace";
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

    _out << sp << nl << "Public NotInheritable class _" << name << "DelD";
    _out.inc();
    _out << nl << "Inherits Ice._ObjectDelD";
    _out << nl << "Implements _" << name << "Del";

    OperationList ops = p->allOperations();

    OperationList::const_iterator r;
    for(r = ops.begin(); r != ops.end(); ++r)
    {
        OperationPtr op = *r;
        string opName = fixId(op->name(), DotNet::ICloneable, true);
        TypePtr ret = op->returnType();
        string retS = typeToString(ret);
	string vbOp = ret ? "Function" : "Sub";
	ClassDefPtr containingClass = ClassDefPtr::dynamicCast(op->container());

        vector<string> params = getParams(op);
        vector<string> args = getArgs(op);

	_out << sp;
        _out << nl << "Public " << vbOp << ' ' << opName << spar << params << "ByVal __context As Ice.Context" << epar;
	if(ret)
	{
	    _out << " As " << retS;
	}
	_out << " Implements _" << name << "Del." << opName; // TODO: should be containing class?
        _out.inc();
	if(containingClass->hasMetaData("amd") || op->hasMetaData("amd"))
	{
	    _out << nl << "Throw New Ice.CollocationOptimizationException()";
	}
	else
	{
	    _out << nl << "Dim __current As Ice.Current = New Ice.Current";
	    _out << nl << "__initCurrent(__current, \"" << op->name() << "\", " << sliceModeToIceMode(op)
		 << ", __context)";
	    _out << nl << "While True";
	    _out.inc();
	    _out << nl << "Dim __direct As IceInternal.Direct = New IceInternal.Direct(__current)";
	    _out << nl << "Dim __servant As Object = __direct.servant()";
	    _out << nl << "If TypeOf __servant Is " << fixId(name) << " Then";
	    _out.inc();
	    _out << nl << "Try";
	    _out.inc();
	    _out << nl;
	    if(ret)
	    {
		_out << "Return ";
	    }
	    _out << "CType(__servant, " << fixId(containingClass->scope())
	         << '_' << containingClass->name() << "Operations" << ")."
		 << opName << spar << args << "__current" << epar;
	    if(!ret)
	    {
	        _out << nl << "Return";
	    }
	    _out.dec();
	    _out << nl << "Catch __ex As Ice.LocalException";
	    _out.inc();
	    _out << nl << "Throw New IceInternal.NonRepeatable(__ex)";
	    _out.dec();
	    _out << nl << "Finally";
	    _out.inc();
	    _out << nl << "__direct.destroy()";
	    _out.dec();
	    _out << nl << "End Try";
	    _out.dec();
	    _out << nl << "Else";
	    _out.inc();
	    _out << nl << "__direct.destroy()";
	    _out << nl << "Dim __opEx As Ice.OperationNotExistException = new Ice.OperationNotExistException()";
	    _out << nl << "__opEx.id = __current.id";
	    _out << nl << "__opEx.facet = __current.facet";
	    _out << nl << "__opEx.operation = __current.operation";
	    _out << nl << "Throw __opEx";
	    _out.dec();
	    _out << nl << "End If";
	    _out.dec();
	    _out << nl << "End While";
	}
	_out.dec();
        _out << nl << "End " << vbOp;
    }

    return true;
}

void
Slice::Gen::DelegateDVisitor::visitClassDefEnd(const ClassDefPtr&)
{
    _out.dec();
    _out << sp << nl << "End Class";
}

Slice::Gen::DispatcherVisitor::DispatcherVisitor(::IceUtil::Output &out)
    : VbVisitor(out)
{
}

bool
Slice::Gen::DispatcherVisitor::visitModuleStart(const ModulePtr& p)
{
    if(!p->hasNonLocalClassDecls())
    {
        return false;
    }

    _out << sp << nl << "Namespace " << fixId(p->name());
    _out.inc();
    return true;
}

void
Slice::Gen::DispatcherVisitor::visitModuleEnd(const ModulePtr&)
{
    _out.dec();
    _out << sp << nl << "End Namespace";
}

bool
Slice::Gen::DispatcherVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    if(p->isLocal() || !p->isInterface())
    {
        return false;
    }

    _out << sp << nl << "Public MustInherit class _" << p->name() << "Disp";
    _out.inc();
    _out << nl << "Inherits Ice.ObjectImpl";
    _out << nl << "Implements " << fixId(p->name());

    OperationList ops = p->operations();
    if(!ops.empty())
    {
	_out.zeroIndent();
        _out << sp << nl << "#Region \"Slice operations\"";
	_out.restoreIndent();
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

	string vbOp = ret ? "Function" : "Sub";

	_out << sp << nl << "Public " << vbOp << ' ' << name << spar << params << epar;
	if(ret)
	{
	    _out << " As " << typeToString(ret);
	}
	_out << " Implements _" << p->name() << "OperationsNC" << '.' << name; // TODO: should be containing class?
	_out.inc();
	_out << nl;
	if(ret)
	{
	    _out << "Return ";
	}
	_out << name << spar << args << "Ice.ObjectImpl.defaultCurrent" << epar;
	_out.dec();
	_out << nl << "End " << vbOp;

	_out << sp << nl << "Public MustOverride " << vbOp << ' ' << name << spar << params;
	if(!p->isLocal())
	{
	    _out << "ByVal __current As Ice.Current";
	}
	_out << epar;
	if(ret)
	{
	    _out << " As " << typeToString(ret);
	}
	_out << " Implements _" << p->name() << "Operations" << '.' << name; // TODO: should be containing class?
    }

    if(!ops.empty())
    {
	_out.zeroIndent();
        _out << sp << nl << "#End Region"; // Slice operations
	_out.restoreIndent();
    }

    writeInheritedOperations(p);

    writeDispatch(p);

    _out.dec();
    _out << sp << nl << "End Class";

    return true;
}

Slice::Gen::AsyncVisitor::AsyncVisitor(::IceUtil::Output &out)
    : VbVisitor(out)
{
}

bool
Slice::Gen::AsyncVisitor::visitModuleStart(const ModulePtr& p)
{
    if(!p->hasAsyncOps())
    {
        return false;
    }

    _out << sp << nl << "Namespace " << fixId(p->name());
    _out.inc();
    return true;
}

void
Slice::Gen::AsyncVisitor::visitModuleEnd(const ModulePtr&)
{
    _out.dec();
    _out << sp << nl << "End Namespace";
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

	_out << sp << nl << "Public MustInherit Class AMI_" << cl->name() << '_' << name;
	_out.inc();
	_out << nl << "Inherits IceInternal.OutgoingAsync";
	_out << sp;
	_out << nl << "Public MustOverride Sub ice_response" << spar << params << epar;
	
	_out << sp << nl << "Public Sub __invoke" << spar << "ByVal __prx As Ice.ObjectPrx"
	    << paramsInvoke << "ByVal __ctx As Ice.Context" << epar;
	_out.inc();
	_out << nl << "Try";
	_out.inc();
	_out << nl << "__prepare(__prx, \"" << p->name() << "\", " << sliceModeToIceMode(p) << ", __ctx)";
	for(q = inParams.begin(); q != inParams.end(); ++q)
	{
	    string typeS = typeToString(q->first);
	    writeMarshalUnmarshalCode(_out, q->first, fixId(q->second), true, false, false);
	}
	if(p->sendsClasses())
	{
	    _out << nl << "__os.writePendingObjects()";
	}
	_out << nl << "__os.endWriteEncaps()";
	_out.dec();
	_out << nl << "Catch __ex As Ice.LocalException";
	_out.inc();
	_out << nl << "__finished(__ex)";
	_out << nl << "Return";
	_out.dec();
	_out << nl << "End Try";
	_out << nl << "__send()";
	_out.dec();
	_out << nl << "End Sub";

	_out << sp << nl << "Protected Overrides Sub __response(__ok As Boolean)";
	_out.inc();
        for(q = outParams.begin(); q != outParams.end(); ++q)
        {
	    _out << nl << "Dim " << fixId(q->second) << " As " << typeToString(q->first);
        }
        if(ret)
        {
	    _out << nl << "Dim __ret As " << retS;
        }
	_out << nl << "Try";
	_out.inc();
	_out << nl << "If Not __ok Then";
        _out.inc();
	_out << nl << "Try";
	_out.inc();
	_out << nl << "__is.throwException()";
	_out.dec();
	for(ExceptionList::const_iterator r = throws.begin(); r != throws.end(); ++r)
	{
	    _out << nl << "Catch __ex As " << fixId((*r)->scoped());
	    _out.inc();
	    _out << nl << "Throw __ex";
	    _out.dec();
	}
	_out << nl << "Catch __ex As Ice.UserException";
	_out.inc();
        _out << nl << "Throw New Ice.UnknownUserException()";
	_out.dec();
	_out << nl << "End Try";
	_out.dec();
	_out << nl << "End If";
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
	    _out << nl << "__is.readPendingObjects()";
	}
	for(q = outParams.begin(); q != outParams.end(); ++q)
	{
	    string param = fixId(q->second);
	    BuiltinPtr builtin = BuiltinPtr::dynamicCast(q->first);
	    if((builtin && builtin->kind() == Builtin::KindObject) || ClassDeclPtr::dynamicCast(q->first))
	    {
		string type = typeToString(q->first);
		_out << nl << param << " = CType(" << param << "_PP.value, " << type << ')';
	    }
	    else
	    {
		StructPtr st = StructPtr::dynamicCast(q->first);
		bool patchStruct = st && !st->hasMetaData("vb:class") && st->classDataMembers().size() != 0;
		if(patchStruct)
		{
		    _out << nl << param << ".__patch()";
		}
	    }
	}
	if(ret)
	{
	    BuiltinPtr builtin = BuiltinPtr::dynamicCast(ret);
	    if((builtin && builtin->kind() == Builtin::KindObject) || ClassDeclPtr::dynamicCast(ret))
	    {
		string type = typeToString(ret);
		_out << nl << "__ret = CType(__ret_PP.value, " << retS << ')';
	    }
	    else
	    {
		StructPtr st = StructPtr::dynamicCast(ret);
		bool patchStruct = st && !st->hasMetaData("vb:class") && st->classDataMembers().size() != 0;
		if(patchStruct)
		{
		    _out << nl << "__ret.__patch()";
		}
	    }
	}
   	_out.dec();
	_out << nl << "Catch __ex As Ice.LocalException";
	_out.inc();
	_out << nl << "__finished(__ex)";
	_out << nl << "Return";
	_out.dec();
	if(!throws.empty())
	{
	    _out << nl << "Catch __ex As Ice.UserException";
	    _out.inc();
	    _out << nl << "ice_exception(__ex)";
	    _out << nl << "Return";
	    _out.dec();
	}
	_out << nl << "End Try";
	_out << nl << "ice_response" << spar << args << epar;
	_out.dec();
	_out << nl << "End Sub";
	_out.dec();
	_out << nl << "End Class";
    }

    if(cl->hasMetaData("amd") || p->hasMetaData("amd"))
    {
	string classNameAMD = "AMD_" + cl->name();
	string classNameAMDI = "_AMD_" + cl->name();

	vector<string> paramsAMD = getParamsAsyncCB(p);

	_out << sp << nl << "Public Interface " << classNameAMD << '_' << name;
	_out.inc();
	_out << sp << nl << "Sub ice_response" << spar << paramsAMD << epar;
	_out << sp << nl << "Sub ice_exception(ex As _System.Exception)";
	_out.dec();
	_out << nl << "End Interface";
    
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
	_out << sp << nl << "Class " << classNameAMDI << '_' << name;
	_out.inc();
	_out << nl << "Inherits IceInternal.IncomingAsync";
	_out << nl << "Implements " << classNameAMD << '_' << name;

	_out << sp << nl << "Public Sub New(ByVal inc As IceInternal.Incoming)";
	_out.inc();
	_out << nl << "MyBase.New(inc)";
	_out.dec();
	_out << nl << "End Sub";

	_out << sp << nl << "Public Sub ice_response" << spar << paramsAMD << epar
	     << " Implements " << classNameAMD << '_' << name << ".ice_response"; // TODO: should be containing class?
	_out.inc();
	if(ret || !outParams.empty())
	{
	    _out << nl << "Try";
	    _out.inc();
	    _out << nl << "Dim __os As IceInternal.BasicStream = Me.__os()";
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
		_out << nl << "__os.writePendingObjects()";
	    }
	    _out.dec();
	    _out << nl << "Catch __ex As Ice.LocalException";
	    _out.inc();
	    _out << nl << "ice_exception(__ex)";
	    _out.dec();
	    _out << nl << "End Try";
	}
	_out << nl << "__response(true)";
	_out.dec();
	_out << nl << "End Sub";

	_out << sp << nl << "Public Sub ice_exception(ByVal ex As _System.Exception)"
	     << " Implements " << classNameAMD << '_' << name << ".ice_exception"; // TODO: should be containing class?
	_out.inc();
	if(throws.empty())
	{
	    _out << nl << "__exception(ex)";
	}
	else
	{
	    _out << nl << "Try";
	    _out.inc();
	    _out << nl << "throw ex";
	    _out.dec();
	    ExceptionList::const_iterator r;
	    for(r = throws.begin(); r != throws.end(); ++r)
	    {
		string exS = fixId((*r)->scoped());
		_out << nl << "Catch __ex As " << exS;
		_out.inc();
		_out << nl << "__os().writeUserException(__ex)";
		_out << nl << "__response(false)";
		_out.dec();
	    }
	    _out << nl << "Catch __ex As _System.Exception";
	    _out.inc();
	    _out << nl << "__exception(__ex)";
	    _out.dec();
	    _out << nl << "End Try";
	}
	_out.dec();
	_out << nl << "End Sub";

	_out.dec();
	_out << nl << "End Class";
    }
}

Slice::Gen::TieVisitor::TieVisitor(IceUtil::Output& out)
    : VbVisitor(out)
{
}

bool
Slice::Gen::TieVisitor::visitModuleStart(const ModulePtr& p)
{
    if(!p->hasClassDefs())
    {
        return false;
    }

    _out << sp << nl << "Namespace " << fixId(p->name());
    _out.inc();

    return true;
}

void
Slice::Gen::TieVisitor::visitModuleEnd(const ModulePtr&)
{
    _out.dec();
    _out << sp << nl << "End Namespace";
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

    _out << sp << nl << "Public Class _" << name << "Tie";
    _out.inc();
    _out << nl << "Inherits ";
    if(p->isInterface())
    {
	if(p->isLocal())
	{
	    _out << fixId(name);
	}
	else
	{
	    _out << '_' << name << "Disp";
	}
    }
    else
    {
        _out << fixId(name);
    }
    _out << nl << "Implements Ice.TieBase";

    _out << sp << nl << "Public Sub New()";
    _out << nl << "End Sub";

    _out << sp << nl << "Public Sub New(ByVal del As _" << name << opIntfName << ")";
    _out.inc();
    _out << nl << "_ice_delegate = del";
    _out.dec();
    _out << nl << "End Sub";

    _out << sp << nl << "Public Function ice_delegate() As Object Implements Ice.TieBase.ice_delegate";
    _out.inc();
    _out << nl << "Return _ice_delegate";
    _out.dec();
    _out << nl << "End Function";

    _out << sp << nl << "Public Sub ice_delegate(ByVal del As Object) Implements Ice.TieBase.ice_delegate";
    _out.inc();
    _out << nl << "_ice_delegate = CType(del, _" << name << opIntfName << ")";
    _out.dec();
    _out << nl << "End Sub";

    _out << sp << nl << "Public ";
    if(!p->isInterface() || !p->isLocal())
    {
        _out << "Overrides ";
    }
    _out << "Function ice_hash() As Integer";

    _out.inc();
    _out << nl << "Return GetHashCode()";
    _out.dec();
    _out << nl << "End Function";

    _out << sp << nl << "Public Overrides Function GetHashCode() As Integer";
    _out.inc();
    _out << nl << "If _ice_delegate Is Nothing Then";
    _out.inc();
    _out << nl << "Return 0";
    _out.dec();
    _out << nl << "Else";
    _out.inc();
    _out << nl << "' Bug in VB 7.1: cast to Object should not be necessary.";
    _out << nl << "Return CType(_ice_delegate, Object).GetHashCode()";
    _out.dec();
    _out << nl << "End If";
    _out.dec();
    _out << nl << "End Function";

    _out << sp << nl << "Public Overloads Overrides Function Equals(ByVal rhs As Object) As Boolean";
    _out.inc();
    _out << nl << "If Object.ReferenceEquals(Me, rhs) Then";
    _out.inc();
    _out << nl << "Return true";
    _out.dec();
    _out << nl << "End If";
    _out << nl << "If Not TypeOf rhs Is _" << name << "Tie Then";
    _out.inc();
    _out << nl << "Return False";
    _out.dec();
    _out << nl << "End If";
    _out << nl << "If _ice_delegate Is Nothing Then";
    _out.inc();
    _out << nl << "Return CType(rhs, _" << name << "Tie)._ice_delegate Is Nothing";
    _out.dec();
    _out << nl << "End If";
    _out << nl << "' Bug in VB 7.1: cast to Object should not be necessary.";
    _out << nl << "Return CType(_ice_delegate, Object).Equals(CType(rhs, _" << name << "Tie)._ice_delegate)";
    _out.dec();
    _out << nl << "End Function";

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

        _out << sp << nl << "Public Overloads ";
	if(!p->isInterface() || !p->isLocal())
	{
	    _out << "Overrides ";
	}
	string vbOp = (!ret || hasAMD) ? "Sub" : "Function";
	_out << vbOp << ' ' << opName << spar << params;
        if(!p->isLocal())
        {
            _out << "ByVal __current As Ice.Current";
        }
        _out << epar;
	if(ret && !hasAMD)
	{
	    _out << " As " << retS;
	}
        _out.inc();
        _out << nl;
        if(ret && !hasAMD)
        {
            _out << "Return ";
        }
        _out << "_ice_delegate." << opName << spar << args;
        if(!p->isLocal())
        {
            _out << "__current";
        }
        _out << epar;
        _out.dec();
	_out << nl << "End " << vbOp;
    }

    NameSet opNames;
    ClassList bases = p->bases();
    for(ClassList::const_iterator i = bases.begin(); i != bases.end(); ++i)
    {
        writeInheritedOperationsWithOpNames(*i, opNames);
    }

    _out << sp << nl << "Private _ice_delegate As _" << name << opIntfName;

    return true;
}

void
Slice::Gen::TieVisitor::visitClassDefEnd(const ClassDefPtr&)
{
    _out.dec();
    _out << nl << "End Class";
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

        _out << sp << nl << "Public Overloads ";
	if(!p->isInterface() || !p->isLocal())
	{
	    _out << "Overrides ";
	}
	string vbOp = (ret && !hasAMD) ? "Function" : "Sub";
	_out << vbOp << ' ' << opName << spar << params;
        if(!p->isLocal())
        {
            _out << "ByVal __current As Ice.Current";
        }
        _out << epar;
	if(ret && !hasAMD)
	{
	    _out << " As " << retS;
	}
        _out.inc();
        _out << nl;
        if(ret && !hasAMD)
        {
            _out << "Return ";
        }
        _out << "_ice_delegate." << opName << spar << args;
        if(!p->isLocal())
        {
            _out << "__current";
        }
        _out << epar;
	_out.dec();
        _out << nl << "End " << vbOp;
    }

    ClassList bases = p->bases();
    for(ClassList::const_iterator i = bases.begin(); i != bases.end(); ++i)
    {
        writeInheritedOperationsWithOpNames(*i, opNames);
    }
}

Slice::Gen::BaseImplVisitor::BaseImplVisitor(IceUtil::Output& out)
    : VbVisitor(out)
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
	_out << nl << "' ";
    }
    else
    {
	_out << sp << nl;
    }

    if(!cl->isLocal() && (cl->hasMetaData("amd") || op->hasMetaData("amd")))
    {
        vector<string> pDecl = getParamsAsync(op, true);

	_out << "Public Overloads ";
	if(!forTie)
	{
	    _out << "Overrides ";
	}
	_out << "Sub " << opName << "_async" << spar << pDecl << "ByVal __current As Ice.Current" << epar;

	if(comment)
	{
	    return;
	}
	else
	{
	    if(forTie)
	    {
		_out << " Implements _" << cl->name() << "Operations." << opName << "_async"; // TODO: should be containing class?
	    }
	}

	_out.inc();
	if(ret)
	{
	    _out << nl << "Dim __ret As " << typeToString(ret) << " = " << writeValue(ret);
	}
	for(i = params.begin(); i != params.end(); ++i)
	{
	    if((*i)->isOutParam())
	    {
		string name = fixId((*i)->name());
		TypePtr type = (*i)->type();
		_out << nl << "Dim " << name << " As " << typeToString(type) << " = " << writeValue(type);
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
	_out << epar;
	_out.dec();
	_out << nl << "End Sub";
    }
    else
    {
	vector<string> pDecls = getParams(op);
	string vbOp = ret ? "Function" : "Sub";

	_out << "Public Overloads ";
	if(!forTie && !cl->isLocal())
	{
	    _out << "Overrides ";
	}
	_out << vbOp << ' ' << fixId(opName, DotNet::ICloneable, true) << spar << pDecls;
	if(!cl->isLocal())
	{
	    _out << "ByVal __current As Ice.Current";
	}
	_out << epar;
	if(ret)
	{
	    _out << " As " << retS;
	}
	if(comment)
	{
	    return;
	}
	else
	{
	    if(forTie)
	    {
		_out << " Implements _" << cl->name() << "Operations." << fixId(opName, DotNet::ICloneable, true); // TODO: should be containing class?
	    }
	}
	_out.inc();
	for(ParamDeclList::const_iterator i = params.begin(); i != params.end(); ++i)
	{
	    if((*i)->isOutParam())
	    {
		string name = fixId((*i)->name());
		TypePtr type = (*i)->type();
		_out << nl << name << " = " << writeValue(type);
	    }
	}
	if(ret)
	{
	    _out << nl << "Return " << writeValue(ret);
	}
	_out.dec();
	_out << nl << "End " << vbOp;
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
	        return "0.0F";
		break;
	    }
	    case Builtin::KindDouble:
	    {
	        return "0.0R";
		break;
	    }
	    default:
	    {
	        return "Nothing";
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
        return st->hasMetaData("vb:class") ? "Nothing" : "New " + fixId(st->scoped()) + "()";
    }

    return "Nothing";
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

    _out << sp << nl << "Namespace " << fixId(p->name());
    _out.inc();

    return true;
}

void
Slice::Gen::ImplVisitor::visitModuleEnd(const ModulePtr&)
{
    _out.dec();
    _out << sp << nl << "End Namespace";
}

bool
Slice::Gen::ImplVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    if(!p->isAbstract())
    {
        return false;
    }

    string name = p->name();

    _out << sp << nl << "Public NotInheritable Class " << name << 'I';
    _out << nl;
    if(p->isInterface())
    {
        if(p->isLocal())
	{
	    _out << "Inherits Ice.LocalObjectImpl";
	    _out << nl << "Implements fixId(name)";
	}
	else
	{
	    _out << "Inherits _" << name << "Disp";
	}
    }
    else
    {
        _out << "Inherits " << fixId(name);
    }
    _out.inc();

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
    _out.dec();
    _out << sp << nl << "End Class";
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

    _out << sp << nl << "Namespace " << fixId(p->name());
    _out.inc();

    return true;
}

void
Slice::Gen::ImplTieVisitor::visitModuleEnd(const ModulePtr&)
{
    _out.dec();
    _out << sp << nl << "End Namespace";
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

    _out << sp << nl << "Public Class " << name << "I";
    _out.inc();
    if(inheritImpl)
    {
        _out << nl << "Inherits ";
	if(bases.front()->isAbstract())
	{
	    _out << bases.front()->name() << 'I';
	}
	else
	{
	    _out << fixId(bases.front()->name());
	}
    }
    _out << nl << "Implements _" << name << "Operations";

    _out << sp << nl << "Public Sub New()";
    _out << nl << "End Sub";

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
            _out << nl << "'";
            _out << nl << "' Implemented by " << bases.front()->name() << 'I';
            _out << nl << "'";
            writeOperation(*r, true, true);
        }
        else
        {
            writeOperation(*r, false, true);
        }
    }

    _out.dec();
    _out << sp << nl << "End Class";

    return true;
}
