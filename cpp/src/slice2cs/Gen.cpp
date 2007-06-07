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
// Don't use "using namespace IceUtil", or VC++ 6.0 complains about
// ambigious symbols for constructs like
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
emitDeprecate(const ContainedPtr& p1, const ContainedPtr& p2, Output& out, const string& type)
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
        out << nl << "[System.Obsolete(\"" << deprecateReason << "\")]";
    }
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
                    _out << "Ice.Current current__";
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
                     << spar << params << "Ice.Current current__" << epar << ';';
            }
        }

        _out << sp << nl << "#endregion"; // Inherited Slice operations
    }
}

void
Slice::CsVisitor::writeDispatchAndMarshalling(const ClassDefPtr& p, bool stream)
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
    
    _out << sp << nl << "#region Slice type-related members";

    _out << sp << nl << "public static new string[] ids__ = ";
    _out << sb;

    {
        StringList::const_iterator q = ids.begin();
        while(q != ids.end())
        {
            _out << nl << '"' << *q << '"';
            if(++q != ids.end())
            {
                _out << ',';
            }
        }
    }
    _out << eb << ";";

    _out << sp << nl << "public override bool ice_isA(string s)";
    _out << sb;
    _out << nl << "return _System.Array.BinarySearch(ids__, s, IceUtil.StringUtil.OrdinalStringComparer) >= 0;";
    _out << eb;

    _out << sp << nl << "public override bool ice_isA(string s, Ice.Current current__)";
    _out << sb;
    _out << nl << "return _System.Array.BinarySearch(ids__, s, IceUtil.StringUtil.OrdinalStringComparer) >= 0;";
    _out << eb;

    _out << sp << nl << "public override string[] ice_ids()";
    _out << sb;
    _out << nl << "return ids__;";
    _out << eb;

    _out << sp << nl << "public override string[] ice_ids(Ice.Current current__)";
    _out << sb;
    _out << nl << "return ids__;";
    _out << eb;

    _out << sp << nl << "public override string ice_id()";
    _out << sb;
    _out << nl << "return ids__[" << scopedPos << "];";
    _out << eb;

    _out << sp << nl << "public override string ice_id(Ice.Current current__)";
    _out << sb;
    _out << nl << "return ids__[" << scopedPos << "];";
    _out << eb;

    _out << sp << nl << "public static new string ice_staticId()";
    _out << sb;
    _out << nl << "return ids__[" << scopedPos << "];";
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
        _out << sp << nl << "public static Ice.DispatchStatus " << opName << "___(" << name
             << " obj__, IceInternal.Incoming inS__, Ice.Current current__)";
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
            
            _out << nl << "checkMode__(" << sliceModeToIceMode(op->mode()) << ", current__.mode);";
            if(!inParams.empty())
            {
                _out << nl << "IceInternal.BasicStream is__ = inS__.istr();";
            }
            if(!outParams.empty() || ret || !throws.empty())
            {
                _out << nl << "IceInternal.BasicStream os__ = inS__.ostr();";
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
                _out << nl << "is__.readPendingObjects();";
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
                _out << retS << " ret__ = ";
            }
            _out << "obj__." << fixId(opName, DotNet::ICloneable, true) << spar;
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
            _out << "current__" << epar << ';';
            
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
                _out << nl << "os__.writePendingObjects();";
            }
            _out << nl << "return Ice.DispatchStatus.DispatchOK;";
            
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
                    _out << nl << "os__.writeUserException(ex);";
                    _out << nl << "return Ice.DispatchStatus.DispatchUserException;";
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
            _out << nl << "checkMode__(" << sliceModeToIceMode(op->mode()) << ", current__.mode);";
    
            if(!inParams.empty())
            {
                _out << nl << "IceInternal.BasicStream is__ = inS__.istr();";
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
                _out << nl << "is__.readPendingObjects();";
            }
            
            //
            // Call on the servant.
            //
            string classNameAMD = "AMD_" + p->name();
            _out << nl << classNameAMD << '_' << opName << " cb__ = new _" << classNameAMD << '_' << opName
                 << "(inS__);";
            _out << nl << "try";
            _out << sb;
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
            _out << "current__" << epar << ';';
            _out << eb;
            _out << nl << "catch(_System.Exception ex)";
            _out << sb;
            _out << nl << "cb__.ice_exception(ex);";
            _out << eb;
            _out << nl << "return Ice.DispatchStatus.DispatchAsync;";

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
        allOpNames.sort();
        allOpNames.unique();

        StringList::const_iterator q;

        _out << sp << nl << "private static string[] all__ =";
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

        _out << sp << nl << "public override Ice.DispatchStatus "
             << "dispatch__(IceInternal.Incoming inS__, Ice.Current current__)";
        _out << sb;
        _out << nl << "int pos = _System.Array.BinarySearch(all__, current__.operation, "
             << "IceUtil.StringUtil.OrdinalStringComparer);";
        _out << nl << "if(pos < 0)";
        _out << sb;
        _out << nl << "throw new Ice.OperationNotExistException(current__.id, current__.facet, current__.operation);";
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
                _out << nl << "return ice_id___(this, inS__, current__);";
            }
            else if(opName == "ice_ids")
            {
                _out << nl << "return ice_ids___(this, inS__, current__);";
            }
            else if(opName == "ice_isA")
            {
                _out << nl << "return ice_isA___(this, inS__, current__);";
            }
            else if(opName == "ice_ping")
            {
                _out << nl << "return ice_ping___(this, inS__, current__);";
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
                            _out << nl << "return " << opName << "___(this, inS__, current__);";
                        }
                        else
                        {
                            string base = cl->scoped();
                            if(cl->isInterface())
                            {
                                base += "Disp_";
                            }
                            _out << nl << "return " << fixId(base) << "." << opName << "___(this, inS__, current__);";
                        }
                        break;
                    }
                }
            }
            _out << eb;
        }
        _out << eb;
        _out << sp << nl << "_System.Diagnostics.Debug.Assert(false);";
        _out << nl << "throw new Ice.OperationNotExistException(current__.id, current__.facet, current__.operation);";
        _out << eb;
    }

    if(!p->isInterface() || ops.size() != 0)
    {
        _out << sp << nl << "#endregion"; // Operation dispatch
    }


    // Marshalling support
    DataMemberList allClassMembers = p->allClassDataMembers();
    DataMemberList::const_iterator d;
    DataMemberList members = p->dataMembers();
    DataMemberList classMembers = p->classDataMembers();
    ClassList bases = p->bases();
    bool hasBaseClass = !bases.empty() && !bases.front()->isInterface();

    _out << sp << nl << "#region Marshaling support";

    _out << sp << nl << "public override void write__(IceInternal.BasicStream os__)";
    _out << sb;
    _out << nl << "os__.writeTypeId(ice_staticId());";
    _out << nl << "os__.startWriteSlice();";
    for(d = members.begin(); d != members.end(); ++d)
    {
        writeMarshalUnmarshalCode(_out, (*d)->type(),
                                  fixId((*d)->name(), DotNet::ICloneable, true),
                                  true, false, false);
    }
    _out << nl << "os__.endWriteSlice();";
    _out << nl << "base.write__(os__);";
    _out << eb;

    if(allClassMembers.size() != 0)
    {
        _out << sp << nl << "public sealed ";
        if(hasBaseClass && bases.front()->declaration()->usesClasses())
        {
            _out << "new ";
        }
        _out << "class Patcher__ : IceInternal.Patcher";
        _out << sb;
        _out << sp << nl << "internal Patcher__(Ice.ObjectImpl instance";
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
        _out << nl << "try";
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
            _out << nl << "type_ = typeof(" << memberType << ");";
            _out << nl << "_instance." << memberName << " = (" << memberType << ")v;";
            _out << nl << "_typeId = \"" << (*d)->type()->typeId() << "\";";
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
        _out << nl << "catch(System.InvalidCastException)";
        _out << sb;
        _out << nl << "Ice.UnexpectedObjectException _e = new Ice.UnexpectedObjectException();";
        _out << nl << "_e.type = v.ice_id();";
        _out << nl << "_e.expectedType = _typeId;";
        _out << nl << "throw _e;";
        _out << eb;
        _out << eb;

        _out << sp << nl << "private " << name << " _instance;";
        if(allClassMembers.size() > 1)
        {
            _out << nl << "private int _member;";
        }
        _out << nl << "private string _typeId;";
        _out << eb;
    }

    _out << sp << nl << "public override void read__(IceInternal.BasicStream is__, bool rid__)";
    _out << sb;
    _out << nl << "if(rid__)";
    _out << sb;
    _out << nl << "/* string myId = */ is__.readTypeId();";
    _out << eb;
    _out << nl << "is__.startReadSlice();";
    int classMemberCount = static_cast<int>(allClassMembers.size() - classMembers.size());
    for(d = members.begin(); d != members.end(); ++d)
    {
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
    _out << nl << "is__.endReadSlice();";
    _out << nl << "base.read__(is__, true);";
    _out << eb;

    //
    // Write streaming API.
    //
    if(stream)
    {
        _out << sp << nl << "public override void write__(Ice.OutputStream outS__)";
        _out << sb;
        _out << nl << "outS__.writeTypeId(ice_staticId());";
        _out << nl << "outS__.startSlice();";
        for(d = members.begin(); d != members.end(); ++d)
        {
            writeMarshalUnmarshalCode(_out, (*d)->type(),
                                      fixId((*d)->name(), DotNet::ICloneable, true),
                                      true, true, false);
        }
        _out << nl << "outS__.endSlice();";
        _out << nl << "base.write__(outS__);";
        _out << eb;

        _out << sp << nl << "public override void read__(Ice.InputStream inS__, bool rid__)";
        _out << sb;
        _out << nl << "if(rid__)";
        _out << sb;
        _out << nl << "/* string myId = */ inS__.readTypeId();";
        _out << eb;
        _out << nl << "inS__.startSlice();";
        for(d = members.begin(); d != members.end(); ++d)
        {
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
        _out << nl << "inS__.endSlice();";
        _out << nl << "base.read__(inS__, true);";
        _out << eb;
    }
    else
    {
        //
        // Emit placeholder functions to catch errors.
        //
        string scoped = p->scoped();
        _out << sp << nl << "public override void write__(Ice.OutputStream outS__)";
        _out << sb;
        _out << nl << "Ice.MarshalException ex = new Ice.MarshalException();";
        _out << nl << "ex.reason = \"type " << scoped.substr(2) << " was not generated with stream support\";";
        _out << nl << "throw ex;";
        _out << eb;

        _out << sp << nl << "public override void read__(Ice.InputStream inS__, bool rid__)";
        _out << sb;
        _out << nl << "Ice.MarshalException ex = new Ice.MarshalException();";
        _out << nl << "ex.reason = \"type " << scoped.substr(2) << " was not generated with stream support\";";
        _out << nl << "throw ex;";
        _out << eb;
    }

    _out << sp << nl << "#endregion"; // Marshalling support

}

string
Slice::CsVisitor::getParamAttributes(const ParamDeclPtr& p)
{
    string result;
    StringList metaData = p->getMetaData();
    for(StringList::const_iterator i = metaData.begin(); i != metaData.end(); ++i)
    {
        static const string prefix = "cs:attribute:";
        if(i->find(prefix) == 0)
        {
            result += "[" + i->substr(prefix.size()) + "] ";
        }
    }
    return result;
}

vector<string>
Slice::CsVisitor::getParams(const OperationPtr& op)
{
    vector<string> params;
    ParamDeclList paramList = op->parameters();
    for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
        string param = getParamAttributes(*q);
        if((*q)->isOutParam())
        {
            param += "out ";
        }
        param += typeToString((*q)->type()) + " " + fixId((*q)->name());
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
    params.push_back(scope + (amd ? "AMD_" : "AMI_") + cl->name() + '_' + op->name() + " cb__");

    ParamDeclList paramList = op->parameters();
    for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
        if(!(*q)->isOutParam())
        {
            params.push_back(getParamAttributes(*q) + typeToString((*q)->type()) + " " + fixId((*q)->name()));
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
        params.push_back(typeToString(ret) + " ret__");
    }

    ParamDeclList paramList = op->parameters();
    for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
        if((*q)->isOutParam())
        {
            params.push_back(getParamAttributes(*q) + typeToString((*q)->type()) + ' ' + fixId((*q)->name()));
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
Slice::CsVisitor::getArgsAsyncCB(const OperationPtr& op)
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
Slice::CsVisitor::emitAttributes(const ContainedPtr& p)
{
    StringList metaData = p->getMetaData();
    for(StringList::const_iterator i = metaData.begin(); i != metaData.end(); ++i)
    {
        static const string prefix = "cs:attribute:";
        if(i->find(prefix) == 0)
        {
            _out << nl << '[' << i->substr(prefix.size()) << ']';
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
    string file = fileBase + ".cs";
    string fileImpl = fileBase + "I.cs";

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

    _out << nl << "// Generated from file `" << fileBase << ".ice'";

    _out << sp << nl << "using _System = System;";
    _out << nl << "using _Microsoft = Microsoft;";

    if(impl || implTie)
    {
        struct stat st;
        if(stat(fileImpl.c_str(), &st) == 0)
        {
            cerr << name << ": `" << fileImpl << "' already exists--will not overwrite" << endl;
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

    DispatcherVisitor dispatcherVisitor(_out, _stream);
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
Slice::Gen::generateChecksums(const UnitPtr& u)
{
    ChecksumMap map = createChecksums(u);
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
"// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.\n"
"//\n"
"// This copy of Ice is licensed to you under the terms described in the\n"
"// ICE_LICENSE file included in this distribution.\n"
"//\n"
"// **********************************************************************\n"
        ;

    _out << header;
    _out << "\n// Ice version " << ICE_STRING_VERSION;
}

Slice::Gen::UnitVisitor::UnitVisitor(IceUtil::Output& out, bool stream)
    : CsVisitor(out), _stream(stream), _globalMetaDataDone(false)
{
}

bool
Slice::Gen::UnitVisitor::visitModuleStart(const ModulePtr& p)
{
    if(!_globalMetaDataDone)
    {
        DefinitionContextPtr dc = p->definitionContext();
        StringList globalMetaData = dc->getMetaData();

        static const string attributePrefix = "cs:attribute:";

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
                _out << nl << '[' << attrib << ']';
            }
        }
        _globalMetaDataDone = true; // Do this only once per source file.
    }
    return false;
}

Slice::Gen::TypesVisitor::TypesVisitor(IceUtil::Output& out, bool stream)
    : CsVisitor(out), _stream(stream)
{
}

bool
Slice::Gen::TypesVisitor::visitModuleStart(const ModulePtr& p)
{

    string name = fixId(p->name());
    _out << sp;
    emitAttributes(p);
    _out << nl << "namespace " << name;

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
    bool hasBaseClass = !bases.empty() && !bases.front()->isInterface();

    if(!p->isLocal() && _stream)
    {
        _out << sp << nl << "public sealed class " << name << "Helper";
        _out << sb;

        _out << sp << nl << "public " << name << "Helper(Ice.InputStream inS__)";
        _out << sb;
        _out << nl << "_in = inS__;";
        _out << nl << "_pp = new IceInternal.ParamPatcher(typeof(" << scoped << "), \"" << p->scoped() << "\");";
        _out << eb;

        _out << sp << nl << "public static void write(Ice.OutputStream outS__, " << fixId(name) << " v__)";
        _out << sb;
        _out << nl << "outS__.writeObject(v__);";
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

    _out << sp;
    emitAttributes(p);
    if(p->isInterface())
    {
        _out << nl << "public interface " << fixId(name) << " : ";
        if(p->isLocal())
        {
            _out << name << "OperationsNC_";
        }
        else
        {
            _out << "Ice.Object, ";
            _out << name << "Operations_, " << name << "OperationsNC_";
        }
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
        _out << nl << "public ";
        if(p->isAbstract())
        {
            _out << "abstract ";
        }
        _out << "class " << fixId(name);

        bool baseWritten = false;

        if(!hasBaseClass)
        {
            if(!p->isLocal())
            {
                _out << " : Ice.ObjectImpl";
                baseWritten = true;
            }
        }
        else
        {
            _out << " : " << fixId(bases.front()->scoped());
            baseWritten = true;
            bases.pop_front();
        }
        if(p->isAbstract())
        {
            if(baseWritten)
            {
                _out << ", ";
            }
            else
            {
                _out << " : ";
                baseWritten = true;
            }

            if(!p->isLocal())
            {
                _out << name << "Operations_, ";
            }
            _out << name << "OperationsNC_";
        }
        for(ClassList::const_iterator q = bases.begin(); q != bases.end(); ++q)
        {
            if((*q)->isAbstract())
            {
                if(baseWritten)
                {
                    _out << ", ";
                }
                else
                {
                    _out << " : ";
                    baseWritten = true;
                }

                _out << fixId((*q)->scoped());
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
    DataMemberList classMembers = p->classDataMembers();
    DataMemberList allClassMembers = p->allClassDataMembers();
    DataMemberList dataMembers = p->dataMembers();
    DataMemberList allDataMembers = p->allDataMembers();
    ClassList bases = p->bases();
    bool hasBaseClass = !bases.empty() && !bases.front()->isInterface();
    DataMemberList::const_iterator d;

    if(!p->isInterface())
    {
        if(p->hasDataMembers() && !p->hasOperations())
        {
            _out << sp << nl << "#endregion"; // Slice data members"
        }
        else if(p->hasDataMembers())
        {
            _out << sp << nl << "#endregion"; // Slice data members and operations"
        }
        else if(p->hasOperations())
        {
            _out << sp << nl << "#endregion"; // Slice operations"
        }

        if(!allDataMembers.empty())
        {
            _out << sp << nl << "#region Constructors";

            _out << sp << nl << "public " << name << spar << epar;
            if(hasBaseClass)
            {
                _out << " : base()";
            }
            _out << sb;
            _out << eb;

            _out << sp << nl << "public " << name << spar;
            vector<string> paramDecl;
            for(d = allDataMembers.begin(); d != allDataMembers.end(); ++d)
            {
                string memberName = fixId((*d)->name());
                string memberType = typeToString((*d)->type());
                paramDecl.push_back(memberType + " " + memberName);
            }
            _out << paramDecl << epar;
            if(hasBaseClass && allDataMembers.size() != dataMembers.size())
            {
                _out << " : base" << spar;
                vector<string> baseParamNames;
                DataMemberList baseDataMembers = bases.front()->allDataMembers();
                for(d = baseDataMembers.begin(); d != baseDataMembers.end(); ++d)
                {
                    baseParamNames.push_back(fixId((*d)->name()));
                }
                _out << baseParamNames << epar;
            }
            _out << sb;
            vector<string> paramNames;
            for(d = dataMembers.begin(); d != dataMembers.end(); ++d)
            {
                paramNames.push_back(fixId((*d)->name()));
            }
            for(vector<string>::const_iterator i = paramNames.begin(); i != paramNames.end(); ++i)
            {
                _out << nl << "this." << *i << " = " << *i << ';';
            }
            _out << eb;

            _out << sp << nl << "#endregion"; // Constructors
        }

        writeInheritedOperations(p);
    }

    if(!p->isInterface() && !p->isLocal())
    {
        writeDispatchAndMarshalling(p, _stream);
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

    _out << sp;
    emitAttributes(p);
    _out << nl << "public ";
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
             << spar << params << "Ice.Current current__" << epar << ';';
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
    _out << nl << "public class " << name
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

    _out << sp << nl << "public " << name << "(" << s << "[] a__)";
    _out << sb;
    _out << nl << "InnerList.AddRange(a__);";
    _out << eb;

    _out << sp << nl << "#endregion"; // Constructors

    _out << sp << nl << "#region Array copy and conversion";

    _out << sp << nl << "public void CopyTo(" << s << "[] a__)";
    _out << sb;
    _out << nl << "InnerList.CopyTo(a__);";
    _out << eb;

    _out << sp << nl << "public void CopyTo(" << s << "[] a__, int i__)";
    _out << sb;
    _out << nl << "InnerList.CopyTo(a__, i__);";
    _out << eb;

    _out << sp << nl << "public void CopyTo(int i__, " << s << "[] a__, int ai__, int c__)";
    _out << sb;
    _out << nl << "InnerList.CopyTo(i__, a__, ai__, c__);";
    _out << eb;

    _out << sp << nl << "public " << s << "[] ToArray()";
    _out << sb;
    _out << nl << s << "[] a__ = new " << toArrayAlloc(s + "[]", "InnerList.Count") << ';';
    _out << nl << "InnerList.CopyTo(a__, 0);";
    _out << nl << "return a__;";
    _out << eb;

    _out << sp << nl << "#endregion"; // Array copy and conversion

    _out << sp << nl << "#region ArrayList members";

    _out << sp << nl << "public virtual void TrimToSize()";
    _out << sb;
    _out << nl << "InnerList.TrimToSize();";
    _out << eb;

    _out << sp << nl << "public virtual void Sort()";
    _out << sb;
    _out << nl << "InnerList.Sort();";
    _out << eb;

    _out << sp << nl << "public virtual void Sort(_System.Collections.IComparer comparer)";
    _out << sb;
    _out << nl << "InnerList.Sort(comparer);";
    _out << eb;

    _out << sp << nl << "public virtual void Sort(int index, int count, _System.Collections.IComparer comparer)";
    _out << sb;
    _out << nl << "InnerList.Sort(index, count, comparer);";
    _out << eb;

    _out << sp << nl << "public virtual void Reverse()";
    _out << sb;
    _out << nl << "InnerList.Reverse();";
    _out << eb;

    _out << sp << nl << "public virtual void Reverse(int index, int count)";
    _out << sb;
    _out << nl << "InnerList.Reverse(index, count);";
    _out << eb;

    _out << sp << nl << "public virtual int BinarySearch(" << s << " value)";
    _out << sb;
    _out << nl << "return InnerList.BinarySearch(value);";
    _out << eb;

    _out << sp << nl << "public virtual int BinarySearch(" << s << " value, _System.Collections.IComparer comparer)";
    _out << sb;
    _out << nl << "return InnerList.BinarySearch(value, comparer);";
    _out << eb;

    _out << sp << nl << "public virtual int BinarySearch(int index, int count, " << s << " value, "
         << "_System.Collections.IComparer comparer)";
    _out << sb;
    _out << nl << "return InnerList.BinarySearch(index, count, value, comparer);";
    _out << eb;

    _out << sp << nl << "public virtual void InsertRange(int index, " << name << " c)";
    _out << sb;
    _out << nl << "InnerList.InsertRange(index, c);";
    _out << eb;

    _out << sp << nl << "public virtual void InsertRange(int index, " << s << "[] c)";
    _out << sb;
    _out << nl << "InnerList.InsertRange(index, c);";
    _out << eb;

    _out << sp << nl << "public virtual void RemoveRange(int index, int count)";
    _out << sb;
    _out << nl << "InnerList.RemoveRange(index, count);";
    _out << eb;

    _out << sp << nl << "public virtual " << name << " GetRange(int index, int count)";
    _out << sb;
    _out << nl << "_System.Collections.ArrayList al = InnerList.GetRange(index, count);";
    _out << nl << name << " r = new " << name << "(al.Count);";
    _out << nl << "r.InnerList.AddRange(al);";
    _out << nl << "return r;";
    _out << eb;

    _out << sp << nl << "public virtual void SetRange(int index, " << name << " c)";
    _out << sb;
    _out << nl << "InnerList.SetRange(index, c);";
    _out << eb;

    _out << sp << nl << "public virtual void SetRange(int index, " << s << "[] c)";
    _out << sb;
    _out << nl << "InnerList.SetRange(index, c);";
    _out << eb;

    _out << sp << nl << "public virtual int LastIndexOf(" << s << " value)";
    _out << sb;
    _out << nl << "return InnerList.LastIndexOf(value);";
    _out << eb;

    _out << sp << nl << "public virtual int LastIndexOf(" << s << " value, int startIndex)";
    _out << sb;
    _out << nl << "return InnerList.LastIndexOf(value, startIndex);";
    _out << eb;

    _out << sp << nl << "public virtual int LastIndexOf(" << s << " value, int startIndex, int count)";
    _out << sb;
    _out << nl << "return InnerList.LastIndexOf(value, startIndex, count);";
    _out << eb;

    _out << sp << nl << "public static " << name << " Repeat(" << s << " value, int count)";
    _out << sb;
    _out << nl << name << " r = new " << name << "(count);";
    _out << nl << "for(int i = 0; i < count; ++i)";
    _out << sb;
    _out << nl << "r.Add(value);";
    _out << eb;
    _out << nl << "return r;";
    _out << eb;

    _out << sp << nl << "#endregion"; // ArrayList members

    _out << sp << nl << "#region AddRange members";

    _out << sp << nl << "public void AddRange(" << name << " s__)";
    _out << sb;
    _out << nl << "InnerList.AddRange(s__);";
    _out << eb;

    _out << sp << nl << "public void AddRange(" << s << "[] a__)";
    _out << sb;
    _out << nl << "InnerList.AddRange(a__);";
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
    _out << nl << name << " s = new " << name << "(Count);";
    _out << nl << "s.InnerList.AddRange(InnerList);";
    _out << nl << "return s;";
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
    _out << nl << "for(int i__ = 0; i__ < Count; ++i__)";
    _out << sb;
    if(!isValue)
    {
        _out << nl << "if(InnerList[i__] == null)";
        _out << sb;
        _out << nl << "if(((" << name << ")other)[i__] != null)";
        _out << sb;
        _out << nl << "return false;";
        _out << eb;
        _out << eb;
        _out << nl << "else";
        _out << sb;
    }
    _out << nl << "if(!((" << s << ")(InnerList[i__])).Equals(((" << name << ")other)[i__]))";
    _out << sb;
    _out << nl << "return false;";
    _out << eb;
    if(!isValue)
    {
        _out << eb;
    }
    _out << eb;
    _out << nl << "return true;";
    _out << eb;

    _out << sp << nl << "#endregion"; // Object members

    _out << sp << nl << "#region Comparison members";

    _out << sp << nl << "public static bool operator==(" << name << " lhs__, " << name << " rhs__)";
    _out << sb;
    _out << nl << "return Equals(lhs__, rhs__);";
    _out << eb;

    _out << sp << nl << "public static bool operator!=(" << name << " lhs__, " << name << " rhs__)";
    _out << sb;
    _out << nl << "return !Equals(lhs__, rhs__);";
    _out << eb;

    _out << sp << nl << "#endregion"; // Comparison members

    _out << eb;
}

bool
Slice::Gen::TypesVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    string name = fixId(p->name());
    ExceptionPtr base = p->base();

    _out << sp;

    emitDeprecate(p, 0, _out, "type");

    emitAttributes(p);
    _out << nl << "public class " << name << " : ";
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

    DataMemberList allDataMembers = p->allDataMembers();
    DataMemberList dataMembers = p->dataMembers();
    DataMemberList::const_iterator q;

    vector<string> allParamDecl;
    for(q = allDataMembers.begin(); q != allDataMembers.end(); ++q)
    {
        string memberName = fixId((*q)->name());
        string memberType = typeToString((*q)->type());
        allParamDecl.push_back(memberType + " " + memberName);
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
        paramDecl.push_back(memberType + " " + memberName);
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
        _out << sp << nl << "#endregion"; // Slice data members
    }

    _out << sp << nl << "#region Constructors";

    _out << sp << nl << "public " << name << "()";
    _out << sb;
    _out << eb;

    _out << sp << nl << "public " << name << "(_System.Exception ex__) : base(ex__)";
    _out << sb;
    _out << eb;

    if(!allDataMembers.empty())
    {
        if(!dataMembers.empty())
        {
            _out << sp << nl << "private void initDM__" << spar << paramDecl << epar;
            _out << sb;
            for(q = dataMembers.begin(); q != dataMembers.end(); ++q)
            {
                _out << nl << "this." << fixId((*q)->name()) << " = " << fixId((*q)->name()) << ';';
            }
            _out << eb;
        }

        _out << sp << nl << "public " << name << spar << allParamDecl << epar;
        if(p->base() && allDataMembers.size() != dataMembers.size())
        {
            _out << " : base" << spar << baseParamNames << epar;
        }
        _out << sb;
        if(!dataMembers.empty())
        {
            _out << nl << "initDM__" << spar << paramNames << epar << ';';
        }
        _out << eb;

        vector<string> exceptionParam;
        exceptionParam.push_back("ex__");
        vector<string> exceptionDecl;
        exceptionDecl.push_back("_System.Exception ex__");
        _out << sp << nl << "public " << name << spar << allParamDecl << exceptionDecl << epar << " : base" << spar;
        if(p->base() && allDataMembers.size() != dataMembers.size())
        {
            _out << baseParamNames;
        }
        _out << exceptionParam << epar;
        _out << sb;
        if(!dataMembers.empty())
        {
            _out << nl << "initDM__" << spar << paramNames << epar << ';';
        }
        _out << eb;
    }

    _out << sp << nl << "#endregion"; // Constructors

    _out << sp << nl << "#region Object members";

    _out << sp << nl << "public override int GetHashCode()";
    _out << sb;
    _out << nl << "int h__ = 0;";
    for(q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        string memberName = fixId((*q)->name(), DotNet::ApplicationException);
        bool isValue = isValueType((*q)->type());
        if(!isValue)
        {
            _out << nl << "if((object)" << memberName << " != null)";
            _out << sb;
        }
        _out << nl << "h__ = 5 * h__ + " << memberName << ".GetHashCode();";
        if(!isValue)
        {
            _out << eb;
        }
    }
    _out << nl << "return h__;";
    _out << eb;

    _out << sp << nl << "public override bool Equals(object other__)";
    _out << sb;
    _out << nl << "if(other__ == null)";
    _out << sb;
    _out << nl << "return false;";
    _out << eb;
    _out << nl << "if(object.ReferenceEquals(this, other__))";
    _out << sb;
    _out << nl << "return true;";
    _out << eb;
    _out << nl << "if(!(other__ is " << name << "))";
    _out << sb;
    _out << nl << "return false;";
    _out << eb;
    for(q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        string memberName = fixId((*q)->name(), DotNet::ApplicationException);
        bool isValue = isValueType((*q)->type());
        if(!isValue)
        {
            _out << nl << "if(" << memberName << " == null)";
            _out << sb;
            _out << nl << "if(((" << name << ")other__)." << memberName << " != null)";
            _out << sb;
            _out << nl << "return false;";
            _out << eb;
            _out << eb;
            _out << nl << "else";
            _out << sb;
        }
        _out << nl << "if(!" << memberName << ".Equals(((" << name << ")other__)." << memberName << "))";
        _out << sb;
        _out << nl << "return false;";
        _out << eb;
        if(!isValue)
        {
            _out << eb;
        }
    }
    _out << nl << "return true;";
    _out << eb;

    _out << sp << nl << "#endregion"; // Object members

    _out << sp << nl << "#region Comparison members";

    _out << sp << nl << "public static bool operator==(" << name << " lhs__, " << name << " rhs__)";
    _out << sb;
    _out << nl << "return Equals(lhs__, rhs__);";
    _out << eb;

    _out << sp << nl << "public static bool operator!=(" << name << " lhs__, " << name << " rhs__)";
    _out << sb;
    _out << nl << "return !Equals(lhs__, rhs__);";
    _out << eb;

    _out << sp << nl << "#endregion"; // Comparison members

    if(!p->isLocal())
    {
        _out << sp << nl << "#region Marshaling support";

        string scoped = p->scoped();
        ExceptionPtr base = p->base();


        _out << sp << nl << "public override void write__(IceInternal.BasicStream os__)";
        _out << sb;
        _out << nl << "os__.writeString(\"" << scoped << "\");";
        _out << nl << "os__.startWriteSlice();";
        for(q = dataMembers.begin(); q != dataMembers.end(); ++q)
        {
            writeMarshalUnmarshalCode(_out, (*q)->type(),
                                      fixId((*q)->name(), DotNet::ApplicationException),
                                      true, false, false);
        }
        _out << nl << "os__.endWriteSlice();";
        if(base)
        {
            _out << nl << "base.write__(os__);";
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
            _out << "class Patcher__ : IceInternal.Patcher";
            _out << sb;
            _out << sp << nl << "internal Patcher__(Ice.Exception instance";
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
            _out << nl << "try";
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
                _out << nl << "type_ = typeof(" << memberType << ");";
                _out << nl << "_instance." << memberName << " = (" << memberType << ")v;";
                _out << nl << "_typeId = \"" << (*q)->type()->typeId() << "\";";
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
            _out << nl << "catch(System.InvalidCastException)";
            _out << sb;
            _out << nl << "Ice.UnexpectedObjectException _e = new Ice.UnexpectedObjectException();";
            _out << nl << "_e.type = v.ice_id();";
            _out << nl << "_e.expectedType = _typeId;";
            _out << nl << "throw _e;";
            _out << eb;
            _out << eb;

            _out << sp << nl << "private " << name << " _instance;";
            if(allClassMembers.size() > 1)
            {
                _out << nl << "private int _member;";
            }
            _out << nl << "private string _typeId;";
            _out << eb;
        }

        _out << sp << nl << "public override void read__(IceInternal.BasicStream is__, bool rid__)";
        _out << sb;
        _out << nl << "if(rid__)";
        _out << sb;
        _out << nl << "/* string myId = */ is__.readString();";
        _out << eb;
        _out << nl << "is__.startReadSlice();";
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
        _out << nl << "is__.endReadSlice();";
        if(base)
        {
            _out << nl << "base.read__(is__, true);";
        }
        _out << eb;

        if(_stream)
        {
            _out << sp << nl << "public override void write__(Ice.OutputStream outS__)";
            _out << sb;
            _out << nl << "outS__.writeString(\"" << scoped << "\");";
            _out << nl << "outS__.startSlice();";
            for(q = dataMembers.begin(); q != dataMembers.end(); ++q)
            {
                writeMarshalUnmarshalCode(_out, (*q)->type(),
                                          fixId((*q)->name(), DotNet::ApplicationException),
                                          true, true, false);
            }
            _out << nl << "outS__.endSlice();";
            if(base)
            {
                _out << nl << "base.write__(outS__);";
            }
            _out << eb;

            _out << sp << nl << "public override void read__(Ice.InputStream inS__, bool rid__)";
            _out << sb;
            _out << nl << "if(rid__)";
            _out << sb;
            _out << nl << "/* string myId = */ inS__.readString();";
            _out << eb;
            _out << nl << "inS__.startSlice();";
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
            _out << nl << "inS__.endSlice();";
            if(base)
            {
                _out << nl << "base.read__(inS__, true);";
            }
            _out << eb;
        }
        else
        {
            //
            // Emit placeholder functions to catch errors.
            //
            _out << sp << nl << "public override void write__(Ice.OutputStream outS__)";
            _out << sb;
            _out << nl << "Ice.MarshalException ex = new Ice.MarshalException();";
            _out << nl << "ex.reason = \"exception " << scoped.substr(2) << " was not generated with stream support\";";
            _out << nl << "throw ex;";
            _out << eb;

            _out << sp << nl << "public override void read__(Ice.InputStream inS__, bool rid__)";
            _out << sb;
            _out << nl << "Ice.MarshalException ex = new Ice.MarshalException();";
            _out << nl << "ex.reason = \"exception " << scoped.substr(2) << " was not generated with stream support\";";
            _out << nl << "throw ex;";
            _out << eb;
        }

        if(!base || base && !base->usesClasses())
        {
            _out << sp << nl << "public override bool usesClasses__()";
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

        _out << sp << nl << "public static void write(Ice.OutputStream outS__, " << name << " v__)";
        _out << sb;
        _out << nl << "v__.ice_write(outS__);";
        _out << eb;

        _out << sp << nl << "public static " << name << " read(Ice.InputStream inS__)";
        _out << sb;
        _out << nl << name << " v__ = new " << name << "();";
        _out << nl << "v__.ice_read(inS__);";
        _out << nl << "return v__;";
        _out << eb;

        _out << eb;
    }

    _out << sp;

    emitDeprecate(p, 0, _out, "type");

    emitAttributes(p);
    if(isValueType(p))
    {
        _out << nl << "public struct " << name;
    }
    else
    {
        _out << nl << "public class " << name << " : _System.ICloneable";
    }
    _out << sb;

    _out << sp << nl << "#region Slice data members";

    return true;
}

void
Slice::Gen::TypesVisitor::visitStructEnd(const StructPtr& p)
{
    string name = fixId(p->name());

    DataMemberList classMembers = p->classDataMembers();
    DataMemberList dataMembers = p->dataMembers();
    DataMemberList::const_iterator q;

    _out << sp << nl << "#endregion"; // Slice data members

    bool isClass = !isValueType(p);

    _out << sp << nl << "#region Constructor";
    if(isClass)
    {
        _out << "s";
        _out << sp << nl << "public " << name << "()";
        _out << sb;
        _out << eb;
    }

    _out << sp << nl << "public " << name << spar;
    vector<string> paramDecl;
    vector<string> paramNames;
    for(q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        string memberName = fixId((*q)->name());
        string memberType = typeToString((*q)->type());
        paramDecl.push_back(memberType + " " + memberName);
        paramNames.push_back(memberName);
    }
    _out << paramDecl << epar;
    _out << sb;
    for(vector<string>::const_iterator i = paramNames.begin(); i != paramNames.end(); ++i)
    {
        _out << nl << "this." << *i << " = " << *i << ';';
    }
    _out << eb;

    _out << sp << nl << "#endregion"; // Constructor(s)

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
    _out << nl << "int h__ = 0;";
    for(q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        string memberName = fixId((*q)->name(), isClass ? DotNet::ICloneable : 0);
        bool isValue = isValueType((*q)->type());
        if(!isValue)
        {
            _out << nl << "if(" << memberName << " != null)";
            _out << sb;
        }
        _out << nl << "h__ = 5 * h__ + " << memberName << ".GetHashCode();";
        if(!isValue)
        {
            _out << eb;
        }
    }
    _out << nl << "return h__;";
    _out << eb;

    _out << sp << nl << "public override bool Equals(object other__)";
    _out << sb;
    if(isClass)
    {
        _out << nl << "if(object.ReferenceEquals(this, other__))";
        _out << sb;
        _out << nl << "return true;";
        _out << eb;
    }
    if(isClass)
    {
        _out << nl << "if(other__ == null)";
        _out << sb;
        _out << nl << "return false;";
        _out << eb;
        _out << nl << "if(GetType() != other__.GetType())";
    }
    else
    {
        _out << nl << "if(!(other__ is " << name << "))";
    }
    _out << sb;
    _out << nl << "return false;";
    _out << eb;
    if(!dataMembers.empty())
    {
        _out << nl << name << " o__ = (" << name << ")other__;";
    }
    for(q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        string memberName = fixId((*q)->name(), isClass ? DotNet::ICloneable : 0);
        if(!isValueType((*q)->type()))
        {
            _out << nl << "if(" << memberName << " == null)";
            _out << sb;
            _out << nl << "if(o__." << memberName << " != null)";
            _out << sb;
            _out << nl << "return false;";
            _out << eb;
            _out << eb;
            _out << nl << "else";
            _out << sb;
            _out << nl << "if(!" << memberName << ".Equals(o__." << memberName << "))";
            _out << sb;
            _out << nl << "return false;";
            _out << eb;
            _out << eb;
        }
        else
        {
            _out << nl << "if(!" << memberName << ".Equals(o__." << memberName << "))";
            _out << sb;
            _out << nl << "return false;";
            _out << eb;
        }
    }
    _out << nl << "return true;";
    _out << eb;

    _out << sp << nl << "#endregion"; // Object members

    _out << sp << nl << "#region Comparison members";

    _out << sp << nl << "public static bool operator==(" << name << " lhs__, " << name << " rhs__)";
    _out << sb;
    _out << nl << "return Equals(lhs__, rhs__);";
    _out << eb;

    _out << sp << nl << "public static bool operator!=(" << name << " lhs__, " << name << " rhs__)";
    _out << sb;
    _out << nl << "return !Equals(lhs__, rhs__);";
    _out << eb;

    _out << sp << nl << "#endregion"; // Comparison members

    if(!p->isLocal())
    {
        _out << sp << nl << "#region Marshalling support";

        _out << sp << nl << "public void write__(IceInternal.BasicStream os__)";
        _out << sb;
        for(q = dataMembers.begin(); q != dataMembers.end(); ++q)
        {
            writeMarshalUnmarshalCode(_out, (*q)->type(),
                                      fixId((*q)->name(), isClass ? DotNet::ICloneable : 0),
                                      true, false, false);
        }
        _out << eb;


        if(isClass && classMembers.size() != 0)
        {
            _out << sp << nl << "public sealed class Patcher__ : IceInternal.Patcher";
            _out << sb;
            _out << sp << nl << "internal Patcher__(" << name << " instance";
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
            _out << nl << "try";
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
                _out << nl << "type_ = typeof(" << memberType << ");";
                _out << nl << "_instance." << memberName << " = (" << memberType << ")v;";
                _out << nl << "_typeId = \"" << (*q)->type()->typeId() << "\";";
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
            _out << nl << "catch(System.InvalidCastException)";
            _out << sb;
            _out << nl << "Ice.UnexpectedObjectException _e = new Ice.UnexpectedObjectException();";
            _out << nl << "_e.type = v.ice_id();";
            _out << nl << "_e.expectedType = _typeId;";
            _out << nl << "throw _e;";
            _out << eb;
            _out << eb;

            _out << sp << nl << "private " << name;
            _out << " _instance;";
            if(classMembers.size() > 1)
            {
                _out << nl << "private int _member;";
            }
            _out << nl << "private string _typeId;";
            _out << eb;
        }

        _out << sp << nl << "public void read__(IceInternal.BasicStream is__)";
        _out << sb;
        int classMemberCount = 0;
        for(q = dataMembers.begin(); q != dataMembers.end(); ++q)
        {
            ostringstream patchParams;
            patchParams << "this";
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
            _out << sp << nl << "public void ice_write(Ice.OutputStream outS__)";
            _out << sb;
            for(q = dataMembers.begin(); q != dataMembers.end(); ++q)
            {
                writeMarshalUnmarshalCode(_out, (*q)->type(),
                                          fixId((*q)->name(), isClass ? DotNet::ICloneable : 0),
                                          true, true, false);
            }
            _out << eb;

            _out << sp << nl << "public void ice_read(Ice.InputStream inS__)";
            _out << sb;
            classMemberCount = 0;
            for(q = dataMembers.begin(); q != dataMembers.end(); ++q)
            {
                ostringstream patchParams;
                patchParams << "this";
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

    _out << sp;

    emitDeprecate(p, 0, _out, "type");

    emitAttributes(p);
    _out << nl << "public class " << name
         << " : _System.Collections.DictionaryBase, _System.ICloneable";
    _out << sb;

    _out << sp << nl << "#region " << name << " members";

    _out << sp << nl << "public void AddRange(" << name << " d__)";
    _out << sb;
    _out << nl << "foreach(_System.Collections.DictionaryEntry e in d__)";
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
    _out << nl << name << " d = new " << name << "();";
    _out << nl << "foreach(_System.Collections.DictionaryEntry e in InnerHashtable)";
    _out << sb;
    _out << nl << "d.InnerHashtable.Add(e.Key, e.Value);";
    _out << eb;
    _out << nl << "return d;";
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
    _out << nl << ks << "[] klhs__ = new " << ks << "[Count];";
    _out << nl << "Keys.CopyTo(klhs__, 0);";
    _out << nl << "_System.Array.Sort(klhs__);";
    _out << nl << ks << "[] krhs__ = new " << ks << "[((" << name << ")other).Count];";
    _out << nl << "((" << name << ")other).Keys.CopyTo(krhs__, 0);";
    _out << nl << "_System.Array.Sort(krhs__);";
    _out << nl << "for(int i = 0; i < Count; ++i)";
    _out << sb;
    _out << nl << "if(!klhs__[i].Equals(krhs__[i]))";
    _out << sb;
    _out << nl << "return false;";
    _out << eb;
    _out << eb;
    SequencePtr seq = SequencePtr::dynamicCast(p->valueType());
    bool valueIsArray = seq && !seq->hasMetaData("clr:collection");
    if(valueIsArray)
    {
        _out << nl << vs << "[] vlhs__ = new " << toArrayAlloc(vs + "[]", "Count") << ';';
    }
    else
    {
        _out << nl << vs << "[] vlhs__ = new " << vs << "[Count];";
    }
    _out << nl << "Values.CopyTo(vlhs__, 0);";
    _out << nl << "_System.Array.Sort(vlhs__);";
    string vrhsCount = "((" + name + ")other).Count";
    if(valueIsArray)
    {
        _out << nl << vs << "[] vrhs__ = new " << toArrayAlloc(vs + "[]", vrhsCount) << ';';
    }
    else
    {
        _out << nl << vs << "[] vrhs__ = new " << vs << '[' << vrhsCount << "];";
    }
    _out << nl << "((" << name << ")other).Values.CopyTo(vrhs__, 0);";
    _out << nl << "_System.Array.Sort(vrhs__);";
    _out << nl << "for(int i = 0; i < Count; ++i)";
    _out << sb;
    if(!valueIsValue)
    {
        _out << nl << "if(vlhs__[i] == null)";
        _out << sb;
        _out << nl << "if(vrhs__[i] != null)";
        _out << sb;
        _out << nl << "return false;";
        _out << eb;
        _out << eb;
        _out << nl << "else if(!vlhs__[i].Equals(vrhs__[i]))";
        _out << sb;
        _out << nl << "return false;";
        _out << eb;
    }
    else
    {
        _out << nl << "if(!vlhs__[i].Equals(vrhs__[i]))";
        _out << sb;
        _out << nl << "return false;";
        _out << eb;
    }
    _out << eb;
    _out << nl << "return true;";
    _out << eb;

    _out << sp << nl << "#endregion"; // Object members

    _out << sp << nl << "#region Comparison members";

    _out << sp << nl << "public static bool operator==(" << name << " lhs__, " << name << " rhs__)";
    _out << sb;
    _out << nl << "return Equals(lhs__, rhs__);";
    _out << eb;

    _out << sp << nl << "public static bool operator!=(" << name << " lhs__, " << name << " rhs__)";
    _out << sb;
    _out << nl << "return !Equals(lhs__, rhs__);";
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
    _out << sp;

    emitDeprecate(p, 0, _out, "type");

    emitAttributes(p);
    _out << nl << "public enum " << name;
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

        _out << sp << nl << "public static void write(Ice.OutputStream outS__, " << scoped << " v__)";
        _out << sb;
        writeMarshalUnmarshalCode(_out, p, "v__", true, true, false);
        _out << eb;

        _out << sp << nl << "public static " << scoped << " read(Ice.InputStream inS__)";
        _out << sb;
        _out << nl << scoped << " v__;";
        writeMarshalUnmarshalCode(_out, p, "v__", false, true, false);
        _out << nl << "return v__;";
        _out << eb;

        _out << eb;
    }
}

void
Slice::Gen::TypesVisitor::visitConst(const ConstPtr& p)
{
    string name = fixId(p->name());
    _out << sp;
    emitAttributes(p);
    _out << nl << "public abstract class " << name;
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
                                               "_{}[]#()<>%:;.?*+-/^&|~!=,\\\"' ";
        static const set<char> charSet(basicSourceChars.begin(), basicSourceChars.end());

        _out << "\"";                                    // Opening "

        const string val = p->value();
        for(string::const_iterator c = val.begin(); c != val.end(); ++c)
        {
            if(charSet.find(*c) == charSet.end())
            {
                unsigned char uc = *c;                   // char may be signed, so make it positive
                ostringstream s;
                s << "\\u";                      // Print as unicode if not in basic source character set
                s << hex;
                s.width(4);
                s.fill('0');
                s << static_cast<unsigned>(uc);
                _out << s.str();
            }
            else
            {
                _out << *c;                              // Print normally if in basic source character set
            }
        }

        _out << "\"";                                    // Closing "
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
    _out << nl << (propertyMapping ? "private" : "public") << ' ' << type << ' ' << dataMemberName << ';';

    if(!propertyMapping)
    {
        return;
    }

    _out << nl << "public";
    if(!isValue)
    {
        _out << " virtual";
    }
    _out << ' ' << type << ' ' << propertyName;
    _out << sb;
    _out << nl << "get";
    _out << sb;
    _out << nl << "return " << dataMemberName << ';';
    _out << eb;
    _out << nl << "set";
    _out << sb;
    _out << nl << dataMemberName << " = value;";
    _out << eb;
    _out << eb;
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
        _out << nl << "[System.Obsolete(\"" << deprecateReason << "\")]";
    }
    _out << nl << typeToString(p->returnType()) << " " << name << spar << params << epar << ';';

    if(!deprecateReason.empty())
    {
        _out << nl << "[System.Obsolete(\"" << deprecateReason << "\")]";
    }
    _out << nl << typeToString(p->returnType()) << " " << name
         << spar << params << "Ice.Context context__" << epar << ';'; 

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
            _out << nl << "[System.Obsolete(\"" << deprecateReason << "\")]";
        }
        _out << nl << "void " << p->name() << "_async" << spar << paramsAMI << epar << ';';
        if(!deprecateReason.empty())
        {
            _out << nl << "[System.Obsolete(\"" << deprecateReason << "\")]";
        }
        _out << nl << "void " << p->name() << "_async" << spar << paramsAMI << "Ice.Context ctx__" << epar << ';';
    }
}

Slice::Gen::OpsVisitor::OpsVisitor(IceUtil::Output& out)
    : CsVisitor(out)
{
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

    _out << sp << nl << "public interface " << name << opIntfName << '_';
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
                s += "Operations";
                if(noCurrent)
                {
                    s += "NC";
                }
                _out << fixId(s) << '_';
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

        _out << sp;

        emitDeprecate(op, p, _out, "operation");

        emitAttributes(op);
        string retS = typeToString(ret);
        _out << nl << retS << ' ' << opname << spar << params;
        if(!noCurrent && !p->isLocal())
        { 
            _out << "Ice.Current current__";
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
        _out << opName << spar << args << "null" << "false" << epar << ';';
        _out << eb;

        _out << sp << nl << "public " << retS << " " << opName << spar << params 
             << "Ice.Context context__" << epar;
        _out << sb;
        _out << nl;
        if(ret)
        {
            _out << "return ";
        }
        _out << opName << spar << args << "context__" << "true" << epar << ';';
        _out << eb;

        _out << sp << nl << "private " << retS << " " << opName << spar << params 
             << "Ice.Context context__" << "bool explicitContext__" << epar;
        _out << sb;

        _out << nl << "if(explicitContext__ && context__ == null)";
        _out << sb;
        _out << nl << "context__ = emptyContext_;";
        _out << eb;
        _out << nl << "int cnt__ = 0;";
        _out << nl << "while(true)";
        _out << sb;
        _out << nl << "Ice.ObjectDel_ delBase__ = null;";
        _out << nl << "try";
        _out << sb;
        if(op->returnsData())
        {
            _out << nl << "checkTwowayOnly__(\"" << op->name() << "\");";
        }
        _out << nl << "delBase__ = getDelegate__();";
        _out << nl << name << "Del_ del__ = (" << name << "Del_)delBase__;";
        _out << nl;
        if(ret)
        {
            _out << "return ";
        }
        _out << "del__." << opName << spar << args << "context__" << epar << ';';
        if(!ret)
        {
            _out << nl << "return;";
        }
        _out << eb;
        _out << nl << "catch(IceInternal.LocalExceptionWrapper ex__)";
        _out << sb;
        if(op->mode() == Operation::Idempotent || op->mode() == Operation::Nonmutating)
        {
            _out << nl << "cnt__ = handleExceptionWrapperRelaxed__(delBase__, ex__, cnt__);";
        }
        else
        {
            _out << nl << "handleExceptionWrapper__(delBase__, ex__);";
        }
        _out << eb;
        _out << nl << "catch(Ice.LocalException ex__)";
        _out << sb;
        _out << nl << "cnt__ = handleException__(delBase__, ex__, cnt__);";
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
            _out << nl << "cb__.invoke__" << spar << "this" << argsAMI << "null" << epar << ';';
            _out << eb;

            _out << nl << "public void " << opName << "_async" << spar << paramsAMI << "Ice.Context ctx__" << epar;
            _out << sb;
            _out << nl << "if(ctx__ == null)";
            _out << sb;
            _out << nl << "ctx__ = emptyContext_;";
            _out << eb;
            _out << nl << "cb__.invoke__" << spar << "this" << argsAMI << "ctx__" << epar << ';';
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
    _out << nl << "h.copyFrom__(b);";
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
    _out << nl << "h.copyFrom__(b);";
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
    _out << nl << "Ice.ObjectPrx bb = b.ice_facet(f);";
    _out << nl << "try";
    _out << sb;
    _out << nl << "if(bb.ice_isA(\"" << p->scoped() << "\"))";
    _out << sb;
    _out << nl << name << "PrxHelper h = new " << name << "PrxHelper();";
    _out << nl << "h.copyFrom__(bb);";
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
    _out << nl << "Ice.ObjectPrx bb = b.ice_facet(f);";
    _out << nl << "try";
    _out << sb;
    _out << nl << "if(bb.ice_isA(\"" << p->scoped() << "\", ctx))";
    _out << sb;
    _out << nl << name << "PrxHelper h = new " << name << "PrxHelper();";
    _out << nl << "h.copyFrom__(bb);";
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
    _out << nl << "h.copyFrom__(b);";
    _out << nl << "return h;";
    _out << eb;

    _out << sp << nl << "public static " << name << "Prx uncheckedCast(Ice.ObjectPrx b, string f)";
    _out << sb;
    _out << nl << "if(b == null)";
    _out << sb;
    _out << nl << "return null;";
    _out << eb;
    _out << nl << "Ice.ObjectPrx bb = b.ice_facet(f);";
    _out << nl << name << "PrxHelper h = new " << name << "PrxHelper();";
    _out << nl << "h.copyFrom__(bb);";
    _out << nl << "return h;";
    _out << eb;

    _out << sp << nl << "#endregion"; // Checked and unchecked cast operations

    _out << sp << nl << "#region Marshaling support";

    _out << sp << nl << "protected override Ice.ObjectDelM_ createDelegateM__()";
    _out << sb;
    _out << nl << "return new " << name << "DelM_();";
    _out << eb;

    _out << sp << nl << "protected override Ice.ObjectDelD_ createDelegateD__()";
    _out << sb;
    _out << nl << "return new " << name << "DelD_();";
    _out << eb;

    _out << sp << nl << "public static void write__(IceInternal.BasicStream os__, " << name << "Prx v__)";
    _out << sb;
    _out << nl << "os__.writeProxy(v__);";
    _out << eb;

    _out << sp << nl << "public static " << name << "Prx read__(IceInternal.BasicStream is__)";
    _out << sb;
    _out << nl << "Ice.ObjectPrx proxy = is__.readProxy();";
    _out << nl << "if(proxy != null)";
    _out << sb;
    _out << nl << name << "PrxHelper result = new " << name << "PrxHelper();";
    _out << nl << "result.copyFrom__(proxy);";
    _out << nl << "return result;";
    _out << eb;
    _out << nl << "return null;";
    _out << eb;

    if(_stream)
    {
        _out << sp << nl << "public static void write(Ice.OutputStream outS__, " << name << "Prx v__)";
        _out << sb;
        _out << nl << "outS__.writeProxy(v__);";
        _out << eb;

        _out << sp << nl << "public static " << name << "Prx read(Ice.InputStream inS__)";
        _out << sb;
        _out << nl << "Ice.ObjectPrx proxy = inS__.readProxy();";
        _out << nl << "if(proxy != null)";
        _out << sb;
        _out << nl << name << "PrxHelper result = new " << name << "PrxHelper();";
        _out << nl << "result.copyFrom__(proxy);";
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

    _out << nl << "public static void write(IceInternal.BasicStream os__, " << typeS << " v__)";
    _out << sb;
    writeSequenceMarshalUnmarshalCode(_out, p, "v__", true, false);
    _out << eb;

    _out << sp << nl << "public static " << typeS << " read(IceInternal.BasicStream is__)";
    _out << sb;
    _out << nl << typeS << " v__;";
    writeSequenceMarshalUnmarshalCode(_out, p, "v__", false, false);
    _out << nl << "return v__;";
    _out << eb;

    if(_stream)
    {
        _out << sp << nl << "public static void write(Ice.OutputStream outS__, " << typeS << " v__)";
        _out << sb;
        writeSequenceMarshalUnmarshalCode(_out, p, "v__", true, true);
        _out << eb;

        _out << sp << nl << "public static " << typeS << " read(Ice.InputStream inS__)";
        _out << sb;
        _out << nl << typeS << " v__;";
        writeSequenceMarshalUnmarshalCode(_out, p, "v__", false, true);
        _out << nl << "return v__;";
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

    _out << nl << "public static void write(IceInternal.BasicStream os__, " << name << " v__)";
    _out << sb;
    _out << nl << "if(v__ == null)";
    _out << sb;
    _out << nl << "os__.writeSize(0);";
    _out << eb;
    _out << nl << "else";
    _out << sb;
    _out << nl << "os__.writeSize(v__.Count);";
    _out << nl << "foreach(_System.Collections.DictionaryEntry e__ in v__)";
    _out << sb;
    string keyArg = "((" + keyS + ")e__.Key)";
    writeMarshalUnmarshalCode(_out, key, keyArg, true, false, false);
    string valueArg = "((" + valueS + ")e__.Value)";
    writeMarshalUnmarshalCode(_out, value, valueArg, true, false, false);
    _out << eb;
    _out << eb;
    _out << eb;

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(value);
    bool hasClassValue = (builtin && builtin->kind() == Builtin::KindObject) || ClassDeclPtr::dynamicCast(value);
    if(hasClassValue)
    {
        _out << sp << nl << "public sealed class Patcher__ : IceInternal.Patcher";
        _out << sb;
        _out << sp << nl << "internal Patcher__(" << name << " m, " << keyS << " key)";
        _out << sb;
        _out << nl << "_m = m;";
        _out << nl << "_key = key;";
        _out << eb;

        _out << sp << nl << "public override void" << nl << "patch(Ice.Object v)";
        _out << sb;
        _out << nl << "type_ = typeof(" << typeToString(p->valueType()) << ");";
        _out << nl << "try";
        _out << sb;
        _out << nl << "_m[_key] = (" << valueS << ")v;";
        _out << eb;
        _out << nl << "catch(System.InvalidCastException)";
        _out << sb;
        _out << nl << "Ice.UnexpectedObjectException _e = new Ice.UnexpectedObjectException();";
        _out << nl << "_e.type = v.ice_id();";
        _out << nl << "_e.expectedType = \"" << value->typeId() << "\";";
        _out << nl << "throw _e;";
        _out << eb;
        _out << eb;

        _out << sp << nl << "private " << name << " _m;";
        _out << nl << "private " << keyS << " _key;";
        _out << eb;
    }

    _out << sp << nl << "public static " << name << " read(IceInternal.BasicStream is__)";
    _out << sb;
    _out << nl << "int sz__ = is__.readSize();";
    _out << nl << name << " r__ = new " << name << "();";
    _out << nl << "for(int i__ = 0; i__ < sz__; ++i__)";
    _out << sb;
    _out << nl << keyS << " k__;";
    writeMarshalUnmarshalCode(_out, key, "k__", false, false, false);
    if(!hasClassValue)
    {
        _out << nl << valueS << " v__;";
    }
    writeMarshalUnmarshalCode(_out, value, "v__", false, false, false, "r__, k__");
    if(!hasClassValue)
    {
        _out << nl << "r__[k__] = v__;";
    }
    _out << eb;
    _out << nl << "return r__;";
    _out << eb;

    if(_stream)
    {
        _out << sp << nl << "public static void write(Ice.OutputStream outS__, " << name << " v__)";
        _out << sb;
        _out << nl << "if(v__ == null)";
        _out << sb;
        _out << nl << "outS__.writeSize(0);";
        _out << eb;
        _out << nl << "else";
        _out << sb;
        _out << nl << "outS__.writeSize(v__.Count);";
        _out << nl << "foreach(_System.Collections.DictionaryEntry e__ in v__)";
        _out << sb;
        writeMarshalUnmarshalCode(_out, key, keyArg, true, true, false);
        writeMarshalUnmarshalCode(_out, value, valueArg, true, true, false);
        _out << eb;
        _out << eb;
        _out << eb;

        _out << sp << nl << "public static " << name << " read(Ice.InputStream inS__)";
        _out << sb;
        _out << nl << "int sz__ = inS__.readSize();";
        _out << nl << name << " r__ = new " << name << "();";
        _out << nl << "for(int i__ = 0; i__ < sz__; ++i__)";
        _out << sb;
        _out << nl << keyS << " k__;";
        writeMarshalUnmarshalCode(_out, key, "k__", false, true, false);
        if(!hasClassValue)
        {
            _out << nl << valueS << " v__;";
        }
        writeMarshalUnmarshalCode(_out, value, "v__", false, true, false, "r__, k__");
        if(!hasClassValue)
        {
            _out << nl << "r__[k__] = v__;";
        }
        _out << eb;
        _out << nl << "return r__;";
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

    _out << sp << nl << "public interface " << name << "Del_ : ";
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
            s += "Del_";
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

        _out << sp << nl << retS << ' ' << opName << spar << params << "Ice.Context context__" << epar << ';';
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

    _out << sp << nl << "public sealed class " << name << "DelM_ : Ice.ObjectDelM_, " << name << "Del_";
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

        _out << sp << nl << "public " << retS << ' ' << opName << spar << params << "Ice.Context context__" << epar;
        _out << sb;

        _out << nl << "IceInternal.Outgoing og__ = getOutgoing(\"" << op->name() << "\", " 
             << sliceModeToIceMode(op->sendMode())
             << ", context__);";
        _out << nl << "try";
        _out << sb;
        if(!inParams.empty())
        {
            _out << nl << "try";
            _out << sb;
            _out << nl << "IceInternal.BasicStream os__ = og__.ostr();";
            for(q = inParams.begin(); q != inParams.end(); ++q)
            {
                writeMarshalUnmarshalCode(_out, q->first, fixId(q->second), true, false, false);
            }
            if(op->sendsClasses())
            {
                _out << nl << "os__.writePendingObjects();";
            }
            _out << eb;
            _out << nl << "catch(Ice.LocalException ex__)";
            _out << sb;
            _out << nl << "og__.abort(ex__);";
            _out << eb;
        }
        _out << nl << "bool ok__ = og__.invoke();";
        _out << nl << "try";
        _out << sb;
        _out << nl << "IceInternal.BasicStream is__ = og__.istr();";
        _out << nl << "if(!ok__)";
        _out << sb;
        //
        // The try/catch block is necessary because throwException()
        // can raise UserException.
        //
        _out << nl << "try";
        _out << sb;
        _out << nl << "is__.throwException();";
        _out << eb;
        for(ExceptionList::const_iterator t = throws.begin(); t != throws.end(); ++t)
        {
            _out << nl << "catch(" << fixId((*t)->scoped()) << ')';
            _out << sb;
            _out << nl << "throw;";
            _out << eb;
        }
        _out << nl << "catch(Ice.UserException ex)";
        _out << sb;
        _out << nl << "throw new Ice.UnknownUserException(ex);";
        _out << eb;
        _out << eb;
        for(q = outParams.begin(); q != outParams.end(); ++q)
        {
            writeMarshalUnmarshalCode(_out, q->first, fixId(q->second), false, false, true, "");
        }
        if(ret)
        {
            BuiltinPtr builtin = BuiltinPtr::dynamicCast(ret);
            if((builtin && builtin->kind() == Builtin::KindObject) || ClassDeclPtr::dynamicCast(ret))
            {
                _out << nl << retS << " ret__;";
                ContainedPtr contained = ContainedPtr::dynamicCast(ret);
                _out << nl << "IceInternal.ParamPatcher ret___PP = new IceInternal.ParamPatcher(typeof("
                    << retS << "), \"" << (contained? contained->scoped() : string("::Ice::Object")) << "\");";
                _out << nl << "is__.readObject(ret___PP);";
            }
            else
            {
                _out << nl << retS << " ret__;";
                writeMarshalUnmarshalCode(_out, ret, "ret__", false, false, true, "");
            }
        }
        if(op->returnsClasses())
        {
            _out << nl << "is__.readPendingObjects();";
            for(q = outParams.begin(); q != outParams.end(); ++q)
            {
                string param = fixId(q->second);
                BuiltinPtr builtin = BuiltinPtr::dynamicCast(q->first);
                if((builtin && builtin->kind() == Builtin::KindObject) || ClassDeclPtr::dynamicCast(q->first))
                {           
                    string type = typeToString(q->first);
                    _out << nl << "try";
                    _out << sb;
                    _out << nl << param << " = (" << type << ")" << param << "_PP.value;";
                    _out << eb;
                    _out << nl << "catch(System.InvalidCastException)";
                    _out << sb;
                    _out << nl << "Ice.UnexpectedObjectException ex = new Ice.UnexpectedObjectException();";
                    _out << nl << "ex.type = " << param << "_PP.value.ice_id();";
                    _out << nl << "ex.expectedType = \"" << q->first->typeId() << "\";";
                    _out << nl << "throw ex;";
                    _out << eb;
                }
            }
        }
        if(ret)
        {
            BuiltinPtr builtin = BuiltinPtr::dynamicCast(ret);
            if((builtin && builtin->kind() == Builtin::KindObject) || ClassDeclPtr::dynamicCast(ret))
            {
                _out << nl << "try";
                _out << sb;
                _out << nl << "ret__ = (" << retS << ")ret___PP.value;";
                _out << eb;
                _out << nl << "catch(System.InvalidCastException)";
                _out << sb;
                _out << nl << "Ice.UnexpectedObjectException ex = new Ice.UnexpectedObjectException();";
                _out << nl << "ex.type = ret___PP.value.ice_id();";
                _out << nl << "ex.expectedType = \"" << ret->typeId() << "\";";
                _out << nl << "throw ex;";
                _out << eb;
            }
            _out << nl << "return ret__;";
        }
        _out << eb;
        _out << nl << "catch(Ice.LocalException ex__)";
        _out << sb;
        _out << nl << "throw new IceInternal.LocalExceptionWrapper(ex__, false);";
        _out << eb;
        _out << eb;
        _out << nl << "finally";
        _out << sb;
        _out << nl << "reclaimOutgoing(og__);";
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

    _out << sp << nl << "public sealed class " << name << "DelD_ : Ice.ObjectDelD_, " << name << "Del_";
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
        _out << nl << "public " << retS << ' ' << opName << spar << params << "Ice.Context context__" << epar;
        _out << sb;
        if(containingClass->hasMetaData("amd") || op->hasMetaData("amd"))
        {
            _out << nl << "throw new Ice.CollocationOptimizationException();";
        }
        else
        {
            _out << nl << "Ice.Current current__ = new Ice.Current();";
            _out << nl << "initCurrent__(ref current__, \"" << op->name() << "\", " 
                 << sliceModeToIceMode(op->sendMode())
                 << ", context__);";
            _out << nl << "while(true)";
            _out << sb;
            _out << nl << "IceInternal.Direct direct__ = new IceInternal.Direct(current__);";
            _out << nl << "object servant__ = direct__.servant();";
            _out << nl << "if(servant__ is " << fixId(name) << ")";
            _out << sb;
            _out << nl << "try";
            _out << sb;
            _out << nl;
            if(ret)
            {
                _out << "return ";
            }
            _out << "((" << fixId(containingClass->scoped()) << ")servant__)."
                 << opName << spar << args << "current__" << epar << ';';
            if(!ret)
            {
                _out << nl << "return;";
            }
            _out << eb;
            _out << nl << "catch(Ice.LocalException ex__)";
            _out << sb;
            _out << nl << "throw new IceInternal.LocalExceptionWrapper(ex__, false);";
            _out << eb;
            _out << nl << "finally";
            _out << sb;
            _out << nl << "direct__.destroy();";
            _out << eb;
            _out << eb;
            _out << nl << "else";
            _out << sb;
            _out << nl << "direct__.destroy();";
            _out << nl << "Ice.OperationNotExistException opEx__ = new Ice.OperationNotExistException();";
            _out << nl << "opEx__.id = current__.id;";
            _out << nl << "opEx__.facet = current__.facet;";
            _out << nl << "opEx__.operation = current__.operation;";
            _out << nl << "throw opEx__;";
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

Slice::Gen::DispatcherVisitor::DispatcherVisitor(::IceUtil::Output &out, bool stream)
    : CsVisitor(out), _stream(stream)
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

    _out << sp << nl << "public abstract class " << name << "Disp_ : Ice.ObjectImpl, " << fixId(name);
    _out << sb;

    OperationList ops = p->operations();
    if(!ops.empty())
    {
        _out << sp << nl << "#region Slice operations";
    }

    for(OperationList::const_iterator op = ops.begin(); op != ops.end(); ++op)
    {
        bool amd = p->hasMetaData("amd") || (*op)->hasMetaData("amd");

        string opname = (*op)->name();
        vector<string> params;
        vector<string> args;
        TypePtr ret;

        if(amd)
        {
            opname = opname + "_async";
            params = getParamsAsync(*op, true);
            args = getArgsAsync(*op);
        }
        else
        {
            opname = fixId(opname, DotNet::ICloneable, true);
            params = getParams(*op);
            ret = (*op)->returnType();
            args = getArgs(*op);
        }

        _out << sp << nl << "public " << typeToString(ret) << " " << opname << spar << params << epar;
        _out << sb << nl;
        if(ret)
        {
            _out << "return ";
        }
        _out << opname << spar << args << "Ice.ObjectImpl.defaultCurrent" << epar << ';';
        _out << eb;

        _out << sp << nl << "public abstract " << typeToString(ret) << " " << opname << spar << params;
        if(!p->isLocal())
        {
            _out << "Ice.Current current__";
        }
        _out << epar << ';';
    }

    if(!ops.empty())
    {
        _out << sp << nl << "#endregion"; // Slice operations
    }

    writeInheritedOperations(p);

    writeDispatchAndMarshalling(p, _stream);

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
        
        _out << sp << nl << "public void invoke__" << spar << "Ice.ObjectPrx prx__"
            << paramsInvoke << "Ice.Context ctx__" << epar;
        _out << sb;
        _out << nl << "try";
        _out << sb;
        _out << nl << "prepare__(prx__, \"" << name << "\", " << sliceModeToIceMode(p->sendMode()) 
             << ", ctx__);";
        for(q = inParams.begin(); q != inParams.end(); ++q)
        {
            string typeS = typeToString(q->first);
            writeMarshalUnmarshalCode(_out, q->first, fixId(q->second), true, false, false);
        }
        if(p->sendsClasses())
        {
            _out << nl << "os__.writePendingObjects();";
        }
        _out << nl << "os__.endWriteEncaps();";
        _out << eb;
        _out << nl << "catch(Ice.LocalException ex__)";
        _out << sb;
        _out << nl << "finished__(ex__);";
        _out << nl << "return;";
        _out << eb;
        _out << nl << "send__();";
        _out << eb;

        _out << sp << nl << "protected override void response__(bool ok__)";
        _out << sb;
        for(q = outParams.begin(); q != outParams.end(); ++q)
        {
            _out << nl << typeToString(q->first) << ' ' << fixId(q->second) << ';';
        }
        if(ret)
        {
            _out << nl << retS << " ret__;";
        }
        _out << nl << "try";
        _out << sb;
        _out << nl << "if(!ok__)";
        _out << sb;
        _out << nl << "try";
        _out << sb;
        _out << nl << "is__.throwException();";
        _out << eb;
        for(ExceptionList::const_iterator r = throws.begin(); r != throws.end(); ++r)
        {
            _out << nl << "catch(" << fixId((*r)->scoped()) << ')';
            _out << sb;
            _out << nl << "throw;";
            _out << eb;
        }
        _out << nl << "catch(Ice.UserException ex)";
        _out << sb;
        _out << nl << "throw new Ice.UnknownUserException(ex);";
        _out << eb;
        _out << eb;
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
            _out << nl << "is__.readPendingObjects();";
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
        }
        if(ret)
        {
            BuiltinPtr builtin = BuiltinPtr::dynamicCast(ret);
            if((builtin && builtin->kind() == Builtin::KindObject) || ClassDeclPtr::dynamicCast(ret))
            {
                string type = typeToString(ret);
                _out << nl << "ret__ = (" << retS << ")ret___PP.value;";
            }
        }
        _out << eb;
        _out << nl << "catch(Ice.LocalException ex__)";
        _out << sb;
        _out << nl << "finished__(ex__);";
        _out << nl << "return;";
        _out << eb;
        if(!throws.empty())
        {
            _out << nl << "catch(Ice.UserException ex__)";
            _out << sb;
            _out << nl << "ice_exception(ex__);";
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
            _out << nl << "IceInternal.BasicStream os__ = this.os__();";
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
                _out << nl << "os__.writePendingObjects();";
            }
            _out << eb;
            _out << nl << "catch(Ice.LocalException ex__)";
            _out << sb;
            _out << nl << "ice_exception(ex__);";
            _out << eb;
        }
        _out << nl << "response__(true);";
        _out << eb;

        _out << sp << nl << "public void ice_exception(_System.Exception ex)";
        _out << sb;
        if(throws.empty())
        {
            _out << nl << "exception__(ex);";
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
                _out << nl << "catch(" << exS << " ex__)";
                _out << sb;
                _out << nl << "os__().writeUserException(ex__);";
                _out << nl << "response__(false);";
                _out << eb;
            }
            _out << nl << "catch(_System.Exception ex__)";
            _out << sb;
            _out << nl << "exception__(ex__);";
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

    _out << sp << nl << "public class " << name << "Tie_ : ";
    if(p->isInterface())
    {
        if(p->isLocal())
        {
            _out << fixId(name) << ", Ice.TieBase";
        }
        else
        {
            _out << name << "Disp_, Ice.TieBase";
        }
    }
    else
    {
        _out << fixId(name) << ", Ice.TieBase";
    }
    _out << sb;

    _out << sp << nl << "public " << name << "Tie_()";
    _out << sb;
    _out << eb;

    _out << sp << nl << "public " << name << "Tie_(" << name << opIntfName << "_ del)";
    _out << sb;
    _out << nl << "_ice_delegate = del;";
    _out << eb;

    _out << sp << nl << "public object ice_delegate()";
    _out << sb;
    _out << nl << "return _ice_delegate;";
    _out << eb;

    _out << sp << nl << "public void ice_delegate(object del)";
    _out << sb;
    _out << nl << "_ice_delegate = (" << name << opIntfName << "_)del;";
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
    _out << nl << "if(!(rhs is " << name << "Tie_))";
    _out << sb;
    _out << nl << "return false;";
    _out << eb;
    _out << nl << "if(_ice_delegate == null)";
    _out << sb;
    _out << nl << "return ((" << name << "Tie_)rhs)._ice_delegate == null;";
    _out << eb;
    _out << nl << "return _ice_delegate.Equals(((" << name << "Tie_)rhs)._ice_delegate);";
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
        _out << (hasAMD ? string("void") : retS) << ' ' << opName << spar << params;
        if(!p->isLocal())
        {
            _out << "Ice.Current current__";
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
            _out << "current__";
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

    _out << sp << nl << "private " << name << opIntfName << "_ _ice_delegate;";

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
        _out << (hasAMD ? string("void") : retS) << ' ' << opName << spar << params;
        if(!p->isLocal())
        {
            _out << "Ice.Current current__";
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
            _out << "current__";
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

    if(comment)
    {
        _out << nl << "// ";
    }
    else
    {
        _out << sp << nl;
    }

        ParamDeclList::const_iterator i;
    if(!cl->isLocal() && (cl->hasMetaData("amd") || op->hasMetaData("amd")))
    {
        ParamDeclList::const_iterator i;
        vector<string> pDecl = getParamsAsync(op, true);

        _out << "public ";
        if(!forTie)
        {
            _out << "override ";
        }
        _out << "void " << opName << "_async" << spar << pDecl << "Ice.Current current__" << epar;

        if(comment)
        {
            _out << ';';
            return;
        }

        _out << sb;
        if(ret)
        {
            _out << nl << typeToString(ret) << " ret__ = " << writeValue(ret) << ';';
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
            _out << "Ice.Current current__";
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
        return st->hasMetaData("clr:class") ? string("null") : "new " + fixId(st->scoped()) + "()";
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
            _out << " : " << fixId(name);
        }
        else
        {
            _out << " : " << name << "Disp_";
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
    _out << name << "Operations";
    if(p->isLocal())
    {
        _out << "NC";
    }
    _out << '_';
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
