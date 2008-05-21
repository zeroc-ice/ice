// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
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
#include <IceUtil/Iterator.h>
#include <IceUtil/UUID.h>
#include <Slice/DotNetNames.h>
#include <Slice/SignalHandler.h>

using namespace std;
using namespace Slice;

//
// Don't use "using namespace IceUtil", or VC++ 6.0 complains about
// ambigious symbols for constructs like
// "IceUtil::constMemFun(&Slice::Exception::isLocal)".
//
using IceUtilInternal::Output;
using IceUtilInternal::nl;
using IceUtilInternal::sp;
using IceUtilInternal::sb;
using IceUtilInternal::eb;
using IceUtilInternal::spar;
using IceUtilInternal::epar;

//
// Callback for Crtl-C signal handling
//
static Gen* _gen = 0;

static void closeCallback()
{
    if(_gen != 0)
    {
        _gen->closeOutput();
    }
}

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
            string name = fixId((*op)->name(), DotNet::ICloneable, true);

            vector<string> params = getParams(*op);
            vector<string> args = getArgs(*op);
            string retS = typeToString((*op)->returnType());

            _out << sp << nl << "public abstract " << retS << ' ' << name << spar << params;
            _out << epar << ';';
        }

        _out << sp << nl << "#endregion"; // Inherited Slice operations
    }
}

void
Slice::CsVisitor::writeDispatchAndMarshalling(const ClassDefPtr& p)
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
    StringList::difference_type scopedPos = IceUtilInternal::distance(firstIter, scopedIter);

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
    _out << nl << "return _System.Array.BinarySearch(ids__, 0, ids__.Length, s, IceUtil.StringUtil.OrdinalStringComparer) >= 0;";
    _out << eb;

    _out << sp << nl << "public override string[] ice_ids()";
    _out << sb;
    _out << nl << "return ids__;";
    _out << eb;

    _out << sp << nl << "public override string ice_id()";
    _out << sb;
    _out << nl << "return ids__[" << scopedPos << "];";
    _out << eb;

    _out << sp << nl << "public static new string ice_staticId()";
    _out << sb;
    _out << nl << "return ids__[" << scopedPos << "];";
    _out << eb;

    _out << sp << nl << "#endregion"; // Slice type-related members

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
        writeMarshalUnmarshalCode(_out, (*d)->type(), fixId(*d, DotNet::ICloneable, true),
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
        _out << "class Patcher__ : IceInternal.Patcher<" << name << ">";
        _out << sb;
        _out << sp << nl << "internal Patcher__(string type, Ice.ObjectImpl instance";
        if(allClassMembers.size() > 1)
        {
            _out << ", int member";
        }
        _out << ") : base(type)";
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
            _out << nl << "_instance." << memberName << " = (" << memberType << ")v;";
            ContainedPtr contained = ContainedPtr::dynamicCast((*d)->type());
            string sliceId = contained ? contained->scoped() : string("::Ice::Object");
            _out << nl << "_typeId = \"" << sliceId << "\";";
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
        _out << nl << "IceInternal.Ex.throwUOE(_typeId, v.ice_id());";
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
        writeMarshalUnmarshalCode(_out, (*d)->type(), fixId(*d, DotNet::ICloneable, true),
                                  false, false, false, patchParams.str());
    }
    _out << nl << "is__.endReadSlice();";
    _out << nl << "base.read__(is__, true);";
    _out << eb;

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
Slice::CsVisitor::getParamsAsync(const OperationPtr& op, AsyncType type )
{
    vector<string> params;

    string name = fixId(op->name());
    ContainerPtr container = op->container();
    ClassDefPtr cl = ClassDefPtr::dynamicCast(container); // Get the class containing the op.
    string scope = fixId(cl->scope());
    if(type == AMIDelegate)
    {
        params.push_back(scope + "AMI_" + cl->name() + '_' + op->name() + "_response resp__");
        params.push_back(scope + "AMI_" + cl->name() + '_' + op->name() + "_exception ex__");
    }
    else
    {
        params.push_back(scope + (type == AMD ? "AMD_" : "AMI_") + cl->name() + '_' + op->name() + " cb__");
    }

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

Slice::Gen::Gen(const string& name, const string& base, const vector<string>& includePaths, const string& dir) :
    _includePaths(includePaths)
{
    _gen = this;
    SignalHandler::setCallback(closeCallback);

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
    SignalHandler::addFile(file);
    SignalHandler::addFile(fileImpl);

    _out.open(file.c_str());
    if(!_out)
    {
        cerr << name << ": can't open `" << file << "' for writing" << endl;
        return;
    }
    printHeader();

    _out << nl << "// Generated from file `" << fileBase << ".ice'";

    _out << sp << nl << "using _System = global::System;";
    _out << nl << "using _Microsoft = global::Microsoft;";
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

    SignalHandler::setCallback(0);
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

    UnitVisitor unitVisitor(_out);
    p->visit(&unitVisitor, false);

    TypesVisitor typesVisitor(_out);
    p->visit(&typesVisitor, false);

    ProxyVisitor proxyVisitor(_out);
    p->visit(&proxyVisitor, false);

    OpsVisitor opsVisitor(_out);
    p->visit(&opsVisitor, false);

    HelperVisitor helperVisitor(_out);
    p->visit(&helperVisitor, false);

    DispatcherVisitor dispatcherVisitor(_out);
    p->visit(&dispatcherVisitor, false);

    AsyncVisitor asyncVisitor(_out);
    p->visit(&asyncVisitor, false);
}

void
Slice::Gen::closeOutput()
{
    _out.close();
    _impl.close();
}

void
Slice::Gen::printHeader()
{
    static const char* header =
"// **********************************************************************\n"
"//\n"
"// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.\n"
"//\n"
"// This copy of Ice is licensed to you under the terms described in the\n"
"// ICE_LICENSE file included in this distribution.\n"
"//\n"
"// **********************************************************************\n"
        ;

    _out << header;
    _out << "\n// Ice for Silverlight version " << ICESL_STRING_VERSION;
}

Slice::Gen::UnitVisitor::UnitVisitor(IceUtilInternal::Output& out) :
    CsVisitor(out), _globalMetaDataDone(false)
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

Slice::Gen::TypesVisitor::TypesVisitor(IceUtilInternal::Output& out) :
    CsVisitor(out)
{
}

bool
Slice::Gen::TypesVisitor::visitModuleStart(const ModulePtr& p)
{
    DictionaryList dicts;
    if(p->hasOnlyDictionaries(dicts))
    {
        //
        // If this module contains only dictionaries and they
        // all use the new dictionary mapping, we don't need to generate
        // anything for the dictionary types. The early return prevents
        // an empty namespace from being emitted--the namespace will
        // be emitted later by the dictionary helper (which is generated
        // for both old and new dictionaries).
        //
        bool foundOld = false;
        for(DictionaryList::const_iterator i = dicts.begin(); i != dicts.end() && !foundOld; ++i)
        {
            if((*i)->hasMetaData("clr:collection"))
            {
                foundOld = true;
            }
        }
        if(!foundOld)
        {
            return false;
        }
    }

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
/*
    if(p->isInterface() && !p->isLocal())
    {
        return false;
    }
*/

    string name = p->name();
    ClassList bases = p->bases();
    bool hasBaseClass = !bases.empty() && !bases.front()->isInterface();

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
            _out << "Ice.Object, " << name << "OperationsNC_";
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

    if(p->isInterface())
    {
        _out << eb;
        return;
    }

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
    if(!p->isLocal())
    {
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
        other.push_back(scoped);
        other.push_back("::Ice::Object");
        other.sort();
        ids.merge(other);
        ids.unique();

        StringList::const_iterator firstIter = ids.begin();
        StringList::const_iterator scopedIter = find(ids.begin(), ids.end(), scoped);
        assert(scopedIter != ids.end());
        StringList::difference_type scopedPos = IceUtilInternal::distance(firstIter, scopedIter);

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
        _out << nl << "return _System.Array.BinarySearch(ids__, 0, ids__.Length, s, IceUtil.StringUtil.OrdinalStringComparer) >= 0;";
        _out << eb;

        _out << sp << nl << "public override string[] ice_ids()";
        _out << sb;
        _out << nl << "return ids__;";
        _out << eb;

        _out << sp << nl << "public override string ice_id()";
        _out << sb;
        _out << nl << "return ids__[" << scopedPos << "];";
        _out << eb;

        _out << sp << nl << "public static new string ice_staticId()";
        _out << sb;
        _out << nl << "return ids__[" << scopedPos << "];";
        _out << eb;

        _out << sp << nl << "#endregion"; // Slice type-related members

        // Marshalling support
        _out << sp << nl << "#region Marshaling support";

        _out << sp << nl << "public override void write__(IceInternal.BasicStream os__)";
        _out << sb;
        _out << nl << "os__.writeTypeId(ice_staticId());";
        _out << nl << "os__.startWriteSlice();";
        for(d = dataMembers.begin(); d != dataMembers.end(); ++d)
        {
            writeMarshalUnmarshalCode(_out, (*d)->type(), fixId(*d, DotNet::ICloneable, true),
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
            _out << "class Patcher__ : IceInternal.Patcher<" << name << ">";
            _out << sb;
            _out << sp << nl << "internal Patcher__(string type, Ice.ObjectImpl instance";
            if(allClassMembers.size() > 1)
            {
                _out << ", int member";
            }
            _out << ") : base(type)";
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
                _out << nl << "_instance." << memberName << " = (" << memberType << ")v;";
                _out << nl << "_typeId = \"" << memberType << "\";";
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
            _out << nl << "IceInternal.Ex.throwUOE(_typeId, v.ice_id());";
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
        for(d = dataMembers.begin(); d != dataMembers.end(); ++d)
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
            writeMarshalUnmarshalCode(_out, (*d)->type(), fixId(*d, DotNet::ICloneable, true),
                                      false, false, false, patchParams.str());
        }
        _out << nl << "is__.endReadSlice();";
        _out << nl << "base.read__(is__, true);";
        _out << eb;

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

    string name = p->name();
    ParamDeclList paramList = p->parameters();
    vector<string> params;
    vector<string> args;
    string retS;

    params = getParams(p);
    args = getArgs(p);
    name = fixId(name, DotNet::ICloneable, true);
    retS = typeToString(p->returnType());

    _out << sp;
    emitAttributes(p);
    _out << nl << "public abstract ";
    _out << retS << " " << name << spar << params << epar;
    _out << ";";
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

    //
    // No need to generate anything for custom sequences.
    //
    string prefix = "clr:type:";
    string meta;
    if(p->findMetaData(prefix, meta))
    {
        return;
    }

    string name = fixId(p->name());
    string s = typeToString(p->type());

    _out << sp;

    emitDeprecate(p, 0, _out, "type");

    emitAttributes(p);
    _out << nl << "public class " << name
         << " : Ice.CollectionBase<" << s << ">, _System.ICloneable";
    _out << sb;

    _out << sp << nl << "#region Constructors";

    _out << sp << nl << "public " << name << "() : base()";
    _out << sb;
    _out << eb;

    _out << sp << nl << "public " << name << "(int capacity) : base(capacity)";
    _out << sb;
    _out << eb;

    _out << sp << nl << "public " << name << "(" << s << "[] a__) : base(a__)";
    _out << sb;
    _out << eb;

    _out << sp << nl << "public " << name << "(_System.Collections.Generic.IEnumerable<" << s << "> l__) : base(l__)";
    _out << sb;
    _out << eb;

    _out << sp << nl << "#endregion"; // Constructors

    _out << sp << nl << "#region Implicit conversion to generic List";

    _out << sp << nl << "public static implicit operator _System.Collections.Generic.List<"
         << s << ">(" << name << " s__)";
    _out << sb;
    _out << nl << "return s__.list_;";
    _out << eb;

    _out << sp << nl << "#endregion"; // Implicit conversion to generic List

    _out << sp << nl << "#region Operations returning a new sequence";

    _out << sp << nl << "public virtual " << name << " GetRange(int index, int count)";
    _out << sb;
    _out << nl << "_System.Collections.Generic.List<" << s << "> l = list_.GetRange(index, count);";
    _out << nl << name << " r = new " << name << "(l.Count);";
    _out << nl << "r.list_.AddRange(l);";
    _out << nl << "return r;";
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

    _out << sp << nl << "public object Clone()";
    _out << sb;
    _out << nl << name << " s = new " << name << "(Count);";
    _out << nl << "s.list_.AddRange(list_);";
    _out << nl << "return s;";
    _out << eb;

    _out << sp << nl << "#endregion"; // Operations returning a new sequence

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
                string name = fixId((*q)->name(), DotNet::Exception, false);
                _out << nl << "this." << name << " = " << fixId((*q)->name()) << ';';
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
    if(p->base())
    {
        _out << nl << "int h__ = base.GetHashCode();";
    }
    else
    {
        _out << nl << "int h__ = 0;";
    }
    writeMemberHashCode(dataMembers, DotNet::Exception);
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
    if(p->base())
    {
        _out << nl << "if(!base.Equals(other__))";
        _out << sb;
        _out << nl << "return false;";
        _out << eb;
    }
    if(!dataMembers.empty())
    {
        _out << nl << name << " o__ = (" << name << ")other__;";
    }
    writeMemberEquals(dataMembers, DotNet::Exception);
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
            writeMarshalUnmarshalCode(_out, (*q)->type(), fixId(*q, DotNet::Exception),
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
            _out << "class Patcher__ : IceInternal.Patcher<" << name << ">";
            _out << sb;
            _out << sp << nl << "internal Patcher__(string type, Ice.Exception instance";
            if(allClassMembers.size() > 1)
            {
                _out << ", int member";
            }
            _out << ") : base(type)";
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
                string memberName = fixId((*q)->name(), DotNet::Exception);
                string memberType = typeToString((*q)->type());
                _out << nl << "_instance." << memberName << " = (" << memberType << ")v;";
                ContainedPtr contained = ContainedPtr::dynamicCast((*q)->type());
                string sliceId = contained ? contained->scoped() : string("::Ice::Object");
                _out << nl << "_typeId = \"" << sliceId << "\";";
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
            _out << nl << "IceInternal.Ex.throwUOE(_typeId, v.ice_id());";
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
            writeMarshalUnmarshalCode(_out, (*q)->type(), fixId((*q)->name(), DotNet::Exception),
                                      false, false, false, patchParams.str());
        }
        _out << nl << "is__.endReadSlice();";
        if(base)
        {
            _out << nl << "base.read__(is__, true);";
        }
        _out << eb;

        if(!base || !base->usesClasses())
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

    bool propertyMapping = p->hasMetaData("clr:property");

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
        string memberName = fixId((*q)->name(), isClass ? DotNet::ICloneable : 0);
        string memberType = typeToString((*q)->type());
        paramDecl.push_back(memberType + " " + memberName);
        paramNames.push_back(memberName);
    }
    _out << paramDecl << epar;
    _out << sb;
    for(vector<string>::const_iterator i = paramNames.begin(); i != paramNames.end(); ++i)
    {
        _out << nl << "this." << *i;
        if(propertyMapping)
        {
            _out << "_prop";
        }
        _out << " = " << *i << ';';
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
    writeMemberHashCode(dataMembers, isClass ? DotNet::ICloneable : 0);
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
    writeMemberEquals(dataMembers, isClass ? DotNet::ICloneable : 0);
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
            writeMarshalUnmarshalCode(_out, (*q)->type(), fixId(*q, isClass ? DotNet::ICloneable : 0),
                                      true, false, false);
        }
        _out << eb;


        if(isClass && classMembers.size() != 0)
        {
            _out << sp << nl << "public sealed class Patcher__ : IceInternal.Patcher<" << name << ">";
            _out << sb;
            _out << sp << nl << "internal Patcher__(string type, " << name << " instance";
            if(classMembers.size() > 1)
            {
                _out << ", int member";
            }
            _out << ") : base(type)";
            _out << sb;
            _out << nl << "_instance = (" << name << ")instance;";
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
                _out << nl << "_instance." << memberName << " = (" << memberType << ")v;";
                ContainedPtr contained = ContainedPtr::dynamicCast((*q)->type());
                string sliceId = contained ? contained->scoped() : string("::Ice::Object");
                _out << nl << "_typeId = \"" << sliceId << "\";";
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
            _out << nl << "IceInternal.Ex.throwUOE(_typeId, v.ice_id());";
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
            writeMarshalUnmarshalCode(_out, (*q)->type(), fixId(*q, isClass ? DotNet::ICloneable : 0 ),
                                      false, false, false, patchParams.str());
        }
        _out << eb;

        _out << sp << nl << "#endregion"; // Marshalling support
    }

    _out << eb;
}

void
Slice::Gen::TypesVisitor::visitDictionary(const DictionaryPtr& p)
{
    if(!p->hasMetaData("clr:collection"))
    {
        return;
    }

    string name = fixId(p->name());
    string ks = typeToString(p->keyType());
    string vs = typeToString(p->valueType());

    _out << sp;

    emitDeprecate(p, 0, _out, "type");

    emitAttributes(p);
    _out << nl << "public class " << name
         << " : Ice.DictionaryBase<" << ks << ", " << vs << ">, _System.ICloneable";
    _out << sb;

    _out << sp << nl << "#region " << name << " members";

    _out << sp << nl << "public void AddRange(" << name << " d__)";
    _out << sb;
    _out << nl << "foreach(_System.Collections.Generic.KeyValuePair<" << ks << ", " << vs << "> e in d__.dict_)";
    _out << sb;
    _out << nl << "try";
    _out << sb;
    _out << nl << "dict_.Add(e.Key, e.Value);";
    _out << eb;
    _out << nl << "catch(_System.ArgumentException)";
    _out << sb;
    _out << nl << "// ignore";
    _out << eb;
    _out << eb;
    _out << eb;

    _out << sp << nl << "#endregion"; // <name> members

    _out << sp << nl << "#region ICloneable members";

    _out << sp << nl << "public object Clone()";
    _out << sb;
    _out << nl << name << " d = new " << name << "();";
    _out << nl << "foreach(_System.Collections.Generic.KeyValuePair<" << ks << ", " << vs <<"> e in dict_)";
    _out << sb;
    _out << nl << "d.dict_.Add(e.Key, e.Value);";
    _out << eb;
    _out << nl << "return d;";
    _out << eb;

    _out << sp << nl << "#endregion"; // ICloneable members

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
    bool isProtected = false;
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
        baseTypes = DotNet::Exception;
    }
    else if(ClassDefPtr::dynamicCast(cont))
    {
        baseTypes = DotNet::ICloneable;
        isClass = true;
        if(cont->hasMetaData("clr:property"))
        {
            propertyMapping = true;
        }
        isProtected = cont->hasMetaData("protected") || p->hasMetaData("protected");
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

    _out << nl;
    if(propertyMapping)
    {
        _out << "private";
    }
    else if(isProtected)
    {
        _out << "protected";
    }
    else
    {
        _out << "public";
    }
    _out << ' ' << type << ' ' << dataMemberName << ';';

    if(!propertyMapping)
    {
        return;
    }

    _out << nl << (isProtected ? "protected" : "public");
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

void
Slice::Gen::TypesVisitor::writeMemberHashCode(const DataMemberList& dataMembers, int baseTypes)
{
    for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        string memberName = fixId((*q)->name(), baseTypes);
        TypePtr memberType = (*q)->type();
        bool isValue = isValueType(memberType);
        if(!isValue)
        {
            _out << nl << "if(" << memberName << " != null)";
            _out << sb;
        }
        SequencePtr seq = SequencePtr::dynamicCast(memberType);
        if(seq)
        {
            string genericType;
            bool isGeneric = seq->findMetaData("clr:generic:", genericType);
            bool isArray = !isGeneric && !seq->hasMetaData("clr:collection");
            if(isArray)
            {
                //
                // GetHashCode() for native arrays does not have value semantics.
                //
                _out << nl << "h__ = 5 * h__ + IceUtil.Arrays.GetHashCode(" << memberName << ");";
            }
            else if(isGeneric)
            {
                //
                // GetHashCode() for generic types does not have value semantics.
                //
                _out << nl << "h__ = 5 * h__ + IceUtil.Collections.SequenceGetHashCode(" << memberName << ");";
            }
            else
            {
                //
                // GetHashCode() for CollectionBase has value semantics.
                //
                _out << nl << "h__ = 5 * h__ + " << memberName << ".GetHashCode();";
            }
        }
        else
        {
            DictionaryPtr dict = DictionaryPtr::dynamicCast(memberType);
            if(dict)
            {
                if(dict->hasMetaData("clr:collection"))
                {
                    //
                    // GetHashCode() for DictionaryBase has value semantics.
                    //
                    _out << nl << "h__ = 5 * h__ + " << memberName << ".GetHashCode();";
                }
                else
                {
                    //
                    // GetHashCode() for generic types does not have value semantics.
                    //
                    _out << nl << "h__ = 5 * h__ + IceUtil.Collections.DictionaryGetHashCode(" << memberName
                         << ");";
                }
            }
            else
            {
                _out << nl << "h__ = 5 * h__ + " << memberName << ".GetHashCode();";
            }
        }
        if(!isValue)
        {
            _out << eb;
        }
    }
}

void
Slice::Gen::TypesVisitor::writeMemberEquals(const DataMemberList& dataMembers, int baseTypes)
{
    for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        string memberName = fixId((*q)->name(), baseTypes);
        TypePtr memberType = (*q)->type();
        if(!isValueType(memberType))
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
            SequencePtr seq = SequencePtr::dynamicCast(memberType);
            if(seq)
            {
                string genericType;
                bool isGeneric = seq->findMetaData("clr:generic:", genericType);
                bool isArray = !isGeneric && !seq->hasMetaData("clr:collection");
                if(isArray)
                {
                    //
                    // Equals() for native arrays does not have value semantics.
                    //
                    _out << nl << "if(!IceUtil.Arrays.Equals(" << memberName << ", o__." << memberName << "))";
                }
                else if(isGeneric)
                {
                    //
                    // Equals() for generic types does not have value semantics.
                    //
                    _out << nl << "if(!IceUtil.Collections.SequenceEquals(" << memberName << ", o__."
                         << memberName << "))";
                }
                else
                {
                    //
                    // Equals() for CollectionBase has value semantics.
                    //
                    _out << nl << "if(!" << memberName << ".Equals(o__." << memberName << "))";
                }
            }
            else
            {
                DictionaryPtr dict = DictionaryPtr::dynamicCast(memberType);
                if(dict)
                {
                    if(dict->hasMetaData("clr:collection"))
                    {
                        //
                        // Equals() for DictionaryBase has value semantics.
                        //
                        _out << nl << "if(!" << memberName << ".Equals(o__." << memberName << "))";
                    }
                    else
                    {
                        //
                        // Equals() for generic types does not have value semantics.
                        //
                        _out << nl << "if(!IceUtil.Collections.DictionaryEquals(" << memberName << ", o__."
                             << memberName << "))";
                    }
                }
                else
                {
                    _out << nl << "if(!" << memberName << ".Equals(o__." << memberName << "))";
                }
            }
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
}

Slice::Gen::ProxyVisitor::ProxyVisitor(IceUtilInternal::Output& out)
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
         << spar << params << "_System.Collections.Generic.Dictionary<string, string> context__" << epar << ';';

    if(cl->hasMetaData("ami") || p->hasMetaData("ami"))
    {
        vector<string> paramsAMI = getParamsAsync(p, AMIDelegate);

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
        _out << nl << "void " << p->name() << "_async" << spar << paramsAMI
             << "_System.Collections.Generic.Dictionary<string, string> ctx__" << epar << ';';
    }
}

Slice::Gen::OpsVisitor::OpsVisitor(IceUtilInternal::Output& out)
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
            params = getParamsAsync(op, AMD);
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

Slice::Gen::HelperVisitor::HelperVisitor(IceUtilInternal::Output& out) :
    CsVisitor(out)
{
}

bool
Slice::Gen::HelperVisitor::visitModuleStart(const ModulePtr& p)
{
    if(!p->hasNonLocalClassDecls() && !p->hasNonLocalSequences() && !p->hasDictionaries())
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
             << "_System.Collections.Generic.Dictionary<string, string> context__" << epar;
        _out << sb;
        _out << nl;
        if(ret)
        {
            _out << "return ";
        }
        _out << opName << spar << args << "context__" << "true" << epar << ';';
        _out << eb;

        _out << sp << nl << "private " << retS << " " << opName << spar << params
             << "_System.Collections.Generic.Dictionary<string, string> context__"
             << "bool explicitContext__" << epar;
        _out << sb;

        _out << nl << "if(explicitContext__ && context__ == null)";
        _out << sb;
        _out << nl << "context__ = emptyContext_;";
        _out << eb;
        _out << nl << "int cnt__ = 0;";
        _out << nl << "while(true)";
        _out << sb;
        //XXX:
        _out << nl << "try";
        _out << sb;
        _out << nl << "IceInternal.Outgoing og__ = getOutgoing(\"" << op->name() << "\", "
             << sliceModeToIceMode(op->sendMode())
             << ", context__);";
        if(!inParams.empty())
        {
            _out << nl << "IceInternal.BasicStream os__ = og__.ostr();";
            for(q = inParams.begin(); q != inParams.end(); ++q)
            {
                writeMarshalUnmarshalCode(_out, q->first, fixId(q->second), true, false, false);
            }
            if(op->sendsClasses())
            {
                _out << nl << "os__.writePendingObjects();";
            }
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
            string param = fixId(q->second);
            StructPtr st = StructPtr::dynamicCast(q->first);
            if(st)
            {
                if(isValueType(q->first))
                {
                    _out << nl << param << " = new " << typeToString(q->first) << "();";
                }
                else
                {
                    _out << nl << param << " = null;";
                }
            }
            writeMarshalUnmarshalCode(_out, q->first, fixId(q->second), false, false, true, "");
        }
        if(ret)
        {
            BuiltinPtr builtin = BuiltinPtr::dynamicCast(ret);
            if((builtin && builtin->kind() == Builtin::KindObject) || ClassDeclPtr::dynamicCast(ret))
            {
                _out << nl << retS << " ret__;";
                ContainedPtr contained = ContainedPtr::dynamicCast(ret);
                string sliceId = contained ? contained->scoped() : string("::Ice::Object");
                _out << nl << "IceInternal.ParamPatcher<" << retS << "> ret___PP = new IceInternal.ParamPatcher<"
                     << retS << ">(\"" << sliceId << "\");";
                _out << nl << "is__.readObject(ret___PP);";
            }
            else
            {
                _out << nl << retS << " ret__;";
                StructPtr st = StructPtr::dynamicCast(ret);
                if(st)
                {
                    if(isValueType(st))
                    {
                        _out << nl << "ret__ = new " << retS << "();";
                    }
                    else
                    {
                        _out << nl << "ret__ = null;";
                    }
                }
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
                    _out << nl << param << " = null;";
                    _out << nl << "IceInternal.Ex.throwUOE(" << param << "_PP.type(), "
                         << param << "_PP.value.ice_id());";
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
                _out << nl << "ret__ = null;";
                _out << nl << "IceInternal.Ex.throwUOE(ret___PP.type(), ret___PP.value.ice_id());";
                _out << eb;
            }
            _out << nl << "return ret__;";
        }
        _out << eb;
        _out << nl << "catch(Ice.LocalException ex__)";
        _out << sb;
        _out << nl << "throw new IceInternal.LocalExceptionWrapper(ex__, false);";
        _out << eb;

        if(!ret)
        {
            _out << nl << "return;";
        }

        // XXX: Generated code for marshaling, etc.
        _out << eb;
        _out << nl << "catch(IceInternal.LocalExceptionWrapper ex__)";
        _out << sb;
        if(op->mode() == Operation::Idempotent || op->mode() == Operation::Nonmutating)
        {
            _out << nl << "cnt__ = handleExceptionWrapperRelaxed__(ex__, cnt__);";
        }
        else
        {
            _out << nl << "handleExceptionWrapper__(ex__);";
        }
        _out << eb;
        _out << nl << "catch(Ice.LocalException ex__)";
        _out << sb;
        _out << nl << "cnt__ = handleException__(ex__, cnt__);";
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
            vector<string> paramsAMI = getParamsAsync(op, AMI);
            vector<string> paramsAMIDel = getParamsAsync(op, AMIDelegate);
            vector<string> argsAMI = getArgsAsync(op);

            string opName = op->name();

            //
            // Write two versions of the operation - with and without a
            // context parameter
            //

            string callbackName = fixId(p->scope()) + "AMI_" + containingClass->name() + "_" + opName;

            _out << sp;
            _out << nl << "public void " << opName << "_async" << spar << paramsAMIDel << epar;
            _out << sb;
            _out << nl << callbackName << "  cb__ = new " << callbackName << "(resp__, ex__);";
            _out << nl << opName << "_async" << spar << argsAMI << "null" << "false" << epar << ';';
            _out << eb;

            _out << sp;
            _out << nl << "public void " << opName << "_async" << spar << paramsAMIDel
                 << "_System.Collections.Generic.Dictionary<string, string> ctx__" << epar;
            _out << sb;
            _out << nl << callbackName << "  cb__ = new " << callbackName << "(resp__, ex__);";
            _out << nl << opName << "_async" << spar << argsAMI << "ctx__" << "true" << epar << ';';
            _out << eb;

            _out << sp;
            _out << nl << "public void " << opName << "_async" << spar << paramsAMI
                 << "_System.Collections.Generic.Dictionary<string, string> ctx__"
                 << "bool explicitContext__" << epar;
            _out << sb;
            _out << nl << "if(explicitContext__ && ctx__ == null)";
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

    _out << sp << nl << "public static " << name
         << "Prx checkedCast(Ice.ObjectPrx b, _System.Collections.Generic.Dictionary<string, string> ctx)";
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

    _out << sp << nl << "public static " << name
         << "Prx checkedCast(Ice.ObjectPrx b, string f, "
         << "_System.Collections.Generic.Dictionary<string, string> ctx)";
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
    // Don't generate helper for sequence of a local type.
    //
    if(p->isLocal())
    {
        return;
    }

    string typeS = typeToString(p);

    _out << sp << nl << "public sealed class " << p->name() << "Helper";
    _out << sb;

    _out << sp << nl << "public static void write(IceInternal.BasicStream os__, " << typeS << " v__)";
    _out << sb;
    writeSequenceMarshalUnmarshalCode(_out, p, "v__", true, false);
    _out << eb;

    _out << sp << nl << "public static " << typeS << " read(IceInternal.BasicStream is__)";
    _out << sb;
    _out << nl << typeS << " v__;";
    writeSequenceMarshalUnmarshalCode(_out, p, "v__", false, false);
    _out << nl << "return v__;";
    _out << eb;

    _out << eb;

    string prefix = "clr:generic:";
    string meta;
    if(p->findMetaData(prefix, meta))
    {
        string type = meta.substr(prefix.size());
        if(type == "List" || type == "LinkedList" || type == "Queue" || type == "Stack")
        {
            return;
        }
        BuiltinPtr builtin = BuiltinPtr::dynamicCast(p->type());
        bool isClass = (builtin && builtin->kind() == Builtin::KindObject)
                        || ClassDeclPtr::dynamicCast(p->type());

        if(!isClass)
        {
            return;
        }

        //
        // The sequence is a custom sequence with elements of class type.
        // Emit a dummy class that causes a compile-time error if the
        // custom sequence type does not implement an indexer.
        //
        _out << sp << nl << "public class " << p->name() << "_Tester";
        _out << sb;
        _out << nl << p->name() << "_Tester()";
        _out << sb;
        _out << nl << typeS << " test = new " << typeS << "();";
        _out << nl << "test[0] = null;";
        _out << eb;
        _out << eb;
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

    string meta;
    bool isNewMapping = !p->hasMetaData("clr:collection");

    string prefix = "clr:generic:";
    string genericType;
    if(!p->findMetaData(prefix, meta))
    {
        genericType = "Dictionary";
    }
    else
    {
        genericType = meta.substr(prefix.size());
    }

    string keyS = typeToString(key);
    string valueS = typeToString(value);
    string name = isNewMapping
                        ? "_System.Collections.Generic." + genericType + "<" + keyS + ", " + valueS + ">"
                        : fixId(p->name());

    _out << sp << nl << "public sealed class " << p->name() << "Helper";
    _out << sb;

    _out << sp << nl << "public static void write(";
    _out.useCurrentPosAsIndent();
    _out << "IceInternal.BasicStream os__,";
    _out << nl << name << " v__)";
    _out.restoreIndent();
    _out << sb;
    _out << nl << "if(v__ == null)";
    _out << sb;
    _out << nl << "os__.writeSize(0);";
    _out << eb;
    _out << nl << "else";
    _out << sb;
    _out << nl << "os__.writeSize(v__.Count);";
    _out << nl << "foreach(_System.Collections.";
    if(isNewMapping)
    {
        _out << "Generic.KeyValuePair<" << keyS << ", " << valueS << ">";
    }
    else
    {
        _out << "DictionaryEntry";
    }
    _out << " e__ in v__)";
    _out << sb;
    string keyArg = isNewMapping ? string("e__.Key") : "((" + keyS + ")e__.Key)";
    writeMarshalUnmarshalCode(_out, key, keyArg, true, false, false);
    string valueArg = isNewMapping ? string("e__.Value") : "((" + valueS + ")e__.Value)";
    writeMarshalUnmarshalCode(_out, value, valueArg, true, false, false);
    _out << eb;
    _out << eb;
    _out << eb;

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(value);
    bool hasClassValue = (builtin && builtin->kind() == Builtin::KindObject) || ClassDeclPtr::dynamicCast(value);
    if(hasClassValue)
    {
        string expectedType = ContainedPtr::dynamicCast(value)->scoped();
        _out << sp << nl << "public sealed class Patcher__ : IceInternal.Patcher<" << valueS << ">";
        _out << sb;
        _out << sp << nl << "internal Patcher__(string type, " << name << " m, " << keyS << " key) : base(type)";
        _out << sb;
        _out << nl << "_m = m;";
        _out << nl << "_key = key;";
        _out << eb;

        _out << sp << nl << "public override void" << nl << "patch(Ice.Object v)";
        _out << sb;
        _out << nl << "try";
        _out << sb;
        _out << nl << "_m[_key] = (" << valueS << ")v;";
        _out << eb;
        _out << nl << "catch(System.InvalidCastException)";
        _out << sb;
        _out << nl << "IceInternal.Ex.throwUOE(type(), v.ice_id());";
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
    StructPtr st = StructPtr::dynamicCast(key);
    if(st)
    {
        if(isValueType(key))
        {
            _out << nl << "v__ = new " << typeToString(key) << "();";
        }
        else
        {
            _out << nl << "k__ = null;";
        }
    }
    writeMarshalUnmarshalCode(_out, key, "k__", false, false, false);
    if(!hasClassValue)
    {
        _out << nl << valueS << " v__;";

        StructPtr st = StructPtr::dynamicCast(value);
        if(st)
        {
            if(isValueType(value))
            {
                _out << nl << "v__ = new " << typeToString(value) << "();";
            }
            else
            {
                _out << nl << "v__ = null;";
            }
        }
    }
    writeMarshalUnmarshalCode(_out, value, "v__", false, false, false, "r__, k__");
    if(!hasClassValue)
    {
        _out << nl << "r__[k__] = v__;";
    }
    _out << eb;
    _out << nl << "return r__;";
    _out << eb;

    _out << eb;
}

Slice::Gen::DispatcherVisitor::DispatcherVisitor(::IceUtilInternal::Output &out) :
    CsVisitor(out)
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
        for(OperationList::const_iterator op = ops.begin(); op != ops.end(); ++op)
        {
            string opname = (*op)->name();
            vector<string> params = getParams(*op);
            vector<string> args = getArgs(*op);
            TypePtr ret = (*op)->returnType();

            opname = fixId(opname, DotNet::ICloneable, true);

            _out << sp << nl << "public abstract " << typeToString(ret) << " "
                 << opname << spar << params << epar << ';';
        }
        _out << sp << nl << "#endregion"; // Slice operations
    }

    writeInheritedOperations(p);

    writeDispatchAndMarshalling(p);

    _out << eb;

    return true;
}

Slice::Gen::AsyncVisitor::AsyncVisitor(::IceUtilInternal::Output &out)
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

        vector<string> paramsInvoke = getParamsAsync(p, AMI);

        _out << nl << "public delegate void AMI_" << cl->name() << "_" << name << "_response" << spar << params 
             << epar << ";";
        _out << nl << "public delegate void AMI_" << cl->name() << "_" << name << "_exception(Ice.Exception ex);";

        _out << sp << nl << "public sealed class AMI_" << cl->name() << '_'
             << name << " : IceInternal.OutgoingAsync";
        _out << sb;
        _out << sp;

        _out << nl << "public AMI_" << cl->name() << "_" << name << "(AMI_" << cl->name() << "_" << name 
             << "_response response, AMI_" << cl->name() << "_" << name << "_exception exception)";
        _out << sb;
        _out << nl << "response_ = response;";
        _out << nl << "exception_ = exception;";
        _out << nl;
        _out << nl << "System.Diagnostics.Debug.Assert(response_ != null);";
        _out << nl << "System.Diagnostics.Debug.Assert(exception_ != null);";
        _out << eb;
        
        _out << sp << nl << "public void invoke__" << spar << "Ice.ObjectPrx prx__"
             << paramsInvoke << "_System.Collections.Generic.Dictionary<string, string> ctx__" << epar;
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
            string param = fixId(q->second);
            string typeS = typeToString(q->first);
            _out << nl << typeS << ' ' << param << ';';
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
            string param = fixId(q->second);
            StructPtr st = StructPtr::dynamicCast(q->first);
            if(st)
	    {
		if(isValueType(st))
		{
		    _out << nl << param << " = new " << typeToString(q->first) << "();";
		}
		else
		{
		    _out << nl << param << " = null;";
		}
	    }
            writeMarshalUnmarshalCode(_out, q->first, fixId(q->second), false, false, true);
        }
        if(ret)
        {
            StructPtr st = StructPtr::dynamicCast(ret);
            if(st)
            {
                if(isValueType(ret))
                {
                    _out << nl << "ret__ = new " << retS << "();";
                }
                else
                {
                    _out << nl << "ret__ = null;";
                }
            }
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
            _out << nl << "exception_(ex__);";
            _out << nl << "return;";
            _out << eb;
        }
        _out << nl << "response_" << spar << args << epar << ';';
        _out << eb;
        
        _out << sp << nl << "protected override void exception__(Ice.Exception ex)";
        _out << sb;
        _out << nl << "exception_(ex);";
        _out << eb;

        _out << nl;
        _out << nl << "private AMI_" << cl->name() << "_" << name << "_response response_ = null;";
        _out << nl << "private AMI_" << cl->name() << "_" << name << "_exception exception_ = null;";
        _out << eb;
    }
}
