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
#include <Gen.h>
#include <limits>

using namespace std;
using namespace Slice;
using namespace IceUtil;

static string					// Should be an anonymous namespace, but VC++ 6 can't handle that.
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
Slice::JavaVisitor::getArgs(const OperationPtr& op, const string& scope)
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

static bool
exceptionIsLocal(const ::Slice::ExceptionPtr exception)
{
    return exception->isLocal();
}

void
Slice::JavaVisitor::writeThrowsClause(const string& scope, const ExceptionList& throws)
{
    //
    // Don't include local exceptions in the throws clause
    //
    ExceptionList::size_type localCount = 0;

    //
    // MSVC gets confused if
    // ::IceUtil::constMemFun(&::Slice::Exception::isLocal)); is used hence
    // the exceptionIsLocal function.
    //
    localCount = count_if(throws.begin(), throws.end(),	exceptionIsLocal);

    Output& out = output();
    if(throws.size() - localCount > 0)
    {
        out.inc();
        out << nl;
        out << "throws ";
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
    out << nl;
    out << "throws ";
    out.useCurrentPosAsIndent();
    out << "Ice.LocationForward,";
    out << nl << "IceInternal.NonRepeatable";

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
                out << nl << "__h = 5 * __h + "
                    << "java.lang.Float.floatToIntBits(" << name << ");";
                break;
            }
            case Builtin::KindDouble:
            {
                out << nl << "__h = 5 * __h + (int)"
                    << "java.lang.Double.doubleToLongBits(" << name << ");";
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
            out << nl << "for(int __i" << iter << " = 0; __i" << iter
                << " < " << name << ".length; __i" << iter << "++)";
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
    transform(allBases.begin(), allBases.end(), back_inserter(ids), ::IceUtil::constMemFun(&ClassDef::scoped));
    StringList other;
    other.push_back(scoped);
    other.push_back("::Ice::Object");
    other.sort();
    ids.merge(other);
    ids.unique();
    StringList::const_iterator firstIter = ids.begin();
    StringList::const_iterator scopedIter = find(ids.begin(), ids.end(), scoped);
    assert(scopedIter != ids.end());
    int scopedPos = distance(firstIter, scopedIter);

    StringList::const_iterator q;

    out << sp << nl << "public static final String[] __ids =";
    out << sb;
    q = ids.begin();
    while(q != ids.end())
    {
        out << nl << '"' << *q << '"';
        if(++q != ids.end())
        {
            out << ',';
        }
    }
    out << eb << ';';

    //
    // ice_isA
    //
    out << sp << nl << "public boolean"
        << nl << "ice_isA(String s, Ice.Current current)";
    out << sb;
    out << nl << "return java.util.Arrays.binarySearch(__ids, s) >= 0;";
    out << eb;

    //
    // ice_ids
    //
    out << sp << nl << "public String[]"
        << nl << "ice_ids(Ice.Current current)";
    out << sb;
    out << nl << "return __ids;";
    out << eb;

    //
    // ice_id
    //
    out << sp << nl << "public String"
        << nl << "ice_id(Ice.Current current)";
    out << sb;
    out << nl << "return __ids[" << scopedPos << "];";
    out << eb;

    //
    // ice_staticId
    //
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
        OperationPtr op = (*r);
        ContainerPtr container = op->container();
        ClassDefPtr cl = ClassDefPtr::dynamicCast(container);
        assert(cl);

        string opName = fixKwd(op->name());
        out << sp << nl << "public static IceInternal.DispatchStatus"
            << nl << "___" << opName << "(" << name << " __obj, IceInternal.Incoming __in, Ice.Current __current)";
        out << sb;

        TypePtr ret = op->returnType();
        string retS = typeToString(ret, TypeModeReturn, scope);
        int iter;

        TypeStringList inParams;
        TypeStringList outParams;
	ParamDeclList paramList = op->parameters();
	for(ParamDeclList::const_iterator pli = paramList.begin(); pli != paramList.end(); ++pli)
	{
	    TypeStringList &listref = (*pli)->isOutParam() ? outParams : inParams;
	    listref.push_back(make_pair((*pli)->type(), (*pli)->name()));
	}

        TypeStringList::const_iterator q;

        ExceptionList throws = op->throws();
        throws.sort();
        throws.unique();

	//
	// MSVC gets confused if
	// ::IceUtil::constMemFun(&::Slice::Exception::isLocal)); is used
	// hence the exceptionIsLocal function.
	//
        remove_if(throws.begin(), throws.end(), exceptionIsLocal);

        if(!inParams.empty())
        {
            out << nl << "IceInternal.BasicStream __is = __in.is();";
        }
        if(!outParams.empty() || ret || throws.size() > 0)
        {
            out << nl << "IceInternal.BasicStream __os = __in.os();";
        }

        //
        // Unmarshal 'in' params
        //
        iter = 0;
        for(q = inParams.begin(); q != inParams.end(); ++q)
        {
            string typeS = typeToString(q->first, TypeModeIn, scope);
            out << nl << typeS << ' ' << fixKwd(q->second) << ';';
            writeMarshalUnmarshalCode(out, scope, q->first, fixKwd(q->second), false, iter);
        }

        //
        // Create holders for 'out' params
        //
        for(q = outParams.begin(); q != outParams.end(); ++q)
        {
            string typeS = typeToString(q->first, TypeModeOut, scope);
            out << nl << typeS << ' ' << fixKwd(q->second) << " = new " << typeS << "();";
        }

        if(!throws.empty())
        {
            out << nl << "try";
            out << sb;
        }

        //
        // Call servant
        //
        out << nl;
        if(ret)
        {
            out << retS << " __ret = ";
        }
        out << "__obj." << opName << '(';
        for(q = inParams.begin(); q != inParams.end(); ++q)
        {
            out << fixKwd(q->second) << ", ";
        }
        for(q = outParams.begin(); q != outParams.end(); ++q)
        {
            out << fixKwd(q->second) << ", ";
        }
        out << "__current);";

        //
        // Marshal 'out' params
        //
        for(q = outParams.begin(); q != outParams.end(); ++q)
        {
            writeMarshalUnmarshalCode(out, scope, q->first, fixKwd(q->second), true, iter, true);
        }
        //
        // Marshal result
        //
        if(ret)
        {
            writeMarshalUnmarshalCode(out, scope, ret, "__ret", true, iter);
        }

        out << nl << "return IceInternal.DispatchStatus.DispatchOK;";

        //
        // User exceptions
        //
        if(!throws.empty())
        {
            out << eb;
            ExceptionList::const_iterator r;
            for(r = throws.begin(); r != throws.end(); ++r)
            {
                string exS = getAbsolute((*r)->scoped(), scope);
                out << nl << "catch(" << exS << " ex)";
                out << sb;
                out << nl << "__os.writeUserException(ex);";
                out << nl << "return IceInternal.DispatchStatus.DispatchUserException;";
                out << eb;
            }
        }

        out << eb;
    }

    //
    // __dispatch
    //
    OperationList allOps = p->allOperations();
    if(!allOps.empty())
    {
        StringList allOpNames;
        transform(allOps.begin(), allOps.end(), back_inserter(allOpNames), ::IceUtil::constMemFun(&Operation::name));
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

        out << sp << nl << "public IceInternal.DispatchStatus"
            << nl << "__dispatch(IceInternal.Incoming in, Ice.Current current)";
        out << sb;
        out << nl << "int pos = java.util.Arrays.binarySearch(__all, current.operation);";
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
                out << nl << "return ___ice_facets(this, in, current);";
            }
            else if(opName == "ice_id")
            {
                out << nl << "return ___ice_id(this, in, current);";
            }
            else if(opName == "ice_ids")
            {
                out << nl << "return ___ice_ids(this, in, current);";
            }
            else if(opName == "ice_isA")
            {
                out << nl << "return ___ice_isA(this, in, current);";
            }
            else if(opName == "ice_ping")
            {
                out << nl << "return ___ice_ping(this, in, current);";
            }
            else
            {
                //
                // There's probably a better way to do this
                //
                for(OperationList::const_iterator r = allOps.begin(); r != allOps.end(); ++r)
                {
                    if((*r)->name() == (*q))
                    {
                        ContainerPtr container = (*r)->container();
                        ClassDefPtr cl = ClassDefPtr::dynamicCast(container);
                        assert(cl);
                        if(cl->scoped() == p->scoped())
                        {
                            out << nl << "return ___" << opName << "(this, in, current);";
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
                            out << nl << "return " << base << ".___" << opName << "(this, in, current);";
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
Slice::Gen::generate(const UnitPtr& unit)
{
    OpsVisitor opsVisitor(_dir, _package);
    unit->visit(&opsVisitor);

    TypesVisitor typesVisitor(_dir, _package);
    unit->visit(&typesVisitor);

    HolderVisitor holderVisitor(_dir, _package);
    unit->visit(&holderVisitor);

    HelperVisitor helperVisitor(_dir, _package);
    unit->visit(&helperVisitor);

    ProxyVisitor proxyVisitor(_dir, _package);
    unit->visit(&proxyVisitor);

    DelegateVisitor delegateVisitor(_dir, _package);
    unit->visit(&delegateVisitor);

    DelegateMVisitor delegateMVisitor(_dir, _package);
    unit->visit(&delegateMVisitor);

    DelegateDVisitor delegateDVisitor(_dir, _package);
    unit->visit(&delegateDVisitor);

    DispatcherVisitor dispatcherVisitor(_dir, _package);
    unit->visit(&dispatcherVisitor);
}

void
Slice::Gen::generateTie(const UnitPtr& unit)
{
    TieVisitor tieVisitor(_dir, _package);
    unit->visit(&tieVisitor);
}

void
Slice::Gen::generateImpl(const UnitPtr& unit)
{
    ImplVisitor implVisitor(_dir, _package);
    unit->visit(&implVisitor);
}

void
Slice::Gen::generateImplTie(const UnitPtr& unit)
{
    ImplTieVisitor implTieVisitor(_dir, _package);
    unit->visit(&implTieVisitor);
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
    string absolute = getAbsolute(scoped);

    //
    // Don't generate an Operations interface for non-abstract classes
    //
    if(!p->isAbstract())
    {
        return false;
    }

    if(!open(absolute + "Operations"))
    {
        return false;
    }

    Output& out = output();

    //
    // Generate the operations interface
    //
    out << sp << nl << "public interface " << name << "Operations";
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
                out << getAbsolute((*q)->scoped(), scope, "", "Operations");
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

    TypePtr ret = p->returnType();
    string retS = typeToString(ret, TypeModeReturn, scope);

    string params = getParams(p, scope);

    Output& out = output();

    out << sp;
    out << nl;
    out << retS << ' ' << name << '(' << params;
    if(!cl->isLocal())
    {
        if(!params.empty())
        {
            out << ", ";
        }
        out << "Ice.Current current";
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
    string absolute = getAbsolute(scoped);

    //
    // Don't generate a TIE class for a non-abstract class
    //
    if(!p->isAbstract())
    {
        return false;
    }

    if(!open(absolute + "Tie"))
    {
        return false;
    }

    Output& out = output();

    //
    // Generate the TIE class
    //
    out << sp << nl << "public class " << name << "Tie";
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

    out << sp << nl << "public" << nl << name << "Tie()";
    out << sb;
    out << eb;

    out << sp << nl << "public" << nl << name << "Tie(" << name << "Operations delegate)";
    out << sb;
    out << nl << "_ice_delegate = delegate;";
    out << eb;

    out << sp << nl << "public " << name << "Operations" << nl << "ice_delegate()";
    out << sb;
    out << nl << "return _ice_delegate;";
    out << eb;

    out << sp << nl << "public void" << nl << "ice_delegate(" << name << "Operations delegate)";
    out << sb;
    out << nl << "_ice_delegate = delegate;";
    out << eb;

    out << sp << nl << "public boolean" << nl << "equals(java.lang.Object rhs)";
    out << sb;
    out << nl << "if(this == rhs)";
    out << sb;
    out << nl << "return true;";
    out << eb;
    out << nl << "if(!(rhs instanceof " << name << "Tie))";
    out << sb;
    out << nl << "return false;";
    out << eb;
    out << sp << nl << "return _ice_delegate.equals(((" << name << "Tie)rhs)._ice_delegate);";
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
        string args = getArgs((*r), scope);

        out << sp;
        out << nl;
        out << "public " << retS << nl << opName << '(' << params;
        if(!p->isLocal())
        {
            if(!params.empty())
            {
                out << ", ";
            }
            out << "Ice.Current current";
        }
        out << ')';

        ExceptionList throws = (*r)->throws();
        throws.sort();
        throws.unique();
        writeThrowsClause(scope, throws);
        out << sb;
        out << nl;
        if(ret)
        {
            out << "return ";
        }
        out << "_ice_delegate." << opName << '(' << args;
        if(!p->isLocal())
        {
            if(!args.empty())
            {
                out << ", ";
            }
            out << "current";
        }
        out << ");";
        out << eb;
    }

    out << sp << nl << "private " << name << "Operations _ice_delegate;";
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
        out << "," << nl << name << "Operations";
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
            implements.push_back(name + "Operations");
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
    // hashCode
    //
    if(!p->isInterface())
    {
        //
        // Does the class have (or inherit) any data members?
        //
        bool baseHasDataMembers = false;
        ClassList l = p->bases();
        while(!l.empty() && !l.front()->isInterface())
        {
            if(l.front()->hasDataMembers())
            {
                baseHasDataMembers = true;
                break;
            }
            l = l.front()->bases();
        }

        if(p->hasDataMembers() || baseHasDataMembers)
        {
            out << sp << nl << "public int"
                << nl << "hashCode()";
            out << sb;
            if(p->hasDataMembers())
            {
                DataMemberList members = p->dataMembers();
                DataMemberList::const_iterator d;
                int iter;

                out << nl << "int __h = 0;";
                iter = 0;
                for(d = members.begin(); d != members.end(); ++d)
                {
                    string memberName = fixKwd((*d)->name());
                    list<string> metaData = (*d)->getMetaData();
                    writeHashCode(out, (*d)->type(), memberName, iter, metaData);
                }
                if(baseHasDataMembers)
                {
                    out << nl << "__h = 5 * __h + super.hashCode();";
                }
                out << nl << "return __h;";
            }
            else
            {
                out << nl << "return super.hashCode();";
            }
            out << eb;
        }
    }

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
        out << nl << "public static Ice.ObjectFactory _factory = new __F();";
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
        //
        // __dispatch, etc.
        //
        writeDispatch(out, p);

        DataMemberList members = p->dataMembers();
        DataMemberList::const_iterator d;
        int iter;

        //
        // __write
        //
        out << sp << nl << "public void"
            << nl << "__write(IceInternal.BasicStream __os)";
        out << sb;
        iter = 0;
        for(d = members.begin(); d != members.end(); ++d)
        {
            list<string> metaData = (*d)->getMetaData();
            writeMarshalUnmarshalCode(out, scope, (*d)->type(), fixKwd((*d)->name()), true, iter, false, metaData);
        }
        out << nl << "super.__write(__os);";
        out << eb;

        //
        // __read
        //
        out << sp << nl << "public void"
            << nl << "__read(IceInternal.BasicStream __is)";
        out << sb;
        iter = 0;
        for(d = members.begin(); d != members.end(); ++d)
        {
            list<string> metaData = (*d)->getMetaData();
            writeMarshalUnmarshalCode(out, scope, (*d)->type(), fixKwd((*d)->name()), false, iter, false, metaData);
        }
        out << nl << "super.__read(__is);";
        out << eb;

        //
        // __marshal
        //
        out << sp << nl << "public void" << nl << "__marshal(Ice.Stream __os)";
        out << sb;
        out << nl << "super.__marshal(__os);"; // Base must come first (due to schema rules).
        iter = 0;
        for(d = members.begin(); d != members.end(); ++d)
        {
            string s = (*d)->name();
            list<string> metaData = (*d)->getMetaData();
            writeGenericMarshalUnmarshalCode(out, scope, (*d)->type(), "\"" + s + "\"", fixKwd(s), true, iter, false,
                                             metaData);
        }
        out << eb;

        //
        // __unmarshal
        //
        out << sp << nl << "public void" << nl << "__unmarshal(Ice.Stream __is)";
        out << sb;
        out << nl << "super.__unmarshal(__is);"; // Base must come first (due to schema rules).
        iter = 0;
        for(d = members.begin(); d != members.end(); ++d)
        {
            string s = (*d)->name();
            list<string> metaData = (*d)->getMetaData();
            writeGenericMarshalUnmarshalCode(out, scope, (*d)->type(), "\"" + s + "\"", fixKwd(s), false, iter, false,
                                             metaData);
        }
        out << eb;

        //
        // ice_unmarshal
        //
        out << sp << nl << "public static Ice.Object" << nl << "ice_unmarshal(String __name, Ice.Stream __is)";
        out << sb;
        out << nl << name << " __val;";
        iter = 0;
        writeGenericMarshalUnmarshalCode(out, scope, p->declaration(), "__name", "__val", false, iter, false);
        out << nl << "return __val;";
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

    if(!p->isLocal())
    {
        ExceptionList allBases = p->allBases();
        StringList exceptionIds;
        transform(allBases.begin(), allBases.end(),
                  back_inserter(exceptionIds),
                  ::IceUtil::constMemFun(&Exception::scoped));
        exceptionIds.push_front(scoped);
        exceptionIds.push_back("::Ice::UserException");

        StringList::const_iterator q;

        out << sp << nl << "private static final String[] __exceptionIds =";
        out << sb;
        q = exceptionIds.begin();
        while(q != exceptionIds.end())
        {
            out << nl << '"' << *q << '"';
            if(++q != exceptionIds.end())
            {
                out << ',';
            }
        }
        out << eb << ';';
        out << sp << nl << "public String[]" << nl << "__getExceptionIds()";
        out << sb;
        out << nl << "return __exceptionIds;";
        out << eb;
    }

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

        //
        // __write
        //
        out << sp << nl << "public void"
            << nl << "__write(IceInternal.BasicStream __os)";
        out << sb;
        iter = 0;
        for(d = members.begin(); d != members.end(); ++d)
        {
            list<string> metaData = (*d)->getMetaData();
            writeMarshalUnmarshalCode(out, scope, (*d)->type(), fixKwd((*d)->name()), true, iter, false, metaData);
        }
        if(base)
        {
            out << nl << "super.__write(__os);";
        }
        out << eb;

        //
        // __read
        //
        out << sp << nl << "public void"
            << nl << "__read(IceInternal.BasicStream __is)";
        out << sb;
        iter = 0;
        for(d = members.begin(); d != members.end(); ++d)
        {
            list<string> metaData = (*d)->getMetaData();
            writeMarshalUnmarshalCode(out, scope, (*d)->type(), fixKwd((*d)->name()), false, iter, false, metaData);
        }
        if(base)
        {
            out << nl << "super.__read(__is);";
        }
        out << eb;

        //
        // __marshal
        //
        out << sp << nl << "public void"
            << nl << "__marshal(Ice.Stream __os)";
        out << sb;
        if(base)
        {
            out << nl << "super.__marshal(__os);"; // Base must come first (due to schema rules).
        }
        iter = 0;
        for(d = members.begin(); d != members.end(); ++d)
        {
            string s = (*d)->name();
            list<string> metaData = (*d)->getMetaData();
            writeGenericMarshalUnmarshalCode(out, scope, (*d)->type(), "\"" + s + "\"", fixKwd(s), true, iter, false,
                                             metaData);
        }
        out << eb;

        //
        // __unmarshal
        //
        out << sp << nl << "public void"
            << nl << "__unmarshal(Ice.Stream __is)";
        out << sb;
        if(base)
        {
            out << nl << "super.__unmarshal(__is);"; // Base must come first (due to schema rules).
        }
        iter = 0;
        for(d = members.begin(); d != members.end(); ++d)
        {
            string s = (*d)->name();
            list<string> metaData = (*d)->getMetaData();
            writeGenericMarshalUnmarshalCode(out, scope, (*d)->type(), "\"" + s + "\"", fixKwd(s), false, iter, false,
                                             metaData);
        }
        out << eb;

        //
        // ice_unmarshal
        //
        out << sp << nl << "public void" << nl << "ice_unmarshal(String __name, Ice.Stream __is)";
        out << sb;
        out << nl << "__is.startReadException(__name);";
        out << nl << "__unmarshal(__is);";
        out << nl << "__is.endReadException();";
        out << eb;
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

    //
    // equals
    //
    out << sp << nl << "public boolean"
        << nl << "equals(java.lang.Object rhs)";
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
                    out << nl << "if(" << memberName << " != _r." << memberName << ")";
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
            out << nl << "if(!" << memberName << ".equals(_r." << memberName << "))";
            out << sb;
            out << nl << "return false;";
            out << eb;
        }
    }
    out << sp << nl << "return true;";
    out << eb;
    out << sp << nl << "return false;";
    out << eb;

    //
    // hashCode
    //
    out << sp << nl << "public int"
        << nl << "hashCode()";
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

    //
    // clone - need to override it to make it public
    //
    out << sp << nl << "public java.lang.Object"
        << nl << "clone()";
    out.inc();
    out << nl << "throws java.lang.CloneNotSupportedException";
    out.dec();
    out << sb;
    out << nl << "return super.clone();";
    out << eb;

    if(!p->isLocal())
    {
        //
        // __write
        //
        out << sp << nl << "public final void" << nl << "__write(IceInternal.BasicStream __os)";
        out << sb;
        iter = 0;
        for(d = members.begin(); d != members.end(); ++d)
        {
            list<string> metaData = (*d)->getMetaData();
            writeMarshalUnmarshalCode(out, scope, (*d)->type(), fixKwd((*d)->name()), true, iter, false, metaData);
        }
        out << eb;

        //
        // __read
        //
        out << sp << nl << "public final void" << nl << "__read(IceInternal.BasicStream __is)";
        out << sb;
        iter = 0;
        for(d = members.begin(); d != members.end(); ++d)
        {
            list<string> metaData = (*d)->getMetaData();
            writeMarshalUnmarshalCode(out, scope, (*d)->type(), fixKwd((*d)->name()), false, iter, false, metaData);
        }
        out << eb;

        //
        // ice_marshal
        //
        out << sp << nl << "public final void"
            << nl << "ice_marshal(String __name, Ice.Stream __os)";
        out << sb;
        out << nl << "__os.startWriteStruct(__name);";
        iter = 0;
        for(d = members.begin(); d != members.end(); ++d)
        {
            string s = (*d)->name();
            list<string> metaData = (*d)->getMetaData();
            writeGenericMarshalUnmarshalCode(out, scope, (*d)->type(), "\"" + s + "\"", fixKwd(s), true, iter, false,
                                             metaData);
        }
        out << nl << "__os.endWriteStruct();";
        out << eb;

        //
        // ice_unmarshal
        //
        out << sp << nl << "public final void"
            << nl << "ice_unmarshal(String __name, Ice.Stream __is)";
        out << sb;
        out << nl << "__is.startReadStruct(__name);";
        iter = 0;
        for(d = members.begin(); d != members.end(); ++d)
        {
            string s = (*d)->name();
            list<string> metaData = (*d)->getMetaData();
            writeGenericMarshalUnmarshalCode(out, scope, (*d)->type(), "\"" + s + "\"", fixKwd(s), false, iter, false,
                                             metaData);
        }
        out << nl << "__is.endReadStruct();";
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
    int sz = enumerators.size();

    if(!open(absolute))
    {
        return;
    }

    Output& out = output();

    out << sp << nl << "public final class " << name;
    out << sb;
    out << nl << "private static " << name << "[] __values = new "
        << name << "[" << sz << "];";
    out << nl << "private int __value;";
    out << sp;
    int n;
    for(en = enumerators.begin(), n = 0; en != enumerators.end(); ++en, ++n)
    {
        string member = fixKwd((*en)->name());
        out << nl << "public static final int _" << member << " = "
            << n << ';';
        out << nl << "public static final " << name << ' ' << member
            << " = new " << name << "(_" << member << ");";
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
        //
        // __write
        //
        out << sp << nl << "public final void" << nl
            << "__write(IceInternal.BasicStream __os)";
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

        //
        // __read
        //
        out << sp << nl << "public static " << name << nl
            << "__read(IceInternal.BasicStream __is)";
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

        //
        // ice_marshal
        //
        out << sp << nl << "public final void" << nl << "ice_marshal(String __name, Ice.Stream __os)";
        out << sb;
        out << nl << "__os.writeEnum(__name, __T, __value);";
        out << eb;

        //
        // ice_unmarshal
        //
        out << sp << nl << "public static " << name << nl << "ice_unmarshal(String __name, Ice.Stream __is)";
        out << sb;
        out << nl << "int __val = __is.readEnum(__name, __T);";
        out << nl << "return convert(__val);";
        out << eb;
    }

    out << eb;
    close();
}

void
Slice::Gen::TypesVisitor::visitConstDef(const ConstDefPtr& p)
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
    out << ";" << eb;
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
    out << sp << nl << "public final class " << name
        << "PrxHelper extends Ice.ObjectPrxHelper implements " << name
        << "Prx";

    out << sb;

    OperationList ops = p->allOperations();

    OperationList::const_iterator r;
    for(r = ops.begin(); r != ops.end(); ++r)
    {
        OperationPtr op = (*r);
        string opName = fixKwd(op->name());
        TypePtr ret = op->returnType();
        string retS = typeToString(ret, TypeModeReturn, scope);

        string params = getParams(op, scope);
        string args = getArgs(op, scope);

        ExceptionList throws = op->throws();
        throws.sort();
        throws.unique();

        //
        // Write two versions of the operation - with and without a
        // context parameter
        //
        out << sp;
        out << nl;
        out << "public final " << retS << nl << opName << '(' << params
            << ")";
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
        out << "null);";
        out << eb;

        out << sp;
        out << nl;
        out << "public final " << retS << nl << opName << '(' << params;
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
        out << nl << "Ice._ObjectDel __delBase = __getDelegate();";
        out << nl << '_' << name << "Del __del = (_" << name
            << "Del)__delBase;";
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
        out << nl << "catch(Ice.LocationForward __ex)";
        out << sb;
        out << nl << "__locationForward(__ex);";
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
    }

    //
    // checkedCast
    //
    out << sp << nl << "public static " << name << "Prx"
        << nl << "checkedCast(Ice.ObjectPrx b)";
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

    out << sp << nl << "public static " << name << "Prx"
        << nl << "checkedCast(Ice.ObjectPrx b, String f)";
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

    //
    // uncheckedCast
    //
    out << sp << nl << "public static " << name << "Prx"
        << nl << "uncheckedCast(Ice.ObjectPrx b)";
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

    out << sp << nl << "public static " << name << "Prx"
        << nl << "uncheckedCast(Ice.ObjectPrx b, String f)";
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

    //
    // __createDelegateM
    //
    out << sp << nl << "protected Ice._ObjectDelM"
        << nl << "__createDelegateM()";
    out << sb;
    out << nl << "return new _" << name << "DelM();";
    out << eb;

    //
    // __createDelegateD
    //
    out << sp << nl << "protected Ice._ObjectDelD"
        << nl << "__createDelegateD()";
    out << sb;
    out << nl << "return new _" << name << "DelD();";
    out << eb;

    //
    // __write
    //
    out << sp << nl << "public static void"
        << nl << "__write(IceInternal.BasicStream __os, " << name << "Prx v)";
    out << sb;
    out << nl << "__os.writeProxy(v);";
    out << eb;

    //
    // __read
    //
    out << sp << nl << "public static " << name << "Prx"
        << nl << "__read(IceInternal.BasicStream __is)";
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

    //
    // ice_marshal
    //
    out << sp << nl << "public static void"
        << nl << "ice_marshal(String __name, Ice.Stream __os, " << name << "Prx v)";
    out << sb;
    out << nl << "__os.writeProxy(__name, v);";
    out << eb;

    //
    // ice_unmarshal
    //
    out << sp << nl << "public static " << name << "Prx"
        << nl << "ice_unmarshal(String __name, Ice.Stream __is)";
    out << sb;
    out << nl << "Ice.ObjectPrx proxy = __is.readProxy(__name);";
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

        //
        // write
        //
        out << nl << "public static void"
            << nl << "write(IceInternal.BasicStream __os, " << typeS << " __v)";
        out << sb;
        iter = 0;
        writeSequenceMarshalUnmarshalCode(out, scope, p, "__v", true, iter, false);
        out << eb;

        //
        // read
        //
        out << sp << nl << "public static " << typeS
            << nl << "read(IceInternal.BasicStream __is)";
        out << sb;
        out << nl << typeS << " __v;";
        iter = 0;
        writeSequenceMarshalUnmarshalCode(out, scope, p, "__v", false, iter, false);
        out << nl << "return __v;";
        out << eb;

        //
        // ice_marshal
        //
        out << sp << nl << "public static void"
            << nl << "ice_marshal(String __name, Ice.Stream __os, " << typeS << " __v)";
        out << sb;
        iter = 0;
        writeGenericSequenceMarshalUnmarshalCode(out, scope, p, "__name", "__v", true, iter, false);
        out << eb;

        //
        // ice_unmarshal
        //
        out << sp << nl << "public static " << typeS
            << nl << "ice_unmarshal(String __name, Ice.Stream __is)";
        out << sb;
        out << nl << typeS << " __v;";
        iter = 0;
        writeGenericSequenceMarshalUnmarshalCode(out, scope, p, "__name", "__v", false, iter, false);
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

        //
        // write
        //
        out << nl << "public static void" << nl << "write(IceInternal.BasicStream __os, " << "java.util.Map __v)";
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
                val = "((" + typeToString(type, TypeModeIn, scope) + ")" + arg + ")";
            }
            writeMarshalUnmarshalCode(out, scope, type, val, true, iter, false);
        }
        out << eb;
        out << eb;

        //
        // read
        //
        out << sp << nl << "public static java.util.Map"
            << nl << "read(IceInternal.BasicStream __is)";
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
                        out << nl << "Ice.Object " << arg << " = __is.readObject(\"\", null);";
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
                out << nl << s << ' ' << arg << ';';
                writeMarshalUnmarshalCode(out, scope, type, arg, false, iter, false);
            }
        }
        out << nl << "__r.put(__key, __value);";
        out << eb;
        out << nl << "return __r;";
        out << eb;

        //
        // ice_marshal
        //
        out << nl << "public static void" << nl << "ice_marshal(String __name, Ice.Stream __os, java.util.Map __v)";
        out << sb;
        out << nl << "__os.startWriteDictionary(__name, __v.size());";
        out << nl << "java.util.Iterator __i = __v.entrySet().iterator();";
        out << nl << "while(__i.hasNext())";
        out << sb;
        out << nl << "java.util.Map.Entry __e = (java.util.Map.Entry)" << "__i.next();";
        out << nl << "__os.startWriteDictionaryElement();";
        iter = 0;
        for(i = 0; i < 2; i++)
        {
            string val;
            string arg;
            TypePtr type;
            string tag;
            if(i == 0)
            {
                arg = "__e.getKey()";
                type = key;
                tag = "\"key\"";
            }
            else
            {
                arg = "__e.getValue()";
                type = value;
                tag = "\"value\"";
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
                val = "((" + typeToString(type, TypeModeIn, scope) + ")" + arg + ")";
            }
            writeGenericMarshalUnmarshalCode(out, scope, type, tag, val, true, iter, false);
        }
        out << nl << "__os.endWriteDictionaryElement();";
        out << eb;
        out << nl << "__os.endWriteDictionary();";
        out << eb;

        //
        // ice_unmarshal
        //
        out << sp << nl << "public static java.util.Map" << nl << "ice_unmarshal(String __name, Ice.Stream __is)";
        out << sb;
        out << nl << "int __sz = __is.startReadDictionary(__name);";
        out << nl << "java.util.Map __r = new java.util.HashMap(__sz);";
        out << nl << "for(int __i = 0; __i < __sz; __i++)";
        out << sb;
        out << nl << "__is.startReadDictionaryElement();";
        iter = 0;
        for(i = 0; i < 2; i++)
        {
            string arg;
            TypePtr type;
            string tag;
            if(i == 0)
            {
                arg = "__key";
                type = key;
                tag = "\"key\"";
            }
            else
            {
                arg = "__value";
                type = value;
                tag = "\"value\"";
            }

            BuiltinPtr b = BuiltinPtr::dynamicCast(type);
            if(b)
            {
                switch(b->kind())
                {
                    case Builtin::KindByte:
                    {
                        out << nl << "java.lang.Byte " << arg
                            << " = new java.lang.Byte(__is.readByte(" << tag << "));";
                        break;
                    }
                    case Builtin::KindBool:
                    {
                        out << nl << "java.lang.Boolean " << arg
                            << " = new java.lang.Boolean(__is.readBool(" << tag << "));";
                        break;
                    }
                    case Builtin::KindShort:
                    {
                        out << nl << "java.lang.Short " << arg
                            << " = new java.lang.Short(__is.readShort(" << tag << "));";
                        break;
                    }
                    case Builtin::KindInt:
                    {
                        out << nl << "java.lang.Integer " << arg
                            << " = new java.lang.Integer(__is.readInt(" << tag << "));";
                        break;
                    }
                    case Builtin::KindLong:
                    {
                        out << nl << "java.lang.Long " << arg
                            << " = new java.lang.Long(__is.readLong(" << tag << "));";
                        break;
                    }
                    case Builtin::KindFloat:
                    {
                        out << nl << "java.lang.Float " << arg
                            << " = new java.lang.Float(__is.readFloat(" << tag << "));";
                        break;
                    }
                    case Builtin::KindDouble:
                    {
                        out << nl << "java.lang.Double " << arg
                            << " = new java.lang.Double(__is.readDouble(" << tag << "));";
                        break;
                    }
                    case Builtin::KindString:
                    {
                        out << nl << "java.lang.String " << arg << " = __is.readString(" << tag << ");";
                        break;
                    }
                    case Builtin::KindObject:
                    {
                        out << nl << "Ice.Object " << arg << " = __is.readObject(" << tag << ", \"\", null);";
                        break;
                    }
                    case Builtin::KindObjectProxy:
                    {
                        out << nl << "Ice.ObjectPrx " << arg << " = __is.readProxy(" << tag << ");";
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
                out << nl << s << ' ' << arg << ';';
                writeGenericMarshalUnmarshalCode(out, scope, type, tag, arg, false, iter, false);
            }
        }
        out << nl << "__is.endReadDictionaryElement();";
        out << nl << "__r.put(__key, __value);";
        out << eb;
        out << nl << "__is.endReadDictionary();";
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

    TypePtr ret = p->returnType();
    string retS = typeToString(ret, TypeModeReturn, scope);

    string params = getParams(p, scope);

    Output& out = output();

    ExceptionList throws = p->throws();
    throws.sort();
    throws.unique();

    //
    // Write two versions of the operation - with and without a
    // context parameter
    //
    out << sp;
    out << nl;
    out << "public " << retS << ' ' << name << '(' << params << ")";
    writeThrowsClause(scope, throws);
    out << ';';
    out << nl;
    out << "public " << retS << ' ' << name << '(' << params;
    if(!params.empty())
    {
        out << ", ";
    }
    out << "java.util.Map __context)";
    writeThrowsClause(scope, throws);
    out << ';';
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

    out << sp << nl << "public interface _" << name
        << "Del extends ";
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
        OperationPtr op = (*r);
        string opName = fixKwd(op->name());
        TypePtr ret = op->returnType();
        string retS = typeToString(ret, TypeModeReturn, scope);

        string params = getParams(op, scope);

        ExceptionList throws = op->throws();
        throws.sort();
        throws.unique();

        out << sp;
        out << nl;
        out << retS << ' ' << opName << '(' << params;
        if(!params.empty())
        {
            out << ", ";
        }
        out << "java.util.Map __context)";
        writeDelegateThrowsClause(scope, throws);
        out << ';';
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

    out << sp << nl << "public final class _" << name
        << "DelM extends Ice._ObjectDelM implements _" << name << "Del";
    out << sb;

    OperationList ops = p->allOperations();

    OperationList::const_iterator r;
    for(r = ops.begin(); r != ops.end(); ++r)
    {
        OperationPtr op = (*r);
        string opName = fixKwd(op->name());
        TypePtr ret = op->returnType();
        string retS = typeToString(ret, TypeModeReturn, scope);
        int iter;

        TypeStringList inParams;
        TypeStringList outParams;
	ParamDeclList paramList = op->parameters();
	for(ParamDeclList::const_iterator pli = paramList.begin(); pli != paramList.end(); ++pli)
	{
	    TypeStringList &listref = (*pli)->isOutParam() ? outParams : inParams;
	    listref.push_back(make_pair((*pli)->type(), (*pli)->name()));
	}

        TypeStringList::const_iterator q;

        ExceptionList throws = op->throws();
        throws.sort();
        throws.unique();

	//
	// MSVC gets confused if
	// ::IceUtil::constMemFun(&::Slice::Exception::isLocal)); is used
	// hence the exceptionIsLocal function.
	//
        throws.erase(remove_if(throws.begin(), throws.end(), exceptionIsLocal), throws.end());

        string params = getParams(op, scope);

        out << sp;
        out << nl;
        out << "public " << retS << nl << opName << '(' << params;
        if(!params.empty())
        {
            out << ", ";
        }
        out << "java.util.Map __context)";
        writeDelegateThrowsClause(scope, throws);
        out << sb;
	list<string> metaData = op->getMetaData();
        out << nl << "IceInternal.Outgoing __out = getOutgoing(\"" << op->name() << "\", "
	    << sliceModeToIceMode(op) << ", __context);";
        out << nl << "try";
        out << sb;
        if(!inParams.empty())
        {
            out << nl << "IceInternal.BasicStream __os = __out.os();";
        }
        if(!outParams.empty() || ret || throws.size() > 0)
        {
            out << nl << "IceInternal.BasicStream __is = __out.is();";
        }
        iter = 0;
        for(q = inParams.begin(); q != inParams.end(); ++q)
        {
            writeMarshalUnmarshalCode(out, scope, q->first, fixKwd(q->second),
                                      true, iter);
        }
        out << nl << "if(!__out.invoke())";
        out << sb;
        if(throws.size() > 0)
        {
            //
            // The try/catch block is necessary because throwException()
            // can raise UserException
            //
            out << nl << "try";
            out << sb;
            out << nl << "final String[] __throws =";
            out << sb;
            ExceptionList::const_iterator r;
            for(r = throws.begin(); r != throws.end(); ++r)
            {
                if(r != throws.begin())
                {
                    out << ",";
                }
                out << nl << "\"" << (*r)->scoped() << "\"";
            }
            out << eb;
            out << ';';
            out << nl << "switch(__is.throwException(__throws))";
            out << sb;
            int count = 0;
            for(r = throws.begin(); r != throws.end(); ++r)
            {
                out << nl << "case " << count << ':';
                out << sb;
                string abs = getAbsolute((*r)->scoped(), scope);
                out << nl << abs << " __ex = new " << abs << "();";
                out << nl << "__ex.__read(__is);";
                out << nl << "throw __ex;";
                out << eb;
                count++;
            }
            out << eb;
            out << eb;
            for(r = throws.begin(); r != throws.end(); ++r)
            {
                out << nl << "catch(" << getAbsolute((*r)->scoped(), scope) << " __ex)";
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
            writeMarshalUnmarshalCode(out, scope, q->first, fixKwd(q->second), false, iter, true);
        }

        if(ret)
        {
            out << nl << retS << " __ret;";
            writeMarshalUnmarshalCode(out, scope, ret, "__ret", false, iter);
            out << nl << "return __ret;";
        }

        out << eb;
        out << nl << "finally";
        out << sb;
        out << nl << "reclaimOutgoing(__out);";
        out << eb;

        out << eb;
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

    out << sp << nl << "public final class _" << name
        << "DelD extends Ice._ObjectDelD implements _" << name << "Del";
    out << sb;

    OperationList ops = p->allOperations();

    OperationList::const_iterator r;
    for(r = ops.begin(); r != ops.end(); ++r)
    {
        OperationPtr op = (*r);
        string opName = fixKwd(op->name());
        TypePtr ret = op->returnType();
        string retS = typeToString(ret, TypeModeReturn, scope);

        ExceptionList throws = op->throws();
        throws.sort();
        throws.unique();

	//
	// MSVC gets confused if
	// ::IceUtil::constMemFun(&::Slice::Exception::isLocal)); is used
	// hence the exceptionIsLocal function.
	//
        throws.erase(remove_if(throws.begin(), throws.end(), exceptionIsLocal), throws.end());

        string params = getParams(op, scope);
        string args = getArgs(op, scope);

        out << sp;
        out << nl;
        out << "public " << retS << nl << opName << '(' << params;
        if(!params.empty())
        {
            out << ", ";
        }
        out << "java.util.Map __context)";
        writeDelegateThrowsClause(scope, throws);
        out << sb;
	list<string> metaData = op->getMetaData();
        out << nl << "Ice.Current __current = new Ice.Current();";
        out << nl << "__initCurrent(__current, \"" << op->name() << "\", "
	    << sliceModeToIceMode(op) << ", __context);";
        out << nl << "while(true)";
        out << sb;
        out << nl << "IceInternal.Direct __direct = new IceInternal.Direct(__adapter, __current);";
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
        ExceptionList::const_iterator r;
        for(r = throws.begin(); r != throws.end(); ++r)
        {
            out << nl << "catch(" << getAbsolute((*r)->scoped(), scope) << " __ex)";
            out << sb;
            out << nl << "throw __ex;";
            out << eb;
        }
        //
        // No need to catch Ice.UserException because it's not possible in Java
        //
        out << nl << "catch(Ice.LocalException __ex)";
        out << sb;
        out << nl << "throw __ex;";
        out << eb;
        out << nl << "catch(java.lang.RuntimeException __ex)";
        out << sb;
        out << nl << "Ice.UnknownException __e = new Ice.UnknownException();";
        out << nl << "__e.initCause(__ex);";
        out << nl << "throw __e;";
        out << eb;
        out << eb;
        out << nl << "finally";
        out << sb;
        out << nl << "__direct.destroy();";
        out << eb;
        out << eb;

        out << eb;
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

    out << sp << nl << "public abstract class _" << name
        << "Disp extends Ice.ObjectImpl implements " << name;
    out << sb;

    //
    // ice_copyStateFrom
    //
    out << sp << nl << "protected void"
        << nl << "ice_copyStateFrom(Ice.Object __obj)";
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

    out << sp << nl << "public " << retS << nl << opName << "(" << params;
    if(!local)
    {
        if(!params.empty())
        {
            out << ", ";
        }
        out << "Ice.Current current";
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
            out << " implements " << name;
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
    out << " implements " << name << "Operations";
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
