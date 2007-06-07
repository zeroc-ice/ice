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
#include <Slice/Checksum.h>
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

Slice::JavaVisitor::JavaVisitor(const string& dir) :
    JavaGenerator(dir)
{
}

Slice::JavaVisitor::~JavaVisitor()
{
}

vector<string>
Slice::JavaVisitor::getParams(const OperationPtr& op, const string& package, bool final)
{
    vector<string> params;

    ParamDeclList paramList = op->parameters();
    for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
        StringList metaData = (*q)->getMetaData();
        string typeString = typeToString((*q)->type(), (*q)->isOutParam() ? TypeModeOut : TypeModeIn, package,
                                         metaData);
        if(final)
        {
            typeString = "final " + typeString;
        }
        params.push_back(typeString + ' ' + fixKwd((*q)->name()));
    }

    return params;
}

vector<string>
Slice::JavaVisitor::getParamsAsync(const OperationPtr& op, const string& package, bool amd)
{
    vector<string> params;

    string name = op->name();
    ContainerPtr container = op->container();
    ClassDefPtr cl = ClassDefPtr::dynamicCast(container);
    string classNameAsync = getAbsolute(cl, package, amd ? "AMD_" : "AMI_", '_' + name);
    params.push_back(classNameAsync + " __cb");

    ParamDeclList paramList = op->parameters();
    for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
        if(!(*q)->isOutParam())
        {
            StringList metaData = (*q)->getMetaData();
            string typeString = typeToString((*q)->type(), TypeModeIn, package, metaData);
            params.push_back(typeString + ' ' + fixKwd((*q)->name()));
        }
    }

    return params;
}

vector<string>
Slice::JavaVisitor::getParamsAsyncCB(const OperationPtr& op, const string& package)
{
    vector<string> params;

    TypePtr ret = op->returnType();
    if(ret)
    {
        string retS = typeToString(ret, TypeModeIn, package, op->getMetaData());
        params.push_back(retS + " __ret");
    }

    ParamDeclList paramList = op->parameters();
    for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
        if((*q)->isOutParam())
        {
            string typeString = typeToString((*q)->type(), TypeModeIn, package, (*q)->getMetaData());
            params.push_back(typeString + ' ' + fixKwd((*q)->name()));
        }
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

vector<string>
Slice::JavaVisitor::getArgsAsync(const OperationPtr& op)
{
    vector<string> args;

    args.push_back("__cb");

    ParamDeclList paramList = op->parameters();
    for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
        if(!(*q)->isOutParam())
        {
            args.push_back(fixKwd((*q)->name()));
        }
    }

    return args;
}

vector<string>
Slice::JavaVisitor::getArgsAsyncCB(const OperationPtr& op)
{
    vector<string> args;

    TypePtr ret = op->returnType();
    if(ret)
    {
        BuiltinPtr builtin = BuiltinPtr::dynamicCast(ret);
        if((builtin && builtin->kind() == Builtin::KindObject) || ClassDeclPtr::dynamicCast(ret))
        {
            args.push_back("__ret.value");
        }
        else
        {
            args.push_back("__ret");
        }
    }

    ParamDeclList paramList = op->parameters();
    for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
        if((*q)->isOutParam())
        {
            BuiltinPtr builtin = BuiltinPtr::dynamicCast((*q)->type());
            if((builtin && builtin->kind() == Builtin::KindObject) || ClassDeclPtr::dynamicCast((*q)->type()))
            {
                args.push_back(fixKwd((*q)->name()) + ".value");
            }
            else
            {
                args.push_back(fixKwd((*q)->name()));
            }
        }
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
Slice::JavaVisitor::writeDelegateThrowsClause(const string& package, const ExceptionList& throws)
{
    Output& out = output();
    out.inc();
    out << nl << "throws ";
    out.useCurrentPosAsIndent();
    out << "IceInternal.LocalExceptionWrapper";

    ExceptionList::const_iterator r;
    for(r = throws.begin(); r != throws.end(); ++r)
    {
        out << "," << nl;
        out << getAbsolute(*r, package);
    }
    out.restoreIndent();
    out.dec();
}

void
Slice::JavaVisitor::writeHashCode(Output& out, const TypePtr& type, const string& name, int& iter,
                                  const StringList& metaData)
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
        bool customType = hasTypeMetaData(seq, metaData);

        out << nl << "if(" << name << " != null)";
        out << sb;
        if(customType)
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
        out << eb;
        return;
    }

    ConstructedPtr constructed = ConstructedPtr::dynamicCast(type);
    assert(constructed);
    out << nl << "__h = 5 * __h + " << name << ".hashCode();";
}

void
Slice::JavaVisitor::writeDispatchAndMarshalling(Output& out, const ClassDefPtr& p, bool stream)
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
    out << nl << "return java.util.Arrays.binarySearch(__ids, s) >= 0;";
    out << eb;

    out << sp << nl << "public boolean" << nl << "ice_isA(String s, Ice.Current __current)";
    out << sb;
    out << nl << "return java.util.Arrays.binarySearch(__ids, s) >= 0;";
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

        bool amd = cl->hasMetaData("amd") || op->hasMetaData("amd");

        vector<string> params;
        vector<string> args;
        TypePtr ret;

        if(amd)
        {
            opName += "_async";
            params = getParamsAsync(op, package, true);
            args = getArgsAsync(op);
        }
        else
        {
            opName = fixKwd(opName);
            ret = op->returnType();
            params = getParams(op, package);
            args = getArgs(op);
        }

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
                out << "return ";
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
            << " __obj, IceInternal.Incoming __inS, Ice.Current __current)";
        out << sb;

        bool amd = cl->hasMetaData("amd") || op->hasMetaData("amd");
        if(!amd)
        {
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
                out << nl << "IceInternal.BasicStream __is = __inS.is();";
            }
            if(!outParams.empty() || ret || !throws.empty())
            {
                out << nl << "IceInternal.BasicStream __os = __inS.os();";
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
            if(op->sendsClasses())
            {
                out << nl << "__is.readPendingObjects();";
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
            if(op->returnsClasses())
            {
                out << nl << "__os.writePendingObjects();";
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
        else
        {
            ParamDeclList inParams;
            ParamDeclList paramList = op->parameters();
            ParamDeclList::const_iterator pli;
            for(pli = paramList.begin(); pli != paramList.end(); ++pli)
            {
                if(!(*pli)->isOutParam())
                {
                    inParams.push_back(*pli);
                }
            }
            
            int iter;
            
            out << nl << "__checkMode(" << sliceModeToIceMode(op->mode()) << ", __current.mode);";

            if(!inParams.empty())
            {
                out << nl << "IceInternal.BasicStream __is = __inS.is();";
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
                    writeMarshalUnmarshalCode(out, package, paramType, paramName, false, iter, true, metaData,
                                              string());
                }
                else
                {
                    out << nl << typeS << ' ' << paramName << ';';
                    writeMarshalUnmarshalCode(out, package, paramType, paramName, false, iter, false, metaData);
                }
            }
            if(op->sendsClasses())
            {
                out << nl << "__is.readPendingObjects();";
            }
            
            //
            // Call on the servant.
            //
            string classNameAMD = "AMD_" + p->name();
            out << nl << classNameAMD << '_' << opName << " __cb = new _" << classNameAMD << '_' << opName
                << "(__inS);";
            out << nl << "try";
            out << sb;
            out << nl << "__obj." << (amd ? opName + "_async" : fixKwd(opName)) << (amd ? "(__cb, " : "(");
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
            out << "__current);";
            out << eb;
            out << nl << "catch(java.lang.Exception ex)";
            out << sb;
            out << nl << "__cb.ice_exception(ex);";
            out << eb;
            out << nl << "return Ice.DispatchStatus.DispatchAsync;";

            out << eb;
        }
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
        out << nl << "int pos = java.util.Arrays.binarySearch(__all, __current.operation);";
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
        out << sp << nl << "assert(false);";
        out << nl << "throw new Ice.OperationNotExistException(__current.id, __current.facet, __current.operation);";
        out << eb;


        //
        // Check if we need to generate ice_operationAttributes()
        //
        
        StringList freezeWriteOpNames;
        for(OperationList::iterator r = allOps.begin(); r != allOps.end(); ++r)
        {
            ClassDefPtr classDef = ClassDefPtr::dynamicCast((*r)->container());
            assert(classDef != 0);
            
            if((*r)->hasMetaData("freeze:write") || 
               (classDef->hasMetaData("freeze:write") && !(*r)->hasMetaData("freeze:read")))
            {
                freezeWriteOpNames.push_back((*r)->name());
            }
        }

        if(!freezeWriteOpNames.empty())
        {
            freezeWriteOpNames.sort();
            
            StringList::iterator q;

            out << sp << nl << "private final static String[] __freezeWriteOperations =";
            out << sb;
            q = freezeWriteOpNames.begin();
            while(q != freezeWriteOpNames.end())
            {
                out << nl << '"' << *q << '"';
                if(++q != freezeWriteOpNames.end())
                {
                    out << ',';
                }
            }
            out << eb << ';';

            out << sp << nl << "public int ice_operationAttributes(String operation)";
            out << sb;
            out << nl << "return (java.util.Arrays.binarySearch(__freezeWriteOperations, operation) >= 0) ? 1 : 0;";
            out << eb;
        }
    }

    int iter;
    DataMemberList members = p->dataMembers();
    DataMemberList::const_iterator d;

    out << sp << nl << "public void" << nl << "__write(IceInternal.BasicStream __os)";
    out << sb;
    out << nl << "__os.writeTypeId(ice_staticId());";
    out << nl << "__os.startWriteSlice();";
    iter = 0;
    for(d = members.begin(); d != members.end(); ++d)
    {
        StringList metaData = (*d)->getMetaData();
        writeMarshalUnmarshalCode(out, package, (*d)->type(), fixKwd((*d)->name()), true, iter, false, metaData);
    }
    out << nl << "__os.endWriteSlice();";
    out << nl << "super.__write(__os);";
    out << eb;

    DataMemberList allClassMembers = p->allClassDataMembers();
    if(allClassMembers.size() != 0)
    {
        out << sp << nl << "private class Patcher implements IceInternal.Patcher";
        if(stream)
        {
            out << ", Ice.ReadObjectCallback";
        }
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
        out << nl << "try";
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
        out << nl << "catch(ClassCastException ex)";
        out << sb;
        out << nl << "Ice.UnexpectedObjectException _e = new Ice.UnexpectedObjectException();";
        out << nl << "_e.type = v.ice_id();";
        out << nl << "_e.expectedType = type();";
        out << nl << "throw _e;";
        out << eb;
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

        if(stream)
        {
            out << sp << nl << "public void" << nl << "invoke(Ice.Object v)";
            out << sb;
            out << nl << "patch(v);";
            out << eb;
        }

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
    out << nl << "__is.readTypeId();";
    out << eb;
    out << nl << "__is.startReadSlice();";
    iter = 0;
    DataMemberList classMembers = p->classDataMembers();
    size_t classMemberCount = allClassMembers.size() - classMembers.size();
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
        writeMarshalUnmarshalCode(out, package, (*d)->type(), fixKwd((*d)->name()), false, iter, false, metaData,
                                  patchParams.str());
    }
    out << nl << "__is.endReadSlice();";
    out << nl << "super.__read(__is, true);";
    out << eb;

    if(stream)
    {
        out << sp << nl << "public void" << nl << "__write(Ice.OutputStream __outS)";
        out << sb;
        out << nl << "__outS.writeTypeId(ice_staticId());";
        out << nl << "__outS.startSlice();";
        iter = 0;
        for(d = members.begin(); d != members.end(); ++d)
        {
            StringList metaData = (*d)->getMetaData();
            writeStreamMarshalUnmarshalCode(out, package, (*d)->type(), fixKwd((*d)->name()), true, iter, false,
                                            metaData);
        }
        out << nl << "__outS.endSlice();";
        out << nl << "super.__write(__outS);";
        out << eb;

        out << sp << nl << "public void" << nl << "__read(Ice.InputStream __inS, boolean __rid)";
        out << sb;
        out << nl << "if(__rid)";
        out << sb;
        out << nl << "__inS.readTypeId();";
        out << eb;
        out << nl << "__inS.startSlice();";
        iter = 0;
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
            writeStreamMarshalUnmarshalCode(out, package, (*d)->type(), fixKwd((*d)->name()), false, iter, false,
                                            metaData, patchParams.str());
        }
        out << nl << "__inS.endSlice();";
        out << nl << "super.__read(__inS, true);";
        out << eb;
    }
    else
    {
        //
        // Emit placeholder functions to catch errors.
        //
        string scoped = p->scoped();
        out << sp << nl << "public void" << nl << "__write(Ice.OutputStream __outS)";
        out << sb;
        out << nl << "Ice.MarshalException ex = new Ice.MarshalException();";
        out << nl << "ex.reason = \"type " << scoped.substr(2) << " was not generated with stream support\";";
        out << nl << "throw ex;";
        out << eb;

        out << sp << nl << "public void" << nl << "__read(Ice.InputStream __inS, boolean __rid)";
        out << sb;
        out << nl << "Ice.MarshalException ex = new Ice.MarshalException();";
        out << nl << "ex.reason = \"type " << scoped.substr(2) << " was not generated with stream support\";";
        out << nl << "throw ex;";
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
Slice::Gen::generate(const UnitPtr& p, bool stream)
{
    JavaGenerator::validateMetaData(p);

    OpsVisitor opsVisitor(_dir);
    p->visit(&opsVisitor, false);

    PackageVisitor packageVisitor(_dir);
    p->visit(&packageVisitor, false);

    TypesVisitor typesVisitor(_dir, stream);
    p->visit(&typesVisitor, false);

    HolderVisitor holderVisitor(_dir, stream);
    p->visit(&holderVisitor, false);

    HelperVisitor helperVisitor(_dir, stream);
    p->visit(&helperVisitor, false);

    ProxyVisitor proxyVisitor(_dir);
    p->visit(&proxyVisitor, false);

    DelegateVisitor delegateVisitor(_dir);
    p->visit(&delegateVisitor, false);

    DelegateMVisitor delegateMVisitor(_dir);
    p->visit(&delegateMVisitor, false);

    DelegateDVisitor delegateDVisitor(_dir);
    p->visit(&delegateDVisitor, false);

    DispatcherVisitor dispatcherVisitor(_dir, stream);
    p->visit(&dispatcherVisitor, false);

    AsyncVisitor asyncVisitor(_dir);
    p->visit(&asyncVisitor, false);
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

void
Slice::Gen::writeChecksumClass(const string& checksumClass, const string& dir, const ChecksumMap& m, bool java2)
{
    //
    // Attempt to open the source file for the checksum class.
    //
    JavaOutput out;
    if(!out.openClass(checksumClass, dir))
    {
        cerr << "can't open class `" << checksumClass << "' for writing: " << strerror(errno) << endl;
        return;
    }

    //
    // Get the class name.
    //
    string className;
    string::size_type pos = checksumClass.rfind('.');
    if(pos == string::npos)
    {
        className = checksumClass;
    }
    else
    {
        className = checksumClass.substr(pos + 1);
    }

    //
    // Emit the class.
    //
    out << sp << nl << "public class " << className;
    out << sb;

    //
    // Use a static initializer to populate the checksum map.
    //
    if(java2)
    {
        out << sp << nl << "public static java.util.Map checksums;";
    }
    else
    {
        out << sp << nl << "public static java.util.Map<String, String> checksums;";
    }
    out << sp << nl << "static";
    out << sb;
    if(java2)
    {
        out << nl << "java.util.Map map = new java.util.HashMap();";
    }
    else
    {
        out << nl << "java.util.Map<String, String> map = new java.util.HashMap<String, String>();";
    }
    for(ChecksumMap::const_iterator p = m.begin(); p != m.end(); ++p)
    {
        out << nl << "map.put(\"" << p->first << "\", \"";
        ostringstream str;
        str.flags(ios_base::hex);
        str.fill('0');
        for(vector<unsigned char>::const_iterator q = p->second.begin(); q != p->second.end(); ++q)
        {
            str << (int)(*q);
        }
        out << str.str() << "\");";
    }
    out << nl << "checksums = java.util.Collections.unmodifiableMap(map);";

    out << eb;
    out << eb;
    out << nl;
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
    ClassList bases = p->bases();
    string package = getPackage(p);
    string opIntfName = "Operations";
    if(noCurrent || p->isLocal())
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

        bool amd = !p->isLocal() && (cl->hasMetaData("amd") || op->hasMetaData("amd"));

        if(amd)
        {
            params = getParamsAsync(op, package, true);
        }
        else
        {
            params = getParams(op, package);
            ret = op->returnType();
        }

        string deprecateReason = getDeprecateReason(*r, p, "operation");

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
        out << nl << retS << ' ' << (amd ? opname + "_async" : fixKwd(opname)) << spar << params;
        if(!noCurrent && !p->isLocal())
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
    if(p->isLocal())
    {
        opIntfName += "NC";
    }

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
            out << " implements " << fixKwd(name) << ", Ice.TieBase";
        }
        else
        {
            out << " extends " << '_' << name << "Disp implements Ice.TieBase";
        }
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

    if(p->isLocal())
    {
        out << sp << nl << "public int" << nl << "ice_hash()";
        out << sb;
        out << nl << "return hashCode();";
        out << eb;

        out << sp << nl << "public java.lang.Object" << nl << "clone()";
        out.inc();
        out << nl << "throws java.lang.CloneNotSupportedException";
        out.dec();
        out << sb;
        out << nl << "return super.clone();";
        out << eb;
    }

    OperationList ops = p->allOperations();
    OperationList::const_iterator r;
    for(r = ops.begin(); r != ops.end(); ++r)
    {
        ContainerPtr container = (*r)->container();
        ClassDefPtr cl = ClassDefPtr::dynamicCast(container);
        bool hasAMD = cl->hasMetaData("amd") || (*r)->hasMetaData("amd");
#if defined(__SUNPRO_CC) && (__SUNPRO_CC==0x550)
        //
        // Work around for Sun CC 5.5 bug #4853566
        //
        string opName;
        if(hasAMD)
        {
            opName = (*r)->name() + "_async";
        }
        else
        {
           opName = fixKwd((*r)->name());
        }
#else   
        string opName = hasAMD ? (*r)->name() + "_async" : fixKwd((*r)->name());
#endif
        TypePtr ret = (*r)->returnType();
        string retS = typeToString(ret, TypeModeReturn, package, (*r)->getMetaData());

        vector<string> params;
        vector<string> args;
        if(hasAMD)
        {
            params = getParamsAsync((*r), package, true);
            args = getArgsAsync(*r);
        }
        else
        {
            params = getParams((*r), package);
            args = getArgs(*r);
        }

        out << sp;
        out << nl << "public " << (hasAMD ? string("void") : retS) << nl << opName << spar << params;
        if(!p->isLocal())
        {
            out << "Ice.Current __current";
        }
        out << epar;

        ExceptionList throws = (*r)->throws();
        throws.sort();
        throws.unique();
        writeThrowsClause(package, throws);
        out << sb;
        out << nl;
        if(ret && !hasAMD)
        {
            out << "return ";
        }
        out << "_ice_delegate." << opName << spar << args;
        if(!p->isLocal())
        {
            out << "__current";
        }
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

Slice::Gen::TypesVisitor::TypesVisitor(const string& dir, bool stream) :
    JavaVisitor(dir), _stream(stream)
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
        if(p->isLocal())
        {
            out << "_" << name << "OperationsNC";
        }
        else
        {
            out << "Ice.Object";
            out << "," << nl << '_' << name;
            out << "Operations, _" << name << "OperationsNC";
        }
      
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
        bool implementsOnNewLine = true;
        if(bases.empty() || bases.front()->isInterface())
        {
            if(p->isLocal())
            {
                implementsOnNewLine = false;
            }
            else
            {
                out << " extends Ice.ObjectImpl";
            }
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
            if(!p->isLocal())
            {
                implements.push_back("_" + name + "Operations");
            }
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
            if(implementsOnNewLine)
            {
                out << nl;
            }

            out << " implements ";
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
    // Default factory for non-abstract classes.
    //
    if(!p->isAbstract() && !p->isLocal())
    {
        out << sp;
        out << nl << "private static class __F implements Ice.ObjectFactory";
        out << sb;
        out << nl << "public Ice.Object" << nl << "create(String type)";
        out << sb;
        out << nl << "assert(type.equals(ice_staticId()));";
        out << nl << "return new " << fixKwd(name) << "();";
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
        writeDispatchAndMarshalling(out, p, _stream);
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
            if(_stream)
            {
                out << ", Ice.ReadObjectCallback";
            }
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
            out << nl << "try";
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
            out << nl << "catch(ClassCastException ex)";
            out << sb;
            out << nl << "Ice.UnexpectedObjectException _e = new Ice.UnexpectedObjectException();";
            out << nl << "_e.type = v.ice_id();";
            out << nl << "_e.expectedType = type();";
            out << nl << "throw _e;";
            out << eb;
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

            if(_stream)
            {
                out << sp << nl << "public void" << nl << "invoke(Ice.Object v)";
                out << sb;
                out << nl << "patch(v);";
                out << eb;
            }

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
        out << nl << "__is.readString();";
        out << eb;
        out << nl << "__is.startReadSlice();";
        iter = 0;
        DataMemberList classMembers = p->classDataMembers();
        size_t classMemberCount = allClassMembers.size() - classMembers.size();
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

        if(_stream)
        {
            out << sp << nl << "public void" << nl << "__write(Ice.OutputStream __outS)";
            out << sb;
            out << nl << "__outS.writeString(\"" << scoped << "\");";
            out << nl << "__outS.startSlice();";
            iter = 0;
            for(d = members.begin(); d != members.end(); ++d)
            {
                StringList metaData = (*d)->getMetaData();
                writeStreamMarshalUnmarshalCode(out, package, (*d)->type(), fixKwd((*d)->name()), true, iter, false,
                                                metaData);
            }
            out << nl << "__outS.endSlice();";
            if(base)
            {
                out << nl << "super.__write(__outS);";
            }
            out << eb;

            out << sp << nl << "public void" << nl << "__read(Ice.InputStream __inS, boolean __rid)";
            out << sb;
            out << nl << "if(__rid)";
            out << sb;
            out << nl << "__inS.readString();";
            out << eb;
            out << nl << "__inS.startSlice();";
            iter = 0;
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
                StringList metaData = (*d)->getMetaData();
                writeStreamMarshalUnmarshalCode(out, package, (*d)->type(), fixKwd((*d)->name()), false, iter, false,
                                                metaData, patchParams.str());
            }
            out << nl << "__inS.endSlice();";
            if(base)
            {
                out << nl << "super.__read(__inS, true);";
            }
            out << eb;
        }
        else
        {
            //
            // Emit placeholder functions to catch errors.
            //
            out << sp << nl << "public void" << nl << "__write(Ice.OutputStream __outS)";
            out << sb;
            out << nl << "Ice.MarshalException ex = new Ice.MarshalException();";
            out << nl << "ex.reason = \"exception " << scoped.substr(2) << " was not generated with stream support\";";
            out << nl << "throw ex;";
            out << eb;

            out << sp << nl << "public void" << nl << "__read(Ice.InputStream __inS, boolean __rid)";
            out << sb;
            out << nl << "Ice.MarshalException ex = new Ice.MarshalException();";
            out << nl << "ex.reason = \"exception " << scoped.substr(2) << " was not generated with stream support\";";
            out << nl << "throw ex;";
            out << eb;
        }

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

    out << nl << "public final class " << name << " implements java.lang.Cloneable";
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
    for(d = members.begin(); d != members.end(); ++d)
    {
        string memberName = fixKwd((*d)->name());
        string memberType = typeToString((*d)->type(), TypeModeMember, package, (*d)->getMetaData());
        paramDecl.push_back(memberType + " " + memberName);
        paramNames.push_back(memberName);
    }

    out << sp << nl << "public " << name << spar << paramDecl << epar;
    out << sb;
    for(vector<string>::const_iterator i = paramNames.begin(); i != paramNames.end(); ++i)
    {
        out << nl << "this." << *i << " = " << *i << ';';
    }
    out << eb;

    out << sp << nl << "public boolean" << nl << "equals(java.lang.Object rhs)";
    out << sb;
    out << nl << "if(this == rhs)";
    out << sb;
    out << nl << "return true;";
    out << eb;
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
            // a Java array does not perform a deep comparison. If the mapped type
            // is not overridden via metadata, we use the helper method
            // java.util.Arrays.equals() to compare native arrays.
            //
            // For all other types, we can use the native equals() method.
            //
            SequencePtr seq = SequencePtr::dynamicCast((*d)->type());
            if(seq)
            {
                if(hasTypeMetaData(seq, (*d)->getMetaData()))
                {
                    out << nl << "if(" << memberName << " != _r." << memberName << " && " << memberName
                        << " != null && !" << memberName << ".equals(_r." << memberName << "))";
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
        StringList metaData = (*d)->getMetaData();
        writeHashCode(out, (*d)->type(), memberName, iter, metaData);
    }
    out << nl << "return __h;";
    out << eb;

    out << sp << nl << "public java.lang.Object" << nl << "clone()";
    out << sb;
    out << nl << "java.lang.Object o = null;";
    out << nl << "try";
    out << sb;
    out << nl << "o = super.clone();";
    out << eb;
    out << nl << "catch(CloneNotSupportedException ex)";
    out << sb;
    out << nl << "assert false; // impossible";
    out << eb;
    out << nl << "return o;";
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
            if(_stream)
            {
                out << ", Ice.ReadObjectCallback";
            }
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
            out << nl << "try";
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
            out << nl << "catch(ClassCastException ex)";
            out << sb;
            out << nl << "Ice.UnexpectedObjectException _e = new Ice.UnexpectedObjectException();";
            out << nl << "_e.type = v.ice_id();";
            out << nl << "_e.expectedType = type();";
            out << nl << "throw _e;";
            out << eb;
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

            if(_stream)
            {
                out << sp << nl << "public void" << nl << "invoke(Ice.Object v)";
                out << sb;
                out << nl << "patch(v);";
                out << eb;
            }

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
            StringList metaData = (*d)->getMetaData();
            writeMarshalUnmarshalCode(out, package, (*d)->type(), fixKwd((*d)->name()), false, iter, false, metaData,
                                      patchParams.str());
        }
        out << eb;

        if(_stream)
        {
            out << sp << nl << "public void" << nl << "ice_write(Ice.OutputStream __outS)";
            out << sb;
            iter = 0;
            for(d = members.begin(); d != members.end(); ++d)
            {
                StringList metaData = (*d)->getMetaData();
                writeStreamMarshalUnmarshalCode(out, package, (*d)->type(), fixKwd((*d)->name()), true, iter, false,
                                                metaData);
            }
            out << eb;

            out << sp << nl << "public void" << nl << "ice_read(Ice.InputStream __inS)";
            out << sb;
            iter = 0;
            classMemberCount = 0;
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
                StringList metaData = (*d)->getMetaData();
                writeStreamMarshalUnmarshalCode(out, package, (*d)->type(), fixKwd((*d)->name()), false, iter, false,
                                                metaData, patchParams.str());
            }
            out << eb;
        }
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
    TypePtr type = p->type();
    string s = typeToString(type, TypeModeMember, getPackage(contained), metaData);
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

    //
    // Getter/Setter.
    //
    if(p->hasMetaData(_getSetMetaData) || contained->hasMetaData(_getSetMetaData))
    {
        string capName = p->name();
        capName[0] = toupper(capName[0]);

        //
        // If container is a class, get all of its operations so that we can check for conflicts.
        //
        OperationList ops;
        string file, line;
        ClassDefPtr cls = ClassDefPtr::dynamicCast(container);
        if(cls)
        {
            ops = cls->allOperations();
            DefinitionContextPtr dc = p->definitionContext();
            file = dc->filename();
            line = p->line();
            if(!validateGetterSetter(ops, "get" + capName, 0, file, line) ||
               !validateGetterSetter(ops, "set" + capName, 1, file, line))
            {
                return;
            }
        }

        //
        // Getter.
        //
        out << sp;
        if(!deprecateReason.empty())
        {
            out << nl << "/**";
            out << nl << " * @deprecated " << deprecateReason;
            out << nl << " **/";
        }
        out << nl << "public " << s;
        out << nl << "get" << capName << "()";
        out << sb;
        out << nl << "return " << name << ';';
        out << eb;

        //
        // Setter.
        //
        out << sp;
        if(!deprecateReason.empty())
        {
            out << nl << "/**";
            out << nl << " * @deprecated " << deprecateReason;
            out << nl << " **/";
        }
        out << nl << "public void";
        out << nl << "set" << capName << '(' << s << " _" << name << ')';
        out << sb;
        out << nl << name << " = _" << name << ';';
        out << eb;

        //
        // Check for bool type.
        //
        BuiltinPtr b = BuiltinPtr::dynamicCast(type);
        if(b && b->kind() == Builtin::KindBool)
        {
            if(cls && !validateGetterSetter(ops, "is" + capName, 0, file, line))
            {
                return;
            }
            out << sp;
            if(!deprecateReason.empty())
            {
                out << nl << "/**";
                out << nl << " * @deprecated " << deprecateReason;
                out << nl << " **/";
            }
            out << nl << "public boolean";
            out << nl << "is" << capName << "()";
            out << sb;
            out << nl << "return " << name << ';';
            out << eb;
        }

        //
        // Check for unmodified sequence type and emit indexing methods.
        //
        SequencePtr seq = SequencePtr::dynamicCast(type);
        if(seq)
        {
            if(!hasTypeMetaData(seq, metaData))
            {
                if(cls &&
                   (!validateGetterSetter(ops, "get" + capName, 1, file, line) ||
                    !validateGetterSetter(ops, "set" + capName, 2, file, line)))
                {
                    return;
                }

                string elem = typeToString(seq->type(), TypeModeMember, getPackage(contained));

                //
                // Indexed getter.
                //
                out << sp;
                if(!deprecateReason.empty())
                {
                    out << nl << "/**";
                    out << nl << " * @deprecated " << deprecateReason;
                    out << nl << " **/";
                }
                out << nl << "public " << elem;
                out << nl << "get" << capName << "(int _index)";
                out << sb;
                out << nl << "return " << name << "[_index];";
                out << eb;

                //
                // Indexed setter.
                //
                out << sp;
                if(!deprecateReason.empty())
                {
                    out << nl << "/**";
                    out << nl << " * @deprecated " << deprecateReason;
                    out << nl << " **/";
                }
                out << nl << "public void";
                out << nl << "set" << capName << "(int _index, " << elem << " _val)";
                out << sb;
                out << nl << name << "[_index] = _val;";
                out << eb;
            }
        }
    }
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

    bool java2 = p->definitionContext()->findMetaData("java:java2") == "java:java2";

    if(java2)
    {
        out << nl << "public final class " << name;
    }
    else
    {
        out << nl << "public enum " << name;
    }
    out << sb;

    if(!java2)
    {
        int n;
        for(en = enumerators.begin(), n = 0; en != enumerators.end(); ++en, ++n)
        {
            if(en != enumerators.begin())
            {
                out << ',';
            }
            out << nl << fixKwd((*en)->name()) << '(' << n << ')';
        }
        out << ';';
        out << sp;
    }

    out << nl << "private static " << name << "[] __values = new " << name << "[" << sz << "];";
    if(!java2)
    {
        out << nl << "static";
        out << sb;
        int n;
        for(en = enumerators.begin(), n = 0; en != enumerators.end(); ++en, ++n)
        {
            out << nl << "__values[" << n << "] = " << fixKwd((*en)->name()) << ';';
        }
        out << eb;
    }
    out << nl << "private int __value;";

    //
    // For backward compatibility, we keep the integer member in the Java5 mapping.
    //
    {
        out << sp;
        int n;
        for(en = enumerators.begin(), n = 0; en != enumerators.end(); ++en, ++n)
        {
            string member = fixKwd((*en)->name());
            out << nl << "public static final int _" << member << " = " << n << ';';
            if(java2)
            {
                out << nl << "public static final " << name << ' ' << fixKwd(member)
                    << " = new " << name << "(_" << member << ");";
            }
        }
    }

    out << sp << nl << "public static " << name << nl << "convert(int val)";
    out << sb;
    out << nl << "assert val < " << sz << ';';
    out << nl << "return __values[val];";
    out << eb;

    out << sp << nl << "public static " << name << nl << "convert(String val)";
    out << sb;
    out << nl << "for(int __i = 0; __i < __values.length; ++__i)";
    out << sb;
    out << nl << "if(__values[__i].toString().equals(val))";
    out << sb;
    out << nl << "return __values[__i];";
    out << eb;
    out << eb;
    out << nl << "assert false;";
    out << nl << "return null;";
    out << eb;

    out << sp << nl << "public int" << nl << "value()";
    out << sb;
    out << nl << "return __value;";
    out << eb;

    if(java2)
    {
        out << sp << nl << "public String" << nl << "toString()";
        out << sb;
        out << nl << "return __T[__value];";
        out << eb;
    }

    out << sp << nl << "private" << nl << name << "(int val)";
    out << sb;
    out << nl << "__value = val;";
    if(java2)
    {
        out << nl << "__values[val] = this;";
    }
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

        if(_stream)
        {
            out << sp << nl << "public void" << nl << "ice_write(Ice.OutputStream __outS)";
            out << sb;
            if(sz <= 0x7f)
            {
                out << nl << "__outS.writeByte((byte)__value);";
            }
            else if(sz <= 0x7fff)
            {
                out << nl << "__outS.writeShort((short)__value);";
            }
            else
            {
                out << nl << "__outS.writeInt(__value);";
            }
            out << eb;

            out << sp << nl << "public static " << name << nl << "ice_read(Ice.InputStream __inS)";
            out << sb;
            if(sz <= 0x7f)
            {
                out << nl << "int __v = __inS.readByte();";
            }
            else if(sz <= 0x7fff)
            {
                out << nl << "int __v = __inS.readShort();";
            }
            else
            {
                out << nl << "int __v = __inS.readInt();";
            }
            out << nl << "if(__v < 0 || __v >= " << sz << ')';
            out << sb;
            out << nl << "throw new Ice.MarshalException();";
            out << eb;
            out << nl << "return " << name << ".convert(__v);";
            out << eb;
        }
    }

    if(java2)
    {
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

bool
Slice::Gen::TypesVisitor::validateGetterSetter(const OperationList& ops, const std::string& name, int numArgs,
                                               const string& file, const string& line)
{
    for(OperationList::const_iterator i = ops.begin(); i != ops.end(); ++i)
    {
        if((*i)->name() == name)
        {
            int numParams = static_cast<int>((*i)->parameters().size());
            if(numArgs >= numParams && numArgs - numParams <= 1)
            {
                cerr << file << ":" << line
                     << ": error: operation `" << name << "' conflicts with getter/setter method" << endl;
                return false;
            }
            break;
        }
    }
    return true;
}

Slice::Gen::HolderVisitor::HolderVisitor(const string& dir, bool stream) :
    JavaVisitor(dir), _stream(stream)
{
}

bool
Slice::Gen::HolderVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    ClassDeclPtr decl = p->declaration();
    writeHolder(decl);

    if(!p->isLocal())
    {
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
        if(!p->isLocal())
        {
            BuiltinPtr builtin = BuiltinPtr::dynamicCast(p);
            if((builtin && builtin->kind() == Builtin::KindObject) || ClassDeclPtr::dynamicCast(p))
            {
                out << sp << nl << "public class Patcher implements IceInternal.Patcher";
                if(_stream)
                {
                    out << ", Ice.ReadObjectCallback";
                }
                out << sb;
                out << nl << "public void";
                out << nl << "patch(Ice.Object v)";
                out << sb;
                out << nl << "try";
                out << sb;
                out << nl << "value = (" << typeS << ")v;";
                out << eb;
                out << nl << "catch(ClassCastException ex)";
                out << sb;
                out << nl << "Ice.UnexpectedObjectException _e = new Ice.UnexpectedObjectException();";
                out << nl << "_e.type = v.ice_id();";
                out << nl << "_e.expectedType = type();";
                out << nl << "throw _e;";
                out << eb;
                out << eb;

                out << sp << nl << "public String" << nl << "type()";
                out << sb;
                out << nl << "return \"" << p->typeId() << "\";";
                out << eb;

                if(_stream)
                {
                    out << sp << nl << "public void" << nl << "invoke(Ice.Object v)";
                    out << sb;
                    out << nl << "patch(v);";
                    out << eb;
                }
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

Slice::Gen::HelperVisitor::HelperVisitor(const string& dir, bool stream) :
    JavaVisitor(dir), _stream(stream)
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

    bool java2 = p->definitionContext()->findMetaData("java:java2") == "java:java2";
    string contextType = java2 ? "java.util.Map" : "java.util.Map<String, String>";
    string contextParam = contextType + " __ctx";
    string explicitContextParam = "boolean __explicitCtx";

    OperationList ops = p->allOperations();

    OperationList::const_iterator r;
    for(r = ops.begin(); r != ops.end(); ++r)
    {
        OperationPtr op = *r;
        ContainerPtr container = op->container();
        ClassDefPtr cl = ClassDefPtr::dynamicCast(container);
        string opName = fixKwd(op->name());
        TypePtr ret = op->returnType();
        string retS = typeToString(ret, TypeModeReturn, package, op->getMetaData());

        vector<string> params = getParams(op, package);
        vector<string> args = getArgs(op);

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
        out << opName << spar << args << "null" << "false" << epar << ';';
        out << eb;

        out << sp;
        out << nl << "public " << retS << nl << opName << spar << params << contextParam << epar;
        writeThrowsClause(package, throws);
        out << sb;
        out << nl;
        if(ret)
        {
            out << "return ";
        }
        out << opName << spar << args << "__ctx" << "true" << epar << ';';
        out << eb;

        out << sp;
        //
        // TODO: If we eventually drop support for Java2, we can remove this
        // SupressWarnings annotation. Meanwhile, it is necessary to prevent
        // a compiler warning about an unchecked conversion. This is caused
        // by the fact that _emptyContext returns the unchecked type
        // java.util.Map but Ice.Context is mapped to Map<String, String>.
        //
        if(!java2)
        {
            out << nl << "@SuppressWarnings(\"unchecked\")";
        }
        out << nl << "private " << retS << nl << opName << spar << params << contextParam
            << explicitContextParam << epar;
        writeThrowsClause(package, throws);
        out << sb;
        out << nl << "if(__explicitCtx && __ctx == null)";
        out << sb;
        out << nl << "__ctx = _emptyContext;";
        out << eb;
        out << nl << "int __cnt = 0;";
        out << nl << "while(true)";
        out << sb;
        out << nl << "Ice._ObjectDel __delBase = null;";
        out << nl << "try";
        out << sb;
        if(op->returnsData())
        {
            out << nl << "__checkTwowayOnly(\"" << opName << "\");";
        }
        out << nl << "__delBase = __getDelegate();";
        out << nl << '_' << name << "Del __del = (_" << name << "Del)__delBase;";
        out << nl;
        if(ret)
        {
            out << "return ";
        }
        out << "__del." << opName << spar << args << "__ctx" << epar << ';';
        if(!ret)
        {
            out << nl << "return;";
        }
        out << eb;
        out << nl << "catch(IceInternal.LocalExceptionWrapper __ex)";
        out << sb;
        if(op->mode() == Operation::Idempotent || op->mode() == Operation::Nonmutating)
        {
            out << nl << "__cnt = __handleExceptionWrapperRelaxed(__delBase, __ex, __cnt);";
        }
        else
        {
            out << nl << "__handleExceptionWrapper(__delBase, __ex);";
        }
        out << eb;
        out << nl << "catch(Ice.LocalException __ex)";
        out << sb;
        out << nl << "__cnt = __handleException(__delBase, __ex, __cnt);";
        out << eb;
        out << eb;
        out << eb;

        if(cl->hasMetaData("ami") || op->hasMetaData("ami"))
        {
            vector<string> paramsAMI = getParamsAsync(op, package, false);
            vector<string> argsAMI = getArgsAsync(op);
            
            //
            // Write two versions of the operation - with and without a
            // context parameter
            //
            out << sp;
            out << nl << "public void" << nl << op->name() << "_async" << spar << paramsAMI << epar;
            out << sb;
            out << nl << "__cb.__invoke" << spar << "this" << argsAMI << "null" << epar << ';';
            out << eb;

            out << sp;
            //
            // TODO: If we eventually drop support for Java2, we can remove this
            // SupressWarnings annotation. Meanwhile, it is necessary to prevent
            // a compiler warning about an unchecked conversion. This is caused
            // by the fact that _emptyContext returns the unchecked type
            // java.util.Map but Ice.Context is mapped to Map<String, String>.
            //
            if(!java2)
            {
                out << nl << "@SuppressWarnings(\"unchecked\")";
            }
            out << nl << "public void" << nl << op->name() << "_async" << spar << paramsAMI 
                << contextParam << epar;
            out << sb;
            out << nl << "if( __ctx == null)";
            out << sb;
            out << nl << "__ctx = _emptyContext;";
            out << eb;
            out << nl << "__cb.__invoke" << spar << "this" << argsAMI << "__ctx" << epar << ';';
            out << eb;
        }
    }

    out << sp << nl << "public static " << name << "Prx" << nl << "checkedCast(Ice.ObjectPrx __obj)";
    out << sb;
    out << nl << name << "Prx __d = null;";
    out << nl << "if(__obj != null)";
    out << sb;
    out << nl << "try";
    out << sb;
    out << nl << "__d = (" << name << "Prx)__obj;";
    out << eb;
    out << nl << "catch(ClassCastException ex)";
    out << sb;
    out << nl << "if(__obj.ice_isA(\"" << scoped << "\"))";
    out << sb;
    out << nl << name << "PrxHelper __h = new " << name << "PrxHelper();";
    out << nl << "__h.__copyFrom(__obj);";
    out << nl << "__d = __h;";
    out << eb;
    out << eb;
    out << eb;
    out << nl << "return __d;";
    out << eb;

    out << sp << nl << "public static " << name << "Prx" << nl << "checkedCast(Ice.ObjectPrx __obj, " << contextParam
        << ')';
    out << sb;
    out << nl << name << "Prx __d = null;";
    out << nl << "if(__obj != null)";
    out << sb;
    out << nl << "try";
    out << sb;
    out << nl << "__d = (" << name << "Prx)__obj;";
    out << eb;
    out << nl << "catch(ClassCastException ex)";
    out << sb;
    out << nl << "if(__obj.ice_isA(\"" << scoped << "\", __ctx))";
    out << sb;
    out << nl << name << "PrxHelper __h = new " << name << "PrxHelper();";
    out << nl << "__h.__copyFrom(__obj);";
    out << nl << "__d = __h;";
    out << eb;
    out << eb;
    out << eb;
    out << nl << "return __d;";
    out << eb;

    out << sp << nl << "public static " << name << "Prx" << nl << "checkedCast(Ice.ObjectPrx __obj, String __facet)";
    out << sb;
    out << nl << name << "Prx __d = null;";
    out << nl << "if(__obj != null)";
    out << sb;
    out << nl << "Ice.ObjectPrx __bb = __obj.ice_facet(__facet);";
    out << nl << "try";
    out << sb;
    out << nl << "if(__bb.ice_isA(\"" << scoped << "\"))";
    out << sb;
    out << nl << name << "PrxHelper __h = new " << name << "PrxHelper();";
    out << nl << "__h.__copyFrom(__bb);";
    out << nl << "__d = __h;";
    out << eb;
    out << eb;
    out << nl << "catch(Ice.FacetNotExistException ex)";
    out << sb;
    out << eb;
    out << eb;
    out << nl << "return __d;";
    out << eb;

    out << sp << nl << "public static " << name << "Prx"
        << nl << "checkedCast(Ice.ObjectPrx __obj, String __facet, " << contextParam << ')';
    out << sb;
    out << nl << name << "Prx __d = null;";
    out << nl << "if(__obj != null)";
    out << sb;
    out << nl << "Ice.ObjectPrx __bb = __obj.ice_facet(__facet);";
    out << nl << "try";
    out << sb;
    out << nl << "if(__bb.ice_isA(\"" << scoped << "\", __ctx))";
    out << sb;
    out << nl << name << "PrxHelper __h = new " << name << "PrxHelper();";
    out << nl << "__h.__copyFrom(__bb);";
    out << nl << "__d = __h;";
    out << eb;
    out << eb;
    out << nl << "catch(Ice.FacetNotExistException ex)";
    out << sb;
    out << eb;
    out << eb;
    out << nl << "return __d;";
    out << eb;

    out << sp << nl << "public static " << name << "Prx" << nl << "uncheckedCast(Ice.ObjectPrx __obj)";
    out << sb;
    out << nl << name << "Prx __d = null;";
    out << nl << "if(__obj != null)";
    out << sb;
    out << nl << name << "PrxHelper __h = new " << name << "PrxHelper();";
    out << nl << "__h.__copyFrom(__obj);";
    out << nl << "__d = __h;";
    out << eb;
    out << nl << "return __d;";
    out << eb;

    out << sp << nl << "public static " << name << "Prx" << nl << "uncheckedCast(Ice.ObjectPrx __obj, String __facet)";
    out << sb;
    out << nl << name << "Prx __d = null;";
    out << nl << "if(__obj != null)";
    out << sb;
    out << nl << "Ice.ObjectPrx __bb = __obj.ice_facet(__facet);";
    out << nl << name << "PrxHelper __h = new " << name << "PrxHelper();";
    out << nl << "__h.__copyFrom(__bb);";
    out << nl << "__d = __h;";
    out << eb;
    out << nl << "return __d;";
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

    if(_stream)
    {
        out << sp << nl << "public static void" << nl << "write(Ice.OutputStream __outS, " << name << "Prx v)";
        out << sb;
        out << nl << "__outS.writeProxy(v);";
        out << eb;

        out << sp << nl << "public static " << name << "Prx" << nl << "read(Ice.InputStream __inS)";
        out << sb;
        out << nl << "Ice.ObjectPrx proxy = __inS.readProxy();";
        out << nl << "if(proxy != null)";
        out << sb;
        out << nl << name << "PrxHelper result = new " << name << "PrxHelper();";
        out << nl << "result.__copyFrom(proxy);";
        out << nl << "return result;";
        out << eb;
        out << nl << "return null;";
        out << eb;
    }

    out << eb;
    close();

    if(_stream)
    {
        //
        // Class helper.
        //
        if(!open(getAbsolute(p, "", "", "Helper")))
        {
            return false;
        }

        Output& out2 = output();

        out2 << sp << nl << "public final class " << name << "Helper";
        out2 << sb;

        out2 << sp << nl << "public static void" << nl << "write(Ice.OutputStream __outS, " << fixKwd(name) << " __v)";
        out2 << sb;
        out2 << nl << "__outS.writeObject(__v);";
        out2 << eb;

        out2 << sp << nl << "public static void" << nl << "read(Ice.InputStream __inS, " << name << "Holder __h)";
        out2 << sb;
        out2 << nl << "__inS.readObject(__h.getPatcher());";
        out2 << eb;

        out2 << eb;
        close();
    }

    return false;
}

bool
Slice::Gen::HelperVisitor::visitStructStart(const StructPtr& p)
{
    if(!p->isLocal() && _stream)
    {
        string name = p->name();
        string fixedName = fixKwd(name);

        if(!open(getAbsolute(p, "", "", "Helper")))
        {
            return false;
        }

        Output& out = output();

        out << sp << nl << "public final class " << name << "Helper";
        out << sb;

        out << sp << nl << "public static void" << nl << "write(Ice.OutputStream __outS, " << fixedName << " __v)";
        out << sb;
        out << nl << "__v.ice_write(__outS);";
        out << eb;

        out << sp << nl << "public static " << fixedName << nl << "read(Ice.InputStream __inS)";
        out << sb;
        out << nl << fixedName << " __v = new " << fixedName << "();";
        out << nl << "__v.ice_read(__inS);";
        out << nl << "return __v;";
        out << eb;

        out << eb;
        close();
    }

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

        if(_stream)
        {
            out << sp << nl << "public static void" << nl << "write(Ice.OutputStream __outS, " << typeS << " __v)";
            out << sb;
            iter = 0;
            writeStreamSequenceMarshalUnmarshalCode(out, package, p, "__v", true, iter, false);
            out << eb;

            out << sp << nl << "public static " << typeS << nl << "read(Ice.InputStream __inS)";
            out << sb;
            out << nl << typeS << " __v;";
            iter = 0;
            writeStreamSequenceMarshalUnmarshalCode(out, package, p, "__v", false, iter, false);
            out << nl << "return __v;";
            out << eb;
        }

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

    string name = p->name();
    string absolute = getAbsolute(p);
    string helper = getAbsolute(p, "", "", "Helper");
    string package = getPackage(p);
    StringList metaData = p->getMetaData();
    string abstractType = typeToString(p, TypeModeIn, package, StringList(), true);

    if(open(helper))
    {
        Output& out = output();
        int iter;

        out << sp << nl << "public final class " << name << "Helper";
        out << sb;

        out << nl << "public static void" << nl << "write(IceInternal.BasicStream __os, " << abstractType << " __v)";
        out << sb;
        iter = 0;
        writeDictionaryMarshalUnmarshalCode(out, package, p, "__v", true, iter, false);
        out << eb;

        out << sp << nl << "public static " << abstractType
            << nl << "read(IceInternal.BasicStream __is)";
        out << sb;
        out << nl << abstractType << " __v;";
        iter = 0;
        writeDictionaryMarshalUnmarshalCode(out, package, p, "__v", false, iter, false);
        out << nl << "return __v;";
        out << eb;

        if(_stream)
        {
            out << sp << nl << "public static void" << nl << "write(Ice.OutputStream __outS, " << abstractType
                << " __v)";
            out << sb;
            iter = 0;
            writeStreamDictionaryMarshalUnmarshalCode(out, package, p, "__v", true, iter, false);
            out << eb;

            out << sp << nl << "public static " << abstractType
                << nl << "read(Ice.InputStream __inS)";
            out << sb;
            out << nl << abstractType << " __v;";
            iter = 0;
            writeStreamDictionaryMarshalUnmarshalCode(out, package, p, "__v", false, iter, false);
            out << nl << "return __v;";
            out << eb;
        }

        out << eb;
        close();
    }
}

void
Slice::Gen::HelperVisitor::visitEnum(const EnumPtr& p)
{
    if(!p->isLocal() && _stream)
    {
        string name = p->name();
        string fixedName = fixKwd(name);

        if(!open(getAbsolute(p, "", "", "Helper")))
        {
            return;
        }

        Output& out = output();

        out << sp << nl << "public final class " << name << "Helper";
        out << sb;

        out << sp << nl << "public static void" << nl << "write(Ice.OutputStream __outS, " << fixedName << " __v)";
        out << sb;
        out << nl << "__v.ice_write(__outS);";
        out << eb;

        out << sp << nl << "public static " << fixedName << nl << "read(Ice.InputStream __inS)";
        out << sb;
        out << nl << "return " << fixedName << ".ice_read(__inS);";
        out << eb;

        out << eb;
        close();
    }
}

Slice::Gen::ProxyVisitor::ProxyVisitor(const string& dir) :
    JavaVisitor(dir)
{
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

    string deprecateReason = getDeprecateReason(p, cl, "operation");

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

    bool java2 = p->definitionContext()->findMetaData("java:java2") == "java:java2";
    string contextParam = java2 ? "java.util.Map __ctx" : "java.util.Map<String, String> __ctx";

    out << nl << "public " << retS << ' ' << name << spar << params << contextParam << epar;
    writeThrowsClause(package, throws);
    out << ';';

    if(cl->hasMetaData("ami") || p->hasMetaData("ami"))
    {
        vector<string> paramsAMI = getParamsAsync(p, package, false);

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
        out << nl << "public void " << p->name() << "_async" << spar << paramsAMI << epar << ';';
        if(!deprecateReason.empty())
        {
            out << nl << "/**";
            out << nl << " * @deprecated " << deprecateReason;
            out << nl << " **/";
        }
        out << nl << "public void " << p->name() << "_async" << spar << paramsAMI << contextParam << epar << ';';
    }
}

Slice::Gen::DelegateVisitor::DelegateVisitor(const string& dir) :
    JavaVisitor(dir)
{
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
    string package = getPackage(p);
    string absolute = getAbsolute(p, "", "_", "Del");

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
            out << getAbsolute(*q, package, "_", "Del");
            if(++q != bases.end())
            {
                out << ',' << nl;
            }
        }
        out.restoreIndent();
    }

    out << sb;

    bool java2 = p->definitionContext()->findMetaData("java:java2") == "java:java2";
    string contextParam = java2 ? "java.util.Map __ctx" : "java.util.Map<String, String> __ctx";

    OperationList ops = p->operations();

    OperationList::const_iterator r;
    for(r = ops.begin(); r != ops.end(); ++r)
    {
        OperationPtr op = *r;
        string opName = fixKwd(op->name());
        TypePtr ret = op->returnType();
        string retS = typeToString(ret, TypeModeReturn, package, op->getMetaData());

        vector<string> params = getParams(op, package);

        ExceptionList throws = op->throws();
        throws.sort();
        throws.unique();

        out << sp;
        out << nl << retS << ' ' << opName << spar << params << contextParam << epar;
        writeDelegateThrowsClause(package, throws);
        out << ';';
    }

    out << eb;
    close();

    return false;
}

Slice::Gen::DelegateMVisitor::DelegateMVisitor(const string& dir) :
    JavaVisitor(dir)
{
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
    string package = getPackage(p);
    string absolute = getAbsolute(p, "", "_", "DelM");

    if(!open(absolute))
    {
        return false;
    }

    Output& out = output();

    out << sp << nl << "public final class _" << name << "DelM extends Ice._ObjectDelM implements _" << name << "Del";
    out << sb;

    bool java2 = p->definitionContext()->findMetaData("java:java2") == "java:java2";
    string contextParam = java2 ? "java.util.Map __ctx" : "java.util.Map<String, String> __ctx";

    OperationList ops = p->allOperations();

    OperationList::const_iterator r;
    for(r = ops.begin(); r != ops.end(); ++r)
    {
        OperationPtr op = *r;
        StringList opMetaData = op->getMetaData();
        string opName = fixKwd(op->name());
        TypePtr ret = op->returnType();
        string retS = typeToString(ret, TypeModeReturn, package, opMetaData);
        int iter = 0;

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

        vector<string> params = getParams(op, package);

        out << sp;
        out << nl << "public " << retS << nl << opName << spar << params << contextParam << epar;
        writeDelegateThrowsClause(package, throws);
        out << sb;

        out << nl << "IceInternal.Outgoing __og = __connection.getOutgoing(__reference, \"" << op->name() << "\", "
            << sliceModeToIceMode(op->sendMode()) << ", __ctx, __compress);";
        out << nl << "try";
        out << sb;
        if(!inParams.empty())
        {
            out << nl << "try";
            out << sb;
            out << nl << "IceInternal.BasicStream __os = __og.os();";
            for(pli = inParams.begin(); pli != inParams.end(); ++pli)
            {
                writeMarshalUnmarshalCode(out, package, (*pli)->type(), fixKwd((*pli)->name()), true, iter, false,
                                          (*pli)->getMetaData());
            }
            if(op->sendsClasses())
            {
                out << nl << "__os.writePendingObjects();";
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
        out << nl << "IceInternal.BasicStream __is = __og.is();";
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
        out << nl << "throw new Ice.UnknownUserException(__ex.ice_name());";
        out << eb;
        out << eb;
        for(pli = outParams.begin(); pli != outParams.end(); ++pli)
        {
            writeMarshalUnmarshalCode(out, package, (*pli)->type(), fixKwd((*pli)->name()), false, iter, true,
                                      (*pli)->getMetaData());
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
                writeMarshalUnmarshalCode(out, package, ret, "__ret", false, iter, false, opMetaData);
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
        out << eb;
    }

    out << eb;
    close();

    return false;
}

Slice::Gen::DelegateDVisitor::DelegateDVisitor(const string& dir) :
    JavaVisitor(dir)
{
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
    string package = getPackage(p);
    string absolute = getAbsolute(p, "", "_", "DelD");

    if(!open(absolute))
    {
        return false;
    }

    Output& out = output();

    out << sp << nl << "public final class _" << name << "DelD extends Ice._ObjectDelD implements _" << name << "Del";
    out << sb;

    bool java2 = p->definitionContext()->findMetaData("java:java2") == "java:java2";
    string contextParam = java2 ? "java.util.Map __ctx" : "java.util.Map<String, String> __ctx";

    OperationList ops = p->allOperations();

    OperationList::const_iterator r;
    for(r = ops.begin(); r != ops.end(); ++r)
    {
        OperationPtr op = *r;
        ContainerPtr container = op->container();
        ClassDefPtr cl = ClassDefPtr::dynamicCast(container);
        string opName = fixKwd(op->name());
        TypePtr ret = op->returnType();
        string retS = typeToString(ret, TypeModeReturn, package, op->getMetaData());

        ExceptionList throws = op->throws();
        throws.sort();
        throws.unique();

        //
        // Arrange exceptions into most-derived to least-derived order. If we don't
        // do this, a base exception handler can appear before a derived exception
        // handler, causing compiler warnings
        //
#if defined(__SUNPRO_CC)
        throws.sort(Slice::derivedToBaseCompare);
#else
        throws.sort(Slice::DerivedToBaseCompare());
#endif


        vector<string> params = getParams(op, package, true);
        vector<string> args = getArgs(op);

        out << sp;
        out << nl << "public " << retS << nl << opName << spar << params << contextParam << epar;
        writeDelegateThrowsClause(package, throws);
        out << sb;
        if(cl->hasMetaData("amd") || op->hasMetaData("amd"))
        {
            out << nl << "throw new Ice.CollocationOptimizationException();";
        }
        else
        {
            StringList metaData = op->getMetaData();
            out << nl << "final Ice.Current __current = new Ice.Current();";
            out << nl << "__initCurrent(__current, \"" << op->name() << "\", " 
                << sliceModeToIceMode(op->sendMode())
                << ", __ctx);";
            if(ret)
            {
                string resultTypeHolder = typeToString(ret, TypeModeOut, package, op->getMetaData());

                out << nl << "final " << resultTypeHolder << " __result = new " << resultTypeHolder << "();";
            }
            
            out << nl << "IceInternal.Direct __direct = new IceInternal.Direct(__current)";
            out << sb;
            out << nl << "public Ice.DispatchStatus run(Ice.Object __obj)";
            out << sb;
            out << nl << fixKwd(name) << " __servant = null;";
            out << nl << "try";
            out << sb;
            out << nl << "__servant = (" << fixKwd(name) << ")__obj;";
            out << eb;
            out << nl << "catch(ClassCastException __ex)";
            out << sb;
            out << nl << "throw new Ice.OperationNotExistException(__current.id, __current.facet, __current.operation);";
            out << eb;

            if(!throws.empty())
            {
                out << nl << "try";
                out << sb;
            }

            out << nl;
            if(ret)
            {
                out << "__result.value = ";
            }
            out << "__servant." << opName << spar << args << "__current" << epar << ';';
            out << nl << "return Ice.DispatchStatus.DispatchOK;";
            
            if(!throws.empty())
            {
                out << eb;
                out << nl << "catch(Ice.UserException __ex)";
                out << sb;
                out << nl << "setUserException(__ex);";
                out << nl << "return Ice.DispatchStatus.DispatchUserException;";
                out << eb;
            }
            out << eb;
            out << eb;
            out << ";";
          
            out << nl << "try";
            out << sb;
            out << nl << "Ice.DispatchStatus __status = __direct.servant().__collocDispatch(__direct);";
            if(!throws.empty())
            {
                out << nl << "if(__status == Ice.DispatchStatus.DispatchUserException)";
                out << sb;
                out << nl << "try";
                out << sb;
                out << nl << "__direct.throwUserException();";
                out << eb;
                for(ExceptionList::const_iterator t = throws.begin(); t != throws.end(); ++t)
                {
                    string exS = getAbsolute(*t, package);
                    out << nl << "catch(" << exS << " __ex)";
                    out << sb;
                    out << nl << "throw __ex;";
                    out << eb;
                }
                out << nl << "catch(Ice.UserException __ex)";
                out << sb;
                out << nl << "assert false;";
                out << nl << "throw new Ice.UnknownUserException(__ex.toString());";
                out << eb;
                out << eb;
            }
            out << nl << "assert __status == Ice.DispatchStatus.DispatchOK;";
            if(ret)
            {
                out << nl << "return __result.value;";
            }
            out << eb;
            out << nl << "catch(Ice.LocalException __ex)";
            out << sb;
            out << nl << "throw new IceInternal.LocalExceptionWrapper(__ex, false);";
            out << eb;
            out << nl << "finally";
            out << sb;
            out << nl << "__direct.destroy();";
            out << eb;
        }
        out << eb;
    }

    out << eb;
    close();

    return false;
}

Slice::Gen::DispatcherVisitor::DispatcherVisitor(const string& dir, bool stream) :
    JavaVisitor(dir), _stream(stream)
{
}

bool
Slice::Gen::DispatcherVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    if(p->isLocal() || !p->isInterface())
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
    out << nl << "throws java.lang.CloneNotSupportedException";
    out.dec();
    out << sb;
    out << nl << "throw new java.lang.CloneNotSupportedException();";
    out << eb;

    writeDispatchAndMarshalling(out, p, _stream);

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
Slice::Gen::BaseImplVisitor::writeOperation(Output& out, const string& package, const OperationPtr& op, bool local)
{
    string opName = op->name();

    TypePtr ret = op->returnType();
    StringList opMetaData = op->getMetaData();
    string retS = typeToString(ret, TypeModeReturn, package, opMetaData);
    vector<string> params = getParams(op, package);

    ContainerPtr container = op->container();
    ClassDefPtr cl = ClassDefPtr::dynamicCast(container);

    if(!local && (cl->hasMetaData("amd") || op->hasMetaData("amd")))
    {
        vector<string> paramsAMD = getParamsAsync(op, package, true);

        out << sp << nl << "public void" << nl << opName << "_async" << spar << paramsAMD << "Ice.Current __current"
            << epar;

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
        writeThrowsClause(package, throws);

        out << sb;

        string result = "__r";
        ParamDeclList paramList = op->parameters();
        ParamDeclList::const_iterator q;
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
            writeDecl(out, package, result, ret, opMetaData);
        }
        for(q = paramList.begin(); q != paramList.end(); ++q)
        {
            if((*q)->isOutParam())
            {
                writeDecl(out, package, fixKwd((*q)->name()), (*q)->type(), (*q)->getMetaData());
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
        if(p->isLocal())
        {
            out << " implements " << fixKwd(name);
        }
        else
        {
            out << " extends _" << name << "Disp";
        }
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
        writeOperation(out, package, *r, p->isLocal());
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
    if(p->isLocal())
    {
        out << "NC";
    }
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
            writeOperation(out, package, *r, p->isLocal());
            out << sp;
            out << nl << "*/";
        }
        else
        {
            writeOperation(out, package, *r, p->isLocal());
        }
    }

    out << eb;
    close();

    return false;
}

Slice::Gen::AsyncVisitor::AsyncVisitor(const string& dir) :
    JavaVisitor(dir)
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
    string classPkg = getPackage(cl);
    StringList opMetaData = p->getMetaData();
    
    if(cl->hasMetaData("ami") || p->hasMetaData("ami"))
    {
        string classNameAMI = "AMI_" + cl->name();
        string absoluteAMI = getAbsolute(cl, "", "AMI_", "_" + name);

        if(!open(absoluteAMI))
        {
            return;
        }
        
        Output& out = output();

        TypePtr ret = p->returnType();

        ParamDeclList inParams;
        ParamDeclList outParams;
        ParamDeclList paramList = p->parameters();
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

        int iter;

        vector<string> params = getParamsAsyncCB(p, classPkg);
        vector<string> args = getArgsAsyncCB(p);

        vector<string> paramsInvoke = getParamsAsync(p, classPkg, false);

        out << sp << nl << "public abstract class " << classNameAMI << '_' << name
            << " extends IceInternal.OutgoingAsync";
        out << sb;
        out << sp;
        out << nl << "public abstract void ice_response" << spar << params << epar << ';';
        out << nl << "public abstract void ice_exception(Ice.LocalException ex);";
        if(!throws.empty())
        {
            out << nl << "public abstract void ice_exception(Ice.UserException ex);";
        }
        
        bool java2 = p->definitionContext()->findMetaData("java:java2") == "java:java2";
        string contextParam = java2 ? "java.util.Map __ctx" : "java.util.Map<String, String> __ctx";

        out << sp << nl << "public final void" << nl << "__invoke" << spar << "Ice.ObjectPrx __prx"
            << paramsInvoke << contextParam << epar;
        out << sb;
        out << nl << "try";
        out << sb;
        out << nl << "__prepare(__prx, \"" << p->name() << "\", " 
            << sliceModeToIceMode(p->sendMode()) << ", __ctx);";
        iter = 0;
        for(pli = inParams.begin(); pli != inParams.end(); ++pli)
        {
            StringList metaData = (*pli)->getMetaData();
            string typeS = typeToString((*pli)->type(), TypeModeIn, classPkg, metaData);
            writeMarshalUnmarshalCode(out, classPkg, (*pli)->type(), fixKwd((*pli)->name()), true, iter, false,
                                      metaData);
        }
        if(p->sendsClasses())
        {
            out << nl << "__os.writePendingObjects();";
        }
        out << nl << "__os.endWriteEncaps();";
        out << eb;
        out << nl << "catch(Ice.LocalException __ex)";
        out << sb;
        out << nl << "__finished(__ex);";
        out << nl << "return;";
        out << eb;
        out << nl << "__send();";
        out << eb;

        out << sp << nl << "protected final void" << nl << "__response(boolean __ok)";
        out << sb;
        for(pli = outParams.begin(); pli != outParams.end(); ++pli)
        {
            TypePtr paramType = (*pli)->type();
            string paramName = fixKwd((*pli)->name());
            string typeS = typeToString(paramType, TypeModeIn, classPkg, (*pli)->getMetaData());
            BuiltinPtr builtin = BuiltinPtr::dynamicCast(paramType);
            if((builtin && builtin->kind() == Builtin::KindObject) || ClassDeclPtr::dynamicCast(paramType))
            {
                out << nl << typeS << "Holder " << paramName << " = new " << typeS << "Holder();";
            }
            else
            {
                out << nl << typeS << ' ' << paramName << ';';
            }
        }
        if(ret)
        {
            string retS = typeToString(ret, TypeModeIn, classPkg, opMetaData);
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
        out << nl << "if(!__ok)";
        out << sb;
        out << nl << "try";
        out << sb;
        out << nl << "__is.throwException();";
        out << eb;
        for(ExceptionList::const_iterator r = throws.begin(); r != throws.end(); ++r)
        {
            out << nl << "catch(" << getAbsolute(*r, classPkg) << " __ex)";
            out << sb;
            out << nl << "ice_exception(__ex);";
            out << nl << "return;";
            out << eb;
        }
        out << nl << "catch(Ice.UserException __ex)";
        out << sb;
        out << nl << "throw new Ice.UnknownUserException(__ex.ice_name());";
        out << eb;
        out << eb;
        for(pli = outParams.begin(); pli != outParams.end(); ++pli)
        {
            TypePtr paramType = (*pli)->type();
            string paramName = fixKwd((*pli)->name());
            BuiltinPtr builtin = BuiltinPtr::dynamicCast(paramType);
            if((builtin && builtin->kind() == Builtin::KindObject) || ClassDeclPtr::dynamicCast(paramType))
            {
                out << nl << "__is.readObject(" << paramName << ".getPatcher());";
            }
            else
            {
                writeMarshalUnmarshalCode(out, classPkg, paramType, paramName, false, iter, false,
                                          (*pli)->getMetaData());
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
                writeMarshalUnmarshalCode(out, classPkg, ret, "__ret", false, iter, false, opMetaData);
            }
        }
        if(p->returnsClasses())
        {
            out << nl << "__is.readPendingObjects();";
        }
        out << eb;
        out << nl << "catch(Ice.LocalException __ex)";
        out << sb;
        out << nl << "__finished(__ex);";
        out << nl << "return;";
        out << eb;
        out << nl << "ice_response" << spar << args << epar << ';';
        out << eb;
        out << eb;

        close();
    }

    if(cl->hasMetaData("amd") || p->hasMetaData("amd"))
    {
        string classNameAMD = "AMD_" + cl->name();
        string absoluteAMD = getAbsolute(cl, "", "AMD_", "_" + name);

        string classNameAMDI = "_AMD_" + cl->name();
        string absoluteAMDI = getAbsolute(cl, "", "_AMD_", "_" + name);

        vector<string> paramsAMD = getParamsAsyncCB(p, classPkg);

        {
            if(!open(absoluteAMD))
            {
                return;
            }

            Output& out = output();
            
            out << sp << nl << "public interface " << classNameAMD << '_' << name;
            out << sb;
            out << sp << nl << "void ice_response" << spar << paramsAMD << epar << ';';
            out << sp << nl << "void ice_exception(java.lang.Exception ex);";
            out << eb;
            
            close();
        }
        
        {
            if(!open(absoluteAMDI))
            {
                return;
            }
            
            Output& out = output();
            
            TypePtr ret = p->returnType();
            
            ParamDeclList outParams;
            ParamDeclList paramList = p->parameters();
            ParamDeclList::const_iterator pli;
            for(pli = paramList.begin(); pli != paramList.end(); ++pli)
            {
                if((*pli)->isOutParam())
                {
                    outParams.push_back(*pli);
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

            int iter;

            out << sp << nl << "final class " << classNameAMDI << '_' << name
                << " extends IceInternal.IncomingAsync implements " << classNameAMD << '_' << name;
            out << sb;

            out << sp << nl << "public" << nl << classNameAMDI << '_' << name << "(IceInternal.Incoming in)";
            out << sb;
            out << nl << "super(in);";
            out << eb;

            out << sp << nl << "public void" << nl << "ice_response" << spar << paramsAMD << epar;
            out << sb;
            iter = 0;
            out << nl << "if(__validateResponse(true))";
            out << sb;
            if(ret || !outParams.empty())
            {
                out << nl << "try";
                out << sb;
                out << nl << "IceInternal.BasicStream __os = this.__os();";
                for(pli = outParams.begin(); pli != outParams.end(); ++pli)
                {
                    StringList metaData = (*pli)->getMetaData();
                    string typeS = typeToString((*pli)->type(), TypeModeIn, classPkg, metaData);
                    writeMarshalUnmarshalCode(out, classPkg, (*pli)->type(), fixKwd((*pli)->name()), true, iter,
                                              false, metaData);
                }
                if(ret)
                {
                    string retS = typeToString(ret, TypeModeIn, classPkg, opMetaData);
                    writeMarshalUnmarshalCode(out, classPkg, ret, "__ret", true, iter, false, opMetaData);
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
            if(throws.empty())
            {
                out << nl << "if(__validateException(ex))";
                out << sb;
                out << nl << "__exception(ex);";
                out << eb;
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
                    string exS = getAbsolute(*r, classPkg);
                    out << nl << "catch(" << exS << " __ex)";
                    out << sb;
                    out << nl << "if(__validateResponse(false))";
                    out << sb;
                    out << nl << "__os().writeUserException(__ex);";
                    out << nl << "__response(false);";
                    out << eb;
                    out << eb;
                }
                out << nl << "catch(java.lang.Exception __ex)";
                out << sb;
                out << nl << "if(__validateException(__ex))";
                out << sb;
                out << nl << "__exception(__ex);";
                out << eb;
                out << eb;
            }
            out << eb;

            out << eb;
            
            close();
        }
    }
}
