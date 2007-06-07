// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/DisableWarnings.h>
#include <Gen.h>
#include <IceUtil/Functional.h>
#include <IceUtil/Algorithm.h>
#include <IceUtil/Iterator.h>

#include <limits>

using namespace std;
using namespace Slice;

//
// Don't use "using namespace IceUtil", or VC++ 6.0 complains
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

static string
sliceModeToIceMode(Operation::Mode opMode)
{
    string mode;
    switch(opMode)
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

static string
getDeprecateReason(const ContainedPtr& p1, const ContainedPtr& p2, const string& type)
{
    string deprecateMetadata, deprecateReason;
    if(p1->findMetaData("deprecate", deprecateMetadata) ||
       (p2 != 0 && p2->findMetaData("deprecate", deprecateMetadata)))
    {
        deprecateReason = "This " + type + " has been deprecated.";
        if(deprecateMetadata.find("deprecate:") == 0 && deprecateMetadata.size() > 10)
        {
            deprecateReason = deprecateMetadata.substr(10);
        }
    }
    return deprecateReason;
}

void
Slice::JavaEOutput::printHeader()
{
    static const char* header =
"// **********************************************************************\n"
"//\n"
"// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.\n"
"//\n"
"// This copy of Ice-E is licensed to you under the terms described in the\n"
"// ICEE_LICENSE file included in this distribution.\n"
"//\n"
"// **********************************************************************\n"
        ;

    print(header);
    print("\n// Ice-E version ");
    print(ICEE_STRING_VERSION);
}

Slice::JavaVisitor::JavaVisitor(const string& dir) :
    JavaGenerator(dir, Slice::IceE)
{
}

Slice::JavaVisitor::~JavaVisitor()
{
}

JavaOutput*
Slice::JavaVisitor::createOutput()
{
    return new JavaEOutput;
}

vector<string>
Slice::JavaVisitor::getParams(const OperationPtr& op, const string& package)
{
    vector<string> params;

    ParamDeclList paramList = op->parameters();
    for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
        StringList metaData = (*q)->getMetaData();
        string typeString = typeToString((*q)->type(), (*q)->isOutParam() ? TypeModeOut : TypeModeIn, package,
                                         metaData);
        params.push_back(typeString + ' ' + fixKwd((*q)->name()));
    }

    return params;
}

vector<string>
Slice::JavaVisitor::getArgs(const OperationPtr& op)
{
    vector<string> args;

    ParamDeclList paramList = op->parameters();
    for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
        args.push_back(fixKwd((*q)->name()));
    }

    return args;
}

void
Slice::JavaVisitor::writeThrowsClause(const string& package, const ExceptionList& throws)
{
    Output& out = output();
    if(throws.size() > 0)
    {
        out.inc();
        out << nl << "throws ";
        out.useCurrentPosAsIndent();
        ExceptionList::const_iterator r;
        int count = 0;
        for(r = throws.begin(); r != throws.end(); ++r)
        {
            if(count > 0)
            {
                out << "," << nl;
            }
            out << getAbsolute(*r, package);
            count++;
        }
        out.restoreIndent();
        out.dec();
    }
}

void
Slice::JavaVisitor::writeHashCode(Output& out, const TypePtr& type, const string& name, int& iter)
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
                out << nl << "if(" << name << " != null)";
                out << sb;
                out << nl << "__h = 5 * __h + " << name << ".hashCode();";
                out << eb;
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
    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
    DictionaryPtr dict = DictionaryPtr::dynamicCast(type);
    if(prx || cl || dict)
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
        out << nl << "if(" << name << " != null)";
        out << sb;
        out << nl << "for(int __i" << iter << " = 0; __i" << iter << " < " << name << ".length; __i" << iter
            << "++)";
        out << sb;
        ostringstream elem;
        elem << name << "[__i" << iter << ']';
        iter++;
        writeHashCode(out, seq->type(), elem.str(), iter);
        out << eb;
        out << eb;
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
    string package = getPackage(p);
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
    StringList::difference_type scopedPos = ice_distance(firstIter, scopedIter);

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

    out << sp << nl << "public boolean" << nl << "ice_isA(String s)";
    out << sb;
    out << nl << "return IceUtil.Arrays.search(__ids, s) >= 0;";
    out << eb;

    out << sp << nl << "public boolean" << nl << "ice_isA(String s, Ice.Current __current)";
    out << sb;
    out << nl << "return IceUtil.Arrays.search(__ids, s) >= 0;";
    out << eb;

    out << sp << nl << "public String[]" << nl << "ice_ids()";
    out << sb;
    out << nl << "return __ids;";
    out << eb;

    out << sp << nl << "public String[]" << nl << "ice_ids(Ice.Current __current)";
    out << sb;
    out << nl << "return __ids;";
    out << eb;

    out << sp << nl << "public String" << nl << "ice_id()";
    out << sb;
    out << nl << "return __ids[" << scopedPos << "];";
    out << eb;

    out << sp << nl << "public String" << nl << "ice_id(Ice.Current __current)";
    out << sb;
    out << nl << "return __ids[" << scopedPos << "];";
    out << eb;

    out << sp << nl << "public static String" << nl << "ice_staticId()";
    out << sb;
    out << nl << "return __ids[" << scopedPos << "];";
    out << eb;

    OperationList ops = p->allOperations();
    OperationList::const_iterator r;

    //
    // Write the "no Current" implementation of each operation.
    //
    for(r = ops.begin(); r != ops.end(); ++r)
    {
        OperationPtr op = *r;
        string opName = op->name();

        ContainerPtr container = op->container();
        ClassDefPtr cl = ClassDefPtr::dynamicCast(container);
        assert(cl);

        vector<string> params;
        vector<string> args;
        TypePtr ret;

        opName = fixKwd(opName);
        ret = op->returnType();
        params = getParams(op, package);
        args = getArgs(op);

        ExceptionList throws = op->throws();
        throws.sort();
        throws.unique();

        //
        // Only generate a "no current" version of the operation if it hasn't been done in a base
        // class already, because the "no current" version is final.
        //
        bool generateOperation = cl == p; // Generate if the operation is defined in this class.
        if(!generateOperation)
        {
            //
            // The operation is not defined in this class.
            //
            if(!bases.empty())
            {
                //
                // Check if the operation is already implemented by a base class.
                //
                bool implementedByBase = false;
                if(!bases.front()->isInterface())
                {
                    OperationList baseOps = bases.front()->allOperations();
                    OperationList::const_iterator i;
                    for(i = baseOps.begin(); i != baseOps.end(); ++i)
                    {
                        if((*i)->name() == op->name())
                        {
                            implementedByBase = true;
                            break;
                        }
                    }
                    if(i == baseOps.end())
                    {
                        generateOperation = true;
                    }
                }
                if(!generateOperation && !implementedByBase)
                {
                     //
                     // No base class defines the operation. Check if one of the
                     // interfaces defines it, in which case this class must provide it.
                     //
                     if(bases.front()->isInterface() || bases.size() > 1)
                     {
                         generateOperation = true;
                     }
                }
            }
        }
        if(generateOperation)
        {
            out << sp << nl << "public final " << typeToString(ret, TypeModeReturn, package, op->getMetaData())
                << nl << opName << spar << params << epar;
            writeThrowsClause(package, throws);
            out << sb << nl;
            if(ret)
            {
                out << nl << "return ";
            }
            out << opName << spar << args << "null" << epar << ';';
            out << eb;
        }
    }

    //
    // Dispatch operations. We only generate methods for operations
    // defined in this ClassDef, because we reuse existing methods
    // for inherited operations.
    //
    ops = p->operations();
    for(r = ops.begin(); r != ops.end(); ++r)
    {
        OperationPtr op = *r;
        StringList opMetaData = op->getMetaData();
        ContainerPtr container = op->container();
        ClassDefPtr cl = ClassDefPtr::dynamicCast(container);
        assert(cl);

        string opName = op->name();
        out << sp << nl << "public static Ice.DispatchStatus" << nl << "___" << opName << '(' << name
            << " __obj, IceInternal.Incoming __in, Ice.Current __current)";
        out << sb;

        TypePtr ret = op->returnType();
            
        ParamDeclList inParams;
        ParamDeclList outParams;
        ParamDeclList paramList = op->parameters();
        ParamDeclList::const_iterator pli;
        for(pli = paramList.begin(); pli != paramList.end(); ++pli)
        {
            if((*pli)->isOutParam())
            {
                outParams.push_back(*pli);
            }
            else
            {
                inParams.push_back(*pli);
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

        int iter;
            
        out << nl << "__checkMode(" << sliceModeToIceMode(op->mode()) << ", __current.mode);";
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
        for(pli = inParams.begin(); pli != inParams.end(); ++pli)
        {
            StringList metaData = (*pli)->getMetaData();
            TypePtr paramType = (*pli)->type();
            string paramName = fixKwd((*pli)->name());
            string typeS = typeToString(paramType, TypeModeIn, package, metaData);
            BuiltinPtr builtin = BuiltinPtr::dynamicCast(paramType);
            if((builtin && builtin->kind() == Builtin::KindObject) || ClassDeclPtr::dynamicCast(paramType))
            {
                out << nl << typeS << "Holder " << paramName << " = new " << typeS << "Holder();";
                writeMarshalUnmarshalCode(out, package, paramType, paramName, false, iter, true,
                                          metaData, string());
            }
            else
            {
                out << nl << typeS << ' ' << paramName << ';';
                writeMarshalUnmarshalCode(out, package, paramType, paramName, false, iter, false, metaData);
            }
        }
            
        //
        // Create holders for 'out' parameters.
        //
        for(pli = outParams.begin(); pli != outParams.end(); ++pli)
        {
            string typeS = typeToString((*pli)->type(), TypeModeOut, package, (*pli)->getMetaData());
            out << nl << typeS << ' ' << fixKwd((*pli)->name()) << " = new " << typeS << "();";
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
            string retS = typeToString(ret, TypeModeReturn, package, opMetaData);
            out << retS << " __ret = ";
        }
        out << "__obj." << fixKwd(opName) << '(';
        for(pli = inParams.begin(); pli != inParams.end(); ++pli)
        {
            TypePtr paramType = (*pli)->type();
            out << fixKwd((*pli)->name());
            BuiltinPtr builtin = BuiltinPtr::dynamicCast(paramType);
            if((builtin && builtin->kind() == Builtin::KindObject) || ClassDeclPtr::dynamicCast(paramType))
            {
                out << ".value";
            }
            out << ", ";
        }
        for(pli = outParams.begin(); pli != outParams.end(); ++pli)
        {
            out << fixKwd((*pli)->name()) << ", ";
        }
        out << "__current);";
            
        //
        // Marshal 'out' parameters and return value.
        //
        for(pli = outParams.begin(); pli != outParams.end(); ++pli)
        {
            writeMarshalUnmarshalCode(out, package, (*pli)->type(), fixKwd((*pli)->name()), true, iter, true,
                                      (*pli)->getMetaData());
        }
        if(ret)
        {
            writeMarshalUnmarshalCode(out, package, ret, "__ret", true, iter, false, opMetaData);
        }
        out << nl << "return Ice.DispatchStatus.DispatchOK;";
            
        //
        // Handle user exceptions.
        //
        if(!throws.empty())
        {
            out << eb;
            ExceptionList::const_iterator t;
            for(t = throws.begin(); t != throws.end(); ++t)
            {
                string exS = getAbsolute(*t, package);
                out << nl << "catch(" << exS << " ex)";
                out << sb;
                out << nl << "__os.writeUserException(ex);";
                out << nl << "return Ice.DispatchStatus.DispatchUserException;";
                out << eb;
            }
        }

        out << eb;
    }

    OperationList allOps = p->allOperations();
    if(!allOps.empty())
    {
        StringList allOpNames;
        transform(allOps.begin(), allOps.end(), back_inserter(allOpNames), ::IceUtil::constMemFun(&Contained::name));
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

        out << sp << nl << "public Ice.DispatchStatus" << nl
            << "__dispatch(IceInternal.Incoming in, Ice.Current __current)";
        out << sb;
        out << nl << "int pos = IceUtil.Arrays.search(__all, __current.operation);";
        out << nl << "if(pos < 0)";
        out << sb;
        out << nl << "throw new Ice.OperationNotExistException(__current.id, __current.facet, __current.operation);";
        out << eb;
        out << sp << nl << "switch(pos)";
        out << sb;
        int i = 0;
        for(q = allOpNames.begin(); q != allOpNames.end(); ++q)
        {
            string opName = *q;

            out << nl << "case " << i++ << ':';
            out << sb;
            if(opName == "ice_id")
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
                // There's probably a better way to do this.
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
                                base = getAbsolute(cl, package, "_", "Disp");
                            }
                            else
                            {
                                base = getAbsolute(cl, package);
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
        out << nl << "if(IceUtil.Debug.ASSERT)";
        out << sb;
        out << sp << nl << "IceUtil.Debug.Assert(false);";
        out << eb;
        out << nl << "throw new Ice.OperationNotExistException(__current.id, __current.facet, __current.operation);";
        out << eb;
    }
}

Slice::Gen::Gen(const string& name, const string& base, const vector<string>& includePaths, const string& dir) :
    _base(base),
    _includePaths(includePaths),
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
    JavaGenerator::validateMetaData(p);

    OpsVisitor opsVisitor(_dir);
    p->visit(&opsVisitor, false);

    PackageVisitor packageVisitor(_dir);
    p->visit(&packageVisitor, false);

    TypesVisitor typesVisitor(_dir);
    p->visit(&typesVisitor, false);

    HolderVisitor holderVisitor(_dir);
    p->visit(&holderVisitor, false);

    HelperVisitor helperVisitor(_dir);
    p->visit(&helperVisitor, false);

    ProxyVisitor proxyVisitor(_dir);
    p->visit(&proxyVisitor, false);

    DispatcherVisitor dispatcherVisitor(_dir);
    p->visit(&dispatcherVisitor, false);
}

void
Slice::Gen::generateTie(const UnitPtr& p)
{
    TieVisitor tieVisitor(_dir);
    p->visit(&tieVisitor, false);
}

void
Slice::Gen::generateImpl(const UnitPtr& p)
{
    ImplVisitor implVisitor(_dir);
    p->visit(&implVisitor, false);
}

void
Slice::Gen::generateImplTie(const UnitPtr& p)
{
    ImplTieVisitor implTieVisitor(_dir);
    p->visit(&implTieVisitor, false);
}

Slice::Gen::OpsVisitor::OpsVisitor(const string& dir) :
    JavaVisitor(dir)
{
}

bool
Slice::Gen::OpsVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    //
    // Don't generate an Operations interface for non-abstract classes
    //
    if(!p->isAbstract())
    {
        return false;
    }

    writeOperations(p, false);
    writeOperations(p, true);

    return false;
}

void
Slice::Gen::OpsVisitor::writeOperations(const ClassDefPtr& p, bool noCurrent)
{
    string name = p->name();
    ClassList bases = p->bases();
    string package = getPackage(p);
    string opIntfName = "Operations";
    if(noCurrent)
    {
        opIntfName += "NC";
    }
    string absolute = getAbsolute(p, "", "_", opIntfName);

    if(!open(absolute))
    {
        return;
    }

    Output& out = output();

    //
    // Generate the operations interface
    //
    out << sp << nl << "public interface " << '_' << name << opIntfName;
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
                out << getAbsolute(*q, package, "_", opIntfName);
            }
            ++q;
        }
        out.restoreIndent();
    }
    out << sb;

    OperationList ops = p->operations();
    OperationList::const_iterator r;
    for(r = ops.begin(); r != ops.end(); ++r)
    {
        OperationPtr op = *r;
        ContainerPtr container = op->container();
        ClassDefPtr cl = ClassDefPtr::dynamicCast(container);
        string opname = op->name();
        
        TypePtr ret;
        vector<string> params;

        params = getParams(op, package);
        ret = op->returnType();

        string deprecateReason = getDeprecateReason(op, p, "operation");

        string retS = typeToString(ret, TypeModeReturn, package, op->getMetaData());
        
        ExceptionList throws = op->throws();
        throws.sort();
        throws.unique();
        out << sp;
        if(!deprecateReason.empty())
        {
            out << nl << "/**";
            out << nl << " * @deprecated " << deprecateReason;
            out << nl << " **/";
        }
        out << nl << retS << ' ' << fixKwd(opname) << spar << params;
        if(!noCurrent)
        {
            out << "Ice.Current __current";
        }
        out << epar;
        writeThrowsClause(package, throws);
        out << ';';
    }

    out << eb;

    close();
}

Slice::Gen::TieVisitor::TieVisitor(const string& dir) :
    JavaVisitor(dir)
{
}

bool
Slice::Gen::TieVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    string name = p->name();
    ClassList bases = p->bases();
    string package = getPackage(p);
    string absolute = getAbsolute(p, "", "_", "Tie");
    string opIntfName = "Operations";

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
        out << " extends " << '_' << name << "Disp implements Ice.TieBase";
    }
    else
    {
        out << " extends " << fixKwd(name) << " implements Ice.TieBase";
    }

    out << sb;

    out << sp << nl << "public" << nl << '_' << name << "Tie()";
    out << sb;
    out << eb;

    out << sp << nl << "public" << nl << '_' << name << "Tie(" << '_' << name << opIntfName
        << " delegate)";
    out << sb;
    out << nl << "_ice_delegate = delegate;";
    out << eb;

    out << sp << nl << "public java.lang.Object" << nl << "ice_delegate()";
    out << sb;
    out << nl << "return _ice_delegate;";
    out << eb;

    out << sp << nl << "public void" << nl << "ice_delegate(java.lang.Object delegate)";
    out << sb;
    out << nl << "_ice_delegate = (_" << name << opIntfName << ")delegate;";
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
        ContainerPtr container = (*r)->container();
        ClassDefPtr cl = ClassDefPtr::dynamicCast(container);

        string opName = fixKwd((*r)->name());

        TypePtr ret = (*r)->returnType();
        string retS = typeToString(ret, TypeModeReturn, package, (*r)->getMetaData());

        vector<string> params = getParams((*r), package);
        vector<string> args = getArgs(*r);

        out << sp;
        out << nl << "public " << retS << nl << opName << spar << params;
        out << "Ice.Current __current";
        out << epar;

        ExceptionList throws = (*r)->throws();
        throws.sort();
        throws.unique();
        writeThrowsClause(package, throws);
        out << sb;
        out << nl;
        if(ret)
        {
            out << "return ";
        }
        out << "_ice_delegate." << opName << spar << args;
        out << "__current";
        out << epar << ';';
        out << eb;
    }

    out << sp << nl << "private " << '_' << name << opIntfName << " _ice_delegate;";
    out << eb;
    close();

    return false;
}

Slice::Gen::PackageVisitor::PackageVisitor(const string& dir) :
    JavaVisitor(dir)
{
}

bool
Slice::Gen::PackageVisitor::visitModuleStart(const ModulePtr& p)
{
    DefinitionContextPtr dc = p->definitionContext();
    assert(dc);
    StringList globalMetaData = dc->getMetaData();

    static const string packagePrefix = "java:package:";

    for(StringList::const_iterator q = globalMetaData.begin(); q != globalMetaData.end(); ++q)
    {
        string s = *q;
        if(s.find(packagePrefix) == 0)
        {
            string markerClass = s.substr(packagePrefix.size()) + "." + fixKwd(p->name()) + "._Marker";

            if(!open(markerClass))
            {
                cerr << "can't open class `" << markerClass << "' for writing: " << strerror(errno) << endl;
                return false;
            }

            Output& out = output();
            out << sp << nl << "interface _Marker";
            out << sb;
            out << eb;

            close();
        }
    }

    return false;
}

Slice::Gen::TypesVisitor::TypesVisitor(const string& dir) :
    JavaVisitor(dir)
{
}

bool
Slice::Gen::TypesVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    string name = p->name();
    ClassList bases = p->bases();
    ClassDefPtr baseClass;
    if(!bases.empty() && !bases.front()->isInterface())
    {
        baseClass = bases.front();
    }

    string package = getPackage(p);
    string absolute = getAbsolute(p);
    DataMemberList members = p->dataMembers();
    DataMemberList allDataMembers = p->allDataMembers();
    DataMemberList::const_iterator d;

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
        out << sp << nl << "public interface " << fixKwd(name) << " extends ";
        out.useCurrentPosAsIndent();
        out << "Ice.Object";
        out << "," << nl << '_' << name;
        out << "Operations, _" << name << "OperationsNC";
        if(!bases.empty())
        {
            ClassList::const_iterator q = bases.begin();
            while(q != bases.end())
            {
                out << ',' << nl << getAbsolute(*q, package);
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
        out << "class " << fixKwd(name);
        out.useCurrentPosAsIndent();
        if(bases.empty() || bases.front()->isInterface())
        {
            out << " extends Ice.ObjectImpl";
        }
        else
        {
            out << " extends ";
            out << getAbsolute(baseClass, package);
            bases.pop_front();
        }

        //
        // Implement interfaces
        //
        StringList implements;
        if(p->isAbstract())
        {
            implements.push_back("_" + name + "Operations");
            implements.push_back("_" + name + "OperationsNC");
        }
        if(!bases.empty())
        {
            ClassList::const_iterator q = bases.begin();
            while(q != bases.end())
            {
                implements.push_back(getAbsolute(*q, package));
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
                    out << ',' << nl;
                }
                out << *q;
                q++;
            }

            out.restoreIndent();
        }

        out.restoreIndent();
    }

    out << sb;

    if(!p->isInterface() && !allDataMembers.empty())
    {
        //
        // Constructors.
        //
        out << sp;
        out << nl << "public " << name << "()";
        out << sb;
        if(baseClass)
        {
            out << nl << "super();";
        }
        out << eb;

        out << sp << nl << "public " << name << spar;
        vector<string> paramDecl;
        for(d = allDataMembers.begin(); d != allDataMembers.end(); ++d)
        {
            string memberName = fixKwd((*d)->name());
            string memberType = typeToString((*d)->type(), TypeModeMember, package, (*d)->getMetaData());
            paramDecl.push_back(memberType + " " + memberName);
        }
        out << paramDecl << epar;
        out << sb;
        if(baseClass && allDataMembers.size() != members.size())
        {
            out << nl << "super" << spar;
            vector<string> baseParamNames;
            DataMemberList baseDataMembers = baseClass->allDataMembers();
            for(d = baseDataMembers.begin(); d != baseDataMembers.end(); ++d)
            {
                baseParamNames.push_back(fixKwd((*d)->name()));
            }
            out << baseParamNames << epar << ';';
        }
        vector<string> paramNames;
        for(d = members.begin(); d != members.end(); ++d)
        {
            paramNames.push_back(fixKwd((*d)->name()));
        }
        for(vector<string>::const_iterator i = paramNames.begin(); i != paramNames.end(); ++i)
        {
            out << nl << "this." << *i << " = " << *i << ';';
        }
        out << eb;
    }

    //
    // Marshalling & dispatch support.
    //
    if(!p->isInterface())
    {
        writeDispatch(out, p);
    }

    if(!p->isInterface() && !members.empty())
    {
        out << sp << nl << "protected void" << nl << "__copyFrom(java.lang.Object __obj)";
        out << sb;
        if(baseClass)
        {
            out << nl << "super.__copyFrom(__obj);";
        }
        out << nl << name << " __src = (" << name << ")__obj;";
        for(d = members.begin(); d != members.end(); ++d)
        {
            string memberName = fixKwd((*d)->name());
            out << nl << memberName << " = __src." << memberName << ";";
        }
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
    string package = getPackage(p);
    string absolute = getAbsolute(p);
    DataMemberList allDataMembers = p->allDataMembers();
    DataMemberList members = p->dataMembers();
    DataMemberList::const_iterator d;

    if(!open(absolute))
    {
        return false;
    }

    Output& out = output();

    out << sp;

    string deprecateReason = getDeprecateReason(p, 0, "type");
    if(!deprecateReason.empty())
    {
        out << nl << "/**";
        out << nl << " * @deprecated " << deprecateReason;
        out << nl << " **/";
    }

    out << nl << "public class " << name << " extends ";

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
        out << getAbsolute(base, package);
    }
    out << sb;

    if(!allDataMembers.empty())
    {
        //
        // Constructors.
        //
        out << sp;
        out << nl << "public " << name << "()";
        out << sb;
        if(base)
        {
            out << nl << "super();";
        }
        out << eb;

        out << sp << nl << "public " << name << spar;
        vector<string> paramDecl;
        for(d = allDataMembers.begin(); d != allDataMembers.end(); ++d)
        {
            string memberName = fixKwd((*d)->name());
            string memberType = typeToString((*d)->type(), TypeModeMember, package, (*d)->getMetaData());
            paramDecl.push_back(memberType + " " + memberName);
        }
        out << paramDecl << epar;
        out << sb;
        if(base && allDataMembers.size() != members.size())
        {
            out << nl << "super" << spar;
            vector<string> baseParamNames;
            DataMemberList baseDataMembers = base->allDataMembers();
            for(d = baseDataMembers.begin(); d != baseDataMembers.end(); ++d)
            {
                baseParamNames.push_back(fixKwd((*d)->name()));
            }
            out << baseParamNames << epar << ';';
        }
        vector<string> paramNames;
        for(d = members.begin(); d != members.end(); ++d)
        {
            paramNames.push_back(fixKwd((*d)->name()));
        }
        for(vector<string>::const_iterator i = paramNames.begin(); i != paramNames.end(); ++i)
        {
            out << nl << "this." << *i << " = " << *i << ';';
        }
        out << eb;
    }

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
        string package = getPackage(p);
        ExceptionPtr base = p->base();

        DataMemberList members = p->dataMembers();
        DataMemberList::const_iterator d;
        int iter;

        out << sp << nl << "public void" << nl << "__write(IceInternal.BasicStream __os)";
        out << sb;
        out << nl << "__os.writeString(\"" << scoped << "\");";
        out << nl << "__os.startWriteSlice();";
        iter = 0;
        for(d = members.begin(); d != members.end(); ++d)
        {
            StringList metaData = (*d)->getMetaData();
            writeMarshalUnmarshalCode(out, package, (*d)->type(), fixKwd((*d)->name()), true, iter, false, metaData);
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
                string memberType = typeToString((*d)->type(), TypeModeMember, package);
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
        for(d = members.begin(); d != members.end(); ++d)
        {
            ostringstream patchParams;
            StringList metaData = (*d)->getMetaData();
            writeMarshalUnmarshalCode(out, package, (*d)->type(), fixKwd((*d)->name()), false, iter, false, metaData,
                                      patchParams.str());
        }
        out << nl << "__is.endReadSlice();";
        if(base)
        {
            out << nl << "super.__read(__is, true);";
        }
        out << eb;

    }

    out << eb;
    close();
}

bool
Slice::Gen::TypesVisitor::visitStructStart(const StructPtr& p)
{
    string name = fixKwd(p->name());
    string absolute = getAbsolute(p);

    if(!open(absolute))
    {
        return false;
    }

    Output& out = output();

    out << sp;

    string deprecateReason = getDeprecateReason(p, 0, "type");
    if(!deprecateReason.empty())
    {
        out << nl << "/**";
        out << nl << " * @deprecated " << deprecateReason;
        out << nl << " **/";
    }

    out << nl << "public final class " << name;
    out << sb;

    return true;
}

void
Slice::Gen::TypesVisitor::visitStructEnd(const StructPtr& p)
{
    string package = getPackage(p);

    Output& out = output();

    DataMemberList members = p->dataMembers();
    DataMemberList::const_iterator d;
    int iter;

    string name = fixKwd(p->name());
    string typeS = typeToString(p, TypeModeIn, package);

    out << sp << nl << "public " << name << "()";
    out << sb;
    out << eb;

    vector<string> paramDecl;
    vector<string> paramNames;
    vector<string>::const_iterator q;
    for(d = members.begin(); d != members.end(); ++d)
    {
        string memberName = fixKwd((*d)->name());
        string memberType = typeToString((*d)->type(), TypeModeMember, package, (*d)->getMetaData());
        paramDecl.push_back(memberType + " " + memberName);
        paramNames.push_back(memberName);
    }

    out << sp << nl << "public " << name << spar << paramDecl << epar;
    out << sb;
    for(q = paramNames.begin(); q != paramNames.end(); ++q)
    {
        out << nl << "this." << *q << " = " << *q << ';';
    }
    out << eb;

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
                    out << nl << "if(" << memberName << " != _r." << memberName << " && " << memberName
                        << " != null && !" << memberName << ".equals(_r." << memberName << "))";
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
            // a Java array does not perform a deep comparison. We use the helper method
            // IceUtil.Arrays.equals() to compare native arrays.
            //
            // For all other types, we can use the native equals() method.
            //
            SequencePtr seq = SequencePtr::dynamicCast((*d)->type());
            if(seq)
            {
                out << nl << "if(!IceUtil.Arrays.equals(" << memberName << ", _r." << memberName << "))";
                out << sb;
                out << nl << "return false;";
                out << eb;
            }
            else
            {
                out << nl << "if(" << memberName << " != _r." << memberName << " && " << memberName
                    << " != null && !" << memberName << ".equals(_r." << memberName << "))";
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
        writeHashCode(out, (*d)->type(), memberName, iter);
    }
    out << nl << "return __h;";
    out << eb;

    out << sp << nl << "public java.lang.Object" << nl << "ice_clone()";
    out.inc();
    out << nl << "throws IceUtil.CloneException";
    out.dec();
    out << sb;
    out << nl << "return new " << name << '(';
    for(q = paramNames.begin(); q != paramNames.end(); ++q)
    {
        if(q != paramNames.begin())
        {
            out << ", ";
        }
        out << *q;
    }
    out << ");";
    out << eb;

    if(!p->isLocal())
    {
        out << sp << nl << "public void" << nl << "__write(IceInternal.BasicStream __os)";
        out << sb;
        iter = 0;
        for(d = members.begin(); d != members.end(); ++d)
        {
            StringList metaData = (*d)->getMetaData();
            writeMarshalUnmarshalCode(out, package, (*d)->type(), fixKwd((*d)->name()), true, iter, false, metaData);
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
                string memberType = typeToString((*d)->type(), TypeModeMember, package);
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
        for(d = members.begin(); d != members.end(); ++d)
        {
            ostringstream patchParams;
            StringList metaData = (*d)->getMetaData();
            writeMarshalUnmarshalCode(out, package, (*d)->type(), fixKwd((*d)->name()), false, iter, false, metaData,
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
    StringList metaData = p->getMetaData();
    string s = typeToString(p->type(), TypeModeMember, getPackage(contained), metaData);
    Output& out = output();

    out << sp;

    string deprecateReason = getDeprecateReason(p, contained, "member");
    if(!deprecateReason.empty())
    {
        out << nl << "/**";
        out << nl << " * @deprecated " << deprecateReason;
        out << nl << " **/";
    }
    out << nl << "public " << s << ' ' << name << ';';
}

void
Slice::Gen::TypesVisitor::visitEnum(const EnumPtr& p)
{
    string name = fixKwd(p->name());
    string absolute = getAbsolute(p);
    EnumeratorList enumerators = p->getEnumerators();
    EnumeratorList::const_iterator en;
    size_t sz = enumerators.size();

    if(!open(absolute))
    {
        return;
    }

    Output& out = output();

    out << sp;

    string deprecateReason = getDeprecateReason(p, 0, "type");
    if(!deprecateReason.empty())
    {
        out << nl << "/**";
        out << nl << " * @deprecated " << deprecateReason;
        out << nl << " **/";
    }

    out << nl << "public final class " << name;
    out << sb;
    out << nl << "private static " << name << "[] __values = new " << name << "[" << sz << "];";
    out << nl << "private int __value;";
    out << sp;
    int n;
    for(en = enumerators.begin(), n = 0; en != enumerators.end(); ++en, ++n)
    {
        string member = fixKwd((*en)->name());
        out << nl << "public static final int _" << member << " = " << n << ';';
        out << nl << "public static final " << name << ' ' << fixKwd(member)
            << " = new " << name << "(_" << member << ");";
    }

    out << sp << nl << "public static " << name << nl << "convert(int val)";
    out << sb;
    out << nl << "if(IceUtil.Debug.ASSERT)";
    out << sb;
    out << nl << "IceUtil.Debug.Assert(val < " << sz << ");";
    out << eb;
    out << nl << "return __values[val];";
    out << eb;

    out << sp << nl << "public int" << nl << "value()";
    out << sb;
    out << nl << "return __value;";
    out << eb;

    out << sp << nl << "public String" << nl << "toString()";
    out << sb;
    out << nl << "return __T[__value];";
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
        out << nl << "if(__v < 0 || __v >= " << sz << ')';
        out << sb;
        out << nl << "throw new Ice.MarshalException();";
        out << eb;
        out << nl << "return " << name << ".convert(__v);";
        out << eb;
    }

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

    out << eb;
    close();
}

void
Slice::Gen::TypesVisitor::visitConst(const ConstPtr& p)
{
    string name = fixKwd(p->name());
    string package = getPackage(p);
    string absolute = getAbsolute(p);
    TypePtr type = p->type();

    if(!open(absolute))
    {
        return;
    }
    Output& out = output();
    out << sp << nl << "public interface " << name;
    out << sb;
    out << nl << typeToString(type, TypeModeIn, package) << " value = ";

    BuiltinPtr bp;
    EnumPtr ep;
    if(bp = BuiltinPtr::dynamicCast(type))
    {
        switch(bp->kind())
        {
            case Builtin::KindString:
            {
                out << "\"";

                const string val = p->value();
                for(string::const_iterator c = val.begin(); c != val.end(); ++c)
                {
                    if(isascii(*c) && isprint(*c))
                    {
                        switch(*c)
                        {
                            case '\\':
                            case '"':
                            {
                                out << "\\";
                                break;
                            }
                        }
                        out << *c;
                    }
                    else
                    {
                        switch(*c)
                        {
                            case '\r':
                            {
                                out << "\\r";
                                break;
                            }
                            case '\n':
                            {
                                out << "\\n";
                                break;
                            }
                            default:
                            {
                                unsigned char uc = *c;
                                ostringstream s;
                                s << "\\u";
                                s.flags(ios_base::hex);
                                s.width(4);
                                s.fill('0');
                                s << static_cast<unsigned>(uc);
                                out << s.str();
                                break;
                            }
                        }
                    }
                }

                out << "\"";
                break;
            }
            case Builtin::KindByte:
            {
                int i = atoi(p->value().c_str());
                if(i > 127)
                {
                    i -= 256;
                }
                out << i; // Slice byte runs from 0-255, Java byte runs from -128 - 127.
                break;
            }
            case Builtin::KindLong:
            {
                out << p->value() << "L"; // Need to append "L" modifier for long constants.
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
        string val = p->value();
        string::size_type pos = val.rfind(':');
        if(pos != string::npos)
        {
            val.erase(0, pos + 1);
        }
        out << getAbsolute(ep, package) << '.' << fixKwd(val);
    }
    else
    {
        out << p->value();
    }
    out << ';' << eb;
    close();
}

Slice::Gen::HolderVisitor::HolderVisitor(const string& dir) :
    JavaVisitor(dir)
{
}

bool
Slice::Gen::HolderVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    ClassDeclPtr decl = p->declaration();
    writeHolder(decl);

    string name = p->name();
    string absolute = getAbsolute(p, "", "", "PrxHolder");

    if(open(absolute))
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
    string name = contained->name();
    string absolute = getAbsolute(contained, "", "", "Holder");

    if(open(absolute))
    {
        Output& out = output();
        string typeS = typeToString(p, TypeModeIn, getPackage(contained));
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

Slice::Gen::HelperVisitor::HelperVisitor(const string& dir) :
    JavaVisitor(dir)
{
}

bool
Slice::Gen::HelperVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    //
    // Proxy helper
    //
    string name = p->name();
    string scoped = p->scoped();
    ClassList bases = p->bases();
    string package = getPackage(p);
    string absolute = getAbsolute(p);

    if(!open(getAbsolute(p, "", "", "PrxHelper")))
    {
        return false;
    }

    Output& out = output();

    //
    // A proxy helper class serves two purposes: it implements the
    // proxy interface, and provides static helper methods for use
    // by applications (e.g., checkedCast, etc.)
    //
    out << sp << nl << "public final class " << name << "PrxHelper extends Ice.ObjectPrxHelperBase implements " << name
        << "Prx";

    out << sb;

    OperationList ops = p->allOperations();

    OperationList::const_iterator r;
    for(r = ops.begin(); r != ops.end(); ++r)
    {
        OperationPtr op = *r;
        StringList opMetaData = op->getMetaData();
        ContainerPtr container = op->container();
        ClassDefPtr cl = ClassDefPtr::dynamicCast(container);
        string opName = fixKwd(op->name());
        TypePtr ret = op->returnType();
        string retS = typeToString(ret, TypeModeReturn, package, op->getMetaData());
        int iter;

        vector<string> params = getParams(op, package);
        vector<string> args = getArgs(op);

        ParamDeclList inParams;
        ParamDeclList outParams;
        ParamDeclList paramList = op->parameters();
        ParamDeclList::const_iterator pli;
        for(pli = paramList.begin(); pli != paramList.end(); ++pli)
        {
            if((*pli)->isOutParam())
            {
                outParams.push_back(*pli);
            }
            else
            {
                inParams.push_back(*pli);
            }
        }

        ExceptionList throws = op->throws();
        throws.sort();
        throws.unique();

        //
        // Write two versions of the operation - with and without a
        // context parameter
        //
        out << sp;
        out << nl << "public " << retS << nl << opName << spar << params << epar;
        writeThrowsClause(package, throws);
        out << sb;
        out << nl;
        if(ret)
        {
            out << "return ";
        }
        out << opName << spar << args << "null, false" << epar << ';';
        out << eb;

        out << sp;
        out << nl << "public " << retS << nl << opName << spar << params << "java.util.Hashtable __ctx" << epar;
        writeThrowsClause(package, throws);
        out << sb;
        out << nl;
        if(ret)
        {
            out << "return ";
        }
        out << opName << spar << args << "__ctx, true" << epar << ';';
        out << eb;

        out << sp;
        out << nl << "public " << retS << nl << opName << spar << params 
            << "java.util.Hashtable __ctx, boolean __explicitCtx" << epar;
        writeThrowsClause(package, throws);
        out << sb;
        out << nl << "if(__explicitCtx && __ctx == null)";
        out << sb;
        out << nl << "__ctx = _emptyContext;";
        out << eb;
        out << nl << "int __cnt = 0;";
        out << nl << "while(true)";
        out << sb;
        out << nl << "Ice.Connection __connection = null;";
        out << nl << "try";
        out << sb;
        if(op->returnsData())
        {
            out << nl << "__checkTwowayOnly(\"" << opName << "\");";
        }
        out << nl << "__connection = ice_getConnection();";
        out << nl << "IceInternal.Outgoing __og = __connection.getOutgoing(_reference, \"" << op->name() << "\", "
            << sliceModeToIceMode(op->sendMode()) << ", __ctx);";
        out << nl << "try";
        out << sb;
        if(!inParams.empty())
        {
            out << nl << "try";
            out << sb;
            out << nl << "IceInternal.BasicStream __os = __og.stream();";
            iter = 0;
            for(pli = inParams.begin(); pli != inParams.end(); ++pli)
            {
                writeMarshalUnmarshalCode(out, package, (*pli)->type(), fixKwd((*pli)->name()), true, iter, false,
                                          (*pli)->getMetaData());
            }
            out << eb;
            out << nl << "catch(Ice.LocalException __ex)";
            out << sb;
            out << nl << "__og.abort(__ex);";
            out << eb;
        }
        out << nl << "boolean __ok = __og.invoke();";
        out << nl << "try";
        out << sb;
        out << nl << "IceInternal.BasicStream __is = __og.stream();";
        out << nl << "if(!__ok)";
        out << sb;
        out << nl << "try";
        out << sb;
        out << nl << "__is.throwException();";
        out << eb;
        for(ExceptionList::const_iterator t = throws.begin(); t != throws.end(); ++t)
        {
            out << nl << "catch(" << getAbsolute(*t, package) << " __ex)";
            out << sb;
            out << nl << "throw __ex;";
            out << eb;
        }
        out << nl << "catch(Ice.UserException __ex)";
        out << sb;
        out << nl << "Ice.UnknownUserException __uex = new Ice.UnknownUserException();";
        out << nl << "__uex.unknown = __ex.ice_name();";
        out << nl << "throw __uex;";
        out << eb;
        out << eb;
        for(pli = outParams.begin(); pli != outParams.end(); ++pli)
        {
            writeMarshalUnmarshalCode(out, package, (*pli)->type(), fixKwd((*pli)->name()), false, iter, true,
                                      (*pli)->getMetaData());
        }
        if(ret)
        {
            out << nl << retS << " __ret;";
            writeMarshalUnmarshalCode(out, package, ret, "__ret", false, iter, false, opMetaData);
        }
        if(ret)
        {
            out << nl << "return __ret;";
        }
        out << eb;
        out << nl << "catch(Ice.LocalException __ex)";
        out << sb;
        out << nl << "throw new IceInternal.LocalExceptionWrapper(__ex, false);";
        out << eb;
        out << eb;
        out << nl << "finally";
        out << sb;
        out << nl << "__connection.reclaimOutgoing(__og);";
        out << eb;
        if(!ret)
        {
            out << nl << "return ;";
        }
        out << eb;
        out << nl << "catch(IceInternal.LocalExceptionWrapper __ex)";
        out << sb;
        if(op->mode() == Operation::Idempotent || op->mode() == Operation::Nonmutating)
        {
            out << nl << "__cnt = __handleExceptionWrapperRelaxed(__connection, __ex, __cnt);";
        }
        else
        {
            out << nl << "__handleExceptionWrapper(__connection, __ex);";
        }
        out << eb;
        out << nl << "catch(Ice.LocalException __ex)";
        out << sb;
        out << nl << "__cnt = __handleException(__connection, __ex, __cnt);";
        out << eb;
        out << eb;
        out << eb;
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

    out << sp << nl << "public static " << name << "Prx" << nl << "checkedCast(Ice.ObjectPrx b, java.util.Hashtable ctx)";
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
    out << nl << "if(b.ice_isA(\"" << scoped << "\", ctx))";
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
    out << nl << "Ice.ObjectPrx bb = b.ice_facet(f);";
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

    out << sp << nl << "public static " << name << "Prx"
        << nl << "checkedCast(Ice.ObjectPrx b, String f, java.util.Hashtable ctx)";
    out << sb;
    out << nl << name << "Prx d = null;";
    out << nl << "if(b != null)";
    out << sb;
    out << nl << "Ice.ObjectPrx bb = b.ice_facet(f);";
    out << nl << "try";
    out << sb;
    out << nl << "if(bb.ice_isA(\"" << scoped << "\", ctx))";
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
    out << nl << "Ice.ObjectPrx bb = b.ice_facet(f);";
    out << nl << name << "PrxHelper h = new " << name << "PrxHelper();";
    out << nl << "h.__copyFrom(bb);";
    out << nl << "d = h;";
    out << eb;
    out << nl << "return d;";
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

bool
Slice::Gen::HelperVisitor::visitStructStart(const StructPtr& p)
{
    return false;
}

void
Slice::Gen::HelperVisitor::visitSequence(const SequencePtr& p)
{
    //
    // Don't generate helper for a sequence of a local type.
    //
    if(p->isLocal())
    {
        return;
    }

    string name = p->name();
    string absolute = getAbsolute(p);
    string helper = getAbsolute(p, "", "", "Helper");
    string package = getPackage(p);
    string typeS = typeToString(p, TypeModeIn, package);

    if(open(helper))
    {
        Output& out = output();
        int iter;

        out << sp << nl << "public final class " << name << "Helper";
        out << sb;

        out << nl << "public static void" << nl << "write(IceInternal.BasicStream __os, " << typeS << " __v)";
        out << sb;
        iter = 0;
        writeSequenceMarshalUnmarshalCode(out, package, p, "__v", true, iter, false);
        out << eb;

        out << sp << nl << "public static " << typeS << nl << "read(IceInternal.BasicStream __is)";
        out << sb;
        out << nl << typeS << " __v;";
        iter = 0;
        writeSequenceMarshalUnmarshalCode(out, package, p, "__v", false, iter, false);
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
    // Don't generate helper for a dictionary containing a local type.
    //
    if(p->isLocal())
    {
        return;
    }

    TypePtr key = p->keyType();
    TypePtr value = p->valueType();

    string absolute = getAbsolute(p);
    string helper = getAbsolute(p, "", "", "Helper");

    if(open(helper))
    {
        Output& out = output();
        string name = p->name();
        string package = getPackage(p);
        string keyS = typeToString(key, TypeModeIn, package);
        string valueS = typeToString(value, TypeModeIn, package);
        int iter;
        int i;

        out << sp << nl << "public final class " << name << "Helper";
        out << sb;

        out << nl << "public static void" << nl << "write(IceInternal.BasicStream __os, " << "java.util.Hashtable __v)";
        out << sb;
        out << nl << "if(__v == null)";
        out << sb;
        out << nl << "__os.writeSize(0);";
        out << eb;
        out << nl << "else";
        out << sb;
        out << nl << "__os.writeSize(__v.size());";
        out << nl << "java.util.Enumeration __i = __v.keys();";
        out << nl << "while(__i.hasMoreElements())";
        out << sb;
        out << nl << "java.lang.Object key = __i.nextElement();";
        out << nl << "java.lang.Object value = __v.get(key);";
        iter = 0;
        for(i = 0; i < 2; i++)
        {
            string val;
            string arg;
            TypePtr type;
            if(i == 0)
            {
                arg = "key";
                type = key;
            }
            else
            {
                arg = "value";
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
                    case Builtin::KindObjectProxy:
                    {
                        break;
                    }

                    case Builtin::KindObject:
                    case Builtin::KindLocalObject:
                    {
                        assert(false);
                        break;
                    }
                }
            }

            if(val.empty())
            {
                val = "((" + typeToString(type, TypeModeIn, package) + ')' + arg + ')';
            }
            writeMarshalUnmarshalCode(out, package, type, val, true, iter, false);
        }
        out << eb;
        out << eb;
        out << eb;

        out << sp << nl << "public static java.util.Hashtable";
        out << nl << "read(IceInternal.BasicStream __is)";
        out << sb;
        //
        // JDK 1.1 raises IllegalArgumentException if you pass 0 to the Hashtable constructor.
        //
        out << nl << "int __sz = __is.readSize();";
        out << nl << "java.util.Hashtable __r = new java.util.Hashtable(__sz == 0 ? 1 : __sz);";
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
                    case Builtin::KindObjectProxy:
                    {
                        out << nl << "Ice.ObjectPrx " << arg << " = __is.readProxy();";
                        break;
                    }
                    case Builtin::KindObject:
                    case Builtin::KindLocalObject:
                    {
                        assert(false);
                        break;
                    }
                }
            }
            else
            {
                string s = typeToString(type, TypeModeIn, package);
                out << nl << s << ' ' << arg << ';';
                writeMarshalUnmarshalCode(out, package, type, arg, false, iter, false);
            }
        }
        out << nl << "__r.put(__key, __value);";
        out << eb;
        out << nl << "return __r;";
        out << eb;

        out << eb;
        close();
    }
}

void
Slice::Gen::HelperVisitor::visitEnum(const EnumPtr& p)
{
}

Slice::Gen::ProxyVisitor::ProxyVisitor(const string& dir) :
    JavaVisitor(dir)
{
}

bool
Slice::Gen::ProxyVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    string name = p->name();
    ClassList bases = p->bases();
    string package = getPackage(p);
    string absolute = getAbsolute(p, "", "", "Prx");

    if(!open(absolute))
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
            out << getAbsolute(*q, package, "", "Prx");
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
    string package = getPackage(cl);

    Output& out = output();

    TypePtr ret = p->returnType();
    string retS = typeToString(ret, TypeModeReturn, package, p->getMetaData());
    vector<string> params = getParams(p, package);
    ExceptionList throws = p->throws();
    throws.sort();
    throws.unique();

    string deprecateReason = getDeprecateReason(p, cl, "type");

    //
    // Write two versions of the operation - with and without a
    // context parameter.
    //
    out << sp;
    if(!deprecateReason.empty())
    {
        out << nl << "/**";
        out << nl << " * @deprecated " << deprecateReason;
        out << nl << " **/";
    }
    out << nl << "public " << retS << ' ' << name << spar << params << epar;
    writeThrowsClause(package, throws);
    out << ';';
    if(!deprecateReason.empty())
    {
        out << nl << "/**";
        out << nl << " * @deprecated " << deprecateReason;
        out << nl << " **/";
    }
    out << nl << "public " << retS << ' ' << name << spar << params << "java.util.Hashtable __ctx" << epar;
    writeThrowsClause(package, throws);
    out << ';';
}

Slice::Gen::DispatcherVisitor::DispatcherVisitor(const string& dir) :
    JavaVisitor(dir)
{
}

bool
Slice::Gen::DispatcherVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    if(!p->isInterface())
    {
        return false;
    }

    string name = p->name();
    ClassList bases = p->bases();
    string absolute = getAbsolute(p, "", "_", "Disp");

    if(!open(absolute))
    {
        return false;
    }

    Output& out = output();

    out << sp << nl << "public abstract class _" << name << "Disp extends Ice.ObjectImpl implements " << fixKwd(name);
    out << sb;

    out << sp << nl << "protected void" << nl << "ice_copyStateFrom(Ice.Object __obj)";
    out.inc();
    out << nl << "throws IceUtil.CloneException";
    out.dec();
    out << sb;
    out << nl << "throw new IceUtil.CloneException();";
    out << eb;

    writeDispatch(out, p);

    out << eb;
    close();

    return false;
}

Slice::Gen::BaseImplVisitor::BaseImplVisitor(const string& dir) :
    JavaVisitor(dir)
{
}

void
Slice::Gen::BaseImplVisitor::writeDecl(Output& out, const string& package, const string& name, const TypePtr& type,
                                       const StringList& metaData)
{
    out << nl << typeToString(type, TypeModeIn, package, metaData) << ' ' << name;

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
            out << " = " << getAbsolute(en, package) << '.' << fixKwd(enumerators.front()->name());
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
            case Builtin::KindObjectProxy:
            case Builtin::KindLocalObject:
            {
                out << nl << "return null;";
                break;
            }
            case Builtin::KindObject:
            {
                cerr << "Ice objects cannot be returned by value in IceE." << endl;
                break;
            }
        }
        return;
    }

    out << nl << "return null;";
}

void
Slice::Gen::BaseImplVisitor::writeOperation(Output& out, const string& package, const OperationPtr& op, bool local)
{
    string opName = op->name();

    TypePtr ret = op->returnType();
    StringList opMetaData = op->getMetaData();
    string retS = typeToString(ret, TypeModeReturn, package, opMetaData);
    vector<string> params = getParams(op, package);

    ContainerPtr container = op->container();
    ClassDefPtr cl = ClassDefPtr::dynamicCast(container);

    out << sp << nl << "public " << retS << nl << fixKwd(opName) << spar << params;
    if(!local)
    {
        out << "Ice.Current __current";
    }
    out << epar;

    ExceptionList throws = op->throws();
    throws.sort();
    throws.unique();

    writeThrowsClause(package, throws);

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

Slice::Gen::ImplVisitor::ImplVisitor(const string& dir) :
    BaseImplVisitor(dir)
{
}

bool
Slice::Gen::ImplVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    if(!p->isAbstract())
    {
        return false;
    }

    string name = p->name();
    ClassList bases = p->bases();
    string package = getPackage(p);
    string absolute = getAbsolute(p, "", "", "I");

    if(!open(absolute))
    {
        return false;
    }

    Output& out = output();

    out << sp << nl << "public final class " << name << 'I';
    if(p->isInterface())
    {
        out << " extends _" << name << "Disp";
    }
    else
    {
        out << " extends " << fixKwd(name);
    }
    out << sb;

    out << nl << "public" << nl << name << "I()";
    out << sb;
    out << eb;

    OperationList ops = p->allOperations();

    OperationList::const_iterator r;
    for(r = ops.begin(); r != ops.end(); ++r)
    {
        writeOperation(out, package, *r, false);
    }

    out << eb;
    close();

    return false;
}

Slice::Gen::ImplTieVisitor::ImplTieVisitor(const string& dir) :
    BaseImplVisitor(dir)
{
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
    string package = getPackage(p);
    string absolute = getAbsolute(p, "", "", "I");

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
        out << " extends ";
        if(bases.front()->isAbstract())
        {
            out << bases.front()->name() << 'I';
        }
        else
        {
            out << fixKwd(bases.front()->name());
        }
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
            out << nl << " * Implemented by " << bases.front()->name() << 'I';
            out << nl << " *";
            writeOperation(out, package, *r, false);
            out << sp;
            out << nl << "*/";
        }
        else
        {
            writeOperation(out, package, *r, false);
        }
    }

    out << eb;
    close();

    return false;
}
