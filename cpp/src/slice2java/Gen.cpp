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

Slice::JavaVisitor::JavaVisitor(const string& dir, const string& package) :
    JavaGenerator(dir, package)
{
}

Slice::JavaVisitor::~JavaVisitor()
{
}

string
Slice::JavaVisitor::getParams(const OperationPtr& op, const string& scope)
{
    string params;
    ParamDeclList paramList = op->parameters();
    for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
        if(q != paramList.begin())
        {
            params += ", ";
        }
	string typeString = typeToString((*q)->type(), (*q)->isOutParam() ? TypeModeOut : TypeModeIn, scope);
        params += typeString;
        params += ' ';
        params += fixKwd((*q)->name());
    }
    return params;
}

string
Slice::JavaVisitor::getParamsAsync(const OperationPtr& op, const string& scope, bool amd)
{
    string name = fixKwd(op->name());

    ContainerPtr container = op->container();
    ClassDefPtr cl = ClassDefPtr::dynamicCast(container);
    string classNameAsync = getAbsolute(cl->scoped(), scope, amd ? "AMD_" : "AMI_", '_' + name);

    string params = classNameAsync + " __cb";
    ParamDeclList paramList = op->parameters();
    for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
	if(!(*q)->isOutParam())
	{
	    string typeString = typeToString((*q)->type(), TypeModeIn, scope);
	    params += ", ";
	    params += typeString;
	    params += ' ';
	    params += fixKwd((*q)->name());
	}
    }
    return params;
}

string
Slice::JavaVisitor::getParamsAsyncCB(const OperationPtr& op, const string& scope)
{
    string params;
    TypePtr ret = op->returnType();
    if(ret)
    {
	string retS = typeToString(ret, TypeModeIn, scope);
	params += retS;
	params += " __ret";
    }
    ParamDeclList paramList = op->parameters();
    for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
	if((*q)->isOutParam())
	{
	    if(!params.empty())
	    {
		params += ", ";
	    }
	    string typeString = typeToString((*q)->type(), TypeModeIn, scope);
	    params += typeString;
	    params += ' ';
	    params += fixKwd((*q)->name());
	}
    }
    return params;
}

string
Slice::JavaVisitor::getArgs(const OperationPtr& op)
{
    string args;
    ParamDeclList paramList = op->parameters();
    for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
        if(q != paramList.begin())
        {
            args += ", ";
        }
        args += fixKwd((*q)->name());
    }
    return args;
}

string
Slice::JavaVisitor::getArgsAsync(const OperationPtr& op)
{
    string args = "__cb";
    ParamDeclList paramList = op->parameters();
    for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
	if(!(*q)->isOutParam())
	{
            args += ", ";
	    args += fixKwd((*q)->name());
	}
    }
    return args;
}

string
Slice::JavaVisitor::getArgsAsyncCB(const OperationPtr& op)
{
    string args;
    TypePtr ret = op->returnType();
    if(ret)
    {
	args += "__ret";
	BuiltinPtr builtin = BuiltinPtr::dynamicCast(ret);
	if((builtin && builtin->kind() == Builtin::KindObject) || ClassDeclPtr::dynamicCast(ret))
	{
	    args += ".value";
	}
    }
    ParamDeclList paramList = op->parameters();
    for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
	if((*q)->isOutParam())
	{
	    if(!args.empty())
	    {
		args += ", ";
	    }
	    args += fixKwd((*q)->name());
	    BuiltinPtr builtin = BuiltinPtr::dynamicCast((*q)->type());
	    if((builtin && builtin->kind() == Builtin::KindObject) || ClassDeclPtr::dynamicCast((*q)->type()))
	    {
		args += ".value";
	    }
	}
    }
    return args;
}

void
Slice::JavaVisitor::writeThrowsClause(const string& scope, const ExceptionList& throws)
{
    //
    // Don't include local exceptions in the throws clause
    //
    ExceptionList::size_type localCount = ice_count_if(throws.begin(), throws.end(),
						       IceUtil::constMemFun(&Exception::isLocal));

    Output& out = output();
    if(throws.size() - localCount > 0)
    {
        out.inc();
        out << nl << "throws ";
        out.useCurrentPosAsIndent();
        ExceptionList::const_iterator r;
        int count = 0;
        for(r = throws.begin(); r != throws.end(); ++r)
        {
            if(!(*r)->isLocal())
            {
                if(count > 0)
                {
                    out << "," << nl;
                }
                out << getAbsolute((*r)->scoped(), scope);
                count++;
            }
        }
        out.restoreIndent();
        out.dec();
    }
}

void
Slice::JavaVisitor::writeDelegateThrowsClause(const string& scope, const ExceptionList& throws)
{
    Output& out = output();
    out.inc();
    out << nl << "throws ";
    out.useCurrentPosAsIndent();
    out << "IceInternal.NonRepeatable";

    //
    // Don't include local exceptions in the throws clause
    //
    ExceptionList::const_iterator r;
    for(r = throws.begin(); r != throws.end(); ++r)
    {
        if(!(*r)->isLocal())
        {
            out << "," << nl;
            out << getAbsolute((*r)->scoped(), scope);
        }
    }
    out.restoreIndent();
    out.dec();
}

void
Slice::JavaVisitor::writeHashCode(Output& out, const TypePtr& type, const string& name, int& iter,
                                  const list<string>& metaData)
{
    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if(builtin)
    {
        switch(builtin->kind())
        {
            case Builtin::KindByte:
            case Builtin::KindShort:
            case Builtin::KindLong:
            {
                out << nl << "__h = 5 * __h + (int)" << name << ';';
                break;
            }
            case Builtin::KindBool:
            {
                out << nl << "__h = 5 * __h + (" << name << " ? 1 : 0);";
                break;
            }
            case Builtin::KindInt:
            {
                out << nl << "__h = 5 * __h + " << name << ';';
                break;
            }
            case Builtin::KindFloat:
            {
                out << nl << "__h = 5 * __h + java.lang.Float.floatToIntBits(" << name << ");";
                break;
            }
            case Builtin::KindDouble:
            {
                out << nl << "__h = 5 * __h + (int)java.lang.Double.doubleToLongBits(" << name << ");";
                break;
            }
            case Builtin::KindString:
            {
                out << nl << "__h = 5 * __h + " << name << ".hashCode();";
                break;
            }
            case Builtin::KindObject:
            case Builtin::KindObjectProxy:
            case Builtin::KindLocalObject:
            {
                out << nl << "if(" << name << " != null)";
                out << sb;
                out << nl << "__h = 5 * __h + " << name << ".hashCode();";
                out << eb;
                break;
            }
        }
        return;
    }

    ProxyPtr prx = ProxyPtr::dynamicCast(type);
    if(prx)
    {
        out << nl << "if(" << name << " != null)";
        out << sb;
        out << nl << "__h = 5 * __h + " << name << ".hashCode();";
        out << eb;
        return;
    }

    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
    if(cl)
    {
        out << nl << "if(" << name << " != null)";
        out << sb;
        out << nl << "__h = 5 * __h + " << name << ".hashCode();";
        out << eb;
        return;
    }

    SequencePtr seq = SequencePtr::dynamicCast(type);
    if(seq)
    {
        string listType = findMetaData(metaData);
        if(listType.empty())
        {
            list<string> l = seq->getMetaData();
            listType = findMetaData(l);
        }

        if(!listType.empty())
        {
            out << nl << "__h = 5 * __h + " << name << ".hashCode();";
        }
        else
        {
            out << nl << "for(int __i" << iter << " = 0; __i" << iter << " < " << name << ".length; __i" << iter
		<< "++)";
            out << sb;
            ostringstream elem;
            elem << name << "[__i" << iter << ']';
            iter++;
            writeHashCode(out, seq->type(), elem.str(), iter);
            out << eb;
        }
        return;
    }

    ConstructedPtr constructed = ConstructedPtr::dynamicCast(type);
    assert(constructed);
    out << nl << "__h = 5 * __h + " << name << ".hashCode();";
}

void
Slice::JavaVisitor::writeDispatch(Output& out, const ClassDefPtr& p)
{
    string name = fixKwd(p->name());
    string scope = p->scope();
    string scoped = p->scoped();
    ClassList bases = p->bases();

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
    StringList::difference_type scopedPos 
	= ice_distance(firstIter, scopedIter);

    out << sp << nl << "public static final String[] __ids =";
    out << sb;

    {
	StringList::const_iterator q = ids.begin();
	while(q != ids.end())
	{
	    out << nl << '"' << *q << '"';
	    if(++q != ids.end())
	    {
		out << ',';
	    }
	}
    }
    out << eb << ';';

    out << sp << nl << "public boolean" << nl << "ice_isA(String s, Ice.Current __current)";
    out << sb;
    out << nl << "return java.util.Arrays.binarySearch(__ids, s) >= 0;";
    out << eb;

    out << sp << nl << "public String[]" << nl << "ice_ids(Ice.Current __current)";
    out << sb;
    out << nl << "return __ids;";
    out << eb;

    out << sp << nl << "public String" << nl << "ice_id(Ice.Current __current)";
    out << sb;
    out << nl << "return __ids[" << scopedPos << "];";
    out << eb;

    out << sp << nl << "public static String" << nl << "ice_staticId()";
    out << sb;
    out << nl << "return __ids[" << scopedPos << "];";
    out << eb;

    //
    // Dispatch operations. We only generate methods for operations
    // defined in this ClassDef, because we reuse existing methods
    // for inherited operations.
    //
    OperationList ops = p->operations();
    OperationList::const_iterator r;
    for(r = ops.begin(); r != ops.end(); ++r)
    {
        OperationPtr op = *r;
        ContainerPtr container = op->container();
        ClassDefPtr cl = ClassDefPtr::dynamicCast(container);
        assert(cl);

        string opName = fixKwd(op->name());
        out << sp << nl << "public static IceInternal.DispatchStatus" << nl << "___" << opName << '(' << name
	    << " __obj, IceInternal.Incoming __in, Ice.Current __current)";
        out << sb;

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
	    remove_if(throws.begin(), throws.end(), IceUtil::constMemFun(&Exception::isLocal));

	    TypeStringList::const_iterator q;
	    int iter;
	    
	    if(!inParams.empty())
	    {
		out << nl << "IceInternal.BasicStream __is = __in.is();";
	    }
	    if(!outParams.empty() || ret || !throws.empty())
	    {
		out << nl << "IceInternal.BasicStream __os = __in.os();";
	    }
	    
	    //
	    // Unmarshal 'in' parameters.
	    //
	    iter = 0;
	    for(q = inParams.begin(); q != inParams.end(); ++q)
	    {
		string typeS = typeToString(q->first, TypeModeIn, scope);
		BuiltinPtr builtin = BuiltinPtr::dynamicCast(q->first);
		if((builtin && builtin->kind() == Builtin::KindObject) || ClassDeclPtr::dynamicCast(q->first))
		{
		    out << nl << typeS << "Holder " << fixKwd(q->second) << " = new " << typeS << "Holder();";
		    writeMarshalUnmarshalCode(out, scope, q->first, fixKwd(q->second), false, iter, true,
					      std::list<std::string>(), std::string());
		}
		else
		{
		    out << nl << typeS << ' ' << fixKwd(q->second) << ';';
		    writeMarshalUnmarshalCode(out, scope, q->first, fixKwd(q->second), false, iter);
		}
	    }
	    if(op->sendsClasses())
	    {
		out << nl << "__is.readPendingObjects();";
	    }
	    
	    //
	    // Create holders for 'out' parameters.
	    //
	    for(q = outParams.begin(); q != outParams.end(); ++q)
	    {
		string typeS = typeToString(q->first, TypeModeOut, scope);
		out << nl << typeS << ' ' << fixKwd(q->second) << " = new " << typeS << "();";
	    }
	    
	    //
	    // Call on the servant.
	    //
	    if(!throws.empty())
	    {
		out << nl << "try";
		out << sb;
	    }
	    out << nl;
	    if(ret)
	    {
		string retS = typeToString(ret, TypeModeReturn, scope);
		out << retS << " __ret = ";
	    }
	    out << "__obj." << opName << '(';
	    for(q = inParams.begin(); q != inParams.end(); ++q)
	    {
		out << fixKwd(q->second);
		BuiltinPtr builtin = BuiltinPtr::dynamicCast(q->first);
		if((builtin && builtin->kind() == Builtin::KindObject) || ClassDeclPtr::dynamicCast(q->first))
		{
		    out << ".value";
		}
		out << ", ";
	    }
	    for(q = outParams.begin(); q != outParams.end(); ++q)
	    {
		out << fixKwd(q->second) << ", ";
	    }
	    out << "__current);";
	    
	    //
	    // Marshal 'out' parameters and return value.
	    //
	    for(q = outParams.begin(); q != outParams.end(); ++q)
	    {
		writeMarshalUnmarshalCode(out, scope, q->first, fixKwd(q->second), true, iter, true);
	    }
	    if(ret)
	    {
		writeMarshalUnmarshalCode(out, scope, ret, "__ret", true, iter);
	    }
	    if(op->returnsClasses())
	    {
		out << nl << "__os.writePendingObjects();";
	    }
	    out << nl << "return IceInternal.DispatchStatus.DispatchOK;";
	    
	    //
	    // Handle user exceptions.
	    //
	    if(!throws.empty())
	    {
		out << eb;
		ExceptionList::const_iterator t;
		for(t = throws.begin(); t != throws.end(); ++t)
		{
		    string exS = getAbsolute((*t)->scoped(), scope);
		    out << nl << "catch(" << exS << " ex)";
		    out << sb;
		    out << nl << "__os.writeUserException(ex);";
		    out << nl << "return IceInternal.DispatchStatus.DispatchUserException;";
		    out << eb;
		}
	    }

	    out << eb;
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
	    int iter;
	    
	    if(!inParams.empty())
	    {
		out << nl << "IceInternal.BasicStream __is = __in.is();";
	    }
	    
	    //
	    // Unmarshal 'in' parameters.
	    //
	    iter = 0;
	    for(q = inParams.begin(); q != inParams.end(); ++q)
	    {
		string typeS = typeToString(q->first, TypeModeIn, scope);
		BuiltinPtr builtin = BuiltinPtr::dynamicCast(q->first);
		if((builtin && builtin->kind() == Builtin::KindObject) || ClassDeclPtr::dynamicCast(q->first))
		{
		    out << nl << typeS << "Holder " << fixKwd(q->second) << " = new " << typeS << "Holder();";
		    writeMarshalUnmarshalCode(out, scope, q->first, fixKwd(q->second), false, iter, true,
					      std::list<std::string>(), std::string());
		}
		else
		{
		    out << nl << typeS << ' ' << fixKwd(q->second) << ';';
		    writeMarshalUnmarshalCode(out, scope, q->first, fixKwd(q->second), false, iter);
		}
	    }
	    if(op->sendsClasses())
	    {
		out << nl << "__is.readPendingObjects();";
	    }
	    
	    //
	    // Call on the servant.
	    //
	    string classNameAMD = "AMD_" + fixKwd(p->name());
	    out << nl << classNameAMD << '_' << opName << " __cb = new _" << classNameAMD << '_' << opName
		<< "(__in);";
            out << nl << "try";
            out << sb;
	    out << nl << "__obj." << opName << (amd ? "_async(__cb, " : "(");
	    for(q = inParams.begin(); q != inParams.end(); ++q)
	    {
		out << fixKwd(q->second);
		BuiltinPtr builtin = BuiltinPtr::dynamicCast(q->first);
		if((builtin && builtin->kind() == Builtin::KindObject) || ClassDeclPtr::dynamicCast(q->first))
		{
		    out << ".value";
		}
		out << ", ";
	    }
	    out << "__current);";
	    out << eb;
	    out << nl << "catch(java.lang.Exception ex)";
	    out << sb;
	    out << nl << "__cb.ice_exception(ex);";
	    out << eb;
	    out << nl << "return IceInternal.DispatchStatus.DispatchAsync;";

	    out << eb;
	}
    }

    OperationList allOps = p->allOperations();
    if(!allOps.empty())
    {
        StringList allOpNames;
        transform(allOps.begin(), allOps.end(), back_inserter(allOpNames), ::IceUtil::constMemFun(&Contained::name));
        allOpNames.push_back("ice_facets");
        allOpNames.push_back("ice_id");
        allOpNames.push_back("ice_ids");
        allOpNames.push_back("ice_isA");
        allOpNames.push_back("ice_ping");
        allOpNames.sort();
        allOpNames.unique();

        StringList::const_iterator q;

        out << sp << nl << "private final static String[] __all =";
        out << sb;
        q = allOpNames.begin();
        while(q != allOpNames.end())
        {
            out << nl << '"' << *q << '"';
            if(++q != allOpNames.end())
            {
                out << ',';
            }
        }
        out << eb << ';';

        out << sp << nl << "public IceInternal.DispatchStatus" << nl
	    << "__dispatch(IceInternal.Incoming in, Ice.Current __current)";
        out << sb;
        out << nl << "int pos = java.util.Arrays.binarySearch(__all, __current.operation);";
        out << nl << "if(pos < 0)";
        out << sb;
        out << nl << "return IceInternal.DispatchStatus.DispatchOperationNotExist;";
        out << eb;
        out << sp << nl << "switch(pos)";
        out << sb;
        int i = 0;
        for(q = allOpNames.begin(); q != allOpNames.end(); ++q)
        {
            string opName = fixKwd(*q);

            out << nl << "case " << i++ << ':';
            out << sb;
            if(opName == "ice_facets")
            {
                out << nl << "return ___ice_facets(this, in, __current);";
            }
            else if(opName == "ice_id")
            {
                out << nl << "return ___ice_id(this, in, __current);";
            }
            else if(opName == "ice_ids")
            {
                out << nl << "return ___ice_ids(this, in, __current);";
            }
            else if(opName == "ice_isA")
            {
                out << nl << "return ___ice_isA(this, in, __current);";
            }
            else if(opName == "ice_ping")
            {
                out << nl << "return ___ice_ping(this, in, __current);";
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
                            out << nl << "return ___" << opName << "(this, in, __current);";
                        }
                        else
                        {
                            string base;
                            if(cl->isInterface())
                            {
                                base = getAbsolute(cl->scoped(), scope, "_", "Disp");
                            }
                            else
                            {
                                base = getAbsolute(cl->scoped(), scope);
                            }
                            out << nl << "return " << base << ".___" << opName << "(this, in, __current);";
                        }
                        break;
                    }
                }
            }
            out << eb;
        }
        out << eb;
        out << sp << nl << "assert(false);";
        out << nl << "return IceInternal.DispatchStatus.DispatchOperationNotExist;";
        out << eb;
    }
}

Slice::Gen::Gen(const string& name, const string& base, const vector<string>& includePaths, const string& package,
                const string& dir) :
    _base(base),
    _includePaths(includePaths),
    _package(package),
    _dir(dir)
{
}

Slice::Gen::~Gen()
{
}

bool
Slice::Gen::operator!() const
{
    return false;
}

void
Slice::Gen::generate(const UnitPtr& p)
{
    OpsVisitor opsVisitor(_dir, _package);
    p->visit(&opsVisitor);

    TypesVisitor typesVisitor(_dir, _package);
    p->visit(&typesVisitor);

    HolderVisitor holderVisitor(_dir, _package);
    p->visit(&holderVisitor);

    HelperVisitor helperVisitor(_dir, _package);
    p->visit(&helperVisitor);

    ProxyVisitor proxyVisitor(_dir, _package);
    p->visit(&proxyVisitor);

    DelegateVisitor delegateVisitor(_dir, _package);
    p->visit(&delegateVisitor);

    DelegateMVisitor delegateMVisitor(_dir, _package);
    p->visit(&delegateMVisitor);

    DelegateDVisitor delegateDVisitor(_dir, _package);
    p->visit(&delegateDVisitor);

    DispatcherVisitor dispatcherVisitor(_dir, _package);
    p->visit(&dispatcherVisitor);

    AsyncVisitor asyncVisitor(_dir, _package);
    p->visit(&asyncVisitor);
}

void
Slice::Gen::generateTie(const UnitPtr& p)
{
    TieVisitor tieVisitor(_dir, _package);
    p->visit(&tieVisitor);
}

void
Slice::Gen::generateImpl(const UnitPtr& p)
{
    ImplVisitor implVisitor(_dir, _package);
    p->visit(&implVisitor);
}

void
Slice::Gen::generateImplTie(const UnitPtr& p)
{
    ImplTieVisitor implTieVisitor(_dir, _package);
    p->visit(&implTieVisitor);
}

Slice::Gen::OpsVisitor::OpsVisitor(const string& dir, const string& package) :
    JavaVisitor(dir, package)
{
}

bool
Slice::Gen::OpsVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    string name = fixKwd(p->name());
    string scoped = p->scoped();
    ClassList bases = p->bases();
    string scope = p->scope();
    string absolute = getAbsolute(scoped, "", "_", "Operations");

    //
    // Don't generate an Operations interface for non-abstract classes
    //
    if(!p->isAbstract())
    {
        return false;
    }

    if(!open(absolute))
    {
        return false;
    }

    Output& out = output();

    //
    // Generate the operations interface
    //
    out << sp << nl << "public interface " << '_' << name << "Operations";
    if((bases.size() == 1 && bases.front()->isAbstract()) || bases.size() > 1)
    {
        out << " extends ";
        out.useCurrentPosAsIndent();
        ClassList::const_iterator q = bases.begin();
        bool first = true;
        while(q != bases.end())
        {
            if((*q)->isAbstract())
            {
                if(!first)
                {
                    out << ',' << nl;
                }
                else
                {
                    first = false;
                }
                out << getAbsolute((*q)->scoped(), scope, "_", "Operations");
            }
            ++q;
        }
        out.restoreIndent();
    }

    out << sb;

    return true;
}

void
Slice::Gen::OpsVisitor::visitClassDefEnd(const ClassDefPtr& p)
{
    Output& out = output();
    out << eb;
    close();
}

void
Slice::Gen::OpsVisitor::visitOperation(const OperationPtr& p)
{
    string name = fixKwd(p->name());
    ContainerPtr container = p->container();
    ClassDefPtr cl = ClassDefPtr::dynamicCast(container);
    string scope = cl->scope();

    TypePtr ret;
    string params;

    bool amd = !cl->isLocal() && (cl->hasMetaData("amd") || p->hasMetaData("amd"));

    if(amd)
    {
	params = getParamsAsync(p, scope, true);
    }
    else
    {
	params = getParams(p, scope);
	ret = p->returnType();
    }

    string retS = typeToString(ret, TypeModeReturn, scope);
    
    Output& out = output();
    
    out << sp;
    out << nl << retS << ' ' << name << (amd ? "_async(" : "(") << params;
    if(!cl->isLocal())
    {
	if(!params.empty())
	{
	    out << ", ";
	}
	out << "Ice.Current __current";
    }
    out << ')';
    ExceptionList throws = p->throws();
    throws.sort();
    throws.unique();
    writeThrowsClause(scope, throws);
    out << ';';
}

Slice::Gen::TieVisitor::TieVisitor(const string& dir, const string& package) :
    JavaVisitor(dir, package)
{
}

bool
Slice::Gen::TieVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    string name = fixKwd(p->name());
    string scoped = p->scoped();
    ClassList bases = p->bases();
    string scope = p->scope();
    string absolute = getAbsolute(scoped, "", "_", "Tie");

    //
    // Don't generate a TIE class for a non-abstract class
    //
    if(!p->isAbstract())
    {
        return false;
    }

    if(!open(absolute))
    {
        return false;
    }

    Output& out = output();

    //
    // Generate the TIE class
    //
    out << sp << nl << "public class " << '_' << name << "Tie";
    if(p->isInterface())
    {
        if(p->isLocal())
        {
            out << " implements " << name;
        }
        else
        {
            out << " extends " << '_' << name << "Disp";
        }
    }
    else
    {
        out << " extends " << name;
    }

    out << sb;

    out << sp << nl << "public" << nl << '_' << name << "Tie()";
    out << sb;
    out << eb;

    out << sp << nl << "public" << nl << '_' << name << "Tie(" << '_' << name << "Operations delegate)";
    out << sb;
    out << nl << "_ice_delegate = delegate;";
    out << eb;

    out << sp << nl << "public " << '_' << name << "Operations" << nl << "ice_delegate()";
    out << sb;
    out << nl << "return _ice_delegate;";
    out << eb;

    out << sp << nl << "public void" << nl << "ice_delegate(" << '_' << name << "Operations delegate)";
    out << sb;
    out << nl << "_ice_delegate = delegate;";
    out << eb;

    out << sp << nl << "public boolean" << nl << "equals(java.lang.Object rhs)";
    out << sb;
    out << nl << "if(this == rhs)";
    out << sb;
    out << nl << "return true;";
    out << eb;
    out << nl << "if(!(rhs instanceof " << '_' << name << "Tie))";
    out << sb;
    out << nl << "return false;";
    out << eb;
    out << sp << nl << "return _ice_delegate.equals(((" << '_' << name << "Tie)rhs)._ice_delegate);";
    out << eb;

    out << sp << nl << "public int" << nl << "hashCode()";
    out << sb;
    out << nl << "return _ice_delegate.hashCode();";
    out << eb;

    OperationList ops = p->allOperations();
    OperationList::const_iterator r;
    for(r = ops.begin(); r != ops.end(); ++r)
    {
        string opName = fixKwd((*r)->name());

        TypePtr ret = (*r)->returnType();
        string retS = typeToString(ret, TypeModeReturn, scope);

        string params = getParams((*r), scope);
        string args = getArgs(*r);

	bool hasAMD = p->hasMetaData("amd") || (*r)->hasMetaData("amd");

        out << sp;
        out << nl << "public " << (hasAMD ? "void" : retS) << nl << opName;
	if(hasAMD)
	{
	    out << "_async";
	}
	out << '(';
	if(hasAMD)
	{
	    ContainedPtr definingContainer = ContainedPtr::dynamicCast((*r)->container());
	    out << "AMD_" << fixKwd(definingContainer->name()) << '_' << opName << " __cb, ";
	}
	out << params;

        if(!p->isLocal())
        {
            if(!params.empty() || hasAMD)
            {
                out << ", ";
            }
            out << "Ice.Current __current";
        }
        out << ')';

        ExceptionList throws = (*r)->throws();
        throws.sort();
        throws.unique();
        writeThrowsClause(scope, throws);
        out << sb;
        out << nl;
        if(ret && !hasAMD)
        {
            out << "return ";
        }
        out << "_ice_delegate." << opName << (hasAMD ? "_async(__cb, " : "(");
	out << args;
        if(!p->isLocal())
        {
            if(!args.empty())
            {
                out << ", ";
            }
            out << "__current";
        }
        out << ");";
        out << eb;
    }

    out << sp << nl << "private " << '_' << name << "Operations _ice_delegate;";
    out << eb;
    close();

    return false;
}

Slice::Gen::TypesVisitor::TypesVisitor(const string& dir, const string& package) :
    JavaVisitor(dir, package)
{
}

bool
Slice::Gen::TypesVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    string name = fixKwd(p->name());
    string scoped = p->scoped();
    ClassList bases = p->bases();
    string scope = p->scope();
    string absolute = getAbsolute(scoped);

    if(!open(absolute))
    {
        return false;
    }

    Output& out = output();

    //
    // Slice interfaces map to Java interfaces.
    //
    if(p->isInterface())
    {
        out << sp << nl << "public interface " << name << " extends ";
        out.useCurrentPosAsIndent();
        if(p->isLocal())
        {
            out << "Ice.LocalObject";
        }
        else
        {
            out << "Ice.Object";
        }
        out << "," << nl << '_' << name << "Operations";
        if(!bases.empty())
        {
            ClassList::const_iterator q = bases.begin();
            while(q != bases.end())
            {
                out << ',' << nl << getAbsolute((*q)->scoped(), scope);
                q++;
            }
        }
        out.restoreIndent();
    }
    else
    {
        out << sp << nl << "public ";
        if(p->isAbstract())
        {
            out << "abstract ";
        }
        out << "class " << name;
        out.useCurrentPosAsIndent();
        if(bases.empty() || bases.front()->isInterface())
        {
            if(p->isLocal())
            {
                out << " extends Ice.LocalObjectImpl";
            }
            else
            {
                out << " extends Ice.ObjectImpl";
            }
        }
        else
        {
            out << " extends ";
            ClassDefPtr base = bases.front();
            out << getAbsolute(base->scoped(), scope);
            bases.pop_front();
        }

        //
        // Implement interfaces
        //
        StringList implements;
        if(p->isAbstract())
        {
            implements.push_back("_" + name + "Operations");
        }
        if(!bases.empty())
        {
            ClassList::const_iterator q = bases.begin();
            while(q != bases.end())
            {
                implements.push_back(getAbsolute((*q)->scoped(), scope));
                q++;
            }
        }

        if(!implements.empty())
        {
            out << nl << " implements ";
            out.useCurrentPosAsIndent();

            StringList::const_iterator q = implements.begin();
            while(q != implements.end())
            {
                if(q != implements.begin())
                {
                    out << nl << ',';
                }
                out << *q;
                q++;
            }

            out.restoreIndent();
        }

        out.restoreIndent();
    }

    out << sb;

    //
    // Default factory for non-abstract classes.
    //
    if(!p->isAbstract())
    {
        out << sp;
        out << nl << "private static class __F extends Ice.LocalObjectImpl implements Ice.ObjectFactory";
        out << sb;
        out << nl << "public Ice.Object" << nl << "create(String type)";
        out << sb;
        out << nl << "assert(type.equals(ice_staticId()));";
        out << nl << "return new " << name << "();";
        out << eb;
        out << sp << nl << "public void" << nl << "destroy()";
        out << sb;
        out << eb;
        out << eb;
        out << nl << "private static Ice.ObjectFactory _factory = new __F();";
        out << sp;
        out << nl << "public static Ice.ObjectFactory" << nl << "ice_factory()";
        out << sb;
        out << nl << "return _factory;";
        out << eb;
    }

    //
    // Marshalling & dispatch support.
    //
    if(!p->isInterface() && !p->isLocal())
    {
        writeDispatch(out, p);

        DataMemberList members = p->dataMembers();
        DataMemberList::const_iterator d;
        int iter;

        out << sp << nl << "public void" << nl << "__write(IceInternal.BasicStream __os, boolean __marshalFacets)";
        out << sb;
	out << nl << "__os.writeTypeId(ice_staticId());";
	out << nl << "__os.startWriteSlice();";
        iter = 0;
        for(d = members.begin(); d != members.end(); ++d)
        {
            list<string> metaData = (*d)->getMetaData();
            writeMarshalUnmarshalCode(out, scope, (*d)->type(), fixKwd((*d)->name()), true, iter, false, metaData);
        }
	out << nl << "__os.endWriteSlice();";
        out << nl << "super.__write(__os, __marshalFacets);";
        out << eb;

	DataMemberList allClassMembers = p->allClassDataMembers();
	if(allClassMembers.size() != 0)
	{
	    out << sp << nl << "private class Patcher implements IceInternal.Patcher";
	    out << sb;
	    if(allClassMembers.size() > 1)
	    {
		out << sp << nl << "Patcher(int member)";
		out << sb;
		out << nl << "__member = member;";
		out << eb;
	    }

	    out << sp << nl << "public void" << nl << "patch(Ice.Object v)";
	    out << sb;
	    if(allClassMembers.size() > 1)
	    {
		out << nl << "switch(__member)";
		out << sb;
	    }
	    int memberCount = 0;
	    for(d = allClassMembers.begin(); d != allClassMembers.end(); ++d)
	    {
		if(allClassMembers.size() > 1)
		{
		    out.dec();
		    out << nl << "case " << memberCount << ":";
		    out.inc();
		}
		if(allClassMembers.size() > 1)
		{
		    out << nl << "__typeId = \"" << (*d)->type()->typeId() << "\";";
		}
		string memberName = fixKwd((*d)->name());
		string memberScope = fixKwd((*d)->scope());
		string memberType = typeToString((*d)->type(), TypeModeMember, memberScope);
		out << nl << memberName << " = (" << memberType << ")v;";
		if(allClassMembers.size() > 1)
		{
		    out << nl << "break;";
		}
		memberCount++;
	    }
	    if(allClassMembers.size() > 1)
	    {
		out << eb;
	    }
	    out << eb;

	    out << sp << nl << "public String" << nl << "type()";
	    out << sb;
	    if(allClassMembers.size() > 1)
	    {
		out << nl << "return __typeId;";
	    }
	    else
	    {
		out << nl << "return \"" << (*allClassMembers.begin())->type()->typeId() << "\";";
	    }
	    out << eb;

	    if(allClassMembers.size() > 1)
	    {
		out << sp << nl << "private int __member;";
		out << nl << "private String __typeId;";
	    }
	    out << eb;
	}

        out << sp << nl << "public void" << nl << "__read(IceInternal.BasicStream __is, boolean __rid)";
        out << sb;
	out << nl << "if(__rid)";
	out << sb;
	out << nl << "String myId = __is.readTypeId();";
	out << eb;
	out << nl << "__is.startReadSlice();";
        iter = 0;
	DataMemberList classMembers = p->classDataMembers();
	long classMemberCount = allClassMembers.size() - classMembers.size();
        for(d = members.begin(); d != members.end(); ++d)
        {
            list<string> metaData = (*d)->getMetaData();
	    ostringstream patchParams;
	    BuiltinPtr builtin = BuiltinPtr::dynamicCast((*d)->type());
	    if((builtin && builtin->kind() == Builtin::KindObject) || ClassDeclPtr::dynamicCast((*d)->type()))
	    {
		if(classMembers.size() > 1 || allClassMembers.size() > 1)
		{
		    patchParams << "new Patcher(" << classMemberCount++ << ')';
		}
	    }
            writeMarshalUnmarshalCode(out, scope, (*d)->type(), fixKwd((*d)->name()), false, iter, false, metaData,
		                      patchParams.str());
        }
	out << nl << "__is.endReadSlice();";
        out << nl << "super.__read(__is, true);";
        out << eb;
    }

    return true;
}

void
Slice::Gen::TypesVisitor::visitClassDefEnd(const ClassDefPtr& p)
{
    Output& out = output();
    out << eb;
    close();
}

bool
Slice::Gen::TypesVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    string name = fixKwd(p->name());
    string scoped = p->scoped();
    ExceptionPtr base = p->base();
    string scope = p->scope();
    string absolute = getAbsolute(scoped);

    if(!open(absolute))
    {
        return false;
    }

    Output& out = output();

    out << sp << nl << "public class " << name << " extends ";

    if(!base)
    {
        if(p->isLocal())
        {
            out << "Ice.LocalException";
        }
        else
        {
            out << "Ice.UserException";
        }
    }
    else
    {
        out << getAbsolute(base->scoped(), p->scope());
    }
    out << sb;

    out << sp << nl << "public String" << nl << "ice_name()";
    out << sb;
    out << nl << "return \"" << scoped.substr(2) << "\";";
    out << eb;

    return true;
}

void
Slice::Gen::TypesVisitor::visitExceptionEnd(const ExceptionPtr& p)
{
    Output& out = output();

    if(!p->isLocal())
    {
        string name = fixKwd(p->name());
        string scoped = p->scoped();
        string scope = p->scope();
        ExceptionPtr base = p->base();

        DataMemberList members = p->dataMembers();
        DataMemberList::const_iterator d;
        int iter;

        out << sp << nl << "public void" << nl << "__write(IceInternal.BasicStream __os)";
        out << sb;
	out << nl << "__os.writeString(\"" << p->scoped() << "\");";
	out << nl << "__os.startWriteSlice();";
        iter = 0;
        for(d = members.begin(); d != members.end(); ++d)
        {
            list<string> metaData = (*d)->getMetaData();
            writeMarshalUnmarshalCode(out, scope, (*d)->type(), fixKwd((*d)->name()), true, iter, false, metaData);
        }
	out << nl << "__os.endWriteSlice();";
        if(base)
        {
            out << nl << "super.__write(__os);";
        }
        out << eb;

	DataMemberList allClassMembers = p->allClassDataMembers();
	if(allClassMembers.size() != 0)
	{
	    out << sp << nl << "private class Patcher implements IceInternal.Patcher";
	    out << sb;
	    if(allClassMembers.size() > 1)
	    {
		out << sp << nl << "Patcher(int member)";
		out << sb;
		out << nl << "__member = member;";
		out << eb;
	    }

	    out << sp << nl << "public void" << nl << "patch(Ice.Object v)";
	    out << sb;
	    if(allClassMembers.size() > 1)
	    {
		out << nl << "switch(__member)";
		out << sb;
	    }
	    int memberCount = 0;
	    for(d = allClassMembers.begin(); d != allClassMembers.end(); ++d)
	    {
		if(allClassMembers.size() > 1)
		{
		    out.dec();
		    out << nl << "case " << memberCount << ":";
		    out.inc();
		}
		if(allClassMembers.size() > 1)
		{
		    out << nl << "__typeId = \"" << (*d)->type()->typeId() << "\";";
		}
		string memberName = fixKwd((*d)->name());
		string memberScope = fixKwd((*d)->scope());
		string memberType = typeToString((*d)->type(), TypeModeMember, memberScope);
		out << nl << memberName << " = (" << memberType << ")v;";
		if(allClassMembers.size() > 1)
		{
		    out << nl << "break;";
		}
		memberCount++;
	    }
	    if(allClassMembers.size() > 1)
	    {
		out << eb;
	    }
	    out << eb;

	    out << sp << nl << "public String" << nl << "type()";
	    out << sb;
	    if(allClassMembers.size() > 1)
	    {
		out << nl << "return __typeId;";
	    }
	    else
	    {
		out << nl << "return \"" << (*allClassMembers.begin())->type()->typeId() << "\";";
	    }
	    out << eb;
	    if(allClassMembers.size() > 1)
	    {
		out << sp << nl << "private int __member;";
		out << nl << "private String __typeId;";
	    }
	    out << eb;
	}
        out << sp << nl << "public void" << nl << "__read(IceInternal.BasicStream __is, boolean __rid)";
        out << sb;
	out << nl << "if(__rid)";
	out << sb;
	out << nl << "String myId = __is.readString();";
	out << eb;
	out << nl << "__is.startReadSlice();";
        iter = 0;
	DataMemberList classMembers = p->classDataMembers();
	long classMemberCount = allClassMembers.size() - classMembers.size();
        for(d = members.begin(); d != members.end(); ++d)
        {
	    ostringstream patchParams;
	    BuiltinPtr builtin = BuiltinPtr::dynamicCast((*d)->type());
	    if((builtin && builtin->kind() == Builtin::KindObject) || ClassDeclPtr::dynamicCast((*d)->type()))
	    {
		if(classMembers.size() > 1 || allClassMembers.size() > 1)
		{
		    patchParams << "new Patcher(" << classMemberCount++ << ')';
		}
	    }
            list<string> metaData = (*d)->getMetaData();
            writeMarshalUnmarshalCode(out, scope, (*d)->type(), fixKwd((*d)->name()), false, iter, false, metaData,
		    		      patchParams.str());
        }
	out << nl << "__is.endReadSlice();";
        if(base)
        {
            out << nl << "super.__read(__is, true);";
        }
        out << eb;

	if(p->usesClasses())
	{
	    if(!base || base && !base->usesClasses())
	    {
		out << sp << nl << "public boolean" << nl << "__usesClasses()";
		out << sb;
		out << nl << "return true;";
		out << eb;
	    }
	}
    }

    out << eb;
    close();
}

bool
Slice::Gen::TypesVisitor::visitStructStart(const StructPtr& p)
{
    string name = fixKwd(p->name());
    string scoped = p->scoped();
    string absolute = getAbsolute(scoped);

    if(!open(absolute))
    {
        return false;
    }

    Output& out = output();

    out << sp << nl << "public final class " << name << " implements java.lang.Cloneable";
    out << sb;

    return true;
}

void
Slice::Gen::TypesVisitor::visitStructEnd(const StructPtr& p)
{
    string scope = p->scope();

    Output& out = output();

    DataMemberList members = p->dataMembers();
    DataMemberList::const_iterator d;
    int iter;

    string typeS = typeToString(p, TypeModeIn, scope);

    out << sp << nl << "public boolean" << nl << "equals(java.lang.Object rhs)";
    out << sb;
    out << nl << typeS << " _r = null;";
    out << nl << "try";
    out << sb;
    out << nl << "_r = (" << typeS << ")rhs;";
    out << eb;
    out << nl << "catch(ClassCastException ex)";
    out << sb;
    out << eb;
    out << sp << nl << "if(_r != null)";
    out << sb;
    for(d = members.begin(); d != members.end(); ++d)
    {
        string memberName = fixKwd((*d)->name());
        BuiltinPtr b = BuiltinPtr::dynamicCast((*d)->type());
        if(b)
        {
            switch(b->kind())
            {
                case Builtin::KindByte:
                case Builtin::KindBool:
                case Builtin::KindShort:
                case Builtin::KindInt:
                case Builtin::KindLong:
                case Builtin::KindFloat:
                case Builtin::KindDouble:
                {
                    out << nl << "if(" << memberName << " != _r." << memberName << ')';
                    out << sb;
                    out << nl << "return false;";
                    out << eb;
                    break;
                }

                case Builtin::KindString:
                case Builtin::KindObject:
                case Builtin::KindObjectProxy:
                case Builtin::KindLocalObject:
                {
                    out << nl << "if(!" << memberName << ".equals(_r." << memberName << "))";
                    out << sb;
                    out << nl << "return false;";
                    out << eb;
                    break;
                }
            }
        }
        else
        {
            //
            // We treat sequences differently because the native equals() method for
            // a Java array does not perform a deep comparison. If the mapped type
            // is not overridden via metadata, we use the helper method
            // java.util.Arrays.equals() to compare native arrays.
            //
            // For all other types, we can use the native equals() method.
            //
            SequencePtr seq = SequencePtr::dynamicCast((*d)->type());
            if(seq)
            {
                list<string> metaData = (*d)->getMetaData();
                string listType = findMetaData(metaData);
                if(listType.empty())
                {
                    list<string> l = seq->getMetaData();
                    listType = findMetaData(l);
                }
                if(!listType.empty())
                {
                    out << nl << "if(!" << memberName << ".equals(_r." << memberName << "))";
                    out << sb;
                    out << nl << "return false;";
                    out << eb;
                }
                else
                {
                    out << nl << "if(!java.util.Arrays.equals(" << memberName << ", _r." << memberName << "))";
                    out << sb;
                    out << nl << "return false;";
                    out << eb;
                }
            }
            else
            {
                out << nl << "if(!" << memberName << ".equals(_r." << memberName << "))";
                out << sb;
                out << nl << "return false;";
                out << eb;
            }
        }
    }
    out << sp << nl << "return true;";
    out << eb;
    out << sp << nl << "return false;";
    out << eb;

    out << sp << nl << "public int" << nl << "hashCode()";
    out << sb;
    out << nl << "int __h = 0;";
    iter = 0;
    for(d = members.begin(); d != members.end(); ++d)
    {
        string memberName = fixKwd((*d)->name());
        list<string> metaData = (*d)->getMetaData();
        writeHashCode(out, (*d)->type(), memberName, iter, metaData);
    }
    out << nl << "return __h;";
    out << eb;

    out << sp << nl << "public java.lang.Object" << nl << "clone()";
    out.inc();
    out << nl << "throws java.lang.CloneNotSupportedException";
    out.dec();
    out << sb;
    out << nl << "return super.clone();";
    out << eb;

    if(!p->isLocal())
    {
        out << sp << nl << "public void" << nl << "__write(IceInternal.BasicStream __os)";
        out << sb;
        iter = 0;
        for(d = members.begin(); d != members.end(); ++d)
        {
            list<string> metaData = (*d)->getMetaData();
            writeMarshalUnmarshalCode(out, scope, (*d)->type(), fixKwd((*d)->name()), true, iter, false, metaData);
        }
        out << eb;

	DataMemberList classMembers = p->classDataMembers();

	if(classMembers.size() != 0)
	{
	    out << sp << nl << "private class Patcher implements IceInternal.Patcher";
	    out << sb;
	    if(classMembers.size() > 1)
	    {
		out << sp << nl << "Patcher(int member)";
		out << sb;
		out << nl << "__member = member;";
		out << eb;
	    }

	    out << sp << nl << "public void" << nl << "patch(Ice.Object v)";
	    out << sb;
	    if(classMembers.size() > 1)
	    {
		out << nl << "switch(__member)";
		out << sb;
	    }
	    int memberCount = 0;
	    for(d = classMembers.begin(); d != classMembers.end(); ++d)
	    {
		if(classMembers.size() > 1)
		{
		    out.dec();
		    out << nl << "case " << memberCount << ":";
		    out.inc();
		}
		if(classMembers.size() > 1)
		{
		    out << nl << "__typeId = \"" << (*d)->type()->typeId() << "\";";
		}
		string memberName = fixKwd((*d)->name());
		string memberScope = fixKwd((*d)->scope());
		string memberType = typeToString((*d)->type(), TypeModeMember, memberScope);
		out << nl << memberName << " = (" << memberType << ")v;";
		if(classMembers.size() > 1)
		{
		    out << nl << "break;";
		}
		memberCount++;
	    }
	    if(classMembers.size() > 1)
	    {
		out << eb;
	    }
	    out << eb;

	    out << sp << nl << "public String" << nl << "type()";
	    out << sb;
	    if(classMembers.size() > 1)
	    {
		out << nl << "return __typeId;";
	    }
	    else
	    {
		out << nl << "return \"" << (*classMembers.begin())->type()->typeId() << "\";";
	    }
	    out << eb;

	    if(classMembers.size() > 1)
	    {
		out << sp << nl << "private int __member;";
		out << nl << "private String __typeId;";
	    }
	    out << eb;
	}

        out << sp << nl << "public void" << nl << "__read(IceInternal.BasicStream __is)";
        out << sb;
        iter = 0;
	int classMemberCount = 0;
        for(d = members.begin(); d != members.end(); ++d)
        {
	    ostringstream patchParams;
	    BuiltinPtr builtin = BuiltinPtr::dynamicCast((*d)->type());
	    if((builtin && builtin->kind() == Builtin::KindObject) || ClassDeclPtr::dynamicCast((*d)->type()))
	    {
		if(classMembers.size() > 1)
		{
		    patchParams << "new Patcher(" << classMemberCount++ << ')';
		}
	    }
            list<string> metaData = (*d)->getMetaData();
            writeMarshalUnmarshalCode(out, scope, (*d)->type(), fixKwd((*d)->name()), false, iter, false, metaData,
		    		      patchParams.str());
        }
        out << eb;
    }

    out << eb;
    close();
}

void
Slice::Gen::TypesVisitor::visitDataMember(const DataMemberPtr& p)
{
    string name = fixKwd(p->name());
    ContainerPtr container = p->container();
    ContainedPtr contained = ContainedPtr::dynamicCast(container);
    list<string> metaData = p->getMetaData();
    string s = typeToString(p->type(), TypeModeMember, contained->scope(), metaData);
    Output& out = output();
    out << sp << nl << "public " << s << ' ' << name << ';';
}

void
Slice::Gen::TypesVisitor::visitEnum(const EnumPtr& p)
{
    string name = fixKwd(p->name());
    string scoped = p->scoped();
    string absolute = getAbsolute(scoped);
    EnumeratorList enumerators = p->getEnumerators();
    EnumeratorList::const_iterator en;
    size_t sz = enumerators.size();

    if(!open(absolute))
    {
        return;
    }

    Output& out = output();

    out << sp << nl << "public final class " << name;
    out << sb;
    out << nl << "private static " << name << "[] __values = new " << name << "[" << sz << "];";
    out << nl << "private int __value;";
    out << sp;
    int n;
    for(en = enumerators.begin(), n = 0; en != enumerators.end(); ++en, ++n)
    {
        string member = fixKwd((*en)->name());
        out << nl << "public static final int _" << member << " = " << n << ';';
        out << nl << "public static final " << name << ' ' << member << " = new " << name << "(_" << member << ");";
    }

    out << sp << nl << "public static " << name << nl << "convert(int val)";
    out << sb;
    out << nl << "assert val < " << sz << ';';
    out << nl << "return __values[val];";
    out << eb;

    out << sp << nl << "public int" << nl << "value()";
    out << sb;
    out << nl << "return __value;";
    out << eb;

    out << sp << nl << "private" << nl << name << "(int val)";
    out << sb;
    out << nl << "__value = val;";
    out << nl << "__values[val] = this;";
    out << eb;

    if(!p->isLocal())
    {
        out << sp << nl << "public void" << nl << "__write(IceInternal.BasicStream __os)";
        out << sb;
        if(sz <= 0x7f)
        {
            out << nl << "__os.writeByte((byte)__value);";
        }
        else if(sz <= 0x7fff)
        {
            out << nl << "__os.writeShort((short)__value);";
        }
        else
        {
            out << nl << "__os.writeInt(__value);";
        }
        out << eb;

        out << sp << nl << "public static " << name << nl << "__read(IceInternal.BasicStream __is)";
        out << sb;
        if(sz <= 0x7f)
        {
            out << nl << "int __v = __is.readByte();";
        }
        else if(sz <= 0x7fff)
        {
            out << nl << "int __v = __is.readShort();";
        }
        else
        {
            out << nl << "int __v = __is.readInt();";
        }
        out << nl << "return " << name << ".convert(__v);";
        out << eb;

        out << sp << nl << "final static private String[] __T =";
        out << sb;
        en = enumerators.begin();
        while(en != enumerators.end())
        {
            out << nl << "\"" << (*en)->name() << "\"";
            if(++en != enumerators.end())
            {
                out << ',';
            }
        }
        out << eb << ';';
    }

    out << eb;
    close();
}

void
Slice::Gen::TypesVisitor::visitConst(const ConstPtr& p)
{
    string name = fixKwd(p->name());
    string scoped = p->scoped();
    string scope = p->scope();
    string absolute = getAbsolute(scoped);
    TypePtr type = p->type();

    if(!open(absolute))
    {
        return;
    }
    Output& out = output();
    out << sp << nl << "public interface " << name;
    out << sb;
    out << nl << typeToString(type, TypeModeIn, scope) << " value = ";

    BuiltinPtr bp;
    EnumPtr ep;
    if(bp = BuiltinPtr::dynamicCast(type))
    {
	switch(bp->kind())
	{
	    case Builtin::KindString:
	    {
		out << "\"";					// Opening "

		ios_base::fmtflags originalFlags = out.flags();	// Save stream state
		streamsize originalWidth = out.width();
		ostream::char_type originalFill = out.fill();

		const string val = p->value();
		for(string::const_iterator c = val.begin(); c != val.end(); ++c)
		{
		    if(isascii(*c) && isprint(*c))
		    {
			switch(*c)
			{
			    case '\\':				// Take care of \ and "
			    case '"':
			    {
				out << "\\";
				break;
			    }
			}
			out << *c;				// Print normally if printable
		    }
		    else
		    {
			switch(*c)
			{
			    case '\r':				// CR can't be written as a universal char name in Java
			    {
				out << "\\r";
				break;
			    }
			    case '\n':				// Ditto for NL
			    {
				out << "\\n";
				break;
			    }
			    default:
			    {
				unsigned char uc = *c;		// char may be signed, so make it positive
				out << "\\u";    		// Print as universal character name if non-printable
				out.flags(ios_base::hex);
				out.width(4);
				out.fill('0');
				out << static_cast<unsigned>(uc);
				break;
			    }
			}
		    }
		}

		out.fill(originalFill);				// Restore stream state
		out.width(originalWidth);
		out.flags(originalFlags);

		out << "\"";					// Closing "
		break;
	    }
	    case Builtin::KindByte:
	    {
		out << p->value() << " - 128";	// Slice byte runs from 0-255, Java byte runs from -128 - 127
		break;
	    }
	    case Builtin::KindLong:
	    {
		out << p->value() << "L";	// Need to append "L" modifier for long constants
		break;
	    }
            case Builtin::KindBool:
            case Builtin::KindShort:
            case Builtin::KindInt:
            case Builtin::KindFloat:
            case Builtin::KindDouble:
            case Builtin::KindObject:
            case Builtin::KindObjectProxy:
            case Builtin::KindLocalObject:
	    {
		out << p->value();
                break;
	    }
	}

    }
    else if(ep = EnumPtr::dynamicCast(type))
    {
	out << getAbsolute(ep->scoped(), scope) << '.' << fixKwd(p->value());
    }
    else
    {
	out << p->value();
    }
    out << ';' << eb;
    close();
}

Slice::Gen::HolderVisitor::HolderVisitor(const string& dir, const string& package) :
    JavaVisitor(dir, package)
{
}

bool
Slice::Gen::HolderVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    ClassDeclPtr decl = p->declaration();
    writeHolder(decl);

    if(!p->isLocal())
    {
        string name = fixKwd(p->name());
        string absolute = getAbsolute(p->scoped());

        if(open(absolute + "PrxHolder"))
        {
            Output& out = output();
            out << sp << nl << "public final class " << name << "PrxHolder";
            out << sb;
            out << sp << nl << "public" << nl << name << "PrxHolder()";
            out << sb;
            out << eb;
            out << sp << nl << "public" << nl << name << "PrxHolder(" << name << "Prx value)";
            out << sb;
            out << nl << "this.value = value;";
            out << eb;
            out << sp << nl << "public " << name << "Prx value;";
            out << eb;
            close();
        }
    }

    return false;
}

bool
Slice::Gen::HolderVisitor::visitStructStart(const StructPtr& p)
{
    writeHolder(p);
    return false;
}

void
Slice::Gen::HolderVisitor::visitSequence(const SequencePtr& p)
{
    writeHolder(p);
}

void
Slice::Gen::HolderVisitor::visitDictionary(const DictionaryPtr& p)
{
    writeHolder(p);
}

void
Slice::Gen::HolderVisitor::visitEnum(const EnumPtr& p)
{
    writeHolder(p);
}

void
Slice::Gen::HolderVisitor::writeHolder(const TypePtr& p)
{
    ContainedPtr contained = ContainedPtr::dynamicCast(p);
    assert(contained);
    string name = fixKwd(contained->name());
    string absolute = getAbsolute(contained->scoped());
    string holder = absolute + "Holder";

    if(open(holder))
    {
        Output& out = output();
        string typeS = typeToString(p, TypeModeIn, contained->scope());
        out << sp << nl << "public final class " << name << "Holder";
        out << sb;
        out << sp << nl << "public" << nl << name << "Holder()";
        out << sb;
        out << eb;
        out << sp << nl << "public" << nl << name << "Holder(" << typeS << " value)";
        out << sb;
        out << nl << "this.value = value;";
        out << eb;
	if(!p->isLocal())
	{
	    BuiltinPtr builtin = BuiltinPtr::dynamicCast(p);
	    if((builtin && builtin->kind() == Builtin::KindObject) || ClassDeclPtr::dynamicCast(p))
	    {
		out << sp << nl << "public class Patcher implements IceInternal.Patcher";
		out << sb;
		out << nl << "public void";
		out << nl << "patch(Ice.Object v)";
		out << sb;
		out << nl << "value = (" << typeS << ")v;";
		out << eb;

		out << sp << nl << "public String" << nl << "type()";
		out << sb;
		out << nl << "return \"" << p->typeId() << "\";";
		out << eb;
		out << eb;

		out << sp << nl << "public Patcher";
		out << nl << "getPatcher()";
		out << sb;
		out << nl << "return new Patcher();";
		out << eb;
	    }
	}
        out << sp << nl << "public " << typeS << " value;";
        out << eb;
        close();
    }
}

Slice::Gen::HelperVisitor::HelperVisitor(const string& dir, const string& package) :
    JavaVisitor(dir, package)
{
}

bool
Slice::Gen::HelperVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    if(p->isLocal())
    {
        return false;
    }

    //
    // Proxy helper
    //
    string name = fixKwd(p->name());
    string scoped = p->scoped();
    ClassList bases = p->bases();
    string scope = p->scope();
    string absolute = getAbsolute(scoped);

    if(!open(absolute + "PrxHelper"))
    {
        return false;
    }

    Output& out = output();

    //
    // A proxy helper class serves two purposes: it implements the
    // proxy interface, and provides static helper methods for use
    // by applications (e.g., checkedCast, etc.)
    //
    out << sp << nl << "public final class " << name << "PrxHelper extends Ice.ObjectPrxHelper implements " << name
        << "Prx";

    out << sb;

    OperationList ops = p->allOperations();

    OperationList::const_iterator r;
    for(r = ops.begin(); r != ops.end(); ++r)
    {
        OperationPtr op = *r;
        string opName = fixKwd(op->name());
        TypePtr ret = op->returnType();
        string retS = typeToString(ret, TypeModeReturn, scope);

        string params = getParams(op, scope);
        string args = getArgs(op);

        ExceptionList throws = op->throws();
        throws.sort();
        throws.unique();

        //
        // Write two versions of the operation - with and without a
        // context parameter
        //
        out << sp;
        out << nl << "public " << retS << nl << opName << '(' << params << ')';
        writeThrowsClause(scope, throws);
        out << sb;
        out << nl;
        if(ret)
        {
            out << "return ";
        }
        out << opName << '(' << args;
        if(!args.empty())
        {
            out << ", ";
        }
        out << "__defaultContext());";
        out << eb;

        out << sp;
        out << nl << "public " << retS << nl << opName << '(' << params;
        if(!params.empty())
        {
            out << ", ";
        }
        out << "java.util.Map __context)";
        writeThrowsClause(scope, throws);
        out << sb;
        out << nl << "int __cnt = 0;";
        out << nl << "while(true)";
        out << sb;
        out << nl << "try";
        out << sb;
	if(op->returnsData())
	{
	    out << "__checkTwowayOnly(\"" << opName << "\");";
	}
        out << nl << "Ice._ObjectDel __delBase = __getDelegate();";
        out << nl << '_' << name << "Del __del = (_" << name << "Del)__delBase;";
        out << nl;
        if(ret)
        {
            out << "return ";
        }
        out << "__del." << opName << '(' << args;
        if(!args.empty())
        {
            out << ", ";
        }
        out << "__context);";
        if(!ret)
        {
            out << nl << "return;";
        }
        out << eb;
        out << nl << "catch(IceInternal.NonRepeatable __ex)";
        out << sb;
        if(op->mode() == Operation::Idempotent || op->mode() == Operation::Nonmutating)
        {
            out << nl << "__cnt = __handleException(__ex.get(), __cnt);";
        }
        else
        {
            out << nl << "__rethrowException(__ex.get());";
        }
        out << eb;
        out << nl << "catch(Ice.LocalException __ex)";
        out << sb;
        out << nl << "__cnt = __handleException(__ex, __cnt);";
        out << eb;
        out << eb;
        out << eb;

	if(p->hasMetaData("ami") || op->hasMetaData("ami"))
	{
	    string paramsAMI = getParamsAsync(op, scope, false);
	    string argsAMI = getArgsAsync(op);
	    
	    //
	    // Write two versions of the operation - with and without a
	    // context parameter
	    //
	    out << sp;
	    out << nl << "public void" << nl << opName << "_async(" << paramsAMI << ')';
	    out << sb;
	    out << nl << opName << "_async(" << argsAMI << ", __defaultContext());";
	    out << eb;

	    out << sp;
	    out << nl << "public void" << nl << opName << "_async(" << paramsAMI << ", java.util.Map __context)";
	    out << sb;
	    out << nl << "int __cnt = 0;";
	    out << nl << "while(true)";
	    out << sb;
	    out << nl << "try";
	    out << sb;
	    if(op->returnsData())
	    {
		out << "__checkTwowayOnly(\"" << opName << "\");";
	    }
	    out << nl << "Ice._ObjectDel __delBase = __getDelegate();";
	    out << nl << '_' << name << "Del __del = (_" << name
		<< "Del)__delBase;";
	    out << nl << "__del." << opName << "_async(" << argsAMI << ", __context);";
            out << nl << "return;";
	    out << eb;
	    out << nl << "catch(Ice.LocalException __ex)";
	    out << sb;
	    out << nl << "__cnt = __handleException(__ex, __cnt);";
	    out << eb;
	    out << eb;
	    out << eb;
	}
    }

    out << sp << nl << "public static " << name << "Prx" << nl << "checkedCast(Ice.ObjectPrx b)";
    out << sb;
    out << nl << name << "Prx d = null;";
    out << nl << "if(b != null)";
    out << sb;
    out << nl << "try";
    out << sb;
    out << nl << "d = (" << name << "Prx)b;";
    out << eb;
    out << nl << "catch(ClassCastException ex)";
    out << sb;
    out << nl << "if(b.ice_isA(\"" << scoped << "\"))";
    out << sb;
    out << nl << name << "PrxHelper h = new " << name << "PrxHelper();";
    out << nl << "h.__copyFrom(b);";
    out << nl << "d = h;";
    out << eb;
    out << eb;
    out << eb;
    out << nl << "return d;";
    out << eb;

    out << sp << nl << "public static " << name << "Prx" << nl << "checkedCast(Ice.ObjectPrx b, String f)";
    out << sb;
    out << nl << name << "Prx d = null;";
    out << nl << "if(b != null)";
    out << sb;
    out << nl << "Ice.ObjectPrx bb = b.ice_appendFacet(f);";
    out << nl << "try";
    out << sb;
    out << nl << "if(bb.ice_isA(\"" << scoped << "\"))";
    out << sb;
    out << nl << name << "PrxHelper h = new " << name << "PrxHelper();";
    out << nl << "h.__copyFrom(bb);";
    out << nl << "d = h;";
    out << eb;
    out << eb;
    out << nl << "catch(Ice.FacetNotExistException ex)";
    out << sb;
    out << eb;
    out << eb;
    out << nl << "return d;";
    out << eb;

    out << sp << nl << "public static " << name << "Prx" << nl << "uncheckedCast(Ice.ObjectPrx b)";
    out << sb;
    out << nl << name << "Prx d = null;";
    out << nl << "if(b != null)";
    out << sb;
    out << nl << name << "PrxHelper h = new " << name << "PrxHelper();";
    out << nl << "h.__copyFrom(b);";
    out << nl << "d = h;";
    out << eb;
    out << nl << "return d;";
    out << eb;

    out << sp << nl << "public static " << name << "Prx" << nl << "uncheckedCast(Ice.ObjectPrx b, String f)";
    out << sb;
    out << nl << name << "Prx d = null;";
    out << nl << "if(b != null)";
    out << sb;
    out << nl << "Ice.ObjectPrx bb = b.ice_appendFacet(f);";
    out << nl << name << "PrxHelper h = new " << name << "PrxHelper();";
    out << nl << "h.__copyFrom(bb);";
    out << nl << "d = h;";
    out << eb;
    out << nl << "return d;";
    out << eb;

    out << sp << nl << "protected Ice._ObjectDelM" << nl << "__createDelegateM()";
    out << sb;
    out << nl << "return new _" << name << "DelM();";
    out << eb;

    out << sp << nl << "protected Ice._ObjectDelD" << nl << "__createDelegateD()";
    out << sb;
    out << nl << "return new _" << name << "DelD();";
    out << eb;

    out << sp << nl << "public static void" << nl << "__write(IceInternal.BasicStream __os, " << name << "Prx v)";
    out << sb;
    out << nl << "__os.writeProxy(v);";
    out << eb;

    out << sp << nl << "public static " << name << "Prx" << nl << "__read(IceInternal.BasicStream __is)";
    out << sb;
    out << nl << "Ice.ObjectPrx proxy = __is.readProxy();";
    out << nl << "if(proxy != null)";
    out << sb;
    out << nl << name << "PrxHelper result = new " << name << "PrxHelper();";
    out << nl << "result.__copyFrom(proxy);";
    out << nl << "return result;";
    out << eb;
    out << nl << "return null;";
    out << eb;

    out << eb;
    close();

    return false;
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

    string name = fixKwd(p->name());
    string absolute = getAbsolute(p->scoped());
    string helper = absolute + "Helper";
    string scope = p->scope();
    string typeS = typeToString(p, TypeModeIn, scope);

    if(open(helper))
    {
        Output& out = output();
        int iter;

        out << sp << nl << "public final class " << name << "Helper";
        out << sb;

        out << nl << "public static void" << nl << "write(IceInternal.BasicStream __os, " << typeS << " __v)";
        out << sb;
        iter = 0;
        writeSequenceMarshalUnmarshalCode(out, scope, p, "__v", true, iter, false);
        out << eb;

        out << sp << nl << "public static " << typeS << nl << "read(IceInternal.BasicStream __is)";
        out << sb;
        out << nl << typeS << " __v;";
        iter = 0;
	writeSequenceMarshalUnmarshalCode(out, scope, p, "__v", false, iter, false);
        out << nl << "return __v;";
        out << eb;

        out << eb;
        close();
    }
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

    string absolute = getAbsolute(p->scoped());
    string helper = absolute + "Helper";

    if(open(helper))
    {
        Output& out = output();
        string name = fixKwd(p->name());
        string scope = p->scope();
        string keyS = typeToString(key, TypeModeIn, scope);
        string valueS = typeToString(value, TypeModeIn, scope);
        int iter;
        int i;

        out << sp << nl << "public final class " << name << "Helper";
        out << sb;

        out << nl << "public static void" << nl << "write(IceInternal.BasicStream __os, " << "java.util.Map __v)";
        out << sb;
        out << nl << "if(__v == null)";
        out << sb;
        out << nl << "__os.writeSize(0);";
        out << eb;
        out << nl << "else";
        out << sb;
        out << nl << "__os.writeSize(__v.size());";
        out << nl << "java.util.Iterator __i = __v.entrySet().iterator();";
        out << nl << "while(__i.hasNext())";
        out << sb;
        out << nl << "java.util.Map.Entry __e = (java.util.Map.Entry)" << "__i.next();";
        iter = 0;
        for(i = 0; i < 2; i++)
        {
            string val;
            string arg;
            TypePtr type;
            if(i == 0)
            {
                arg = "__e.getKey()";
                type = key;
            }
            else
            {
                arg = "__e.getValue()";
                type = value;
            }

            BuiltinPtr b = BuiltinPtr::dynamicCast(type);
            if(b)
            {
                switch(b->kind())
                {
                    case Builtin::KindByte:
                    {
                        val = "((java.lang.Byte)" + arg + ").byteValue()";
                        break;
                    }
                    case Builtin::KindBool:
                    {
                        val = "((java.lang.Boolean)" + arg + ").booleanValue()";
                        break;
                    }
                    case Builtin::KindShort:
                    {
                        val = "((java.lang.Short)" + arg + ").shortValue()";
                        break;
                    }
                    case Builtin::KindInt:
                    {
                        val = "((java.lang.Integer)" + arg + ").intValue()";
                        break;
                    }
                    case Builtin::KindLong:
                    {
                        val = "((java.lang.Long)" + arg + ").longValue()";
                        break;
                    }
                    case Builtin::KindFloat:
                    {
                        val = "((java.lang.Float)" + arg + ").floatValue()";
                        break;
                    }
                    case Builtin::KindDouble:
                    {
                        val = "((java.lang.Double)" + arg + ").doubleValue()";
                        break;
                    }
                    case Builtin::KindString:
                    case Builtin::KindObject:
                    case Builtin::KindObjectProxy:
                    {
                        break;
                    }
                    case Builtin::KindLocalObject:
                    {
                        assert(false);
                        break;
                    }
                }
            }

            if(val.empty())
            {
                val = "((" + typeToString(type, TypeModeIn, scope) + ')' + arg + ')';
            }
            writeMarshalUnmarshalCode(out, scope, type, val, true, iter, false);
        }
        out << eb;
        out << eb;
        out << eb;

	BuiltinPtr builtin = BuiltinPtr::dynamicCast(value);
	if((builtin && builtin->kind() == Builtin::KindObject) || ClassDeclPtr::dynamicCast(value))
	{
	    //
	    // The dictionary uses class values.
	    //
	    out << sp << nl << "private static class Patcher implements IceInternal.Patcher";
	    out << sb;
	    string keyTypeS = keyS;
            BuiltinPtr b = BuiltinPtr::dynamicCast(key);
	    if(b)
	    {
		switch(b->kind())
		{
                    case Builtin::KindByte:
                    {
                        keyTypeS = "java.lang.Byte";
                        break;
                    }
                    case Builtin::KindBool:
                    {
                        keyTypeS = "java.lang.Boolean";
                        break;
                    }
                    case Builtin::KindShort:
                    {
                        keyTypeS = "java.lang.Short";
                        break;
                    }
                    case Builtin::KindInt:
                    {
                        keyTypeS = "java.lang.Integer";
                        break;
                    }
                    case Builtin::KindLong:
                    {
                        keyTypeS = "java.lang.Long";
                        break;
                    }
                    case Builtin::KindFloat:
                    {
                        keyTypeS = "java.lang.Float";
                        break;
                    }
                    case Builtin::KindDouble:
                    {
                        keyTypeS = "java.lang.Double";
                        break;
                    }
		    default:
		    {
			break;	// Do nothing
		    }
		}
	    }
	    out << sp << nl << "Patcher(java.util.Map m, " << keyTypeS << " key)";
	    out << sb;
	    out << nl << "__m = m;";
	    out << nl << "__key = key;";
	    out << eb;

	    out << sp << nl << "public void" << nl << "patch(Ice.Object v)";
	    out << sb;
	    out << nl << valueS << " _v = (" << valueS << ")v;";
	    out << nl << "__m.put(__key, v);";
	    out << eb;

	    out << sp << nl << "public String" << nl << "type()";
	    out << sb;
	    out << nl << "return \"" << value->typeId() << "\";";
	    out << eb;

	    out << sp << nl << "private java.util.Map __m;";
	    out << nl << "private " << keyTypeS << " __key;";
	    out << eb;
	}

        out << sp << nl << "public static java.util.Map" << nl << "read(IceInternal.BasicStream __is)";
        out << sb;
        out << nl << "int __sz = __is.readSize();";
        out << nl << "java.util.Map __r = new java.util.HashMap(__sz);";
        out << nl << "for(int __i = 0; __i < __sz; __i++)";
        out << sb;
        iter = 0;
        for(i = 0; i < 2; i++)
        {
            string arg;
            TypePtr type;
            if(i == 0)
            {
                arg = "__key";
                type = key;
            }
            else
            {
                arg = "__value";
                type = value;
            }

            BuiltinPtr b = BuiltinPtr::dynamicCast(type);
            if(b)
            {
                switch(b->kind())
                {
                    case Builtin::KindByte:
                    {
                        out << nl << "java.lang.Byte " << arg << " = new java.lang.Byte(__is.readByte());";
                        break;
                    }
                    case Builtin::KindBool:
                    {
                        out << nl << "java.lang.Boolean " << arg << " = new java.lang.Boolean(__is.readBool());";
                        break;
                    }
                    case Builtin::KindShort:
                    {
                        out << nl << "java.lang.Short " << arg << " = new java.lang.Short(__is.readShort());";
                        break;
                    }
                    case Builtin::KindInt:
                    {
                        out << nl << "java.lang.Integer " << arg << " = new java.lang.Integer(__is.readInt());";
                        break;
                    }
                    case Builtin::KindLong:
                    {
                        out << nl << "java.lang.Long " << arg << " = new java.lang.Long(__is.readLong());";
                        break;
                    }
                    case Builtin::KindFloat:
                    {
                        out << nl << "java.lang.Float " << arg << " = new java.lang.Float(__is.readFloat());";
                        break;
                    }
                    case Builtin::KindDouble:
                    {
                        out << nl << "java.lang.Double " << arg << " = new java.lang.Double(__is.readDouble());";
                        break;
                    }
                    case Builtin::KindString:
                    {
                        out << nl << "java.lang.String " << arg << " = __is.readString();";
                        break;
                    }
                    case Builtin::KindObject:
                    {
                        out << nl << "__is.readObject(new Patcher(__r, __key));";
                        break;
                    }
                    case Builtin::KindObjectProxy:
                    {
                        out << nl << "Ice.ObjectPrx " << arg << " = __is.readProxy();";
                        break;
                    }
                    case Builtin::KindLocalObject:
                    {
                        assert(false);
                        break;
                    }
                }
            }
            else
            {
                string s = typeToString(type, TypeModeIn, scope);
		BuiltinPtr builtin2 = BuiltinPtr::dynamicCast(type);
		if((builtin2 && builtin2->kind() == Builtin::KindObject) || ClassDeclPtr::dynamicCast(type))
		{
		    writeMarshalUnmarshalCode(out, scope, type, arg, false, iter, false, list<string>(),
					      "new Patcher(__r, __key)");
		}
		else
		{
		    out << nl << s << ' ' << arg << ';';
		    writeMarshalUnmarshalCode(out, scope, type, arg, false, iter, false);
		}
            }
        }
	if(!(builtin && builtin->kind() == Builtin::KindObject) && !ClassDeclPtr::dynamicCast(value))
	{
	    out << nl << "__r.put(__key, __value);";
	}
        out << eb;
        out << nl << "return __r;";
        out << eb;

        out << eb;
        close();
    }
}

Slice::Gen::ProxyVisitor::ProxyVisitor(const string& dir, const string& package) :
    JavaVisitor(dir, package)
{
}

bool
Slice::Gen::ProxyVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    if(p->isLocal())
    {
        return false;
    }

    string name = fixKwd(p->name());
    string scoped = p->scoped();
    ClassList bases = p->bases();
    string scope = p->scope();
    string absolute = getAbsolute(scoped);

    if(!open(absolute + "Prx"))
    {
        return false;
    }

    Output& out = output();

    //
    // Generate a Java interface as the user-visible type
    //
    out << sp << nl << "public interface " << name << "Prx extends ";
    if(bases.empty())
    {
        out << "Ice.ObjectPrx";
    }
    else
    {
        out.useCurrentPosAsIndent();
        ClassList::const_iterator q = bases.begin();
        while(q != bases.end())
        {
            out << getAbsolute((*q)->scoped(), scope) << "Prx";
            if(++q != bases.end())
            {
                out << ',' << nl;
            }
        }
        out.restoreIndent();
    }

    out << sb;

    return true;
}

void
Slice::Gen::ProxyVisitor::visitClassDefEnd(const ClassDefPtr& p)
{
    Output& out = output();
    out << eb;
    close();
}

void
Slice::Gen::ProxyVisitor::visitOperation(const OperationPtr& p)
{
    string name = fixKwd(p->name());
    ContainerPtr container = p->container();
    ClassDefPtr cl = ClassDefPtr::dynamicCast(container);
    string scope = cl->scope();

    Output& out = output();

    TypePtr ret = p->returnType();
    string retS = typeToString(ret, TypeModeReturn, scope);
    string params = getParams(p, scope);
    ExceptionList throws = p->throws();
    throws.sort();
    throws.unique();

    //
    // Write two versions of the operation - with and without a
    // context parameter.
    //
    out << sp;
    out << nl << "public " << retS << ' ' << name << '(' << params << ')';
    writeThrowsClause(scope, throws);
    out << ';';
    out << nl << "public " << retS << ' ' << name << '(' << params;
    if(!params.empty())
    {
        out << ", ";
    }
    out << "java.util.Map __context)";
    writeThrowsClause(scope, throws);
    out << ';';

    if(cl->hasMetaData("ami") || p->hasMetaData("ami"))
    {
	string paramsAMI = getParamsAsync(p, scope, false);

	//
	// Write two versions of the operation - with and without a
	// context parameter.
	//
	out << sp;
	out << nl << "public void " << name << "_async(" << paramsAMI << ");";
	out << nl << "public void " << name << "_async(" << paramsAMI << ", java.util.Map __context);";
    }
}

Slice::Gen::DelegateVisitor::DelegateVisitor(const string& dir, const string& package) :
    JavaVisitor(dir, package)
{
}

bool
Slice::Gen::DelegateVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    if(p->isLocal())
    {
        return false;
    }

    string name = fixKwd(p->name());
    string scoped = p->scoped();
    ClassList bases = p->bases();
    string scope = p->scope();
    string absolute = getAbsolute(scoped, "", "_", "Del");

    if(!open(absolute))
    {
        return false;
    }

    Output& out = output();

    out << sp << nl << "public interface _" << name << "Del extends ";
    if(bases.empty())
    {
        out << "Ice._ObjectDel";
    }
    else
    {
        out.useCurrentPosAsIndent();
        ClassList::const_iterator q = bases.begin();
        while(q != bases.end())
        {
            out << getAbsolute((*q)->scoped(), scope, "_", "Del");
            if(++q != bases.end())
            {
                out << ',' << nl;
            }
        }
        out.restoreIndent();
    }

    out << sb;

    OperationList ops = p->operations();

    OperationList::const_iterator r;
    for(r = ops.begin(); r != ops.end(); ++r)
    {
        OperationPtr op = *r;
        string opName = fixKwd(op->name());
        TypePtr ret = op->returnType();
        string retS = typeToString(ret, TypeModeReturn, scope);

        string params = getParams(op, scope);

        ExceptionList throws = op->throws();
        throws.sort();
        throws.unique();

        out << sp;
        out << nl << retS << ' ' << opName << '(' << params;
        if(!params.empty())
        {
            out << ", ";
        }
        out << "java.util.Map __context)";
        writeDelegateThrowsClause(scope, throws);
        out << ';';

	if(p->hasMetaData("ami") || op->hasMetaData("ami"))
	{
	    string paramsAMI = getParamsAsync(op, scope, false);
	    
	    out << sp;
	    out << nl << "void " << opName << "_async(" << paramsAMI << ", java.util.Map __context);";
	}
    }

    out << eb;
    close();

    return false;
}

Slice::Gen::DelegateMVisitor::DelegateMVisitor(const string& dir, const string& package) :
    JavaVisitor(dir, package)
{
}

bool
Slice::Gen::DelegateMVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    if(p->isLocal())
    {
        return false;
    }

    string name = fixKwd(p->name());
    string scoped = p->scoped();
    ClassList bases = p->bases();
    string scope = p->scope();
    string absolute = getAbsolute(scoped, "", "_", "DelM");

    if(!open(absolute))
    {
        return false;
    }

    Output& out = output();

    out << sp << nl << "public final class _" << name << "DelM extends Ice._ObjectDelM implements _" << name << "Del";
    out << sb;

    OperationList ops = p->allOperations();

    OperationList::const_iterator r;
    for(r = ops.begin(); r != ops.end(); ++r)
    {
        OperationPtr op = *r;
        string opName = fixKwd(op->name());
        TypePtr ret = op->returnType();
        string retS = typeToString(ret, TypeModeReturn, scope);
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

        string params = getParams(op, scope);

        out << sp;
        out << nl << "public " << retS << nl << opName << '(' << params;
        if(!params.empty())
        {
            out << ", ";
        }
        out << "java.util.Map __context)";
        writeDelegateThrowsClause(scope, throws);
        out << sb;

        out << nl << "IceInternal.Outgoing __out = getOutgoing(\"" << op->name() << "\", " << sliceModeToIceMode(op)
	    << ", __context);";
        out << nl << "try";
        out << sb;
        if(!inParams.empty())
        {
            out << nl << "IceInternal.BasicStream __os = __out.os();";
        }
        if(!outParams.empty() || ret || !throws.empty())
        {
            out << nl << "IceInternal.BasicStream __is = __out.is();";
        }
        iter = 0;
        for(q = inParams.begin(); q != inParams.end(); ++q)
        {
            writeMarshalUnmarshalCode(out, scope, q->first, fixKwd(q->second), true, iter);
        }
	if(op->sendsClasses())
	{
	    out << nl << "__os.writePendingObjects();";
	}
        out << nl << "if(!__out.invoke())";
        out << sb;
        if(!throws.empty())
        {
            //
            // The try/catch block is necessary because throwException()
            // can raise UserException.
            //
            out << nl << "try";
            out << sb;
            out << nl << "__is.throwException();";
            out << eb;
            for(ExceptionList::const_iterator t = throws.begin(); t != throws.end(); ++t)
            {
                out << nl << "catch(" << getAbsolute((*t)->scoped(), scope) << " __ex)";
                out << sb;
                out << nl << "throw __ex;";
                out << eb;
            }
            out << nl << "catch(Ice.UserException __ex)";
            out << sb;
            out << eb;
        }
        out << nl << "throw new Ice.UnknownUserException();";
        out << eb;
        if(!outParams.empty() || ret)
        {
            out << nl << "try";
            out << sb;
        }
        for(q = outParams.begin(); q != outParams.end(); ++q)
        {
            writeMarshalUnmarshalCode(out, scope, q->first, fixKwd(q->second), false, iter, true);
        }
        if(ret)
        {
	    BuiltinPtr builtin = BuiltinPtr::dynamicCast(ret);
	    if((builtin && builtin->kind() == Builtin::KindObject) || ClassDeclPtr::dynamicCast(ret))
	    {
		out << nl << retS << "Holder __ret = new " << retS << "Holder();";
		out << nl << "__is.readObject(__ret.getPatcher());";
	    }
	    else
	    {
		out << nl << retS << " __ret;";
		writeMarshalUnmarshalCode(out, scope, ret, "__ret", false, iter);
	    }
        }
	if(op->returnsClasses())
	{
	    out << nl << "__is.readPendingObjects();";
	}
	if(ret)
	{
	    BuiltinPtr builtin = BuiltinPtr::dynamicCast(ret);
	    if((builtin && builtin->kind() == Builtin::KindObject) || ClassDeclPtr::dynamicCast(ret))
	    {
		out << nl << "return __ret.value;";
	    }
	    else
	    {
		out << nl << "return __ret;";
	    }
	}
        if(!outParams.empty() || ret)
        {
            out << eb;
            out << nl << "catch(Ice.LocalException __ex)";
            out << sb;
            out << nl << "throw new IceInternal.NonRepeatable(__ex);";
            out << eb;
        }
        out << eb;
        out << nl << "finally";
        out << sb;
        out << nl << "reclaimOutgoing(__out);";
        out << eb;
        out << eb;

	if(p->hasMetaData("ami") || op->hasMetaData("ami"))
	{
	    string paramsAMI = getParamsAsync(op, scope, false);
	    
	    out << sp;
	    out << nl << "public void" << nl << opName << "_async(" << paramsAMI << ", java.util.Map __context)";
	    out << sb;
	    
	    out << nl << "__cb.__setup(__connection, __reference, \"" << op->name() << "\", " << sliceModeToIceMode(op)
		<< ", __context);";
	    if(!inParams.empty())
	    {
		out << nl << "IceInternal.BasicStream __os = __cb.__os();";
	    }
	    iter = 0;
	    for(q = inParams.begin(); q != inParams.end(); ++q)
	    {
		writeMarshalUnmarshalCode(out, scope, q->first, fixKwd(q->second), true, iter);
	    }
	    if(op->sendsClasses())
	    {
		out << nl << "__os.writePendingObjects();";
	    }
	    out << nl << "__cb.__invoke();";
	    out << eb;
	}
    }

    out << eb;
    close();

    return false;
}

Slice::Gen::DelegateDVisitor::DelegateDVisitor(const string& dir, const string& package) :
    JavaVisitor(dir, package)
{
}

bool
Slice::Gen::DelegateDVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    if(p->isLocal())
    {
        return false;
    }

    string name = fixKwd(p->name());
    string scoped = p->scoped();
    ClassList bases = p->bases();
    string scope = p->scope();
    string absolute = getAbsolute(scoped, "", "_", "DelD");

    if(!open(absolute))
    {
        return false;
    }

    Output& out = output();

    out << sp << nl << "public final class _" << name << "DelD extends Ice._ObjectDelD implements _" << name << "Del";
    out << sb;

    OperationList ops = p->allOperations();

    OperationList::const_iterator r;
    for(r = ops.begin(); r != ops.end(); ++r)
    {
        OperationPtr op = *r;
        string opName = fixKwd(op->name());
        TypePtr ret = op->returnType();
        string retS = typeToString(ret, TypeModeReturn, scope);

        ExceptionList throws = op->throws();
        throws.sort();
        throws.unique();
        throws.erase(remove_if(throws.begin(), throws.end(), IceUtil::constMemFun(&Exception::isLocal)), throws.end());

        string params = getParams(op, scope);
        string args = getArgs(op);

	out << sp;
        out << nl << "public " << retS << nl << opName << '(' << params;
        if(!params.empty())
        {
            out << ", ";
        }
        out << "java.util.Map __context)";
        writeDelegateThrowsClause(scope, throws);
        out << sb;
	if(p->hasMetaData("amd") || op->hasMetaData("amd"))
	{
	    out << nl << "throw new Ice.CollocationOptimizationException();";
	}
	else
	{
	    list<string> metaData = op->getMetaData();
	    out << nl << "Ice.Current __current = new Ice.Current();";
	    out << nl << "__initCurrent(__current, \"" << op->name() << "\", " << sliceModeToIceMode(op)
		<< ", __context);";
	    out << nl << "while(true)";
	    out << sb;
	    out << nl << "IceInternal.Direct __direct = new IceInternal.Direct(__current);";
	    out << nl << "try";
	    out << sb;
	    out << nl << name << " __servant = null;";
	    out << nl << "try";
	    out << sb;
	    out << nl << "__servant = (" << name << ")__direct.facetServant();";
	    out << eb;
	    out << nl << "catch(ClassCastException __ex)";
	    out << sb;
	    out << nl << "Ice.OperationNotExistException __opEx = new Ice.OperationNotExistException();";
	    out << nl << "__opEx.id = __current.id;";
	    out << nl << "__opEx.facet = __current.facet;";
	    out << nl << "__opEx.operation = __current.operation;";
	    out << nl << "throw __opEx;";
	    out << eb;
            out << nl << "try";
            out << sb;
	    out << nl;
	    if(ret)
	    {
		out << "return ";
	    }
	    out << "__servant." << opName << '(' << args;
	    if(!args.empty())
	    {
		out << ", ";
	    }
	    out << "__current);";
	    if(!ret)
	    {
		out << nl << "return;";
	    }
            out << eb;
            out << nl << "catch(Ice.LocalException __ex)";
            out << sb;
            out << nl << "throw new IceInternal.NonRepeatable(__ex);";
	    out << eb;
	    out << eb;
	    out << nl << "finally";
	    out << sb;
	    out << nl << "__direct.destroy();";
	    out << eb;
	    out << eb;
	}
        out << eb;

	if(p->hasMetaData("ami") || op->hasMetaData("ami"))
	{
	    string paramsAMI = getParamsAsync(op, scope, false);
	    
	    out << sp;
	    out << nl << "public void" << nl << opName << "_async(" << paramsAMI << ", java.util.Map __context)";
	    out << sb;
	    out << nl << "throw new Ice.CollocationOptimizationException();";
	    out << eb;
	}
    }

    out << eb;
    close();

    return false;
}

Slice::Gen::DispatcherVisitor::DispatcherVisitor(const string& dir, const string& package) :
    JavaVisitor(dir, package)
{
}

bool
Slice::Gen::DispatcherVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    if(p->isLocal() || !p->isInterface())
    {
        return false;
    }

    string name = fixKwd(p->name());
    string scoped = p->scoped();
    ClassList bases = p->bases();
    string scope = p->scope();
    string absolute = getAbsolute(scoped, "", "_", "Disp");

    if(!open(absolute))
    {
        return false;
    }

    Output& out = output();

    out << sp << nl << "public abstract class _" << name << "Disp extends Ice.ObjectImpl implements " << name;
    out << sb;

    out << sp << nl << "protected void" << nl << "ice_copyStateFrom(Ice.Object __obj)";
    out.inc();
    out << nl << "throws java.lang.CloneNotSupportedException";
    out.dec();
    out << sb;
    out << nl << "throw new java.lang.CloneNotSupportedException();";
    out << eb;

    writeDispatch(out, p);

    out << eb;
    close();

    return false;
}

Slice::Gen::BaseImplVisitor::BaseImplVisitor(const string& dir, const string& package) :
    JavaVisitor(dir, package)
{
}

void
Slice::Gen::BaseImplVisitor::writeDecl(Output& out, const string& scope, const string& name, const TypePtr& type)
{
    out << nl << typeToString(type, TypeModeIn, scope) << ' ' << name;

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if(builtin)
    {
        switch(builtin->kind())
        {
            case Builtin::KindBool:
            {
                out << " = false";
                break;
            }
            case Builtin::KindByte:
            {
                out << " = (byte)0";
                break;
            }
            case Builtin::KindShort:
            {
                out << " = (short)0";
                break;
            }
            case Builtin::KindInt:
            case Builtin::KindLong:
            {
                out << " = 0";
                break;
            }
            case Builtin::KindFloat:
            {
                out << " = (float)0.0";
                break;
            }
            case Builtin::KindDouble:
            {
                out << " = 0.0";
                break;
            }
            case Builtin::KindString:
            {
                out << " = \"\"";
                break;
            }
            case Builtin::KindObject:
            case Builtin::KindObjectProxy:
            case Builtin::KindLocalObject:
            {
                out << " = null";
                break;
            }
        }
    }
    else
    {
        EnumPtr en = EnumPtr::dynamicCast(type);
        if(en)
        {
            EnumeratorList enumerators = en->getEnumerators();
            out << " = " << getAbsolute(en->scoped()) << '.' << fixKwd(enumerators.front()->name());
        }
        else
        {
            out << " = null";
        }
    }

    out << ';';
}

void
Slice::Gen::BaseImplVisitor::writeReturn(Output& out, const TypePtr& type)
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
            {
                out << nl << "return (byte)0;";
                break;
            }
            case Builtin::KindShort:
            {
                out << nl << "return (short)0;";
                break;
            }
            case Builtin::KindInt:
            case Builtin::KindLong:
            {
                out << nl << "return 0;";
                break;
            }
            case Builtin::KindFloat:
            {
                out << nl << "return (float)0.0;";
                break;
            }
            case Builtin::KindDouble:
            {
                out << nl << "return 0.0;";
                break;
            }
            case Builtin::KindString:
            case Builtin::KindObject:
            case Builtin::KindObjectProxy:
            case Builtin::KindLocalObject:
            {
                out << nl << "return null;";
                break;
            }
        }
        return;
    }

    out << nl << "return null;";
}

void
Slice::Gen::BaseImplVisitor::writeOperation(Output& out, const string& scope, const OperationPtr& op, bool local)
{
    string opName = fixKwd(op->name());

    TypePtr ret = op->returnType();
    string retS = typeToString(ret, TypeModeReturn, scope);
    string params = getParams(op, scope);

    ContainerPtr container = op->container();
    ClassDefPtr cl = ClassDefPtr::dynamicCast(container);

    if(!local && (cl->hasMetaData("amd") || op->hasMetaData("amd")))
    {
        ParamDeclList paramList = op->parameters();
        ParamDeclList::const_iterator q;

        out << sp << nl << "public void" << nl << opName << "_async(" << getParamsAsync(op, scope, true)
            << ", Ice.Current __current)";

        ExceptionList throws = op->throws();
        throws.sort();
        throws.unique();
        writeThrowsClause(scope, throws);

        out << sb;

        string result = "r";
        for(q = paramList.begin(); q != paramList.end(); ++q)
        {
            if((*q)->name() == result)
            {
                result = "_" + result;
                break;
            }
        }
        if(ret)
        {
            writeDecl(out, scope, result, ret);
        }
        for(q = paramList.begin(); q != paramList.end(); ++q)
        {
            if((*q)->isOutParam())
            {
                writeDecl(out, scope, fixKwd((*q)->name()), (*q)->type());
            }
        }

        out << nl << "__cb.ice_response(";
        if(ret)
        {
            out << result;
        }
        for(q = paramList.begin(); q != paramList.end(); ++q)
        {
            if((*q)->isOutParam())
            {
                if(ret || q != paramList.begin())
                {
                    out << ", ";
                }
                out << fixKwd((*q)->name());
            }
        }
        out << ");";

        out << eb;
    }
    else
    {
        out << sp << nl << "public " << retS << nl << opName << '(' << params;
        if(!local)
        {
            if(!params.empty())
            {
                out << ", ";
            }
            out << "Ice.Current __current";
        }
        out << ')';

        ExceptionList throws = op->throws();
        throws.sort();
        throws.unique();
        writeThrowsClause(scope, throws);

        out << sb;

        //
        // Return value
        //
        if(ret)
        {
            writeReturn(out, ret);
        }

        out << eb;
    }
}

Slice::Gen::ImplVisitor::ImplVisitor(const string& dir, const string& package) :
    BaseImplVisitor(dir, package)
{
}

bool
Slice::Gen::ImplVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    if(!p->isAbstract())
    {
        return false;
    }

    string name = fixKwd(p->name());
    string scoped = p->scoped();
    ClassList bases = p->bases();
    string scope = p->scope();
    string absolute = getAbsolute(scoped, "", "", "I");

    if(!open(absolute))
    {
        return false;
    }

    Output& out = output();

    out << sp << nl << "public final class " << name << 'I';
    if(p->isInterface())
    {
        if(p->isLocal())
        {
            out << " extends Ice.LocalObjectImpl implements " << name;
        }
        else
        {
            out << " extends _" << name << "Disp";
        }
    }
    else
    {
        out << " extends " << name;
    }
    out << sb;

    out << nl << "public" << nl << name << "I()";
    out << sb;
    out << eb;

    OperationList ops = p->allOperations();

    OperationList::const_iterator r;
    for(r = ops.begin(); r != ops.end(); ++r)
    {
        writeOperation(out, scope, *r, p->isLocal());
    }

    out << eb;
    close();

    return false;
}

Slice::Gen::ImplTieVisitor::ImplTieVisitor(const string& dir, const string& package) :
    BaseImplVisitor(dir, package)
{
}

bool
Slice::Gen::ImplTieVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    if(!p->isAbstract())
    {
        return false;
    }

    string name = fixKwd(p->name());
    string scoped = p->scoped();
    ClassList bases = p->bases();
    string scope = p->scope();
    string absolute = getAbsolute(scoped, "", "", "I");

    if(!open(absolute))
    {
        return false;
    }

    Output& out = output();

    //
    // Use implementation inheritance in the following situations:
    //
    // * if a class extends another class
    // * if a class implements a single interface
    // * if an interface extends only one interface
    //
    bool inheritImpl = (!p->isInterface() && !bases.empty() && !bases.front()->isInterface()) || (bases.size() == 1);

    out << sp << nl << "public class " << name << 'I';
    if(inheritImpl)
    {
        out << " extends " << fixKwd(bases.front()->name()) << 'I';
    }
    out << " implements " << '_' << name << "Operations";
    out << sb;

    out << nl << "public" << nl << name << "I()";
    out << sb;
    out << eb;

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
            out << sp;
            out << nl << "/*";
            out << nl << " * Implemented by " << fixKwd(bases.front()->name()) << 'I';
            out << nl << " *";
            writeOperation(out, scope, *r, p->isLocal());
            out << sp;
            out << nl << "*/";
        }
        else
        {
            writeOperation(out, scope, *r, p->isLocal());
        }
    }

    out << eb;
    close();

    return false;
}

Slice::Gen::AsyncVisitor::AsyncVisitor(const string& dir, const string& package) :
    JavaVisitor(dir, package)
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

    string name = fixKwd(p->name());
    string classScope = fixKwd(cl->scope());
    
    if(cl->hasMetaData("ami") || p->hasMetaData("ami"))
    {
	string classNameAMI = "AMI_" + fixKwd(cl->name());
	string classScopedAMI = classScope + classNameAMI;
	string absoluteAMI = getAbsolute(classScopedAMI);

	if(!open(absoluteAMI + '_' + name))
	{
	    return;
	}
	
	Output& out = output();

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

        TypeStringList::const_iterator q;
        int iter;

	string paramsAMI = getParamsAsyncCB(p, classScope);
	string argsAMI = getArgsAsyncCB(p);

	out << sp << nl << "public abstract class " << classNameAMI << '_' << name
	    << " extends IceInternal.OutgoingAsync";
	out << sb;
	out << sp << nl << "public abstract void ice_response(" << paramsAMI << ");";
	out << sp << nl << "public abstract void ice_exception(Ice.LocalException ex);";
	if(!throws.empty())
	{
	    out << sp << nl << "public abstract void ice_exception(Ice.UserException ex);";
	}
	out << sp << nl << "protected final void __response(boolean __ok)";
	out << sb;
        for(q = outParams.begin(); q != outParams.end(); ++q)
        {
            string typeS = typeToString(q->first, TypeModeIn, classScope);
	    BuiltinPtr builtin = BuiltinPtr::dynamicCast(q->first);
	    if((builtin && builtin->kind() == Builtin::KindObject) || ClassDeclPtr::dynamicCast(q->first))
	    {
		out << nl << typeS << "Holder " << fixKwd(q->second) << " = new " << typeS << "Holder();";
	    }
	    else
	    {
		out << nl << typeS << ' ' << fixKwd(q->second) << ";";
	    }
        }
        if(ret)
        {
	    string retS = typeToString(ret, TypeModeIn, classScope);
	    BuiltinPtr builtin = BuiltinPtr::dynamicCast(ret);
	    if((builtin && builtin->kind() == Builtin::KindObject) || ClassDeclPtr::dynamicCast(ret))
	    {
		out << nl << retS << "Holder __ret = new " << retS << "Holder();";
	    }
	    else
	    {
		out << nl << retS << " __ret;";
	    }
        }
	out << nl << "try";
	out << sb;
	if(ret || !outParams.empty() || !throws.empty())
	{
	    out << nl << "IceInternal.BasicStream __is = this.__is();";
	}
	out << nl << "if(!__ok)";
        out << sb;
        if(!throws.empty())
        {
            //
            // The try/catch block is necessary because throwException()
            // can raise UserException.
            //
            out << nl << "try";
            out << sb;
            out << nl << "__is.throwException();";
            out << eb;
            for(ExceptionList::const_iterator r = throws.begin(); r != throws.end(); ++r)
            {
                out << nl << "catch(" << getAbsolute((*r)->scoped(), classScope) << " __ex)";
                out << sb;
                out << nl << "throw __ex;";
                out << eb;
            }
            out << nl << "catch(Ice.UserException __ex)";
            out << sb;
            out << eb;
        }
        out << nl << "throw new Ice.UnknownUserException();";
        out << eb;
        for(q = outParams.begin(); q != outParams.end(); ++q)
        {
	    BuiltinPtr builtin = BuiltinPtr::dynamicCast(q->first);
	    if((builtin && builtin->kind() == Builtin::KindObject) || ClassDeclPtr::dynamicCast(q->first))
	    {
		out << nl << "__is.readObject(" << fixKwd(q->second) << ".getPatcher());";
	    }
	    else
	    {
		writeMarshalUnmarshalCode(out, classScope, q->first, fixKwd(q->second), false, iter);
	    }
        }
        if(ret)
        {
	    BuiltinPtr builtin = BuiltinPtr::dynamicCast(ret);
	    if((builtin && builtin->kind() == Builtin::KindObject) || ClassDeclPtr::dynamicCast(ret))
	    {
		out << nl << "__is.readObject(__ret.getPatcher());";
	    }
	    else
	    {
		writeMarshalUnmarshalCode(out, classScope, ret, "__ret", false, iter);
	    }
        }
	if(p->returnsClasses())
	{
	    out << nl << "__is.readPendingObjects();";
	}
   	out << eb;
	out << nl << "catch(Ice.LocalException __ex)";
	out << sb;
	out << nl << "ice_exception(__ex);";
	out << nl << "return;";
	out << eb;
	if(!throws.empty())
	{
	    out << nl << "catch(Ice.UserException __ex)";
	    out << sb;
	    out << nl << "ice_exception(__ex);";
	    out << nl << "return;";
	    out << eb;
	}
	out << nl << "ice_response(" << argsAMI << ");";
	out << eb;
	out << eb;

	close();
    }

    if(cl->hasMetaData("amd") || p->hasMetaData("amd"))
    {
	string classNameAMD = "AMD_" + fixKwd(cl->name());
	string classScopedAMD = classScope + classNameAMD;
	string absoluteAMD = getAbsolute(classScopedAMD);

	string classNameAMDI = "_AMD_" + fixKwd(cl->name());
	string classScopedAMDI = classScope + classNameAMDI;
	string absoluteAMDI = getAbsolute(classScopedAMDI);

	string paramsAMD = getParamsAsyncCB(p, classScope);

	{
	    if(!open(absoluteAMD + '_' + name))
	    {
		return;
	    }

	    Output& out = output();
	    
	    out << sp << nl << "public interface " << classNameAMD << '_' << name;
	    out << sb;
	    out << sp << nl << "void ice_response(" << paramsAMD << ");";
	    out << sp << nl << "void ice_exception(java.lang.Exception ex);";
	    out << eb;
	    
	    close();
	}
	
	{
	    if(!open(absoluteAMDI + '_' + name))
	    {
		return;
	    }
	    
	    Output& out = output();
	    
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

	    TypeStringList::const_iterator q;
	    int iter;

	    out << sp << nl << "final class " << classNameAMDI << '_' << name
		<< " extends IceInternal.IncomingAsync implements " << classNameAMD << '_' << name;
	    out << sb;

	    out << sp << nl << "public" << nl << classNameAMDI << '_' << name << "(IceInternal.Incoming in)";
	    out << sb;
	    out << nl << "super(in);";
	    out << eb;

	    out << sp << nl << "public void" << nl << "ice_response(" << paramsAMD << ")";
	    out << sb;
	    out << nl << "if(!_finished)";
	    out << sb;
	    if(ret || !outParams.empty())
	    {
		out << nl << "try";
		out << sb;
		out << nl << "IceInternal.BasicStream __os = this.__os();";
		for(q = outParams.begin(); q != outParams.end(); ++q)
		{
		    string typeS = typeToString(q->first, TypeModeIn, classScope);
		    writeMarshalUnmarshalCode(out, classScope, q->first, fixKwd(q->second), true, iter);
		}
		if(ret)
		{
		    string retS = typeToString(ret, TypeModeIn, classScope);
		    writeMarshalUnmarshalCode(out, classScope, ret, "__ret", true, iter);
		}
		if(p->returnsClasses())
		{
		    out << nl << "__os.writePendingObjects();";
		}
		out << eb;
		out << nl << "catch(Ice.LocalException __ex)";
		out << sb;
		out << nl << "ice_exception(__ex);";
		out << eb;
	    }
	    out << nl << "__response(true);";
	    out << eb;
	    out << eb;

	    out << sp << nl << "public void" << nl << "ice_exception(java.lang.Exception ex)";
	    out << sb;
	    out << nl << "if(!_finished)";
	    out << sb;
	    if(throws.empty())
	    {
		out << nl << "__exception(ex);";
	    }
	    else
	    {
		out << nl << "try";
		out << sb;
		out << nl << "throw ex;";
		out << eb;
		ExceptionList::const_iterator r;
		for(r = throws.begin(); r != throws.end(); ++r)
		{
		    string exS = getAbsolute((*r)->scoped(), classScope);
		    out << nl << "catch(" << exS << " __ex)";
		    out << sb;
		    out << nl << "__os().writeUserException(__ex);";
		    out << nl << "__response(false);";
		    out << eb;
		}
		out << nl << "catch(java.lang.Exception __ex)";
		out << sb;
		out << nl << "__exception(__ex);";
		out << eb;
	    }
	    out << eb;
	    out << eb;

	    out << eb;
	    
	    close();
	}
    }
}
