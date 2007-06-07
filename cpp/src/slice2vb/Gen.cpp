// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/DisableWarnings.h>
#include <IceUtil/Functional.h>
#include <Gen.h>
#include <limits>
#include <sys/stat.h>
#ifndef _WIN32
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
// Don't use "using namespace IceUtil", or VC++ 6.0 complains
// about ambigious symbols for constructs like
// "IceUtil::constMemFun(&Slice::Exception::isLocal)".
//
using IceUtil::Output;
using IceUtil::nl;
using IceUtil::sp;
using IceUtil::spar;
using IceUtil::epar;

static string // Should be an anonymous namespace, but VC++ 6 can't handle that.
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

static void
emitDeprecate(const ContainedPtr& p1, const ContainedPtr& p2, Output& out, string type)
{
    string deprecateMetadata;
    if(p1->findMetaData("deprecate", deprecateMetadata) || 
       (p2 != 0 && p2->findMetaData("deprecate", deprecateMetadata)))
    {
        string deprecateReason = "This " + type + " has been deprecated.";
        if(deprecateMetadata.find("deprecate:") == 0 && deprecateMetadata.size() > 10)
        {
            deprecateReason = deprecateMetadata.substr(10);
        }
        out << nl << "<System.Obsolete(\"" << deprecateReason << "\")>";
    }
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
                _out << " Implements " << fixId(containingClass->scoped() + "OperationsNC_.")
                     << fixId(name, DotNet::ICloneable, true);
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
                    _out << "ByVal current__ As Ice.Current";
                }
                _out << epar;
                if(ret)
                {
                    _out << " As " << retS;
                }
                _out << " Implements " << fixId(containingClass->scoped() + "Operations_.")
                     << fixId(name, DotNet::ICloneable, true);
            }
            else
            {
                vector<string> params = getParamsAsync(*op, true);
                vector<string> args = getArgsAsync(*op);

                _out << sp << nl << "Public Sub " << ' ' << name << "_async" << spar << params << epar
                     << " Implements " << fixId(containingClass->scoped() + "OperationsNC_.")
                     << name << "_async";
                _out.inc();
                _out << nl << name << "_async" << spar << args << epar;
                _out.dec();
                _out << nl << "End Sub";

                _out << sp << nl << "Public MustOverride Sub " << name << "_async"
                     << spar << params << "ByVal current__ As Ice.Current" << epar
                     << " Implements " << fixId(containingClass->scoped() + "Operations_.")
                     << name << "_async";
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

    StringList::const_iterator firstIter = ids.begin();
    StringList::const_iterator scopedIter = find(ids.begin(), ids.end(), scoped);
    assert(scopedIter != ids.end());
    StringList::difference_type scopedPos = ice_distance(firstIter, scopedIter);
    
    _out.zeroIndent();
    _out << sp << nl << "#Region \"Slice type-related members\"";
    _out.restoreIndent();

    _out << sp << nl << "Public Shared Shadows ids__ As String() = New String() _";
    _out.inc();
    _out << nl << "{ _";
    _out.inc();

    {
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
    }
    _out.dec();
    _out << nl << '}';
    _out.dec();

    _out << sp << nl << "Public Overloads Overrides Function ice_isA(ByVal s As String) As Boolean";
    _out.inc();
    _out << nl << "Return _System.Array.BinarySearch(ids__, s, IceUtil.StringUtil.OrdinalStringComparer) >= 0";
    _out.dec();
    _out << nl << "End Function";

    _out << sp << nl << "Public Overloads Overrides Function ice_isA(ByVal s As String, Byval current__ As Ice.Current)"
                        " As Boolean";
    _out.inc();
    _out << nl << "Return _System.Array.BinarySearch(ids__, s, IceUtil.StringUtil.OrdinalStringComparer) >= 0";
    _out.dec();
    _out << nl << "End Function";

    _out << sp << nl << "Public Overloads Overrides Function ice_ids() As String()";
    _out.inc();
    _out << nl << "Return ids__";
    _out.dec();
    _out << nl << "End Function";

    _out << sp << nl << "Public Overloads Overrides Function ice_ids(ByVal current__ As Ice.Current) As String()";
    _out.inc();
    _out << nl << "Return ids__";
    _out.dec();
    _out << nl << "End Function";

    _out << sp << nl << "Public Overloads Overrides Function ice_id() As String";
    _out.inc();
    _out << nl << "Return ids__(" << scopedPos << ")";
    _out.dec();
    _out << nl << "End Function";

    _out << sp << nl << "Public Overloads Overrides Function ice_id(ByVal current__ As Ice.Current) As String";
    _out.inc();
    _out << nl << "Return ids__(" << scopedPos << ")";
    _out.dec();
    _out << nl << "End Function";

    _out << sp << nl << "Public Overloads Shared Function ice_staticId() As String";
    _out.inc();
    _out << nl << "Return ids__(" << scopedPos << ")";
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
        _out << sp << nl << "Public Shared Function "<< opName << "___( _";
        _out.inc();
        _out.inc();
        _out << nl << "ByVal obj__ As " << p->name() << "Operations_, _";
        _out << nl << "ByVal inS__ As IceInternal.Incoming, _";
        _out << nl << "ByVal current__ As Ice.Current) As Ice.DispatchStatus";
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
            _out << nl << "checkMode__(" << sliceModeToIceMode(op->mode()) << ", current__.mode)";
            if(!inParams.empty())
            {
                _out << nl << "Dim is__ As IceInternal.BasicStream = inS__.istr()";
            }
            if(!outParams.empty() || ret || !throws.empty())
            {
                _out << nl << "Dim os__ As IceInternal.BasicStream = inS__.ostr()";
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
                    _out << nl << "Dim " << param << " As " << typeS << " = Nothing";
                }
                writeMarshalUnmarshalCode(_out, q->first, param, false, false, true);
            }
            if(op->sendsClasses())
            {
                _out << nl << "is__.readPendingObjects()";
            }
            
            for(q = outParams.begin(); q != outParams.end(); ++q)
            {
                string typeS = typeToString(q->first);
                _out << nl << "Dim " << fixId(q->second) << " As " << typeS << " = Nothing";
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
                _out << "Dim ret__ As " << retS << " = ";
            }
            _out << "obj__." << fixId(opName, DotNet::ICloneable, true) << spar;
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
            _out << "current__" << epar;
            
            //
            // Marshal 'out' parameters and return value.
            //
            for(q = outParams.begin(); q != outParams.end(); ++q)
            {
                writeMarshalUnmarshalCode(_out, q->first, fixId(q->second), true, false, true, "");
            }
            if(ret)
            {
                writeMarshalUnmarshalCode(_out, ret, "ret__", true, false, true, "");
            }
            if(op->returnsClasses())
            {
                _out << nl << "os__.writePendingObjects()";
            }
            _out << nl << "Return Ice.DispatchStatus.DispatchOK";
            
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
                    _out << nl << "os__.writeUserException(ex)";
                    _out << nl << "Return Ice.DispatchStatus.DispatchUserException";
                }
                _out.dec();
                _out << nl << "End Try";
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
            _out << nl << "checkMode__(" << sliceModeToIceMode(op->mode()) << ", current__.mode)";
            if(!inParams.empty())
            {
                _out << nl << "Dim is__ As IceInternal.BasicStream = inS__.istr()";
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
                _out << nl << "is__.readPendingObjects()";
            }
            
            //
            // Call on the servant.
            //
            string classNameAMD = "AMD_" + p->name();
            _out << nl << "Dim cb__ As " << classNameAMD << '_' << op->name() << " = new _"
                 << classNameAMD << '_' << op->name() << "(inS__)";
            _out << nl << "Try";
            _out.inc();
            _out << nl << "obj__.";
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
                _out << "cb__";
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
            _out << "current__" << epar;
            _out.dec();
            _out << nl << "Catch ex As _System.Exception";
            _out.inc();
            _out << nl << "cb__.ice_exception(ex)";
            _out.dec();
            _out << nl << "End Try";
            _out << nl << "Return Ice.DispatchStatus.DispatchAsync";

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
	allOpNames.sort();
        allOpNames.unique();

        StringList::const_iterator q;

        _out << sp << nl << "Public Shared Shadows all__ As String() = New String() _";
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

        _out << sp << nl << "Public Overloads Overrides Function dispatch__( _";
        _out.inc();
        _out.inc();
        _out << nl << "ByVal inS__ As IceInternal.Incoming, _";
        _out << nl << "ByVal current__ As Ice.Current) As Ice.DispatchStatus";
        _out.dec();
        _out << nl << "Dim pos As Integer";
        _out << nl << "pos = _System.Array.BinarySearch(all__, current__.operation, "
             << "IceUtil.StringUtil.OrdinalStringComparer)";
        _out << nl << "If pos < 0 Then";
        _out.inc();
        _out << nl << "Throw New Ice.ObjectNotExistException(current__.id, current__.facet, current__.operation)";
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
                _out << nl << "Return ice_id___(Me, inS__, current__)";
            }
            else if(opName == "ice_ids")
            {
                _out << nl << "Return ice_ids___(Me, inS__, current__)";
            }
            else if(opName == "ice_isA")
            {
                _out << nl << "Return ice_isA___(Me, inS__, current__)";
            }
            else if(opName == "ice_ping")
            {
                _out << nl << "Return ice_ping___(Me, inS__, current__)";
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
                            _out << nl << "Return " << opName << "___(Me, inS__, current__)";
                        }
                        else
                        {
                            string base = cl->scoped();
                            if(cl->isInterface())
                            {
                                base += "Disp_";
                            }
                            _out << nl << "Return " << fixId(base) << "." << opName << "___(Me, inS__, current__)";
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
        _out << nl << "Throw New Ice.ObjectNotExistException(current__.id, current__.facet, current__.operation)";
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

string
Slice::VbVisitor::getParamAttributes(const ParamDeclPtr& p)
{
    string result;
    StringList metaData = p->getMetaData();
    for(StringList::const_iterator i = metaData.begin(); i != metaData.end(); ++i)
    {
        static const string prefix = "vb:attribute:";
        if(i->find(prefix) == 0)
        {
            result += "<" + i->substr(prefix.size()) + "> ";
        }
    }
    return result;
}

vector<string>
Slice::VbVisitor::getParams(const OperationPtr& op)
{
    vector<string> params;
    ParamDeclList paramList = op->parameters();
    for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
        string param = getParamAttributes(*q);
        param += ((*q)->isOutParam() ? "<_System.Runtime.InteropServices.Out()> ByRef " : "ByVal ")
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
    string param = "ByVal cb__ As ";
    string scope = fixId(cl->scope());
    param += scope + (amd ? "AMD_" : "AMI_") + cl->name() + "_" + op->name();
    params.push_back(param);

    ParamDeclList paramList = op->parameters();
    for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
        if(!(*q)->isOutParam())
        {
            params.push_back(getParamAttributes(*q) + "ByVal " + fixId((*q)->name()) + " As "
                             + typeToString((*q)->type()));
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
        params.push_back("ByVal ret__ As " + typeToString(ret));
    }

    ParamDeclList paramList = op->parameters();
    for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
        if((*q)->isOutParam())
        {
            params.push_back(getParamAttributes(*q) + "ByVal " + fixId((*q)->name()) + " As "
                             + typeToString((*q)->type()));
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

    args.push_back("cb__");

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
        args.push_back("ret__");
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

void
Slice::VbVisitor::emitAttributes(const ContainedPtr& p)
{
    StringList metaData = p->getMetaData();
    for(StringList::const_iterator i = metaData.begin(); i != metaData.end(); ++i)
    {
        static const string prefix = "vb:attribute:";
        if(i->find(prefix) == 0)
        {
            _out << nl << '<' << i->substr(prefix.size()) << '>';
        }
    }
}

Slice::Gen::Gen(const string& name, const string& base, const vector<string>& includePaths, const string& dir,
                bool impl, bool implTie, bool stream)
    : _includePaths(includePaths),
      _stream(stream)
{
    string fileBase = base;
    string::size_type pos = base.find_last_of("/\\");
    if(pos != string::npos)
    {
        fileBase = base.substr(pos + 1);
    }
    string file = fileBase + ".vb";
    string fileImpl = fileBase + "I.vb";

    if(!dir.empty())
    {
        file = dir + '/' + file;
        fileImpl = dir + '/' + fileImpl;
    }

    _out.open(file.c_str());
    if(!_out)
    {
        cerr << name << ": can't open `" << file << "' for writing" << endl;
        return;
    }
    printHeader();

    _out << nl << "' Generated from file `" << fileBase << ".ice'";

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

    UnitVisitor unitVisitor(_out, _stream);
    p->visit(&unitVisitor, false);

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
        string className = "X" + IceUtil::generateUUID();
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
        for(ChecksumMap::const_iterator q = map.begin(); q != map.end(); ++q)
        {
            _out << nl << "map.Add(\"" << q->first << "\", \"";
            ostringstream str;
            str.flags(ios_base::hex);
            str.fill('0');
            for(vector<unsigned char>::const_iterator r = q->second.begin(); r != q->second.end(); ++r)
            {
                str << (int)(*r);
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
"' Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.\n"
"'\n"
"' This copy of Ice is licensed to you under the terms described in the\n"
"' ICE_LICENSE file included in this distribution.\n"
"'\n"
"' **********************************************************************\n"
        ;

    _out << header;
    _out << "\n' Ice version " << ICE_STRING_VERSION;
}

Slice::Gen::UnitVisitor::UnitVisitor(IceUtil::Output& out, bool stream)
    : VbVisitor(out), _stream(stream), _globalMetaDataDone(false)
{
}

bool
Slice::Gen::UnitVisitor::visitModuleStart(const ModulePtr& p)
{
    if(!_globalMetaDataDone)
    {
        DefinitionContextPtr dc = p->definitionContext();
        StringList globalMetaData = dc->getMetaData();

        static const string attributePrefix = "vb:attribute:";

        if(!globalMetaData.empty())
        {
            _out << sp;
        }
        for(StringList::const_iterator q = globalMetaData.begin(); q != globalMetaData.end(); ++q)
        {
            string::size_type pos = q->find(attributePrefix);
            if(pos == 0)
            {
                string attrib = q->substr(pos + attributePrefix.size());
                _out << nl << '<' << attrib << '>';
            }
        }
        _globalMetaDataDone = true; // Do this only once per source file.
    }
    return false;
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
    _out << sp;
    emitAttributes(p);
    _out << nl << "Namespace " << name;

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
    string name = fixId(p->name());
    string scoped = fixId(p->scoped());
    ClassList bases = p->bases();
    bool hasBaseClass = !bases.empty() && !bases.front()->isInterface();

    if(!p->isLocal() && _stream)
    {
        _out << sp << nl << "Public NotInheritable Class " << p->name() << "Helper";
        _out.inc();

        _out << sp << nl << "Public Sub New(ByVal inS__ As Ice.InputStream)";
        _out.inc();
        _out << nl << "_in = inS__";
        _out << nl << "_pp = New IceInternal.ParamPatcher(GetType(" << scoped << "), \"" << p->scoped() << "\")";
        _out.dec();
        _out << nl << "End Sub";

        _out << sp << nl << "Public Shared Sub write(ByVal outS__ As Ice.OutputStream, ByVal v__ As " << name << ')';
        _out.inc();
        _out << nl << "outS__.writeObject(v__)";
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
        _out << sp << nl << "End Class";
    }

    _out << sp;
    emitAttributes(p);
    if(p->isInterface())
    {
        _out << nl << "Public Interface " << name;
        _out.inc();
        if(p->isLocal())
        {
            _out << nl << "Inherits Ice.LocalObject";
        }
        else
        {
            _out << nl << "Inherits Ice.Object";
        }
        _out << ", " << p->name();
        if(!p->isLocal())
        {
            _out << "Operations_, " << p->name();
        }
        _out << "OperationsNC_";
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
        _out << nl << "Public ";
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
            _out << nl << "Implements " << p->name();
            if(!p->isLocal())
            {
                _out << "Operations_, " << p->name();
            }
            _out << "OperationsNC_";
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

        _out.zeroIndent();
        _out << sp << nl << "#Region \"Marshaling support\"";
        _out.restoreIndent();

        _out << sp << nl << "Public Overloads Overrides Sub write__(ByVal os__ As IceInternal.BasicStream)";
        _out.inc();
        _out << nl << "os__.writeTypeId(ice_staticId())";
        _out << nl << "os__.startWriteSlice()";
        for(d = members.begin(); d != members.end(); ++d)
        {
            writeMarshalUnmarshalCode(_out, (*d)->type(), fixId((*d)->name(), DotNet::ICloneable, true), true, false,
                                      false);
        }
        _out << nl << "os__.endWriteSlice()";
        _out << nl << "MyBase.write__(os__)";
        _out.dec();
        _out << nl << "End Sub";

        DataMemberList allClassMembers = p->allClassDataMembers();
        if(allClassMembers.size() != 0)
        {
            _out << sp << nl << "Public Shadows NotInheritable Class Patcher__";
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
            _out << nl << "Try";
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
                _out << nl << "type_ = GetType(" << memberType << ')';
                _out << nl << "_instance." << memberName << " = CType(v, " << memberType << ')';
                _out << nl << "_typeId = \"" << (*d)->type()->typeId() << "\"";
                if(allClassMembers.size() > 1)
                {
                    _out.dec();
                }
                memberCount++;
            }
            if(allClassMembers.size() > 1)
            {
                _out << nl << "End Select";
            }
            _out.dec();
            _out << nl << "Catch _ex As System.InvalidCastException";
            _out.inc();
            _out << nl << "Dim _e As Ice.UnexpectedObjectException = New Ice.UnexpectedObjectException";
            _out << nl << "_e.type = v.ice_id()";
            _out << nl << "_e.expectedType = _typeId";
            _out << nl << "Throw _e";
            _out.dec();
            _out << nl << "End Try";
            _out.dec();
            _out << nl << "End Sub";

            _out << sp << nl << "Private _instance As " << name;
            if(allClassMembers.size() > 1)
            {
                _out << nl << "Private _member As Integer";
            }
            _out << nl << "Private _typeId As String";
            _out.dec();
            _out << sp << nl << "End Class";
        }

        _out << sp << nl << "Public Overloads Overrides Sub read__(ByVal is__ As IceInternal.BasicStream, "
                            "ByVal rid__ As Boolean)";
        _out.inc();
        _out << nl << "If rid__ Then";
        _out.inc();
        _out << nl << "Dim myId As String = is__.readTypeId()";
        _out.dec();
        _out << nl << "End If";
        _out << nl << "is__.startReadSlice()";
        DataMemberList classMembers = p->classDataMembers();
        int classMemberCount = static_cast<int>(allClassMembers.size() - classMembers.size());
        for(d = members.begin(); d != members.end(); ++d)
        {
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
        _out << nl << "is__.endReadSlice()";
        _out << nl << "MyBase.read__(is__, true)";
        _out.dec();
        _out << nl << "End Sub";

        //
        // Write streaming API.
        //
        if(_stream)
        {
            _out << sp << nl << "Public Overloads Overrides Sub write__(ByVal outS__ As Ice.OutputStream)";
            _out.inc();
            _out << nl << "outS__.writeTypeId(ice_staticId())";
            _out << nl << "outS__.startSlice()";
            for(d = members.begin(); d != members.end(); ++d)
            {
                writeMarshalUnmarshalCode(_out, (*d)->type(),
                                          fixId((*d)->name(), DotNet::ICloneable, true),
                                          true, true, false);
            }
            _out << nl << "outS__.endSlice()";
            _out << nl << "MyBase.write__(outS__)";
            _out.dec();
            _out << nl << "End Sub";

            _out << sp << nl << "Public Overloads Overrides Sub read__"
                 << "(ByVal inS__ As Ice.InputStream, ByVal rid__ As Boolean)";
            _out.inc();
            _out << nl << "If rid__ Then";
            _out.inc();
            _out << nl << "Dim myId As String = inS__.readTypeId()";
            _out.dec();
            _out << nl << "End If";
            _out << nl << "inS__.startSlice()";
            for(d = members.begin(); d != members.end(); ++d)
            {
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
            _out << nl << "inS__.endSlice()";
            _out << nl << "MyBase.read__(inS__, True)";
            _out.dec();
            _out << nl << "End Sub";
        }
        else
        {
            //
            // Emit placeholder functions to catch errors.
            //
            string scoped = p->scoped();
            _out << sp << nl << "Public Overloads Overrides Sub write__(ByVal outS__ As Ice.OutputStream)";
            _out.inc();
            _out << nl << "Dim ex As Ice.MarshalException = New Ice.MarshalException";
            _out << nl << "ex.reason = \"type " << scoped.substr(2) << " was not generated with stream support\"";
            _out << nl << "Throw ex";
            _out.dec();
            _out << nl << "End Sub";

            _out << sp << nl << "Public Overloads Overrides Sub read__"
                 << "(ByVal inS__ As Ice.InputStream, ByVal rid__ As Boolean)";
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

    _out << sp;
    emitAttributes(p);
    _out << nl << "Public ";
    if(isLocal)
    {
        _out << "MustOverride ";
    }
    _out << vbOp << ' ' << name << spar << params << epar;
    if(ret)
    {
        _out << " As " << retS;
    }
    _out << " Implements " << classDef->name() << "OperationsNC_." << name;
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
             << spar << params << "ByVal current__ As Ice.Current" << epar;
        if(ret)
        {
            _out << " As " << retS;
        }
        _out << " Implements " << classDef->name() << "Operations_." << name;
    }
}

void
Slice::Gen::TypesVisitor::visitSequence(const SequencePtr& p)
{
    //
    // No need to generate anything if the sequence is mapped as an array.
    //
    if(!p->hasMetaData("clr:collection"))
    {
        return;
    }

    string name = fixId(p->name());
    string s = typeToString(p->type());
    bool isValue = isValueType(p->type());

    _out << sp;

    emitDeprecate(p, 0, _out, "type");

    emitAttributes(p);
    _out << nl << "Public Class " << name;
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

    _out << sp << nl << "Public Sub New(ByVal a__ As " << s << "())";
    _out.inc();
    _out << nl << "InnerList.AddRange(a__)";
    _out.dec();
    _out << nl << "End Sub";

    _out.zeroIndent();
    _out << sp << nl << "#End Region"; // Constructors

    _out << sp << nl << "#Region \"Array copy and conversion\"";
    _out.restoreIndent();

    _out << sp << nl << "Public Sub CopyTo(ByVal a__ As " << s << "())";
    _out.inc();
    _out << nl << "InnerList.CopyTo(a__)";
    _out.dec();
    _out << nl << "End Sub";

    _out << sp << nl << "Public Sub CopyTo(ByVal a__ As " << s << "(), ByVal i__ As Integer)";
    _out.inc();
    _out << nl << "InnerList.CopyTo(a__, i__)";
    _out.dec();
    _out << nl << "End Sub";

    _out << sp << nl << "Public Sub CopyTo(ByVal i__ As Integer, ByVal a__ As " << s << "(), "
                        "ByVal ai__ As Integer, ByVal c__ As Integer)";
    _out.inc();
    _out << nl << "InnerList.CopyTo(i__, a__, ai__, c__)";
    _out.dec();
    _out << nl << "End Sub";

    _out << sp << nl << "Public Function ToArray() As " << s << "()";
    _out.inc();
    _out << nl << "Dim a__ As " << s << "() = New " << toArrayAlloc(s + "()", "InnerList.Count - 1") << " {}";
    _out << nl << "InnerList.CopyTo(a__)";
    _out << nl << "Return a__";
    _out.dec();
    _out << nl << "End Function";

    _out.zeroIndent();
    _out << sp << nl << "#End Region"; // Array copy and conversion

    _out << sp << nl << "#Region \"ArrayList members\"";
    _out.restoreIndent();

    _out << sp << nl << "#If VBC_VER = Nothing Then";
    _out << nl << "Property Capacity As Integer";
    _out.inc();
    _out << nl << "Get";
    _out.inc();
    _out << nl << "Return InnerList.Capacity";
    _out.dec();
    _out << nl << "End Get";
    _out << nl << "Set";
    _out.inc();
    _out << nl << "InnerList.Capacity = value";
    _out.dec();
    _out << nl << "End Set";
    _out.dec();
    _out << nl << "End Property";
    _out << nl << "#End If";

    _out << sp << nl << "Public Overridable Sub TrimToSize()";
    _out.inc();
    _out << nl << "InnerList.TrimToSize()";
    _out.dec();
    _out << nl << "End Sub";

    _out << sp << nl << "Public Overloads Overridable Sub Sort()";
    _out.inc();
    _out << nl << "InnerList.Sort()";
    _out.dec();
    _out << nl << "End Sub";

    _out << sp << nl << "Public Overloads Overridable Sub Sort(ByVal comparer As _System.Collections.IComparer)";
    _out.inc();
    _out << nl << "InnerList.Sort(comparer)";
    _out.dec();
    _out << nl << "End Sub";

    _out << sp << nl << "Public Overloads Overridable Sub Sort(";
    _out.useCurrentPosAsIndent();
    _out << "ByVal index As Integer, ByVal count As Integer, _";
    _out << nl << "ByVal Comparer As _System.Collections.IComparer)";
    _out.restoreIndent();
    _out.inc();
    _out << nl << "InnerList.Sort(index, count, comparer)";
    _out.dec();
    _out << nl << "End Sub";

    _out << sp << nl << "Public Sub Reverse()";
    _out.inc();
    _out << nl << "InnerList.Reverse()";
    _out.dec();
    _out << nl << "End Sub";

    _out << sp << nl << "Public Overridable Sub Reverse(ByVal index As Integer, ByVal count As Integer)";
    _out.inc();
    _out << nl << "InnerList.Reverse(index, count)";
    _out.dec();
    _out << nl << "End Sub";

    _out << sp << nl << "Public Overridable Function BinarySearch(ByVal value As " << s << ") As Integer";
    _out.inc();
    _out << nl << "Return InnerList.BinarySearch(value)";
    _out.dec();
    _out << nl << "End Function";

    _out << sp << nl << "Public Overridable Function BinarySearch(";
    _out.useCurrentPosAsIndent();
    _out << "ByVal value As " << s << ", _";
    _out << nl << "ByVal comparer As _System.Collections.IComparer) As Integer";
    _out.restoreIndent();
    _out.inc();
    _out << nl << "Return InnerList.BinarySearch(value, comparer)";
    _out.dec();
    _out << nl << "End Function";

    _out << sp << nl << "Public Overridable Function BinarySearch(";
    _out.useCurrentPosAsIndent();
    _out << "ByVal index As Integer, _";
    _out << nl << "ByVal count As Integer, _";
    _out << nl << "ByVal value As " << s << ", _";
    _out << nl << "ByVal comparer As _System.Collections.IComparer) As Integer";
    _out.restoreIndent();
    _out.inc();
    _out << nl << "Return InnerList.BinarySearch(index, count, value, comparer)";
    _out.dec();
    _out << nl << "End Function";

    _out << sp << nl << "Public Overridable Sub InsertRange(ByVal index As Integer, ByVal c As " << name << ')';
    _out.inc();
    _out << nl << "InnerList.InsertRange(index, c)";
    _out.dec();
    _out << nl << "End Sub";

    _out << sp << nl << "Public Overridable Sub InsertRange(ByVal index As Integer, ByVal c As " << s << "())";
    _out.inc();
    _out << nl << "InnerList.InsertRange(index, c)";
    _out.dec();
    _out << nl << "End Sub";

    _out << sp << nl << "Public Overridable Sub RemoveRange(ByVal index As Integer, ByVal count As Integer)";
    _out.inc();
    _out << nl << "InnerList.RemoveRange(index, count)";
    _out.dec();
    _out << nl << "End Sub";

    _out << sp << nl << "Public Overridable Function GetRange(ByVal index As Integer, ByVal count As Integer) As " << name;
    _out.inc();
    _out << nl << "Dim al As _System.Collections.ArrayList = InnerList.GetRange(index, count)";
    _out << nl << "Dim r As " << name << " = New " << name << "(al.Count)";
    _out << nl << "r.InnerList.AddRange(al)";
    _out << nl << "Return r";
    _out.dec();
    _out << nl << "End Function";

    _out << sp << nl << "Public Overridable Sub SetRange(ByVal index As Integer, ByVal c As " << name << ')';
    _out.inc();
    _out << nl << "InnerList.SetRange(index, c)";
    _out.dec();
    _out << nl << "End Sub";

    _out << sp << nl << "Public Overridable Sub SetRange(ByVal index As Integer, ByVal c As " << s << "())";
    _out.inc();
    _out << nl << "InnerList.SetRange(index, c)";
    _out.dec();
    _out << nl << "End Sub";

    _out << sp << nl << "Public Overridable Function LastIndexOf(ByVal value As " << s << ") As Integer";
    _out.inc();
    _out << nl << "Return InnerList.LastIndexOf(value)";
    _out.dec();
    _out << nl << "End Function";

    _out << sp << nl << "Public Overridable Function LastIndexOf(ByVal value As " << s << ", ByVal startIndex As Integer) As Integer";
    _out.inc();
    _out << nl << "Return InnerList.LastIndexOf(value, startIndex)";
    _out.dec();
    _out << nl << "End Function";

    _out << sp << nl << "Public Overridable Function LastIndexOf(";
    _out.useCurrentPosAsIndent();
    _out << "ByVal value As " << s << ", _";
    _out << nl << "ByVal startIndex As Integer, _";
    _out << nl << "ByVal count As Integer) As Integer";
    _out.restoreIndent();
    _out.inc();
    _out << nl << "Return InnerList.LastIndexOf(value, startIndex, count)";
    _out.dec();
    _out << nl << "End Function";

    _out << sp << nl << "Public Shared Function Repeat(ByVal value As " << s << ", ByVal count As Integer) As " << name;
    _out.inc();
    _out << nl << "Dim r As " << name << " = New " << name << "(count)";
    _out << nl << "For i As Integer = 0 To count - 1";
    _out.inc();
    _out << nl << "r.Add(value)";
    _out.dec();
    _out << nl << "Next";
    _out << nl << "Return r";
    _out.dec();
    _out << nl << "End Function";

    _out.zeroIndent();
    _out << sp << nl << "#End Region"; // ArrayList members

    _out << sp << nl << "#Region \"AddRange members\"";
    _out.restoreIndent();

    _out << sp << nl << "Public Sub AddRange(ByVal s__ As " << name << ')';
    _out.inc();
    _out << nl << "InnerList.AddRange(s__)";
    _out.dec();
    _out << nl << "End Sub";

    _out << sp << nl << "Public Sub AddRange(ByVal a__ As " << s << "())";
    _out.inc();
    _out << nl << "InnerList.AddRange(a__)";
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
    _out << nl << "Dim s As " << name << " = New " << name;
    _out << nl << "s.InnerList.AddRange(InnerList)";
    _out << nl << "Return s";
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
    _out << nl << "For i__ As Integer = 0 To Count - 1";
    _out.inc();
    if(!isValue)
    {
        _out << nl << "If InnerList(i__) Is Nothing Then";
        _out.inc();
        _out << nl << "If Not CType(other, " << name << ")(i__) Is Nothing Then";
        _out.inc();
        _out << nl << "Return False";
        _out.dec();
        _out << nl << "End If";
        _out.dec();
        _out << nl << "Else";
        _out.inc();
    }
    _out << nl << "If Not InnerList(i__).Equals(CType(other, " << name << ")(i__)) Then";
    _out.inc();
    _out << nl << "Return False";
    _out.dec();
    _out << nl << "End If";
    _out.dec();
    if(!isValue)
    {
        _out.dec();
        _out << nl << "End If";
    }
    _out << nl << "Next";
    _out << nl << "Return True";
    _out.dec();
    _out << nl << "End Function";

    _out.zeroIndent();
    _out << sp << nl << "#End Region"; // Object members
    _out.restoreIndent();

    _out.dec();
    _out << sp << nl << "End Class";
}

bool
Slice::Gen::TypesVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    string name = fixId(p->name());
    ExceptionPtr base = p->base();

    _out << sp;

    emitDeprecate(p, 0, _out, "type");

    emitAttributes(p);
    _out << nl << "Public Class " << name;
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

    DataMemberList allDataMembers = p->allDataMembers();
    DataMemberList dataMembers = p->dataMembers();
    DataMemberList::const_iterator q;

    vector<string> allParamDecl;
    for(q = allDataMembers.begin(); q != allDataMembers.end(); ++q)
    {
        string memberName = fixId((*q)->name());
        string memberType = typeToString((*q)->type());
        allParamDecl.push_back("ByVal " + memberName + " As " + memberType);
    }

    vector<string> paramNames;
    for(q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        paramNames.push_back(fixId((*q)->name()));
    }

    vector<string> paramDecl;
    for(q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        string memberName = fixId((*q)->name());
        string memberType = typeToString((*q)->type());
        paramDecl.push_back("ByVal " + memberName + " As " + memberType);
    }

    vector<string> baseParamNames;
    DataMemberList baseDataMembers;

    if(p->base())
    {
        baseDataMembers = p->base()->allDataMembers();
        for(q = baseDataMembers.begin(); q != baseDataMembers.end(); ++q)
        {
            baseParamNames.push_back(fixId((*q)->name()));
        }
    }

    if(!dataMembers.empty())
    {
        _out.zeroIndent();
        _out << sp << nl << "#End Region"; // Slice data members
        _out.restoreIndent();
    }

    _out.zeroIndent();
    _out << sp << nl << "#Region \"Constructors\"";
    _out.restoreIndent();

    _out << sp << nl << "Public Sub New()";
    _out.inc();
    _out << nl << "MyBase.New()";
    _out.dec();
    _out << nl << "End Sub";

    _out << sp << nl << "Public Sub New(ByVal ex__ As _System.Exception)";
    _out.inc();
    _out << nl << "MyBase.New(ex__)";
    _out.dec();
    _out << nl << "End Sub";

    if(!allDataMembers.empty())
    {
        if(!dataMembers.empty())
        {
            _out << sp << nl << "Private Sub initDM__" << spar << paramDecl << epar;
            _out.inc();
            for(q = dataMembers.begin(); q != dataMembers.end(); ++q)
            {
                _out << nl << "Me." << fixId((*q)->name()) << " = " << fixId((*q)->name());
            }
            _out.dec();
            _out << nl << "End Sub";
        }

        _out << sp << nl << "Public Sub New" << spar << allParamDecl << epar;
        _out.inc();
        if(p->base() && allDataMembers.size() != dataMembers.size())
        {
            _out << nl << "MyBase.New" << spar << baseParamNames << epar;
        }
        if(!dataMembers.empty())
        {
            _out << nl << "initDM__" << spar << paramNames << epar;
        }
        _out.dec();
        _out << nl << "End Sub";

        vector<string> exceptionParam;
        exceptionParam.push_back("ex__");
        vector<string> exceptionDecl;
        exceptionDecl.push_back("ByVal ex__ As _System.Exception");
        _out << sp << nl << "Public Sub New" << spar << allParamDecl << exceptionDecl << epar;
        _out.inc();
        _out << nl << "MyBase.New" << spar;
        if(p->base() && allDataMembers.size() != dataMembers.size())
        {
            _out << baseParamNames;
        }
        _out << exceptionParam << epar;
        if(!dataMembers.empty())
        {
            _out << nl << "initDM__" << spar << paramNames << epar;
        }
        _out.dec();
        _out << nl << "End Sub";
    }
    _out.zeroIndent();
    _out << sp << nl << "#End Region"; // Constructors

    _out << sp << nl << "#Region \"Object members\"";
    _out.restoreIndent();

    _out << sp << nl << "Public Overrides Function GetHashCode() As Integer";
    _out.inc();
    _out << nl << "Dim h__ As Integer = 0";
    for(q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        string memberName = fixId((*q)->name(), DotNet::ApplicationException);
        bool isValue = isValueType((*q)->type());
        if(!isValue)
        {
            _out << nl << "If Not " << memberName << " Is Nothing Then";
            _out.inc();
        }
        _out << nl << "h__ = 5 * h__ + ";
        invokeObjectMethod((*q)->type(), memberName, "GetHashCode", "");
        if(!isValue)
        {
            _out.dec();
            _out << nl << "End If";
        }
    }
    _out << nl << "Return h__";
    _out.dec();
    _out << nl << "End Function";

    _out << sp << nl << "Public Overloads Overrides Function Equals(ByVal other__ As Object) As Boolean";
    _out.inc();
    _out << nl << "If other__ Is Nothing Then";
    _out.inc();
    _out << nl << "Return False";
    _out.dec();
    _out << nl << "End If";
    _out << nl << "If Object.ReferenceEquals(Me, other__) Then";
    _out.inc();
    _out << nl << "Return True";
    _out.dec();
    _out << nl << "End If";
    _out << nl << "If Not TypeOf other__ Is " << name << " Then";
    _out.inc();
    _out << nl << "Return False";
    _out.dec();
    _out << nl << "End If";
    for(q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        string memberName = fixId((*q)->name(), DotNet::ApplicationException);
        bool isValue = isValueType((*q)->type());

        if(!isValue)
        {
            _out << nl << "If " << memberName << " Is Nothing Then";
            _out.inc();
            _out << nl << "If Not (CType(other__, " + name + "))." << memberName << " Is Nothing";
            _out.inc();
            _out << nl << "Return False";
            _out.dec();
            _out << nl << "End If";
            _out.dec();
            _out << nl << "Else";
            _out.inc();
        }
        _out << nl << "If Not ";
        invokeObjectMethod((*q)->type(), memberName, "Equals", "CType(other__, " + name + ")." + memberName);
        _out << " Then";
        _out.inc();
        _out << nl << "Return False";
        _out.dec();
        _out << nl << "End If";
        if(!isValue)
        {
            _out.dec();
            _out << nl << "End If";
        }
    }
    _out << nl << "Return True";
    _out.dec();
    _out << nl << "End Function";

    _out.zeroIndent();
    _out << sp << nl << "#End Region"; // Object members
    _out.restoreIndent();

    if(!p->isLocal())
    {
        _out.zeroIndent();
        _out << sp << nl << "#Region \"Marshaling support\"";
        _out.restoreIndent();

        string scoped = p->scoped();
        ExceptionPtr base = p->base();


        _out << sp << nl << "Public Overloads Overrides Sub write__(ByVal os__ As IceInternal.BasicStream)";
        _out.inc();
        _out << nl << "os__.writeString(\"" << scoped << "\")";
        _out << nl << "os__.startWriteSlice()";
        for(q = dataMembers.begin(); q != dataMembers.end(); ++q)
        {
            writeMarshalUnmarshalCode(_out, (*q)->type(),
                                      fixId((*q)->name(), DotNet::ApplicationException),
                                      true, false, false);
        }
        _out << nl << "os__.endWriteSlice()";
        if(base)
        {
            _out << nl << "MyBase.write__(os__)";
        }
        _out.dec();
        _out << nl << "End Sub";

        DataMemberList allClassMembers = p->allClassDataMembers();
        if(allClassMembers.size() != 0)
        {
            _out << sp << nl << "Public Shadows NotInheritable Class Patcher__";
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
            _out << nl << "Try";
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
                _out << nl << "type_ = GetType(" << memberType << ')';
                _out << nl << "_instance." << memberName << " = CType(v, " << memberType << ')';
                _out << nl << "_typeId = \"" << (*q)->type()->typeId() << "\"";
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
            _out << nl << "Catch _ex As System.InvalidCastException";
            _out.inc();
            _out << nl << "Dim _e As Ice.UnexpectedObjectException = New Ice.UnexpectedObjectException";
            _out << nl << "_e.type = v.ice_id()";
            _out << nl << "_e.expectedType = _typeId";
            _out << nl << "Throw _e";
            _out.dec();
            _out << nl << "End Try";
            _out.dec();
            _out << nl << "End Sub";

            _out << sp << nl << "Private _instance As " << name;
            if(allClassMembers.size() > 1)
            {
                _out << nl << "Private _member As Integer";
            }
            _out << nl << "Private _typeId As String";
            _out.dec();
            _out << sp << nl << "End Class";
        }
        _out << sp << nl << "Public Overloads Overrides Sub read__(ByVal is__ As IceInternal.BasicStream, "
                            "ByVal rid__ As Boolean)";
        _out.inc();
        _out << nl << "If rid__ Then";
        _out.inc();
        _out << nl << "Dim myId As String = is__.readString()";
        _out.dec();
        _out << nl << "End If";
        _out << nl << "is__.startReadSlice()";
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
        _out << nl << "is__.endReadSlice()";
        if(base)
        {
            _out << nl << "MyBase.read__(is__, true)";
        }
        _out.dec();
        _out << nl << "End Sub";

        if(_stream)
        {
            _out << sp << nl << "Public Overloads Overrides Sub write__(ByVal outS__ As Ice.OutputStream)";
            _out.inc();
            _out << nl << "outS__.writeString(\"" << scoped << "\")";
            _out << nl << "outS__.startSlice()";
            for(q = dataMembers.begin(); q != dataMembers.end(); ++q)
            {
                writeMarshalUnmarshalCode(_out, (*q)->type(),
                                          fixId((*q)->name(), DotNet::ApplicationException),
                                          true, true, false);
            }
            _out << nl << "outS__.endSlice()";
            if(base)
            {
                _out << nl << "MyBase.write__(outS__)";
            }
            _out.dec();
            _out << nl << "End Sub";

            _out << sp << nl << "Public Overloads Overrides Sub read__(ByVal inS__ As Ice.InputStream, "
                 << "ByVal rid__ As Boolean)";
            _out.inc();
            _out << nl << "If rid__ Then";
            _out.inc();
            _out << nl << "Dim myId As String = inS__.readString()";
            _out.dec();
            _out << nl << "End If";
            _out << nl << "inS__.startSlice()";
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
            _out << nl << "inS__.endSlice()";
            if(base)
            {
                _out << nl << "MyBase.read__(inS__, true)";
            }
            _out.dec();
            _out << nl << "End Sub";
        }
        else
        {
            //
            // Emit placeholder functions to catch errors.
            //
            _out << sp << nl << "Public Overloads Overrides Sub write__(ByVal outS__ As Ice.OutputStream)";
            _out.inc();
            _out << nl << "Dim ex As Ice.MarshalException = New Ice.MarshalException";
            _out << nl << "ex.reason = \"type " << scoped.substr(2) << " was not generated with stream support\"";
            _out << nl << "Throw ex";
            _out.dec();
            _out << nl << "End Sub";

            _out << sp << nl << "Public Overloads Overrides Sub read__"
                 << "(ByVal inS__ As Ice.InputStream, ByVal rid__ As Boolean)";
            _out.inc();
            _out << nl << "Dim ex As Ice.MarshalException = New Ice.MarshalException";
            _out << nl << "ex.reason = \"type " << scoped.substr(2) << " was not generated with stream support\"";
            _out << nl << "Throw ex";
            _out.dec();
            _out << nl << "End Sub";
        }

        if(!base || base && !base->usesClasses())
        {
            _out << sp << nl << "Public Overrides Function usesClasses__() As Boolean";
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

        _out << sp << nl << "Public Shared Sub write(ByVal outS__ As Ice.OutputStream, ByVal v__ As " << name << ')';
        _out.inc();
        _out << nl << "v__.ice_write(outS__)";
        _out.dec();
        _out << nl << "End Sub";

        _out << sp << nl << "Public Shared Function read(ByVal inS__ As Ice.InputStream) As " << name;
        _out.inc();
        _out << nl << "Dim v__ As " << name << " = New " << name;
        _out << nl << "v__.ice_read(inS__)";
        _out << nl << "Return v__";
        _out.dec();
        _out << nl << "End Function";

        _out.dec();
        _out << sp << nl << "End Class";
    }

    _out << sp;

    emitDeprecate(p, 0, _out, "type");

    emitAttributes(p);
    if(isValueType(p))
    {
        _out << nl << "Public Structure " << name;
    }
    else
    {
        _out << nl << "Public Class " << name;
        _out.inc();
        _out << nl << " Implements _System.ICloneable";
        _out.dec();
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

    bool isClass = !isValueType(p);

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
    _out << nl << "Dim h__ As Integer = 0";
    for(q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        string memberName = fixId((*q)->name(), isClass ? DotNet::ICloneable : 0);
        bool isValue = isValueType((*q)->type());
        if(!isValue)
        {
            _out << nl << "If Not " << memberName << " Is Nothing Then";
            _out.inc();
        }
        _out << nl << "h__ = 5 * h__ + ";
        invokeObjectMethod((*q)->type(), memberName, "GetHashCode", "");
        if(!isValue)
        {
            _out.dec();
            _out << nl << "End If";
        }
    }
    _out << nl << "Return h__";
    _out.dec();
    _out << nl << "End Function";

    _out << sp << nl << "Public Overloads Overrides Function Equals(ByVal other__ As Object) As Boolean";
    _out.inc();
    if(isClass)
    {
        _out << nl << "If Object.ReferenceEquals(Me, other__) Then";
        _out.inc();
        _out << nl << "Return True";
        _out.dec();
        _out << nl << "End If";
    }
    if(isClass)
    {
        _out << nl << "If other__ Is Nothing Then";
        _out.inc();
        _out << nl << "Return False";
        _out.dec();
        _out << nl << "End If";
    }
    else
    {
        _out << nl << "If Not TypeOf other__ Is " << name << " Then";
        _out.inc();
        _out << nl << "Return False";
        _out.dec();
        _out << nl << "End If";
    }
    if(!dataMembers.empty())
    {
        _out << nl << "Dim o__ As " << name << " = CType(other__, " << name << ")";
    }
    for(q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        string memberName = fixId((*q)->name(), isClass ? DotNet::ICloneable : 0);
        if(!isValueType((*q)->type()))
        {
            _out << nl << "If " << memberName << " Is Nothing Then";
            _out.inc();
            _out << nl << "If Not o__." << memberName << " Is Nothing Then";
            _out.inc();
            _out << nl << "Return False";
            _out.dec();
            _out << nl << "End If";
            _out.dec();
            _out << nl << "Else";
            _out.inc();
            _out << nl << "If Not ";
            invokeObjectMethod((*q)->type(), memberName, "Equals", "o__." + memberName);
            _out << " Then";
            _out.inc();
            _out << nl << "Return False";
            _out.dec();
            _out << nl << "End If";
            _out.dec();
            _out << nl << "End If";
        }
        else
        {
            _out << nl << "If Not " << memberName << ".Equals(o__." << memberName << ") Then";
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
    _out.restoreIndent();

    if(!p->isLocal())
    {
        _out.zeroIndent();
        _out << sp << nl << "#Region \"Marshalling support\"";
        _out.restoreIndent();

        _out << sp << nl << "Public Sub write__(ByVal os__ As IceInternal.BasicStream)";
        _out.inc();
        for(q = dataMembers.begin(); q != dataMembers.end(); ++q)
        {
            writeMarshalUnmarshalCode(_out, (*q)->type(),
                                      fixId((*q)->name(), isClass ? DotNet::ICloneable : 0),
                                      true, false, false);
        }
        _out.dec();
        _out << nl << "End Sub";


        if(isClass && classMembers.size() != 0)
        {
            _out << sp << nl << "Public NotInheritable Class Patcher__";
            _out.inc();
            _out << nl << "Inherits IceInternal.Patcher";
            _out << sp << nl << "Friend Sub New(ByVal instance As " << name;
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
            _out << nl << "Try";
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
                _out << nl << "type_ = GetType(" << memberType << ')';
                _out << nl << "_instance." << memberName << " = CType(v, " << memberType << ')';
                _out << nl << "_typeId = \"" << (*q)->type()->typeId() << "\"";
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
            _out << nl << "Catch _ex As System.InvalidCastException";
            _out.inc();
            _out << nl << "Dim _e As Ice.UnexpectedObjectException = New Ice.UnexpectedObjectException";
            _out << nl << "_e.type = v.ice_id()";
            _out << nl << "_e.expectedType = _typeId";
            _out << nl << "Throw _e";
            _out.dec();
            _out << nl << "End Try";
            _out.dec();
            _out << nl << "End Sub";

            _out << sp << nl << "Private _instance As " << name;
            if(classMembers.size() > 1)
            {
                _out << nl << "Private _member As Integer";
            }
            _out << nl << "Private _typeId As String";
            _out.dec();
            _out << sp << nl << "End Class";
        }

        _out << sp << nl << "Public Sub read__(ByVal is__ As IceInternal.BasicStream)";
        _out.inc();
        int classMemberCount = 0;
        for(q = dataMembers.begin(); q != dataMembers.end(); ++q)
        {
            ostringstream patchParams;
            patchParams << "Me";
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
            _out << sp << nl << "Public Sub ice_write(ByVal outS__ As Ice.OutputStream)";
            _out.inc();
            for(q = dataMembers.begin(); q != dataMembers.end(); ++q)
            {
                writeMarshalUnmarshalCode(_out, (*q)->type(),
                                          fixId((*q)->name(), isClass ? DotNet::ICloneable : 0),
                                          true, true, false);
            }
            _out.dec();
            _out << nl << "End Sub";

            _out << sp << nl << "Public Sub ice_read(ByVal inS__ As Ice.InputStream)";
            _out.inc();
            classMemberCount = 0;
            for(q = dataMembers.begin(); q != dataMembers.end(); ++q)
            {
                ostringstream patchParams;
                patchParams << "Me";
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
    _out << sp << nl << "End " << (isClass ? "Class" : "Structure");
}

void
Slice::Gen::TypesVisitor::visitDictionary(const DictionaryPtr& p)
{
    string name = fixId(p->name());
    string ks = typeToString(p->keyType());
    string vs = typeToString(p->valueType());
    bool valueIsValue = isValueType(p->valueType());

    _out << sp;

    emitDeprecate(p, 0, _out, "type");

    emitAttributes(p);
    _out << nl << "Public Class " << name;
    _out.inc();
    _out << nl << "Inherits _System.Collections.DictionaryBase";
    _out << nl << "Implements _System.ICloneable";

    _out.zeroIndent();
    _out << sp << nl << "#Region \"" << name << " members\"";
    _out.restoreIndent();

    _out << sp << nl << "Public Sub AddRange(ByVal d__ As " << name << ')';
    _out.inc();
    _out << nl << "For Each e As _System.Collections.DictionaryEntry in d__";
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

    _out << sp << nl << "Public Function Contains(ByVal key As " << ks << ") As Boolean";
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
    _out << nl << "Dim d As " << name << " = New " << name;
    _out << nl << "For Each e As _System.Collections.DictionaryEntry in d";
    _out.inc();
    _out << nl << "d.InnerHashtable.Add(e.Key, e.Value)";
    _out.dec();
    _out << nl << "Next";
    _out << nl << "Return d";
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
    _out << nl << "Dim klhs__() As " << ks << " = New " << ks << "(Count - 1) {}";
    _out << nl << "Keys.CopyTo(klhs__, 0)";
    _out << nl << "_System.Array.Sort(klhs__)";
    _out << nl << "Dim krsh__() As " << ks << " = New " << ks << "(CType(other, " << name << ").Count - 1) {}";
    _out << nl << "CType(other, " << name << ").Keys.CopyTo(krsh__, 0)";
    _out << nl << "_System.Array.Sort(krsh__)";
    _out << nl << "For i As Integer = 0 To Count - 1";
    _out.inc();
    _out << nl << "If Not klhs__(i).Equals(krsh__(i))";
    _out.inc();
    _out << nl << "Return False";
    _out.dec();
    _out << nl << "End If";
    _out.dec();
    _out << nl << "Next";
    SequencePtr seq = SequencePtr::dynamicCast(p->valueType());
    bool valueIsArray = seq && !seq->hasMetaData("clr:collection");
    if(valueIsArray)
    {
        _out << nl << "Dim vlhs__ As " << vs << "() = New " << toArrayAlloc(vs + "()", "Count - 1") << " {}";
    }
    else
    {
        _out << nl << "Dim vlhs__ As " << vs << "() = New " << vs << "(Count - 1) {}";
    }
    _out << nl << "Values.CopyTo(vlhs__, 0)";
    _out << nl << "_System.Array.Sort(vlhs__)";
    string vrhsCount = "Ctype(other, " + name + ").Count - 1";
    if(valueIsArray)
    {
        _out << nl << "Dim vrhs__ As " << vs << "() = New " << toArrayAlloc(vs + "()", vrhsCount) << " {}";
    }
    else
    {
        _out << nl << "Dim vrhs__ As " << vs << "() = New " << vs << '(' << vrhsCount << ") {}";
    }
    _out << nl << "CType(other, " << name << ").Values.CopyTo(vrhs__, 0)";
    _out << nl << "_System.Array.Sort(vrhs__)";
    _out << nl << "For i As Integer = 0 To Count - 1";
    _out.inc();
    if(!valueIsValue)
    {
        _out << nl << "If vlhs__(i) Is Nothing";
        _out.inc();
        _out << nl << "If Not vrhs__(i) Is Nothing";
        _out.inc();
        _out << nl << "Return False";
        _out.dec();
        _out << nl << "End If";
        _out.dec();
        _out << nl << "ElseIf Not CType(vlhs__(i), Object).Equals(vrhs__(i)) Then";
        _out.inc();
        _out << nl << "Return False";
        _out.dec();
        _out << nl << "End If";
        _out.dec();
    }
    else
    {
        _out << nl << "If Not vlhs__(i).Equals(vrhs__(i)) Then";
        _out.inc();
        _out << nl << "Return False";
        _out.dec();
        _out << nl << "End If";
        _out.dec();
    }
    _out << nl << "Next";
    _out << nl << "Return True";
    _out.dec();
    _out << nl << "End Function";

    _out.zeroIndent();
    _out << sp << nl << "#End Region"; // Object members
    _out.restoreIndent();

    _out.dec();
    _out << sp << nl << "End Class";
}

void
Slice::Gen::TypesVisitor::visitEnum(const EnumPtr& p)
{
    string name = fixId(p->name());
    string scoped = fixId(p->scoped());
    _out << sp;

    emitDeprecate(p, 0, _out, "type");

    emitAttributes(p);
    _out << nl << "Public Enum " << name;
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

        _out << sp << nl << "Public Shared Sub write(ByVal outS__ As Ice.OutputStream, ByVal v__ As " << scoped << ')';
        _out.inc();
        writeMarshalUnmarshalCode(_out, p, "v__", true, true, false);
        _out.dec();
        _out << nl << "End Sub";

        _out << sp << nl << "Public Shared Function read(ByVal inS__ As Ice.InputStream) As " << scoped;
        _out.inc();
        _out << nl << "Dim v__ As " << scoped;
        writeMarshalUnmarshalCode(_out, p, "v__", false, true, false);
        _out << nl << "Return v__";
        _out.dec();
        _out << nl << "End Function";

        _out.dec();
        _out << sp << nl << "End Class";
    }
}

void
Slice::Gen::TypesVisitor::visitConst(const ConstPtr& p)
{
    string name = fixId(p->name());
    _out << sp;
    emitAttributes(p);
    _out << nl << "Public NotInheritable Class " << name;
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
    _out << sp << nl << "End Class";
}

void
Slice::Gen::TypesVisitor::visitDataMember(const DataMemberPtr& p)
{
    int baseTypes = 0;
    bool isClass = false;
    bool propertyMapping = false;
    bool isValue = false;
    ContainedPtr cont = ContainedPtr::dynamicCast(p->container());
    assert(cont);
    if(StructPtr::dynamicCast(cont))
    {
        isValue = isValueType(StructPtr::dynamicCast(cont));
        if(!isValue || cont->hasMetaData("clr:class"))
        {
            baseTypes = DotNet::ICloneable;
        }
        if(cont->hasMetaData("clr:property"))
        {
            propertyMapping = true;
        }
    }
    else if(ExceptionPtr::dynamicCast(cont))
    {
        baseTypes = DotNet::ApplicationException;
    }
    else if(ClassDefPtr::dynamicCast(cont))
    {
        baseTypes = DotNet::ICloneable;
        isClass = true;
        if(cont->hasMetaData("clr:property"))
        {
            propertyMapping = true;
        }
    }
    _out << sp;

    emitDeprecate(p, cont, _out, "member");

    emitAttributes(p);

    string type = typeToString(p->type());
    string propertyName = fixId(p->name(), baseTypes, isClass);
    string dataMemberName = propertyName;
    if(propertyMapping)
    {
        dataMemberName += "_prop";
    }

    _out << nl << (propertyMapping ? "Private" : "Public") << ' ' << dataMemberName << " As " << type;

    if(!propertyMapping)
    {
        return;
    }

    _out << nl << "Public";
    if(!isValue)
    {
        _out << " Overridable";
    }
    _out << " Property " << propertyName << " As " << type;
    _out.inc();
    _out << nl << "Get";
    _out.inc();
    _out << nl << "Return " << dataMemberName;
    _out.dec();
    _out << nl << "End Get";
    _out << nl << "Set";
    _out.inc();
    _out << nl << dataMemberName << " = Value";
    _out.dec();
    _out << nl << "End Set";
    _out.dec();
    _out << nl << "End Property";
}

void
Slice::Gen::TypesVisitor::invokeObjectMethod(const TypePtr& type, const string& obj, const string& method,
                                             const string& arg)
{
    BuiltinPtr b = BuiltinPtr::dynamicCast(type);
    ClassDeclPtr cl;
    if(!b)
    {
        cl = ClassDeclPtr::dynamicCast(type);
    }

    //
    // Visual Basic requires us to cast an interface type to Object in order to
    // invoke Object methods such as GetHashCode and Equals.
    //
    if((b && (b->kind() == Builtin::KindObject || b->kind() == Builtin::KindObjectProxy)) ||
       (cl && cl->isInterface()) || ProxyPtr::dynamicCast(type))
    {
        _out << "CType(" << obj << ", Object)." << method << '(' << arg << ')';
    }
    else
    {
        _out << obj << '.' << method << '(' << arg << ')';
    }
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

    _out << sp;
    emitAttributes(p);
    _out << nl << "Namespace " << fixId(p->name());
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
    ClassDefPtr cl = ClassDefPtr::dynamicCast(p->container());
    string name = fixId(p->name(), DotNet::ICloneable, true);
    vector<string> params = getParams(p);

    TypePtr ret = p->returnType();
    string retS = typeToString(ret);
    string vbOp = ret ? "Function" : "Sub";

    _out << sp;

    string deprecateMetadata, deprecateReason;
    if(p->findMetaData("deprecate", deprecateMetadata) || cl->findMetaData("deprecate", deprecateMetadata))
    {
        deprecateReason = "This operation has been deprecated.";
        if(deprecateMetadata.find("deprecate:") == 0 && deprecateMetadata.size() > 10)
        {
            deprecateReason = deprecateMetadata.substr(10);
        }
    }

    //
    // Write two versions of the operation - with and without a
    // context parameter.
    //
    if(!deprecateReason.empty())
    {
        _out << nl << "<System.Obsolete(\"" << deprecateReason << "\")>";
    }
    _out << nl << vbOp << ' ' << name << spar << params << epar;
    if(ret)
    {
        _out << " As " << retS;
    }

    if(!deprecateReason.empty())
    {
        _out << nl << "<System.Obsolete(\"" << deprecateReason << "\")>";
    }
    _out << nl << vbOp << ' ' << name << spar << params << "ByVal context__ As Ice.Context" << epar; 
    if(ret)
    {
        _out << " As " << retS;
    }

    if(cl->hasMetaData("ami") || p->hasMetaData("ami"))
    {
        vector<string> paramsAMI = getParamsAsync(p, false);

        //
        // Write two versions of the operation - with and without a
        // context parameter.
        //
        _out << sp;
        if(!deprecateReason.empty())
        {
            _out << nl << "<System.Obsolete(\"" << deprecateReason << "\")>";
        }
        _out << nl << "Sub " << p->name() << "_async" << spar << paramsAMI << epar;
        if(!deprecateReason.empty())
        {
            _out << nl << "<System.Obsolete(\"" << deprecateReason << "\")>";
        }
        _out << nl << "Sub " << p->name() << "_async" << spar << paramsAMI << "ByVal ctx__ As Ice.Context" << epar;
    }
}

Slice::Gen::OpsVisitor::OpsVisitor(IceUtil::Output& out)
    : VbVisitor(out)
{
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

    _out << sp << nl << "Public Interface " << name << opIntfName << '_';
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
                s += "Operations";
                if(noCurrent)
                {
                    s += "NC";
                }
                s += "_";
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
        string opname = amd ? (op->name() + "_async") : fixId(op->name(), DotNet::ICloneable, true);

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

        _out << sp;

        emitDeprecate(op, p, _out, "operation");

        emitAttributes(op);
        _out << nl << vbOp << ' ' << opname << spar << params;
        if(!noCurrent && !p->isLocal())
        {
            _out << "ByVal current__ As Ice.Current";
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
        _out << opName << spar << args << "Nothing" << "False" << epar;
        _out.dec();
        _out << nl << "End " << vbOp;


        _out << sp << nl << "Public " << vbOp << ' ' << opName << spar << params
             << "ByVal context__ As Ice.Context" << epar;
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
        _out << opName << spar << args << "context__" << "True" << epar;
        _out.dec();
        _out << nl << "End " << vbOp;

        _out << sp << nl << "Private " << vbOp << ' ' << opName << spar << params
             << "ByVal context__ As Ice.Context" << "explicitContext__ As Boolean" << epar;
        if(ret)
        {
            _out << " As " << retS;
        }
        _out.inc();
        _out << nl << "If explicitContext__ And context__ Is Nothing Then";
        _out.inc();
        _out << nl << "context__ = emptyContext_";
        _out.dec();
        _out << nl << "End If";
        _out << nl << "Dim cnt__ As Integer = 0";
        _out << nl << "While True";
        _out.inc();
        _out << nl << "Dim delBase__ As Ice.ObjectDel_ = Nothing";
        _out << nl << "Try";
        _out.inc();
        if(op->returnsData())
        {
            _out << nl << "checkTwowayOnly__(\"" << op->name() << "\")";
        }
        _out << nl << "delBase__ = getDelegate__()";
        _out << nl << "Dim del__ As _" << name << "Del = CType(delBase__, _" << name << "Del)";
        _out << nl;
        if(ret)
        {
            _out << "Return ";
        }
        _out << "del__." << opName << spar << args << "context__" << epar;
        if(!ret)
        {
            _out << nl << "Return";
        }
        _out.dec();
        _out << nl << "Catch ex__ As IceInternal.LocalExceptionWrapper";
        _out.inc();
        if(op->mode() == Operation::Idempotent || op->mode() == Operation::Nonmutating)
        {
            _out << nl << "cnt__ = handleExceptionWrapperRelaxed__(delBase__, ex__, cnt__)";
        }
        else
        {
            _out << nl << "handleExceptionWrapper__(delBase__, ex__)";
        }
        _out.dec();
        _out << nl << "Catch ex__ As Ice.LocalException";
        _out.inc();
        _out << nl << "cnt__ = handleException__(delBase__, ex__, cnt__)";
        _out.dec();
        _out << nl << "End Try";
        _out.dec();
        _out << nl << "End While";
        if(ret)
        {
            _out << nl << "Return Nothing"; // Satisfy the VB2005 compiler.
        }

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
            _out << nl << opName << "_async" << spar << argsAMI << "Nothing" << epar;
            _out.dec();
            _out << nl << "End Sub";

            _out << sp << nl << "Public Sub " << opName << "_async" << spar << paramsAMI
                 << "ByVal ctx__ As Ice.Context" << epar
                 << " Implements " << name << "Prx." << opName << "_async"; // TODO: should be containing class?
            _out.inc();
            _out << nl << "If ctx__ Is Nothing Then";
            _out.inc();
            _out << nl << "ctx__ = emptyContext_";
            _out.dec();
            _out << nl << "End If";
            _out << nl << "cb__.invoke__" << spar << "Me" << argsAMI << "ctx__" << epar;
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
    _out << nl << "h.copyFrom__(b)";
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
    _out << nl << "h.copyFrom__(b)";
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
    _out << nl << "Dim bb As Ice.ObjectPrx = b.ice_facet(f)";
    _out << nl << "Try";
    _out.inc();
    _out << nl << "If bb.ice_isA(\"" << p->scoped() << "\") Then";
    _out.inc();
    _out << nl << "Dim h As " << name << "PrxHelper = new " << name << "PrxHelper()";
    _out << nl << "h.copyFrom__(bb)";
    _out << nl << "Return h";
    _out.dec();
    _out << nl << "End If";
    _out.dec();
    _out << nl << "Catch ex__ As Ice.FacetNotExistException";
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
    _out << nl << "Dim bb As Ice.ObjectPrx = b.ice_facet(f)";
    _out << nl << "Try";
    _out.inc();
    _out << nl << "If bb.ice_isA(\"" << p->scoped() << "\", ctx) Then";
    _out.inc();
    _out << nl << "Dim h As " << name << "PrxHelper = new " << name << "PrxHelper()";
    _out << nl << "h.copyFrom__(bb)";
    _out << nl << "Return h";
    _out.dec();
    _out << nl << "End If";
    _out.dec();
    _out << nl << "Catch ex__ As Ice.FacetNotExistException";
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
    _out << nl << "h.copyFrom__(b)";
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
    _out << nl << "Dim bb As Ice.ObjectPrx = b.ice_facet(f)";
    _out << nl << "Dim h As " << name << "PrxHelper = new " << name << "PrxHelper()";
    _out << nl << "h.copyFrom__(bb)";
    _out << nl << "Return h";
    _out.dec();
    _out << nl << "End Function";

    _out.zeroIndent();
    _out << sp << nl << "#End Region"; // Checked and unchecked cast operations

    _out << sp << nl << "#Region \"Marshaling support\"";
    _out.restoreIndent();

    _out << sp << nl << "Protected Overrides Function createDelegateM__() As Ice.ObjectDelM_";
    _out.inc();
    _out << nl << "Return New _" << name << "DelM()";
    _out.dec();
    _out << nl << "End Function";

    _out << sp << nl << "Protected Overrides Function createDelegateD__() As Ice.ObjectDelD_";
    _out.inc();
    _out << nl << "Return New _" << name << "DelD()";
    _out.dec();
    _out << nl << "End Function";

    _out << sp << nl << "Public Shared Sub write__(ByVal os__ As IceInternal.BasicStream, ByVal v__ As "
         << name << "Prx)";
    _out.inc();
    _out << nl << "os__.writeProxy(v__)";
    _out.dec();
    _out << nl << "End Sub";

    _out << sp << nl << "Public Shared Function read__(ByVal is__ As IceInternal.BasicStream) As " << name << "Prx";
    _out.inc();
    _out << nl << "Dim proxy As Ice.ObjectPrx = is__.readProxy()";
    _out << nl << "If Not proxy Is Nothing Then";
    _out.inc();
    _out << nl << "Dim result As " << name << "PrxHelper = New " << name << "PrxHelper";
    _out << nl << "result.copyFrom__(proxy)";
    _out << nl << "Return result";
    _out.dec();
    _out << nl << "End If";
    _out << nl << "Return Nothing";
    _out.dec();
    _out << nl << "End Function";

    if(_stream)
    {
        _out << sp << nl << "Public Shared Sub write(ByVal outS__ As Ice.OutputStream, ByVal v__ As " << name << "Prx)";
        _out.inc();
        _out << nl << "outS__.writeProxy(v__)";
        _out.dec();
        _out << nl << "End Sub";

        _out << sp << nl << "Public Shared Function read(ByVal inS__ As Ice.InputStream) As " << name << "Prx";
        _out.inc();
        _out << nl << "Dim proxy As Ice.ObjectPrx = inS__.readProxy()";
        _out << nl << "If Not proxy Is Nothing";
        _out.inc();
        _out << nl << "Dim result As " << name << "PrxHelper = New " << name << "PrxHelper";
        _out << nl << "result.copyFrom__(proxy)";
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

    _out << sp << nl << "Public Shared Sub write(ByVal os__ As IceInternal.BasicStream, ByVal v__ As " << typeS << ')';
    _out.inc();
    writeSequenceMarshalUnmarshalCode(_out, p, "v__", true, false);
    _out.dec();
    _out << nl << "End Sub";

    _out << sp << nl << "Public Shared Function read(ByVal is__ As IceInternal.BasicStream) As " << typeS;
    _out.inc();
    _out << nl << "Dim v__ As " << typeS << " = Nothing";
    writeSequenceMarshalUnmarshalCode(_out, p, "v__", false, false);
    _out << nl << "Return v__";
    _out.dec();
    _out << nl << "End Function";

    if(_stream)
    {
        _out << sp << nl << "Public Shared Sub write(ByVal outS__ As Ice.OutputStream, ByVal v__ As " << typeS << ')';
        _out.inc();
        writeSequenceMarshalUnmarshalCode(_out, p, "v__", true, true);
        _out.dec();
        _out << nl << "End Sub";

        _out << sp << nl << "Public Shared Function read(ByVal inS__ As Ice.InputStream) As " << typeS;
        _out.inc();
        _out << nl << "Dim v__ As " << typeS << " = Nothing";
        writeSequenceMarshalUnmarshalCode(_out, p, "v__", false, true);
        _out << nl << "Return v__";
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

    _out << sp << nl << "Public Shared Sub write(ByVal os__ As IceInternal.BasicStream, ByVal v__ As "
         << name << ')';
    _out.inc();
    _out << nl << "If v__ Is Nothing Then";
    _out.inc();
    _out << nl << "os__.writeSize(0)";
    _out.dec();
    _out << nl << "Else";
    _out.inc();
    _out << nl << "os__.writeSize(v__.Count)";
    _out << nl << "For Each e__ As _System.Collections.DictionaryEntry In v__";
    _out.inc();
    string keyArg = "CType(e__.Key, " + keyS + ")";
    writeMarshalUnmarshalCode(_out, key, keyArg, true, false, false);
    string valueArg = "e__.Value";
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
        _out << sp << nl << "Public NotInheritable Class Patcher__";
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
        _out << nl << "type_ = GetType(" << typeToString(p->valueType()) << ')';
        _out << nl << "Try";
        _out.inc();
        _out << nl << "_m(_key) = CType(v, " << valueS << ')';
        _out.dec();
        _out << nl << "Catch _ex As System.InvalidCastException";
        _out.inc();
        _out << nl << "Dim _e As Ice.UnexpectedObjectException = New Ice.UnexpectedObjectException";
        _out << nl << "_e.type = v.ice_id()";
        _out << nl << "_e.expectedType = \"" << value->typeId() << "\"";
        _out << nl << "Throw _e";
        _out.dec();
        _out << nl << "End Try";
        _out.dec();
        _out << nl << "End Sub";

        _out << sp << nl << "Private _m As " << name;
        _out << nl << "Private _key As " << keyS;
        _out.dec();
        _out << sp << nl << "End Class";
    }

    _out << sp << nl << "Public Shared Function " << " read(ByVal is__ As IceInternal.BasicStream) As " << name;
    _out.inc();

    _out << nl << "Dim sz__ As Integer = is__.readSize()";
    _out << nl << "Dim r__ As " << name << " = New " << name;
    _out << nl << "For i__ As Integer = 0 To sz__ - 1";
    _out.inc();
    _out << nl << "Dim k__ As " << keyS;
    writeMarshalUnmarshalCode(_out, key, "k__", false, false, false);

    if(!hasClassValue)
    {
        _out << nl << "Dim v__ As " << valueS;
    }
    writeMarshalUnmarshalCode(_out, value, "v__", false, false, false, "r__, k__");
    if(!hasClassValue)
    {
        _out << nl << "r__(k__) = v__";
    }
    _out.dec();
    _out << nl << "Next";
    _out << nl << "Return r__";

    _out.dec();
    _out << nl << "End Function";

    if(_stream)
    {
        _out << nl << "Public Shared Sub write(ByVal outS__ As Ice.OutputStream, ByVal v__ As " << name << ')';
        _out.inc();
        _out << nl << "If v__ Is Nothing Then";
        _out.inc();
        _out << nl << "outS__.writeSize(0)";
        _out.dec();
        _out << nl << "Else";
        _out.inc();
        _out << nl << "outS__.writeSize(v__.Count)";
        _out << nl << "For Each e__ As _System.Collections.DictionaryEntry In v__";
        _out.inc();
        writeMarshalUnmarshalCode(_out, key, keyArg, true, true, false);
        writeMarshalUnmarshalCode(_out, value, valueArg, true, true, false);
        _out.dec();
        _out << nl << "Next";
        _out.dec();
        _out << nl << "End If";
        _out.dec();
        _out << nl << "End Sub";

        _out << sp << nl << "Public Shared Function read(ByVal inS__ As Ice.InputStream) As " << name;
        _out.inc();
        _out << nl << "Dim sz__ As Integer = inS__.readSize()";
        _out << nl << "Dim r__ As " << name << " = New " << name;
        _out << nl << "For i__ As Integer = 0 To sz__ - 1";
        _out.inc();
        _out << nl << "Dim k__ As " << keyS;
        writeMarshalUnmarshalCode(_out, key, "k__", false, true, false);
        if(!hasClassValue)
        {
            _out << nl << "Dim v__ As " << valueS;
        }
        writeMarshalUnmarshalCode(_out, value, "v__", false, true, false, "r__, k__");
        if(!hasClassValue)
        {
            _out << nl << "r__(k__) = v__";
        }
        _out.dec();
        _out << nl << "Next";
        _out << nl << "Return r__";
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
        _out << "Ice.ObjectDel_";
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

        _out << sp << nl << vbOp << ' ' << opName << spar << params << "ByVal context__ As Ice.Context" << epar;
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
    _out << nl << "Inherits Ice.ObjectDelM_";
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
             << "ByVal context__ As Ice.Context" << epar;
        if(ret)
        {
            _out << " As " << retS;
        }
        _out << " Implements _" << name << "Del." << opName; // TODO: should be containing class?
        _out.inc();

        _out << nl << "Dim og__ As IceInternal.Outgoing = getOutgoing(\""
             << op->name() << "\", " << sliceModeToIceMode(op->sendMode()) << ", context__)";
        _out << nl << "Try";
        _out.inc();
        if(!inParams.empty())
        {
            _out << nl << "Try";
            _out.inc();
            _out << nl << "Dim os__ As IceInternal.BasicStream = og__.ostr()";
            for(q = inParams.begin(); q != inParams.end(); ++q)
            {
                writeMarshalUnmarshalCode(_out, q->first, fixId(q->second), true, false, false);
            }
            if(op->sendsClasses())
            {
                _out << nl << "os__.writePendingObjects()";
            }
            _out.dec();
            _out << nl << "Catch ex__ As Ice.LocalException";
            _out.inc();
            _out << nl << "og__.abort(ex__)";
            _out.dec();
            _out << nl << "End Try";
        }
        _out << nl << "Dim ok__ As Boolean = og__.invoke()";
        _out << nl << "Try";
        _out.inc();
        _out << nl << "Dim is__ As IceInternal.BasicStream = og__.istr()";
        _out << nl << "If Not ok__ Then";
        _out.inc();
        //
        // The try/catch block is necessary because throwException()
        // can raise UserException.
        //
        _out << nl << "Try";
        _out.inc();
        _out << nl << "is__.throwException()";
        _out.dec();
        for(ExceptionList::const_iterator t = throws.begin(); t != throws.end(); ++t)
        {
            _out << nl << "Catch ex__ As " << fixId((*t)->scoped());
            _out.inc();
            _out << nl << "Throw";
            _out.dec();
        }
        _out << nl << "Catch ex__ As Ice.UserException";
        _out.inc();
        _out << nl << "Throw New Ice.UnknownUserException(ex__)";
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
                ContainedPtr contained = ContainedPtr::dynamicCast(ret);
                _out << nl << "Dim ret__ As " << retS << " = Nothing";
                _out << nl << "Dim ret___PP As IceInternal.ParamPatcher = New IceInternal.ParamPatcher(GetType("
                     << retS << "), \"" << (contained ? contained->scoped() : string("::Ice::Object")) << "\")";
                _out << nl << "is__.readObject(ret___PP)";
            }
            else
            {
                _out << nl << "Dim ret__ As " << retS << " = Nothing";
                writeMarshalUnmarshalCode(_out, ret, "ret__", false, false, true, "");
            }
        }
        if(op->returnsClasses())
        {
            _out << nl << "is__.readPendingObjects()";
            for(q = outParams.begin(); q != outParams.end(); ++q)
            {
                string param = fixId(q->second);
                BuiltinPtr builtin = BuiltinPtr::dynamicCast(q->first);
                if((builtin && builtin->kind() == Builtin::KindObject) || ClassDeclPtr::dynamicCast(q->first))
                {           
                    string type = typeToString(q->first);
                    _out << nl << param << " = CType(" << param << "_PP.value, " << type << ')';
                }
            }
        }
        if(ret)
        {
            BuiltinPtr builtin = BuiltinPtr::dynamicCast(ret);
            if((builtin && builtin->kind() == Builtin::KindObject) || ClassDeclPtr::dynamicCast(ret))
            {
                _out << nl << "ret__ = CType(ret___PP.value, " << retS << ')';
            }
            _out << nl << "Return ret__";
        }
        _out.dec();
        _out << nl << "Catch ex__ As Ice.LocalException";
        _out.inc();
        _out << nl << "throw New IceInternal.LocalExceptionWrapper(ex__, false)";
        _out.dec();
        _out << nl << "End Try";
        _out.dec();
        _out << nl << "Finally";
        _out.inc();
        _out << nl << "reclaimOutgoing(og__)";
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
    _out << nl << "Inherits Ice.ObjectDelD_";
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
        _out << nl << "Public " << vbOp << ' ' << opName << spar << params << "ByVal context__ As Ice.Context" << epar;
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
            _out << nl << "Dim current__ As Ice.Current = New Ice.Current";
            _out << nl << "initCurrent__(current__, \"" << op->name() << "\", " 
                 << sliceModeToIceMode(op->sendMode())
                 << ", context__)";
            _out << nl << "While True";
            _out.inc();
            _out << nl << "Dim direct__ As IceInternal.Direct = New IceInternal.Direct(current__)";
            _out << nl << "Dim servant__ As Object = direct__.servant()";
            _out << nl << "If TypeOf servant__ Is " << fixId(name) << " Then";
            _out.inc();
            _out << nl << "Try";
            _out.inc();
            _out << nl;
            if(ret)
            {
                _out << "Return ";
            }
            _out << "CType(servant__, " << fixId(containingClass->scoped() + "Operations_")
                 << ")." << opName << spar << args << "current__" << epar;
            if(!ret)
            {
                _out << nl << "Return";
            }
            _out.dec();
            _out << nl << "Catch ex__ As Ice.LocalException";
            _out.inc();
            _out << nl << "Throw New IceInternal.LocalExceptionWrapper(ex__, false)";
            _out.dec();
            _out << nl << "Finally";
            _out.inc();
            _out << nl << "direct__.destroy()";
            _out.dec();
            _out << nl << "End Try";
            _out.dec();
            _out << nl << "Else";
            _out.inc();
            _out << nl << "direct__.destroy()";
            _out << nl << "Dim opEx__ As Ice.OperationNotExistException = new Ice.OperationNotExistException()";
            _out << nl << "opEx__.id = current__.id";
            _out << nl << "opEx__.facet = current__.facet";
            _out << nl << "opEx__.operation = current__.operation";
            _out << nl << "Throw opEx__";
            _out.dec();
            _out << nl << "End If";
            _out.dec();
            _out << nl << "End While";
            if(ret)
            {
                _out << nl << "Return Nothing"; // Satisfy the VB2005 compiler.
            }
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

    _out << sp << nl << "Public MustInherit Class " << p->name() << "Disp_";
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
        _out << " Implements " << p->name() << "OperationsNC_" << '.' << name; // TODO: should be containing class?
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
            _out << "ByVal current__ As Ice.Current";
        }
        _out << epar;
        if(ret)
        {
            _out << " As " << typeToString(ret);
        }
        _out << " Implements " << p->name() << "Operations_" << '.' << name; // TODO: should be containing class?
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
        
        _out << sp << nl << "Public Sub invoke__" << spar << "ByVal prx__ As Ice.ObjectPrx"
            << paramsInvoke << "ByVal ctx__ As Ice.Context" << epar;
        _out.inc();
        _out << nl << "Try";
        _out.inc();
        _out << nl << "prepare__(prx__, \"" << p->name() << "\", " 
             << sliceModeToIceMode(p->sendMode()) << ", ctx__)";
        if(p->returnsData())
        for(q = inParams.begin(); q != inParams.end(); ++q)
        {
            string typeS = typeToString(q->first);
            writeMarshalUnmarshalCode(_out, q->first, fixId(q->second), true, false, false);
        }
        if(p->sendsClasses())
        {
            _out << nl << "os__.writePendingObjects()";
        }
        _out << nl << "os__.endWriteEncaps()";
        _out.dec();
        _out << nl << "Catch ex__ As Ice.LocalException";
        _out.inc();
        _out << nl << "finished__(ex__)";
        _out << nl << "Return";
        _out.dec();
        _out << nl << "End Try";
        _out << nl << "send__()";
        _out.dec();
        _out << nl << "End Sub";

        _out << sp << nl << "Protected Overrides Sub response__(ok__ As Boolean)";
        _out.inc();
        for(q = outParams.begin(); q != outParams.end(); ++q)
        {
            _out << nl << "Dim " << fixId(q->second) << " As " << typeToString(q->first) << " = Nothing";
        }
        if(ret)
        {
            _out << nl << "Dim ret__ As " << retS << " = Nothing";
        }
        _out << nl << "Try";
        _out.inc();
        _out << nl << "If Not ok__ Then";
        _out.inc();
        _out << nl << "Try";
        _out.inc();
        _out << nl << "is__.throwException()";
        _out.dec();
        for(ExceptionList::const_iterator r = throws.begin(); r != throws.end(); ++r)
        {
            _out << nl << "Catch ex__ As " << fixId((*r)->scoped());
            _out.inc();
            _out << nl << "Throw";
            _out.dec();
        }
        _out << nl << "Catch ex__ As Ice.UserException";
        _out.inc();
        _out << nl << "Throw New Ice.UnknownUserException(ex__)";
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
            writeMarshalUnmarshalCode(_out, ret, "ret__", false, false, true);
        }
        if(p->returnsClasses())
        {
            _out << nl << "is__.readPendingObjects()";
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
        }
        if(ret)
        {
            BuiltinPtr builtin = BuiltinPtr::dynamicCast(ret);
            if((builtin && builtin->kind() == Builtin::KindObject) || ClassDeclPtr::dynamicCast(ret))
            {
                string type = typeToString(ret);
                _out << nl << "ret__ = CType(ret___PP.value, " << retS << ')';
            }
        }
        _out.dec();
        _out << nl << "Catch ex__ As Ice.LocalException";
        _out.inc();
        _out << nl << "finished__(ex__)";
        _out << nl << "Return";
        _out.dec();
        if(!throws.empty())
        {
            _out << nl << "Catch ex__ As Ice.UserException";
            _out.inc();
            _out << nl << "ice_exception(ex__)";
            _out << nl << "Return";
            _out.dec();
        }
        _out << nl << "End Try";
        _out << nl << "ice_response" << spar << args << epar;
        _out.dec();
        _out << nl << "End Sub";
        _out.dec();
        _out << sp << nl << "End Class";
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
            _out << nl << "Dim os__ As IceInternal.BasicStream = Me.os__()";
            for(q = outParams.begin(); q != outParams.end(); ++q)
            {
                string typeS = typeToString(q->first);
                writeMarshalUnmarshalCode(_out, q->first, fixId(q->second), true, false, false);
            }
            if(ret)
            {
                string retS = typeToString(ret);
                writeMarshalUnmarshalCode(_out, ret, "ret__", true, false, false);
            }
            if(p->returnsClasses())
            {
                _out << nl << "os__.writePendingObjects()";
            }
            _out.dec();
            _out << nl << "Catch ex__ As Ice.LocalException";
            _out.inc();
            _out << nl << "ice_exception(ex__)";
            _out.dec();
            _out << nl << "End Try";
        }
        _out << nl << "response__(true)";
        _out.dec();
        _out << nl << "End Sub";

        _out << sp << nl << "Public Sub ice_exception(ByVal ex As _System.Exception)"
             << " Implements " << classNameAMD << '_' << name << ".ice_exception"; // TODO: should be containing class?
        _out.inc();
        if(throws.empty())
        {
            _out << nl << "exception__(ex)";
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
                _out << nl << "Catch ex__ As " << exS;
                _out.inc();
                _out << nl << "os__().writeUserException(ex__)";
                _out << nl << "response__(false)";
                _out.dec();
            }
            _out << nl << "Catch ex__ As _System.Exception";
            _out.inc();
            _out << nl << "exception__(ex__)";
            _out.dec();
            _out << nl << "End Try";
        }
        _out.dec();
        _out << nl << "End Sub";

        _out.dec();
        _out << sp << nl << "End Class";
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

    _out << sp << nl << "Public Class " << name << "Tie_";
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
            _out << name << "Disp_";
        }
    }
    else
    {
        _out << fixId(name);
    }
    _out << nl << "Implements Ice.TieBase";

    _out << sp << nl << "Public Sub New()";
    _out << nl << "End Sub";

    _out << sp << nl << "Public Sub New(ByVal del As " << name << opIntfName << "_)";
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
    _out << nl << "_ice_delegate = CType(del, " << name << opIntfName << "_)";
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
    _out << nl << "If Not TypeOf rhs Is " << name << "Tie_ Then";
    _out.inc();
    _out << nl << "Return False";
    _out.dec();
    _out << nl << "End If";
    _out << nl << "If _ice_delegate Is Nothing Then";
    _out.inc();
    _out << nl << "Return CType(rhs, " << name << "Tie_)._ice_delegate Is Nothing";
    _out.dec();
    _out << nl << "End If";
    _out << nl << "' Bug in VB 7.1: cast to Object should not be necessary.";
    _out << nl << "Return CType(_ice_delegate, Object).Equals(CType(rhs, " << name << "Tie_)._ice_delegate)";
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
            _out << "ByVal current__ As Ice.Current";
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
            _out << "current__";
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

    _out << sp << nl << "Private _ice_delegate As " << name << opIntfName << '_';

    return true;
}

void
Slice::Gen::TieVisitor::visitClassDefEnd(const ClassDefPtr&)
{
    _out.dec();
    _out << sp << nl << "End Class";
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
            _out << "ByVal current__ As Ice.Current";
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
            _out << "current__";
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
        _out << "Sub " << opName << "_async" << spar << pDecl << "ByVal current__ As Ice.Current" << epar;

        if(comment)
        {
            return;
        }
        else
        {
            if(forTie)
            {
                _out << " Implements " << cl->name() << "Operations_." << opName << "_async"; // TODO: should be containing class?
            }
        }

        _out.inc();
        if(ret)
        {
            _out << nl << "Dim ret__ As " << typeToString(ret) << " = " << writeValue(ret);
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
        _out << nl << "cb__.ice_response" << spar;
        if(ret)
        {
            _out << "ret__";
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
            _out << "ByVal current__ As Ice.Current";
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
                _out << " Implements " << cl->name() << "Operations_." << fixId(opName, DotNet::ICloneable, true); // TODO: should be containing class?
            }
        }
        _out.inc();
        for(i = params.begin(); i != params.end(); ++i)
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
        return st->hasMetaData("clr:class") ? string("Nothing") : "New " + fixId(st->scoped()) + "()";
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
            _out << "Inherits " << name << "Disp_";
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
    _out << nl << "Implements " << name << "Operations_";

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
